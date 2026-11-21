#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;
constexpr int MAX_WORDS = (5000 + 63) / 64;
using Mask = array<U64, MAX_WORDS>;

struct Evidence {
    struct Node {
        Mask zero{}, one{};
    };

    int base = 1, words;
    vector<Node> tree;

    Evidence(int n, int w) : words(w) {
        while (base <= n) base <<= 1;
        tree.resize(2 * base);
    }

    void add(int rank, const Mask& bits) {
        for (int j = 0; j < words; ++j) {
            U64 z = ~bits[j], o = bits[j];
            for (int p = base + rank; p; p >>= 1) {
                Node& a = tree[p];
                z &= ~a.zero[j];
                o &= ~a.one[j];
                if (!(z | o)) break;
                a.zero[j] |= z;
                a.one[j] |= o;
            }
        }
    }

    Mask candidates(int door, int bestRank, const Mask& best,
                    const Mask& unused) const {
        Mask c{};
        const Node& closed = tree[base + door];
        for (int j = 0; j < words; ++j) {
            U64 pos = (door == bestRank ? ~best[j] : best[j]);
            c[j] = unused[j] &
                   ~((pos & closed.one[j]) | (~pos & closed.zero[j]));
        }
        if (door < bestRank) {
            for (int p = base + door; p > 1; p >>= 1) {
                if (p & 1) continue;
                const Node& open = tree[p + 1];
                for (int j = 0; j < words; ++j) {
                    c[j] &= ~((best[j] & open.zero[j]) |
                              (~best[j] & open.one[j]));
                }
            }
        }
        return c;
    }

    Mask agreement(int rank, const Mask& unused) const {
        Mask m{};
        const Node& a = tree[base + rank];
        for (int j = 0; j < words; ++j)
            m[j] = unused[j] & ~(a.one[j] & a.zero[j]);
        return m;
    }
};

class Solver {
    int n, words;
    Evidence evidence;
    Mask unused{}, unassigned{}, correct{}, best{};
    int bestRank = -1;
    vector<int> doorOf, switchOf;
    set<int> remaining;
    vector<int> pool;
    string line;
    array<array<char, 8>, 256> digits{};
    U64 state;

    U64 randomWord() {
        U64 z = (state += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    static bool bit(const Mask& m, int i) {
        return (m[i >> 6] >> (i & 63)) & 1;
    }

    static void setBit(Mask& m, int i) {
        m[i >> 6] |= U64(1) << (i & 63);
    }

    static void clearBit(Mask& m, int i) {
        m[i >> 6] &= ~(U64(1) << (i & 63));
    }

    int countBits(const Mask& m) const {
        int k = 0;
        for (int j = 0; j < words; ++j) k += popcount(m[j]);
        return k;
    }

    int singleton(const Mask& m) const {
        int index = -1;
        for (int j = 0; j < words; ++j) {
            U64 b = m[j];
            if (!b) continue;
            if (index >= 0 || (b & (b - 1))) return -1;
            index = 64 * j + countr_zero(b);
        }
        return index;
    }

    int countBefore(int d) const {
        int k = 0;
        for (int j = 0; j < (d >> 6); ++j) k += popcount(unassigned[j]);
        if (d & 63)
            k += popcount(unassigned[d >> 6] & ((U64(1) << (d & 63)) - 1));
        return k;
    }

    void fix(int d, int s, bool value) {
        doorOf[s] = d;
        switchOf[d] = s;
        clearBit(unused, s);
        clearBit(unassigned, d);
        if (value) setBit(correct, s);
        remaining.erase(d);
    }

    void infer(int d) {
        if (d < 0 || d >= n || d > bestRank || switchOf[d] >= 0) return;
        int s = singleton(evidence.candidates(d, bestRank, best, unused));
        if (s >= 0) fix(d, s, bit(best, s) ^ (d == bestRank));
    }

    int ask(const Mask& m) {
        int bytes = (n + 7) / 8;
        for (int b = 0; b < bytes; ++b) {
            unsigned v = (m[b >> 3] >> (8 * (b & 7))) & 255;
            memcpy(line.data() + 2 + 8 * b, digits[v].data(), 8);
        }
        line[n + 2] = '\n';
        cout.write(line.data(), n + 3);
        cout.flush();

        int reply;
        if (!(cin >> reply)) exit(0);
        if (reply == -1) return n;
        if (reply < 0 || reply >= n) exit(0);
        return reply;
    }

    Mask subset(const Mask& c, int total, int take) {
        bool complement = take > total / 2;
        if (complement) take = total - take;
        pool.clear();
        for (int j = 0; j < words; ++j) {
            for (U64 b = c[j]; b; b &= b - 1)
                pool.push_back(64 * j + countr_zero(b));
        }

        Mask picked{};
        int size = total;
        while (take--) {
            int i = randomWord() % size;
            setBit(picked, pool[i]);
            pool[i] = pool[--size];
        }
        if (complement)
            for (int j = 0; j < words; ++j) picked[j] ^= c[j];
        return picked;
    }

    void finishFromOpen() {
        if (remaining.empty()) return;

        // With all positions known, a single flipped switch reveals its door.
        // Keep the earlier evidence to infer as many assignments as possible.
        vector<Mask> rows(n), cols(n);
        vector<int> rowCount(n), colCount(n);
        vector<int> forced;
        for (int d : remaining) {
            rows[d] = evidence.candidates(d, n, best, unused);
            rowCount[d] = countBits(rows[d]);
            if (rowCount[d] == 1) forced.push_back(d);
            for (int j = 0; j < words; ++j) {
                for (U64 b = rows[d][j]; b; b &= b - 1) {
                    int s = 64 * j + countr_zero(b);
                    setBit(cols[s], d);
                    ++colCount[s];
                }
            }
        }
        for (int s = 0; s < n; ++s)
            if (colCount[s] == 1) forced.push_back(n + s);

        auto assign = [&](int d, int s) {
            fix(d, s, bit(best, s));
            for (int j = 0; j < words; ++j) {
                for (U64 b = rows[d][j] & unused[j]; b; b &= b - 1) {
                    int t = 64 * j + countr_zero(b);
                    if (--colCount[t] == 1) forced.push_back(n + t);
                }
                for (U64 b = cols[s][j] & unassigned[j]; b; b &= b - 1) {
                    int e = 64 * j + countr_zero(b);
                    if (--rowCount[e] == 1) forced.push_back(e);
                }
            }
        };

        auto firstCommon = [&](const Mask& a, const Mask& b) {
            for (int j = 0; j < words; ++j)
                if (U64 x = a[j] & b[j]) return 64 * j + countr_zero(x);
            return -1;
        };

        size_t next = 0;
        while (!remaining.empty()) {
            if (next < forced.size()) {
                int id = forced[next++];
                if (id < n) {
                    if (switchOf[id] < 0 && rowCount[id] == 1)
                        assign(id, firstCommon(rows[id], unused));
                } else {
                    int s = id - n;
                    if (doorOf[s] < 0 && colCount[s] == 1)
                        assign(firstCommon(cols[s], unassigned), s);
                }
                continue;
            }

            int d = *remaining.begin();
            for (int e : remaining)
                if (rowCount[e] < rowCount[d]) d = e;
            int s = firstCommon(rows[d], unused);
            Mask query = best;
            query[s >> 6] ^= U64(1) << (s & 63);
            int reply = ask(query);
            if (reply == n) exit(0);
            assign(reply, s);
        }
    }

public:
    explicit Solver(int size)
        : n(size), words((n + 63) / 64), evidence(n, words),
          doorOf(n, -1), switchOf(n, -1), line(n + 10, ' '),
          state(chrono::steady_clock::now().time_since_epoch().count()) {
        pool.reserve(n);
        line[0] = '?';
        for (int i = 0; i < n; ++i) {
            setBit(unused, i);
            setBit(unassigned, i);
            remaining.insert(i);
        }
        for (int v = 0; v < 256; ++v)
            for (int b = 0; b < 8; ++b)
                digits[v][b] = char('0' + ((v >> b) & 1));
    }

    void run() {
        int active = -1, budget = 0;
        while (!remaining.empty()) {
            if (bestRank == n) {
                finishFromOpen();
                break;
            }

            int d = *remaining.begin();
            Mask query{};
            if (bestRank < d) {
                for (int j = 0; j < words; ++j)
                    query[j] = (randomWord() & unused[j]) | correct[j];
            } else {
                Mask c = evidence.candidates(d, bestRank, best, unused);
                int m = countBits(c);
                if (m == 1) {
                    fix(d, singleton(c), bit(best, singleton(c)) ^ (d == bestRank));
                    continue;
                }

                Mask eligible = evidence.agreement(bestRank, unused);
                int earlier = countBefore(bestRank);
                Mask otherEarly{};
                bool found = false;
                if (earlier <= 8 && d < bestRank) {
                    for (auto it = next(remaining.begin());
                         it != remaining.end() && *it < bestRank; ++it) {
                        int e = *it;
                        Mask a = evidence.candidates(e, bestRank, best, unused);
                        int s = singleton(a);
                        if (s >= 0) {
                            fix(e, s, bit(best, s));
                            found = true;
                            break;
                        }
                        for (int j = 0; j < words; ++j) otherEarly[j] |= a[j];
                    }
                    if (found) continue;
                } else if (earlier > 8) {
                    otherEarly = eligible;
                }

                int need = bit_width(unsigned(m - 1));
                if (active != d) {
                    active = d;
                    budget = need + (m >= 6);
                }
                budget = max(budget, need);

                int through = m - max(1, (m + 2) / 4);
                through = min(through, 1 << (budget - 1));
                int flips = (d == bestRank ? through : m - through);

                bool dense = d < bestRank &&
                    countBits(eligible) <= earlier + (switchOf[bestRank] < 0) + 1;
                if (dense) flips = 1;
                Mask flip = subset(c, m, flips);

                int sample = 1;
                while (sample < 7 && (1 << (sample + 1)) <= earlier - 1)
                    ++sample;

                for (int j = 0; j < words; ++j) {
                    if (d < bestRank && !dense) {
                        U64 spare = eligible[j] & ~c[j];
                        U64 safe = spare & ~otherEarly[j];
                        if (switchOf[bestRank] >= 0) safe &= randomWord();
                        flip[j] |= safe;

                        U64 overlap = spare & otherEarly[j];
                        if (flips > 1 && overlap) {
                            for (int p = 0; p < sample && overlap; ++p)
                                overlap &= randomWord();
                            flip[j] |= overlap;
                        }
                    }
                    query[j] = (((best[j] ^ flip[j]) & eligible[j]) |
                                (randomWord() & ~eligible[j])) & unused[j];
                    query[j] |= correct[j];
                }
                --budget;
            }

            int reply = ask(query);
            evidence.add(reply, query);
            int oldRank = bestRank;
            if (reply > bestRank) {
                bestRank = reply;
                best = query;
                infer(oldRank);
            }
            infer(reply);
        }

        string positions(n, '0');
        for (int s = 0; s < n; ++s)
            if (bit(correct, s)) positions[s] = '1';
        cout << "! " << positions;
        for (int d : doorOf) cout << ' ' << d;
        cout << '\n' << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    Solver(n).run();
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;
using Byte = unsigned char;

int N, totalTrue, fixedTrue = 0, unresolved;
string fixedSheet;

int submit(const string& sheet) {
    cout << sheet << '\n' << flush;
    int reply;
    if (!(cin >> reply) || reply < 0 || reply == N) exit(0);
    return reply;
}

// The indices in selected are unresolved. Every resolved answer is sent correctly.
int weigh(const vector<int>& selected) {
    string sheet = fixedSheet;
    for (int i : selected) sheet[i] = 'T';
    int reply = submit(sheet);
    return (reply - (N - totalTrue + fixedTrue) + int(selected.size())) / 2;
}

// Both a set and its complement in the region provide the same information.
// Prefer the complement if it can be a successful submission and the set cannot.
int weighIn(const vector<int>& region, int weight, const vector<Byte>& mask) {
    int size = accumulate(mask.begin(), mask.end(), 0);
    if (size == 0) return 0;
    if (size == int(region.size())) return weight;
    bool flip = size != weight && int(region.size()) - size == weight;
    vector<int> selected;
    for (int i = 0; i < int(region.size()); ++i)
        if (bool(mask[i]) != flip) selected.push_back(region[i]);
    int v = weigh(selected);
    return flip ? weight - v : v;
}

void record(const vector<int>& region, const vector<Byte>& values) {
    for (int i = 0; i < int(region.size()); ++i) {
        if (values[i]) {
            fixedSheet[region[i]] = 'T';
            ++fixedTrue;
        }
    }
    unresolved -= int(region.size());
}

struct SmallGame {
    int m, k;
    vector<int> keys;
    vector<array<U64, 8>> buckets;
    vector<U64> hits;
    unordered_map<U64, int> memo[2][9];

    SmallGame(int m_, int k_) : m(m_), k(k_) {
        int lim = 1 << m;
        for (int s = 0; s < lim; ++s)
            if (__builtin_popcount(unsigned(s)) == k) keys.push_back(s);
        buckets.resize(lim);
        hits.resize(lim);
        for (int q = 0; q < lim; ++q) {
            for (int j = 0; j < int(keys.size()); ++j) {
                U64 bit = U64(1) << j;
                buckets[q][__builtin_popcount(unsigned(q & keys[j]))] |= bit;
                if (q == keys[j]) hits[q] = bit;
            }
        }
    }

    // -1: impossible within depth; -2: there is nothing left to identify.
    int choose(U64 state, int depth, bool finish) {
        if (!state) return -2;
        int cnt = __builtin_popcountll(state);
        if (cnt == 1)
            return finish ? (depth ? keys[__builtin_ctzll(state)] : -1) : -2;
        if (depth == 0 || (finish && depth == 1)) return -1;

        auto& mem = memo[finish][depth];
        auto it = mem.find(state);
        if (it != mem.end()) return it->second;

        vector<tuple<int, int, int>> options;
        for (int q = 1; q + 1 < (1 << m); ++q) {
            int opposite = ((1 << m) - 1) ^ q;
            if ((q & 1) && (!finish || (!hits[q] && !hits[opposite]))) continue;
            U64 rest = finish ? state & ~hits[q] : state;
            int largest = 0, squares = 0;
            for (int j = 0; j <= k; ++j) {
                int c = __builtin_popcountll(rest & buckets[q][j]);
                largest = max(largest, c);
                squares += c * c;
            }
            if (largest == cnt || (depth == 1 && largest > 1)) continue;
            options.emplace_back(largest, squares, q);
        }
        sort(options.begin(), options.end());
        for (auto [largest, squares, q] : options) {
            U64 rest = finish ? state & ~hits[q] : state;
            bool good = true;
            for (int j = 0; j <= k; ++j) {
                U64 next = rest & buckets[q][j];
                if (choose(next, depth - 1, finish) == -1) {
                    good = false;
                    break;
                }
            }
            if (good) return mem[state] = q;
        }
        return mem[state] = -1;
    }

    void play(const vector<int>& region) {
        bool finish = unresolved == m;
        U64 state = (U64(1) << keys.size()) - 1;
        while (state & (state - 1)) {
            int q = -1;
            for (int depth = 1; q < 0 && depth <= m + int(finish); ++depth)
                q = choose(state, depth, finish);
            vector<int> selected;
            for (int i = 0; i < m; ++i)
                if ((q >> i) & 1) selected.push_back(region[i]);
            int reply = weigh(selected);
            state &= buckets[q][reply];
        }
        int answer = keys[__builtin_ctzll(state)];
        vector<Byte> values(m);
        for (int i = 0; i < m; ++i) values[i] = (answer >> i) & 1;
        record(region, values);
    }
};

unique_ptr<SmallGame> small[8][8];

mt19937 rng(0x4b1d72e9);
vector<vector<int>> seeds(12);

// The first digit records cardinality. Other digits record the binary rows.
// A new column is accepted only if the shifted and unshifted subset sums
// are disjoint, so each returned seed is a certified detecting matrix.
vector<int> findSeed(int d, int budget) {
    int universe = 1 << d;
    vector<U64> encoded(universe, 1);
    for (int mask = 0; mask < universe; ++mask)
        for (int j = 0; j < d; ++j)
            if ((mask >> j) & 1) encoded[mask] |= U64(1) << (5 * (j + 1));

    vector<int> cur{0}, best{0};
    for (int round = 0; round < 480 && budget > 0; ++round) {
        int r = int(cur.size());
        int count = 1 << r;
        if (r >= 21 || budget < 2 * count) break;
        budget -= 2 * count;

        vector<U64> sums(count);
        for (int i = 0; i < r; ++i) {
            int bit = 1 << i;
            for (int j = 0; j < bit; ++j) sums[bit + j] = sums[j] + encoded[cur[i]];
        }

        int tableMask = 4 * count - 1;
        int shift = 64 - r - 2;
        vector<U64> table(4 * count);
        auto indexOf = [shift](U64 x) {
            x ^= x >> 23;
            return int((x * 0x9e3779b97f4a7c15ULL) >> shift);
        };
        for (U64 x : sums) {
            U64 key = x + 1;
            int i = indexOf(key);
            while (table[i]) i = (i + 1) & tableMask;
            table[i] = key;
        }

        int candidate = int(rng() & (universe - 1));
        int step = int(rng() & (universe - 1)) | 1;
        int start = int(rng() & (count - 1));
        int stride = int(rng() & (count - 1)) | 1;
        bool added = false;
        for (int t = 0; t < universe && budget > 0; ++t) {
            U64 delta = encoded[candidate] + 1;
            int at = start;
            int j = 0;
            for (; j < count && budget > 0; ++j) {
                --budget;
                U64 key = sums[at] + delta;
                int i = indexOf(key);
                while (table[i] && table[i] != key) i = (i + 1) & tableMask;
                if (table[i]) break;
                at = (at + stride) & (count - 1);
            }
            if (j == count) {
                cur.push_back(candidate);
                if (cur.size() > best.size()) best = cur;
                added = true;
                break;
            }
            candidate = (candidate + step) & (universe - 1);
        }
        if (!added) {
            if (round % 19 == 0) cur = {0};
            else if (round % 7 == 0) cur = best;
            int drop = 1 + int(rng() % 4);
            while (drop-- && cur.size() > 1) {
                int j = 1 + int(rng() % (cur.size() - 1));
                cur[j] = cur.back();
                cur.pop_back();
            }
        }
    }
    return best;
}

vector<int> capacity, paired, rowsFor;
vector<vector<unsigned short>> cost, cutAt;
vector<double> logFact;

void prepare() {
    seeds[1] = {0};
    seeds[4] = {0, 3, 5, 6, 7};

    int plainCapacity = 1, plainRows = 1;
    while (plainCapacity < N)
        plainCapacity += __builtin_popcount(unsigned(plainRows++));
    const int budgets[] = {450000, 1100000, 2400000, 4500000,
                           7500000, 9500000, 12000000};
    for (int d = 4; d <= 10 && d + 1 < plainRows; ++d)
        seeds[d + 1] = findSeed(d, budgets[d - 4]);

    capacity.assign(N + 1, 0);
    paired.assign(N + 1, 0);
    rowsFor.assign(N + 1, 0);
    int filled = 0;
    for (int m = 1; filled < N; ++m) {
        if (m < int(seeds.size())) capacity[m] = int(seeds[m].size());
        for (int b = m / 2; b >= 1; --b) {
            int a = m - b;
            int value = capacity[a] + capacity[b] + b - 1;
            if (value > capacity[m]) {
                capacity[m] = value;
                paired[m] = b;
            }
        }
        while (filled < min(N, capacity[m])) rowsFor[++filled] = m;
    }

    cost.assign(N + 1, {});
    cutAt.assign(N + 1, {});
    logFact.assign(N + 1, 0);
    for (int len = 1; len <= N; ++len) {
        logFact[len] = logFact[len - 1] + log(double(len));
        cost[len].assign(len + 1, 0);
        cutAt[len].assign(len + 1, 0);
        vector<int> cuts;
        auto add = [&](int a) {
            a = min(a, len - a);
            if (a > 0 && find(cuts.begin(), cuts.end(), a) == cuts.end())
                cuts.push_back(a);
        };
        add(len / 2);
        if (len <= 18) {
            for (int a = len / 2 - 1; a >= 1; --a) add(a);
        } else {
            int p = 1 << (31 - __builtin_clz(unsigned(len - 1)));
            add(p);
            add(len / 3);
            add(len / 4);
            add(len * 2 / 5);
        }

        for (int k = 1; k <= len / 2; ++k) {
            int best = rowsFor[len] - 1;
            int bestCut = 0;
            for (int a : cuts) {
                int b = len - a;
                int worst = 0;
                for (int j = max(0, k - b); j <= min(k, a); ++j) {
                    worst = max(worst, int(cost[a][j]) + int(cost[b][k - j]));
                    if (worst >= best) break;
                }
                if (worst + 1 < best || (worst + 1 == best && !bestCut)) {
                    best = worst + 1;
                    bestCut = a;
                }
            }
            cost[len][k] = cost[len][len - k] = best;
            cutAt[len][k] = cutAt[len][len - k] = bestCut;
        }
    }
}

struct Matrix {
    int n, m, extras = 0;
    vector<vector<Byte>> row;
    unique_ptr<Matrix> x, z;

    Matrix(int m_, int n_) : n(n_), m(m_), row(m, vector<Byte>(n)) {
        if (n <= 1) {
            if (n) row[0][0] = 1;
            return;
        }
        int b = paired[m];
        if (!b) {
            fill(row[0].begin(), row[0].end(), 1);
            for (int i = 1; i < m; ++i)
                for (int j = 0; j < n; ++j)
                    row[i][j] = (seeds[m][j] >> (i - 1)) & 1;
            return;
        }
        int a = m - b;
        extras = max(0, n - capacity[a] - capacity[b]);
        int nz = min(capacity[b], n - extras - 1);
        int nx = n - extras - nz;
        x = make_unique<Matrix>(a, nx);
        z = make_unique<Matrix>(b, nz);

        // The paired difference for row i>0 is 2*B_i*z - |z| + e_i.
        // Row 0 supplies |z|, and the low bits supply all the e_i.
        for (int i = 0; i < a; ++i) {
            copy(x->row[i].begin(), x->row[i].end(), row[i].begin());
            if (i >= b) continue;
            copy(x->row[i].begin(), x->row[i].end(), row[a + i].begin());
            for (int j = 0; j < nz; ++j) {
                row[i][nx + j] = z->row[i][j];
                row[a + i][nx + j] = 1 - z->row[i][j];
            }
        }
        for (int j = 0; j < extras; ++j) {
            int col = nx + nz + j;
            row[0][col] = row[a][col] = 1;
            row[j + 1][col] = 1;
        }
    }

    void decode(const vector<int>& s, vector<Byte>& answer, int off) const {
        if (n <= 1) {
            if (n) answer[off] = s[0];
            return;
        }
        if (!x) {
            vector<U64> col(n);
            U64 target = 0;
            for (int i = 0; i < m; ++i) {
                target |= U64(s[i]) << (5 * i);
                for (int j = 0; j < n; ++j)
                    col[j] |= U64(row[i][j]) << (5 * i);
            }
            int half = n / 2;
            vector<pair<U64, int>> left(1 << half);
            for (int mask = 1; mask < int(left.size()); ++mask) {
                int bit = __builtin_ctz(unsigned(mask));
                left[mask] = {left[mask & (mask - 1)].first + col[bit], mask};
            }
            sort(left.begin(), left.end());
            vector<U64> right(1 << (n - half));
            for (int mask = 0; mask < int(right.size()); ++mask) {
                if (mask) {
                    int bit = __builtin_ctz(unsigned(mask));
                    right[mask] = right[mask & (mask - 1)] + col[half + bit];
                }
                if (right[mask] > target) continue;
                U64 want = target - right[mask];
                auto it = lower_bound(left.begin(), left.end(), make_pair(want, 0));
                if (it != left.end() && it->first == want) {
                    for (int j = 0; j < half; ++j) answer[off + j] = (it->second >> j) & 1;
                    for (int j = half; j < n; ++j) answer[off + j] = (mask >> (j - half)) & 1;
                    return;
                }
            }
            abort(); // A valid reply sequence always has a match.
        }
        int a = x->m, b = z->m;
        vector<int> sx(a), sz(b);
        sz[0] = s[0] - s[a];
        sx[0] = s[a];
        for (int i = 1; i < b; ++i) {
            int value = s[i] - s[a + i] + sz[0];
            int bit = (i <= extras) ? (value & 1) : 0;
            if (i <= extras) answer[off + x->n + z->n + i - 1] = bit;
            sz[i] = (value - bit) / 2;
            sx[i] = s[i] - sz[i] - bit;
            sx[0] -= bit;
        }
        for (int i = b; i < a; ++i) sx[i] = s[i];
        x->decode(sx, answer, off);
        z->decode(sz, answer, off + x->n);
    }
};

void solve(vector<int> region, int k);

void solveParts(vector<int> p, int kp, vector<int> q, int kq) {
    auto known = [](const vector<int>& v, int k) { return k == 0 || k == int(v.size()); };
    if (known(p, kp)) {
        solve(move(p), kp);
        solve(move(q), kq);
    } else if (known(q, kq)) {
        solve(move(q), kq);
        solve(move(p), kp);
    } else {
        auto uncertainty = [](int len, int k) {
            return logFact[len] - logFact[k] - logFact[len - k];
        };
        if (uncertainty(int(p.size()), kp) < uncertainty(int(q.size()), kq)) {
            swap(p, q);
            swap(kp, kq);
        }
        solve(move(p), kp);
        solve(move(q), kq);
    }
}

void solve(vector<int> region, int k) {
    int n = int(region.size());
    if (k == 0 || k == n) {
        record(region, vector<Byte>(n, k != 0));
        return;
    }
    if (n <= 7) {
        if (!small[n][k]) small[n][k] = make_unique<SmallGame>(n, k);
        small[n][k]->play(region);
        return;
    }

    int cut = cutAt[n][k];
    if (cut) {
        vector<Byte> mask(n);
        fill(mask.begin(), mask.begin() + cut, 1);
        int w = weighIn(region, k, mask);
        vector<int> p(region.begin(), region.begin() + cut);
        vector<int> q(region.begin() + cut, region.end());
        solveParts(move(p), w, move(q), k - w);
        return;
    }

    Matrix mat(rowsFor[n], n);
    vector<int> replies(mat.m, -1);
    replies[0] = k;
    vector<vector<int>> groups(1);
    vector<int> weights(1, k);
    if (mat.x) {
        int a = mat.x->m;
        int nx = mat.x->n, nz = mat.z->n;
        replies[a] = weighIn(region, k, mat.row[a]);
        int wz = k - replies[a];

        if (int(cost[nz][wz]) + int(cost[n - nz][k - wz]) <= mat.m - 2) {
            vector<int> p(region.begin() + nx, region.begin() + nx + nz);
            vector<int> q(region.begin(), region.begin() + nx);
            q.insert(q.end(), region.begin() + nx + nz, region.end());
            solveParts(move(p), wz, move(q), k - wz);
            return;
        }

        groups.resize(2);
        weights = {k - wz, wz};
        for (int j = 0; j < n; ++j) groups[j >= nx && j < nx + nz].push_back(j);
    } else {
        groups[0].resize(n);
        iota(groups[0].begin(), groups[0].end(), 0);
    }

    // Remove contributions already implied by group weights, and reuse
    // measurements for identical or complementary remaining selections.
    map<string, int> measured;
    for (int i = 1; i < mat.m; ++i) {
        if (replies[i] >= 0) continue;
        const auto& row = mat.row[i];
        vector<int> active;
        int constant = 0, activeWeight = 0, selectedCount = 0;
        for (int g = 0; g < int(groups.size()); ++g) {
            int count = 0;
            for (int j : groups[g]) count += row[j];
            if (count == 0 || weights[g] == 0) continue;
            if (count == int(groups[g].size())) constant += weights[g];
            else if (weights[g] == int(groups[g].size())) constant += count;
            else {
                active.insert(active.end(), groups[g].begin(), groups[g].end());
                activeWeight += weights[g];
                selectedCount += count;
            }
        }
        if (active.empty()) {
            replies[i] = constant;
            continue;
        }
        bool flip = row[active[0]];
        string key(n, '0');
        for (int j : active) if (bool(row[j]) != flip) key[j] = '1';
        auto it = measured.find(key);
        int value;
        if (it != measured.end()) {
            value = flip ? activeWeight - it->second : it->second;
        } else {
            bool sendFlipped = selectedCount != activeWeight &&
                               int(active.size()) - selectedCount == activeWeight;
            vector<int> selected;
            for (int j : active)
                if (bool(row[j]) != sendFlipped) selected.push_back(region[j]);
            value = weigh(selected);
            if (sendFlipped) value = activeWeight - value;
            measured.emplace(move(key), flip ? activeWeight - value : value);
        }
        replies[i] = constant + value;
    }
    vector<Byte> values(n);
    mat.decode(replies, values, 0);
    record(region, values);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N)) return 0;
    fixedSheet.assign(N, 'F');
    unresolved = N;
    totalTrue = N - submit(fixedSheet);

    if (N > 7 && totalTrue != N) prepare();
    vector<int> all(N);
    iota(all.begin(), all.end(), 0);
    solve(move(all), totalTrue);
    submit(fixedSheet);
    return 0;
}

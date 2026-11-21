#include <bits/stdc++.h>
using namespace std;

int n, lim;
mt19937 rng(712367821);
const int fact[] = {1, 1, 2, 6, 24, 120, 720, 5040, 40320};

int ask(int i, int j) {
    cout << "? " << i << ' ' << j << endl;
    int r;
    if (!(cin >> r) || r < 0) exit(0);
    return r;
}

void report(const vector<int>& p) {
    int count = 0;
    for (int d = 0; d < (n & -n); ++d) {
        bool ok = true;
        for (int i = 0; i < n; ++i) {
            if (p[i ^ d] != (p[i] ^ d)) {
                ok = false;
                break;
            }
        }
        count += ok;
    }
    cout << '!' << endl;
    cout << count << endl;
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << p[i];
    }
    cout << endl;
}

struct Endgame {
    struct State {
        vector<uint16_t> a, b;
        int shift;
    };
    vector<State> states;
    unordered_map<uint64_t, vector<int>> hashes;

    void add(const vector<int>& p) {
        State s{vector<uint16_t>(n), vector<uint16_t>(n), 0};
        for (int i = 0; i < n; ++i)
            if (p[i] == 0) s.shift = i;
        for (int i = 0; i < n; ++i) {
            s.a[i] = p[i] ^ s.shift;
            s.b[p[i]] = i ^ s.shift;
        }
        uint64_t h = 0xcbf29ce484222325ULL;
        for (int i = 0; i < n; ++i)
            h = (h ^ (uint64_t(s.a[i]) * lim + s.b[i])) * 1099511628211ULL;
        auto& bucket = hashes[h];
        for (int id : bucket)
            if (s.a == states[id].a && s.b == states[id].b) return;
        bucket.push_back((int)states.size());
        states.push_back(move(s));
    }

    void complete(vector<int> p, bool inverted = false) {
        vector<int> missing, values, used(n, 0), inv;
        for (int i = 0; i < n; ++i) {
            if (p[i] < 0) missing.push_back(i);
            else used[p[i]] = 1;
        }
        for (int j = 0; j < n; ++j)
            if (!used[j]) values.push_back(j);
        if (inverted) inv.resize(n);
        do {
            for (int i = 0; i < (int)missing.size(); ++i)
                p[missing[i]] = values[i];
            if (inverted) {
                for (int i = 0; i < n; ++i) inv[p[i]] = i;
                add(inv);
            } else {
                add(p);
            }
        } while (next_permutation(values.begin(), values.end()));
    }

    vector<int> solve() {
        int m = (int)states.size();
        if (m == 1) {
            vector<int> p(n);
            for (int i = 0; i < n; ++i) p[i] = states[0].a[i] ^ states[0].shift;
            return p;
        }
        vector<uint16_t> a(size_t(n) * m), b(size_t(n) * m);
        vector<int> shift(m), ids(m), ri{0}, cj{0};
        iota(ids.begin(), ids.end(), 0);
        for (int base = 0; base < n; base += 32) {
            for (int s = 0; s < m; ++s) {
                for (int i = base; i < min(n, base + 32); ++i) {
                    a[size_t(i) * m + s] = states[s].a[i];
                    b[size_t(i) * m + s] = states[s].b[i];
                }
            }
        }
        for (int s = 0; s < m; ++s) shift[s] = states[s].shift;
        states.clear();
        hashes.clear();
        for (int i = 1; i < n; ++i) {
            const uint16_t* aa = &a[size_t(i) * m];
            const uint16_t* bb = &b[size_t(i) * m];
            for (int s = 1; s < m; ++s) {
                if (aa[s] != aa[0]) {
                    ri.push_back(i);
                    break;
                }
            }
            for (int s = 1; s < m; ++s) {
                if (bb[s] != bb[0]) {
                    cj.push_back(i);
                    break;
                }
            }
        }

        vector<int> stamp(lim), hist(lim);
        int tick = 0;
        while (ids.size() > 1) {
            int k = (int)ids.size();
            int w1 = ids.front(), w2 = ids.back();
            int wi = 0, wj = 0;
            if (a[w1] == a[w2]) {
                while (wi < n && a[size_t(wi) * m + w1] == a[size_t(wi) * m + w2])
                    ++wi;
                if (wi == n) {
                    wi = 0;
                    while (b[size_t(wj) * m + w1] == b[size_t(wj) * m + w2]) ++wj;
                }
            }
            int budget = clamp(110000 / k, 20, 650);
            vector<int> rows = ri, cols = cj;
            if (k <= 12 && n > 7) {
                auto compress = [&](const vector<int>& inds, const vector<uint16_t>& mat) {
                    set<vector<uint16_t>> seen;
                    vector<int> out;
                    for (int i : inds) {
                        const uint16_t* z = &mat[size_t(i) * m];
                        vector<uint16_t> key;
                        key.reserve(k - 1);
                        for (int h = 1; h < k; ++h)
                            key.push_back(z[ids[h]] ^ z[ids[0]]);
                        if (seen.insert(move(key)).second) out.push_back(i);
                    }
                    return out;
                };
                rows = compress(ri, a);
                cols = compress(cj, b);
            }

            vector<pair<int, int>> questions;
            questions.emplace_back(wi, wj);
            if (n <= 7 || (long long)rows.size() * cols.size() <= budget) {
                for (int i : rows)
                    for (int j : cols)
                        if (i != wi || j != wj) questions.emplace_back(i, j);
            } else {
                unordered_set<int> seen;
                seen.insert(wi * n + wj);
                auto put = [&](int i, int j) {
                    if (seen.insert(i * n + j).second) questions.emplace_back(i, j);
                };
                int axes = (int)rows.size() + (int)cols.size() - 1;
                if (axes <= budget / 2) {
                    for (int i : rows) put(i, 0);
                    for (int j : cols) put(0, j);
                }
                for (int t = 0; t < 3 * budget && (int)questions.size() < budget; ++t) {
                    int i = rows[rng() % rows.size()];
                    int j = cols[rng() % cols.size()];
                    if (t % 4 == 1) i = 0;
                    if (t % 4 == 2) j = 0;
                    put(i, j);
                }
            }

            pair<int, int> best = questions[0];
            int bestMax = k + 1;
            long long bestSq = LLONG_MAX;
            bool exact = k <= 12;
            vector<pair<vector<int>, pair<int, int>>> partitions;
            set<vector<int>> usedPartitions;
            for (auto [i, j] : questions) {
                const uint16_t* x = &a[size_t(i) * m];
                const uint16_t* y = &b[size_t(j) * m];
                ++tick;
                int mx = 0;
                long long sq = 0;
                if (exact) {
                    vector<int> touched;
                    for (int t = 0; t < k; ++t) {
                        int r = x[ids[t]] ^ y[ids[t]];
                        if (stamp[r] != tick) {
                            stamp[r] = tick;
                            hist[r] = 0;
                            touched.push_back(r);
                        }
                        hist[r] |= 1 << t;
                    }
                    if (touched.size() == 1) continue;
                    if ((int)touched.size() == k) {
                        best = {i, j};
                        bestMax = 1;
                        break;
                    }
                    vector<int> groups;
                    for (int r : touched) {
                        groups.push_back(hist[r]);
                        int c = popcount((unsigned)hist[r]);
                        mx = max(mx, c);
                        sq += c * c;
                    }
                    sort(groups.begin(), groups.end());
                    if (usedPartitions.insert(groups).second)
                        partitions.emplace_back(move(groups), make_pair(i, j));
                } else {
                    for (int s : ids) {
                        int r = x[s] ^ y[s];
                        if (stamp[r] != tick) {
                            stamp[r] = tick;
                            hist[r] = 0;
                        }
                        int c = ++hist[r];
                        mx = max(mx, c);
                        sq += 2 * c - 1;
                    }
                }
                if (mx < bestMax || (mx == bestMax && sq < bestSq)) {
                    best = {i, j};
                    bestMax = mx;
                    bestSq = sq;
                }
                if (bestMax == 1) break;
            }

            if (exact && bestMax > 1 && !partitions.empty()) {
                vector<pair<int, int>> memo(1 << k, {-1, 0});
                pair<int, int> opt{INT_MAX, INT_MAX};
                auto dp = [&](auto&& self, int mask) -> pair<int, int> {
                    int cnt = popcount((unsigned)mask);
                    if (cnt <= 1) return {0, 0};
                    if (memo[mask].first != -1) return memo[mask];
                    pair<int, int> val{INT_MAX, INT_MAX};
                    for (auto& [groups, q] : partitions) {
                        int worst = 0, sum = cnt;
                        bool split = true;
                        for (int g : groups) {
                            int z = mask & g;
                            if (!z) continue;
                            if (z == mask) {
                                split = false;
                                break;
                            }
                            auto [d, s] = self(self, z);
                            worst = max(worst, d + 1);
                            sum += s;
                            if (worst > val.first) {
                                split = false;
                                break;
                            }
                        }
                        if (split && make_pair(worst, sum) < val) {
                            val = {worst, sum};
                            if (mask == (1 << k) - 1 && val < opt) {
                                opt = val;
                                best = q;
                            }
                            if (worst == 1) break;
                        }
                    }
                    return memo[mask] = val;
                };
                dp(dp, (1 << k) - 1);
            }

            auto [i, j] = best;
            int r = ask(i, j);
            const uint16_t* x = &a[size_t(i) * m];
            const uint16_t* y = &b[size_t(j) * m];
            ids.erase(remove_if(ids.begin(), ids.end(),
                                [&](int s) { return (x[s] ^ y[s]) != r; }),
                      ids.end());
        }
        int s = ids[0];
        vector<int> p(n);
        for (int i = 0; i < n; ++i) p[i] = a[size_t(i) * m + s] ^ shift[s];
        return p;
    }
};

vector<int> knownShift(vector<int> p, vector<int> b) {
    int iteration = 0;
    while (true) {
        vector<int> u, v;
        for (int i = 0; i < n; ++i) {
            if (p[i] < 0) u.push_back(i);
            if (b[i] < 0) v.push_back(i);
        }
        int k = (int)u.size();
        if (k <= 1) {
            if (k == 1) p[u[0]] = v[0];
            return p;
        }
        if (k <= 6) {
            Endgame game;
            game.complete(p);
            return game.solve();
        }

        bool flip = (iteration++ & 1);
        if (flip) swap(u, v);
        auto& f = flip ? b : p;
        auto& inv = flip ? p : b;
        auto query = [&](int i, int j) { return flip ? ask(j, i) : ask(i, j); };

        shuffle(u.begin(), u.end(), rng);
        shuffle(v.begin(), v.end(), rng);
        int pivot = u[0], first = v[0];
        if (k <= 210) {
            vector<int> freq(lim);
            for (int i : u)
                for (int j : v) ++freq[i ^ j];
            int rare = k + 1;
            vector<pair<int, int>> pairs;
            for (int i : u) {
                for (int j : v) {
                    int t = freq[i ^ j];
                    if (t < rare) {
                        rare = t;
                        pairs.clear();
                    }
                    if (t == rare) pairs.emplace_back(i, j);
                }
            }
            int best = INT_MIN;
            int trials = min(36, (int)pairs.size());
            for (int t = 0; t < trials; ++t) {
                int at = t + rng() % (pairs.size() - t);
                swap(pairs[t], pairs[at]);
                auto [i, j] = pairs[t];
                int score = 0;
                auto dec = [&](int r) {
                    int c = freq[r]--;
                    score += min(c, 30);
                    if (c == 2) score += 8;
                    if (c == 1) score -= 8;
                };
                for (int x : u) if (x != i) dec(x ^ j);
                for (int y : v) if (y != j) dec(i ^ y);
                for (int x : u) if (x != i) ++freq[x ^ j];
                for (int y : v) if (y != j) ++freq[i ^ y];
                if (score > best) {
                    best = score;
                    pivot = i;
                    first = j;
                }
            }
        }

        vector<int> cand = v;
        vector<char> asked(n), inCand(n);
        vector<pair<int, int>> obs;
        int chosen = -1;
        while (cand.size() > 1) {
            int q = (int)obs.size();
            bool risk = false;
            fill(inCand.begin(), inCand.end(), 0);
            for (int x : cand) {
                inCand[x] = 1;
                risk |= bool(asked[x]);
            }

            if (k - q <= 7) {
                int total = 0;
                for (int x : cand) total += fact[k - q - 1 + asked[x]];
                if (total <= 720 || k - q <= 1) {
                    Endgame game;
                    for (int x : cand) {
                        vector<int> partial = f;
                        partial[pivot] = x;
                        for (auto [j, r] : obs) partial[r ^ x] = j;
                        game.complete(move(partial), flip);
                    }
                    return game.solve();
                }
            }

            int j = -1;
            if (q == 0) {
                j = first;
            } else {
                for (int x : v) {
                    if (!asked[x] && !inCand[x]) {
                        j = x;
                        break;
                    }
                }
                int cap = cand.size() <= 8 ? 48 : 24;
                if (!risk && (q >= cap || (j == -1 && q >= 4 && cand.size() > 8))) {
                    int anchor = 0;
                    while (inv[anchor] < 0) ++anchor;
                    chosen = query(pivot, anchor) ^ inv[anchor];
                    break;
                }
                if (j == -1) {
                    for (int x : v) if (!asked[x]) {
                        j = x;
                        break;
                    }
                }
            }

            int r = query(pivot, j);
            obs.emplace_back(j, r);
            asked[j] = 1;
            cand.erase(remove_if(cand.begin(), cand.end(), [&](int x) {
                int i = r ^ x;
                return i >= n || f[i] >= 0 || ((i == pivot) != (x == j));
            }), cand.end());
        }
        if (chosen < 0) chosen = cand[0];
        f[pivot] = chosen;
        inv[chosen] = pivot;
        for (auto [j, r] : obs) {
            int i = r ^ chosen;
            f[i] = j;
            inv[j] = i;
        }
    }
}

vector<int> solve() {
    if (n <= 2) {
        vector<int> p(n);
        iota(p.begin(), p.end(), 0);
        return p;
    }
    if (n <= 7) {
        Endgame game;
        game.complete(vector<int>(n, -1));
        return game.solve();
    }

    vector<int> a(n, -1), d(n, -1), ra(lim, -1), rd(lim, -1);
    a[0] = ask(0, 0);
    d[0] = 0;
    ra[a[0]] = rd[0] = 0;

    struct Hypothesis { int x, known; };
    vector<Hypothesis> hs;
    for (int x = 0; x < n; ++x) {
        int y = a[0] ^ x;
        if (y < n && ((y == 0) == (x == 0)))
            hs.push_back({x, 1 + (x != 0)});
    }
    vector<int> todo[2], done[2], pos[2];
    for (int t = 0; t < 2; ++t) {
        pos[t].resize(n);
        done[t].push_back(0);
        for (int i = 1; i < n; ++i) {
            pos[t][i] = (int)todo[t].size();
            todo[t].push_back(i);
        }
    }

    auto partial = [&](int x) {
        vector<int> p(n, -1);
        for (int i : done[0]) p[i] = a[i] ^ x;
        for (int j : done[1]) p[d[j] ^ x] = j;
        return p;
    };

    bool translation = a[0] == 0;
    int warmup = int(3.2 * sqrt(n)) + 8;
    vector<int> seen(lim), count(lim);
    int stamp = 0;
    for (int steps = 1; ; ++steps) {
        int sz = (int)hs.size();
        if (sz == 1) {
            vector<int> p = partial(hs[0].x), b(n, -1);
            for (int i = 0; i < n; ++i)
                if (p[i] >= 0) b[p[i]] = i;
            return knownShift(move(p), move(b));
        }
        int sum = 0;
        bool allReady = true;
        for (auto h : hs) {
            int k = n - h.known;
            allReady &= (k <= 1);
            sum = min(1601, sum + fact[min(8, k)]);
        }
        if (allReady || sum <= 1600) {
            Endgame game;
            for (auto h : hs) game.complete(partial(h.x));
            return game.solve();
        }

        int bestSide = -1, bestIndex = -1;
        double bestScore = -1e100;
        int balanceSide = done[0].size() <= done[1].size() ? 0 : 1;
        int mainSide = done[0].size() >= done[1].size() ? 0 : 1;
        bool warm = steps < warmup && !translation;
        double lambda = sz <= 12 ? 2.8 : (warm ? 1.90 : 1.42);

        auto examine = [&](int t, int i) {
            if (i == 0 || (t == 0 ? a[i] : d[i]) >= 0) return;
            const vector<int>& rev = t == 0 ? rd : ra;
            int fixed = 0, largest = 0;
            ++stamp;
            for (auto h : hs) {
                int v = rev[i ^ h.x];
                if (v < 0) continue;
                int key = v ^ h.x;
                ++fixed;
                if (seen[key] != stamp) {
                    seen[key] = stamp;
                    count[key] = 0;
                }
                largest = max(largest, ++count[key]);
            }
            if (largest == sz) return;
            double f = double(fixed) / sz;
            double score = 1.0 - f + lambda * (f * (1.0 - f) + double(fixed - largest) / sz);
            if (warm && t == balanceSide) score += 0.12;
            if (!warm && t == mainSide) score += 0.045;
            if (translation && t == 0) score += 0.24;
            if (score > bestScore) {
                bestScore = score;
                bestSide = t;
                bestIndex = i;
            }
        };

        for (int t = 0; t < 2; ++t) {
            int len = (int)todo[t].size();
            if (!len) continue;
            int samples = sz > 800 ? 10 : (sz > 40 ? 18 : 44);
            if (len <= samples) {
                for (int i : todo[t]) examine(t, i);
            } else {
                for (int s = 0; s < samples; ++s)
                    examine(t, todo[t][rng() % len]);
                if (!translation) {
                    for (int s = 0; s < samples / 2; ++s) {
                        int x = hs[rng() % sz].x;
                        int j = done[t ^ 1][rng() % done[t ^ 1].size()];
                        int i = (t == 0 ? d[j] : a[j]) ^ x;
                        if (i < n) examine(t, i);
                    }
                }
            }
        }
        if (bestSide < 0) {
            for (int t = 0; t < 2 && bestSide < 0; ++t)
                for (int i : todo[t]) {
                    examine(t, i);
                    if (bestSide >= 0) break;
                }
        }

        int t = bestSide, i = bestIndex;
        int r = t == 0 ? ask(i, 0) : (ask(0, i) ^ a[0]);
        translation &= r == i;
        const auto& rev = t == 0 ? rd : ra;
        const auto& opposite = t == 0 ? d : a;
        int w = 0;
        for (auto h : hs) {
            int value = r ^ h.x;
            if (value >= n) continue;
            int already = rev[i ^ h.x];
            if (already >= 0) {
                if (already != value) continue;
            } else {
                if (opposite[value] >= 0) continue;
                ++h.known;
            }
            hs[w++] = h;
        }
        hs.resize(w);

        if (t == 0) {
            a[i] = r;
            ra[r] = i;
        } else {
            d[i] = r;
            rd[r] = i;
        }
        int last = todo[t].back();
        todo[t][pos[t][i]] = last;
        pos[t][last] = pos[t][i];
        todo[t].pop_back();
        done[t].push_back(i);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n) || n < 1) return 0;
    lim = 1;
    while (lim < n) lim <<= 1;
    report(solve());
    return 0;
}

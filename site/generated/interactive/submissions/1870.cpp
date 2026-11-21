#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 1000;
constexpr int SMALL = 9;
constexpr int INF = 10000;
using Bits = bitset<MAXN>;

string ask(const vector<int>& d) {
    cout << '?';
    for (int x : d) cout << ' ' << x;
    cout << endl;
    string s;
    if (!(cin >> s) || s.size() != d.size() ||
        s.find_first_not_of("01") != string::npos) {
        exit(0);
    }
    return s;
}

void report(const vector<pair<int, int>>& edges) {
    cout << '!' << endl;
    for (auto [u, v] : edges) cout << u + 1 << ' ' << v + 1 << '\n';
    cout.flush();
}

vector<pair<int, int>> decode(int id, int n) {
    int degree[8], code[8];
    fill(degree, degree + n, 1);
    for (int i = 0; i < n - 2; ++i) {
        code[i] = id % n;
        id /= n;
        ++degree[code[i]];
    }
    vector<pair<int, int>> edges;
    for (int i = 0; i < n - 2; ++i) {
        int u = 0;
        while (degree[u] != 1) ++u;
        int v = code[i];
        edges.emplace_back(u, v);
        --degree[u];
        --degree[v];
    }
    int u = 0;
    while (degree[u] != 1) ++u;
    int v = u + 1;
    while (degree[v] != 1) ++v;
    edges.emplace_back(u, v);
    return edges;
}

void solveSmall(int n) {
    int trees = 1;
    for (int i = 0; i < n - 2; ++i) trees *= n;
    if (trees == 1) {
        report(decode(0, n));
        return;
    }

    using Byte = unsigned char;
    vector<array<Byte, 49>> balls(trees);
    for (int t = 0; t < trees; ++t) {
        int adj[7]{};
        for (auto [u, v] : decode(t, n)) {
            adj[u] |= 1 << v;
            adj[v] |= 1 << u;
        }
        for (int v = 0; v < n; ++v) {
            int reached = 1 << v, layer = reached;
            balls[t][v * n] = 0;
            for (int r = 1; r < n; ++r) {
                int next = 0;
                for (int b = layer; b; b &= b - 1)
                    next |= adj[__builtin_ctz(unsigned(b))];
                layer = next & ~reached;
                reached |= layer;
                balls[t][v * n + r] = Byte(reached ^ (1 << v));
            }
        }
    }

    struct Query {
        array<Byte, 7> slot{};
        bool sorted = false;
    };
    vector<Query> queries;
    int radix = (n <= 6 ? n : 3);
    int configurations = 1;
    for (int i = 0; i < n; ++i) configurations *= radix;
    for (int id = 1; id < configurations; ++id) {
        int x = id, prev = 0;
        Query q;
        q.sorted = true;
        for (int v = 0; v < n; ++v) {
            int d = x % radix;
            x /= radix;
            q.slot[v] = Byte(v * n + d);
            q.sorted &= (prev <= d);
            prev = d;
        }
        queries.push_back(q);
    }
    if (n > 6) {
        for (int v = 0; v < n; ++v) {
            for (int r = radix; r < n; ++r) {
                Query q;
                for (int u = 0; u < n; ++u)
                    q.slot[u] = Byte(u * n + (u == v ? r : 0));
                q.sorted = (v == n - 1);
                queries.push_back(q);
            }
        }
    }

    vector<int> possible(trees);
    iota(possible.begin(), possible.end(), 0);
    auto response = [&](int t, const Query& q) {
        int mask = 0;
        for (int v = 0; v < n; ++v) mask |= balls[t][q.slot[v]];
        return mask;
    };
    while (possible.size() > 1) {
        int best = -1, bestWorst = int(possible.size());
        long long bestSquares = LLONG_MAX;
        for (int i = 0; i < int(queries.size()); ++i) {
            if (int(possible.size()) == trees && !queries[i].sorted) continue;
            int freq[128]{};
            int worst = 0;
            long long squares = 0;
            for (int t : possible) {
                int f = ++freq[response(t, queries[i])];
                worst = max(worst, f);
                squares += 2 * f - 1;
                if (worst > bestWorst) break;
            }
            if (worst < bestWorst || (worst == bestWorst && squares < bestSquares)) {
                best = i;
                bestWorst = worst;
                bestSquares = squares;
                if (worst == 1) break;
            }
        }
        if (best < 0) exit(0);
        vector<int> d(n);
        for (int v = 0; v < n; ++v) d[v] = queries[best].slot[v] - v * n;
        string s = ask(d);
        int mask = 0;
        for (int v = 0; v < n; ++v) if (s[v] == '1') mask |= 1 << v;
        erase_if(possible, [&](int t) { return response(t, queries[best]) != mask; });
        if (possible.empty()) exit(0);
    }
    report(decode(possible.front(), n));
}

class Solver {
    struct Row {
        vector<int> adj;
        vector<unsigned char> feasible;
        array<int, SMALL> with{}, without{};
        vector<double> gain;
        Bits sole;
        int lower = 0;
    };

    int n, components, unknown, calls = 0;
    Bits all;
    vector<Bits> cand, known, members, tests;
    vector<vector<int>> needs;
    vector<int> parent, sz;
    vector<char> pending;
    deque<int> dirty;
    vector<pair<int, int>> answer;
    mt19937 rng;
    vector<Row> rows;
    vector<int> compLower;
    int slack = 0;

    int root(int v) {
        while (parent[v] != v) {
            parent[v] = parent[parent[v]];
            v = parent[v];
        }
        return v;
    }

    void touch(int v) {
        if (!pending[v]) {
            pending[v] = true;
            dirty.push_back(v);
        }
    }

    void eraseEdges(int v, const Bits& subset) {
        Bits gone = cand[v] & subset;
        if (gone.none()) return;
        cand[v] ^= gone;
        touch(v);
        for (size_t u = gone._Find_first(); u < size_t(n); u = gone._Find_next(u)) {
            cand[u].reset(v);
            touch(int(u));
            --unknown;
        }
    }

    void confirm(int u, int v) {
        if (!cand[u][v]) return;
        int a = root(u), b = root(v);
        if (a == b) exit(0);
        known[u].set(v);
        known[v].set(u);
        answer.emplace_back(u, v);
        --components;
        if (sz[a] > sz[b]) swap(a, b);
        for (size_t x = members[a]._Find_first(); x < size_t(n);
             x = members[a]._Find_next(x)) {
            eraseEdges(int(x), members[b]);
        }
        parent[a] = b;
        sz[b] += sz[a];
        members[b] |= members[a];
    }

    void propagateRows() {
        while (!dirty.empty() && components > 1) {
            int v = dirty.front();
            dirty.pop_front();
            pending[v] = false;
            auto& list = needs[v];
            for (size_t i = 0; i < list.size();) {
                const Bits& s = tests[list[i]];
                if ((s & known[v]).any()) {
                    list[i] = list.back();
                    list.pop_back();
                    continue;
                }
                Bits p = s & cand[v];
                size_t u = p._Find_first();
                if (u >= size_t(n)) exit(0);
                if (p._Find_next(u) >= size_t(n)) {
                    confirm(v, int(u));
                    list[i] = list.back();
                    list.pop_back();
                    if (components == 1) return;
                    continue;
                }

                // This clause can certify a particular pair of components.
                // A tree has at most one edge between two connected subtrees.
                int a = root(v), b = root(int(u));
                if ((p & ~members[b]).none()) {
                    eraseEdges(v, members[b] & ~p);
                    if (sz[a] > 1) {
                        for (size_t w = members[a]._Find_first(); w < size_t(n);
                             w = members[a]._Find_next(w)) {
                            if (w != size_t(v)) eraseEdges(int(w), members[b]);
                        }
                    }
                }
                ++i;
            }
        }
    }

    bool takeBridges() {
        if (components <= 2 || unknown > 32 * n) return false;
        vector<int> head(n, -1), to, next, rt(n);
        vector<pair<int, int>> edges;
        to.reserve(2 * unknown);
        next.reserve(2 * unknown);
        edges.reserve(unknown);
        for (int v = 0; v < n; ++v) rt[v] = root(v);
        auto arc = [&](int u, int v) {
            to.push_back(v);
            next.push_back(head[u]);
            head[u] = int(to.size()) - 1;
        };
        for (int v = 0; v < n; ++v) {
            for (size_t u = cand[v]._Find_next(v); u < size_t(n);
                 u = cand[v]._Find_next(u)) {
                edges.emplace_back(v, int(u));
                arc(rt[v], rt[u]);
                arc(rt[u], rt[v]);
            }
        }
        vector<int> tin(n), low(n), bridges;
        int timer = 0;
        auto dfs = [&](auto&& self, int v, int back) -> void {
            tin[v] = low[v] = ++timer;
            for (int e = head[v]; e != -1; e = next[e]) {
                if (e == back) continue;
                int u = to[e];
                if (tin[u]) low[v] = min(low[v], tin[u]);
                else {
                    self(self, u, e ^ 1);
                    low[v] = min(low[v], low[u]);
                    if (low[u] > tin[v]) bridges.push_back(e / 2);
                }
            }
        };
        dfs(dfs, rt[0], -1);
        for (int i : bridges) confirm(edges[i].first, edges[i].second);
        return !bridges.empty();
    }

    void propagate() {
        do {
            propagateRows();
        } while (components > 1 && takeBridges());
    }

    void interact(const Bits& s) {
        vector<int> d(n);
        for (int v = 0; v < n; ++v) d[v] = s[v];
        string reply = ask(d);
        ++calls;
        int id = int(tests.size());
        tests.push_back(s);

        for (int v = 0; v < n; ++v)
            if (reply[v] == '0') eraseEdges(v, s);

        for (int v = 0; v < n; ++v) {
            if (reply[v] != '1' || (known[v] & s).any()) continue;
            Bits p = cand[v] & s;
            bool redundant = false;
            auto& list = needs[v];
            for (int k : list) {
                if ((known[v] & tests[k]).none() &&
                    (cand[v] & tests[k] & ~s).none()) {
                    redundant = true;
                    break;
                }
            }
            if (redundant) continue;
            erase_if(list, [&](int k) {
                return (p & ~tests[k]).none();
            });
            list.push_back(id);
            touch(v);
        }
    }

    // Compute lower bounds on the number of still unknown incident edges.
    // For small candidate sets enumerate all local possibilities exactly.
    void buildRows() {
        compLower.assign(n, 0);
        for (int v = 0; v < n; ++v) {
            Row& r = rows[v];
            r.adj.clear();
            r.feasible.clear();
            r.lower = 0;
            r.sole.reset();
            for (size_t u = cand[v]._Find_first(); u < size_t(n);
                 u = cand[v]._Find_next(u)) r.adj.push_back(int(u));
            int d = int(r.adj.size());
            if (!needs[v].empty()) {
                r.sole = cand[v];
                for (int k : needs[v]) r.sole &= tests[k];
            }
            if (d <= SMALL) {
                int full = (1 << d) - 1;
                r.feasible.assign(full + 1, 1);
                for (int k : needs[v]) {
                    int clause = 0;
                    for (int j = 0; j < d; ++j)
                        if (tests[k][r.adj[j]]) clause |= 1 << j;
                    r.feasible[full ^ clause] = 0;
                }
                for (int j = 0; j < d; ++j) {
                    for (int m = 0; m <= full; ++m) {
                        if ((m & (1 << j)) == 0)
                            r.feasible[m] &= r.feasible[m | (1 << j)];
                    }
                }
                array<int, SMALL + 1> some{}, every;
                every.fill(full);
                r.lower = INF;
                for (int m = 0; m <= full; ++m) if (r.feasible[m]) {
                    int k = __builtin_popcount(unsigned(m));
                    r.lower = min(r.lower, k);
                    some[k] |= m;
                    every[k] &= m;
                }
                if (r.lower == INF) exit(0);
                r.with.fill(INF);
                r.without.fill(INF);
                for (int k = r.lower; k <= d; ++k) {
                    for (int j = 0; j < d; ++j) {
                        if (some[k] & (1 << j)) r.with[j] = min(r.with[j], k);
                        if (!(every[k] & (1 << j))) r.without[j] = min(r.without[j], k);
                    }
                }
            } else if (!needs[v].empty()) {
                vector<pair<int, int>> order;
                for (int k : needs[v])
                    order.emplace_back(int((cand[v] & tests[k]).count()), k);
                sort(order.begin(), order.end());
                Bits covered;
                for (auto [size, k] : order) {
                    Bits clause = cand[v] & tests[k];
                    if ((covered & clause).none()) {
                        ++r.lower;
                        covered |= clause;
                    }
                }
                if (r.sole.none()) r.lower = max(r.lower, 2);
            }
            compLower[root(v)] += r.lower;
        }
        slack = 2 * (components - 1);
        for (int v = 0; v < n; ++v)
            if (parent[v] == v) slack -= max(1, compLower[v]);
        if (slack < 0) exit(0);
    }

    bool useDegreeBudget() {
        if (slack > 2 * SMALL) return false;
        vector<pair<int, int>> must;
        bool changed = false;
        auto increase = [&](int v, int u, bool included) {
            const Row& r = rows[v];
            int bound = included ? max(r.lower, 1) : r.lower;
            if (int(r.adj.size()) <= SMALL) {
                int j = int(lower_bound(r.adj.begin(), r.adj.end(), u) - r.adj.begin());
                bound = included ? r.with[j] : r.without[j];
            }
            if (bound == INF) return INF;
            int base = compLower[root(v)];
            return max(1, base + bound - r.lower) - max(1, base);
        };
        for (int v = 0; v < n; ++v) {
            Bits drop;
            for (int u : rows[v].adj) if (u > v) {
                if (increase(v, u, true) + increase(u, v, true) > slack)
                    drop.set(u);
                else if (increase(v, u, false) + increase(u, v, false) > slack)
                    must.emplace_back(v, u);
            }
            if (drop.any()) {
                eraseEdges(v, drop);
                changed = true;
            }
        }
        for (auto [u, v] : must) confirm(u, v);
        return changed || !must.empty();
    }

    int bestSingle() const {
        int u = 0;
        for (int v = 1; v < n; ++v)
            if (cand[v].count() > cand[u].count()) u = v;
        return u;
    }

    Bits twoComponents() {
        Bits a, b;
        int rt = root(0);
        for (int v = 0; v < n; ++v) if (cand[v].any()) {
            if (root(v) == rt) a.set(v);
            else b.set(v);
        }
        Bits best = a;
        int worstBest = INT_MAX;
        long long squaresBest = LLONG_MAX;
        for (Bits s : {a | b, a, b}) {
            vector<int> freq(n + 1);
            Bits visible;
            for (int v = 0; v < n; ++v)
                if ((known[v] & s).none()) visible.set(v);
            int worst = 1;
            long long squares = unknown;
            for (size_t u = a._Find_first(); u < size_t(n); u = a._Find_next(u)) {
                for (size_t v = cand[u]._Find_first(); v < size_t(n);
                     v = cand[u]._Find_next(v)) {
                    bool x = visible[u] && s[v], y = visible[v] && s[u];
                    if (x && y) continue;
                    int key = x ? int(u) : (y ? int(v) : n);
                    int f = ++freq[key];
                    worst = max(worst, f);
                    squares += 2LL * (f - 1);
                }
            }
            if (make_pair(worst, squares) < make_pair(worstBest, squaresBest)) {
                worstBest = worst;
                squaresBest = squares;
                best = s;
            }
        }
        return best;
    }

    static double gain(double off, int k, int d) {
        if (k == 0 || off <= 1e-12 || off >= 1.0 - 1e-12) return 0.0;
        double on = 1.0 - off;
        double h = -off * log2(off) - on * log2(on);
        return h + 0.45 * off * k / d + (k == 1 ? 0.35 + 1.25 * on : 0.0);
    }

    bool informative(const Bits& s) const {
        for (int v = 0; v < n; ++v) {
            if ((known[v] & s).any() || (cand[v] & s).none()) continue;
            bool implied = false;
            for (int k : needs[v]) {
                if ((cand[v] & tests[k] & ~s).none()) {
                    implied = true;
                    break;
                }
            }
            if (!implied) return true;
        }
        return false;
    }

    Bits choose() {
        Bits active;
        vector<int> act, deg(n), soleCount(n);
        vector<double> votes(n);
        for (int v = 0; v < n; ++v) {
            deg[v] = int(rows[v].adj.size());
            soleCount[v] = int(rows[v].sole.count());
            if (!deg[v]) continue;
            active.set(v);
            act.push_back(v);
            if (soleCount[v]) {
                double w = 1.0 / soleCount[v];
                for (size_t u = rows[v].sole._Find_first(); u < size_t(n);
                     u = rows[v].sole._Find_next(u)) votes[u] += w;
            }
        }

        int hub = -1;
        double hubScore = 0;
        for (int v : act) {
            double value = votes[v];
            int k = int(known[v].count());
            if (k >= max(7, n / 5) && deg[v] >= max(5, int(act.size()) / 4))
                value += min(double(k), 0.25 * (components - 1));
            if (value > hubScore) hubScore = value, hub = v;
        }
        if (hub >= 0 && hubScore >= max(6.5, 0.13 * (components - 1))) {
            Bits s;
            return s.set(hub);
        }

        double mean = 2.0 * (components - 1) / act.size();
        vector<vector<pair<int, int>>> effects(n);
        vector<Bits> liveKnown(n);
        for (int v : act) {
            Row& r = rows[v];
            int d = deg[v];
            liveKnown[v] = known[v] & active;
            for (int j = 0; j < d; ++j)
                effects[r.adj[j]].emplace_back(v, d <= SMALL ? 1 << j : 0);
            if (d <= SMALL) {
                int full = (1 << d) - 1;
                int cap = r.lower + slack + (compLower[root(v)] == 0);
                array<double, SMALL> odds{};
                for (int j = 0; j < d; ++j) {
                    int u = r.adj[j];
                    double bonus = rows[u].sole[v] ? 0.6 / sqrt(soleCount[u]) : 0;
                    odds[j] = min(1.2, 1.25 / max(2, d) + bonus + 0.2 / (deg[u] + 1));
                }
                double raw[1 << SMALL], zeta[1 << SMALL];
                for (int m = 0; m <= full; ++m) {
                    raw[m] = m ? raw[m & (m - 1)] * odds[__builtin_ctz(unsigned(m))] : 1.0;
                    zeta[m] = r.feasible[m] && __builtin_popcount(unsigned(m)) <= cap
                                  ? raw[m] : 0.0;
                }
                for (int j = 0; j < d; ++j)
                    for (int m = 0; m <= full; ++m)
                        if (m & (1 << j)) zeta[m] += zeta[m ^ (1 << j)];
                r.gain.resize(full + 1);
                for (int m = 0; m <= full; ++m)
                    r.gain[m] = gain(zeta[full ^ m] / zeta[full],
                                     __builtin_popcount(unsigned(m)), d);
            } else {
                double est = max(r.lower + 0.22, mean);
                if (calls > 2 && soleCount[v]) est = max(1.35, min(est, 1.75));
                double zero = needs[v].empty() ? 0.32 : 0.0;
                r.gain.resize(d + 1);
                for (int k = 0; k <= d; ++k) {
                    double off = zero + (1 - zero) * pow(1.0 - double(k) / d, est);
                    r.gain[k] = gain(off, k, d);
                }
            }
        }

        auto evaluate = [&](const Bits& s) {
            double value = 0.0;
            for (int v : act) {
                if ((liveKnown[v] & s).any()) continue;
                int k = 0;
                if (deg[v] <= SMALL) {
                    for (int j = 0; j < deg[v]; ++j)
                        if (s[rows[v].adj[j]]) k |= 1 << j;
                } else k = int((cand[v] & s).count());
                value += rows[v].gain[k];
            }
            return value;
        };

        Bits best;
        double bestValue = -1;
        auto consider = [&](const Bits& s, double value) {
            if (value > bestValue + 1e-8 && s.any() && informative(s)) {
                best = s;
                bestValue = value;
            }
        };
        Bits singleton;
        singleton.set(bestSingle());
        consider(singleton, evaluate(singleton));
        consider(active, evaluate(active));
        if (calls) {
            Bits complement = active & ~tests.back();
            consider(complement, evaluate(complement));
        }

        vector<int> rt(n);
        for (int v : act) rt[v] = root(v);
        vector<pair<double, Bits>> starts;
        const double dense[] = {0.24, 0.29, 0.34, 0.38, 0.43, 0.49, 0.56, 0.32};
        const double sparse[] = {0.15, 0.23, 0.31, 0.39, 0.48, 0.59, 0.73, 0.87};
        int trials = unknown > 40 * n ? 24 : 48;
        for (int t = 0; t < trials; ++t) {
            double p = (unknown > 12 * n ? dense : sparse)[t % 8];
            uint32_t cutoff = uint32_t(p * 4294967296.0);
            Bits s;
            if (t % 3 == 2) {
                Bits picked;
                for (int v : act)
                    if (rt[v] == v || !active[rt[v]]) {
                        // The representative itself need not still be active.
                        if (!active[rt[v]]) continue;
                        if (rng() < cutoff) picked.set(v);
                    }
                Bits seen = active;
                for (int v : act) {
                    if (!seen[rt[v]]) {
                        seen.set(rt[v]);
                        if (rng() < cutoff) picked.set(rt[v]);
                    }
                    if (picked[rt[v]]) s.set(v);
                }
            } else {
                for (int v : act) if (rng() < cutoff) s.set(v);
            }
            double value = evaluate(s);
            consider(s, value);
            starts.emplace_back(value, s);
        }
        starts.emplace_back(evaluate(best), best);
        sort(starts.begin(), starts.end(), [](const auto& a, const auto& b) {
            return a.first > b.first;
        });

        int restarts = unknown > 100 * n ? 1 : (unknown > 20 * n ? 2 : 4);
        vector<int> order = act;
        for (int t = 0; t < restarts; ++t) {
            Bits s = starts[t].second;
            vector<int> index(n), masked(n);
            for (int u : act) if (s[u]) {
                for (auto [v, bit] : effects[u]) index[v] += bit ? bit : 1;
                for (size_t v = liveKnown[u]._Find_first(); v < size_t(n);
                     v = liveKnown[u]._Find_next(v)) ++masked[v];
            }
            int passes = unknown > 100 * n ? 1 : 3;
            for (int pass = 0; pass < passes; ++pass) {
                bool any = false;
                shuffle(order.begin(), order.end(), rng);
                for (int u : order) {
                    int sign = s[u] ? -1 : 1;
                    double delta = 0;
                    for (auto [v, bit] : effects[u]) if (!masked[v]) {
                        const auto& g = rows[v].gain;
                        delta += g[index[v] + sign * (bit ? bit : 1)] - g[index[v]];
                    }
                    for (size_t v = liveKnown[u]._Find_first(); v < size_t(n);
                         v = liveKnown[u]._Find_next(v)) {
                        if (masked[v] == 0 && sign == 1)
                            delta -= rows[v].gain[index[v]];
                        else if (masked[v] == 1 && sign == -1)
                            delta += rows[v].gain[index[v]];
                    }
                    if (delta <= 1e-8) continue;
                    any = true;
                    s.flip(u);
                    for (auto [v, bit] : effects[u]) index[v] += sign * (bit ? bit : 1);
                    for (size_t v = liveKnown[u]._Find_first(); v < size_t(n);
                         v = liveKnown[u]._Find_next(v)) masked[v] += sign;
                }
                if (!any) break;
            }
            consider(s, evaluate(s));
        }
        return best.any() ? best : singleton;
    }

public:
    explicit Solver(int n)
        : n(n), components(n), unknown(n * (n - 1) / 2),
          cand(n), known(n), members(n), needs(n), parent(n), sz(n, 1),
          pending(n), rng(uint32_t(chrono::steady_clock::now().time_since_epoch().count())),
          rows(n) {
        for (int v = 0; v < n; ++v) all.set(v);
        tests.push_back(all);  // Every vertex has a neighbor, without querying.
        for (int v = 0; v < n; ++v) {
            cand[v] = all;
            cand[v].reset(v);
            members[v].set(v);
            parent[v] = v;
            needs[v].push_back(0);
        }
    }

    void run() {
        int budgetPasses = 0;
        while (components > 1) {
            propagate();
            if (components == 1) break;
            Bits s;
            if (components == 2) {
                s = twoComponents();
            } else if (calls >= 65) {
                s.set(bestSingle());
            } else {
                buildRows();
                if (budgetPasses < 6 && useDegreeBudget()) {
                    ++budgetPasses;
                    continue;
                }
                s = choose();
            }
            interact(s);
            budgetPasses = 0;
        }
        report(answer);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 2 || n > MAXN) return 0;
    if (n <= 7) solveSmall(n);
    else Solver(n).run();
    return 0;
}

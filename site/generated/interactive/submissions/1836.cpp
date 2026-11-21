#include <bits/stdc++.h>
using namespace std;

using Mask = uint64_t;
constexpr int INF = 100000000;

struct Query {
    int u = 0, k = 0, balance = 0;
};

struct Prefix {
    int lo, hi, count;
    Mask mask;
};

struct Cut {
    Mask mask;
    int u, k;
    bool flipped;
};

struct Tree {
    int n, timer = 0;
    vector<int> parent, depth, tin, tout, first, preorder, euler;
    vector<array<int, 2>> child;
    vector<vector<int>> up, rmq;
    vector<int> frequency, distances, stamp;
    int epoch = 0;
    mt19937 rng{712367821};

    explicit Tree(int n) : n(n), parent(n + 1, 1), depth(n + 1),
        tin(n + 1), tout(n + 1), first(n + 1), child(n + 1, {0, 0}),
        frequency(n + 1), stamp(n + 1) {}

    void build() {
        vector<pair<int, int>> st;
        st.emplace_back(1, 0);
        first[1] = 0;
        tin[1] = ++timer;
        preorder.push_back(1);
        euler.push_back(1);
        while (!st.empty()) {
            int u = st.back().first;
            int &i = st.back().second;
            if (i == 2) {
                tout[u] = timer;
                st.pop_back();
                if (!st.empty()) euler.push_back(st.back().first);
                continue;
            }
            int v = child[u][i++];
            if (!v) continue;
            tin[v] = ++timer;
            first[v] = (int)euler.size();
            preorder.push_back(v);
            euler.push_back(v);
            st.emplace_back(v, 0);
        }

        int lg = bit_width((unsigned)n);
        up.assign(lg, vector<int>(n + 1, 1));
        up[0] = parent;
        for (int j = 1; j < lg; ++j)
            for (int u = 1; u <= n; ++u)
                up[j][u] = up[j - 1][up[j - 1][u]];

        rmq.push_back(euler);
        int e = (int)euler.size();
        for (int j = 1; (1 << j) <= e; ++j) {
            int half = 1 << (j - 1);
            vector<int> row(e - 2 * half + 1);
            const auto &prev = rmq.back();
            for (int i = 0; i < (int)row.size(); ++i) {
                int a = prev[i], b = prev[i + half];
                row[i] = depth[a] < depth[b] ? a : b;
            }
            rmq.push_back(move(row));
        }
    }

    bool ancestor(int a, int b) const {
        return tin[a] <= tin[b] && tin[b] <= tout[a];
    }

    int lca(int a, int b) const {
        int l = first[a], r = first[b];
        if (l > r) swap(l, r);
        int j = 31 - __builtin_clz((unsigned)(r - l + 1));
        int x = rmq[j][l], y = rmq[j][r - (1 << j) + 1];
        return depth[x] < depth[y] ? x : y;
    }

    int dist(int a, int b) const {
        return depth[a] + depth[b] - 2 * depth[lca(a, b)];
    }

    int lift(int u, int d) const {
        while (d) {
            int j = __builtin_ctz((unsigned)d);
            u = up[j][u];
            d &= d - 1;
        }
        return u;
    }

    int pathVertex(int a, int b, int c, int d) const {
        int da = depth[a] - depth[c], db = depth[b] - depth[c];
        return d <= da ? lift(a, d) : lift(b, da + db - d);
    }

    array<int, 3> neighbors(int u) const {
        return {u == 1 ? 0 : parent[u], child[u][0], child[u][1]};
    }

    // Radius of the target-th closest candidate and the two adjacent counts.
    array<int, 3> quantile(int u, int target, const vector<int> &s) {
        distances.clear();
        int lo = n, hi = 0;
        for (int x : s) {
            int d = dist(u, x);
            lo = min(lo, d);
            hi = max(hi, d);
            ++frequency[d];
            distances.push_back(d);
        }
        int r, below = 0, through = 0;
        if (hi - lo <= 2 * (int)s.size()) {
            r = lo;
            while (below + frequency[r] < target) below += frequency[r++];
            through = below + frequency[r];
            fill(frequency.begin() + lo, frequency.begin() + hi + 1, 0);
        } else {
            nth_element(distances.begin(), distances.begin() + target - 1,
                        distances.end());
            r = distances[target - 1];
            for (int d : distances) {
                below += d < r;
                through += d <= r;
                --frequency[d];
            }
        }
        return {r, below, through};
    }

    vector<pair<int, int>> virtualEdges(const vector<int> &s) const {
        vector<int> v = s;
        for (int i = 1; i < (int)s.size(); ++i) v.push_back(lca(s[i-1], s[i]));
        sort(v.begin(), v.end(), [&](int a, int b) { return tin[a] < tin[b]; });
        v.erase(unique(v.begin(), v.end()), v.end());
        vector<int> st;
        vector<pair<int, int>> edges;
        for (int u : v) {
            while (!st.empty() && !ancestor(st.back(), u)) st.pop_back();
            if (!st.empty()) edges.emplace_back(st.back(), u);
            st.push_back(u);
        }
        return edges;
    }

    static vector<Prefix> makePrefixes(vector<pair<int, int>> &v, bool masks) {
        sort(v.begin(), v.end());
        vector<Prefix> p;
        p.reserve(v.size() + 1);
        int lo = -INF, i = 0;
        Mask mask = 0;
        while (i < (int)v.size()) {
            int d = v[i].first;
            p.push_back({lo, d - 1, i, mask});
            lo = d;
            do {
                if (masks) mask |= Mask(1) << v[i].second;
                ++i;
            } while (i < (int)v.size() && v[i].first == d);
        }
        p.push_back({lo, INF, i, mask});
        return p;
    }

    pair<vector<Prefix>, vector<Prefix>>
    edgePrefixes(int a, int b, const vector<int> &s, bool masks) const {
        vector<pair<int, int>> left, right;
        left.reserve(s.size());
        right.reserve(s.size());
        for (int i = 0; i < (int)s.size(); ++i) {
            int x = s[i];
            if (ancestor(b, x)) right.emplace_back(depth[x] - depth[b], i);
            else left.emplace_back(dist(a, x), i);
        }
        return {makePrefixes(left, masks), makePrefixes(right, masks)};
    }

    // On the edge path, the two distance thresholds differ by L - 2*t.
    static int offset(const Prefix &a, const Prefix &b, int L) {
        int d = max(-L, a.lo - b.hi);
        if ((d + L) % 2) ++d;
        if (d > min(L, a.hi - b.lo)) return -1;
        return (L - d) / 2;
    }

    vector<Cut> allCuts(const vector<int> &s) const {
        int m = (int)s.size();
        Mask full = m == 64 ? ~Mask(0) : (Mask(1) << m) - 1;
        vector<Cut> cuts;
        unordered_set<Mask> seen;
        seen.reserve(m * m);
        for (auto [a, b] : virtualEdges(s)) {
            int L = depth[b] - depth[a];
            auto [left, right] = edgePrefixes(a, b, s, true);
            for (const auto &p : left) {
                for (const auto &q : right) {
                    Mask raw = p.mask | q.mask;
                    if (!raw || raw == full) continue;
                    bool flip = raw & 1;
                    Mask key = flip ? full ^ raw : raw;
                    if (seen.contains(key)) continue;
                    int t = offset(p, q, L);
                    if (t < 0) continue;
                    seen.insert(key);
                    cuts.push_back({key, lift(b, L - t),
                                    max(p.lo + t, q.lo + L - t), flip});
                }
            }
        }
        return cuts;
    }

    void exactBalance(const vector<int> &s, Query &best) const {
        int m = (int)s.size(), goal = m / 2;
        for (auto [a, b] : virtualEdges(s)) {
            int L = depth[b] - depth[a];
            auto [left, right] = edgePrefixes(a, b, s, false);
            array<vector<Prefix>, 2> parity;
            for (auto q : right) {
                for (int e = 0; e < 2; ++e) {
                    Prefix z = q;
                    z.lo += ((z.lo + INF) & 1) != e;
                    z.hi -= ((z.hi + INF) & 1) != e;
                    if (z.lo <= z.hi) parity[e].push_back(z);
                }
            }
            for (const auto &p : left) {
                const auto &v = p.lo == p.hi
                    ? parity[(p.lo + L + INF) & 1] : right;
                auto lo = lower_bound(v.begin(), v.end(), p.lo - L,
                    [](const Prefix &q, int x) { return q.hi < x; });
                auto hi = upper_bound(lo, v.end(), p.hi + L,
                    [](int x, const Prefix &q) { return x < q.lo; });
                if (lo == hi) continue;
                auto it = lower_bound(lo, hi, goal - p.count,
                    [](const Prefix &q, int x) { return q.count < x; });
                auto test = [&](const Prefix &q) {
                    int c = p.count + q.count, score = min(c, m - c);
                    if (score <= best.balance) return;
                    int t = offset(p, q, L);
                    if (t >= 0)
                        best = {lift(b, L - t), max(p.lo + t, q.lo + L - t), score};
                };
                if (it != hi) test(*it);
                if (it != lo) test(*prev(it));
                if (best.balance == goal) return;
            }
        }
    }

    // Find a local minimum of the required radius by descending through
    // components of successively removed centroids.
    Query guaranteedSplit(const vector<int> &s, int want) {
        int m = (int)s.size();
        vector<int> walkParent(n + 1), size(n + 1), order;
        vector<char> removed(n + 1);
        int start = 1;
        while (true) {
            order.clear();
            order.push_back(start);
            walkParent[start] = 0;
            for (int i = 0; i < (int)order.size(); ++i) {
                int u = order[i];
                size[u] = 1;
                for (int v : neighbors(u)) {
                    if (!v || removed[v] || v == walkParent[u]) continue;
                    walkParent[v] = u;
                    order.push_back(v);
                }
            }
            int total = (int)order.size();
            int c = start, largest = total;
            for (int i = total - 1; i >= 0; --i) {
                int u = order[i], worst = total - size[u];
                for (int v : neighbors(u))
                    if (v && !removed[v] && walkParent[v] == u)
                        worst = max(worst, size[v]);
                if (worst < largest) largest = worst, c = u;
                if (walkParent[u]) size[walkParent[u]] += size[u];
            }
            auto [r, below, through] = quantile(c, want, s);
            if (through <= m - want) return {c, r, min(through, m - through)};
            int next = 0;
            for (int v : neighbors(c)) {
                if (!v || removed[v]) continue;
                int count = 0;
                for (int x : s) {
                    count += dist(v, x) < r;
                    if (count == want) break;
                }
                if (count == want) {
                    next = v;
                    break;
                }
            }
            if (!next) return {s.front(), 0, 1};
            removed[c] = 1;
            start = next;
        }
    }

    Query choose(const vector<int> &s) {
        int m = (int)s.size(), goal = m / 2;
        Query best{s.front(), 0, 1};
        ++epoch;
        auto evaluate = [&](int u) {
            if (!u || stamp[u] == epoch || best.balance == goal) return;
            stamp[u] = epoch;
            auto [r, below, through] = quantile(u, goal, s);
            int x = min(below, m - below), y = min(through, m - through);
            if (x > best.balance) best = {u, r - 1, x};
            if (y > best.balance) best = {u, r, y};
        };

        vector<int> weight(n + 1);
        for (int x : s) weight[x] = 1;
        for (int u = n; u > 1; --u) weight[parent[u]] += weight[u];
        int c = 1;
        while (true) {
            int next = 0;
            for (int v : child[c]) if (v && weight[v] > goal) next = v;
            if (!next) break;
            c = next;
        }
        evaluate(c);
        for (int v : neighbors(c)) evaluate(v);
        if (best.balance == goal) return best;

        auto farthest = [&](int u) {
            int a = s.front(), far = -1;
            for (int x : s) {
                int d = dist(u, x);
                if (d > far) far = d, a = x;
            }
            return a;
        };
        int a = farthest(s.front()), b = farthest(a), l = lca(a, b);
        int d = depth[a] + depth[b] - 2 * depth[l];
        evaluate(a);
        evaluate(b);
        evaluate(l);
        for (int j = 1; j < 8; ++j) evaluate(pathVertex(a, b, l, d * j / 8));

        int rounds = m <= 800 ? 8 : 40;
        for (int i = 0; i < rounds && best.balance < goal; ++i) {
            int x = s[rng() % m], y = s[rng() % m], z = lca(x, y);
            int length = depth[x] + depth[y] - 2 * depth[z];
            evaluate(x);
            if (i % 2 == 0) evaluate(z);
            evaluate(pathVertex(x, y, z, rng() % (length + 1)));
            if (i % 5 == 0) {
                int u = best.u;
                for (int v : neighbors(u)) evaluate(v);
            }
        }
        if (best.balance < goal && m <= 800) exactBalance(s, best);
        int want = (m + 2) / 4;
        if (best.balance < want) {
            Query q = guaranteedSplit(s, want);
            if (q.balance > best.balance) best = q;
        }
        return best;
    }
};

struct Endgame {
    struct Record {
        int failed = -1, depth = 100, cut = -1;
    };

    vector<Cut> cuts;
    unordered_map<Mask, Record> memo;
    array<unordered_map<Mask, pair<int, int>>, 13> costMemo;
    long long work = 0, costWork = 0;
    static constexpr long long workLimit = 5000000, costLimit = 1200000;

    explicit Endgame(vector<Cut> cuts) : cuts(move(cuts)) {
        memo.reserve(4096);
    }

    static int count(Mask s) { return __builtin_popcountll(s); }
    static int lowerDepth(int n) { return bit_width((unsigned)(n - 1)); }
    static int idealCost(int n) {
        int h = lowerDepth(n);
        return n * (h + 1) - (1 << h);
    }

    vector<pair<int, int>> options(Mask s, int depth) const {
        int n = count(s), cap = 1 << (depth - 1);
        int low = max(1, n - cap);
        vector<pair<int, int>> moves;
        for (int i = 0; i < (int)cuts.size(); ++i) {
            int a = count(s & cuts[i].mask), score = min(a, n - a);
            if (score >= low) moves.emplace_back(-score, i);
        }
        sort(moves.begin(), moves.end());
        return moves;
    }

    bool possible(Mask s, int depth) {
        int n = count(s);
        if (n <= 1) return true;
        if (depth < lowerDepth(n)) return false;
        if (n == 2) return true;
        auto it = memo.find(s);
        if (it != memo.end()) {
            if (it->second.depth <= depth) return true;
            if (it->second.failed >= depth) return false;
        }
        if (work >= workLimit) return false;
        work += cuts.size();
        auto moves = options(s, depth);
        for (auto [score, i] : moves) {
            Mask a = s & cuts[i].mask, b = s ^ a;
            if (count(a) < count(b)) swap(a, b);
            if (possible(a, depth - 1) && possible(b, depth - 1)) {
                auto &r = memo[s];
                if (depth < r.depth) r.depth = depth, r.cut = i;
                return true;
            }
            if (work >= workLimit) return false;
        }
        auto &r = memo[s];
        r.failed = max(r.failed, depth);
        return false;
    }

    pair<int, int> totalCost(Mask s, int depth) {
        int n = count(s);
        if (n <= 1) return {0, -1};
        if (depth < lowerDepth(n)) return {INF, -1};
        if (n == 2) return {2, -1};
        depth = min(depth, n - 1);
        auto &cache = costMemo[depth];
        if (auto it = cache.find(s); it != cache.end()) return it->second;
        if (costWork >= costLimit) return {INF, -1};
        costWork += cuts.size();
        auto moves = options(s, depth);
        pair<int, int> best{INF, -1};
        int ideal = idealCost(n);
        unordered_set<Mask> seen;
        for (auto [score, i] : moves) {
            Mask a = s & cuts[i].mask, b = s ^ a;
            if (count(a) < count(b)) swap(a, b);
            int nb = count(b);
            if (n + idealCost(n - nb) + idealCost(nb) >= best.first) continue;
            if (!seen.insert(min(a, b)).second) continue;
            int ca = totalCost(a, depth - 1).first;
            if (n + ca + idealCost(nb) >= best.first) continue;
            int cb = totalCost(b, depth - 1).first;
            if (n + ca + cb < best.first) best = {n + ca + cb, i};
            if (best.first == ideal || costWork >= costLimit) break;
        }
        if (costWork < costLimit || best.first == ideal) cache.emplace(s, best);
        return best;
    }

    int choose(Mask s) {
        int n = count(s);
        for (int h = lowerDepth(n); h < n && work < workLimit; ++h)
            if (possible(s, h)) break;
        auto it = memo.find(s);
        if (it != memo.end() && it->second.cut >= 0) {
            if (n <= 12 && costWork < costLimit) {
                auto [cost, i] = totalCost(s, it->second.depth);
                if (i >= 0) return i;
            }
            return it->second.cut;
        }
        int best = -1, balance = 0;
        for (int i = 0; i < (int)cuts.size(); ++i) {
            int a = count(s & cuts[i].mask), val = min(a, n - a);
            if (val > balance) best = i, balance = val;
            if (balance == n / 2) break;
        }
        return best;
    }
};

int signal(int u, int k) {
    cout << "? " << u << ' ' << k << '\n' << flush;
    int reply;
    if (!(cin >> reply) || (reply != 0 && reply != 1)) exit(0);
    return reply;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2) return 0;
    Tree tree(n);
    for (int u = 2; u <= n; ++u) {
        int p;
        if (!(cin >> p)) return 0;
        tree.parent[u] = p;
        tree.depth[u] = tree.depth[p] + 1;
        tree.child[p][tree.child[p][0] != 0] = u;
    }
    tree.build();
    vector<int> s = tree.preorder;
    while (s.size() > 64) {
        Query q = tree.choose(s);
        int reply = signal(q.u, q.k);
        s.erase(remove_if(s.begin(), s.end(), [&](int x) {
            return (tree.dist(q.u, x) <= q.k) != reply;
        }), s.end());
    }

    int m = (int)s.size();
    Mask alive = m == 64 ? ~Mask(0) : (Mask(1) << m) - 1;
    if (m > 2) {
        Endgame endgame(tree.allCuts(s));
        while (Endgame::count(alive) > 2) {
            int i = endgame.choose(alive);
            if (i < 0) {
                int j = __builtin_ctzll(alive);
                if (signal(s[j], 0)) alive = Mask(1) << j;
                else alive &= alive - 1;
                continue;
            }
            const Cut &q = endgame.cuts[i];
            int reply = signal(q.u, q.k);
            alive &= (reply ^ q.flipped) ? q.mask : ~q.mask;
        }
    }
    if (Endgame::count(alive) == 2) {
        int j = __builtin_ctzll(alive);
        if (signal(s[j], 0)) alive = Mask(1) << j;
        else alive &= alive - 1;
    }
    cout << "! " << s[__builtin_ctzll(alive)] << '\n' << flush;
    return 0;
}

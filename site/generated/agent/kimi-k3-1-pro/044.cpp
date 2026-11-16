#include <bits/stdc++.h>
using namespace std;

using Grid = vector<vector<int>>;

struct Dinic {
    struct Edge { int to, rev, cap; };
    vector<vector<Edge>> g;
    vector<int> level, it;
    explicit Dinic(int n) : g(n), level(n), it(n) {}
    void add(int u, int v, int cap) {
        g[u].push_back({v, (int)g[v].size(), cap});
        g[v].push_back({u, (int)g[u].size() - 1, 0});
    }
    int dfs(int u, int t, int f) {
        if (u == t) return f;
        for (int &i = it[u]; i < (int)g[u].size(); ++i) {
            Edge &e = g[u][i];
            if (e.cap && level[e.to] == level[u] + 1) {
                int z = dfs(e.to, t, min(f, e.cap));
                if (z) {
                    e.cap -= z;
                    g[e.to][e.rev].cap += z;
                    return z;
                }
            }
        }
        return 0;
    }
    int flow(int s, int t) {
        int ans = 0;
        while (true) {
            fill(level.begin(), level.end(), -1);
            level[s] = 0;
            queue<int> q;
            q.push(s);
            while (!q.empty()) {
                int u = q.front(); q.pop();
                for (auto &e : g[u]) if (e.cap && level[e.to] < 0) {
                    level[e.to] = level[u] + 1;
                    q.push(e.to);
                }
            }
            if (level[t] < 0) return ans;
            fill(it.begin(), it.end(), 0);
            while (int z = dfs(s, t, INT_MAX)) ans += z;
        }
    }
};

struct Solver {
    int n, m, limit;
    vector<pair<int,int>> edges;
    vector<vector<int>> adj;
    bool connectedEdge[40][40]{};
    mt19937 rng{712367};

    Grid fromWalk(const vector<int> &w, int k) {
        Grid g(k, vector<int>(k));
        for (int r = 0; r < k; ++r)
            for (int c = 0; c < k; ++c)
                g[r][c] = w[r - c + k - 1];
        return g;
    }

    bool valid(const Grid &g) {
        int k = g.size();
        if (k < 1 || k > limit) return false;
        bool seen[40]{}, got[40][40]{};
        for (int r = 0; r < k; ++r) {
            if ((int)g[r].size() != k) return false;
            for (int c = 0; c < k; ++c) {
                int a = g[r][c];
                if (a < 0 || a >= n) return false;
                seen[a] = true;
                for (auto [dr, dc] : vector<pair<int,int>>{{1,0},{0,1}}) {
                    int rr = r + dr, cc = c + dc;
                    if (rr >= k || cc >= k) continue;
                    int b = g[rr][cc];
                    if (a == b) continue;
                    if (!connectedEdge[a][b]) return false;
                    got[a][b] = got[b][a] = true;
                }
            }
        }
        for (int i = 0; i < n; ++i) if (!seen[i]) return false;
        for (auto [u, v] : edges) if (!got[u][v]) return false;
        return true;
    }

    Grid sparseConstruction() {
        if (m + 1 > limit) return {};
        vector<vector<pair<int,int>>> g(n);
        for (int i = 0; i < m; ++i) {
            auto [u, v] = edges[i];
            for (int z = 0; z < 2; ++z) {
                int id = 2 * i + z;
                g[u].push_back({v, id});
                g[v].push_back({u, id});
            }
        }
        vector<int> ptr(n), st{0}, walk;
        vector<char> used(2 * m);
        while (!st.empty()) {
            int u = st.back();
            while (ptr[u] < (int)g[u].size() && used[g[u][ptr[u]].second]) ++ptr[u];
            if (ptr[u] == (int)g[u].size()) {
                walk.push_back(u);
                st.pop_back();
            } else {
                auto [v, id] = g[u][ptr[u]++];
                used[id] = true;
                st.push_back(v);
            }
        }
        if ((int)walk.size() != 2 * m + 1) return {};
        return fromWalk(walk, m + 1);
    }

    vector<pair<int,int>> treeMatching(const vector<vector<int>> &tree,
                                       const vector<char> &inU,
                                       const vector<int> &depth) {
        vector<int> mt(n, -1);
        function<bool(int, vector<char>&)> aug = [&](int u, vector<char> &seen) {
            for (int v : tree[u]) {
                if (!inU[v] || seen[v]) continue;
                seen[v] = true;
                if (mt[v] == -1 || aug(mt[v], seen)) {
                    mt[v] = u;
                    return true;
                }
            }
            return false;
        };
        for (int u = 0; u < n; ++u) if (inU[u] && depth[u] % 2 == 0) {
            vector<char> seen(n);
            aug(u, seen);
        }
        vector<pair<int,int>> pairs;
        for (int v = 0; v < n; ++v) if (mt[v] != -1) pairs.push_back({mt[v], v});
        return pairs;
    }

    Grid denseAttempt(int root, int strategy, int residue, bool shuffleOrder) {
        vector<vector<int>> order = adj;
        if (shuffleOrder) for (auto &v : order) shuffle(v.begin(), v.end(), rng);
        vector<vector<int>> tree(n), children(n);
        vector<int> parent(n, -2), depth(n);
        parent[root] = -1;
        function<void(int)> dfs = [&](int u) {
            for (int v : order[u]) {
                if (parent[v] != -2) continue;
                parent[v] = u;
                depth[v] = depth[u] + 1;
                tree[u].push_back(v);
                tree[v].push_back(u);
                children[u].push_back(v);
                dfs(v);
            }
        };
        dfs(root);

        vector<int> demand(n);
        for (auto [u, v] : edges) {
            if (parent[u] == v || parent[v] == u) continue;
            if (depth[u] > depth[v]) swap(u, v);
            ++demand[u];
        }

        int end = root;
        for (int u = 0; u < n; ++u)
            if (children[u].empty() && depth[u] > depth[end]) end = u;
        vector<char> onPath(n);
        for (int u = end; u != -1; u = parent[u]) onPath[u] = true;

        vector<int> w{root};
        function<void(int,int)> build = [&](int u, int finalVertex) {
            if (u == finalVertex) return;
            int last = -1;
            if (finalVertex != -1)
                for (int c : children[u]) if (onPath[c]) last = c;
            for (int c : children[u]) {
                if (c == last) continue;
                w.push_back(c);
                build(c, -1);
                w.push_back(u);
            }
            if (last != -1) {
                w.push_back(last);
                build(last, finalVertex);
            }
        };
        build(root, end);

        vector<char> has(n), inU(n);
        for (int t = 1; t + 1 < (int)w.size(); ++t)
            if (w[t-1] == w[t+1]) has[w[t-1]] = true;
        for (int u = 0; u < n; ++u)
            inU[u] = demand[u] && !children[u].empty() && !has[u];

        auto pairs = treeMatching(tree, inU, depth);
        vector<char> paired(n);
        struct Insertion { int at, u, v; };
        vector<Insertion> insertions;
        for (auto [a, b] : pairs) {
            int p = parent[b] == a ? a : b;
            int c = parent[b] == a ? b : a;
            paired[p] = paired[c] = true;
            int at = -1;
            for (int i = 0; i + 1 < (int)w.size(); ++i)
                if (w[i] == p && w[i+1] == c) { at = i; break; }
            if (at == -1) return {};
            insertions.push_back({at, p, c});
        }
        for (int u = 0; u < n; ++u) if (inU[u] && !paired[u]) {
            int at = find(w.begin(), w.end(), u) - w.begin();
            insertions.push_back({at, u, tree[u][0]});
        }
        sort(insertions.begin(), insertions.end(), [](auto a, auto b) { return a.at > b.at; });
        for (auto [at, u, v] : insertions) {
            if (at >= (int)w.size() || w[at] != u) return {};
            w.insert(w.begin() + at + 1, u);
            w.insert(w.begin() + at + 1, v);
        }

        int k = limit, target = 2 * k - 1;
        if ((int)w.size() > target) return {};
        if ((target - (int)w.size()) % 2) w.push_back(w.back());
        vector<int> usedPadding(n);
        while ((int)w.size() < target) {
            int mid = w.size() / 2, v = w[mid];
            if (strategy <= 1) {
                int best = -1;
                for (int u = 0; u < n; ++u) if (demand[u]) {
                    if (best == -1) { best = u; continue; }
                    if (strategy == 0) {
                        long long lhs = 1LL * demand[u] * (1 + usedPadding[best]);
                        long long rhs = 1LL * demand[best] * (1 + usedPadding[u]);
                        if (lhs > rhs || (lhs == rhs && demand[u] > demand[best])) best = u;
                    } else {
                        int a = demand[u] - usedPadding[u];
                        int b = demand[best] - usedPadding[best];
                        if (a > b || (a == b && demand[u] > demand[best])) best = u;
                    }
                }
                if (best != -1) v = best;
            }
            int at = -1;
            for (int i = 0; i < (int)w.size(); ++i) if (w[i] == v) {
                if (at == -1 || abs(i - mid) < abs(at - mid)) at = i;
            }
            int u = adj[v][0];
            w.insert(w.begin() + at + 1, v);
            w.insert(w.begin() + at + 1, u);
            ++usedPadding[v];
        }

        Grid base = fromWalk(w, k);
        bool baseEdge[40][40]{};
        int edgeR[40][40], edgeC[40][40], edgeRR[40][40], edgeCC[40][40];
        for (int r = 0; r < k; ++r) for (int c = 0; c < k; ++c) {
            for (auto [dr, dc] : vector<pair<int,int>>{{1,0},{0,1}}) {
                int rr = r + dr, cc = c + dc;
                if (rr >= k || cc >= k) continue;
                int a = base[r][c], b = base[rr][cc];
                if (a != b && !baseEdge[a][b]) {
                    baseEdge[a][b] = baseEdge[b][a] = true;
                    edgeR[a][b] = edgeR[b][a] = r;
                    edgeC[a][b] = edgeC[b][a] = c;
                    edgeRR[a][b] = edgeRR[b][a] = rr;
                    edgeCC[a][b] = edgeCC[b][a] = cc;
                }
            }
        }
        vector<pair<int,int>> required;
        for (auto [u, v] : edges) if (!baseEdge[u][v]) required.push_back({u, v});
        if (required.empty()) return base;

        bool protectedCell[50][50]{};
        for (int v = 0; v < n; ++v) {
            bool found = false;
            for (int r = 0; r < k && !found; ++r)
                for (int c = 0; c < k; ++c) if (base[r][c] == v) {
                    protectedCell[r][c] = true;
                    found = true;
                    break;
                }
            if (!found) return {};
        }
        for (auto [u, v] : edges) if (baseEdge[u][v]) {
            protectedCell[edgeR[u][v]][edgeC[u][v]] = true;
            protectedCell[edgeRR[u][v]][edgeCC[u][v]] = true;
        }

        int d = 2 * k - 1;
        vector<vector<pair<int,int>>> cells(d);
        for (int r = 0; r < k; ++r) for (int c = 0; c < k; ++c)
            if ((r + c) % 3 == residue && !protectedCell[r][c])
                cells[r - c + k - 1].push_back({r, c});

        int rcount = required.size(), source = rcount + d, sink = source + 1;
        Dinic din(sink + 1);
        for (int i = 0; i < rcount; ++i) din.add(source, i, 1);
        for (int t = 0; t < d; ++t) if (!cells[t].empty())
            din.add(rcount + t, sink, cells[t].size());
        for (int i = 0; i < rcount; ++i) {
            auto [u, v] = required[i];
            for (int t = 1; t + 1 < d; ++t) {
                if (cells[t].empty()) continue;
                int a = w[t-1], b = w[t+1];
                auto compatible = [&](int x) {
                    return (x == a || connectedEdge[x][a]) &&
                           (x == b || connectedEdge[x][b]);
                };
                if (((u == a || u == b) && compatible(v)) ||
                    ((v == a || v == b) && compatible(u)))
                    din.add(i, rcount + t, 1);
            }
        }
        if (din.flow(source, sink) != rcount) return {};

        Grid result = base;
        for (int i = 0; i < rcount; ++i) {
            int t = -1;
            for (auto &e : din.g[i])
                if (e.to >= rcount && e.to < rcount + d && e.cap == 0) {
                    t = e.to - rcount;
                    break;
                }
            if (t == -1 || cells[t].empty()) return {};
            auto [u, v] = required[i];
            int a = w[t-1], b = w[t+1];
            auto compatible = [&](int x) {
                return (x == a || connectedEdge[x][a]) &&
                       (x == b || connectedEdge[x][b]);
            };
            int x;
            if ((u == a || u == b) && compatible(v)) x = v;
            else if ((v == a || v == b) && compatible(u)) x = u;
            else return {};
            auto [r, c] = cells[t].back();
            cells[t].pop_back();
            result[r][c] = x;
        }
        return result;
    }

    Grid solve() {
        if (n == 1) return Grid(1, vector<int>(1, 0));
        if (Grid g = sparseConstruction(); !g.empty() && valid(g)) return g;
        int attempts = max(600, 15 * n);
        for (int at = 0; at < attempts; ++at) {
            int root = at % n;
            int strategy = (at / n) % 3;
            int residue = (at / (3 * n)) % 3;
            Grid g = denseAttempt(root, strategy, residue, at >= n);
            if (!g.empty() && valid(g)) return g;
        }
        return {};
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    while (T--) {
        Solver s;
        cin >> s.n >> s.m;
        s.limit = 5 * s.n / 4;
        s.adj.assign(s.n, {});
        for (int i = 0; i < s.m; ++i) {
            int u, v;
            cin >> u >> v;
            --u; --v;
            s.edges.push_back({u, v});
            s.adj[u].push_back(v);
            s.adj[v].push_back(u);
            s.connectedEdge[u][v] = s.connectedEdge[v][u] = true;
        }
        Grid g = s.solve();
        if (g.empty()) return 1;
        cout << g.size() << '\n';
        for (auto &row : g) {
            for (int i = 0; i < (int)row.size(); ++i) {
                if (i) cout << ' ';
                cout << row[i] + 1;
            }
            cout << '\n';
        }
    }
}

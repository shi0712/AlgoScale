#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;
    DSU() {}
    DSU(int n) { init(n); }
    void init(int n) {
        p.resize(n + 1);
        sz.assign(n + 1, 1);
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    void unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
    }
};

struct AdjEdge {
    int to, e, dir;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    const int LIMIT = 25;

    while (T--) {
        int n;
        cin >> n;

        vector<int> p(n + 1);
        for (int i = 1; i <= n; ++i) cin >> p[i];

        if (n == 1) {
            cout << 1 << '\n';
            continue;
        }

        vector<int> U(n), V(n);
        vector<vector<AdjEdge>> g(n + 1);

        for (int i = 1; i <= n - 1; ++i) {
            int u, v;
            cin >> u >> v;
            --i;
            U[i] = u; V[i] = v;
            g[u].push_back({v, i, 0});
            g[v].push_back({u, i, 1});
            ++i;
        }

        int LOG = 1;
        while ((1 << LOG) <= n) ++LOG;

        vector<int> parent(n + 1), depth(n + 1), parentEdge(n + 1, -1);
        vector<vector<int>> up(LOG, vector<int>(n + 1, 0));

        vector<int> order;
        order.reserve(n);
        vector<int> st;
        st.push_back(1);
        parent[1] = 0;
        depth[1] = 0;
        vector<char> visTree(n + 1, 0);
        visTree[1] = 1;

        while (!st.empty()) {
            int u = st.back();
            st.pop_back();
            order.push_back(u);

            for (auto &ed : g[u]) {
                int v = ed.to;
                if (visTree[v]) continue;
                visTree[v] = 1;
                parent[v] = u;
                parentEdge[v] = ed.e;
                depth[v] = depth[u] + 1;
                st.push_back(v);
            }
        }

        for (int i = 1; i <= n; ++i) up[0][i] = parent[i];
        for (int k = 1; k < LOG; ++k) {
            for (int i = 1; i <= n; ++i) {
                up[k][i] = up[k - 1][up[k - 1][i]];
            }
        }

        auto lca = [&](int a, int b) {
            if (depth[a] < depth[b]) swap(a, b);
            int diff = depth[a] - depth[b];
            for (int k = 0; k < LOG; ++k) {
                if (diff & (1 << k)) a = up[k][a];
            }
            if (a == b) return a;
            for (int k = LOG - 1; k >= 0; --k) {
                if (up[k][a] != up[k][b]) {
                    a = up[k][a];
                    b = up[k][b];
                }
            }
            return parent[a];
        };

        auto dirIndex = [&](int x, int e) -> int {
            return (U[e] == x ? 0 : 1);
        };

        vector<array<unsigned char, 2>> used(n, {0, 0});
        vector<int> capRem(n + 1);
        for (int i = 1; i <= n; ++i) {
            capRem[i] = (int)g[i].size() - 1;
        }

        vector<char> active(n + 1, 1);
        set<int> activeSet;
        for (int i = 1; i <= n; ++i) activeSet.insert(i);

        DSU dsu(n);
        vector<int> answer(n + 1);

        vector<int> visited(n + 1, 0), prv(n + 1, 0);
        int timer = 0;

        vector<int> path;
        vector<int> tail;
        path.reserve(n);
        tail.reserve(n);

        auto checkCandidate = [&](int v, int w, int step, vector<int>* outPath) -> bool {
            if (w == v) return false;

            int l = lca(v, w);

            // Check direction v -> l
            int cur = v;
            while (cur != l) {
                int nxt = parent[cur];
                int e = parentEdge[cur];
                int d = dirIndex(cur, e);
                if (used[e][d]) return false;
                cur = nxt;
            }

            // Check direction l -> w (walk backwards from w)
            cur = w;
            while (cur != l) {
                int nxt = parent[cur];
                int e = parentEdge[cur];
                int d = dirIndex(nxt, e); // nxt -> cur
                if (used[e][d]) return false;
                cur = nxt;
            }

            // Build full path and check internal capacities
            path.clear();
            cur = v;
            while (cur != l) {
                path.push_back(cur);
                cur = parent[cur];
            }
            path.push_back(l);

            tail.clear();
            cur = w;
            while (cur != l) {
                tail.push_back(cur);
                cur = parent[cur];
            }
            for (int i = (int)tail.size() - 1; i >= 0; --i) {
                path.push_back(tail[i]);
            }

            for (int i = 1; i + 1 < (int)path.size(); ++i) {
                int u = path[i];
                if (capRem[u] <= 0) return false;
            }

            if (outPath) *outPath = path;
            return true;
        };

        for (int step = 1; step <= n; ++step) {
            int v = p[step];
            int chosen = -1;
            path.clear();

            int checked = 0;
            bool found = false;

            for (auto it = activeSet.begin(); it != activeSet.end() && checked < LIMIT; ++it) {
                int w = *it;
                if (w == v) continue;

                if (step < n && dsu.find(w) == dsu.find(v)) {
                    ++checked;
                    continue;
                }

                if (checkCandidate(v, w, step, &path)) {
                    chosen = w;
                    found = true;
                    break;
                }
                ++checked;
            }

            if (!found) {
                ++timer;
                vector<int> stack;
                stack.push_back(v);
                visited[v] = timer;
                prv[v] = 0;

                int best = n + 1;

                while (!stack.empty()) {
                    int u = stack.back();
                    stack.pop_back();

                    if (u != v && active[u] && (step == n || dsu.find(u) != dsu.find(v))) {
                        if (u < best) best = u;
                    }

                    if (u == v || capRem[u] > 0) {
                        for (auto &ed : g[u]) {
                            if (used[ed.e][ed.dir]) continue;
                            int nxt = ed.to;
                            if (visited[nxt] != timer) {
                                visited[nxt] = timer;
                                prv[nxt] = u;
                                stack.push_back(nxt);
                            }
                        }
                    }
                }

                chosen = best;

                // Reconstruct path v -> chosen
                vector<int> rev;
                int cur = chosen;
                while (cur != v) {
                    rev.push_back(cur);
                    cur = prv[cur];
                }
                rev.push_back(v);
                reverse(rev.begin(), rev.end());
                path = rev;
            }

            answer[step] = chosen;

            active[chosen] = false;
            activeSet.erase(chosen);

            // Apply the chosen arc: mark used directed edges, consume internal capacities.
            for (int i = 0; i + 1 < (int)path.size(); ++i) {
                int a = path[i], b = path[i + 1];
                int e;
                if (parent[b] == a) e = parentEdge[b];
                else e = parentEdge[a];

                int d = dirIndex(a, e);
                used[e][d] = 1;
            }

            for (int i = 1; i + 1 < (int)path.size(); ++i) {
                --capRem[path[i]];
            }

            dsu.unite(v, chosen);
        }

        for (int i = 1; i <= n; ++i) {
            if (i > 1) cout << ' ';
            cout << answer[i];
        }
        cout << '\n';
    }

    return 0;
}

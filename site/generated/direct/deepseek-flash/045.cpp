#include <bits/stdc++.h>
using namespace std;

const long long INF = 4e18;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, k;
    cin >> n >> m >> k;

    struct Edge {
        int u, v;
        long long w;
    };
    vector<Edge> edges(m);
    for (int i = 0; i < m; ++i) {
        cin >> edges[i].u >> edges[i].v >> edges[i].w;
        --edges[i].u;
        --edges[i].v;
    }

    vector<long long> c(k);
    vector<vector<long long>> a(k, vector<long long>(n));
    for (int j = 0; j < k; ++j) {
        cin >> c[j];
        for (int i = 0; i < n; ++i) {
            cin >> a[j][i];
        }
    }

    if (n == 1) {
        cout << 0 << '\n';
        return 0;
    }

    // Kruskal on the original graph to obtain an MST.
    sort(edges.begin(), edges.end(), [](const Edge& x, const Edge& y) {
        return x.w < y.w;
    });

    vector<int> par(n), sz(n, 1);
    iota(par.begin(), par.end(), 0);
    function<int(int)> find = [&](int x) {
        return par[x] == x ? x : par[x] = find(par[x]);
    };

    vector<vector<pair<int, long long>>> tree(n);
    for (const auto& e : edges) {
        int ru = find(e.u), rv = find(e.v);
        if (ru != rv) {
            par[ru] = rv;
            tree[e.u].push_back({e.v, e.w});
            tree[e.v].push_back({e.u, e.w});
        }
    }

    // Root the MST at vertex 0.
    vector<int> parent(n, -1);
    vector<long long> parentW(n, 0);
    vector<vector<int>> children(n);
    vector<int> order;
    order.reserve(n);

    stack<int> st;
    st.push(0);
    parent[0] = -2;
    while (!st.empty()) {
        int v = st.top();
        st.pop();
        order.push_back(v);
        for (auto [u, w] : tree[v]) {
            if (u == parent[v]) continue;
            parent[u] = v;
            parentW[u] = w;
            children[v].push_back(u);
            st.push(u);
        }
    }

    const int K = 1 << k;

    // DP on the tree.
    // dp[v][mask] = minimum cost for the subtree of v such that the connected
    // component containing v is connected to exactly the hubs in mask.
    function<vector<long long>(int)> dfs = [&](int v) -> vector<long long> {
        vector<long long> cur(K, INF);
        cur[0] = 0;

        // v itself may be connected to any subset of hubs.
        for (int j = 0; j < k; ++j) {
            int bit = 1 << j;
            long long cost = a[j][v];
            for (int mask = 0; mask < K; ++mask) {
                if (!(mask & bit)) {
                    cur[mask | bit] = min(cur[mask | bit], cur[mask] + cost);
                }
            }
        }

        // Merge children one by one.
        for (int u : children[v]) {
            vector<long long> child = dfs(u);
            long long w = parentW[u];
            vector<long long> nxt(K, INF);

            for (int m1 = 0; m1 < K; ++m1) {
                if (cur[m1] >= INF) continue;
                for (int m2 = 0; m2 < K; ++m2) {
                    if (child[m2] >= INF) continue;
                    long long add = (m1 & m2) ? 0 : w;
                    int mask = m1 | m2;
                    long long val = cur[m1] + child[m2] + add;
                    if (val < nxt[mask]) nxt[mask] = val;
                }
            }
            cur.swap(nxt);
        }

        return cur;
    };

    vector<long long> root_dp = dfs(0);

    long long ans = INF;
    for (int mask = 0; mask < K; ++mask) {
        long long cost = root_dp[mask];
        for (int j = 0; j < k; ++j) {
            if (mask & (1 << j)) {
                cost += c[j];
            }
        }
        ans = min(ans, cost);
    }

    cout << ans << '\n';
    return 0;
}

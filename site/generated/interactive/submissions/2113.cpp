#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF64 = (1LL << 60);
const int64 NEG_INF = - (int64)1e12;          // safe large negative

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    // ----- query all intervals -----
    vector<vector<int64>> M(n + 2, vector<int64>(n + 2, 0));
    for (int l = 1; l <= n; ++l) {
        for (int r = l; r <= n; ++r) {
            cout << "? " << l << ' ' << r << "\n";
            cout.flush();
            int64 ans;
            if (!(cin >> ans)) return 0;
            if (ans == -1) return 0;
            M[l][r] = ans;
        }
    }

    // ----- build equality graph (edges for whole maximal intervals) -----
    vector<vector<pair<int,int64>>> eq_adj(n + 1);
    for (int l = 1; l <= n; ++l) {
        for (int r = l; r <= n; ++r) {
            int64 val = M[l][r];
            if (val <= 0) continue;
            bool whole = false;
            if (l == r) whole = true;
            else {
                int64 left  = (l + 1 <= r) ? M[l + 1][r] : -1;
                int64 right = (l <= r - 1) ? M[l][r - 1] : -1;
                if (val > left && val > right) whole = true;
            }
            if (whole) {
                int u = l - 1;
                int v = r;
                eq_adj[u].push_back({v, val});
                eq_adj[v].push_back({u, -val});
            }
        }
    }

    // ----- BFS to obtain relative prefix sums and components -----
    vector<int64> P_rel(n + 1, 0);
    vector<int> comp_id(n + 1, -1);
    vector<char> visited(n + 1, 0);
    int comp_cnt = 0;
    for (int i = 0; i <= n; ++i) {
        if (visited[i]) continue;
        queue<int> q;
        visited[i] = 1;
        P_rel[i] = 0;
        comp_id[i] = comp_cnt;
        q.push(i);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (auto [v, w] : eq_adj[u]) {
                if (!visited[v]) {
                    visited[v] = 1;
                    P_rel[v] = P_rel[u] + w;
                    comp_id[v] = comp_cnt;
                    q.push(v);
                } else {
                    // consistency check – the input guarantees consistency
                }
            }
        }
        ++comp_cnt;
    }

    // ----- build difference constraints for component offsets -----
    struct Edge {int u, v; int64 w;};
    vector<Edge> diff_edges;
    for (int i = 1; i <= n; ++i) {
        int cu = comp_id[i - 1];
        int cv = comp_id[i];
        if (cu == cv) continue;
        int64 delta = P_rel[i] - P_rel[i - 1];
        // C[cv] - C[cu] <= -delta
        diff_edges.push_back({cu, cv, -delta});
    }

    // ----- Bellman‑Ford to find feasible offsets C[·] -----
    vector<int64> C(comp_cnt, 0);          // all start with 0 (super source)
    for (int it = 0; it < comp_cnt; ++it) {
        bool upd = false;
        for (const auto &e : diff_edges) {
            if (C[e.v] > C[e.u] + e.w) {
                C[e.v] = C[e.u] + e.w;
                upd = true;
            }
        }
        if (!upd) break;
    }

    // ----- compute final answer -----
    vector<int64> ans(n + 1);
    for (int i = 1; i <= n; ++i) {
        int64 a_i = (P_rel[i] - P_rel[i - 1]) + (C[comp_id[i]] - C[comp_id[i - 1]]);
        ans[i] = a_i;
    }

    // ----- output -----
    cout << "!";
    for (int i = 1; i <= n; ++i) cout << ' ' << ans[i];
    cout << "\n";
    cout.flush();
    return 0;
}

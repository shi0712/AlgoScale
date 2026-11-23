#include <bits/stdc++.h>
using namespace std;

int main() {
    int n, m;
    scanf("%d %d", &n, &m);
    vector<vector<int>> G(n + 2, vector<int>(n + 2, 0));
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++) scanf("%d", &G[i][j]);
    const int INF = INT_MAX;
    // LB[l][T]
    vector<vector<int>> LB(n + 1, vector<int>(2 * n + 1, 0));
    // prefix max of row 1
    vector<int> pm(n + 2, 0);
    for (int j = 1; j <= n; j++) pm[j] = max(pm[j - 1], G[1][j]);
    // formula 2: path from (2,1)
    {
        for (int l = 1; l <= n; l++) LB[l][1] = pm[l];
        vector<int> g(n + 2, INF), ng(n + 2, INF);
        g[2] = G[2][1];
        // diag 3
        vector<int> h(2 * n + 2, INF);
        h[3] = G[2][1];
        for (int d = 4; d <= 2 * n; d++) {
            int lo = max(2, d - n), hi = min(n, d - 1);
            for (int x = 1; x <= n; x++) ng[x] = INF;
            int mn = INF;
            for (int x = lo; x <= hi; x++) {
                int best = min(g[x - 1], g[x]);
                if (best == INF) continue;
                ng[x] = max(G[x][d - x], best);
                mn = min(mn, ng[x]);
            }
            h[d] = mn;
            swap(g, ng);
        }
        for (int T = 2; T <= 2 * n - 1; T++) {
            for (int l = T; l <= n; l++) {
                int rm = pm[l - T + 1];
                LB[l][T] = max(h[T + 1], rm);
            }
        }
    }
    // formula 1
    {
        vector<int> g(n + 2, INF), ng(n + 2, INF);
        for (int a = 3; a <= 2 * n; a++) {
            for (int x = 1; x <= n; x++) g[x] = INF;
            int lo = max(2, a - n), hi = min(n, a - 1);
            for (int x = lo; x <= hi; x++) g[x] = G[x][a - x];
            for (int l = 1; l <= n; l++) {
                int b = a + l - 1;
                if (b > 2 * n) break;
                if (l > 1) {
                    int lo2 = max(2, b - n), hi2 = min(n, b - 1);
                    for (int x = 1; x <= n; x++) ng[x] = INF;
                    for (int x = lo2; x <= hi2; x++) {
                        int best = min(g[x - 1], g[x]);
                        if (best == INF) continue;
                        ng[x] = max(G[x][b - x], best);
                    }
                    swap(g, ng);
                }
                int T = b - 1;
                if (T > l) {
                    int lo2 = max(2, b - n), hi2 = min(n, b - 1);
                    int mn = INF;
                    for (int x = lo2; x <= hi2; x++) mn = min(mn, g[x]);
                    LB[l][T] = mn;
                }
            }
        }
    }
    vector<array<int, 3>> pairs;
    pairs.reserve((size_t)n * (2 * n - 1));
    for (int l = 1; l <= n; l++)
        for (int T = 1; T <= 2 * n - 1; T++) pairs.push_back({LB[l][T], l, T});
    sort(pairs.begin(), pairs.end());
    priority_queue<int> pq;
    for (auto &p : pairs) {
        if ((int)pq.size() == m && pq.top() <= p[0]) break;
        printf("? %d %d\n", p[1], p[2]);
        fflush(stdout);
        int v;
        if (scanf("%d", &v) != 1) return 0;
        if (v == -1) return 0;
        pq.push(v);
        if ((int)pq.size() > m) pq.pop();
    }
    vector<int> ans;
    while (!pq.empty()) {
        ans.push_back(pq.top());
        pq.pop();
    }
    reverse(ans.begin(), ans.end());
    printf("!");
    for (int v : ans) printf(" %d", v);
    printf("\n");
    fflush(stdout);
    return 0;
}

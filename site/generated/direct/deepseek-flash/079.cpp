#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int u, v, w;
};

static double karpCost(const vector<double>& cost, int K, const vector<Edge>& edges,
                       bool extract, double& outVar) {
    const double INF = 1e100;
    outVar = INF;

    if (K == 0) return INF;

    vector<vector<double>> f(K + 1, vector<double>(K, INF));
    for (int v = 0; v < K; ++v) f[0][v] = 0.0;

    vector<vector<int>> prv, pw;
    if (extract) {
        prv.assign(K + 1, vector<int>(K, -1));
        pw.assign(K + 1, vector<int>(K, 0));
    }

    const int M = (int)edges.size();

    for (int step = 1; step <= K; ++step) {
        auto& cur = f[step];
        const auto& pre = f[step - 1];

        for (int ei = 0; ei < M; ++ei) {
            int u = edges[ei].u;
            int v = edges[ei].v;
            int w = edges[ei].w;

            if (pre[u] >= INF / 2) continue;

            double val = pre[u] + cost[ei];
            if (val < cur[v]) {
                cur[v] = val;
                if (extract) {
                    prv[step][v] = u;
                    pw[step][v] = w;
                }
            }
        }
    }

    double mu = INF;
    int bestV = -1;

    for (int v = 0; v < K; ++v) {
        if (f[K][v] >= INF / 2) continue;

        double mx = -INF;
        for (int k = 0; k < K; ++k) {
            if (f[k][v] >= INF / 2) continue;
            double val = (f[K][v] - f[k][v]) / (double)(K - k);
            if (val > mx) mx = val;
        }

        if (mx < mu) {
            mu = mx;
            bestV = v;
        }
    }

    if (mu >= INF / 2) return INF;

    if (extract && bestV != -1) {
        vector<int> verts(K + 1);
        vector<int> ws(K);

        int v = bestV;
        verts[K] = v;
        bool ok = true;

        for (int s = K; s >= 1; --s) {
            int u = prv[s][v];
            if (u == -1) {
                ok = false;
                break;
            }
            ws[s - 1] = pw[s][v];
            verts[s - 1] = u;
            v = u;
        }

        double bestVar = INF;

        if (ok) {
            for (int p = 0; p < K; ++p) {
                int W = 0, Q = 0;
                for (int i = p + 1; i <= K; ++i) {
                    int ww = ws[i - 1];
                    W += ww;
                    Q += ww * ww;

                    if (verts[p] == verts[i]) {
                        int L = i - p;
                        double mean = (double)W / L;
                        double var = (double)Q / L - mean * mean;
                        if (var < 0.0 && var > -1e-12) var = 0.0;
                        bestVar = min(bestVar, var);
                    }
                }
            }
        }

        outVar = bestVar;
    }

    return mu;
}

struct Interval {
    double lb, l, r;

    bool operator>(const Interval& other) const {
        return lb > other.lb;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    struct InputEdge {
        int x, y, w;
    };

    vector<InputEdge> input(m);
    vector<vector<int>> adj(n), radj(n);

    for (int i = 0; i < m; ++i) {
        int x, y, w;
        cin >> x >> y >> w;
        --x;
        --y;
        input[i] = {x, y, w};
        adj[x].push_back(y);
        radj[y].push_back(x);
    }

    auto bfs = [&](int src, vector<int>& vis, const vector<vector<int>>& g) {
        queue<int> q;
        q.push(src);
        vis[src] = 1;

        while (!q.empty()) {
            int v = q.front();
            q.pop();

            for (int to : g[v]) {
                if (!vis[to]) {
                    vis[to] = 1;
                    q.push(to);
                }
            }
        }
    };

    vector<int> reach1(n, 0), reachN(n, 0);
    bfs(0, reach1, adj);
    bfs(n - 1, reachN, radj);

    if (!reach1[n - 1]) {
        cout << -1 << '\n';
        return 0;
    }

    vector<int> id(n, -1);
    int K = 0;

    for (int i = 0; i < n; ++i) {
        if (reach1[i] && reachN[i]) {
            id[i] = K++;
        }
    }

    vector<Edge> edges;
    edges.reserve(m);

    for (const auto& e : input) {
        if (id[e.x] != -1 && id[e.y] != -1) {
            edges.push_back({id[e.x], id[e.y], e.w});
        }
    }

    int start = id[0];
    int finish = id[n - 1];

    const int INF_INT = 1e9;
    const double INF_DOUBLE = 1e100;

    double ansPath = INF_DOUBLE;

    int maxW = 20 * K;
    vector<vector<int>> dp(K, vector<int>(maxW + 1, INF_INT));
    dp[start][0] = 0;

    for (int len = 1; len <= K - 1; ++len) {
        vector<vector<int>> ndp(K, vector<int>(maxW + 1, INF_INT));

        int limW = 20 * (len - 1);

        for (const auto& e : edges) {
            int u = e.u;
            int v = e.v;
            int w = e.w;

            for (int W = 0; W <= limW; ++W) {
                int cur = dp[u][W];
                if (cur == INF_INT) continue;

                int nW = W + w;
                if (nW > 20 * len) continue;

                int nQ = cur + w * w;
                if (nQ < ndp[v][nW]) {
                    ndp[v][nW] = nQ;
                }
            }
        }

        for (int W = 0; W <= 20 * len; ++W) {
            if (ndp[finish][W] == INF_INT) continue;

            double mean = (double)W / len;
            double var = (double)ndp[finish][W] / len - mean * mean;
            if (var < 0.0 && var > -1e-12) var = 0.0;

            ansPath = min(ansPath, var);
        }

        dp.swap(ndp);
    }

    if (ansPath < 0.0 && ansPath > -1e-12) ansPath = 0.0;

    double best = ansPath;

    if (best <= 1e-15) {
        cout << fixed << setprecision(15) << 0.0 << '\n';
        return 0;
    }

    if (edges.empty()) {
        cout << fixed << setprecision(15) << best << '\n';
        return 0;
    }

    const double GAP = 1e-10;
    const double DELTA = 1e-6;

    int M = (int)edges.size();
    vector<double> costs(M);

    for (int i = 0; i < M; ++i) costs[i] = 0.0;

    double dummyVar;
    double rootLB = karpCost(costs, K, edges, false, dummyVar);

    if (rootLB >= INF_DOUBLE / 2) {
        cout << fixed << setprecision(15) << best << '\n';
        return 0;
    }

    priority_queue<Interval, vector<Interval>, greater<Interval>> pq;
    pq.push({rootLB, 0.0, 20.0});

    while (!pq.empty()) {
        Interval cur = pq.top();

        if (best - cur.lb <= GAP) break;

        pq.pop();

        if (cur.lb >= best - GAP) continue;

        for (int i = 0; i < M; ++i) {
            int w = edges[i].w;
            double d = 0.0;

            if (w < cur.l) d = cur.l - w;
            else if (w > cur.r) d = w - cur.r;

            costs[i] = d * d;
        }

        double lb = karpCost(costs, K, edges, false, dummyVar);

        if (lb >= best - GAP) continue;

        double mid = (cur.l + cur.r) * 0.5;

        for (int i = 0; i < M; ++i) {
            double d = edges[i].w - mid;
            costs[i] = d * d;
        }

        double var;
        double val = karpCost(costs, K, edges, true, var);

        if (val < best) best = val;
        if (var < best) best = var;

        if (best < 0.0 && best > -1e-12) best = 0.0;

        if (best <= 1e-15) break;
        if (lb >= best - GAP) continue;
        if (cur.r - cur.l <= DELTA) continue;

        double m = (cur.l + cur.r) * 0.5;

        pq.push({lb, cur.l, m});
        pq.push({lb, m, cur.r});
    }

    if (best < 0.0 && best > -1e-12) best = 0.0;

    cout << fixed << setprecision(15) << best << '\n';

    return 0;
}

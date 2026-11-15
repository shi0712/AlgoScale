#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static const ll INFLL = (1LL << 62);

struct MinCostFlow {
    struct Edge { int to, rev, cap; ll cost; };
    int n;
    vector<vector<Edge>> g;
    MinCostFlow(int n = 0) : n(n), g(n) {}
    void add_edge(int u, int v, int cap, ll cost) {
        Edge a{v, (int)g[v].size(), cap, cost};
        Edge b{u, (int)g[u].size(), 0, -cost};
        g[u].push_back(a);
        g[v].push_back(b);
    }
    pair<int, ll> run(int s, int t, int maxf) {
        int flow = 0;
        ll cost = 0;
        vector<ll> h(n, 0), dist(n);
        vector<int> pv(n), pe(n);
        using P = pair<ll, int>;
        priority_queue<P, vector<P>, greater<P>> pq;
        while (flow < maxf) {
            fill(dist.begin(), dist.end(), INFLL);
            dist[s] = 0;
            pq.push({0, s});
            while (!pq.empty()) {
                auto [d, v] = pq.top();
                pq.pop();
                if (d != dist[v]) continue;
                for (int i = 0; i < (int)g[v].size(); ++i) {
                    const Edge &e = g[v][i];
                    if (e.cap <= 0) continue;
                    ll nd = d + e.cost + h[v] - h[e.to];
                    if (nd < dist[e.to]) {
                        dist[e.to] = nd;
                        pv[e.to] = v;
                        pe[e.to] = i;
                        pq.push({nd, e.to});
                    }
                }
            }
            if (dist[t] == INFLL) break;
            for (int v = 0; v < n; ++v)
                if (dist[v] < INFLL) h[v] += dist[v];
            int add = maxf - flow;
            for (int v = t; v != s; v = pv[v])
                add = min(add, g[pv[v]][pe[v]].cap);
            for (int v = t; v != s; v = pv[v]) {
                Edge &e = g[pv[v]][pe[v]];
                e.cap -= add;
                g[v][e.rev].cap += add;
            }
            flow += add;
            cost += (ll)add * (h[t] - h[s]);
        }
        return {flow, cost};
    }
};

struct Casino {
    int p = -1;      // player with maximum unique positive count
    int q = -1;      // player with second maximum unique positive count
    ll U = 0;        // maximum unique positive count (0 if none)
    ll winCost = 1;  // smallest b > U not present among existing counts
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<Casino> cas(n);
    for (int i = 0; i < n; ++i) {
        vector<pair<int, int>> vp(m);
        for (int j = 0; j < m; ++j) {
            int x;
            cin >> x;
            vp[j] = {x, j};
        }
        sort(vp.begin(), vp.end());
        vector<int> distinct;
        distinct.reserve(m);
        int maxU = -1, p = -1, secondU = -1, q = -1;
        for (int l = 0; l < m;) {
            int r = l;
            while (r < m && vp[r].first == vp[l].first) ++r;
            int val = vp[l].first;
            distinct.push_back(val);
            if (r - l == 1 && val > 0) {
                int player = vp[l].second;
                if (val > maxU) {
                    secondU = maxU;
                    q = p;
                    maxU = val;
                    p = player;
                } else if (val > secondU) {
                    secondU = val;
                    q = player;
                }
            }
            l = r;
        }
        Casino c;
        c.U = (maxU > 0 ? (ll)maxU : 0);
        c.p = (maxU > 0 ? p : -1);
        c.q = (secondU > 0 ? q : -1);
        ll x = c.U + 1;
        int idx = (int)(lower_bound(distinct.begin(), distinct.end(), (int)x) - distinct.begin());
        while (idx < (int)distinct.size() && (ll)distinct[idx] == x) {
            ++x;
            ++idx;
        }
        c.winCost = x;
        cas[i] = c;
    }

    // Node indices.
    int S = 0;
    int casBase = 1;
    int playBase = casBase + n;
    int N = playBase + m;
    int WIN = N + 1;
    int T = WIN + 1;
    int Q = T + 1;
    int V = Q + 1;

    auto compute = [&](int k, vector<int>* outAction) -> ll {
        MinCostFlow mcf(V);
        for (int i = 0; i < n; ++i)
            mcf.add_edge(S, casBase + i, 1, 0);

        vector<array<int, 3>> eidx(n);
        for (int i = 0; i < n; ++i) {
            int cnode = casBase + i;
            const Casino &c = cas[i];

            // Default: keep the current winner.
            eidx[i][0] = (int)mcf.g[cnode].size();
            if (c.p != -1) mcf.add_edge(cnode, playBase + c.p, 1, 0);
            else mcf.add_edge(cnode, N, 1, 0);

            // Cancel the maximum unique player; the second one wins (or nobody).
            eidx[i][1] = (int)mcf.g[cnode].size();
            if (c.q != -1) mcf.add_edge(cnode, playBase + c.q, 1, c.U);
            else mcf.add_edge(cnode, N, 1, c.U);

            // Win this casino outright.
            eidx[i][2] = (int)mcf.g[cnode].size();
            mcf.add_edge(cnode, WIN, 1, c.winCost);
        }

        for (int j = 0; j < m; ++j)
            mcf.add_edge(playBase + j, T, k, 0);
        mcf.add_edge(N, T, n, 0);
        mcf.add_edge(WIN, Q, k, 0);
        mcf.add_edge(WIN, T, n - k, 0);
        mcf.add_edge(T, Q, n - k, 0);

        auto [f, cost] = mcf.run(S, Q, n);
        if (outAction) {
            outAction->assign(n, -1);
            for (int i = 0; i < n; ++i) {
                int cnode = casBase + i;
                for (int t = 0; t < 3; ++t) {
                    if (mcf.g[cnode][eidx[i][t]].cap == 0) {
                        (*outAction)[i] = t;
                        break;
                    }
                }
            }
        }
        if (f < n) return INFLL;
        return cost;
    };

    vector<ll> memo(n + 1, -1);
    auto getF = [&](int k) -> ll {
        if (memo[k] != -1) return memo[k];
        memo[k] = compute(k, nullptr);
        return memo[k];
    };

    // F(k) = minimum cost with at least k of our wins and every opponent <= k.
    // It is a convex function of k. Find the first k with F(k+1) >= F(k).
    int lo = 0, hi = n - 1, bestK = n;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        ll f0 = getF(mid);
        ll f1 = getF(mid + 1);
        if (f1 >= f0) {
            bestK = mid;
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }

    vector<int> action;
    compute(bestK, &action);

    vector<ll> b(n, 0);
    for (int i = 0; i < n; ++i) {
        int act = action[i];
        if (act == 0) b[i] = 0;
        else if (act == 1) b[i] = cas[i].U;
        else b[i] = cas[i].winCost;
    }
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << b[i];
    }
    cout << '\n';
    return 0;
}

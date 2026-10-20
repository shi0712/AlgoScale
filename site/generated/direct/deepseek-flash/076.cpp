#include <bits/stdc++.h>
using namespace std;

using ll = long long;

struct MCF {
    struct Edge {
        int to, rev, cap;
        ll cost;
    };

    vector<vector<Edge>> g;

    MCF(int n) : g(n) {}

    int addEdge(int u, int v, int cap, ll cost) {
        Edge a{v, (int)g[v].size(), cap, cost};
        Edge b{u, (int)g[u].size(), 0, -cost};
        g[u].push_back(a);
        g[v].push_back(b);
        return (int)g[u].size() - 1;
    }

    void minCostVariable(int s, int t, ll K) {
        int n = (int)g.size();
        const ll INF = (1LL << 62);

        vector<ll> pot(n, 0), dist(n);
        vector<int> pv(n), pe(n);

        while (true) {
            fill(dist.begin(), dist.end(), INF);
            dist[s] = 0;

            priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<pair<ll, int>>> pq;
            pq.push({0, s});

            while (!pq.empty()) {
                auto [d, u] = pq.top();
                pq.pop();

                if (d != dist[u]) continue;

                for (int i = 0; i < (int)g[u].size(); ++i) {
                    const Edge &e = g[u][i];
                    if (e.cap <= 0) continue;

                    ll nd = d + e.cost + pot[u] - pot[e.to];
                    if (nd < dist[e.to]) {
                        dist[e.to] = nd;
                        pv[e.to] = u;
                        pe[e.to] = i;
                        pq.push({nd, e.to});
                    }
                }
            }

            if (dist[t] == INF) break;

            ll pathCost = dist[t] + pot[t] - pot[s];
            if (pathCost >= K) break;

            for (int v = 0; v < n; ++v) {
                if (dist[v] < INF) pot[v] += dist[v];
            }

            int v = t;
            while (v != s) {
                int u = pv[v];
                int ei = pe[v];

                Edge &e = g[u][ei];
                e.cap -= 1;
                g[v][e.rev].cap += 1;

                v = u;
            }
        }
    }
};

struct Arc {
    int u, v, idx;
    ll w;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    const ll INFLL = (1LL << 62);

    while (T--) {
        int n, m, k;
        cin >> n >> m >> k;

        vector<vector<ll>> hor(n, vector<ll>(m - 1));
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m - 1; ++j) {
                cin >> hor[i][j];
            }
        }

        vector<vector<ll>> ver(n - 1, vector<ll>(m));
        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < m; ++j) {
                cin >> ver[i][j];
            }
        }

        int N = n * m;

        auto cell = [&](int i, int j) {
            return i * m + j;
        };

        // Original shortest distance D, with any first-column cell as source
        // and any last-column cell as target.
        vector<ll> dist(N, INFLL);
        priority_queue<pair<ll, int>, vector<pair<ll, int>>, greater<pair<ll, int>>> pq;

        for (int i = 0; i < n; ++i) {
            int id = cell(i, 0);
            dist[id] = 0;
            pq.push({0, id});
        }

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (d != dist[u]) continue;

            int i = u / m;
            int j = u % m;

            auto relax = [&](int v, ll w) {
                if (dist[v] > d + w) {
                    dist[v] = d + w;
                    pq.push({dist[v], v});
                }
            };

            if (j + 1 < m) relax(u + 1, hor[i][j]);
            if (j > 0) relax(u - 1, hor[i][j - 1]);
            if (i + 1 < n) relax(u + m, ver[i][j]);
            if (i > 0) relax(u - m, ver[i - 1][j]);
        }

        ll D = INFLL;
        for (int i = 0; i < n; ++i) {
            D = min(D, dist[cell(i, m - 1)]);
        }

        ll K = D + k;

        int S = N;
        int TT = N + 1;

        MCF mcf(N + 2);

        const int INF_CAP = 1000000000;

        // Super source and super sink.
        for (int i = 0; i < n; ++i) {
            mcf.addEdge(S, cell(i, 0), INF_CAP, 0);
            mcf.addEdge(cell(i, m - 1), TT, INF_CAP, 0);
        }

        vector<Arc> arcs;
        arcs.reserve(2 * (n * (m - 1) + (n - 1) * m));

        auto addPhysicalEdge = [&](int u, int v, ll w) {
            int id1 = mcf.addEdge(u, v, 1, w);
            arcs.push_back({u, v, id1, w});

            int id2 = mcf.addEdge(v, u, 1, w);
            arcs.push_back({v, u, id2, w});
        };

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m - 1; ++j) {
                int u = cell(i, j);
                int v = cell(i, j + 1);
                addPhysicalEdge(u, v, hor[i][j]);
            }
        }

        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < m; ++j) {
                int u = cell(i, j);
                int v = cell(i + 1, j);
                addPhysicalEdge(u, v, ver[i][j]);
            }
        }

        mcf.minCostVariable(S, TT, K);

        // Build difference constraints for potentials h.
        int R = N;
        int VH = N + 1;

        vector<vector<pair<int, ll>>> diff(VH);

        for (const Arc &a : arcs) {
            bool used = (mcf.g[a.u][a.idx].cap == 0);

            if (used) {
                // h_v - h_u >= w  <=>  h_u <= h_v - w
                diff[a.v].push_back({a.u, -a.w});
            } else {
                // h_v - h_u <= w
                diff[a.u].push_back({a.v, a.w});
            }
        }

        // All first-column cells have potential 0.
        for (int i = 0; i < n; ++i) {
            int c = cell(i, 0);
            diff[R].push_back({c, 0});
            diff[c].push_back({R, 0});
        }

        // All last-column cells have potential K.
        for (int i = 0; i < n; ++i) {
            int c = cell(i, m - 1);
            diff[R].push_back({c, K});
            diff[c].push_back({R, -K});
        }

        // SPFA for shortest paths in the difference constraints graph.
        vector<ll> h(VH, INFLL);
        vector<char> inq(VH, 0);
        deque<int> q;

        h[R] = 0;
        q.push_back(R);
        inq[R] = 1;

        while (!q.empty()) {
            int u = q.front();
            q.pop_front();
            inq[u] = 0;

            for (auto [v, c] : diff[u]) {
                if (h[v] > h[u] + c) {
                    h[v] = h[u] + c;

                    if (!inq[v]) {
                        if (!q.empty() && h[v] < h[q.front()]) {
                            q.push_front(v);
                        } else {
                            q.push_back(v);
                        }
                        inq[v] = 1;
                    }
                }
            }
        }

        ll answer = 0;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m - 1; ++j) {
                int u = cell(i, j);
                int v = cell(i, j + 1);

                ll diffH = llabs(h[u] - h[v]);
                ll nw = max(hor[i][j], diffH);

                answer += nw - hor[i][j];
                hor[i][j] = nw;
            }
        }

        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < m; ++j) {
                int u = cell(i, j);
                int v = cell(i + 1, j);

                ll diffH = llabs(h[u] - h[v]);
                ll nw = max(ver[i][j], diffH);

                answer += nw - ver[i][j];
                ver[i][j] = nw;
            }
        }

        cout << answer << '\n';

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m - 1; ++j) {
                if (j) cout << ' ';
                cout << hor[i][j];
            }
            cout << '\n';
        }

        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < m; ++j) {
                if (j) cout << ' ';
                cout << ver[i][j];
            }
            cout << '\n';
        }
    }

    return 0;
}

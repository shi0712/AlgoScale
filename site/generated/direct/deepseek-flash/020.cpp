#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int to, next;
    int cap, cost;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    string s;
    cin >> n;
    cin >> s;
    vector<int> c(n);
    for (int i = 0; i < n; ++i) cin >> c[i];

    int cntA = 0, cntB = 0;
    for (int i = 0; i < n; ++i) {
        if (c[i] == 0 || c[i] == 2) ++cntA;
        if (c[i] == 1 || c[i] == 2) ++cntB;
    }

    if ((cntA & 1) || (cntB & 1)) {
        cout << -1 << '\n';
        return 0;
    }

    int mA = cntA / 2; // number of A-slots = (colors 0,2)/2
    int mB = cntB / 2; // number of B-slots = (colors 1,2)/2

    // Node ids
    int s0 = 0;
    int t0 = 1;
    auto A = [&](int i) { // i = 1..mA
        return 2 + (i - 1);
    };
    auto B = [&](int i) { // i = 1..mB
        return 2 + mA + (i - 1);
    };
    int SS = 2 + mA + mB;
    int TT = SS + 1;
    int N = TT + 1;

    vector<int> head(N, -1);
    vector<Edge> edges;
    edges.reserve( (size_t)(4 * n + 4 * (mA + mB) + 10) );

    auto addEdge = [&](int u, int v, int cap, int cost) {
        edges.push_back({v, head[u], cap, cost});
        head[u] = (int)edges.size() - 1;
        edges.push_back({u, head[v], 0, -cost});
        head[v] = (int)edges.size() - 1;
    };

    const int INF = 1000000000;

    // Lower-bound transformation:
    // s0 -> A_i lower 1, B_j -> t0 lower 1
    for (int i = 1; i <= mA; ++i) {
        addEdge(SS, A(i), 1, 0);      // A_i has +1 balance
    }
    addEdge(s0, TT, mA, 0);           // s0 has -mA balance

    for (int j = 1; j <= mB; ++j) {
        addEdge(B(j), TT, 1, 0);      // B_j has -1 balance
    }
    addEdge(SS, t0, mB, 0);           // t0 has +mB balance

    addEdge(t0, s0, INF, 0);

    // A chain: A_i -> A_{i+1}
    for (int i = 1; i + 1 <= mA; ++i) {
        addEdge(A(i), A(i + 1), INF, 0);
    }

    // B chain: B_j -> B_{j-1}
    for (int j = 2; j <= mB; ++j) {
        addEdge(B(j), B(j - 1), INF, 0);
    }

    long long base = 0;
    for (char ch : s) if (ch == ')') ++base;

    int ca = 0, cb = 0;
    for (int i = 0; i < n; ++i) {
        bool inA = (c[i] == 0 || c[i] == 2);
        bool inB = (c[i] == 1 || c[i] == 2);

        int a = 0, b = 0;
        if (inA) {
            ++ca;
            a = ca / 2;
        }
        if (inB) {
            ++cb;
            b = cb / 2;
        }

        int delta = (s[i] == ')') ? -1 : +1;

        if (c[i] == 0) {
            if (a >= 1) addEdge(A(a), t0, 1, delta);
        } else if (c[i] == 1) {
            if (b >= 1) addEdge(s0, B(b), 1, delta);
        } else { // color 2
            if (a >= 1 && b >= 1) {
                addEdge(A(a), B(b), 1, delta);
            }
        }
    }

    long long need = (long long)mA + mB;

    if (need == 0) {
        cout << base << '\n';
        return 0;
    }

    // Initial potentials by shortest paths in the initial DAG.
    vector<int> pot(N, INF);
    pot[SS] = 0;

    vector<int> order;
    order.reserve(N);
    order.push_back(SS);
    for (int i = 1; i <= mA; ++i) order.push_back(A(i));
    order.push_back(t0);
    order.push_back(s0);
    for (int j = mB; j >= 1; --j) order.push_back(B(j));
    order.push_back(TT);

    for (int u : order) {
        if (pot[u] == INF) continue;
        for (int e = head[u]; e != -1; e = edges[e].next) {
            if (edges[e].cap == 0) continue;
            int v = edges[e].to;
            int w = edges[e].cost;
            if (pot[v] > pot[u] + w) {
                pot[v] = pot[u] + w;
            }
        }
    }
    for (int i = 0; i < N; ++i) {
        if (pot[i] == INF) pot[i] = 0;
    }

    auto reduced_cost = [&](int e, int u, int v) -> int {
        return edges[e].cost + pot[u] - pot[v];
    };

    // Push-relabel max flow using only zero reduced-cost edges.
    auto max_flow_zero = [&](long long limit) -> pair<long long, long long> {
        vector<int> excess(N, 0), height(N, 0), cur(N);
        vector<char> active(N, 0);
        deque<int> q;

        height[SS] = N;
        for (int i = 0; i < N; ++i) cur[i] = head[i];

        long long reached = 0;
        long long pushed_from_source = 0;

        // Initial pushes from SS.
        for (int e = head[SS]; e != -1; e = edges[e].next) {
            int v = edges[e].to;
            if (edges[e].cap == 0) continue;
            if (reduced_cost(e, SS, v) != 0) continue;

            long long can = min((long long)edges[e].cap, limit - pushed_from_source);
            if (can <= 0) break;
            int f = (int)can;

            edges[e].cap -= f;
            edges[e ^ 1].cap += f;
            excess[v] += f;
            pushed_from_source += f;

            if (v != TT && v != SS && !active[v]) {
                active[v] = 1;
                q.push_back(v);
            }
            if (pushed_from_source == limit) break;
        }

        while (!q.empty()) {
            int u = q.front();
            q.pop_front();
            active[u] = 0;

            while (excess[u] > 0) {
                int &e = cur[u];
                while (e != -1) {
                    int v = edges[e].to;
                    if (edges[e].cap > 0 &&
                        height[u] == height[v] + 1 &&
                        reduced_cost(e, u, v) == 0) {
                        break;
                    }
                    e = edges[e].next;
                }

                if (e != -1) {
                    int v = edges[e].to;
                    int f = min(excess[u], edges[e].cap);

                    edges[e].cap -= f;
                    edges[e ^ 1].cap += f;
                    excess[u] -= f;

                    if (v == TT) {
                        reached += f;
                    } else if (v != SS) {
                        excess[v] += f;
                        if (!active[v]) {
                            active[v] = 1;
                            q.push_back(v);
                        }
                    }

                    if (excess[u] == 0) break;
                } else {
                    int best = INF;
                    for (int ee = head[u]; ee != -1; ee = edges[ee].next) {
                        int v = edges[ee].to;
                        if (edges[ee].cap > 0 && reduced_cost(ee, u, v) == 0) {
                            best = min(best, height[v]);
                        }
                    }
                    if (best == INF) {
                        height[u] = N + 1;
                        break;
                    }
                    height[u] = best + 1;
                    cur[u] = head[u];
                }
            }

            if (excess[u] > 0 && height[u] <= N) {
                if (!active[u]) {
                    active[u] = 1;
                    q.push_back(u);
                }
            }
        }

        return {reached, pushed_from_source};
    };

    long long flow = 0;
    long long minCost = 0;

    vector<int> dist(N), pv(N), pe(N);
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

    while (flow < need) {
        fill(dist.begin(), dist.end(), INF);
        dist[SS] = 0;
        pq.push({0, SS});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();
            if (d != dist[u]) continue;

            for (int e = head[u]; e != -1; e = edges[e].next) {
                if (edges[e].cap == 0) continue;
                int v = edges[e].to;
                int rc = reduced_cost(e, u, v);
                if (rc < 0) rc = 0; // safety, should not happen
                if (dist[v] > dist[u] + rc) {
                    dist[v] = dist[u] + rc;
                    pv[v] = u;
                    pe[v] = e;
                    pq.push({dist[v], v});
                }
            }
        }

        if (dist[TT] == INF) break;

        for (int i = 0; i < N; ++i) {
            if (dist[i] < INF) pot[i] += dist[i];
        }

        long long phaseCost = (long long)pot[TT] - pot[SS];

        auto [pushed, _] = max_flow_zero(need - flow);
        if (pushed == 0) break;

        flow += pushed;
        minCost += pushed * phaseCost;
    }

    if (flow < need) {
        cout << -1 << '\n';
    } else {
        cout << base + minCost << '\n';
    }

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = numeric_limits<int64>::max() / 4;

struct MinCostFlow {
    struct Edge {
        int to, rev, cap;
        int64 cost;
    };

    int n;
    vector<vector<Edge>> g;

    explicit MinCostFlow(int n) : n(n), g(n) {}

    int addEdge(int v, int to, int cap, int64 cost) {
        int idx = (int)g[v].size();
        g[v].push_back({to, (int)g[to].size(), cap, cost});
        g[to].push_back({v, idx, 0, -cost});
        return idx;
    }

    pair<int, int64> run(int s, int t, int maxFlow) {
        vector<int64> potential(n, 0), dist(n);
        vector<int> prevNode(n), prevEdge(n);
        int flow = 0;
        int64 cost = 0;

        while (flow < maxFlow) {
            fill(dist.begin(), dist.end(), INF);
            dist[s] = 0;
            priority_queue<pair<int64, int>, vector<pair<int64, int>>, greater<>> pq;
            pq.push({0, s});

            while (!pq.empty()) {
                auto [d, v] = pq.top();
                pq.pop();
                if (d != dist[v]) continue;
                for (int i = 0; i < (int)g[v].size(); ++i) {
                    const Edge &e = g[v][i];
                    if (e.cap == 0) continue;
                    int64 nd = d + e.cost + potential[v] - potential[e.to];
                    if (nd < dist[e.to]) {
                        dist[e.to] = nd;
                        prevNode[e.to] = v;
                        prevEdge[e.to] = i;
                        pq.push({nd, e.to});
                    }
                }
            }

            if (dist[t] == INF) break;
            for (int v = 0; v < n; ++v) {
                if (dist[v] != INF) potential[v] += dist[v];
            }

            int add = maxFlow - flow;
            for (int v = t; v != s; v = prevNode[v]) {
                add = min(add, g[prevNode[v]][prevEdge[v]].cap);
            }
            int64 pathCost = 0;
            for (int v = t; v != s; v = prevNode[v]) {
                Edge &e = g[prevNode[v]][prevEdge[v]];
                pathCost += e.cost;
                e.cap -= add;
                g[v][e.rev].cap += add;
            }
            flow += add;
            cost += add * pathCost;
        }
        return {flow, cost};
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<vector<int>> a(n, vector<int>(m));
    for (auto &row : a) {
        for (int &x : row) cin >> x;
    }

    vector<int> winner(n, -1), secondWinner(n, -1);
    vector<int64> maximum(n, 0), winCost(n);

    for (int i = 0; i < n; ++i) {
        vector<int> order(m), values = a[i];
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int x, int y) {
            return a[i][x] < a[i][y];
        });
        sort(values.begin(), values.end());

        for (int l = 0; l < m; ) {
            int r = l + 1;
            while (r < m && a[i][order[r]] == a[i][order[l]]) ++r;
            if (r - l == 1 && a[i][order[l]] > 0) {
                secondWinner[i] = winner[i];
                winner[i] = order[l];
                maximum[i] = a[i][order[l]];
            }
            l = r;
        }

        int64 x = maximum[i] + 1;
        while (binary_search(values.begin(), values.end(), (int)x)) ++x;
        winCost[i] = x;
    }

    const int64 PENALTY = 1000000000000LL;
    vector<int64> bestCost(n + 1, -1);

    auto solve = [&](int k, bool reconstruct, vector<int64> &answer) -> int64 {
        int source = n + m;
        int sink = source + 1;
        int me = source + 2;
        MinCostFlow flow(me + 1);
        vector<int> takeEdge(n), keepEdge(n, -1);

        for (int i = 0; i < n; ++i) {
            flow.addEdge(source, i, 1, 0);
            int blockTarget = secondWinner[i] == -1 ? sink : n + secondWinner[i];
            flow.addEdge(i, blockTarget, 1, maximum[i] + PENALTY);
            takeEdge[i] = flow.addEdge(i, me, 1, winCost[i]);
            if (winner[i] != -1) {
                keepEdge[i] = flow.addEdge(i, n + winner[i], 1, PENALTY);
            }
        }
        for (int j = 0; j < m; ++j) {
            flow.addEdge(n + j, sink, k, 0);
        }
        flow.addEdge(me, sink, k, 0);

        auto [sent, cost] = flow.run(source, sink, n);
        if (sent != n) return INF;
        cost -= PENALTY * (n - k);

        if (reconstruct) {
            answer.assign(n, 0);
            for (int i = 0; i < n; ++i) {
                if (flow.g[i][takeEdge[i]].cap == 0) {
                    answer[i] = winCost[i];
                } else if (keepEdge[i] == -1 || flow.g[i][keepEdge[i]].cap != 0) {
                    answer[i] = maximum[i];
                }
            }
        }
        return cost;
    };

    vector<int64> dummy;
    auto evaluate = [&](int k) {
        if (bestCost[k] == -1) bestCost[k] = solve(k, false, dummy);
        return bestCost[k];
    };

    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        int64 left = evaluate(mid);
        int64 right = evaluate(mid + 1);
        if (left == INF || left > right) lo = mid + 1;
        else hi = mid;
    }

    vector<int64> answer;
    solve(lo, true, answer);
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';
    return 0;
}

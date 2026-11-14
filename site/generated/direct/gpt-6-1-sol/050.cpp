#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
static constexpr int64 INF = (1LL << 62);

struct MinCostFlow {
    struct Edge {
        int to, next, cap;
        int64 cost;
    };

    int N;
    vector<int> head;
    vector<Edge> edges;
    vector<int64> potential, dist;
    vector<int> parent;

    explicit MinCostFlow(int n)
        : N(n), head(n, -1), potential(n, 0), dist(n), parent(n) {
        edges.reserve(12000);
    }

    int addEdge(int u, int v, int cap, int64 cost) {
        int id = (int)edges.size();
        edges.push_back({v, head[u], cap, cost});
        head[u] = id;
        edges.push_back({u, head[v], 0, -cost});
        head[v] = id + 1;
        return id;
    }

    int64 minCostFlow(int source, int sink, int requiredFlow) {
        int flow = 0;
        int64 totalCost = 0;

        using State = pair<int64, int>;
        priority_queue<State, vector<State>, greater<State>> pq;

        while (flow < requiredFlow) {
            fill(dist.begin(), dist.end(), INF);
            fill(parent.begin(), parent.end(), -1);
            while (!pq.empty()) pq.pop();

            dist[source] = 0;
            pq.push({0, source});

            while (!pq.empty()) {
                auto [du, u] = pq.top();
                pq.pop();

                if (du != dist[u]) continue;
                if (u == sink) break;

                for (int id = head[u]; id != -1; id = edges[id].next) {
                    const Edge &e = edges[id];
                    if (e.cap == 0) continue;

                    int64 nd = du + e.cost + potential[u] - potential[e.to];
                    if (nd < dist[e.to]) {
                        dist[e.to] = nd;
                        parent[e.to] = id;
                        pq.push({nd, e.to});
                    }
                }
            }

            if (dist[sink] == INF) return INF;

            // Truncated distance updates allow Dijkstra to stop at the sink.
            int64 limit = dist[sink];
            for (int v = 0; v < N; ++v) {
                potential[v] += min(dist[v], limit);
            }

            int add = requiredFlow - flow;
            for (int v = sink; v != source; ) {
                int id = parent[v];
                add = min(add, edges[id].cap);
                v = edges[id ^ 1].to;
            }

            for (int v = sink; v != source; ) {
                int id = parent[v];
                totalCost += (int64)add * edges[id].cost;
                edges[id].cap -= add;
                edges[id ^ 1].cap += add;
                v = edges[id ^ 1].to;
            }

            flow += add;
        }

        return totalCost;
    }
};

struct Casino {
    int first = -1;
    int second = -1;
    int top = 0;
    int winCost = 1;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<Casino> casinos(n);
    vector<int> originalWins(m, 0);
    vector<pair<int, int>> row(m);

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cin >> row[j].first;
            row[j].second = j;
        }
        sort(row.begin(), row.end());

        Casino &c = casinos[i];

        for (int l = 0; l < m; ) {
            int r = l + 1;
            while (r < m && row[r].first == row[l].first) ++r;

            if (row[l].first > 0 && r - l == 1) {
                c.second = c.first;
                c.first = row[l].second;
                c.top = row[l].first;
            }
            l = r;
        }

        int candidate = c.top + 1;
        for (const auto &[value, player] : row) {
            if (value < candidate) continue;
            if (value == candidate) ++candidate;
            else break;
        }
        c.winCost = candidate;

        if (c.first != -1) ++originalWins[c.first];
    }

    int upper = *max_element(originalWins.begin(), originalWins.end());

    if (upper == 0) {
        for (int i = 0; i < n; ++i) {
            cout << 0 << (i + 1 == n ? '\n' : ' ');
        }
        return 0;
    }

    vector<int64> cached(n + 1, -1);
    int64 bestCost = INF;
    vector<int> bestAnswer(n, 0);

    auto evaluate = [&](int k) -> int64 {
        if (cached[k] != -1) return cached[k];

        const int SOURCE = 0;
        const int SINK = 1;
        const int SELF = 2;
        const int OTHERS = 3;
        const int PLAYER_BASE = 4;
        const int NOBODY = PLAYER_BASE + m;
        const int CASINO_BASE = NOBODY + 1;
        const int vertices = CASINO_BASE + n;

        MinCostFlow flow(vertices);

        flow.addEdge(SOURCE, SELF, k, 0);
        flow.addEdge(SOURCE, OTHERS, n - k, 0);

        for (int j = 0; j < m; ++j) {
            flow.addEdge(OTHERS, PLAYER_BASE + j, k, 0);
        }
        flow.addEdge(OTHERS, NOBODY, n, 0);

        vector<int> selfEdge(n), redirectEdge(n, -1);

        for (int i = 0; i < n; ++i) {
            const Casino &c = casinos[i];
            int node = CASINO_BASE + i;

            flow.addEdge(node, SINK, 1, 0);

            selfEdge[i] = flow.addEdge(SELF, node, 1, c.winCost);

            int defaultWinner =
                (c.first == -1 ? NOBODY : PLAYER_BASE + c.first);
            flow.addEdge(defaultWinner, node, 1, 0);

            if (c.first != -1) {
                int redirectedWinner =
                    (c.second == -1 ? NOBODY : PLAYER_BASE + c.second);
                redirectEdge[i] =
                    flow.addEdge(redirectedWinner, node, 1, c.top);
            }
        }

        int64 result = flow.minCostFlow(SOURCE, SINK, n);
        cached[k] = result;

        if (result < bestCost) {
            bestCost = result;
            for (int i = 0; i < n; ++i) {
                if (flow.edges[selfEdge[i]].cap == 0) {
                    bestAnswer[i] = casinos[i].winCost;
                } else if (redirectEdge[i] != -1 &&
                           flow.edges[redirectEdge[i]].cap == 0) {
                    bestAnswer[i] = casinos[i].top;
                } else {
                    bestAnswer[i] = 0;
                }
            }
        }

        return result;
    };

    int low = 0, high = upper;
    while (low < high) {
        int mid = (low + high) / 2;
        int64 left = evaluate(mid);
        int64 right = evaluate(mid + 1);

        if (left != INF && left <= right) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }
    evaluate(low);

    for (int i = 0; i < n; ++i) {
        cout << bestAnswer[i] << (i + 1 == n ? '\n' : ' ');
    }

    return 0;
}

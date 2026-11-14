#include <bits/stdc++.h>
using namespace std;

using ll = long long;

const ll INF = (1LL << 62);
const ll BIG_REWARD = 1000000000000LL; // Greater than any possible real total cost.

struct MinCostMaxFlow {
    struct Edge {
        int to, rev, cap;
        ll cost;
    };

    int n;
    vector<vector<Edge>> g;

    explicit MinCostMaxFlow(int n) : n(n), g(n) {}

    int addEdge(int v, int to, int cap, ll cost) {
        int index = static_cast<int>(g[v].size());
        Edge a{to, static_cast<int>(g[to].size()), cap, cost};
        Edge b{v, index, 0, -cost};
        g[v].push_back(a);
        g[to].push_back(b);
        return index;
    }

    pair<int, ll> run(int s, int t, int maxFlow) {
        vector<ll> potential(n, INF);

        // Initial shortest-path potentials, since one forward edge has negative cost.
        vector<bool> inQueue(n, false);
        queue<int> q;
        potential[s] = 0;
        q.push(s);
        inQueue[s] = true;

        while (!q.empty()) {
            int v = q.front();
            q.pop();
            inQueue[v] = false;

            for (const Edge &e : g[v]) {
                if (e.cap == 0) continue;
                if (potential[e.to] > potential[v] + e.cost) {
                    potential[e.to] = potential[v] + e.cost;
                    if (!inQueue[e.to]) {
                        inQueue[e.to] = true;
                        q.push(e.to);
                    }
                }
            }
        }

        for (ll &x : potential) {
            if (x == INF) x = 0;
        }

        int flow = 0;
        ll cost = 0;
        vector<ll> dist(n);
        vector<int> prevVertex(n), prevEdge(n);

        while (flow < maxFlow) {
            fill(dist.begin(), dist.end(), INF);
            dist[s] = 0;

            priority_queue<pair<ll, int>, vector<pair<ll, int>>,
                           greater<pair<ll, int>>> pq;
            pq.push({0, s});

            while (!pq.empty()) {
                auto [d, v] = pq.top();
                pq.pop();
                if (d != dist[v]) continue;

                for (int i = 0; i < static_cast<int>(g[v].size()); ++i) {
                    const Edge &e = g[v][i];
                    if (e.cap == 0) continue;

                    ll reducedCost = e.cost + potential[v] - potential[e.to];
                    ll nd = d + reducedCost;

                    if (nd < dist[e.to]) {
                        dist[e.to] = nd;
                        prevVertex[e.to] = v;
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
            for (int v = t; v != s; v = prevVertex[v]) {
                add = min(add, g[prevVertex[v]][prevEdge[v]].cap);
            }

            for (int v = t; v != s; v = prevVertex[v]) {
                Edge &e = g[prevVertex[v]][prevEdge[v]];
                cost += static_cast<ll>(add) * e.cost;
                e.cap -= add;
                g[v][e.rev].cap += add;
            }

            flow += add;
        }

        return {flow, cost};
    }
};

struct CasinoInfo {
    int topPlayer = -1;
    int secondPlayer = -1;
    int demoteCost = 0;
    int myWinCost = 0;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<CasinoInfo> casinos(n);

    for (int i = 0; i < n; ++i) {
        map<int, int> count;
        map<int, int> owner;

        for (int j = 0; j < m; ++j) {
            int x;
            cin >> x;
            if (x > 0) {
                ++count[x];
                owner[x] = j;
            }
        }

        vector<pair<int, int>> uniqueCounts;
        for (const auto &[value, occurrences] : count) {
            if (occurrences == 1) {
                uniqueCounts.push_back({value, owner[value]});
            }
        }

        int largestRemaining = 0;
        if (!uniqueCounts.empty()) {
            largestRemaining = uniqueCounts.back().first;
            casinos[i].topPlayer = uniqueCounts.back().second;
            casinos[i].demoteCost = largestRemaining;

            if (uniqueCounts.size() >= 2) {
                casinos[i].secondPlayer =
                    uniqueCounts[uniqueCounts.size() - 2].second;
            }
        }

        int b = largestRemaining + 1;
        while (count.find(b) != count.end()) ++b;
        casinos[i].myWinCost = b;
    }

    struct ChoiceRef {
        int edgeIndex;
        int dice;
    };

    auto solveFixedK = [&](int k, vector<int> *placement) -> ll {
        int source = 0;
        int casinoStart = 1;
        int playerStart = casinoStart + n;
        int myNode = playerStart + m;
        int sink = myNode + 1;
        int nodeCount = sink + 1;

        MinCostMaxFlow flow(nodeCount);
        vector<vector<ChoiceRef>> choices(n);

        auto addChoice = [&](int i, int to, ll cost, int dice) {
            int node = casinoStart + i;
            int edgeIndex = flow.addEdge(node, to, 1, cost);
            choices[i].push_back({edgeIndex, dice});
        };

        for (int i = 0; i < n; ++i) {
            int node = casinoStart + i;
            flow.addEdge(source, node, 1, 0);

            const CasinoInfo &info = casinos[i];

            if (info.topPlayer == -1) {
                addChoice(i, sink, 0, 0);
            } else {
                addChoice(i, playerStart + info.topPlayer, 0, 0);

                if (info.secondPlayer != -1) {
                    addChoice(i, playerStart + info.secondPlayer,
                              info.demoteCost, info.demoteCost);
                } else {
                    addChoice(i, sink, info.demoteCost, info.demoteCost);
                }
            }

            addChoice(i, myNode, info.myWinCost, info.myWinCost);
        }

        for (int j = 0; j < m; ++j) {
            flow.addEdge(playerStart + j, sink, k, 0);
        }

        int myEdgeIndex =
            flow.addEdge(myNode, sink, k, -BIG_REWARD);

        auto [sent, totalCost] = flow.run(source, sink, n);

        int myWins = k - flow.g[myNode][myEdgeIndex].cap;
        if (sent != n || myWins != k) return INF;

        if (placement != nullptr) {
            placement->assign(n, 0);

            for (int i = 0; i < n; ++i) {
                int node = casinoStart + i;
                for (const ChoiceRef &choice : choices[i]) {
                    if (flow.g[node][choice.edgeIndex].cap == 0) {
                        (*placement)[i] = choice.dice;
                        break;
                    }
                }
            }
        }

        return totalCost + static_cast<ll>(k) * BIG_REWARD;
    };

    vector<ll> memo(n + 1, 0);
    vector<bool> computed(n + 1, false);

    auto value = [&](int k) -> ll {
        if (!computed[k]) {
            memo[k] = solveFixedK(k, nullptr);
            computed[k] = true;
        }
        return memo[k];
    };

    // F(k) is convex, and infeasible values form a prefix.
    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        ll current = value(mid);

        if (current == INF) {
            lo = mid + 1;
            continue;
        }

        ll next = value(mid + 1);
        if (current <= next) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    vector<int> answer;
    solveFixedK(lo, &answer);

    for (int i = 0; i < n; ++i) {
        if (i > 0) cout << ' ';
        cout << answer[i];
    }
    cout << '\n';

    return 0;
}

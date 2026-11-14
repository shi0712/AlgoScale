#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll INF = (1LL << 62);

struct MinCostFlow {
    struct Edge { int to, rev, cap; ll cost; };
    int n;
    vector<vector<Edge>> g;
    explicit MinCostFlow(int n) : n(n), g(n) {}
    int addEdge(int u, int v, int cap, ll cost) {
        int id = (int)g[u].size();
        g[u].push_back({v, (int)g[v].size(), cap, cost});
        g[v].push_back({u, id, 0, -cost});
        return id;
    }
    ll run(int source, int sink, int required) {
        vector<ll> potential(n, 0), dist(n);
        vector<int> prevNode(n), prevEdge(n);
        ll total = 0;
        for (int flow = 0; flow < required; ++flow) {
            fill(dist.begin(), dist.end(), INF);
            priority_queue<pair<ll,int>, vector<pair<ll,int>>, greater<pair<ll,int>>> pq;
            dist[source] = 0;
            pq.push({0, source});
            while (!pq.empty()) {
                auto [du, u] = pq.top();
                pq.pop();
                if (du != dist[u]) continue;
                if (u == sink) break;
                for (int e = 0; e < (int)g[u].size(); ++e) {
                    const Edge &edge = g[u][e];
                    if (!edge.cap) continue;
                    ll nd = du + edge.cost + potential[u] - potential[edge.to];
                    if (nd < dist[edge.to]) {
                        dist[edge.to] = nd;
                        prevNode[edge.to] = u;
                        prevEdge[edge.to] = e;
                        pq.push({nd, edge.to});
                    }
                }
            }
            if (dist[sink] == INF) return INF;
            // Capping distances also updates nodes not settled before the sink.
            for (int v = 0; v < n; ++v)
                potential[v] += min(dist[v], dist[sink]);
            total += potential[sink] - potential[source];
            for (int v = sink; v != source; v = prevNode[v]) {
                Edge &edge = g[prevNode[v]][prevEdge[v]];
                --edge.cap;
                ++g[v][edge.rev].cap;
            }
        }
        return total;
    }
};

struct Casino {
    int first = -1, second = -1;
    int cancelCost = 0, winCost = 1;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    cin >> n >> m;
    vector<Casino> casinos(n);
    vector<bool> active(m, false);
    for (auto &casino : casinos) {
        vector<pair<int,int>> a(m);
        for (int j = 0; j < m; ++j) {
            cin >> a[j].first;
            a[j].second = j;
        }
        sort(a.begin(), a.end());
        vector<pair<int,int>> singles;
        for (int l = 0; l < m;) {
            int r = l + 1;
            while (r < m && a[r].first == a[l].first) ++r;
            if (r == l + 1 && a[l].first > 0) singles.push_back(a[l]);
            l = r;
        }
        if (!singles.empty()) {
            casino.first = singles.back().second;
            casino.cancelCost = singles.back().first;
            active[casino.first] = true;
            if (singles.size() >= 2) {
                casino.second = singles[singles.size() - 2].second;
                active[casino.second] = true;
            }
        }
        casino.winCost = casino.cancelCost + 1;
        for (auto [value, player] : a) {
            if (value == casino.winCost) ++casino.winCost;
        }
    }

    vector<int> playerNode(m, -1);
    int nodeCount = 4;
    for (int j = 0; j < m; ++j)
        if (active[j]) playerNode[j] = nodeCount++;
    const int casinoBase = nodeCount;
    const int sink = casinoBase + n;
    const int source = 0, ordinary = 1, self = 2, nobody = 3;
    vector<ll> cached(n + 1, -1);
    vector<vector<int>> placements(n + 1);

    auto evaluate = [&](int k) -> ll {
        if (cached[k] != -1) return cached[k];
        MinCostFlow flow(sink + 1);
        // These two capacities force exactly k wins for us in a full flow.
        flow.addEdge(source, self, k, 0);
        flow.addEdge(source, ordinary, n - k, 0);
        flow.addEdge(ordinary, nobody, n, 0);
        for (int j = 0; j < m; ++j)
            if (active[j]) flow.addEdge(ordinary, playerNode[j], k, 0);
        vector<int> winEdge(n), cancelEdge(n, -1), cancelNode(n, -1);
        for (int i = 0; i < n; ++i) {
            const Casino &c = casinos[i];
            int v = casinoBase + i;
            flow.addEdge(v, sink, 1, 0);
            winEdge[i] = flow.addEdge(self, v, 1, c.winCost);
            int baseline = c.first == -1 ? nobody : playerNode[c.first];
            flow.addEdge(baseline, v, 1, 0);
            if (c.first != -1) {
                int next = c.second == -1 ? nobody : playerNode[c.second];
                cancelNode[i] = next;
                cancelEdge[i] = flow.addEdge(next, v, 1, c.cancelCost);
            }
        }
        ll result = flow.run(source, sink, n);
        cached[k] = result;
        if (result != INF) {
            vector<int> &answer = placements[k];
            answer.assign(n, 0);
            for (int i = 0; i < n; ++i) {
                if (flow.g[self][winEdge[i]].cap == 0)
                    answer[i] = casinos[i].winCost;
                else if (cancelEdge[i] != -1 &&
                         flow.g[cancelNode[i]][cancelEdge[i]].cap == 0)
                    answer[i] = casinos[i].cancelCost;
            }
        }
        return result;
    };

    // For fixed k, this is a min-cost flow with capacities/supplies affine
    // in k. Its LP optimum is convex in k, and integral for integer k.
    int lo = 0, hi = n;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        ll left = evaluate(mid);
        if (left == INF) {
            lo = mid + 1;
        } else if (left <= evaluate(mid + 1)) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }
    evaluate(lo);
    for (int i = 0; i < n; ++i)
        cout << placements[lo][i] << (i + 1 == n ? '\n' : ' ');
}

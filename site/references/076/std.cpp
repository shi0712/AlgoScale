#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

struct BucketQueue {
    array<int, 256> head{};
    array<unsigned long long, 4> mask{};
    unsigned groups = 0;
    vector<pair<int,int>> nodes;

    BucketQueue() { head.fill(-1); }

    void clear() {
        while (groups) {
            int g = __builtin_ctz(groups);
            while (mask[g]) {
                int b = __builtin_ctzll(mask[g]);
                head[g * 64 + b] = -1;
                mask[g] &= mask[g] - 1;
            }
            groups &= groups - 1;
        }
        nodes.clear();
    }

    bool empty() const { return groups == 0; }

    void push(int d, int u) {
        nodes.emplace_back(u, head[d]);
        head[d] = (int)nodes.size() - 1;
        mask[d >> 6] |= 1ULL << (d & 63);
        groups |= 1U << (d >> 6);
    }

    pair<int,int> pop() {
        int g = __builtin_ctz(groups);
        int b = __builtin_ctzll(mask[g]);
        int d = g * 64 + b;
        int p = head[d];
        int u = nodes[p].first;
        head[d] = nodes[p].second;
        if (head[d] == -1) {
            mask[g] &= ~(1ULL << b);
            if (!mask[g]) groups &= ~(1U << g);
        }
        return {d, u};
    }
};

struct MinCostResidual {
    struct Edge {
        int to, next;
        int64 cap, cost;
    };

    int n;
    vector<int> head;
    vector<Edge> edges;

    explicit MinCostResidual(int n_) : n(n_), head(n_, -1) {}

    int addEdge(int u, int v, int64 cap, int64 cost, int64 revCap = 0) {
        int id = (int)edges.size();
        edges.push_back({v, head[u], cap, cost});
        head[u] = id;
        edges.push_back({u, head[v], revCap, -cost});
        head[v] = id + 1;
        return id;
    }

    // Potentials are feasible on entry. Only reduced distances smaller than
    // stopCost - potential[t] are relevant. In this problem the bound is <= 251.
    int64 augmentUntil(int s, int t, int64 flowLimit, int64 stopCost,
                       vector<int64>& potential) {
        vector<int> dist(n), pre(n);
        BucketQueue q;
        int64 flow = 0;

        while (flow < flowLimit && potential[t] < stopCost) {
            int bound = (int)(stopCost - potential[t]);
            if (!(1 <= bound && bound <= 256)) {
                throw runtime_error("internal bucket bound exceeded");
            }

            fill(dist.begin(), dist.end(), bound);
            fill(pre.begin(), pre.end(), -1);
            q.clear();
            dist[s] = 0;
            q.push(0, s);

            while (!q.empty()) {
                auto [du, u] = q.pop();
                if (du != dist[u]) continue;
                if (u == t) break;

                for (int id = head[u]; id != -1; id = edges[id].next) {
                    const Edge& e = edges[id];
                    if (e.cap == 0) continue;
                    int64 rcost = e.cost + potential[u] - potential[e.to];
                    if (rcost < 0) throw runtime_error("negative reduced cost");
                    int64 nd = du + rcost;
                    if (nd < dist[e.to]) {
                        dist[e.to] = (int)nd;
                        pre[e.to] = id;
                        q.push((int)nd, e.to);
                    }
                }
            }

            int step = dist[t];
            for (int v = 0; v < n; ++v) potential[v] += min(dist[v], step);
            if (step == bound) break;

            int64 add = flowLimit - flow;
            for (int v = t; v != s; v = edges[pre[v] ^ 1].to)
                add = min(add, edges[pre[v]].cap);
            for (int v = t; v != s; v = edges[pre[v] ^ 1].to) {
                int id = pre[v];
                edges[id].cap -= add;
                edges[id ^ 1].cap += add;
            }
            flow += add;
        }
        return flow;
    }
};

struct OriginalEdge {
    int u, v;
    int64 w;
    int dualA, dualB; // -1 for vertical edges on the left/right boundary
    int paid1 = -1, paid2 = -1;
};

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    bool objectiveOnly = (argc >= 2 && string(argv[1]) == "--objective-only");

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n, m, k;
        cin >> n >> m >> k;
        const int N = n * m;
        const int dualS = (n - 1) * (m - 1);
        const int dualT = dualS + 1;

        vector<OriginalEdge> original;
        original.reserve(n * (m - 1) + (n - 1) * m);
        vector<vector<pair<int,int64>>> graph(N);

        auto addOriginal = [&](int u, int v, int64 w, int a, int b) {
            original.push_back({u, v, w, a, b});
            graph[u].push_back({v, w});
            graph[v].push_back({u, w});
        };

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j + 1 < m; ++j) {
                int64 w; cin >> w;
                int a = (i == 0 ? dualS : (i - 1) * (m - 1) + j);
                int b = (i == n - 1 ? dualT : i * (m - 1) + j);
                addOriginal(i * m + j, i * m + j + 1, w, a, b);
            }
        }
        for (int i = 0; i + 1 < n; ++i) {
            for (int j = 0; j < m; ++j) {
                int64 w; cin >> w;
                int a = -1, b = -1;
                if (0 < j && j + 1 < m) {
                    a = i * (m - 1) + j;
                    b = i * (m - 1) + j - 1;
                }
                addOriginal(i * m + j, (i + 1) * m + j, w, a, b);
            }
        }

        const int64 INF = (1LL << 62);
        vector<int64> h(N, INF);
        priority_queue<pair<int64,int>, vector<pair<int64,int>>, greater<pair<int64,int>>> pq;
        for (int i = 0; i < n; ++i) {
            h[i * m] = 0;
            pq.push({0, i * m});
        }
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d != h[u]) continue;
            for (auto [v, w] : graph[u]) {
                if (d + w < h[v]) {
                    h[v] = d + w;
                    pq.push({h[v], v});
                }
            }
        }

        int64 D = INF;
        for (int i = 0; i < n; ++i) D = min(D, h[i * m + (m - 1)]);
        for (auto& x : h) x = min(x, D);

        vector<int64> result(original.size());

        if (n <= k) {
            // Primal construction. First and last columns are contracted.
            int s = 0, t = 1, V = 2;
            vector<int> id(N);
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < m; ++j) {
                    if (j == 0) id[i * m + j] = s;
                    else if (j == m - 1) id[i * m + j] = t;
                    else id[i * m + j] = V++;
                }
            }

            MinCostResidual mf(V);
            vector<int64> potential(V, 0);
            for (int v = 0; v < N; ++v) potential[id[v]] = h[v];
            for (const auto& e : original) {
                int u = id[e.u], v = id[e.v];
                if (u == v) continue;
                mf.addEdge(u, v, 1, e.w);
                mf.addEdge(v, u, 1, e.w);
            }

            mf.augmentUntil(s, t, n + 1, D + k, potential);
            if (potential[t] != D + k) throw runtime_error("primal construction failed");

            for (size_t i = 0; i < original.size(); ++i) {
                const auto& e = original[i];
                result[i] = max(e.w, llabs(potential[id[e.u]] - potential[id[e.v]]));
            }
        } else {
            // Planar-dual construction. h induces a free dual flow of value D.
            MinCostResidual mf(dualT + 1);
            vector<int64> potential(dualT + 1, 0);

            for (auto& e : original) {
                if (e.dualA == -1) continue;
                int64 f = h[e.v] - h[e.u];
                if (llabs(f) > e.w) throw runtime_error("invalid induced dual flow");

                mf.addEdge(e.dualA, e.dualB, e.w - f, 0, e.w + f);
                e.paid1 = mf.addEdge(e.dualA, e.dualB, k, 1);
                e.paid2 = mf.addEdge(e.dualB, e.dualA, k, 1);
            }

            int64 sent = mf.augmentUntil(dualS, dualT, k, n + 1, potential);
            if (sent != k) throw runtime_error("dual construction failed");

            for (size_t i = 0; i < original.size(); ++i) {
                const auto& e = original[i];
                result[i] = e.w;
                if (e.dualA != -1) {
                    result[i] += mf.edges[e.paid1 ^ 1].cap;
                    result[i] += mf.edges[e.paid2 ^ 1].cap;
                }
            }
        }

        int64 answer = 0;
        for (size_t i = 0; i < original.size(); ++i) {
            if (result[i] < original[i].w || result[i] > 2000000000LL)
                throw runtime_error("invalid output edge weight");
            answer += result[i] - original[i].w;
        }

        cout << answer << '\n';
        if (objectiveOnly) continue;

        size_t p = 0;
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j + 1 < m; ++j) {
                if (j) cout << ' ';
                cout << result[p++];
            }
            cout << '\n';
        }
        for (int i = 0; i + 1 < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (j) cout << ' ';
                cout << result[p++];
            }
            cout << '\n';
        }
    }
    return 0;
}

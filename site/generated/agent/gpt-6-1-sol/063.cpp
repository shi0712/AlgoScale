#include <bits/stdc++.h>
using namespace std;

/*
The shortest left-to-right path is the minimum top-to-bottom cut in the
planar dual, whose capacities are the original edge lengths. Rotating
truncated primal distance differences gives a maximum dual flow for free.

In its residual network, existing capacity costs zero, and buying one
additional unit of capacity costs one. Send k more units with minimum cost;
the purchased capacities are precisely the required edge increments.

All shortest augmenting path costs are at most n: a straight dual path
has n expansion arcs of cost one. Integer buckets therefore make each
augmentation O(nm). Total time: O(nm log(nm) + k nm), space: O(nm).
*/

class FastInput {
    static constexpr int SZ = 1 << 16;
    char buf[SZ];
    int at = 0, len = 0;
public:
    inline char get() {
        if (at == len) { len = fread(buf, 1, SZ, stdin); at = 0; if (!len) return 0; }
        return buf[at++];
    }
    int nextInt() {
        char c;
        do { c = get(); } while (c <= ' ' && c);
        int x = 0;
        while (c >= '0' && c <= '9') { x = x * 10 + c - '0'; c = get(); }
        return x;
    }
};

struct MinCostFlow {
    struct Edge { int to, next, cap, cost; };
    int N;
    vector<int> head, potential, dist, prev;
    vector<Edge> edges;
    MinCostFlow(int n, int reserveEdges): N(n), head(n, -1), potential(n), dist(n), prev(n) {
        edges.reserve(reserveEdges);
    }
    int addPair(int u, int v, int cap, int reverseCap, int cost) {
        int id = (int)edges.size();
        edges.push_back({v, head[u], cap, cost}); head[u] = id;
        edges.push_back({u, head[v], reverseCap, -cost}); head[v] = id + 1;
        return id;
    }
    void send(int source, int sink, int amount, int maxCost) {
        const int INF = 1000000000;
        vector<int> bucket(maxCost + 1), next(N), back(N);
        while (amount) {
            fill(dist.begin(), dist.end(), INF);
            fill(bucket.begin(), bucket.end(), -1);
            dist[source] = 0;
            bucket[0] = source;
            next[source] = back[source] = -1;
            bool found = false;
            // A straight dual path uses n expansion arcs, each of cost 1.
            // Thus the shortest reduced distance is at most maxCost = n.
            for (int value = 0; value <= maxCost && !found; ++value) {
                while (bucket[value] != -1) {
                    int u = bucket[value];
                    bucket[value] = next[u];
                    if (next[u] != -1) back[next[u]] = -1;
                    if (u == sink) { found = true; break; }
                    for (int id = head[u]; id != -1; id = edges[id].next) {
                        const auto &e = edges[id];
                        if (!e.cap) continue;
                        int nd = value + e.cost + potential[u] - potential[e.to];
                        int v = e.to;
                        if (nd > maxCost || nd >= dist[v]) continue;
                        if (dist[v] != INF) {
                            // A vertex with a smaller key is still in its old bucket.
                            if (back[v] == -1) bucket[dist[v]] = next[v];
                            else next[back[v]] = next[v];
                            if (next[v] != -1) back[next[v]] = back[v];
                        }
                        dist[v] = nd;
                        prev[v] = id;
                        back[v] = -1;
                        next[v] = bucket[nd];
                        if (next[v] != -1) back[next[v]] = v;
                        bucket[nd] = v;
                    }
                }
            }
            int limit = dist[sink];
            for (int v = 0; v < N; ++v) potential[v] += min(dist[v], limit);
            int pushed = amount;
            for (int v = sink; v != source; v = edges[prev[v] ^ 1].to)
                pushed = min(pushed, edges[prev[v]].cap);
            amount -= pushed;
            for (int v = sink; v != source; v = edges[prev[v] ^ 1].to) {
                edges[prev[v]].cap -= pushed;
                edges[prev[v] ^ 1].cap += pushed;
            }
        }
    }
};

int main() {
    FastInput in;
    int T = in.nextInt();
    string out;
    out.reserve(1 << 23);
    auto print = [&](long long x, char ending) {
        char buf[24];
        auto result = to_chars(buf, buf + 24, x);
        out.append(buf, result.ptr);
        out.push_back(ending);
    };
    while (T--) {
        int n = in.nextInt(), m = in.nextInt(), k = in.nextInt();
        vector<int> r(n * (m - 1)), c((n - 1) * m);
        for (int &w : r) w = in.nextInt();
        for (int &w : c) w = in.nextInt();
        int vertices = n * m;
        const long long INF = (1LL << 60);
        vector<long long> d(vertices, INF);
        priority_queue<pair<long long,int>, vector<pair<long long,int>>, greater<pair<long long,int>>> pq;
        for (int i = 0; i < n; ++i) {
            d[i * m] = 0;
            pq.emplace(0, i * m);
        }
        while (!pq.empty()) {
            auto [du, u] = pq.top(); pq.pop();
            if (du != d[u]) continue;
            int i = u / m, j = u % m;
            auto relax = [&](int v, int w) {
                long long nd = du + w;
                if (nd < d[v]) { d[v] = nd; pq.emplace(nd, v); }
            };
            if (j) relax(u - 1, r[i * (m - 1) + j - 1]);
            if (j + 1 < m) relax(u + 1, r[i * (m - 1) + j]);
            if (i) relax(u - m, c[(i - 1) * m + j]);
            if (i + 1 < n) relax(u + m, c[i * m + j]);
        }
        long long original = INF;
        for (int i = 0; i < n; ++i) original = min(original, d[i * m + m - 1]);
        for (auto &x : d) x = min(x, original);

        int cells = (n - 1) * (m - 1), top = cells, bottom = cells + 1;
        int dualEdges = n * (m - 1) + (n - 1) * (m - 2);
        MinCostFlow flow(cells + 2, 6 * dualEdges);
        struct Change { bool horizontal; int index, positive, negative; };
        vector<Change> changes;
        changes.reserve(dualEdges);
        auto addDual = [&](int u, int v, int w, long long f, bool horizontal, int index) {
            // f is an initial maximum flow: a rotated difference of primal distances.
            flow.addPair(u, v, int(w - f), int(w + f), 0);
            int positive = flow.addPair(u, v, k, 0, 1);
            int negative = flow.addPair(v, u, k, 0, 1);
            changes.push_back({horizontal, index, positive, negative});
        };
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j + 1 < m; ++j) {
                int above = i == 0 ? top : (i - 1) * (m - 1) + j;
                int below = i + 1 == n ? bottom : i * (m - 1) + j;
                int index = i * (m - 1) + j;
                addDual(above, below, r[index], d[i * m + j + 1] - d[i * m + j], true, index);
            }
        }
        // Vertical edges on the first and last columns never need to be changed.
        for (int i = 0; i + 1 < n; ++i) {
            for (int j = 1; j + 1 < m; ++j) {
                int left = i * (m - 1) + j - 1;
                int right = i * (m - 1) + j;
                int index = i * m + j;
                addDual(left, right, c[index], d[i * m + j] - d[(i + 1) * m + j], false, index);
            }
        }
        flow.send(top, bottom, k, n);
        long long operations = 0;
        for (const auto &change : changes) {
            int increase = 2 * k - flow.edges[change.positive].cap - flow.edges[change.negative].cap;
            operations += increase;
            if (change.horizontal) r[change.index] += increase;
            else c[change.index] += increase;
        }
        print(operations, '\n');
        for (int i = 0; i < n; ++i)
            for (int j = 0; j + 1 < m; ++j)
                print(r[i * (m - 1) + j], j + 2 == m ? '\n' : ' ');
        for (int i = 0; i + 1 < n; ++i)
            for (int j = 0; j < m; ++j)
                print(c[i * m + j], j + 1 == m ? '\n' : ' ');
    }
    fwrite(out.data(), 1, out.size(), stdout);
}

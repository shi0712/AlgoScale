#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

class FastInput {
    static constexpr int SZ = 1 << 16;
    char buffer[SZ];
    int pos = 0, len = 0;

    char getChar() {
        if (pos == len) {
            len = (int)fread(buffer, 1, SZ, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

public:
    int readInt() {
        char ch;
        do {
            ch = getChar();
        } while (ch <= ' ' && ch != 0);

        int value = 0;
        while (ch >= '0' && ch <= '9') {
            value = value * 10 + ch - '0';
            ch = getChar();
        }
        return value;
    }
};

class Solver {
    struct Edge {
        int to, rev, cap;
        int64 cost;
    };

    int vertices, source, sink;
    vector<vector<Edge>> graph;
    vector<int> level, current;

    bool buildLevels() {
        fill(level.begin(), level.end(), -1);

        vector<int> que(vertices);
        int head = 0, tail = 0;
        que[tail++] = source;
        level[source] = 0;

        while (head < tail) {
            int u = que[head++];
            for (const Edge& e : graph[u]) {
                if (e.cap == 0 || level[e.to] != -1) continue;
                if (e.cost + potential[u] - potential[e.to] != 0) continue;

                level[e.to] = level[u] + 1;
                que[tail++] = e.to;
            }
        }
        return level[sink] != -1;
    }

    int sendFlow(int u, int amount) {
        if (u == sink) return amount;

        for (int& i = current[u]; i < (int)graph[u].size(); ++i) {
            Edge& e = graph[u][i];
            if (e.cap == 0 || level[e.to] != level[u] + 1) continue;
            if (e.cost + potential[u] - potential[e.to] != 0) continue;

            int sent = sendFlow(e.to, min(amount, e.cap));
            if (sent != 0) {
                e.cap -= sent;
                graph[e.to][e.rev].cap += sent;
                return sent;
            }
        }
        return 0;
    }

    // Send a maximum flow using only zero reduced-cost residual arcs.
    void sendAllZeroCostFlow() {
        while (buildLevels()) {
            fill(current.begin(), current.end(), 0);
            while (sendFlow(source, INT_MAX) != 0) {
            }
        }
    }

    // Increase potentials, but by at most limit.
    // Only reduced distances strictly smaller than limit are needed.
    int raisePotentials(int limit) {
        vector<int> dist(vertices, limit);
        vector<vector<int>> buckets(limit);

        dist[source] = 0;
        buckets[0].push_back(source);

        int delta = limit;
        bool reachedSink = false;

        for (int d = 0; d < limit && !reachedSink; ++d) {
            while (!buckets[d].empty()) {
                int u = buckets[d].back();
                buckets[d].pop_back();

                if (dist[u] != d) continue;
                if (u == sink) {
                    delta = d;
                    reachedSink = true;
                    break;
                }

                for (const Edge& e : graph[u]) {
                    if (e.cap == 0) continue;

                    int64 reduced = e.cost + potential[u] - potential[e.to];
                    if (reduced >= limit - d) continue;

                    int nd = d + (int)reduced;
                    if (nd < dist[e.to]) {
                        dist[e.to] = nd;
                        buckets[nd].push_back(e.to);
                    }
                }
            }
        }

        for (int v = 0; v < vertices; ++v) {
            potential[v] += min(dist[v], delta);
        }
        return delta;
    }

public:
    vector<int64> potential;

    Solver(int v, int s, int t)
        : vertices(v), source(s), sink(t), graph(v),
          level(v), current(v), potential(v) {}

    void addDirected(int u, int v, int64 cost) {
        int a = (int)graph[u].size();
        int b = (int)graph[v].size();
        graph[u].push_back({v, b, 1, cost});
        graph[v].push_back({u, a, 0, -cost});
    }

    void addUndirected(int u, int v, int64 cost) {
        if (u == v) return;
        addDirected(u, v, cost);
        addDirected(v, u, cost);
    }

    void solve(int k) {
        const int64 INF = (1LL << 62);
        vector<int64> dist(vertices, INF);

        priority_queue<pair<int64, int>,
                       vector<pair<int64, int>>,
                       greater<pair<int64, int>>> pq;

        dist[source] = 0;
        pq.push({0, source});

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (d != dist[u]) continue;
            if (u == sink) break;

            for (const Edge& e : graph[u]) {
                if (e.cap == 0) continue;
                int64 nd = d + e.cost;
                if (nd < dist[e.to]) {
                    dist[e.to] = nd;
                    pq.push({nd, e.to});
                }
            }
        }

        int64 originalDistance = dist[sink];

        // Truncation gives feasible potentials, with all values in [0, D].
        for (int v = 0; v < vertices; ++v) {
            potential[v] = min(dist[v], originalDistance);
        }

        int remaining = k;
        while (remaining > 0) {
            sendAllZeroCostFlow();
            int delta = raisePotentials(remaining);
            remaining -= delta;
        }
    }
};

static void appendNumber(string& out, int64 value, char ending) {
    char buffer[32];
    auto result = to_chars(buffer, buffer + sizeof(buffer), value);
    out.append(buffer, result.ptr);
    out.push_back(ending);
}

int main() {
    FastInput input;
    int T = input.readInt();

    string output;
    output.reserve(1 << 23);

    while (T--) {
        int n = input.readInt();
        int m = input.readInt();
        int k = input.readInt();

        vector<int> horizontal(n * (m - 1));
        vector<int> vertical((n - 1) * m);

        for (int& w : horizontal) w = input.readInt();
        for (int& w : vertical) w = input.readInt();

        // Merge the entire first column into source, and the last into sink.
        int source = 0;
        int sink = n * (m - 2) + 1;
        int vertexCount = sink + 1;

        vector<int> id(n * m);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m; ++j) {
                if (j == 0) id[i * m + j] = source;
                else if (j == m - 1) id[i * m + j] = sink;
                else id[i * m + j] = 1 + i * (m - 2) + j - 1;
            }
        }

        Solver solver(vertexCount, source, sink);

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m - 1; ++j) {
                solver.addUndirected(
                    id[i * m + j],
                    id[i * m + j + 1],
                    horizontal[i * (m - 1) + j]
                );
            }
        }

        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < m; ++j) {
                solver.addUndirected(
                    id[i * m + j],
                    id[(i + 1) * m + j],
                    vertical[i * m + j]
                );
            }
        }

        solver.solve(k);

        int64 answer = 0;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m - 1; ++j) {
                int index = i * (m - 1) + j;
                int u = id[i * m + j];
                int v = id[i * m + j + 1];

                int64 required = llabs(solver.potential[u] - solver.potential[v]);
                int64 newWeight = max<int64>(horizontal[index], required);

                answer += newWeight - horizontal[index];
                horizontal[index] = (int)newWeight;
            }
        }

        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < m; ++j) {
                int index = i * m + j;
                int u = id[i * m + j];
                int v = id[(i + 1) * m + j];

                int64 required = llabs(solver.potential[u] - solver.potential[v]);
                int64 newWeight = max<int64>(vertical[index], required);

                answer += newWeight - vertical[index];
                vertical[index] = (int)newWeight;
            }
        }

        appendNumber(output, answer, '\n');

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < m - 1; ++j) {
                appendNumber(
                    output,
                    horizontal[i * (m - 1) + j],
                    j == m - 2 ? '\n' : ' '
                );
            }
        }

        for (int i = 0; i < n - 1; ++i) {
            for (int j = 0; j < m; ++j) {
                appendNumber(
                    output,
                    vertical[i * m + j],
                    j == m - 1 ? '\n' : ' '
                );
            }
        }
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}

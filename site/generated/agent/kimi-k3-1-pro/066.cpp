#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int from, to, weight;
};

struct CycleLine {
    int length;
    long long weight, square;

    long double mean() const {
        return (long double)weight / length;
    }

    long double intercept() const {
        return (long double)square / length;
    }

    long double value(long double mu) const {
        return intercept() - 2.0L * mu * mean();
    }

    long double variance() const {
        long double m = mean();
        return intercept() - m * m;
    }

    bool operator==(const CycleLine &other) const {
        return length == other.length && weight == other.weight && square == other.square;
    }
};

class Solver {
public:
    int n, m;
    vector<Edge> edges;
    vector<vector<int>> outgoing;
    vector<char> active;
    vector<int> vertices;
    long double answer = numeric_limits<long double>::infinity();

    CycleLine minimumMeanCycle(long double mu) {
        const int sz = n;
        const long double inf = numeric_limits<long double>::infinity();
        vector<vector<long double>> dp(sz + 1, vector<long double>(sz, inf));
        vector<vector<int>> parent(sz + 1, vector<int>(sz, -1));
        vector<vector<int>> parentEdge(sz + 1, vector<int>(sz, -1));

        for (int v = 0; v < sz; ++v) dp[0][v] = 0.0L;
        for (int len = 1; len <= sz; ++len) {
            for (int i = 0; i < m; ++i) {
                const Edge &e = edges[i];
                if (dp[len - 1][e.from] == inf) continue;
                long double w = e.weight;
                long double candidate = dp[len - 1][e.from] + w * w - 2.0L * mu * w;
                if (candidate < dp[len][e.to]) {
                    dp[len][e.to] = candidate;
                    parent[len][e.to] = e.from;
                    parentEdge[len][e.to] = i;
                }
            }
        }

        long double bestRatio = inf;
        int bestVertex = -1;
        for (int v = 0; v < sz; ++v) {
            if (dp[sz][v] == inf) continue;
            long double worst = -inf;
            for (int len = 0; len < sz; ++len) {
                if (dp[len][v] == inf) continue;
                long double ratio = (dp[sz][v] - dp[len][v]) / (sz - len);
                if (ratio > worst) {
                    worst = ratio;
                }
            }
            if (worst < bestRatio) {
                bestRatio = worst;
                bestVertex = v;
            }
        }

        vector<int> walkVertices(sz + 1);
        vector<int> walkEdges(sz);
        int current = bestVertex;
        for (int len = sz; len >= 0; --len) {
            walkVertices[len] = current;
            if (len > 0) {
                walkEdges[len - 1] = parentEdge[len][current];
                current = parent[len][current];
            }
        }

        int left = 0, right = 1;
        for (int i = 0; i <= sz; ++i) {
            bool found = false;
            for (int j = i + 1; j <= sz; ++j) {
                if (walkVertices[i] == walkVertices[j]) {
                    left = i;
                    right = j;
                    found = true;
                    break;
                }
            }
            if (found) break;
        }

        CycleLine result{right - left, 0, 0};
        for (int i = left; i < right; ++i) {
            int w = edges[walkEdges[i]].weight;
            result.weight += w;
            result.square += (long long)w * w;
        }
        answer = min(answer, max(0.0L, result.variance()));
        return result;
    }

    void findEnvelope(long double left, const CycleLine &leftLine,
                      long double right, const CycleLine &rightLine) {
        if (leftLine == rightLine) return;

        long double denominator = 2.0L * (leftLine.mean() - rightLine.mean());
        if (denominator == 0.0L) return;
        long double middle = (leftLine.intercept() - rightLine.intercept()) / denominator;
        if (!(middle > left && middle < right)) return;

        CycleLine middleLine = minimumMeanCycle(middle);
        long double difference = middleLine.value(middle) - leftLine.value(middle);
        if (fabsl(difference) <= 1e-12L) return;

        findEnvelope(left, leftLine, middle, middleLine);
        findEnvelope(middle, middleLine, right, rightLine);
    }

    void solveCyclic() {
        CycleLine left = minimumMeanCycle(0.0L);
        if (answer == 0.0L) return;
        CycleLine right = minimumMeanCycle(20.0L);
        findEnvelope(0.0L, left, 20.0L, right);
    }

    void solveShortWalks() {
        const int maxWeight = 20 * (n - 1);
        const int inf = numeric_limits<int>::max() / 4;
        vector<vector<int>> current(n, vector<int>(maxWeight + 1, inf));
        vector<vector<int>> next(n, vector<int>(maxWeight + 1, inf));
        current[0][0] = 0;

        for (int length = 1; length < n; ++length) {
            for (int v = 0; v < n; ++v) {
                fill(next[v].begin(), next[v].end(), inf);
            }
            int previousLimit = 20 * (length - 1);
            for (int u = 0; u < n; ++u) {
                if (!active[u]) continue;
                for (int sum = 0; sum <= previousLimit; ++sum) {
                    int square = current[u][sum];
                    if (square == inf) continue;
                    for (int edgeIndex : outgoing[u]) {
                        const Edge &e = edges[edgeIndex];
                        int newSum = sum + e.weight;
                        int newSquare = square + e.weight * e.weight;
                        int &target = next[e.to][newSum];
                        target = min(target, newSquare);
                    }
                }
            }
            for (int sum = 0; sum <= 20 * length; ++sum) {
                if (next[n - 1][sum] == inf) continue;
                long double mean = (long double)sum / length;
                long double variance = (long double)next[n - 1][sum] / length - mean * mean;
                answer = min(answer, max(0.0L, variance));
            }
            if (answer == 0.0L) return;
            current.swap(next);
        }
    }

    void run() {
        cin >> n >> m;
        edges.resize(m);
        outgoing.assign(n, {});
        vector<vector<int>> reverseGraph(n);
        for (int i = 0; i < m; ++i) {
            cin >> edges[i].from >> edges[i].to >> edges[i].weight;
            --edges[i].from;
            --edges[i].to;
            outgoing[edges[i].from].push_back(i);
            reverseGraph[edges[i].to].push_back(edges[i].from);
        }

        vector<char> fromSource(n, false), toSink(n, false);
        queue<int> q;
        q.push(0);
        fromSource[0] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int edgeIndex : outgoing[u]) {
                int v = edges[edgeIndex].to;
                if (!fromSource[v]) {
                    fromSource[v] = true;
                    q.push(v);
                }
            }
        }
        if (!fromSource[n - 1]) {
            cout << -1 << '\n';
            return;
        }

        q.push(n - 1);
        toSink[n - 1] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            for (int v : reverseGraph[u]) {
                if (!toSink[v]) {
                    toSink[v] = true;
                    q.push(v);
                }
            }
        }

        active.assign(n, false);
        for (int v = 0; v < n; ++v) active[v] = fromSource[v] && toSink[v];

        vector<Edge> allEdges = move(edges);
        vector<vector<int>> allOutgoing = move(outgoing);
        edges.clear();
        outgoing.assign(n, {});
        for (const Edge &e : allEdges) {
            if (!active[e.from] || !active[e.to]) continue;
            int index = edges.size();
            edges.push_back(e);
            outgoing[e.from].push_back(index);
        }
        m = edges.size();

        vector<int> indegree(n, 0);
        for (const Edge &e : edges) ++indegree[e.to];
        queue<int> topologicalQueue;
        for (int v = 0; v < n; ++v) {
            if (active[v] && indegree[v] == 0) topologicalQueue.push(v);
        }
        vector<int> topological;
        while (!topologicalQueue.empty()) {
            int u = topologicalQueue.front();
            topologicalQueue.pop();
            topological.push_back(u);
            for (int edgeIndex : outgoing[u]) {
                int v = edges[edgeIndex].to;
                if (--indegree[v] == 0) topologicalQueue.push(v);
            }
        }

        solveShortWalks();
        if (answer > 0.0L &&
            (int)topological.size() < count(active.begin(), active.end(), true)) {
            solveCyclic();
        }

        cout << fixed << setprecision(15) << (double)answer << '\n';
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    solver.run();
    return 0;
}

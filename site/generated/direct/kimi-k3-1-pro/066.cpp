#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int u, v, w;
};

struct LocalEdge {
    int u, v, w, sq;
};

static constexpr uint16_t INF_Q = numeric_limits<uint16_t>::max();

static long double getVariance(int len, int sum, int sqSum) {
    long long numerator = 1LL * len * sqSum - 1LL * sum * sum;
    return (long double)numerator / ((long double)len * len);
}

static void printAnswer(long double answer) {
    cout << fixed << setprecision(15) << answer << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<Edge> edges(m);
    vector<vector<int>> graph(n), reverseGraph(n);

    for (int i = 0; i < m; ++i) {
        int x, y, w;
        cin >> x >> y >> w;
        --x;
        --y;
        edges[i] = {x, y, w};
        graph[x].push_back(y);
        reverseGraph[y].push_back(x);
    }

    auto reachableFrom = [&](int source, const vector<vector<int>>& g) {
        vector<char> reached(n, false);
        queue<int> q;
        reached[source] = true;
        q.push(source);

        while (!q.empty()) {
            int v = q.front();
            q.pop();
            for (int to : g[v]) {
                if (!reached[to]) {
                    reached[to] = true;
                    q.push(to);
                }
            }
        }
        return reached;
    };

    vector<char> fromStart = reachableFrom(0, graph);
    vector<char> toFinish = reachableFrom(n - 1, reverseGraph);

    if (!fromStart[n - 1]) {
        cout << -1 << '\n';
        return 0;
    }

    vector<char> useful(n, false);
    for (int v = 0; v < n; ++v) {
        useful[v] = fromStart[v] && toFinish[v];
    }

    vector<int> id(n, -1);
    int N = 0;
    for (int v = 0; v < n; ++v) {
        if (useful[v]) {
            id[v] = N++;
        }
    }

    vector<LocalEdge> localEdges;
    vector<vector<int>> outgoing(N);
    int W = 0;

    for (const Edge& e : edges) {
        if (useful[e.u] && useful[e.v]) {
            int index = (int)localEdges.size();
            localEdges.push_back({id[e.u], id[e.v], e.w, e.w * e.w});
            outgoing[id[e.u]].push_back(index);
            W = max(W, e.w);
        }
    }

    const int start = id[0];
    const int target = id[n - 1];

    auto hasCycle = [&](int requiredWeight) {
        vector<char> color(N, 0);

        auto dfs = [&](auto&& self, int v) -> bool {
            color[v] = 1;
            for (int ei : outgoing[v]) {
                const LocalEdge& e = localEdges[ei];
                if (requiredWeight != -1 && e.w != requiredWeight) {
                    continue;
                }
                if (color[e.v] == 1) {
                    return true;
                }
                if (color[e.v] == 0 && self(self, e.v)) {
                    return true;
                }
            }
            color[v] = 2;
            return false;
        };

        for (int v = 0; v < N; ++v) {
            if (color[v] == 0 && dfs(dfs, v)) {
                return true;
            }
        }
        return false;
    };

    bool anyCycle = hasCycle(-1);

    // A constant-weight cycle has variance zero.
    if (anyCycle) {
        for (int w = 0; w <= W; ++w) {
            if (hasCycle(w)) {
                printAnswer(0.0L);
                return 0;
            }
        }
    }

    // -------------------- Paths from 1 to n --------------------

    long double answer = numeric_limits<long double>::infinity();

    const int maxPathLength = N - 1;
    const int pathStride = W * maxPathLength + 1;

    vector<uint16_t> previous((size_t)N * pathStride, INF_Q);
    vector<uint16_t> current((size_t)N * pathStride, INF_Q);

    previous[(size_t)start * pathStride] = 0;

    for (int len = 1; len <= maxPathLength; ++len) {
        fill(current.begin(), current.end(), INF_Q);
        int oldMaxSum = W * (len - 1);

        for (int v = 0; v < N; ++v) {
            const uint16_t* row =
                previous.data() + (size_t)v * pathStride;

            for (int sum = 0; sum <= oldMaxSum; ++sum) {
                int oldQ = row[sum];
                if (oldQ == INF_Q) {
                    continue;
                }

                for (int ei : outgoing[v]) {
                    const LocalEdge& e = localEdges[ei];
                    int newSum = sum + e.w;
                    int newQ = oldQ + e.sq;

                    uint16_t& slot =
                        current[(size_t)e.v * pathStride + newSum];

                    if (newQ < slot) {
                        slot = (uint16_t)newQ;
                    }
                }
            }
        }

        int maxSum = W * len;
        const uint16_t* targetRow =
            current.data() + (size_t)target * pathStride;

        for (int sum = 0; sum <= maxSum; ++sum) {
            int q = targetRow[sum];
            if (q == INF_Q) {
                continue;
            }

            answer = min(answer, getVariance(len, sum, q));
            if (answer == 0.0L) {
                printAnswer(0.0L);
                return 0;
            }
        }

        previous.swap(current);
    }

    if (!anyCycle) {
        printAnswer(answer);
        return 0;
    }

    // Every nonzero cycle variance is at least 1/N^2.
    const long double smallestPositiveCycleVariance =
        1.0L / ((long double)N * N);

    if (answer <= smallestPositiveCycleVariance) {
        printAnswer(answer);
        return 0;
    }

    // -------------------- Cycles --------------------

    const int stride = W * N + 1;
    const size_t layerSize = (size_t)N * stride;
    const size_t totalStates = (size_t)(N + 1) * layerSize;

    vector<uint16_t> dp(totalStates, INF_Q);
    vector<int16_t> parent(totalStates, -1);

    // Empty walks at every possible starting vertex.
    for (int v = 0; v < N; ++v) {
        dp[(size_t)v * stride] = 0;
    }

    for (int len = 1; len <= N; ++len) {
        const size_t previousLayer = (size_t)(len - 1) * layerSize;
        const size_t currentLayer = (size_t)len * layerSize;
        const int oldMaxSum = W * (len - 1);

        for (int v = 0; v < N; ++v) {
            const uint16_t* row =
                dp.data() + previousLayer + (size_t)v * stride;

            for (int sum = 0; sum <= oldMaxSum; ++sum) {
                int oldQ = row[sum];
                if (oldQ == INF_Q) {
                    continue;
                }

                for (int ei : outgoing[v]) {
                    const LocalEdge& e = localEdges[ei];
                    int newSum = sum + e.w;
                    int newQ = oldQ + e.sq;

                    size_t index =
                        currentLayer + (size_t)e.v * stride + newSum;

                    if (newQ < dp[index]) {
                        dp[index] = (uint16_t)newQ;
                        parent[index] = (int16_t)ei;
                    }
                }
            }
        }
    }

    vector<int> walk(N);
    vector<int> position(N);

    const size_t finalLayer = (size_t)N * layerSize;
    bool finished = false;

    for (int end = 0; end < N && !finished; ++end) {
        for (int sum = 0; sum <= W * N && !finished; ++sum) {
            size_t finalIndex =
                finalLayer + (size_t)end * stride + sum;

            if (dp[finalIndex] == INF_Q) {
                continue;
            }

            int v = end;
            int currentSum = sum;
            bool valid = true;

            for (int len = N; len >= 1; --len) {
                size_t index =
                    (size_t)len * layerSize +
                    (size_t)v * stride + currentSum;

                int ei = parent[index];
                if (ei < 0) {
                    valid = false;
                    break;
                }

                walk[len - 1] = ei;
                currentSum -= localEdges[ei].w;
                v = localEdges[ei].u;

                if (currentSum < 0) {
                    valid = false;
                    break;
                }
            }

            if (!valid || currentSum != 0) {
                continue;
            }

            // v is now the starting vertex of the reconstructed walk.
            fill(position.begin(), position.end(), -1);
            position[v] = 0;

            int cycleBegin = -1;
            int cycleEnd = -1;

            for (int i = 0; i < N; ++i) {
                v = localEdges[walk[i]].v;

                if (position[v] != -1) {
                    cycleBegin = position[v];
                    cycleEnd = i + 1;
                    break;
                }
                position[v] = i + 1;
            }

            if (cycleBegin == -1) {
                continue;
            }

            int cycleSum = 0;
            int cycleSqSum = 0;

            for (int i = cycleBegin; i < cycleEnd; ++i) {
                const LocalEdge& e = localEdges[walk[i]];
                cycleSum += e.w;
                cycleSqSum += e.sq;
            }

            int cycleLength = cycleEnd - cycleBegin;
            answer = min(
                answer,
                getVariance(cycleLength, cycleSum, cycleSqSum)
            );

            if (answer == 0.0L ||
                answer <= smallestPositiveCycleVariance) {
                finished = true;
            }
        }
    }

    printAnswer(answer);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static constexpr int64 INF = (1LL << 60);

struct Edge {
    int u, v, w;
};

struct Point {
    // Mean = s / len, second moment = q / len.
    int64 s, q;
    int len;
};

struct State {
    int64 cost = INF;
    int64 sum = 0;
};

struct Ratio {
    int64 cost, sum;
    int len;
};

// Lexicographic comparison of (cost / len, sum / len).
static int compareRatios(const Ratio& a, const Ratio& b) {
    i128 x = (i128)a.cost * b.len;
    i128 y = (i128)b.cost * a.len;
    if (x != y) return x < y ? -1 : 1;

    x = (i128)a.sum * b.len;
    y = (i128)b.sum * a.len;
    if (x != y) return x < y ? -1 : 1;
    return 0;
}

static void relax(State& dst, int64 cost, int64 sum) {
    if (cost < dst.cost || (cost == dst.cost && sum < dst.sum)) {
        dst.cost = cost;
        dst.sum = sum;
    }
}

static Point normalize(Point p) {
    int64 g = gcd((int64)p.len, gcd(llabs(p.s), llabs(p.q)));
    p.len /= (int)g;
    p.s /= g;
    p.q /= g;
    return p;
}

static long double meanOf(const Point& p) {
    return (long double)p.s / p.len;
}

static long double secondMomentOf(const Point& p) {
    return (long double)p.q / p.len;
}

static long double varianceOf(const Point& p) {
    int64 numerator = p.q * p.len - p.s * p.s;
    return (long double)numerator / ((long double)p.len * p.len);
}

class Solver {
    int n, source, target;
    vector<Edge> edges;
    bool hasCycle;

public:
    Solver(int n_, int source_, int target_,
           vector<Edge> edges_, bool hasCycle_)
        : n(n_), source(source_), target(target_),
          edges(move(edges_)), hasCycle(hasCycle_) {}

    /*
       Return a point minimizing

           q / len - (A / B) * s / len.

       Ties are broken by the smaller mean.
    */
    Point oracle(int64 A, int64 B) const {
        vector<int64> edgeCost(edges.size());
        for (int i = 0; i < (int)edges.size(); ++i) {
            int64 w = edges[i].w;
            edgeCost[i] = B * w * w - A * w;
        }

        vector<State> previous(n), current(n);
        previous[source] = {0, 0};

        vector<State> cycleDP;
        if (hasCycle) {
            cycleDP.resize((n + 1) * n);
            for (int v = 0; v < n; ++v)
                cycleDP[v] = {0, 0};
        }

        Ratio best{};
        bool haveBest = false;

        for (int k = 1; k <= n; ++k) {
            fill(current.begin(), current.end(), State{});

            State* cyclePrevious = nullptr;
            State* cycleCurrent = nullptr;
            if (hasCycle) {
                cyclePrevious = &cycleDP[(k - 1) * n];
                cycleCurrent = &cycleDP[k * n];
            }

            for (int i = 0; i < (int)edges.size(); ++i) {
                const auto& e = edges[i];
                int64 c = edgeCost[i];

                if (previous[e.u].cost != INF) {
                    relax(current[e.v],
                          previous[e.u].cost + c,
                          previous[e.u].sum + e.w);
                }

                if (hasCycle && cyclePrevious[e.u].cost != INF) {
                    relax(cycleCurrent[e.v],
                          cyclePrevious[e.u].cost + c,
                          cyclePrevious[e.u].sum + e.w);
                }
            }

            if (current[target].cost != INF) {
                Ratio candidate{
                    current[target].cost,
                    current[target].sum,
                    k
                };
                if (!haveBest || compareRatios(candidate, best) < 0) {
                    best = candidate;
                    haveBest = true;
                }
            }

            previous.swap(current);
        }

        if (hasCycle) {
            // Karp's minimum-mean-cycle formula.
            for (int v = 0; v < n; ++v) {
                const State& last = cycleDP[n * n + v];
                if (last.cost == INF) continue;

                Ratio worst{};
                bool haveWorst = false;

                for (int k = 0; k < n; ++k) {
                    const State& earlier = cycleDP[k * n + v];
                    if (earlier.cost == INF) continue;

                    Ratio candidate{
                        last.cost - earlier.cost,
                        last.sum - earlier.sum,
                        n - k
                    };

                    if (!haveWorst ||
                        compareRatios(candidate, worst) > 0) {
                        worst = candidate;
                        haveWorst = true;
                    }
                }

                if (haveWorst &&
                    (!haveBest || compareRatios(worst, best) < 0)) {
                    best = worst;
                    haveBest = true;
                }
            }
        }

        // best.cost = B * Q - A * S.
        int64 q = (best.cost + A * best.sum) / B;
        return normalize({best.sum, q, best.len});
    }

    long double solve() const {
        Point left = oracle(0, 1);
        Point right = oracle(40, 1);

        long double answer = min(varianceOf(left), varianceOf(right));

        struct Task {
            Point a, b;
            long double ta, tb;
        };

        vector<Task> tasks;
        tasks.push_back({left, right, 0.0L, 40.0L});

        while (!tasks.empty()) {
            Task task = tasks.back();
            tasks.pop_back();

            const Point& a = task.a;
            const Point& b = task.b;

            if ((i128)a.s * b.len == (i128)b.s * a.len)
                continue;

            /*
               Optional lower-bound pruning.

               Every feasible point (mu,q) lies above both supporting
               lines belonging to a and b. Their maximum, minus mu^2,
               gives a lower bound on the variance in this interval.
            */
            {
                long double ma = meanOf(a);
                long double mb = meanOf(b);
                long double qa = secondMomentOf(a);
                long double qb = secondMomentOf(b);

                long double ha = qa - task.ta * ma;
                long double hb = qb - task.tb * mb;

                long double lowerBound =
                    min(varianceOf(a), varianceOf(b));

                if (task.tb > task.ta) {
                    long double mu =
                        (ha - hb) / (task.tb - task.ta);
                    mu = max(ma, min(mb, mu));

                    long double qLower =
                        max(ha + task.ta * mu,
                            hb + task.tb * mu);

                    lowerBound = min(lowerBound,
                                     qLower - mu * mu);
                }

                // The permitted numerical error is much larger.
                if (lowerBound >= answer - 1e-12L)
                    continue;
            }

            // Intersection of q_a - t*mu_a and q_b - t*mu_b.
            int64 A = b.q * a.len - a.q * b.len;
            int64 B = b.s * a.len - a.s * b.len;

            if (B < 0) {
                A = -A;
                B = -B;
            }

            int64 g = gcd(llabs(A), B);
            A /= g;
            B /= g;

            Point middle = oracle(A, B);
            answer = min(answer, varianceOf(middle));

            if (answer <= 0)
                return 0;

            int64 middleCost = B * middle.q - A * middle.s;
            int64 endpointCost = B * a.q - A * a.s;

            // No point is strictly below the intersection:
            // these two envelope pieces are adjacent.
            if ((i128)middleCost * a.len ==
                (i128)endpointCost * middle.len) {
                continue;
            }

            long double t = (long double)A / B;
            tasks.push_back({middle, b, t, task.tb});
            tasks.push_back({a, middle, task.ta, t});
        }

        return max(0.0L, answer);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int originalN, m;
    cin >> originalN >> m;

    vector<Edge> originalEdges(m);
    vector<vector<int>> graph(originalN), reverseGraph(originalN);

    for (int i = 0; i < m; ++i) {
        int u, v, w;
        cin >> u >> v >> w;
        --u;
        --v;
        originalEdges[i] = {u, v, w};
        graph[u].push_back(v);
        reverseGraph[v].push_back(u);
    }

    auto reachable = [&](int start,
                         const vector<vector<int>>& adjacency) {
        vector<char> visited(originalN, false);
        vector<int> stack = {start};
        visited[start] = true;

        while (!stack.empty()) {
            int u = stack.back();
            stack.pop_back();
            for (int v : adjacency[u]) {
                if (!visited[v]) {
                    visited[v] = true;
                    stack.push_back(v);
                }
            }
        }
        return visited;
    };

    vector<char> fromSource = reachable(0, graph);
    if (!fromSource[originalN - 1]) {
        cout << "-1\n";
        return 0;
    }

    vector<char> toTarget =
        reachable(originalN - 1, reverseGraph);

    vector<int> id(originalN, -1);
    int n = 0;
    for (int v = 0; v < originalN; ++v) {
        if (fromSource[v] && toTarget[v])
            id[v] = n++;
    }

    int source = id[0];
    int target = id[originalN - 1];

    vector<Edge> edges;
    vector<vector<int>> outgoing(n);
    for (const auto& e : originalEdges) {
        if (id[e.u] != -1 && id[e.v] != -1) {
            int index = (int)edges.size();
            edges.push_back({id[e.u], id[e.v], e.w});
            outgoing[id[e.u]].push_back(index);
        }
    }

    /*
       A monochromatic source-target path has variance zero.
       A monochromatic useful cycle makes the infimum zero.
    */
    for (int weight = 0; weight <= 20; ++weight) {
        vector<char> visited(n, false);
        vector<int> stack = {source};
        visited[source] = true;

        while (!stack.empty()) {
            int u = stack.back();
            stack.pop_back();

            for (int index : outgoing[u]) {
                const auto& e = edges[index];
                if (e.w == weight && !visited[e.v]) {
                    visited[e.v] = true;
                    stack.push_back(e.v);
                }
            }
        }

        if (visited[target]) {
            cout << fixed << setprecision(15) << 0.0L << '\n';
            return 0;
        }

        vector<int> indegree(n, 0);
        for (const auto& e : edges)
            if (e.w == weight)
                ++indegree[e.v];

        queue<int> queue;
        for (int v = 0; v < n; ++v)
            if (indegree[v] == 0)
                queue.push(v);

        int removed = 0;
        while (!queue.empty()) {
            int u = queue.front();
            queue.pop();
            ++removed;

            for (int index : outgoing[u]) {
                const auto& e = edges[index];
                if (e.w == weight && --indegree[e.v] == 0)
                    queue.push(e.v);
            }
        }

        if (removed != n) {
            cout << fixed << setprecision(15) << 0.0L << '\n';
            return 0;
        }
    }

    // Determine whether Karp's cycle DP is needed.
    vector<int> indegree(n, 0);
    for (const auto& e : edges)
        ++indegree[e.v];

    queue<int> queue;
    for (int v = 0; v < n; ++v)
        if (indegree[v] == 0)
            queue.push(v);

    int removed = 0;
    while (!queue.empty()) {
        int u = queue.front();
        queue.pop();
        ++removed;

        for (int index : outgoing[u]) {
            int v = edges[index].v;
            if (--indegree[v] == 0)
                queue.push(v);
        }
    }

    bool hasCycle = (removed != n);

    Solver solver(n, source, target, move(edges), hasCycle);
    cout << fixed << setprecision(15) << solver.solve() << '\n';
    return 0;
}

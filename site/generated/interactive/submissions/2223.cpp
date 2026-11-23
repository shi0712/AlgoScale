#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
const int64 INF = (1LL << 62);

struct QueryEdge {
    int to;
    int64 w;
};

struct WeightedDSU {
    vector<int> parent, sz;
    vector<int64> diff; // value(x) - value(parent(x))

    explicit WeightedDSU(int n = 0) { init(n); }

    void init(int n) {
        parent.resize(n);
        sz.assign(n, 1);
        diff.assign(n, 0);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        int p = parent[x];
        int r = find(p);
        diff[x] += diff[p];
        parent[x] = r;
        return r;
    }

    bool connected(int a, int b) {
        return find(a) == find(b);
    }

    // Returns value(b) - value(a); a and b must be connected.
    int64 valueDiff(int a, int b) {
        int ra = find(a);
        int rb = find(b);
        (void)ra;
        (void)rb;
        return diff[b] - diff[a];
    }

    // Enforces value(b) - value(a) = w.
    void unite(int a, int b, int64 w) {
        int ra = find(a);
        int rb = find(b);
        int64 da = diff[a]; // value(a) - value(ra)
        int64 db = diff[b]; // value(b) - value(rb)
        if (ra == rb) return;

        if (sz[ra] < sz[rb]) {
            // Attach ra below rb.
            parent[ra] = rb;
            diff[ra] = db - da - w;
            sz[rb] += sz[ra];
        } else {
            // Attach rb below ra.
            parent[rb] = ra;
            diff[rb] = w + da - db;
            sz[ra] += sz[rb];
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto ask = [&](int l, int r) -> int64 {
        cout << "? " << l << ' ' << r << '\n';
        cout.flush();
        int64 response;
        if (!(cin >> response)) exit(0);
        if (response == -1) exit(0);
        return response;
    };

    vector<int64> singleton(n + 1);
    vector<int> positivePositions;
    vector<int64> positiveValues;

    for (int i = 1; i <= n; ++i) {
        singleton[i] = ask(i, i);
        if (singleton[i] > 0) {
            positivePositions.push_back(i - 1); // zero-based internally
            positiveValues.push_back(singleton[i]);
        }
    }

    vector<int64> answer(n, 0);

    int k = (int)positiveValues.size();
    if (k == 0) {
        cout << '!';
        for (int i = 0; i < n; ++i) cout << ' ' << 0;
        cout << '\n';
        cout.flush();
        return 0;
    }

    if (k == 1) {
        answer[positivePositions[0]] = positiveValues[0];
        cout << '!';
        for (int64 x : answer) cout << ' ' << x;
        cout << '\n';
        cout.flush();
        return 0;
    }

    // Positive vertices are numbered 1..k. Gap g lies between positive
    // vertices g and g+1. Prefix variables are x_0,...,x_{k-1}.
    vector<int64> values(k + 1, 0), prefixValues(k + 1, 0);
    for (int i = 1; i <= k; ++i) {
        values[i] = positiveValues[i - 1];
        prefixValues[i] = prefixValues[i - 1] + values[i];
    }

    const int N = k;
    WeightedDSU dsu(N);

    vector<tuple<int, int, int64>> upperConstraints;
    vector<tuple<int, int, int64>> equalityConstraints;
    vector<vector<QueryEdge>> forwardGraph(N);

    int constraintVersion = 0;

    auto addUpper = [&](int a, int b, int64 w) {
        upperConstraints.emplace_back(a, b, w);
        forwardGraph[a].push_back({b, w});
        ++constraintVersion;
    };

    auto addEquality = [&](int a, int b, int64 w) {
        equalityConstraints.emplace_back(a, b, w);
        dsu.unite(a, b, w);
    };

    vector<vector<int64>> mss(k + 1, vector<int64>(k + 1, 0));
    for (int i = 1; i <= k; ++i) mss[i][i] = values[i];

    // Cached shortest paths using only upper-bound constraints. Stale caches
    // remain valid upper bounds after new constraints are added.
    vector<vector<int64>> cachedDist(N, vector<int64>(N, INF));
    vector<int> cacheVersion(N, -1);

    int64 graphWork = 0;
    const int64 graphWorkLimit = 30000000LL;

    auto recomputeRow = [&](int source) {
        fill(cachedDist[source].begin(), cachedDist[source].end(), INF);
        cachedDist[source][source] = 0;
        for (int u = source; u < N; ++u) {
            int64 du = cachedDist[source][u];
            if (du == INF) continue;
            for (const auto &e : forwardGraph[u]) {
                int64 candidate = du + e.w;
                if (candidate < cachedDist[source][e.to]) {
                    cachedDist[source][e.to] = candidate;
                }
            }
        }
        cacheVersion[source] = constraintVersion;
        graphWork += (int64)(N - source) + (int64)upperConstraints.size();
    };

    auto getUpperDistance = [&](int source, int target, int64 fullValue,
                                int64 lowerMSS) -> int64 {
        int64 old = cachedDist[source][target];
        if (cacheVersion[source] != constraintVersion) {
            if (old != INF && fullValue + old <= lowerMSS) return old;

            int64 estimatedCost =
                (int64)(N - source) + (int64)upperConstraints.size();
            if (graphWork + estimatedCost <= graphWorkLimit) {
                recomputeRow(source);
                return cachedDist[source][target];
            }
            return old;
        }
        return old;
    };

    for (int length = 2; length <= k; ++length) {
        for (int s = 1; s + length - 1 <= k; ++s) {
            int t = s + length - 1;
            int a = s - 1;
            int b = t - 1;

            int64 V = prefixValues[t] - prefixValues[s - 1];
            int64 L = max(mss[s][t - 1], mss[s + 1][t]);

            // An empty original gap has exactly zero cost and needs no query.
            if (length == 2 &&
                positivePositions[s] == positivePositions[s - 1] + 1) {
                mss[s][t] = V;
                addUpper(a, b, 0);
                addEquality(a, b, 0);
                continue;
            }

            if (dsu.connected(a, b)) {
                int64 gapSum = dsu.valueDiff(a, b);
                mss[s][t] = max(L, V + gapSum);
                continue;
            }

            int64 upperDistance = getUpperDistance(a, b, V, L);
            if (upperDistance != INF && V + upperDistance <= L) {
                mss[s][t] = L;
                continue;
            }

            int left = positivePositions[s - 1] + 1;
            int right = positivePositions[t - 1] + 1;
            int64 M = ask(left, right);
            mss[s][t] = M;

            int64 U = M - V;
            addUpper(a, b, U);
            if (M > L) addEquality(a, b, U);
        }
    }

    // Find a feasible set of prefix variables satisfying every learned
    // constraint. Lower bounds on individual gaps keep the output small.
    vector<tuple<int, int, int64>> constraints;
    constraints.reserve(upperConstraints.size() +
                        2 * equalityConstraints.size() + 2 * k);

    for (auto [a, b, w] : upperConstraints) {
        constraints.emplace_back(a, b, w); // x_b <= x_a + w
    }
    for (auto [a, b, w] : equalityConstraints) {
        constraints.emplace_back(a, b, w);
        constraints.emplace_back(b, a, -w);
    }

    for (int g = 1; g <= k - 1; ++g) {
        int gapLength = positivePositions[g] - positivePositions[g - 1] - 1;
        if (gapLength > 0) {
            int64 lowerMagnitude = (int64)gapLength * 1000000000LL;
            // x_g - x_{g-1} >= -lowerMagnitude.
            constraints.emplace_back(g, g - 1, lowerMagnitude);
        }
    }

    vector<int64> dist(N, 0), x(N, 0);
    vector<int> relaxCount(N, 0);
    vector<char> inQueue(N, 1);
    queue<int> q;
    for (int i = 0; i < N; ++i) q.push(i);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        inQueue[u] = 0;

        for (auto [a, b, w] : constraints) {
            if (a != u) continue;
            if (dist[b] > dist[a] + w) {
                dist[b] = dist[a] + w;
                if (!inQueue[b]) {
                    inQueue[b] = 1;
                    q.push(b);
                }
            }
        }
    }

    int64 shift = dist[0];
    for (int i = 0; i < N; ++i) x[i] = dist[i] - shift;

    vector<int64> gapCost(k, 0);
    for (int g = 1; g <= k - 1; ++g) {
        gapCost[g] = x[g] - x[g - 1];
    }

    for (int i = 0; i < k; ++i) {
        answer[positivePositions[i]] = positiveValues[i];
    }
    for (int g = 1; g <= k - 1; ++g) {
        int leftPositive = positivePositions[g - 1];
        int rightPositive = positivePositions[g];
        if (rightPositive > leftPositive + 1) {
            answer[leftPositive + 1] = gapCost[g];
        }
    }

    cout << '!';
    for (int64 v : answer) cout << ' ' << v;
    cout << '\n';
    cout.flush();
    return 0;
}

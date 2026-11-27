#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

class Interactive {
public:
    int64 ask(int l, int r) {
        cout << "? " << l << ' ' << r << '\n';
        cout.flush();

        int64 response;
        if (!(cin >> response) || response < 0) {
            exit(0);
        }
        return response;
    }
};

struct DSU {
    vector<int> parent, sz;
    vector<int64> diff;

    explicit DSU(int n) : parent(n), sz(n, 1), diff(n, 0) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] == x) return x;
        int p = parent[x];
        int root = find(p);
        diff[x] += diff[p];
        parent[x] = root;
        return root;
    }

    // Adds the equation value[v] - value[u] = w.
    void addEquation(int u, int v, int64 w) {
        int ru = find(u);
        int rv = find(v);
        int64 du = diff[u];
        int64 dv = diff[v];

        if (ru == rv) {
            return;
        }

        // value[rv] - value[ru] = w + du - dv.
        int64 relation = w + du - dv;

        if (sz[ru] >= sz[rv]) {
            parent[rv] = ru;
            diff[rv] = relation;
            sz[ru] += sz[rv];
        } else {
            parent[ru] = rv;
            diff[ru] = -relation;
            sz[rv] += sz[ru];
        }
    }

    int64 potential(int x) {
        find(x);
        return diff[x];
    }
};

struct Edge {
    int to;
    int64 weight;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    Interactive interactor;

    int64 whole = interactor.ask(1, n);
    if (whole == 0) {
        cout << "!";
        for (int i = 0; i < n; ++i) cout << ' ' << 0;
        cout << '\n';
        cout.flush();
        return 0;
    }

    if (n == 1) {
        cout << "! " << whole << '\n';
        cout.flush();
        return 0;
    }

    vector<int64> positiveValue(n, 0);
    vector<int> positivePositions;

    for (int i = 1; i <= n; ++i) {
        int64 value = interactor.ask(i, i);
        if (value > 0) {
            positiveValue[i] = value;
            positivePositions.push_back(i);
        }
    }

    struct Block {
        int left, right;
        int64 sum;
    };

    vector<Block> blocks;
    for (int p : positivePositions) {
        if (blocks.empty() || blocks.back().right + 1 != p) {
            blocks.push_back({p, p, 0});
        }
        blocks.back().right = p;
        blocks.back().sum += positiveValue[p];
    }

    int m = static_cast<int>(blocks.size());

    vector<int64> gapCost(max(0, m - 1), 0);

    if (m == 1) {
        vector<int64> answer(n, 0);
        for (int i = 1; i <= n; ++i) {
            if (positiveValue[i] > 0) answer[i - 1] = positiveValue[i];
        }

        cout << "!";
        for (int64 value : answer) cout << ' ' << value;
        cout << '\n';
        cout.flush();
        return 0;
    }

    if (m == 2) {
        int64 maximum = interactor.ask(blocks[0].left, blocks[1].right);
        int64 leftSum = blocks[0].sum;
        int64 rightSum = blocks[1].sum;

        if (maximum > max(leftSum, rightSum)) {
            gapCost[0] = leftSum + rightSum - maximum;
        } else {
            gapCost[0] = min(leftSum, rightSum);
        }

        vector<int64> answer(n, 0);
        for (int i = 1; i <= n; ++i) {
            if (positiveValue[i] > 0) answer[i - 1] = positiveValue[i];
        }

        int gapPosition = blocks[0].right + 1;
        answer[gapPosition - 1] = -gapCost[0];

        cout << "!";
        for (int64 value : answer) cout << ' ' << value;
        cout << '\n';
        cout.flush();
        return 0;
    }

    vector<vector<int64>> maximum(m, vector<int64>(m, 0));
    for (int i = 0; i < m; ++i) {
        maximum[i][i] = blocks[i].sum;
    }

    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m; ++j) {
            maximum[i][j] = interactor.ask(blocks[i].left, blocks[j].right);
        }
    }

    vector<int64> prefixSum(m + 1, 0);
    for (int i = 0; i < m; ++i) {
        prefixSum[i + 1] = prefixSum[i] + blocks[i].sum;
    }

    // X[t] is the sum of the first t gap costs.
    DSU dsu(m);

    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m; ++j) {
            bool strict =
                maximum[i][j] > maximum[i + 1][j] &&
                maximum[i][j] > maximum[i][j - 1];

            if (strict) {
                int64 required =
                    prefixSum[j + 1] - prefixSum[i] - maximum[i][j];
                dsu.addEquation(i - 1, j - 1, required);
            }
        }
    }

    vector<int64> potential(m);
    vector<int> rootId(m, -1);
    vector<int> roots;
    for (int i = 0; i < m; ++i) {
        int root = dsu.find(i);
        potential[i] = dsu.diff[i];
        if (rootId[root] == -1) {
            rootId[root] = static_cast<int>(roots.size());
            roots.push_back(root);
        }
    }

    int componentCount = static_cast<int>(roots.size());
    vector<vector<Edge>> graph(componentCount);

    auto addConstraint = [&](int low, int high, int64 requiredDifference) {
        int from = rootId[dsu.find(low)];
        int to = rootId[dsu.find(high)];

        if (from == to) return;

        int64 weight =
            requiredDifference + potential[low] - potential[high];
        graph[from].push_back({to, weight});
    };

    // For every block range, its total gap cost is at least this value.
    for (int i = 0; i < m; ++i) {
        for (int j = i + 1; j < m; ++j) {
            int64 required =
                prefixSum[j + 1] - prefixSum[i] - maximum[i][j];
            addConstraint(i - 1, j - 1, required);
        }
    }

    constexpr int64 GAP_LIMIT = 2'000'000'000'000LL;

    for (int j = 1; j < m; ++j) {
        // 0 <= X[j] - X[j-1] <= GAP_LIMIT.
        addConstraint(j - 1, j, 0);

        int from = rootId[dsu.find(j)];
        int to = rootId[dsu.find(j - 1)];
        if (from != to) {
            int64 weight = potential[j] - potential[j - 1] - GAP_LIMIT;
            graph[from].push_back({to, weight});
        }
    }

    // Solve lower-bounded difference constraints.
    vector<int64> offset(componentCount, 0);
    vector<char> inQueue(componentCount, false);
    deque<int> queue;

    for (int i = 0; i < componentCount; ++i) {
        queue.push_back(i);
        inQueue[i] = true;
    }

    while (!queue.empty()) {
        int u = queue.front();
        queue.pop_front();
        inQueue[u] = false;

        for (const Edge& edge : graph[u]) {
            int64 candidate = offset[u] + edge.weight;
            if (candidate > offset[edge.to]) {
                offset[edge.to] = candidate;
                if (!inQueue[edge.to]) {
                    if (!queue.empty() &&
                        offset[edge.to] > offset[queue.front()]) {
                        queue.push_front(edge.to);
                    } else {
                        queue.push_back(edge.to);
                    }
                    inQueue[edge.to] = true;
                }
            }
        }
    }

    vector<int64> x(m);
    for (int i = 0; i < m; ++i) {
        x[i] = potential[i] + offset[rootId[dsu.find(i)]];
    }

    for (int j = 1; j < m; ++j) {
        gapCost[j - 1] = x[j] - x[j - 1];
    }

    vector<int64> answer(n, 0);
    for (int i = 1; i <= n; ++i) {
        if (positiveValue[i] > 0) {
            answer[i - 1] = positiveValue[i];
        }
    }

    for (int j = 0; j + 1 < m; ++j) {
        int gapPosition = blocks[j].right + 1;
        answer[gapPosition - 1] = -gapCost[j];
    }

    cout << "!";
    for (int64 value : answer) cout << ' ' << value;
    cout << '\n';
    cout.flush();

    return 0;
}

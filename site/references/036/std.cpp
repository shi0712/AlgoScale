#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

struct Matrix {
    int a = 0, b = 0, c = 0, d = 0;
};

Matrix multiply(const Matrix &x, const Matrix &y) {
    return {
        int((1LL * x.a * y.a + 1LL * x.b * y.c) % MOD),
        int((1LL * x.a * y.b + 1LL * x.b * y.d) % MOD),
        int((1LL * x.c * y.a + 1LL * x.d * y.c) % MOD),
        int((1LL * x.c * y.b + 1LL * x.d * y.d) % MOD)
    };
}

class DynamicDP {
    struct Node {
        int left = 0, right = 0, light = 0, parent = 0;
        Matrix value;
    };

    int n, total, expressionRoot;
    const vector<vector<int>> &graph;
    vector<Node> tree;
    vector<int> parent, size, heavy;
    vector<unsigned char> active;

    void pull(int u) {
        Node &t = tree[u];
        if (u > n) {
            // 合并两组轻子树的贡献，只使用矩阵的第一列。
            const Matrix &x = tree[t.left].value;
            const Matrix &y = tree[t.right].value;
            t.value = {
                int(1LL * x.a * y.a % MOD), 0,
                int((1LL * x.c * y.a + 1LL * x.a * y.c) % MOD), 0
            };
            return;
        }

        // 轻子树合并结果为 (A, B)，空集合为 (1, 0)。
        int A = tree[t.light].value.a;
        int B = tree[t.light].value.c;
        Matrix m;
        if (active[u]) {
            int s = A + B;
            if (s >= MOD) s -= MOD;
            m = {s, A, B, A};
        } else {
            m = {A, 0, A, 0};
        }

        // 按重链从上到下的顺序：左区间 * 当前点矩阵 * 右区间。
        if (t.left)  m = multiply(tree[t.left].value, m);
        if (t.right) m = multiply(m, tree[t.right].value);
        t.value = m;
    }

    int joinRake(int x, int y) {
        if (!x) return y;
        if (!y) return x;
        int u = ++total;
        tree[u].left = x;
        tree[u].right = y;
        tree[x].parent = tree[y].parent = u;
        pull(u);
        return u;
    }

    // prefix 是正权值的前缀和，返回区间的带权中位数。
    static int weightedMedian(const vector<int> &prefix, int l, int r) {
        int target = prefix[l] + (prefix[r + 1] - prefix[l] + 1) / 2;
        return int(lower_bound(prefix.begin() + l + 1,
                               prefix.begin() + r + 2, target)
                   - prefix.begin()) - 1;
    }

    int buildRake(const vector<int> &ids, const vector<int> &prefix,
                  int l, int r) {
        if (l > r) return 0;
        if (l == r) return ids[l];
        int m = weightedMedian(prefix, l, r);
        int x = buildRake(ids, prefix, l, m - 1);
        int y = ids[m];
        int z = buildRake(ids, prefix, m + 1, r);
        return joinRake(joinRake(x, y), z);
    }

    int buildCompress(const vector<int> &ids, const vector<int> &prefix,
                      int l, int r) {
        if (l > r) return 0;
        int m = weightedMedian(prefix, l, r);
        int u = ids[m];
        int x = buildCompress(ids, prefix, l, m - 1);
        int y = buildCompress(ids, prefix, m + 1, r);
        tree[u].left = x;
        tree[u].right = y;
        if (x) tree[x].parent = u;
        if (y) tree[y].parent = u;
        pull(u);
        return u;
    }

    int buildChain(int head) {
        vector<int> ids;
        vector<int> prefix(1, 0);
        for (int u = head; u; u = heavy[u]) {
            vector<int> lightIds;
            vector<int> lightPrefix(1, 0);
            for (int v : graph[u]) {
                if (parent[v] == u && v != heavy[u]) {
                    lightIds.push_back(buildChain(v));
                    lightPrefix.push_back(lightPrefix.back() + size[v]);
                }
            }
            int lightRoot = buildRake(lightIds, lightPrefix,
                                      0, int(lightIds.size()) - 1);
            tree[u].light = lightRoot;
            if (lightRoot) tree[lightRoot].parent = u;

            ids.push_back(u);
            // 点权为当前点及其所有轻子树的大小之和。
            prefix.push_back(prefix.back() + size[u] - size[heavy[u]]);
        }
        return buildCompress(ids, prefix, 0, int(ids.size()) - 1);
    }

public:
    DynamicDP(int n_, const vector<vector<int>> &g)
        : n(n_), total(n_), graph(g), tree(2 * n_ + 5),
          parent(n_ + 1), size(n_ + 1), heavy(n_ + 1), active(n_ + 1) {
        // 空链取单位矩阵，其第一列也恰好是空轻子树集合 (1, 0)。
        tree[0].value = {1, 0, 0, 1};

        vector<int> order;
        order.reserve(n);
        order.push_back(1);
        for (int i = 0; i < int(order.size()); ++i) {
            int u = order[i];
            for (int v : graph[u]) {
                if (v == parent[u]) continue;
                parent[v] = u;
                order.push_back(v);
            }
        }
        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            size[u] = 1;
            for (int v : graph[u]) {
                if (parent[v] != u) continue;
                size[u] += size[v];
                if (size[v] > size[heavy[u]]) heavy[u] = v;
            }
        }
        expressionRoot = buildChain(1);
    }

    bool isActive(int u) const { return active[u] != 0; }

    void activate(int u) {
        active[u] = 1;
        for (int x = u; x; x = tree[x].parent) pull(x);
    }

    int forestCount() const {
        // 根所在重链的矩阵积乘 (1, 0)^T，第二维就是 f_1[root]。
        return tree[expressionRoot].value.c;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<vector<int>> graph(n + 1);
    for (int i = 1; i < n; ++i) {
        int u, v;
        cin >> u >> v;
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    vector<int> power2(n + 1, 1);
    for (int i = 1; i <= n; ++i)
        power2[i] = int(2LL * power2[i - 1] % MOD);

    DynamicDP dp(n, graph);
    int exponent = 0; // 未操作点诱导森林的连通块数减一。
    for (int i = 1; i < n; ++i) {
        int x;
        cin >> x;
        int unoperatedNeighbors = 0;
        for (int v : graph[x])
            if (!dp.isActive(v)) ++unoperatedNeighbors;

        exponent += unoperatedNeighbors - 1;
        dp.activate(x);
        cout << 1LL * power2[exponent] * dp.forestCount() % MOD << '\n';
    }
    return 0;
}

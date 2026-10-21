#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <cassert>
using namespace std;

struct Operation {
    array<int, 4> from, to;
};

// 标准形：r-B-C，其余顶点都与 r 相连。
vector<Operation> normalize(const vector<vector<int>>& g,
                            int r, int B, int C) {
    int n = (int)g.size() - 1;
    vector<int> parent(n + 1), depth(n + 1), order{r};
    for (int i = 0; i < (int)order.size(); ++i) {
        int u = order[i];
        for (int v : g[u]) {
            if (v == parent[u]) continue;
            parent[v] = u;
            depth[v] = depth[u] + 1;
            order.push_back(v);
        }
    }

    vector<Operation> ops;
    auto add = [&](array<int, 4> from, array<int, 4> to) {
        ops.push_back({from, to});
    };

    int a = 0, b = 0, c = 0;
    if (g[r].size() == 1) {
        // 非星形树中，从叶子 r 出发一定能找到长度为 3 的路径。
        for (int v : order) {
            if (depth[v] == 3) { c = v; break; }
        }
        b = parent[c];
        a = parent[b];
        add({r, a, b, c}, {a, r, b, c});
        parent[b] = r;
    } else {
        for (int v : order) {
            if (depth[v] == 2) { c = v; break; }
        }
        b = parent[c];
        for (int v : g[r]) {
            if (v != b) { a = v; break; }
        }
    }

    assert(a && b && c);
    // 保持核心路径 a-r-b-c，按父亲先于儿子的顺序处理。
    for (int v : order) {
        if (v == r || v == a || v == b || v == c) continue;
        int p = parent[v];
        if (p == r) continue;

        if (p == c) {
            add({r, b, c, v}, {c, b, r, v});
        } else if (p == b) {
            add({a, r, b, c}, {a, r, c, b});
            add({r, c, b, v}, {c, b, r, v});
        } else {
            add({b, r, p, v}, {b, p, r, v});
            add({r, p, b, c}, {p, r, b, c});
        }
        parent[v] = r;
    }

    // 把特殊分支 b-c 的编号统一成 B-C。
    if (b != B) {
        if (c == B) {
            add({a, r, b, c}, {a, r, c, b});
            swap(b, c);
        } else {
            add({B, r, b, c}, {b, r, B, c});
            b = B;
        }
    }
    if (c != C) {
        add({C, r, b, c}, {c, r, b, C});
    }
    return ops;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<vector<int>> S(n + 1), T(n + 1);
    auto read = [&](vector<vector<int>>& g) {
        for (int i = 1, u, v; i < n; ++i) {
            cin >> u >> v;
            g[u].push_back(v);
            g[v].push_back(u);
        }
    };
    read(S);
    read(T);

    auto starCenter = [&](const vector<vector<int>>& g) {
        for (int i = 1; i <= n; ++i) {
            if ((int)g[i].size() == n - 1) return i;
        }
        return 0;
    };
    int s = starCenter(S), t = starCenter(T);
    if (s) {
        if (s == t) cout << "YES\n0\n";
        else cout << "NO\n";
        return 0;
    }

    int r = t ? t : 1;
    vector<int> other;
    for (int i = 1; i <= n; ++i) {
        if (i != r) other.push_back(i);
    }
    int B = other[0], C = other[1], D = other[2];

    vector<Operation> ans = normalize(S, r, B, C);
    if (!t) {
        auto ops = normalize(T, r, B, C);
        for (int i = (int)ops.size() - 1; i >= 0; --i) {
            ans.push_back({ops[i].to, ops[i].from});
        }
    }

    assert(ans.size() + (t != 0) <= 400);
    cout << "YES\n" << ans.size() + (t != 0) << '\n';
    for (const auto& op : ans) {
        const auto& p = op.from;
        const auto& q = op.to;
        cout << p[0] << ' ' << p[1] << ' ' << p[2] << ' ' << p[3] << '\n';
        cout << q[0] << ' ' << q[1] << ' '
             << q[1] << ' ' << q[2] << ' '
             << q[2] << ' ' << q[3] << '\n';
    }
    if (t) {
        // 最后一步把标准形变成以 r 为中心的星形树。
        cout << D << ' ' << r << ' ' << B << ' ' << C << '\n';
        cout << r << ' ' << D << ' ' << r << ' ' << B << ' '
             << r << ' ' << C << '\n';
    }
}

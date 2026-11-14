#include <bits/stdc++.h>
using namespace std;

struct Operation {
    array<int, 4> from, to;
    bool star = false;
};

struct Tree {
    int n;
    vector<vector<unsigned char>> edge;
    Tree(int size) : n(size), edge(size, vector<unsigned char>(size)) {}

    int degree(int v) const {
        int d = 0;
        for (int u = 0; u < n; ++u) d += edge[v][u];
        return d;
    }

    int starCenter() const {
        for (int v = 0; v < n; ++v)
            if (degree(v) == n - 1) return v;
        return -1;
    }

    void move(array<int, 4> from, array<int, 4> to,
              vector<Operation>& ops) {
        for (int i = 0; i < 3; ++i) {
            int a = from[i], b = from[i + 1];
            edge[a][b] = edge[b][a] = 0;
        }
        for (int i = 0; i < 3; ++i) {
            int a = to[i], b = to[i + 1];
            edge[a][b] = edge[b][a] = 1;
        }
        ops.push_back({from, to});
    }

    // Make r adjacent to every vertex except one leaf. Each increase in
    // degree(r) takes at most two reversible path-to-path operations.
    pair<int, int> normalize(int r, vector<Operation>& ops) {
        while (degree(r) < n - 2) {
            vector<int> parent(n, -1), depth(n, -1), q;
            q.push_back(r);
            depth[r] = 0;
            for (int i = 0; i < (int)q.size(); ++i) {
                int v = q[i];
                for (int u = 0; u < n; ++u) {
                    if (edge[v][u] && depth[u] == -1) {
                        depth[u] = depth[v] + 1;
                        parent[u] = v;
                        q.push_back(u);
                    }
                }
            }
            int deep = -1;
            for (int v = 0; v < n; ++v)
                if (depth[v] == 3) { deep = v; break; }
            if (deep != -1) {
                int c = deep, b = parent[c], a = parent[b];
                move({r, a, b, c}, {a, r, b, c}, ops);
                continue;
            }

            // All vertices are within distance two. There are at least
            // two distance-two leaves because degree(r) < n-2.
            vector<int> leaves;
            for (int v = 0; v < n; ++v)
                if (depth[v] == 2) leaves.push_back(v);
            int b = leaves[0], c = leaves[1];
            int a = parent[b], d = parent[c];
            if (a == d) {
                int x = -1;
                for (int v = 0; v < n; ++v)
                    if (edge[r][v] && v != a) { x = v; break; }
                // Such an x exists: otherwise the whole tree is a star.
                move({b, a, r, x}, {a, b, r, x}, ops);
                move({r, b, a, c}, {b, r, a, c}, ops);
            } else {
                move({b, a, r, d}, {a, r, b, d}, ops);
                move({r, b, d, c}, {b, r, d, c}, ops);
            }
        }
        int tail = -1, attachment = -1;
        for (int v = 0; v < n; ++v)
            if (v != r && !edge[r][v]) tail = v;
        for (int v = 0; v < n; ++v)
            if (edge[tail][v]) attachment = v;
        return {attachment, tail};
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    Tree s(n), t(n);
    for (Tree* tr : {&s, &t}) {
        for (int i = 0; i < n - 1; ++i) {
            int a, b;
            cin >> a >> b;
            --a; --b;
            tr->edge[a][b] = tr->edge[b][a] = 1;
        }
    }
    if (s.edge == t.edge) {
        cout << "YES\n0\n";
        return 0;
    }
    if (s.starCenter() != -1) {
        cout << "NO\n";
        return 0;
    }

    vector<Operation> answer;
    int targetCenter = t.starCenter();
    if (targetCenter != -1) {
        int r = targetCenter;
        auto [a, tail] = s.normalize(r, answer);
        int x = -1;
        for (int v = 0; v < n; ++v)
            if (s.edge[r][v] && v != a) { x = v; break; }
        // The final operation replaces the handle by the missing star edge.
        answer.push_back({{tail, a, r, x}, {r, tail, a, x}, true});
    } else {
        const int r = 0;
        vector<Operation> targetOps;
        // Each normalization costs at most 2(n-3) moves, and joining the
        // two normalized trees costs at most two: 4n-10 <= 390 in total.
        auto [a, tail] = s.normalize(r, answer);
        auto [b, targetTail] = t.normalize(r, targetOps);

        if (tail != targetTail) {
            if (a == targetTail) {
                int x = -1;
                for (int v = 0; v < n; ++v)
                    if (s.edge[r][v] && v != a) { x = v; break; }
                s.move({tail, a, r, x}, {a, tail, r, x}, answer);
                swap(a, tail);
            } else {
                s.move({tail, a, r, targetTail},
                       {targetTail, a, r, tail}, answer);
                tail = targetTail;
            }
        }
        if (a != b) {
            s.move({tail, a, r, b}, {a, r, b, tail}, answer);
        }
        for (int i = (int)targetOps.size() - 1; i >= 0; --i)
            answer.push_back({targetOps[i].to, targetOps[i].from});
    }

    cout << "YES\n" << answer.size() << '\n';
    for (const auto& op : answer) {
        for (int i = 0; i < 4; ++i)
            cout << op.from[i] + 1 << (i == 3 ? '\n' : ' ');
        for (int i = 0; i < 3; ++i) {
            int a = op.star ? op.to[0] : op.to[i];
            int b = op.to[i + 1];
            cout << a + 1 << ' ' << b + 1 << (i == 2 ? '\n' : ' ');
        }
    }
}

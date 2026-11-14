#include <bits/stdc++.h>
using namespace std;

using Edge = pair<int, int>;
using Path = array<int, 4>;

struct Operation {
    Path path;
    Path inversePath;
    array<Edge, 3> oldEdges;
    array<Edge, 3> newEdges;
};

Edge edge(int a, int b) {
    if (a > b) swap(a, b);
    return {a, b};
}

struct Tree {
    int n;
    set<Edge> edges;

    bool has(int a, int b) const {
        return edges.count(edge(a, b));
    }

    vector<int> neighbors(int v) const {
        vector<int> result;
        for (auto [a, b] : edges) {
            if (a == v) result.push_back(b);
            if (b == v) result.push_back(a);
        }
        return result;
    }

    Operation apply(Path p, array<Edge, 3> replacement, Path inversePath) {
        Operation op;
        op.path = p;
        op.inversePath = inversePath;
        op.oldEdges = {edge(p[0], p[1]), edge(p[1], p[2]), edge(p[2], p[3])};
        op.newEdges = replacement;
        for (Edge e : op.oldEdges) edges.erase(e);
        for (Edge e : replacement) edges.insert(e);
        return op;
    }
};

vector<Operation> reduceToNearStar(Tree &tree, int c) {
    vector<Operation> ops;
    int n = tree.n;

    while (true) {
        vector<Edge> outer;
        for (Edge e : tree.edges) {
            if (e.first != c && e.second != c) outer.push_back(e);
        }
        if (outer.size() <= 1) break;

        vector<int> parent(n + 1, -1), depth(n + 1, -1);
        queue<int> q;
        q.push(c);
        depth[c] = 0;
        while (!q.empty()) {
            int v = q.front();
            q.pop();
            for (int u : tree.neighbors(v)) {
                if (depth[u] == -1) {
                    depth[u] = depth[v] + 1;
                    parent[u] = v;
                    q.push(u);
                }
            }
        }

        int d = -1;
        for (int v = 1; v <= n; ++v) {
            if (depth[v] == 3) {
                d = v;
                break;
            }
        }

        if (d != -1) {
            int b = parent[d], a = parent[b];
            ops.push_back(tree.apply(
                {c, a, b, d},
                {edge(c, a), edge(c, b), edge(b, d)},
                {a, c, b, d}
            ));
            continue;
        }

        auto oriented = [&](Edge e) {
            if (depth[e.first] == 2) swap(e.first, e.second);
            return e;
        };
        Edge e1 = oriented(outer[0]), e2 = oriented(outer[1]);
        int b = e1.first, leaf = e1.second;

        if (e1.first != e2.first) {
            int qv = e2.first, e = e2.second;
            ops.push_back(tree.apply(
                {qv, c, b, leaf},
                {edge(qv, leaf), edge(leaf, c), edge(c, b)},
                {qv, leaf, c, b}
            ));
            ops.push_back(tree.apply(
                {c, leaf, qv, e},
                {edge(c, leaf), edge(c, qv), edge(qv, e)},
                {leaf, c, qv, e}
            ));
        } else {
            int e = e2.second;
            int qv = -1;
            for (int v : tree.neighbors(c)) {
                if (v != b) {
                    qv = v;
                    break;
                }
            }
            ops.push_back(tree.apply(
                {qv, c, b, leaf},
                {edge(qv, c), edge(c, leaf), edge(leaf, b)},
                {qv, c, leaf, b}
            ));
            ops.push_back(tree.apply(
                {c, leaf, b, e},
                {edge(c, leaf), edge(c, b), edge(b, e)},
                {leaf, c, b, e}
            ));
        }
    }
    return ops;
}

Operation changeNearStar(Tree &tree, int c, int a, int b, int q, int z, int w) {
    int r = -1;
    for (int v : {q, a, b}) {
        if (v != z && v != w) r = v;
    }
    array<Edge, 3> replacement = {edge(z, w), edge(c, w), edge(c, r)};
    return tree.apply({q, c, a, b}, replacement, {r, c, w, z});
}

vector<Operation> normalize(Tree &tree, int c, int z, int w) {
    vector<Operation> ops = reduceToNearStar(tree, c);
    Edge last;
    for (Edge e : tree.edges) {
        if (e.first != c && e.second != c) last = e;
    }
    int a = last.first, b = last.second;
    if (!tree.has(c, a)) swap(a, b);

    auto otherVertex = [&](int x, int y) {
        for (int v = 1; v <= tree.n; ++v) {
            if (v != c && v != x && v != y) return v;
        }
        return -1;
    };

    if (edge(a, b) == edge(z, w) && !tree.has(c, z)) return ops;

    if (edge(a, b) == edge(z, w)) {
        int q = otherVertex(a, b);
        ops.push_back(changeNearStar(tree, c, a, b, q, z, w));
        return ops;
    }

    if (a != z && a != w && b != z && b != w) {
        int q = z;
        ops.push_back(changeNearStar(tree, c, a, b, q, z, a));
        b = z;
    }
    int q = (a == z || b == z) ? w : z;
    ops.push_back(changeNearStar(tree, c, a, b, q, z, w));
    return ops;
}

void printOperation(const Operation &op, bool reverse) {
    const Path &p = reverse ? op.inversePath : op.path;
    const auto &es = reverse ? op.oldEdges : op.newEdges;
    cout << p[0] << ' ' << p[1] << ' ' << p[2] << ' ' << p[3] << '\n';
    cout << es[0].first << ' ' << es[0].second << ' '
         << es[1].first << ' ' << es[1].second << ' '
         << es[2].first << ' ' << es[2].second << '\n';
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    Tree source{n, {}}, target{n, {}};
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        cin >> a >> b;
        source.edges.insert(edge(a, b));
    }
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        cin >> a >> b;
        target.edges.insert(edge(a, b));
    }

    if (source.edges == target.edges) {
        cout << "YES\n0\n";
        return 0;
    }

    auto starCenter = [&](const Tree &tree) {
        for (int v = 1; v <= n; ++v) {
            if ((int)tree.neighbors(v).size() == n - 1) return v;
        }
        return -1;
    };

    int sourceStar = starCenter(source);
    if (sourceStar != -1) {
        cout << "NO\n";
        return 0;
    }

    vector<Operation> sourceOps, targetOps;
    int targetStar = starCenter(target);
    if (targetStar != -1) {
        int c = targetStar;
        sourceOps = reduceToNearStar(source, c);
        Edge last;
        for (Edge e : source.edges) {
            if (e.first != c && e.second != c) last = e;
        }
        int a = last.first, b = last.second, q = -1;
        if (!source.has(c, a)) swap(a, b);
        for (int v = 1; v <= n; ++v) {
            if (v != c && v != a && v != b) {
                q = v;
                break;
            }
        }
        sourceOps.push_back(source.apply(
            {q, c, a, b},
            {edge(q, c), edge(c, a), edge(c, b)},
            {0, 0, 0, 0}
        ));
    } else {
        sourceOps = normalize(source, 1, 2, 3);
        targetOps = normalize(target, 1, 2, 3);
    }

    cout << "YES\n" << sourceOps.size() + targetOps.size() << '\n';
    for (const Operation &op : sourceOps) printOperation(op, false);
    for (int i = (int)targetOps.size() - 1; i >= 0; --i) {
        printOperation(targetOps[i], true);
    }
    return 0;
}

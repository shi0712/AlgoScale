#include <algorithm>
#include <array>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

using namespace std;

using Edge = pair<int, int>;
using Path = array<int, 4>;
using Edges3 = array<Edge, 3>;

struct Operation {
    Path path;
    Edges3 edges;
    int kind = 0;
};

struct Tree {
    int n;
    bool adj[105][105]{};

    explicit Tree(int size = 0) : n(size) {}

    int degree(int v) const {
        int result = 0;
        for (int u = 1; u <= n; ++u) result += adj[v][u];
        return result;
    }

    bool hasPath(const Path &p) const {
        return adj[p[0]][p[1]] && adj[p[1]][p[2]] && adj[p[2]][p[3]];
    }

    void apply(const Path &p, const Edges3 &edges) {
        for (int i = 0; i < 3; ++i) {
            adj[p[i]][p[i + 1]] = false;
            adj[p[i + 1]][p[i]] = false;
        }
        for (auto [a, b] : edges) {
            adj[a][b] = true;
            adj[b][a] = true;
        }
    }

    set<Edge> edgeSet() const {
        set<Edge> result;
        for (int a = 1; a <= n; ++a) {
            for (int b = a + 1; b <= n; ++b) {
                if (adj[a][b]) result.insert({a, b});
            }
        }
        return result;
    }
};

static Edges3 pathEdges(const Path &p) {
    return {Edge{p[0], p[1]}, Edge{p[1], p[2]}, Edge{p[2], p[3]}};
}

static bool centerPath(const Tree &tree, int center, Path &path) {
    for (int y = 1; y <= tree.n; ++y) {
        if (!tree.adj[center][y]) continue;
        for (int z = 1; z <= tree.n; ++z) {
            if (z == center || !tree.adj[y][z]) continue;
            for (int w = 1; w <= tree.n; ++w) {
                if (w == center || w == y || !tree.adj[z][w]) continue;
                path = {center, y, z, w};
                return true;
            }
        }
    }
    return false;
}

static vector<pair<int, int>> centerTails(const Tree &tree, int center) {
    vector<pair<int, int>> tails;
    for (int a = 1; a <= tree.n; ++a) {
        if (!tree.adj[center][a]) continue;
        for (int b = 1; b <= tree.n; ++b) {
            if (b != center && tree.adj[a][b]) tails.push_back({a, b});
        }
    }
    return tails;
}

static void addOperation(Tree &tree, vector<Operation> &operations,
                         const Path &path, const Edges3 &edges, int kind = 0) {
    tree.apply(path, edges);
    operations.push_back({path, edges, kind});
}

static vector<Operation> toStar(Tree &tree, int center) {
    vector<Operation> operations;
    Path path;
    while (centerPath(tree, center, path)) {
        auto [c, y, z, w] = path;
        addOperation(tree, operations, path,
                     {Edge{c, y}, Edge{c, z}, Edge{c, w}});
    }

    while (true) {
        auto tails = centerTails(tree, center);
        if (tails.empty()) break;
        int leaf = 0;
        for (int v = 1; v <= tree.n; ++v) {
            if (tree.adj[center][v] && tree.degree(v) == 1) {
                leaf = v;
                break;
            }
        }
        if (leaf != 0) {
            auto [a, b] = tails[0];
            addOperation(tree, operations, {leaf, center, a, b},
                         {Edge{center, leaf}, Edge{center, a}, Edge{center, b}});
        } else {
            auto [a, b] = tails[0];
            int d = 0;
            for (auto [other, tail] : tails) {
                if (other != a) {
                    d = other;
                    break;
                }
            }
            addOperation(tree, operations, {b, a, center, d},
                         {Edge{center, b}, Edge{center, a}, Edge{center, d}});
        }
    }
    return operations;
}

static vector<Operation> canonicalize(Tree &tree) {
    const int center = 1;
    vector<Operation> operations;
    Path path;

    while (centerPath(tree, center, path)) {
        auto [c, y, z, w] = path;
        addOperation(tree, operations, path,
                     {Edge{c, y}, Edge{c, z}, Edge{z, w}});
    }

    while (true) {
        auto tails = centerTails(tree, center);
        if (tails.size() <= 1) break;

        int leaf = 0;
        for (int v = 1; v <= tree.n; ++v) {
            if (tree.adj[center][v] && tree.degree(v) == 1) {
                leaf = v;
                break;
            }
        }

        if (leaf != 0) {
            auto [a, b] = tails[0];
            addOperation(tree, operations, {leaf, center, a, b},
                         {Edge{center, leaf}, Edge{center, a}, Edge{center, b}}, 1);
        } else {
            auto [a, b] = tails[0];
            int d = 0;
            for (auto [other, tail] : tails) {
                if (other != a) {
                    d = other;
                    break;
                }
            }
            addOperation(tree, operations, {b, a, center, d},
                         {Edge{center, b}, Edge{center, a}, Edge{center, d}}, 2);
        }
    }

    auto tails = centerTails(tree, center);
    int a = tails[0].first;
    int b = tails[0].second;

    auto swapIntermediate = [&](int leaf) {
        addOperation(tree, operations, {leaf, center, a, b},
                     {Edge{center, leaf}, Edge{leaf, b}, Edge{center, a}});
        a = leaf;
    };
    auto swapTail = [&](int leaf) {
        addOperation(tree, operations, {leaf, center, a, b},
                     {Edge{center, a}, Edge{a, leaf}, Edge{center, b}});
        b = leaf;
    };

    if (a != 2) {
        if (b == 2) {
            int leaf = 0;
            for (int v = 1; v <= tree.n; ++v) {
                if (tree.adj[center][v] && tree.degree(v) == 1) {
                    leaf = v;
                    break;
                }
            }
            swapTail(leaf);
        }
        swapIntermediate(2);
    }
    if (b != 3) swapTail(3);

    return operations;
}

static Path inducedPath(const Tree &tree, Path vertices) {
    sort(vertices.begin(), vertices.end());
    do {
        if (tree.hasPath(vertices)) return vertices;
    } while (next_permutation(vertices.begin(), vertices.end()));
    return {0, 0, 0, 0};
}

static void undoOperation(Tree &tree, const Operation &operation,
                          vector<Operation> &output) {
    const auto &p = operation.path;

    if (operation.kind == 0) {
        Path current = inducedPath(tree, p);
        addOperation(tree, output, current, pathEdges(p));
        return;
    }

    if (operation.kind == 2) {
        auto [b, a, c, d] = p;
        int e = 0;
        for (int v = 1; v <= tree.n; ++v) {
            if (v != c && tree.adj[d][v]) {
                e = v;
                break;
            }
        }
        addOperation(tree, output, {b, c, d, e},
                     {Edge{c, b}, Edge{b, d}, Edge{d, e}});
        addOperation(tree, output, {a, c, b, d},
                     {Edge{c, a}, Edge{a, b}, Edge{c, d}});
        return;
    }

    auto [leaf, c, a, b] = p;
    int tail = 0;
    for (int v = 1; v <= tree.n; ++v) {
        if (v != c && tree.adj[a][v]) {
            tail = v;
            break;
        }
    }
    if (tail != 0) {
        addOperation(tree, output, {b, c, a, tail},
                     {Edge{a, b}, Edge{c, a}, Edge{a, tail}});
        return;
    }

    int intermediate = 0;
    tail = 0;
    for (int v = 1; v <= tree.n && intermediate == 0; ++v) {
        if (!tree.adj[c][v]) continue;
        for (int u = 1; u <= tree.n; ++u) {
            if (u != c && tree.adj[v][u]) {
                intermediate = v;
                tail = u;
                break;
            }
        }
    }
    addOperation(tree, output, {a, c, intermediate, tail},
                 {Edge{intermediate, tail}, Edge{c, a}, Edge{a, intermediate}});
    addOperation(tree, output, {b, c, a, intermediate},
                 {Edge{a, b}, Edge{c, a}, Edge{c, intermediate}});
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    Tree source(n), target(n);
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        cin >> a >> b;
        source.adj[a][b] = source.adj[b][a] = true;
    }
    for (int i = 0; i < n - 1; ++i) {
        int a, b;
        cin >> a >> b;
        target.adj[a][b] = target.adj[b][a] = true;
    }

    if (source.edgeSet() == target.edgeSet()) {
        cout << "YES\n0\n";
        return 0;
    }

    int sourceCenter = 0, targetCenter = 0;
    for (int v = 1; v <= n; ++v) {
        if (source.degree(v) == n - 1) sourceCenter = v;
        if (target.degree(v) == n - 1) targetCenter = v;
    }
    if (sourceCenter != 0) {
        cout << "NO\n";
        return 0;
    }

    vector<Operation> answer;
    if (targetCenter != 0) {
        answer = toStar(source, targetCenter);
    } else {
        Tree work = source;
        auto forward = canonicalize(work);
        answer.insert(answer.end(), forward.begin(), forward.end());
        source = work;

        work = target;
        auto backward = canonicalize(work);
        for (auto it = backward.rbegin(); it != backward.rend(); ++it) {
            undoOperation(source, *it, answer);
        }
    }

    cout << "YES\n" << answer.size() << '\n';
    for (const auto &operation : answer) {
        const auto &p = operation.path;
        cout << p[0] << ' ' << p[1] << ' ' << p[2] << ' ' << p[3] << '\n';
        for (int i = 0; i < 3; ++i) {
            if (i) cout << ' ';
            cout << operation.edges[i].first << ' ' << operation.edges[i].second;
        }
        cout << '\n';
    }
    return 0;
}

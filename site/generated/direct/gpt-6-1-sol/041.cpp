#include <bits/stdc++.h>
using namespace std;

struct Operation {
    array<int, 4> path;
    // Three replacement edges: (edges[0], edges[1]), etc.
    array<int, 6> edges;
};

using Graph = vector<vector<int>>;

struct FlattenResult {
    int a = -1, b = -1;
    vector<Operation> forward;
    // Each entry reverses one corresponding forward operation.
    vector<vector<Operation>> inverse;
};

static void applyOperation(Graph& g, const Operation& op) {
    for (int i = 0; i < 3; ++i) {
        int u = op.path[i], v = op.path[i + 1];
        g[u][v] = g[v][u] = 0;
    }
    for (int i = 0; i < 6; i += 2) {
        int u = op.edges[i], v = op.edges[i + 1];
        g[u][v] = g[v][u] = 1;
    }
}

// Turns a non-star tree into:
// r-a-b, with every other vertex adjacent to r.
// Also records valid operations for reversing the transformation.
static FlattenResult flatten(Graph& g, int r) {
    int n = (int)g.size() - 1;
    FlattenResult result;

    auto addStep = [&](const Operation& op,
                       vector<Operation> inverseOperations) {
        applyOperation(g, op);
        result.forward.push_back(op);
        result.inverse.push_back(move(inverseOperations));
    };

    int degree = 0;
    for (int v = 1; v <= n; ++v) degree += g[r][v];

    int a = -1, b = -1;

    if (degree == 1) {
        int u = -1;
        for (int v = 1; v <= n; ++v) {
            if (g[r][v]) u = v;
        }

        // Since the tree is not a star, there is a path r-u-v-w.
        int vFound = -1, wFound = -1;
        for (int v = 1; v <= n && vFound == -1; ++v) {
            if (v == r || !g[u][v]) continue;
            for (int w = 1; w <= n; ++w) {
                if (w != u && g[v][w]) {
                    vFound = v;
                    wFound = w;
                    break;
                }
            }
        }

        int v = vFound, w = wFound;

        // r-u-v-w -> u-r-v-w.
        addStep(
            Operation{{r, u, v, w}, {r, u, r, v, v, w}},
            {Operation{{u, r, v, w}, {r, u, u, v, v, w}}}
        );

        a = v;
        b = w;
    } else {
        // Find a length-two path starting at r.
        for (int u = 1; u <= n && a == -1; ++u) {
            if (!g[r][u]) continue;
            for (int v = 1; v <= n; ++v) {
                if (v != r && g[u][v]) {
                    a = u;
                    b = v;
                    break;
                }
            }
        }
    }

    result.a = a;
    result.b = b;

    while (true) {
        int u = -1, v = -1;

        // Find a depth-two vertex other than the protected vertex b.
        for (int x = 1; x <= n && u == -1; ++x) {
            if (!g[r][x]) continue;
            for (int y = 1; y <= n; ++y) {
                if (y != r && y != b && g[x][y]) {
                    u = x;
                    v = y;
                    break;
                }
            }
        }

        if (u != -1) {
            int c = -1;
            for (int x = 1; x <= n; ++x) {
                if (x != u && g[r][x]) {
                    c = x;
                    break;
                }
            }

            // Replace u-v by r-v using the path v-u-r-c.
            Operation forward{
                {v, u, r, c},
                {r, v, r, u, r, c}
            };

            if (u == a) {
                // The protected handle allows a one-operation inverse.
                addStep(
                    forward,
                    {Operation{{b, a, r, v}, {b, a, a, r, a, v}}}
                );
            } else {
                // Two operations restore u-v while preserving r-a-b.
                addStep(
                    forward,
                    {
                        Operation{{b, a, r, v}, {b, a, a, v, v, r}},
                        Operation{{a, v, r, u}, {a, r, r, u, u, v}}
                    }
                );
            }
            continue;
        }

        // No other depth-two vertex remains. Move a child of b to r.
        v = -1;
        for (int x = 1; x <= n; ++x) {
            if (x != a && g[b][x]) {
                v = x;
                break;
            }
        }

        if (v != -1) {
            addStep(
                Operation{{v, b, a, r}, {r, v, b, a, a, r}},
                {Operation{{v, r, a, b}, {v, b, b, a, a, r}}}
            );
            continue;
        }

        break;
    }

    return result;
}

static int starCenter(const Graph& g) {
    int n = (int)g.size() - 1;
    for (int u = 1; u <= n; ++u) {
        int degree = 0;
        for (int v = 1; v <= n; ++v) degree += g[u][v];
        if (degree == n - 1) return u;
    }
    return -1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    Graph s(n + 1, vector<int>(n + 1));
    Graph t(n + 1, vector<int>(n + 1));

    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        s[u][v] = s[v][u] = 1;
    }
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        t[u][v] = t[v][u] = 1;
    }

    if (s == t) {
        cout << "YES\n0\n";
        return 0;
    }

    // A star has no path on four distinct vertices.
    if (starCenter(s) != -1) {
        cout << "NO\n";
        return 0;
    }

    vector<Operation> answer;

    auto emit = [&](const Operation& op) {
        applyOperation(s, op);
        answer.push_back(op);
    };

    int targetCenter = starCenter(t);

    if (targetCenter != -1) {
        // Flatten around the desired star center, then remove the handle.
        int r = targetCenter;
        FlattenResult fs = flatten(s, r);
        answer = fs.forward;

        int a = fs.a, b = fs.b;
        int c = -1;
        for (int v = 1; v <= n; ++v) {
            if (v != a && s[r][v]) {
                c = v;
                break;
            }
        }

        emit(Operation{{b, a, r, c}, {r, b, r, a, r, c}});
    } else {
        const int r = 1;

        FlattenResult fs = flatten(s, r);
        FlattenResult ft = flatten(t, r);
        answer = fs.forward;

        int a = fs.a, b = fs.b;
        int c = ft.a, d = ft.b;

        // Change the handle r-a-b into r-c-d in at most two operations.
        if (c == b) {
            int e = -1;
            for (int v = 1; v <= n; ++v) {
                if (v != a && s[r][v]) {
                    e = v;
                    break;
                }
            }

            // Swap the two handle vertices.
            emit(Operation{{b, a, r, e}, {a, b, b, r, r, e}});
            swap(a, b);
        }

        if (a != c) {
            // Change the handle's parent, retaining its child.
            emit(Operation{{b, a, r, c}, {b, c, c, r, r, a}});
            a = c;
        }

        if (b != d) {
            // Change the handle's child, retaining its parent.
            emit(Operation{{b, a, r, d}, {b, r, r, a, a, d}});
            b = d;
        }

        // Undo the target's flattening, using its explicitly valid inverses.
        for (int i = (int)ft.inverse.size() - 1; i >= 0; --i) {
            for (const Operation& op : ft.inverse[i]) {
                emit(op);
            }
        }
    }

    cout << "YES\n" << answer.size() << '\n';
    for (const Operation& op : answer) {
        for (int i = 0; i < 4; ++i) {
            cout << op.path[i] << (i == 3 ? '\n' : ' ');
        }
        for (int i = 0; i < 6; ++i) {
            cout << op.edges[i] << (i == 5 ? '\n' : ' ');
        }
    }

    return 0;
}

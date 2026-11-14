#include <bits/stdc++.h>
using namespace std;

struct Tree {
    int n;
    vector<set<int>> g;

    Tree() {}
    explicit Tree(int n_) : n(n_), g(n_ + 1) {}

    void addEdge(int u, int v) {
        g[u].insert(v);
        g[v].insert(u);
    }

    void removeEdge(int u, int v) {
        g[u].erase(v);
        g[v].erase(u);
    }

    bool hasEdge(int u, int v) const {
        return g[u].count(v);
    }

    int degree(int v) const {
        return (int)g[v].size();
    }
};

struct Op {
    array<int, 4> p{};
    array<pair<int, int>, 3> e{};
};

bool sameTree(const Tree& a, const Tree& b) {
    for (int u = 1; u <= a.n; u++) {
        for (int v = u + 1; v <= a.n; v++) {
            if (a.hasEdge(u, v) != b.hasEdge(u, v)) return false;
        }
    }
    return true;
}

bool isStar(const Tree& t) {
    for (int v = 1; v <= t.n; v++) {
        if (t.degree(v) == t.n - 1) return true;
    }
    return false;
}

int starCenter(const Tree& t) {
    for (int v = 1; v <= t.n; v++) {
        if (t.degree(v) == t.n - 1) return v;
    }
    return -1;
}

array<pair<int, int>, 3> pathEdges(const array<int, 4>& p) {
    array<pair<int, int>, 3> e;
    e[0] = {p[0], p[1]};
    e[1] = {p[1], p[2]};
    e[2] = {p[2], p[3]};
    return e;
}

void applyOp(Tree& t, const Op& o, vector<Op>& log) {
    for (int i = 0; i < 3; i++) {
        t.removeEdge(o.p[i], o.p[i + 1]);
    }
    for (auto [u, v] : o.e) {
        t.addEdge(u, v);
    }
    log.push_back(o);
}

// Makes all but 'stop' vertices adjacent to c.
// The input tree is assumed not to be a star.
void flatten(Tree& t, int c, int stop, vector<Op>& ops) {
    // If c is a leaf, first give it two neighbors.
    if (t.degree(c) == 1) {
        int a = *t.g[c].begin();
        int b = -1, d = -1;

        for (int bb : t.g[a]) {
            if (bb == c) continue;
            for (int dd : t.g[bb]) {
                if (dd != a) {
                    b = bb;
                    d = dd;
                    break;
                }
            }
            if (b != -1) break;
        }

        assert(b != -1); // Exists because the tree is not a star.

        // c-a-b-d  ->  a-c-b-d
        Op o;
        o.p = {c, a, b, d};
        o.e[0] = {c, a};
        o.e[1] = {c, b};
        o.e[2] = {b, d};
        applyOp(t, o, ops);
    }

    while (true) {
        int k = 0;
        for (int v = 1; v <= t.n; v++) {
            if (v != c && !t.hasEdge(c, v)) k++;
        }
        if (k <= stop) break;

        int a = -1, b = -1;
        for (int aa : t.g[c]) {
            for (int bb : t.g[aa]) {
                if (bb != c) {
                    a = aa;
                    b = bb;
                    break;
                }
            }
            if (a != -1) break;
        }

        int p = -1;
        for (int pp : t.g[c]) {
            if (pp != a) {
                p = pp;
                break;
            }
        }

        assert(a != -1 && p != -1);

        // p-c-a-b  ->  local star centered at c.
        // Exactly one additional vertex, namely b, becomes adjacent to c.
        Op o;
        o.p = {p, c, a, b};
        o.e[0] = {p, c};
        o.e[1] = {c, a};
        o.e[2] = {c, b};
        applyOp(t, o, ops);
    }
}

// Transform a non-star tree into:
// 1-v for all v != 1,3, together with edge 2-3.
void canonicalize(Tree& t, vector<Op>& ops) {
    const int c = 1;
    const int wantedParent = 2;
    const int wantedLeaf = 3;

    flatten(t, c, 1, ops);

    // There is exactly one vertex not adjacent to c.
    int leaf = -1;
    for (int v = 1; v <= t.n; v++) {
        if (v != c && !t.hasEdge(c, v)) {
            leaf = v;
            break;
        }
    }
    assert(leaf != -1);

    int parent = *t.g[leaf].begin();

    // First make wantedLeaf the unique vertex not adjacent to c.
    if (leaf != wantedLeaf) {
        int p, newLeaf;

        if (parent == wantedLeaf) {
            newLeaf = parent;
            p = -1;
            for (int v : t.g[c]) {
                if (v != parent) {
                    p = v;
                    break;
                }
            }
        } else {
            p = wantedLeaf;
            newLeaf = p;
        }

        int newParent = (parent != newLeaf ? parent : leaf);
        int q = -1;
        for (int v : {leaf, parent, p}) {
            if (v != newLeaf && v != newParent) q = v;
        }

        // leaf-parent-c-p is a path. Rewire it into
        // newLeaf-newParent-c-q.
        Op o;
        o.p = {leaf, parent, c, p};
        o.e[0] = {newLeaf, newParent};
        o.e[1] = {newParent, c};
        o.e[2] = {c, q};
        applyOp(t, o, ops);

        leaf = newLeaf;
        parent = newParent;
    }

    // Now leaf == 3. Make its parent equal to 2.
    if (parent != wantedParent) {
        int p = wantedParent;

        Op o;
        o.p = {leaf, parent, c, p};
        o.e[0] = {leaf, p};
        o.e[1] = {p, c};
        o.e[2] = {c, parent};
        applyOp(t, o, ops);
    }
}

// Undo one recorded operation. The current tree must be exactly the state
// immediately after that operation, and must not be a star.
void invertOp(Tree& t, const Op& o, vector<Op>& out) {
    vector<char> inA(t.n + 1, false);
    for (int v : o.p) inA[v] = true;

    vector<vector<int>> local(t.n + 1);
    for (auto [u, v] : o.e) {
        local[u].push_back(v);
        local[v].push_back(u);
    }

    int starCenterLocal = -1;
    for (int v : o.p) {
        if (local[v].size() == 3) starCenterLocal = v;
    }

    auto oldEdges = pathEdges(o.p);

    // If the new local tree is a path, undo directly.
    if (starCenterLocal == -1) {
        int start = -1;
        for (int v : o.p) {
            if (local[v].size() == 1) {
                start = v;
                break;
            }
        }

        array<int, 4> order{};
        int prev = -1, cur = start;
        for (int i = 0; i < 4; i++) {
            order[i] = cur;
            int nxt = -1;
            for (int v : local[cur]) {
                if (v != prev) {
                    nxt = v;
                    break;
                }
            }
            prev = cur;
            cur = nxt;
        }

        Op inv;
        inv.p = order;
        inv.e = oldEdges;
        applyOp(t, inv, out);
        return;
    }

    // Otherwise the local tree is a star.
    int u = starCenterLocal;
    vector<int> leaves;
    for (int v : o.p) {
        if (v != u) leaves.push_back(v);
    }

    // Case 1: some local leaf has an external neighbor q.
    int a = -1, q = -1;
    for (int v : leaves) {
        for (int z : t.g[v]) {
            if (!inA[z]) {
                a = v;
                q = z;
                break;
            }
        }
        if (a != -1) break;
    }

    if (a != -1) {
        vector<int> rem;
        for (int v : leaves) {
            if (v != a) rem.push_back(v);
        }
        int b = rem[0], d = rem[1];

        // q-a-u-b -> q-a-b-u, making a-b-u-d a local path.
        Op o1;
        o1.p = {q, a, u, b};
        o1.e[0] = {q, a};
        o1.e[1] = {a, b};
        o1.e[2] = {b, u};
        applyOp(t, o1, out);

        Op o2;
        o2.p = {a, b, u, d};
        o2.e = oldEdges;
        applyOp(t, o2, out);
        return;
    }

    // Case 2: only u has external branches. Since the whole tree is not
    // a star, there is an external path r-q-u.
    q = -1;
    int r = -1;
    for (int qq : t.g[u]) {
        if (inA[qq]) continue;
        for (int rr : t.g[qq]) {
            if (rr != u && !inA[rr]) {
                q = qq;
                r = rr;
                break;
            }
        }
        if (q != -1) break;
    }
    assert(q != -1);

    a = leaves[0];
    int b = leaves[1], d = leaves[2];

    Op o1;
    o1.p = {r, q, u, a};
    o1.e[0] = {r, q};
    o1.e[1] = {q, a};
    o1.e[2] = {a, u};
    applyOp(t, o1, out);

    Op o2;
    o2.p = {q, a, u, b};
    o2.e[0] = {q, u};
    o2.e[1] = {a, b};
    o2.e[2] = {b, u};
    applyOp(t, o2, out);

    Op o3;
    o3.p = {a, b, u, d};
    o3.e = oldEdges;
    applyOp(t, o3, out);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    Tree S(n), T(n);
    for (int i = 0, u, v; i < n - 1; i++) {
        cin >> u >> v;
        S.addEdge(u, v);
    }
    for (int i = 0, u, v; i < n - 1; i++) {
        cin >> u >> v;
        T.addEdge(u, v);
    }

    if (sameTree(S, T)) {
        cout << "YES\n0\n";
        return 0;
    }

    // A star has no path on four distinct vertices, so it is stuck.
    if (isStar(S)) {
        cout << "NO\n";
        return 0;
    }

    vector<Op> answer;
    Tree cur = S;

    if (isStar(T)) {
        int center = starCenter(T);
        flatten(cur, center, 0, answer);
    } else {
        // S -> C.
        canonicalize(cur, answer);

        // Compute T -> C separately.
        Tree targetWork = T;
        vector<Op> targetOps;
        canonicalize(targetWork, targetOps);

        // Reverse T -> C, obtaining C -> T.
        for (int i = (int)targetOps.size() - 1; i >= 0; i--) {
            invertOp(cur, targetOps[i], answer);
        }
    }

    assert(sameTree(cur, T));
    assert(answer.size() <= 400);

    cout << "YES\n" << answer.size() << '\n';
    for (const Op& o : answer) {
        cout << o.p[0] << ' ' << o.p[1] << ' '
             << o.p[2] << ' ' << o.p[3] << '\n';
        cout << o.e[0].first << ' ' << o.e[0].second << ' '
             << o.e[1].first << ' ' << o.e[1].second << ' '
             << o.e[2].first << ' ' << o.e[2].second << '\n';
    }

    return 0;
}

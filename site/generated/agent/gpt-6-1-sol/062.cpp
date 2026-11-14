#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;
    explicit DSU(int n) : p(n), sz(n, 1) { iota(p.begin(), p.end(), 0); }
    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return true;
    }
};

struct Edge {
    int a, b, c, d;
};

struct RootedTree {
    vector<int> parent, depth, parentEdge;
    RootedTree(const vector<vector<pair<int, int>>>& adj)
        : parent(adj.size(), -1), depth(adj.size()), parentEdge(adj.size(), -1) {
        vector<int> order{0};
        for (size_t k = 0; k < order.size(); ++k) {
            int u = order[k];
            for (auto [v, id] : adj[u]) {
                if (v == parent[u]) continue;
                parent[v] = u;
                parentEdge[v] = id;
                depth[v] = depth[u] + 1;
                order.push_back(v);
            }
        }
    }

    template<class F>
    void path(int u, int v, F visit) const {
        while (u != v) {
            if (depth[u] < depth[v]) swap(u, v);
            visit(parentEdge[u]);
            u = parent[u];
        }
    }
};

bool solve(int n, const vector<Edge>& edges) {
    int m = (int)edges.size();
    DSU da(n), db(n);
    vector<int> nonTree;
    vector<vector<pair<int, int>>> ta(n), tb(n);
    bool commonTree = true;
    for (int i = 0; i < m; ++i) {
        const auto& e = edges[i];
        if (da.unite(e.a, e.b)) {
            ta[e.a].emplace_back(e.b, i);
            ta[e.b].emplace_back(e.a, i);
            tb[e.c].emplace_back(e.d, i);
            tb[e.d].emplace_back(e.c, i);
            if (!db.unite(e.c, e.d)) commonTree = false;
        } else {
            nonTree.push_back(i);
        }
    }
    if (!commonTree) return true;
    if (nonTree.empty()) return false;

    RootedTree a(ta), b(tb);

    // Relative to the common tree, binary representations are [I | X]
    // and [I | Y]. X[t,e] is 1 exactly when t is on e's tree path in A.
    // Connect the rows and columns linked by a 1 in X.
    DSU components(m);
    for (int id : nonTree) {
        const auto& e = edges[id];
        a.path(e.a, e.b, [&](int t) { components.unite(t, id); });
    }
    vector<int> component(m);
    for (int i = 0; i < m; ++i) component[i] = components.find(i);

    vector<vector<int>> graph(m);
    vector<int> indegree(m), marked(m, -1);
    // Deduplicate arcs from one source component for each non-tree edge.
    vector<int> lastSource(m, -1);
    for (int id : nonTree) {
        const auto& e = edges[id];
        int lengthA = 0, common = 0;
        a.path(e.a, e.b, [&](int t) {
            marked[t] = id;
            ++lengthA;
        });
        bool bad = false;
        int target = component[id];
        b.path(e.c, e.d, [&](int t) {
            if (marked[t] == id) {
                ++common;
            } else {
                int source = component[t];
                if (source == target) {
                    bad = true;
                } else if (lastSource[source] != id) {
                    lastSource[source] = id;
                    graph[source].push_back(target);
                    ++indegree[target];
                }
            }
        });
        // An X entry missing from Y immediately gives a failing one-edge
        // exchange. An extra Y entry inside an X component also fails.
        if (common != lengthA || bad) return true;
    }

    // Extra Y entries must form an acyclic graph between X components.
    // In that case every nonsingular minor of X stays nonsingular in Y:
    // order components topologically to obtain block triangular minors.
    // Conversely, a shortest directed cycle containing an extra Y entry
    // gives a minor with one perfect matching in X and two in Y. Over
    // GF(2), that minor is nonsingular in X and singular in Y.
    vector<int> queue;
    queue.reserve(m);
    for (int i = 0; i < m; ++i) {
        if (indegree[i] == 0) queue.push_back(i);
    }
    for (size_t k = 0; k < queue.size(); ++k) {
        for (int v : graph[queue[k]]) {
            if (--indegree[v] == 0) queue.push_back(v);
        }
    }
    return (int)queue.size() != m;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int tests;
    cin >> tests;
    while (tests--) {
        int n, m;
        cin >> n >> m;
        vector<Edge> edges(m);
        for (auto& e : edges) {
            cin >> e.a >> e.b >> e.c >> e.d;
            --e.a; --e.b; --e.c; --e.d;
        }
        cout << (solve(n, edges) ? "YES\n" : "NO\n");
    }
    return 0;
}

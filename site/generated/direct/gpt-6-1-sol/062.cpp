#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;

    explicit DSU(int n) : p(n), sz(n, 1) {
        iota(p.begin(), p.end(), 0);
    }

    int find(int x) {
        while (x != p[x]) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return true;
    }
};

struct Edge {
    int ua, va, ub, vb;
};

struct RootedTree {
    vector<int> parent, depth, parentEdge;

    explicit RootedTree(const vector<vector<pair<int, int>>>& adj) {
        int n = (int)adj.size();
        parent.assign(n, -1);
        depth.assign(n, 0);
        parentEdge.assign(n, -1);

        vector<int> order;
        order.reserve(n);
        order.push_back(0);
        parent[0] = 0;

        for (int i = 0; i < (int)order.size(); ++i) {
            int u = order[i];
            for (auto [v, id] : adj[u]) {
                if (v == parent[u]) continue;
                parent[v] = u;
                parentEdge[v] = id;
                depth[v] = depth[u] + 1;
                order.push_back(v);
            }
        }
    }
};

bool solveCase(int n, int m, const vector<Edge>& edges) {
    // Choose a spanning tree T of A.
    DSU dsuA(n);
    vector<char> inTree(m, false);
    vector<int> treeRow(m, -1), nonTree;
    int treeSize = 0;

    for (int i = 0; i < m; ++i) {
        if (dsuA.unite(edges[i].ua, edges[i].va)) {
            inTree[i] = true;
            treeRow[i] = treeSize++;
        } else {
            nonTree.push_back(i);
        }
    }

    // If T is not a spanning tree of B, it is already a witness.
    DSU dsuB(n);
    for (int i = 0; i < m; ++i) {
        if (inTree[i] && !dsuB.unite(edges[i].ub, edges[i].vb)) {
            return true;
        }
    }

    int q = (int)nonTree.size();
    if (q == 0) return false;

    vector<vector<pair<int, int>>> adjA(n), adjB(n);
    for (int i = 0; i < m; ++i) {
        if (!inTree[i]) continue;

        const auto& e = edges[i];
        adjA[e.ua].push_back({e.va, i});
        adjA[e.va].push_back({e.ua, i});
        adjB[e.ub].push_back({e.vb, i});
        adjB[e.vb].push_back({e.ub, i});
    }

    RootedTree ta(adjA), tb(adjB);

    // XA[row][column] is stored column by column as bitsets.
    int wordsA = (n - 1 + 63) / 64;
    vector<uint64_t> matrixA((size_t)q * wordsA, 0);
    vector<int> pathLengthA(q, 0);

    // Components of the undirected bipartite graph of the 1-entries of XA.
    // Its nodes are exactly the m edge indices:
    // tree edges are row nodes, non-tree edges are column nodes.
    DSU components(m);

    for (int col = 0; col < q; ++col) {
        int id = nonTree[col];
        int u = edges[id].ua;
        int v = edges[id].va;
        size_t base = (size_t)col * wordsA;

        while (u != v) {
            if (ta.depth[u] < ta.depth[v]) swap(u, v);

            int treeEdge = ta.parentEdge[u];
            int row = treeRow[treeEdge];

            matrixA[base + row / 64] |= 1ULL << (row % 64);
            ++pathLengthA[col];
            components.unite(id, treeEdge);

            u = ta.parent[u];
        }
    }

    vector<int> rootId(m, -1), component(m);
    int componentCount = 0;

    for (int i = 0; i < m; ++i) {
        int root = components.find(i);
        if (rootId[root] == -1) rootId[root] = componentCount++;
        component[i] = rootId[root];
    }

    // Directed graph of extra 1-entries of XB, after contracting
    // the connected components of XA.
    int wordsGraph = (componentCount + 63) / 64;
    vector<uint64_t> graph((size_t)componentCount * wordsGraph, 0);
    vector<int> indegree(componentCount, 0);

    for (int col = 0; col < q; ++col) {
        int id = nonTree[col];
        int u = edges[id].ub;
        int v = edges[id].vb;
        int commonEntries = 0;
        size_t base = (size_t)col * wordsA;

        while (u != v) {
            if (tb.depth[u] < tb.depth[v]) swap(u, v);

            int treeEdge = tb.parentEdge[u];
            int row = treeRow[treeEdge];

            bool inA = (matrixA[base + row / 64] >> (row % 64)) & 1ULL;

            if (inA) {
                ++commonEntries;
            } else {
                int from = component[treeEdge];
                int to = component[id];

                if (from == to) return true;

                size_t pos = (size_t)from * wordsGraph + to / 64;
                uint64_t mask = 1ULL << (to % 64);

                if (!(graph[pos] & mask)) {
                    graph[pos] |= mask;
                    ++indegree[to];
                }
            }

            u = tb.parent[u];
        }

        // Some 1-entry of XA is absent from XB.
        if (commonEntries != pathLengthA[col]) return true;
    }

    // A directed cycle means that a desired spanning tree exists.
    vector<int> queue;
    queue.reserve(componentCount);

    for (int i = 0; i < componentCount; ++i) {
        if (indegree[i] == 0) queue.push_back(i);
    }

    for (int head = 0; head < (int)queue.size(); ++head) {
        int u = queue[head];
        size_t base = (size_t)u * wordsGraph;

        for (int w = 0; w < wordsGraph; ++w) {
            uint64_t bits = graph[base + w];

            while (bits) {
                int bit = countr_zero(bits);
                int v = w * 64 + bit;
                bits &= bits - 1;

                if (--indegree[v] == 0) queue.push_back(v);
            }
        }
    }

    return (int)queue.size() != componentCount;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;

    while (t--) {
        int n, m;
        cin >> n >> m;

        vector<Edge> edges(m);
        for (auto& e : edges) {
            cin >> e.ua >> e.va >> e.ub >> e.vb;
            --e.ua;
            --e.va;
            --e.ub;
            --e.vb;
        }

        cout << (solveCase(n, m, edges) ? "YES\n" : "NO\n");
    }

    return 0;
}

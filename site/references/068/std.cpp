#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;

    DSU(int n = 0) { init(n); }

    void init(int n) {
        p.resize(n);
        iota(p.begin(), p.end(), 0);
        sz.assign(n, 1);
    }

    int find(int x) {
        while (p[x] != x) {
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
    int a, b, c, d;
};

struct TreeInfo {
    int n, m, LOG, timer = 0;
    vector<vector<pair<int, int>>> adj;
    vector<int> par, dep, pe, tin, tout, child;
    vector<vector<int>> up;

    TreeInfo() {}

    TreeInfo(
        int n_,
        int m_,
        const vector<Edge>& E,
        const vector<int>& T,
        bool useB
    ) : n(n_), m(m_) {
        LOG = 1;
        while ((1 << LOG) <= n) ++LOG;

        adj.assign(n, {});
        for (int id : T) {
            int u = useB ? E[id].c : E[id].a;
            int v = useB ? E[id].d : E[id].b;
            adj[u].push_back({v, id});
            adj[v].push_back({u, id});
        }

        par.assign(n, -1);
        dep.assign(n, 0);
        pe.assign(n, -1);
        tin.assign(n, 0);
        tout.assign(n, 0);
        child.assign(m, -1);

        dfs0(0, 0, -1);

        up.assign(LOG, vector<int>(n));
        for (int v = 0; v < n; ++v) up[0][v] = par[v];

        for (int j = 1; j < LOG; ++j)
            for (int v = 0; v < n; ++v)
                up[j][v] = up[j - 1][up[j - 1][v]];
    }

    void dfs0(int u, int p, int eid) {
        par[u] = p;
        pe[u] = eid;
        tin[u] = timer++;

        if (eid != -1) child[eid] = u;

        for (auto [v, id] : adj[u]) {
            if (v == p) continue;
            dep[v] = dep[u] + 1;
            dfs0(v, u, id);
        }

        tout[u] = timer - 1;
    }

    int lca(int a, int b) const {
        if (dep[a] < dep[b]) swap(a, b);

        int d = dep[a] - dep[b];
        for (int j = 0; j < LOG; ++j)
            if ((d >> j) & 1)
                a = up[j][a];

        if (a == b) return a;

        for (int j = LOG - 1; j >= 0; --j) {
            if (up[j][a] != up[j][b]) {
                a = up[j][a];
                b = up[j][b];
            }
        }

        return par[a];
    }
};

struct Fenwick {
    int n;
    vector<int> bit;

    Fenwick(int n_ = 0) : n(n_), bit(n + 2, 0) {}

    void add(int x, int v) {
        for (++x; x <= n + 1; x += x & -x)
            bit[x] += v;
    }

    void rangeAdd(int l, int r, int v) {
        add(l, v);
        if (r + 1 < n) add(r + 1, -v);
    }

    int point(int x) const {
        int s = 0;
        for (++x; x > 0; x -= x & -x)
            s += bit[x];
        return s;
    }
};

/*
 * Vertex-biconnected edge blocks of graph A.
 * Every bridge forms a one-edge block.
 */
struct BCC {
    int n, m, timer = 0;
    const vector<Edge>& E;

    vector<vector<pair<int, int>>> adj;
    vector<int> tin, low, stk;
    vector<int> comp, compSize;

    BCC(int n_, const vector<Edge>& E_)
        : n(n_), m((int)E_.size()), E(E_) {

        adj.assign(n, {});
        for (int i = 0; i < m; ++i) {
            adj[E[i].a].push_back({E[i].b, i});
            adj[E[i].b].push_back({E[i].a, i});
        }

        tin.assign(n, 0);
        low.assign(n, 0);
        comp.assign(m, -1);
        stk.reserve(m);

        for (int i = 0; i < n; ++i)
            if (!tin[i])
                dfs(i, -1);
    }

    void dfs(int u, int parentEdge) {
        tin[u] = low[u] = ++timer;

        for (auto [v, id] : adj[u]) {
            if (id == parentEdge) continue;

            if (!tin[v]) {
                stk.push_back(id);

                dfs(v, id);
                low[u] = min(low[u], low[v]);

                if (low[v] >= tin[u]) {
                    int cid = (int)compSize.size();
                    int cnt = 0;

                    while (true) {
                        int e = stk.back();
                        stk.pop_back();

                        comp[e] = cid;
                        ++cnt;

                        if (e == id) break;
                    }

                    compSize.push_back(cnt);
                }
            } else if (tin[v] < tin[u]) {
                stk.push_back(id);
                low[u] = min(low[u], tin[v]);
            }
        }
    }
};

struct SCC {
    const vector<vector<int>>& g;

    int timer = 0, sccCnt = 0;
    vector<int> dfn, low, stk, bel;
    vector<char> inStack;

    SCC(const vector<vector<int>>& g_)
        : g(g_) {
        int n = g.size();
        dfn.assign(n, 0);
        low.assign(n, 0);
        bel.assign(n, -1);
        inStack.assign(n, 0);
        stk.reserve(n);
    }

    void dfs(int u) {
        dfn[u] = low[u] = ++timer;
        stk.push_back(u);
        inStack[u] = 1;

        for (int v : g[u]) {
            if (!dfn[v]) {
                dfs(v);
                low[u] = min(low[u], low[v]);
            } else if (inStack[v]) {
                low[u] = min(low[u], dfn[v]);
            }
        }

        if (low[u] == dfn[u]) {
            while (true) {
                int v = stk.back();
                stk.pop_back();

                inStack[v] = 0;
                bel[v] = sccCnt;

                if (v == u) break;
            }
            ++sccCnt;
        }
    }

    vector<int> run() {
        for (int i = 0; i < (int)g.size(); ++i)
            if (!dfn[i])
                dfs(i);

        return bel;
    }
};

struct CrossQuery {
    int y, eid, coef;
};

struct ColorQuery {
    int color, eid, coef;
};

bool solveCase(int n, int m, const vector<Edge>& E) {
    /*
     * 1. Choose a spanning tree T of A.
     */
    DSU dsuA(n);
    vector<int> treeEdges;
    vector<char> isTree(m, 0);

    for (int i = 0; i < m; ++i) {
        if (dsuA.unite(E[i].a, E[i].b)) {
            treeEdges.push_back(i);
            isTree[i] = 1;
        }
    }

    /*
     * If the same set is not a spanning tree of B, immediate YES.
     */
    DSU dsuB(n);
    bool commonTree = true;

    for (int id : treeEdges) {
        if (!dsuB.unite(E[id].c, E[id].d)) {
            commonTree = false;
            break;
        }
    }

    if (commonTree) {
        int r = dsuB.find(0);
        for (int v = 1; v < n; ++v) {
            if (dsuB.find(v) != r) {
                commonTree = false;
                break;
            }
        }
    }

    if (!commonTree)
        return true;

    TreeInfo A(n, m, E, treeEdges, false);
    TreeInfo B(n, m, E, treeEdges, true);

    vector<int> chords;
    for (int i = 0; i < m; ++i)
        if (!isTree[i])
            chords.push_back(i);

    if (chords.empty())
        return false;

    /*
     * 2. Check P_A(e) subset P_B(e).
     *
     * We batch all F(x,y)=|R_A(x) intersect R_B(y)|
     * queries using a DFS on A and a Fenwick tree on B's Euler order.
     */
    vector<vector<CrossQuery>> crossQueries(n);
    vector<long long> intersection(m, 0);
    vector<int> lenA(m, 0);

    for (int e : chords) {
        int ua = E[e].a, va = E[e].b;
        int ub = E[e].c, vb = E[e].d;

        int la = A.lca(ua, va);
        int lb = B.lca(ub, vb);

        lenA[e] =
            A.dep[ua] + A.dep[va] - 2 * A.dep[la];

        int xs[3] = {ua, va, la};
        int cx[3] = {1, 1, -2};

        int ys[3] = {ub, vb, lb};
        int cy[3] = {1, 1, -2};

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                crossQueries[xs[i]].push_back({
                    ys[j],
                    e,
                    cx[i] * cy[j]
                });
            }
        }
    }

    Fenwick fw(n);

    function<void(int, int)> dfsCross =
        [&](int u, int p) {

        int eid = A.pe[u];

        if (eid != -1) {
            int childB = B.child[eid];
            fw.rangeAdd(
                B.tin[childB],
                B.tout[childB],
                +1
            );
        }

        for (auto q : crossQueries[u]) {
            intersection[q.eid] +=
                1LL * q.coef * fw.point(B.tin[q.y]);
        }

        for (auto [v, id] : A.adj[u]) {
            if (v != p)
                dfsCross(v, u);
        }

        if (eid != -1) {
            int childB = B.child[eid];
            fw.rangeAdd(
                B.tin[childB],
                B.tout[childB],
                -1
            );
        }
    };

    dfsCross(0, -1);

    for (int e : chords) {
        if (intersection[e] != lenA[e])
            return true;
    }

    /*
     * 3. Contract the undirected part of the official auxiliary graph.
     *
     * Its components are exactly the cyclic biconnected edge blocks of A.
     */
    BCC bcc(n, E);

    int blockCnt = bcc.compSize.size();

    vector<int> nodeOfBlock(blockCnt, -1);

    int cyclicBlocks = 0;
    for (int c = 0; c < blockCnt; ++c) {
        if (bcc.compSize[c] > 1)
            nodeOfBlock[c] = cyclicBlocks++;
    }

    /*
     * 4. Build the compressed path-to-block directed graph.
     */
    vector<vector<int>> g(cyclicBlocks);

    auto newNode = [&]() {
        g.emplace_back();
        return (int)g.size() - 1;
    };

    auto addEdge = [&](int u, int v) {
        g[u].push_back(v);
    };

    /*
     * down[v][j]:
     * the 2^j parent edges starting at v in T_B.
     */
    vector<vector<int>> down(
        n,
        vector<int>(B.LOG, -1)
    );

    /*
     * Leaves: one node per tree edge.
     */
    for (int v = 1; v < n; ++v) {
        down[v][0] = newNode();

        int eid = B.pe[v];
        int c = bcc.comp[eid];

        if (bcc.compSize[c] > 1) {
            addEdge(
                down[v][0],
                nodeOfBlock[c]
            );
        }
    }

    /*
     * Binary lifting interval DAG.
     */
    for (int j = 1; j < B.LOG; ++j) {
        int need = 1 << j;

        for (int v = 0; v < n; ++v) {
            if (B.dep[v] < need)
                continue;

            down[v][j] = newNode();

            int mid = B.up[j - 1][v];

            addEdge(
                down[v][j],
                down[v][j - 1]
            );

            addEdge(
                down[v][j],
                down[mid][j - 1]
            );
        }
    }

    auto addUpPath = [&](int src, int x, int anc) {
        int d = B.dep[x] - B.dep[anc];

        for (int j = B.LOG - 1; j >= 0; --j) {
            if ((d >> j) & 1) {
                addEdge(src, down[x][j]);
                x = B.up[j][x];
            }
        }
    };

    for (int e : chords) {
        int c = bcc.comp[e];

        // Every non-tree edge lies on a cycle.
        int src = nodeOfBlock[c];

        int u = E[e].c;
        int v = E[e].d;
        int l = B.lca(u, v);

        addUpPath(src, u, l);
        addUpPath(src, v, l);
    }

    /*
     * 5. SCC of the compressed graph.
     */
    SCC solver(g);
    vector<int> scc = solver.run();

    /*
     * 6. For each chord e, count tree edges on P_B(e)
     * whose block belongs to the same SCC as C(e).
     *
     * We can answer all such path-color queries by one DFS.
     */
    vector<vector<ColorQuery>> colorQueries(n);
    vector<long long> sameSccCount(m, 0);

    for (int e : chords) {
        int color =
            scc[nodeOfBlock[bcc.comp[e]]];

        int u = E[e].c;
        int v = E[e].d;
        int l = B.lca(u, v);

        colorQueries[u].push_back({
            color, e, +1
        });

        colorQueries[v].push_back({
            color, e, +1
        });

        colorQueries[l].push_back({
            color, e, -2
        });
    }

    vector<int> active(solver.sccCnt, 0);

    function<void(int, int)> dfsColor =
        [&](int u, int p) {

        int eid = B.pe[u];
        int color = -1;

        if (eid != -1) {
            int c = bcc.comp[eid];

            if (bcc.compSize[c] > 1) {
                color =
                    scc[nodeOfBlock[c]];
                ++active[color];
            }
        }

        for (auto q : colorQueries[u]) {
            sameSccCount[q.eid] +=
                1LL * q.coef * active[q.color];
        }

        for (auto [v, id] : B.adj[u]) {
            if (v != p)
                dfsColor(v, u);
        }

        if (color != -1)
            --active[color];
    };

    dfsColor(0, -1);

    /*
     * Exactly |P_A(e)| of them are the harmless
     * P_A(e) subset of P_B(e) edges.
     *
     * Any extra one is a genuine directed dependency
     * lying inside one SCC.
     */
    for (int e : chords) {
        if (sameSccCount[e] > lenA[e])
            return true;
    }

    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<Edge> E(m);

        for (auto& e : E) {
            cin >> e.a >> e.b >> e.c >> e.d;
            --e.a;
            --e.b;
            --e.c;
            --e.d;
        }

        cout << (solveCase(n, m, E) ? "YES" : "NO")
             << '\n';
    }
}

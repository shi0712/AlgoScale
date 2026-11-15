#include <bits/stdc++.h>
using namespace std;

struct Edge {
    int uA, vA, uB, vB;
};

struct LazyDSU {
    vector<int> p, r, mark;
    int stamp = 0;
    void init(int n) {
        p.resize(n);
        r.resize(n);
        mark.assign(n, 0);
        stamp = 0;
    }
    void reset() { ++stamp; }
    int find(int x) {
        if (mark[x] != stamp) {
            mark[x] = stamp;
            p[x] = x;
            r[x] = 0;
        }
        int root = x;
        while (p[root] != root) root = p[root];
        while (p[x] != x) {
            int y = p[x];
            p[x] = root;
            x = y;
        }
        return root;
    }
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (r[a] < r[b]) swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
        return true;
    }
};

static int NMAX = 10005;
static vector<int> markMapA, parStampB;
static vector<char> parityBv;

void getBridges(int n, const vector<Edge>& E, bool isA, vector<char>& isBridge) {
    int m = (int)E.size();
    isBridge.assign(m, 0);
    vector<vector<pair<int,int>>> adj(n);
    for (int i = 0; i < m; ++i) {
        int u = isA ? E[i].uA : E[i].uB;
        int v = isA ? E[i].vA : E[i].vB;
        if (u == v) continue;
        adj[u].push_back({v, i});
        adj[v].push_back({u, i});
    }
    vector<int> tin(n, -1), low(n, 0);
    int timer = 0;
    function<void(int,int)> dfs = [&](int v, int pe) {
        tin[v] = low[v] = timer++;
        for (auto [to, id] : adj[v]) {
            if (id == pe) continue;
            if (tin[to] != -1) {
                low[v] = min(low[v], tin[to]);
            } else {
                dfs(to, id);
                low[v] = min(low[v], low[to]);
                if (low[to] > tin[v]) isBridge[id] = 1;
            }
        }
    };
    for (int v = 0; v < n; ++v)
        if (tin[v] == -1) dfs(v, -1);
}

vector<vector<int>> getBiconnected(int n, const vector<Edge>& E) {
    int m = (int)E.size();
    vector<vector<pair<int,int>>> adj(n);
    for (int i = 0; i < m; ++i) {
        int u = E[i].uA, v = E[i].vA;
        if (u == v) continue;
        adj[u].push_back({v, i});
        adj[v].push_back({u, i});
    }
    vector<int> tin(n, -1), low(n, 0);
    vector<int> st;
    vector<vector<int>> comps;
    int timer = 0;
    function<void(int,int)> dfs = [&](int v, int pe) {
        tin[v] = low[v] = timer++;
        for (auto [to, id] : adj[v]) {
            if (id == pe) continue;
            if (tin[to] != -1) {
                if (tin[to] < tin[v]) st.push_back(id);
                low[v] = min(low[v], tin[to]);
            } else {
                st.push_back(id);
                dfs(to, id);
                low[v] = min(low[v], low[to]);
                if (low[to] >= tin[v]) {
                    vector<int> comp;
                    while (true) {
                        int e = st.back();
                        st.pop_back();
                        comp.push_back(e);
                        if (e == id) break;
                    }
                    comps.push_back(move(comp));
                }
            }
        }
    };
    for (int v = 0; v < n; ++v)
        if (tin[v] == -1) dfs(v, -1);
    return comps;
}

int rankSubset(const vector<Edge>& E, const vector<int>& C, bool isA, LazyDSU& dsu) {
    dsu.reset();
    int r = 0;
    for (int i : C) {
        int u = isA ? E[i].uA : E[i].uB;
        int v = isA ? E[i].vA : E[i].vB;
        if (u == v) continue;
        if (dsu.unite(u, v)) ++r;
    }
    return r;
}

// Compare cycle matroids of A[C] and B[C]. Caller guarantees equal ranks.
bool matEqualOn(int nA, const vector<Edge>& E, const vector<int>& C) {
    int k = (int)C.size();
    if (k == 0) return true;

    // Map A-vertices of C to local indices [0, cntA).
    static int curStampA = 0;
    ++curStampA;
    static vector<int> idMapA;
    if ((int)idMapA.size() < nA) idMapA.resize(nA, 0);
    // Use curStampA and idMapA, and markMapA as stamp array
    vector<int> touchedA;
    touchedA.reserve(k * 2);
    auto loc = [&](int v) -> int {
        if (markMapA[v] != curStampA) {
            markMapA[v] = curStampA;
            int id = (int)touchedA.size();
            idMapA[v] = id;
            touchedA.push_back(v);
        }
        return idMapA[v];
    };
    int cntA = 0;
    // First pass: assign local ids
    for (int i : C) {
        loc(E[i].uA);
        loc(E[i].vA);
    }
    cntA = (int)touchedA.size();
    vector<vector<pair<int,int>>> adj(cntA);
    vector<int> par(cntA, -1), pedge(cntA, -1), depth(cntA, 0);
    // Build a spanning tree of A[C]
    // Use a local DSU for connectedness; C is a block of A so connected.
    vector<int> dpar(cntA, -1);
    function<int(int)> dfind = [&](int x) {
        if (dpar[x] < 0) return x;
        return dpar[x] = dfind(dpar[x]);
    };
    vector<int> nonTree;
    nonTree.reserve(k);
    for (int i : C) {
        int a = loc(E[i].uA), b = loc(E[i].vA);
        int ra = dfind(a), rb = dfind(b);
        if (ra == rb) {
            nonTree.push_back(i);
        } else {
            if (dpar[ra] > dpar[rb]) swap(ra, rb); // size negative
            dpar[ra] += dpar[rb];
            dpar[rb] = ra;
            adj[a].push_back({b, i});
            adj[b].push_back({a, i});
        }
    }
    // Root the tree at local vertex 0 (C connected)
    vector<int> stack;
    stack.push_back(0);
    vector<char> vis(cntA, 0);
    vis[0] = 1;
    while (!stack.empty()) {
        int v = stack.back(); stack.pop_back();
        for (auto [to, id] : adj[v]) {
            if (!vis[to]) {
                vis[to] = 1;
                par[to] = v;
                pedge[to] = id;
                depth[to] = depth[v] + 1;
                stack.push_back(to);
            }
        }
    }

    // B parity toggling with stamps, per cycle
    static int curB = 0;
    auto cycleOk = [&](int extraEdge) -> bool {
        ++curB;
        vector<int> touched;
        touched.reserve(64);
        auto toggle = [&](int v) {
            if (parStampB[v] != curB) {
                parStampB[v] = curB;
                parityBv[v] = 1;
                touched.push_back(v);
            } else {
                parityBv[v] ^= 1;
            }
        };
        auto addEdge = [&](int id) {
            toggle(E[id].uB);
            toggle(E[id].vB);
        };
        addEdge(extraEdge);
        int a = loc(E[extraEdge].uA), b = loc(E[extraEdge].vA);
        while (depth[a] > depth[b]) {
            addEdge(pedge[a]);
            a = par[a];
        }
        while (depth[b] > depth[a]) {
            addEdge(pedge[b]);
            b = par[b];
        }
        while (a != b) {
            addEdge(pedge[a]);
            a = par[a];
            addEdge(pedge[b]);
            b = par[b];
        }
        for (int v : touched)
            if (parityBv[v]) return false;
        return true;
    };

    for (int i : nonTree) {
        if (!cycleOk(i)) return false;
    }
    return true;
}


static bool contractEdges(int& nA, int& nB, vector<Edge>& E, const vector<char>& sel) {
    int mm = (int)E.size();
    vector<int> parA(nA), parB(nB), szA(nA, 1), szB(nB, 1);
    iota(parA.begin(), parA.end(), 0);
    iota(parB.begin(), parB.end(), 0);
    function<int(vector<int>&, int)> fnd = [&](vector<int>& p, int x) -> int {
        while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; }
        return x;
    };
    for (int i = 0; i < mm; ++i) if (sel[i]) {
        int a = fnd(parA, E[i].uA), b = fnd(parA, E[i].vA);
        if (a != b) { if (szA[a] < szA[b]) swap(a,b); parA[b]=a; szA[a]+=szA[b]; }
        int c = fnd(parB, E[i].uB), d = fnd(parB, E[i].vB);
        if (c != d) { if (szB[c] < szB[d]) swap(c,d); parB[d]=c; szB[c]+=szB[d]; }
    }
    vector<int> idA(nA, -1), idB(nB, -1);
    int na = 0, nb = 0;
    for (int v = 0; v < nA; ++v) { int r = fnd(parA, v); if (idA[r] == -1) idA[r] = na++; }
    for (int v = 0; v < nB; ++v) { int r = fnd(parB, v); if (idB[r] == -1) idB[r] = nb++; }
    vector<Edge> NE;
    NE.reserve(mm);
    for (int i = 0; i < mm; ++i) if (!sel[i]) {
        Edge e = E[i];
        e.uA = idA[fnd(parA, e.uA)];
        e.vA = idA[fnd(parA, e.vA)];
        e.uB = idB[fnd(parB, e.uB)];
        e.vB = idB[fnd(parB, e.vB)];
        NE.push_back(e);
    }
    E.swap(NE);
    nA = na; nB = nb;
    return true;
}

bool solveCase(int n, vector<Edge> E) {
    int nA = n, nB = n;
    while (true) {
        // Remove loops in A (they are in no A-basis).
        vector<Edge> NE;
        NE.reserve(E.size());
        for (auto& e : E)
            if (e.uA != e.vA) NE.push_back(e);
        bool removedLoops = (NE.size() != E.size());
        E.swap(NE);
        int mm = (int)E.size();
        if (removedLoops) {
            // Deleting A-loops from B is harmless, but if it disconnects B then
            // every A-basis is B-disconnected, so the answer is YES.
            vector<int> p(nB);
            iota(p.begin(), p.end(), 0);
            function<int(int)> fnd = [&](int x) {
                while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; }
                return x;
            };
            for (auto& e : E) {
                int a = fnd(e.uB), b = fnd(e.vB);
                if (a != b) p[a] = b;
            }
            int compsB = 0;
            for (int v = 0; v < nB; ++v) if (fnd(v) == v) ++compsB;
            if (compsB > 1) return true;
        }
        if (mm == 0) return false;
        bool bLoop = false;
        for (auto& e : E)
            if (e.uB == e.vB) { bLoop = true; break; }
        if (bLoop) return true;
        // Both graphs are connected; ranks are nA-1 and nB-1.
        if (nA != nB) return true;

        // Biconnected components of A; singleton components are exactly bridges.
        vector<vector<int>> comps = getBiconnected(nA, E);
        vector<char> brA(mm, 0), brB;
        bool anyBridgeA = false;
        for (auto& C : comps) if (C.size() == 1) { brA[C[0]] = 1; anyBridgeA = true; }
        getBridges(nB, E, false, brB);
        bool bNotA = false;
        for (int i = 0; i < mm; ++i) {
            if (brB[i] && !brA[i]) { bNotA = true; break; }
        }
        if (bNotA) return true;
        if (anyBridgeA) {
            // Contract all bridges of A (coloops of A) in both graphs.
            contractEdges(nA, nB, E, brA);
            continue;
        }

        // A has no bridges; comps is its decomposition into biconnected blocks.
        LazyDSU dsuA, dsuB;
        dsuA.init(nA); dsuB.init(nB);
        vector<int> equalComp;
        bool allStrict = true;   // every block has r_B(C) > r_A(C)
        bool noStrict = true;    // every block has r_B(C) == r_A(C)
        for (auto& C : comps) {
            int ra = rankSubset(E, C, true, dsuA);
            int rb = rankSubset(E, C, false, dsuB);
            if (rb < ra) return true;
            if (rb == ra) {
                allStrict = false;
                if (equalComp.empty()) equalComp = C;
            } else {
                noStrict = false;
            }
        }
        if (allStrict) return true;
        if (noStrict) {
            // If every A-block is equal (as a matroid) to the corresponding
            // B-restriction, then the two matroids coincide.
            for (auto& C : comps)
                if (!matEqualOn(nA, E, C)) return true;
            return false;
        }

        if (!matEqualOn(nA, E, equalComp)) return true;
        // Contract this equal component in both graphs.
        vector<char> sel(mm, 0);
        for (int i : equalComp) sel[i] = 1;
        contractEdges(nA, nB, E, sel);
    }
}

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    NMAX = 10005;
    markMapA.assign(NMAX, 0);
    parStampB.assign(NMAX, 0);
    parityBv.assign(NMAX, 0);

    while (T--) {
        int n, m;
        if (scanf("%d %d", &n, &m) != 2) return 0;
        vector<Edge> E(m);
        for (int i = 0; i < m; ++i) {
            int uA, vA, uB, vB;
            if (scanf("%d %d %d %d", &uA, &vA, &uB, &vB) != 4) return 0;
            --uA; --vA; --uB; --vB;
            E[i] = {uA, vA, uB, vB};
        }
        puts(solveCase(n, E) ? "YES" : "NO");
    }
    return 0;
}

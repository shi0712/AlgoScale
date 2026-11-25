#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;
    DSU(int n) : p(n), sz(n,1) { iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x]==x ? x : p[x]=find(p[x]); }
    bool unite(int a, int b) {
        a=find(a); b=find(b);
        if (a==b) return false;
        if (sz[a]<sz[b]) swap(a,b);
        p[b]=a; sz[a]+=sz[b];
        return true;
    }
};

int n, m;
vector<pair<int,int>> edges;
vector<int> treeEdges, nonTreeEdges;
int Q0;
vector<int> x; // x[e] for tree edges (and some non-tree)
vector<int> parent, depth, parentEdge;
vector<vector<pair<int,int>>> treeAdj;

vector<int> get_path_edges(int u, int v) {
    vector<int> res;
    int a=u, b=v;
    while (depth[a] > depth[b]) {
        res.push_back(parentEdge[a]);
        a = parent[a];
    }
    while (depth[b] > depth[a]) {
        res.push_back(parentEdge[b]);
        b = parent[b];
    }
    while (a != b) {
        res.push_back(parentEdge[a]);
        res.push_back(parentEdge[b]);
        a = parent[a];
        b = parent[b];
    }
    return res;
}

int query(const vector<int>& es) {
    cout << "?";
    for (int e : es) cout << " " << e;
    cout << endl;
    int ans;
    cin >> ans;
    if (ans == -1) exit(0);
    return ans;
}

int query_nonTree_forest(const vector<int>& F) {
    if (F.empty()) return 0;
    DSU dsu(n);
    vector<int> T = F;
    for (int e : F) {
        dsu.unite(edges[e].first, edges[e].second);
    }
    for (int e : treeEdges) {
        int u = edges[e].first, v = edges[e].second;
        if (dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v);
            T.push_back(e);
        }
    }
    int QT = query(T);
    vector<bool> inT(m, false);
    for (int e : T) inT[e] = true;
    int sum_removed = 0;
    for (int e : treeEdges) if (!inT[e]) sum_removed += x[e];
    return QT - Q0 + sum_removed;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);

    cin >> n >> m;
    edges.resize(m);
    for (int i = 0; i < m; ++i) {
        cin >> edges[i].first >> edges[i].second;
    }

    if (m == n - 1) {
        cout << "!";
        for (int i = 0; i < m; ++i) cout << " " << i;
        cout << endl;
        return 0;
    }

    // Build initial spanning tree T0
    DSU dsu0(n);
    for (int i = 0; i < m; ++i) {
        int u = edges[i].first, v = edges[i].second;
        if (dsu0.unite(u, v)) treeEdges.push_back(i);
        else nonTreeEdges.push_back(i);
    }

    Q0 = query(treeEdges);
    if (Q0 == n - 1) {
        cout << "!";
        for (int e : treeEdges) cout << " " << e;
        cout << endl;
        return 0;
    }

    // Root T0
    parent.assign(n, -1);
    depth.assign(n, 0);
    parentEdge.assign(n, -1);
    treeAdj.assign(n, {});
    for (int e : treeEdges) {
        int u = edges[e].first, v = edges[e].second;
        treeAdj[u].push_back({v, e});
        treeAdj[v].push_back({u, e});
    }
    function<void(int,int)> dfs = [&](int u, int p) {
        for (auto [v, e] : treeAdj[u]) {
            if (v == p) continue;
            parent[v] = u;
            depth[v] = depth[u] + 1;
            parentEdge[v] = e;
            dfs(v, u);
        }
    };
    dfs(0, -1);

    x.assign(m, 0);
    vector<int> f_e(m, -1);
    for (int f : nonTreeEdges) {
        int u = edges[f].first, v = edges[f].second;
        vector<int> path = get_path_edges(u, v);
        for (int e : path) {
            if (f_e[e] == -1) f_e[e] = f;
        }
    }

    struct Eq { int e, f, D; };
    vector<Eq> equations;
    for (int e : treeEdges) {
        if (f_e[e] == -1) {
            x[e] = 1; // bridge
        } else {
            vector<int> T = treeEdges;
            T.erase(find(T.begin(), T.end(), e));
            T.push_back(f_e[e]);
            int resp = query(T);
            int D = resp - Q0;
            equations.push_back({e, f_e[e], D});
        }
    }

    // Build H_eq
    unordered_map<int,int> id;
    vector<int> varEdges;
    vector<vector<pair<int,int>>> adj;
    auto get_id = [&](int e) {
        auto it = id.find(e);
        if (it != id.end()) return it->second;
        int newId = varEdges.size();
        id[e] = newId;
        varEdges.push_back(e);
        adj.emplace_back();
        return newId;
    };
    for (auto& eq : equations) {
        int u = get_id(eq.e);
        int v = get_id(eq.f);
        adj[u].push_back({v, eq.D});
        adj[v].push_back({u, -eq.D});
    }
    int Nvar = varEdges.size();
    vector<int> comp(Nvar, -1), val(Nvar, 0);
    vector<bool> isFixedComp;
    vector<int> compSizeT;
    int compCount = 0;
    for (int i = 0; i < Nvar; ++i) {
        if (comp[i] != -1) continue;
        vector<int> nodes;
        queue<int> q;
        q.push(i);
        comp[i] = compCount;
        val[i] = 0;
        int mn = 0, mx = 0;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            nodes.push_back(u);
            mn = min(mn, val[u]);
            mx = max(mx, val[u]);
            for (auto [v, w] : adj[u]) {
                if (comp[v] == -1) {
                    comp[v] = compCount;
                    val[v] = val[u] + w;
                    q.push(v);
                }
            }
        }
        if (mx - mn == 1) {
            int t = (mn == 0 ? 0 : 1);
            for (int u : nodes) x[varEdges[u]] = val[u] + t;
            isFixedComp.push_back(true);
            compSizeT.push_back(0);
        } else if (mx - mn == 0) {
            isFixedComp.push_back(false);
            int cntT = 0;
            for (int u : nodes) {
                int e = varEdges[u];
                if (find(treeEdges.begin(), treeEdges.end(), e) != treeEdges.end()) cntT++;
            }
            compSizeT.push_back(cntT);
        } else {
            // should not happen
            exit(1);
        }
        compCount++;
    }

    // Resolve free components
    bool hasFree = false;
    for (bool b : isFixedComp) if (!b) hasFree = true;
    if (hasFree) {
        DSU compDsu(compCount);
        vector<vector<tuple<int,int,int>>> compGraph(compCount);
        for (int g : nonTreeEdges) {
            int u = edges[g].first, v = edges[g].second;
            vector<int> path = get_path_edges(u, v);
            vector<int> roots;
            for (int e : path) {
                int cid = comp[get_id(e)];
                roots.push_back(cid);
            }
            sort(roots.begin(), roots.end());
            roots.erase(unique(roots.begin(), roots.end()), roots.end());
            if (roots.size() >= 2) {
                int c0 = roots[0];
                int e0 = -1;
                for (int e : path) if (comp[get_id(e)] == c0) { e0 = e; break; }
                for (int i = 1; i < (int)roots.size(); ++i) {
                    int c = roots[i];
                    if (compDsu.find(c0) != compDsu.find(c)) {
                        int ec = -1;
                        for (int e : path) if (comp[get_id(e)] == c) { ec = e; break; }
                        vector<int> T1 = treeEdges;
                        T1.erase(find(T1.begin(), T1.end(), e0));
                        T1.push_back(g);
                        int R1 = query(T1);
                        int D1 = R1 - Q0;
                        vector<int> T2 = treeEdges;
                        T2.erase(find(T2.begin(), T2.end(), ec));
                        T2.push_back(g);
                        int R2 = query(T2);
                        int D2 = R2 - Q0;
                        int w = D1 - D2; // x_ec - x_e0
                        int W = w - val[get_id(ec)] + val[get_id(e0)];
                        compGraph[c0].push_back({c, W, 0});
                        compGraph[c].push_back({c0, -W, 0});
                        compDsu.unite(c0, c);
                    }
                }
            }
        }

        vector<int> t(compCount, -1);
        vector<bool> visited(compCount, false);
        vector<int> freeOnlyRoot = {-1};
        for (int i = 0; i < compCount; ++i) {
            if (visited[i]) continue;
            vector<int> nodes;
            queue<int> q;
            q.push(i);
            visited[i] = true;
            t[i] = 0;
            bool hasFixed = false;
            int fixedNode = -1;
            while (!q.empty()) {
                int u = q.front(); q.pop();
                nodes.push_back(u);
                if (isFixedComp[u]) {
                    hasFixed = true;
                    fixedNode = u;
                }
                for (auto [v, W, _] : compGraph[u]) {
                    if (!visited[v]) {
                        visited[v] = true;
                        t[v] = t[u] + W;
                        q.push(v);
                    }
                }
            }
            if (hasFixed) {
                // fixedNode has t = 0 in absolute, but our BFS set t[fixedNode] relative.
                int offset = -t[fixedNode];
                for (int u : nodes) t[u] += offset;
            } else {
                // free-only set
                freeOnlyRoot[0] = i;
            }
        }

        // Compute Q0 equation to determine free-only offset if needed
        if (freeOnlyRoot[0] != -1) {
            int root = freeOnlyRoot[0];
            // collect all nodes in this set
            vector<int> nodes;
            queue<int> q;
            q.push(root);
            vector<bool> vis(compCount, false);
            vis[root] = true;
            while (!q.empty()) {
                int u = q.front(); q.pop();
                nodes.push_back(u);
                for (auto [v, W, _] : compGraph[u]) {
                    if (!vis[v]) {
                        vis[v] = true;
                        q.push(v);
                    }
                }
            }
            int total_size = 0;
            int const_sum = 0;
            for (int u : nodes) {
                total_size += compSizeT[u];
                // sum of val for tree edges in this component
                // We need to iterate over varEdges in this component? We don't have list per component.
                // Instead, we can compute contribution from all tree edges using t.
            }
            // We'll compute total_const and coeff by iterating over all tree edges
            int total_const = 0;
            int coeff = 0;
            for (int e : treeEdges) {
                int cid = comp[get_id(e)];
                if (t[cid] == -1) {
                    // in free-only set, t is relative (root=0)
                    total_const += val[get_id(e)];
                    coeff++;
                } else {
                    total_const += val[get_id(e)] + t[cid];
                }
            }
            // Q0 = total_const + coeff * offset
            int offset = Q0 - total_const;
            if (coeff != 0) {
                if (offset % coeff != 0) exit(1);
                offset /= coeff;
            } else {
                offset = 0;
            }
            for (int u : nodes) t[u] += offset;
        }

        // Assign x for all varEdges
        for (int i = 0; i < Nvar; ++i) {
            int e = varEdges[i];
            x[e] = val[i] + t[comp[i]];
        }
    }

    // Now x for all tree edges is known
    vector<int> R_tree;
    for (int e : treeEdges) if (x[e] == 1) R_tree.push_back(e);

    int r = n - 1 - Q0;
    if (r == 0) {
        cout << "!";
        for (int e : R_tree) cout << " " << e;
        cout << endl;
        return 0;
    }

    // Build contracted graph H
    DSU dsu(n);
    for (int e : R_tree) {
        dsu.unite(edges[e].first, edges[e].second);
    }
    vector<int> rootToId(n, -1);
    int c = 0;
    for (int i = 0; i < n; ++i) {
        int r = dsu.find(i);
        if (rootToId[r] == -1) rootToId[r] = c++;
    }
    vector<vector<pair<int,int>>> adjH(c);
    vector<int> nonTreeH;
    for (int e : nonTreeEdges) {
        int u = edges[e].first, v = edges[e].second;
        int ru = rootToId[dsu.find(u)];
        int rv = rootToId[dsu.find(v)];
        if (ru != rv) {
            adjH[ru].push_back({rv, e});
            adjH[rv].push_back({ru, e});
            nonTreeH.push_back(e);
        }
    }

    // Prim's algorithm to find hidden non-tree edges
    vector<bool> inTree(c, false);
    inTree[0] = true;
    vector<int> hiddenNonTree;
    for (int step = 1; step < c; ++step) {
        int found_v = -1;
        vector<int> candidate_edges;
        for (int v = 0; v < c; ++v) {
            if (inTree[v]) continue;
            vector<int> F;
            for (auto [to, idx] : adjH[v]) {
                if (inTree[to]) F.push_back(idx);
            }
            if (F.empty()) continue;
            int cnt = query_nonTree_forest(F);
            if (cnt == 1) {
                found_v = v;
                candidate_edges = F;
                break;
            }
        }
        // binary search on candidate_edges
        int lo = 0, hi = candidate_edges.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            vector<int> F(candidate_edges.begin() + lo, candidate_edges.begin() + mid + 1);
            int cnt = query_nonTree_forest(F);
            if (cnt == 1) hi = mid;
            else lo = mid + 1;
        }
        int hidden_edge = candidate_edges[lo];
        hiddenNonTree.push_back(hidden_edge);
        inTree[found_v] = true;
    }

    // Output answer
    cout << "!";
    for (int e : R_tree) cout << " " << e;
    for (int e : hiddenNonTree) cout << " " << e;
    cout << endl;
    return 0;
}

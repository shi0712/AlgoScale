#include <bits/stdc++.h>
using namespace std;

/*** Disjoint Set Union ***/
struct DSU {
    vector<int> p, r;
    DSU(int n = 0) { init(n); }
    void init(int n) {
        p.resize(n);
        r.assign(n, 0);
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (r[a] < r[b]) swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<int> U(m), V(m);
    for (int i = 0; i < m; ++i) cin >> U[i] >> V[i];

    DSU dsu_main(n);                     // current contracted components
    vector<int> answer_edges;            // hidden edges found
    answer_edges.reserve(n - 1);

    while (true) {
        /* ----- current components ----- */
        vector<int> comp_of_node(n);
        vector<int> comp_index(n, -1);
        int cur_n = 0;
        for (int v = 0; v < n; ++v) {
            int root = dsu_main.find(v);
            if (comp_index[root] == -1) comp_index[root] = cur_n++;
            comp_of_node[v] = comp_index[root];
        }
        if (cur_n == 1) break;           // all contracted

        /* ----- build adjacency of the contracted graph ----- */
        vector<vector<pair<int,int>>> adj(cur_n);
        vector<pair<int,int>> edge_end(m, {-1,-1});   // active edges only
        int active_edges = 0;
        for (int id = 0; id < m; ++id) {
            int a = dsu_main.find(U[id]);
            int b = dsu_main.find(V[id]);
            if (a == b) continue;        // loop, ignore
            int ca = comp_index[a];
            int cb = comp_index[b];
            edge_end[id] = {ca, cb};
            adj[ca].push_back({cb, id});
            adj[cb].push_back({ca, id});
            ++active_edges;
        }

        /* ----- if the graph already is a tree, all remaining edges are hidden ----- */
        if (active_edges == cur_n - 1) {
            for (int id = 0; id < m; ++id)
                if (edge_end[id].first != -1) answer_edges.push_back(id);
            break;
        }

        /* ----- build an arbitrary spanning tree ----- */
        DSU dsu_tmp(cur_n);
        vector<int> tree_edges;
        tree_edges.reserve(cur_n - 1);
        for (int id = 0; id < m; ++id) {
            if (edge_end[id].first == -1) continue;
            int a = edge_end[id].first, b = edge_end[id].second;
            if (dsu_tmp.unite(a, b)) {
                tree_edges.push_back(id);
                if ((int)tree_edges.size() == cur_n - 1) break;
            }
        }

        /* ----- leaf vertices of the tree ----- */
        vector<int> degT(cur_n, 0);
        for (int id : tree_edges) {
            int a = edge_end[id].first, b = edge_end[id].second;
            ++degT[a]; ++degT[b];
        }

        bool contracted = false;
        for (int v = 0; v < cur_n && !contracted; ++v) {
            if (degT[v] != 1) continue;          // not a leaf
            /* leaf edge incident to v */
            int leaf_edge = -1;
            for (int id : tree_edges) {
                int a = edge_end[id].first, b = edge_end[id].second;
                if (a == v || b == v) { leaf_edge = id; break; }
            }
            if (leaf_edge == -1) continue;        // should not happen

            int deg_orig = (int)adj[v].size();
            if (deg_orig == 1) {                  // forced hidden edge
                answer_edges.push_back(leaf_edge);
                dsu_main.unite(U[leaf_edge], V[leaf_edge]);
                contracted = true;
                break;
            }

            /* leaf has other incident edges – need a query */
            int other_edge = -1;
            for (auto [to, id] : adj[v]) if (id != leaf_edge) { other_edge = id; break; }
            if (other_edge == -1) continue;       // should not happen (deg_orig>1)

            /* ----- first query : the tree ----- */
            cout << "?";
            for (int id : tree_edges) cout << ' ' << id;
            cout << "\n" << flush;
            int k;  if (!(cin >> k) || k == -1) return 0;

            /* ----- second query : swap leaf_edge with other_edge ----- */
            vector<int> swapped;
            swapped.reserve(cur_n - 1);
            for (int id : tree_edges) if (id != leaf_edge) swapped.push_back(id);
            swapped.push_back(other_edge);
            cout << "?";
            for (int id : swapped) cout << ' ' << id;
            cout << "\n" << flush;
            int k2; if (!(cin >> k2) || k2 == -1) return 0;

            int chosen;
            if (k2 > k)          chosen = other_edge;   // other_edge is hidden
            else if (k2 < k)     chosen = leaf_edge;    // leaf_edge is hidden
            else                 chosen = leaf_edge;    // both are hidden

            answer_edges.push_back(chosen);
            dsu_main.unite(U[chosen], V[chosen]);
            contracted = true;
            break;
        }

        if (!contracted) {
            /* fallback – should never be needed, but contract any leaf */
            for (int v = 0; v < cur_n && !contracted; ++v) {
                if (degT[v] != 1) continue;
                int leaf_edge = -1;
                for (int id : tree_edges) {
                    int a = edge_end[id].first, b = edge_end[id].second;
                    if (a == v || b == v) { leaf_edge = id; break; }
                }
                if (leaf_edge == -1) continue;
                answer_edges.push_back(leaf_edge);
                dsu_main.unite(U[leaf_edge], V[leaf_edge]);
                contracted = true;
                break;
            }
        }
    }

    /* ----- output the hidden tree ----- */
    cout << "!";
    for (int id : answer_edges) cout << ' ' << id;
    cout << "\n" << flush;
    return 0;
}

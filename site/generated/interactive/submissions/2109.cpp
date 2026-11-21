#include <bits/stdc++.h>
using namespace std;

int n;

int query(int p) {
    cout << "? " << p << endl;
    cout.flush();
    int resp;
    if (!(cin >> resp)) exit(0);
    if (resp == -1) exit(0);
    return resp;
}

struct DSU {
    vector<int> parent, sz;
    int comp;
    DSU(int n = 0) { init(n); }
    void init(int n) {
        parent.resize(n + 1);
        sz.assign(n + 1, 1);
        iota(parent.begin(), parent.end(), 0);
        comp = n;
    }
    int find(int x) {
        return parent[x] == x ? x : parent[x] = find(parent[x]);
    }
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        parent[b] = a;
        sz[a] += sz[b];
        --comp;
        return true;
    }
};

vector<vector<int>> adj;
vector<char> visited;

void dfs(int u, int parent) {
    visited[u] = 1;
    for (int v : adj[u]) {
        if (v == parent) continue;
        // bring v to match u
        while (true) {
            int resp = query(v);
            if (resp == 1) break;
        }
        dfs(v, u);
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    DSU dsu(n);
    adj.assign(n + 1, {});
    int root = 1;

    // Phase 1: build a spanning tree connecting all positions
    while (dsu.comp > 1) {
        int p = root;
        // Increment p until we get a collision (response 1)
        while (true) {
            int resp = query(p);
            if (resp == 1) break;
        }
        // Change p's value once more to differentiate it
        query(p);
        bool merged = false;
        for (int r = 1; r <= n; ++r) {
            if (r == p) continue;
            int resp = query(r);
            if (resp == 1) {
                if (dsu.find(p) != dsu.find(r)) {
                    dsu.unite(p, r);
                    adj[p].push_back(r);
                    adj[r].push_back(p);
                    merged = true;
                    break;
                }
            }
        }
        // If no merge happened, the loop will repeat and try again
    }

    // Phase 2: synchronize all positions using the tree
    visited.assign(n + 1, 0);
    dfs(root, 0);

    // Declare success
    cout << "!" << endl;
    cout.flush();
    return 0;
}

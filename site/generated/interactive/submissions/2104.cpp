#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<vector<int>> adj(n + 1);
    // Query all pairs to reconstruct the graph
    for (int i = 1; i <= n; ++i) {
        for (int j = i + 1; j <= n; ++j) {
            cout << "? 2\n" << i << ' ' << j << "\n" << flush;
            int ans;
            if (!(cin >> ans)) return 0;
            if (ans == -1) return 0;
            if (ans == 1) {
                adj[i].push_back(j);
                adj[j].push_back(i);
            }
        }
    }
    // BFS to check bipartiteness and find a certificate
    vector<int> color(n + 1, -1), parent(n + 1, -1), depth(n + 1, 0);
    queue<int> q;
    for (int start = 1; start <= n; ++start) {
        if (color[start] != -1) continue;
        color[start] = 0;
        depth[start] = 0;
        q.push(start);
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (color[v] == -1) {
                    color[v] = color[u] ^ 1;
                    parent[v] = u;
                    depth[v] = depth[u] + 1;
                    q.push(v);
                } else if (color[v] == color[u]) {
                    // odd cycle found
                    int a = u, b = v;
                    int x = a, y = b;
                    // bring to same depth
                    while (depth[x] > depth[y]) x = parent[x];
                    while (depth[y] > depth[x]) y = parent[y];
                    while (x != y) {
                        x = parent[x];
                        y = parent[y];
                    }
                    int lca = x;
                    vector<int> pathA, pathB;
                    for (int cur = a; cur != lca; cur = parent[cur]) pathA.push_back(cur);
                    for (int cur = b; cur != lca; cur = parent[cur]) pathB.push_back(cur);
                    vector<int> cycle;
                    for (int vtx : pathA) cycle.push_back(vtx);
                    cycle.push_back(lca);
                    for (int i = (int)pathB.size() - 1; i >= 0; --i) cycle.push_back(pathB[i]);
                    cout << "N " << cycle.size() << "\n";
                    for (size_t i = 0; i < cycle.size(); ++i) {
                        if (i) cout << ' ';
                        cout << cycle[i];
                    }
                    cout << "\n" << flush;
                    return 0;
                }
            }
        }
    }
    // Graph is bipartite: output one side of the partition
    vector<int> side;
    for (int i = 1; i <= n; ++i) if (color[i] == 0) side.push_back(i);
    cout << "Y " << side.size() << "\n";
    for (size_t i = 0; i < side.size(); ++i) {
        if (i) cout << ' ';
        cout << side[i];
    }
    cout << "\n" << flush;
    return 0;
}

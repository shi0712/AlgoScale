#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    vector<vector<int>> adj(n + 1);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    vector<long long> f(n + 1);
    for (int i = 1; i <= n; ++i) {
        cout << "? 1 1 " << i << "\n";
        cout.flush();
        long long ans;
        if (!(cin >> ans)) return 0;
        if (ans == -1) return 0;
        f[i] = ans;
    }

    vector<int> val(n + 1, 0);
    bool found = false;
    for (int r = 1; r <= n && !found; ++r) {
        if (abs(f[r]) != 1) continue; // root value must be +/-1
        vector<int> parent(n + 1, -1);
        vector<int> curval(n + 1, 0);
        bool ok = true;
        curval[r] = (int)f[r];
        queue<int> q;
        q.push(r);
        parent[r] = 0;
        while (!q.empty() && ok) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (v == parent[u]) continue;
                long long diff = f[v] - f[u];
                if (diff != 1 && diff != -1) {
                    ok = false;
                    break;
                }
                curval[v] = (int)diff;
                parent[v] = u;
                q.push(v);
            }
        }
        if (ok) {
            val = curval;
            found = true;
        }
    }

    // In the unlikely case no root was found (should not happen), fallback to any root
    if (!found) {
        // Use node 1 as root (its value may be wrong, but problem guarantees a solution)
        vector<int> parent(n + 1, -1);
        vector<int> curval(n + 1, 0);
        curval[1] = (int)f[1];
        queue<int> q;
        q.push(1);
        parent[1] = 0;
        while (!q.empty()) {
            int u = q.front(); q.pop();
            for (int v : adj[u]) {
                if (v == parent[u]) continue;
                long long diff = f[v] - f[u];
                curval[v] = (int)diff;
                parent[v] = u;
                q.push(v);
            }
        }
        val = curval;
    }

    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << " " << val[i];
    }
    cout << "\n";
    cout.flush();
    return 0;
}

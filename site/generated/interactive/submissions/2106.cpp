#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 2) {
        cout << "! 1 2\n" << flush;
        return 0;
    }

    vector<vector<int>> adj(n + 1);
    int need = n - 2;               // number of missing edges
    int found = 0;
    int d = n - 2;                  // current threshold

    while (found < need) {
        if (d < 0) d = 0;           // safety
        cout << "? " << d << "\n" << flush;
        int v, u;
        if (!(cin >> v >> u)) return 0;
        if (v == -1 && u == -1) return 0;   // error from judge
        if (v == 0 && u == 0) {   // no vertex with degree >= d
            --d;
            continue;
        }
        if (u != 0) {
            adj[v].push_back(u);
            adj[u].push_back(v);
            ++found;
        }
        // vertex v is removed automatically
    }

    // ----- rebuild the forest -----
    vector<int> comp(n + 1, -1), col(n + 1, 0);
    int compCnt = 0;
    queue<int> q;
    for (int i = 1; i <= n; ++i) {
        if (comp[i] != -1) continue;
        comp[i] = compCnt;
        col[i] = 0;
        q.push(i);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int to : adj[v]) {
                if (comp[to] == -1) {
                    comp[to] = compCnt;
                    col[to] = col[v] ^ 1;
                    q.push(to);
                }
            }
        }
        ++compCnt;
    }

    // ----- split by colour -----
    vector<int> group0, group1;
    for (int i = 1; i <= n; ++i) {
        if (col[i] == 0) group0.push_back(i);
        else group1.push_back(i);
    }

    // ----- find a cross‑component pair -----
    int a = -1, b = -1;
    for (int x : group0) {
        for (int y : group1) {
            if (comp[x] != comp[y]) {
                a = x; b = y;
                break;
            }
        }
        if (a != -1) break;
    }

    // If no such pair exists (no missing edges), any order works
    if (a == -1) {
        cout << "! ";
        for (int i = 1; i <= n; ++i) {
            if (i > 1) cout << ' ';
            cout << i;
        }
        cout << "\n" << flush;
        return 0;
    }

    // move a to the end of group0, b to the beginning of group1
    auto itA = find(group0.begin(), group0.end(), a);
    if (itA != group0.end()) swap(*itA, group0.back());
    auto itB = find(group1.begin(), group1.end(), b);
    if (itB != group1.end()) swap(*itB, group1[0]);

    // ----- output the Hamiltonian path -----
    cout << "! ";
    bool first = true;
    for (int v : group0) {
        if (!first) cout << ' ';
        first = false;
        cout << v;
    }
    for (int v : group1) {
        if (!first) cout << ' ';
        first = false;
        cout << v;
    }
    cout << "\n" << flush;
    return 0;
}

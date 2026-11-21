#include <bits/stdc++.h>
using namespace std;

long long query_path_sum(int u) {
    cout << "? 1 1 " << u << "\n" << flush;
    long long resp;
    if (!(cin >> resp)) exit(0);
    return resp;
}

void toggle_node(int u) {
    cout << "? 2 " << u << "\n" << flush;
    // no response
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v; // edges are irrelevant for the solution
    }

    vector<long long> a(n + 1), b(n + 1);
    // initial queries
    for (int i = 1; i <= n; ++i) {
        a[i] = query_path_sum(i);
    }

    // toggle node 1 once
    toggle_node(1);

    // queries after toggle
    for (int i = 1; i <= n; ++i) {
        b[i] = query_path_sum(i);
    }

    // determine the root
    int root = 1; // assume root is 1
    for (int i = 2; i <= n; ++i) {
        if (b[i] - a[i] == 0) {
            root = i;
            break;
        }
    }

    // recover initial values
    vector<int> val(n + 1);
    if (root == 1) {
        val[1] = (int)a[1];
        for (int i = 2; i <= n; ++i) {
            val[i] = (int)(a[i] - a[1]);
        }
    } else {
        val[root] = (int)a[root];
        val[1] = (int)(a[1] - a[root]);
        for (int i = 2; i <= n; ++i) {
            if (i == root) continue;
            val[i] = (int)(a[i] - a[1]);
        }
    }

    // final values after the performed toggle (node 1 flipped)
    vector<int> final_val(n + 1);
    final_val[1] = -val[1];
    for (int i = 2; i <= n; ++i) final_val[i] = val[i];

    // output answer
    cout << "!";
    for (int i = 1; i <= n; ++i) {
        cout << " " << final_val[i];
    }
    cout << "\n" << flush;
    return 0;
}

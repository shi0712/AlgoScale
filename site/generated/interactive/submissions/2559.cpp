#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
    }
    vector<long long> Q(n + 1);
    for (int u = 1; u <= n; ++u) {
        cout << "? 2 " << u << endl;
        cout << "? 1 " << n;
        for (int i = 1; i <= n; ++i) cout << " " << i;
        cout << endl;
        long long val;
        if (!(cin >> val)) return 0;
        Q[u] = val;
    }
    vector<int> a(n + 1);
    for (int u = 2; u <= n; ++u) {
        long long diff = Q[u] - Q[u - 1];
        if (diff < 0) a[u] = 1;
        else a[u] = -1;
    }
    long long Q0 = -Q[n];
    long long diff1 = Q[1] - Q0;
    if (diff1 < 0) a[1] = 1;
    else a[1] = -1;
    cout << "!";
    for (int u = 1; u <= n; ++u) {
        cout << " " << -a[u];
    }
    cout << endl;
    return 0;
}

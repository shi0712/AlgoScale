#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    
    vector<int> a(n), c(n), d(n);
    // Query "? i 0" for all i
    for (int i = 0; i < n; ++i) {
        cout << "? " << i << " " << 0 << endl;
        int resp;
        if (!(cin >> resp)) return 0;
        if (resp == -1) return 0;
        a[i] = resp;
    }
    // Query "? 0 j" for j = 1..n-1
    for (int j = 1; j < n; ++j) {
        cout << "? " << 0 << " " << j << endl;
        int resp;
        if (!(cin >> resp)) return 0;
        if (resp == -1) return 0;
        c[j] = resp;
    }
    c[0] = a[0]; // "? 0 0" already asked
    
    // Compute d_j = c_j XOR a_0
    for (int i = 0; i < n; ++i) {
        d[i] = c[i] ^ a[0];
    }
    
    int cnt = 0;
    vector<int> answer;
    vector<int> p(n), inv(n);
    
    for (int x = 0; x < n; ++x) {
        bool ok = true;
        fill(inv.begin(), inv.end(), -1);
        for (int i = 0; i < n; ++i) {
            int pi = a[i] ^ x;
            if (pi < 0 || pi >= n) { ok = false; break; }
            if (inv[pi] != -1) { ok = false; break; }
            p[i] = pi;
            inv[pi] = i;
        }
        if (!ok) continue;
        for (int i = 0; i < n; ++i) {
            if (d[p[i]] != (i ^ x)) { ok = false; break; }
        }
        if (!ok) continue;
        ++cnt;
        if (answer.empty()) answer = p;
    }
    
    cout << "!" << endl;
    cout << cnt << endl;
    for (int i = 0; i < n; ++i) {
        if (i) cout << ' ';
        cout << answer[i];
    }
    cout << endl;
    cout.flush();
    return 0;
}

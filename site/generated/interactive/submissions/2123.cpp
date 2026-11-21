#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    if (n == -1) return 0;
    int m = n - 1;
    const int MAXM = 500;
    
    string zeros(m, '0');
    vector<string> mask(m);
    for (int i = 0; i < m; ++i) {
        mask[i] = zeros;
        mask[i][i] = '1';
    }
    
    vector<int> ans0(n + 1);
    vector<vector<int8_t>> w(n + 1, vector<int8_t>(m));
    
    auto query = [&](int v, const string& b) -> int {
        cout << "? " << v << " " << b << "\n";
        cout.flush();
        int resp;
        if (!(cin >> resp)) exit(0);
        if (resp == -1) exit(0);
        return resp;
    };
    
    for (int v = 1; v <= n; ++v) {
        int a0 = query(v, zeros);
        ans0[v] = a0;
        for (int i = 0; i < m; ++i) {
            int ai = query(v, mask[i]);
            w[v][i] = static_cast<int8_t>(ai - a0); // should be +1 or -1
        }
    }
    
    vector<bitset<MAXM>> sig(n + 1);
    for (int v = 1; v <= n; ++v) {
        bitset<MAXM> bs;
        for (int i = 0; i < m; ++i) {
            if (w[v][i] == 1) bs.set(i);
        }
        sig[v] = bs;
    }
    
    vector<int> s(m), f(m);
    vector<char> found(m, 0);
    for (int u = 1; u <= n; ++u) {
        for (int v = u + 1; v <= n; ++v) {
            bitset<MAXM> diff = sig[u] ^ sig[v];
            if (diff.count() == 1) {
                int idx = -1;
                for (int i = 0; i < m; ++i) {
                    if (diff[i]) { idx = i; break; }
                }
                if (idx == -1) continue;
                if (found[idx]) continue;
                if (w[u][idx] == -1) {
                    s[idx] = u;
                    f[idx] = v;
                } else {
                    s[idx] = v;
                    f[idx] = u;
                }
                found[idx] = 1;
            }
        }
    }
    
    // Output answer
    cout << "!";
    for (int i = 0; i < m; ++i) {
        cout << " " << s[i] << " " << f[i];
    }
    cout << "\n";
    cout.flush();
    return 0;
}

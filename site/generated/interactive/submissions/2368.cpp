#include <bits/stdc++.h>
using namespace std;

int main() {
    int n;
    if (!(cin >> n)) return 0;
    vector<int> s(n, 0), d(n, 0);
    vector<int> U(n);
    iota(U.begin(), U.end(), 0);
    auto query = [&](const vector<int>& cfg) -> int {
        string t(n, '0');
        for (int i = 0; i < n; i++) t[i] = cfg[i] ? '1' : '0';
        cout << "? " << t << endl;
        int r;
        if (!(cin >> r)) exit(0);
        if (r == -2) exit(0);
        return r;
    };
    for (int i = 0; i < n; i++) {
        for (int u : U) s[u] = 0;
        int r = query(s);
        int v = (r == i) ? 1 : 0;
        int lo = 0, hi = (int)U.size() - 1;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            for (int j = 0; j < (int)U.size(); j++) {
                s[U[j]] = (j <= mid) ? v : 1 - v;
            }
            int rr = query(s);
            if (rr != i) hi = mid;
            else lo = mid + 1;
        }
        int target = U[lo];
        s[target] = v;
        d[target] = i;
        U.erase(U.begin() + lo);
    }
    string out(n, '0');
    for (int i = 0; i < n; i++) out[i] = s[i] ? '1' : '0';
    cout << "! " << out;
    for (int i = 0; i < n; i++) cout << " " << d[i];
    cout << endl;
    return 0;
}

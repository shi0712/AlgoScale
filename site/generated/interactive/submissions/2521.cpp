#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    cout << "? 1 " << n << endl;
    long long S;
    if (!(cin >> S) || S == -1) return 0;
    if (n == 1) {
        cout << "! " << S << endl;
        return 0;
    }
    long long max_val = 0;
    long long upper = S / 2;
    for (int i = 1; i <= n; ++i) {
        cout << "? " << i << " " << i << endl;
        long long x;
        if (!(cin >> x) || x == -1) return 0;
        if (x > max_val) max_val = x;
        if (max_val == upper) break;
    }
    cout << "! " << max_val << endl;
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

int main() {
    int n, k;
    if (!(cin >> n >> k)) return 0;
    if (n == 1) {
        cout << "! 1" << endl;
        return 0;
    }
    int b = (k == 1) ? 1 : k / 2;
    int m = n / b;
    vector<bool> alive(n + 1, true);
    for (int p = 0; p < m / 2; p++) {
        cout << "R" << endl;
        for (int t = 0; t < m; t++) {
            int off = (t % 2 == 1) ? (t + 1) / 2 : -(t / 2);
            int v = ((p + off) % m + m) % m;
            for (int i = v * b + 1; i <= v * b + b; i++) {
                if (!alive[i]) continue;
                cout << "? " << i << endl;
                string s;
                if (!(cin >> s)) exit(0);
                if (s == "E" || s == "-1") exit(0);
                if (s == "Y") alive[i] = false;
            }
        }
    }
    int d = 0;
    for (int i = 1; i <= n; i++) if (alive[i]) d++;
    cout << "! " << d << endl;
    return 0;
}

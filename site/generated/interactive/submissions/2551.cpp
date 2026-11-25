#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int p;
    if (!(cin >> p)) return 0;
    long long S = 0;

    while (p > 0) {
        cout << "- 1" << endl;
        int p1;
        if (!(cin >> p1) || p1 == -1) return 0;
        S += 1;

        if (p1 == p - 1) {
            // current was odd, bit 0 cleared
            p = p1;
        } else {
            // current was even, t = number of trailing zeros
            int t = p1 - p + 1;
            long long x = (1LL << t) - 1;
            cout << "- " << x << endl;
            int p2;
            if (!(cin >> p2) || p2 == -1) return 0;
            S += x;
            p = p2;
        }
    }

    cout << "! " << S << endl;
    return 0;
}

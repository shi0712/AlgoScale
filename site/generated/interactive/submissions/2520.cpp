#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    long long totalA = 1; // mushroom 0 is A

    for (int i = 1; i < n; i += 2) {
        if (i + 1 < n) {
            cout << "? 3 " << i << " 0 " << i + 1 << "\n";
            cout.flush();
            int s;
            if (!(cin >> s) || s == -1) return 0;
            totalA += s;
        } else {
            cout << "? 2 0 " << i << "\n";
            cout.flush();
            int s;
            if (!(cin >> s) || s == -1) return 0;
            totalA += s;
        }
    }

    cout << "! " << totalA << "\n";
    cout.flush();
    return 0;
}

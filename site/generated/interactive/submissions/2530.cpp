#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    int k;
    cin >> k;
    if (k == 1) return 0;

    // Make all wheels display distinct symbols (k = n)
    while (k < n) {
        cout << 1 << " " << 1 << endl;
        cin >> k;
        if (k == 1) return 0;
    }

    vector<bool> frozen(n + 1, false);
    while (k > 1) {
        bool progress = false;
        for (int i = 1; i <= n; ++i) {
            if (frozen[i]) continue;
            int steps = 0;
            while (k > 1 && steps < n) {
                int oldk = k;
                cout << i << " " << 1 << endl;
                cin >> k;
                if (k == 1) return 0;
                ++steps;
                if (k < oldk) {
                    frozen[i] = true;
                    progress = true;
                    break;
                }
            }
            if (k > 1 && steps == n) {
                frozen[i] = true;
            }
            if (k == 1) return 0;
        }
        if (!progress && k > 1) {
            fill(frozen.begin(), frozen.end(), false);
        }
    }
    return 0;
}

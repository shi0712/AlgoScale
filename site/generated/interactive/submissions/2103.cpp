#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    // ---------- small n : brute force, look for gcd = n-1 ----------
    if (n <= 130) {
        for (int i = 1; i <= n; ++i) {
            for (int j = i + 1; j <= n; ++j) {
                cout << "? " << i << ' ' << j << "\n" << flush;
                int r;
                if (!(cin >> r)) return 0;
                if (r == -1) return 0;
                if (r == n - 1) {
                    cout << "! " << i << ' ' << j << "\n" << flush;
                    int finalAns;
                    cin >> finalAns;          // read the judge's final reply
                    return 0;
                }
            }
        }
        // fallback (should never be reached)
        cout << "! 1 2\n" << flush;
        int finalAns;
        cin >> finalAns;
        return 0;
    }

    // ---------- large n : repeat detection ----------
    vector<char> eliminated(n + 1, 0);
    int remaining = n;

    for (int i = 1; i <= n; ++i) {
        if (eliminated[i]) continue;

        if (remaining == 1) {               // only the zero index left
            cout << "! " << i << ' ' << i << "\n" << flush;
            int finalAns;
            cin >> finalAns;
            return 0;
        }

        vector<int> seen;
        bool repeat = false;

        for (int j = 1; j <= n; ++j) {
            if (j == i) continue;
            cout << "? " << i << ' ' << j << "\n" << flush;
            int r;
            if (!(cin >> r)) return 0;
            if (r == -1) return 0;

            if (r == n - 1) {               // found pair (0 , n-1)
                cout << "! " << i << ' ' << j << "\n" << flush;
                int finalAns;
                cin >> finalAns;
                return 0;
            }

            bool already = false;
            for (int v : seen) if (v == r) { already = true; break; }
            if (already) {
                repeat = true;
                break;
            }
            seen.push_back(r);
        }

        if (!repeat) {                      // no repeat → zero index
            cout << "! " << i << ' ' << i << "\n" << flush;
            int finalAns;
            cin >> finalAns;
            return 0;
        } else {
            eliminated[i] = 1;
            --remaining;
            if (remaining == 1) {
                int zeroIdx = -1;
                for (int k = 1; k <= n; ++k)
                    if (!eliminated[k]) { zeroIdx = k; break; }
                if (zeroIdx != -1) {
                    cout << "! " << zeroIdx << ' ' << zeroIdx << "\n" << flush;
                    int finalAns;
                    cin >> finalAns;
                    return 0;
                }
            }
        }
    }

    // Should never reach here, but output something safe.
    cout << "! 1 1\n" << flush;
    int finalAns;
    cin >> finalAns;
    return 0;
}

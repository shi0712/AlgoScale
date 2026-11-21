#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;

    // Read the public grid (not used for queries)
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int x;
            cin >> x;
        }
    }

    vector<int> vals;
    vals.reserve(static_cast<size_t>(n) * (2 * n - 1));

    for (int l = 1; l <= n; ++l) {
        for (int T = 1; T <= 2 * n - 1; ++T) {
            cout << "? " << l << ' ' << T << "\n" << flush;
            int ans;
            if (!(cin >> ans)) return 0;
            if (ans == -1) return 0;   // failure response
            vals.push_back(ans);
        }
    }

    sort(vals.begin(), vals.end());

    cout << "! ";
    for (int i = 0; i < m; ++i) {
        if (i) cout << ' ';
        cout << vals[i];
    }
    cout << "\n" << flush;
    return 0;
}

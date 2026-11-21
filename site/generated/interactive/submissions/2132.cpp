#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    int visible;
    if (!(cin >> visible)) return 0;

    // list of all intervals [i,j]
    vector<pair<int,int>> intervals;
    for (int i = 1; i <= n; ++i)
        for (int j = i; j <= n; ++j)
            intervals.emplace_back(i, j);

    while (visible < n) {
        for (auto [l, r] : intervals) {
            cout << l << ' ' << r << "\n" << flush;
            int new_vis;
            if (!(cin >> new_vis)) return 0;
            if (new_vis == -1) return 0;          // judge error

            int delta = new_vis - visible;
            if (delta < 0) {
                // revert the flip
                cout << l << ' ' << r << "\n" << flush;
                int revert_vis;
                if (!(cin >> revert_vis)) return 0;
                if (revert_vis == -1) return 0;
                // visible stays unchanged
            } else {
                // keep the flip
                visible = new_vis;
                if (visible == n) return 0;       // all cells visible
            }
        }
    }
    return 0;
}

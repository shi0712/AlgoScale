#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<int> a(n), b(n);
    for (int i = 0; i < n; ++i) {
        cin >> a[i] >> b[i];
        --a[i];
        --b[i];
    }

    // This implementation is correct for small n.
    // It enumerates all non-empty subsets and checks whether the induced
    // graph on the subset is strongly connected.
    long long ans = 0;

    int N = 1 << n;
    for (int mask = 1; mask < N; ++mask) {
        vector<int> verts;
        for (int i = 0; i < n; ++i)
            if (mask >> i & 1) verts.push_back(i);

        if (verts.size() == 1) {
            ++ans;
            continue;
        }

        bool ok = true;

        // Forward reachability from the first vertex inside the subset.
        int start = verts[0];
        int reached = 1 << start;
        bool changed = true;
        while (changed) {
            changed = false;
            for (int i = 0; i < n; ++i) {
                if (!(reached >> i & 1)) continue;
                for (int j = a[i]; j <= b[i]; ++j) {
                    if ((mask >> j & 1) && !(reached >> j & 1)) {
                        reached |= 1 << j;
                        changed = true;
                    }
                }
            }
        }
        if (reached != mask) ok = false;

        // Reverse reachability to the first vertex inside the subset.
        if (ok) {
            reached = 1 << start;
            changed = true;
            while (changed) {
                changed = false;
                for (int i = 0; i < n; ++i) {
                    if (!(mask >> i & 1)) continue;
                    if (reached >> i & 1) continue;
                    bool can = false;
                    for (int j = a[i]; j <= b[i]; ++j) {
                        if ((reached >> j & 1)) {
                            can = true;
                            break;
                        }
                    }
                    if (can) {
                        reached |= 1 << i;
                        changed = true;
                    }
                }
            }
            if (reached != mask) ok = false;
        }

        if (ok) {
            ++ans;
            if (ans >= MOD) ans -= MOD;
        }
    }

    cout << ans % MOD << '\n';
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<int> trump(n);
    for (int i = 0; i < n; ++i) cin >> trump[i];

    vector<vector<long long>> a(n, vector<long long>(m));
    vector<vector<long long>> d(n, vector<long long>(m));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            cin >> a[i][j];
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j)
            cin >> d[i][j];

    const int total_masks = 1 << m;
    vector<long long> need(total_masks, 0);
    vector<long long> avail(total_masks, 0);
    vector<char> valid(total_masks, 1);

    // Precompute lsb index for each mask to speed up subset DP
    vector<int> lsb_idx(total_masks);
    for (int S = 1; S < total_masks; ++S) {
        int lsb = S & -S;
        lsb_idx[S] = __builtin_ctz(lsb);
    }

    for (int i = 0; i < n; ++i) {
        if (trump[i]) {
            vector<long long> best(total_masks, 0);
            vector<long long> f(total_masks, 0);
            for (int r = m - 1; r >= 0; --r) {
                long long c[18];
                for (int s = 0; s < m; ++s) {
                    c[s] = 0;
                    if (s >= r) c[s] += a[i][s];
                    if (s > r) c[s] -= d[i][s];
                }
                f[0] = 0;
                for (int S = 1; S < total_masks; ++S) {
                    int idx = lsb_idx[S];
                    f[S] = f[S ^ (S & -S)] + c[idx];
                    if (f[S] > best[S]) best[S] = f[S];
                }
            }
            // diff[S] = sum_{s in S} (d[i][s] - a[i][s])
            vector<long long> diff(total_masks, 0);
            for (int S = 1; S < total_masks; ++S) {
                int idx = lsb_idx[S];
                diff[S] = diff[S ^ (S & -S)] + (d[i][idx] - a[i][idx]);
            }
            for (int S = 0; S < total_masks; ++S) {
                if (best[S] > 0) valid[S] = 0;
                avail[S] += diff[S];
            }
        } else {
            vector<long long> best(total_masks, 0);
            vector<long long> f(total_masks, 0);
            for (int r = m - 1; r >= 0; --r) {
                long long c[18];
                for (int s = 0; s < m; ++s) {
                    c[s] = 0;
                    if (s >= r) c[s] += a[i][s];
                    if (s > r) c[s] -= d[i][s];
                }
                f[0] = 0;
                for (int S = 1; S < total_masks; ++S) {
                    int idx = lsb_idx[S];
                    f[S] = f[S ^ (S & -S)] + c[idx];
                    if (f[S] > best[S]) best[S] = f[S];
                }
            }
            for (int S = 0; S < total_masks; ++S) {
                need[S] += best[S];
            }
        }
    }

    vector<char> goodRank(m, 0);
    for (int S = 0; S < total_masks; ++S) {
        if (valid[S] && need[S] <= avail[S]) {
            for (int r = 0; r < m; ++r) {
                if (S & (1 << r)) goodRank[r] = 1;
            }
        }
    }

    long long ans = 0;
    for (int i = 0; i < n; ++i) {
        for (int r = 0; r < m; ++r) {
            if (!goodRank[r]) ans += a[i][r];
        }
    }
    cout << ans << endl;

    return 0;
}

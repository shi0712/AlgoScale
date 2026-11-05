#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<pair<int, long long>> a(n);
    long long totalT = 0;
    for (int i = 0; i < n; ++i) {
        long long v;
        int t;
        cin >> v >> t;
        a[i] = {t, v};
        totalT += t;
    }

    // Process smaller weights first to keep DP intervals smaller.
    sort(a.begin(), a.end());

    int K;
    if (n <= 250) {
        K = min(k, n);
    } else {
        // For weights <= 13, an optimal answer can be chosen with at most 13
        // doubled chips. For small n we keep the exact k anyway.
        K = min(k, 13);
    }

    int maxBound = (int)totalT + 13 * K + 5;
    int offset = maxBound + 2;
    int SZ = 2 * maxBound + 5;

    const long long NEG = -(1LL << 60);

    vector<long long> dp((K + 1) * SZ, NEG);
    vector<long long> ndp((K + 1) * SZ, NEG);

    dp[offset] = 0;

    int processed = 0;
    int sumT = 0;

    for (auto [t, v] : a) {
        int oldSum = sumT;
        sumT += t;
        ++processed;

        int maxD = min(K, processed);

        for (int d = 0; d <= maxD; ++d) {
            int boundNew = sumT + 13 * d;
            int low = offset - boundNew;
            int high = offset + boundNew;

            int Lsame = 1, Hsame = 0;
            if (d <= processed - 1) {
                int b = oldSum + 13 * d;
                Lsame = offset - b;
                Hsame = offset + b;
            }

            int Lprev = 1, Hprev = 0;
            if (d > 0 && d - 1 <= processed - 1) {
                int b = oldSum + 13 * (d - 1);
                Lprev = offset - b;
                Hprev = offset + b;
            }

            long long* oldd = &dp[d * SZ];
            long long* out = &ndp[d * SZ];
            long long* oldp = (d > 0 ? &dp[(d - 1) * SZ] : nullptr);

            for (int idx = low; idx <= high; ++idx) {
                long long best = NEG;

                // Use the chip neither in A nor in B.
                if (idx >= Lsame && idx <= Hsame) {
                    best = oldd[idx];
                }

                // Put the chip into A or B without doubling.
                int j = idx - t;
                if (j >= Lsame && j <= Hsame) {
                    long long cand = oldd[j] + v;
                    if (cand > best) best = cand;
                }

                j = idx + t;
                if (j >= Lsame && j <= Hsame) {
                    long long cand = oldd[j] + v;
                    if (cand > best) best = cand;
                }

                // Put the chip into A or B and double it.
                if (d > 0) {
                    j = idx - 2 * t;
                    if (j >= Lprev && j <= Hprev) {
                        long long cand = oldp[j] + v;
                        if (cand > best) best = cand;
                    }

                    j = idx + 2 * t;
                    if (j >= Lprev && j <= Hprev) {
                        long long cand = oldp[j] + v;
                        if (cand > best) best = cand;
                    }
                }

                out[idx] = best;
            }
        }

        dp.swap(ndp);
    }

    long long ans = 0;
    int maxD = min(K, k);

    for (int d = 0; d <= maxD; ++d) {
        ans = max(ans, dp[d * SZ + offset]);
    }

    cout << ans << '\n';
    return 0;
}

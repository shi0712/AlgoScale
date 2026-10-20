#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    long long MOD;
    cin >> n >> MOD;

    // dp[a][b] = current number of permutations with
    // a excedances and b descents.
    vector<vector<long long>> dp(n, vector<long long>(n, 0));
    dp[0][0] = 1;

    for (int len = 2; len <= n; ++len) {
        vector<vector<long long>> ndp(n, vector<long long>(n, 0));

        for (int a = 0; a < len; ++a) {
            for (int b = 0; b < len; ++b) {
                long long add = 0;

                // Insert the new maximum into a slot which does not
                // increase the descent number.
                if (a < n && b < n && dp[a][b]) {
                    add += (long long)(a + 1) * dp[a][b] % MOD;
                }

                // Insert the new maximum into a slot which increases
                // the excedance number.
                if (a > 0 && a - 1 < n && b < n && dp[a - 1][b]) {
                    add += (long long)(len - a) * dp[a - 1][b] % MOD;
                }

                if (a < n && b > 0 && dp[a][b - 1]) {
                    add += (long long)(b + 1) * dp[a][b - 1] % MOD;
                }

                if (a > 0 && b > 0 && dp[a - 1][b - 1]) {
                    add += (long long)(len - b) * dp[a - 1][b - 1] % MOD;
                }

                ndp[a][b] = add % MOD;
            }
        }

        dp.swap(ndp);
    }

    for (int a = 0; a < n; ++a) {
        for (int b = 0; b < n; ++b) {
            if (b) cout << ' ';
            cout << dp[a][b] % MOD;
        }
        cout << '\n';
    }

    return 0;
}

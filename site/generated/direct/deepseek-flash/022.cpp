#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    vector<pair<int,int>> qs(T);
    int N = 0;
    for (int i = 0; i < T; ++i) {
        int n, m;
        cin >> n >> m;
        qs[i] = {n, m};
        N = max(N, n);
    }

    // Binomial coefficients up to N.
    vector<vector<int>> C(N + 1, vector<int>(N + 1, 0));
    for (int i = 0; i <= N; ++i) {
        C[i][0] = C[i][i] = 1;
        for (int j = 1; j < i; ++j) {
            C[i][j] = C[i - 1][j - 1] + C[i - 1][j];
            if (C[i][j] >= MOD) C[i][j] -= MOD;
        }
    }

    // c[n][k] = number of legal words of length n using exactly k distinct values.
    vector<vector<int>> c(N + 1, vector<int>(N + 1, 0));

    vector<vector<int>> dp(N + 1, vector<int>(N + 1, 0));
    vector<vector<int>> ndp(N + 1, vector<int>(N + 1, 0));

    dp[0][0] = 1; // 0 used values, total length 0, active suffix length 0

    for (int used = 1; used <= N; ++used) {
        for (int i = 0; i <= N; ++i)
            fill(ndp[i].begin(), ndp[i].end(), 0);

        for (int len = 0; len <= N; ++len) {
            for (int a = 0; a <= len; ++a) {
                int cur = dp[len][a];
                if (!cur) continue;

                // Add one new distinct value.
                // f = number of occurrences of this value.
                for (int f = 1; len + f <= N; ++f) {
                    // This value is not selected as a bad value in IE.
                    {
                        int b = a + f;
                        int w = C[b][a];
                        int &to = ndp[len + f][b];
                        to = (to + 1LL * cur * w) % MOD;
                    }

                    // This value is selected as a bad value in IE.
                    if (f >= 2) {
                        for (int u = 0; u <= a; ++u) {
                            int b = u + f - 1;
                            int w = C[b - 1][u];
                            int sub = 1LL * cur * w % MOD;
                            int &to = ndp[len + f][b];
                            to -= sub;
                            if (to < 0) to += MOD;
                        }
                    }
                }
            }
        }

        for (int n = 1; n <= N; ++n) {
            int sum = 0;
            for (int a = 0; a <= n; ++a) {
                sum += ndp[n][a];
                if (sum >= MOD) sum -= MOD;
            }
            c[n][used] = sum;
        }

        dp.swap(ndp);
    }

    auto C_big = [&](int n, int k) -> int {
        if (k < 0 || k > n) return 0;
        long long res = 1;
        for (int i = 1; i <= k; ++i) {
            res = res * ((n - i + 1LL) % MOD + MOD) % MOD;
            res = res * mod_pow(i, MOD - 2) % MOD;
        }
        return (int)res;
    };

    for (auto [n, m] : qs) {
        long long ans = 0;
        int lim = min(n, m); // binomial is zero for k > m
        for (int k = 1; k <= lim; ++k) {
            if (!c[n][k]) continue;
            ans += 1LL * c[n][k] * C_big(m, k) % MOD;
            if (ans >= MOD) ans -= MOD;
        }
        cout << ans % MOD << '\n';
    }

    return 0;
}

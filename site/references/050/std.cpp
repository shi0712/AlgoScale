#include <bits/stdc++.h>
using namespace std;
using int64 = long long;
constexpr int MOD = 998244353;

int64 power(int64 a, int b) {
    int64 ans = 1;
    for (; b; b >>= 1, a = a * a % MOD)
        if (b & 1) ans = ans * a % MOD;
    return ans;
}

int solve_inverse_free_optimized(int n, int k) {
    if (n == 1) return k;
    int64 C[12][12] = {};
    for (int i = 0; i <= n; ++i) {
        C[i][0] = C[i][i] = 1;
        for (int j = 1; j < i; ++j)
            C[i][j] = C[i - 1][j - 1] + C[i - 1][j];
    }

    int64 f[12][12] = {}, scale = 1;
    int active = 0;
    f[n][0] = 1;
    for (int t = 1; t < k; ++t) {
        int64 pre[12], suf[12], factor[12];
        pre[0] = t;
        suf[n] = 1;
        for (int s = 1; s + 1 < n; ++s)
            pre[s] = pre[s - 1] * (t + s) % MOD;
        for (int s = n - 1; s >= 1; --s)
            suf[s] = suf[s + 1] * (t + s) % MOD;
        factor[0] = suf[1];
        for (int s = 1; s < n; ++s)
            factor[s] = pre[s - 1] * suf[s + 1] % MOD;
        scale = scale * factor[0] % MOD;

        for (int r = 2; r < n; ++r) {
            int64 diagonal = 0;
            for (int a = r; a <= n; ++a)
                diagonal += C[n - r][a - r] * f[a][0];
            f[r][r] = diagonal % MOD;
            for (int s = 0; s < r; ++s) {
                int64 value = (r * f[r][s + 1]
                    + (n - r) * (f[r][s] - f[r + 1][s + 1])) % MOD;
                if (value < 0) value += MOD;
                f[r][s] = value * factor[s] % MOD;
            }
        }
        const int next = active == 0 ? n - 1 : active - 1;
        int64 value = n * f[n][active] % MOD * factor[next] % MOD;
        f[n][active] = 0;
        f[n][next] = value;
        active = next;
    }

    int64 correction = 0;
    for (int r = 2; r <= n; ++r) {
        int64 value = (r - 1) * f[r][0] + f[r][1];
        correction = (correction + C[n][r] * value) % MOD;
    }
    return k * (power(n, k - 1) + correction * power(scale, MOD - 2) % MOD) % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, k;
    cin >> n >> k;
    cout << solve_inverse_free_optimized(n, k) << '\n';
    return 0;
}

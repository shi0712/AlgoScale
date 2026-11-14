#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int mod_pow(int a, int e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<int> fact(k + 1), inv_fact(k + 1);
    fact[0] = 1;
    for (int i = 1; i <= k; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
    inv_fact[k] = mod_pow(fact[k], MOD - 2);
    for (int i = k; i; --i) inv_fact[i - 1] = (long long)inv_fact[i] * i % MOD;

    int small_binom[11][11]{};
    for (int i = 0; i <= n; ++i) {
        small_binom[i][0] = small_binom[i][i] = 1;
        for (int j = 1; j < i; ++j)
            small_binom[i][j] = (small_binom[i - 1][j - 1] + small_binom[i - 1][j]) % MOD;
    }

    int inv[11]{};
    for (int i = 1; i <= n; ++i) inv[i] = mod_pow(i, MOD - 2);

    int prev[11][10]{}, cur[11][10]{};
    for (int c = 0; c < n; ++c) prev[0][c] = (k == c);

    int tail_sum = 0;
    int inv_fact_power[11];

    for (int x = 1; x <= k; ++x) {
        int y = x - 1;
        inv_fact_power[0] = 1;
        for (int s = 1; s <= n; ++s)
            inv_fact_power[s] = (long long)inv_fact_power[s - 1] * inv_fact[y] % MOD;

        memset(cur, 0, sizeof(cur));
        for (int c = 0; c < n; ++c) cur[0][c] = (k - n * x == c);

        for (int j = 1; j <= n; ++j) {
            int r = n - j;
            int t = k - r * x;
            if (t < 0 || t > j * y) continue;

            unsigned long long sum = 0;
            for (int s = 0; s <= j; ++s) {
                int u = t - s * y;
                if (u < 0) break;
                int source = prev[j - s][r];
                if (!source) continue;
                long long weight = (long long)small_binom[j][s] * inv_fact_power[s] % MOD;
                weight = weight * fact[t] % MOD;
                weight = weight * inv_fact[u] % MOD;
                sum += (unsigned long long)weight * source;
            }
            cur[j][0] = sum % MOD;
        }

        for (int c = 1; c < n; ++c) {
            for (int j = 1; j <= n; ++j) {
                int r = n - j;
                int t = k - r * x - c;
                if (t < 0 || t > j * y) continue;

                unsigned long long value = (unsigned long long)inv[j] * cur[j][c - 1];
                if (t >= y) {
                    long long binom = (long long)fact[t] * inv_fact[y] % MOD;
                    binom = binom * inv_fact[t - y] % MOD;
                    value += (unsigned long long)binom * cur[j - 1][c - 1];
                }
                cur[j][c] = value % MOD;
            }
        }

        tail_sum += cur[n][0];
        if (tail_sum >= MOD) tail_sum -= MOD;
        memcpy(prev, cur, sizeof(prev));
    }

    int total = (long long)k * mod_pow(n, k) % MOD;
    int answer = total - tail_sum;
    if (answer < 0) answer += MOD;
    cout << answer << '\n';
}

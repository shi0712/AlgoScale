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
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<int> a(m), b(m);
    for (int &x : a) cin >> x;
    for (int &x : b) cin >> x;

    vector<int> fact(n + 1), inv_fact(n + 1), inv(n + 1);
    fact[0] = 1;
    for (int i = 1; i <= n; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
    inv_fact[n] = mod_pow(fact[n], MOD - 2);
    for (int i = n; i >= 1; --i) inv_fact[i - 1] = (long long)inv_fact[i] * i % MOD;
    for (int i = 1; i <= n; ++i) inv[i] = (long long)fact[i - 1] * inv_fact[i] % MOD;

    auto choose = [&](int x, int y) {
        return (int)((long long)fact[x] * inv_fact[y] % MOD * inv_fact[x - y] % MOD);
    };

    vector<int> dp(n + 1);
    dp[0] = 1;
    int max_rate = 0, total_b = 0;
    long long extra = 0;

    for (int i = 0; i < m; ++i) {
        total_b += b[i];
        for (int k = 0; k < b[i]; ++k) {
            extra = (extra + (long long)(b[i] - k) * inv[a[i] - k]) % MOD;
        }
        if (b[i] == 0) continue;

        vector<int> coeff(b[i]);
        for (int j = 0; j < b[i]; ++j) {
            int c = (long long)choose(a[i], j) * choose(a[i] - j - 1, b[i] - j - 1) % MOD;
            if ((b[i] - 1 - j) & 1) c = (MOD - c) % MOD;
            coeff[j] = c;
        }

        for (int rate = max_rate; rate >= 0; --rate) {
            if (dp[rate] == 0) continue;
            for (int j = 0; j < b[i]; ++j) {
                int to = rate + a[i] - j;
                dp[to] = (dp[to] - (long long)dp[rate] * coeff[j]) % MOD;
            }
        }
        max_rate += a[i];
    }

    long long rate_sum = 0;
    for (int rate = 1; rate <= n; ++rate) {
        rate_sum = (rate_sum + (long long)dp[rate] * inv[rate]) % MOD;
    }

    long long answer = (extra + (long long)(total_b - n) * rate_sum) % MOD;
    if (answer < 0) answer += MOD;
    cout << answer << '\n';
    return 0;
}

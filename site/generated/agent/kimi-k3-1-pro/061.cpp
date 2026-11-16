#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 4000;

long long modpow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

long long fact[MAXN + 1], invfact[MAXN + 1], inv[MAXN + 1];
long long dfact[MAXN + 1];

void precompute() {
    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) fact[i] = fact[i - 1] * i % MOD;
    invfact[MAXN] = modpow(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) invfact[i - 1] = invfact[i] * i % MOD;
    inv[1] = 1;
    for (int i = 2; i <= MAXN; ++i) inv[i] = MOD - MOD / i * inv[MOD % i] % MOD;
    dfact[0] = 1;
    for (int i = 2; i <= MAXN; i += 2) dfact[i] = dfact[i - 2] * (i - 1) % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    precompute();

    auto comb = [](int n, int k) -> long long {
        return fact[n] * invfact[k] % MOD * invfact[n - k] % MOD;
    };

    int T;
    cin >> T;
    while (T--) {
        int n, m;
        cin >> n >> m;
        int N = 2 * m;
        vector<int> a(n);
        for (int &x : a) cin >> x;

        vector<long long> mono(N + 1, 0);
        for (int x : a) {
            long long term = x * inv[N] % MOD;
            for (int k = 1; k <= x; ++k) {
                if (k > 1) term = term * (x - k + 1) % MOD * inv[N - k + 1] % MOD;
                mono[k] += term;
                if (mono[k] >= MOD) mono[k] -= MOD;
            }
        }

        vector<long long> dp(N + 1, 0);
        long long invTotalMatchings = modpow(dfact[N], MOD - 2);
        for (int k = 2; k <= N; ++k) {
            long long rhs = (1 - mono[k] + MOD) % MOD;
            long long p0 = 0;
            int jmin = max(0, k - m);
            for (int j = jmin; j <= k / 2; ++j) {
                int activeInactive = k - 2 * j;
                int remaining = N - 2 * k + 2 * j;
                long long ways = comb(k, 2 * j) * dfact[2 * j] % MOD;
                ways = ways * comb(N - k, activeInactive) % MOD;
                ways = ways * fact[activeInactive] % MOD;
                ways = ways * dfact[remaining] % MOD;
                long long prob = ways * invTotalMatchings % MOD;
                if (j == 0) p0 = prob;
                else rhs = (rhs + prob * dp[k - j]) % MOD;
            }
            dp[k] = rhs * modpow((1 - p0 + MOD) % MOD, MOD - 2) % MOD;
        }
        cout << dp[N] << '\n';
    }
    return 0;
}

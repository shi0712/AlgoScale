#include <bits/stdc++.h>
using namespace std;

const int MOD = 998244353;
const int MAXN = 4000;

long long modpow(long long a, long long e) {
    long long res = 1;
    while (e) {
        if (e & 1) res = res * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Precompute factorials, inverse factorials, inverses, double factorials
    vector<long long> fact(MAXN + 1), invfact(MAXN + 1), inv(MAXN + 1);
    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) fact[i] = fact[i - 1] * i % MOD;

    inv[1] = 1;
    for (int i = 2; i <= MAXN; ++i)
        inv[i] = MOD - MOD / i * inv[MOD % i] % MOD;

    invfact[MAXN] = modpow(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i)
        invfact[i - 1] = invfact[i] * i % MOD;

    int maxHalf = MAXN / 2;
    vector<long long> dfact(maxHalf + 1), inv_dfact(maxHalf + 1);
    dfact[0] = 1;
    for (int i = 1; i <= maxHalf; ++i)
        dfact[i] = dfact[i - 1] * (2 * i - 1) % MOD;
    for (int i = 0; i <= maxHalf; ++i)
        inv_dfact[i] = modpow(dfact[i], MOD - 2);

    int T;
    cin >> T;
    while (T--) {
        int n, m;
        cin >> n >> m;
        int N = 2 * m;
        vector<int> a(n);
        for (int i = 0; i < n; ++i) cin >> a[i];

        // S[k] = sum_i C(a_i, k)
        vector<long long> S(N + 1, 0);
        for (int ai : a) {
            long long comb = 1; // C(ai, 0)
            for (int k = 1; k <= ai; ++k) {
                comb = comb * (ai - k + 1) % MOD * inv[k] % MOD;
                S[k] = (S[k] + comb) % MOD;
            }
        }

        // c[k] = 1 - sum_i C(a_i, k) / C(N, k)
        vector<long long> c(N + 1, 0);
        for (int k = 1; k <= N; ++k) {
            long long invC = fact[k] * fact[N - k] % MOD * invfact[N] % MOD;
            long long f = S[k] * invC % MOD;
            c[k] = (1 - f + MOD) % MOD;
        }

        // E[k] = expected remaining cost starting with k lineages
        vector<long long> E(N + 1, 0);
        E[1] = 0;
        long long inv_total = inv_dfact[N / 2]; // 1 / (N-1)!!

        for (int k = 2; k <= N; ++k) {
            int lower = max(0, k - N / 2);
            int upper = k / 2;

            long long sum = c[k];
            long long denom = 1;
            long long base = fact[N - k] * inv_total % MOD;

            for (int x = lower; x <= upper; ++x) {
                // Probability of exactly x internal edges in the occupied set
                long long comb = fact[k] * invfact[2 * x] % MOD * invfact[k - 2 * x] % MOD;
                int s = N - 2 * k + 2 * x; // even, >= 0
                long long term = comb * dfact[x] % MOD;
                term = term * base % MOD;
                term = term * invfact[s] % MOD;
                term = term * dfact[s / 2] % MOD;

                if (x == 0) {
                    denom = (1 - term + MOD) % MOD;
                } else {
                    sum = (sum + term * E[k - x]) % MOD;
                }
            }

            E[k] = sum * modpow(denom, MOD - 2) % MOD;
        }

        cout << E[N] << '\n';
    }

    return 0;
}

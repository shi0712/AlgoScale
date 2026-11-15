#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 4005;

long long mod_pow(long long a, long long e) {
    long long r = 1 % MOD;
    a %= MOD;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

int invInt[MAXN + 2];
int fact[MAXN + 2], invFact[MAXN + 2];

long long C(int n, int k) {
    if (k < 0 || k > n || n < 0) return 0;
    return (long long)fact[n] * invFact[k] % MOD * invFact[n - k] % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    invInt[1] = 1;
    for (int i = 2; i <= MAXN + 1; ++i) {
        invInt[i] = (long long)(MOD - MOD / i) * invInt[MOD % i] % MOD;
    }
    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
    invFact[MAXN] = (int)mod_pow(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) invFact[i - 1] = (long long)invFact[i] * i % MOD;

    int T;
    cin >> T;
    while (T--) {
        int n, m;
        cin >> n >> m;
        int N = 2 * m, K = m;
        vector<int> a(n);
        for (int i = 0; i < n; ++i) cin >> a[i];

        // a[k][i] : coefficient of C(y,i) in C(2y,k)
        vector<vector<int>> A(K + 1, vector<int>(K + 1, 0));
        A[0][0] = 1;
        for (int k = 1; k <= K; ++k) {
            for (int i = 0; i <= k; ++i) {
                long long v = 0;
                if (i <= k - 1) {
                    long long coef = 2LL * i - (k - 1);
                    v += coef * A[k - 1][i];
                }
                if (i >= 1) {
                    v += 2LL * i * A[k - 1][i - 1];
                }
                v %= MOD;
                if (v < 0) v += MOD;
                A[k][i] = (int)(v * invInt[k] % MOD);
            }
        }

        // lambda[k] = C(K,k)/C(2K,k)
        vector<int> lambda(K + 1);
        lambda[0] = 1;
        for (int k = 1; k <= K; ++k) {
            lambda[k] = (long long)lambda[k - 1] * (K - k + 1) % MOD * invInt[N - k + 1] % MOD;
        }

        // mu[k] = 2^k * lambda[k]; diagonal of T in this basis.
        // We need 1 - mu[k] for k >= 2.
        vector<int> invOneMinusMu(K + 1);
        long long mu = 1;
        invOneMinusMu[0] = invOneMinusMu[1] = 0;
        for (int k = 1; k <= K; ++k) {
            mu = mu * (2LL * K - 2LL * (k - 1)) % MOD * invInt[N - k + 1] % MOD;
            if (k >= 2) {
                long long d = (1 - mu) % MOD;
                if (d < 0) d += MOD;
                invOneMinusMu[k] = (int)mod_pow(d, MOD - 2);
            }
        }

        vector<int> b(K + 1, 0), c(K + 1, 0);
        b[1] = (b[1] + invInt[K]) % MOD;
        b[K] = (b[K] - 1 + MOD) % MOD;

        for (int i = K; i >= 2; --i) {
            long long s = 0;
            for (int k = i + 1; k <= K; ++k) {
                s += (long long)c[k] * lambda[k] % MOD * A[k][i] % MOD;
                if (s >= (long long)MOD * MOD) s %= MOD;
            }
            s %= MOD;
            long long rhs = (b[i] + s) % MOD;
            c[i] = (int)(rhs * invOneMinusMu[i] % MOD);
        }

        // P(K) = sum_{k>=2} c[k] * C(K,k)
        long long PK = 0;
        for (int k = 2; k <= K; ++k) {
            PK = (PK + (long long)c[k] * C(K, k)) % MOD;
        }
        c[1] = (int)((MOD - PK) * invInt[K] % MOD);

        vector<int> f(N + 1, 0);
        for (int s = 1; s <= N; ++s) {
            long long val = 0;
            if ((s & 1) == 0) {
                int y = s / 2;
                for (int k = 1; k <= min(K, y); ++k) {
                    val += (long long)c[k] * C(y, k) % MOD;
                    if (val >= (long long)MOD * MOD) val %= MOD;
                }
            } else {
                val = (long long)s * invInt[N] % MOD;
                for (int k = 1; k <= min(K, s); ++k) {
                    val += (long long)c[k] * lambda[k] % MOD * C(s, k) % MOD;
                    if (val >= (long long)MOD * MOD) val %= MOD;
                }
            }
            f[s] = (int)(val % MOD);
        }

        long long ans = 0;
        for (int x : a) ans += f[x];
        ans %= MOD;
        cout << ans << '\n';
    }
    return 0;
}

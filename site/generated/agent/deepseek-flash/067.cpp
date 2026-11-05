#include <bits/stdc++.h>
using namespace std;

static const long long MOD = 998244353;
static const int MAXF = 8005;

long long fact[MAXF], invfact[MAXF], invnum[MAXF], pow2v[MAXF];

long long modpow(long long a, long long e) {
    long long r = 1 % MOD;
    a %= MOD;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

inline long long Cmod(int n, int k) {
    if (k < 0 || k > n || n < 0) return 0;
    return fact[n] * invfact[k] % MOD * invfact[n - k] % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    fact[0] = 1;
    for (int i = 1; i < MAXF; ++i) fact[i] = fact[i - 1] * i % MOD;
    invfact[MAXF - 1] = modpow(fact[MAXF - 1], MOD - 2);
    for (int i = MAXF - 1; i >= 1; --i) invfact[i - 1] = invfact[i] * i % MOD;
    for (int i = 1; i < MAXF; ++i) invnum[i] = fact[i - 1] * invfact[i] % MOD;
    pow2v[0] = 1;
    for (int i = 1; i < MAXF; ++i) pow2v[i] = pow2v[i - 1] * 2 % MOD;

    int T;
    cin >> T;
    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<int> a(n);
        for (int &x : a) cin >> x;
        int N = 2 * m;

        if (n == 1) {
            cout << 0 << '\n';
            continue;
        }

        // b(t) = t/m for 1 <= t <= m-1, and b(0) = b(m) = 0.
        vector<long long> b(m + 1, 0);
        long long invm = invnum[m];
        for (int t = 1; t < m; ++t) b[t] = (long long)t * invm % MOD;

        // Newton coefficients beta_j = Delta^j b(0).
        vector<long long> beta(m + 1);
        vector<long long> cur = b;
        for (int j = 0; j <= m; ++j) {
            beta[j] = cur[0];
            for (int i = 0; i < m - j; ++i) {
                cur[i] = cur[i + 1] - cur[i];
                if (cur[i] < 0) cur[i] += MOD;
            }
        }

        // lambda_j = C(m,j) / C(2m,j)
        vector<long long> lam(m + 1);
        lam[0] = 1;
        for (int j = 1; j <= m; ++j) {
            lam[j] = lam[j - 1] * (m - j + 1) % MOD * invnum[N - j + 1] % MOD;
        }

        // Solve (I-K)a = b in the Newton basis C(t,j).
        vector<long long> c(m + 1, 0);
        for (int k = m; k >= 2; --k) {
            long long s = beta[k];
            int jmax = min(m, 2 * k);
            for (int j = k + 1; j <= jmax; ++j) {
                long long Bjk = Cmod(k, j - k) * pow2v[2 * k - j] % MOD;
                s += lam[j] * Bjk % MOD * c[j] % MOD;
                if (s >= MOD) s -= MOD;
            }
            long long mu = lam[k] * pow2v[k] % MOD; // diagonal eigenvalue
            long long den = (1 - mu + MOD) % MOD;
            c[k] = s * modpow(den, MOD - 2) % MOD;
        }

        // c[0] = a(0) = 0; c[1] is fixed by a(m) = 0.
        long long sm = 0;
        for (int j = 2; j <= m; ++j) {
            sm += c[j] * Cmod(m, j) % MOD;
            if (sm >= MOD) sm -= MOD;
        }
        c[1] = (MOD - sm) % MOD * invnum[m] % MOD;

        long long invN = invnum[N];
        long long ans = 0;
        for (int s : a) {
            if (s == N) continue; // f(N)=0
            long long val = (long long)s * invN % MOD;
            int lim = min(m, s);
            long long comb = 1;
            for (int j = 1; j <= lim; ++j) {
                comb = comb * ((s - j + 1) % MOD) % MOD * invnum[j] % MOD;
                val += c[j] * lam[j] % MOD * comb % MOD;
                if (val >= MOD) val -= MOD;
            }
            ans += val;
            if (ans >= MOD) ans -= MOD;
        }
        cout << ans % MOD << '\n';
    }
    return 0;
}

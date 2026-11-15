#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll MOD = 998244353;

ll pw(ll b, ll e) {
    ll r = 1;
    b %= MOD;
    if (b < 0) b += MOD;
    while (e) {
        if (e & 1) r = r * b % MOD;
        b = b * b % MOD;
        e >>= 1;
    }
    return r;
}

const int MX = 4105;
ll fact[MX], ifact[MX], df[MX];

ll C(int n, int k) {
    if (k < 0 || n < 0 || k > n) return 0;
    return fact[n] * ifact[k] % MOD * ifact[n - k] % MOD;
}

int main() {
    fact[0] = 1;
    for (int i = 1; i < MX; i++) fact[i] = fact[i - 1] * i % MOD;
    ifact[MX - 1] = pw(fact[MX - 1], MOD - 2);
    for (int i = MX - 1; i > 0; i--) ifact[i - 1] = ifact[i] * i % MOD;
    df[0] = 1;
    for (int k = 1; k < MX; k++) df[k] = df[k - 1] * (2 * k - 1) % MOD;

    int T;
    scanf("%d", &T);
    while (T--) {
        int n, m;
        scanf("%d %d", &n, &m);
        int N = 2 * m;
        vector<int> a(n);
        for (auto &x : a) scanf("%d", &x);

        // S[r] = sum_i C(a_i, r)
        vector<ll> S(N + 2, 0);
        for (int i = 0; i < n; i++) {
            for (int r = 1; r <= a[i]; r++) {
                S[r] = (S[r] + C(a[i], r)) % MOD;
            }
        }

        ll invD = pw(df[m], MOD - 2);
        vector<ll> inflow(N + 2, 0);
        inflow[N] = 1;
        ll ans = 0;
        for (int r = N; r >= 2; r--) {
            // j = 0 term
            ll P0 = 0;
            if (r <= N - r) {
                int j = 0;
                ll t = C(r, 2 * j) * df[j] % MOD * C(N - r, r - 2 * j) % MOD *
                       fact[r - 2 * j] % MOD * df[(N - 2 * r + 2 * j) / 2] % MOD * invD % MOD;
                P0 = t;
            }
            ll denom = ((1 - P0) % MOD + MOD) % MOD;
            ll G = inflow[r] % MOD * pw(denom, MOD - 2) % MOD;
            // q(r)
            ll q = (1 - S[r] * ifact[N] % MOD * fact[r] % MOD * fact[N - r] % MOD + MOD) % MOD;
            // C(N,r)^{-1} = ifact[N]^{-1}... compute properly below
            ll invCN = fact[r] * fact[N - r] % MOD * ifact[N] % MOD;
            q = ((1 - S[r] * invCN) % MOD + MOD) % MOD;
            ans = (ans + q * G) % MOD;

            int jmin = max(1, (2 * r - N + 1) / 2);
            if (2 * r - N <= 0) jmin = 1;
            int jmax = r / 2;
            for (int j = jmin; j <= jmax; j++) {
                if (r - 2 * j > N - r) continue;
                ll t = C(r, 2 * j) * df[j] % MOD * C(N - r, r - 2 * j) % MOD *
                       fact[r - 2 * j] % MOD * df[(N - 2 * r + 2 * j) / 2] % MOD * invD % MOD;
                inflow[r - j] = (inflow[r - j] + G * t) % MOD;
            }
        }
        printf("%lld\n", ans % MOD);
    }
    return 0;
}

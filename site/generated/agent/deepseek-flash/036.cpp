#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll MOD = 998244353;

ll pw(ll b, ll e) { ll r = 1 % MOD; b %= MOD; while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; } return r; }

int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return 0;
    vector<int> a(m), b(m);
    for (int i = 0; i < m; i++) scanf("%d", &a[i]);
    for (int i = 0; i < m; i++) scanf("%d", &b[i]);

    vector<ll> fact(n + 1, 1), inv_fact(n + 1, 1);
    for (int i = 1; i <= n; i++) fact[i] = fact[i - 1] * i % MOD;
    inv_fact[n] = pw(fact[n], MOD - 2);
    for (int i = n; i >= 1; i--) inv_fact[i - 1] = inv_fact[i] * i % MOD;
    auto C = [&](int N, int K) -> ll {
        if (K < 0 || K > N) return 0;
        return fact[N] * inv_fact[K] % MOD * inv_fact[N - K] % MOD;
    };

    int B = 0;
    for (int x : b) B += x;

    // F[i] = P(Gamma_i <= t) as polynomial in x = exp(-t)
    // G[i] = E[(b_i - c_i(t))^+] as polynomial in x = exp(-t)
    vector<vector<ll>> F(m), G(m);
    for (int i = 0; i < m; i++) {
        int ai = a[i], bi = b[i];
        vector<ll> S(ai + 1, 0), gi(ai + 1, 0);
        for (int j = 0; j < bi; j++) {
            ll cS = C(ai, j);
            ll cG = (ll)(bi - j) % MOD * C(ai, j) % MOD;
            // (1-x)^j
            for (int t = 0; t <= j; t++) {
                ll cj = C(j, t);
                ll s1 = cS * cj % MOD;
                ll s2 = cG * cj % MOD;
                if (t & 1) { s1 = (MOD - s1) % MOD; s2 = (MOD - s2) % MOD; }
                int d = ai - j + t;
                S[d] = (S[d] + s1) % MOD;
                gi[d] = (gi[d] + s2) % MOD;
            }
        }
        vector<ll> fi(ai + 1, 0);
        fi[0] = 1;
        for (int d = 0; d <= ai; d++) fi[d] = (fi[d] - S[d] + MOD) % MOD;
        F[i] = fi;
        G[i] = gi;
    }

    // Q = prod_i F[i]  (degree <= n)
    vector<ll> Q(1, 1);
    for (int i = 0; i < m; i++) {
        vector<ll> R(Q.size() + F[i].size() - 1, 0);
        for (size_t p = 0; p < Q.size(); p++) {
            if (!Q[p]) continue;
            for (size_t q = 0; q < F[i].size(); q++)
                R[p + q] = (R[p + q] + Q[p] * F[i][q]) % MOD;
        }
        Q.swap(R);
    }

    // P(x) = (n-B)*(1 - Q) + sum_i G[i]
    vector<ll> P(n + 1, 0);
    ll w = (ll)(n - B) % MOD;
    for (size_t d = 0; d < Q.size(); d++) {
        ll v = (d == 0 ? (1 - Q[0] + MOD) % MOD : (MOD - Q[d]) % MOD);
        P[d] = (P[d] + w * v) % MOD;
    }
    for (int i = 0; i < m; i++)
        for (size_t d = 0; d < G[i].size(); d++) P[d] = (P[d] + G[i][d]) % MOD;

    // E[T] = integral_0^inf P(exp(-t)) dt = sum_R P[R] / R
    vector<ll> inv(n + 1, 0);
    if (n >= 1) inv[1] = 1;
    for (int i = 2; i <= n; i++) inv[i] = (MOD - (MOD / i) * inv[MOD % i] % MOD) % MOD;
    ll ans = 0;
    for (int R = 1; R <= n; R++) {
        if (!P[R]) continue;
        ans = (ans + P[R] * inv[R]) % MOD;
    }
    printf("%lld\n", ans);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll MOD = 998244353;
ll pw(ll b, ll e) {
    ll r = 1; b %= MOD;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}
int main() {
    int n, m;
    scanf("%d %d", &n, &m);
    vector<int> a(m), b(m);
    for (auto &x : a) scanf("%d", &x);
    for (auto &x : b) scanf("%d", &x);
    int M = 2105;
    vector<ll> f(M), inv(M);
    f[0] = 1;
    for (int i = 1; i < M; i++) f[i] = f[i - 1] * i % MOD;
    vector<ll> fi(M);
    fi[M - 1] = pw(f[M - 1], MOD - 2);
    for (int i = M - 1; i > 0; i--) fi[i - 1] = fi[i] * i % MOD;
    for (int i = 1; i < M; i++) inv[i] = fi[i] * f[i - 1] % MOD;
    auto C = [&](int x, int y) -> ll {
        if (y < 0 || y > x) return 0;
        return f[x] * fi[y] % MOD * fi[x - y] % MOD;
    };
    vector<ll> G(1, 1);
    for (int i = 0; i < m; i++) {
        if (b[i] == 0) continue;
        int A = a[i];
        vector<ll> F(A + 1, 0);
        for (int j = b[i]; j <= A; j++) {
            ll cj = C(A, j);
            for (int l = 0; l <= j; l++) {
                ll v = cj * C(j, l) % MOD;
                int idx = A - j + l;
                if (l & 1) F[idx] = (F[idx] - v + MOD) % MOD;
                else F[idx] = (F[idx] + v) % MOD;
            }
        }
        vector<ll> H(G.size() + A, 0);
        for (size_t p = 0; p < G.size(); p++) {
            if (!G[p]) continue;
            for (int q = 0; q <= A; q++) {
                H[p + q] = (H[p + q] + G[p] * F[q]) % MOD;
            }
        }
        G = H;
    }
    ll ET = 0;
    for (size_t k = 1; k < G.size(); k++) {
        ET = (ET + (MOD - G[k]) % MOD * inv[k]) % MOD;
    }
    ll ans = 0;
    for (int i = 0; i < m; i++) {
        ans = (ans + b[i]) % MOD;
        ll Ti = 0;
        for (int k = 0; k < b[i]; k++) Ti = (Ti + inv[a[i] - k]) % MOD;
        ll c = a[i] - b[i];
        ans = (ans + c % MOD * ((ET - Ti + MOD) % MOD)) % MOD;
    }
    printf("%lld\n", ans);
    return 0;
}

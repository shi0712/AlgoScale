#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll MOD = 998244353;

ll power(ll b, ll e) {
    ll r = 1;
    b %= MOD;
    while (e) {
        if (e & 1) r = r * b % MOD;
        b = b * b % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    int T;
    scanf("%d", &T);
    while (T--) {
        int n;
        scanf("%d", &n);
        vector<int> p(n + 2), pos(n + 2);
        for (int i = 1; i <= n; i++) {
            scanf("%d", &p[i]);
            pos[p[i]] = i;
        }
        bool sorted = true;
        for (int i = 1; i <= n; i++)
            if (p[i] != i) sorted = false;
        if (sorted) {
            puts("0");
            continue;
        }
        ll x = power(n, MOD - 2);
        ll inv1mx = (ll)n % MOD * power(n - 1, MOD - 2) % MOD;
        int W = n + 2;
        vector<int> e((size_t)W * W, 0);
        vector<ll> colsum(W, 0);
        for (int u = 1; u <= n + 1; u++) {
            ll rowrun = 0;
            for (int v = 1; v <= n + 1; v++) {
                ll val = (1 + x * ((rowrun + colsum[v]) % MOD)) % MOD * inv1mx % MOD;
                e[(size_t)u * W + v] = (int)val;
                rowrun = (rowrun + val) % MOD;
                colsum[v] = (colsum[v] + val) % MOD;
            }
        }
        vector<ll> pw(n + 2);
        pw[0] = 1;
        for (int i = 1; i <= n + 1; i++) pw[i] = pw[i - 1] * ((1 + x) % MOD) % MOD;
        vector<int> pmax(n + 2, 0), smin(n + 3, n + 1);
        for (int i = 1; i <= n; i++) pmax[i] = max(pmax[i - 1], p[i]);
        smin[n + 1] = n + 1;
        for (int i = n; i >= 1; i--) smin[i] = min(smin[i + 1], p[i]);
        ll total = 0;
        for (int i = 1; i <= n - 1; i++) {
            int e0 = pmax[i], s0 = smin[i + 1];
            if (s0 > e0) continue;
            int Z = 0, O = 0;
            for (int v = s0 + 1; v <= e0; v++) {
                if (pos[v] <= i) O++;
                else if (v < e0) Z++;
            }
            ll E = pw[Z];
            int zc = 0, oc = O;
            for (int c = s0 + 1; c <= e0; c++) {
                if (pos[c] <= i) {
                    oc--;
                    ll h;
                    if (oc >= 1)
                        h = (e[(size_t)(zc + 1) * W + oc + 1] - e[(size_t)(zc + 1) * W + oc] + MOD) % MOD;
                    else
                        h = (e[(size_t)(zc + 1) * W + 1] - pw[zc] + MOD) % MOD;
                    E += h;
                } else {
                    zc++;
                }
            }
            total = (total + E) % MOD;
        }
        ll ans = (1 + x * total) % MOD;
        printf("%lld\n", ans);
    }
    return 0;
}

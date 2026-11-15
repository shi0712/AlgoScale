#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef long long ll;
const ll MOD = 998244353;

ll pw(ll b, ll e) {
    b %= MOD;
    if (b < 0) b += MOD;
    ll r = 1;
    while (e > 0) {
        if (e & 1) r = r * b % MOD;
        b = b * b % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    int n;
    scanf("%d", &n);
    const int V = 6000;
    vector<int> cnt(V + 1, 0);
    for (int i = 0; i < n; i++) {
        int a;
        scanf("%d", &a);
        cnt[a]++;
    }
    vector<int> smallP;
    for (int p = 2; p <= 77; p++) {
        bool ok = true;
        for (int d = 2; d * d <= p; d++)
            if (p % d == 0) { ok = false; break; }
        if (ok) smallP.push_back(p);
    }
    int B = smallP.size(); // 21
    auto maskOf = [&](int k) {
        int m = 0;
        for (int i = 0; i < B; i++)
            if (k % smallP[i] == 0) m |= 1 << i;
        return m;
    };
    // large primes
    vector<int> largeP;
    for (int p = 79; p <= V; p++) {
        bool ok = true;
        for (int d = 2; d * d <= p; d++)
            if (p % d == 0) { ok = false; break; }
        if (ok) largeP.push_back(p);
    }
    int N = 1 << B;
    vector<unsigned> F(N, 1);
    // smooth values: all prime factors <= 73
    for (int v = 1; v <= V; v++) {
        if (!cnt[v]) continue;
        int x = v;
        int m = 0;
        for (int i = 0; i < B; i++) {
            if (x % smallP[i] == 0) {
                m |= 1 << i;
                while (x % smallP[i] == 0) x /= smallP[i];
            }
        }
        if (x == 1) {
            F[m] = (ull)F[m] * pw(1 + v, cnt[v]) % MOD;
        }
    }
    for (int b = 0; b < B; b++) {
        int bit = 1 << b;
        for (int mask = 0; mask < N; mask++) {
            if (mask & bit) F[mask] = (ull)F[mask] * F[mask ^ bit] % MOD;
        }
    }
    vector<vector<unsigned>> G(B + 1);
    ll G0 = 1;
    for (int q : largeP) {
        int K = V / q;
        int orm = 0;
        bool any = false;
        for (int k = 1; k <= K; k++) {
            if (cnt[q * k]) {
                any = true;
                orm |= maskOf(k);
            }
        }
        if (!any) continue;
        int j = 0;
        while ((1 << j) <= orm) j++;
        int sz = 1 << j;
        vector<unsigned> A(sz, 1);
        for (int k = 1; k <= K; k++) {
            int v = q * k;
            if (cnt[v]) {
                int m = maskOf(k);
                A[m] = (ull)A[m] * pw(1 + v, cnt[v]) % MOD;
            }
        }
        for (int b = 0; b < j; b++) {
            int bit = 1 << b;
            for (int mask = 0; mask < sz; mask++)
                if (mask & bit) A[mask] = (ull)A[mask] * A[mask ^ bit] % MOD;
        }
        ll c = (ll)(q - 1) % MOD * pw(q, MOD - 2) % MOD;
        if (j == 0) {
            ll h = (1 + c * ((A[0] + MOD - 1) % MOD)) % MOD;
            G0 = G0 * h % MOD;
        } else {
            if (G[j].empty()) G[j].assign(sz, 1);
            for (int T = 0; T < sz; T++) {
                ll h = (1 + c * ((A[T] + MOD - 1) % MOD)) % MOD;
                G[j][T] = (ull)G[j][T] * h % MOD;
            }
        }
    }
    for (int j = 1; j <= B; j++) {
        if (G[j].empty()) continue;
        int msk = (1 << j) - 1;
        for (int T = 0; T < N; T++) {
            F[T] = (ull)F[T] * G[j][T & msk] % MOD;
        }
    }
    // inverse zeta (subset)
    for (int b = 0; b < B; b++) {
        int bit = 1 << b;
        for (int mask = 0; mask < N; mask++) {
            if (mask & bit) {
                unsigned x = F[mask] + MOD - F[mask ^ bit];
                if (x >= MOD) x -= MOD;
                F[mask] = x;
            }
        }
    }
    vector<unsigned> r(B);
    for (int i = 0; i < B; i++) {
        r[i] = (ll)(smallP[i] - 1) * pw(smallP[i], MOD - 2) % MOD;
    }
    vector<unsigned> wt(N);
    wt[0] = 1;
    ll ans = F[0];
    for (int mask = 1; mask < N; mask++) {
        int low = __builtin_ctz(mask);
        wt[mask] = (ull)wt[mask & (mask - 1)] * r[low] % MOD;
        ans = (ans + (ull)wt[mask] * F[mask]) % MOD;
    }
    ans = ans % MOD * G0 % MOD;
    printf("%lld\n", ans);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef vector<ll> poly;
const ll MOD = 998244353;

ll pw(ll b, ll e) {
    ll r = 1; b %= MOD; if (b < 0) b += MOD;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}

void ntt(vector<ll>& a, bool inv) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        ll w = pw(3, (MOD - 1) / len);
        if (inv) w = pw(w, MOD - 2);
        int half = len / 2;
        vector<ll> ws(half);
        ws[0] = 1;
        for (int i = 1; i < half; i++) ws[i] = ws[i - 1] * w % MOD;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; j++) {
                ll u = a[i + j], v = a[i + j + half] * ws[j] % MOD;
                a[i + j] = u + v >= MOD ? u + v - MOD : u + v;
                a[i + j + half] = u - v < 0 ? u - v + MOD : u - v;
            }
        }
    }
    if (inv) {
        ll ni = pw(n, MOD - 2);
        for (auto& x : a) x = x * ni % MOD;
    }
}

poly mul(poly a, poly b, int need) {
    int sz = 1;
    while (sz < (int)(a.size() + b.size())) sz <<= 1;
    a.resize(sz); b.resize(sz);
    ntt(a, false); ntt(b, false);
    for (int i = 0; i < sz; i++) a[i] = a[i] * b[i] % MOD;
    ntt(a, true);
    a.resize(need);
    return a;
}

poly inverse(const poly& a, int n) {
    poly b = {pw(a[0], MOD - 2)};
    int len = 1;
    while (len < n) {
        len <<= 1;
        poly ac(a.begin(), a.begin() + min((int)a.size(), len));
        poly t = mul(ac, b, len);
        poly bb = mul(b, b, len);
        (void)bb;
        // b = b*(2 - a*b)
        for (auto& x : t) x = (MOD - x) % MOD;
        t[0] = (t[0] + 2) % MOD;
        b = mul(b, t, len);
    }
    b.resize(n);
    return b;
}

poly lnp(const poly& a, int n) {
    poly d(n, 0);
    for (int i = 1; i < (int)a.size() && i <= n; i++) d[i - 1] = a[i] * i % MOD;
    poly ia = inverse(a, n);
    poly p = mul(d, ia, n);
    poly r(n, 0);
    for (int i = 1; i < n; i++) r[i] = p[i - 1] * pw(i, MOD - 2) % MOD;
    return r;
}

poly expp(const poly& a, int n) {
    poly b = {1};
    int len = 1;
    while (len < n) {
        len <<= 1;
        poly bb = b; bb.resize(len);
        poly l = lnp(bb, len);
        poly t(len);
        for (int i = 0; i < len; i++) {
            ll ai = i < (int)a.size() ? a[i] : 0;
            t[i] = ((ai - l[i]) % MOD + MOD) % MOD;
        }
        t[0] = (t[0] + 1) % MOD;
        b = mul(bb, t, len);
    }
    b.resize(n);
    return b;
}

int main() {
    int n;
    scanf("%d", &n);
    vector<int> a(n);
    ll m = 0;
    for (auto& x : a) { scanf("%d", &x); m += x; }
    int K = m / 2;
    vector<ll> cnt(n + 2, 0);
    for (int x : a) cnt[x + 1]++;
    vector<ll> s(K + 1, 0);
    for (int d = 1; d <= min<ll>(K, n + 1); d++) {
        if (!cnt[d]) continue;
        ll v = (ll)d * cnt[d] % MOD;
        for (int j = d; j <= K; j += d) s[j] = (s[j] + v) % MOD;
    }
    poly g(K + 1, 0);
    // inverses
    vector<ll> iv(K + 2, 1);
    for (int i = 2; i <= K + 1; i++) iv[i] = (MOD - (MOD / i) * iv[MOD % i] % MOD) % MOD;
    for (int j = 1; j <= K; j++) {
        ll h = ((ll)n % MOD - s[j] + MOD) % MOD;
        g[j] = h * iv[j] % MOD;
    }
    poly f = expp(g, K + 1);
    // factorials
    vector<ll> fact(m + 1), ifact(m + 1);
    fact[0] = 1;
    for (ll i = 1; i <= m; i++) fact[i] = fact[i - 1] * i % MOD;
    ifact[m] = pw(fact[m], MOD - 2);
    for (ll i = m; i > 0; i--) ifact[i - 1] = ifact[i] * i % MOD;
    auto C = [&](ll nn, ll k) -> ll {
        if (k < 0 || k > nn) return 0;
        return fact[nn] * ifact[k] % MOD * ifact[nn - k] % MOD;
    };
    ll ans = 0;
    for (int k = 0; k <= K; k++) {
        ll kk = (f[k] - (k ? f[k - 1] : 0) + MOD) % MOD;
        ll ff = (C(m, k) - C(m, k - 1) + MOD) % MOD;
        ans = (ans + kk * ff) % MOD;
    }
    printf("%lld\n", ans);
    return 0;
}

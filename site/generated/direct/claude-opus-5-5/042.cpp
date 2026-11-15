#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll MOD = 998244353;

ll pw(ll b, ll e) {
    ll r = 1; b %= MOD;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}

struct Key { int t, o; ll lo, hi, base; };

int main() {
    ll n, k;
    scanf("%lld %lld", &n, &k);
    int K2 = (int)k + 100;
    vector<int> f(K2 + 1), ifact(K2 + 1), inv(K2 + 2);
    f[0] = 1;
    for (int i = 1; i <= K2; i++) f[i] = (ll)f[i - 1] * i % MOD;
    ifact[K2] = pw(f[K2], MOD - 2);
    for (int i = K2; i >= 1; i--) ifact[i - 1] = (ll)ifact[i] * i % MOD;
    for (int i = 1; i <= K2; i++) inv[i] = (ll)ifact[i] * f[i - 1] % MOD;

    // pwf[s][N] = s^N / N!
    vector<vector<int>> pwf(n + 1, vector<int>(K2 + 1, 0));
    for (int s = 0; s <= n; s++) {
        ll p = 1;
        for (int N = 0; N <= K2; N++) {
            pwf[s][N] = p * ifact[N] % MOD;
            p = p * s % MOD;
        }
    }
    ll C[12][12];
    memset(C, 0, sizeof(C));
    for (int i = 0; i < 12; i++) {
        C[i][0] = 1;
        for (int j = 1; j <= i; j++) C[i][j] = (C[i - 1][j - 1] + (j <= i - 1 ? C[i - 1][j] : 0)) % MOD;
    }

    const int OFF = 100;
    static int kid[12][260];
    memset(kid, -1, sizeof(kid));
    vector<Key> keys;

    auto rng = [&](int t, int o, ll &lo, ll &hi) -> bool {
        if (t > n - 2) return false;
        if (k + o < 1) return false;
        lo = (k + o + n - 1) / n;
        hi = (k + o - 1) / (t + 1);
        return hi >= lo;
    };

    ll S = 0;
    if (n >= 2) {
        queue<pair<int, int>> q;
        ll lo, hi;
        if (rng(0, 0, lo, hi)) {
            kid[0][OFF] = 0;
            keys.push_back({0, 0, lo, hi, 0});
            q.push({0, 0});
        }
        while (!q.empty()) {
            auto [t, o] = q.front(); q.pop();
            auto add = [&](int t2, int o2) {
                if (t2 > n) return;
                if (o2 + OFF < 0 || o2 + OFF >= 260) return;
                if (kid[t2][o2 + OFF] != -1) return;
                ll l2, h2;
                if (!rng(t2, o2, l2, h2)) return;
                kid[t2][o2 + OFF] = keys.size();
                keys.push_back({t2, o2, l2, h2, 0});
                q.push({t2, o2});
            };
            for (int u = 0; u < t; u++) add(t + 1, o + u);
            for (int j = 1; j <= n - t; j++) add(t + j, o - j);
        }
        ll total = 0;
        for (auto &ky : keys) {
            ky.base = total - ky.lo;
            total += ky.hi - ky.lo + 1;
        }
        vector<int> store(total + 1);
        vector<int> order(keys.size());
        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int a, int b) { return keys[a].t > keys[b].t; });

        auto get = [&](int t, int o, ll c) -> ll {
            ll N = k - (ll)t * c + o;
            if (N < 0) return 0;
            int s = n - t;
            if (N <= c) return pwf[s][N];
            if (N > (ll)s * c) return 0;
            return store[keys[kid[t][o + OFF]].base + c];
        };

        for (int id : order) {
            Key &ky = keys[id];
            int t = ky.t, o = ky.o;
            int s = n - t;
            for (ll cp = ky.hi; cp >= ky.lo; --cp) {
                ll c = cp + 1;
                ll w = ifact[c];
                ll N0 = k - (ll)t * c + o, N1 = N0 + t;
                ll cur, N;
                if (N0 >= 0) { cur = get(t, o, c); N = N0; }
                else { cur = 1; N = 0; }
                for (; N < N1; N++) {
                    ll low = 0;
                    if (N >= c) low = get(t + 1, o + (int)(N - N0), c);
                    ll v = (cur - low * w % MOD + MOD) % MOD;
                    cur = (ll)s * inv[N + 1] % MOD * v % MOD;
                }
                ll res = cur, wp = 1;
                for (int j = 1; j <= s; j++) {
                    wp = wp * w % MOD;
                    ll Nj = N1 - (ll)j * c;
                    if (Nj < 0) break;
                    ll term = C[s][j] * wp % MOD * get(t + j, o - j, cp) % MOD;
                    res = (res - term + MOD) % MOD;
                }
                store[ky.base + cp] = (int)res;
            }
        }
        if (!keys.empty() && keys[0].t == 0 && keys[0].o == 0) {
            Key &tp = keys[0];
            for (ll c = tp.lo; c <= tp.hi; c++) S = (S + store[tp.base + c]) % MOD;
        }
    }
    ll ans = ((k % MOD) * pw(n, k) % MOD - (ll)f[k] * S % MOD + MOD) % MOD;
    printf("%lld\n", ans);
    return 0;
}

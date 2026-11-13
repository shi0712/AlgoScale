#include <bits/stdc++.h>
using namespace std;

using ll = long long;

constexpr int MOD = 998244353;
constexpr int MAXN = 4005;

ll modpow(ll a, ll e) {
    ll r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

ll fact[MAXN], invfact[MAXN], dfact[MAXN];

void precompute() {
    fact[0] = 1;
    for (int i = 1; i < MAXN; ++i)
        fact[i] = fact[i - 1] * i % MOD;

    invfact[MAXN - 1] = modpow(fact[MAXN - 1], MOD - 2);
    for (int i = MAXN - 2; i >= 0; --i)
        invfact[i] = invfact[i + 1] * (i + 1) % MOD;

    dfact[0] = 1;
    for (int i = 2; i < MAXN; i += 2)
        dfact[i] = dfact[i - 2] * (i - 1) % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    precompute();

    int T;
    cin >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        int N = 2 * m;
        vector<int> a(n);
        for (int &x : a) cin >> x;

        vector<ll> sumComb(N + 1, 0);

        for (int x : a) {
            for (int k = 2; k <= x; ++k) {
                ll c = fact[x] * invfact[k] % MOD * invfact[x - k] % MOD;
                sumComb[k] += c;
                if (sumComb[k] >= MOD) sumComb[k] -= MOD;
            }
        }

        vector<ll> R(N + 1, 0);

        for (int k = 2; k <= N; ++k) {
            ll invC = fact[k] * fact[N - k] % MOD * invfact[N] % MOD;
            R[k] = sumComb[k] * invC % MOD;
        }

        vector<ll> reach(N + 1, 0);
        reach[N] = 1;

        ll invMatchN = modpow(dfact[N], MOD - 2);
        ll ans = 0;

        int half = N / 2;

        for (int k = N; k >= 2; --k) {
            if (reach[k] == 0) continue;

            int cmin = max(0, k - half);
            int cmax = k / 2;

            ll p0 = 0;

            if (cmin == 0) {
                int r = N - 2 * k;
                p0 = fact[N - k] * invfact[r] % MOD * dfact[r] % MOD * invMatchN % MOD;
            }

            ll q = (1 - p0 + MOD) % MOD;
            ll invq = modpow(q, MOD - 2);

            ll occupation = reach[k] * invq % MOD;
            ll coeff = (1 - R[k] + MOD) % MOD;
            ans = (ans + coeff * occupation) % MOD;

            int cstart = max(1, cmin);

            for (int c = cstart; c <= cmax; ++c) {
                int r = N - 2 * k + 2 * c;

                ll p = fact[k] * invfact[2 * c] % MOD * invfact[k - 2 * c] % MOD;
                p = p * dfact[2 * c] % MOD;
                p = p * fact[N - k] % MOD * invfact[r] % MOD;
                p = p * dfact[r] % MOD * invMatchN % MOD;

                ll leaveProb = p * invq % MOD;
                int nxt = k - c;

                reach[nxt] += reach[k] * leaveProb % MOD;
                if (reach[nxt] >= MOD) reach[nxt] -= MOD;
            }
        }

        cout << ans % MOD << '\n';
    }

    return 0;
}

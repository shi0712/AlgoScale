#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
static const int MOD = 998244353;

static inline ll pw(ll b, ll e) {
    b %= MOD;
    if (b < 0) b += MOD;
    ll r = 1;
    while (e) {
        if (e & 1) r = r * b % MOD;
        b = b * b % MOD;
        e >>= 1;
    }
    return r;
}
static inline ll inv_mod(ll a) { return pw(a, MOD - 2); }

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    vector<int> p;
    while (T--) {
        int n;
        scanf("%d", &n);
        p.resize(n);
        for (int i = 0; i < n; i++) scanf("%d", &p[i]);

        if (n == 1) { puts("0"); continue; }
        bool sorted = true;
        for (int i = 0; i < n; i++) if (p[i] != i + 1) { sorted = false; break; }
        if (sorted) { puts("0"); continue; }

        // v = 1/(n-1), q = n/(n-1), onepv = 1+v, t = (v/(1+v))^2 = 1/n^2
        ll v = inv_mod(n - 1);
        ll q = (ll)n % MOD * v % MOD;
        ll onepv = (1 + v) % MOD;
        ll t = v * v % MOD * inv_mod(onepv * onepv % MOD) % MOD;
        ll tm1 = (t - 1 + MOD) % MOD;
        ll onepv2 = onepv * onepv % MOD;

        // powers of (1+v)
        vector<int> p1v(n + 1);
        p1v[0] = 1;
        for (int k = 1; k <= n; k++) p1v[k] = (int)((ll)p1v[k - 1] * onepv % MOD);

        int N = n;
        // U[A][B] = v*(H(A,B) + H'(A,B))
        vector<vector<int>> U(N + 1, vector<int>(N + 1, 0));
        vector<int> H0(N + 1), H1(N + 1), P0(N + 1, 0), P1(N + 1);
        for (int B = 0; B <= N; B++) {
            H0[B] = p1v[B];
            ll h1 = (ll)p1v[B] * onepv % MOD;
            if (B > 0) h1 = (h1 + (ll)B % MOD * v % MOD * v % MOD * p1v[B - 1]) % MOD;
            H1[B] = (int)h1;
            P1[B] = (int)(v * p1v[B] % MOD);
            U[0][B] = (int)(v * (ll)(H0[B] + P0[B]) % MOD);
            U[1][B] = (int)(v * (ll)((H1[B] + P1[B]) % MOD) % MOD);
        }
        vector<int> invA(N + 2);
        for (int a = 1; a <= N + 1; a++) invA[a] = (int)inv_mod(a);

        vector<int> nH(N + 1), nP(N + 1);
        for (int A = 1; A <= N - 1; A++) {
            ll invA1 = invA[A + 1];
            ll cstH = ((ll)onepv * (((2LL * A + 1) % MOD - (ll)A * t % MOD + MOD) % MOD)) % MOD * invA1 % MOD;
            ll coefH = (ll)onepv * t % MOD * invA1 % MOD;
            ll c2H = onepv2 * ((ll)A % MOD) % MOD * tm1 % MOD * invA1 % MOD;
            ll cstP = ((ll)onepv * ((2LL * A % MOD - (ll)(A - 1) * t % MOD + MOD) % MOD)) % MOD * invA[A] % MOD;
            ll coefP = (ll)onepv * t % MOD * invA[A] % MOD;
            ll c2P = onepv2 * tm1 % MOD;
            for (int B = 0; B <= N; B++) {
                ll c1h = (cstH + coefH * B) % MOD;
                ll nh = (c1h * H1[B] + c2H * H0[B]) % MOD;
                ll c1p = (cstP + coefP * B) % MOD;
                ll np = (c1p * P1[B] + c2P * P0[B]) % MOD;
                nH[B] = (int)nh;
                nP[B] = (int)np;
                U[A + 1][B] = (int)(v * ((nh + np) % MOD) % MOD);
            }
            H0.swap(H1); H1.swap(nH);
            P0.swap(P1); P1.swap(nP);
        }

        vector<char> present(n, 0);
        int alpha = 0, r = -1;
        ll total = 0;
        for (int i = 1; i <= n; i++) {
            int x = p[i - 1] - 1;
            present[x] = 1;
            if (x == alpha) { while (alpha < n && present[alpha]) alpha++; }
            if (x > r) r = x;
            if (alpha == i) continue;  // already sorted state
            int lo = alpha + 1, hi = r - 1;
            int ones = 0;
            for (int pos = lo; pos <= hi; pos++) if (present[pos]) ones++;
            int Z = (hi - lo + 1) - ones;
            ll f = p1v[Z];
            int zeros_seen = 0, ones_seen = 0;
            for (int pos = lo; pos <= hi; pos++) {
                if (present[pos]) {
                    int Aq = zeros_seen, Bq = ones - ones_seen - 1;
                    f += U[Aq][Bq];
                    if (f >= MOD) f -= MOD;
                    ones_seen++;
                } else zeros_seen++;
            }
            total = (total + q * f) % MOD;
        }
        ll ans = (1 + total * inv_mod(n)) % MOD;
        printf("%lld\n", ans);
    }
    return 0;
}

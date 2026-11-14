#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef long long ll;
static const u64 MOD = 998244353ULL;

static u64 pw(u64 b, u64 e) {
    u64 r = 1; b %= MOD;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}

static vector<u64> fact, ifact, inv_;

static char buf[1 << 25];
static int bufLen = 0, bufPos = 0;
static inline int readChar() {
    if (bufPos == bufLen) { bufLen = (int)fread(buf, 1, sizeof(buf), stdin); bufPos = 0; if (bufLen <= 0) return -1; }
    return buf[bufPos++];
}
static inline ll readInt() {
    int c = readChar();
    while (c != '-' && (c < '0' || c > '9')) c = readChar();
    bool neg = false; if (c == '-') { neg = true; c = readChar(); }
    ll x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = readChar(); }
    return neg ? -x : x;
}

// One forward Y-step on vector v with support [lo,hi] (v is zero outside), parameter beta.
// v_new[d] = v[d]*(beta-d) + v[d+1]*(d+1), for d in [max(0,lo-1), hi]. Normalization 1/(N+1) handled by caller.
static inline void ystep(vector<u64>& v, int& lo, int hi, ll beta) {
    int nlo = lo > 0 ? lo - 1 : 0;
    ll c = beta - nlo;
    c %= (ll)MOD; if (c < 0) c += MOD;
    u64 cd = (u64)c;
    u64* p = v.data();
    for (int d = nlo; d <= hi; ++d) {
        u64 x = p[d] * cd + p[d + 1] * (u64)(d + 1);
        p[d] = x % MOD;
        cd = (cd == 0) ? MOD - 1 : cd - 1;
    }
    lo = nlo;
}

// levels: (p,q) in descending value order.
static u64 runDP(int K, int n_nu, int rho, const vector<pair<int,int>>& lv, ll m) {
    int K2 = K - n_nu;
    vector<u64> h(K2 + 3, 0), G(K2 + 3, 0);
    h[K2] = 1;
    u64 scale = 1;
    int lo = K2, hi = K2;
    ll NX = 0, NY = 0;
    ll pendP = 0, pendAlpha = 0, pendBase = 0;
    bool dead = false;
    auto trimHi = [&](ll yrem) {
        if (yrem < hi) {
            int nh = (int)max<ll>(yrem, -1);
            for (int d = nh + 1; d <= hi; ++d) h[d] = 0;
            hi = nh;
        }
        if (hi < lo) dead = true;
    };
    auto flushX = [&]() {
        if (pendP == 0) return;
        scale = scale * fact[pendBase - 1] % MOD * ifact[pendBase + pendP - 1] % MOD;
        for (int d = lo; d <= hi; ++d) {
            ll a = pendAlpha - d;
            if (a == 0) h[d] = 0;
            else h[d] = h[d] * fact[a + pendP - 1] % MOD * ifact[a - 1] % MOD;
        }
        pendP = 0;
    };
    for (size_t li = 0; li < lv.size() && !dead; ++li) {
        int p = lv[li].first, q = lv[li].second;
        if (q == 0) {
            if (pendP == 0) { pendAlpha = NX + K - n_nu; pendBase = NX + K; }
            pendP += p; NX += p;
            continue;
        }
        flushX();
        if (p == 0) {
            for (int t = 0; t < q; ++t) {
                ll N = NY + t;
                ystep(h, lo, hi, N + 1 - rho);
                scale = scale * inv_[N + 1] % MOD;
                trimHi(m - (N + 1));
                if (dead) break;
            }
            NY += q;
        } else {
            int S = min(p, q);
            ll alpha = NX + K - n_nu, base = NX + K;
            // G[d] = h[d] * ifact[d+n_nu-1] * ifact[alpha-d-1] for regular d
            int specA = -1, specB = -1; // d with d+n_nu==0 ; d with alpha-d==0
            for (int d = lo; d <= hi; ++d) {
                ll x1 = d + n_nu, x2 = alpha - d;
                if (x1 == 0) { specA = d; G[d] = 0; continue; }
                if (x2 == 0) { specB = d; G[d] = 0; continue; }
                G[d] = h[d] * ifact[x1 - 1] % MOD * ifact[x2 - 1] % MOD;
            }
            // invRising(base,p) = fact[base-1]*ifact[base+p-1]
            u64 invRb = fact[base - 1] * ifact[base + p - 1] % MOD;
            // c_s = C(p,s)*falling(q,s)*invRb*fact[NY+q-s]*ifact[NY+q]
            vector<u64> w(K2 + 3, 0);
            int wlo = lo, whi = hi;
            u64 cinv = 1; // product of (N+1) of applied steps
            for (int s = 0; s <= S; ++s) {
                if (s > 0) {
                    ll N = NY + q - s;
                    ystep(w, wlo, whi, N + 1 - rho);
                    cinv = cinv * (u64)((N + 1) % MOD) % MOD;
                }
                u64 cs = fact[p] * ifact[p - s] % MOD * ifact[s] % MOD * fact[q] % MOD * ifact[q - s] % MOD
                         * invRb % MOD * fact[NY + q - s] % MOD * ifact[NY + q] % MOD * cinv % MOD;
                int r = p - s;
                for (int d = lo; d <= hi; ++d) {
                    if (d == specA || d == specB) continue;
                    ll x1 = d + n_nu, x2 = alpha - d;
                    u64 term = fact[x1 + s - 1] * fact[x2 + r - 1] % MOD * G[d] % MOD;
                    w[d] = (w[d] + term * cs) % MOD;
                }
                // special entries
                if (specA >= 0) {
                    int d = specA; ll x2 = alpha - d;
                    // R1 = [s==0]; R2 = rising(x2, r)
                    if (s == 0) {
                        u64 R2;
                        if (x2 == 0) R2 = (r == 0) ? 1 : 0;
                        else R2 = fact[x2 + r - 1] * ifact[x2 - 1] % MOD;
                        // term = R1*R2*h[d] / (s! * rising(base, p-s)) ; c_s already includes invfact[s]*... let's compute directly
                        // c_s includes C(p,s)falling(q,s)/(rising(base,p)) * stuff; we need tau_s*Ahat_{p-s}*kappa_s
                        // tau_0=1, Ahat_r = R2 / rising(base,r), kappa_s as defined; c_s = kappa_s * g_s * cinv with g_s = 1/(s! rising(base,p-s))
                        // so contribution = c_s * R1 * R2 * h[d] (R1 = rising(d+n_nu,s) = 1 for s=0)
                        w[d] = (w[d] + cs * R2 % MOD * h[d]) % MOD;
                    }
                }
                if (specB >= 0 && specB != specA) {
                    int d = specB; ll x1 = d + n_nu;
                    // R2 = [r==0]; R1 = rising(x1, s)
                    if (r == 0) {
                        u64 R1;
                        if (x1 == 0) R1 = (s == 0) ? 1 : 0;
                        else R1 = fact[x1 + s - 1] * ifact[x1 - 1] % MOD;
                        w[d] = (w[d] + cs * R1 % MOD * h[d]) % MOD;
                    }
                }
            }
            for (int t = 0; t < q - S; ++t) {
                ll N = NY + t;
                ystep(w, wlo, whi, N + 1 - rho);
                cinv = cinv * (u64)((N + 1) % MOD) % MOD;
            }
            // total normalization: product over all q steps of inv(N+1) = fact[NY]*ifact[NY+q]
            scale = scale * fact[NY] % MOD * ifact[NY + q] % MOD;
            h.swap(w);
            lo = wlo; hi = whi;
            NX += p; NY += q;
            trimHi(m - NY);
        }
    }
    if (dead) return 0;
    flushX();
    if (lo > 0 || hi < 0) return 0;
    return scale * h[0] % MOD;
}

int main() {
    ll n = readInt(), m = readInt(), k = readInt();
    const int MAXV = 100000;
    vector<int> ca(MAXV + 2, 0), cb(MAXV + 2, 0);
    for (ll i = 0; i < n; ++i) { ll v = readInt(); ca[v]++; }
    for (ll i = 0; i < m; ++i) { ll v = readInt(); cb[v]++; }
    int K = (int)k + 1;
    ll LIM = n + m + K + 10;
    fact.assign(LIM + 1, 1); ifact.assign(LIM + 1, 1); inv_.assign(LIM + 1, 1);
    for (ll i = 1; i <= LIM; ++i) fact[i] = fact[i - 1] * (u64)i % MOD;
    ifact[LIM] = pw(fact[LIM], MOD - 2);
    for (ll i = LIM; i >= 1; --i) ifact[i - 1] = ifact[i] * (u64)i % MOD;
    for (ll i = 1; i <= LIM; ++i) inv_[i] = fact[i - 1] * ifact[i] % MOD;
    vector<pair<int,int>> lv;
    for (int v = MAXV; v >= 1; --v) if (ca[v] || cb[v]) lv.push_back({ca[v], cb[v]});
    u64 A = runDP(K, 0, 0, lv, m);       // P(Y wins)
    u64 B = runDP(K, 1, 1, lv, m);       // P(draw)
    u64 X = (2 * MOD + 1 - A - B) % MOD;
    printf("%llu\n%llu\n%llu\n", X, A, B);
    return 0;
}

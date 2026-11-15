#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;
static const ll MOD = 1000000007LL;

int n;
vector<ll> Ls, Rs, Ms;
ll Total;
vector<ll> invs;

static ll A0[4004], A1[4004], A2[4004];
static ll B0[4004], B1[4004], B2[4004];
static ll TMP[4004];

/* multiply polynomial p (binomial basis C(t,k)) by (alpha + t) */
static inline int mulPlus(const ll* p, int dp, ll alpha, ll* out) {
    if (dp < 0) { out[0] = 0; return -1; }
    out[0] = alpha * p[0] % MOD;
    for (int k = 1; k <= dp; k++) {
        ll diag = alpha + k; if (diag >= MOD) diag -= MOD;
        out[k] = (diag * (ull)p[k] + (ull)k * (ull)p[k - 1]) % MOD;
    }
    out[dp + 1] = (ull)(dp + 1) * (ull)p[dp] % MOD;
    return dp + 1;
}
/* multiply polynomial p by (alpha - t) */
static inline int mulMinus(const ll* p, int dp, ll alpha, ll* out) {
    if (dp < 0) { out[0] = 0; return -1; }
    out[0] = alpha * p[0] % MOD;
    for (int k = 1; k <= dp; k++) {
        ll diag = alpha - k; if (diag < 0) diag += MOD;
        ll v = (ll)(diag * (ull)p[k]) - (ll)((ull)k * (ull)p[k - 1]);
        v %= MOD; if (v < 0) v += MOD;
        out[k] = v;
    }
    ll last = (ull)(dp + 1) * (ull)p[dp] % MOD;
    out[dp + 1] = (MOD - last) % MOD;
    return dp + 1;
}

vector<int> tokType, tokIdx;
vector<ll> tokVal;

vector<ll> prePoly, sufPoly;
vector<int> preOff, sufOff, preDeg, sufDeg;
vector<ll> prefM, suffM;
vector<ll> cA, cB;

/* prefix independence polynomials for all prefix lengths, all positions treated as middle */
void buildPrefix(const vector<ll>& L, const vector<ll>& R, vector<ll>& poly, vector<int>& off, vector<int>& deg) {
    int nn = (int)L.size();
    poly.assign((size_t)(nn + 1) * (nn + 2) / 2 + 8, 0);
    off.assign(nn + 1, 0); deg.assign(nn + 1, -1);
    int cur = 0;
    for (int k = 0; k <= nn; k++) { off[k] = cur; cur += (k + 1); }
    ll *s0 = A0, *s1 = A1, *s2 = A2, *t0 = B0, *t1 = B1, *t2 = B2;
    int d0 = -1, d1 = -1, d2 = 0; s2[0] = 1;
    {   // k = 0
        int dt = 0; deg[0] = 0;
        for (int j = 0; j <= dt; j++) poly[off[0] + j] = 1;
    }
    for (int i = 0; i < nn; i++) {
        ll l = L[i], h = R[i] + 1;              // w0 = v - l, w1 = h - v
        int nd0 = mulMinus(s2, d2, h, t0);      // w1 * s2
        ll al = (MOD - l % MOD) % MOD;
        int nd1 = mulPlus(s0, d0, al, t1);      // w0 * s0
        int dt = max(d1, d2);
        for (int k = 0; k <= dt; k++) { ll v = (k <= d1 ? s1[k] : 0) + (k <= d2 ? s2[k] : 0); if (v >= MOD) v -= MOD; TMP[k] = v; }
        int nd2 = mulPlus(TMP, dt, al, t2);     // w0 * (s1+s2)
        d0 = nd0; d1 = nd1; d2 = nd2;
        swap(s0, t0); swap(s1, t1); swap(s2, t2);
        int kk = i + 1;
        int dT = max(d0, max(d1, d2)); deg[kk] = dT;
        for (int j = 0; j <= dT; j++) {
            ll v = (j <= d0 ? s0[j] : 0) + (j <= d1 ? s1[j] : 0);
            if (v >= MOD) v -= MOD;
            v += (j <= d2 ? s2[j] : 0); if (v >= MOD) v -= MOD;
            poly[(size_t)off[kk] + j] = v;
        }
    }
}
/* out[j] = C(X,j), j=0..upto+1 */
static void binomSeq(ll X, int upto, vector<ll>& out) {
    out.assign(upto + 2, 0); out[0] = 1;
    ll x = X % MOD;
    for (int j = 1; j <= upto + 1; j++) {
        ll num = (x - (ll)(j - 1)) % MOD; if (num < 0) num += MOD;
        out[j] = out[j - 1] * num % MOD * invs[j] % MOD;
    }
}

int main() {
    int TT;
    if (scanf("%d", &TT) != 1) return 0;
    while (TT--) {
        if (scanf("%d", &n) != 1) return 0;
        Ls.resize(n); Rs.resize(n); Ms.resize(n);
        for (int i = 0; i < n; i++) scanf("%lld", &Ls[i]);
        for (int i = 0; i < n; i++) scanf("%lld", &Rs[i]);
        Total = 1; ll maxR = 0;
        vector<ll> bps; bps.reserve(2 * n + 5); bps.push_back(1);
        for (int i = 0; i < n; i++) {
            Ms[i] = Rs[i] - Ls[i] + 1;
            Total = Total * (Ms[i] % MOD) % MOD;
            if (Rs[i] > maxR) maxR = Rs[i];
            bps.push_back(Ls[i] + 1); bps.push_back(Rs[i] + 1);
        }
        bps.push_back(maxR + 1);
        sort(bps.begin(), bps.end());
        bps.erase(unique(bps.begin(), bps.end()), bps.end());
        invs.assign(n + 3, 1);
        for (int i = 2; i < (int)invs.size(); i++) invs[i] = (MOD - (MOD / i) * invs[MOD % i] % MOD) % MOD;

        buildPrefix(Ls, Rs, prePoly, preOff, preDeg);
        {
            vector<ll> rL(n), rR(n);
            for (int i = 0; i < n; i++) { rL[i] = Ls[n - 1 - i]; rR[i] = Rs[n - 1 - i]; }
            vector<ll> poly; vector<int> off, deg;
            buildPrefix(rL, rR, poly, off, deg);
            sufPoly = move(poly);
            sufOff.assign(n + 1, 0); sufDeg.assign(n + 1, -1);
            for (int k = 0; k <= n; k++) { sufOff[k] = off[n - k]; sufDeg[k] = deg[n - k]; }
        }
        prefM.assign(n + 1, 1); suffM.assign(n + 1, 1);
        for (int i = 0; i < n; i++) prefM[i + 1] = prefM[i] * (Ms[i] % MOD) % MOD;
        for (int i = n - 1; i >= 0; i--) suffM[i] = suffM[i + 1] * (Ms[i] % MOD) % MOD;

        ll ans = 0;
        ll *s0 = A0, *s1 = A1, *s2 = A2, *t0 = B0, *t1 = B1, *t2 = B2;
        for (size_t si = 0; si + 1 < bps.size(); si++) {
            ll A = bps[si], B = bps[si + 1] - 1;
            if (A > B) continue;
            if (A > maxR) break;
            if (B > maxR) B = maxR;
            if (A > B) continue;
            ll len = B - A + 1;

            // classification: 0 middle, 1 forced high (single), 2 forced low (single), 3 forced low run
            tokType.clear(); tokIdx.clear(); tokVal.clear();
            int d = 0; bool bad = false;
            int i = 0;
            while (i < n) {
                if (B <= Ls[i]) {                       // forced high
                    int j = i; while (j + 1 < n && B <= Ls[j + 1]) j++;
                    if (j - i + 1 >= 2) { bad = true; break; }
                    tokType.push_back(1); tokIdx.push_back(i); tokVal.push_back(Ms[i]); i = j + 1;
                } else if (A > Rs[i]) {                 // forced low
                    int j = i; while (j + 1 < n && A > Rs[j + 1]) j++;
                    if (j - i + 1 == 1) { tokType.push_back(2); tokIdx.push_back(i); tokVal.push_back(Ms[i]); }
                    else {
                        ll C = 1;
                        for (int k = i; k <= j; k++) C = C * (Ms[k] % MOD) % MOD;
                        tokType.push_back(3); tokIdx.push_back(i); tokVal.push_back(C);
                    }
                    i = j + 1;
                } else {                                // middle
                    tokType.push_back(0); tokIdx.push_back(i); tokVal.push_back(0); d++; i++;
                }
            }
            if (bad) { ans = (ans + len % MOD * Total) % MOD; continue; }

            // prefix / suffix structure (all forced-low on one side, no forced-high)
            bool fast = false; int fastK = -1; ll fastWeight = 1; bool rev = false;
            {
                bool anyF1 = false;
                for (size_t t = 0; t < tokType.size(); t++) if (tokType[t] == 1) { anyF1 = true; break; }
                if (!anyF1) {
                    bool allMiddleFirst = true, allF0First = true, seenNZ = false, seenM = false;
                    for (size_t t = 0; t < tokType.size(); t++) {
                        if (tokType[t] != 0) seenNZ = true;
                        else if (seenNZ) allMiddleFirst = false;
                    }
                    for (size_t t = 0; t < tokType.size(); t++) {
                        if (tokType[t] == 0) seenM = true;
                        else if (seenM) allF0First = false;
                    }
                    if (allMiddleFirst) { fast = true; rev = false; fastK = d; fastWeight = suffM[d]; }
                    else if (allF0First) { fast = true; rev = true; fastK = n - d; fastWeight = prefM[n - d]; }
                }
            }
            if (fast) {
                const vector<ll>& poly = rev ? sufPoly : prePoly;
                const vector<int>& off = rev ? sufOff : preOff;
                const vector<int>& deg = rev ? sufDeg : preDeg;
                int kk = rev ? (n - d) : d;
                int dg = deg[kk];
                ll sum = 0;
                binomSeq(A, dg, cA);
                binomSeq(B + 1, dg, cB);
                for (int j = 0; j <= dg; j++) {
                    ll c = poly[(size_t)off[kk] + j];
                    if (!c) continue;
                    ll df = cB[j + 1] - cA[j + 1]; df %= MOD; if (df < 0) df += MOD;
                    sum = (sum + c * df) % MOD;
                }
                ll segsum = fastWeight * sum % MOD;
                ans = (ans + (len % MOD) * Total % MOD - segsum + MOD) % MOD;
                continue;
            }

            bool scalar = ((ll)n * len <= (ll)d * d / 2 + 4);
            if (scalar) {
                ll segsum = 0;
                for (ll v = A; v <= B; v++) {
                    ll c0 = 0, c1 = 0, c2 = 1;
                    for (size_t t = 0; t < tokType.size(); t++) {
                        int ty = tokType[t];
                        if (ty == 0) {
                            int idx = tokIdx[t];
                            ll w0 = (v - Ls[idx]) % MOD, w1 = (Rs[idx] + 1 - v) % MOD;
                            ll n0 = w1 * c2 % MOD;
                            ll n1 = w0 * c0 % MOD;
                            ll n2 = w0 * ((c1 + c2) % MOD) % MOD;
                            c0 = n0; c1 = n1; c2 = n2;
                        } else if (ty == 1) {
                            ll m = tokVal[t] % MOD; c0 = m * c2 % MOD; c1 = 0; c2 = 0;
                        } else if (ty == 2) {
                            ll m = tokVal[t] % MOD;
                            ll n1 = m * c0 % MOD, n2 = m * ((c1 + c2) % MOD) % MOD;
                            c0 = 0; c1 = n1; c2 = n2;
                        } else {
                            ll C = tokVal[t] % MOD;
                            ll s = ((c0 + c1) % MOD + c2) % MOD;
                            c0 = 0; c1 = 0; c2 = C * s % MOD;
                        }
                    }
                    segsum = (segsum + ((c0 + c1) % MOD + c2) % MOD) % MOD;
                }
                ans = (ans + len % MOD * Total % MOD - segsum + MOD) % MOD;
            } else {
                int d0 = -1, d1 = -1, d2 = 0; s2[0] = 1;
                for (size_t t = 0; t < tokType.size(); t++) {
                    int ty = tokType[t];
                    if (ty == 0) {
                        int idx = tokIdx[t]; ll l = A - Ls[idx], h = Rs[idx] + 1 - A;
                        int nd0 = mulMinus(s2, d2, h, t0);
                        int nd1 = mulPlus(s0, d0, l, t1);
                        int dt = max(d1, d2);
                        for (int k = 0; k <= dt; k++) { ll v = (k <= d1 ? s1[k] : 0) + (k <= d2 ? s2[k] : 0); if (v >= MOD) v -= MOD; TMP[k] = v; }
                        int nd2 = mulPlus(TMP, dt, l, t2);
                        d0 = nd0; d1 = nd1; d2 = nd2;
                        swap(s0, t0); swap(s1, t1); swap(s2, t2);
                    } else if (ty == 1) {
                        ll m = tokVal[t] % MOD;
                        for (int k = 0; k <= d2; k++) t0[k] = s2[k] * m % MOD;
                        d0 = d2; d1 = -1; d2 = -1; swap(s0, t0);
                    } else if (ty == 2) {
                        ll m = tokVal[t] % MOD; int dt = max(d1, d2);
                        for (int k = 0; k <= dt; k++) { ll v = (k <= d1 ? s1[k] : 0) + (k <= d2 ? s2[k] : 0); if (v >= MOD) v -= MOD; TMP[k] = v; }
                        for (int k = 0; k <= d0; k++) t1[k] = s0[k] * m % MOD;
                        for (int k = 0; k <= dt; k++) t2[k] = TMP[k] * m % MOD;
                        d1 = d0; d2 = dt; d0 = -1; swap(s1, t1); swap(s2, t2);
                    } else {
                        ll C = tokVal[t] % MOD; int dt = max(d0, max(d1, d2));
                        for (int k = 0; k <= dt; k++) {
                            ll v = (k <= d0 ? s0[k] : 0) + (k <= d1 ? s1[k] : 0); if (v >= MOD) v -= MOD;
                            v += (k <= d2 ? s2[k] : 0); if (v >= MOD) v -= MOD;
                            t2[k] = v * C % MOD;
                        }
                        d2 = dt; d0 = -1; d1 = -1; swap(s2, t2);
                    }
                }
                int dt = max(d0, max(d1, d2));
                ll binom = 1, segsum = 0, lm = len % MOD;
                for (int k = 0; k <= dt; k++) {
                    ll v = (k <= d0 ? s0[k] : 0) + (k <= d1 ? s1[k] : 0); if (v >= MOD) v -= MOD;
                    v += (k <= d2 ? s2[k] : 0); if (v >= MOD) v -= MOD;
                    ll num = lm - k; if (num < 0) num += MOD;
                    binom = binom * num % MOD * invs[k + 1] % MOD;
                    segsum = (segsum + v * binom) % MOD;
                }
                ans = (ans + (len % MOD) * Total % MOD - segsum + MOD) % MOD;
            }
        }
        printf("%lld\n", ans % MOD);
    }
    return 0;
}

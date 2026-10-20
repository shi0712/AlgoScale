#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;
static const int MOD = 1000000007;

static int n;
static vector<int> A, B;

/* ---------- interval DAG ---------- */
static vector<int> ivS, ivE;
static vector<array<int, 3>> ivRoots; // up to 3 candidate roots (middle +/-2)
static vector<int> ivNr;
static unordered_map<ll, int> idmap;

static int getId(int s, int e) {
    if (s > e) return -1;
    ll key = (ll)s * (n + 1) + e;
    auto it = idmap.find(key);
    if (it != idmap.end()) return it->second;
    int id = (int)ivS.size();
    idmap[key] = id;
    ivS.push_back(s);
    ivE.push_back(e);
    ivRoots.push_back({-1, -1, -1});
    ivNr.push_back(0);
    for (int m = s; m <= e; m++)
        if (abs((m - s) - (e - m)) <= 2) ivRoots[id][ivNr[id]++] = m;
    for (int r = 0; r < ivNr[id]; r++) { int m = ivRoots[id][r]; getId(s, m - 1); getId(m + 1, e); }
    return id;
}
static int lid(int s, int e) {
    if (s > e) return -1;
    return idmap[(ll)s * (n + 1) + e];
}

static vector<ll> fact, invfact, Bh; // Bh[m] = B^+_m / m!

static ll pw(ll b, ll e) {
    ll r = 1; b %= MOD;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}

struct Blk { ll lo, hi; int deg; int off; };

int main() {
    if (scanf("%d", &n) != 1) return 0;
    A.resize(n); B.resize(n);
    for (int i = 0; i < n; i++) { if (scanf("%d %d", &A[i], &B[i]) != 2) return 0; }

    idmap.reserve(40000);
    getId(0, n - 1);
    int M = (int)ivS.size();

    int maxd = n + 3;
    fact.assign(maxd + 1, 1); invfact.assign(maxd + 1, 1);
    for (int i = 1; i <= maxd; i++) fact[i] = fact[i - 1] * i % MOD;
    invfact[maxd] = pw(fact[maxd], MOD - 2);
    for (int i = maxd; i >= 1; i--) invfact[i - 1] = invfact[i] * i % MOD;
    // Bernoulli numbers (B^-: B_0=1, sum_{j=0}^{m} C(m+1,j) B_j = 0 for m>=1)
    vector<ll> Bern(maxd + 2, 0);
    Bern[0] = 1;
    for (int m = 1; m <= maxd + 1; m++) {
        ll s = 0;
        for (int j = 0; j < m; j++) {
            ll c = fact[m + 1] * invfact[j] % MOD * invfact[m + 1 - j] % MOD;
            s += c * Bern[j] % MOD;
            if ((j & 7) == 7) s %= MOD;
        }
        s %= MOD;
        Bern[m] = (MOD - s) * pw(m + 1, MOD - 2) % MOD;
    }
    Bern[1] = (MOD + 1) / 2; // B^+_1 = +1/2
    Bh.assign(maxd + 2, 0);
    for (int m = 0; m <= maxd + 1; m++) Bh[m] = Bern[m] * invfact[m] % MOD;

    vector<vector<Blk>> iblk(M);
    vector<int> pool;
    pool.reserve(1 << 21);
    vector<ll> ivTotal(M, 0);

    vector<int> order(M);
    for (int i = 0; i < M; i++) order[i] = i;
    sort(order.begin(), order.end(), [&](int x, int y) {
        int lx = ivE[x] - ivS[x], ly = ivE[y] - ivS[y];
        if (lx != ly) return lx < ly;
        return x < y;
    });

    vector<ull> fbuf(n + 4);
    vector<ll> gbuf(n + 4), tbuf(n + 4);
    vector<int> bps;
    vector<int> ptrL, ptrR, Lc, Rc;
    int one = 1;
    int cvalL[3], cvalR[3];

    int rootId = -1;

    for (int id : order) {
        int s = ivS[id], e = ivE[id];
        if (s == 0 && e == n - 1) rootId = id;
        bps.clear();
        for (int i = s; i <= e; i++) { bps.push_back(A[i]); bps.push_back(B[i] + 1); }
        sort(bps.begin(), bps.end());
        bps.erase(unique(bps.begin(), bps.end()), bps.end());
        int Kb = (int)bps.size();
        int nr = ivNr[id];
        ptrL.assign(nr, 0); ptrR.assign(nr, 0);
        Lc.assign(nr, -1); Rc.assign(nr, -1);
        for (int r = 0; r < nr; r++) {
            int m = ivRoots[id][r];
            Lc[r] = lid(s, m - 1);
            Rc[r] = lid(m + 1, e);
        }
        auto &bl = iblk[id];
        bl.clear();
        if (Kb > 1) bl.reserve(Kb - 1);
        ll base = 0;
        for (int t = 0; t + 1 < Kb; t++) {
            ll lo = bps[t], hi = (ll)bps[t + 1] - 1;
            /* --- resolve children for every active root --- */
            // codes: kindL/kindR : 0 = empty(1), 1 = zero, 2 = const, 3 = poly
            int kindL[3], kindR[3];
            int degL[3], degR[3];
            int bIdxL[3], bIdxR[3];
            const int *pLp[3], *pRp[3];
            int dfMax = -1;
            for (int r = 0; r < nr; r++) {
                int m = ivRoots[id][r];
                degL[r] = degR[r] = 0;
                bIdxL[r] = bIdxR[r] = -1;
                kindL[r] = kindR[r] = 0;
                if (!((ll)A[m] <= lo && (ll)B[m] >= hi)) { kindL[r] = -1; continue; }
                // left child : use pA
                if (Lc[r] >= 0) {
                    auto &cb = iblk[Lc[r]];
                    int c = Lc[r];
                    if (lo < cb.front().lo) { kindL[r] = 1; continue; }
                    else if (lo > cb.back().hi) { kindL[r] = 2; cvalL[r] = (int)ivTotal[c]; }
                    else {
                        int p = ptrL[r];
                        while (p + 1 < (int)cb.size() && cb[p + 1].lo <= lo) p++;
                        ptrL[r] = p;
                        kindL[r] = 3; degL[r] = cb[p].deg; bIdxL[r] = p;
                    }
                }
                // right child : use pS
                if (Rc[r] >= 0) {
                    auto &cb = iblk[Rc[r]];
                    int c = Rc[r];
                    if (lo < cb.front().lo) { kindL[r] = 1; continue; }
                    else if (lo > cb.back().hi) { kindR[r] = 2; cvalR[r] = (int)ivTotal[c]; }
                    else {
                        int p = ptrR[r];
                        while (p + 1 < (int)cb.size() && cb[p + 1].lo <= lo) p++;
                        ptrR[r] = p;
                        kindR[r] = 3; degR[r] = cb[p].deg; bIdxR[r] = p;
                    }
                }
                { int dsum = degL[r] + degR[r]; if (dsum > dfMax) dfMax = dsum; }
            }
            int paDeg;
            if (dfMax < 0) paDeg = 0; else paDeg = dfMax + 1;
            int L = paDeg + 1;
            int off = (int)pool.size();
            pool.resize(off + 2 * L);
            int *pA = pool.data() + off;
            int *pS = pA + L;
            for (int r = 0; r < nr; r++) {
                if (kindL[r] == 3) pLp[r] = pool.data() + iblk[Lc[r]][bIdxL[r]].off;
                else if (kindL[r] == 2) pLp[r] = &cvalL[r];
                else pLp[r] = &one;
                if (kindR[r] == 3) { auto &cb = iblk[Rc[r]]; pRp[r] = pool.data() + cb[bIdxR[r]].off + (cb[bIdxR[r]].deg + 1); }
                else if (kindR[r] == 2) pRp[r] = &cvalR[r];
                else pRp[r] = &one;
            }

            if (dfMax < 0) {
                pA[0] = (int)base;
                pS[0] = (int)base;
            } else {
                int df = dfMax;
                for (int k = 0; k <= df; k++) fbuf[k] = 0;
                int cnt = 0;
                for (int r = 0; r < nr; r++) {
                    if (kindL[r] < 0 || kindL[r] == 1) continue;
                    const int *pL = pLp[r]; int dL = degL[r];
                    const int *pR = pRp[r]; int dR = degR[r];
                    if (cnt) { for (int k = 0; k <= df; k++) fbuf[k] %= MOD; }
                    cnt++;
                    if (dL == 0 && pL == &one) {
                        if (dR == 0 && pR == &one) fbuf[0] += 1;
                        else for (int j = 0; j <= dR; j++) fbuf[j] += (ull)pR[j];
                    } else if (dR == 0 && pR == &one) {
                        for (int i = 0; i <= dL; i++) fbuf[i] += (ull)pL[i];
                    } else {
                        for (int i = 0; i <= dL; i++) {
                            ull ai = (ull)pL[i];
                            if (ai) {
                                ull *dst = fbuf.data() + i;
                                for (int j = 0; j <= dR; j++) dst[j] += ai * (ull)pR[j];
                            }
                            if ((i & 7) == 7) for (int k = 0; k <= df; k++) fbuf[k] %= MOD;
                        }
                    }
                }
                for (int k = 0; k <= df; k++) fbuf[k] %= MOD;

                /* --- prefix sum : T(H) = sum_{h=0}^{H} f(h) --- */
                for (int j = 0; j <= df; j++) gbuf[j] = (ll)fbuf[j] * fact[j] % MOD;
                tbuf[0] = (ll)fbuf[0];
                for (int k = 1; k <= df + 1; k++) {
                    int mmax = df + 1 - k;
                    ull acc = 0;
                    const ll *gp = gbuf.data() + k - 1;
                    int m = 0;
                    while (m <= mmax) {
                        int lim = min(mmax + 1, m + 8);
                        for (; m < lim; m++) acc += (ull)gp[m] * (ull)Bh[m];
                        acc %= MOD;
                    }
                    tbuf[k] = (ll)(acc % MOD) * invfact[k] % MOD;
                }
                ll x = (lo - 1) % MOD; if (x < 0) x += MOD;
                ll Tl = 0;
                for (int k = df + 1; k >= 0; k--) Tl = (Tl * x + tbuf[k]) % MOD;
                ll cst = (base - Tl + MOD) % MOD;
                pA[0] = (int)((tbuf[0] + cst) % MOD);
                for (int k = 1; k <= df + 1; k++) pA[k] = (int)tbuf[k];
                for (int k = 0; k <= df; k++) {
                    ll v = (ll)pA[k] - (ll)fbuf[k];
                    if (v < 0) v += MOD;
                    pS[k] = (int)v;
                }
                pS[df + 1] = pA[df + 1];
            }
            /* --- advance base : base = pA(hi) --- */
            ll y = hi % MOD;
            ll val = 0;
            for (int k = paDeg; k >= 0; k--) val = (val * y + pA[k]) % MOD;
            base = val;
            bl.push_back({lo, hi, paDeg, off});
        }
        ivTotal[id] = base;
    }

    printf("%lld\n", ivTotal[rootId] % MOD);
    return 0;
}

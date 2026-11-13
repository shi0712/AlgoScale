#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const ll NEG = -(1LL << 60);
static int K;

/* ---------- fast input ---------- */
static char rbuf[1 << 16];
static int rlen = 0, rpos = 0;
static inline int gc() {
    if (rpos >= rlen) {
        rlen = (int)fread(rbuf, 1, sizeof(rbuf), stdin);
        rpos = 0;
        if (rlen <= 0) return -1;
    }
    return rbuf[rpos++];
}
static inline int readInt() {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9')) c = gc();
    if (c == -1) return -1;
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return x;
}

struct Item { int w[10]; };

/* ---------- max-plus convolution with a concave kernel ----------
   B[t] = max_{s in [t-T, t]} A[s] + h[t-s],  h concave (h[s] = pref of sorted desc values)
   Matrix M[t][s] = A[s] + h[t-s] is Monge  =>  leftmost argmax is monotone
   => divide & conquer optimisation.                                        */
static void dcSolve(int tlo, int thi, int slo, int shi,
                    const ll* __restrict A, const ll* __restrict h, int T,
                    ll* __restrict B) {
    if (tlo > thi) return;
    int mid = (tlo + thi) >> 1;
    int lo = (slo > mid - T) ? slo : (mid - T);
    int hi = (shi < mid) ? shi : mid;
    ll best = NEG;
    int bs = lo;
    for (int s = lo; s <= hi; ++s) {
        ll v = A[s] + h[mid - s];
        if (v > best) { best = v; bs = s; }
    }
    B[mid] = best;
    dcSolve(tlo, mid - 1, slo, bs, A, h, T, B);
    dcSolve(mid + 1, thi, bs, shi, A, h, T, B);
}

static vector<ll> mergA, mergB;

/* merge one weight class p (kernel h[0..T]) into dp, in place */
static void mergeClass(vector<ll>& dp, int p, const vector<ll>& h, int T) {
    if (T <= 0) return;
    for (int res = 0; res < p && res <= K; ++res) {
        int L = (K - res) / p + 1;
        mergA.resize(L); mergB.resize(L);
        for (int s = 0, idx = res; s < L; ++s, idx += p) mergA[s] = dp[idx];
        if (T <= 12) {
            const ll* __restrict A = mergA.data();
            const ll* __restrict hh = h.data();
            for (int t = 0; t < L; ++t) {
                ll best = NEG;
                int lo = t - T; if (lo < 0) lo = 0;
                for (int s = lo; s <= t; ++s) {
                    ll v = A[s] + hh[t - s];
                    if (v > best) best = v;
                }
                mergB[t] = best;
            }
        } else {
            dcSolve(0, L - 1, 0, L - 1, mergA.data(), h.data(), T, mergB.data());
        }
        for (int t = 0, idx = res; t < L; ++t, idx += p) dp[idx] = mergB[t];
    }
}

int main() {
    int n = readInt();
    K = readInt();
    if (n < 0) return 0;

    static vector<Item> cls[11];
    ll totalP = 0, sumAll = 0;
    for (int i = 0; i < n; ++i) {
        int p = readInt();
        Item it;
        for (int j = 0; j < p; ++j) it.w[j] = readInt();
        totalP += p;
        sumAll += it.w[p - 1];
        cls[p].push_back(it);
    }

    /* if every item fits, take everything (all w >= 1) */
    if (totalP <= (ll)K) { printf("%lld\n", sumAll); return 0; }

    for (int p = 1; p <= 10; ++p)
        sort(cls[p].begin(), cls[p].end(),
             [p](const Item& a, const Item& b) { return a.w[p - 1] > b.w[p - 1]; });

    /* per class: prefix sums of sorted-descending full values */
    static vector<ll> h[11];
    static int Tf[11];
    for (int p = 1; p <= 10; ++p) {
        int m = (int)cls[p].size();
        Tf[p] = (int)min<ll>((ll)m, (ll)(K / p));
        int up = (int)min<ll>((ll)m, (ll)Tf[p] + 1);
        h[p].assign(up + 1, 0);
        ll s = 0;
        for (int t = 1; t <= up; ++t) { s += cls[p][t - 1].w[p - 1]; h[p][t] = s; }
    }

    vector<ll> ident(K + 1, NEG);
    ident[0] = 0;

    /* prefix DPs:  Psave[p] = DP over classes 1..p-1 */
    static vector<ll> Psave[11];
    {
        vector<ll> cur = ident;
        for (int p = 1; p <= 10; ++p) {
            if (p >= 6) Psave[p] = cur;
            if (Tf[p] > 0) mergeClass(cur, p, h[p], Tf[p]);
        }
        ll ans = (cur[K] > NEG / 2) ? cur[K] : 0;
        /* suffix DPs: Qsave[p] = DP over classes p+1..10 */
        static vector<ll> Qsave[6];
        {
            vector<ll> s = ident;
            for (int p = 10; p >= 1; --p) {
                if (p <= 5) Qsave[p] = s;
                if (Tf[p] > 0) mergeClass(s, p, h[p], Tf[p]);
            }
            s.clear(); s.shrink_to_fit();
        }
        cur.clear(); cur.shrink_to_fit();

        /* for every class p and remainder r, one item j of class p is partial */
        vector<ll> C;
        vector<ll> prefMax, sufMax;
        for (int p = 2; p <= 10; ++p) {
            int m = (int)cls[p].size();
            if (m == 0) continue;
            int Tfl = Tf[p];
            const vector<ll>& pref = h[p];   /* pref[0..min(m,Tfl+1)] */

            /* DP over all classes except p */
            if (p <= 5) {
                C = Qsave[p];
                for (int q = 1; q <= p - 1; ++q)
                    if (Tf[q] > 0) mergeClass(C, q, h[q], Tf[q]);
            } else {
                C = Psave[p];
                for (int q = p + 1; q <= 10; ++q)
                    if (Tf[q] > 0) mergeClass(C, q, h[q], Tf[q]);
            }

            for (int r = 1; r <= p - 1; ++r) {
                int c = K - r;
                if (c < 0) continue;
                int Tmax  = min(Tfl, c / p);        /* t items of class p taken (excluding j) */
                int Tmax2 = min(Tmax, m - 1);       /* t items + one extra rank available        */

                prefMax.assign(Tmax + 1, NEG);
                {
                    ll cur2 = NEG;
                    for (int t = 0; t <= Tmax; ++t) {
                        ll g = C[c - t * p];
                        if (g > NEG / 2) { ll v = g + pref[t]; if (v > cur2) cur2 = v; }
                        prefMax[t] = cur2;
                    }
                }
                sufMax.assign(Tmax2 + 2, NEG);
                for (int t = Tmax2; t >= 0; --t) {
                    ll g = C[c - t * p];
                    ll v = NEG;
                    if (g > NEG / 2) v = g + pref[t + 1];
                    sufMax[t] = (v > sufMax[t + 1]) ? v : sufMax[t + 1];
                }

                for (int rho = 1; rho <= m; ++rho) {
                    int i1 = (rho - 1 < Tmax) ? rho - 1 : Tmax;
                    ll part1 = prefMax[i1];
                    ll part2 = NEG;
                    if (rho <= Tmax2 && sufMax[rho] > NEG / 2)
                        part2 = sufMax[rho] - (pref[rho] - pref[rho - 1]);
                    ll best = (part1 > part2) ? part1 : part2;
                    if (best > NEG / 2) {
                        ll cand = best + (ll)cls[p][rho - 1].w[r - 1];
                        if (cand > ans) ans = cand;
                    }
                }
            }
            C.clear();
        }
        printf("%lld\n", ans);
    }
    return 0;
}

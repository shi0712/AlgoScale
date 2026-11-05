// 加倍配平 (Doubling Balancing)
//
// ---------------------------------------------------------------- model ----
// A solution picks disjoint sets A, B with equal post-doubling weight sums.
// For one weight class t (all chips of weight t) let a / b be the numbers of
// used chips put on side A / B and u / v the numbers of those that get
// doubled.  With c = a+b used chips and qp = u+v doublings the class changes
// the weight difference in units of t by
//        K = a - b + u - v,
// so the whole selection is balanced iff  sum_t t * K_t = 0.  Weight classes
// are independent: only the (count, imbalance, doublings) triple of a class
// matters, and the triples of different classes can be combined freely.
//
// Whether a class with c chips can reach imbalance K using qp doublings is
// decided exactly by ach_exact() below; its conditions follow from writing
// K = d + e with d = a-b (side imbalance) and e = u-v (doubling imbalance):
//   |d| <= c, d = c (mod 2), |e| <= qp <= c, qp-e <= c-d, qp+e <= c+d,
//   K = d+e (mod 2)  <=>  K = c+qp (mod 2).
//
// ------------------------------------------------------------------ value ---
// Inside a class the best subset of a given size is the prefix of the largest
// values (negative values are kept in the list because they may be needed to
// fix a parity).  Reachability of K is monotone in the count: if c works then
// c+2 works at the same or lower cost, so for every (K, budget) and every
// count parity the best value is a suffix maximum of prefix sums.  Hence a
// single class contributes  max{ prefix[j] : j = p (mod 2), j >= mincount }.
//
// --------------------------------------------------------------------- DP ---
// DP over the 13 classes in decreasing weight with state
//      (current imbalance D, doublings charged so far).
// The imbalance window |D| <= 1200 and the doubling budget cap 13 are both
// much larger than what any instance actually needs; the bounds come from the
// weight set {1,...,13} being tiny.  Whenever a multiset of chips is feasible
// at all, its per-class imbalances can be chosen so that every imbalance is
// at most lcm(12,13) = 156 (the worst case is a 12-class against a 13-class
// relation), and the total number of doublings needed by the cheapest witness
// never exceeded 7 in exhaustive scans over class-count vectors (all pairs with
// counts <= 60, all triples with counts <= 6, all-ones multisets, and hill
// climbing over counts up to 200 for every k).  The window is additionally at
// least 13*(1+2+...+13) = 1183, i.e. it contains every path whose class
// imbalances are all <= 13; and 13 doublings are enough for the trivial
// "use all positive chips" configuration in the classes where balance 0 is
// reachable at price 0 (even counts) or 1 (odd counts >= 3).
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const ll NEG = LLONG_MIN / 4;
static const int B = 1200;      // imbalance window of the DP
static const int KCAP = 1200;   // per-class |K| cap: KCAP/t + KCAP_B
static const int KCAP_B = 8;
static const int QCAP = 13;     // doublings budget cap (see above)
static const int SMALL = 23;    // brute-force regime for the per-class cost
static const int KTAB = 80;     // |K| range of the precomputed cost table

static inline ll ceildiv2(ll x) { return (x >= 0) ? (x + 1) / 2 : -((-x) / 2); }
static inline ll floordiv2(ll x) { return (x >= 0) ? x / 2 : -(((-x) + 1) / 2); }

// c chips of one class, exactly qp doublings, imbalance K?
static inline bool ach_exact(int c, int qp, int K) {
    if (qp > c) return false;
    if (((K - c - qp) & 1) != 0) return false;      // parity of K = d + e
    ll lo = max<ll>(-c, (ll)K - qp);                // |d| <= c and |e| <= qp
    ll hi = min<ll>(c, (ll)K + qp);
    lo = max(lo, ceildiv2((ll)qp + K - c));         // qp + e <= c + d
    hi = min(hi, floordiv2((ll)c - qp + K));        // qp - e <= c - d
    if (lo > hi) return false;
    if (((lo - c) & 1) != 0) lo++;                  // d = c (mod 2)
    return lo <= hi;
}

// costTab[c][K] = minimum doublings for c <= SMALL chips to reach imbalance K,
// INT_MAX if impossible within QCAP doublings.
static int costTab[SMALL + 1][2 * KTAB + 1];

// Minimum count of parity p reaching imbalance K with at most q doublings;
// a result > cnt means "impossible".
static inline int minC(int K, int q, int p, int cnt) {
    if (K >= -KTAB && K <= KTAB) {                  // small counts: table
        int lim = cnt < SMALL ? cnt : SMALL;
        for (int c = p; c <= lim; c += 2)
            if (costTab[c][K + KTAB] <= q) return c;
    }
    if (cnt <= SMALL) return INT_MAX;
    // For c >= 24 the conditions collapse to: exists qp <= q with
    // K = c+qp (mod 2) and c+qp >= |K|, i.e. cost = max(|K|-c, (K-c)&1).
    ll c0 = max<ll>(SMALL + 1, (ll)llabs((ll)K) - q);
    if (q == 0) {
        if (((c0 - K) & 1) != 0) c0++;              // qp = 0 forces c = K (2)
        if (((c0 - p) & 1) != 0) return INT_MAX;
    } else {
        if (((c0 - p) & 1) != 0) c0++;              // qp >= 1: both parities
    }
    if (c0 > cnt) return INT_MAX;
    return (int)c0;
}

// ---------------------------------------------------------------- input ----
static char *ibuf;
static size_t ipos = 0, ilen = 0;

static inline int gc() {
    if (ipos == ilen) {
        ilen = fread(ibuf, 1, 1 << 16, stdin);
        ipos = 0;
        if (ilen == 0) return -1;
    }
    return ibuf[ipos++];
}

static inline ll readInt() {
    int c = gc();
    while (c != '-' && (c < '0' || c > '9')) c = gc();
    int sg = 1;
    if (c == '-') { sg = -1; c = gc(); }
    ll x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return sg * x;
}

int main() {
    ibuf = (char *)malloc(1 << 16);
    int n = (int)readInt();
    int k = (int)readInt();

    vector<ll> vals[14];
    for (int i = 0; i < n; i++) {
        ll v = readInt();
        int t = (int)readInt();
        vals[t].push_back(v);
    }

    // per-class prefix sums and suffix maxima split by the parity of the count
    vector<ll> pref[14], Par0[14], Par1[14];
    int cnt[14] = {0};
    for (int t = 1; t <= 13; t++) {
        auto &vv = vals[t];
        int c = (int)vv.size();
        cnt[t] = c;
        sort(vv.rbegin(), vv.rend());
        pref[t].assign(c + 2, 0);
        for (int i = 0; i < c; i++) pref[t][i + 1] = pref[t][i] + vv[i];
        Par0[t].assign(c + 2, NEG);
        Par1[t].assign(c + 2, NEG);
        ll b0 = NEG, b1 = NEG;
        for (int j = c; j >= 0; j--) {
            if ((j & 1) == 0) b0 = max(b0, pref[t][j]);
            else b1 = max(b1, pref[t][j]);
            Par0[t][j] = b0;
            Par1[t][j] = b1;
        }
    }

    // per-class doubling cost for small counts
    for (int c = 0; c <= SMALL; c++)
        for (int K = -KTAB; K <= KTAB; K++) {
            int best = INT_MAX;
            for (int qp = 0; qp <= min(c, QCAP); qp++)
                if (ach_exact(c, qp, K)) { best = qp; break; }
            costTab[c][K + KTAB] = best;
        }

    int Q = min(k, QCAP);
    int S = Q + 1;
    vector<ll> dp((size_t)(2 * B + 1) * S, NEG), ndp((size_t)(2 * B + 1) * S, NEG);
    dp[(size_t)B * S] = 0;

    for (int t = 13; t >= 1; t--) {                 // decreasing weight order
        int c = cnt[t];
        if (c == 0) continue;
        int Kmax = min(2 * c, B / t + 1);
        Kmax = min(Kmax, KCAP / t + KCAP_B);

        int Kw = 2 * Kmax + 1;
        vector<ll> val((size_t)S * Kw, NEG);
        for (int qt = 0; qt <= Q; qt++) {
            ll *vrow = &val[(size_t)qt * Kw];
            for (int K = -Kmax; K <= Kmax; K++) {
                ll best = NEG;
                int c0 = minC(K, qt, 0, c);
                if (c0 <= c) best = Par0[t][c0];
                int c1 = minC(K, qt, 1, c);
                if (c1 <= c) best = max(best, Par1[t][c1]);
                vrow[K + Kmax] = best;
            }
        }

        fill(ndp.begin(), ndp.end(), NEG);
        for (int qt = 0; qt <= Q; qt++) {
            const ll *vrow = &val[(size_t)qt * Kw];
            int qlim = Q - qt;
            for (int K = -Kmax; K <= Kmax; K++) {
                ll v = vrow[K + Kmax];
                if (v <= NEG / 2) continue;
                int shift = t * K;
                int dlo = max(-B, -B - shift);
                int dhi = min(B, B - shift);
                for (int D = dlo; D <= dhi; D++) {
                    const ll *src = &dp[(size_t)(D + B) * S];
                    ll *dst = &ndp[(size_t)(D + shift + B) * S];
                    for (int q = 0; q <= qlim; q++) {
                        ll cur = src[q];
                        if (cur <= NEG / 2) continue;
                        ll nv = cur + v;
                        if (nv > dst[q + qt]) dst[q + qt] = nv;
                    }
                }
            }
        }
        dp.swap(ndp);
    }

    ll ans = 0;
    for (int q = 0; q <= Q; q++) ans = max(ans, dp[(size_t)B * S + q]);
    printf("%lld\n", ans);
    return 0;
}

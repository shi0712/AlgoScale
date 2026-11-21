#include <bits/stdc++.h>
using namespace std;

// ---------------------------------------------------------------------------
// Interactive: find the k-th zero of a hidden 0/1 array, t times.
// Guessed zeros become ones.
//
// Strategy
//   We keep a partition of [1..n] into blocks whose number of ones (and hence
//   zeros) is known exactly.  Every answer "? l r" is used to split one block
//   into two blocks with known counts, so information is never thrown away.
//
//   The k-th zero lies in the block found by walking the block-cumulative zero
//   counts (Fenwick tree).  Inside a block [cl,cr] with jc = index of the
//   wanted zero, let
//        A     = cl + jc - 1                       (leftmost possible position)
//        u*    = pos - A  =  #ones in [cl,pos]     (0 <= u* <= #ones(block))
//   Querying [cl, A+t] returns v = #ones in [cl, A+t] and then, because the
//   ones-prefix function is monotone with steps of 0/1,
//        v <= t  =>  the answer is in [cl, A+t]   and  u* <= v
//        v >  t  =>  the answer is in [A+t+1, cr] and  u* >= v
//   so after every query the interval of possible u* shrinks to [lo, v] or
//   [v, hi].  The pivot t is the expected value of u* for a random arrangement
//   of the block's ones, clamped so that at least 1/8 of the current range is
//   cut away at every step (this keeps the worst case logarithmic while being
//   much faster than bisection on random arrays).
//
//   When the answer x is found, the block is split for free around x: the part
//   left of x has jc-1 zeros, the right part has zc-jc zeros.
// ---------------------------------------------------------------------------

// ------------------------------------------------------------------ I/O ----
#ifdef SIM_LOCAL
extern int simN;
extern vector<int> simA;
extern long long simQueries;
int  jask(int l, int r);
void jguess(int x);
static inline int  ask(int l, int r) { return jask(l, r); }
static inline void answer(int x)     { jguess(x); }
#else
static inline int ask(int l, int r) {
    printf("? %d %d\n", l, r);
    fflush(stdout);
    int s;
    if (scanf("%d", &s) != 1) exit(0);
    return s;
}
static inline void answer(int x) {
    printf("! %d\n", x);
    fflush(stdout);
}
#endif

// -------------------------------------------------------------- fenwick ----
struct Fenwick {
    int n;
    vector<int> t;
    void init(int n_) { n = n_; t.assign(n + 2, 0); }
    void add(int i, int d) { for (; i <= n; i += i & -i) t[i] += d; }
    int pref(int i) const { int s = 0; for (; i > 0; i -= i & -i) s += t[i]; return s; }
    // smallest index i with pref(i) >= k  (k >= 1, k <= total)
    int kth(int k) const {
        int idx = 0, bit = 1;
        while ((bit << 1) <= n) bit <<= 1;
        for (; bit; bit >>= 1) {
            int nx = idx + bit;
            if (nx <= n && t[nx] < k) { idx = nx; k -= t[nx]; }
        }
        return idx + 1;
    }
};

// --------------------------------------------------------------- solver ----
static int N;
static Fenwick fw;
static vector<int> BR, BZ;   // block start -> right end / zero count

static inline void addBlock(int l, int r, int z) { BR[l] = r; BZ[l] = z; fw.add(l, z); }
static inline void delBlock(int l)               { fw.add(l, -BZ[l]); BR[l] = 0; }

// clamp the model pivot so that a constant fraction of the range is removed
#ifndef CLAMPDIV
#define CLAMPDIV 8
#endif

// Optional: fully resolve a block that contains very few ones but lots of
// zeros when many future rounds are likely to visit it.  Such a block costs
// about one query per round otherwise, while resolving it once makes every
// future lookup inside it free.
#ifndef RESFRAC
#define RESFRAC 16
#endif
#ifndef RESFACTOR
#define RESFACTOR 2.0
#endif

static bool wantResolve(int l, int r, int z, int remaining) {
    int len = r - l + 1;
    int m = len - z;
    if (m <= 0 || len <= 1) return false;
    long long totalZeros = fw.pref(N);
    if (totalZeros <= 0) return false;
    // only blocks holding a large share of the remaining zeros are guaranteed
    // to be visited often, whatever the k sequence looks like
    if ((long long)z * RESFRAC < totalZeros) return false;
    double expect = (double)z / (double)totalZeros * (double)remaining;
    double perRound = 1.0 + 0.5 * log2((double)m + 1.0);
    double cost = (double)m * (log2((double)len / (double)m) + 2.0);
    return expect * perRound > cost * RESFACTOR;
}

static void resolveBlock(int l, int r, int z, int remaining) {
    int len = r - l + 1;
    int m = len - z;
    if (m <= 0 || len <= 1) return;
    if (!wantResolve(l, r, z, remaining)) return;
    int mid = (l + r) >> 1;
    int wq = ask(l, mid);
    int zl = (mid - l + 1) - wq, zr = z - zl;
    delBlock(l);
    addBlock(l, mid, zl);
    if (mid + 1 <= r) addBlock(mid + 1, r, zr);
    resolveBlock(l, mid, zl, remaining);
    resolveBlock(mid + 1, r, zr, remaining);
}

static int solveRound(int k, int remaining) {
    int l0 = fw.kth(k);
    int r0 = BR[l0], z0 = BZ[l0];
    resolveBlock(l0, r0, z0, remaining); // may resolve the block completely
    // the target's block may have changed; look it up again
    l0 = fw.kth(k);
    int before = fw.pref(l0 - 1);
    r0 = BR[l0]; z0 = BZ[l0];
    int j0 = k - before;                 // index of the wanted zero inside the block
    int A = l0 + j0 - 1;                 // leftmost possible answer
    int m0 = (r0 - l0 + 1) - z0;         // ones inside the block
    int lo = 0, hi = m0;                 // bounds on u*
    int cl = l0, cr = r0, zc = z0, jc = j0, base = 0;

    while (lo < hi) {
        int mcur = (cr - cl + 1) - zc;
        long long tm = base + (long long)mcur * jc / (zc + 1);
        long long w = hi - lo;
        long long nlo = lo + w / CLAMPDIV, nhi = hi - w / CLAMPDIV;
        if (nlo > nhi) nlo = nhi = (lo + hi) >> 1;
        if (tm < nlo) tm = nlo;
        if (tm > nhi) tm = nhi;
        int t = (int)tm;
        if (t < lo) t = lo;
        if (t > hi - 1) t = hi - 1;

        int q = A + t;
        int wq = ask(cl, q);             // ones in [cl, q]
        int zl = (q - cl + 1) - wq;      // zeros in [cl, q]
        int zr = zc - zl;
        delBlock(cl);
        addBlock(cl, q, zl);
        if (q + 1 <= cr) addBlock(q + 1, cr, zr);

        int v = base + wq;               // = #ones in [l0, A+t]
        if (v > t) {                     // answer is right of q, u* >= v
            base = v; cl = q + 1; zc = zr; jc -= zl;
            if (t + 1 > lo) lo = t + 1;
            if (v > lo) lo = v;
        } else {                         // answer is at or left of q, u* <= v
            cr = q; zc = zl;
            if (v < hi) hi = v;
        }
    }

    int x = A + lo;
    // free refinement around the answer
    delBlock(cl);
    if (x - 1 >= cl) addBlock(cl, x - 1, jc - 1);
    addBlock(x, x, 0);
    if (x + 1 <= cr) addBlock(x + 1, cr, zc - jc);
    return x;
}

#ifndef SIM_LOCAL
int main() {
    int T;
    if (scanf("%d %d", &N, &T) != 2) return 0;
    fw.init(N);
    BR.assign(N + 2, 0);
    BZ.assign(N + 2, 0);
    // The k of the current round is always sent before its queries
    // (the first one arrives together with "n t").
    int k;
    if (T <= 0) return 0;
    if (scanf("%d", &k) != 1) return 0;
    int tot = ask(1, N);                 // total number of ones
    addBlock(1, N, N - tot);
    for (int r = 0; r < T; r++) {
        answer(solveRound(k, T - r - 1));
        if (r + 1 < T && scanf("%d", &k) != 1) return 0;
    }
    return 0;
}
#endif

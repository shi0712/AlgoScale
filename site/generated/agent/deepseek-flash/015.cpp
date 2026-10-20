// 云阶之差 (UOJ)
//
// h_i = (i + b/a)^2.  For i < j:
//   h_j - h_i = (j-i)(i+j+2b/a) = d*(a*s+2b)/a,   d = j-i, s = i+j.
// With t = i in [1, n-d]:
//   N := d*(a*s+2b) = a*d^2 + 2*b*d + 2*a*d*t
// For each d in [1,n-1] the values form an AP over t = 1..n-d.
// The answer is N*/a in lowest terms, where N* is the k-th smallest N.
//
// count(M) = #{pairs : N <= M} is nondecreasing; the answer is the smallest M
// with count(M) >= k.  Since maxval(d) = d*(a*(2n-d)+2b) increases with d and
// row d's minimum is minval(d) = a*d^2+2bd+2ad:
//   * rows 1..ds   (maxval <= M) contribute n-d each,           ds = max d with maxval(d) <= M
//   * rows ds+1..Q (minval <= M) contribute (M-a*d^2-2*b*d)/(2*a*d) each,  Q = max d with minval(d) <= M
//
// Search: a smooth (Euler-Maclaurin) model of count() gives a starting point;
// exact counts, anchored to the model's shape, drive Newton steps down to a
// bracket holding at most VCAP values; those values are then enumerated and
// selected directly.
#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long u64;
typedef __uint128_t u128;
typedef __int128_t i128;

static u64 n, K, a, b;
static u128 MAXVAL_LAST;   // largest value over all pairs
static u64 CLAMP;          // saturation for count_upto
static const u64 VCAP = 200000;

static inline u128 maxval(u64 d) {
    if (!d) return 0;
    return (u128)d * ((u128)a * (2 * (u128)n - d) + 2 * (u128)b);
}
static inline u128 minval(u64 d) {
    if (!d) return 0;
    return (u128)d * ((u128)a * (u128)d + 2 * (u128)b) + (u128)2 * a * d;
}

// largest d in [0,n-1] with maxval(d) <= M
static inline u64 bs_max(u128 M) {
    if (MAXVAL_LAST <= M) return n - 1;
    u64 lo = 0, hi = n - 1;
    while (lo < hi) {
        u64 mid = lo + (hi - lo + 1) / 2;
        if (maxval(mid) <= M) lo = mid; else hi = mid - 1;
    }
    return lo;
}
// largest d in [0,n-1] with minval(d) <= M
static inline u64 bs_min(u128 M) {
    if (minval(n - 1) <= M) return n - 1;
    u64 lo = 0, hi = n - 1;
    while (lo < hi) {
        u64 mid = lo + (hi - lo + 1) / 2;
        if (minval(mid) <= M) lo = mid; else hi = mid - 1;
    }
    return lo;
}

// number of pairs with N <= M, saturated at CLAMP
static u64 count_upto(u128 M) {
    if (M == 0) return 0;
    u64 ds = bs_max(M);
    u128 capv = (u128)ds * n - (u128)ds * (ds + 1) / 2;
    if (capv >= (u128)CLAMP) return CLAMP;
    u64 Q = bs_min(M);
    u128 sum = capv;
    if (Q <= ds) return (u64)sum;

    u64 d = ds + 1;
    u128 A = (u128)a * ((u128)d * d) + (u128)2 * b * d;   // row constant
    u128 Ainc = (u128)a * (2 * (u128)d + 1) + 2 * (u128)b;
    u64 den = 2 * a * d;
    const u64 dinc = 2 * a;
    const double Mf = (double)M;
    const double af = (double)a, bf = 2.0 * (double)b;
    for (; d <= Q; ++d) {
        // quotient estimate in doubles: exact value is (M - A)/den, and it is
        // at most n-1 <= 1e12, so the relative error (<=1e-16) is far below the
        // 1/100 guard band below.
        double Aest = (double)d * (af * (double)d + bf);
        double qf = (Mf - Aest) / (double)den;
        double fl = floor(qf);
        u64 q = (u64)fl;
        if (qf - fl < 0.01 || fl - qf < 0.01) {              // near an integer: exact
            u128 num = M - A;
            q = (u64)(num / den);
        }
        sum += q;
        if (sum >= (u128)CLAMP) return CLAMP;
        A += Ainc; Ainc += dinc; den += dinc;
    }
    return (u64)sum;
}

// ------------------------- smooth model of count() -------------------------

static double harm(u64 x) {
    if (x == 0) return 0.0;
    if (x < 64) { double s = 0; for (int i = 1; i <= (int)x; ++i) s += 1.0 / i; return s; }
    double v = 1.0 / (double)x, v2 = v * v;
    return log((double)x) + 0.5772156649015328606 + 0.5 * v - v2 / 12.0 + v2 * v2 / 120.0;
}
static double harm_diff(u64 hi, u64 lo) {  // H_hi - H_lo
    if (lo == 0) return harm(hi);
    if (hi < 64 || lo < 64) return harm(hi) - harm(lo);
    double x = 1.0 / (double)hi, y = 1.0 / (double)lo;
    double x2 = x * x, y2 = y * y;
    return log((double)hi / (double)lo) + 0.5 * (x - y)
         - (x2 - y2) / 12.0 + (x2 * x2 - y2 * y2) / 120.0;
}

struct ModelInfo { double val, err, rho; };

static ModelInfo model(u128 M) {
    ModelInfo r;
    u64 ds = bs_max(M), Q = bs_min(M);
    double capv = (double)ds * (double)n - 0.5 * (double)ds * (double)(ds + 1);
    if (Q <= ds) { r.val = capv; r.err = 1.0; r.rho = 0.0; return r; }
    double q1 = (double)Q, d1 = (double)ds;
    double hd = harm_diff(Q, ds);
    // sum over d in (ds, Q] of (M - a*d^2 - 2*b*d) / (2*a*d)
    //   = M/(2a)*(H_Q - H_ds) - (Q(Q+1)-ds(ds+1))/4 - (b/a)*(Q-ds)
    // minus 1/2 per row for the floor.
    r.val = capv
          + (double)M / (2.0 * (double)a) * hd
          - (q1 * (q1 + 1.0) - d1 * (d1 + 1.0)) / 4.0
          - ((double)b / (double)a + 0.5) * (q1 - d1);
    r.err = 0.5 * (q1 - d1) + 8.0 + 1e-12 * fabs(r.val);
    r.rho = hd / (2.0 * (double)a);
    return r;
}

static u128 model_solve(double target) {
    u128 lo = 0, hi = MAXVAL_LAST;
    while (lo + 1 < hi) {
        u128 mid = lo + (hi - lo) / 2;
        if (model(mid).val >= target) hi = mid; else lo = mid;
    }
    return hi;
}
// smallest x in [lo,hi] with model(x) >= target (returns hi if none)
static u128 model_ge_in(u128 lo, u128 hi, double target) {
    if (model(lo).val >= target) return lo;
    while (lo + 1 < hi) {
        u128 mid = lo + (hi - lo) / 2;
        if (model(mid).val >= target) hi = mid; else lo = mid;
    }
    return hi;
}

static u128 dbl_to_u128(double x) {
    if (!(x > 0)) return 0;
    if (x > 1e34) x = 1e34;
    return (u128)x;
}
static u128 add_clamped(u128 x, double d) {  // x + d, kept in [0, MAXVAL_LAST]
    if (d >= 0) {
        u128 add = dbl_to_u128(d);
        if (MAXVAL_LAST - x <= add) return MAXVAL_LAST;
        return x + add;
    } else {
        u128 sub = dbl_to_u128(-d);
        if (x <= sub) return 0;
        return x - sub;
    }
}

static void print_u128(u128 v) {
    if (!v) { putchar('0'); return; }
    char buf[45]; int p = 0;
    while (v) { buf[p++] = char('0' + (int)(v % 10)); v /= 10; }
    while (p) putchar(buf[--p]);
}

int main() {
    if (scanf("%llu %llu %llu %llu", &n, &K, &a, &b) != 4) return 0;

    MAXVAL_LAST = maxval(n - 1);
    CLAMP = K + VCAP + 16;

    u128 answer = 0;

    if (K == 1) {
        answer = (u128)a + 2 * (u128)b + 2 * (u128)a;   // value at d = 1, t = 1
    } else {
        // ---------- initial bracket around the model's prediction ----------
        u128 M0 = model_solve((double)K);
        ModelInfo mi0 = model(M0);
        double rho0 = mi0.rho;
        if (!(rho0 > 1e-18)) rho0 = 1e-18;
        double W = (mi0.err + 16.0) / rho0;
        if (!isfinite(W) || W > 1e30) W = 1e30;
        if (W < 1.0) W = 1.0;

        u128 lo = 0, hi = MAXVAL_LAST;
        u64 c_lo = 0, c_hi = 0;
        bool c_hi_ok = false;

        {   // hi: grow geometrically until count(hi) >= K
            for (int k = 0; k < 64; ++k) {
                u128 cand = add_clamped(M0, ldexp(2.0 * W, k));
                if (cand < hi) hi = cand;
                c_hi = count_upto(hi);
                c_hi_ok = (c_hi < CLAMP);
                if (c_hi >= K) break;
                if (hi >= MAXVAL_LAST) { hi = MAXVAL_LAST; c_hi = count_upto(hi); c_hi_ok = (c_hi < CLAMP); break; }
            }
            if (c_hi < K) { hi = MAXVAL_LAST; c_hi = count_upto(hi); c_hi_ok = (c_hi < CLAMP); }
        }
        {   // lo: shrink geometrically until count(lo) < K
            for (int k = 0; k < 64; ++k) {
                u128 cand = (M0 > dbl_to_u128(ldexp(2.0 * W, k))) ? add_clamped(M0, -ldexp(2.0 * W, k)) : 0;
                if (cand > lo) lo = cand;
                c_lo = count_upto(lo);
                if (c_lo < K) break;
                if (lo == 0) { c_lo = 0; break; }
            }
            if (c_lo >= K) { lo = 0; c_lo = 0; }
        }

        // ---------- tighten to an enumerable window ----------
        // Goal: c_lo < K <= c_hi with c_hi - c_lo <= VCAP.  Each probe aims at a
        // count of K -/+ 0.45*VCAP, which puts both bracket ends close to K in a
        // single step.  The model is shifted to pass through the last exact
        // count, which cancels its systematic bias and makes the aim accurate.
        u128 M = 0;          // last probe position
        u64 cM = 0;          // its count (meaningful only if Mok)
        bool Mok = false;    // true when cM is an exact count
        bool done = false;
        const double BAND = 0.45 * (double)VCAP;
        for (int it = 0; it < 64; ++it) {
            if (hi <= lo + 1) { answer = hi; done = true; break; }
            if (c_hi_ok && c_hi - c_lo <= VCAP) break;      // few enough values: enumerate

            double target = ((double)c_lo < (double)K - BAND) ? ((double)K - BAND)
                                                             : ((double)K + BAND);
            u128 probe; bool have = false;
            {
                double mt = Mok ? model(M).val + (target - (double)cM) : target;
                u128 p = model_ge_in(lo, hi, mt);
                if (p > lo && p < hi && p != M) { probe = p; have = true; }
            }
            if (!have) probe = lo + (hi - lo) / 2;
            if (probe <= lo) probe = lo + 1;
            if (probe >= hi) probe = hi - 1;
            if (probe <= lo || probe >= hi) break;
            u64 c = count_upto(probe);
            Mok = (c < CLAMP);
            cM = c;
            if (c >= K) { if (probe < hi) { hi = probe; c_hi = c; c_hi_ok = Mok; } }
            else        { if (probe > lo) { lo = probe; c_lo = c; } }
            M = probe;
        }

        if (!done) {
            if (hi <= lo + 1) {
                answer = hi;
            } else if (c_hi_ok && c_hi - c_lo <= VCAP) {
                u64 rank = K - c_lo;      // rank-th smallest value in (lo, hi]
                u64 ds = bs_max(lo), Qh = bs_min(hi);
                vector<u128> vals;
                vals.reserve(VCAP + 16);
                const double loF = (double)lo, hiF = (double)hi;
                const double af = (double)a, bf = 2.0 * (double)b;
                for (u64 d = ds + 1; d <= Qh; ++d) {
                    double Aest = (double)d * (af * (double)d + bf);
                    u128 A = (u128)a * ((u128)d * d) + (u128)2 * b * d;
                    u64 step = 2 * a * d;
                    u64 maxT = n - d;
                    u64 tlo = 1, thi = maxT;
                    if (lo > A) {
                        u64 t = (Aest < loF) ? (u64)floor((loF - Aest) / (double)step) : 0;
                        int g = 0;
                        while (t > 0 && A + (u128)step * t > lo && g < 4) { --t; ++g; }
                        while (A + (u128)step * (t + 1) <= lo && g < 4) { ++t; ++g; }
                        if (g >= 4) t = (u64)((lo - A) / step);      // exact fallback
                        tlo = t + 1;
                    }
                    if (hi > A) {
                        u64 t = (Aest < hiF) ? (u64)floor((hiF - Aest) / (double)step) : 0;
                        int g = 0;
                        while (t > 0 && A + (u128)step * t > hi && g < 4) { --t; ++g; }
                        while (A + (u128)step * (t + 1) <= hi && g < 4) { ++t; ++g; }
                        if (g >= 4) t = (u64)((hi - A) / step);      // exact fallback
                        thi = t;
                    } else thi = 0;
                    if (thi > maxT) thi = maxT;
                    if (tlo < 1) tlo = 1;
                    if (tlo <= thi) {
                        u128 v = A + (u128)step * (u128)tlo;
                        for (u64 t = tlo; t <= thi; ++t, v += step) vals.push_back(v);
                    }
                }
                if (rank >= 1 && rank <= vals.size()) {
                    nth_element(vals.begin(), vals.begin() + (rank - 1), vals.end());
                    answer = vals[rank - 1];
                } else {
                    // should not happen; fall back to exact bisection
                    while (lo + 1 < hi) {
                        u128 mid = lo + (hi - lo) / 2;
                        if (count_upto(mid) >= K) hi = mid; else lo = mid;
                    }
                    answer = hi;
                }
            } else {
                // last resort: plain binary search
                while (lo + 1 < hi) {
                    u128 mid = lo + (hi - lo) / 2;
                    if (count_upto(mid) >= K) hi = mid; else lo = mid;
                }
                answer = hi;
            }
        }
    }

    u128 x = answer, y = (u128)a;
    while (y) { u128 t = x % y; x = y; y = t; }
    print_u128(answer / x);
    putchar(' ');
    print_u128((u128)a / x);
    putchar('\n');
    return 0;
}

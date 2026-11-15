// Double Sort  --  expected sorted-gap prefix sums of a random n-subset of [1..m]
//
//  r = n-i,  T_j = sum_{u>=0} C(m-j*u, n)
//  E[c_{n-r}] = C(m,n)^{-1} * sum_{j>=r+1} (-1)^{j+r-1} C(j-2,r-1) C(n,j) T_j
//  E[c_n]     = n * T_1 / C(m,n)
//
// The alternating sum loses ~1.4e2 digits of relative accuracy for n=300, so a
// software binary float with a 576-bit mantissa is used throughout.

#include <bits/stdc++.h>
using namespace std;

typedef unsigned long long ull;
typedef __int128 lll;

static const int L = 9;          // mantissa limbs
static const int W = 64 * L;     // 576 bits

struct BF {
    ull m[L];   // mantissa integer, top bit of m[L-1] set when s != 0
    int e;      // value = s * m * 2^{-W} * 2^{e}
    int s;      // -1, 0, 1
};

static inline void bfZero(BF &a) { for (int i = 0; i < L; i++) a.m[i] = 0; a.e = 0; a.s = 0; }

/* dst[0..nout-1] = (src << t), src has nin limbs */
static inline void shlN(ull *dst, const ull *src, int nin, int nout, int t) {
    int sh = t >> 6, b = t & 63;
    if (b == 0) {
        for (int i = 0; i < nout; i++) { int j = i - sh; dst[i] = (j >= 0 && j < nin) ? src[j] : 0ULL; }
    } else {
        for (int i = 0; i < nout; i++) {
            ull v = 0; int j = i - sh;
            if (j >= 0 && j < nin) v = src[j] << b;
            if (j - 1 >= 0 && j - 1 < nin) v |= src[j - 1] >> (64 - b);
            dst[i] = v;
        }
    }
}

/* dst[0..nout-1] = (src >> t), src has nin limbs */
static inline void shrN(ull *dst, const ull *src, int nin, int nout, int t) {
    int sh = t >> 6, b = t & 63;
    if (b == 0) {
        for (int i = 0; i < nout; i++) { int j = i + sh; dst[i] = (j < nin) ? src[j] : 0ULL; }
    } else {
        for (int i = 0; i < nout; i++) {
            ull v = 0; int j = i + sh;
            if (j < nin) v = src[j] >> b;
            if (j + 1 < nin) v |= src[j + 1] << (64 - b);
            dst[i] = v;
        }
    }
}

/* value = P * 2^{-W} * 2^{Ebase} (truncated/rnded to L limbs) */
static inline BF packFrom(const ull *P, int nin, int Ebase, int hiHint = -1) {
    BF r; bfZero(r);
    int hi = (hiHint >= 0 && hiHint < nin && P[hiHint] != 0) ? hiHint : -1;
    if (hi < 0) {
        for (int i = nin - 1; i >= 0; --i) if (P[i]) { hi = i; break; }
    }
    if (hi < 0) return r;
    int h = hi * 64 + (63 - __builtin_clzll(P[hi]));
    int t = h - (W - 1);
    r.s = 1;
    if (t == 0) {
        for (int i = 0; i < L; i++) r.m[i] = (i < nin) ? P[i] : 0ULL;
        r.e = Ebase;
        return r;
    }
    if (t < 0) {
        ull tmp[L + 2];
        shlN(tmp, P, nin, L + 1, -t);
        for (int i = 0; i < L; i++) r.m[i] = tmp[i];
        r.e = Ebase + t;
        return r;
    }
    ull Q[2 * L + 4];
    int nn = nin + 1;
    for (int i = 0; i < nin; i++) Q[i] = P[i];
    Q[nin] = 0;
    {
        int rb = t - 1, i = rb >> 6;
        ull carry = 1ULL << (rb & 63);
        while (carry && i < nn) {
            lll cur = (lll)Q[i] + carry;
            Q[i] = (ull)cur; carry = (ull)(cur >> 64); ++i;
        }
    }
    ull R2[L + 2];
    shrN(R2, Q, nn, L + 2, t);
    if (R2[L] != 0) {
        ull T3[L + 2];
        shrN(T3, R2, L + 2, L + 2, 1);
        for (int i = 0; i < L; i++) r.m[i] = T3[i];
        r.e = Ebase + t + 1;
    } else {
        for (int i = 0; i < L; i++) r.m[i] = R2[i];
        r.e = Ebase + t;
    }
    return r;
}

static inline BF bfFromU64(ull x) {
    BF r; bfZero(r);
    if (!x) return r;
    ull P[1] = { x };
    return packFrom(P, 1, W);
}

static inline BF negBF(BF a) { a.s = -a.s; return a; }

static inline BF mulBF(const BF &a, const BF &b) {
    if (a.s == 0 || b.s == 0) { BF z; bfZero(z); return z; }
    ull P[2 * L + 2];
    for (int i = 0; i < 2 * L + 2; i++) P[i] = 0;
    for (int i = 0; i < L; i++) {
        ull ai = a.m[i];
        if (!ai) continue;
        ull carry = 0;
        for (int j = 0; j < L; j++) {
            lll cur = (lll)ai * b.m[j] + P[i + j] + carry;
            P[i + j] = (ull)cur; carry = (ull)(cur >> 64);
        }
        int idx = i + L;
        while (carry) { lll cur = (lll)P[idx] + carry; P[idx] = (ull)cur; carry = (ull)(cur >> 64); ++idx; }
    }
    int hi = (P[2 * L - 1] != 0) ? 2 * L - 1 : 2 * L - 2;
    BF r = packFrom(P, 2 * L + 1, a.e + b.e - W, hi);
    r.s = a.s * b.s;
    if (r.m[L - 1] == 0) r.s = 0;
    return r;
}

/* multiply BF by integer c (nc limbs) */
static inline BF mulBB(const BF &a, const ull *c, int nc) {
    if (a.s == 0 || nc == 0) { BF z; bfZero(z); return z; }
    ull P[L + 8];
    int tot = L + nc;
    for (int i = 0; i < tot + 1; i++) P[i] = 0;
    for (int i = 0; i < L; i++) {
        ull ai = a.m[i];
        if (!ai) continue;
        ull carry = 0;
        for (int j = 0; j < nc; j++) {
            lll cur = (lll)ai * c[j] + P[i + j] + carry;
            P[i + j] = (ull)cur; carry = (ull)(cur >> 64);
        }
        int idx = i + nc;
        while (carry) { lll cur = (lll)P[idx] + carry; P[idx] = (ull)cur; carry = (ull)(cur >> 64); ++idx; }
    }
    int hi = (P[tot - 1] != 0) ? tot - 1 : tot - 2;
    BF r = packFrom(P, tot, a.e, hi);
    r.s = a.s;
    if (r.m[L - 1] == 0) r.s = 0;
    return r;
}

static inline BF mulBFsmall(const BF &a, ull d) {
    ull c[1] = { d };
    return mulBB(a, c, 1);
}

static inline BF divBFsmall(const BF &a, ull d) {
    BF r;
    if (a.s == 0) return a;
    ull q[L]; ull rem = 0;
    for (int i = L - 1; i >= 0; --i) {
        lll cur = ((lll)rem << 64) | a.m[i];
        q[i] = (ull)(cur / (lll)d);
        rem = (ull)(cur % (lll)d);
    }
    for (int i = 0; i < L; i++) r.m[i] = q[i];
    r.e = a.e; r.s = a.s;
    BF z = packFrom(r.m, L, r.e);
    z.s = a.s;
    if (z.m[L - 1] == 0) z.s = 0;
    return z;
}

static inline int cmpMag(const BF &a, const BF &b) {  // both nonzero
    if (a.e != b.e) return a.e < b.e ? -1 : 1;
    for (int i = L - 1; i >= 0; --i) if (a.m[i] != b.m[i]) return a.m[i] < b.m[i] ? -1 : 1;
    return 0;
}

static inline BF addBF(BF a, BF b) {
    if (a.s == 0) return b;
    if (b.s == 0) return a;
    if (a.e < b.e || (a.e == b.e && cmpMag(a, b) < 0)) swap(a, b);
    int d = a.e - b.e;
    ull B[L];
    if (d >= W + 2) return a;
    shrN(B, b.m, L, L, d);
    if (a.s == b.s) {
        ull P[L + 1]; ull carry = 0;
        for (int i = 0; i < L; i++) {
            lll cur = (lll)a.m[i] + B[i] + carry;
            P[i] = (ull)cur; carry = (ull)(cur >> 64);
        }
        P[L] = carry;
        BF r = packFrom(P, L + 1, a.e);
        r.s = a.s;
        if (r.m[L - 1] == 0) r.s = 0;
        return r;
    } else {
        ull P[L]; ull borrow = 0;
        for (int i = 0; i < L; i++) {
            lll cur = (lll)a.m[i] - B[i] - borrow;
            if (cur < 0) { cur += ((lll)1 << 64); borrow = 1; } else borrow = 0;
            P[i] = (ull)cur;
        }
        BF r = packFrom(P, L, a.e);
        r.s = (r.m[L - 1] == 0) ? 0 : a.s;
        return r;
    }
}

static inline BF subBF(BF a, BF b) { return addBF(a, negBF(b)); }

static inline long double toLD(const BF &a) {
    if (a.s == 0) return 0.0L;
    long double v = ldexpl((long double)a.m[L - 1], a.e - 64);
    return a.s < 0 ? -v : v;
}

/* ---------------- exact small bignum (for binomial coefficients) ------------- */
static const int NB = 6;
struct Big { ull v[NB]; };
static inline void bigSet(Big &a, ull x) { for (int i = 0; i < NB; i++) a.v[i] = 0; a.v[0] = x; }
static inline void bigMulSmall(Big &a, ull d) {
    ull carry = 0;
    for (int i = 0; i < NB; i++) { lll cur = (lll)a.v[i] * d + carry; a.v[i] = (ull)cur; carry = (ull)(cur >> 64); }
}
static inline void bigDivSmall(Big &a, ull d) {
    ull rem = 0;
    for (int i = NB - 1; i >= 0; --i) { lll cur = ((lll)rem << 64) | a.v[i]; a.v[i] = (ull)(cur / (lll)d); rem = (ull)(cur % (lll)d); }
}
static inline int bigNz(const Big &a) { for (int i = NB - 1; i >= 0; --i) if (a.v[i]) return i + 1; return 0; }

/* ----------------------------------- main ---------------------------------- */
int main() {
    int n; long long m;
    if (scanf("%d %lld", &n, &m) != 2) return 0;

    const long long MU = 60;   // direct summation when (m-n)/j < MU

    // C(n,j)
    vector<Big> Cnj(n + 1);
    bigSet(Cnj[0], 1);
    for (int j = 0; j < n; j++) {
        Cnj[j + 1] = Cnj[j];
        bigMulSmall(Cnj[j + 1], (ull)(n - j));
        bigDivSmall(Cnj[j + 1], (ull)(j + 1));
    }

    vector<BF> R(n + 1);
    {
        vector<BF> c(n + 2), beta(n + 2), rho(n + 2);
        vector<Big> D(n + 1);
        for (int j = 1; j <= n; j++) {
            long long U = (m - (long long)n) / j;
            if (U >= MU) {
                long long M = m - (long long)j * U;
                int K = n + 1;
                // D[i] = C(j,i+1), i=0..j-1
                static int Dnz[512];
                {
                    Big t; bigSet(t, 1);
                    for (int i = 0; i < j; i++) {
                        bigMulSmall(t, (ull)(j - i));
                        bigDivSmall(t, (ull)(i + 1));
                        D[i] = t;
                        Dnz[i] = bigNz(t);
                    }
                }
                // c[k] = [x^k] 1/D_j(x)
                c[0] = divBFsmall(bfFromU64(1), (ull)j);
                for (int k = 1; k <= K; k++) {
                    BF s; bfZero(s);
                    int imax = min(k, j - 1);
                    for (int i = 1; i <= imax; i++) {
                        BF t2 = mulBB(c[k - i], D[i].v, Dnz[i]);
                        s = addBF(s, t2);
                    }
                    s = divBFsmall(s, (ull)j);
                    s = negBF(s);
                    if (s.s == 0) bfZero(s);
                    c[k] = s;
                }
                // beta[k] = C(m+j, n+1-k)/C(m,n)
                {
                    BF b0 = bfFromU64(1);
                    for (int i = 0; i < n; i++) {
                        b0 = mulBFsmall(b0, (ull)(m + j - i));
                        b0 = divBFsmall(b0, (ull)(m - i));
                    }
                    b0 = mulBFsmall(b0, (ull)(m + j - n));
                    b0 = divBFsmall(b0, (ull)(n + 1));
                    beta[0] = b0;
                    for (int k = 0; k <= n; k++) {
                        BF t2 = mulBFsmall(beta[k], (ull)(n + 1 - k));
                        t2 = divBFsmall(t2, (ull)(m + j - n + k));
                        beta[k + 1] = t2;
                    }
                }
                // rho[K] = C(M,K)/C(m+j,K)
                {
                    rho[0] = bfFromU64(1);
                    bool z = false;
                    for (int Kk = 0; Kk <= n; Kk++) {
                        long long num = M - Kk;
                        if (num <= 0) { z = true; }
                        if (z) { BF q; bfZero(q); rho[Kk + 1] = q; }
                        else {
                            BF t2 = mulBFsmall(rho[Kk], (ull)num);
                            t2 = divBFsmall(t2, (ull)(m + j - Kk));
                            rho[Kk + 1] = t2;
                        }
                    }
                }
                BF res; bfZero(res);
                for (int k = 0; k <= K; k++) {
                    int Kk = n + 1 - k;
                    BF one = bfFromU64(1);
                    BF diff = subBF(one, rho[Kk]);
                    if (diff.s == 0) continue;
                    BF t1 = mulBF(c[k], beta[k]);
                    BF t2 = mulBF(t1, diff);
                    res = addBF(res, t2);
                }
                R[j] = res;
            } else {
                BF term = bfFromU64(1);
                BF res = bfFromU64(1);
                long long Mc = m;
                for (long long u = 0; u < U; u++) {
                    long long Mn = Mc - j;
                    for (int i = 0; i < n; i++) {
                        term = mulBFsmall(term, (ull)(Mn - i));
                        term = divBFsmall(term, (ull)(Mc - i));
                    }
                    res = addBF(res, term);
                    Mc = Mn;
                }
                R[j] = res;
            }
        }
    }

    vector<BF> ans(n + 1);
    ans[n] = mulBFsmall(R[1], (ull)n);
    for (int r = 1; r <= n - 1; r++) {
        BF sum; bfZero(sum);
        Big coef; bigSet(coef, 1);
        for (int j = r + 1; j <= n; j++) {
            BF t = mulBB(R[j], Cnj[j].v, bigNz(Cnj[j]));
            t = mulBB(t, coef.v, bigNz(coef));
            if (t.s != 0) {
                if ((j + r - 1) & 1) t = negBF(t);
                sum = addBF(sum, t);
            }
            if (j < n) {
                bigMulSmall(coef, (ull)(j - 1));
                bigDivSmall(coef, (ull)(j - r));
            }
        }
        ans[n - r] = sum;
    }

    string out;
    for (int i = 1; i <= n; i++) {
        long double v = toLD(ans[i]);
        char buf[64];
        snprintf(buf, sizeof buf, "%.10Lf", v);
        out += buf;
        out += '\n';
    }
    fputs(out.c_str(), stdout);
    return 0;
}

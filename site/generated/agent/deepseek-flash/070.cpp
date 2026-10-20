#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <algorithm>
using namespace std;

// Double Sort:  E[ans_k] = k + E[sum of the k smallest gaps].
//   M = m-n, N = C(M+n,n), S_l = sum_{t>=1} C(M-l*t+n, n)
//   ans_k = k + ( n*S_1 if k==n else sum_{mu=n-k+1..n} (-1)^(mu+L) C(mu-2,L-2) C(n,mu) S_mu ) / N
// The alternating sum cancels ~40 decimal digits at n=300,m=1e9, so everything is
// done in exact big integers; only the final division becomes a long double.
// Each S_l is obtained exactly by Newton forward interpolation of the degree-(n+1)
// polynomial V(j) = sum_{i<j} C(M%l + l*i + n, n) evaluated at j = M/l.

// ---------- fixed capacity unsigned big integer (little endian limbs) ----------
static const int MAXL = 200;

struct Big {
    int n;
    uint64_t d[MAXL];
};

#ifdef DEBUG_EXACT
int g_maxLimbs = 0;
int g_maxNeed = 0;
static void track(const Big &a) { if (a.n > g_maxLimbs) g_maxLimbs = a.n; }
static void printBig(const Big &a) {
    if (a.n == 1 && a.d[0] == 0) { fputs("0", stdout); return; }
    static const uint64_t B = 1000000000ULL;
    uint64_t tmp[MAXL];
    memcpy(tmp, a.d, sizeof(uint64_t) * a.n);
    int n = a.n;
    std::vector<uint32_t> chunks;
    while (!(n == 1 && tmp[0] == 0)) {
        __uint128_t cur = 0;
        for (int i = n - 1; i >= 0; --i) {
            cur = (cur << 64) | tmp[i];
            tmp[i] = (uint64_t)(cur / B);
            cur %= B;
        }
        chunks.push_back((uint32_t)cur);
        while (n > 1 && tmp[n - 1] == 0) --n;
    }
    printf("%u", chunks.back());
    for (int i = (int)chunks.size() - 2; i >= 0; --i) printf("%09u", chunks[i]);
}
#endif

static inline void trim(Big &a) { while (a.n > 1 && a.d[a.n - 1] == 0) --a.n; }
static inline void setZero(Big &a) { a.n = 1; a.d[0] = 0; }
static inline void setU64(Big &a, uint64_t v) { a.n = 1; a.d[0] = v; }

static inline void badd(Big &r, const Big &a, const Big &b) {
    const int na = a.n, nb = b.n;
    const int n = na > nb ? na : nb;
    uint64_t carry = 0;
    for (int i = 0; i < n; ++i) {
        __uint128_t s = (__uint128_t)(i < na ? a.d[i] : 0) + (i < nb ? b.d[i] : 0) + carry;
        r.d[i] = (uint64_t)s;
        carry = (uint64_t)(s >> 64);
    }
    if (carry) r.d[n] = carry, r.n = n + 1;
    else r.n = n;
#ifdef DEBUG_EXACT
    if (r.n >= g_maxLimbs) { g_maxLimbs = r.n; if (r.n >= MAXL) { fprintf(stderr, "LIMB OVERFLOW in badd\n"); exit(1); } }
#endif
}

// requires a >= b
static inline void bsub(Big &r, const Big &a, const Big &b) {
    const int na = a.n, nb = b.n;
    uint64_t borrow = 0;
    for (int i = 0; i < na; ++i) {
        __uint128_t s = (__uint128_t)a.d[i] - (i < nb ? b.d[i] : 0) - borrow;
        r.d[i] = (uint64_t)s;
        borrow = (uint64_t)(s >> 64) & 1u;
    }
    r.n = na;
    trim(r);
}

static inline void bmulsmall(Big &r, const Big &a, uint64_t m) {
    if (m == 0) { r.n = 1; r.d[0] = 0; return; }
    const int na = a.n;
    uint64_t carry = 0;
    for (int i = 0; i < na; ++i) {
        __uint128_t p = (__uint128_t)a.d[i] * m + carry;
        r.d[i] = (uint64_t)p;
        carry = (uint64_t)(p >> 64);
    }
    int n = na;
    while (carry) { r.d[n++] = carry; carry = 0; }
    r.n = n;
#ifdef DEBUG_EXACT
    if (r.n >= g_maxLimbs) { g_maxLimbs = r.n; if (r.n >= MAXL) { fprintf(stderr, "LIMB OVERFLOW in bmulsmall\n"); exit(1); } }
#endif
}

// exact division by small m
static inline void bdivsmall(Big &r, const Big &a, uint64_t m) {
    __uint128_t cur = 0;
    for (int i = a.n - 1; i >= 0; --i) {
        cur = (cur << 64) | a.d[i];
        r.d[i] = (uint64_t)(cur / m);
        cur %= m;
    }
    r.n = a.n;
    trim(r);
}

static uint64_t bmulTmp[MAXL * 2 + 4];

static void bmul(Big &r, const Big &a, const Big &b) {
    const int need = a.n + b.n;
#ifdef DEBUG_EXACT
    if (need >= g_maxNeed) { g_maxNeed = need; if (need > MAXL) { fprintf(stderr, "BMUL RESULT OVERFLOW need=%d\n", need); exit(1); } }
#endif
    for (int i = 0; i < need; ++i) bmulTmp[i] = 0;
    for (int i = 0; i < a.n; ++i) {
        const uint64_t ai = a.d[i];
        if (!ai) continue;
        __uint128_t carry = 0;
        int j = 0;
        for (; j < b.n; ++j) {
            __uint128_t p = (__uint128_t)ai * b.d[j] + bmulTmp[i + j] + carry;
            bmulTmp[i + j] = (uint64_t)p;
            carry = p >> 64;
        }
        int k = i + j;
        while (carry) {
            __uint128_t p = (__uint128_t)bmulTmp[k] + carry;
            bmulTmp[k] = (uint64_t)p;
            carry = p >> 64;
            ++k;
        }
    }
    r.n = need;
    memcpy(r.d, bmulTmp, (size_t)need * 8);
    trim(r);
#ifdef DEBUG_EXACT
    if (r.n >= g_maxLimbs) { g_maxLimbs = r.n; if (r.n >= MAXL) { fprintf(stderr, "LIMB OVERFLOW in bmul\n"); exit(1); } }
#endif
}

// a / b  as long double; a,b > 0, only ~64 bits of relative accuracy needed
static long double ratioLD(const Big &a, const Big &b) {
    int na = a.n, nb = b.n;
    __uint128_t ha = (na >= 2) ? ((((__uint128_t)a.d[na - 1]) << 64) | a.d[na - 2]) : (__uint128_t)a.d[0];
    int ea = (na >= 2) ? 64 * (na - 2) : 0;
    __uint128_t hb = (nb >= 2) ? ((((__uint128_t)b.d[nb - 1]) << 64) | b.d[nb - 2]) : (__uint128_t)b.d[0];
    int eb = (nb >= 2) ? 64 * (nb - 2) : 0;
    long double r = (long double)ha / (long double)hb;
    return ldexpl(r, ea - eb);
}

int main() {
    long long n_, m_;
    if (scanf("%lld %lld", &n_, &m_) != 2) return 0;
    const int n = (int)n_;
    const long long M = m_ - n_;

    // N = C(M+n, n)
    Big N; setU64(N, 1);
    for (int i = 1; i <= n; ++i) {
        bmulsmall(N, N, (uint64_t)(M + i));
        bdivsmall(N, N, (uint64_t)i);
    }

    // fact = (n+1)!
    Big fact; setU64(fact, 1);
    for (int i = 2; i <= n + 1; ++i) bmulsmall(fact, fact, (uint64_t)i);

    // fscale[p] = (n+1)!/p!
    vector<Big> fscale(n + 2);
    setU64(fscale[n + 1], 1);
    for (int p = n; p >= 0; --p) bmulsmall(fscale[p], fscale[p + 1], (uint64_t)(p + 1));

    const int Lmax = (int)((long long)n < M ? (long long)n : M);

    // c[y] = C(y+n, n), y = 0 .. Ymax
    vector<Big> c;
    if (Lmax > 0) {
        const long long Ymax = (long long)Lmax * (n + 1) + n + 4;
        c.resize((size_t)Ymax + 1);
        setU64(c[0], 1);
        for (long long y = 1; y <= Ymax; ++y) {
            bmulsmall(c[y], c[y - 1], (uint64_t)(y + n));
            bdivsmall(c[y], c[y], (uint64_t)y);
        }
    }

    // Sval[l] = (n+1)! * S_l  where S_l = sum_{x>=1} C(M - l*x + n, n)
    vector<Big> Sval(n + 1);
    for (int i = 0; i <= n; ++i) setU64(Sval[i], 0);

    if (Lmax > 0) {
        const int D = n + 1;                  // V(j) is a polynomial of degree D in j
        vector<Big> V(D + 1), dd(D + 1);
        for (int l = 1; l <= Lmax; ++l) {
            const long long M0 = M % l;
            const long long J = M / l;        // number of terms
            setZero(V[0]);
            for (int j = 1; j <= D; ++j)
                badd(V[j], V[j - 1], c[(size_t)(M0 + (long long)l * (j - 1))]);
            if (J <= (long long)D) {
                bmul(Sval[l], V[J], fact);    // direct value
            } else {
                for (int i = 0; i <= D; ++i) dd[i] = V[i];
                for (int p = 1; p <= D; ++p)
                    for (int u = D; u >= p; --u)
                        bsub(dd[u], dd[u], dd[u - 1]);
                // S_l = sum_p dd[p] * C(J,p)  (Newton forward interpolation)
                // (n+1)! * S_l via Horner in the falling factorial basis
                Big R; R = dd[D];
                Big tmp;
                for (int p = D - 1; p >= 0; --p) {
                    bmulsmall(R, R, (uint64_t)(J - p));
                    bmul(tmp, dd[p], fscale[p]);
                    badd(R, R, tmp);
                }
                Sval[l] = R;
            }
        }
    }

    // G[l] = C(n,l) * Sval[l]
    vector<Big> G(n + 1);
    {
        Big bl; setU64(bl, 1);
        for (int l = 1; l <= n; ++l) {
            bmulsmall(bl, bl, (uint64_t)(n - l + 1));
            bdivsmall(bl, bl, (uint64_t)l);
            if (l <= Lmax) bmul(G[l], bl, Sval[l]);
            else setU64(G[l], 0);
        }
    }

    Big den; bmul(den, fact, N);   // (n+1)! * N

    for (int k = 1; k <= n; ++k) {
        const int L = n - k + 1;
        long double acc;
        Big num;
        if (L == 1) {
            bmulsmall(num, Sval[1], (uint64_t)n);
            acc = ratioLD(num, den);
        } else {
            Big pos, neg, coef, t;
            setZero(pos); setZero(neg);
            setU64(coef, 1);              // C(l-2, L-2)
            for (int l = L; l <= n; ++l) {
                if (l > L) {
                    // C(l-2,L-2) = C(l-3,L-2) * (l-2) / (l-L)
                    bmulsmall(coef, coef, (uint64_t)(l - 2));
                    bdivsmall(coef, coef, (uint64_t)(l - L));
                }
                bmul(t, coef, G[l]);
                if (((l + L) & 1) == 0) badd(pos, pos, t);
                else badd(neg, neg, t);
            }
            // (n+1)! * ans_k * N = pos - neg, and pos >= neg always holds
            // (finite differences of a C(.,n) prefix sum are non-negative here).
            bsub(num, pos, neg);
            acc = ratioLD(num, den);
        }
        long double ans = (long double)k + acc;
#ifdef DEBUG_EXACT
        printf("%d ", k);
        printBig(num);
        printf(" ");
        printBig(den);
        printf("\n");
#else
        printf("%.12Lf\n", ans);
#endif
    }
#ifdef DEBUG_EXACT
    fprintf(stderr, "maxlimbs=%d maxneed=%d (cap %d)\n", g_maxLimbs, g_maxNeed, MAXL);
#endif
    return 0;
}

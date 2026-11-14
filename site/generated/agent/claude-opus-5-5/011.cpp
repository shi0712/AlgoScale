#include <bits/stdc++.h>
#include <immintrin.h>
using namespace std;
typedef long long ll;
typedef __int128 i128;

static ll n, K, A, B;
static const double EPS = 0.01;

static inline i128 V(i128 d, i128 i) { return d * (A * (2 * i + d) + 2 * B); }

// exact floor of t(d) clipped to [0, n-d]
static inline ll exactCnt(ll d, i128 X, double e) {
    ll lim = n - d;
    if (e > (double)lim + 2) return lim;
    if (e < -2) return 0;
    ll i = (ll)floor(e);
    if (i < 0) i = 0;
    while (i + 1 <= lim + 1 && V(d, i + 1) <= X) i++;
    while (i >= 1 && V(d, i) > X) i--;
    return min(i, lim);
}

struct Res { double lo, hi; };

__attribute__((target("avx2,fma"))) static Res loopAvx(ll ds, ll de, double Yh, double c, double kk) {
    // sum over d in [ds, de]
    double lo = 0, hi = 0;
    __m256d vY = _mm256_set1_pd(Yh), vc = _mm256_set1_pd(c), vh = _mm256_set1_pd(0.5);
    __m256d vE = _mm256_set1_pd(EPS), vz = _mm256_setzero_pd(), vn = _mm256_set1_pd((double)n);
    __m256d four = _mm256_set1_pd(4.0);
    ll d = ds;
    while (d + 3 <= de) {
        ll blockEnd = min(de, d + 4096 * 4);
        __m256d vd = _mm256_set_pd((double)(d + 3), (double)(d + 2), (double)(d + 1), (double)d);
        __m256d slo = vz, shi = vz;
        for (; d + 3 <= blockEnd; d += 4) {
            __m256d e = _mm256_sub_pd(_mm256_div_pd(vY, vd), _mm256_add_pd(_mm256_mul_pd(vd, vh), vc));
            __m256d lim = _mm256_sub_pd(vn, vd);
            __m256d fl = _mm256_floor_pd(_mm256_sub_pd(e, vE));
            __m256d fh = _mm256_floor_pd(_mm256_add_pd(e, vE));
            fl = _mm256_min_pd(_mm256_max_pd(fl, vz), lim);
            fh = _mm256_min_pd(_mm256_max_pd(fh, vz), lim);
            slo = _mm256_add_pd(slo, fl);
            shi = _mm256_add_pd(shi, fh);
            vd = _mm256_add_pd(vd, four);
        }
        alignas(32) double t1[4], t2[4];
        _mm256_store_pd(t1, slo);
        _mm256_store_pd(t2, shi);
        lo += t1[0] + t1[1] + t1[2] + t1[3];
        hi += t2[0] + t2[1] + t2[2] + t2[3];
        if (lo >= kk) return {lo, hi};
    }
    for (; d <= de; d++) {
        double dd = (double)d;
        double e = Yh / dd - (0.5 * dd + c);
        double lim = (double)(n - d);
        lo += min(max(floor(e - EPS), 0.0), lim);
        hi += min(max(floor(e + EPS), 0.0), lim);
    }
    return {lo, hi};
}

static Res loopGen(ll ds, ll de, double Yh, double c, double kk) {
    double lo = 0, hi = 0;
    ll d = ds;
    while (d <= de) {
        ll blockEnd = min(de, d + 16384);
        for (; d <= blockEnd; d++) {
            double dd = (double)d;
            double e = Yh / dd - (0.5 * dd + c);
            double lim = (double)(n - d);
            lo += min(max(floor(e - EPS), 0.0), lim);
            hi += min(max(floor(e + EPS), 0.0), lim);
        }
        if (lo >= kk) return {lo, hi};
    }
    return {lo, hi};
}

static bool hasAvx;

// largest d in [0, n-1] with f(d) <= X, f increasing
template <class F> static ll lastLE(i128 X, F f) {
    ll lo = 0, hi = n - 1;
    while (lo < hi) {
        ll m = lo + (hi - lo + 1) / 2;
        if (f(m) <= X) lo = m; else hi = m - 1;
    }
    return lo;
}


static ll D0(i128 X) { return lastLE(X, [](ll d) { return V(d, n - d); }); }
static ll D1(i128 X) { return lastLE(X, [](ll d) { return V(d, 1); }); }

// bounds on count(X): returns pair(lower, upper) as long double; exact flag if known
struct CB { long double lo, hi; };
static CB countBounds(i128 X) {
    ll d0 = D0(X), d1 = D1(X);
    long double base = (long double)((i128)d0 * n - (i128)d0 * (d0 + 1) / 2);
    if (d1 <= d0) return {base, base};
    double Yh = (double)X / (2.0 * (double)A);
    double c = (double)B / (double)A;
    Res r = hasAvx ? loopAvx(d0 + 1, d1, Yh, c, 1e300) : loopGen(d0 + 1, d1, Yh, c, 1e300);
    return {base + r.lo, base + r.hi};
}

static inline ll cntD(ll d, i128 X, double Yh, double c) {
    double dd = (double)d;
    double e = Yh / dd - (0.5 * dd + c);
    double lim = (double)(n - d);
    double fl = floor(e - EPS), fh = floor(e + EPS);
    if (fl == fh || fh <= 0 || fl >= lim) return (ll)min(max(fl, 0.0), lim);
    return exactCnt(d, X, e);
}

static i128 exactCount(i128 X) {
    ll d0 = D0(X), d1 = D1(X);
    i128 s = (i128)d0 * n - (i128)d0 * (d0 + 1) / 2;
    double Yh = (double)X / (2.0 * (double)A);
    double c = (double)B / (double)A;
    for (ll d = d0 + 1; d <= d1; d++) s += cntD(d, X, Yh, c);
    return s;
}
static string toStr(i128 x) {
    if (x == 0) return "0";
    string s;
    while (x > 0) { s += char('0' + (int)(x % 10)); x /= 10; }
    reverse(s.begin(), s.end());
    return s;
}

int main() {
    cin >> n >> K >> A >> B;
    __builtin_cpu_init();
    hasAvx = __builtin_cpu_supports("avx2") && __builtin_cpu_supports("fma");
    i128 hi;
    if (K <= n - 1) hi = V(1, K);
    else hi = V(n - 1, 1);
    i128 lo = 0;
    long double clo = 0;  // lower bound on count(lo) (exact: < K)
    CB hb = countBounds(hi);
    long double chi = hb.hi;  // upper bound of count(hi) (count(hi) >= K)
    const long double M = 3e6;
    long double flo = clo - K, fhi = chi - K;  // for Illinois
    int side = 0;
    i128 lastW = hi - lo;
    int stall = 0;
    while (hi - lo > 1 && chi - clo > M) {
        i128 w = hi - lo;
        i128 m;
        if (stall >= 2) {
            m = lo + w / 2;
            stall = 0;
        } else {
            long double t = -flo / (fhi - flo);
            if (!(t > 0)) t = 0;
            if (!(t < 1)) t = 1;
            m = lo + (i128)((long double)w * t);
        }
        if (m <= lo) m = lo + 1;
        if (m >= hi) m = hi - 1;
        CB b = countBounds(m);
        bool ge;
        if (b.lo >= K) ge = true;
        else if (b.hi < K) ge = false;
        else {
            i128 e = exactCount(m);
            b.lo = b.hi = (long double)e;
            ge = e >= K;
        }
        if (ge) {
            hi = m; chi = b.hi; fhi = chi - K;
            if (side == 1) flo /= 2;
            side = 1;
        } else {
            lo = m; clo = b.lo; flo = clo - K;
            if (side == -1) fhi /= 2;
            side = -1;
        }
        i128 nw = hi - lo;
        if (nw * 2 > lastW) stall++; else { stall = 0; lastW = nw; }
    }
    // enumerate values in (lo, hi]
    i128 X;
    {
        ll d1 = D1(hi), d0 = D0(lo);
        i128 base = (i128)d0 * n - (i128)d0 * (d0 + 1) / 2;
        double Yl = (double)lo / (2.0 * (double)A), Yh = (double)hi / (2.0 * (double)A);
        double c = (double)B / (double)A;
        vector<i128> vals;
        i128 cl = base;
        for (ll d = d0 + 1; d <= d1; d++) {
            ll a1 = cntD(d, lo, Yl, c), a2 = cntD(d, hi, Yh, c);
            cl += a1;
            for (ll i = a1 + 1; i <= a2; i++) vals.push_back(V(d, i));
        }
        ll idx = (ll)(K - cl - 1);
        nth_element(vals.begin(), vals.begin() + idx, vals.end());
        X = vals[idx];
    }

    ll g = (ll)(X % A);
    ll aa = A;
    g = std::gcd(g, aa);
    if (g == 0) g = aa;
    printf("%s %s\n", toStr(X / g).c_str(), toStr((i128)(A / g)).c_str());
}

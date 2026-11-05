#include <bits/stdc++.h>
using namespace std;
using i128 = __int128_t;
using int64 = long long;

static long long isqrt_u64(unsigned long long x) {
    unsigned long long lo = 0, hi = 4000000001ULL;
    while (lo + 1 < hi) {
        unsigned long long md = (lo + hi) >> 1;
        if ((__uint128_t)md * md <= x) lo = md;
        else hi = md;
    }
    return (long long)lo;
}

static unsigned long long floor_sqrt_ratio(long long h, long long s, long long w) {
    i128 q = (i128)h * s / w;
    unsigned long long lo = 0, hi = 1000000000000000001ULL;
    while (lo + 1 < hi) {
        unsigned long long md = (lo + hi) >> 1;
        i128 z = (i128)md * md;
        if (z <= q) lo = md; else hi = md;
    }
    return lo;
}

static long long ceil_div(long long a, long long b) { return a / b + (a % b != 0); }

static long long F(long long h, long long w, long long s, long long x) {
    long long y = s / x;
    return ceil_div(h, x) + ceil_div(w, y);
}

static long long G(long long h, long long w, long long s, long long p) {
    long long x = ceil_div(h, p);
    if (x > s) return (long long)4e18;
    long long y = s / x;
    return p + ceil_div(w, y);
}

static long long solve_one(long long h, long long w, long long s) {
    if (h > w) swap(h, w);
    if ((i128)h * w <= s) return 0;
    long long lim = min(h, s);
    long long best = (long long)4e18;

    if (w > s) {
        long long x0 = (long long)floor_sqrt_ratio(h, s, w);
        x0 = max(1LL, min(x0, lim));
        long long u = F(h, w, s, x0), ux = x0;
        if (x0 < lim && F(h, w, s, x0 + 1) < u) u = F(h, w, s, x0 + 1), ux = x0 + 1;
        auto ok = [&](long long x) {
            // w*x^2 - u*s*x + h*s <= 0, rearranged to avoid overflow.
            i128 a = (i128)u * s - (i128)w * x;
            if (a < 0) return false;
            i128 need = ((i128)h * s + x - 1) / x;
            return need <= a;
        };
        long long L = 1, R = ux;
        while (L < R) { long long md = L + (R - L) / 2; if (ok(md)) R = md; else L = md + 1; }
        long long left = L;
        L = ux; R = lim;
        while (L < R) { long long md = L + (R - L + 1) / 2; if (ok(md)) L = md; else R = md - 1; }
        long long right = L;
        for (long long x = left; x <= right; ++x) best = min(best, F(h, w, s, x));
    } else {
        long long x0 = (long long)floor_sqrt_ratio(h, s, w);
        x0 = max(1LL, min(x0, lim));
        long long u = F(h, w, s, x0), ux = x0;
        if (x0 < lim && F(h, w, s, x0 + 1) < u) u = F(h, w, s, x0 + 1), ux = x0 + 1;
        long long p0 = ceil_div(h, ux);
        long long pmax = min(h, u - 1);
        if (pmax >= 1) {
            p0 = max(1LL, min(p0, pmax));
            auto ok = [&](long long p) {
                // s*p^2 - u*s*p + h*w <= 0, rearranged to avoid overflow.
                i128 a = (i128)u * s - (i128)s * p;
                if (a < 0) return false;
                i128 need = ((i128)h * w + p - 1) / p;
                return need <= a;
            };
            long long L = 1, R = p0;
            while (L < R) { long long md = L + (R - L) / 2; if (ok(md)) R = md; else L = md + 1; }
            long long left = L;
            L = p0; R = pmax;
            while (L < R) { long long md = L + (R - L + 1) / 2; if (ok(md)) L = md; else R = md - 1; }
            long long right = L;
            for (long long p = left; p <= right; ++p) best = min(best, G(h, w, s, p));
        }
    }
    return best - 2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T; if (!(cin >> T)) return 0;
    while (T--) { long long h, w, s; cin >> h >> w >> s; cout << solve_one(h, w, s) << '\n'; }
}

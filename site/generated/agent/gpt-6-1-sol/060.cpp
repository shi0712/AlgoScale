#include <bits/stdc++.h>
using namespace std;
using u64 = unsigned long long;
using u128 = __uint128_t;

static inline u64 ceil_div(u64 a, u64 b) {
    return (a - 1) / b + 1;
}

static u64 isqrt(u128 n) {
    u64 r = (u64)sqrtl((long double)n);
    while ((u128)r * r > n) --r;
    while ((u128)(r + 1) * (r + 1) <= n) ++r;
    return r;
}

struct Range {
    u64 lo = 1, hi = 0;
    u64 size() const { return lo > hi ? 0 : hi - lo + 1; }
};

// If a solution has a+b <= k, its maximum strip height x satisfies
// h/x + w*x/s <= k, or w*x*x + h*s <= k*s*x.
// The integer points satisfying this convex quadratic form one interval.
static Range dimension_range(u64 h, u64 w, u64 s, u64 k) {
    u64 limit = min(h, s);
    u64 center = (u64)min<u128>(limit, (u128)k * s / ((u128)2 * w));
    center = max(1ULL, center);
    const u128 b = (u128)k * s;
    const u128 c = (u128)h * s;
    auto good = [&](u64 x) {
        // Divide the quadratic by x, rounding its sole fractional term up.
        // This keeps every intermediate within 128 bits.
        return (u128)w * x + (c - 1) / x + 1 <= b;
    };
    // The integer minimum is at one of the two integers around the vertex.
    if (!good(center)) {
        if (center == limit || !good(center + 1)) return {};
        ++center;
    }
    u64 l = 1, r = center;
    while (l < r) {
        u64 m = l + (r - l) / 2;
        if (good(m)) r = m;
        else l = m + 1;
    }
    u64 left = l;
    l = center;
    r = limit;
    while (l < r) {
        u64 m = l + (r - l + 1) / 2;
        if (good(m)) l = m;
        else r = m - 1;
    }
    return {left, l};
}

static u64 solve(u64 h, u64 w, u64 s) {
    const u128 area = (u128)h * w;
    if (area <= s) return 0;

    u64 best = h + w; // Number of strips in both directions, i.e. cuts + 2.
    auto try_height = [&](u64 x) {
        x = max(1ULL, min(x, min(h, s)));
        best = min(best, ceil_div(h, x) + ceil_div(w, s / x));
    };
    auto try_width = [&](u64 y) {
        y = max(1ULL, min(y, min(w, s)));
        best = min(best, ceil_div(w, y) + ceil_div(h, s / y));
    };

    // Seed the bound using the continuous optimum, in both parameterizations.
    u64 x = isqrt((u128)h * s / w);
    try_height(x);
    try_height(x + 1);
    u64 y = isqrt((u128)w * s / h);
    try_width(y);
    try_width(y + 1);
    const u128 p = (area - 1) / s + 1;
    u64 q = isqrt(p);
    for (u64 a : {q, q + 1}) {
        try_height(ceil_div(h, max(1ULL, min(h, a))));
        try_width(ceil_div(w, max(1ULL, min(w, a))));
    }

    // Search only solutions strictly better than the current feasible bound.
    u64 k = best - 1;
    if ((u128)k * k < 4 * p) return best - 2;
    u64 d = isqrt((u128)k * k - 4 * p);
    u64 left = (k - d + 1) / 2;
    u64 right = (k + d) / 2;
    // Necessarily ab >= ceil(hw/s) and a+b <= k.
    Range ra{max(left, (u64)((p - 1) / w + 1)), min(right, h)};
    Range rb{max(left, (u64)((p - 1) / h + 1)), min(right, w)};
    if (!ra.size() || !rb.size()) return best - 2;

    int mode = 0;
    Range chosen = ra;
    auto consider = [&](Range r, int m) {
        if (r.size() < chosen.size()) chosen = r, mode = m;
    };
    consider(rb, 1);
    consider(dimension_range(h, w, s, k), 2);
    if (!chosen.size()) return best - 2;
    consider(dimension_range(w, h, s, k), 3);
    if (!chosen.size()) return best - 2;

    // Enumerate whichever interval is shortest: a, b, x, or y.
    // Near the continuous optimum, the count interval has length
    // O(r/sqrt(u) + sqrt(r)), and the shorter-side dimension interval
    // has length O(sqrt(u) + u/sqrt(r)), where
    // r=sqrt(hw/s), u=sqrt(min(h,w)*s/max(h,w)). Their minimum is
    // O(sqrt(u)+1) = O(s^(1/4)+1).
    if (mode == 1 || mode == 3) swap(h, w);
    if (mode < 2) {
        for (u64 a = chosen.lo; a <= chosen.hi; ++a) {
            u64 height = ceil_div(h, a);
            if (height > s) continue;
            u64 b = ceil_div(w, s / height);
            best = min(best, a + b);
        }
    } else {
        for (u64 height = chosen.lo; height <= chosen.hi; ++height) {
            u64 a = ceil_div(h, height);
            u64 b = ceil_div(w, s / height);
            best = min(best, a + b);
        }
    }
    return best - 2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int t;
    cin >> t;
    while (t--) {
        u64 h, w, s;
        cin >> h >> w >> s;
        cout << solve(h, w, s) << '\n';
    }
}

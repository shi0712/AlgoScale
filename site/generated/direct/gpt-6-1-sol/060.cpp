#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static int64 ceil_div(int64 a, int64 b) {
    return (a - 1) / b + 1;
}

static int64 integer_sqrt(int64 n) {
    int64 r = (int64)sqrtl((long double)n);
    while ((i128)(r + 1) * (r + 1) <= n) ++r;
    while ((i128)r * r > n) --r;
    return r;
}

// All integers x in [1, maximum] satisfying
//     A / x + B * x <= C
// form an interval. 'inside' is known to satisfy the inequality.
static pair<int64, int64> find_interval(
    i128 A, i128 B, i128 C,
    int64 maximum, int64 inside
) {
    auto valid = [&](int64 x) -> bool {
        // Exact comparison, without floating-point arithmetic.
        return B * x + (A + x - 1) / x <= C;
    };

    int64 lo = 1, hi = inside;
    while (lo < hi) {
        int64 mid = lo + (hi - lo) / 2;
        if (valid(mid)) hi = mid;
        else lo = mid + 1;
    }
    int64 left = lo;

    lo = inside;
    hi = maximum;
    while (lo < hi) {
        int64 mid = lo + (hi - lo + 1) / 2;
        if (valid(mid)) lo = mid;
        else hi = mid - 1;
    }

    return {left, lo};
}

static int64 solve(int64 h, int64 w, int64 s) {
    if ((i128)h * w <= s) return 0;

    if (h > w) swap(h, w);

    if (h == 1) return ceil_div(w, s) - 1;
    if (s == 1) return h + w - 2;

    const int64 max_a = min(h, s);

    // Continuous optimum for the maximum height of a rectangle:
    // a ~= sqrt(h*s/w).
    int64 a0 = integer_sqrt((int64)((i128)h * s / w));
    a0 = max<int64>(1, min(a0, max_a));

    int64 best = h + w;
    int64 best_a = 1;
    int64 best_p = h;

    auto consider_a = [&](int64 a) {
        if (a < 1 || a > max_a) return;

        int64 b = min(w, s / a);
        int64 p = ceil_div(h, a);
        int64 q = ceil_div(w, b);

        if (p + q < best) {
            best = p + q;
            best_a = a;
            best_p = p;
        }
    };

    consider_a(a0);
    consider_a(a0 + 1);
    consider_a(1);
    consider_a(max_a);

    const i128 C = (i128)best * s;

    // Any improving solution must lie in each corresponding interval.
    auto [al, ar] = find_interval(
        (i128)h * s, (i128)w, C, max_a, best_a
    );
    auto [pl, pr] = find_interval(
        (i128)h * w, (i128)s, C, h, best_p
    );

    // Enumerate whichever parameter has the shorter possible range.
    if (ar - al <= pr - pl) {
        for (int64 a = al; a <= ar; ++a) {
            int64 b = min(w, s / a);
            int64 p = ceil_div(h, a);
            int64 q = ceil_div(w, b);
            best = min(best, p + q);
        }
    } else {
        for (int64 p = pl; p <= pr; ++p) {
            int64 a = ceil_div(h, p);
            if (a > s) continue;

            int64 b = min(w, s / a);
            int64 q = ceil_div(w, b);
            best = min(best, p + q);
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
        int64 h, w, s;
        cin >> h >> w >> s;
        cout << solve(h, w, s) << '\n';
    }

    return 0;
}

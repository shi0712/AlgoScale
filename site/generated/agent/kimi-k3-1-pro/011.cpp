#include <algorithm>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <limits>
#include <numeric>
#include <vector>

using namespace std;

using u128 = unsigned __int128;
using u64 = unsigned long long;
using i64 = long long;

struct Thresholds {
    i64 full = 0;
    i64 positive = 0;
};

struct Solver {
    i64 n;
    u64 k;
    u64 a;
    u64 b;
    u128 maxValue;

    u128 zeroThreshold(i64 d) const {
        return (u128)d * ((u128)a * d + 2 * (u128)b);
    }

    u128 fullThreshold(i64 d) const {
        return (u128)d * (2 * (u128)a * n + 2 * (u128)b - (u128)a * d);
    }

    Thresholds thresholds(u128 x) const {
        Thresholds t;
        i64 lo = 0, hi = n - 1;
        while (lo < hi) {
            i64 mid = lo + (hi - lo + 1) / 2;
            if (fullThreshold(mid) <= x)
                lo = mid;
            else
                hi = mid - 1;
        }
        t.full = lo;

        lo = 0;
        hi = n - 1;
        while (lo < hi) {
            i64 mid = lo + (hi - lo + 1) / 2;
            if (zeroThreshold(mid) < x)
                lo = mid;
            else
                hi = mid - 1;
        }
        t.positive = lo;
        return t;
    }

    static long double harmonic(i64 x) {
        if (x == 0)
            return 0;
        if (x < 10000) {
            long double result = 0;
            for (i64 i = 1; i <= x; ++i)
                result += 1.0L / i;
            return result;
        }
        long double y = x;
        long double y2 = y * y;
        return logl(y) + 0.577215664901532860606512090082402431L
               + 1 / (2 * y) - 1 / (12 * y2) + 1 / (120 * y2 * y2)
               - 1 / (252 * y2 * y2 * y2)
               + 1 / (240 * y2 * y2 * y2 * y2);
    }

    long double approximateCount(u128 x) const {
        Thresholds t = thresholds(x);
        long double full = (long double)t.full * n
                           - (long double)t.full * (t.full + 1) / 2;
        i64 m = t.positive - t.full;
        if (m == 0)
            return full;
        long double sumD = ((long double)t.positive * (t.positive + 1)
                            - (long double)t.full * (t.full + 1)) / 2;
        long double result = full;
        result += (long double)x / (2 * (long double)a)
                  * (harmonic(t.positive) - harmonic(t.full));
        result -= sumD / 2;
        result -= (long double)b / a * m;
        return result;
    }

    u128 approximateLower(long double target) const {
        if (target <= 0)
            return 0;
        u128 lo = 0, hi = maxValue;
        while (lo < hi) {
            u128 mid = lo + (hi - lo + 1) / 2;
            if (approximateCount(mid) < target)
                lo = mid;
            else
                hi = mid - 1;
        }
        return lo;
    }

    u128 approximateUpper(long double target) const {
        if (target <= 0)
            return 0;
        if (approximateCount(maxValue) < target)
            return maxValue;
        u128 lo = 0, hi = maxValue;
        while (lo < hi) {
            u128 mid = lo + (hi - lo) / 2;
            if (approximateCount(mid) >= target)
                hi = mid;
            else
                lo = mid + 1;
        }
        return lo;
    }

    u64 rowCount(u128 x, i64 d, const Thresholds &t) const {
        if (d <= t.full)
            return n - d;
        if (d > t.positive)
            return 0;
        u128 numerator = x - zeroThreshold(d);
        u128 denominator = 2 * (u128)a * d;
        u128 result = numerator / denominator;
        u128 cap = n - d;
        return (u64)min(result, cap);
    }

    u64 exactCount(u128 x, u64 limit, Thresholds *out = nullptr) const {
        Thresholds t = thresholds(x);
        if (out)
            *out = t;
        u128 total = 0;
        for (i64 d = 1; d <= t.positive; ++d) {
            total += rowCount(x, d, t);
            if (total >= limit)
                return limit;
        }
        return (u64)total;
    }

    u128 value(i64 d, u64 i) const {
        return (u128)d * ((u128)a * (2 * (u128)i + d) + 2 * (u128)b);
    }

    template <typename T>
    u128 selectInBand(u128 lower, u128 upper, const Thresholds &tl,
                      const Thresholds &tu, u64 rank) const {
        vector<T> values;
        for (i64 d = 1; d <= tu.positive; ++d) {
            u64 left = rowCount(lower, d, tl);
            u64 right = rowCount(upper, d, tu);
            for (u64 i = left + 1; i <= right; ++i)
                values.push_back((T)(value(d, i) - lower));
        }
        if (rank == 0 || values.size() < rank)
            return maxValue + 1;
        nth_element(values.begin(), values.begin() + (rank - 1), values.end());
        return lower + (u128)values[rank - 1];
    }

    u128 solve() const {
        long double targetLower = k > 1 ? (long double)k - 1 : 0;
        u128 lower = approximateLower(targetLower);
        Thresholds tl;
        u64 countLower = exactCount(lower, k, &tl);

        while (countLower >= k) {
            long double reduction = tl.positive - tl.full + 3;
            targetLower = max(0.0L, targetLower - reduction);
            u128 next = approximateLower(targetLower);
            if (next >= lower) {
                if (lower == 0)
                    break;
                next = lower - 1;
            }
            lower = next;
            countLower = exactCount(lower, k, &tl);
        }

        u64 rank = k - countLower;
        i64 errorRows = tl.positive - tl.full;
        long double targetUpper = (long double)k + errorRows + 1;
        u128 upper = approximateUpper(targetUpper);

        for (int attempt = 0; attempt < 20; ++attempt) {
            if (upper <= lower)
                upper = lower + 1;
            Thresholds tu = thresholds(upper);
            u128 answer;
            if (upper - lower <= numeric_limits<u64>::max())
                answer = selectInBand<u64>(lower, upper, tl, tu, rank);
            else
                answer = selectInBand<u128>(lower, upper, tl, tu, rank);
            if (answer <= maxValue)
                return answer;

            i64 newErrorRows = tu.positive - tu.full;
            long double nextTarget = (long double)k + newErrorRows + 1;
            if (nextTarget <= targetUpper)
                nextTarget = targetUpper + rank + newErrorRows + 1;
            targetUpper = nextTarget;
            if (upper == maxValue)
                break;
            upper = approximateUpper(targetUpper);
        }

        u128 lo = lower, hi = maxValue;
        while (lo + 1 < hi) {
            u128 mid = lo + (hi - lo) / 2;
            if (exactCount(mid, k) >= k)
                hi = mid;
            else
                lo = mid;
        }
        Thresholds tu = thresholds(hi);
        if (hi - lower <= numeric_limits<u64>::max())
            return selectInBand<u64>(lower, hi, tl, tu, rank);
        return selectInBand<u128>(lower, hi, tl, tu, rank);
    }
};

void printU128(u128 x) {
    if (x == 0) {
        cout << '0';
        return;
    }
    char buffer[64];
    int length = 0;
    while (x > 0) {
        buffer[length++] = char('0' + x % 10);
        x /= 10;
    }
    while (length--)
        cout << buffer[length];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    if (!(cin >> solver.n >> solver.k >> solver.a >> solver.b))
        return 0;
    solver.maxValue = solver.value(solver.n - 1, 1);

    u128 numerator = solver.solve();
    u64 denominator = solver.a;
    u64 divisor = gcd((u64)(numerator % denominator), denominator);
    numerator /= divisor;
    denominator /= divisor;

    printU128(numerator);
    cout << ' ' << denominator << '\n';
    return 0;
}

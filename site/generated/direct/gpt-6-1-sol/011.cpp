#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;
using ld = long double;

class Solver {
    u64 n, k, a, b;
    u128 maximumLength;

    static constexpr int HARMONIC_TABLE_SIZE = 1024;
    ld harmonic[HARMONIC_TABLE_SIZE + 1]{};

    struct CountResult {
        u128 count;
        u64 fullRows;
        u64 lastRow;
    };

    static ld toLD(u128 x) {
        return static_cast<ld>(x);
    }

    static string decimal(u128 x) {
        if (x == 0) return "0";
        string s;
        while (x != 0) {
            s.push_back(char('0' + x % 10));
            x /= 10;
        }
        reverse(s.begin(), s.end());
        return s;
    }

    // Minimum and maximum numerator in row d.
    u128 rowMinimum(u64 d) const {
        return u128(d) * (u128(a) * (d + 2) + 2 * u128(b));
    }

    u128 rowMaximum(u64 d) const {
        return u128(d) * (u128(a) * (2 * n - d) + 2 * u128(b));
    }

    static u64 quotient(u128 x, u64 d) {
        if (x <= numeric_limits<u64>::max()) {
            return static_cast<u64>(x) / d;
        }
        return static_cast<u64>(x / d);
    }

    // Called only for a row which is neither empty nor completely included.
    u64 partialRowCount(u128 x, u64 d) const {
        u64 t = quotient(x, d);
        u64 subtract = a * d + 2 * b;
        return (t - subtract) / (2 * a);
    }

    CountResult exactCount(u128 x) const {
        if (x >= maximumLength) {
            return {u128(n) * (n - 1) / 2, n - 1, n - 1};
        }

        // Last nonempty row:
        // a*d^2 + 2*(a+b)*d <= x.
        ld xx = toLD(x);
        ld B = static_cast<ld>(a) + b;
        ld root = xx / (B + sqrtl(B * B + static_cast<ld>(a) * xx));

        u64 last = static_cast<u64>(
            min<ld>(n - 1, max<ld>(0, floorl(root)))
        );

        while (last < n - 1 && rowMinimum(last + 1) <= x) ++last;
        while (last > 0 && rowMinimum(last) > x) --last;

        // Completely included rows:
        // d * (a*(2*n-d) + 2*b) <= x.
        B = static_cast<ld>(a) * n + b;
        ld discriminant = max<ld>(0, B * B - static_cast<ld>(a) * xx);
        root = xx / (B + sqrtl(discriminant));

        u64 full = static_cast<u64>(
            min<ld>(last, max<ld>(0, floorl(root)))
        );

        while (full < last && rowMaximum(full + 1) <= x) ++full;
        while (full > 0 && rowMaximum(full) > x) --full;

        u128 answer = u128(full) * n - u128(full) * (full + 1) / 2;

        for (u64 d = full + 1; d <= last; ++d) {
            answer += partialRowCount(x, d);
        }

        return {answer, full, last};
    }

    ld harmonicNumber(u64 x) const {
        if (x <= HARMONIC_TABLE_SIZE) return harmonic[x];

        static const ld gamma =
            0.577215664901532860606512090082402431L;

        ld inv = 1.0L / static_cast<ld>(x);
        ld z = inv * inv;

        // Euler--Maclaurin expansion.
        ld correction =
            z * (-1.0L / 12 +
            z * ( 1.0L / 120 +
            z * (-1.0L / 252 +
            z * ( 1.0L / 240 +
            z * (-1.0L / 132)))));

        return logl(static_cast<ld>(x)) + gamma + inv / 2 + correction;
    }

    ld harmonicDifference(u64 left, u64 right) const {
        if (left == right) return 0;
        if (left <= HARMONIC_TABLE_SIZE) {
            return harmonicNumber(right) - harmonic[left];
        }

        // Evaluate the difference directly, avoiding cancellation of logarithms.
        ld l = static_cast<ld>(left);
        ld r = static_cast<ld>(right);
        ld il = 1.0L / l;
        ld ir = 1.0L / r;

        auto correction = [](ld inv) {
            ld z = inv * inv;
            return z * (-1.0L / 12 +
                   z * ( 1.0L / 120 +
                   z * (-1.0L / 252 +
                   z * ( 1.0L / 240 +
                   z * (-1.0L / 132)))));
        };

        return log1pl((r - l) / l)
             + (ir - il) / 2
             + correction(ir) - correction(il);
    }

    /*
       Continuous count:
       sum_d clamp(x/(2*a*d) - d/2 - b/a - shift, 0, n-d).

       shift = 0 gives an upper bound on the exact count.
       shift = 1 gives a lower bound.
    */
    ld continuousCount(ld x, int shift) const {
        if (x <= 0) return 0;

        ld aa = static_cast<ld>(a);
        ld bb = static_cast<ld>(b) + aa * shift;

        ld root = x / (bb + sqrtl(bb * bb + aa * x));
        u64 last = static_cast<u64>(
            min<ld>(n - 1, max<ld>(0, floorl(root)))
        );

        ld allRowsThreshold =
            static_cast<ld>(n - 1) *
            (aa * (static_cast<ld>(n) + 1) + 2 * bb);

        u64 full;
        if (x >= allRowsThreshold) {
            full = n - 1;
        } else {
            ld B = aa * n + bb;
            ld discriminant = max<ld>(0, B * B - aa * x);
            root = x / (B + sqrtl(discriminant));
            full = static_cast<u64>(
                min<ld>(last, max<ld>(0, floorl(root)))
            );
        }

        ld answer =
            static_cast<ld>(full) *
            (static_cast<ld>(n) - (static_cast<ld>(full) + 1) / 2);

        if (full < last) {
            ld rows = static_cast<ld>(last - full);
            answer +=
                x / (2 * aa) * harmonicDifference(full, last)
                - rows * (
                    (static_cast<ld>(last) + full + 1) / 4
                    + static_cast<ld>(b) / aa + shift
                );
        }

        return max<ld>(0, answer);
    }

    ld approximateRoot(int shift, u128 upperBound) const {
        ld left = 0;
        ld right = toLD(upperBound);

        if (continuousCount(right, shift) < static_cast<ld>(k)) {
            return right;
        }

        for (int iteration = 0; iteration < 90; ++iteration) {
            ld middle = (left + right) / 2;
            if (continuousCount(middle, shift) >= static_cast<ld>(k)) {
                right = middle;
            } else {
                left = middle;
            }
        }
        return right;
    }

public:
    void run() {
        cin >> n >> k >> a >> b;

        for (int i = 1; i <= HARMONIC_TABLE_SIZE; ++i) {
            harmonic[i] = harmonic[i - 1] + 1.0L / i;
        }

        maximumLength =
            u128(n - 1) * (u128(a) * (n + 1) + 2 * u128(b));

        u128 total = u128(n) * (n - 1) / 2;
        if (total == k) {
            u64 g = gcd(a, static_cast<u64>(maximumLength % a));
            cout << decimal(maximumLength / g) << ' ' << a / g << '\n';
            return;
        }

        // All pairs among the first m indices already provide at least k values.
        u64 m = static_cast<u64>(sqrtl(2.0L * k)) + 1;
        m = min(m, n);

        while (u128(m) * (m - 1) / 2 < k) ++m;
        while (m > 2 && u128(m - 1) * (m - 2) / 2 >= k) --m;

        u128 upperBound =
            u128(m - 1) * (u128(a) * (m + 1) + 2 * u128(b));

        ld lowerRoot = approximateRoot(0, upperBound);
        ld upperRoot = approximateRoot(1, upperBound);

        // Floating point is only used to suggest an interval.
        // Its validity is checked below using exact integer counts.
        ld lowerMargin = max<ld>(2, fabsl(lowerRoot) * 1e-15L);
        ld upperMargin = max<ld>(2, fabsl(upperRoot) * 1e-15L);

        u128 low = static_cast<u128>(
            floorl(max<ld>(0, lowerRoot - lowerMargin))
        );

        u128 high;
        if (upperRoot + upperMargin >= toLD(upperBound)) {
            high = upperBound;
        } else {
            high = static_cast<u128>(ceill(upperRoot + upperMargin));
            high = min(high, upperBound);
        }

        low = min(low, upperBound);
        if (high < low) high = low;

        CountResult lowInfo = exactCount(low);
        CountResult highInfo = exactCount(high);

        // Repair the suggested interval, if necessary.
        u128 step = max<u128>(1, high - low);
        while (lowInfo.count >= k) {
            low = low > step ? low - step : 0;
            lowInfo = exactCount(low);
            step *= 2;
        }

        step = max<u128>(1, high - low);
        while (highInfo.count < k) {
            high = min(upperBound, high + step);
            highInfo = exactCount(high);
            step *= 2;
        }

        /*
           Keep the candidate array comfortably below the memory limit.
           Also ensure every candidate can be stored as a 64-bit offset.
        */
        constexpr u64 MAX_CANDIDATES = 12'000'000;

        while (highInfo.count - lowInfo.count > MAX_CANDIDATES ||
               high - low > numeric_limits<u64>::max()) {
            u128 middle = low + (high - low) / 2;
            CountResult middleInfo = exactCount(middle);

            if (middleInfo.count >= k) {
                high = middle;
                highInfo = middleInfo;
            } else {
                low = middle;
                lowInfo = middleInfo;
            }
        }

        u64 candidateCount =
            static_cast<u64>(highInfo.count - lowInfo.count);

        vector<u64> candidates;
        candidates.reserve(candidateCount);

        // Enumerate exactly the values in (low, high].
        for (u64 d = lowInfo.fullRows + 1; d <= highInfo.lastRow; ++d) {
            u64 lowerIndex = 0;
            if (d <= lowInfo.lastRow) {
                lowerIndex = partialRowCount(low, d);
            }

            u64 upperIndex;
            if (d <= highInfo.fullRows) {
                upperIndex = n - d;
            } else {
                upperIndex = partialRowCount(high, d);
            }

            if (lowerIndex >= upperIndex) continue;

            u128 value =
                u128(d) *
                (u128(a) * (2 * (lowerIndex + 1) + d) + 2 * u128(b));

            u128 increment = 2 * u128(a) * d;

            for (u64 i = lowerIndex + 1; i <= upperIndex; ++i) {
                candidates.push_back(static_cast<u64>(value - low));
                value += increment;
            }
        }

        size_t target =
            static_cast<size_t>(u128(k) - lowInfo.count - 1);

        nth_element(
            candidates.begin(),
            candidates.begin() + target,
            candidates.end()
        );

        u128 numerator = low + candidates[target];
        u64 g = gcd(a, static_cast<u64>(numerator % a));

        cout << decimal(numerator / g) << ' ' << a / g << '\n';
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    solver.run();
    return 0;
}

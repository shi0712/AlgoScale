#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;

static constexpr long double TWO64_LD = 18446744073709551616.0L;
static constexpr double TWO64_D = 18446744073709551616.0;

u64 N, A, C; // Reduced coefficients: z = g * d * (A*(2*i+d)+C)

static inline long double toLD(u128 x) {
    return (long double)(u64)(x >> 64) * TWO64_LD + (long double)(u64)x;
}

static inline double toDouble(u128 x) {
    return (double)(u64)(x >> 64) * TWO64_D + (double)(u64)x;
}

static u128 fromLD(long double x) {
    if (!(x > 0)) return 0;
    long double hd = floorl(x / TWO64_LD);
    if (hd < 0) hd = 0;
    u64 h = (u64)hd;
    long double r = x - (long double)h * TWO64_LD;
    if (r < 0) {
        --h;
        r += TWO64_LD;
    }
    if (r >= TWO64_LD) {
        ++h;
        r = 0;
    }
    return ((u128)h << 64) + (u64)r;
}

// Number of pairs whose difference j-i is at most d.
static inline u128 prefixPairs(u64 d) {
    return (u128)d * N - (u128)d * (d + 1) / 2;
}

// Smallest value in column d, at i=1.
static inline u128 minValue(u64 d) {
    if (d == 0) return 0;
    return (u128)d * (A * (d + 2) + C);
}

// Largest value in column d, at i=N-d.
static inline u128 maxValue(u64 d) {
    if (d == 0) return 0;
    return (u128)d * (A * (2 * N - d) + C);
}

static inline u128 cellValue(u64 d, u64 i) {
    return (u128)d * (A * (2 * i + d) + C);
}

struct Bounds {
    u64 full; // Columns 1..full are completely included.
    u64 any;  // Columns full+1..any are partially included.
};

static Bounds getBounds(u128 y) {
    u64 lo = 0, hi = N - 1;
    while (lo < hi) {
        u64 mid = lo + (hi - lo + 1) / 2;
        if (minValue(mid) <= y) lo = mid;
        else hi = mid - 1;
    }
    u64 any = lo;

    lo = 0;
    hi = any;
    while (lo < hi) {
        u64 mid = lo + (hi - lo + 1) / 2;
        if (maxValue(mid) <= y) lo = mid;
        else hi = mid - 1;
    }
    return {lo, any};
}

struct CountResult {
    u64 count;
    u64 full;
    u64 any;
};

// Returns min(actual count, limit).
static CountResult countLE(u128 y, u64 limit) {
    Bounds b = getBounds(y);
    if (b.any == 0) return {0, b.full, b.any};

    u128 complete = prefixPairs(b.full);
    if (complete >= limit) return {limit, b.full, b.any};

    u64 total = (u64)complete;

    for (u64 d = b.full + 1; d <= b.any; ++d) {
        // In this column:
        // value(i) = d*B + den*i, den = 2*A*d.
        u64 B = A * d + C;
        u64 den = 2 * A * d;
        u64 cap = N - d;

        u128 rest = y - (u128)d * B;
        // The exact answer is floor(rest/den), already known to be < cap.

        double estimate = toDouble(rest) / (double)den;
        u64 m;
        if (estimate <= 0) m = 0;
        else if (estimate >= (double)cap) m = cap;
        else m = (u64)estimate;

        auto allowed = [&](u64 t) -> bool {
            return (u128)den * t <= rest;
        };

        bool fallback = false;
        int adjustments = 0;

        if (m > 0 && !allowed(m)) {
            while (m > 0 && !allowed(m)) {
                --m;
                if (++adjustments > 8) {
                    fallback = true;
                    break;
                }
            }
        } else {
            while (m < cap && allowed(m + 1)) {
                ++m;
                if (++adjustments > 8) {
                    fallback = true;
                    break;
                }
            }
        }

        if (fallback) {
            m = (u64)(rest / (u128)den);
            if (m > cap) m = cap;
        }

        total += m;
        if (total >= limit) return {limit, b.full, b.any};
    }

    return {total, b.full, b.any};
}

static long double harmonic(u64 x) {
    if (x == 0) return 0;
    if (x < 32) {
        long double s = 0;
        for (u64 i = 1; i <= x; ++i) s += 1.0L / i;
        return s;
    }
    long double t = x;
    long double t2 = t * t;
    return logl(t) + 0.577215664901532860606512L
           + 1.0L / (2 * t)
           - 1.0L / (12 * t2)
           + 1.0L / (120 * t2 * t2);
}

// Continuous approximation, used only to propose search points.
static long double approximateCount(u128 y) {
    Bounds b = getBounds(y);
    long double result = toLD(prefixPairs(b.full));

    if (b.any > b.full) {
        u64 f = b.full, d = b.any;
        long double h = harmonic(d) - harmonic(f);
        long double sumD = (long double)d * (d + 1) / 2;
        long double sumF = (long double)f * (f + 1) / 2;

        result += toLD(y) / (2.0L * A) * h;
        result -= (sumD - sumF) / 2;
        result -= (long double)C * (d - f) / (2.0L * A);
        result -= 0.5L * (d - f); // Average effect of the floor operation.
    }

    return result > 0 ? result : 0;
}

static u128 selectValue(u64 k, u128 totalPairs) {
    if ((u128)k == totalPairs) return maxValue(N - 1);
    if (k == 1) return minValue(1);

    // A guaranteed upper bound: take complete initial columns.
    u64 lo = 0, hi = N - 1;
    while (lo < hi) {
        u64 mid = lo + (hi - lo) / 2;
        if (prefixPairs(mid) >= (u128)k) hi = mid;
        else lo = mid + 1;
    }
    u128 upper = maxValue(lo);

    // Often much sharper: the first k entries of column d=1.
    if (k <= N - 1) upper = min(upper, cellValue(1, k));

    u128 lim128 = (u128)k * 4;
    if (lim128 > totalPairs) lim128 = totalPairs;
    u64 limit = (u64)lim128;

    // Approximately invert the continuous counting function.
    u128 alo = 0, ahi = upper;
    for (int it = 0; it < 80 && ahi - alo > 1; ++it) {
        u128 mid = alo + (ahi - alo) / 2;
        if (approximateCount(mid) < (long double)k) alo = mid;
        else ahi = mid;
    }

    u128 below = 0, above = upper;
    u64 belowCount = 0, aboveCount = limit;

    u128 x = ahi;
    if (x == 0) x = 1;
    if (x >= upper) x = upper - 1;

    CountResult res = countLE(x, limit);
    bool lastBelow;
    if (res.count >= k) {
        above = x;
        aboveCount = res.count;
        lastBelow = false;
    } else {
        below = x;
        belowCount = res.count;
        lastBelow = true;
    }

    // Obtain a close bracket around the answer using local density estimates.
    long double boost = 1;
    for (int pre = 0; pre < 6 && above - below > 1; ++pre) {
        if (below != 0 && above != upper) break;

        long double density =
            (harmonic(res.any) - harmonic(res.full)) / (2.0L * A);
        if (!(density > 1e-30L)) {
            density = (long double)N /
                      (2.0L * A * N + C);
        }

        long double gap = lastBelow
            ? (long double)k - res.count
            : (long double)res.count - k;
        long double uncertainty = gap + (res.any - res.full) + 64;
        long double stepLD = boost * (2 * uncertainty / density + 64);

        if (!(stepLD >= 64)) stepLD = 64;
        u128 step;
        if (!isfinite(stepLD) || stepLD >= toLD(upper)) step = upper;
        else step = fromLD(stepLD);
        if (step < 64) step = 64;

        u128 next;
        if (lastBelow) {
            if (step >= above - x) next = x + (above - x) / 2;
            else next = x + step;
        } else {
            if (step >= x - below) next = below + (x - below) / 2;
            else next = x - step;
        }

        if (next <= below || next >= above)
            next = below + (above - below) / 2;
        if (next <= below || next >= above) break;

        x = next;
        res = countLE(x, limit);
        bool nowBelow;
        if (res.count >= k) {
            above = x;
            aboveCount = res.count;
            nowBelow = false;
        } else {
            below = x;
            belowCount = res.count;
            nowBelow = true;
        }

        if (nowBelow == lastBelow) boost *= 2;
        else boost = 1;
        lastBelow = nowBelow;
    }

    // Exact interpolation search, with a binary-search fallback.
    int stalls = 0;
    u128 previousRange = above - below;

    for (int iteration = 0; above - below > 1; ++iteration) {
        u128 range = above - below;
        u128 candidate = 0;
        bool useMidpoint = (iteration >= 32 || stalls >= 2);

        if (!useMidpoint) {
            long double proposed = -1;
            bool haveProposal = false;

            if (below != 0 && belowCount != 0) {
                long double denominator =
                    logl(toLD(above) / toLD(below));
                long double alpha =
                    logl((long double)aboveCount / belowCount) / denominator;

                if (isfinite(alpha) && alpha > 0.02L && alpha < 50.0L) {
                    long double targetLog =
                        logl(toLD(below)) +
                        logl((long double)k / belowCount) / alpha;
                    proposed = expl(targetLog);
                    haveProposal = isfinite(proposed);
                }
            }

            if (!haveProposal) {
                long double ratio =
                    ((long double)k - belowCount) /
                    ((long double)aboveCount - belowCount);
                proposed = toLD(below) + toLD(range) * ratio;
            }

            if (isfinite(proposed) && proposed > 0)
                candidate = fromLD(proposed);
        }

        if (useMidpoint || candidate <= below || candidate >= above) {
            candidate = below + range / 2;
            if (stalls >= 2) stalls = 0;
        }

        CountResult cur = countLE(candidate, limit);
        if (cur.count >= k) {
            above = candidate;
            aboveCount = cur.count;
        } else {
            below = candidate;
            belowCount = cur.count;
        }

        u128 newRange = above - below;
        if (newRange >= previousRange - previousRange / 100) ++stalls;
        else stalls = 0;
        previousRange = newRange;
    }

    return above;
}

static string decimal(u128 x) {
    if (x == 0) return "0";
    string s;
    while (x) {
        s.push_back(char('0' + x % 10));
        x /= 10;
    }
    reverse(s.begin(), s.end());
    return s;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    u64 k, a, b;
    cin >> N >> k >> a >> b;

    u64 originalA = a;
    u64 doubledB = 2 * b;
    u64 g = std::gcd(a, doubledB);

    A = a / g;
    C = doubledB / g;

    u128 totalPairs = (u128)N * (N - 1) / 2;
    u128 selected = selectValue(k, totalPairs);

    // Original integer numerator z = g * selected; requested length = z/a.
    u128 z = (u128)g * selected;
    u64 rem = (u64)(z % originalA);
    u64 divisor = std::gcd(originalA, rem);

    u128 p = z / divisor;
    u64 q = originalA / divisor;

    cout << decimal(p) << ' ' << q << '\n';
    return 0;
}

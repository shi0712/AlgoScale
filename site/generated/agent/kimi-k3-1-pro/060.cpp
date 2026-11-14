#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>

using int64 = long long;
using i128 = __int128_t;

struct Solver {
    int64 h, w, s;
    int64 best;

    int64 ceilDiv(int64 a, int64 b) const {
        return a / b + (a % b != 0);
    }

    void tryA(int64 a) {
        if (a < 1 || a > h) return;
        int64 x = ceilDiv(h, a);
        if (x > s) return;
        int64 y = s / x;
        int64 b = ceilDiv(w, y);
        if (b <= w) best = std::min(best, a + b);
    }

    void tryB(int64 b) {
        if (b < 1 || b > w) return;
        int64 y = ceilDiv(w, b);
        if (y > s) return;
        int64 x = s / y;
        int64 a = ceilDiv(h, x);
        if (a <= h) best = std::min(best, a + b);
    }

    i128 maxProduct(int64 k) const {
        return i128(k / 2) * ((k + 1) / 2);
    }

    int64 lowerBound(i128 need) const {
        int64 k = static_cast<int64>(2 * std::sqrt(static_cast<long double>(need)));
        if (k < 2) k = 2;
        while (maxProduct(k) < need) ++k;
        while (k > 2 && maxProduct(k - 1) >= need) --k;
        return k;
    }

    void scanA(int64 limit) {
        limit = std::min({limit, h, best - 1});
        for (int64 a = 1; a <= limit; ++a) tryA(a);
    }

    void scanX(int64 k, long double q, bool horizontal) {
        int64 dimension = horizontal ? h : w;
        int64 other = horizontal ? w : h;
        int64 hi = std::min(dimension, s);

        long double center = static_cast<long double>(k) * s / (2.0L * other);
        long double radius = std::sqrt(std::max(0.0L,
            (static_cast<long double>(k) - 2 * q) *
            (static_cast<long double>(k) + 2 * q))) * s / (2.0L * other);
        long double left = std::max(1.0L, center - radius);
        long double right = std::min(static_cast<long double>(hi), center + radius);
        if (left > right) return;

        int64 lo = std::max<int64>(1, static_cast<int64>(std::ceil(left)) - 8);
        int64 end = std::min<int64>(hi, static_cast<int64>(std::floor(right)) + 8);
        for (int64 x = lo; x <= end; ++x) {
            if (horizontal) {
                int64 a = ceilDiv(h, x);
                int64 y = std::min(w, s / x);
                int64 b = ceilDiv(w, y);
                best = std::min(best, a + b);
            } else {
                int64 b = ceilDiv(w, x);
                int64 y = std::min(h, s / x);
                int64 a = ceilDiv(h, y);
                best = std::min(best, a + b);
            }
        }
    }

    int64 run() {
        if (i128(h) * w <= s) return 0;

        best = h + w;
        long double q = std::sqrt(static_cast<long double>(h) * w / s);
        int64 center = static_cast<int64>(q);
        for (int d = -4; d <= 4; ++d) {
            int64 a = std::max<int64>(1, std::min<int64>(h, center + d));
            int64 b = std::max<int64>(1, std::min<int64>(w, center + d));
            tryA(a);
            tryB(b);
        }
        tryA(1);
        tryA(h);
        tryB(1);
        tryB(w);

        i128 need = (i128(h) * w + s - 1) / s;
        int64 lower = lowerBound(need);
        if (best == lower) return best - 2;

        if (best <= 100000) {
            scanA(best - 1);
            return best - 2;
        }

        int64 k = best - 1;
        long double common = std::sqrt(std::max(0.0L,
            (static_cast<long double>(k) - 2 * q) *
            (static_cast<long double>(k) + 2 * q))) * s;
        long double widthX = common / w;
        long double widthY = common / h;
        scanX(k, q, widthX <= widthY);
        return best - 2;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int t;
    std::cin >> t;
    while (t--) {
        Solver solver;
        std::cin >> solver.h >> solver.w >> solver.s;
        std::cout << solver.run() << '\n';
    }
}

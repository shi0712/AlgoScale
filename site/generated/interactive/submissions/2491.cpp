#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <utility>

using int64 = long long;
using uint64 = unsigned long long;

class Solver {
public:
    explicit Solver(int n) : n_(n) {}

    void run() {
        int64 total = query(1, n_);
        int64 answer = 1;
        solve(1, n_, total, answer);
        std::cout << "! " << answer << '\n' << std::flush;
    }

private:
    int n_;

    static std::pair<int64, int64> bounds(int len, int64 sum) {
        uint64 average = (static_cast<uint64>(sum) + len - 1) / len;
        uint64 low = 1;
        while (low < average) low <<= 1;

        uint64 capacity = static_cast<uint64>(sum) - len + 1;
        uint64 high = 1ULL << (63 - __builtin_clzll(capacity));
        return {static_cast<int64>(low), static_cast<int64>(high)};
    }

    int64 query(int left, int right) {
        std::cout << "? " << left << ' ' << right << '\n' << std::flush;
        int64 sum;
        if (!(std::cin >> sum) || sum == -1) std::exit(0);
        return sum;
    }

    void consider(int left, int right, int64 sum, int64 &best) {
        auto [low, high] = bounds(right - left + 1, sum);
        if (high <= best) return;
        if (low == high) {
            best = std::max(best, low);
            return;
        }
        solve(left, right, sum, best);
    }

    void solve(int left, int right, int64 sum, int64 &best) {
        int len = right - left + 1;
        auto [low, high] = bounds(len, sum);
        if (high <= best) return;
        if (low == high) {
            best = std::max(best, low);
            return;
        }

        int middle = left + len / 2 - 1;
        int64 left_sum = query(left, middle);
        int64 right_sum = sum - left_sum;

        int left_len = middle - left + 1;
        int right_len = right - middle;
        auto left_bounds = bounds(left_len, left_sum);
        auto right_bounds = bounds(right_len, right_sum);

        auto priority = [](const std::pair<int64, int64> &a,
                           const std::pair<int64, int64> &b) {
            bool a_exact = a.first == a.second;
            bool b_exact = b.first == b.second;
            if (a_exact != b_exact) return a_exact;
            if (a.second != b.second) return a.second > b.second;
            return a.first > b.first;
        };

        if (priority(left_bounds, right_bounds)) {
            consider(left, middle, left_sum, best);
            consider(middle + 1, right, right_sum, best);
        } else {
            consider(middle + 1, right, right_sum, best);
            consider(left, middle, left_sum, best);
        }
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n;
    if (!(std::cin >> n)) return 0;
    Solver solver(n);
    solver.run();
    return 0;
}

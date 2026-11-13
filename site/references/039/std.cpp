#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>

using int64 = std::int64_t;

namespace {
constexpr int T = 13;
constexpr int MAX_USED_DOUBLES = 6 * T - 1;  // 77
constexpr int MAX_DIFF = 9 * T * T;          // 1521
constexpr int64 NEG_INF = -(int64{1} << 60);

int64 dp[2][MAX_USED_DOUBLES + 1][MAX_DIFF + 1];
}  // namespace

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, k;
    if (!(std::cin >> n >> k)) return 0;
    k = std::min(k, MAX_USED_DOUBLES);

    for (auto& layer : dp)
        for (auto& row : layer)
            std::fill(std::begin(row), std::end(row), NEG_INF);
    dp[0][0][0] = 0;

    int cur = 0;
    int prefix_t_sum = 0;

    for (int i = 1; i <= n; ++i) {
        int64 value;
        int t;
        std::cin >> value >> t;
        const int next = cur ^ 1;

        for (int used = 0; used <= std::min(k, i); ++used)
            std::fill(std::begin(dp[next][used]), std::end(dp[next][used]), NEG_INF);

        const int old_max_diff = std::min(MAX_DIFF, 2 * prefix_t_sum);
        for (int used = 0; used <= std::min(k, i - 1); ++used) {
            for (int diff = 0; diff <= old_max_diff; ++diff) {
                const int64 old = dp[cur][used][diff];
                if (old == NEG_INF) continue;

                // Do not select this item.
                dp[next][used][diff] = std::max(dp[next][used][diff], old);
                const int64 selected = old + value;

                // Select without doubling, putting it on either side.
                dp[next][used][std::abs(diff - t)] =
                    std::max(dp[next][used][std::abs(diff - t)], selected);
                if (diff + t <= MAX_DIFF)
                    dp[next][used][diff + t] =
                        std::max(dp[next][used][diff + t], selected);

                if (used < k) {
                    const int doubled_t = 2 * t;
                    dp[next][used + 1][std::abs(diff - doubled_t)] =
                        std::max(dp[next][used + 1][std::abs(diff - doubled_t)], selected);
                    if (diff + doubled_t <= MAX_DIFF)
                        dp[next][used + 1][diff + doubled_t] =
                            std::max(dp[next][used + 1][diff + doubled_t], selected);
                }
            }
        }

        prefix_t_sum += t;
        cur = next;
    }

    int64 answer = 0;  // Selecting no item is always legal.
    for (int used = 0; used <= k; ++used)
        answer = std::max(answer, dp[cur][used][0]);
    std::cout << answer << '\n';
    return 0;
}

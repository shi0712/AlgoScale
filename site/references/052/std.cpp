#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>

using i64 = long long;
static constexpr i64 NEG = -(1LL << 60);

struct Item {
    int p = 0;
    std::array<i64, 11> w{};
};

// Max-plus convolution of an arbitrary sequence A with a discrete concave
// sequence B. Source indices increase by step; offset is added to the final
// resource usage. The totally monotone row maxima are found by divide-and-
// conquer optimization. This costs O(K log K) per convolution.
static std::vector<i64> concave_transition(
    const std::vector<i64>& a,
    const std::vector<i64>& b,
    int step,
    int offset
) {
    const int k = static_cast<int>(a.size()) - 1;
    std::vector<i64> out(k + 1, NEG);

    for (int rem = 0; rem < step && rem <= k; ++rem) {
        const int na = (k - rem) / step + 1;
        if (k - offset < rem) continue;
        const int rows = (k - offset - rem) / step + 1;

        auto solve = [&](auto&& self, int left, int right, int opt_left, int opt_right) -> void {
            if (left > right) return;
            const int mid = (left + right) / 2;
            const int low = std::max({0, opt_left, mid - static_cast<int>(b.size()) + 1});
            const int high = std::min({na - 1, opt_right, mid});

            int best_j = low;
            __int128 best_value = -((__int128)1 << 120);
            for (int j = low; j <= high; ++j) {
                const i64 av = a[rem + j * step];
                const __int128 value = static_cast<__int128>(av) + b[mid - j];
                // Pick the rightmost maximum. This is the tie rule used by
                // the monotone divide-and-conquer recursion.
                if (value >= best_value) {
                    best_value = value;
                    best_j = j;
                }
            }

            const int target = rem + mid * step + offset;
            const i64 source = a[rem + best_j * step];
            if (source != NEG) out[target] = source + b[mid - best_j];

            self(self, left, mid - 1, opt_left, best_j);
            self(self, mid + 1, right, best_j, opt_right);
        };

        solve(solve, 0, rows - 1, 0, na - 1);
    }
    return out;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, k;
    if (!(std::cin >> n >> k)) return 0;

    std::array<std::vector<Item>, 11> groups;
    i64 total_power = 0;
    i64 all_full_reward = 0;
    for (int i = 0; i < n; ++i) {
        Item item;
        std::cin >> item.p;
        total_power += item.p;
        for (int t = 1; t <= item.p; ++t) std::cin >> item.w[t];
        all_full_reward += item.w[item.p];
        groups[item.p].push_back(item);
    }

    if (total_power <= k) {
        std::cout << all_full_reward << '\n';
        return 0;
    }

    std::vector<i64> no_partial(k + 1, NEG), has_partial(k + 1, NEG);
    no_partial[0] = 0;

    for (int p = 1; p <= 10; ++p) {
        auto& items = groups[p];
        if (items.empty()) continue;
        std::sort(items.begin(), items.end(), [p](const Item& lhs, const Item& rhs) {
            return lhs.w[p] > rhs.w[p];
        });

        const int count = static_cast<int>(items.size());
        std::vector<i64> full(count + 1, 0);
        for (int i = 0; i < count; ++i) full[i + 1] = full[i] + items[i].w[p];

        const std::vector<i64> old_no_partial = no_partial;
        const std::vector<i64> old_has_partial = has_partial;
        no_partial = concave_transition(old_no_partial, full, p, 0);
        has_partial = concave_transition(old_has_partial, full, p, 0);

        for (int t = 1; t < p; ++t) {
            std::vector<i64> suffix_best(count + 1, NEG);
            for (int i = count - 1; i >= 0; --i) {
                suffix_best[i] = std::max(suffix_best[i + 1], items[i].w[t]);
            }

            std::vector<i64> prefix_best(count, NEG);
            for (int i = 0; i < count; ++i) {
                const i64 value = items[i].w[t] - items[i].w[p];
                prefix_best[i] = (i == 0 ? value : std::max(prefix_best[i - 1], value));
            }

            std::vector<i64> partial(count, NEG);
            for (int chosen_full = 0; chosen_full < count; ++chosen_full) {
                partial[chosen_full] = full[chosen_full] + suffix_best[chosen_full];
                if (chosen_full > 0) {
                    partial[chosen_full] = std::max(
                        partial[chosen_full],
                        full[chosen_full + 1] + prefix_best[chosen_full - 1]
                    );
                }
            }

            const auto candidate = concave_transition(old_no_partial, partial, p, t);
            for (int used = 0; used <= k; ++used) {
                has_partial[used] = std::max(has_partial[used], candidate[used]);
            }
        }
    }

    std::cout << std::max(no_partial[k], has_partial[k]) << '\n';
    return 0;
}

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

using i64 = long long;
using i128 = __int128_t;

static constexpr i64 INF64 = (1LL << 62);
static constexpr i128 INF128 = (i128(1) << 120);

struct State {
    i128 value;
    int groups;
    State(i128 value_ = INF128, int groups_ = 0) : value(value_), groups(groups_) {}
};

class Solver {
public:
    Solver(std::vector<i64> times, int capacity)
        : n_(static_cast<int>(times.size())), c_(capacity) {
        std::sort(times.begin(), times.end());
        a_.assign(n_ + 1, 0);
        prefix_.assign(n_ + 1, 0);
        for (int i = 1; i <= n_; ++i) {
            a_[i] = times[i - 1];
            prefix_[i] = prefix_[i - 1] + a_[i];
        }
    }

    i64 solve() {
        if (n_ <= c_) return a_[n_];
        prepare();

        int logarithm = 0;
        for (i64 x = 2 * bound_ + 1; x; x >>= 1) ++logarithm;
        if (i128(c_) * c_ * c_ * c_ * logarithm <= n_) {
            return solve_small_capacity();
        }
        return solve_large_capacity();
    }

private:
    int n_, c_, h_limit_;
    std::vector<i64> a_, prefix_, refill_;
    std::vector<i128> dp_value_;
    std::vector<int> dp_groups_;
    i64 bound_ = 0;

    void prepare() {
        h_limit_ = n_ / c_;
        refill_.assign(h_limit_ + 1, INF64);
        refill_[0] = 0;
        for (int k = 2; k <= std::min(c_, h_limit_ + 1); ++k) {
            i64 cost = a_[k] + prefix_[k];
            for (int h = k - 1; h <= h_limit_; ++h) {
                refill_[h] = std::min(refill_[h], refill_[h - k + 1] + cost);
            }
        }
        bound_ = i64(n_) * (a_[n_] + prefix_[c_ - 1]) + 1;
        dp_value_.resize(n_);
        dp_groups_.resize(n_);
    }

    i64 solve_large_capacity() const {
        int max_groups = (n_ - 1) / (c_ - 1);
        std::vector<i64> dp(2 * h_limit_ + 1, INF64);
        std::vector<i64> next(2 * h_limit_ + 1, INF64);
        dp[h_limit_] = 0;
        i64 answer = INF64;

        for (int q = 0; q <= max_groups; ++q) {
            std::fill(next.begin(), next.end(), INF64);
            int low = std::max({-h_limit_, -q * (c_ - 2), q - max_groups});
            int high = std::min({h_limit_, q, n_ - 1 - q * (c_ - 1)});
            for (int h = low; h <= high; ++h) {
                i64 current = dp[h + h_limit_];
                if (current == INF64) continue;
                int placed = q * (c_ - 1) + h;
                if (placed >= n_ - c_ && h >= 0) {
                    answer = std::min(answer, current + a_[n_ - placed] + refill_[h]);
                }
                if (placed > n_ - c_ || q == max_groups) continue;

                int left = std::max({0, placed + c_ - (n_ - 1), h + 1 - h_limit_});
                int right = std::min({c_ - 1, h + 1 + h_limit_, h + max_groups - q});
                for (int returning = left; returning <= right; ++returning) {
                    int next_h = h + 1 - returning;
                    next[next_h + h_limit_] = std::min(
                        next[next_h + h_limit_], current + a_[n_ - placed] + prefix_[returning]);
                }
            }
            dp.swap(next);
        }
        return answer;
    }

    State penalized(int target, i64 reward) {
        dp_value_[0] = 0;
        dp_groups_[0] = 0;
        for (int y = 1; y <= target; ++y) {
            i128 best = INF128;
            int count = -1;
            int left = std::max(0, y - c_);
            int right = std::min(y - 1, n_ - c_);
            for (int x = left; x <= right; ++x) {
                i128 candidate = dp_value_[x] + a_[n_ - x] + prefix_[c_ - y + x] - reward;
                int candidate_count = dp_groups_[x] + 1;
                if (candidate < best || (candidate == best && candidate_count > count)) {
                    best = candidate;
                    count = candidate_count;
                }
            }
            dp_value_[y] = best;
            dp_groups_[y] = count;
        }
        return State(dp_value_[target], dp_groups_[target]);
    }

    i64 exact_cost(int target, int groups) {
        i64 left = -bound_, right = bound_;
        while (left < right) {
            i64 middle = left + (right - left) / 2;
            if (penalized(target, middle).groups >= groups) right = middle;
            else left = middle + 1;
        }
        State result = penalized(target, left);
        return static_cast<i64>(result.value + i128(left) * groups);
    }

    i64 solve_small_capacity() {
        std::vector<i64> item_cost(c_);
        for (int weight = 1; weight < c_; ++weight) {
            item_cost[weight] = a_[weight + 1] + prefix_[weight + 1];
        }
        int best_weight = 1;
        for (int weight = 2; weight < c_; ++weight) {
            if (i128(item_cost[weight]) * best_weight < i128(item_cost[best_weight]) * weight) {
                best_weight = weight;
            }
        }

        i64 best_cost = item_cost[best_weight];
        int periodic_start = (best_weight - 1) * (c_ - 1);
        int residue_count = best_weight / std::gcd(best_weight, c_ - 1);
        int exact_limit = periodic_start + (c_ - 1) * residue_count;
        i64 answer = INF64;

        if (periodic_start <= h_limit_) {
            std::vector<std::vector<State>> dp(c_ + 1, std::vector<State>(residue_count));
            dp[0][0] = State(0, 0);
            for (int x = 0; x < n_; ++x) {
                auto &row = dp[x % (c_ + 1)];
                if (x >= n_ - c_) {
                    for (int residue = 0; residue < residue_count; ++residue) {
                        const State &state = row[residue];
                        if (state.value == INF128) continue;
                        int h = x - state.groups * (c_ - 1);
                        if (h < periodic_start || h > h_limit_) continue;
                        i128 cost = (state.value + i128(best_cost) * (c_ - 1) * state.groups) / best_weight;
                        answer = std::min(answer, static_cast<i64>(cost) + refill_[h] + a_[n_ - x]);
                    }
                }
                if (x <= n_ - c_) {
                    int limit = std::min(c_, n_ - 1 - x);
                    for (int placed = 1; placed <= limit; ++placed) {
                        auto &destination = dp[(x + placed) % (c_ + 1)];
                        i128 edge = i128(best_weight) * (a_[n_ - x] + prefix_[c_ - placed])
                                  - i128(best_cost) * (c_ - 1);
                        for (int residue = 0; residue < residue_count; ++residue) {
                            if (row[residue].value == INF128) continue;
                            int next_residue = residue + 1 == residue_count ? 0 : residue + 1;
                            i128 value = row[residue].value + edge;
                            int groups = row[residue].groups + 1;
                            if (value < destination[next_residue].value ||
                                (value == destination[next_residue].value &&
                                 groups < destination[next_residue].groups)) {
                                destination[next_residue] = State(value, groups);
                            }
                        }
                    }
                }
                std::fill(row.begin(), row.end(), State());
            }
        }

        for (int placed = n_ - c_; placed < n_; ++placed) {
            int limit = std::min({h_limit_, exact_limit - 1, placed});
            for (int h = placed % (c_ - 1); h <= limit; h += c_ - 1) {
                int groups = (placed - h) / (c_ - 1);
                if (groups < (placed + c_ - 1) / c_ || groups > std::min(placed, n_ - c_ + 1)) continue;
                answer = std::min(answer, exact_cost(placed, groups) + refill_[h] + a_[n_ - placed]);
            }
        }
        return answer;
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, c;
    if (!(std::cin >> n >> c)) return 0;
    std::vector<i64> times(n);
    for (i64 &time : times) std::cin >> time;
    std::cout << Solver(std::move(times), c).solve() << '\n';
}

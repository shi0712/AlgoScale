#include <algorithm>
#include <array>
#include <bit>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

using Int = long long;

constexpr Int limit = 100'000'000'000'000LL;
constexpr Int infinity = std::numeric_limits<Int>::max() / 4;

class UnitCost {
    static constexpr int maxCredit = 54;
    struct Counts {
        Int nodes = 0;
        Int cost = 0;
        Int shallow = 0;
    };
    static constexpr int maxMarginal = 55;
    std::array<std::array<Counts, maxCredit + 1>, maxMarginal + 1> table{};

    int cutoff(Int n, int height, int credit) const {
        int first = height;
        int last = maxMarginal;
        while (first < last) {
            int mid = (first + last) / 2;
            if (table[mid][credit].nodes >= n - 1) last = mid;
            else first = mid + 1;
        }
        return first;
    }

public:
    constexpr UnitCost() {
        // With unit-sized transfers, a branch at depth d with f preceding
        // forced deposits increases the sum of leaf depths by d + 2(f + 1).
        for (int cap = 2; cap <= maxMarginal; ++cap) {
            for (int credit = 0; credit <= maxCredit; ++credit) {
                int ready = std::max(credit, 1);
                int steps = 1 + ready - credit;
                if (cap < 2 * steps) continue;
                auto low = table[cap - steps][ready - 1];
                auto high = table[cap - steps][std::min(ready + 1, maxCredit)];
                table[cap][credit] = {
                    1 + low.nodes + high.nodes,
                    2 * steps + low.cost + high.cost + steps * (low.nodes + high.nodes),
                    (steps <= cap - 2) + low.shallow + high.shallow
                };
            }
        }
    }

    static int depth(Int n) {
        return static_cast<int>(std::bit_width(static_cast<unsigned long long>(n - 1)));
    }

    static Int unrestricted(Int n, int height) {
        return n * (height + 1) - (1LL << height);
    }

    Int cost(Int n, int height, int credit) const {
        if (credit >= height || n == 1) return unrestricted(n, height);
        int threshold = cutoff(n, height, credit);
        auto cheaper = table[threshold - 1][credit];
        return cheaper.cost + threshold * (n - 1 - cheaper.nodes);
    }

    Int split(Int n, int credit) const {
        int height = depth(n);
        if (credit >= height) return n / 2;
        int threshold = cutoff(n, height, credit);
        // Among equal-cost expansions, prefer the ones whose children are
        // shallower whenever this reduces the maximum depth of the tree.
        bool shallow = table[threshold][credit].shallow >= n - 1;
        auto count = [&](int childCredit) {
            auto child = table[threshold - 1][childCredit];
            return shallow ? child.shallow : child.nodes;
        };
        Int leftMin = std::max(table[threshold - 2][credit - 1].nodes,
            n - 2 - count(credit + 1));
        Int leftMax = std::min(count(credit - 1),
            n - 2 - table[threshold - 2][credit + 1].nodes);
        return 1 + std::clamp(n / 2 - 1, leftMin, leftMax);
    }

    int certificate(Int l, Int h, Int cash) const {
        Int n = h - l + 1;
        int height = depth(n);
        // Charge h per virtual unit and reserve h-l for every possible
        // shortfall on a successful transfer. A marginal cutoff of k
        // allows leaf depths of at most k-1.
        int steps = cutoff(n, height, 0) - 1;
        Int available = cash - steps * (h - l);
        if (available < 0) return -1;
        int safe = std::min<Int>(height, available / h);
        // At the true threshold, cash divided by M can grow by at most
        // one on success and falls by more than one on failure.
        int optimistic = std::min<Int>(height, cash / l);
        return cost(n, height, safe) == cost(n, height, optimistic) ? safe : -1;
    }

    constexpr bool coversLimit() const {
        return table[maxMarginal][0].nodes >= limit;
    }
};

constexpr UnitCost unitCost;
static_assert(unitCost.coversLimit());

class SmallSearch {
    struct Plan {
        Int cash;
        int cost;
    };
    Int base = 0;
    int size = 0;
    std::vector<std::vector<Plan>> plans;

    std::vector<Plan>& at(int a, int b) {
        return plans[a * size + b];
    }

    int cost(int a, int b, Int cash) {
        const auto& options = at(a, b);
        auto it = std::lower_bound(options.begin(), options.end(), cash,
            [](const Plan& p, Int funds) { return p.cash > funds; });
        return it == options.end() ? 1000000 : it->cost;
    }

    void build(Int l, Int h) {
        base = l;
        size = static_cast<int>(h - l + 1);
        plans.assign(size * size, {});
        for (int a = 0; a < size; ++a) at(a, a).push_back({0, 0});
        for (int n = 2; n <= size; ++n) {
            for (int a = 0; a + n <= size; ++a) {
                int b = a + n - 1;
                // Balanced comparisons become affordable after a bounded
                // number of guaranteed deposits at the lower endpoint.
                Int deposit = base + a;
                int depth = UnitCost::depth(n);
                Int deposits = (depth * (base + b) + deposit - 1) / deposit;
                int bound = static_cast<int>(UnitCost::unrestricted(n, depth) + n * deposits);
                std::vector<Int> cheapest(bound + 1, infinity);
                for (int k = a + 1; k <= b; ++k) {
                    Int query = base + k;
                    for (const Plan& left : at(a, k - 1)) {
                        for (const Plan& right : at(k, b)) {
                            int sum = n + left.cost + right.cost;
                            if (sum > bound) break;
                            // The lower branch must pay the query. The
                            // upper branch receives the query amount.
                            Int requirement = std::max(query + left.cash, right.cash - query);
                            cheapest[sum] = std::min(cheapest[sum], requirement);
                        }
                    }
                }
                Int previous = infinity;
                for (int sum = 0; sum <= bound; ++sum) {
                    Int requirement = cheapest[sum];
                    if (requirement >= previous) continue;
                    at(a, b).push_back({requirement, sum});
                    previous = requirement;
                    if (requirement == 0) break;
                    if (sum + n <= bound) {
                        cheapest[sum + n] = std::min(cheapest[sum + n], std::max(0LL, requirement - deposit));
                    }
                }
            }
        }
    }

public:
    Int choose(Int l, Int h, Int cash) {
        if (size == 0) build(l, h);
        int a = static_cast<int>(l - base);
        int b = static_cast<int>(h - base);
        int best = 1000000;
        Int bestQuery = l + 1;
        int bestImbalance = b - a;
        for (int k = a + 1; k <= b && base + k <= cash; ++k) {
            Int x = base + k;
            int fail = cost(a, k - 1, cash - x);
            int pass = cost(k, b, cash + x);
            int total = fail + pass;
            int imbalance = std::abs(2 * k - a - b - 1);
            if (total < best || (total == best && imbalance < bestImbalance)) {
                best = total;
                bestQuery = x;
                bestImbalance = imbalance;
            }
        }
        return bestQuery;
    }
};

class Strategy {
    static constexpr int exactLimit = 48;
    static constexpr int rootGrid = 12;
    static constexpr int lookaheadGrid = 7;
    static constexpr int lookahead = 2;

    bool growing = true;
    SmallSearch small;
    int unitCredit = -1;

    static Int estimate(Int l, Int h, Int cash) {
        Int n = h - l + 1;
        if (n <= 1) return 0;
        int depth = UnitCost::depth(n);
        if (cash >= depth * h) return UnitCost::unrestricted(n, depth);
        if (cash <= l) {
            int deposits = cash == 0 ? 2 : 1;
            return deposits * n + estimate(l, h, cash + deposits * l);
        }
        // Estimate the number of units of cash across all possible values
        // of M. Discount the balance for future transfers away from M.
        Int effective = std::max(0LL, cash - 1 - 2 * (h - l) / 5);
        int from = std::min<Int>(depth, effective / h);
        int to = std::min<Int>(depth, effective / l);
        if (from == to) return unitCost.cost(n, depth, from);
        __int128 total = 0;
        Int end = h;
        for (int b = from; b < to; ++b) {
            Int next = effective / (b + 1);
            total += static_cast<__int128>(end - next) * unitCost.cost(n, depth, b);
            end = next;
        }
        total += static_cast<__int128>(end - l + 1) * unitCost.cost(n, depth, to);
        return static_cast<Int>(total / n);
    }

    static std::pair<Int, Int> queryRange(Int l, Int h, Int cash) {
        Int n = h - l + 1;
        Int minPart = std::max(1LL, n / 8);
        if (cash >= l + minPart) {
            return {l + minPart, std::min(h + 1 - minPart, cash)};
        }
        return {l + 1, std::min(h, cash)};
    }

    static Int predict(Int l, Int h, Int cash, int levels) {
        Int n = h - l + 1;
        if (n <= 1) return 0;
        if (levels == 0 || 16 * (h - l) < l) return estimate(l, h, cash);
        int depth = UnitCost::depth(n);
        if (cash >= depth * h) return UnitCost::unrestricted(n, depth);
        if (cash <= l) return n + predict(l, h, cash + l, levels - 1);
        auto [first, last] = queryRange(l, h, cash);
        Int best = infinity;
        auto consider = [&](Int x) {
            Int value = predict(l, x - 1, cash - x, levels - 1)
                      + predict(x, h, cash + x, levels - 1);
            best = std::min(best, value);
        };
        for (int i = 0; i <= lookaheadGrid; ++i) consider(first + (last - first) * i / lookaheadGrid);
        consider(std::clamp(l + n / 2, first, last));
        if (last > first && last == cash) consider(last - 1);
        return n + best;
    }

    Int refine() {
        auto [first, last] = queryRange(low, high, cash);
        Int n = high - low + 1;
        int depth = UnitCost::depth(n);
        if (cash >= depth * high) return low + n / 2;
        Int best = first;
        Int bestValue = infinity;
        auto consider = [&](Int x) {
            Int left = predict(low, x - 1, cash - x, lookahead);
            Int right = predict(x, high, cash + x, lookahead);
            Int value = left + right;
            if (value < bestValue || (value == bestValue &&
                    std::abs(2 * x - low - high - 1) < std::abs(2 * best - low - high - 1))) {
                bestValue = value;
                best = x;
            }
        };
        Int span = last - first;
        for (int i = 0; i <= rootGrid; ++i) consider(first + span * i / rootGrid);
        consider(std::clamp(low + n / 2, first, last));
        if (last > first && last == cash) consider(last - 1);
        // Refine around the best grid point. The estimates also have kinks
        // when a child gains or loses enough cash for another comparison.
        for (Int stride = span / rootGrid; stride > 1; stride /= 4) {
            Int center = best;
            for (int offset = -3; offset <= 3; ++offset) {
                if (offset != 0) consider(std::clamp(center + offset * stride / 4, first, last));
            }
        }
        for (Int x = std::max(first, best - 2), end = std::min(last, best + 2); x <= end; ++x) consider(x);
        return best;
    }

public:
    Int low = 0;
    Int high = limit;
    Int cash = 1;

    Int query() {
        if (low == 0) return 1;
        // A successful full-balance query doubles the money available.
        if (growing && 3 * cash < high) return cash;
        growing = false;
        if (unitCredit < 0) {
            if (cash <= low) return low;  // Guaranteed deposit.
            if (high - low < exactLimit) return small.choose(low, high, cash);
            if (8 * (high - low) < low) unitCredit = unitCost.certificate(low, high, cash);
        }
        if (unitCredit == 0) return low;
        if (unitCredit > 0) return low + unitCost.split(high - low + 1, unitCredit);
        return refine();
    }

    void response(Int x, bool lucky) {
        if (unitCredit >= 0) unitCredit += lucky ? 1 : -1;
        if (lucky) {
            low = std::max(low, x);
            cash += x;
        } else {
            high = x - 1;
            cash -= x;
            growing = false;
        }
    }
};

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    Strategy strategy;
    while (strategy.low < strategy.high) {
        Int x = strategy.query();
        std::cout << "? " << x << '\n' << std::flush;
        std::string response;
        if (!(std::cin >> response)) return 0;
        if (response == "Lucky!") strategy.response(x, true);
        else if (response == "Fraudster!") strategy.response(x, false);
        else return 0;
    }
    std::cout << "! " << strategy.low << '\n' << std::flush;
    return 0;
}

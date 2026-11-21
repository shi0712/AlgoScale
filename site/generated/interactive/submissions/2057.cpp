#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
constexpr i64 limit = 100000000000000LL;
constexpr int levels = 64;

int height(i64 n) {
    return bit_width(static_cast<unsigned long long>(n - 1));
}

i64 unrestricted(i64 n) {
    int h = height(n);
    return n * (h + 1) - (1LL << h);
}

// Auxiliary problem in which every transfer and fine is one unit.
// Expanding a leaf at depth d adds d+2s to the total path length,
// where s is 1 for an immediate split and 2 if a refill is needed.
// These increments never decrease along a path, so the cheapest n-1
// expansions give the optimal tree with n leaves.
struct UnitModel {
    i64 count[levels + 1][levels + 1]{};
    i64 area[levels + 1][levels + 1]{};

    UnitModel() {
        for (int t = 1; t <= levels; ++t) {
            for (int b = 0; b <= levels; ++b) {
                int s = b ? 1 : 2;
                if (t >= 2 * s) {
                    int effective = max(b, 1);
                    count[b][t] = min(limit + 1, 1 +
                        count[effective - 1][t - s] +
                        count[min(levels, effective + 1)][t - s]);
                }
                area[b][t] = area[b][t - 1] + count[b][t - 1];
            }
        }
    }

    int threshold(i64 n, int b) const {
        return lower_bound(count[b], count[b] + levels + 1, n - 1) - count[b];
    }

    i64 cost(i64 n, int b) const {
        if (b >= height(n)) return unrestricted(n);
        int t = threshold(n, b);
        return (n - 1) * t - area[b][t];
    }

    i64 split(i64 n, int b) const {
        if (b >= height(n)) return n / 2;
        int s = b ? 1 : 2;
        int left = max(1, b) - 1;
        int right = min(levels, max(1, b) + 1);
        int t = threshold(n, b) - s;
        i64 low = max(1 + count[left][t - 1], n - 1 - count[right][t]);
        i64 high = min(1 + count[left][t], n - 1 - count[right][t - 1]);
        double fraction = (1.0 + count[left][t]) /
                          (2.0 + count[left][t] + count[right][t]);
        return clamp<i64>(llround(n * fraction), low, high);
    }
} units;

struct Choice {
    double cost;
    i64 query;
};

// Check whether rounding every price to the middle of the interval
// preserves affordability for the entire relevant part of the tree.
int fixed_units(i64 l, i64 u, i64 b) {
    i64 n = u - l + 1;
    if (b >= height(n) * u) return levels;
    int horizon = units.threshold(n, 0);
    i64 price = (l + u) / 2;
    i64 error = (horizon + 1) * (u - price);
    i64 remainder = b % price;
    if (remainder >= error && remainder + error < price)
        return min<i64>(levels, b / price);
    return -1;
}

// Approximate the residual cost of a wide interval by averaging the
// unit-price model over the still possible thresholds.
double estimate(i64 l, i64 u, i64 b) {
    i64 n = u - l + 1;
    i64 funds = max(0LL, b - (u - l) / 3);
    double total = 0;
    for (i64 first = l; first <= u; ) {
        int k = min<i64>(height(n), funds / first);
        i64 last = k ? min(u, funds / k) : u;
        total += double(last - first + 1) / n * units.cost(n, k);
        first = last + 1;
    }
    return total;
}

Choice plan(i64 l, i64 u, i64 b, int depth) {
    i64 n = u - l + 1;
    if (n == 1) return {0, l};
    if (b <= l) {
        i64 refills = (l - b) / l + 1;
        return {refills * n + plan(l, u, b + refills * l, depth).cost, l};
    }
    int k = fixed_units(l, u, b);
    if (k >= 0) return {double(units.cost(n, k)), l + units.split(n, k)};
    if (!depth) return {estimate(l, u, b), l};

    i64 cap = min(n - 1, b - l);
    int grid = depth == 4 ? 28 : depth == 3 ? 14 : depth == 2 ? 10 : 7;
    vector<i64> candidates;
    auto add = [&](i64 size) {
        candidates.push_back(clamp(size, 1LL, cap));
    };
    int nominal = min<i64>(levels, b / ((l + u) / 2));
    i64 target = units.split(n, nominal);
    add(target);
    add(units.split(n, max(0, nominal - 1)));
    add(n / 2);
    add(1);
    for (int i = 1; i <= grid; ++i) add((cap * i + grid / 2) / grid);
    sort(candidates.begin(), candidates.end());
    candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());

    Choice best{numeric_limits<double>::infinity(), l + target};
    auto consider = [&](i64 size) {
        i64 x = l + size;
        double cost = n + plan(l, x - 1, b - x, depth - 1).cost +
                          plan(x, u, b + x, depth - 1).cost;
        if (cost < best.cost ||
            (cost == best.cost && abs(size - target) < abs(best.query - l - target)))
            best = {cost, x};
    };
    for (i64 size : candidates) consider(size);
    if (depth == 4 && cap > 4 * grid) {
        i64 center = best.query - l;
        for (int i = -5; i <= 5; ++i) {
            i64 size = center + cap * i / (6 * grid);
            if (size >= 1 && size <= cap && size != center) consider(size);
        }
    }
    return best;
}

struct ExactResult {
    int cost, height;
    i64 query;
};

// In short intervals, optimize total path length directly, breaking ties
// by worst-case depth. Trying l+1 dominates a voluntary refill at l
// whenever affordable: failure resolves the interval, success earns more.
struct ExactSearch {
    map<tuple<i64, i64, i64>, ExactResult> memo;
    ExactResult solve(i64 l, i64 u, i64 b) {
        int n = int(u - l + 1);
        if (n == 1) return {0, 0, l};
        if (b <= l) {
            int r = int((l - b) / l + 1);
            auto child = solve(l, u, b + r * l);
            return {child.cost + r * n, child.height + r, l};
        }
        int h = height(n);
        if (b >= h * u) return {int(unrestricted(n)), h, l + n / 2};
        auto key = make_tuple(l, u, b);
        if (auto it = memo.find(key); it != memo.end()) return it->second;

        int k = min<i64>(levels, b / ((l + u) / 2));
        i64 cap = min(u, b);
        i64 center = min(cap, l + units.split(n, k));
        ExactResult best{INT_MAX, INT_MAX, center};
        auto consider = [&](i64 x) {
            int base = n + int(unrestricted(u - x + 1));
            if (base + unrestricted(x - l) > best.cost) return;
            auto left = solve(l, x - 1, b - x);
            if (base + left.cost > best.cost) return;
            auto right = solve(x, u, b + x);
            int cost = n + left.cost + right.cost;
            int hgt = 1 + max(left.height, right.height);
            if (tie(cost, hgt) < tie(best.cost, best.height))
                best = {cost, hgt, x};
        };
        for (int d = 0; center - d > l || center + d <= cap; ++d) {
            if (center - d > l) consider(center - d);
            if (d && center + d <= cap) consider(center + d);
            if (best.cost == unrestricted(n) && best.height == h) break;
        }
        return memo[key] = best;
    }
};

// After comparisons have confined the threshold to the vicinity of an
// original endpoint, also search for its distance from that endpoint.
// Repeated successful narrowing reduces the logarithm of the gap
// geometrically. An interior value leaves this search on the first miss.
struct Strategy {
    i64 initial_low = -1, initial_high = -1;

    i64 next(i64 l, i64 u, i64 balance) {
        if (u == limit && 4 * balance < u) return balance;
        // A one-euro reserve makes l+1 affordable after a single refill
        // if this last growth probe fails.
        if (u == limit && 2 * balance < u) return balance - 1;
        if (initial_low < 0) {
            initial_low = l;
            initial_high = u;
        }
        if (balance <= l) return l;
        i64 n = u - l + 1;
        if (n <= 28) return ExactSearch().solve(l, u, balance).query;

        if (n * 64 <= initial_high - initial_low + 1) {
            i64 edge = max(1LL, i64(cbrt(double(n))));
            if (l == initial_low) return min(balance, l + edge);
            if (u == initial_high && balance >= 3 * u) return u - edge + 1;
        }
        i64 proposed = plan(l, u, balance, 4).query;
        return clamp(proposed, l + 1, min(balance, u));
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    i64 l = 0, u = limit, balance = 1;
    Strategy strategy;
    while (l < u) {
        i64 x = strategy.next(l, u, balance);
        cout << "? " << x << endl;
        string reply;
        if (!(cin >> reply)) return 0;
        if (reply == "Lucky!") {
            balance += x;
            l = max(l, x);
        } else if (reply == "Fraudster!") {
            balance -= x;
            u = x - 1;
        } else {
            return 0;
        }
    }
    cout << "! " << l << endl;
    return 0;
}

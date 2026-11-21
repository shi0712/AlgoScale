#include <bits/stdc++.h>
using namespace std;

using Mass = uint64_t;

// Full cap-sized blocks need one leaf each. At either end, a partial
// block's binary decomposition gives the minimum number of leaves.
Mass minimum_pieces(Mass left, Mass right, Mass cap) {
    if (left == right) return 0;
    Mass first = (left + cap - 1) & ~(cap - 1);
    Mass last = right & ~(cap - 1);
    if (first <= last) {
        return (last - first) / cap + popcount(first - left) + popcount(right - last);
    }
    // Both endpoints are in one cap-sized block. Split at the first
    // dyadic boundary that separates the first and last units of mass.
    Mass half = bit_floor(left ^ (right - 1));
    if (half == 0) return 1;
    Mass middle = (right - 1) & ~(half - 1);
    Mass before = middle - left;
    Mass after = right - middle;
    if (before == half && after == half) return 1;
    return popcount(before) + popcount(after);
}

struct Rank {
    int low;
    int high;
    int estimate;
    Mass target;

    int width() const { return high - low; }
};

struct Slice {
    int smallest = 0;
    int largest = 0;
    Mass upper = 0;
};

class Solver {
    int n;
    map<Mass, int> prefixes{{0, 0}};
    map<Mass, pair<int, int>> recent;
    Mass lower = 1;
    Mass upper = Mass{1} << 30;

    Mass ask(int right) {
        cout << "? 1 " << right << endl;
        long long result;
        if (!(cin >> result) || result == -1) exit(0);
        return result;
    }

    void bound_from_gap(Mass left, Mass right, int count) {
        lower = max(lower, bit_ceil((right - left + count - 1) / count));
        while (minimum_pieces(left, right, lower) > static_cast<Mass>(count)) {
            lower *= 2;
        }
    }

    bool learn(Mass mass, int index) {
        auto [point, inserted] = prefixes.emplace(mass, index);
        if (!inserted) return false;
        if (point != prefixes.begin()) {
            auto left = prev(point);
            bound_from_gap(left->first, mass, index - left->second);
        }
        auto right = next(point);
        if (right != prefixes.end()) {
            bound_from_gap(mass, right->first, right->second - index);
        }
        return true;
    }

    Rank rank_of(Mass mass) const {
        auto right = prefixes.lower_bound(mass);
        if (right->first == mass) {
            return {right->second, right->second, right->second, mass};
        }
        auto left = prev(right);
        Mass count = right->second - left->second;
        Mass before = mass - left->first;
        Mass after = right->first - mass;
        // Each side needs its minimum dyadic cover and has at most one
        // leaf per unit of mass. These constraints bound the prefix rank.
        int low = left->second + static_cast<int>(max(
            minimum_pieces(left->first, mass, upper), count > after ? count - after : 0));
        int high = right->second - static_cast<int>(max(
            minimum_pieces(mass, right->first, upper), count > before ? count - before : 0));

        // Use a nearby large average to predict the rank in a short,
        // heavy half. Estimates affect probe placement only.
        int estimate = left->second + (count * before + (before + after) / 2) / (before + after);
        auto next_right = next(right);
        if (next_right != prefixes.end()) {
            Mass sample = next_right->first - right->first;
            Mass steps = next_right->second - right->second;
            if (sample * count > 3 * steps * (before + after) && after <= 64 * sample) {
                estimate = right->second - (after * steps + sample / 2) / sample;
            }
        }
        if (left != prefixes.begin()) {
            auto previous = prev(left);
            Mass sample = left->first - previous->first;
            Mass steps = left->second - previous->second;
            if (sample * count > 3 * steps * (before + after) && before <= 64 * sample) {
                estimate = left->second + (before * steps + sample / 2) / sample;
            }
        }
        // When close to a measured point, aim slightly beyond the
        // interpolated rank to bracket the boundary from the other side.
        if (before < after / 10) {
            int delta = estimate - left->second;
            estimate += max(1, delta / 2);
        } else if (after < before / 10) {
            int delta = right->second - estimate;
            estimate -= max(1, delta / 2);
        }
        return {low, high, clamp(estimate, low, high), mass};
    }

    Slice inspect(Mass start, Mass end) const {
        Slice result;
        auto left = prev(prefixes.upper_bound(start));
        for (auto right = next(left); left->first < end; ++left, ++right) {
            Mass from = max(start, left->first);
            Mass to = min(end, right->first);
            Mass mass = to - from;
            Mass count = right->second - left->second;
            Mass outside = from - left->first + right->first - to;
            Mass smallest = max(minimum_pieces(from, to, upper), count > outside ? count - outside : 0);
            Mass largest = min(mass, count - minimum_pieces(left->first, from, upper)
                                          - minimum_pieces(to, right->first, upper));
            // Leave at least one unit of mass for every other leaf.
            Mass candidate = bit_floor(min(upper, mass - smallest + 1));
            if (((from + candidate - 1) & ~(candidate - 1)) + candidate > to) {
                candidate /= 2;
            }
            result.smallest += smallest;
            result.largest += largest;
            result.upper = max(result.upper, candidate);
        }
        return result;
    }

    void probe(int index) {
        learn(ask(index), index);
    }

    int split_probe(const Rank &rank) {
        auto &[width, slow] = recent[rank.target];
        int span = rank.width();
        if (width > 0 && span * 4 > width * 3) {
            ++slow;
        } else {
            slow = 0;
        }
        width = span;
        // Force a binary step when repeated interpolation barely shrinks
        // this boundary's rank range.
        if (slow >= 2) {
            slow = 0;
            return (rank.low + rank.high) / 2;
        }
        return rank.estimate;
    }

    int balance_probe(const Rank &left, const Rank &middle, const Rank &right) {
        int outer = max(left.width(), right.width());
        if (outer == 0) return (left.low + right.low) / 2;
        if (middle.width() > 0 && 2 * middle.width() >= outer) {
            return split_probe(middle);
        }
        return split_probe(left.width() >= right.width() ? left : right);
    }

public:
    explicit Solver(int size) : n(size) {}

    Mass solve() {
        if (n <= 2) return ask(1);

        // The omitted last leaf has mass at most half of the total.
        Mass prefix = ask(n - 1);
        Mass end = 2 * bit_floor(prefix);
        Mass start = 0;
        learn(end, n);
        learn(prefix, n - 1);
        int length_limit = n;
        Mass recent_check = 0;

        // Anything above the global lower bound is still represented in
        // the active slice. Its boundaries need only be known in mass.
        while (lower < upper) {
            Mass mass = end - start;
            Slice active = inspect(start, end);
            upper = min(upper, active.upper);
            length_limit = min(length_limit, active.largest);
            lower = max(lower, bit_ceil((mass + length_limit - 1) / length_limit));
            if (lower >= upper) break;

            Rank left = rank_of(start);
            Rank right = rank_of(end);
            left.low = max(left.low, right.low - length_limit);
            right.high = min(right.high, left.high + length_limit);
            left.high = min(left.high, right.high - active.smallest);
            right.low = max(right.low, left.low + active.smallest);
            left.estimate = clamp(left.estimate, left.low, left.high);
            right.estimate = clamp(right.estimate, right.low, right.high);
            if (left.width() == 0 && learn(start, left.low)) continue;
            if (right.width() == 0 && learn(end, right.low)) continue;

            if (upper == mass) {
                // The interval can still be a single leaf. Its midpoint is
                // a boundary only after ruling that out.
                int likely_length = right.estimate - left.estimate;
                if (recent_check != mass && likely_length >= 6 && length_limit >= 6 && mass >= 4 * lower) {
                    recent_check = mass;
                    auto from = prev(prefixes.upper_bound(start));
                    auto to = prefixes.lower_bound(end);
                    int wanted = (left.estimate + right.estimate) / 2;
                    probe(clamp(wanted, from->second + 1, to->second - 1));
                } else if (left.width() == 0) {
                    probe(left.low + 1);
                } else if (right.width() == 0) {
                    probe(right.low - 1);
                } else {
                    probe(split_probe(left.width() <= right.width() ? left : right));
                }
                continue;
            }

            Mass mid = start + mass / 2;
            Rank middle = rank_of(mid);
            if (middle.width() == 0 && learn(mid, middle.low)) continue;

            Slice first = inspect(start, mid);
            Slice second = inspect(mid, end);
            first.largest = min(first.largest, length_limit - second.smallest);
            second.largest = min(second.largest, length_limit - first.smallest);
            bool left_shorter = first.largest <= second.smallest
                             || 2 * middle.high <= left.low + right.low;
            bool right_shorter = second.largest <= first.smallest
                              || 2 * middle.low >= left.high + right.high;

            // Equal-mass siblings start from a common array and undergo
            // local greedy flattening. The count orders the number of
            // operations, so the shorter side has at least as large a max.
            bool take_left;
            if (left_shorter || right_shorter) {
                take_left = left_shorter && (!right_shorter || first.upper <= second.upper);
                length_limit /= 2;
            } else if (min(first.upper, second.upper) <= lower) {
                take_left = first.upper > second.upper;
            } else {
                probe(balance_probe(left, middle, right));
                continue;
            }

            if (take_left) {
                end = mid;
                upper = min(upper, first.upper);
                length_limit = min(length_limit, first.largest);
            } else {
                start = mid;
                upper = min(upper, second.upper);
                length_limit = min(length_limit, second.largest);
            }
        }
        return lower;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;
    Mass answer = Solver(n).solve();
    cout << "! " << answer << endl;
}

#include <algorithm>
#include <array>
#include <bit>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <tuple>
#include <utility>

using namespace std;
using Int = long long;

struct Range {
    Int low = 1;
    Int high = 0;

    Int size() const {
        return max(0LL, high - low + 1);
    }
};

// An affine function of the next query's clockwise coordinate.
struct Line {
    Int constant;
    Int slope;

    Line(Int constant = 0, Int slope = 0) : constant(constant), slope(slope) {}

    Int at(Int x) const { return constant + slope * x; }
    Line half() const { return {constant / 2, slope / 2}; }

    friend Line operator+(Line a, Line b) {
        return {a.constant + b.constant, a.slope + b.slope};
    }
    friend Line operator-(Line a, Line b) {
        return {a.constant - b.constant, a.slope - b.slope};
    }
    friend Line operator*(Int factor, Line a) {
        return {factor * a.constant, factor * a.slope};
    }
};

struct QuerySpan {
    Int first;
    Int last;

    // Keep the entire span on the same side of every comparison we use.
    bool le(Line a, Line b) {
        Line difference = a - b;
        Int value = difference.at(first);
        if (value <= 0 && difference.slope > 0) {
            last = min(last, first - value / difference.slope);
        }
        if (value > 0 && difference.slope < 0) {
            last = min(last, first + (value - 1) / -difference.slope);
        }
        return value <= 0;
    }

    Line min_of(Line a, Line b) { return le(a, b) ? a : b; }
    Line max_of(Line a, Line b) { return le(a, b) ? b : a; }
    Line clamp_to(Line a, Int low, Int high) {
        return min_of(max_of(a, low), high);
    }
};

// One candidate produces each value in this progression, with step two.
struct Ramp {
    Line low;
    Line high;

    bool contains(Line value, QuerySpan& span) const {
        return (value.constant - low.constant) % 2 == 0
            && span.le(low, value) && span.le(value, high);
    }
};

struct SameSideReadings {
    Line plateau_value;
    Line plateau_size;
    Ramp ramp;
};

class Finder {
    const Int n;
    const Int m;
    const Int sum;
    const Int ramp_width;
    // Positive-side pairs, negative-side pairs, and pairs straddling zero.
    // Assign any pair with a transmitter at zero to the third family.
    array<Range, 3> candidates;

    static int mixed_depth(Int size, Int unit) {
        if (size <= 1) return 0;
        Int nodes = (size + unit - 2) / unit;
        return bit_width(static_cast<unsigned long long>(nodes));
    }

    // A mixed interval has two possible recursive branches and at most
    // ramp_width singleton answers per query. Fill its internal nodes
    // breadth-first, then remove surplus leaves from the deepest level.
    Int mixed_cost(Int size, int depth) const {
        if (depth == 0) return 0;
        Int unit = ramp_width + 1;
        Int nodes = (size + unit - 2) / unit;
        Int power = 1LL << (depth - 1);
        Int internal_depth_sum =
            (depth - 3) * power + 2 + (depth - 1) * (nodes - power + 1);
        Int surplus = 1 + unit * nodes - size;
        return (unit + 1) * nodes + unit * internal_depth_sum - depth * surplus;
    }

    int fix_mixed_depth(Line size, QuerySpan& span) const {
        Int unit = ramp_width + 1;
        int depth = mixed_depth(size.at(span.first), unit);
        if (depth == 0) {
            span.le(size, 1);
        } else {
            Int power = 1LL << (depth - 1);
            span.le((power - 1) * unit + 2, size);
            span.le(size, (2 * power - 1) * unit + 1);
        }
        return depth;
    }

    SameSideReadings same_side(Line center_distance, QuerySpan& span) const {
        const Range& range = candidates[0];
        Int shortest = sum - 2 * range.high;
        Int longest = sum - 2 * range.low;
        // For separation J and doubled center distance Q, the response is
        // max(Q, J) on the near half and min(Q, n - J) on the far half.
        bool low_reading = span.le(center_distance, m);
        Line threshold = low_reading ? center_distance : n - center_distance;
        Int step = (threshold.constant - shortest) % 2 == 0 ? 2 : 1;
        Line first_ramp = span.clamp_to(threshold + step, shortest, longest + 2);
        Ramp ramp = low_reading ? Ramp{first_ramp, longest}
                                : Ramp{n - longest, n - first_ramp};
        return {center_distance, (first_ramp - shortest).half(), ramp};
    }

    Int choose_second_query() const {
        Int best_query = 1;
        tuple<Int, int, Int> best{numeric_limits<Int>::max(), 0, 0};
        const Line x{0, 1};
        const Range& mixed = candidates[2];
        const Int unit = ramp_width + 1;

        for (Int start = 1; start <= m;) {
            QuerySpan span{start, m};
            // The center distances are doubled to keep half-integers exact.
            Line near_center = span.max_of(2 * x - sum, sum - 2 * x);
            Line far_center = span.min_of(2 * x + sum, 2 * n - 2 * x - sum);
            auto near = same_side(near_center, span);
            auto far = same_side(far_center, span);

            // The mixed reading changes from one plateau to the other as
            // the positive transmitter's coordinate crosses this window.
            // The left plateau reads n - sum and the right plateau reads sum.
            Line ramp_begin = sum <= m ? x - ramp_width : x + 1;
            Line low_cut = span.clamp_to(ramp_begin, mixed.low, mixed.high + 1);
            Line high_cut = span.clamp_to(ramp_begin + ramp_width,
                                          mixed.low, mixed.high + 1);
            Line left_size = low_cut - mixed.low;
            Line right_size = mixed.high + 1 - high_cut;
            Ramp middle = sum <= m
                ? Ramp{sum + 2 * (x + 1 - high_cut), sum + 2 * (x - low_cut)}
                : Ramp{n - sum + 2 * (low_cut - x),
                       n - sum + 2 * (high_cut - 1 - x)};

            // Ramp overlaps identify two mirror candidates. Any other
            // ambiguous result outside the mixed plateaus is a same-side
            // plateau, possibly with one extra candidate.
            Line one_more;
            if ((near.ramp.low.constant - far.ramp.low.constant) % 2 == 0) {
                Line first = span.max_of(near.ramp.low, far.ramp.low);
                Line last = span.min_of(near.ramp.high, far.ramp.high);
                if (span.le(first, last)) one_more = last - first + 2;
            }
            auto add_plateau = [&](const SameSideReadings& side,
                                   const SameSideReadings& other) {
                if (span.le(side.plateau_size, 0)) return;
                Line value = side.plateau_value;
                Line size = side.plateau_size;
                if (other.ramp.contains(value, span)) size = size + 1;
                if (middle.contains(value, span)) size = size + 1;
                // A same-side plateau has parity sum. If it matches the
                // mixed plateau of that parity, the latter has size at most one.
                if (span.le(value, sum) && span.le(sum, value)) {
                    size = size + right_size;
                }
                if (span.le(2, size)) one_more = one_more + size;
            };
            add_plateau(near, far);
            add_plateau(far, near);

            int left_depth = fix_mixed_depth(left_size, span);
            int right_depth = fix_mixed_depth(right_size, span);
            auto consider = [&](Int query) {
                Int left = left_size.at(query);
                Int right = right_size.at(query);
                Int single_step = one_more.at(query);
                Int cost = single_step + mixed_cost(left, left_depth)
                                       + mixed_cost(right, right_depth);
                int worst = max({left_depth, right_depth, int(single_step != 0)});
                auto score = make_tuple(cost, worst, max(left, right));
                if (score < best) {
                    best = score;
                    best_query = query;
                }
            };
            consider(span.first);
            consider(span.last);
            consider((span.first + span.last) / 2);

            // At fixed depth the cost is affine plus ceil((size-1)/unit).
            // Opposite unit slopes can introduce a one-unit oscillation,
            // whose minimum occurs when either numerator is divisible.
            auto consider_divisible = [&](Line size, int depth) {
                if (depth == 0 || size.slope == 0) return;
                Int remainder = (size.at(span.first) - 1) % unit;
                Int offset = size.slope < 0 ? remainder : (unit - remainder) % unit;
                Int first = span.first + offset;
                if (first <= span.last) {
                    Int count = (span.last - first) / unit;
                    consider(first);
                    consider(first + count * unit);
                    consider(first + (count / 2) * unit);
                }
            };
            consider_divisible(left_size, left_depth);
            consider_divisible(right_size, right_depth);
            start = span.last + 1;
        }
        return best_query;
    }

    Int choose_mixed_query() const {
        const Range& range = candidates[2];
        Int size = range.size();
        Int left = 1;
        if (size > ramp_width + 2) {
            Int unit = ramp_width + 1;
            Int nodes = (size + unit - 2) / unit;
            Int power = 1LL << (mixed_depth(size, unit) - 1);
            Int last_level = nodes - (power - 1);
            Int left_nodes = power / 2 - 1 + last_level / 2;
            left += unit * left_nodes;
        }
        return sum <= m ? range.low + left + ramp_width : range.low + left - 1;
    }

    pair<Int, Int> positions(int family, Int parameter) const {
        if (family == 0) return {parameter, sum - parameter};
        if (family == 1) return {-parameter, parameter - sum};
        return {parameter, parameter - sum};
    }

    void narrow(Int query, Int reading) {
        for (int family = 0; family < 3; ++family) {
            Range& range = candidates[family];
            if (range.size() == 0) continue;
            auto distance = [&](Int position) {
                Int difference = abs(query - position);
                return min(difference, n - difference);
            };
            auto value = [&](Int parameter) {
                auto [a, b] = positions(family, parameter);
                return distance(a) + distance(b);
            };
            Int low_value = value(range.low);
            Int high_value = value(range.high);
            if (low_value == high_value) {
                if (reading != low_value) range = {};
                continue;
            }
            Int direction = low_value < high_value ? 1 : -1;
            auto lower_bound = [&](Int target) {
                Int low = range.low;
                Int high = range.high + 1;
                while (low < high) {
                    Int mid = (low + high) / 2;
                    if (direction * value(mid) < target) low = mid + 1;
                    else high = mid;
                }
                return low;
            };
            Int first = lower_bound(direction * reading);
            Int last = lower_bound(direction * reading + 1) - 1;
            range = {first, last};
        }
    }

    Int label(Int coordinate) const {
        return (coordinate < 0 ? coordinate + n : coordinate) + 1;
    }

public:
    static Int ask(Int point) {
        cout << "? " << point << '\n' << flush;
        Int response;
        if (!(cin >> response) || response < 0) exit(0);
        return response;
    }

    Finder(Int n, Int sum)
        : n(n), m(n / 2), sum(sum), ramp_width((abs(n - 2 * sum) - 1) / 2) {
        Range same{max(1LL, sum - m), (sum - 1) / 2};
        candidates = {same, same, Range{max(0LL, sum - m), min(m, sum)}};
    }

    void solve() {
        if (candidates[0].size() > 0) {
            Int query = choose_second_query();
            narrow(query, ask(label(query)));
        }
        while (true) {
            Int total = candidates[0].size() + candidates[1].size()
                        + candidates[2].size();
            if (total == 0) return;
            if (total == 1) {
                for (int family = 0; family < 3; ++family) {
                    if (candidates[family].size() == 0) continue;
                    auto [a, b] = positions(family, candidates[family].low);
                    cout << "! " << label(a) << ' ' << label(b) << '\n' << flush;
                    return;
                }
            }
            Int query;
            if (candidates[0].size() + candidates[1].size() > 0) {
                // The center resolves every candidate in the larger same-side
                // group and separates it from the at-most-one extra candidate.
                query = candidates[0].size() >= candidates[1].size()
                    ? sum / 2 : -(sum / 2);
            } else {
                query = choose_mixed_query();
            }
            narrow(query, ask(label(query)));
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Int n;
    if (!(cin >> n) || n < 3 || n > 1000000000 || n % 2 == 0) return 0;
    Int sum = Finder::ask(1);
    if (sum < 1 || sum >= n) return 0;
    Finder(n, sum).solve();
}

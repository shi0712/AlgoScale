#include <bits/stdc++.h>
using namespace std;

constexpr int planning_limit = 32;

// With divisor 2, ordinary buckets are pairs. A pair opens at its first
// occurrence and closes at its second. Only 1 and, for even n, n are singles.
struct Segment {
    int opens, closes, pending, singles;

    int size() const {
        return opens + closes + singles;
    }
};

struct Range {
    int low = 1, high = 0;

    bool contains(int x) const {
        return low <= x && x <= high;
    }
};

Range possible_opens(const Segment& seg, int left, int singles_left) {
    int ordinary = left - singles_left;
    if (ordinary < 0 || seg.size() - left < seg.singles - singles_left)
        return {};
    // The number of closes cannot exceed either the total number of closes
    // in the segment or the number of pairs open by the end of the left part.
    return {
        max({0, ordinary - seg.closes, (ordinary - seg.pending + 1) / 2}),
        min(seg.opens, ordinary)
    };
}

pair<Segment, Segment> split(const Segment& seg, int left, int opens_left, int singles_left) {
    int closes_left = left - opens_left - singles_left;
    return {
        {opens_left, closes_left, seg.pending, singles_left},
        {seg.opens - opens_left, seg.closes - closes_left,
         seg.pending + opens_left - closes_left, seg.singles - singles_left}
    };
}

struct Plan {
    double expected = 0;
    uint64_t highest_first_replies = 0;
    int worst = 0;
    int left = 0;
    bool suffix = false;
};

// Use a uniform pairing model to choose queries for short intervals.
class Planner {
    int max_closes, pending_base, pending_span;
    vector<double> ways_cache;
    vector<Plan> plan_cache;

    size_t index(int opens, int closes, int pending) const {
        return (opens * (max_closes + 1) + closes) * pending_span + pending - pending_base;
    }

    double ways(int opens, int closes, int pending) {
        if (closes == 0)
            return 1;
        if (pending + opens < closes)
            return 0;
        if (opens == 0) {
            double result = 1;
            for (int j = 0; j < closes; ++j)
                result *= pending - j;
            return result;
        }
        double& value = ways_cache[index(opens, closes, pending)];
        if (value < 0) {
            // Opening creates one new pending pair. Closing chooses one
            // of the pairs currently pending.
            value = ways(opens - 1, closes, pending + 1);
            if (pending > 0)
                value += pending * ways(opens, closes - 1, pending - 1);
        }
        return value;
    }

    static int choices(int positions, int singles) {
        if (singles == 0) return 1;
        if (singles == 1) return positions;
        return positions * (positions - 1) / 2;
    }

    struct Outcome {
        double weight = 0;
        int left_worst = 0, right_worst = 0;
    };

    struct Reply {
        array<Outcome, 3> outcomes{};
        int mask = 0;
    };

    static bool better(int worst, double expected, int left, const Plan& best, int size) {
        if (expected < best.expected - 1e-10) return true;
        if (expected > best.expected + 1e-10) return false;
        if (worst != best.worst) return worst < best.worst;
        return abs(size - 2 * left) < abs(size - 2 * best.left);
    }

public:
    explicit Planner(const Segment& seg)
        : max_closes(seg.closes),
          pending_base(max(0, seg.pending - seg.closes)),
          pending_span(seg.pending + seg.opens - pending_base + 1),
          ways_cache((seg.opens + 1) * (seg.closes + 1) * pending_span, -1),
          plan_cache(2 * ways_cache.size()) {}

    Plan choose(const Segment& seg) {
        int size = seg.size();
        if (size == seg.singles)
            return {double(seg.singles - 1), 0, seg.singles - 1, 0, false};

        Plan& best = plan_cache[2 * index(seg.opens, seg.closes, seg.pending) + seg.singles - 1];
        if (best.left != 0)
            return best;
        best.expected = numeric_limits<double>::infinity();
        double total_weight = choices(size, seg.singles) * ways(seg.opens, seg.closes, seg.pending);
        // A midpoint split needs at most two divisor-2 queries. Distinguishing
        // the two singles costs one extra query the first time they separate.
        int budget = 2 * static_cast<int>(bit_width(static_cast<unsigned>(size - 1)))
                     + seg.singles - 2;

        for (int left = 1; left < size; ++left) {
            array<Reply, planning_limit + 1> prefix{}, suffix{};
            double remaining = 0;
            for (int t = 0; t <= seg.singles; ++t) {
                Range range = possible_opens(seg, left, t);
                int arrangements = choices(left, t) * choices(size - left, seg.singles - t);
                for (int c = range.low; c <= range.high; ++c) {
                    auto [a, b] = split(seg, left, c, t);
                    double weight = arrangements * ways(a.opens, a.closes, a.pending)
                                                 * ways(b.opens, b.closes, b.pending) / total_weight;
                    Plan pa, pb;
                    if (t > 0) pa = choose(a);
                    if (t < seg.singles) pb = choose(b);
                    remaining += weight * (t * pa.expected + (seg.singles - t) * pb.expected)
                                        / seg.singles;
                    Outcome outcome{weight, pa.worst, pb.worst};
                    prefix[c + t].outcomes[t] = outcome;
                    prefix[c + t].mask |= 1 << t;
                    suffix[c].outcomes[t] = outcome;
                    suffix[c].mask |= 1 << t;
                }
            }

            for (bool use_suffix : {false, true}) {
                const auto& replies = use_suffix ? suffix : prefix;
                int distinct_replies = 0, worst = 0;
                for (int r = 0; r <= left; ++r)
                    distinct_replies += replies[r].mask != 0;
                int first_cost = distinct_replies > 1;
                double expected = remaining;
                uint64_t highest_first = 0;
                for (int r = 0; r <= left; ++r) {
                    const Reply& reply = replies[r];
                    int mask = reply.mask;
                    if (mask == 0)
                        continue;

                    bool ambiguous = (mask & (mask - 1)) != 0;
                    int group_worst = 0;
                    double group_expected = 0;
                    for (int t = 0; t <= seg.singles; ++t) {
                        if (!(mask & (1 << t)))
                            continue;
                        const auto& outcome = reply.outcomes[t];
                        int extra = ambiguous + (seg.singles == 2 && t == 1);
                        group_worst = max(group_worst, extra + max(outcome.left_worst, outcome.right_worst));
                        group_expected += outcome.weight * extra;
                    }

                    if (seg.singles == 2 && (mask == 3 || mask == 6)) {
                        const auto& together = reply.outcomes[mask == 3 ? 0 : 2];
                        const auto& apart = reply.outcomes[1];
                        // If the singles may be apart, asking about n first
                        // can settle the split without the other divisor-2 query.
                        double alternate = 2 * together.weight + 1.5 * apart.weight;
                        int alternate_worst = max({
                            2 + max(together.left_worst, together.right_worst),
                            (mask == 3 ? 2 : 1) + apart.left_worst,
                            (mask == 3 ? 1 : 2) + apart.right_worst
                        });
                        if (alternate < group_expected - 1e-12 &&
                            first_cost + alternate_worst <= budget) {
                            group_expected = alternate;
                            group_worst = alternate_worst;
                            highest_first |= uint64_t(1) << r;
                        }
                    }
                    expected += group_expected;
                    worst = max(worst, group_worst);
                }
                expected += first_cost;
                worst += first_cost;

                if (worst <= budget && better(worst, expected, left, best, size))
                    best = {expected, highest_first, worst, left, use_suffix};
            }
        }
        return best;
    }
};

class Search {
    int n, buckets;

    int ask(int l, int r, int k) const {
        if (l == r)
            return 1;
        cout << "? " << l << ' ' << r << ' ' << k << endl;
        int reply;
        if (!(cin >> reply) || reply <= 0 || reply > min(r - l + 1, n / k + 1))
            exit(0);
        return reply;
    }

    bool highest_on_left(int cut) const {
        if (cut == 1)
            return ask(2, n, n) == 1;
        return ask(1, cut, n) == 2;
    }

    int find_in_four() const {
        // Orient the permutation so that 4 is in the last two positions.
        bool reverse = highest_on_left(2);
        auto index = [reverse](int i) { return reverse ? 5 - i : i; };
        auto query = [&](int l, int r, int k) {
            return reverse ? ask(index(r), index(l), k) : ask(l, r, k);
        };
        if (query(1, 2, 2) == 1)
            return index(query(2, 3, 4) == 2 ? 4 : 3);
        return index(query(2, 4, 2) == 2 ? 1 : 2);
    }

    int find_in_six() const {
        // A depth-six decision tree optimized by enumerating the 720
        // permutations. Negative destinations encode the answer.
        struct Probe {
            int l, r, k;
            array<int, 4> next;
        };
        constexpr Probe tree[] = {
            {1, 4, 2, { 0,  2,  3, 18}},
            {1, 5, 6, {-5, -6,  0,  0}},
            {1, 4, 6, { 4,  9,  0,  0}},
            {3, 6, 2, { 0,  0,  5,  8}},
            {2, 6, 2, { 0,  0, -1,  6}},
            {1, 2, 2, { 7, -2,  0,  0}},
            {4, 6, 2, { 0, -3, -4,  0}},
            {1, 3, 2, { 0, -4, -3,  0}},
            {5, 6, 2, {10, 17,  0,  0}},
            {3, 5, 6, {11, 14,  0,  0}},
            {3, 5, 2, { 0, 12, 13,  0}},
            {2, 3, 6, {-2, -1,  0,  0}},
            {1, 3, 2, { 0, -4, -3,  0}},
            {1, 2, 2, {15, 16,  0,  0}},
            {2, 3, 6, {-3, -4,  0,  0}},
            {2, 4, 2, { 0, -1, -2,  0}},
            {1, 5, 2, { 0,  0, -6, -5}},
            {3, 6, 2, { 0, 19, 20, 23}},
            {2, 6, 6, {-2, -1,  0,  0}},
            {3, 5, 6, {21, 22,  0,  0}},
            {4, 6, 2, { 0, -3, -4,  0}},
            {2, 6, 2, { 0,  0, -1, -2}},
            {4, 6, 6, {-4, -3,  0,  0}}
        };
        int node = 1;
        while (node > 0) {
            const auto& [l, r, k, next] = tree[node - 1];
            node = next[ask(l, r, k) - 1];
            if (node == 0)
                exit(0);
        }
        return -node;
    }

public:
    explicit Search(int n) : n(n), buckets(n / 2 + 1) {}

    int find() const {
        if (n == 4)
            return find_in_four();
        if (n == 6)
            return find_in_six();

        int lo = 0, opens_before = 0, singles_before = 0;
        Segment seg{n - buckets, n - buckets, 0, 2 * buckets - n};
        unique_ptr<Planner> planner;

        while (seg.size() > 1) {
            if (seg.size() == 2 && seg.singles == 2)
                return lo + (highest_on_left(lo + 1) ? 2 : 1);

            Plan plan;
            if (seg.size() <= planning_limit) {
                if (!planner)
                    planner = make_unique<Planner>(seg);
                plan = planner->choose(seg);
            } else {
                plan.left = seg.size() / 2;
                plan.suffix = seg.closes < seg.opens;
            }

            int cut = lo + plan.left;
            array<Range, 3> ranges;
            int low = n + 1, high = -1;
            for (int t = 0; t <= seg.singles; ++t) {
                ranges[t] = possible_opens(seg, plan.left, t);
                if (ranges[t].low <= ranges[t].high) {
                    int extra = plan.suffix ? 0 : t;
                    low = min(low, ranges[t].low + extra);
                    high = max(high, ranges[t].high + extra);
                }
            }

            // Prefix distinct counts opens plus singles. The suffix gives the
            // number of buckets already finished, hence the number of opens.
            int observed = low;
            if (low != high) {
                if (plan.suffix)
                    observed = cut - buckets + ask(cut + 1, n, 2) - opens_before;
                else
                    observed = ask(1, cut, 2) - opens_before - singles_before;
            }
            if (observed < low || observed > high)
                exit(0);
            int mask = 0;
            for (int t = 0; t <= seg.singles; ++t)
                if (ranges[t].contains(observed - (plan.suffix ? 0 : t)))
                    mask |= 1 << t;

            int highest_left = -1;
            if (plan.highest_first_replies && ((plan.highest_first_replies >> observed) & 1)) {
                highest_left = highest_on_left(cut);
                mask &= highest_left ? 6 : 3;
            }

            if (mask == 0)
                exit(0);
            int singles_left;
            if ((mask & (mask - 1)) == 0) {
                singles_left = countr_zero(static_cast<unsigned>(mask));
            } else if (plan.suffix) {
                singles_left = ask(1, cut, 2) - opens_before - singles_before - observed;
            } else {
                singles_left = observed - (cut - buckets + ask(cut + 1, n, 2) - opens_before);
            }
            if (singles_left < 0 || singles_left > seg.singles || !(mask & (1 << singles_left)))
                exit(0);

            int opens_left = observed - (plan.suffix ? 0 : singles_left);
            auto [left, right] = split(seg, plan.left, opens_left, singles_left);
            bool keep_left = singles_left != 0;
            if (seg.singles == 2 && singles_left == 1) {
                if (highest_left == -1)
                    highest_left = highest_on_left(cut);
                keep_left = !highest_left;
            }
            if (keep_left) {
                seg = left;
            } else {
                lo = cut;
                opens_before += opens_left;
                singles_before += singles_left;
                seg = right;
            }
        }
        return lo + 1;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 3 || n > 1024)
        return 0;
    int answer = Search(n).find();
    cout << "! " << answer << endl;
}

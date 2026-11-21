#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using Pair = pair<i64, i64>;

struct Interval {
    int family;
    i64 lo, hi;
    i64 size() const { return hi - lo + 1; }
};

// A query on a consecutive set of translates can have w singleton replies
// between two constant end intervals. capacity[h] candidates fit in h queries.
struct TranslationCosts {
    i64 w;
    vector<i64> capacity{1}, full_cost{0};

    TranslationCosts(i64 width, i64 length) : w(width) {
        while (capacity.back() < length) {
            i64 next = 2 * capacity.back() + w;
            full_cost.push_back(next + 2 * full_cost.back());
            capacity.push_back(next);
        }
    }

    int height(i64 count) const {
        return lower_bound(capacity.begin(), capacity.end(), count) - capacity.begin();
    }

    i64 cost(i64 count) const {
        int h = height(count);
        if (h == 0) return 0;
        // From a full tree of height h-1, expanding one bottom leaf adds
        // up to w+1 candidates. Pack expansions to keep other leaves shallow.
        i64 extra = count - capacity[h - 1];
        return full_cost[h - 1] + h * extra + (extra + w) / (w + 1);
    }

    i64 right_size(i64 count) const {
        int h = height(count);
        if (h <= 1) return 1;
        return min(capacity[h - 1], count - w - capacity[h - 2]);
    }
};

struct Search {
    i64 n, m, s;
    i64 d, start, length, lo, hi;
    TranslationCosts costs;
    vector<Interval> possible;

    Search(i64 n, i64 s) : n(n), m(n / 2), s(s), d(min(s, n - s)),
        start(-min(m, s)), length(d + (s <= m)), lo(max(0LL, s - m)),
        hi((s - 1) / 2), costs(m - d, length) {
        // Signed coordinates relative to the first query (point 0).
        // The boundary pairs containing 0 may belong to two families.
        if (lo <= hi) {
            possible.push_back({1, lo, hi});
            possible.push_back({-1, lo, hi});
        }
        possible.push_back({0, start, start + length - 1});
    }

    Pair pair_at(int family, i64 t) const {
        if (family == 1) return {t, s - t};
        if (family == -1) return {t - s, -t};
        return {t, t + s};
    }

    i64 dist(i64 x, i64 y) const {
        i64 delta = abs(x - y);
        return min(delta, n - delta);
    }

    i64 value(Pair p, i64 q) const {
        return dist(p.first, q) + dist(p.second, q);
    }

    void filter(i64 q, i64 reply) {
        vector<Interval> next;
        for (auto seg : possible) {
            // On each initial interval the profile is monotone in t. For
            // translates its length is at most one more than the shorter
            // plateau, so it cannot include both an ascent and a descent.
            auto f = [&](i64 t) { return value(pair_at(seg.family, t), q); };
            int sign = f(seg.lo) <= f(seg.hi) ? 1 : -1;
            auto bound = [&](i64 target) {
                i64 l = seg.lo, r = seg.hi + 1;
                while (l < r) {
                    i64 mid = l + (r - l) / 2;
                    if (sign * f(mid) < target) l = mid + 1;
                    else r = mid;
                }
                return l;
            };
            i64 first = bound(sign * reply), last = bound(sign * reply + 1) - 1;
            if (first <= last) next.push_back({seg.family, first, last});
        }
        possible = move(next);
    }

    i64 second_query() const {
        const i64 w = costs.w, period = w + 1;
        auto plateau = [&](i64 first) {
            return clamp(hi - first + 1, 0LL, hi - lo + 1);
        };

        // Only constant runs can give ambiguous replies. For the two
        // fixed-midpoint families count pairs on one side of q or its
        // antipode. For translates intersect their first plateau with the
        // two plateaus of the new measurement.
        using Score = tuple<i64, int, i64, i64, i64>;
        auto score = [&](i64 q) -> Score {
            i64 a = plateau(min(q, s - q));
            i64 b = plateau(min(s - m + q, m + 1 - q));
            i64 c = max(0LL, length - q), e = max(0LL, q - w);
            auto ambiguous = [](i64 k) { return k > 1 ? k : 0LL; };
            i64 same_side_cost = ambiguous(a) + ambiguous(b);
            i64 total = same_side_cost + costs.cost(c) + costs.cost(e);
            int worst = max({int(same_side_cost > 0), costs.height(c), costs.height(e)});
            return {total, worst, same_side_cost + ambiguous(c) + ambiguous(e),
                    abs(c - e), q};
        };

        if (m <= 256) {
            i64 best = 1;
            for (i64 q = 2; q <= m; ++q)
                if (score(q) < score(best)) best = q;
            return best;
        }

        vector<i64> events{1, m, s / 2, (n - s) / 2, (length + w) / 2,
                           length, w};
        for (i64 t : {lo, hi - 1, hi, hi + 1}) {
            events.insert(events.end(), {t, s - t, t + m - s, m + 1 - t});
        }
        for (i64 cap : costs.capacity) {
            events.push_back(length - cap);
            events.push_back(w + cap);
        }

        // Between events the objective is affine except for two rounded
        // terms, both with period w+1. A minimum occurs by an event or
        // by a rounding step in the first or last period of the interval.
        vector<i64> queries;
        auto nearby = [&](i64 center) {
            for (i64 q = center - 2; q <= center + 2; ++q)
                if (1 <= q && q <= m) queries.push_back(q);
        };
        for (i64 event : events) {
            i64 x = clamp(event, 1LL, m);
            nearby(x);
            for (i64 residue : {length - 1, w + 1}) {
                i64 offset = ((x - residue) % period + period) % period;
                nearby(x - offset);
                nearby(x - offset + period);
            }
        }
        sort(queries.begin(), queries.end());
        queries.erase(unique(queries.begin(), queries.end()), queries.end());
        Score best = score(queries.front());
        for (i64 q : queries) best = min(best, score(q));
        return get<4>(best);
    }

    // Either return the answer, or a signed coordinate for the next query.
    variant<Pair, i64> choose(bool second) const {
        auto largest = *max_element(possible.begin(), possible.end(),
            [](Interval a, Interval b) { return a.size() < b.size(); });
        if (largest.size() == 1) {
            vector<Pair> candidates;
            for (auto seg : possible) candidates.push_back(pair_at(seg.family, seg.lo));
            sort(candidates.begin(), candidates.end());
            candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());
            if (candidates.size() == 1) return candidates[0];

            // An endpoint distinguishes different shortest arcs: it either
            // attains a smaller minimum or lies outside the other minimum arc.
            i64 best = 0;
            size_t best_count = 0;
            for (auto [a, b] : candidates) for (i64 q : {a, b}) {
                set<i64> replies;
                for (auto p : candidates) replies.insert(value(p, q));
                if (replies.size() > best_count) {
                    best_count = replies.size();
                    best = q;
                }
            }
            return best;
        }
        if (second) return second_query();
        if (largest.family != 0) return largest.family * (s / 2);
        // Place the beginning of the original plateau at the first element
        // of the desired right child. The w intermediate values precede it.
        return largest.hi - costs.right_size(largest.size()) + 1 - start;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    i64 n, reply;
    if (!(cin >> n) || n < 3 || n > 1000000000 || n % 2 == 0) return 0;
    auto ask = [&](i64 q) {
        cout << "? " << (q + n) % n + 1 << endl;
        return bool(cin >> reply) && reply >= 1 && reply < n;
    };
    if (!ask(0)) return 0;
    Search search(n, reply);
    bool second = true;
    while (!search.possible.empty()) {
        auto decision = search.choose(second);
        if (auto p = get_if<Pair>(&decision)) {
            cout << "! " << (p->first + n) % n + 1 << ' '
                 << (p->second + n) % n + 1 << endl;
            return 0;
        }
        i64 q = get<i64>(decision);
        if (!ask(q)) return 0;
        search.filter(q, reply);
        second = false;
    }
    return 0;
}

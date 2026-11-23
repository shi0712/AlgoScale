#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using u64 = unsigned long long;

i64 floor_power(i64 x) {
    return static_cast<i64>(bit_floor(static_cast<u64>(x)));
}

i64 ceil_power(i64 x) {
    return static_cast<i64>(bit_ceil(static_cast<u64>(x)));
}

// Minimum number of aligned dyadic pieces of size at most cap in [l, r).
i64 pieces(i64 l, i64 r, i64 cap) {
    if (l == r) return 0;
    i64 a = l / cap, b = r / cap;
    if (a != b) {
        i64 s = l % cap, t = r % cap;
        return b - a - (s != 0)
            + (s ? popcount(static_cast<u64>(cap - s)) : 0)
            + popcount(static_cast<u64>(t));
    }
    i64 len = r - l;
    if ((len & (len - 1)) == 0 && (l & (len - 1)) == 0) return 1;
    i64 d = floor_power(l ^ (r - 1));
    i64 m = (l & ~(d - 1)) + d;
    return popcount(static_cast<u64>(m - l))
         + popcount(static_cast<u64>(r - m));
}

i64 largest_piece(i64 l, i64 r) {
    i64 d = floor_power(r - l);
    if (((l + d - 1) & ~(d - 1)) + d > r) d /= 2;
    return d;
}

struct Range {
    i64 lo, hi;
    i64 width() const { return hi - lo; }
    i64 center() const { return (lo + hi) / 2; }
};

class Solver {
    int n;
    i64 lower = 1, upper = 1LL << 30;
    map<i64, int> by_sum;
    map<int, i64> by_index;

    i64 query(int r) {
        cout << "? 1 " << r << '\n' << flush;
        i64 s;
        if (!(cin >> s) || s == -1) exit(0);
        return s;
    }

    void observe_gap(i64 l, i64 r, i64 count) {
        upper = min({upper, largest_piece(l, r), floor_power(r - l - count + 1)});
        lower = max(lower, ceil_power((r - l + count - 1) / count));
        while (pieces(l, r, lower) > count) lower *= 2;
    }

    bool remember(i64 s, int i) {
        auto [p, inserted] = by_sum.emplace(s, i);
        if (!inserted) return false;
        by_index.emplace(i, s);
        if (p != by_sum.begin()) {
            auto q = prev(p);
            observe_gap(q->first, s, i - q->second);
        }
        auto q = next(p);
        if (q != by_sum.end()) observe_gap(s, q->first, q->second - i);
        return true;
    }

    void probe(int i) {
        auto q = by_index.lower_bound(i);
        if (q != by_index.end() && q->first == i) return;

        auto p = prev(q);
        i64 l = p->second, r = q->second;
        i64 count = q->first - p->first, take = i - p->first;
        i64 s;
        if (r - l == count) {
            s = l + take;
        } else if (pieces(l, r, upper) == count) {
            // A minimum-size partition must use the canonical dyadic pieces.
            s = l;
            while (take) {
                if (s % upper == 0 && r - s >= upper) {
                    i64 run = min(take, (r - s) / upper);
                    s += run * upper;
                    take -= run;
                } else {
                    i64 d = min(upper, floor_power(r - s));
                    if (s != 0) d = min(d, s & -s);
                    s += d;
                    --take;
                }
            }
        } else {
            s = query(i);
        }
        remember(s, i);
    }

    // Called only for mass coordinates that are known to be prefix boundaries.
    Range position(i64 s) const {
        auto q = by_sum.lower_bound(s);
        if (q->first == s) return {q->second, q->second};
        auto p = prev(q);
        return {
            max(p->second + pieces(p->first, s, upper), q->second - (q->first - s)),
            min(q->second - pieces(s, q->first, upper), p->second + (s - p->first))
        };
    }

    Range length_range(i64 l, i64 r, Range x, Range y, Range old) const {
        return {
            max({old.lo, y.lo - x.hi, pieces(l, r, upper)}),
            min({old.hi, y.hi - x.lo, r - l})
        };
    }

    static void tighten(Range &x, Range &y, Range len) {
        x.lo = max(x.lo, y.lo - len.hi);
        x.hi = min(x.hi, y.hi - len.lo);
        y.lo = max(y.lo, x.lo + len.lo);
        y.hi = min(y.hi, x.hi + len.hi);
    }

    i64 block_lower(i64 l, i64 r, i64 max_len) const {
        i64 v = ceil_power((r - l + max_len - 1) / max_len);
        auto first = by_sum.upper_bound(l);
        if (first->first >= r) return v;
        for (;;) {
            i64 count = 0, p = l;
            for (auto it = first; it->first < r; ++it) {
                count += pieces(p, it->first, v);
                p = it->first;
            }
            count += pieces(p, r, v);
            if (count <= max_len) return v;
            v *= 2;
        }
    }

    i64 block_upper(i64 l, i64 r, i64 min_len) const {
        i64 cap = min(upper, floor_power(r - l - min_len + 1));
        auto p = prev(by_sum.upper_bound(l));
        i64 best = 1;
        while (p->first < r) {
            auto q = next(p);
            i64 a = max(l, p->first), b = min(r, q->first);
            i64 gap_cap = floor_power(q->first - p->first - (q->second - p->second) + 1);
            best = max(best, min({cap, gap_cap, largest_piece(a, b)}));
            if (best == cap) break;
            p = q;
        }
        return best;
    }

    static i64 comparison_probe(Range x, Range m, Range y) {
        if (x.width() == 0 && y.width() == 0) return (x.lo + y.lo) / 2;
        if (m.width() == 0 && x.width() == 0)
            return clamp(2 * m.lo - x.lo - 1, y.lo, y.hi);
        if (m.width() == 0 && y.width() == 0)
            return clamp(2 * m.lo - y.lo - 1, x.lo, x.hi);

        Range target;
        i64 preferred;
        if (m.width() > 0 && 2 * m.width() >= max(x.width(), y.width())) {
            target = m;
            preferred = (x.center() + y.center()) / 2;
        } else if (x.width() >= y.width()) {
            target = x;
            preferred = 2 * m.center() - y.center() - 1;
        } else {
            target = y;
            preferred = 2 * m.center() - x.center() - 1;
        }
        i64 margin = target.width() / 4;
        return clamp(preferred, target.lo + margin, target.hi - margin);
    }

public:
    explicit Solver(int n_) : n(n_) {}

    i64 solve() {
        i64 total = query(n);
        remember(0, 0);
        remember(total, n);

        // The active aligned mass block always contains a global maximum.
        i64 l = 0, r = total;
        Range len{n, n};

        while (lower < upper) {
            i64 previous_upper = upper;
            Range x = position(l), y = position(r);
            len = length_range(l, r, x, y, len);
            tighten(x, y, len);
            bool added = false;
            if (x.width() == 0) added |= remember(l, static_cast<int>(x.lo));
            if (y.width() == 0) added |= remember(r, static_cast<int>(y.lo));

            lower = max(lower, block_lower(l, r, len.hi));
            upper = min(upper, block_upper(l, r, len.lo));
            if (lower == upper) break;

            len.lo = max(len.lo, pieces(l, r, upper));
            tighten(x, y, len);

            if (len.lo == 1) {
                if (added) continue;
                // Find whether this block consists of just one element.
                i64 i;
                if (x.width() == 0) i = x.lo + 1;
                else if (y.width() == 0) i = y.lo - 1;
                else i = (x.width() <= y.width() ? x : y).center();
                probe(static_cast<int>(i));
                continue;
            }

            i64 mid = (l + r) / 2;
            Range m = position(mid);
            Range left{1, len.hi - 1}, right{1, len.hi - 1};
            for (int pass = 0; pass < 3; ++pass) {
                left = length_range(l, mid, x, m, left);
                right = length_range(mid, r, m, y, right);
                left.lo = max(left.lo, len.lo - right.hi);
                right.lo = max(right.lo, len.lo - left.hi);
                left.hi = min(left.hi, len.hi - right.lo);
                right.hi = min(right.hi, len.hi - left.lo);
                len.lo = max(len.lo, left.lo + right.lo);
                len.hi = min(len.hi, left.hi + right.hi);
                tighten(x, m, left);
                tighten(m, y, right);
                tighten(x, y, len);
            }
            if (x.width() == 0) added |= remember(l, static_cast<int>(x.lo));
            if (m.width() == 0) added |= remember(mid, static_cast<int>(m.lo));
            if (y.width() == 0) added |= remember(r, static_cast<int>(y.lo));

            i64 low_l = block_lower(l, mid, left.hi);
            i64 low_r = block_lower(mid, r, right.hi);
            i64 up_l = block_upper(l, mid, left.lo);
            i64 up_r = block_upper(mid, r, right.lo);
            lower = max({lower, low_l, low_r, block_lower(l, r, len.hi)});
            upper = min(upper, max(up_l, up_r));
            if (lower == upper) break;

            // Siblings come from a common array by local flatten operations.
            // Additional flatten operations increase length and never increase max.
            bool short_l = left.hi <= right.lo || 2 * m.hi <= x.lo + y.lo;
            bool short_r = right.hi <= left.lo || 2 * m.lo >= x.hi + y.hi;
            bool use_l = short_l || up_r <= low_l || up_r < lower;
            bool use_r = short_r || up_l <= low_r || up_l < lower;
            if (use_l || use_r) {
                if (short_l) left.hi = min(left.hi, len.hi / 2);
                if (short_r) right.hi = min(right.hi, len.hi / 2);
                if (use_l && (!use_r ||
                    make_tuple(up_l, left.hi, x.width()) <=
                    make_tuple(up_r, right.hi, y.width()))) {
                    r = mid;
                    len = left;
                    upper = min(upper, up_l);
                } else {
                    l = mid;
                    len = right;
                    upper = min(upper, up_r);
                }
                continue;
            }

            if (added || upper < previous_upper) continue;
            probe(static_cast<int>(comparison_probe(x, m, y)));
        }
        return lower;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;
    Solver solver(n);
    i64 answer = solver.solve();
    cout << "! " << answer << '\n' << flush;
    return 0;
}

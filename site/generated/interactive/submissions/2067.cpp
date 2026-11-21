#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
constexpr i64 LIMIT = 1'000'000'000;
constexpr i64 INF = 1LL << 60;

// Interval inclusion bounds. In coordinates (l, n+1-r), intervals
// containing [l,r] form a prefix rectangle. Reverse both axes for subsets.
class Oracle {
    int n, stride;
    vector<i64> caps, floors;

public:
    explicit Oracle(int size) : n(size), stride(n + 1),
        caps(stride * stride, INF), floors(stride * stride, 0) {}

    void remember(int l, int r, i64 v) {
        for (int i = l; i <= n; i += i & -i)
            for (int j = n + 1 - r; j <= n; j += j & -j)
                caps[i * stride + j] = min(caps[i * stride + j], v);
        for (int i = n + 1 - l; i <= n; i += i & -i)
            for (int j = r; j <= n; j += j & -j)
                floors[i * stride + j] = max(floors[i * stride + j], v);
    }

    i64 upper(int l, int r) const {
        i64 v = (r - l + 1) * LIMIT;
        for (int i = l; i; i -= i & -i)
            for (int j = n + 1 - r; j; j -= j & -j)
                v = min(v, caps[i * stride + j]);
        return v;
    }

    i64 lower(int l, int r) const {
        i64 low = 0;
        for (int i = n + 1 - l; i; i -= i & -i)
            for (int j = r; j; j -= j & -j)
                low = max(low, floors[i * stride + j]);
        return low;
    }

    i64 ask(int l, int r, i64 low = 0, i64 high = INF) {
        high = min(high, upper(l, r));
        low = max(low, lower(l, r));
        if (high == low) return low;
        cout << "? " << l << ' ' << r << endl;
        i64 v;
        if (!(cin >> v) || v < low || v > high) exit(0);
        remember(l, r, v);
        return v;
    }
};

struct Rise {
    int l, r;
    i64 height;
    int gapAfter = -1;
};

struct Gap {
    i64 low, high;
    int position; // One nonpositive run, or one as-yet unmeasured element.
    int left = -1, right = -1;
    i64 buried = 0;
};

class Solver {
    int n;
    Oracle oracle;
    vector<i64> answer;
    vector<bool> skipped;
    vector<Gap> gaps;

    void setPositivePart(int i, i64 v) {
        answer[i] = v;
        oracle.remember(i, i, v);
    }

    // Adding one cell to a positive run increases its MSS by the
    // positive part of that cell.
    i64 singleton(int i) {
        int r = i;
        while (r < n && oracle.upper(r + 1, r + 1) == 0) ++r;
        i64 s = 0, low = oracle.lower(i, r), high = LIMIT;
        for (int j = i - 1; j >= 1 && answer[j] > 0; --j) {
            s += answer[j];
            low = max(low, oracle.lower(j, r) - s);
            high = min(high, oracle.upper(j, i) - s);
        }
        i64 v = oracle.ask(i, i, low, high);
        setPositivePart(i, v);
        return v;
    }

    // Read up to the first positive part of a nonempty window. A pair
    // needs only one singleton once its maximum is known.
    int decodeWindow(int l, int r, i64 m) {
        while (r - l > 1) {
            int mid = (l + r) / 2;
            i64 leftMax = oracle.ask(l, mid);
            if (leftMax > 0) {
                r = mid;
                m = leftMax;
            } else {
                l = mid + 1;
                oracle.remember(l, r, m);
            }
        }
        if (l == r) {
            setPositivePart(l, m);
        } else {
            i64 left = oracle.upper(l, l), right = oracle.upper(r, r);
            if (left + right == m) {
                setPositivePart(l, left);
                setPositivePart(r, right);
            } else {
                i64 x = singleton(l);
                setPositivePart(r, m - x);
            }
        }
        return r + 1;
    }

    void discover(i64 totalMax) {
        if (n <= 2) {
            decodeWindow(1, n, totalMax);
            return;
        }
        int i = 1, window = 1;
        int lastPositive = 0;
        array<int, 4> distances{};
        bool checkedTail = false, checkedFirstPeak = false;
        while (i <= n) {
            int start = i;
            // Repeated long gaps can be checked with a single group test.
            bool periodic = distances[0] == distances[2] &&
                            distances[1] == distances[3] && distances[1] > 0;
            int spacing = periodic ? distances[1] : min(distances[0], distances[1]);
            int predictedEnd = lastPositive + spacing - 1;
            if (!checkedFirstPeak && lastPositive && answer[lastPositive] == totalMax) {
                checkedFirstPeak = true;
                if (oracle.ask(i, n) == 0) break;
            }
            if (lastPositive && i <= predictedEnd && spacing >= 3) {
                int r = min(n, predictedEnd);
                i64 m = oracle.ask(i, r);
                i = m ? decodeWindow(i, r, m) : r + 1;
                window = m ? 2 : 1;
            } else if (periodic && spacing == 1 && distances[0] >= 3 &&
                       i == lastPositive + 1) {
                singleton(i++);
                window = 2;
            } else if (i > 1 && answer[i - 1] > 0 && i < n) {
                // Leave a one-cell gap unmeasured between two positive
                // anchors. Its sign and relevant magnitude can be learned
                // together when the adjacent rises are compared.
                if (singleton(i + 1) > 0) {
                    skipped[i] = true;
                } else {
                    singleton(i);
                }
                i += 2;
                window = (answer[i - 2] > 0 ? 1 : 2);
            } else {
                int tailThreshold = max(8, 2 * max(distances[0], distances[1]));
                if (!checkedTail && i - lastPositive >= tailThreshold && i < n) {
                    if (lastPositive && oracle.ask(i, n) == 0) break;
                    checkedTail = true;
                    if (!lastPositive) {
                        // The initial nonpositive prefix can be stripped
                        // from the already measured full interval.
                        oracle.remember(i, n, totalMax);
                        window = n - i + 1;
                    }
                }
                int r = min(n, i + window - 1);
                if (!lastPositive && r == n) oracle.remember(i, n, totalMax);
                i64 m = i == r ? singleton(i) : oracle.ask(i, r);
                if (m == 0) {
                    i = r + 1;
                    window *= window < 8 ? 2 : 4;
                } else {
                    i = decodeWindow(i, r, m);
                    window = (i >= 3 && answer[i - 2] > 0 ? 1 : 2);
                }
            }
            for (int j = start; j < i; ++j) {
                if (answer[j] <= 0) continue;
                for (int d = 3; d > 0; --d) distances[d] = distances[d - 1];
                distances[0] = lastPositive ? j - lastPositive : 0;
                lastPositive = j;
                checkedTail = false;
            }
        }
    }

    // Realize a drop. A buried rise consumes part of the two drops on
    // either side, so their sum is drop + buried.
    void realize(int id, i64 drop) {
        const Gap &g = gaps[id];
        if (g.left == -1) {
            answer[g.position] = -drop;
            return;
        }
        i64 both = drop + g.buried;
        i64 d = max(gaps[g.left].low, both - gaps[g.right].high);
        realize(g.left, d);
        realize(g.right, both - d);
    }

    int bury(int left, int right, i64 height) {
        int id = (int)gaps.size();
        gaps.push_back({gaps[left].low + gaps[right].low - height,
                        gaps[left].high + gaps[right].high - height,
                        0, left, right, height});
        return id;
    }

    vector<Rise> makeRises() {
        vector<Rise> rises;
        for (int i = 1; i <= n; ) {
            if (answer[i] == 0) { ++i; continue; }
            int l = i;
            i64 sum = 0;
            while (i <= n && answer[i] > 0) sum += answer[i++];
            if (!rises.empty()) {
                int start = rises.back().r + 1;
                rises.back().gapAfter = (int)gaps.size();
                gaps.push_back({skipped[start] ? -LIMIT : 0,
                                (l - start) * LIMIT, start});
            }
            rises.push_back({l, i - 1, sum});
            oracle.remember(l, i - 1, sum);
        }
        return rises;
    }

    // A rise has its minimum prefix at the start and maximum at the end.
    // A down block has the opposite extrema and its internal MSS is no
    // greater than either neighbor. Thus the only candidate beating the
    // individual rises is their full sum minus the intervening drop.
    bool tryMerge(vector<Rise>& a, int i) {
        Rise &x = a[i], &y = a[i + 1];
        Gap &g = gaps[x.gapAfter];
        i64 small = min(x.height, y.height), big = max(x.height, y.height);
        if (g.low < small) {
            i64 sum = x.height + y.height;
            i64 v = oracle.ask(x.l, y.r, max(big, sum - g.high), sum - g.low);
            if (v > big) g.low = g.high = sum - v;
            else g.low = small;
        }
        if (g.low != g.high || g.low > small) return false;

        realize(x.gapAfter, g.low);
        x.height += y.height - g.low;
        x.r = y.r;
        x.gapAfter = y.gapAfter;
        oracle.remember(x.l, x.r, x.height);
        a.erase(a.begin() + i + 1);
        return true;
    }

    // Lower each drop as far as its observations allow and find the best
    // possible MSS. When this upper bound is attained, every gap common to
    // all optimal intervals must have exactly its minimum drop.
    bool recoverMaximum(vector<Rise>& a, i64 cap) {
        i64 ending = a[0].height, best = ending;
        int start = 0, commonL = 0, commonR = 0;
        for (int i = 1; i < (int)a.size(); ++i) {
            ending -= gaps[a[i-1].gapAfter].low;
            if (ending <= 0) {
                ending = 0;
                start = i;
            }
            ending += a[i].height;
            if (ending > cap) return false;
            if (ending > best) {
                best = ending;
                commonL = start;
                commonR = i;
            } else if (ending == best) {
                commonL = max(commonL, start);
            }
        }
        if (best != cap || commonL >= commonR) return false;
        // Overlapping maximizing intervals have equal starting prefix
        // sums and equal ending prefix sums. Their intersection wins too.
        for (int i = commonL; i < commonR; ++i) {
            int id = a[i].gapAfter;
            realize(id, gaps[id].low);
        }
        a[commonL] = {a[commonL].l, a[commonR].r, cap, a[commonR].gapAfter};
        oracle.remember(a[commonL].l, a[commonL].r, cap);
        a.erase(a.begin() + commonL + 1, a.begin() + commonR + 1);
        return true;
    }

    void reconstruct(vector<Rise> initial, i64 totalMax) {
        oracle.remember(initial.front().l, initial.back().r, totalMax);
        vector<vector<Rise>> todo;
        todo.push_back(move(initial));
        while (!todo.empty()) {
            auto a = move(todo.back());
            todo.pop_back();
            if (a.size() < 2) continue;
            i64 tallest = max_element(a.begin(), a.end(), [](auto &x, auto &y) {
                return x.height < y.height;
            })->height;
            i64 cap = oracle.ask(a.front().l, a.back().r, tallest);
            while (a.size() > 1) {
                // An entire rise attaining the region maximum separates
                // the problems on its two sides. All suffixes immediately
                // before it and prefixes immediately after it sum to <= 0.
                if (tallest == cap || recoverMaximum(a, cap)) {
                    int from = 0;
                    for (int i = 0; i < (int)a.size(); ++i) {
                        if (a[i].height != cap) continue;
                        if (from < i) {
                            todo.emplace_back(a.begin() + from, a.begin() + i);
                            int id = a[i - 1].gapAfter;
                            realize(id, 2 * n * LIMIT);
                        }
                        if (i + 1 < (int)a.size()) {
                            int id = a[i].gapAfter;
                            realize(id, 2 * n * LIMIT);
                        }
                        from = i + 1;
                    }
                    if (from < (int)a.size())
                        todo.emplace_back(a.begin() + from, a.end());
                    break;
                }

                int k = 0, last = (int)a.size() - 1;
                i64 sumHeights = 0;
                int hole = -1;
                for (int i = 0; i <= last; ++i) {
                    if (a[i].height < a[k].height) k = i;
                    sumHeights += a[i].height;
                    if (i < last && gaps[a[i].gapAfter].low < 0) hole = i;
                }
                // On a potentially nonnegative interval, resolve the
                // unmeasured cells before spending queries on known dips.
                if (hole >= 0 && sumHeights <= cap) {
                    if (tryMerge(a, hole)) tallest = max(tallest, a[hole].height);
                    continue;
                }
                if (a[last].height == a[k].height) k = last;
                // Test the gap with the smaller lower bound first.
                bool leftFirst = k > 0 && (k == last ||
                    tie(gaps[a[k-1].gapAfter].low, gaps[a[k-1].gapAfter].high) <=
                    tie(gaps[a[k].gapAfter].low, gaps[a[k].gapAfter].high));
                int first = leftFirst ? k - 1 : k;
                int other = leftFirst ? k : k - 1;
                if (tryMerge(a, first)) {
                    tallest = max(tallest, a[first].height);
                    continue;
                }
                if (other >= 0 && other < last && tryMerge(a, other)) {
                    tallest = max(tallest, a[other].height);
                    continue;
                }
                // Each incident drop is at least this rise's height.
                // Drop, rise, drop is therefore another down block.
                if (k == 0 || k == last) {
                    int id = a[k == 0 ? 0 : k-1].gapAfter;
                    realize(id, 2 * n * LIMIT);
                } else {
                    a[k-1].gapAfter = bury(a[k-1].gapAfter, a[k].gapAfter,
                                           a[k].height);
                }
                a.erase(a.begin() + k);
                if (k == 0 || k == last)
                    oracle.remember(a.front().l, a.back().r, cap);
            }
        }
    }

    // At most LIMIT of deficit from the largest possible sum guarantees
    // that every interior cell is nonnegative. A negative endpoint can
    // be replaced by zero. All interval replies are then additive.
    void recoverAdditive(int l, int r, i64 sum) {
        if (sum == 0) return;
        if (sum == (r - l + 1) * LIMIT) {
            fill(answer.begin() + l, answer.begin() + r + 1, LIMIT);
        } else if (l == r) {
            answer[l] = sum;
        } else {
            int mid = (l + r) / 2;
            i64 left = oracle.ask(l, mid, max(0LL, sum - (r - mid) * LIMIT),
                                  min(sum, (mid - l + 1) * LIMIT));
            recoverAdditive(l, mid, left);
            recoverAdditive(mid + 1, r, sum - left);
        }
    }

public:
    explicit Solver(int size) : n(size), oracle(n), answer(n + 1), skipped(n + 1) {}

    void run() {
        i64 m = oracle.ask(1, n);
        if (m >= (n - 1) * LIMIT) recoverAdditive(1, n, m);
        else if (m > 0) {
            discover(m);
            reconstruct(makeRises(), m);
        }
        cout << '!';
        for (int i = 1; i <= n; ++i) cout << ' ' << answer[i];
        cout << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (cin >> n && n >= 1 && n <= 2000) Solver(n).run();
}

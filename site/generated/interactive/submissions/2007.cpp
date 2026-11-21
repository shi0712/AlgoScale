#include <bits/stdc++.h>
using namespace std;

class Planner {
    // At each depth, store the largest flexible count for each forced count.
    vector<vector<int>> limits;

    static constexpr long long flexibleWeight = 1618033989;
    static constexpr long long forcedWeight = 1000000000;

    static vector<int> invert(const vector<int>& maxForced) {
        vector<int> maxFlexible(maxForced[0] + 1);
        int flexible = static_cast<int>(maxForced.size()) - 1;
        for (int forced = 0; forced < static_cast<int>(maxFlexible.size()); ++forced) {
            while (maxForced[flexible] < forced) {
                --flexible;
            }
            maxFlexible[forced] = flexible;
        }
        return maxFlexible;
    }

    int limit(int depth, int forced) const {
        const auto& row = limits[depth];
        return forced < static_cast<int>(row.size()) ? row[forced] : -1;
    }

    // Reply 0 gives (nextFlexible, flexible - split); reply 1 gives
    // (flexible + forced - nextFlexible, split). Both must fit their budgets.
    bool splitFits(int flexible, int forced, int quick, int slow) const {
        int first = max(0, flexible - static_cast<int>(limits[quick].size()) + 1);
        int last = min(flexible, static_cast<int>(limits[slow].size()) - 1);
        for (int split = first; split <= last; ++split) {
            int low = max(split, flexible + forced - limit(slow, split));
            int high = min(split + forced, limit(quick, flexible - split));
            if (low <= high) {
                return true;
            }
        }
        return false;
    }

public:
    explicit Planner(int n) : limits{{2, 1, 0}} {
        while (limits.back()[0] < n) {
            const auto& previous = limits.back();
            int previousSize = static_cast<int>(previous.size());
            long long peak = 0;
            for (int d = 0; d < previousSize; ++d) {
                peak = max(peak, flexibleWeight * previous[d] + forcedWeight * d);
            }

            int mostFlexible = 0;
            while (mostFlexible < n) {
                int next = mostFlexible + 1;
                int a = next / 2;
                int b = next - a;
                if (b >= previousSize || previous[a] < b) {
                    break;
                }
                mostFlexible = next;
            }

            vector<int> maxForced(mostFlexible + 1);
            int first = 0;
            for (int f = 0; f <= mostFlexible; ++f) {
                // For a <= b, (b,a) is harder than (a,b): a flexible
                // candidate can always be replaced by a forced candidate.
                // Thus a split a+b=f is valid exactly when previous[a] >= b.
                // The same replacement property makes previous[a]+a
                // nondecreasing, so the first valid split only moves right.
                while (previous[first] + first < f) {
                    ++first;
                }
                int middle = f / 2;
                int best = previous[middle] + previous[f - middle];

                // An integer linear bound lets us stop at the first split
                // that reaches the best possible combined flexible count.
                int upper = static_cast<int>(min<long long>(
                    n, (2 * peak - forcedWeight * f) / flexibleWeight));
                for (int a = middle - 1; a >= first && best < upper; --a) {
                    best = max(best, previous[a] + previous[f - a]);
                }
                maxForced[f] = min(n, best) - f;
            }
            limits.push_back(invert(maxForced));
        }
    }

    pair<int, int> choose(int flexible, int forced) const {
        int low = 0;
        int high = static_cast<int>(limits.size()) - 1;
        while (low < high) {
            int depth = (low + high) / 2;
            if (flexible <= limit(depth, forced)) {
                high = depth;
            } else {
                low = depth + 1;
            }
        }
        int slow = low - 1;

        // Keep the optimal worst-case budget and make the faster reply
        // finish in as few queries as possible.
        low = 0;
        high = slow;
        while (low < high) {
            int depth = (low + high) / 2;
            if (splitFits(flexible, forced, depth, slow)) {
                high = depth;
            } else {
                low = depth + 1;
            }
        }
        int quick = low;

        int first = max(0, flexible - static_cast<int>(limits[quick].size()) + 1);
        int last = min(flexible, static_cast<int>(limits[slow].size()) - 1);
        long long bestDifference = LLONG_MAX;
        pair<int, int> best{-1, -1};
        for (int a = first; a <= last; ++a) {
            int minimum = max(a, flexible + forced - limit(slow, a));
            int maximum = min(a + forced, limit(quick, flexible - a));
            if (minimum > maximum) {
                continue;
            }
            long long target = flexibleWeight * (flexible + forced)
                             + forcedWeight * (2 * a - flexible);
            int next = clamp(static_cast<int>((target + flexibleWeight)
                                             / (2 * flexibleWeight)),
                             minimum, maximum);
            long long difference = abs(2 * flexibleWeight * next - target);
            if (difference < bestDifference) {
                bestDifference = difference;
                best = {a, next - a};
            }
        }
        return best;
    }
};

struct Candidate {
    int roll;
    bool lastLie;
    bool forced;
};

using Counts = array<int, 2>;

struct CutSegment {
    Counts start;
    Counts low;
    Counts high;
    int firstCut;
    int axis;
    int step;

    int cutAt(const Counts& point) const {
        return firstCut + step * (point[axis] - start[axis]);
    }
};

class Layout {
    vector<CutSegment> segments;
    Counts signedTotal{};
    Counts totals{};

public:
    explicit Layout(const vector<Candidate>& candidates) {
        int size = static_cast<int>(candidates.size());
        for (int first = 0; first < size;) {
            int axis = candidates[first].forced;
            bool lastLie = candidates[first].lastLie;
            int last = first + 1;
            while (last < size && candidates[last].forced == axis
                               && candidates[last].lastLie == lastLie) {
                ++last;
            }

            int step = lastLie ? -1 : 1;
            Counts start = signedTotal;
            signedTotal[axis] += step * (last - first);
            totals[axis] += last - first;
            CutSegment segment{start, start, start, first, axis, step};
            segment.low[axis] = min(start[axis], signedTotal[axis]);
            segment.high[axis] = max(start[axis], signedTotal[axis]);
            segments.push_back(segment);
            first = last;
        }
    }

    Counts counts() const {
        return totals;
    }

    pair<int, int> interval(Counts preferredZero, const vector<Candidate>& candidates,
                            int n) const {
        Counts delta;
        for (int axis = 0; axis < 2; ++axis) {
            int outsideZero = (totals[axis] - signedTotal[axis]) / 2;
            delta[axis] = preferredZero[axis] - outsideZero;
        }

        // The last two lie bits are each a circular arc. Their endpoints
        // give at most four circular blocks of (lastLie, forced).
        // For four distinct states, each sign of one axis borders both
        // signs of the other. For a repeated state, the order is A B A C:
        // A occupies one axis and B, C have opposite signs on the other.
        // A partial B or C can border either A block. To take more A than
        // one block holds, take one of B or C whole and offset it with the
        // other while taking both A blocks as needed. Thus every pair of
        // preferred-reply counts can be made by a circular arc.
        bool foundWholeClass = false;
        for (int complement = 0; complement < 2; ++complement) {
            for (size_t i = 0; i < segments.size(); ++i) {
                const auto& left = segments[i];
                for (size_t j = i; j < segments.size(); ++j) {
                    const auto& right = segments[j];
                    Counts from, to;
                    bool intersects = true;
                    for (int axis = 0; axis < 2; ++axis) {
                        int low = max(left.low[axis], right.low[axis] - delta[axis]);
                        int high = min(left.high[axis], right.high[axis] - delta[axis]);
                        intersects &= low <= high;
                        // If both cuts can slide, maximize the interval length.
                        from[axis] = left.axis == axis && right.axis == axis
                                      && right.step > left.step ? high : low;
                        to[axis] = from[axis] + delta[axis];
                    }
                    if (!intersects) {
                        continue;
                    }
                    int l = left.cutAt(from);
                    int r = right.cutAt(to);
                    if (l < r) {
                        int first = candidates[l].roll;
                        int last = candidates[r - 1].roll;
                        if (first != 1 || last != n) {
                            return {first, last};
                        }
                        foundWholeClass = true;
                    }
                }
            }
            // A wrapping arc is represented by its complement. This only
            // swaps the two reply branches.
            for (int axis = 0; axis < 2; ++axis) {
                delta[axis] = signedTotal[axis] - delta[axis];
            }
        }
        if (foundWholeClass) {
            // An eliminated roll number gives an empty candidate interval,
            // equivalent to the whole class with the reply labels swapped.
            for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
                if (candidates[i].roll != i + 1) {
                    return {i + 1, i + 1};
                }
            }
            return {1, n};
        }
        return {-1, -1};
    }
};

static int ask(int left, int right) {
    cout << "? " << left << ' ' << right << endl;
    int hands;
    int length = right - left + 1;
    if (!(cin >> hands) || hands < length - 1 || hands > length) {
        exit(0);
    }
    return hands == length - 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 3 || n > 100000) {
        return 0;
    }

    // The first reply establishes a last lie bit for every candidate.
    // Its complement serves as a previous bit with a run of length one.
    int firstReply = ask(1, 1);
    vector<Candidate> candidates;
    candidates.reserve(n);
    for (int roll = 1; roll <= n; ++roll) {
        candidates.push_back({roll, (roll == 1) != firstReply, false});
    }

    Planner planner(n);
    while (candidates.size() > 2) {
        Layout layout(candidates);
        auto [flexible, forced] = layout.counts();
        auto [preferFlexible, preferForced] = planner.choose(flexible, forced);
        auto [left, right] = layout.interval({preferFlexible, preferForced}, candidates, n);
        if (left < 1) {
            return 0;
        }

        int reply = ask(left, right);
        size_t alive = 0;
        for (Candidate candidate : candidates) {
            bool inside = left <= candidate.roll && candidate.roll <= right;
            bool lie = inside != reply;
            bool same = lie == candidate.lastLie;
            if (!(same && candidate.forced)) {
                candidates[alive++] = {candidate.roll, lie, same};
            }
        }
        candidates.resize(alive);
        if (candidates.empty()) {
            return 0;
        }
    }

    for (const auto& candidate : candidates) {
        cout << "! " << candidate.roll << endl;
        int correct;
        if (!(cin >> correct) || (correct != 0 && correct != 1)) {
            return 0;
        }
        if (correct == 1) {
            break;
        }
    }
    cout << '#' << endl;
    return 0;
}

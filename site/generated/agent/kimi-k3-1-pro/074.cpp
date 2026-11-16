#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Interval {
    int family;
    int64 left, right;
};

struct Solver {
    int64 n, radius, first;
    vector<Interval> intervals;
    vector<pair<int64, int64>> explicitCandidates;
    bool explicitMode = false;

    int64 position(int family, int64 t, bool second) const {
        static const int slope[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        int64 value;
        if (!second) {
            value = slope[family][0] == 1 ? t : -t;
        } else {
            value = slope[family][1] == 1 ? first - t : -(first - t);
        }
        value %= n;
        if (value < 0) value += n;
        return value;
    }

    pair<int64, int64> getPair(const Interval &interval, int64 t) const {
        return {position(interval.family, t, false), position(interval.family, t, true)};
    }

    int64 distance(int64 x, int64 y) const {
        int64 d = x - y;
        if (d < 0) d = -d;
        return min(d, n - d);
    }

    int64 response(int64 x, const pair<int64, int64> &p) const {
        return distance(x, p.first) + distance(x, p.second);
    }

    int64 totalCount() const {
        int64 result = 0;
        for (const auto &interval : intervals) {
            result += interval.right - interval.left + 1;
        }
        return result;
    }

    void initialize() {
        for (int family = 0; family < 4; ++family) {
            int64 middle = -1;
            if ((family == 0 || family == 3) && first % 2 == 0) {
                middle = first / 2;
            }
            if (middle < 0) {
                intervals.push_back({family, 0, first});
            } else {
                if (middle > 0) intervals.push_back({family, 0, middle - 1});
                if (middle < first) intervals.push_back({family, middle + 1, first});
            }
        }
    }

    int64 solveParameter(int family, bool second, int64 target) const {
        static const int slope[4][2] = {{1, 1}, {1, -1}, {-1, 1}, {-1, -1}};
        int s = second ? -slope[family][1] : slope[family][0];
        int64 base = second ? (s == 1 ? -first : first) : 0;
        int64 t = (s == 1 ? target - base : base - target) % n;
        if (t < 0) t += n;
        return t;
    }

    vector<Interval> filterInterval(const Interval &interval, int64 x, int64 wanted) const {
        vector<int64> cuts = {interval.left, interval.right};
        int64 targets[3] = {x, (x + radius) % n, (x + radius + 1) % n};
        for (bool second : {false, true}) {
            for (int64 target : targets) {
                int64 t = solveParameter(interval.family, second, target);
                if (interval.left <= t && t <= interval.right) cuts.push_back(t);
            }
        }
        sort(cuts.begin(), cuts.end());
        cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());

        vector<Interval> result;
        auto add = [&](int64 left, int64 right) {
            if (left > right) return;
            if (!result.empty() && result.back().right + 1 == left) {
                result.back().right = right;
            } else {
                result.push_back({interval.family, left, right});
            }
        };

        for (int i = 0; i + 1 < (int)cuts.size(); ++i) {
            int64 left = cuts[i], right = cuts[i + 1];
            int64 atLeft = response(x, getPair(interval, left));
            int64 atRight = response(x, getPair(interval, right));
            if (left == right) {
                if (atLeft == wanted) add(left, left);
                continue;
            }
            if (atLeft == atRight) {
                if (atLeft == wanted) add(left, right);
                continue;
            }
            if (atLeft == wanted) add(left, left);
            if (atRight == wanted) add(right, right);
            int64 low = min(atLeft, atRight), high = max(atLeft, atRight);
            if (low < wanted && wanted < high) {
                int64 numerator = (wanted - atLeft) * (right - left);
                int64 denominator = atRight - atLeft;
                if (numerator % denominator == 0) {
                    int64 t = left + numerator / denominator;
                    if (left < t && t < right) add(t, t);
                }
            }
        }
        return result;
    }

    int64 queryScore(int64 x) const {
        unordered_map<int64, int64> buckets;
        int64 best = 0;
        for (const auto &interval : intervals) {
            vector<int64> cuts = {interval.left, interval.right};
            int64 targets[3] = {x, (x + radius) % n, (x + radius + 1) % n};
            for (bool second : {false, true}) {
                for (int64 target : targets) {
                    int64 t = solveParameter(interval.family, second, target);
                    if (interval.left <= t && t <= interval.right) cuts.push_back(t);
                }
            }
            sort(cuts.begin(), cuts.end());
            cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());
            for (int i = 0; i + 1 < (int)cuts.size(); ++i) {
                int64 left = cuts[i], right = cuts[i + 1];
                int64 atLeft = response(x, getPair(interval, left));
                int64 atRight = response(x, getPair(interval, right));
                int64 weight = atLeft == atRight ? right - left + 1 : 1;
                int64 value = buckets[atLeft] += weight;
                best = max(best, value);
            }
        }
        return best;
    }

    int64 chooseIntervalQuery() const {
        vector<int64> xs;
        for (const auto &interval : intervals) {
            int64 length = interval.right - interval.left;
            for (int k = 0; k <= 16; ++k) {
                int64 t = interval.left + length * k / 16;
                xs.push_back(position(interval.family, t, false));
                xs.push_back(position(interval.family, t, true));
            }
        }
        sort(xs.begin(), xs.end());
        xs.erase(unique(xs.begin(), xs.end()), xs.end());
        int64 bestX = xs[0], bestScore = LLONG_MAX;
        for (int64 x : xs) {
            int64 score = queryScore(x);
            if (score < bestScore) {
                bestScore = score;
                bestX = x;
            }
        }
        return bestX;
    }

    void makeExplicit() {
        set<pair<int64, int64>> uniquePairs;
        for (const auto &interval : intervals) {
            for (int64 t = interval.left; t <= interval.right; ++t) {
                auto p = getPair(interval, t);
                if (p.first > p.second) swap(p.first, p.second);
                uniquePairs.insert(p);
            }
        }
        explicitCandidates.assign(uniquePairs.begin(), uniquePairs.end());
        intervals.clear();
        explicitMode = true;
    }

    int64 chooseExplicitQuery() const {
        vector<int64> xs;
        for (auto p : explicitCandidates) {
            xs.push_back(p.first);
            xs.push_back(p.second);
        }
        sort(xs.begin(), xs.end());
        xs.erase(unique(xs.begin(), xs.end()), xs.end());
        int64 bestX = xs[0], bestScore = LLONG_MAX;
        for (int64 x : xs) {
            unordered_map<int64, int64> buckets;
            int64 score = 0;
            for (auto p : explicitCandidates) {
                score = max(score, ++buckets[response(x, p)]);
            }
            if (score < bestScore) {
                bestScore = score;
                bestX = x;
            }
        }
        return bestX;
    }

    void solve() {
        cout << "? 1" << endl;
        int64 value;
        if (!(cin >> value)) exit(0);
        first = value;
        initialize();

        int queries = 1;
        while (true) {
            if (!explicitMode && totalCount() <= 256) makeExplicit();
            if (explicitMode && explicitCandidates.size() == 1) break;
            if (!explicitMode && totalCount() == 1) {
                makeExplicit();
                break;
            }

            int64 x = explicitMode ? chooseExplicitQuery() : chooseIntervalQuery();
            cout << "? " << x + 1 << endl;
            if (!(cin >> value)) exit(0);
            ++queries;

            if (explicitMode) {
                vector<pair<int64, int64>> remaining;
                for (auto p : explicitCandidates) {
                    if (response(x, p) == value) remaining.push_back(p);
                }
                explicitCandidates.swap(remaining);
            } else {
                vector<Interval> remaining;
                for (const auto &interval : intervals) {
                    auto pieces = filterInterval(interval, x, value);
                    remaining.insert(remaining.end(), pieces.begin(), pieces.end());
                }
                intervals.swap(remaining);
            }
        }

        auto answer = explicitCandidates[0];
        cout << "! " << answer.first + 1 << ' ' << answer.second + 1 << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    if (!(cin >> tests)) return 0;
    while (tests--) {
        Solver solver;
        if (!(cin >> solver.n)) return 0;
        solver.radius = solver.n / 2;
        solver.solve();
    }
    return 0;
}

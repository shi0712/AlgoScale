#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Interval {
    int64 l, r;
};

// The difference between two score histograms. The coefficient of the
// largest nonzero length determines which histogram is better.
class Difference {
    map<int64, int> coefficients;
    int polarity = 1;

public:
    void clear() {
        coefficients.clear();
        polarity = 1;
    }

    void negate() { polarity = -polarity; }

    void add(int64 length, int delta) {
        auto [it, inserted] = coefficients.try_emplace(length, 0);
        it->second += polarity * delta;
        if (it->second == 0) coefficients.erase(it);
    }

    // Compare D + e[hi] - e[lo] with zero without changing D.
    int compare(int64 hi, int64 lo) const {
        auto it = coefficients.rbegin();
        bool haveHi = hi != lo, haveLo = hi != lo;
        while (it != coefficients.rend() || haveHi || haveLo) {
            int64 length = 0;
            if (it != coefficients.rend()) length = it->first;
            if (haveHi) length = max(length, hi);
            if (haveLo) length = max(length, lo);
            int value = 0;
            if (it != coefficients.rend() && it->first == length) {
                value += polarity * it->second;
                ++it;
            }
            if (haveHi && hi == length) {
                ++value;
                haveHi = false;
            }
            if (haveLo && lo == length) {
                --value;
                haveLo = false;
            }
            if (value != 0) return value > 0 ? 1 : -1;
        }
        return 0;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    cin >> tests;
    while (tests--) {
        int n, m;
        cin >> n >> m;
        vector<Interval> intervals(n + m);
        for (auto &interval : intervals) cin >> interval.l >> interval.r;
        sort(intervals.begin(), intervals.end(), [](const Interval &a, const Interval &b) {
            if (a.l != b.l) return a.l < b.l;
            return a.r > b.r;
        });

        // A contained interval can always be replaced by cells of its
        // container, merging its runs and increasing (or preserving) score.
        int k = 0;
        int64 rightmost = 0;
        for (int i = 0; i < n + m; ++i) {
            if (intervals[i].r > rightmost) {
                rightmost = intervals[i].r;
                intervals[k++] = intervals[i];
            }
        }
        intervals.resize(k);

        // edge[i] is the overlap of intervals i-1 and i. Nonconsecutive
        // surviving intervals cannot overlap. Each overlap goes to one end.
        vector<int64> edge(k + 1, 0), base(k);
        for (int i = 1; i < k; ++i) {
            edge[i] = max<int64>(0, intervals[i - 1].r - intervals[i].l + 1);
        }
        for (int i = 0; i < k; ++i) {
            base[i] = intervals[i].r - intervals[i].l + 1 - edge[i] - edge[i + 1];
        }

        // State t says whether the edge on the right goes to the right
        // interval. Keep D = score(state 1) - score(state 0).
        // Only D is needed for comparisons; parent choices recover the runs.
        Difference difference;
        vector<array<unsigned char, 2>> parent(k);
        for (int i = 0; i < k; ++i) {
            int64 chosenLength[2];
            for (int t = 0; t < 2; ++t) {
                int64 low = base[i] + (1 - t) * edge[i + 1];
                int64 high = low + edge[i];
                int s = difference.compare(high, low) > 0;
                parent[i][t] = s;
                chosenLength[t] = low + s * edge[i];
            }

            int s0 = parent[i][0], s1 = parent[i][1];
            if (s0 == s1) difference.clear();
            else if (s1 < s0) difference.negate();
            difference.add(chosenLength[1], 1);
            difference.add(chosenLength[0], -1);
        }

        vector<int64> answer;
        answer.reserve(k);
        int t = 0;
        for (int i = k - 1; i >= 0; --i) {
            int s = parent[i][t];
            answer.push_back(base[i] + s * edge[i] + (1 - t) * edge[i + 1]);
            t = s;
        }
        sort(answer.rbegin(), answer.rend());
        cout << answer.size() << '\n';
        for (int i = 0; i < k; ++i) {
            if (i) cout << ' ';
            cout << answer[i];
        }
        cout << '\n';
    }
}

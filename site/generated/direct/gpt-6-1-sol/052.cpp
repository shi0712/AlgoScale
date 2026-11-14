#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

// Kept finite deliberately: even after adding every possible bonus,
// a state originating from NEG remains negative and cannot affect the answer.
static constexpr int64 NEG = -(1LL << 60);

struct Group {
    int power;
    vector<array<int, 10>> items;
    vector<int64> prefix;
};

/*
For one residue modulo power, compute

    result[j] = max_h(old[h] + prefix[j-h]),

where 0 <= j-h <= number of items.

Since prefix is concave, the maximizing h is nondecreasing in j.
*/
struct ConcaveConvolution {
    const vector<int64>& oldDP;
    vector<int64>& newDP;
    const vector<int64>& prefix;
    int power;
    int residue;
    int count;

    void compute(int left, int right, int optLeft, int optRight) {
        if (left > right) return;

        int mid = (left + right) / 2;
        int from = max(optLeft, mid - count);
        int to = min(optRight, mid);

        int bestIndex = from;
        int64 bestValue =
            oldDP[residue + from * power] + prefix[mid - from];

        for (int h = from + 1; h <= to; ++h) {
            int64 candidate =
                oldDP[residue + h * power] + prefix[mid - h];

            if (candidate > bestValue) {
                bestValue = candidate;
                bestIndex = h;
            }
        }

        newDP[residue + mid * power] = bestValue;

        compute(left, mid - 1, optLeft, bestIndex);
        compute(mid + 1, right, bestIndex, optRight);
    }
};

class Solver {
    int k;
    vector<Group> groups;
    int64 answer = 0;

    void addGroup(vector<int64>& dp, const Group& group) {
        vector<int64> next(k + 1);

        int p = group.power;
        int m = static_cast<int>(group.items.size());

        for (int residue = 0; residue < p && residue <= k; ++residue) {
            int length = (k - residue) / p;

            ConcaveConvolution convolution{
                dp, next, group.prefix, p, residue, m
            };
            convolution.compute(0, length, 0, length);
        }

        dp.swap(next);
    }

    // dp contains the exact-weight knapsack of every other group.
    void evaluateGroup(const Group& group, const vector<int64>& dp) {
        int p = group.power;
        int m = static_cast<int>(group.items.size());

        // suffix[t][r-1] = maximum w_r among items with index >= t.
        vector<array<int, 10>> suffix(m + 1);
        suffix[m].fill(0);

        for (int t = m - 1; t >= 0; --t) {
            suffix[t] = suffix[t + 1];
            for (int r = 1; r < p; ++r) {
                suffix[t][r - 1] =
                    max(suffix[t][r - 1], group.items[t][r - 1]);
            }
        }

        // Among indices j < t, maximum (w_{j,r} - w_{j,p}).
        array<int, 10> bestDelta;
        bestDelta.fill(-1000000000);

        int limit = min(m, k / p);

        for (int t = 0; t <= limit; ++t) {
            int remaining = k - t * p;
            int64 fullValue = group.prefix[t];

            // No partially upgraded item.
            answer = max(answer, fullValue + dp[remaining]);

            if (t < m) {
                int maxPartial = min(p - 1, remaining);

                for (int r = 1; r <= maxPartial; ++r) {
                    // The partial item is outside the first t items.
                    int partialGain = suffix[t][r - 1];

                    // Or it is among the first t, so item t replaces it
                    // as a fully upgraded item.
                    if (t > 0) {
                        partialGain = max(
                            partialGain,
                            group.items[t][p - 1] + bestDelta[r - 1]
                        );
                    }

                    answer = max(
                        answer,
                        fullValue + partialGain + dp[remaining - r]
                    );
                }

                for (int r = 1; r < p; ++r) {
                    bestDelta[r - 1] = max(
                        bestDelta[r - 1],
                        group.items[t][r - 1] - group.items[t][p - 1]
                    );
                }
            }
        }
    }

    /*
    At this node, dp contains all groups outside [left, right].
    Build each child's DP by adding the opposite half.
    */
    void solveExclusions(int left, int right, const vector<int64>& dp) {
        if (left == right) {
            evaluateGroup(groups[left], dp);
            return;
        }

        int mid = (left + right) / 2;

        {
            vector<int64> next = dp;
            for (int i = mid + 1; i <= right; ++i) {
                addGroup(next, groups[i]);
            }
            solveExclusions(left, mid, next);
        }

        {
            vector<int64> next = dp;
            for (int i = left; i <= mid; ++i) {
                addGroup(next, groups[i]);
            }
            solveExclusions(mid + 1, right, next);
        }
    }

public:
    Solver(int allowance, vector<Group> inputGroups)
        : k(allowance), groups(move(inputGroups)) {}

    int64 run() {
        vector<int64> dp(k + 1, NEG);
        dp[0] = 0;

        solveExclusions(0, static_cast<int>(groups.size()) - 1, dp);
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    array<vector<array<int, 10>>, 11> byPower;
    int totalPower = 0;
    int64 totalFullBonus = 0;

    for (int i = 0; i < n; ++i) {
        int p;
        cin >> p;

        array<int, 10> values{};
        for (int j = 0; j < p; ++j) {
            cin >> values[j];
        }

        byPower[p].push_back(values);
        totalPower += p;
        totalFullBonus += values[p - 1];
    }

    if (k == 0) {
        cout << 0 << '\n';
        return 0;
    }

    if (totalPower <= k) {
        cout << totalFullBonus << '\n';
        return 0;
    }

    vector<Group> groups;

    for (int p = 1; p <= 10; ++p) {
        if (byPower[p].empty()) continue;

        Group group;
        group.power = p;
        group.items = move(byPower[p]);

        sort(
            group.items.begin(),
            group.items.end(),
            [p](const auto& a, const auto& b) {
                return a[p - 1] > b[p - 1];
            }
        );

        int m = static_cast<int>(group.items.size());
        group.prefix.assign(m + 1, 0);

        for (int i = 0; i < m; ++i) {
            group.prefix[i + 1] =
                group.prefix[i] + group.items[i][p - 1];
        }

        groups.push_back(move(group));
    }

    Solver solver(k, move(groups));
    cout << solver.run() << '\n';

    return 0;
}

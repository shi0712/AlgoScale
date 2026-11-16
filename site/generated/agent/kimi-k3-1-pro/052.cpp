#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

using int64 = long long;

struct Item {
    int p;
    array<int, 10> w;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<Item> items(n);
    array<vector<int>, 11> byPower;
    int totalPower = 0;
    int64 totalValue = 0;

    for (int i = 0; i < n; ++i) {
        cin >> items[i].p;
        for (int j = 0; j < items[i].p; ++j) {
            cin >> items[i].w[j];
        }
        byPower[items[i].p].push_back(i);
        totalPower += items[i].p;
        totalValue += items[i].w[items[i].p - 1];
    }

    if (k == 0) {
        cout << 0 << '\n';
        return 0;
    }
    if (totalPower <= k) {
        cout << totalValue << '\n';
        return 0;
    }

    for (int p = 1; p <= 10; ++p) {
        sort(byPower[p].begin(), byPower[p].end(), [&](int a, int b) {
            const Item &x = items[a];
            const Item &y = items[b];
            int vx = x.w[x.p - 1];
            int vy = y.w[y.p - 1];
            if (vx != vy) return vx > vy;
            return a < b;
        });
    }

    vector<int> rank(n);
    for (int p = 1; p <= 10; ++p) {
        for (int r = 0; r < static_cast<int>(byPower[p].size()); ++r) {
            rank[byPower[p][r]] = r;
        }
    }

    constexpr int RANGE = 10;
    constexpr int64 NEG = numeric_limits<int64>::min() / 4;

    array<int, 11> greedyCount{};
    int greedyWeight = 0;
    while (true) {
        int bestPower = 0;
        for (int p = 1; p <= 10; ++p) {
            int r = greedyCount[p];
            if (r == static_cast<int>(byPower[p].size()) || greedyWeight + p > k) {
                continue;
            }
            if (bestPower == 0) {
                bestPower = p;
                continue;
            }
            int id = byPower[p][r];
            int bestId = byPower[bestPower][greedyCount[bestPower]];
            int value = items[id].w[p - 1];
            int bestValue = items[bestId].w[bestPower - 1];
            if (value * bestPower > bestValue * p) {
                bestPower = p;
            }
        }
        if (bestPower == 0) break;
        ++greedyCount[bestPower];
        greedyWeight += bestPower;
    }

    array<int, 11> baseCount{}, upperCount{};
    int baseWeight = 0;
    int64 baseValue = 0;
    for (int p = 1; p <= 10; ++p) {
        baseCount[p] = max(0, greedyCount[p] - RANGE);
        upperCount[p] = min(static_cast<int>(byPower[p].size()), greedyCount[p] + RANGE);
        baseWeight += p * baseCount[p];
        for (int r = 0; r < baseCount[p]; ++r) {
            int id = byPower[p][r];
            baseValue += items[id].w[p - 1];
        }
    }

    int capacity = k - baseWeight + 10;
    vector<int64> dp(capacity + 1, NEG);
    dp[0] = 0;
    for (int p = 1; p <= 10; ++p) {
        for (int r = baseCount[p]; r < upperCount[p]; ++r) {
            int id = byPower[p][r];
            int value = items[id].w[p - 1];
            for (int s = capacity; s >= p; --s) {
                if (dp[s - p] != NEG) {
                    dp[s] = max(dp[s], dp[s - p] + value);
                }
            }
        }
    }

    int64 answer = baseValue;
    for (int s = 0; s <= k - baseWeight; ++s) {
        if (dp[s] != NEG) {
            answer = max(answer, baseValue + dp[s]);
        }
    }

    vector<int64> excludedDp;
    for (int i = 0; i < n; ++i) {
        int p = items[i].p;
        int r = rank[i];
        const vector<int64> *currentDp = &dp;

        if (r >= baseCount[p] && r < upperCount[p]) {
            excludedDp.assign(capacity + 1, NEG);
            excludedDp[0] = 0;
            for (int q = 1; q <= 10; ++q) {
                for (int t = baseCount[q]; t < upperCount[q]; ++t) {
                    if (q == p && t == r) continue;
                    int id = byPower[q][t];
                    int value = items[id].w[q - 1];
                    for (int s = capacity; s >= q; --s) {
                        if (excludedDp[s - q] != NEG) {
                            excludedDp[s] = max(excludedDp[s], excludedDp[s - q] + value);
                        }
                    }
                }
            }
            currentDp = &excludedDp;
        }

        int adjustedBaseWeight = baseWeight;
        int64 adjustedBaseValue = baseValue;
        if (r < baseCount[p]) {
            adjustedBaseWeight -= p;
            adjustedBaseValue -= items[i].w[p - 1];
        }

        int firstSum = max(0, k - p + 1);
        for (int s = firstSum; s < k; ++s) {
            int coreWeight = s - adjustedBaseWeight;
            if (coreWeight < 0 || coreWeight > capacity) continue;
            int64 value = (*currentDp)[coreWeight];
            if (value == NEG) continue;
            answer = max(answer, adjustedBaseValue + value + items[i].w[k - s - 1]);
        }
    }

    cout << answer << '\n';
    return 0;
}

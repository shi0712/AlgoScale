#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Card {
    int64 value;
    int points;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<Card> cards(n);
    for (auto &c : cards) {
        cin >> c.value >> c.points;
    }

    constexpr int MAX_POINTS = 13;
    constexpr int DIFF_LIMIT = 4 * MAX_POINTS * MAX_POINTS; // 676
    constexpr int WIDTH = 2 * DIFF_LIMIT + 1;
    constexpr int MAX_USED_DOUBLINGS = MAX_POINTS;

    const int K = min(k, MAX_USED_DOUBLINGS);
    const int64 NEG = numeric_limits<int64>::lowest() / 4;

    sort(cards.begin(), cards.end(),
         [](const Card &a, const Card &b) {
             return a.points < b.points;
         });

    vector<int64> dp((K + 1) * WIDTH, NEG);
    vector<int64> next_dp;
    dp[DIFF_LIMIT] = 0; // zero doublings, zero difference

    auto relax = [](int64 &destination, int64 value) {
        if (value > destination) {
            destination = value;
        }
    };

    for (int i = 0; i < n; ++i) {
        next_dp = dp; // Not using the current card.

        const int reachable_doublings = min(K, i);
        const int reachable_difference = min(DIFF_LIMIT, 26 * i);

        for (int used = 0; used <= reachable_doublings; ++used) {
            const int old_base = used * WIDTH;

            for (int difference = -reachable_difference;
                 difference <= reachable_difference;
                 ++difference) {
                const int64 current = dp[old_base + DIFF_LIMIT + difference];
                if (current == NEG) {
                    continue;
                }

                const int64 value_after_use = current + cards[i].value;

                // Use the card without doubling it.
                {
                    const int weight = cards[i].points;

                    if (difference + weight <= DIFF_LIMIT) {
                        relax(next_dp[old_base + DIFF_LIMIT + difference + weight],
                              value_after_use);
                    }
                    if (difference - weight >= -DIFF_LIMIT) {
                        relax(next_dp[old_base + DIFF_LIMIT + difference - weight],
                              value_after_use);
                    }
                }

                // Use the card and double its point number.
                if (used < K) {
                    const int weight = 2 * cards[i].points;
                    const int new_base = (used + 1) * WIDTH;

                    if (difference + weight <= DIFF_LIMIT) {
                        relax(next_dp[new_base + DIFF_LIMIT + difference + weight],
                              value_after_use);
                    }
                    if (difference - weight >= -DIFF_LIMIT) {
                        relax(next_dp[new_base + DIFF_LIMIT + difference - weight],
                              value_after_use);
                    }
                }
            }
        }

        dp.swap(next_dp);
    }

    int64 answer = 0;
    for (int used = 0; used <= K; ++used) {
        answer = max(answer, dp[used * WIDTH + DIFF_LIMIT]);
    }

    cout << answer << '\n';
    return 0;
}

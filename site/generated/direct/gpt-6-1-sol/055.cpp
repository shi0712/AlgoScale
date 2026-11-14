#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    constexpr int NEG = -1'000'000'000;

    while (T--) {
        string s;
        int K;
        cin >> s >> K;

        const int n = static_cast<int>(s.size());

        // dp[position % 6][state][penalty], for penalties 0,...,4.
        //
        // state 0: the last character was not absorbed.
        // state 1: the last operation does not require an operation on its right.
        // state 2: the last operation requires the next operation to happen first.
        int dp[6][3][5];
        fill(&dp[0][0][0], &dp[0][0][0] + 6 * 3 * 5, NEG);

        for (int p = 0; p < 5; ++p) {
            dp[0][0][p] = 0;
        }

        for (int i = 0; i < n; ++i) {
            const int current = i % 6;
            const int next = (i + 1) % 6;

            int* destination[3];
            int candidateCount = 0;
            bool needsLeft = false;

            if (i + 2 < n) {
                const char a = s[i];
                const char b = s[i + 1];
                const char c = s[i + 2];

                char target = 0;
                if (a == b || a == c) {
                    target = a;
                } else if (b == c) {
                    target = b;
                }

                if (target != 0) {
                    needsLeft = (i > 0 && s[i - 1] == target);

                    int maxLength = 3;
                    while (maxLength < 5 &&
                           i + maxLength < n &&
                           s[i + maxLength] == target) {
                        ++maxLength;
                    }

                    for (int length = 3; length <= maxLength; ++length) {
                        const bool needsRight =
                            i + length < n && s[i + length] == target;

                        destination[candidateCount++] =
                            dp[(i + length) % 6][needsRight ? 2 : 1];
                    }
                }
            }

            for (int penalty = 0; penalty < 5; ++penalty) {
                const int a = dp[current][0][penalty];
                const int b = dp[current][1][penalty];
                const int c = dp[current][2][penalty];

                const int withoutRightRequirement = max(a, b);

                // Leave s[i] unabsorbed. A pending right requirement
                // cannot be satisfied by an unabsorbed character.
                dp[next][0][penalty] = withoutRightRequirement;

                const int base = needsLeft
                    ? b
                    : max(withoutRightRequirement, c);

                int value = base + 3 - penalty;
                for (int j = 0; j < candidateCount; ++j, ++value) {
                    destination[j][penalty] =
                        max(destination[j][penalty], value);
                }
            }

            fill(&dp[current][0][0],
                 &dp[current][0][0] + 3 * 5, NEG);
        }

        // A penalty of 5 gives score 0: no operation absorbs over 5 letters.
        int answer = 5 * K;
        const int last = n % 6;

        for (int penalty = 0; penalty < 5; ++penalty) {
            const int bestPenalizedScore =
                max(dp[last][0][penalty], dp[last][1][penalty]);

            answer = min(answer, bestPenalizedScore + penalty * K);
        }

        cout << answer << '\n';
    }

    return 0;
}

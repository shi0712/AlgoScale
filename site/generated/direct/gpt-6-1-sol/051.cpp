#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 1'000'000'007;

inline int addmod(int a, int b) {
    int result = a + b;
    if (result >= MOD) result -= MOD;
    return result;
}

struct State {
    int ways[3] = {0, 0, 0};
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, Q;
    cin >> n >> Q;

    string s;
    cin >> s;

    // Adjacent equal fixed letters can never be repaired.
    for (int i = 1; i < n; ++i) {
        if (s[i] != '?' && s[i] == s[i - 1]) {
            while (Q--) {
                int x, y, z;
                cin >> x >> y >> z;
                cout << 0 << '\n';
            }
            return 0;
        }
    }

    const int m = count(s.begin(), s.end(), '?');
    const int stride = m + 1;

    vector<State> dp(static_cast<size_t>(stride) * stride);
    vector<State> nextDp(static_cast<size_t>(stride) * stride);

    int processedQuestions = 0;
    int fixedCount[3] = {0, 0, 0};

    // Initialize the first position.
    if (s[0] == '?') {
        processedQuestions = 1;
        dp[stride].ways[0] = 1; // One new 'a'.
        dp[1].ways[1] = 1;      // One new 'b'.
        dp[0].ways[2] = 1;      // One new 'c'.
    } else {
        int letter = s[0] - 'a';
        ++fixedCount[letter];
        dp[0].ways[letter] = 1;
    }

    for (int pos = 1; pos < n; ++pos) {
        const bool unknown = (s[pos] == '?');
        const int letter = unknown ? -1 : s[pos] - 'a';

        if (unknown) {
            ++processedQuestions;
        } else {
            ++fixedCount[letter];
        }

        const int q = processedQuestions;

        /*
         * All states ever written satisfy a + b <= q.
         * Since q never decreases, clearing this triangle removes every
         * possibly nonzero entry in the reused buffer.
         */
        for (int a = 0; a <= q; ++a) {
            memset(nextDp.data() + static_cast<size_t>(a) * stride,
                   0, static_cast<size_t>(q - a + 1) * sizeof(State));
        }

        /*
         * In a valid prefix of length len, any one letter occurs at most
         * ceil(len / 2) times, including its fixed occurrences.
         */
        const int len = pos + 1;
        const int maxOccurrences = (len + 1) / 2;

        const int upperA = min(q, maxOccurrences - fixedCount[0]);
        const int upperB = min(q, maxOccurrences - fixedCount[1]);
        const int upperC = min(q, maxOccurrences - fixedCount[2]);

        const int firstA = max(0, q - upperB - upperC);
        const int lastA = min(q, upperA);

        for (int a = firstA; a <= lastA; ++a) {
            const int firstB = max(0, q - a - upperC);
            const int lastB = min(upperB, q - a);

            const size_t row = static_cast<size_t>(a) * stride;

            if (unknown) {
                for (int b = firstB; b <= lastB; ++b) {
                    const int c = q - a - b;
                    const size_t index = row + b;
                    State &current = nextDp[index];

                    if (a > 0) {
                        const State &previous = dp[index - stride];
                        current.ways[0] =
                            addmod(previous.ways[1], previous.ways[2]);
                    }

                    if (b > 0) {
                        const State &previous = dp[index - 1];
                        current.ways[1] =
                            addmod(previous.ways[0], previous.ways[2]);
                    }

                    if (c > 0) {
                        const State &previous = dp[index];
                        current.ways[2] =
                            addmod(previous.ways[0], previous.ways[1]);
                    }
                }
            } else {
                const int other1 = (letter + 1) % 3;
                const int other2 = (letter + 2) % 3;

                for (int b = firstB; b <= lastB; ++b) {
                    const size_t index = row + b;
                    nextDp[index].ways[letter] =
                        addmod(dp[index].ways[other1],
                               dp[index].ways[other2]);
                }
            }
        }

        dp.swap(nextDp);
    }

    /*
     * Each valid assignment has exactly m newly assigned letters.
     * Store its count in all three pair-coordinate systems.
     */
    const int tableStride = m + 2;
    const size_t tableSize =
        static_cast<size_t>(tableStride) * tableStride;

    vector<int> suffixAB(tableSize, 0);
    vector<int> suffixAC(tableSize, 0);
    vector<int> suffixBC(tableSize, 0);

    for (int a = 0; a <= m; ++a) {
        for (int b = 0; a + b <= m; ++b) {
            const int c = m - a - b;
            const State &state =
                dp[static_cast<size_t>(a) * stride + b];

            const int ways =
                addmod(addmod(state.ways[0], state.ways[1]),
                       state.ways[2]);

            suffixAB[static_cast<size_t>(a) * tableStride + b] = ways;
            suffixAC[static_cast<size_t>(a) * tableStride + c] = ways;
            suffixBC[static_cast<size_t>(b) * tableStride + c] = ways;
        }
    }

    auto buildSuffix = [&](vector<int> &table) {
        for (int i = m; i >= 0; --i) {
            const size_t row = static_cast<size_t>(i) * tableStride;
            const size_t nextRow = row + tableStride;

            for (int j = m; j >= 0; --j) {
                int value = addmod(table[row + j], table[nextRow + j]);
                value = addmod(value, table[row + j + 1]);
                value -= table[nextRow + j + 1];
                if (value < 0) value += MOD;
                table[row + j] = value;
            }
        }
    };

    buildSuffix(suffixAB);
    buildSuffix(suffixAC);
    buildSuffix(suffixBC);

    const int total = suffixAB[0];

    while (Q--) {
        int x, y, z;
        cin >> x >> y >> z;

        // m + 1 is a zero sentinel index.
        const int A = min(x, m) + 1;
        const int B = min(y, m) + 1;
        const int C = min(z, m) + 1;

        /*
         * Inclusion-exclusion over:
         *   new a > x, new b > y, new c > z.
         *
         * Their triple intersection is empty because x + y + z >= m.
         */
        long long answer = total;

        answer -= suffixAB[static_cast<size_t>(A) * tableStride];
        answer -= suffixAB[B];
        answer -= suffixAC[C];

        answer += suffixAB[static_cast<size_t>(A) * tableStride + B];
        answer += suffixAC[static_cast<size_t>(A) * tableStride + C];
        answer += suffixBC[static_cast<size_t>(B) * tableStride + C];

        answer %= MOD;
        if (answer < 0) answer += MOD;

        cout << answer << '\n';
    }

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

struct Candidate {
    int right, length;
    bool needsLeft, needsRight;
};

struct State {
    int value = -1000000000;
    int minOperations = 0;
    int maxOperations = 0;
};

static void update(State &destination, const State &source) {
    if (source.value > destination.value) {
        destination = source;
    } else if (source.value == destination.value) {
        destination.minOperations = min(destination.minOperations, source.minOperations);
        destination.maxOperations = max(destination.maxOperations, source.maxOperations);
    }
}

static State solveWithPenalty(int n, const vector<vector<Candidate>> &startingAt, int penalty) {
    vector<array<State, 3>> dp(n + 1);
    dp[0][0] = {0, 0, 0};

    for (int left = 0; left < n; ++left) {
        for (int state = 0; state < 3; ++state) {
            const State &current = dp[left][state];
            if (current.value < 0) continue;

            if (state != 2) {
                update(dp[left + 1][0], current);
            }

            for (const Candidate &candidate : startingAt[left]) {
                if (candidate.needsLeft && state == 0) continue;
                if (state == 2 && candidate.needsLeft) continue;

                State next = current;
                next.value += candidate.length - penalty;
                ++next.minOperations;
                ++next.maxOperations;
                int nextState = candidate.needsRight ? 2 : 1;
                update(dp[candidate.right][nextState], next);
            }
        }
    }

    State result;
    for (const State &state : dp[n]) update(result, state);
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int testCases;
    cin >> testCases;
    while (testCases--) {
        string s;
        int k;
        cin >> s >> k;
        int n = static_cast<int>(s.size());

        vector<vector<Candidate>> startingAt(n);
        for (int center = 0; center < n; ++center) {
            char letters[4];
            int letterCount = 0;
            for (int distance = -2; distance <= 2; ++distance) {
                if (distance == 0) continue;
                int position = center + distance;
                if (position < 0 || position >= n) continue;
                bool seen = false;
                for (int j = 0; j < letterCount; ++j) {
                    if (letters[j] == s[position]) seen = true;
                }
                if (!seen) letters[letterCount++] = s[position];
            }

            for (int j = 0; j < letterCount; ++j) {
                char letter = letters[j];
                int availableLeft = 0;
                while (availableLeft < 2 && center - availableLeft - 1 >= 0 &&
                       s[center - availableLeft - 1] == letter) {
                    ++availableLeft;
                }
                int availableRight = 0;
                while (availableRight < 2 && center + availableRight + 1 < n &&
                       s[center + availableRight + 1] == letter) {
                    ++availableRight;
                }

                for (int leftLength = 0; leftLength <= availableLeft; ++leftLength) {
                    for (int rightLength = 0; rightLength <= availableRight; ++rightLength) {
                        if (leftLength + rightLength < 2) continue;
                        int left = center - leftLength;
                        int right = center + rightLength + 1;
                        bool needsLeft = left > 0 && s[left - 1] == letter;
                        bool needsRight = right < n && s[right] == letter;
                        startingAt[left].push_back({right, leftLength + rightLength + 1,
                                                    needsLeft, needsRight});
                    }
                }
            }
        }

        int answer = 0;
        for (int penalty = 0; penalty <= 5; ++penalty) {
            State result = solveWithPenalty(n, startingAt, penalty);
            if ((penalty == 0 && result.minOperations <= k) ||
                (result.minOperations <= k && k <= result.maxOperations)) {
                answer = result.value + penalty * k;
                break;
            }
        }
        cout << answer << '\n';
    }
    return 0;
}

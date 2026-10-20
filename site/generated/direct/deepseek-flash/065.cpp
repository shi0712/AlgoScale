#include <bits/stdc++.h>
using namespace std;

struct Node {
    int score;
    int ops;
};

const int NEG = -1000000000;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        string S;
        int K;
        cin >> S >> K;

        int n = (int)S.size();

        // blocks[i] contains all valid operation blocks starting at position i (1-indexed).
        // Each is {length, dominant_letter}.
        vector<array<pair<int, int>, 3>> blocks(n + 2);
        vector<unsigned char> blockCnt(n + 2, 0);

        for (int i = 1; i <= n; ++i) {
            for (int len = 3; len <= 5; ++len) {
                int j = i + len - 1;
                if (j > n) break;

                int cnt[26] = {};
                for (int p = i - 1; p < j; ++p) {
                    ++cnt[S[p] - 'a'];
                }

                int dominant = -1;
                for (int c = 0; c < 26; ++c) {
                    if (cnt[c] == len - 1) {
                        dominant = c;
                        break;
                    }
                }

                if (dominant != -1) {
                    blocks[i][blockCnt[i]++] = {len, dominant};
                }
            }
        }

        // C[lambda] = maximum number of operations among all optimal solutions
        // for maximizing (stars - lambda * operations).
        vector<int> C(6, 0);

        const int STATES = 27;
        // state 0: previous position is not covered by an adjacent block
        // state d+1: previous position is covered and its block's final letter is d
        vector<Node> dp((n + 2) * STATES);

        for (int lambda = 1; lambda <= 5; ++lambda) {
            fill(dp.begin(), dp.end(), Node{NEG, NEG});

            auto relax = [&](int pos, int state, const Node &val) {
                Node &to = dp[pos * STATES + state];
                if (val.score > to.score ||
                    (val.score == to.score && val.ops > to.ops)) {
                    to = val;
                }
            };

            dp[1 * STATES + 0] = {0, 0};

            for (int i = 1; i <= n; ++i) {
                for (int state = 0; state < STATES; ++state) {
                    Node cur = dp[i * STATES + state];
                    if (cur.score == NEG) continue;

                    // Option 1: leave position i unstarred.
                    bool canSkip = true;
                    if (state > 0) {
                        int prevTarget = state - 1;
                        if (prevTarget == (S[i - 1] - 'a')) {
                            canSkip = false;
                        }
                    }

                    if (canSkip) {
                        relax(i + 1, 0, cur);
                    }

                    // Option 2: start an operation block at i.
                    for (int bi = 0; bi < blockCnt[i]; ++bi) {
                        int len = blocks[i][bi].first;
                        int target = blocks[i][bi].second;

                        bool valid = true;

                        if (i > 1) {
                            int firstOfNew = S[i - 1] - 'a';
                            int lastOfPrev = S[i - 2] - 'a';

                            bool newDepends = (target == lastOfPrev);

                            if (state == 0) {
                                // The new block would extend into an unstarred gap.
                                if (newDepends) valid = false;
                            } else {
                                int prevTarget = state - 1;
                                bool prevDepends = (prevTarget == firstOfNew);

                                // Both blocks need the other removed first -> impossible.
                                if (prevDepends && newDepends) {
                                    valid = false;
                                }
                            }
                        }

                        if (!valid) continue;

                        Node nxt;
                        nxt.score = cur.score + len - lambda;
                        nxt.ops = cur.ops + 1;

                        int nxtPos = i + len;
                        int nxtState = target + 1;

                        relax(nxtPos, nxtState, nxt);
                    }
                }
            }

            Node best{NEG, NEG};
            for (int state = 0; state < STATES; ++state) {
                Node cur = dp[(n + 1) * STATES + state];
                if (cur.score > best.score ||
                    (cur.score == best.score && cur.ops > best.ops)) {
                    best = cur;
                }
            }

            C[lambda] = best.ops;
        }

        long long answer = 0;
        for (int lambda = 1; lambda <= 5; ++lambda) {
            answer += min(K, C[lambda]);
        }

        cout << answer << '\n';
    }

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

struct Val {
    int value;
    int cnt;
};

struct Tile {
    int len;
    int target;
};

const int NEG = -1000000000;

static inline void relax(Val &a, const Val &b) {
    if (b.value > a.value || (b.value == a.value && b.cnt > a.cnt)) {
        a = b;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        string s;
        int K;
        cin >> s >> K;

        int n = (int)s.size();

        vector<vector<Tile>> tiles(n);

        for (int pos = 0; pos < n; ++pos) {
            int cnt[26] = {};
            int maxLen = min(5, n - pos);

            for (int len = 1; len <= maxLen; ++len) {
                ++cnt[s[pos + len - 1] - 'a'];

                if (len >= 3) {
                    for (int c = 0; c < 26; ++c) {
                        if (cnt[c] == len - 1) {
                            tiles[pos].push_back({len, c});
                            break;
                        }
                    }
                }
            }
        }

        auto solve = [&](int penalty) -> Val {
            constexpr int B = 8;

            Val noneBuf[B];
            Val adjBuf[B][26];

            for (int i = 0; i < B; ++i) {
                noneBuf[i] = {NEG, 0};
                for (int c = 0; c < 26; ++c) {
                    adjBuf[i][c] = {NEG, 0};
                }
            }

            noneBuf[0] = {0, 0};

            for (int pos = 0; pos < n; ++pos) {
                int idx = pos & 7;

                Val none = noneBuf[idx];

                int x = s[pos] - 'a';

                Val bestAll = {NEG, 0};
                Val bestExcl = {NEG, 0};

                for (int c = 0; c < 26; ++c) {
                    Val cur = adjBuf[idx][c];
                    if (cur.value > NEG / 2) {
                        relax(bestAll, cur);
                        if (c != x) {
                            relax(bestExcl, cur);
                        }
                    }
                }

                noneBuf[idx] = {NEG, 0};
                for (int c = 0; c < 26; ++c) {
                    adjBuf[idx][c] = {NEG, 0};
                }

                int nxt = (pos + 1) & 7;

                if (none.value > NEG / 2) {
                    relax(noneBuf[nxt], none);
                }

                if (bestExcl.value > NEG / 2) {
                    relax(noneBuf[nxt], bestExcl);
                }

                for (const Tile &tile : tiles[pos]) {
                    int len = tile.len;
                    int c = tile.target;

                    int addValue = len - penalty;

                    if (none.value > NEG / 2) {
                        if (pos == 0 || s[pos - 1] - 'a' != c) {
                            Val nv = {
                                none.value + addValue,
                                none.cnt + 1
                            };
                            int e = (pos + len) & 7;
                            relax(adjBuf[e][c], nv);
                        }
                    }

                    Val src;
                    if (pos > 0 && s[pos - 1] - 'a' == c) {
                        src = bestExcl;
                    } else {
                        src = bestAll;
                    }

                    if (src.value > NEG / 2) {
                        Val nv = {
                            src.value + addValue,
                            src.cnt + 1
                        };
                        int e = (pos + len) & 7;
                        relax(adjBuf[e][c], nv);
                    }
                }
            }

            int idx = n & 7;

            Val best = noneBuf[idx];
            for (int c = 0; c < 26; ++c) {
                relax(best, adjBuf[idx][c]);
            }

            if (best.value < NEG / 2) {
                return {0, 0};
            }

            return best;
        };

        if (K == 0) {
            cout << 0 << '\n';
            continue;
        }

        vector<Val> cache(6);
        vector<bool> have(6, false);

        auto getSolve = [&](int p) -> Val {
            if (!have[p]) {
                cache[p] = solve(p);
                have[p] = true;
            }
            return cache[p];
        };

        Val zero = getSolve(0);

        int answer;

        if (zero.cnt < K) {
            answer = zero.value;
        } else {
            int lo = 0, hi = 6;

            while (hi - lo > 1) {
                int mid = (lo + hi) >> 1;

                Val cur = getSolve(mid);

                if (cur.cnt >= K) {
                    lo = mid;
                } else {
                    hi = mid;
                }
            }

            Val best = getSolve(lo);
            answer = best.value + lo * K;
        }

        cout << answer << '\n';
    }

    return 0;
}

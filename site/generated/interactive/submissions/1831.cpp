#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;

    auto query = [&](int c) -> bool {
        cout << "? " << c << '\n' << flush;
        string s;
        if (!(cin >> s)) exit(0);
        if (s == "Y") return true;
        if (s == "N") return false;
        exit(0);
    };

    auto resetMemory = [&]() {
        cout << "R\n" << flush;
    };

    vector<int> R;
    int distinct = 0;
    const int INF = 1e9;

    for (int l = 1; l <= n; l += k) {
        int rgt = min(n, l + k - 1);
        vector<int> B;

        if (k == 1) {
            B.push_back(l);
        } else {
            resetMemory();
            for (int idx = l; idx <= rgt; ++idx) {
                bool y = query(idx);
                if (!y) B.push_back(idx);
            }
        }

        if (R.empty()) {
            for (int x : B) R.push_back(x);
            distinct += (int)B.size();
            continue;
        }

        if (B.empty()) continue;

        vector<char> matched(B.size(), 0);
        vector<int> U;
        for (int i = 0; i < (int)B.size(); ++i) U.push_back(i);

        bool useFast = true;
        int rsz = (int)R.size();

        auto estimateExact = [&](int a, int b) -> int {
            if (b == 0) return 0;
            vector<int> dp(b + 1, INF);
            dp[0] = 0;

            for (int x = 1; x <= b; ++x) {
                for (int s = 1; s <= x; ++s) {
                    int aMax = k - s + 1;
                    if (aMax < 1) continue;
                    int chunks = (a + aMax - 1) / aMax;
                    int cost = a + chunks * s;
                    dp[x] = min(dp[x], dp[x - s] + cost);
                }
            }
            return dp[b];
        };

        auto exactChunk = [&](int rpos, int a, vector<int>& UU) {
            int b = (int)UU.size();
            if (b == 0) return;

            vector<int> dp(b + 1, INF), choice(b + 1, 0);
            dp[0] = 0;

            for (int x = 1; x <= b; ++x) {
                for (int s = 1; s <= x; ++s) {
                    int aMax = k - s + 1;
                    if (aMax < 1) continue;
                    int chunks = (a + aMax - 1) / aMax;
                    int cost = a + chunks * s;
                    if (dp[x - s] + cost < dp[x]) {
                        dp[x] = dp[x - s] + cost;
                        choice[x] = s;
                    }
                }
            }

            vector<int> sizes;
            for (int x = b; x > 0; x -= choice[x]) {
                sizes.push_back(choice[x]);
            }

            int offset = 0;
            for (int s : sizes) {
                vector<int> cur;
                for (int j = 0; j < s; ++j) {
                    cur.push_back(UU[offset + j]);
                }
                offset += s;

                int pos = 0;
                while (pos < a && !cur.empty()) {
                    int u = (int)cur.size();
                    int aMax = k - u + 1;
                    if (aMax < 1) aMax = 1;

                    int take = min(aMax, a - pos);

                    resetMemory();
                    for (int q = 0; q < take; ++q) {
                        query(R[rpos + pos + q]);
                    }

                    vector<int> nxt;
                    for (int id : cur) {
                        bool y = query(B[id]);
                        if (y) matched[id] = 1;
                        else nxt.push_back(id);
                    }

                    cur.swap(nxt);
                    pos += take;
                }
            }

            vector<int> newU;
            for (int id : UU) {
                if (!matched[id]) newU.push_back(id);
            }
            UU.swap(newU);
        };

        int rpos = 0;

        while (rpos < rsz && !U.empty()) {
            int a = min(k, rsz - rpos);
            bool didFast = false;

            if (useFast && a == k && !U.empty()) {
                int u = (int)U.size();
                int exactCost = estimateExact(k, u);
                int fastCost = 2 * k + u;

                if (fastCost < exactCost) {
                    didFast = true;

                    vector<int> oldU = U;
                    vector<int> aY;
                    bool anyY = false;

                    resetMemory();

                    for (int q = 0; q < k; ++q) {
                        query(R[rpos + q]);
                    }

                    for (int id : oldU) {
                        bool y = query(B[id]);
                        if (y) {
                            matched[id] = 1;
                            anyY = true;
                        }
                    }

                    vector<int> newU;
                    for (int id : oldU) {
                        if (!matched[id]) newU.push_back(id);
                    }
                    U.swap(newU);

                    if (!U.empty()) {
                        for (int j = 0; j < k; ++j) {
                            bool y = query(R[rpos + j]);
                            if (y) {
                                anyY = true;
                                aY.push_back(j);
                            }
                        }

                        int u2 = (int)U.size();
                        int exactCost2 = estimateExact(k, u2);
                        long long individualCost = 1LL * (int)aY.size() * (u2 + 1);

                        if (!aY.empty() && individualCost < exactCost2) {
                            for (int jj : aY) {
                                if (U.empty()) break;

                                int val = R[rpos + jj];

                                resetMemory();
                                query(val);

                                int found = -1;
                                for (int id : U) {
                                    bool y = query(B[id]);
                                    if (y) {
                                        found = id;
                                        break;
                                    }
                                }

                                if (found != -1) {
                                    matched[found] = 1;
                                    vector<int> newU2;
                                    newU2.reserve(U.size() - 1);
                                    for (int id : U) {
                                        if (id != found) newU2.push_back(id);
                                    }
                                    U.swap(newU2);
                                }
                            }
                        } else if (!aY.empty()) {
                            exactChunk(rpos, k, U);
                            useFast = false;
                        }
                    }
                }
            }

            if (!didFast) {
                int before = (int)U.size();
                exactChunk(rpos, a, U);
                if (a == k && useFast && before - (int)U.size() > 2) {
                    useFast = false;
                }
            }

            rpos += a;
        }

        int added = (int)U.size();
        for (int id : U) {
            R.push_back(B[id]);
        }
        distinct += added;
    }

    cout << "! " << distinct << '\n' << flush;
    return 0;
}

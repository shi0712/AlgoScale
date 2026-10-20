#include <bits/stdc++.h>
using namespace std;

struct Group {
    int d[4];
    int cnt;
};

class Solver {
public:
    int cap[4];
    vector<Group> groups;
    vector<int> rem;
    int G;

    bool solveDFS(int C, vector<Group> gs) {
        groups = move(gs);
        G = (int)groups.size();
        rem.assign(G, 0);
        for (int i = 0; i < G; ++i) rem[i] = groups[i].cnt;
        for (int i = 0; i < 4; ++i) cap[i] = C;

        // Forced propagation.
        bool changed = true;
        while (changed) {
            changed = false;
            for (int i = 0; i < G; ++i) {
                if (rem[i] == 0) continue;
                int mask = 0, only = -1, cnt = 0;
                for (int c = 0; c < 4; ++c) {
                    if (groups[i].d[c] <= cap[c]) {
                        ++cnt;
                        only = c;
                        mask |= (1 << c);
                    }
                }
                if (cnt == 0) return false;
                if (cnt == 1) {
                    long long need = 1LL * rem[i] * groups[i].d[only];
                    if (need > cap[only]) return false;
                    cap[only] -= (int)need;
                    rem[i] = 0;
                    changed = true;
                }
            }
        }

        // Quick greedy attempts.
        auto runGreedy = [&](int mode, bool sortByCount) -> bool {
            int cp[4] = {cap[0], cap[1], cap[2], cap[3]};
            vector<int> rc = rem;
            vector<int> ord;
            for (int i = 0; i < G; ++i) if (rc[i] > 0) ord.push_back(i);
            if (sortByCount) {
                sort(ord.begin(), ord.end(), [&](int a, int b) {
                    return rc[a] > rc[b];
                });
            }
            for (int id : ord) {
                int q = rc[id];
                for (int t = 0; t < q; ++t) {
                    int best = -1;
                    double bestVal = -1e100;
                    for (int c = 0; c < 4; ++c) {
                        int d = groups[id].d[c];
                        if (d > cp[c]) continue;
                        double val;
                        if (mode == 0) {
                            val = -d;
                        } else if (mode == 1) {
                            val = cp[c] - d;
                        } else if (mode == 2) {
                            val = (double)cp[c] / d;
                        } else {
                            val = cp[c];
                        }
                        if (best == -1 || val > bestVal) {
                            bestVal = val;
                            best = c;
                        }
                    }
                    if (best == -1) return false;
                    cp[best] -= groups[id].d[best];
                }
            }
            return true;
        };

        for (int mode = 0; mode < 4; ++mode) {
            if (runGreedy(mode, false)) return true;
            if (runGreedy(mode, true)) return true;
        }

        return dfs();
    }

private:
    bool dfs() {
        int sumCapMask[16] = {};
        for (int mask = 0; mask < 16; ++mask) {
            int s = 0;
            for (int c = 0; c < 4; ++c)
                if (mask & (1 << c)) s += cap[c];
            sumCapMask[mask] = s;
        }

        long long req[16] = {};
        int chosen = -1;
        int bestFeat = 10;
        long long bestScore = -1;

        for (int i = 0; i < G; ++i) {
            if (rem[i] == 0) continue;
            int mask = 0, minc = INT_MAX;
            for (int c = 0; c < 4; ++c) {
                int d = groups[i].d[c];
                if (d <= cap[c]) {
                    mask |= (1 << c);
                    minc = min(minc, d);
                }
            }
            if (mask == 0) return false;

            long long cost = 1LL * rem[i] * minc;
            for (int s = 0; s < 16; ++s) {
                if ((mask & ~s) == 0) {
                    req[s] += cost;
                }
            }

            int feat = __builtin_popcount((unsigned)mask);
            if (feat < bestFeat || (feat == bestFeat && cost > bestScore)) {
                bestFeat = feat;
                bestScore = cost;
                chosen = i;
            }
        }

        if (chosen == -1) return true;

        for (int s = 0; s < 16; ++s) {
            if (req[s] > sumCapMask[s]) return false;
        }

        int id = chosen;
        int q = rem[id];
        rem[id] = 0;

        vector<int> opts;
        for (int c = 0; c < 4; ++c) {
            if (groups[id].d[c] <= cap[c]) opts.push_back(c);
        }
        sort(opts.begin(), opts.end(), [&](int a, int b) {
            return groups[id].d[a] < groups[id].d[b];
        });

        int m = (int)opts.size();
        vector<int> suffix(m + 1, 0);
        for (int i = m - 1; i >= 0; --i) {
            int c = opts[i];
            int d = groups[id].d[c];
            suffix[i] = suffix[i + 1] + cap[c] / d;
        }

        if (suffix[0] < q) {
            rem[id] = q;
            return false;
        }

        function<bool(int, int)> distribute = [&](int pos, int remaining) -> bool {
            if (remaining < 0) return false;
            if (pos == m) {
                return remaining == 0 && dfs();
            }
            if (suffix[pos] < remaining) return false;

            int c = opts[pos];
            int d = groups[id].d[c];
            int maxTake = min(remaining, cap[c] / d);

            for (int a = maxTake; a >= 0; --a) {
                cap[c] -= a * d;
                bool ok = false;
                if (suffix[pos + 1] >= remaining - a) {
                    ok = distribute(pos + 1, remaining - a);
                }
                cap[c] += a * d;
                if (ok) return true;
            }
            return false;
        };

        bool ok = distribute(0, q);
        rem[id] = q;
        return ok;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int L, N, T;
    cin >> L >> N >> T;

    int C = T / 2;

    vector<array<int, 4>> pts(N);
    vector<array<int, 4>> dist(N);

    auto getDist = [&](int x, int y) -> array<int, 4> {
        // 0: TL, 1: TR, 2: BR, 3: BL
        array<int, 4> d;
        d[0] = (x - 1) + (y - 1);
        d[1] = (x - 1) + (L - y);
        d[2] = (L - x) + (L - y);
        d[3] = (L - x) + (y - 1);
        return d;
    };

    map<array<int, 4>, int> mp;

    for (int i = 0; i < N; ++i) {
        int x, y;
        cin >> x >> y;
        auto d = getDist(x, y);

        bool any = false;
        for (int c = 0; c < 4; ++c) {
            if (d[c] <= C) any = true;
        }
        if (!any) {
            cout << "No\n";
            return 0;
        }

        mp[d]++;
    }

    int H = L - 1;
    bool sparse = (C < H);

    if (sparse) {
        vector<array<int, 4>> edgeItems[4];
        vector<int> fixed(4, 0);

        for (auto [d, cnt] : mp) {
            int mask = 0;
            vector<int> v;
            for (int c = 0; c < 4; ++c) {
                if (d[c] <= C) {
                    mask |= (1 << c);
                    v.push_back(c);
                }
            }
            if ((int)v.size() > 2) {
                sparse = false;
                break;
            }
            if ((int)v.size() == 1) {
                int c = v[0];
                fixed[c] += cnt * d[c];
                if (fixed[c] > C) {
                    cout << "No\n";
                    return 0;
                }
            } else {
                int a = v[0], b = v[1];
                int id = -1, controlled = -1, other = -1;
                if ((a == 0 && b == 1) || (a == 1 && b == 0)) {
                    id = 0;
                    controlled = 0;
                    other = 1;
                } else if ((a == 1 && b == 2) || (a == 2 && b == 1)) {
                    id = 1;
                    controlled = 1;
                    other = 2;
                } else if ((a == 2 && b == 3) || (a == 3 && b == 2)) {
                    id = 2;
                    controlled = 3;
                    other = 2;
                } else if ((a == 0 && b == 3) || (a == 3 && b == 0)) {
                    id = 3;
                    controlled = 0;
                    other = 3;
                } else {
                    sparse = false;
                    break;
                }

                for (int rep = 0; rep < cnt; ++rep) {
                    edgeItems[id].push_back({d[controlled], d[other]});
                }
            }
        }

        if (sparse) {
            int cap[4];
            for (int c = 0; c < 4; ++c) {
                cap[c] = C - fixed[c];
                if (cap[c] < 0) {
                    cout << "No\n";
                    return 0;
                }
            }

            auto buildDP = [&](const vector<pair<int, int>>& items) {
                vector<int> dp(C + 1, -1e9);
                dp[0] = 0;
                for (auto [ca, cb] : items) {
                    if (ca > C) continue;
                    for (int s = C; s >= ca; --s) {
                        if (dp[s - ca] > -1e8) {
                            dp[s] = max(dp[s], dp[s - ca] + cb);
                        }
                    }
                }
                return dp;
            };

            // Top edge 0-1: exact load at 0 -> minimum load at 1.
            vector<int> dpTop = buildDP(edgeItems[0]);
            int totalTop = 0;
            for (auto [a, b] : edgeItems[0]) totalTop += b;

            vector<int> exactTop(C + 1, INT_MAX);
            for (int s = 0; s <= C; ++s) {
                if (dpTop[s] > -1e8) {
                    exactTop[s] = totalTop - dpTop[s];
                }
            }

            auto buildF = [&](const vector<pair<int, int>>& items, int id) {
                vector<int> dp = buildDP(items);
                int totalB = 0;
                for (auto [a, b] : items) totalB += b;

                vector<int> pref = dp;
                for (int i = 1; i <= C; ++i) {
                    pref[i] = max(pref[i], pref[i - 1]);
                }

                vector<int> f(C + 1);
                for (int x = 0; x <= C; ++x) {
                    int bestB = pref[x];
                    if (bestB < 0) bestB = 0;
                    f[x] = totalB - bestB;
                }
                return f;
            };

            vector<int> fRight = buildF(edgeItems[1], 1);   // 1 -> 2
            vector<int> fBottom = buildF(edgeItems[2], 2);  // 3 -> 2
            vector<int> fLeft = buildF(edgeItems[3], 3);    // 0 -> 3

            bool ok = false;

            for (int s = 0; s <= cap[0]; ++s) {
                if (exactTop[s] == INT_MAX) continue;
                int load1 = exactTop[s];
                if (load1 > cap[1]) continue;

                int load3 = fLeft[cap[0] - s];
                if (load3 > cap[3]) continue;

                int rem1 = cap[1] - load1;
                int load2FromRight = fRight[rem1];
                if (load2FromRight > cap[2]) continue;

                int rem3 = cap[3] - load3;
                int rem2 = cap[2] - load2FromRight;

                if (fBottom[rem3] <= rem2) {
                    ok = true;
                    break;
                }
            }

            cout << (ok ? "Yes\n" : "No\n");
            return 0;
        }
    }

    // General solver.
    vector<Group> groups;
    for (auto [d, cnt] : mp) {
        Group g;
        for (int c = 0; c < 4; ++c) g.d[c] = d[c];
        g.cnt = cnt;
        groups.push_back(g);
    }

    Solver solver;
    bool ok = solver.solveDFS(C, groups);
    cout << (ok ? "Yes\n" : "No\n");
    return 0;
}

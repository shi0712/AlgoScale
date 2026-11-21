#include <bits/stdc++.h>
using namespace std;

static int m;
static long long queryCount = 0;
static int lastQueryId = 0;

int ask(int p, int e) {
    cout << "? " << p << " " << e << '\n' << flush;
    int ans;
    if (!(cin >> ans)) exit(0);
    ++queryCount;
    lastQueryId = (int)queryCount;
    if (ans < 0) exit(0);
    return ans;
}

static int ceil_log2_int(int x) {
    if (x <= 1) return 0;
    int r = 0;
    while ((1 << r) < x) ++r;
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> m)) return 0;

    vector<int> order(m);
    iota(order.begin(), order.end(), 1);
    {
        uint64_t seed = chrono::steady_clock::now().time_since_epoch().count();
        seed ^= (uint64_t)(uintptr_t)&seed;
        mt19937_64 rng(seed);
        shuffle(order.begin(), order.end(), rng);
    }

    // valEdge: -1 = bridge, 1..14 = exact cycle length, 15 = big (>=15).
    vector<int> valEdge(m + 1, -100);

    // Layers are 1-indexed.  Bprefix[L][i] is the query id whose removed set
    // consists of the first i edges of layer L.
    vector<vector<int>> B(16);
    vector<vector<int>> Bprefix(16);
    vector<int> BfullId(16, 0);
    Bprefix[0].push_back(0);

    vector<int> curList = order; // R_{t-1}
    int t = 0;

    for (int layer = 1; layer <= 15; ++layer) {
        vector<int> newB;
        vector<int> newPref;
        newPref.push_back(0);
        vector<int> nextList;
        int cur = 0;
        for (int x : curList) {
            int ans = ask(cur, x);
            if (ans == 1) {
                newB.push_back(x);
                newPref.push_back(lastQueryId);
                cur = lastQueryId;
            } else {
                nextList.push_back(x);
            }
        }
        B[layer] = move(newB);
        Bprefix[layer] = move(newPref);
        BfullId[layer] = cur;

        if (B[layer].empty()) {
            // No non-bridge edge remains; all remaining edges are bridges.
            for (int x : curList) valEdge[x] = -1;
            curList.clear();
            t = layer - 1;
            break;
        }

        t = layer;
        if (nextList.empty()) {
            curList.clear();
            break;
        }
        curList = move(nextList);

        if (layer == 15) break;

        long long rem = (long long)curList.size();
        long long cnt = (long long)B[layer].size();
        if (rem > (long long)(15 - layer) * cnt) {
            // Predicted average cycle length > 15: layering further is not useful.
            break;
        }
    }

    const bool maxLenAtMostT = curList.empty();

    auto classifyLayers = [&](int tt, int defaultLen, vector<int> shortNext) {
        for (int u = tt - 1; u >= 1; --u) {
            int cu = (int)B[u].size();
            int cu1 = (int)B[u + 1].size();
            int d = cu - cu1;
            int s = (int)shortNext.size();
            int lg = ceil_log2_int(cu);
            long long linkCost = (long long)cu1 * lg;
            long long testCost = (long long)s * lg + (d > 0 ? (long long)(cu - s) : 0);
            long long directCost = LLONG_MAX;
            if (tt == 15) {
                directCost = (long long)cu * ceil_log2_int(15 - u + 1);
            }

            if (tt == 15 && directCost < testCost && directCost < linkCost) {
                // Determine the length of every edge in this layer directly.
                for (int r : B[u]) {
                    int lo = u + 1, hi = 15;
                    int firstYes = 16;
                    while (lo <= hi) {
                        int mid = (lo + hi) >> 1;
                        int ans = ask(BfullId[mid], r);
                        if (ans == 1) {
                            firstYes = mid;
                            hi = mid - 1;
                        } else {
                            lo = mid + 1;
                        }
                    }
                    valEdge[r] = (firstYes == 16 ? 15 : firstYes - 1);
                }
            } else if (d == 0 || testCost <= linkCost) {
                // Propagate the known short cycles to this layer.
                for (int sEdge : shortNext) {
                    int lo = 1, hi = cu;
                    while (lo < hi) {
                        int mid = (lo + hi) >> 1;
                        int ans = ask(Bprefix[u][mid], sEdge);
                        if (ans == 0) hi = mid;
                        else lo = mid + 1;
                    }
                    valEdge[B[u][lo - 1]] = valEdge[sEdge];
                }
                if (d > 0) {
                    for (int r : B[u]) {
                        if (valEdge[r] != -100) continue;
                        int ans = ask(BfullId[u + 1], r);
                        if (ans == 1) valEdge[r] = u;       // cycle ends here
                        else valEdge[r] = defaultLen;       // continues to default
                    }
                } else {
                    for (int r : B[u]) {
                        if (valEdge[r] == -100) valEdge[r] = defaultLen;
                    }
                }
            } else {
                // Link every edge of layer u+1 to its mate in layer u.
                vector<int> assigned(cu, -1);
                for (int sEdge : B[u + 1]) {
                    int lo = 1, hi = cu;
                    while (lo < hi) {
                        int mid = (lo + hi) >> 1;
                        int ans = ask(Bprefix[u][mid], sEdge);
                        if (ans == 0) hi = mid;
                        else lo = mid + 1;
                    }
                    assigned[lo - 1] = valEdge[sEdge];
                }
                for (int i = 0; i < cu; ++i) {
                    valEdge[B[u][i]] = (assigned[i] == -1 ? u : assigned[i]);
                }
            }

            shortNext.clear();
            for (int r : B[u]) {
                if (valEdge[r] != defaultLen) shortNext.push_back(r);
            }
        }
    };

    if (t == 0) {
        // B1 was empty: every edge is a bridge (already stored in valEdge).
    } else if (t == 15) {
        for (int e : B[15]) valEdge[e] = 15;
        for (int x : curList) {
            int ans = ask(0, x);
            if (ans == 0) valEdge[x] = -1;
            else valEdge[x] = 15;
        }
        curList.clear();

        vector<int> shortNext; // B15 edges all have the default value 15.
        classifyLayers(15, 15, move(shortNext));
    } else {
        // General finish: t < 15.
        int cT = (int)B[t].size();
        vector<int> found(cT, t);
        vector<char> isBig(cT, 0);
        vector<int> bigPref;
        bigPref.push_back(0);

        auto promote = [&](int idx) {
            if (isBig[idx]) return;
            isBig[idx] = 1;
            ask(bigPref.back(), B[t][idx]);
            bigPref.push_back(lastQueryId);
        };

        auto searchT = [&](int x) -> int {
            int lo = 1, hi = cT;
            while (lo < hi) {
                int mid = (lo + hi) >> 1;
                int ans = ask(Bprefix[t][mid], x);
                if (ans == 0) hi = mid;
                else lo = mid + 1;
            }
            return lo - 1;
        };

        vector<int> repOf(m + 1, -1);

        long long fSmall = 0, fBridge = 0, fExplore = 0;
        auto assignRep = [&](int x, int idx) {
            repOf[x] = idx;
            ++found[idx];
            if (found[idx] >= 15) promote(idx);
        };

        for (int x : curList) {
            bool canUseBig = (!bigPref.empty() && bigPref.size() > 1 && cT >= 3);
            bool bigFirst = false;
            if (canUseBig) {
                long long total = fSmall + fBridge;
                if (total < 8) bigFirst = true;
                else if (fSmall >= fBridge) bigFirst = true;
                else {
                    ++fExplore;
                    if ((fExplore & 31) == 0) bigFirst = true;
                }
            }

            if (bigFirst) {
                int ansB = ask(bigPref.back(), x);
                if (ansB == 0) {
                    int ansP = ask(0, x);
                    if (ansP == 0) { ++fBridge; valEdge[x] = -1; }
                    else valEdge[x] = 15;
                } else {
                    ++fSmall;
                    int idx = searchT(x);
                    assignRep(x, idx);
                }
            } else {
                int ansP = ask(0, x);
                if (ansP == 0) {
                    valEdge[x] = -1;
                } else {
                    if (canUseBig) {
                        int ansB = ask(bigPref.back(), x);
                        if (ansB == 0) {
                            valEdge[x] = 15;
                        } else {
                            ++fSmall;
                            int idx = searchT(x);
                            assignRep(x, idx);
                        }
                    } else {
                        int idx = searchT(x);
                        assignRep(x, idx);
                    }
                }
            }
            if (fSmall + fBridge > 64) {
                fSmall /= 2;
                fBridge /= 2;
            }
        }
        curList.clear();

        vector<int> lenT(cT);
        for (int i = 0; i < cT; ++i) {
            if (isBig[i] || found[i] >= 15) lenT[i] = 15;
            else lenT[i] = found[i];
        }
        for (int i = 0; i < cT; ++i) valEdge[B[t][i]] = lenT[i];

        for (int x = 1; x <= m; ++x) {
            if (repOf[x] >= 0) valEdge[x] = lenT[repOf[x]];
        }

        bool allExactlyT = true;
        for (int i = 0; i < cT; ++i) {
            if (lenT[i] != t) { allExactlyT = false; break; }
        }

        int defaultLen;
        vector<int> shortNext;
        if (allExactlyT) {
            // No cycle is longer than t, and every cycle reaching layer t
            // has length exactly t.
            defaultLen = t;
        } else {
            defaultLen = 15;
            for (int r : B[t]) {
                if (valEdge[r] != 15) shortNext.push_back(r);
            }
        }
        classifyLayers(t, defaultLen, move(shortNext));
    }

    cout << "!";
    for (int e = 1; e <= m; ++e) {
        int v = valEdge[e];
        if (v == -1) cout << " -1";
        else if (v == 15) cout << " 0";
        else cout << ' ' << v;
    }
    cout << '\n' << flush;
    int ok;
    if (cin >> ok) { /* done */ }
    return 0;
}

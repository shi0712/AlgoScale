#include <bits/stdc++.h>
using namespace std;

using ll = long long;
const ll NEG = -(1LL << 60);

struct Option {
    int q;       // coefficient of t
    int d;       // number of doubled active cards
    ll val;      // value obtained from this type
};

struct Group {
    int t;
    vector<Option> opts;
    int maxShift = 0;
    int maxAdd = 0;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<vector<ll>> vals(14);
    for (int i = 0; i < n; ++i) {
        ll v;
        int t;
        cin >> v >> t;
        vals[t].push_back(v);
    }

    const int B = 14;
    const int QOFF = 30;

    vector<Group> groups;

    for (int t = 1; t <= 13; ++t) {
        if (vals[t].empty()) continue;

        auto &vec = vals[t];
        sort(vec.begin(), vec.end(), greater<ll>());

        int cnt = (int)vec.size();
        vector<ll> pref(cnt + 1, 0);
        for (int i = 0; i < cnt; ++i) {
            pref[i + 1] = pref[i] + vec[i];
        }

        int A = min(B, cnt);

        // f[a] = maximum value if this type has exactly a active cards.
        // The remaining selected cards are paired one-to-one on the two sides.
        vector<ll> f(A + 1, NEG);
        for (int a = 0; a <= A; ++a) {
            ll best = NEG;
            for (int c = a; c <= cnt; c += 2) {
                best = max(best, pref[c]);
            }
            f[a] = best;
        }

        // poss[len][d][q + QOFF]:
        // using exactly len active cards, d of them doubled,
        // can get coefficient q.
        static bool poss[15][15][61];
        memset(poss, 0, sizeof(poss));
        poss[0][0][QOFF] = true;

        for (int len = 0; len < A; ++len) {
            for (int d = 0; d <= len; ++d) {
                for (int qi = 0; qi < 61; ++qi) {
                    if (!poss[len][d][qi]) continue;

                    // Not doubled, put on S/T: coefficient +1 / -1
                    if (qi > 0) poss[len + 1][d][qi - 1] = true;
                    if (qi + 1 < 61) poss[len + 1][d][qi + 1] = true;

                    // Doubled, put on S/T: coefficient +2 / -2
                    if (d + 1 <= A) {
                        if (qi >= 2) poss[len + 1][d + 1][qi - 2] = true;
                        if (qi + 2 < 61) poss[len + 1][d + 1][qi + 2] = true;
                    }
                }
            }
        }

        // bestLocal[q + QOFF][d] = best value if coefficient q is achieved
        // using exactly d doubles.
        static ll bestLocal[61][15];
        for (int i = 0; i < 61; ++i)
            for (int j = 0; j <= B; ++j)
                bestLocal[i][j] = NEG;

        for (int a = 0; a <= A; ++a) {
            for (int qi = 0; qi < 61; ++qi) {
                for (int d = 0; d <= a; ++d) {
                    if (poss[a][d][qi]) {
                        bestLocal[qi][d] = max(bestLocal[qi][d], f[a]);
                        break; // larger d is dominated for the same a, qi
                    }
                }
            }
        }

        Group g;
        g.t = t;

        for (int qi = 0; qi < 61; ++qi) {
            ll bestSoFar = NEG;
            for (int d = 0; d <= B; ++d) {
                if (bestLocal[qi][d] > bestSoFar) {
                    bestSoFar = bestLocal[qi][d];
                    g.opts.push_back({qi - QOFF, d, bestSoFar});
                }
            }
        }

        sort(g.opts.begin(), g.opts.end(), [](const Option &a, const Option &b) {
            return a.d < b.d;
        });

        for (auto &op : g.opts) {
            g.maxShift = max(g.maxShift, abs(op.q) * t);
            g.maxAdd = max(g.maxAdd, op.d);
        }

        groups.push_back(std::move(g));
    }

    int maxCostAll = 0;
    int totalMaxD = 0;

    for (auto &g : groups) {
        maxCostAll += g.maxAdd;
        totalMaxD += g.maxShift;
    }

    int K = min(k, maxCostAll);

    int W = 2 * totalMaxD + 1;
    int offset = totalMaxD;

    vector<ll> dp((K + 1) * W, NEG);
    vector<ll> ndp((K + 1) * W, NEG);

    dp[0 * W + offset] = 0;

    int curMaxD = 0;
    int curMaxCost = 0;

    for (auto &g : groups) {
        fill(ndp.begin(), ndp.end(), NEG);

        int newMaxD = curMaxD + g.maxShift;
        int newMaxCost = min(K, curMaxCost + g.maxAdd);

        for (int c = 0; c <= curMaxCost; ++c) {
            for (int diff = -curMaxD; diff <= curMaxD; ++diff) {
                int idx = offset + diff;
                ll cur = dp[c * W + idx];
                if (cur == NEG) continue;

                for (auto &op : g.opts) {
                    int nc = c + op.d;
                    if (nc > K) break; // options are sorted by d

                    int nidx = idx + op.q * g.t;
                    ll nv = cur + op.val;
                    ll &ref = ndp[nc * W + nidx];
                    if (nv > ref) ref = nv;
                }
            }
        }

        dp.swap(ndp);
        curMaxD = newMaxD;
        curMaxCost = newMaxCost;
    }

    ll ans = 0;
    for (int c = 0; c <= K; ++c) {
        ans = max(ans, dp[c * W + offset]);
    }

    cout << ans << '\n';
    return 0;
}

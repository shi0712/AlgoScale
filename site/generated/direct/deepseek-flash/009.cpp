#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

struct Level {
    int c;
    int l;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m, k;
    cin >> n >> m >> k;

    vector<int> L(k + 1, n + 1), R(k + 1, 0), cnt(k + 1, 0);
    for (int pos = 1; pos <= n; ++pos) {
        int c;
        cin >> c;
        L[c] = min(L[c], pos);
        R[c] = max(R[c], pos);
        ++cnt[c];
    }

    vector<int> p(m), x(m);
    vector<vector<int>> pByX(k + 1);
    vector<int> maxPByX(k + 1, 0);

    for (int i = 0; i < m; ++i) {
        cin >> p[i] >> x[i];
        pByX[x[i]].push_back(p[i]);
        maxPByX[x[i]] = max(maxPByX[x[i]], p[i]);
    }

    // pref[h][pos] = number of services with x_i > h and p_i < pos
    vector<vector<int>> pref(k + 1, vector<int>(n + 1, 0));
    vector<int> totalXgt(k + 1, 0);
    vector<int> cntP(n + 1, 0);

    for (int h = k; h >= 0; --h) {
        int cur = 0;
        for (int pos = 0; pos <= n; ++pos) {
            pref[h][pos] = cur;
            cur += cntP[pos];
        }
        totalXgt[h] = cur;

        if (h > 0) {
            for (int val : pByX[h]) {
                ++cntP[val];
            }
        }
    }

    vector<int> Pmax(k + 1, 0);
    for (int c = 1; c <= k; ++c) {
        Pmax[c] = max(Pmax[c - 1], maxPByX[c]);
    }

    vector<Level> C;

    for (int c = 1; c <= k; ++c) {
        if (cnt[c] >= 2 && Pmax[c] < R[c]) {
            int uncovered = totalXgt[c] - pref[c][L[c]];
            int cover = m - uncovered;

            // No service can ever cover this level.
            if (cover == 0) {
                cout << 0 << '\n';
                return 0;
            }

            // At level k, the condition is always satisfiable since x_i <= k.
            if (c < k) {
                C.push_back({c, L[c]});
            }
        }
    }

    sort(C.begin(), C.end(), [](const Level& a, const Level& b) {
        return a.c > b.c;
    });

    // pw[base][e] = base^e
    vector<vector<int>> pw(k + 1, vector<int>(m + 1, 1));
    for (int base = 1; base <= k; ++base) {
        pw[base][0] = 1;
        for (int e = 1; e <= m; ++e) {
            pw[base][e] = 1LL * pw[base][e - 1] * base % MOD;
        }
    }

    long long totalInclusion = 0;
    int sz = (int)C.size();

    for (int i = 0; i < sz; ++i) {
        int h = C[i].c;
        int Lh = C[i].l;

        const int* prefH = pref[h].data();

        int baseInit = k - h;
        int uncoveredInit = totalXgt[h] - prefH[Lh];
        int coverInit = m - uncoveredInit;

        int val = pw[baseInit][coverInit];
        if (val != 0) val = MOD - val; // negative sign

        vector<int> dp(n + 1, 0);
        dp[Lh] = val;

        vector<int> active;
        active.reserve(sz + 1);
        active.push_back(Lh);

        for (int j = i + 1; j < sz; ++j) {
            int c = C[j].c;
            int l = C[j].l;
            int base = k - c;

            int sumSelect = 0;
            vector<int> survivors;
            survivors.reserve(active.size() + 1);

            for (int q : active) {
                if (q < l) {
                    int addItems = prefH[l] - prefH[q];
                    int term = 1LL * dp[q] * pw[base][addItems] % MOD;
                    sumSelect += term;
                    if (sumSelect >= MOD) sumSelect -= MOD;
                    survivors.push_back(q);
                } else {
                    dp[q] = 0;
                }
            }

            int newVal = (sumSelect == 0 ? 0 : MOD - sumSelect);
            active.swap(survivors);

            if (newVal != 0) {
                dp[l] = newVal;
                active.push_back(l);
            } else {
                dp[l] = 0;
            }
        }

        int contribution = 0;

        for (int q : active) {
            int uncovered = totalXgt[h] - prefH[q];
            contribution += 1LL * dp[q] * pw[k][uncovered] % MOD;
            if (contribution >= MOD) contribution -= MOD;
        }

        totalInclusion += contribution;
        totalInclusion %= MOD;
    }

    long long answer = pw[k][m] + totalInclusion;
    answer %= MOD;

    cout << answer << '\n';
    return 0;
}

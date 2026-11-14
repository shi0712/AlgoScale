#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
constexpr int MAX_P = 10;
constexpr int64 NEG = -(1LL << 60);
constexpr int64 INVALID = -(1LL << 61);

template <class Value>
vector<int> row_maxima(const vector<int>& rows, const vector<int>& cols,
                       Value& value) {
    if (rows.empty()) return {};

    vector<int> reduced;
    for (int c : cols) {
        while (!reduced.empty()) {
            int r = rows[(int)reduced.size() - 1];
            if (value(r, reduced.back()) > value(r, c)) break;
            reduced.pop_back();
        }
        if ((int)reduced.size() < (int)rows.size()) reduced.push_back(c);
    }

    vector<int> odd_rows;
    for (int i = 1; i < (int)rows.size(); i += 2) {
        odd_rows.push_back(rows[i]);
    }
    vector<int> odd_answer = row_maxima(odd_rows, reduced, value);
    vector<int> answer(rows.size());

    for (int i = 0; i < (int)rows.size(); i += 2) {
        int lo = i == 0 ? 0 : (int)(lower_bound(reduced.begin(), reduced.end(),
                                                odd_answer[i / 2 - 1]) - reduced.begin());
        int hi = i + 1 == (int)rows.size()
                     ? (int)reduced.size() - 1
                     : (int)(lower_bound(reduced.begin(), reduced.end(),
                                         odd_answer[i / 2]) - reduced.begin());
        int best = reduced[lo];
        for (int z = lo + 1; z <= hi; ++z) {
            if (value(rows[i], reduced[z]) >= value(rows[i], best)) {
                best = reduced[z];
            }
        }
        answer[i] = best;
    }
    for (int i = 1; i < (int)rows.size(); i += 2) {
        answer[i] = odd_answer[i / 2];
    }
    return answer;
}

vector<int64> apply_group(const vector<int64>& dp, int p,
                          const vector<int64>& pref, int m, int k) {
    if (m == 0) return dp;
    vector<int64> next(k + 1, NEG);

    for (int r = 0; r < p && r <= k; ++r) {
        int len = (k - r) / p + 1;
        vector<int64> d(len);
        vector<int> indices(len);
        for (int j = 0; j < len; ++j) {
            d[j] = dp[r + p * j];
            indices[j] = j;
        }

        auto value = [&](int j, int u) -> int64 {
            if (u > j) return INVALID;
            if (j - u > m || d[u] == NEG) return NEG;
            return d[u] + pref[j - u];
        };
        vector<int> best = row_maxima(indices, indices, value);
        for (int j = 0; j < len; ++j) {
            next[r + p * j] = value(j, best[j]);
        }
    }
    return next;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<array<int, MAX_P + 1>> bonus(n);
    vector<vector<pair<int, int>>> items(MAX_P + 1);
    for (int id = 0; id < n; ++id) {
        int p;
        cin >> p;
        for (int x = 1; x <= p; ++x) {
            cin >> bonus[id][x];
        }
        items[p].push_back({bonus[id][p], id});
    }

    for (int p = 1; p <= MAX_P; ++p) {
        sort(items[p].begin(), items[p].end(), greater<pair<int, int>>());
    }

    vector<vector<int64>> pref(MAX_P + 1);
    for (int p = 1; p <= MAX_P; ++p) {
        pref[p].resize(items[p].size() + 1, 0);
        for (int i = 0; i < (int)items[p].size(); ++i) {
            pref[p][i + 1] = pref[p][i] + items[p][i].first;
        }
    }

    auto knapsack_without = [&](int excluded) {
        vector<int64> dp(k + 1, NEG);
        dp[0] = 0;
        for (int p = 1; p <= MAX_P; ++p) {
            if (p == excluded) continue;
            int m = min((int)items[p].size(), k / p);
            dp = apply_group(dp, p, pref[p], m, k);
        }
        return dp;
    };

    vector<int64> full = knapsack_without(0);
    int64 answer = max(0LL, *max_element(full.begin(), full.end()));

    for (int p = 2; p <= MAX_P; ++p) {
        if (items[p].empty()) continue;
        vector<int64> other = knapsack_without(p);
        int m = items[p].size();

        for (int x = 1; x < p && x <= k; ++x) {
            int c = k - x;
            int limit = min(m, c / p);

            vector<int64> before(limit + 2, NEG);
            vector<int64> after(limit + 2, NEG);
            before[1] = other[c];
            for (int t = 1; t <= limit; ++t) {
                before[t + 1] = max(before[t], other[c - p * t] + pref[p][t]);
            }
            for (int t = limit; t >= 0; --t) {
                after[t] = after[t + 1];
                if (t < m) {
                    after[t] = max(after[t], other[c - p * t] + pref[p][t + 1]);
                }
            }

            for (int r = 1; r <= limit; ++r) {
                int id = items[p][r - 1].second;
                int64 best = before[r];
                best = max(best, after[r] - items[p][r - 1].first);
                answer = max(answer, best + bonus[id][x]);
            }
            for (int r = limit + 1; r <= m; ++r) {
                int id = items[p][r - 1].second;
                answer = max(answer, full[c] + bonus[id][x]);
            }
        }
    }

    cout << answer << '\n';
    return 0;
}

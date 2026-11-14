#include <bits/stdc++.h>
using namespace std;

struct Matrix {
    // Maximum gain with each choice (0 = upper, 1 = lower) at
    // the leftmost and rightmost positions of an active component.
    int v[2][2];
    int best() const {
        return max(max(v[0][0], v[0][1]), max(v[1][0], v[1][1]));
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<int> a(n);
        for (int &x : a) cin >> x;
        int mn = *min_element(a.begin(), a.end());
        int mx = *max_element(a.begin(), a.end());
        for (int &x : a) x -= mn;
        int V = mx - mn;
        // For prescribed increments d, the minimum number of intervals is
        // d[0] + sum(max(0, d[i] - d[i-1])).
        int baseline = V - a[0];
        for (int i = 1; i < n; ++i)
            baseline += max(0, a[i - 1] - a[i]);
        if (m == 1 || V == 0) {
            cout << baseline << '\n';
            continue;
        }

        vector<vector<int>> at(V + 1);
        for (int i = 0; i < n; ++i) at[a[i]].push_back(i);
        int distinct = 0;
        for (const auto &bucket : at) distinct += !bucket.empty();
        if (distinct <= 2) {
            cout << 0 << '\n';
            continue;
        }
        vector<int> parent(n), sz(n);
        vector<Matrix> dp(n);
        auto find = [&](int u) {
            while (parent[u] != u) {
                parent[u] = parent[parent[u]];
                u = parent[u];
            }
            return u;
        };
        int answer = baseline;
        constexpr int NEG = -1000000000;

        // The two final values are x and x + gap. We may assume
        // 0 <= x <= V and 1 <= gap <= V. If gap > V, reducing it
        // to V keeps the upper value feasible and only decreases costs
        // at transitions into runs assigned the upper value.
        for (int gap = 1; gap <= V && answer > 0; ++gap) {
            fill(parent.begin(), parent.end(), -1);
            int totalGain = 0;
            for (int x = 0; x < V; ++x) {
                for (int i : at[x]) {
                    // Position i can now use the lower final value.
                    // Inactive positions must use the upper value, so
                    // different active components contribute independently.
                    int l = (i > 0 && parent[i - 1] != -1) ? find(i - 1) : -1;
                    int r = (i + 1 < n && parent[i + 1] != -1) ? find(i + 1) : -1;
                    if (l != -1) totalGain -= dp[l].best();
                    if (r != -1) totalGain -= dp[r].best();

                    // Relative to assigning everything the upper value,
                    // the saving is sum(weight[i] * low[i]) plus
                    // sum(max(0, gap-|a[i]-a[i-1]|) * low[i-1]*low[i]).
                    int weight = (i == 0 ? gap : min(gap, max(0, a[i - 1] - a[i])))
                               - (i == n - 1 ? 0 : gap - min(gap, max(0, a[i + 1] - a[i])));
                    int kl = (i == 0 ? 0 : max(0, gap - abs(a[i] - a[i - 1])));
                    int kr = (i == n - 1 ? 0 : max(0, gap - abs(a[i] - a[i + 1])));
                    Matrix cur;
                    if (l == -1 && r == -1) {
                        cur = {{{0, NEG}, {NEG, weight}}};
                    } else if (r == -1) {
                        for (int s = 0; s < 2; ++s) {
                            cur.v[s][0] = max(dp[l].v[s][0], dp[l].v[s][1]);
                            cur.v[s][1] = weight + max(dp[l].v[s][0], dp[l].v[s][1] + kl);
                        }
                    } else if (l == -1) {
                        for (int t = 0; t < 2; ++t) {
                            cur.v[0][t] = max(dp[r].v[0][t], dp[r].v[1][t]);
                            cur.v[1][t] = weight + max(dp[r].v[0][t], dp[r].v[1][t] + kr);
                        }
                    } else {
                        for (int s = 0; s < 2; ++s) {
                            int l0 = max(dp[l].v[s][0], dp[l].v[s][1]);
                            int l1 = max(dp[l].v[s][0], dp[l].v[s][1] + kl);
                            for (int t = 0; t < 2; ++t) {
                                int r0 = max(dp[r].v[0][t], dp[r].v[1][t]);
                                int r1 = max(dp[r].v[0][t], dp[r].v[1][t] + kr);
                                cur.v[s][t] = max(l0 + r0, weight + l1 + r1);
                            }
                        }
                    }

                    int root = i, size = 1;
                    if (l != -1) { root = l; size += sz[l]; }
                    if (r != -1) {
                        if (l == -1 || sz[r] > sz[l]) root = r;
                        size += sz[r];
                    }
                    parent[i] = root;
                    if (l != -1) parent[l] = root;
                    if (r != -1) parent[r] = root;
                    parent[root] = root;
                    sz[root] = size;
                    dp[root] = cur;
                    totalGain += cur.best();
                }
                if (x + gap >= V)
                    answer = min(answer, baseline + x + gap - V - totalGain);
            }
        }
        cout << answer << '\n';
    }
}

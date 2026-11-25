#include <bits/stdc++.h>
using namespace std;

const int INF = 1e9;

struct Pair {
    int L, l, T;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<vector<int>> G(n + 1, vector<int>(n + 1));
    for (int i = 1; i <= n; ++i)
        for (int j = 1; j <= n; ++j)
            cin >> G[i][j];

    int maxT = 2 * n - 1;
    vector<vector<int>> B(maxT + 2, vector<int>(maxT + 2, INF));

    // Compute B[a][T] for a >= 2
    for (int a = 2; a <= maxT; ++a) {
        vector<vector<int>> dp(n + 1, vector<int>(n + 1, INF));
        int start_sum = a + 1;

        if (a == 2) {
            dp[2][1] = G[2][1];
        } else {
            for (int x = 2; x <= n; ++x) {
                int y = start_sum - x;
                if (y >= 1 && y <= n) {
                    dp[x][y] = G[x][y];
                }
            }
        }

        for (int sum = start_sum + 1; sum <= 2 * n; ++sum) {
            int x_min = max(1, sum - n);
            int x_max = min(n, sum - 1);
            for (int x = x_min; x <= x_max; ++x) {
                int y = sum - x;
                int best = INF;
                if (x > 1 && dp[x - 1][y] < INF) {
                    best = min(best, max(dp[x - 1][y], G[x][y]));
                }
                if (y > 1 && dp[x][y - 1] < INF) {
                    best = min(best, max(dp[x][y - 1], G[x][y]));
                }
                dp[x][y] = best;
            }
        }

        vector<int> best_sum(2 * n + 1, INF);
        for (int sum = start_sum; sum <= 2 * n; ++sum) {
            int x_min = max(1, sum - n);
            int x_max = min(n, sum - 1);
            for (int x = x_min; x <= x_max; ++x) {
                int y = sum - x;
                best_sum[sum] = min(best_sum[sum], dp[x][y]);
            }
        }

        for (int T = a; T <= maxT; ++T) {
            B[a][T] = best_sum[T + 1];
        }
    }

    // a = 1
    B[1][1] = G[1][1];
    for (int T = 2; T <= maxT; ++T) {
        B[1][T] = max(G[1][1], B[2][T]);
    }

    vector<Pair> pairs;
    pairs.reserve(n * maxT);

    for (int l = 1; l <= n; ++l) {
        for (int T = 1; T <= maxT; ++T) {
            int s = T - l + 1;
            int L;
            if (s <= 0) {
                int init_max = 0;
                int c_max = 2 - s;
                for (int c = 2; c <= c_max; ++c) {
                    init_max = max(init_max, G[1][c]);
                }
                L = max(init_max, B[1][T]);
            } else {
                int a = s;
                L = B[a][T];
            }
            pairs.push_back({L, l, T});
        }
    }

    sort(pairs.begin(), pairs.end(), [](const Pair& a, const Pair& b) {
        return a.L < b.L;
    });

    priority_queue<int> pq;
    for (const auto& p : pairs) {
        if ((int)pq.size() == m && p.L >= pq.top()) break;

        cout << "? " << p.l << " " << p.T << "\n";
        cout.flush();

        int f;
        cin >> f;
        if (f == -1) return 0;

        pq.push(f);
        if ((int)pq.size() > m) pq.pop();
    }

    vector<int> ans;
    while (!pq.empty()) {
        ans.push_back(pq.top());
        pq.pop();
    }
    sort(ans.begin(), ans.end());

    cout << "!";
    for (int x : ans) cout << " " << x;
    cout << "\n";
    cout.flush();

    return 0;
}

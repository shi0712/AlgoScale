#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<vector<int64>> values(14);
    int M = 1;

    for (int i = 0; i < n; ++i) {
        int64 v;
        int t;
        cin >> v >> t;
        values[t].push_back(v);
        M = max(M, t);
    }

    /*
       Structural bounds for point numbers in [1, M]:

       - At most 3M - 2 doublings are ever necessary.
       - There is an optimal solution whose total positive signed
         contribution is at most A = 3M^2 - M.
       - Its net signed number of units of any one type is at most
         Z = M(M+1) + 2.

       See the explanation below the code.
    */
    const int A = 3 * M * M - M;
    const int Z = M * (M + 1) + 2;
    const int K = min(k, 3 * M - 2);

    int64 base = 0;
    vector<pair<int64, int>> cards;

    /*
       Discard most cards far from the positive/negative boundary.

       Removed nonnegative cards come in pairs and are permanently
       selected, one on each side. Their values are added to base.
    */
    for (int t = 1; t <= M; ++t) {
        auto &a = values[t];
        sort(a.begin(), a.end(), greater<int64>());

        int p = 0;
        while (p < (int)a.size() && a[p] >= 0) {
            ++p;
        }

        const int B = min(Z, A / t);

        int remove = max(0, p - (B + 3));
        remove -= remove % 2;

        for (int i = 0; i < remove; ++i) {
            base += a[i];
        }

        for (int i = remove; i < p; ++i) {
            cards.emplace_back(a[i], t);
        }

        int last = min<int>(a.size(), p + B + 2);
        for (int i = p; i < last; ++i) {
            cards.emplace_back(a[i], t);
        }
    }

    /*
       dp[b][x]:
       maximum value with absolute point-sum difference x,
       using at most b doublings.

       Differences outside [0, A] are unnecessary.
       Padding permits branch-free reads of x+t and x+2t.
    */
    constexpr int64 NEG = -(1LL << 60);
    const int stride = A + 2 * M + 1;

    vector<int64> dp((K + 1) * stride, NEG);
    vector<int64> ndp((K + 1) * stride, NEG);

    for (int b = 0; b <= K; ++b) {
        dp[b * stride] = 0;
    }

    for (auto [v, t] : cards) {
        const int twice = 2 * t;

        for (int b = 0; b <= K; ++b) {
            const int64 *cur = dp.data() + b * stride;
            const int64 *prev = (b > 0)
                                ? dp.data() + (b - 1) * stride
                                : nullptr;
            int64 *out = ndp.data() + b * stride;

            if (b == 0) {
                for (int x = 0; x <= A; ++x) {
                    int64 take = max(cur[abs(x - t)], cur[x + t]);
                    out[x] = max(cur[x], take + v);
                }
            } else {
                for (int x = 0; x <= A; ++x) {
                    int64 take = max(cur[abs(x - t)], cur[x + t]);
                    take = max(take, prev[abs(x - twice)]);
                    take = max(take, prev[x + twice]);
                    out[x] = max(cur[x], take + v);
                }
            }
        }

        dp.swap(ndp);
    }

    cout << base + dp[K * stride] << '\n';
    return 0;
}

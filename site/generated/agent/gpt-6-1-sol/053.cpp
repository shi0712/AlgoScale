#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
constexpr int64 INF = 4'000'000'000'000'000'000LL;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, c;
    cin >> n >> c;
    vector<int64> t(n + 1), prefix(n + 1);
    for (int i = 1; i <= n; ++i) cin >> t[i];
    sort(t.begin() + 1, t.end());
    for (int i = 1; i <= n; ++i) prefix[i] = prefix[i - 1] + t[i];

    if (n == c) {
        cout << t[n] << '\n';
        return 0;
    }

    if (c == 2) {
        vector<int64> dp(n + 1);
        dp[1] = t[1];
        dp[2] = t[2];
        for (int m = 3; m <= n; ++m) {
            dp[m] = dp[m - 1] + t[1] + t[m];
            if (m >= 4) {
                dp[m] = min(dp[m], dp[m - 2] + t[1] + 2 * t[2] + t[m]);
            }
        }
        cout << dp[n] << '\n';
        return 0;
    }

    /*
       Work with the multiset of forward trips. A walker appearing f times
       forwards must return f-1 times. Such a multiset can be scheduled when
       it covers everyone and the total number of returns is one less than
       the number of forward trips.

       An optimal multiset can be peeled from the slowest end as follows:
       for m > c, take the c-i slowest remaining walkers and the i fastest
       walkers, charging t[m] + prefix[i]. The latter i appearances are
       paired with returns, so the remaining original walkers are 1..m-c+i.

       Let e be the excess number of returns still required by the trips
       already peeled. Initially (m,e)=(n,0), and the transition is
         (m,e) -> (m-c+i, e+1-i),  0 <= i < c, e+1-i >= 0.
       Once m <= c, a trip contains all m remaining walkers, and can leave
       i of them to appear again: (m,e) -> (i,e+1-i).
       State (m,0), m <= c, finishes with one trip of cost t[m].
    */

    const int max_excess = (n - 2) / c;
    const int stride = max_excess + 1;

    // Exact terminal costs for 2 <= m <= c. There are only O(n) entries.
    vector<int64> finish(size_t(c + 1) * stride, INF);
    for (int m = 2; m <= c; ++m) finish[size_t(m) * stride] = t[m];
    for (int e = 1; e <= max_excess; ++e) {
        for (int m = 2; m <= c; ++m) {
            int64 best = INF;
            for (int i = 2; i <= min(m, e + 1); ++i) {
                best = min(best, finish[size_t(i) * stride + e + 1 - i] + prefix[i]);
            }
            finish[size_t(m) * stride + e] = t[m] + best;
        }
    }

    /*
       Every full-trip transition decreases m+e by c-1. Process these
       diagonals, retaining only two rows. At diagonal d, m+e=n-d*(c-1)
       and 0 <= e <= d. Store the cost so far PLUS t[m], which makes the
       transition a short min-plus convolution with prefix[].

       There are O(n^2 / (c*(c-1))) states and at most c transitions per
       state: O(n^2/c) time for c >= 3, and O(n) memory.
    */
    vector<int64> cur(max_excess + 3, INF), next(max_excess + 3, INF);
    cur[0] = t[n];
    int diagonal_sum = n;
    int last_excess = 0;
    int64 answer = INF;

    for (int d = 0; diagonal_sum > c; ++d) {
        const int next_sum = diagonal_sum - (c - 1);
        const int next_last = min(d + 1, next_sum - c - 1);

        if (c == 3) {
            // The largest table occurs for c=3; keep this loop vectorizable.
            if (next_last >= 0) {
                next[0] = min(cur[0] + prefix[1], cur[1] + prefix[2]) + t[next_sum];
                for (int e = 1; e <= next_last; ++e) {
                    next[e] = min({cur[e - 1], cur[e] + prefix[1],
                                   cur[e + 1] + prefix[2]}) + t[next_sum - e];
                }
            }
        } else {
            for (int e = 0; e <= next_last; ++e) {
                int64 best = INF;
                const int first_i = max(0, 1 - e);
                const int last_i = min(c - 1, last_excess + 1 - e);
                for (int i = first_i; i <= last_i; ++i) {
                    best = min(best, cur[e - 1 + i] + prefix[i]);
                }
                next[e] = best + t[next_sum - e];
            }
        }

        // Transitions that reach m <= c finish using the precomputed table.
        const int first_terminal = max(0, next_sum - c);
        const int last_terminal = min(d + 1, next_sum - 2);
        for (int e = first_terminal; e <= last_terminal; ++e) {
            int64 best = INF;
            for (int i = max(0, 1 - e); i <= min(c - 1, last_excess + 1 - e); ++i) {
                best = min(best, cur[e - 1 + i] + prefix[i]);
            }
            answer = min(answer, best + finish[size_t(next_sum - e) * stride + e]);
        }

        if (next_last < 0) break;
        next[next_last + 1] = next[next_last + 2] = INF;
        cur.swap(next);
        diagonal_sum = next_sum;
        last_excess = next_last;
    }

    cout << answer << '\n';
}

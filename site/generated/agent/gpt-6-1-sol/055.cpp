#include <bits/stdc++.h>
using namespace std;

static inline char majority(char a, char b, char c) {
    if (a == b || a == c) return a;
    if (b == c) return b;
    return 0;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        string s;
        int k;
        cin >> s >> k;
        int n = (int)s.size();
        if (n < 3) {
            cout << 0 << '\n';
            continue;
        }

        // Every removable interval has one exceptional letter and length 3..5.
        // All eligible intervals with the same right endpoint have the same
        // target letter; every length from 3 through longest[i] is eligible.
        vector<unsigned char> longest(n + 1), blocked(n + 1);
        for (int i = 3; i <= n; ++i) {
            char c = majority(s[i - 3], s[i - 2], s[i - 1]);
            if (!c) continue;
            longest[i] = 3;
            if (i >= 4 && s[i - 4] == c) {
                longest[i] = 4;
                if (i >= 5 && s[i - 5] == c) longest[i] = 5;
            }
        }

        // Adjacent intervals cannot both be performed if each one's run
        // would extend into the other before the other has been removed.
        // At a fixed boundary, this condition does not depend on their lengths.
        for (int i = 3; i + 3 <= n; ++i) {
            char left = majority(s[i - 3], s[i - 2], s[i - 1]);
            char right = majority(s[i], s[i + 1], s[i + 2]);
            blocked[i] = (left == s[i] && right == s[i - 1]);
        }

        // Pack (absorbed - penalty * edits, edits) into one integer, breaking
        // ties in favor of more edits. The count never reaches base.
        const long long base = n / 3 + 1;
        vector<long long> dp(n + 1);
        auto optimize = [&](int penalty) -> long long {
            const long long gain3 = (3 - penalty) * base + 1;
            const long long gain4 = gain3 + base;
            const long long gain5 = gain4 + base;
            dp[0] = 0;
            for (int i = 1; i <= n; ++i) {
                long long best = dp[i - 1];
                if (longest[i] >= 3) {
                    int j = i - 3;
                    best = max(best, dp[j - blocked[j]] + gain3);
                    if (longest[i] >= 4) {
                        --j;
                        best = max(best, dp[j - blocked[j]] + gain4);
                        if (longest[i] == 5) {
                            --j;
                            best = max(best, dp[j - blocked[j]] + gain5);
                        }
                    }
                }
                dp[i] = best;
            }
            return dp[n];
        };

        long long optimum = optimize(0);
        if (optimum % base <= k) {
            cout << optimum / base << '\n';
            continue;
        }

        // The optimum coverage as a function of the edit budget is discretely
        // concave (removal intervals can be trimmed when exchanging two plans).
        // Its integer marginal gains lie in [0,5]. Find the last penalty whose
        // optimum still uses at least k edits, then recover coverage at k.
        int low = 0, high = 6;
        while (high - low > 1) {
            int mid = (low + high) / 2;
            long long candidate = optimize(mid);
            if (candidate % base >= k) {
                low = mid;
                optimum = candidate;
            } else {
                high = mid;
            }
        }
        cout << optimum / base + (long long)low * k << '\n';
    }
}

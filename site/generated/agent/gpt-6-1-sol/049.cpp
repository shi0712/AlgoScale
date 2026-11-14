#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int modpow(int a, int e) {
    int r = 1;
    while (e) {
        if (e & 1) r = (long long)r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return r;
}

int solve(const vector<int>& p) {
    int n = (int)p.size();
    if (is_sorted(p.begin(), p.end())) return 0;

    const int stride = n + 1;
    vector<int> f(stride * stride), column_sum(stride);
    const int inv = modpow(n - 1, MOD - 2);

    // In two sorted blocks, consider an inverted pair. There are b elements
    // from its left member to the split, and c from the split to its right
    // member. f[b][c] is the expected time until this pair is ordered.
    // One cut leaves it unchanged; the other surviving cuts reduce b or c.
    for (int b = 1; b < n; ++b) {
        int row_sum = 0;
        for (int c = 1; b + c <= n; ++c) {
            int sum = row_sum + column_sum[c];
            if (sum >= MOD) sum -= MOD;
            sum += n;
            if (sum >= MOD) sum -= MOD;
            int value = (long long)sum * inv % MOD;
            f[b * stride + c] = value;
            row_sum += value;
            if (row_sum >= MOD) row_sum -= MOD;
            column_sum[c] += value;
            if (column_sum[c] >= MOD) column_sum[c] -= MOD;
        }
    }

    vector<unsigned char> in_left(n + 2);
    int total = 0;
    for (int split = 1; split < n; ++split) {
        in_left[p[split - 1]] = 1;
        int count_left = 0, previous_c = 0;
        for (int k = 1; k < n; ++k) {
            count_left += in_left[k];
            if (!in_left[k] && in_left[k + 1]) {
                int b = split - count_left;
                int c = k - count_left;
                // It suffices to track inverted consecutive values (k+1,k).
                // Surviving such pairs form an interval in value order, so
                // union = sum of singles - sum of neighboring intersections.
                total += f[b * stride + c];
                if (total >= MOD) total -= MOD;
                if (previous_c) {
                    total -= f[b * stride + previous_c];
                    if (total < 0) total += MOD;
                }
                previous_c = c;
            }
        }
    }

    // The first operation is required, and each of its n cuts is equiprobable.
    return (1 + (long long)total * modpow(n, MOD - 2)) % MOD;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    while (t--) {
        int n;
        cin >> n;
        vector<int> p(n);
        for (int& x : p) cin >> x;
        cout << solve(p) << '\n';
    }
}

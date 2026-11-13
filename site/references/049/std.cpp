#include <bits/stdc++.h>
using namespace std;

namespace {

constexpr int MOD = 998244353;
constexpr int MAX_N = 2000;
int inv[MAX_N + 1];

inline void add_mod(int& a, int b) {
    a += b;
    if (a >= MOD) a -= MOD;
}

void solve_case() {
    int n;
    cin >> n;

    vector<int> pos(n + 1);
    bool initially_sorted = true;
    for (int i = 1; i <= n; ++i) {
        int x;
        cin >> x;
        pos[x] = i;
        initially_sorted &= (x == i);
    }

    if (initially_sorted) {
        cout << 0 << '\n';
        return;
    }

    const int inv_q = inv[n - 1];
    const int stride = n + 1;

    // pref[d][k] = sum_{0 <= e < k} K[d][e].
    // Stored contiguously to avoid per-row allocation overhead.
    vector<int> pref(static_cast<size_t>(n) * stride, 0);
    vector<int> column_sum(n, 0);

    for (int d = 0; d < n; ++d) {
        int row_sum = 0;
        for (int e = 0; e < n; ++e) {
            int cur;
            if (d == 0 && e == 0) {
                cur = 1;
            } else {
                int sum = column_sum[e];
                add_mod(sum, row_sum);
                cur = static_cast<int>(1LL * sum * inv_q % MOD);
            }
            add_mod(column_sum[e], cur);
            add_mod(row_sum, cur);
            pref[static_cast<size_t>(d) * stride + e + 1] = row_sum;
        }
    }

    int total_weight = 0;
    for (int cut = 1; cut < n; ++cut) {
        int left_greater = cut;
        int right_smaller = 0;

        for (int value = 1; value <= n; ++value) {
            if (pos[value] <= cut) {
                --left_greater;
                const int contribution =
                    pref[static_cast<size_t>(left_greater) * stride + right_smaller];
                add_mod(total_weight, contribution);
            } else {
                ++right_smaller;
            }
        }
    }

    int answer = 1;
    add_mod(answer, static_cast<int>(1LL * total_weight * inv_q % MOD));
    cout << answer << '\n';
}

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    inv[1] = 1;
    for (int i = 2; i <= MAX_N; ++i) {
        inv[i] = static_cast<int>(MOD - 1LL * (MOD / i) * inv[MOD % i] % MOD);
    }

    int test_count;
    cin >> test_count;
    while (test_count--) solve_case();
    return 0;
}

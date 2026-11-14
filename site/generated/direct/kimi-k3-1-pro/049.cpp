#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int mod_pow(int a, long long e) {
    long long result = 1;
    long long base = a;
    while (e > 0) {
        if (e & 1) result = result * base % MOD;
        base = base * base % MOD;
        e >>= 1;
    }
    return static_cast<int>(result);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<int> p(n);
        bool initially_sorted = true;
        for (int i = 0; i < n; ++i) {
            cin >> p[i];
            if (p[i] != i + 1) initially_sorted = false;
        }

        if (initially_sorted) {
            cout << 0 << '\n';
            continue;
        }

        const int q = mod_pow(n - 1, MOD - 2);
        const int A = (q + 1) % MOD;
        const int B = (1 + 2LL * q) % MOD;
        const int inv_n = mod_pow(n, MOD - 2);

        const int N = n;
        const int stride = N + 1;
        const int table_size = stride * stride;

        vector<int> K(table_size);
        vector<int> F(table_size);

        // Build K and F row by row.
        for (int z = 0; z <= N; ++z) {
            const int row = z * stride;
            const int previous_row = (z - 1) * stride;

            if (z == 0) {
                K[0] = 1;
            } else {
                K[row] = static_cast<int>(1LL * A * K[previous_row] % MOD);
            }
            F[row] = static_cast<int>(1LL * q * K[row] % MOD);

            for (int o = 1; o <= N; ++o) {
                long long value;

                if (z == 0) {
                    value = 1LL * A * K[row + o - 1];
                } else {
                    value = 1LL * A * K[previous_row + o]
                          + 1LL * A * K[row + o - 1]
                          - 1LL * B * K[previous_row + o - 1];
                }

                value %= MOD;
                if (value < 0) value += MOD;
                K[row + o] = static_cast<int>(value);

                value = 1LL * A * F[row + o - 1]
                      + 1LL * q * (K[row + o] - K[row + o - 1]);
                value %= MOD;
                if (value < 0) value += MOD;
                F[row + o] = static_cast<int>(value);
            }
        }

        vector<unsigned char> in_prefix(n + 1, 0);
        int mex = 1;
        int maximum = 0;
        int expectation_sum = 0;

        for (int i = 1; i < n; ++i) {
            in_prefix[p[i - 1]] = 1;
            maximum = max(maximum, p[i - 1]);

            while (in_prefix[mex]) ++mex;

            // The prefix contains exactly {1,...,i}, so this cut sorts everything.
            if (maximum == i) continue;

            int ones = 0;
            for (int value = mex + 1; value < maximum; ++value) {
                ones += in_prefix[value];
            }

            // K(0, ones) = A^ones.
            int h = K[ones];

            int zeros_before = 0;
            int ones_after = ones;

            for (int value = mex + 1; value < maximum; ++value) {
                if (in_prefix[value]) {
                    --ones_after;
                } else {
                    h += F[zeros_before * stride + ones_after];
                    if (h >= MOD) h -= MOD;
                    ++zeros_before;
                }
            }

            const int expectation = static_cast<int>(1LL * A * h % MOD);
            expectation_sum += expectation;
            if (expectation_sum >= MOD) expectation_sum -= MOD;
        }

        const int answer =
            (1 + 1LL * expectation_sum * inv_n) % MOD;
        cout << answer << '\n';
    }

    return 0;
}

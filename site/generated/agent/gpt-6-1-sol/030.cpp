#include <bits/stdc++.h>
using namespace std;

constexpr int MOD = 998244353;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<int> a(m), b(m);
    for (int &x : a) cin >> x;
    int total = 0;
    for (int &x : b) {
        cin >> x;
        total += x;
    }
    if (total == 0 || total == n) {
        cout << total << '\n';
        return 0;
    }

    vector<int> inv(n + 1), fact(n + 1), invfact(n + 1);
    inv[1] = 1;
    fact[0] = invfact[0] = 1;
    for (int i = 1; i <= n; ++i) {
        if (i > 1) inv[i] = MOD - (long long)(MOD / i) * inv[MOD % i] % MOD;
        fact[i] = (long long)fact[i - 1] * i % MOD;
        invfact[i] = (long long)invfact[i - 1] * inv[i] % MOD;
    }

    // Give every remaining card a rate-one exponential clock. A type finishes
    // at its b-th smallest clock time. With x = exp(-t), its CDF is P_i(x),
    // where P_i(0) = 1 and
    // P_i'(x) = -a! / ((a-b)! (b-1)!) * x^(a-b) * (1-x)^(b-1).
    vector<int> product(1, 1);
    int sum_kept_times = 0;
    for (int type = 0; type < m; ++type) {
        int A = a[type], B = b[type];
        if (B == 0) continue;

        int time = 0;
        for (int k = 1; k <= B; ++k) {
            time += inv[A - k + 1];
            if (time >= MOD) time -= MOD;
            sum_kept_times += time;
            if (sum_kept_times >= MOD) sum_kept_times -= MOD;
        }

        int first = A - B + 1;
        int scale = (long long)fact[A] * invfact[A - B] % MOD
                    * invfact[B - 1] % MOD;
        vector<int> coefficients(B);
        for (int j = 0; j < B; ++j) {
            int choose = (long long)fact[B - 1] * invfact[j] % MOD
                         * invfact[B - 1 - j] % MOD;
            int value = (long long)scale * choose % MOD * inv[first + j] % MOD;
            coefficients[j] = (j % 2 == 0) ? MOD - value : value;
        }

        vector<int> next(product.size() + A, 0);
        copy(product.begin(), product.end(), next.begin());
        for (int i = 0; i < (int)product.size(); ++i) {
            if (product[i] == 0) continue;
            for (int j = 0; j < B; ++j) {
                int value = (long long)product[i] * coefficients[j] % MOD;
                int &dest = next[i + first + j];
                dest += value;
                if (dest >= MOD) dest -= MOD;
            }
        }
        product.swap(next);
    }

    // If T is the last completion time, E[T] = integral_0^1 (1-P(x))/x dx.
    int expected_time = 0;
    for (int k = 1; k < (int)product.size(); ++k) {
        expected_time -= (long long)product[k] * inv[k] % MOD;
        if (expected_time < 0) expected_time += MOD;
    }

    // The draw rate is the number of cards still in the pool. Integrating it
    // until T gives (n-total)*T plus the sum of all kept cards' removal times.
    int answer = ((long long)(n - total) * expected_time + sum_kept_times) % MOD;
    cout << answer << '\n';
}

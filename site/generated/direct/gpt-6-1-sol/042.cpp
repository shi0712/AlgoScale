#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int mod_pow(int a, int e) {
    int result = 1;
    while (e > 0) {
        if (e & 1) result = (long long)result * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    if (n == 1) {
        cout << k << '\n';
        return 0;
    }

    // Only coefficients of degrees 0, ..., k-1 are needed.
    vector<int> inv(k);
    if (k > 1) inv[1] = 1;
    for (int i = 2; i < k; ++i) {
        inv[i] = MOD - (long long)(MOD / i) * inv[MOD % i] % MOD;
    }

    int binom[11][11] = {};
    for (int i = 0; i <= n; ++i) {
        binom[i][0] = binom[i][i] = 1;
        for (int j = 1; j < i; ++j) {
            binom[i][j] = binom[i - 1][j - 1] + binom[i - 1][j];
        }
    }

    // signed_binom[j][h] = (-1)^(h-j+1) C(n-j, h-j).
    int signed_binom[11][11] = {};
    for (int j = 1; j < n; ++j) {
        for (int h = j + 1; h <= n; ++h) {
            int v = binom[n - j][h - j];
            signed_binom[j][h] = ((h - j) & 1) ? v : -v;
        }
    }

    int previous[12][12] = {};
    int current[12][12] = {};

    // history[j] is a circular buffer for coefficients of T_{j,0}.
    int history[12][12] = {};
    for (int j = 1; j <= n; ++j) {
        previous[j][0] = 1;
        history[j][0] = 1;
    }

    int answer = n;       // Sum of maxima after one round.
    int factorial = 1;

    // At degree n*m, T_{n,0} has coefficient 1/(m!)^n.
    int equal_coefficient = 1;

    for (int d = 1; d < k; ++d) {
        // The j=n layer has an explicit, sparse form.
        if (d % n == 0) {
            int m = d / n;
            equal_coefficient =
                (long long)equal_coefficient * mod_pow(inv[m], n) % MOD;

            current[n][0] = equal_coefficient;
            for (int r = 1; r < n; ++r) {
                current[n][r] =
                    (long long)current[n][r - 1] * m % MOD;
            }
        } else {
            for (int r = 0; r < n; ++r) current[n][r] = 0;
        }

        int inverse_d = inv[d];

        for (int j = n - 1; j >= 1; --j) {
            int q = n - j;
            int alpha = (long long)j * inverse_d % MOD;
            int beta = (long long)q * inverse_d % MOD;

            int position = d % j;
            long long top = (d >= j ? history[j][position] : 0);

            for (int h = j + 1; h <= n; ++h) {
                top += (long long)signed_binom[j][h] * current[h][j];
            }
            top %= MOD;
            if (top < 0) top += MOD;

            int r = j - 1;
            int delta = previous[j][r] - previous[j + 1][r];
            if (delta < 0) delta += MOD;

            current[j][r] =
                ((unsigned long long)alpha * top +
                 (unsigned long long)beta * delta) % MOD;

            for (--r; r >= 0; --r) {
                delta = previous[j][r] - previous[j + 1][r];
                if (delta < 0) delta += MOD;

                current[j][r] =
                    ((unsigned long long)alpha * current[j][r + 1] +
                     (unsigned long long)beta * delta) % MOD;
            }

            history[j][position] = current[j][0];
        }

        factorial = (long long)factorial * d % MOD;
        int leaders = (long long)factorial * current[1][0] % MOD;
        answer = (long long)n * ((answer + (long long)leaders) % MOD) % MOD;

        for (int j = 1; j <= n; ++j) {
            for (int r = 0; r < j; ++r) {
                previous[j][r] = current[j][r];
            }
        }
    }

    cout << answer << '\n';
    return 0;
}

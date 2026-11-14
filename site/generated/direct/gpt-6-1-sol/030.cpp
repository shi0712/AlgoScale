#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> a(m), b(m);
    for (int &x : a) cin >> x;

    int B = 0;
    for (int &x : b) {
        cin >> x;
        B += x;
    }

    if (B == 0) {
        cout << 0 << '\n';
        return 0;
    }
    if (B == n) {
        cout << n << '\n';
        return 0;
    }

    vector<int> inv(n + 1), fact(n + 1), invfact(n + 1);
    inv[1] = 1;
    for (int i = 2; i <= n; ++i) {
        inv[i] = MOD - static_cast<long long>(MOD / i) * inv[MOD % i] % MOD;
    }

    fact[0] = invfact[0] = 1;
    for (int i = 1; i <= n; ++i) {
        fact[i] = static_cast<long long>(fact[i - 1]) * i % MOD;
        invfact[i] = static_cast<long long>(invfact[i - 1]) * inv[i] % MOD;
    }

    // Coefficients of the product of all completion-probability polynomials.
    vector<int> poly(1, 1);
    int correction = 0;

    for (int type = 0; type < m; ++type) {
        int A = a[type], R = b[type];
        if (R == 0) continue;

        int expectedTypeTime = 0;
        for (int k = 0; k < R; ++k) {
            expectedTypeTime += inv[A - k];
            if (expectedTypeTime >= MOD) expectedTypeTime -= MOD;
        }

        correction = (correction
            + static_cast<long long>(A - R) * expectedTypeTime) % MOD;

        // F(x) = 1 + sum_j term[j] * x^(firstDegree + j).
        int firstDegree = A - R + 1;

        // A! / ((A-R)! (R-1)!)
        int factor = static_cast<long long>(fact[A]) * invfact[A - R] % MOD;
        factor = static_cast<long long>(factor) * invfact[R - 1] % MOD;

        vector<int> term(R);
        for (int j = 0; j < R; ++j) {
            int choose = static_cast<long long>(fact[R - 1]) * invfact[j] % MOD;
            choose = static_cast<long long>(choose) * invfact[R - 1 - j] % MOD;

            int value = static_cast<long long>(factor) * choose % MOD;
            value = static_cast<long long>(value) * inv[firstDegree + j] % MOD;

            // Coefficient is -factor * (-1)^j * C(R-1,j) / degree.
            if ((j & 1) == 0 && value != 0) value = MOD - value;
            term[j] = value;
        }

        int oldSize = static_cast<int>(poly.size());
        vector<int> next(oldSize + A, 0);

        // Contribution from the constant coefficient 1.
        copy(poly.begin(), poly.end(), next.begin());

        for (int k = 0; k < oldSize; ++k) {
            if (poly[k] == 0) continue;
            for (int j = 0; j < R; ++j) {
                int pos = k + firstDegree + j;
                int add = static_cast<long long>(poly[k]) * term[j] % MOD;
                int value = next[pos] + add;
                if (value >= MOD) value -= MOD;
                next[pos] = value;
            }
        }

        poly.swap(next);
    }

    // E[T] = -sum_{k >= 1} poly[k] / k.
    int expectedTime = 0;
    for (int k = 1; k < static_cast<int>(poly.size()); ++k) {
        int value = static_cast<long long>(poly[k]) * inv[k] % MOD;
        expectedTime -= value;
        if (expectedTime < 0) expectedTime += MOD;
    }

    long long answer =
        (B + static_cast<long long>(n - B) * expectedTime - correction) % MOD;
    if (answer < 0) answer += MOD;

    cout << answer << '\n';
    return 0;
}

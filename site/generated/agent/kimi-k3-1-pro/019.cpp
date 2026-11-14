#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, mod;
    cin >> n >> mod;

    const int lim = n * n + n + 1;
    vector<int> fact(lim + 1), invFact(lim + 1), binomN(lim + 1);
    fact[0] = 1;
    for (int i = 1; i <= lim; ++i)
        fact[i] = static_cast<long long>(fact[i - 1]) * i % mod;

    auto power = [&](int a, int e) {
        long long result = 1;
        while (e) {
            if (e & 1) result = result * a % mod;
            a = static_cast<long long>(a) * a % mod;
            e >>= 1;
        }
        return static_cast<int>(result);
    };

    invFact[lim] = power(fact[lim], mod - 2);
    for (int i = lim; i > 0; --i)
        invFact[i - 1] = static_cast<long long>(invFact[i]) * i % mod;
    for (int i = n; i <= lim; ++i)
        binomN[i] = static_cast<long long>(fact[i]) * invFact[n] % mod * invFact[i - n] % mod;

    vector<int> chooseN(n + 1), chooseN1(n + 1);
    for (int i = 0; i <= n; ++i) {
        chooseN[i] = static_cast<long long>(fact[n]) * invFact[i] % mod * invFact[n - i] % mod;
        chooseN1[i] = static_cast<long long>(fact[n + 1]) * invFact[i] % mod * invFact[n + 1 - i] % mod;
    }

    const int width = n + 1;
    vector<int> f(width * width), answer(width * width);

    for (int m = 0; m < n; ++m) {
        for (int r = 0; r < n; ++r) {
            if (r == 0) {
                f[m * width + r] = (m == 0);
                continue;
            }
            if (m == 0) {
                f[r] = binomN[n + r];
                continue;
            }

            long long value = 0;
            int pending = 0;
            for (int j = 0; j <= m; ++j) {
                const int a = (m + 1 - j) * r - j + n;
                const int b = (m - j) * r - j + n;
                int term = (a >= n ? binomN[a] : 0) - (b >= n ? binomN[b] : 0);
                const long long add = static_cast<long long>(chooseN[j]) * term;
                value += (j & 1) ? -add : add;
                if (++pending == 4) {
                    value %= mod;
                    pending = 0;
                }
            }
            value %= mod;
            if (value < 0) value += mod;
            f[m * width + r] = value;
        }
    }

    for (int m = 0; m < n; ++m) {
        for (int k = 0; k < n; ++k) {
            long long value = 0;
            int pending = 0;
            for (int r = 0; r <= k; ++r) {
                const long long add = static_cast<long long>(chooseN1[r]) * f[m * width + k - r];
                value += (r & 1) ? -add : add;
                if (++pending == 4) {
                    value %= mod;
                    pending = 0;
                }
            }
            value %= mod;
            if (value < 0) value += mod;
            answer[m * width + k] = value;
        }
    }

    for (int m = 0; m < n; ++m) {
        for (int k = 0; k < n; ++k) {
            if (k) cout << ' ';
            cout << answer[m * width + k];
        }
        cout << '\n';
    }
}

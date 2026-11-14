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
    for (int &x : b) cin >> x;

    int B = 0;
    for (int x : b) B += x;

    // Binomial coefficients modulo MOD.
    const int stride = n + 1;
    vector<int> bc((n + 1) * stride, 0);
    auto binom = [&](int r, int c) -> int& {
        return bc[r * stride + c];
    };

    for (int i = 0; i <= n; ++i) {
        binom(i, 0) = binom(i, i) = 1;
        for (int j = 1; j < i; ++j) {
            int v = binom(i - 1, j - 1) + binom(i - 1, j);
            if (v >= MOD) v -= MOD;
            binom(i, j) = v;
        }
    }

    // S = sum D_i, P = product C_i.
    vector<int> S(n + 1, 0);
    vector<int> P(1, 1);

    for (int t = 0; t < m; ++t) {
        const int A = a[t];
        const int need = b[t];

        S[0] += A - need;

        if (need == 0) {
            // D_i = a_i and C_i = 1.
            continue;
        }

        // Start with C_i = 1, then subtract all Q_{A,k}, k < need.
        vector<int> C(A + 1, 0);
        C[0] = 1;

        for (int k = 0; k < need; ++k) {
            const int chooseAK = binom(A, k);
            const int multiplier = need - k;

            // Q_{A,k} = binom(A,k) * (1-x)^k * x^(A-k).
            for (int j = 0; j <= k; ++j) {
                int term = int(1LL * chooseAK * binom(k, j) % MOD);
                int degree = A - k + j;
                int dterm = int(1LL * multiplier * term % MOD);

                if (j & 1) {
                    // The coefficient of Q is negative.
                    S[degree] -= dterm;
                    if (S[degree] < 0) S[degree] += MOD;

                    C[degree] += term;
                    if (C[degree] >= MOD) C[degree] -= MOD;
                } else {
                    S[degree] += dterm;
                    if (S[degree] >= MOD) S[degree] -= MOD;

                    C[degree] -= term;
                    if (C[degree] < 0) C[degree] += MOD;
                }
            }
        }

        // Multiply P by C_i.
        vector<int> next(P.size() + A, 0);
        for (int i = 0; i < (int)P.size(); ++i) {
            if (P[i] == 0) continue;
            for (int j = 0; j <= A; ++j) {
                if (C[j] == 0) continue;
                next[i + j] = int((next[i + j] + 1LL * P[i] * C[j]) % MOD);
            }
        }
        P.swap(next);
    }

    vector<int> inv(n + 1, 0);
    inv[1] = 1;
    for (int i = 2; i <= n; ++i) {
        inv[i] = int(1LL * (MOD - MOD / i) * inv[MOD % i] % MOD);
    }

    long long answer = 0;
    const int scale = n - B;

    for (int d = 1; d <= n; ++d) {
        long long coef = S[d];
        if (d < (int)P.size()) {
            coef -= 1LL * scale * P[d] % MOD;
        }
        coef %= MOD;
        if (coef < 0) coef += MOD;

        answer = (answer + coef * inv[d]) % MOD;
    }

    cout << answer << '\n';
    return 0;
}

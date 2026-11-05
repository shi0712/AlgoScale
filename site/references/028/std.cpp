#include <bits/stdc++.h>
using namespace std;

using i64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, P;
    cin >> n >> P;

    const int m = n - 1;
    const int M = n * (n + 1) / 2;
    const int d = M - 1;
    const int lim = max(M, 2 * n + 2);

    // 所有需要的小整数逆元。
    vector<int> inv(lim + 1);
    inv[1] = 1;
    for (int i = 2; i <= lim; ++i) {
        inv[i] = P - (i64)(P / i) * inv[P % i] % P;
    }

    vector<int> L(n);
    for (int k = 0; k <= m; ++k) {
        L[k] = k * (k + 3) / 2;
    }

    // 同时返回 I(s,a) 和它的逆元。
    // 直接分别累乘分子、分母，不需要快速幂。
    auto integral = [&](int s, int a) -> pair<int, int> {
        i64 v = 1, iv = 1;

        for (int i = 1; i <= s; ++i) {
            v = v * inv[a + i] % P;
            iv = iv * (a + i) % P;

            for (int j = i + 1; j <= s; ++j) {
                int numerator = j - i;
                int denominator = 2 * a + i + j;

                v = v * numerator % P;
                v = v * inv[denominator] % P;

                iv = iv * denominator % P;
                iv = iv * inv[numerator] % P;
            }
        }

        return {(int)v, (int)iv};
    };

    // C[k]：G_k 的最低次项系数。
    // seed[k]：构造第 k 个基时，唯一非零的自由量。
    vector<int> C(n), invC(n), seed(n);

    for (int k = 0; k <= m; ++k) {
        auto [a, ia] = integral(k, 1);
        auto [b, ib] = integral(m - k, k + 1);

        C[k] = (i64)a * b % P;
        invC[k] = (i64)ia * ib % P;

        i64 v = C[k];

        for (int r = k + 1; r <= m; ++r) {
            v = v * (P + k - r) % P;
            v = v * (k + r + 3) % P;
            v = v * inv[r] % P;
            v = v * inv[m + r + 4] % P;
        }

        seed[k] = (int)v;
    }

    vector<int> D(n), K(n);

    for (int s = 0; s <= m; ++s) {
        D[s] = (i64)s * (m - s + 4) % P * inv[2] % P;

        K[s] = (i64)(m - s) * (2 * m + 4 - s)
             % P * inv[2] % P;
    }

    // B[k][q]：第 k 个基多项式的 z^q 系数。
    vector<vector<int>> B(n, vector<int>(M));

    // 辅助下标 s 存在 s+1 行。
    // 第 0 行和第 n+1 行作为全零哨兵。
    //
    // pre[s+1][k]：第 k 个基，上一层次数的辅助状态 s。
    // cur[s+1][k]：第 k 个基，当前层次数的辅助状态 s。
    vector<vector<int>> pre(n + 2, vector<int>(n));
    vector<vector<int>> cur(n + 2, vector<int>(n));

    for (int q = 0; q <= d; ++q) {
        for (auto &row : cur) {
            fill(row.begin(), row.end(), 0);
        }

        // 当前层依赖 s+1，因此 s 必须倒序。
        for (int s = m; s >= 0; --s) {
            int r = m - s;
            int u = s + 1;

            if (q == L[r]) {
                // 特殊位置：自由量。
                // 只有第 r 个基在这里取非零值。
                cur[u][r] = seed[r];
                continue;
            }

            int delta = q - L[r];
            int id = delta > 0
                   ? inv[delta]
                   : P - inv[-delta];

            int a = d + s - L[r] - q + 1;
            if (a < 0) a += P;

            for (int k = 0; k <= m; ++k) {
                // P < 1e9，三个乘积之和不会溢出 long long。
                i64 rhs = (i64)a * pre[u][k]
                        + (i64)D[s] * pre[u - 1][k]
                        + (i64)K[s] * cur[u + 1][k];

                cur[u][k] = (rhs % P) * id % P;
            }
        }

        // 只保存辅助状态 s=0，它就是所需基多项式。
        for (int k = 0; k <= m; ++k) {
            B[k][q] = cur[1][k];
        }

        pre.swap(cur);
    }

    // 倒序消元。
    // 处理完 k 后，B[k] 就是真正的目标多项式 G_k。
    for (int k = m; k >= 0; --k) {
        for (int r = m; r > k; --r) {
            int right = d - L[m - r];

            // G_r 的最高次项系数为 C[m-r]。
            int factor = (i64)B[k][right] * invC[m - r] % P;

            if (factor == 0) continue;

            // 已求出的 G_r 只在 [L[r], right] 内非零。
            for (int q = L[r]; q <= right; ++q) {
                int sub = (i64)factor * B[r][q] % P;

                B[k][q] -= sub;
                if (B[k][q] < 0) B[k][q] += P;
            }
        }
    }

    vector<int> fact(M + 1, 1);

    for (int i = 1; i <= M; ++i) {
        fact[i] = (i64)fact[i - 1] * i % P;
    }

    // 单点 {k+1} 位于第 q+1 位。
    for (int k = 0; k <= m; ++k) {
        for (int q = 0; q <= d; ++q) {
            int ans = (i64)B[k][q] * fact[q] % P
                    * fact[d - q] % P;

            if (q) cout << ' ';
            cout << ans;
        }
        cout << '\n';
    }

    return 0;
}

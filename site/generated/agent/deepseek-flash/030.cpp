#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<int> a(m), b(m);
    for (int i = 0; i < m; ++i) cin >> a[i];
    for (int i = 0; i < m; ++i) cin >> b[i];

    int B = 0, A = 0;
    for (int i = 0; i < m; ++i) {
        B += b[i];
        if (b[i] > 0) A += a[i];
    }
    if (B == 0) {
        cout << 0 << '\n';
        return 0;
    }

    // Binomial coefficients up to n.
    vector<vector<int>> C(n + 1);
    for (int i = 0; i <= n; ++i) {
        C[i].assign(i + 1, 0);
        C[i][0] = C[i][i] = 1;
        for (int j = 1; j < i; ++j) {
            int v = C[i - 1][j - 1] + C[i - 1][j];
            if (v >= MOD) v -= MOD;
            C[i][j] = v;
        }
    }

    // Q(p) = product over b_i > 0 of P(Bin(a_i,p) >= b_i).
    vector<int> Q(1, 1);

    for (int i = 0; i < m; ++i) {
        if (b[i] == 0) continue;
        int ai = a[i], bi = b[i];
        vector<int> Pi(ai + 1, 0);
        // P(Bin(ai,p) >= bi) = sum_{k=bi..ai} C(ai,k) p^k (1-p)^(ai-k)
        for (int k = bi; k <= ai; ++k) {
            long long coef = C[ai][k];
            int e = ai - k;
            const int* row = C[e].data();
            int base = k;
            for (int j = 0; j <= e; ++j) {
                long long val = coef * row[j] % MOD;
                int idx = base + j;
                if (j & 1) {
                    int x = Pi[idx] - (int)val;
                    if (x < 0) x += MOD;
                    Pi[idx] = x;
                } else {
                    int x = Pi[idx] + (int)val;
                    if (x >= MOD) x -= MOD;
                    Pi[idx] = x;
                }
            }
        }

        // Multiply Q by Pi.
        vector<int> R(Q.size() + Pi.size() - 1, 0);
        for (size_t x = 0; x < Q.size(); ++x) {
            if (Q[x] == 0) continue;
            long long qx = Q[x];
            for (size_t y = 0; y < Pi.size(); ++y) {
                if (Pi[y] == 0) continue;
                R[x + y] = (int)((R[x + y] + qx * Pi[y]) % MOD);
            }
        }
        Q.swap(R);
    }

    // S(p) = sum_i sum_{k=0}^{b_i-1} (b_i-k) C(a_i,k) p^k (1-p)^(a_i-k)
    vector<int> S(n + 1, 0);
    for (int i = 0; i < m; ++i) {
        int ai = a[i], bi = b[i];
        for (int k = 0; k < bi; ++k) {
            long long coef = (long long)C[ai][k] * (bi - k) % MOD;
            int e = ai - k;
            const int* row = C[e].data();
            int base = k;
            for (int j = 0; j <= e; ++j) {
                long long val = coef * row[j] % MOD;
                int idx = base + j;
                if (j & 1) {
                    int x = S[idx] - (int)val;
                    if (x < 0) x += MOD;
                    S[idx] = x;
                } else {
                    int x = S[idx] + (int)val;
                    if (x >= MOD) x -= MOD;
                    S[idx] = x;
                }
            }
        }
    }

    // T(p) = (n-B)(1-Q(p)) + S(p) = (1-p) * P(p)
    int D = n - B;
    vector<int> T(A + 1, 0);
    for (int j = 0; j <= A; ++j) {
        long long val = (j < (int)S.size() ? S[j] : 0);
        long long q = (j < (int)Q.size() ? Q[j] : 0);
        long long one = (j == 0 ? 1 : 0);
        val += (long long)D * ((one - q + MOD) % MOD) % MOD;
        val %= MOD;
        T[j] = (int)val;
    }

    // P(p) = T(p)/(1-p).  If T(p)=sum T_j p^j, then P_j = sum_{l<=j} T_l.
    // Answer = integral_0^1 P(p) dp.
    vector<int> inv(A + 1);
    if (A >= 1) {
        inv[1] = 1;
        for (int i = 2; i <= A; ++i) {
            inv[i] = (int)(MOD - (long long)(MOD / i) * inv[MOD % i] % MOD);
        }
    }

    long long ans = 0, pref = 0;
    for (int j = 0; j < A; ++j) {
        pref += T[j];
        if (pref >= MOD) pref -= MOD;
        ans += pref * inv[j + 1] % MOD;
        ans %= MOD;
    }

    cout << ans % MOD << '\n';
    return 0;
}

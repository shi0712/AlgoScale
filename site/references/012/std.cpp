#include <bits/stdc++.h>
using namespace std;
using int64 = long long;
using Poly = vector<int>;

template<int MOD, int G = 3>
struct NTT {
    static int power(int a, int e) {
        int r = 1;
        for (; e; e >>= 1, a = (int64)a * a % MOD)
            if (e & 1) r = (int64)r * a % MOD;
        return r;
    }

    static void transform(Poly &a, bool inverse) {
        const int n = (int)a.size();
        static Poly roots{0, 1};
        if ((int)roots.size() < n) {
            int k = __builtin_ctz((unsigned)roots.size());
            roots.resize(n);
            while ((1 << k) < n) {
                int z = power(G, (MOD - 1) >> (k + 1));
                for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                    roots[i << 1] = roots[i];
                    roots[i << 1 | 1] = (int64)roots[i] * z % MOD;
                }
                ++k;
            }
        }
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) swap(a[i], a[j]);
        }
        for (int k = 1; k < n; k <<= 1) {
            for (int i = 0; i < n; i += k << 1) {
                for (int j = 0; j < k; ++j) {
                    int u = a[i + j];
                    int v = (int64)a[i + j + k] * roots[k + j] % MOD;
                    int x = u + v;
                    if (x >= MOD) x -= MOD;
                    int y = u - v;
                    if (y < 0) y += MOD;
                    a[i + j] = x;
                    a[i + j + k] = y;
                }
            }
        }
        if (inverse) {
            reverse(a.begin() + 1, a.end());
            int z = power(n, MOD - 2);
            for (int &x : a) x = (int64)x * z % MOD;
        }
    }

    static Poly multiply(const Poly &a, const Poly &b, int need) {
        int len = 1;
        while (len < (int)a.size() + (int)b.size() - 1) len <<= 1;
        Poly x(len), y(len);
        for (int i = 0; i < (int)a.size(); ++i) x[i] = a[i] % MOD;
        for (int i = 0; i < (int)b.size(); ++i) y[i] = b[i] % MOD;
        transform(x, false);
        transform(y, false);
        for (int i = 0; i < len; ++i) x[i] = (int64)x[i] * y[i] % MOD;
        transform(x, true);
        x.resize(need);
        return x;
    }
};

struct Solver {
    static constexpr int M1 = 998244353;
    static constexpr int M2 = 1004535809;
    static constexpr int M3 = 469762049;
    int n, mod;
    Poly fact, ifact, inv;
    int inv12, inv123, m1p, m12p;

    static int power(int a, int e, int p) {
        int r = 1;
        for (; e; e >>= 1, a = (int64)a * a % p)
            if (e & 1) r = (int64)r * a % p;
        return r;
    }

    int add(int a, int b) const {
        a += b;
        if (a >= mod) a -= mod;
        return a;
    }
    int sub(int a, int b) const {
        a -= b;
        if (a < 0) a += mod;
        return a;
    }

    // Exact convolution modulo the input prime, truncated at x^limit.
    Poly mul(Poly a, Poly b, int limit) const {
        a.resize(min((int)a.size(), limit));
        b.resize(min((int)b.size(), limit));
        while (!a.empty() && a.back() == 0) a.pop_back();
        while (!b.empty() && b.back() == 0) b.pop_back();
        if (a.empty() || b.empty()) return Poly(limit, 0);

        int need = min(limit, (int)a.size() + (int)b.size() - 1);
        Poly c;
        if (min(a.size(), b.size()) <= 12) {
            c.assign(need, 0);
            for (int i = 0; i < (int)a.size(); ++i)
                for (int j = 0; j < (int)b.size() && i + j < need; ++j)
                    c[i + j] = (c[i + j] + (int64)a[i] * b[j]) % mod;
        } else if (mod == M1) {
            c = NTT<M1>::multiply(a, b, need);
        } else if (mod == M2) {
            c = NTT<M2>::multiply(a, b, need);
        } else if (mod == M3) {
            c = NTT<M3>::multiply(a, b, need);
        } else {
            Poly a1 = NTT<M1>::multiply(a, b, need);
            Poly a2 = NTT<M2>::multiply(a, b, need);
            Poly a3 = NTT<M3>::multiply(a, b, need);
            c.resize(need);
            for (int i = 0; i < need; ++i) {
                // Integer coefficient = a1 + M1*t + M1*M2*u.
                int64 t = (int64)(a2[i] - a1[i] + M2) * inv12 % M2;
                int64 r = (a1[i] + (int64)M1 * t) % M3;
                int64 u = (a3[i] - r + M3) * inv123 % M3;
                c[i] = (a1[i] % mod + (int64)m1p * (t % mod)
                        + (int64)m12p * (u % mod)) % mod;
            }
        }
        c.resize(limit, 0);
        return c;
    }

    int run(int input_n, int input_mod) {
        n = input_n;
        mod = input_mod;
        fact.assign(n + 1, 1);
        ifact.assign(n + 1, 1);
        inv.assign(n + 1, 1);
        for (int i = 2; i <= n; ++i)
            inv[i] = mod - (int64)(mod / i) * inv[mod % i] % mod;
        for (int i = 1; i <= n; ++i) {
            fact[i] = (int64)fact[i - 1] * i % mod;
            ifact[i] = (int64)ifact[i - 1] * inv[i] % mod;
        }
        inv12 = power(M1, M2 - 2, M2);
        inv123 = power((int64)M1 * M2 % M3, M3 - 2, M3);
        m1p = M1 % mod;
        m12p = (int64)M1 * M2 % mod;

        Poly D(n + 1, 0); // Ordinary coefficients of D(x).
        for (int m = 1; m < n + 1; ) {
            int N = min(2 * m, n + 1);
            int block = N - m;

            // h[r][t] = [x^(r+t) y^r] exp(x + (y-1)D(x)).
            int rows = (N - 1) / 2;
            vector<Poly> h(rows + 1);
            h[0].assign(N, 0);
            h[0][0] = 1;

            // h[0] = exp(x-D), computed in O(N^2).
            Poly dg(N, 0);
            for (int i = 1; i < N; ++i) {
                int gi = sub(i == 1 ? 1 : 0, D[i]);
                dg[i] = (int64)i * gi % mod;
            }
            for (int t = 1; t < N; ++t) {
                int value = 0;
                for (int i = 1; i <= t; ++i)
                    value = (value + (int64)dg[i] * h[0][t - i]) % mod;
                h[0][t] = (int64)value * inv[t] % mod;
            }

            Poly E(N - 1);
            for (int i = 0; i < N - 1; ++i) E[i] = D[i + 1];
            for (int r = 1; r <= rows; ++r) {
                h[r] = mul(h[r - 1], E, N - 2 * r);
                for (int &x : h[r]) x = (int64)x * inv[r] % mod;
            }

            // Residual R_s = [y^s](L(H)-1); only s>=m is needed.
            Poly R(N, 0);
            for (int r = 0; r <= rows; ++r) {
                for (int t = 0; t < (int)h[r].size(); ++t) {
                    int s = t + 2 * r;
                    R[s] = (R[s] + (int64)fact[t + r] * h[r][t]) % mod;
                }
            }
            R[0] = sub(R[0], 1);

            auto coeffH = [&](int a, int b) -> int {
                if (b < 0 || b > rows || a < b) return 0;
                int t = a - b;
                return t < (int)h[b].size() ? h[b][t] : 0;
            };

            // Store J_(m+i,m+j) / (m+i)! for j<i.
            vector<Poly> mat(block, Poly(block, 0));
            Poly W(fact.begin(), fact.begin() + N);
            for (int d = 1; d < block; ++d) {
                Poly Q((d + 1) / 2 + 1, 0);
                for (int r = 0; r < (int)Q.size(); ++r) {
                    int a = d - r;
                    Q[r] = sub(coeffH(a, r - 1), coeffH(a, r));
                }
                Poly C = mul(W, Q, N);
                for (int s = m + d; s < N; ++s)
                    mat[s - m][s - m - d] = (int64)C[s] * ifact[s] % mod;
            }

            // The diagonal of J is -s!, so forward substitution suffices.
            Poly delta(block, 0);
            for (int i = 0; i < block; ++i) {
                int value = (int64)R[m + i] * ifact[m + i] % mod;
                for (int j = 0; j < i; ++j)
                    value = (value + (int64)mat[i][j] * delta[j]) % mod;
                delta[i] = value;
                D[m + i] = value;
            }
            m = N;
        }
        return (int64)fact[n] * D[n] % mod;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, P;
    if (!(cin >> n >> P)) return 0;
    Solver solver;
    cout << solver.run(n, P) << '\n';
    return 0;
}

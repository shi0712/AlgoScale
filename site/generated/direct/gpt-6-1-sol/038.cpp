#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static constexpr int P1 = 998244353;
static constexpr int P2 = 1004535809;
static constexpr int P3 = 469762049;

int mod_pow(int a, int64 e, int mod) {
    int r = 1;
    while (e > 0) {
        if (e & 1) r = (int)((int64)r * a % mod);
        a = (int)((int64)a * a % mod);
        e >>= 1;
    }
    return r;
}

const vector<int>& bit_reverse_table(int n) {
    static vector<vector<int>> cache(23);
    int lg = __builtin_ctz((unsigned)n);
    auto& rev = cache[lg];
    if (rev.empty()) {
        rev.resize(n);
        for (int i = 1; i < n; ++i) {
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) ? (n >> 1) : 0);
        }
    }
    return rev;
}

template<int MOD>
struct NTT {
    static vector<int>& roots() {
        static vector<int> r{0, 1};
        return r;
    }

    static void ensure_roots(int n) {
        auto& r = roots();
        if ((int)r.size() >= n) return;

        int k = __builtin_ctz((unsigned)r.size());
        r.resize(n);
        while ((1 << k) < n) {
            int z = mod_pow(3, (MOD - 1) >> (k + 1), MOD);
            for (int i = 1 << (k - 1); i < (1 << k); ++i) {
                r[i << 1] = r[i];
                r[i << 1 | 1] = (int)((int64)r[i] * z % MOD);
            }
            ++k;
        }
    }

    static void transform(vector<int>& a) {
        int n = (int)a.size();
        ensure_roots(n);
        const auto& rev = bit_reverse_table(n);
        const auto& r = roots();

        for (int i = 0; i < n; ++i) {
            if (i < rev[i]) swap(a[i], a[rev[i]]);
        }

        for (int len = 1; len < n; len <<= 1) {
            for (int start = 0; start < n; start += len << 1) {
                for (int j = 0; j < len; ++j) {
                    int u = a[start + j];
                    int v = (int)((int64)a[start + j + len] * r[len + j] % MOD);

                    int s = u + v;
                    if (s >= MOD) s -= MOD;
                    int d = u - v;
                    if (d < 0) d += MOD;

                    a[start + j] = s;
                    a[start + j + len] = d;
                }
            }
        }
    }

    static void inverse(vector<int>& a) {
        reverse(a.begin() + 1, a.end());
        transform(a);
        int inv_n = mod_pow((int)a.size(), MOD - 2, MOD);
        for (int& x : a) x = (int)((int64)x * inv_n % MOD);
    }

    static vector<int> convolution(const vector<int>& a, const vector<int>& b) {
        if (a.empty() || b.empty()) return {};

        int need = (int)a.size() + (int)b.size() - 1;
        int n = 1;
        while (n < need) n <<= 1;

        vector<int> x(n), y(n);
        for (int i = 0; i < (int)a.size(); ++i) x[i] = a[i] % MOD;
        for (int i = 0; i < (int)b.size(); ++i) y[i] = b[i] % MOD;

        transform(x);
        transform(y);
        for (int i = 0; i < n; ++i) {
            x[i] = (int)((int64)x[i] * y[i] % MOD);
        }
        inverse(x);
        x.resize(need);
        return x;
    }

    // One bivariate Bostan-Mori step.
    //
    // Q and P are stored by rows:
    // coefficient x^i y^j is at j*(d+1)+i.
    //
    // Returns Q_new followed by P_new.
    static vector<int> bostan_step(
        const vector<int>& q,
        const vector<int>& p,
        int d, int qy, int py,
        int new_d, int new_qy, int new_py,
        int parity
    ) {
        int stride = 2 * (d + 1);
        int need = 2 * d + 2 * stride * qy + 1;
        int ntt_n = 1;
        while (ntt_n < need) ntt_n <<= 1;

        vector<int> a(ntt_n), b(ntt_n);

        for (int j = 0; j <= qy; ++j) {
            for (int i = 0; i <= d; ++i) {
                a[j * stride + i] = q[j * (d + 1) + i] % MOD;
            }
        }
        for (int j = 0; j <= py; ++j) {
            for (int i = 0; i <= d; ++i) {
                b[j * stride + i] = p[j * (d + 1) + i] % MOD;
            }
        }

        transform(a);
        transform(b);

        int half = ntt_n >> 1;
        const auto& r = roots();

        // Since stride is even, changing x to -x is the same
        // as changing the encoded univariate variable z to -z.
        for (int i = 0; i < half; ++i) {
            int a0 = a[i], a1 = a[i + half];
            int b0 = b[i], b1 = b[i + half];

            int u = (int)((int64)b0 * a1 % MOD);
            int v = (int)((int64)b1 * a0 % MOD);

            int z;
            if (parity == 0) {
                z = u + v;
                if (z >= MOD) z -= MOD;
            } else {
                z = u - v;
                if (z < 0) z += MOD;
            }

            if (z & 1) z += MOD;
            z >>= 1;

            if (parity != 0 && i != 0) {
                int inverse_root = MOD - r[ntt_n - i];
                z = (int)((int64)z * inverse_root % MOD);
            }

            a[i] = (int)((int64)a0 * a1 % MOD);
            b[i] = z;
        }

        a.resize(half);
        b.resize(half);
        inverse(a);
        inverse(b);

        int q_count = (new_d + 1) * (new_qy + 1);
        int p_count = (new_d + 1) * (new_py + 1);
        vector<int> result(q_count + p_count);

        // Dividing the encoded exponent by two changes the row
        // stride from 2*(d+1) to d+1.
        for (int j = 0; j <= new_qy; ++j) {
            for (int i = 0; i <= new_d; ++i) {
                result[j * (new_d + 1) + i] = a[j * (d + 1) + i];
            }
        }
        for (int j = 0; j <= new_py; ++j) {
            for (int i = 0; i <= new_d; ++i) {
                result[q_count + j * (new_d + 1) + i]
                    = b[j * (d + 1) + i];
            }
        }

        return result;
    }
};

struct CRT {
    int inv_p1_mod_p2;
    int inv_p12_mod_p3;
    int64 p12;
    i128 product;

    CRT() {
        inv_p1_mod_p2 = mod_pow(P1 % P2, P2 - 2, P2);
        p12 = (int64)P1 * P2;
        inv_p12_mod_p3 = mod_pow((int)(p12 % P3), P3 - 2, P3);
        product = (i128)p12 * P3;
    }

    int restore(int a, int b, int c, int mod) const {
        int64 t = b - (int64)a;
        if (t < 0) t += P2;
        t = t * inv_p1_mod_p2 % P2;

        int64 low = a + (int64)P1 * t;

        int64 u = c - low % P3;
        if (u < 0) u += P3;
        u = u * inv_p12_mod_p3 % P3;

        i128 value = low + (i128)p12 * u;

        // Products involving Q(-x,y) may have negative coefficients.
        if (value > product / 2) value -= product;

        int answer = (int)(value % mod);
        if (answer < 0) answer += mod;
        return answer;
    }
};

struct Solver {
    int mod;
    CRT crt;

    explicit Solver(int m) : mod(m) {}

    vector<int> combine(
        const vector<int>& a,
        const vector<int>& b,
        const vector<int>& c
    ) {
        vector<int> result(a.size());
        for (int i = 0; i < (int)a.size(); ++i) {
            result[i] = crt.restore(a[i], b[i], c[i], mod);
        }
        return result;
    }

    vector<int> convolution(const vector<int>& a, const vector<int>& b) {
        if (a.empty() || b.empty()) return {};

        if ((int64)a.size() * b.size() <= 4096) {
            vector<int> result(a.size() + b.size() - 1);
            for (int i = 0; i < (int)a.size(); ++i) {
                for (int j = 0; j < (int)b.size(); ++j) {
                    result[i + j] = (result[i + j]
                        + (int64)a[i] * b[j]) % mod;
                }
            }
            return result;
        }

        if (mod == P1) return NTT<P1>::convolution(a, b);
        if (mod == P2) return NTT<P2>::convolution(a, b);
        if (mod == P3) return NTT<P3>::convolution(a, b);

        auto x = NTT<P1>::convolution(a, b);
        auto y = NTT<P2>::convolution(a, b);
        auto z = NTT<P3>::convolution(a, b);
        return combine(x, y, z);
    }

    vector<int> step(
        const vector<int>& q,
        const vector<int>& p,
        int d, int qy, int py,
        int new_d, int new_qy, int new_py
    ) {
        int parity = d & 1;

        if (mod == P1) {
            return NTT<P1>::bostan_step(
                q, p, d, qy, py, new_d, new_qy, new_py, parity);
        }
        if (mod == P2) {
            return NTT<P2>::bostan_step(
                q, p, d, qy, py, new_d, new_qy, new_py, parity);
        }
        if (mod == P3) {
            return NTT<P3>::bostan_step(
                q, p, d, qy, py, new_d, new_qy, new_py, parity);
        }

        auto a = NTT<P1>::bostan_step(
            q, p, d, qy, py, new_d, new_qy, new_py, parity);
        auto b = NTT<P2>::bostan_step(
            q, p, d, qy, py, new_d, new_qy, new_py, parity);
        auto c = NTT<P3>::bostan_step(
            q, p, d, qy, py, new_d, new_qy, new_py, parity);

        return combine(a, b, c);
    }

    // Computes h[k] = [x^n] H(x) * (x H(x))^k, for all k.
    vector<int> power_projection(const vector<int>& h, int n) {
        int d = n;
        int qy = 1, py = 0;

        vector<int> p = h;
        vector<int> q(2 * (n + 1));
        q[0] = 1;
        for (int i = 1; i <= n; ++i) {
            q[n + 1 + i] = h[i - 1] == 0 ? 0 : mod - h[i - 1];
        }

        while (d > 1) {
            int new_d = d >> 1;
            int new_qy = min(n, 2 * qy);
            int new_py = min(n, qy + py);

            auto result = step(
                q, p, d, qy, py, new_d, new_qy, new_py);

            int q_count = (new_d + 1) * (new_qy + 1);
            q.assign(result.begin(), result.begin() + q_count);
            p.assign(result.begin() + q_count, result.end());

            d = new_d;
            qy = new_qy;
            py = new_py;
        }

        // Here Q(x,y) = 1 + x Q1(y), modulo x^2.
        // Therefore [x] P/Q = P1 - P0*Q1.
        vector<int> p0(py + 1), p1(py + 1), q1(qy + 1);
        for (int j = 0; j <= py; ++j) {
            p0[j] = p[2 * j];
            p1[j] = p[2 * j + 1];
        }
        for (int j = 0; j <= qy; ++j) {
            q1[j] = q[2 * j + 1];
        }

        auto product = convolution(p0, q1);
        vector<int> result(n + 1);
        for (int k = 0; k <= n; ++k) {
            int value = k < (int)p1.size() ? p1[k] : 0;
            if (k < (int)product.size()) {
                value -= product[k];
                if (value < 0) value += mod;
            }
            result[k] = value;
        }
        return result;
    }

    vector<int> solve(int n) {
        vector<int> fact(2 * n + 2), inv_fact(2 * n + 2);
        fact[0] = 1;
        for (int i = 1; i < (int)fact.size(); ++i) {
            fact[i] = (int)((int64)fact[i - 1] * i % mod);
        }

        inv_fact.back() = mod_pow(fact.back(), mod - 2, mod);
        for (int i = (int)fact.size() - 1; i >= 1; --i) {
            inv_fact[i - 1] = (int)((int64)inv_fact[i] * i % mod);
        }

        vector<int> h(n + 1);
        for (int i = 0; i <= n; ++i) {
            int catalan = (int)((int64)fact[2 * i] * inv_fact[i] % mod
                               * inv_fact[i + 1] % mod);
            h[i] = (int)((int64)catalan * catalan % mod);
        }

        auto projection = power_projection(h, n);

        // b[k] counts pairs together with a choice of k common pairs.
        vector<int> b(n + 1);
        for (int k = 0; k <= n; ++k) {
            // binom(2n,k)/(k+1)
            // = (2n)! / ((2n-k)! (k+1)!).
            int factor = (int)((int64)fact[2 * n] * inv_fact[2 * n - k] % mod
                              * inv_fact[k + 1] % mod);
            b[k] = (int)((int64)projection[k] * factor % mod);
        }

        // Taylor shift: A(z) = B(z-1).
        vector<int> u(n + 1), v(n + 1);
        for (int j = 0; j <= n; ++j) {
            u[n - j] = (int)((int64)b[j] * fact[j] % mod);
            v[j] = (j & 1) && inv_fact[j] != 0
                 ? mod - inv_fact[j] : inv_fact[j];
        }

        auto w = convolution(u, v);
        vector<int> answer(n + 1);
        for (int k = 0; k <= n; ++k) {
            answer[k] = (int)((int64)w[n - k] * inv_fact[k] % mod);
        }
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    cin >> tests;
    while (tests--) {
        int n, mod;
        cin >> n >> mod;

        Solver solver(mod);
        auto answer = solver.solve(n);

        for (int k = 0; k <= n; ++k) {
            if (k) cout << ' ';
            cout << answer[k];
        }
        cout << '\n';
    }

    return 0;
}

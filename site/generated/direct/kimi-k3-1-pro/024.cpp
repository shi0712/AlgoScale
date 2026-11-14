#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int PRIMITIVE_ROOT = 3;

int mod_pow(int a, long long e) {
    long long r = 1;
    long long x = a;
    while (e > 0) {
        if (e & 1) r = r * x % MOD;
        x = x * x % MOD;
        e >>= 1;
    }
    return static_cast<int>(r);
}

void ntt(vector<int>& a, bool invert) {
    const int n = static_cast<int>(a.size());

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        int wlen = mod_pow(PRIMITIVE_ROOT, (MOD - 1) / len);
        if (invert) wlen = mod_pow(wlen, MOD - 2);

        const int half = len >> 1;
        for (int i = 0; i < n; i += len) {
            int w = 1;
            for (int j = 0; j < half; ++j) {
                int u = a[i + j];
                int v = static_cast<int>(
                    1LL * a[i + j + half] * w % MOD
                );

                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;

                a[i + j] = x;
                a[i + j + half] = y;
                w = static_cast<int>(1LL * w * wlen % MOD);
            }
        }
    }

    if (invert) {
        int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) {
            x = static_cast<int>(1LL * x * inv_n % MOD);
        }
    }
}

// Returns the first `need` coefficients, or the full product if need < 0.
vector<int> multiply(const vector<int>& a, const vector<int>& b, int need = -1) {
    if (a.empty() || b.empty()) return {};

    const long long full_size_ll =
        static_cast<long long>(a.size()) + b.size() - 1;
    const int full_size = static_cast<int>(full_size_ll);
    const int result_size = need < 0
        ? full_size
        : static_cast<int>(min<long long>(full_size, need));

    if (result_size == 0) return {};

    if (min(a.size(), b.size()) <= 32 ||
        1LL * a.size() * b.size() <= 5000) {
        vector<int> result(result_size, 0);
        for (int i = 0; i < static_cast<int>(a.size()) && i < result_size; ++i) {
            int limit = min(
                static_cast<int>(b.size()),
                result_size - i
            );
            for (int j = 0; j < limit; ++j) {
                int& dest = result[i + j];
                dest = static_cast<int>(
                    (dest + 1LL * a[i] * b[j]) % MOD
                );
            }
        }
        return result;
    }

    int n = 1;
    while (n < full_size) n <<= 1;

    vector<int> fa(a.begin(), a.end());
    vector<int> fb(b.begin(), b.end());
    fa.resize(n);
    fb.resize(n);

    ntt(fa, false);
    ntt(fb, false);
    for (int i = 0; i < n; ++i) {
        fa[i] = static_cast<int>(1LL * fa[i] * fb[i] % MOD);
    }
    ntt(fa, true);

    fa.resize(result_size);
    return fa;
}

// All series passed here have constant term 1.
vector<int> inverse_series(const vector<int>& a, int n) {
    vector<int> r(1, 1);
    int k = 1;

    while (k < n) {
        int t = min(2 * k, n);

        vector<int> ac(a.begin(), a.begin() + t);
        vector<int> ar = multiply(ac, r, t);

        vector<int> correction(t);
        for (int i = 0; i < t; ++i) {
            correction[i] = (i == 0 ? 2 : 0) - ar[i];
            if (correction[i] < 0) correction[i] += MOD;
        }

        r = multiply(r, correction, t);
        k = t;
    }

    return r;
}

// Computes exp(f), where f[0] = 0.
vector<int> exp_series(
    const vector<int>& f,
    int n,
    const vector<int>& integer_inverse
) {
    if (n == 1) return {1};

    vector<int> g(1, 1);
    int k = 1;

    while (k < n) {
        int t = min(2 * k, n);

        // gp is the current approximation, padded with zeros.
        vector<int> gp(t, 0);
        copy(g.begin(), g.end(), gp.begin());

        vector<int> inv_gp = inverse_series(gp, t);

        // log(gp) = integral(gp' / gp).
        vector<int> derivative;
        if (k > 1) {
            derivative.resize(k - 1);
            for (int i = 1; i < k; ++i) {
                derivative[i - 1] =
                    static_cast<int>(1LL * i * g[i] % MOD);
            }
        }

        vector<int> z(t - 1, 0);
        if (!derivative.empty()) {
            z = multiply(derivative, inv_gp, t - 1);
        }

        vector<int> log_gp(t, 0);
        for (int i = 1; i < t; ++i) {
            log_gp[i] = static_cast<int>(
                1LL * z[i - 1] * integer_inverse[i] % MOD
            );
        }

        vector<int> residual(t);
        for (int i = 0; i < t; ++i) {
            residual[i] = f[i] - log_gp[i];
            if (residual[i] < 0) residual[i] += MOD;
        }
        for (int i = 0; i < k; ++i) residual[i] = 0;

        vector<int> delta = multiply(g, residual, t);

        g = gp;
        for (int i = 0; i < t; ++i) {
            g[i] += delta[i];
            if (g[i] >= MOD) g[i] -= MOD;
        }

        k = t;
    }

    return g;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> a(n);
    long long total = 0;
    int active = 0;

    for (int& x : a) {
        cin >> x;
        total += x;
        if (x > 0) ++active;
    }

    const int m = static_cast<int>(total);
    const int B = m / 2;

    if (B == 0) {
        cout << 1 << '\n';
        return 0;
    }

    vector<int> integer_inverse(B + 1, 0);
    integer_inverse[1] = 1;
    for (int i = 2; i <= B; ++i) {
        integer_inverse[i] = static_cast<int>(
            MOD - 1LL * (MOD / i) * integer_inverse[MOD % i] % MOD
        );
    }

    // P(x) = product (1 - x^(a_i+1)) / (1 - x), over a_i > 0.
    // If L = log P, then
    // d * [x^d]L = active - sum_{(a_i+1) | d} (a_i+1).
    vector<int> frequency(B + 1, 0);
    for (int x : a) {
        int len = x + 1;
        if (x > 0 && len <= B) ++frequency[len];
    }

    vector<long long> c(B + 1, active);
    c[0] = 0;

    for (int len = 1; len <= B; ++len) {
        if (frequency[len] == 0) continue;
        long long contribution = 1LL * len * frequency[len];
        for (int d = len; d <= B; d += len) {
            c[d] -= contribution;
        }
    }

    vector<int> log_p(B + 1, 0);
    for (int d = 1; d <= B; ++d) {
        long long value = c[d] % MOD;
        if (value < 0) value += MOD;
        log_p[d] = static_cast<int>(
            value * integer_inverse[d] % MOD
        );
    }

    vector<int> D = exp_series(log_p, B + 1, integer_inverse);

    long long answer = 0;
    int binomial = 1;       // C(m,b)
    int previous = 0;       // C(m,b-1)

    for (int b = 0; b <= B; ++b) {
        if (b > 0) {
            previous = binomial;
            binomial = static_cast<int>(
                1LL * binomial * (m - b + 1) % MOD
                * integer_inverse[b] % MOD
            );
        }

        int f = binomial - previous;
        if (f < 0) f += MOD;

        int difference = D[b] - (b == 0 ? 0 : D[b - 1]);
        if (difference < 0) difference += MOD;

        answer = (answer + 1LL * f * difference) % MOD;
    }

    cout << answer << '\n';
    return 0;
}

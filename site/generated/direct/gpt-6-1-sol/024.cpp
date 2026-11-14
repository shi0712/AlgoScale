#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int PRIMITIVE_ROOT = 3;

int modPow(int a, int e) {
    int result = 1;
    while (e > 0) {
        if (e & 1) result = (long long)result * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return result;
}

class NTT {
    vector<int> roots;
    vector<int> rev;
    int revSize = 0;

public:
    explicit NTT(int maxSize) : roots(maxSize) {
        for (int len = 1; len < maxSize; len <<= 1) {
            int w = modPow(PRIMITIVE_ROOT, (MOD - 1) / (len << 1));
            roots[len] = 1;
            for (int j = 1; j < len; ++j) {
                roots[len + j] =
                    (long long)roots[len + j - 1] * w % MOD;
            }
        }
    }

    void transform(vector<int>& a, bool inverse = false) {
        int n = (int)a.size();

        if (inverse) {
            reverse(a.begin() + 1, a.end());
        }

        if (revSize != n) {
            revSize = n;
            rev.resize(n);
            rev[0] = 0;
            for (int i = 1; i < n; ++i) {
                rev[i] = (rev[i >> 1] >> 1) |
                         ((i & 1) ? (n >> 1) : 0);
            }
        }

        for (int i = 0; i < n; ++i) {
            if (i < rev[i]) swap(a[i], a[rev[i]]);
        }

        for (int len = 1; len < n; len <<= 1) {
            for (int start = 0; start < n; start += len << 1) {
                for (int j = 0; j < len; ++j) {
                    int u = a[start + j];
                    int v = (long long)a[start + j + len] *
                            roots[len + j] % MOD;

                    int x = u + v;
                    if (x >= MOD) x -= MOD;

                    int y = u - v;
                    if (y < 0) y += MOD;

                    a[start + j] = x;
                    a[start + j + len] = y;
                }
            }
        }

        if (inverse) {
            int invN = modPow(n, MOD - 2);
            for (int& x : a) {
                x = (long long)x * invN % MOD;
            }
        }
    }
};

// Computes exp(f) modulo x^N.
// N is a power of two, and f[0] = 0.
vector<int> polynomialExp(
    const vector<int>& f,
    const vector<int>& invInteger,
    NTT& ntt
) {
    int N = (int)f.size();

    vector<int> g(1, 1); // exp(f) modulo x^k
    vector<int> h(1, 1); // 1/g modulo x^k

    for (int k = 1; k < N; k <<= 1) {
        int len = k << 1;

        vector<int> A(len, 0), B(len, 0), E(len, 0);
        vector<int> extendedInverse(len, 0), H(len, 0), D(len, 0);

        copy(g.begin(), g.end(), A.begin());
        copy(h.begin(), h.end(), B.begin());

        ntt.transform(A);
        ntt.transform(B);

        // Extend 1/g from precision k to precision 2k.
        //
        // Only the upper half of g*h - 1 is needed. Cyclic
        // convolution of length 2k gives that half without aliasing.
        for (int i = 0; i < len; ++i) {
            E[i] = (long long)A[i] * B[i] % MOD;
        }
        ntt.transform(E, true);

        fill(E.begin(), E.begin() + k, 0);
        ntt.transform(E);
        for (int i = 0; i < len; ++i) {
            E[i] = (long long)E[i] * B[i] % MOD;
        }
        ntt.transform(E, true);

        for (int i = 0; i < k; ++i) {
            extendedInverse[i] = h[i];
        }
        for (int i = k; i < len; ++i) {
            extendedInverse[i] = E[i] == 0 ? 0 : MOD - E[i];
        }

        // Compute the required upper half of log(g), using
        // (log g)' = g' / g.
        fill(E.begin(), E.end(), 0);
        for (int i = 1; i < k; ++i) {
            E[i - 1] = (long long)i * g[i] % MOD;
        }

        H = extendedInverse;
        ntt.transform(E);
        ntt.transform(H);
        for (int i = 0; i < len; ++i) {
            E[i] = (long long)E[i] * H[i] % MOD;
        }
        ntt.transform(E, true);

        // D = f - log(g), whose coefficients below k are zero.
        for (int i = k; i < len; ++i) {
            int logCoefficient =
                (long long)E[i - 1] * invInteger[i] % MOD;
            D[i] = f[i] - logCoefficient;
            if (D[i] < 0) D[i] += MOD;
        }
        ntt.transform(D);

        // g_new = g * (1 + D) modulo x^(2k).
        for (int i = 0; i < len; ++i) {
            H[i] = (long long)A[i] * D[i] % MOD;
        }
        ntt.transform(H, true);

        g.resize(len);
        for (int i = k; i < len; ++i) {
            g[i] = H[i];
        }

        if (len < N) {
            // Since D^2 = 0 modulo x^(2k),
            // 1/g_new = (1/g) * (1 - D).
            for (int i = 0; i < len; ++i) {
                H[i] = (long long)B[i] * D[i] % MOD;
            }
            ntt.transform(H, true);

            h.resize(len);
            for (int i = k; i < len; ++i) {
                h[i] = extendedInverse[i] - H[i];
                if (h[i] < 0) h[i] += MOD;
            }
        }
    }

    return g;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    // frequency[d] = number of positive multiplicities equal to d - 1.
    vector<int> frequency(n + 2, 0);

    int m = 0;
    int distinctValues = 0;
    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;
        m += a;
        if (a > 0) {
            ++distinctValues;
            ++frequency[a + 1];
        }
    }

    int limit = m / 2;
    if (limit == 0) {
        cout << 1 << '\n';
        return 0;
    }

    int N = 1;
    while (N <= limit) N <<= 1;

    vector<int> invInteger(N, 0);
    invInteger[1] = 1;
    for (int i = 2; i < N; ++i) {
        invInteger[i] =
            MOD - (long long)(MOD / i) * invInteger[MOD % i] % MOD;
    }

    // For C(x) = product (1 + x + ... + x^a),
    //
    // [x^j] log C(x)
    //   = (distinctValues - sum_{d|j} d * frequency[d]) / j.
    vector<int> logarithm(N, distinctValues);
    logarithm[0] = 0;

    int maxD = min(n + 1, N - 1);
    for (int d = 2; d <= maxD; ++d) {
        if (frequency[d] == 0) continue;

        int contribution = d * frequency[d];
        for (int j = d; j < N; j += d) {
            logarithm[j] -= contribution;
        }
    }

    for (int j = 1; j < N; ++j) {
        int value = logarithm[j];
        if (value < 0) value += MOD;
        logarithm[j] = (long long)value * invInteger[j] % MOD;
    }

    NTT ntt(N);
    vector<int> coefficients =
        polynomialExp(logarithm, invInteger, ntt);

    // Sum (c_k - c_{k-1}) *
    //     (binom(m,k) - binom(m,k-1)).
    int answer = 0;
    int previousCoefficient = 0;
    int previousBinomial = 0;
    int binomial = 1;

    for (int k = 0; k <= limit; ++k) {
        if (k > 0) {
            binomial = (long long)binomial * (m - k + 1) % MOD;
            binomial = (long long)binomial * invInteger[k] % MOD;
        }

        int waysP = coefficients[k] - previousCoefficient;
        if (waysP < 0) waysP += MOD;

        int waysQ = binomial - previousBinomial;
        if (waysQ < 0) waysQ += MOD;

        answer = (answer + (long long)waysP * waysQ) % MOD;

        previousCoefficient = coefficients[k];
        previousBinomial = binomial;
    }

    cout << answer << '\n';
    return 0;
}

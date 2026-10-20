#include <bits/stdc++.h>
#include <boost/multiprecision/cpp_dec_float.hpp>

using namespace std;
using boost::multiprecision::cpp_dec_float_100;
using Real = cpp_dec_float_100;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    long long n_ll, m;
    cin >> n_ll >> m;

    int n = (int)n_ll;
    int L = n + 1;              // number of full gaps (including slack)
    long long K = m - n_ll;     // total sum of y variables

    vector<vector<Real>> C(L + 1, vector<Real>(L + 1, Real(0)));
    for (int i = 0; i <= L; ++i) {
        C[i][0] = C[i][i] = Real(1);
        for (int j = 1; j < i; ++j) {
            C[i][j] = C[i - 1][j - 1] + C[i - 1][j];
        }
    }

    // s[d] = S_d / T, where
    // S_d = sum_{a>=1} C(K - d*a + L - 1, L - 1),
    // T   = C(K + L - 1, L - 1).
    vector<Real> s(L + 1, Real(0));

    if (L >= 2) {
        // d = 1 has the closed form K / L.
        s[1] = Real(K) / Real(L);
    }

    const long long DIRECT_LIMIT = 100000;

    if (K <= DIRECT_LIMIT) {
        // Exact computation by direct summation.  Here K is small.
        int maxFact = (int)(K + L + 2);
        vector<Real> fact(maxFact + 1, Real(1));
        for (int i = 1; i <= maxFact; ++i) {
            fact[i] = fact[i - 1] * Real(i);
        }

        Real T = fact[(int)(K + L - 1)] / fact[L - 1] / fact[(int)K];

        for (int d = 2; d <= L; ++d) {
            if ((long long)d > K) {
                s[d] = Real(0);
                continue;
            }

            long long A = K / d;
            Real sum = 0;

            for (long long a = 1; a <= A; ++a) {
                long long rem = K - d * a;       // = K - d*a >= 0
                long long N = rem + L - 1;       // upper binomial index
                Real comb = fact[(int)N] / fact[L - 1] / fact[(int)rem];
                sum += comb;
            }

            s[d] = sum / T;
        }
    } else {
        // K is large.  Use the principal part at x=1.
        // The remaining periodic part is negligible far beyond
        // the required 1e-6 precision.
        auto comb_large = [](long long N, int r) -> Real {
            if (r < 0 || N < r) return Real(0);
            Real res = 1;
            for (int i = 1; i <= r; ++i) {
                res *= Real(N - r + i);
                res /= Real(i);
            }
            return res;
        };

        Real T = comb_large(K + L - 1, L - 1);

        for (int d = 2; d <= L; ++d) {
            long long nn = K - d; // here nn >= 0 because K is large
            if (nn < 0) {
                s[d] = Real(0);
                continue;
            }

            // B[s] = C(nn + s, s), s = 0..L
            vector<Real> B(L + 1, Real(1));
            B[0] = Real(1);
            for (int q = 1; q <= L; ++q) {
                B[q] = B[q - 1] * Real(nn + q) / Real(q);
            }

            // H(y) = (1 - (1-y)^d) / (d*y)
            // H_j = (-1)^j * C(d, j+1) / d
            vector<Real> H(d + 1, Real(0));
            for (int j = 0; j <= d - 1; ++j) {
                Real val = C[d][j + 1] / Real(d);
                if (j & 1) val = -val;
                H[j] = val;
            }

            // h = 1 / H, up to degree L
            vector<Real> h(L + 1, Real(0));
            h[0] = Real(1);
            for (int r = 1; r <= L; ++r) {
                Real sum = 0;
                int up = min(r, d - 1);
                for (int j = 1; j <= up; ++j) {
                    sum += H[j] * h[r - j];
                }
                h[r] = -sum;
            }

            // Principal part coefficient
            Real P = 0;
            for (int r = 0; r <= L; ++r) {
                P += h[r] * B[L - r];
            }
            P /= Real(d);

            s[d] = P / T;
        }
    }

    // Q[c] = sum_{a>=1} P(exactly c parts are < a).
    // Q[c] = C(L,c) * sum_{r=0}^c (-1)^r C(c,r) s[L-c+r].
    vector<Real> Q(L + 1, Real(0));
    for (int c = 0; c <= L - 1; ++c) {
        Real sum = 0;
        for (int r = 0; r <= c; ++r) {
            int d = L - c + r;
            Real term = C[c][r] * s[d];
            if (r & 1) sum -= term;
            else       sum += term;
        }
        Q[c] = C[L][c] * sum;
    }

    // Ppref[j] = E[sum of j smallest y's], for j=0..L.
    vector<Real> Ppref(L + 1, Real(0));
    Real cumulativeQ = 0;
    for (int j = 1; j <= L; ++j) {
        cumulativeQ += Q[j - 1];
        Ppref[j] = Ppref[j - 1] + cumulativeQ;
    }

    cout.setf(ios::fixed);
    cout << setprecision(10);

    // For each prefix length i, the observed n gaps are the full L gaps
    // after deleting the slack uniformly.  The expected sum of the first i
    // observed sorted y's is:
    // ((n-i) * Ppref[i] + i * Ppref[i+1]) / (n+1)
    for (int i = 1; i <= n; ++i) {
        Real observedY = (Real(n - i) * Ppref[i] + Real(i) * Ppref[i + 1]) / Real(L);
        Real answer = Real(i) + observedY;

        long double out = answer.convert_to<long double>();
        if (fabsl(out) < 1e-12) out = 0;
        cout << out << '\n';
    }

    return 0;
}

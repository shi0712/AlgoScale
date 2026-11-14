#include <bits/stdc++.h>
using namespace std;

struct Complex {
    double re, im;

    Complex(double re = 0.0, double im = 0.0) : re(re), im(im) {}

    Complex operator*(const Complex& other) const {
        return {
            re * other.re - im * other.im,
            re * other.im + im * other.re
        };
    }
};

class FFT {
    int n;
    vector<int> rev;
    vector<Complex> roots;

public:
    explicit FFT(int size) : n(size), rev(size), roots(size / 2) {
        for (int i = 1; i < n; ++i) {
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) ? (n >> 1) : 0);
        }

        const double pi = acos(-1.0);
        for (int i = 0; i < n / 2; ++i) {
            double angle = -2.0 * pi * i / n;
            roots[i] = Complex(cos(angle), sin(angle));
        }
    }

    void transform(vector<Complex>& a, bool inverse = false) const {
        // Reversing all indices except zero changes the sign of the exponent.
        if (inverse) {
            reverse(a.begin() + 1, a.end());
        }

        for (int i = 0; i < n; ++i) {
            if (i < rev[i]) {
                swap(a[i], a[rev[i]]);
            }
        }

        for (int len = 2; len <= n; len <<= 1) {
            int half = len >> 1;
            int step = n / len;

            for (int start = 0; start < n; start += len) {
                for (int j = 0; j < half; ++j) {
                    const Complex& w = roots[j * step];
                    const Complex u = a[start + j];
                    const Complex v = a[start + j + half] * w;

                    a[start + j] = Complex(u.re + v.re, u.im + v.im);
                    a[start + j + half] =
                        Complex(u.re - v.re, u.im - v.im);
                }
            }
        }

        if (inverse) {
            double scale = 1.0 / n;
            for (Complex& z : a) {
                z.re *= scale;
                z.im *= scale;
            }
        }
    }
};

vector<Complex> arbitraryDFT(const vector<int>& g) {
    int n = static_cast<int>(g.size());

    if ((n & (n - 1)) == 0) {
        vector<Complex> result(n);
        for (int i = 0; i < n; ++i) {
            result[i].re = g[i];
        }

        FFT fft(n);
        fft.transform(result);
        return result;
    }

    // Bluestein's algorithm.
    int m = 1;
    while (m < 2 * n - 1) {
        m <<= 1;
    }

    vector<Complex> a(m), b(m), chirp(n);
    const double pi = acos(-1.0);

    for (int i = 0; i < n; ++i) {
        // Reduce i^2 modulo 2n before evaluating trigonometric functions.
        long long residue = 1LL * i * i % (2LL * n);
        double angle = pi * static_cast<double>(residue) / n;

        chirp[i] = Complex(cos(angle), sin(angle));
        a[i] = Complex(g[i] * chirp[i].re, -g[i] * chirp[i].im);

        b[i] = chirp[i];
        if (i != 0) {
            b[m - i] = chirp[i];
        }
    }

    FFT fft(m);
    fft.transform(a);
    fft.transform(b);

    for (int i = 0; i < m; ++i) {
        a[i] = a[i] * b[i];
    }

    fft.transform(a, true);

    vector<Complex> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = a[i] * Complex(chirp[i].re, -chirp[i].im);
    }

    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<int> f(n);
    for (int& x : f) {
        cin >> x;
    }

    // Shift index k to zero, and temporarily set the replaceable entry to zero.
    vector<int> g(n, 0);
    long long S = 0;

    for (int s = 1; s < n; ++s) {
        g[s] = f[(k + s) % n];
        S += abs(g[s]);
    }

    if (S == 0) {
        cout << "0.000000000000000\n";
        return 0;
    }

    vector<Complex> spectrum = arbitraryDFT(g);

    vector<double> realPart(n), imagSquared(n);
    for (int t = 0; t < n; ++t) {
        realPart[t] = spectrum[t].re;
        imagSquared[t] = spectrum[t].im * spectrum[t].im;
    }

    // Q(x) = max_t ((x + Re(G_t))^2 + Im(G_t)^2).
    // Q is convex, so its values on the integers form a convex sequence.
    long long low = -S, high = S;

    while (low < high) {
        long long mid = low + (high - low) / 2;
        double x = static_cast<double>(mid);
        double q0 = 0.0, q1 = 0.0;

        for (int t = 0; t < n; ++t) {
            double u = x + realPart[t];
            q0 = max(q0, u * u + imagSquared[t]);

            u += 1.0;
            q1 = max(q1, u * u + imagSquared[t]);
        }

        if (q0 <= q1) {
            high = mid;
        } else {
            low = mid + 1;
        }
    }

    double bestSquared = 0.0;
    double x = static_cast<double>(low);

    for (int t = 0; t < n; ++t) {
        double u = x + realPart[t];
        bestSquared = max(bestSquared, u * u + imagSquared[t]);
    }

    cout << fixed << setprecision(15) << sqrt(bestSquared) << '\n';
    return 0;
}

#include <algorithm>
#include <cmath>
#include <complex>
#include <iomanip>
#include <iostream>
#include <vector>

using namespace std;

static void fft(vector<complex<double>>& a, bool inverse = false) {
    const int n = static_cast<int>(a.size());
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    const double pi = acos(-1.0);
    for (int len = 2; len <= n; len <<= 1) {
        double angle = (inverse ? 2.0 : -2.0) * pi / len;
        complex<double> step(cos(angle), sin(angle));
        for (int i = 0; i < n; i += len) {
            complex<double> w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                complex<double> u = a[i + j];
                complex<double> v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= step;
            }
        }
    }
    if (inverse) {
        for (complex<double>& value : a) value /= n;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<int> f(n);
    for (int& value : f) cin >> value;
    f[k] = 0;

    int size = 1;
    while (size < 2 * n - 1) size <<= 1;
    vector<complex<double>> a(size), b(size);
    const double pi = acos(-1.0);
    for (int i = 0; i < n; ++i) {
        double angle = pi * (1LL * i * i % (2 * n)) / n;
        complex<double> chirp(cos(angle), sin(angle));
        a[i] = static_cast<double>(f[i]) * conj(chirp);
        b[n - 1 + i] = chirp;
        b[n - 1 - i] = chirp;
    }
    fft(a);
    fft(b);
    for (int i = 0; i < size; ++i) a[i] *= b[i];
    fft(a, true);
    for (int t = 0; t < n; ++t) {
        double angle = pi * (1LL * t * t % (2 * n)) / n;
        a[t] = a[t + n - 1] * conj(complex<double>(cos(angle), sin(angle)));
    }
    a.resize(n);

    vector<double> ux(n), uy(n);
    for (int t = 0; t < n; ++t) {
        long long index = 1LL * k * t % n;
        double angle = -2.0 * pi * index / n;
        ux[t] = cos(angle);
        uy[t] = sin(angle);
    }

    auto objective = [&](long long x) {
        double result = 0.0;
        for (int t = 0; t < n; ++t) {
            double re = a[t].real() + x * ux[t];
            double im = a[t].imag() + x * uy[t];
            result = max(result, re * re + im * im);
        }
        return result;
    };

    long long bound = ceil(sqrt(objective(0)));
    long long lo = -bound, hi = bound;
    while (hi - lo > 3) {
        long long m1 = lo + (hi - lo) / 3;
        long long m2 = hi - (hi - lo) / 3;
        if (objective(m1) <= objective(m2))
            hi = m2;
        else
            lo = m1;
    }

    double answer = objective(lo);
    for (long long x = lo + 1; x <= hi; ++x)
        answer = min(answer, objective(x));
    cout << setprecision(17) << sqrt(answer) << '\n';
    return 0;
}

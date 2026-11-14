#include <bits/stdc++.h>
using namespace std;

using Complex = complex<double>;

class FFT {
    int size;
    vector<int> rev;
    vector<Complex> roots;

public:
    explicit FFT(int n) : size(n), rev(n), roots(n / 2) {
        for (int i = 1; i < size; ++i)
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) ? size / 2 : 0);
        const double pi = acos(-1.0);
        for (int i = 0; i < size / 2; ++i) {
            double angle = 2 * pi * i / size;
            roots[i] = Complex(cos(angle), sin(angle));
        }
    }

    void transform(vector<Complex>& a, bool inverse = false) const {
        if (inverse) reverse(a.begin() + 1, a.end());
        for (int i = 0; i < size; ++i)
            if (i < rev[i]) swap(a[i], a[rev[i]]);
        for (int half = 1; half < size; half <<= 1) {
            int step = size / (2 * half);
            for (int start = 0; start < size; start += 2 * half) {
                for (int j = 0; j < half; ++j) {
                    Complex u = a[start + j];
                    Complex v = a[start + j + half] * roots[j * step];
                    a[start + j] = u + v;
                    a[start + j + half] = u - v;
                }
            }
        }
        if (inverse)
            for (Complex& z : a) z /= size;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;
    vector<int> f(n);
    long long bound = 0;
    for (int i = 0; i < n; ++i) {
        cin >> f[i];
        if (i != k) bound += abs(f[i]);
    }
    if (bound == 0) {
        cout << "0.000000000000000\n";
        return 0;
    }

    // Shifting k to position zero only rotates the Fourier coefficients.
    // Its replacement x then adds the same real number to every coefficient.
    int size = 1;
    while (size < 2 * n - 1) size <<= 1;
    vector<Complex> a(size), b(size), chirp(n);
    const double pi = acos(-1.0);
    for (int s = 0; s < n; ++s) {
        // Reducing the squared index before taking trig functions avoids
        // loss of accuracy from unnecessarily large angles.
        long long phase = 1LL * s * s % (2LL * n);
        double angle = pi * phase / n;
        chirp[s] = Complex(cos(angle), -sin(angle));
        int value = (s == 0 ? 0 : f[(k + s) % n]);
        a[s] = double(value) * chirp[s];
        b[s] = conj(chirp[s]);
        if (s != 0) b[size - s] = b[s];
    }

    // Bluestein: exp(-2*pi*i*s*t/n) is the product of two
    // negative chirps and a positive chirp at the difference s-t.
    FFT fft(size);
    fft.transform(a);
    fft.transform(b);
    for (int i = 0; i < size; ++i) a[i] *= b[i];
    fft.transform(a, true);

    // Real input gives conjugate coefficients, hence only half are needed.
    vector<pair<long double, long double>> coefficients(n / 2 + 1);
    for (int t = 0; t <= n / 2; ++t) {
        Complex z = a[t] * chirp[t];
        coefficients[t] = {z.real(), z.imag()};
    }
    auto objective = [&](long long x) {
        long double result = 0;
        for (auto [re, im] : coefficients) {
            long double real_part = re + x;
            result = max(result, real_part * real_part + im * im);
        }
        return result;
    };

    // Every real part lies in [-bound, bound]. Outside this interval all
    // squared distances decrease when moving towards the interval.
    // The maximum of convex quadratics is convex, also on integer points.
    long long low = -bound, high = bound;
    while (low < high) {
        long long mid = low + (high - low) / 2;
        if (objective(mid) <= objective(mid + 1)) high = mid;
        else low = mid + 1;
    }
    cout << fixed << setprecision(15) << sqrtl(objective(low)) << '\n';
    return 0;
}

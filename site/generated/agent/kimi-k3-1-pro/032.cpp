#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>

using ld = long double;

struct Complex {
    ld x = 0, y = 0;

    Complex() = default;
    Complex(ld x, ld y) : x(x), y(y) {}

    Complex operator+(const Complex& other) const {
        return {x + other.x, y + other.y};
    }
    Complex operator-(const Complex& other) const {
        return {x - other.x, y - other.y};
    }
    Complex operator*(const Complex& other) const {
        return {x * other.x - y * other.y, x * other.y + y * other.x};
    }
    Complex& operator*=(const Complex& other) {
        *this = *this * other;
        return *this;
    }
    Complex& operator+=(const Complex& other) {
        x += other.x;
        y += other.y;
        return *this;
    }
    Complex& operator-=(const Complex& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }
};

static const ld PI = acosl(-1.0L);

void fft(std::vector<Complex>& a, bool inverse) {
    int n = static_cast<int>(a.size());
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        ld angle = (inverse ? 2 : -2) * PI / len;
        Complex root(cosl(angle), sinl(angle));
        for (int i = 0; i < n; i += len) {
            Complex w(1, 0);
            for (int j = 0; j < len / 2; ++j) {
                Complex u = a[i + j];
                Complex v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= root;
            }
        }
    }

    if (inverse) {
        for (Complex& value : a) {
            value.x /= n;
            value.y /= n;
        }
    }
}

std::vector<Complex> transformed_values(const std::vector<int>& f, int k) {
    int n = static_cast<int>(f.size());
    int m = 1;
    while (m < 2 * n - 1) m <<= 1;

    std::vector<Complex> a(m), b(m);
    int period = 2 * n;
    for (int i = 0; i < n; ++i) {
        long long square_mod = (1LL * i * i) % period;
        ld angle = PI * square_mod / n;
        Complex chirp(cosl(angle), sinl(angle));
        if (i != k) a[i] = Complex(f[i], 0) * Complex(chirp.x, -chirp.y);
        b[i] = chirp;
        if (i != 0) b[m - i] = chirp;
    }

    fft(a, false);
    fft(b, false);
    for (int i = 0; i < m; ++i) a[i] *= b[i];
    fft(a, true);

    std::vector<Complex> result(n);
    for (int t = 0; t < n; ++t) {
        long long square_mod = (1LL * t * t) % period;
        long long rotation_mod = (1LL * k * t) % n;
        ld angle = -PI * square_mod / n + 2 * PI * rotation_mod / n;
        Complex phase(cosl(angle), sinl(angle));
        result[t] = a[t] * phase;
    }
    return result;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int n, k;
    std::cin >> n >> k;
    std::vector<int> f(n);
    for (int& value : f) std::cin >> value;

    if (n == 1) {
        std::cout << "0\n";
        return 0;
    }

    std::vector<Complex> b = transformed_values(f, k);

    auto best_at = [&](ld x, bool need_direction) {
        ld maximum = -1;
        int index = 0;
        for (int t = 0; t < n; ++t) {
            ld re = b[t].x + x;
            ld squared = re * re + b[t].y * b[t].y;
            if (squared > maximum) {
                maximum = squared;
                index = t;
            }
        }
        return std::pair<ld, int>{maximum, need_direction ? index : 0};
    };

    ld low = -200000000.0L, high = 200000000.0L;
    for (int iteration = 0; iteration < 80; ++iteration) {
        ld middle = (low + high) / 2;
        auto [maximum, index] = best_at(middle, true);
        if (maximum == 0 || middle + b[index].x == 0) {
            low = high = middle;
            break;
        }
        if (middle + b[index].x > 0)
            high = middle;
        else
            low = middle;
    }

    long long center = floorl((low + high) / 2);
    ld answer = -1;
    for (long long x = center - 3; x <= center + 3; ++x) {
        ld maximum = best_at(x, false).first;
        if (answer < 0 || maximum < answer) answer = maximum;
    }

    std::cout << std::setprecision(18) << sqrtl(answer) << '\n';
    return 0;
}

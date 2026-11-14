#include <bits/stdc++.h>
using namespace std;

using cd = complex<double>;
static const double PI = acos(-1.0);

void fft(vector<cd>& a, bool invert) {
    int n = (int)a.size();

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        double ang = 2.0 * PI / len * (invert ? 1.0 : -1.0);
        cd wlen(cos(ang), sin(ang));

        for (int i = 0; i < n; i += len) {
            cd w(1.0, 0.0);
            for (int j = 0; j < len / 2; ++j) {
                cd u = a[i + j];
                cd v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (invert) {
        for (cd& z : a) z /= (double)n;
    }
}

// Unnormalized DFT with negative exponent.
vector<cd> dft(vector<cd> a) {
    int n = (int)a.size();

    if ((n & (n - 1)) == 0) {
        fft(a, false);
        return a;
    }

    // Bluestein chirp-z transform.
    int m = 1;
    while (m < 2 * n - 1) m <<= 1;

    vector<cd> fa(m, cd(0.0, 0.0));
    vector<cd> fb(m, cd(0.0, 0.0));
    vector<cd> chirp(n);

    long long mod = 2LL * n;
    for (int i = 0; i < n; ++i) {
        long long q = (1LL * i * i) % mod;
        double ang = PI * (double)q / (double)n;
        chirp[i] = cd(cos(ang), sin(ang));

        fa[i] = a[i] * conj(chirp[i]);
        fb[i] = chirp[i];
        if (i > 0) fb[m - i] = chirp[i];
    }

    fft(fa, false);
    fft(fb, false);
    for (int i = 0; i < m; ++i) fa[i] *= fb[i];
    fft(fa, true);

    for (int i = 0; i < n; ++i) {
        a[i] = fa[i] * conj(chirp[i]);
    }
    return a;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<long long> f(n);
    for (long long& x : f) cin >> x;

    vector<cd> a(n, cd(0.0, 0.0));
    long long M = 0;

    for (int s = 0; s < n; ++s) {
        if (s == k) continue;
        int j = s - k;
        if (j < 0) j += n;
        a[j] = cd((double)f[s], 0.0);
        M += llabs(f[s]);
    }

    if (M == 0) {
        cout << "0.0\n";
        return 0;
    }

    vector<cd> B = dft(std::move(a));

    auto evaluate = [&](long long x) -> long double {
        long double result = 0.0L;
        for (int i = 0; i < n; ++i) {
            long double d = (long double)x + (long double)B[i].real();
            long double im = (long double)B[i].imag();
            long double value = d * d + im * im;
            if (value > result) result = value;
        }
        return result;
    };

    long long lo = -M, hi = M;
    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        if (evaluate(mid) <= evaluate(mid + 1)) {
            hi = mid;
        } else {
            lo = mid + 1;
        }
    }

    long double answer = sqrt(evaluate(lo));
    cout << fixed << setprecision(15) << answer << '\n';
    return 0;
}

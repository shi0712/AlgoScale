#include <bits/stdc++.h>
using namespace std;

using cd = complex<long double>;
const long double PI = 3.141592653589793238462643383279502884L;

void fft(vector<cd>& a, bool invert) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        long double ang = 2.0L * PI / len * (invert ? 1.0L : -1.0L);
        cd wlen(cosl(ang), sinl(ang));
        int half = len >> 1;
        for (int i = 0; i < n; i += len) {
            cd w(1.0L, 0.0L);
            for (int j = 0; j < half; ++j) {
                cd u = a[i + j];
                cd v = a[i + j + half] * w;
                a[i + j] = u + v;
                a[i + j + half] = u - v;
                w *= wlen;
            }
        }
    }
    if (invert) {
        for (cd& x : a) x /= (long double)n;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;
    vector<int> f(n);
    for (int i = 0; i < n; ++i) cin >> f[i];

    if (n == 1) {
        cout << "0.000000000000000\n";
        return 0;
    }

    vector<int> g(n, 0);
    for (int d = 1; d < n; ++d) {
        int idx = k + d;
        if (idx >= n) idx -= n;
        g[d] = f[idx];
    }

    int m = 1;
    while (m < 2 * n - 1) m <<= 1;

    vector<cd> A(m, cd(0.0L, 0.0L)), B(m, cd(0.0L, 0.0L));

    for (int j = 0; j < n; ++j) {
        long double ang = -PI * (long double)j * (long double)j / (long double)n;
        A[j] = (long double)g[j] * cd(cosl(ang), sinl(ang));
    }

    for (int q = -(n - 1); q <= n - 1; ++q) {
        long double ang = PI * (long double)q * (long double)q / (long double)n;
        int idx = q % m;
        if (idx < 0) idx += m;
        B[idx] = cd(cosl(ang), sinl(ang));
    }

    fft(A, false);
    fft(B, false);
    for (int i = 0; i < m; ++i) A[i] *= B[i];
    fft(A, true);

    vector<long double> re(n), im(n);
    for (int t = 0; t < n; ++t) {
        long double ang = -PI * (long double)t * (long double)t / (long double)n;
        cd factor(cosl(ang), sinl(ang));
        cd val = A[t] * factor;
        re[t] = val.real();
        im[t] = val.imag();
    }

    long double max2 = 0.0L;
    for (int i = 0; i < n; ++i) {
        long double v = re[i] * re[i] + im[i] * im[i];
        if (v > max2) max2 = v;
    }

    if (max2 == 0.0L) {
        cout << "0.000000000000000\n";
        return 0;
    }

    long double M = sqrtl(max2);

    auto eval = [&](long double x) -> long double {
        long double best = 0.0L;
        for (int i = 0; i < n; ++i) {
            long double dx = x - re[i];
            long double val = dx * dx + im[i] * im[i];
            if (val > best) best = val;
        }
        return best;
    };

    long double lo = -M - 1.0L;
    long double hi = M + 1.0L;

    for (int iter = 0; iter < 80; ++iter) {
        long double m1 = lo + (hi - lo) / 3.0L;
        long double m2 = hi - (hi - lo) / 3.0L;
        if (eval(m1) < eval(m2)) hi = m2;
        else lo = m1;
    }

    long double xreal = (lo + hi) / 2.0L;
    long long center = (long long)floorl(xreal);

    long double best = 1e100L;
    for (long long x = center - 3; x <= center + 3; ++x) {
        long double val = eval((long double)x);
        if (val < best) best = val;
    }

    long double ans = sqrtl(best);
    cout << fixed << setprecision(15) << ans << '\n';

    return 0;
}

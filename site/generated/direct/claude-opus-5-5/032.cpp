#include <bits/stdc++.h>
using namespace std;
typedef complex<double> cd;

static void fft(vector<cd>& a, bool inv) {
    int n = a.size();
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    vector<cd> rt(n / 2);
    const double PI = acos(-1.0);
    for (int i = 0; i < n / 2; i++) {
        double ang = 2 * PI * i / n * (inv ? 1 : -1);
        rt[i] = cd(cos(ang), sin(ang));
    }
    for (int len = 2; len <= n; len <<= 1) {
        int half = len / 2, step = n / len;
        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; j++) {
                cd u = a[i + j], v = a[i + j + half] * rt[j * step];
                a[i + j] = u + v;
                a[i + j + half] = u - v;
            }
        }
    }
    if (inv) for (auto& x : a) x /= n;
}

int main() {
    int n, k;
    scanf("%d %d", &n, &k);
    vector<long long> f(n);
    for (auto& x : f) scanf("%lld", &x);
    vector<double> g(n);
    for (int j = 0; j < n; j++) g[j] = (double)f[(k + j) % n];
    g[0] = 0;
    const double PI = acos(-1.0);
    auto wch = [&](long long m) {
        long long r = (m * m) % (2LL * n);
        double ang = -PI * (double)r / n;
        return cd(cos(ang), sin(ang));
    };
    int N = 1;
    while (N < 2 * n - 1) N <<= 1;
    vector<cd> A(N), B(N);
    for (int j = 0; j < n; j++) A[j] = g[j] * wch(j);
    for (int i = 0; i < 2 * n - 1; i++) B[i] = conj(wch(i - (n - 1)));
    fft(A, false);
    fft(B, false);
    for (int i = 0; i < N; i++) A[i] *= B[i];
    fft(A, true);
    vector<double> a(n), b(n);
    for (int t = 0; t < n; t++) {
        cd c = A[t + n - 1] * wch(t);
        a[t] = c.real();
        b[t] = c.imag();
    }
    auto h = [&](long long x) {
        double best = 0;
        for (int t = 0; t < n; t++) {
            double u = a[t] + (double)x;
            double v = u * u + b[t] * b[t];
            if (v > best) best = v;
        }
        return sqrt(best);
    };
    long long lo = -1000000000LL, hi = 1000000000LL;
    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;
        if (h(mid) <= h(mid + 1)) hi = mid;
        else lo = mid + 1;
    }
    double ans = h(lo);
    ans = min(ans, h(lo - 1));
    ans = min(ans, h(lo + 1));
    printf("%.12f\n", ans);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

using ld = long double;
using C = complex<ld>;

const ld PI = acosl(-1.0L);

void fft(vector<C>& a, bool invert) {
    int n = (int)a.size();

    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        ld ang = 2.0L * PI / (ld)len * (invert ? 1.0L : -1.0L);
        C wlen(cosl(ang), sinl(ang));

        for (int i = 0; i < n; i += len) {
            C w(1.0L, 0.0L);
            for (int j = 0; j < len / 2; j++) {
                C u = a[i + j];
                C v = a[i + j + len / 2] * w;
                a[i + j] = u + v;
                a[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    if (invert) {
        ld invn = 1.0L / (ld)n;
        for (C& x : a) x *= invn;
    }
}

ld eval2(const vector<C>& h, long long y) {
    ld yy = (ld)y;
    ld best = -1.0L;

    for (const C& z : h) {
        ld dx = z.real() + yy;
        ld dy = z.imag();
        ld val = dx * dx + dy * dy;
        if (val > best) best = val;
    }

    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<int> f(n);
    for (int i = 0; i < n; i++) cin >> f[i];

    int m = 1;
    while (m < 2 * n - 1) m <<= 1;

    vector<C> A(m), B(m);
    vector<C> chirp(n);

    int mod = 2 * n;

    for (int r = 0; r < n; r++) {
        long long sq = (1LL * r * r) % mod;
        ld theta = PI * (ld)sq / (ld)n;

        ld cs = cosl(theta);
        ld sn = sinl(theta);

        C cm(cs, -sn);
        chirp[r] = cm;

        ld xr = (r == 0 ? 0.0L : (ld)f[(k + r) % n]);
        A[r] = xr * cm;

        if (r == 0) {
            B[0] = C(cs, sn);
        } else {
            C cp(cs, sn);
            B[r] = cp;
            B[m - r] = cp;
        }
    }

    fft(A, false);
    fft(B, false);

    for (int i = 0; i < m; i++) {
        A[i] *= B[i];
    }

    fft(A, true);

    vector<C> h(n);
    for (int t = 0; t < n; t++) {
        h[t] = A[t] * chirp[t];
    }

    long long lo = -300000000LL;
    long long hi = 300000000LL;

    while (lo < hi) {
        long long mid = lo + (hi - lo) / 2;

        ld v1 = eval2(h, mid);
        ld v2 = eval2(h, mid + 1);

        if (v1 <= v2) hi = mid;
        else lo = mid + 1;
    }

    ld best = eval2(h, lo);

    for (long long y = lo - 5; y <= lo + 5; y++) {
        best = min(best, eval2(h, y));
    }

    ld ans = sqrtl(best);

    cout << fixed << setprecision(18) << ans << '\n';

    return 0;
}

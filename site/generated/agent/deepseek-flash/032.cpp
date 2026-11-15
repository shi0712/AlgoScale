#include <bits/stdc++.h>
using namespace std;

using LD = long double;
using C = complex<LD>;

struct FFT {
    int n;
    vector<int> rev;
    vector<C> roots; // roots[k] = exp(-2*pi*i*k/n), 0 <= k < n/2

    void init(int n_) {
        n = n_;
        int lg = __builtin_ctz((unsigned)n);
        rev.resize(n);
        rev[0] = 0;
        for (int i = 1; i < n; ++i)
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (lg - 1));

        roots.resize(n / 2);
        const LD pi = acosl(-1.0L);
        for (int k = 0; k < n / 2; ++k) {
            LD ang = -2.0L * pi * (LD)k / (LD)n;
            roots[k] = C(cosl(ang), sinl(ang));
        }
    }

    void transform(vector<C>& a, bool invert) {
        for (int i = 0; i < n; ++i) {
            int j = rev[i];
            if (i < j) swap(a[i], a[j]);
        }
        for (int len = 2; len <= n; len <<= 1) {
            int half = len >> 1;
            int step = n / len;
            for (int i = 0; i < n; i += len) {
                for (int j = 0; j < half; ++j) {
                    C w = roots[j * step];
                    if (invert) w = conj(w);
                    C u = a[i + j];
                    C v = a[i + j + half] * w;
                    a[i + j] = u + v;
                    a[i + j + half] = u - v;
                }
            }
        }
        if (invert) {
            LD invn = 1.0L / (LD)n;
            for (auto& x : a) x *= invn;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;
    vector<long long> f(n);
    for (int i = 0; i < n; ++i) cin >> f[i];

    if (n == 1) {
        cout << setprecision(18) << 0.0L << "\n";
        return 0;
    }

    /*
       With f_k replaced by x, write

           F_t = G_t + x * exp(-2*pi*i*k*t/n),

       where G is the transform of f with f_k set to 0.  Multiplying by
       exp(2*pi*i*k*t/n) (modulus 1) gives

           |F_t| = |z_t + x|,
           z_t   = sum_{s != k} f_s * exp(-2*pi*i*(s-k)*t/n).

       Thus z is the DFT of

           y[0] = 0,   y[d] = f[(k+d) mod n]  (1 <= d < n).

       We need min over integers x of max_t |z_t + x|.
    */

    // Bluestein's chirp-z transform for arbitrary n.
    const LD pi = acosl(-1.0L);
    const int mod2n = 2 * n;
    vector<C> chirp(n);
    for (int m = 0; m < n; ++m) {
        long long sq = (long long)m * m;
        int r = (int)(sq % mod2n);       // exact phase reduction modulo 2*pi
        LD ang = pi * (LD)r / (LD)n;
        chirp[m] = C(cosl(ang), sinl(ang));
    }

    int M = 1;
    while (M < 2 * n - 1) M <<= 1;

    vector<C> A(M, C(0, 0)), B(M, C(0, 0));
    // A_s = y_s * exp(-pi*i*s^2/n)
    for (int d = 1; d < n; ++d) {
        int idx = k + d;
        if (idx >= n) idx -= n;
        A[d] = C((LD)f[idx], 0) * conj(chirp[d]);
    }
    // B_m = exp(pi*i*m^2/n), with B_{-m} = B_m.
    B[0] = chirp[0];
    for (int m = 1; m < n; ++m) {
        B[m] = chirp[m];
        B[M - m] = chirp[m];
    }

    FFT fft;
    fft.init(M);
    fft.transform(A, false);
    fft.transform(B, false);
    for (int i = 0; i < M; ++i) A[i] *= B[i];
    fft.transform(A, true);

    vector<LD> ar(n), bi(n);
    for (int t = 0; t < n; ++t) {
        C z = A[t] * conj(chirp[t]);
        ar[t] = z.real();
        bi[t] = z.imag();
    }

    /*
       Binary search the answer R.  For a fixed R, x is admissible iff for
       every t:
           |ar[t] + x| <= sqrt(R^2 - bi[t]^2),
       so the admissible x form an interval [L, U].  We only need to test
       whether this interval contains an integer.
    */
    LD hi = 0.0L;
    for (int t = 0; t < n; ++t) {
        LD v = sqrtl(ar[t] * ar[t] + bi[t] * bi[t]); // x = 0 is allowed
        if (v > hi) hi = v;
    }
    hi += 1.0L;
    LD lo = 0.0L;

    for (int it = 0; it < 100; ++it) {
        LD mid = (lo + hi) * 0.5L;
        LD rr = mid * mid;
        LD L = -1e100L, U = 1e100L;
        bool ok = true;
        for (int t = 0; t < n; ++t) {
            LD bb = bi[t] * bi[t];
            if (bb > rr) { ok = false; break; }
            LD s = sqrtl(rr - bb);
            LD l = -ar[t] - s;
            LD u = -ar[t] + s;
            if (l > L) L = l;
            if (u < U) U = u;
            if (L > U) { ok = false; break; }
        }
        if (ok && ceill(L) <= floorl(U) + 1e-12L) hi = mid;
        else lo = mid;
    }

    cout << setprecision(18) << hi << "\n";
    return 0;
}

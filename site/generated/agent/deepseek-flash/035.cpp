#include <bits/stdc++.h>
using namespace std;

using cd = complex<double>;

/*
 *  Let w = e^{-2*pi*i/n},  u_t = w^{k*t}  (|u_t| = 1)  and
 *      G_t = sum_{s != k} f_s * w^{s*t}.
 *  Setting f_k = x gives F_t = G_t + x*u_t, hence
 *      |F_t| = |G_t * conj(u_t) + x| = |x + w_t|,  w_t := G_t * conj(u_t).
 *  So we must minimize, over integers x, the maximum distance from -x to the
 *  points w_t of the complex plane:
 *      answer = min_{x in Z} max_t sqrt((x + Re w_t)^2 + (Im w_t)^2).
 *  This objective is convex in x (maximum of convex functions), so an integer
 *  ternary search finds the optimum.  All G_t come from one DFT, computed with
 *  Bluestein's algorithm (chirp-z) on a power-of-two FFT.
 */

static int L;              // FFT length (power of two)
static vector<cd> rt;      // rt[i] = e^{-2*pi*i*i/L}, 0 <= i <= L/2
static vector<int> rev;

static void initFFT(int L_) {
    L = L_;
    rt.assign((size_t)(L / 2 + 1), cd(1.0, 0.0));
    for (int i = 0; i <= L / 2; i++) {
        long double ang = -2.0L * acosl(-1.0L) * (long double)i / (long double)L;
        rt[(size_t)i] = cd((double)cosl(ang), (double)sinl(ang));
    }
    rev.assign((size_t)L, 0);
    int lg = 0;
    while ((1 << lg) < L) lg++;
    if (lg > 0)
        for (int i = 0; i < L; i++)
            rev[(size_t)i] = (rev[(size_t)(i >> 1)] >> 1) | ((i & 1) << (lg - 1));
}

static void fft(vector<cd>& a, bool inv) {
    for (int i = 0; i < L; i++)
        if (i < rev[(size_t)i]) swap(a[(size_t)i], a[(size_t)rev[(size_t)i]]);
    for (int len = 2; len <= L; len <<= 1) {
        int half = len >> 1, stride = L / len;
        for (int i = 0; i < L; i += len) {
            for (int j = 0; j < half; j++) {
                cd w = rt[(size_t)(j * stride)];
                if (inv) w = conj(w);
                cd u = a[(size_t)(i + j)];
                cd v = a[(size_t)(i + j + half)] * w;
                a[(size_t)(i + j)] = u + v;
                a[(size_t)(i + j + half)] = u - v;
            }
        }
    }
    if (inv) {
        double invL = 1.0 / (double)L;
        for (auto& z : a) z *= invL;
    }
}

int main() {
    int n, k;
    if (scanf("%d %d", &n, &k) != 2) return 0;
    vector<long long> f((size_t)n);
    for (int i = 0; i < n; i++)
        if (scanf("%lld", &f[(size_t)i]) != 1) return 0;

    // ---- G_t = sum_{s != k} f_s * w^{s*t} via Bluestein ----
    int need = max(1, 2 * n - 1);
    int Lsz = 1;
    while (Lsz < need) Lsz <<= 1;
    initFFT(Lsz);

    const long double PI = acosl(-1.0L);
    vector<cd> chirp((size_t)n);                       // e^{-i*pi*s^2/n}
    vector<cd> A((size_t)Lsz, cd(0.0, 0.0)), B((size_t)Lsz, cd(0.0, 0.0));
    for (int s = 0; s < n; s++) {
        long long sq = (long long)s * (long long)s % (2LL * n);
        long double ang = -PI * (long double)sq / (long double)n;
        long double c = cosl(ang), si = sinl(ang);
        chirp[(size_t)s] = cd((double)c, (double)si);
        B[(size_t)s] = cd((double)c, -(double)si);     // e^{+i*pi*s^2/n}
    }
    for (int j = 1; j < n; j++) B[(size_t)(Lsz - j)] = B[(size_t)j];  // B[-j] = B[j]

    for (int s = 0; s < n; s++) {
        if (s == k) continue;                          // f_k is the free choice
        long long val = f[(size_t)s];
        if (val == 0) continue;
        A[(size_t)s] = (double)val * chirp[(size_t)s];
    }

    fft(A, false);
    fft(B, false);
    for (int i = 0; i < Lsz; i++) A[(size_t)i] *= B[(size_t)i];
    fft(A, true);

    // ---- line representation of |x + w_t|^2 = x^2 + slope_t*x + icept_t ----
    vector<double> slope((size_t)n), icept((size_t)n);
    double M = 0.0;                                    // max_t |w_t|
    for (int t = 0; t < n; t++) {
        cd G = chirp[(size_t)t] * A[(size_t)t];        // G_t
        long long r = ((long long)k * t) % n;          // u_t = e^{-2*pi*i*r/n}
        long double ang = 2.0L * PI * (long double)r / (long double)n;
        cd uc((double)cosl(ang), (double)sinl(ang));   // conj(u_t)
        cd w = G * uc;
        double a = w.real(), b = w.imag();
        slope[(size_t)t] = 2.0 * a;
        icept[(size_t)t] = a * a + b * b;
        double len = sqrt(a * a + b * b);
        if (len > M) M = len;
    }

    // Any |x| > 2M gives value >= |x| - M > M = value at x = 0, so the
    // optimum lies in [-2M, 2M].
    long long X = (long long)(2.0 * M) + 3;

    const double* SL = slope.data();
    const double* IC = icept.data();

    // binary search on the (monotone) difference-free convex objective:
    // F(x) = x^2 + max_t (slope_t * x + icept_t)
    auto F = [&](long long x) -> double {
        double xd = (double)x;
        double best = -1e300;
        for (int t = 0; t < n; t++) {
            double v = SL[t] * xd + IC[t];
            if (v > best) best = v;
        }
        return xd * xd + best;
    };

    long long lo = -X, hi = X;                         // integer ternary search
    while (hi - lo > 3) {
        long long m1 = lo + (hi - lo) / 3;
        long long m2 = hi - (hi - lo) / 3;
        if (F(m1) < F(m2)) hi = m2;
        else lo = m1;
    }
    double bestVal = 1e300;
    for (long long x = lo - 2; x <= hi + 2; x++) {
        double v = F(x);
        if (v < bestVal) bestVal = v;
    }

    double ans = (bestVal > 0.0) ? sqrt(bestVal) : 0.0;
    printf("%.15f\n", ans);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

const long double PI = acosl(-1.0L);

struct C {
    long double x, y;
    C() : x(0), y(0) {}
    C(long double _x, long double _y) : x(_x), y(_y) {}
};

inline C operator+(const C& a, const C& b) {
    return C(a.x + b.x, a.y + b.y);
}

inline C operator-(const C& a, const C& b) {
    return C(a.x - b.x, a.y - b.y);
}

inline C operator*(const C& a, const C& b) {
    return C(
        a.x * b.x - a.y * b.y,
        a.x * b.y + a.y * b.x
    );
}

void fft(
    vector<C>& a,
    bool invert,
    const vector<C>& roots,
    const vector<C>& rootsInv,
    const vector<int>& rev
) {
    int n = (int)a.size();

    for (int i = 0; i < n; ++i) {
        if (i < rev[i]) swap(a[i], a[rev[i]]);
    }

    const vector<C>& rt = invert ? rootsInv : roots;

    for (int len = 2; len <= n; len <<= 1) {
        int half = len >> 1;
        int step = n / len;

        for (int i = 0; i < n; i += len) {
            for (int j = 0; j < half; ++j) {
                C w = rt[j * step];
                C u = a[i + j];
                C v = a[i + j + half] * w;

                a[i + j] = u + v;
                a[i + j + half] = u - v;
            }
        }
    }

    if (invert) {
        long double invN = 1.0L / n;
        for (C& z : a) {
            z.x *= invN;
            z.y *= invN;
        }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    cin >> n >> k;

    vector<int> f(n);
    for (int i = 0; i < n; ++i) cin >> f[i];

    int period = 2 * n;

    vector<long double> chirpC(period), chirpS(period);
    for (int r = 0; r < period; ++r) {
        long double ang = PI * (long double)r / n;
        chirpC[r] = cosl(ang);
        chirpS[r] = sinl(ang);
    }

    vector<long double> rotC(n), rotS(n);
    for (int r = 0; r < n; ++r) {
        long double ang = 2.0L * PI * (long double)r / n;
        rotC[r] = cosl(ang);
        rotS[r] = sinl(ang);
    }

    int M = 1;
    while (M < 2 * n - 1) M <<= 1;

    vector<int> rev(M);
    if (M > 1) {
        int lg = 0;
        while ((1 << lg) < M) ++lg;

        rev[0] = 0;
        for (int i = 1; i < M; ++i) {
            rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (lg - 1));
        }
    }

    vector<C> roots(M / 2), rootsInv(M / 2);
    for (int i = 0; i < M / 2; ++i) {
        long double ang = -2.0L * PI * (long double)i / M;
        long double c = cosl(ang);
        long double s = sinl(ang);

        roots[i] = C(c, s);
        rootsInv[i] = C(c, -s);
    }

    vector<C> A(M), B(M);

    for (int j = 0; j < n; ++j) {
        long double val = (j == k ? 0.0L : (long double)f[j]);

        long long rem = (1LL * j * j) % period;
        long double c = chirpC[rem];
        long double s = chirpS[rem];

        A[j] = C(val * c, -val * s);
    }

    for (int d = -(n - 1); d <= n - 1; ++d) {
        int idx = (n - 1) + d;

        long long dd = (d < 0 ? -(long long)d : (long long)d);
        long long rem = (dd * dd) % period;

        B[idx] = C(chirpC[rem], chirpS[rem]);
    }

    fft(A, false, roots, rootsInv, rev);
    fft(B, false, roots, rootsInv, rev);

    for (int i = 0; i < M; ++i) {
        A[i] = A[i] * B[i];
    }

    fft(A, true, roots, rootsInv, rev);

    vector<long double> a(n), b2(n);

    long double minA = 1e100L;
    long double maxA = -1e100L;

    for (int t = 0; t < n; ++t) {
        C conv = A[t + n - 1];

        long long rem = (1LL * t * t) % period;
        C phase(chirpC[rem], -chirpS[rem]);

        C F = conv * phase;

        int r = (int)((1LL * k * t) % n);
        C rot(rotC[r], rotS[r]);

        C q = F * rot;

        long double x = -q.x;
        long double y = q.y;

        a[t] = x;
        b2[t] = y * y;

        minA = min(minA, x);
        maxA = max(maxA, x);
    }

    auto value2 = [&](long double x) {
        long double res = 0.0L;

        for (int i = 0; i < n; ++i) {
            long double d = a[i] - x;
            long double v = d * d + b2[i];
            if (v > res) res = v;
        }

        return res;
    };

    long double lo = minA;
    long double hi = maxA;

    for (int it = 0; it < 100; ++it) {
        long double third = (hi - lo) / 3.0L;
        long double m1 = lo + third;
        long double m2 = hi - third;

        long double v1 = value2(m1);
        long double v2 = value2(m2);

        if (v1 < v2) {
            hi = m2;
        } else {
            lo = m1;
        }
    }

    long double mid = (lo + hi) / 2.0L;
    long long base = (long long)floorl(mid);

    long double best = 1e100L;

    for (long long x = base - 10; x <= base + 10; ++x) {
        long double cur = value2((long double)x);
        if (cur < best) best = cur;
    }

    long double ans = sqrtl(best);

    cout << fixed << setprecision(15) << ans << '\n';

    return 0;
}

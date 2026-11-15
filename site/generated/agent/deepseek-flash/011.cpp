#pragma GCC optimize("O3,unroll-loops")
#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = unsigned __int128;

static inline void print_u128(u128 x) {
    if (x == 0) { putchar('0'); return; }
    char buf[64];
    int len = 0;
    while (x > 0) {
        buf[len++] = char('0' + (int)(x % 10));
        x /= 10;
    }
    while (len) putchar(buf[--len]);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    u64 n, k, a, b;
    if (!(cin >> n >> k >> a >> b)) return 0;

    const u128 K = (u128)k;
    const u128 N = (u128)n;
    const u128 A = (u128)a;
    const u128 B = (u128)b;

    // A safe upper bound: take all pairs whose difference d is at most D0,
    // where D0 is the smallest value with sum_{d=1..D0} (n-d) >= k.
    u64 loD = 1, hiD = n - 1;
    while (loD < hiD) {
        u64 mid = loD + (hiD - loD) / 2;
        u128 cnt = (u128)mid * N - (u128)mid * (mid + 1) / 2;
        if (cnt >= K) hiD = mid;
        else loD = mid + 1;
    }
    u64 D0 = loD;
    u128 hi = (u128)D0 * ((u128)2 * A * N + (u128)2 * B - A * (u128)D0);
    u128 lo = 0;

    // E(d) = maximum value in row d = d * (2an + 2b - a d).
    auto E = [&](u64 d) -> u128 {
        return (u128)d * ((u128)2 * A * N + (u128)2 * B - A * (u128)d);
    };
    // S(d) = minimum value in row d = d * (a(d+2) + 2b).
    auto S = [&](u64 d) -> u128 {
        return (u128)d * (A * (u128)(d + 2) + (u128)2 * B);
    };

    auto countLE = [&](u128 X) -> u128 {
        // F = largest d with E(d) <= X.  Since E is increasing on [0,n-1].
        u64 f_lo = 0, f_hi = n; // f_hi is exclusive, E(n) > X for all relevant X
        while (f_hi - f_lo > 1) {
            u64 mid = f_lo + (f_hi - f_lo) / 2;
            if (E(mid) <= X) f_lo = mid;
            else f_hi = mid;
        }
        u64 F = f_lo;

        u128 cnt = (u128)F * N - (u128)F * (F + 1) / 2;
        if (cnt >= K) return cnt;

        // D = largest d with S(d) <= X.
        u64 d_lo = 0, d_hi = n;
        while (d_hi - d_lo > 1) {
            u64 mid = d_lo + (d_hi - d_lo) / 2;
            if (S(mid) <= X) d_lo = mid;
            else d_hi = mid;
        }
        u64 D = d_lo;

        for (u64 d = F + 1; d <= D; ++d) {
            u128 ad = A * (u128)d;
            u128 num = X - ad * (u128)d - (u128)2 * B * (u128)d;
            u64 den = (u64)((u128)2 * ad);
            cnt += num / (u128)den;
            if (cnt >= K) return cnt;
        }
        return cnt;
    };

    // Continuous approximation used only to find a tight initial bracket.
    const long double LN = (long double)n;
    const long double LA = (long double)a;
    const long double LB = (long double)b;
    const long double LK = (long double)k;
    const long double GAMMA = 0.577215664901532860606512090082402431L;
    auto Harm = [&](long double x) -> long double {
        if (x <= 0) return 0;
        return logl(x) + GAMMA + 1.0L / (2.0L * x) - 1.0L / (12.0L * x * x)
             + 1.0L / (120.0L * x * x * x * x);
    };
    const long double EmaxLD = (long double)E(n - 1);
    auto contCount = [&](long double X) -> long double {
        if (X <= 0) return 0;
        if (X >= EmaxLD) return LK;
        long double C = 2.0L * LA * LN + 2.0L * LB;
        long double disc = C * C - 4.0L * LA * X;
        long double Fr = 0;
        if (disc > 0) {
            long double sd = sqrtl(disc);
            Fr = (2.0L * X) / (C + sd);
        }
        if (Fr < 0) Fr = 0;
        if (Fr > (long double)(n - 1)) Fr = (long double)(n - 1);
        u64 F = (u64)floorl(Fr);
        long double full = (long double)F * LN - (long double)F * (long double)(F + 1) / 2.0L;
        long double t = LA + LB;
        long double Dr = X / (t + sqrtl(t * t + LA * X));
        if (Dr < 0) Dr = 0;
        if (Dr > (long double)(n - 1)) Dr = (long double)(n - 1);
        u64 D = (u64)floorl(Dr);
        long double L = (long double)F + 1.0L;
        if ((long double)D < L) return full;
        long double terms = (long double)D - L + 1.0L;
        long double res = full;
        res += (X / (2.0L * LA)) * (Harm((long double)D) - Harm((long double)F));
        res -= 0.25L * ((long double)D * (long double)(D + 1) - L * (L - 1.0L));
        res -= (LB / LA) * terms;
        res -= 0.5L * terms;
        return res;
    };

    // Locate a good starting point with the continuous model.
    long double loLD = 0.0L, hiLD = (long double)hi;
    for (int it = 0; it < 110; ++it) {
        long double mid = (loLD + hiLD) * 0.5L;
        if (contCount(mid) >= LK) hiLD = mid;
        else loLD = mid;
    }
    long double x0 = hiLD;
    if (x0 < 1) x0 = 1;

    // Try progressively larger windows around x0, verifying the bracket.
    u128 bestL = 0, bestH = hi;
    const long double epsList[4] = {1e-7L, 1e-4L, 1e-2L, 1.0L};
    for (int ei = 0; ei < 4; ++ei) {
        long double delta = x0 * epsList[ei] + 1000.0L;
        long double lowLD = x0 - delta;
        long double highLD = x0 + delta;
        if (lowLD < 0) lowLD = 0;
        if (highLD > (long double)hi) highLD = (long double)hi;
        u128 L = (u128)lowLD;
        u128 H = (u128)highLD;
        if (H > hi) H = hi;
        if (L >= H) continue;
        if (L > 0 && countLE(L) >= K) continue;
        if (H < hi && countLE(H) < K) continue;
        bestL = L; bestH = H;
        break;
    }
    lo = bestL; hi = bestH;

    while (lo + 1 < hi) {
        u128 mid = lo + (hi - lo) / 2;
        if (countLE(mid) >= K) hi = mid;
        else lo = mid;
    }

    // The k-th value is hi / a. Reduce the fraction.
    u64 rem = (u64)(hi % A);
    u64 g = std::gcd(a, rem);
    u128 p = hi / (u128)g;
    u64 q = a / g;
    print_u128(p);
    putchar(' ');
    print_u128((u128)q);
    putchar('\n');
    return 0;
}

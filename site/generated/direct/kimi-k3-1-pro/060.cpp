#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static inline int64 ceil_div(int64 a, int64 b) {
    return a / b + (a % b != 0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int64 h, w, s;
        cin >> h >> w >> s;

        if ((i128)h * w <= s) {
            cout << 0 << '\n';
            continue;
        }

        if (h < w) swap(h, w);

        const int64 maxP = min(h, s);
        int64 best = h + w; // A + B; the answer is best - 2.

        auto relaxP = [&](int64 p) {
            if (p < 1 || p > maxP) return;
            int64 q = s / p;
            int64 A = ceil_div(h, p);
            int64 B = ceil_div(w, q);
            best = min(best, A + B);
        };

        relaxP(1);
        relaxP(maxP);

        const long double H = h, W = w, S = s;
        const long double R = sqrtl(H * W / S);
        const long double p0 = sqrtl(H * S / W);
        const long double q0 = sqrtl(W * S / H);

        int64 ip = (int64)floorl(p0);
        for (int d = -16; d <= 16; ++d) {
            if (d < 0 && ip < (int64)(-d)) continue;
            relaxP(ip + d);
        }

        int64 iq = (int64)floorl(q0);
        for (int d = -16; d <= 16; ++d) {
            int64 q = iq + d;
            if (q < 1 || q > s) continue;
            int64 p = min(h, s / q);
            relaxP(p);
            if (p > 1) relaxP(p - 1);
            if (p < maxP) relaxP(p + 1);
        }

        for (int64 q : {int64(1), s}) {
            int64 p = min(h, s / q);
            relaxP(p);
        }

        const long double lower = 2.0L * R;
        bool optimal = ((long double)best <= lower);

        // If best equals ceil(lower), it is also optimal.  The tolerance is
        // used only to avoid relying on a rounded value extremely close to
        // an integer.
        if (!optimal) {
            long double fl = floorl(lower);
            long double tolerance = max(1e-9L, 1e-14L * lower);
            if (lower - fl > tolerance && (long double)best <= fl + 1.0L)
                optimal = true;
        }

        if (!optimal) {
            long double t = (long double)best / R;
            if (t <= 2.0L) {
                optimal = true;
            } else {
                long double sd = sqrtl((t - 2.0L) * (t + 2.0L));
                long double rLow = 2.0L / (t + sd);
                long double rHigh = (t + sd) * 0.5L;

                long double approximateLo = p0 * rLow;
                long double approximateHi = p0 * rHigh;

                long double marginLo =
                    64.0L + 1e-12L * max(1.0L, fabsl(approximateLo));
                long double marginHi =
                    64.0L + 1e-12L * max(1.0L, fabsl(approximateHi));

                long double L = approximateLo - marginLo;
                long double U = approximateHi + marginHi;

                if (U >= 1.0L && L <= (long double)maxP) {
                    L = max(L, 1.0L);
                    U = min(U, (long double)maxP);

                    int64 lo = max<int64>(1, (int64)floorl(L));
                    int64 hi = min<int64>(maxP, (int64)ceill(U));

                    if (lo <= hi) {
                        int64 pCount = hi - lo + 1;

                        int64 qLo;
                        if (hi >= maxP) {
                            qLo = 1;
                        } else {
                            int64 v = s / (hi + 1);
                            qLo = v > 64 ? v - 64 : 1;
                        }

                        int64 qHi = s / lo;
                        qHi = (qHi > s - 64) ? s : qHi + 64;

                        int64 qCount = (qLo <= qHi)
                            ? qHi - qLo + 1
                            : numeric_limits<int64>::max() / 4;
                        int64 aCount = min(h, best - 1);

                        if (pCount <= qCount && pCount <= aCount) {
                            // Only the right end of each constant-q block
                            // needs to be examined.
                            int64 p = lo;
                            while (p <= hi) {
                                int64 q = s / p;
                                int64 last = min(hi, s / q);
                                relaxP(last);
                                if (last == hi) break;
                                p = last + 1;
                            }
                        } else if (qCount <= aCount) {
                            // For fixed q, increasing p to floor(s/q)
                            // cannot increase either term of the cost.
                            int64 q = qLo;
                            while (q <= qHi) {
                                int64 rawP = s / q;
                                relaxP(min(h, rawP));

                                int64 last = min(qHi, s / rawP);
                                if (last == qHi) break;
                                q = last + 1;
                            }
                        } else {
                            // Exact enumeration by the number A of horizontal
                            // segments.  Skip blocks having the same ceil(h/A).
                            int64 A = 1;
                            int64 limit = aCount;

                            while (A <= limit && A < best) {
                                int64 x = ceil_div(h, A);
                                if (x <= s) {
                                    int64 q = s / x;
                                    int64 B = ceil_div(w, q);
                                    best = min(best, A + B);
                                }

                                if (x == 1) break;
                                int64 nextA = ceil_div(h, x - 1);
                                if (nextA <= A) nextA = A + 1;
                                A = nextA;
                            }
                        }
                    }
                }
            }
        }

        cout << best - 2 << '\n';
    }

    return 0;
}

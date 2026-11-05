#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
using i128 = __int128_t;

static inline int64 ceil_div(int64 a, int64 b) {
    return (a + b - 1) / b;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const int64 B = 50000;
    const int64 W = 60000;
    const int64 INF = (int64)4e18;

    int T;
    cin >> T;

    while (T--) {
        int64 h, w, s;
        cin >> h >> w >> s;

        // Keep w <= h. The problem is symmetric.
        if (h < w) swap(h, w);

        int64 ans = INF;

        auto evalH = [&](int64 H) {
            if (H <= 0 || H > h || H > s) return;
            int64 Qcap = s / H;
            if (Qcap <= 0) return;

            int64 Q = min(w, Qcap);
            int64 pieces = ceil_div(h, H) + ceil_div(w, Q);
            ans = min(ans, pieces - 2);
        };

        auto evalQ = [&](int64 Q) {
            if (Q <= 0 || Q > w || Q > s) return;
            int64 Hcap = s / Q;
            if (Hcap <= 0) return;

            int64 H = min(h, Hcap);
            int64 pieces = ceil_div(h, H) + ceil_div(w, Q);
            ans = min(ans, pieces - 2);
        };

        int64 maxH = min(h, s);
        int64 maxQ = min(w, s);

        // Enumerate all small possible maximum piece sizes.
        int64 limH = min(maxH, B);
        for (int64 H = 1; H <= limH; ++H) {
            evalH(H);
        }

        int64 limQ = min(maxQ, B);
        for (int64 Q = 1; Q <= limQ; ++Q) {
            evalQ(Q);
        }

        // Boundary cases.
        evalH(maxH);
        evalQ(maxQ);

        // Check candidates around continuous optimum.
        if (w > 0) {
            long double th = sqrt((long double)h * (long double)s / (long double)w);
            int64 cH = (int64)floorl(th);
            cH = max<int64>(1, min<int64>(cH, maxH));
            for (int d = -5; d <= 5; ++d) {
                int64 H = cH + d;
                if (1 <= H && H <= maxH) evalH(H);
            }
        }

        if (h > 0) {
            long double tq = sqrt((long double)w * (long double)s / (long double)h);
            int64 cQ = (int64)floorl(tq);
            cQ = max<int64>(1, min<int64>(cQ, maxQ));
            for (int d = -5; d <= 5; ++d) {
                int64 Q = cQ + d;
                if (1 <= Q && Q <= maxQ) evalQ(Q);
            }
        }

        bool largeK = ((i128)h * w > (i128)s * B * B);

        if (!largeK) {
            // The continuous optimum has O(B) pieces. Enumerate the number
            // of horizontal pieces around it.
            int64 maxA = min(h, 2 * B + 100);
            maxA = min(maxA, ans + 2);

            for (int64 a = 1; a <= maxA; ++a) {
                int64 H = ceil_div(h, a);
                if (H > s) continue;

                int64 Qcap = s / H;
                int64 Q = min(w, Qcap);
                if (Q <= 0) continue;

                int64 b = ceil_div(w, Q);
                int64 pieces = a + b;
                ans = min(ans, pieces - 2);
            }
        } else {
            // K is large. If both maximum dimensions are > B, the optimum
            // is close to the continuous optimum. If one is <= B, it was
            // already covered by the small enumeration above.
            if (w > 0) {
                long double tq = sqrt((long double)w * (long double)s / (long double)h);
                int64 center;
                if (tq < 1.0L) center = 1;
                else if (tq > (long double)maxQ) center = maxQ;
                else center = (int64)floorl(tq);

                int64 lo = max<int64>(1, center - W);
                int64 hi = min<int64>(maxQ, center + W);
                for (int64 Q = lo; Q <= hi; ++Q) {
                    evalQ(Q);
                }
            }

            if (h > 0) {
                long double th = sqrt((long double)h * (long double)s / (long double)w);
                int64 center;
                if (th < 1.0L) center = 1;
                else if (th > (long double)maxH) center = maxH;
                else center = (int64)floorl(th);

                int64 lo = max<int64>(1, center - W);
                int64 hi = min<int64>(maxH, center + W);
                for (int64 H = lo; H <= hi; ++H) {
                    evalH(H);
                }
            }
        }

        cout << ans << '\n';
    }

    return 0;
}

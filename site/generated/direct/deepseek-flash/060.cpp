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

    int T;
    cin >> T;

    while (T--) {
        int64 h, w, s;
        cin >> h >> w >> s;

        // Already one rectangle of area h*w is enough.
        if ((i128)h * w <= s) {
            cout << 0 << '\n';
            continue;
        }

        /*
            Choose a maximum horizontal piece length H.
            Then the maximum vertical piece length can be:
                Q = floor(s / H)
            capped naturally by ceil(w / Q).

            We minimise:
                ceil(h / H) + ceil(w / floor(s / H))
            and subtract 2 (cuts = pieces - 1 in each direction).

            H only needs to be at most min(h, s), because the other
            direction needs at least one unit of length.
         */
        int64 maxH = min(h, s);
        int64 best = LLONG_MAX / 4;

        auto evaluate = [&](int64 H) {
            if (H < 1 || H > maxH) return;
            int64 Q = s / H;
            if (Q == 0) return;

            int64 horizontalPieces = ceil_div(h, H);
            int64 verticalPieces = ceil_div(w, Q);
            best = min(best, horizontalPieces + verticalPieces);
        };

        // For small enough H, simply brute force.
        const int64 BRUTE_LIMIT = 200000;
        if (maxH <= BRUTE_LIMIT) {
            for (int64 H = 1; H <= maxH; ++H) evaluate(H);
            cout << best - 2 << '\n';
            continue;
        }

        // Continuous optimum for H:
        // minimise h/H + w*H/s  ->  H = sqrt(h*s/w)
        long double H0real =
            sqrtl((long double)h * (long double)s / (long double)w);

        int64 floorStart = (int64)floor(H0real);
        int64 ceilStart = (int64)ceil(H0real);

        floorStart = max<int64>(1, min<int64>(floorStart, maxH));
        ceilStart = max<int64>(1, min<int64>(ceilStart, maxH));

        // Evaluate several important candidates.
        vector<int64> candidates;
        candidates.push_back(1);
        candidates.push_back(maxH);

        for (int d = -3; d <= 3; ++d) {
            candidates.push_back(floorStart + d);
            candidates.push_back(ceilStart + d);
        }

        // Symmetric continuous optimum in terms of the vertical maximum:
        // Q = sqrt(w*s/h), and H is around s/Q.
        long double Q0real =
            sqrtl((long double)w * (long double)s / (long double)h);
        int64 qApprox = (int64)floor(Q0real);
        for (int d = -3; d <= 3; ++d) {
            int64 q = qApprox + d;
            if (q >= 1) {
                int64 H = s / q;
                candidates.push_back(H);
                candidates.push_back(H - 1);
                candidates.push_back(H + 1);
            }
        }

        // Also try the continuous optimum number of intervals.
        long double T0real =
            sqrtl((long double)h * (long double)w / (long double)s);
        int64 tApprox = (int64)floor(T0real);
        for (int d = -3; d <= 3; ++d) {
            int64 x = tApprox + d;
            if (x >= 1) {
                int64 H = ceil_div(h, x);
                candidates.push_back(H);
                candidates.push_back(H - 1);
                candidates.push_back(H + 1);
            }
        }

        for (int64 H : candidates) evaluate(H);

        auto lowerBound = [&](int64 H) -> long double {
            long double a = (long double)h / (long double)H;
            long double q = (long double)s / (long double)H;

            if (q > (long double)w) q = (long double)w;
            if (q < 1.0L) q = 1.0L;

            long double b = (long double)w / q;

            if (a < 1.0L) a = 1.0L;
            if (b < 1.0L) b = 1.0L;

            return a + b;
        };

        auto nextRight = [&](int64 H) -> int64 {
            int64 A = ceil_div(h, H);
            int64 nextA = LLONG_MAX;

            if (A > 1) {
                nextA = (h - 1) / (A - 1) + 1;
            }

            int64 Q = s / H;
            int64 nextQ = s / Q + 1;

            return min(nextA, nextQ);
        };

        auto prevLeft = [&](int64 H) -> int64 {
            int64 A = ceil_div(h, H);
            int64 prevA = ceil_div(h, A) - 1;

            int64 Q = s / H;
            int64 prevQ = s / (Q + 1);

            return max(prevA, prevQ);
        };

        // Scan events to the right of the continuous optimum.
        int64 H = ceilStart;
        while (true) {
            evaluate(H);

            // If even the continuous lower bound cannot improve the current
            // integer answer, all further candidates to the right are useless.
            if (lowerBound(H) > (long double)best - 0.25L) break;

            int64 nxt = nextRight(H);
            if (nxt > maxH) break;
            H = nxt;
        }

        // Scan events to the left of the continuous optimum.
        H = floorStart;
        while (true) {
            evaluate(H);

            if (lowerBound(H) > (long double)best - 0.25L) break;

            int64 prv = prevLeft(H);
            if (prv < 1) break;
            H = prv;
        }

        cout << best - 2 << '\n';
    }

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

static bool readD(double &v) {
    if (scanf("%lf", &v) != 1) return false;
    if (v < -0.5) return false;
    return true;
}

int main() {
    const int N = 100000;
    set<int> seen;
    int hitA = -1;
    double L0 = 0;
    int steps[] = {50000, 25000, 12500, 6250, 3125, 1562, 781, 390, 195};
    for (int s : steps) {
        for (int a = s; a < N && hitA < 0; a += s) {
            if (seen.count(a)) continue;
            seen.insert(a);
            printf("query %d %d %d %d\n", a, 0, a, N);
            fflush(stdout);
            double L;
            if (!readD(L)) return 0;
            if (L > 0.5) {
                hitA = a;
                L0 = L;
            }
        }
        if (hitA >= 0) break;
    }
    if (hitA < 0) {
        return 0;
    }
    int a = hitA;
    double h0sq = L0 * L0 / 4.0;
    long long u;
    {
        int b = a + 1;
        printf("query %d %d %d %d\n", b, 0, b, N);
        fflush(stdout);
        double L1;
        if (!readD(L1)) return 0;
        if (L1 > 0.5) {
            double D = (L1 * L1 - L0 * L0) / 4.0;
            // u = a - cx, D = -2u-1
            u = llround(-(D + 1.0) / 2.0);
        } else {
            int c = a - 1;
            printf("query %d %d %d %d\n", c, 0, c, N);
            fflush(stdout);
            double L2;
            if (!readD(L2)) return 0;
            double D = (L2 * L2 - L0 * L0) / 4.0;
            // D = 2u-1
            u = llround((D + 1.0) / 2.0);
        }
    }
    long long cx = a - u;
    long long r = llround(sqrt(h0sq + (double)(u * u)));
    long long lo = r, hi = N - r;
    long long cy = -1;
    while (lo < hi) {
        long long y2 = (lo + hi) / 2;
        if (y2 < 1) y2 = 1;
        printf("query %lld %d %lld %lld\n", cx, 0, cx, y2);
        fflush(stdout);
        double L;
        if (!readD(L)) return 0;
        long long l = llround(L);
        if (l <= 0) {
            lo = y2 + r;
        } else if (l >= 2 * r) {
            hi = y2 - r;
        } else {
            cy = y2 - l + r;
            lo = hi = cy;
        }
    }
    cy = lo;
    printf("answer %lld %lld %lld\n", cx, cy, r);
    fflush(stdout);
    return 0;
}

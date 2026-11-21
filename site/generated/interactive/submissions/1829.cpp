#include <bits/stdc++.h>
using namespace std;

static const long long N = 100000;

long double ask(long long x1, long long y1, long long x2, long long y2) {
    cout << "query " << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << '\n' << flush;
    long double v;
    if (!(cin >> v)) exit(0);
    if (v < -0.5L) exit(0);
    return v;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Horizontal lines covering every possible y-coordinate of the center.
    // If r >= 100, every center y in [100, 99900] is within distance <= 99
    // from at least one of these lines, hence that line intersects the disk.
    vector<long long> ys;
    for (int i = 0; i < 502; ++i) {
        ys.push_back(199LL + 199LL * i);
    }

    // Balanced query order: useful for large radii.
    vector<long long> order;
    function<void(int,int)> build = [&](int l, int r) {
        if (l > r) return;
        int m = (l + r) >> 1;
        order.push_back(ys[m]);
        build(l, m - 1);
        build(m + 1, r);
    };
    build(0, (int)ys.size() - 1);

    long long Yhit = -1;
    long double L0 = 0.0L;

    for (long long Y : order) {
        long double v = ask(0, Y, N, Y);
        if (v > 1e-7L) {
            Yhit = Y;
            L0 = v;
            break;
        }
    }

    // This should never happen with the given constraints.
    if (Yhit == -1) return 0;

    long double L1 = ask(0, Yhit + 1, N, Yhit + 1);

    // Recover y and r from the two adjacent horizontal chords.
    long long bestR = -1, bestY = -1;
    long double bestErr = 1e100L;
    long double h0 = L0 / 2.0L;

    for (long long d = -50000; d <= 50000; ++d) {
        long long yc = Yhit - d;
        if (yc < 0 || yc > N) continue;

        long double R2 = h0 * h0 + (long double)d * d;
        if (R2 < 0) continue;

        long long base = llround(sqrtl(R2));
        long long lo = max(100LL, base - 5);
        long long hi = min(N / 2, base + 5);

        for (long long rr = lo; rr <= hi; ++rr) {
            if (llabs(d) >= rr) continue;
            if (yc < rr || yc > N - rr) continue;

            long double dd = (long double)d;
            long double val0 = (long double)rr * rr - dd * dd;
            if (val0 < 0) continue;

            long double pred0 = 2.0L * sqrtl(max(0.0L, val0));

            long double d1 = dd + 1.0L;
            long double val1 = (long double)rr * rr - d1 * d1;
            long double pred1 = 0.0L;
            if (val1 > 0) pred1 = 2.0L * sqrtl(val1);

            long double err = fabsl(pred0 - L0) + fabsl(pred1 - L1);
            if (err < bestErr) {
                bestErr = err;
                bestR = rr;
                bestY = yc;
            }
        }
    }

    long long r = bestR;
    long long y = bestY;

    // Fallback, normally not used.
    if (r == -1) {
        long double h1 = L1 / 2.0L;
        if (L1 <= 1e-7L) {
            r = llround((h0 * h0 + 1.0L) / 2.0L);
            y = Yhit - (r - 1);
        } else {
            long long d = llround((h0 * h0 - h1 * h1 - 1.0L) / 2.0L);
            y = Yhit - d;
            long long R2i = llround(h0 * h0 + (long double)d * d);
            r = llround(sqrtl((long double)R2i));
        }
    }

    long long x;

    if (r * 2 == N) {
        x = N / 2;
    } else {
        long long delta = r - 1;
        long long y2 = y + delta;

        long double Lx = ask(0, y, N, y2);

        long double denom = sqrtl((long double)N * N + (long double)delta * delta);
        long double coeff = (long double)delta / denom;

        long long bestX = r;
        long double bestXErr = 1e100L;
        long double rr = (long double)r;

        for (long long xc = r; xc <= N - r; ++xc) {
            long double d = coeff * (long double)xc;
            long double val = rr * rr - d * d;
            if (val < 0) val = 0;
            long double pred = 2.0L * sqrtl(val);
            long double err = fabsl(pred - Lx);
            if (err < bestXErr) {
                bestXErr = err;
                bestX = xc;
            }
        }
        x = bestX;
    }

    cout << "answer " << x << ' ' << y << ' ' << r << '\n' << flush;
    return 0;
}

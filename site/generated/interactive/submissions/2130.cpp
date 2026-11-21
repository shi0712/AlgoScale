#include <bits/stdc++.h>
using namespace std;

const double EPS = 1e-6;
const int MAXC = 100000;
const int STEP = 199; // <= minimum interval length (2*100-1 = 199)

double query(int x1, int y1, int x2, int y2) {
    cout << "query " << x1 << ' ' << y1 << ' ' << x2 << ' ' << y2 << "\n";
    cout << flush;
    double val;
    if (!(cin >> val)) {
        exit(0);
    }
    if (val < -0.5) { // judge signals error
        exit(0);
    }
    return val;
}

double queryHorizontal(int y) {
    return query(0, y, MAXC, y);
}

double queryVertical(int x) {
    return query(x, 0, x, MAXC);
}

bool isPositive(double v) {
    return v > EPS;
}

// find any y with L>0 using step scanning
int findAnyY() {
    for (int y = 0; y <= MAXC; y += STEP) {
        double L = queryHorizontal(y);
        if (isPositive(L)) return y;
    }
    // fallback (should never happen)
    for (int y = 0; y <= MAXC; ++y) {
        double L = queryHorizontal(y);
        if (isPositive(L)) return y;
    }
    exit(0);
}

// find any x with L>0 using step scanning
int findAnyX() {
    for (int x = 0; x <= MAXC; x += STEP) {
        double L = queryVertical(x);
        if (isPositive(L)) return x;
    }
    // fallback (should never happen)
    for (int x = 0; x <= MAXC; ++x) {
        double L = queryVertical(x);
        if (isPositive(L)) return x;
    }
    exit(0);
}

// binary search for smallest y with L>0 in [0, hi]
int findYLow(int hi) {
    int lo = 0;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        double L = queryHorizontal(mid);
        if (isPositive(L)) hi = mid;
        else lo = mid + 1;
    }
    // lo == hi, should be positive
    double L = queryHorizontal(lo);
    if (!isPositive(L)) exit(0);
    return lo;
}

// binary search for largest y with L>0 in [lo, MAXC]
int findYHigh(int lo) {
    int hi = MAXC;
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        double L = queryHorizontal(mid);
        if (isPositive(L)) lo = mid;
        else hi = mid - 1;
    }
    double L = queryHorizontal(lo);
    if (!isPositive(L)) exit(0);
    return lo;
}

// binary search for smallest x with L>0 in [0, hi]
int findXLow(int hi) {
    int lo = 0;
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        double L = queryVertical(mid);
        if (isPositive(L)) hi = mid;
        else lo = mid + 1;
    }
    double L = queryVertical(lo);
    if (!isPositive(L)) exit(0);
    return lo;
}

// binary search for largest x with L>0 in [lo, MAXC]
int findXHigh(int lo) {
    int hi = MAXC;
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        double L = queryVertical(mid);
        if (isPositive(L)) lo = mid;
        else hi = mid - 1;
    }
    double L = queryVertical(lo);
    if (!isPositive(L)) exit(0);
    return lo;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Find y-coordinate and radius
    int y0 = findAnyY();
    int yLow = findYLow(y0);
    int yHigh = findYHigh(y0);
    int cy = (yLow + yHigh) / 2;
    int r = (yHigh - yLow) / 2 + 1;

    // Find x-coordinate (radius should match)
    int x0 = findAnyX();
    int xLow = findXLow(x0);
    int xHigh = findXHigh(x0);
    int cx = (xLow + xHigh) / 2;
    int r2 = (xHigh - xLow) / 2 + 1;
    // r2 should equal r; we trust the first computation
    (void)r2; // suppress unused warning

    cout << "answer " << cx << ' ' << cy << ' ' << r << "\n";
    cout << flush;
    return 0;
}

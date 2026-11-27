#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int64 N;

bool ask(int64 x, int64 y) {
    cout << "? " << x << ' ' << y << endl;

    string response;
    if (!(cin >> response)) exit(0);
    if (response == "true") return true;
    if (response == "false") return false;

    exit(0);
}

// Finds the right edge of the contiguous red interval containing (x, y).
int64 findRightEdge(int64 x, int64 y) {
    int64 step = 1;
    int64 lastTrue = x;

    while (x + step <= N) {
        int64 position = x + step;

        if (!ask(position, y)) {
            int64 lo = lastTrue;
            int64 hi = position;

            while (lo + 1 < hi) {
                int64 mid = lo + (hi - lo) / 2;
                if (ask(mid, y)) {
                    lo = mid;
                } else {
                    hi = mid;
                }
            }
            return lo;
        }

        lastTrue = position;
        step *= 2;
    }

    if (lastTrue == N) return N;

    if (ask(N, y)) return N;

    int64 lo = lastTrue;
    int64 hi = N;
    while (lo + 1 < hi) {
        int64 mid = lo + (hi - lo) / 2;
        if (ask(mid, y)) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return lo;
}

// Finds the left edge of the contiguous red interval containing (x, y).
int64 findLeftEdge(int64 x, int64 y) {
    int64 step = 1;
    int64 lastTrue = x;

    while (x - step >= 1) {
        int64 position = x - step;

        if (!ask(position, y)) {
            int64 lo = position;
            int64 hi = lastTrue;

            while (lo + 1 < hi) {
                int64 mid = lo + (hi - lo) / 2;
                if (ask(mid, y)) {
                    hi = mid;
                } else {
                    lo = mid;
                }
            }
            return hi;
        }

        lastTrue = position;
        step *= 2;
    }

    if (lastTrue == 1) return 1;

    if (ask(1, y)) return 1;

    int64 lo = 1;
    int64 hi = lastTrue;
    while (lo + 1 < hi) {
        int64 mid = lo + (hi - lo) / 2;
        if (ask(mid, y)) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    return hi;
}

// Determines the logical block index 0..4 on one axis.
// Shifting by 2M or 4M preserves the checkerboard color.
int locateGeneric(int64 base, int64 other, int64 M, bool horizontal) {
    auto test = [&](int64 delta) -> bool {
        int64 coordinate = base + delta;
        if (coordinate < 1 || coordinate > N) return false;

        if (horizontal) return ask(coordinate, other);
        return ask(other, coordinate);
    };

    bool rightTwo = test(2 * M);

    if (!rightTwo) {
        return test(-4 * M) ? 4 : 3;
    }

    if (test(-2 * M)) return 2;

    return test(4 * M) ? 0 : 1;
}

// The two logical indices have the same parity.
int locateWithParity(int64 base, int64 other, int64 M,
                     bool horizontal, int parity) {
    auto test = [&](int64 delta) -> bool {
        int64 coordinate = base + delta;
        if (coordinate < 1 || coordinate > N) return false;

        if (horizontal) return ask(coordinate, other);
        return ask(other, coordinate);
    };

    if (parity == 0) {
        if (!test(2 * M)) return 4;
        return test(-2 * M) ? 2 : 0;
    }

    return test(2 * M) ? 1 : 3;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int64 X0, Y0;
    if (!(cin >> N >> X0 >> Y0)) return 0;

    const int64 L = findLeftEdge(X0, Y0);
    const int64 R = findRightEdge(X0, Y0);
    const int64 M = R - L + 1;

    const int q = locateGeneric(Y0, X0, M, false);
    const int p = locateWithParity(X0, Y0, M, true, q & 1);

    int64 B;
    if (q > 0) {
        // Y0-M is in the preceding, oppositely colored logical row.
        int64 lo = Y0 - M;
        int64 hi = Y0;

        while (lo + 1 < hi) {
            int64 mid = lo + (hi - lo) / 2;
            if (ask(X0, mid)) {
                hi = mid;
            } else {
                lo = mid;
            }
        }
        B = hi;
    } else {
        // Y0+M is in the next, oppositely colored logical row.
        int64 lo = Y0;
        int64 hi = Y0 + M;

        while (lo + 1 < hi) {
            int64 mid = lo + (hi - lo) / 2;
            if (ask(X0, mid)) {
                lo = mid;
            } else {
                hi = mid;
            }
        }
        B = lo - M + 1;
    }

    const int64 originX = L - p * M;
    const int64 originY = B - q * M;
    const int64 half = (M - 1) / 2;

    const int64 answerX = originX + 2 * M + half;
    const int64 answerY = originY + 2 * M + half;

    cout << "! " << answerX << ' ' << answerY << endl;
    return 0;
}

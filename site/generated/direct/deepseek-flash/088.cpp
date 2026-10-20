#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

int n, m;
int qcnt;

int norm(int64 x) {
    x %= n;
    if (x < 0) x += n;
    return (int)x;
}

int ask(int64 offset) {
    // offset is relative to vertex 0 (0-based)
    int x = norm(offset);
    ++qcnt;
    cout << "? " << x + 1 << '\n';
    cout.flush();
    int res;
    cin >> res;
    if (!cin.good()) exit(0);
    return res;
}

void answer(int64 a, int64 b) {
    a = norm(a);
    b = norm(b);
    if (a == b) {
        // Should never happen, but avoid malformed answer.
        b = norm(b + 1);
    }
    cout << "! " << a + 1 << ' ' << b + 1 << '\n';
    cout.flush();
}

void recoverMax(int Lmax, int delta) {
    int a = norm((int64)Lmax - (m + 1));
    int b = norm((int64)a + delta);
    answer(a, b);
}

void recoverMin(int Lmin, int delta) {
    int b = norm((int64)Lmin + delta);
    answer(Lmin, b);
}

void solveMinSlope(int V, int C_start) {
    // C = [C_start, C_start + V], length V+1.
    int c = C_start + V / 2;
    int delta = ask(c); // minimum value, distance between beacons
    int h = (V - delta) / 2;
    int Lmin = C_start + h;
    recoverMin(Lmin, delta);
}

void solveMaxSlope(int V, int D, int C_start) {
    // C = [C_start, C_start + D], length D+1, level is V.
    int c = C_start + D / 2;
    int M = ask(c);       // maximum value
    int delta = n - M;    // max plateau length = distance between beacons
    int h = (D + 1 - delta) / 2;
    int Lmax = C_start + h;
    recoverMax(Lmax, delta);
}

void solveMinNoSlope(int V, int R) {
    // C is the minimum plateau: [R-V, R], length V+1.
    int L = R - V;
    recoverMin(L, V);
}

void solveMaxNoSlope(int V, int D, int R) {
    // V is the maximum value, D = n-V is max plateau length.
    // Max plateau is [R-D+1, R] relative to 0, length D.
    int S = R - D + 1;
    recoverMax(S, D);
}

void solve_case() {
    cin >> n;
    m = (n - 1) / 2;
    qcnt = 0;

    int V = ask(0);

    if (V <= m) {
        // C = {x | f(x) <= V} is an interval of length V+1 containing 0.
        // Find its clockwise right endpoint R in [0, V].
        int lo = 0, hi = V;

        while (lo < hi) {
            int mid = (lo + hi + 1) / 2; // test R >= mid
            int w = ask(mid);            // positive offset
            if (w < V) {
                // 0 is the left boundary of a larger sublevel component.
                solveMinSlope(V, 0);
                return;
            }
            if (w == V) {
                lo = mid;
            } else {
                // R < mid. Search R <= T using negative queries.
                hi = mid - 1;

                while (lo < hi) {
                    int T = (lo + hi) / 2; // test R <= T
                    int y = T - V;         // <= 0
                    int w2 = ask(y);
                    if (w2 < V) {
                        // 0 is the right boundary of a larger sublevel component.
                        solveMinSlope(V, -V);
                        return;
                    }
                    if (w2 == V) {
                        hi = T;
                    } else {
                        lo = T + 1;
                    }
                }

                int R = lo;

                // Small special case: no interior point was queried,
                // but 0 may be the right boundary of a larger component.
                int L = R - V;
                if ((R == 0 || L == 0) && V <= 3) {
                    int inside = (R == 0 ? -1 : 1);
                    int w3 = ask(inside);
                    if (w3 < V) {
                        solveMinSlope(V, (R == 0 ? -V : 0));
                        return;
                    }
                }

                solveMinNoSlope(V, R);
                return;
            }
        }

        int R = lo;
        int L = R - V;

        if ((R == 0 || L == 0) && V <= 3) {
            int inside = (R == 0 ? -1 : 1);
            int w3 = ask(inside);
            if (w3 < V) {
                solveMinSlope(V, (R == 0 ? -V : 0));
                return;
            }
        }

        solveMinNoSlope(V, R);
        return;
    } else {
        // V > m. Work with the superlevel component around the maximum plateau.
        // If V is the maximum value, the maximum plateau has D vertices.
        // If V is a slope level, the superlevel component has D+1 vertices.
        int D = n - V;
        int lo = 0, hi = D - 1;

        while (lo < hi) {
            int mid = (lo + hi + 1) / 2; // test R >= mid for assumed plateau length D
            int w = ask(mid);
            if (w > V) {
                // 0 is the left boundary of a larger superlevel component.
                solveMaxSlope(V, D, 0);
                return;
            }
            if (w == V) {
                lo = mid;
            } else {
                hi = mid - 1;

                while (lo < hi) {
                    int T = (lo + hi) / 2; // test R <= T
                    int y = T - D + 1;     // <= 0
                    int w2 = ask(y);
                    if (w2 > V) {
                        // 0 is the right boundary of a larger superlevel component.
                        solveMaxSlope(V, D, -D);
                        return;
                    }
                    if (w2 == V) {
                        hi = T;
                    } else {
                        lo = T + 1;
                    }
                }

                int R = lo;

                // For very small D, a left slope may not have been queried.
                if (R == 0 && D <= 3 && D > 1) {
                    int w3 = ask(-1);
                    if (w3 > V) {
                        solveMaxSlope(V, D, -D);
                        return;
                    }
                }

                solveMaxNoSlope(V, D, R);
                return;
            }
        }

        int R = lo;

        if (R == 0 && D <= 3 && D > 1) {
            int w3 = ask(-1);
            if (w3 > V) {
                solveMaxSlope(V, D, -D);
                return;
            }
        }
        if (R == D - 1 && D <= 3 && D > 1) {
            int w3 = ask(1);
            if (w3 > V) {
                solveMaxSlope(V, D, 0);
                return;
            }
        }

        solveMaxNoSlope(V, D, R);
        return;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;

    while (T--) {
        solve_case();
    }

    return 0;
}

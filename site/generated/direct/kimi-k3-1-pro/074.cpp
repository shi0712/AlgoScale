#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct Solver {
    int64 n, m;

    explicit Solver(int64 n_) : n(n_), m((n_ - 1) / 2) {}

    int64 norm(int64 x) const {
        x %= n;
        if (x < 0) x += n;
        return x;
    }

    int64 ask(int64 x) {
        x = norm(x);
        cout << "? " << x + 1 << '\n' << flush;
        int64 value;
        if (!(cin >> value)) exit(0);
        return value;
    }

    void answer(int64 x, int64 y) {
        x = norm(x);
        y = norm(y);
        cout << "! " << x + 1 << ' ' << y + 1 << '\n' << flush;
    }

    // Both transmitters are in the path arc
    // [offset, offset+L], whose length is at most m.
    // Their local coordinates A<B satisfy A+B=S.
    void solvePath(int64 offset, int64 L, int64 S) {
        int64 lo = max<int64>(0, S - L);
        int64 hi = min<int64>(L, (S - 1) / 2); // A<B

        while (lo < hi) {
            int64 mid = (lo + hi + 1) / 2;
            int64 value = ask(offset + mid);

            if (value == S - 2 * mid) {
                // mid <= A
                lo = mid;
            } else if (value == 2 * mid - S) {
                // mid >= B
                hi = mid - 1;
            } else {
                // A < mid < B, so value = B-A.
                int64 A = (S - value) / 2;
                int64 B = S - A;
                answer(offset + A, offset + B);
                return;
            }
        }

        int64 A = lo;
        int64 B = S - A;
        answer(offset + A, offset + B);
    }

    // One transmitter is in [0,m], the other in [m+1,n-1].
    void solveStraddling(int64 f0, int64 fm) {
        // If fm<=m, the short arc goes from a to b=a+fm.
        // Otherwise its direction is reversed.
        bool orientationA = (fm <= m);
        int64 k = orientationA ? fm : f0;
        int64 r = m - k; // r>=1 because the first answer was not m or m+1

        int64 lo = 0, hi = k - 1; // possible values of c

        auto emit = [&](int64 c) {
            if (orientationA) {
                // a=m+1-k+c=r+1+c, b=m+1+c
                answer(r + 1 + c, m + 1 + c);
            } else {
                // a=c, b=c+n-k
                answer(c, c + n - k);
            }
        };

        while (lo < hi) {
            int64 mid = (lo + hi + r) / 2;
            int64 z = mid + 1;
            int64 value = ask(z);
            int64 leftEnd = mid - r;

            if (orientationA) {
                if (value == k) {
                    // c <= mid-r
                    hi = min(hi, leftEnd);
                } else if (value == n - k) {
                    // c >= mid+1
                    lo = max(lo, mid + 1);
                } else {
                    // On a ramp:
                    // value=k+2r+2+2c-2z.
                    int64 c = leftEnd + (value - k) / 2;
                    emit(c);
                    return;
                }
            } else {
                if (value == k) {
                    // c >= mid+1
                    lo = max(lo, mid + 1);
                } else if (value == n - k) {
                    // c <= mid-r
                    hi = min(hi, leftEnd);
                } else {
                    // On a ramp: value=k+2(z-c).
                    int64 c = mid + 1 - (value - k) / 2;
                    emit(c);
                    return;
                }
            }
        }

        emit(lo);
    }

    // The first answer is m or m+1.
    void solveSpecial(int64 f0) {
        int64 z0 = (m + 1) / 2; // ceil(m/2)
        int64 value = ask(z0);

        if (value < m) {
            // Transmitters are a,b in [1,m], a+b=f0.
            // The second answer is f0-2a.
            int64 a = (f0 - value) / 2;
            int64 b = f0 - a;
            answer(a, b);
            return;
        }

        if (value > m + 1) {
            // Reflection of the previous case.
            // The second answer is n-f0+2a.
            int64 a = (value - (n - f0)) / 2;
            int64 b = f0 - a;
            answer(-a, -b);
            return;
        }

        // Now k=m, so every answer is either m or m+1.
        if (f0 == m + 1) {
            // Pairs are (a,a+m), 1<=a<=m.
            // For z in [0,m], f(z)=m exactly when a<=z.
            int64 lo = 1, hi = m;
            if (value == m) hi = z0;
            else lo = z0 + 1;

            while (lo < hi) {
                int64 mid = (lo + hi) / 2;
                int64 cur = ask(mid);
                if (cur == m) hi = mid;
                else lo = mid + 1;
            }

            answer(lo, lo + m);
        } else {
            // Pairs are (-t,m-t), 0<=t<=m.
            // f(z)=m exactly when t<=m-z.
            int64 lo = 0, hi = m;
            if (value == m) hi = m - z0;
            else lo = m - z0 + 1;

            while (lo < hi) {
                int64 mid = (lo + hi) / 2;
                int64 z = m - mid;
                int64 cur = ask(z);
                if (cur == m) hi = mid;
                else lo = mid + 1;
            }

            int64 t = lo;
            answer(-t, m - t);
        }
    }

    void solve() {
        int64 f0 = ask(0);

        if (f0 == m || f0 == m + 1) {
            solveSpecial(f0);
            return;
        }

        int64 fm = ask(m);
        int64 sum = f0 + fm;

        if (sum == 2 * m) {
            // Both transmitters are in [0,m], and their coordinate sum is f0.
            solvePath(0, m, f0);
        } else if (sum == 2 * m + 2) {
            // Both are in [m+1,n-1]. Subtract m+1 to get local coordinates.
            solvePath(m + 1, m - 1, fm - 2);
        } else {
            // sum=n: one transmitter is in each arc.
            solveStraddling(f0, fm);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;

    while (T--) {
        int64 n;
        if (!(cin >> n)) return 0;
        Solver solver(n);
        solver.solve();
    }

    return 0;
}

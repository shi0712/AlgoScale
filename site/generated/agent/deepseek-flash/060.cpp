#include <bits/stdc++.h>
using namespace std;

using ull = unsigned long long;
using i128 = __int128_t;
struct Solver {
    ull H, W, S;

    // Is L(x) = H/x + W*x/S <= T ?  (exact, avoiding overflow)
    inline bool ok(ull x, i128 T) const {
        ull q1 = H / x;
        ull r1 = H % x;
        i128 prod = (i128)W * (i128)x;
        ull q2 = (ull)(prod / (i128)S);
        ull r2 = (ull)(prod % (i128)S);
        i128 d = T - (i128)q1 - (i128)q2;
        if (d < 0) return false;
        if (d >= 2) return true;
        if (d == 0) return (r1 == 0 && r2 == 0);
        // d == 1: r1/x + r2/S <= 1
        return (i128)r1 * (i128)S + (i128)r2 * (i128)x <= (i128)S * (i128)x;
    }

    inline ull F(ull x) const {
        ull y = S / x;          // >= 1 since x <= S
        return H / x + W / y;
    }

    ull solve() {
        if (H > W) swap(H, W);
        if (H == 0) return W / S;

        // zero cuts
        if ((i128)(H + 1) * (i128)(W + 1) <= (i128)S) return 0;

        // one cut
        if ((i128)(H + 1) * (i128)(W / 2 + 1) <= (i128)S) return 1;
        if ((i128)(H / 2 + 1) * (i128)(W + 1) <= (i128)S) return 1;

        if ((i128)H * (i128)W <= (i128)S) return 2;

        // a = floor(sqrt(H*S/W)); since H <= W this is at most sqrt(S)
        ull z = (ull)(((i128)H * (i128)S) / (i128)W);
        ull a = (ull)sqrtl((long double)z);
        while ((i128)(a + 1) * (i128)(a + 1) <= (i128)z) ++a;
        while (a > 0 && (i128)a * (i128)a > (i128)z) --a;
        if (a < 1) a = 1;
        if (a > S) a = S;

        // A good initial upper bound from a few candidates around the continuous optimum.
        ull U = F(1);
        U = min(U, F(S));
        ull lc = (a > 3 ? a - 3 : 1);
        ull rc = min(S, a + 3);
        for (ull x = lc; x <= rc; ++x) U = min(U, F(x));
        if (H + 1 <= S) U = min(U, F(H + 1));

        if (U <= 1) return U;
        if (U == 2) return 2;

        i128 T = (i128)U + 2;

        // Find the first integer x in [1,S] with L(x) <= T.
        ull lo;
        if (ok(1, T)) {
            lo = 1;
        } else if (ok(a, T)) {
            ull L = 2, R = a;
            while (L < R) {
                ull mid = (L + R) >> 1;
                if (ok(mid, T)) R = mid;
                else L = mid + 1;
            }
            lo = L;
        } else {
            // No solution on [1,a]; the first possible one is a+1.
            lo = a + 1;
        }
        if (lo > S) return U; // cannot happen for valid input, kept as a safety net

        // Find the last integer x in [lo,S] with L(x) <= T.
        ull hi = lo;
        {
            ull L = lo, R = S;
            while (L < R) {
                ull mid = (L + R + 1) >> 1;
                if (ok(mid, T)) L = mid;
                else R = mid - 1;
            }
            hi = L;
        }

        ull best = U;

        // Enumerate left endpoints of blocks where floor(H/x) is constant.
        ull x = lo;
        if (x <= H) {
            ull q = H / x;
            ull l = H / (q + 1) + 1;
            if (l < x) x = H / q + 1;   // skip the partial first block
        }
        while (x <= hi) {
            if (x > H) {
                if (x == H + 1) best = min(best, F(x));
                break;
            }
            ull q = H / x;
            best = min(best, q + W / (S / x)); // F(x), reusing q
            ull r = H / q;                     // last x of this block
            x = r + 1;
            if (x > hi) break;
        }
        return best;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    if (!(cin >> t)) return 0;
    while (t--) {
        ull h, w, s;
        cin >> h >> w >> s;
        Solver sol;
        sol.H = h - 1;
        sol.W = w - 1;
        sol.S = s;
        cout << sol.solve() << '\n';
    }
    return 0;
}

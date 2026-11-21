#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

/*
 * Interactive solution for "Flatten or Concatenate".
 *
 * Structure of reachable arrays
 * ----------------------------
 * The hidden array is produced from [2^k] by (1) flatten: replace a *maximal*
 * element x of one array by two copies of x/2, and (2) concatenate: both
 * arrays become a+b.
 *
 * If the whole final array a (sum S) is split at the (unique) point q with
 * prefix sum S/2, the left part x and the right part y are both refinements of
 * one common array C obtained by repeatedly splitting maximal elements.
 *
 * A refinement of C "down to level v" (v = its maximum element) consists of:
 *   - every leaf of C with value > v fully expanded to leaves of value v
 *     (w/v leaves per leaf of value w),
 *   - a subset of the produced v-leaves split once into two v/2 leaves;
 *     at least one v-leaf must stay (otherwise the maximum would drop).
 * Therefore, if a half has maximum v and leaf count N, and C has maximum W,
 * with v = W / 2^j:
 *      N = Cnt + t + L,   0 <= t <= Cnt-1,
 *      Cnt = sum_{leaves w of C, w >= v} w/v,
 *      L   = #{leaves of C with w < v}.
 * From this one proves: if one half has a strictly larger maximum than the
 * other, then it has strictly fewer leaves.  Hence the global maximum is
 * contained in the half with the smaller number of leaves, and we may descend
 * into it until a single element remains.  Each descent step halves the number
 * of leaves, so there are at most log2(n) steps.
 *
 * To descend we must locate the split q of the current node [l,r] (with
 * prefix value pref and node value V): q is the index with
 * pref[q] = pref + V/2.  We locate it with an (interpolation-accelerated)
 * binary search over [l, r-1], which keeps the number of interactions small.
 */

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<ll> pref(n + 1, 0);
    vector<char> known(n + 1, 0);
    known[0] = 1;

    auto askPrefix = [&](int i) -> ll {
        if (known[i]) return pref[i];
        cout << "? 1 " << i << "\n" << flush;
        ll v;
        if (!(cin >> v)) exit(0);
        if (v == -1) exit(0);
        known[i] = 1;
        pref[i] = v;
        return v;
    };

    ll S = askPrefix(n);

    if (n == 1) {
        cout << "! " << S << "\n" << flush;
        return 0;
    }

    int d_hi = 31 - __builtin_clz((unsigned)n);   // exponent <= floor(log2 n)

    // Also the maximum m = S/2^d must satisfy m <= S-n+1 (all other n-1
    // elements are at least 1).  So d >= d_lo.
    int d_lo = 1;
    while (d_lo <= d_hi) {
        ll m = S >> d_lo;
        if (m <= S - n + 1) break;
        ++d_lo;
    }
    if (d_lo > d_hi) d_lo = d_hi;

    int l = 1, r = n;
    ll nodePref = 0;   // prefix sum just before position l
    ll V = S;          // value (sum) of the current node
    int t = 0;         // depth of the current node

    while (true) {
        int L = r - l + 1;

        // Always known boundaries of the current node.
        known[l - 1] = 1;
        pref[l - 1] = nodePref;
        known[r] = 1;
        pref[r] = nodePref + V;

        int loD = max(d_lo, (L == 1 ? t : t + 1));
        int hiD = min(d_hi, t + (31 - __builtin_clz((unsigned)L)));
        if (loD >= hiD) {
            cout << "! " << (S >> loD) << "\n" << flush;
            return 0;
        }

        ll target = nodePref + V / 2;

        int lo = l, hi = r - 1;
        int q = -1;
        int step = 0;
        bool forceBisect = false;
        while (lo <= hi) {
            ll plo = askPrefix(lo - 1);
            ll phi = askPrefix(hi + 1);
            int mid;
            if (forceBisect || step >= 8 || phi <= plo) {
                mid = lo + (hi - lo) / 2;
            } else {
                __int128 num = (__int128)(hi + 2 - lo) * (target - plo);
                int guess = (lo - 1) + (int)(num / (__int128)(phi - plo));
                if (guess < lo) guess = lo;
                if (guess > hi) guess = hi;
                mid = guess;
            }
            int oldSize = hi - lo + 1;
            ll v = askPrefix(mid);
            ++step;
            if (v == target) { q = mid; break; }
            if (v < target) lo = mid + 1;
            else hi = mid - 1;
            int newSize = hi - lo + 1;
            // If the probe made poor progress, force a bisection next time.
            forceBisect = ((ll)newSize * 4 > (ll)oldSize * 3);
        }
        if (q < 0) q = lo;
        if (q < l) q = l;
        if (q > r - 1) q = r - 1;

        int nl = q - l + 1;
        int nr = r - q;
        if (nl < nr) {
            r = q;
        } else if (nr < nl) {
            l = q + 1;
            nodePref = target;
        } else {
            r = q; // both halves have the same maximum
        }
        V /= 2;
        ++t;
    }
}

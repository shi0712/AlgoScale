#include <bits/stdc++.h>
using namespace std;

static int n;
static int queryCount = 0;

int ask(int l, int r, int k) {
    ++queryCount;
    cout << "? " << l << ' ' << r << ' ' << k << '\n' << flush;
    int x;
    if (!(cin >> x)) exit(0);
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    const int D = (n - 1) / 2;      // number of complete value-pairs (2t, 2t+1)
    const bool even = ((n & 1) == 0);

    int lo = 1, hi = n;
    // For even n, after the first ambiguous cut we know on which side of the
    // candidate interval n lies: 0 = n is never in future prefixes,
    // 1 = n is always in future prefixes, -1 = not determined yet.
    int extraKnown = -1;
    // True while the current candidate interval is still known to contain
    // both special values 1 and n (only relevant for even n).
    bool bothSpecial = even;

    while (lo < hi) {
        // If the candidate consists of exactly the two special values, one
        // query with k=n tells which one is at the left position.
        if (even && bothSpecial && hi == lo + 1) {
            int x = lo;
            int ans;
            if (x > 1) {
                int got = ask(x - 1, x, n);
                ans = (got == 2 ? x + 1 : x);
            } else {
                int got = ask(2, 3, n);
                ans = (got == 2 ? 1 : 2);
            }
            cout << "! " << ans << '\n' << flush;
            return 0;
        }

        int mid = (lo + hi) >> 1;

        int dP;
        if (mid == 1) {
            dP = 0;
        } else {
            int got = ask(1, mid, 2);
            dP = mid - got;
        }

        int dS;
        if (mid == n - 1) {
            dS = 0;
        } else {
            int got = ask(mid + 1, n, 2);
            dS = (n - mid) - got;
        }

        int h = D - dP - dS;
        int specialInPrefix = mid - 2 * dP - h; // [1 in P] + [n in P] if n even

        int oneInPrefix;
        if (!even) {
            oneInPrefix = specialInPrefix;
        } else {
            if (extraKnown == -1) {
                if (specialInPrefix == 0) {
                    oneInPrefix = 0;
                } else if (specialInPrefix == 2) {
                    oneInPrefix = 1;
                } else { // exactly one of 1 and n is in the prefix
                    bool nInPrefix;
                    if (mid == 1) {
                        // Prefix has length 1, so test the complement instead.
                        int got = ask(2, n, n);
                        nInPrefix = (got == 1);
                    } else {
                        int got = ask(1, mid, n);
                        nInPrefix = (got == 2);
                    }
                    if (nInPrefix) {
                        // 1 is in the suffix, n to the left of all future cuts.
                        oneInPrefix = 0;
                        extraKnown = 1;
                    } else {
                        // 1 is in the prefix, n to the right of all future cuts.
                        oneInPrefix = 1;
                        extraKnown = 0;
                    }
                    bothSpecial = false;
                }
            } else {
                oneInPrefix = specialInPrefix - extraKnown;
            }
        }

        if (oneInPrefix == 1) hi = mid;
        else lo = mid + 1;
    }

    cout << "! " << lo << '\n' << flush;
    return 0;
}

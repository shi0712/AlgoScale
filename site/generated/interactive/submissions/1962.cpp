#include <bits/stdc++.h>
using namespace std;

typedef long long ll;

// Query point x in [0, n-1]; returns the sum of the two shortest cycle distances.
static ll ask(ll x) {
    cout << "? " << (x + 1) << "\n";
    cout.flush();
    ll v;
    if (!(cin >> v)) exit(0);
    return v;
}

// Pick the plateau-bisection parameter k (1 <= k <= hi) minimizing the worst
// remaining candidate window.  A query at -k yields:
//   j >= k                 -> plateau value        (window [k, hi])
//   k-r <= j <= k-1        -> exact value of j
//   j <= k-r-1             -> the opposite plateau (window [lo, k-r-1])
// with r = m - d.
static ll chooseK(ll lo, ll hi, ll r) {
    ll kl = lo + 1, kh = hi;
    ll bestk = kl, bestMax = LLONG_MAX, bestSum = LLONG_MAX;
    auto eval = [&](ll k) {
        if (k < kl) k = kl;
        if (k > kh) k = kh;
        ll P = hi - k + 1;
        ll O = max(0LL, k - r - lo);
        ll mx = max(P, O);
        ll sum = P + O;
        if (mx < bestMax || (mx == bestMax && (sum < bestSum ||
            (sum == bestSum && k > bestk)))) {
            bestMax = mx;
            bestSum = sum;
            bestk = k;
        }
    };
    ll base = (hi + r + lo + 1) / 2;
    for (ll t = -2; t <= 2; ++t) eval(base + t);
    eval(kl);
    eval(kh);
    return bestk;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll n;
    if (!(cin >> n)) return 0;
    ll m = (n - 1) / 2;      // maximum possible distance between the transmitters
    ll inv2 = (n + 1) / 2;   // inverse of 2 modulo n (n is odd)

    ll y0 = ask(0);
    ll y1 = ask(1);
    ll s = y1 - y0;

    ll A = 0, d = 0;

    if (s == 1) {
        // (0,1) is the +1 slope edge: 0 = a + m
        d = n - 1 - y0;
        A = ((-m) % n + n) % n;
    } else if (s == -1) {
        // (0,1) is the -1 slope edge: 0 = a + m + d
        d = n - y0;
        A = ((y0 - m) % n + n) % n;
    } else if (s == 0) {
        // Both 0 and 1 are inside the same plateau.
        int ptype;               // 0 = minimum plateau, 1 = maximum plateau
        if (y0 <= m) {
            d = y0;
            ptype = 0;
        } else {
            d = n - y0;
            ptype = 1;
        }
        ll r = m - d;
        // plateau start phi = -j, with j in [0, d-1]
        ll lo = 0, hi = d - 1;
        ll exact = -1;
        while (lo < hi && exact < 0) {
            ll k = chooseK(lo, hi, r);
            ll x = (n - k) % n;
            ll val = ask(x);
            if (ptype == 0) {
                if (val == d) {
                    lo = max(lo, k);
                } else if (val == n - d) {
                    hi = min(hi, k - r - 1);
                } else {
                    ll w = (val - d) / 2;
                    exact = k - w;
                }
            } else {
                if (val == n - d) {
                    lo = max(lo, k);
                } else if (val == d) {
                    hi = min(hi, k - r - 1);
                } else {
                    ll w = m + 1 - (val + d) / 2;
                    exact = k - w;
                }
            }
        }
        ll j = (exact >= 0 ? exact : lo);
        ll phi = ((-j) % n + n) % n;
        if (ptype == 0) {
            A = phi;
        } else {
            A = ((phi - m - 1) % n + n) % n;
        }
    } else {
        // |s| == 2: 0 and 1 are on the same rising side, so z = 2a + d is known.
        ll p = y0 & 1;           // parity of d
        ll r;
        if (s == 2) r = ((-y0) % n + n) % n;
        else r = (y0 % n + n) % n;
        ll z2 = (r % 2 == p) ? r : (r + n); // z modulo 2n with correct parity
        ll mod = 2 * n;

        ll lo = 1, hi = m;
        {   // fold in the information already obtained at x = 0 and x = 1
            ll xs[2] = {0, 1};
            for (int i = 0; i < 2; ++i) {
                ll q = ((2 * xs[i] - z2) % mod + mod) % mod;
                ll d2 = min(q, mod - q);
                hi = min(hi, min(d2, n - d2));
            }
        }
        if ((lo & 1LL) != p) ++lo;
        if ((hi & 1LL) != p) --hi;

        d = -1;
        while (lo < hi) {
            ll cnt = (hi - lo) / 2 + 1;
            ll T = lo + 2 * ((cnt - 1) / 2);
            if (T >= hi) T = hi - 2;
            ll S = ((T + z2) % mod + mod) % mod;
            ll x = (S / 2) % n;
            ll val = ask(x);
            ll q = ((2 * x - z2) % mod + mod) % mod;
            ll d2 = min(q, mod - q);
            if (val == d2) {
                hi = T;
            } else {
                d = (val <= m) ? val : (n - val);
                break;
            }
        }
        if (d < 0) d = lo;
        ll zmod = ((z2 - d) % n + n) % n;
        A = (zmod * inv2) % n;
    }

    ll B = (A + d) % n;
    cout << "! " << (A + 1) << " " << (B + 1) << "\n";
    cout.flush();
    return 0;
}

// Vabank - interactive: find M in [0, 1e14] starting with balance 1.
// Query X: if X <= M balance += X ("Lucky!"), else balance -= X ("Fraudster!"),
// and if balance < X we are fired.  We use a value-function-guided search:
// the estimated cost of a state (L,U,B) is  k + number of "doublings" needed to
// fund a balanced binary search, where k = ceil(log2(U-L+1)).  Each step we pick
// the query minimising 1 + max(est(lower branch), est(upper branch)).
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef __int128 lll;

static const ll INFLL   = (ll)4e18;
static const ll MAXM    = 100000000000000LL;   // 10^14
static const ll ALPHA   = 256;                 // money-need scaling (256 == 1.0)

// ceil(log2(n)) for n >= 1
static inline ll ceilLog2(ll n) {
    if (n <= 1) return 0;
    ll r = 0, v = 1;
    while (v < n) { v <<= 1; ++r; }
    return r;
}

// Total money spent along the all-failure path of a balanced binary search
// over [L, U]  (each query is the lower-middle split point).
static ll moneyNeeded(ll L, ll U) {
    ll s = 0;
    while (U > L) {
        ll D  = U - L;
        ll nl = (D + 1) / 2;      // size of the failure (lower) part
        ll x  = L + nl;           // the probe
        s += x;
        U = x - 1;
    }
    return s;
}

// Estimated number of queries needed to resolve the interval [L, U]
// (i.e. to determine M exactly, knowing L <= M <= U) with current balance B.
static ll estimate(ll L, ll U, ll B) {
    if (U <= L) return 0;
    if (B < 1)  return INFLL / 8;
    ll D  = U - L;
    ll k  = ceilLog2(D + 1);
    ll nd = moneyNeeded(L, U);
    if (B >= nd) return k;                     // a balanced search is affordable
    ll q = (ll)(((lll)nd * ALPHA / 256 + B - 1) / B);   // money shortfall factor
    ll c = ceilLog2(q);                        // each doubling of money saves ~1 query
    return k + c + c / 2;                      // small correction for linear farming
}

// Pick the next query for a state with balance B > L.
static ll pickQuery(ll L, ll U, ll B) {
    ll D = U - L;
    vector<ll> cand;
    cand.reserve(128);
    auto add = [&](ll x) { if (x > L && x <= U) cand.push_back(x); };

    add(L + (D + 1) / 2);          // balanced split points
    add(L + D / 2);
    add(U);
    add(L + 1);                    // safest informative probe (failure resolves)
    for (int f = 1; f <= 16; ++f) add(L + max(1LL, L / f));          // geometric in L
    add(L + max(1LL, (ll)((double)L * 0.618)));                      // golden step
    for (int f = 2; f <= 16; ++f) add(L + max(1LL, D / f));          // geometric in D
    if (B > L + 1) {                                              // jumps limited by money
        add(B - 1);
        add(B - 2);
        for (int f = 2; f <= 8; ++f) add(L + (B - L) / f);
    }
    sort(cand.begin(), cand.end());
    cand.erase(unique(cand.begin(), cand.end()), cand.end());

    ll best = INFLL / 8, bx = -1;
    for (ll x : cand) {
        if (x > B) continue;                       // must be affordable
        if (B - x <= 0 && x - 1 > L) continue;     // a failure would leave us stuck
        ll lower = estimate(L, x - 1, B - x);      // M < x  branch
        if (lower >= INFLL / 16) continue;
        ll upper = estimate(x, U, B + x);          // M >= x branch
        ll v = 1 + max(lower, upper);
        if (v < best || (v == best && x > bx)) { best = v; bx = x; }
    }
    return bx;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll L = 0, U = MAXM, B = 1;     // invariant: L <= M <= U
    string resp;
    while (L < U) {
        bool farm = (B <= L);      // a query of value <= L is guaranteed to succeed
        ll x;
        if (farm) {
            x = min(B, L);         // best safe money maker
            if (x < 1) { x = L + 1; farm = false; }
            if (x > U) { x = U; farm = false; }
        } else {
            x = pickQuery(L, U, B);
            if (x < 1 || x > B || x > U) x = L + 1;   // defensive fallback
        }

        cout << "? " << x << '\n';
        cout.flush();

        if (!(cin >> resp)) return 0;
        if (resp == "Lucky!") {
            if (!farm) L = x;      // farm queries carry no information about M
            B += x;
        } else if (resp == "Fraudster!") {
            B -= x;
            U = x - 1;
        } else {
            // "Fired!" (should never happen) or closed stream: stop immediately.
            return 0;
        }
    }
    cout << "! " << L << '\n';
    cout.flush();
    return 0;
}

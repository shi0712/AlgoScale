#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef __int128 i128;
typedef long double ld;

static inline ll ceil_div(ll a, ll b) {
    return (ll)(((i128)a + b - 1) / b);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    const ll LIMIT = 60000;

    while (t--) {
        ll h, w, s;
        cin >> h >> w >> s;

        if ((i128)h * w <= s) {
            cout << 0 << '\n';
            continue;
        }

        ll A = min(h, s);

        ld a0 = sqrt((ld)h * s / (ld)w);
        ll ca = (ll)llround(a0);
        if (ca < 1) ca = 1;
        if (ca > A) ca = A;

        ld M = (ld)h * w / (ld)s;
        ld p0 = sqrt(M);
        ll cp = (ll)llround(p0);
        if (cp < 1) cp = 1;
        if (cp > h) cp = h;

        ll best = LLONG_MAX;

        // search around optimal a
        ll l = max(1LL, ca - LIMIT);
        ll r = min(A, ca + LIMIT);
        for (ll a = l; a <= r; ++a) {
            ll b = s / a;
            if (b == 0) continue;
            ll p = ceil_div(h, a);
            ll q = ceil_div(w, b);
            best = min(best, p + q);
        }

        // search around optimal p
        l = max(1LL, cp - LIMIT);
        r = min(h, cp + LIMIT);
        for (ll p = l; p <= r; ++p) {
            ll a = ceil_div(h, p);
            if (a > s) continue;
            ll b = s / a;
            if (b == 0) continue;
            ll q = ceil_div(w, b);
            best = min(best, p + q);
        }

        // boundaries
        auto eval_a = [&](ll a) -> ll {
            if (a < 1 || a > A) return LLONG_MAX;
            ll b = s / a;
            if (b == 0) return LLONG_MAX;
            return ceil_div(h, a) + ceil_div(w, b);
        };
        auto eval_p = [&](ll p) -> ll {
            if (p < 1 || p > h) return LLONG_MAX;
            ll a = ceil_div(h, p);
            if (a > s) return LLONG_MAX;
            ll b = s / a;
            if (b == 0) return LLONG_MAX;
            return p + ceil_div(w, b);
        };

        best = min(best, eval_a(1));
        best = min(best, eval_a(A));
        best = min(best, eval_p(1));
        best = min(best, eval_p(h));

        cout << best - 2 << '\n';
    }

    return 0;
}

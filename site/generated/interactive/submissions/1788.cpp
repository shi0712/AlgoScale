#include <bits/stdc++.h>
using namespace std;

using ll = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll n;
    if (!(cin >> n)) return 0;

    map<ll, ll> cache;

    auto pref = [&](ll x) -> ll {
        if (x <= 0) return 0;
        auto it = cache.find(x);
        if (it != cache.end()) return it->second;

        cout << "? 1 " << x << '\n' << flush;
        ll ans;
        cin >> ans;
        if (ans == -1) exit(0);

        cache[x] = ans;
        return ans;
    };

    ll S = pref(n);

    // Find the first position where prefix inversion count becomes positive.
    // Since L1 = j - i >= 2, this position is i + 1.
    ll lo = 1, hi = n - 2;
    while (lo < hi) {
        ll mid = (lo + hi) / 2;
        ll val = pref(mid);
        if (val > 0) hi = mid;
        else lo = mid + 1;
    }
    ll i = lo - 1;

    // Find k: the first position where prefix inversion count equals total S.
    // We know i + 2 < k, so using i + 2 also guarantees querying k - 1.
    lo = i + 2;
    hi = n;
    while (lo < hi) {
        ll mid = (lo + hi) / 2;
        ll val = pref(mid);
        if (val >= S) hi = mid;
        else lo = mid + 1;
    }
    ll k = lo;

    // Let L2 = k - j + 1.
    // pref(k - 1) = C(L1, 2) + C(L2 - 1, 2),
    // S          = C(L1, 2) + C(L2, 2).
    // Hence S - pref(k - 1) = L2 - 1.
    ll beforeK = pref(k - 1);
    ll L2 = S - beforeK + 1;

    ll j = k - L2 + 1;

    cout << "! " << i << ' ' << j << ' ' << k << '\n' << flush;
    return 0;
}

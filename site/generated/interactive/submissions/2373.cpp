#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static ll Q(ll l, ll r) {
    printf("? %lld %lld\n", l, r);
    fflush(stdout);
    ll v;
    if (scanf("%lld", &v) != 1) exit(0);
    if (v < 0) exit(0);
    return v;
}
static ll C2(ll m) { return m * (m - 1) / 2; }
int main() {
    ll n;
    if (scanf("%lld", &n) != 1) return 0;
    ll lo = 2, hi = n - 2;
    while (lo < hi) {
        ll mid = (lo + hi) / 2;
        if (Q(1, mid) > 0) hi = mid;
        else lo = mid + 1;
    }
    ll i = lo - 1;
    ll T = Q(1, n);
    ll a = 2, b = n - i - 1;
    while (a < b) {
        ll mid = (a + b + 1) / 2;
        if (Q(1, i + mid - 1) == C2(mid)) a = mid;
        else b = mid - 1;
    }
    ll L1 = a;
    ll R = T - C2(L1);
    ll L2 = (ll)((1.0L + sqrtl(1.0L + 8.0L * (long double)R)) / 2.0L);
    while (C2(L2) > R) L2--;
    while (C2(L2 + 1) <= R) L2++;
    ll j = i + L1;
    ll k = j + L2 - 1;
    printf("! %lld %lld %lld\n", i, j, k);
    fflush(stdout);
    return 0;
}

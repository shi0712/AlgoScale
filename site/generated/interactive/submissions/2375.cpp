#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

ll ask(ll l, ll r) {
    printf("xor %lld %lld\n", l, r);
    fflush(stdout);
    ll x;
    if (scanf("%lld", &x) != 1) exit(0);
    if (x == -1) exit(0);
    return x;
}

int main() {
    ll n;
    if (scanf("%lld", &n) != 1) return 0;
    ll X = ask(1, n);
    ll a;
    if (X != 0) {
        ll lo = 1, hi = n;
        while (lo < hi) {
            ll mid = lo + (hi - lo) / 2;
            if (ask(1, mid) != 0) hi = mid;
            else lo = mid + 1;
        }
        a = lo;
    } else {
        int k = 1;
        while (true) {
            ll m = (1LL << k) - 1;
            if (m > n) m = n;
            if (ask(1, m) != 0) break;
            k++;
        }
        ll lo = 1LL << (k - 1), hi = (1LL << k) - 1;
        if (hi > n) hi = n;
        while (lo < hi) {
            ll mid = lo + (hi - lo) / 2;
            if (ask(1, mid) != 0) hi = mid;
            else lo = mid + 1;
        }
        a = lo;
    }
    ll lo = a + 1, hi = n;
    while (lo < hi) {
        ll mid = lo + (hi - lo) / 2;
        if (ask(a + 1, mid) != 0) hi = mid;
        else lo = mid + 1;
    }
    ll b = lo;
    ll c = X ^ a ^ b;
    printf("ans %lld %lld %lld\n", a, b, c);
    fflush(stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main() {
    int n;
    if (!(cin >> n)) return 0;
    auto query = [&](int l, int r) -> ll {
        printf("? %d %d\n", l, r);
        fflush(stdout);
        ll v;
        if (!(cin >> v)) exit(0);
        if (v == -1) exit(0);
        return v;
    };
    ll S = query(1, n);
    if (n == 1) {
        printf("! %lld\n", S);
        fflush(stdout);
        return 0;
    }
    map<ll, int> byP;
    byP[0] = 0;
    byP[S] = n;
    int l = 1, r = n;
    ll s = S, base = 0;
    auto finishAvg = [&](ll sum, ll L) {
        ll q = 1;
        while (q * L < sum) q <<= 1;
        printf("! %lld\n", q);
        fflush(stdout);
    };
    while (true) {
        ll L = r - l + 1;
        if (L == 1) {
            printf("! %lld\n", s);
            fflush(stdout);
            return 0;
        }
        if (s % 2 == 1) {
            finishAvg(s, L);
            return 0;
        }
        ll T = base + s / 2;
        int p = -1;
        bool found = false;
        bool noB = false;
        while (true) {
            auto it = byP.lower_bound(T);
            ll Py = it->first;
            int y = it->second;
            if (Py == T) {
                p = y;
                found = true;
                break;
            }
            auto pv = prev(it);
            ll Px = pv->first;
            int x = pv->second;
            ll lo = max<ll>(x + 1, (ll)y - (Py - T));
            ll hi = min<ll>(y, (ll)x + (T - Px));
            if (lo > hi) {
                // should not happen; fallback
                lo = x + 1;
                hi = y;
            }
            if (lo == hi) {
                if (lo == y) {
                    noB = true;
                    break;
                }
                ll v = base + query(l, (int)lo);
                byP[v] = (int)lo;
                if (v == T) {
                    p = (int)lo;
                    found = true;
                } else {
                    noB = true;
                }
                break;
            } else {
                int mid = (int)((lo + hi) / 2);
                ll v = base + query(l, mid);
                byP[v] = mid;
                if (v == T) {
                    p = mid;
                    found = true;
                    break;
                }
            }
        }
        if (!found || noB) {
            finishAvg(s, L);
            return 0;
        }
        ll lenLeft = p - l + 1;
        ll lenRight = r - p;
        if (lenLeft <= lenRight) {
            r = p;
            s /= 2;
        } else {
            l = p + 1;
            base = T;
            s /= 2;
        }
    }
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static ll n;

static ll ask(ll l, ll r){
    printf("xor %lld %lld\n", l, r);
    fflush(stdout);
    ll v;
    if(scanf("%lld", &v) != 1) exit(0);
    if(v < 0) exit(0);
    return v;
}

static void give(ll a, ll b, ll c){
    printf("ans %lld %lld %lld\n", a, b, c);
    fflush(stdout);
    exit(0);
}

int main(){
    if(scanf("%lld", &n) != 1) return 0;
    if(n == 3) give(1, 2, 3);
    ll S = ask(1, n);
    int t0 = 0;
    while(t0 < 62 && (1LL << t0) <= n) ++t0;
    ll L = 0; int t = t0;
    ll x = 0, T = 0, L2 = 0; int k = 0;
    while(true){
        ll sz = 1LL << t;
        ll lo = max(L, 1LL), hi = min(L + sz - 1, n);
        ll cnt = hi - lo + 1;
        if(cnt == 3) give(lo, lo + 1, lo + 2);
        if(cnt == 4){
            ll W = lo ^ (lo + 1) ^ (lo + 2) ^ (lo + 3);
            ll e = W ^ S;
            ll r3[3]; int c = 0;
            for(ll i = 0; i < 4; i++){ ll val = lo + i; if(val != e) r3[c++] = val; }
            if(c == 3) give(r3[0], r3[1], r3[2]);
        }
        ll half = sz >> 1;
        ll le = L + half - 1;
        if(le >= n){ --t; continue; }
        ll v = ask(max(L, 1LL), le);
        if(v == S){ --t; continue; }
        if(v == 0){ L += half; --t; continue; }
        k = t - 1;
        x = ((S >> k) & 1LL) ? (S ^ v) : v;
        T = S ^ x;
        L2 = ((x >> k) & 1LL) ? L : (L + half);
        break;
    }
    int j = 63 - __builtin_clzll((unsigned long long)T);
    for(int tt = k; tt >= j + 2; --tt){
        ll sz = 1LL << tt;
        ll lo = max(L2, 1LL), hi = min(L2 + sz - 1, n);
        ll cnt = hi - lo + 1;
        if(cnt == 2) give(x, lo, hi);
        if(cnt == 3){
            ll W = lo ^ (lo + 1) ^ (lo + 2);
            ll e = W ^ T;
            ll r2[2]; int c = 0;
            for(ll i = 0; i < 3; i++){ ll val = lo + i; if(val != e) r2[c++] = val; }
            if(c == 2) give(x, r2[0], r2[1]);
        }
        ll half = sz >> 1;
        ll le = L2 + half - 1;
        if(le >= n) continue;
        ll v = ask(max(L2, 1LL), le);
        if(v == 0) L2 += half;
    }
    {
        ll sz = 1LL << (j + 1);
        ll lo = max(L2, 1LL), hi = min(L2 + sz - 1, n);
        if(hi - lo + 1 == 2) give(x, lo, hi);
        ll yl = max(L2, 1LL), yh = min(L2 + (1LL << j) - 1, n);
        ll y = (yl == yh) ? yl : ask(yl, yh);
        give(x, y, y ^ T);
    }
    return 0;
}

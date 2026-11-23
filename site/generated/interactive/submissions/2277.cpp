#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static ll S = 0, L = 0;
static int f = 0, cw = 0, Q = 0;

static ll ask(ll x){
    printf("- %lld\n", x);
    fflush(stdout);
    ll r;
    if(scanf("%lld", &r) != 1) exit(0);
    if(r < 0) exit(0);
    ++Q;
    return r;
}
static int bitlen(ll v){ int b=0; while(v>0){ ++b; v>>=1; } return b; }
static int maxpc(ll v){
    if(v <= 0) return 0;
    int best = __builtin_popcountll((unsigned long long)v);
    int above = 0;
    for(int b = 40; b >= 0; --b){
        if((v >> b) & 1LL){
            if(above + b > best) best = above + b;
            ++above;
        }
    }
    return best;
}
int main(){
    ll c;
    if(scanf("%lld", &c) != 1) return 0;
    cw = (int)c;
    while(cw > 0){
        ll Wmax = (1000000000LL - S - L) >> f;
        int space = bitlen(Wmax);
        if(space < cw) space = cw;
        ll V = Wmax - ((1LL << cw) - 1);
        int maxr = maxpc(V);
        if(maxr > space - 1) maxr = space - 1;
        if(maxr < 0) maxr = 0;
        bool compact = false;
        if(f >= 4 && cw >= 6){
            ll mask = (1LL << f) - 1;
            if(((S + L) & mask) == mask) compact = true;
        }
        bool useB = (cw >= 2) && (Q + 1 + maxr <= 30) &&
                    (compact || 3LL * cw >= 2LL * space);
        if(useB){
            ll x = L + ((1LL << f) * ((1LL << cw) - 1));
            ll r = ask(x);
            S += x;
            if(r == 0){ L = 0; cw = 0; break; }
            ll plo = (ll)cw - r; if(plo < 0) plo = 0;
            L = 0; f += (int)plo; cw = (int)r;
            ll x2 = (1LL << f);
            ll r2 = ask(x2);
            S += x2;
            ll t = r2 - (ll)cw + 1; if(t < 0) t = 0;
            L = (1LL << f) * ((1LL << t) - 1);
            f += (int)t + 1;
            cw -= 1;
        } else {
            ll x = L + (1LL << f);
            ll r = ask(x);
            S += x;
            ll t = r - (ll)cw + 1; if(t < 0) t = 0;
            L = (1LL << f) * ((1LL << t) - 1);
            f += (int)t + 1;
            cw -= 1;
        }
    }
    printf("! %lld\n", S + L);
    fflush(stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

ll n;

ll query(ll l, ll r){
    printf("xor %lld %lld\n", l, r);
    fflush(stdout);
    ll v;
    if(scanf("%lld",&v)!=1) exit(0);
    if(v==-1) exit(0);
    return v;
}
ll Q(ll y){
    if(y<=0) return 0;
    return query(1,y);
}

struct PS{ ll Hlo,Hhi,sz,half; };

PS prep(ll D, ll L, ll R){
    int h = 63 - __builtin_clzll((unsigned long long)D);
    PS s;
    s.sz = 1LL<<(h+1); s.half = 1LL<<h;
    s.Hlo = L >> (h+1); s.Hhi = R >> (h+1);
    auto yOf=[&](ll H){ return H*s.sz + s.half - 1; };
    while(s.Hlo<=s.Hhi && yOf(s.Hlo) < L) s.Hlo++;
    while(s.Hlo<=s.Hhi && yOf(s.Hhi) >= R) s.Hhi--;
    return s;
}
bool feasible(ll D, ll L, ll R){
    if(D<=0) return false;
    if(L>R) return false;
    PS s=prep(D,L,R);
    return s.Hlo<=s.Hhi;
}
// returns p (smaller of pair), pair xor D, in [L,R], known element e
ll pairSearch(ll D, ll L, ll R, ll e){
    PS s=prep(D,L,R);
    ll Hlo=s.Hlo,Hhi=s.Hhi;
    while(true){
        ll H = Hlo + (Hhi-Hlo)/2;
        ll y = H*s.sz + s.half - 1;
        ll g = Q(y) ^ (e<=y ? e : 0);
        if(g==0) Hlo=H+1;
        else if(g==D) Hhi=H-1;
        else return g;
        if(Hlo>Hhi){
            // should not happen
            return g;
        }
    }
}

int main(){
    if(scanf("%lld",&n)!=1) return 0;
    if(n==3){
        printf("ans 1 2 3\n"); fflush(stdout); return 0;
    }
    ll T = Q(n);
    ll a,b,c;
    if(T!=0){
        ll z0=0,t0=n;
        ll x,v;
        while(true){
            x = z0 + (t0-z0)/2;
            v = Q(x);
            if(v==0) z0=x;
            else if(v==T) t0=x;
            else break;
        }
        bool A = (v> z0 && v<=x) && feasible(T^v, x+1, t0);
        ll cc = T^v;
        bool B = (cc> x && cc<=t0) && feasible(v, z0+1, x);
        if(A && B){
            ll r = query(v,v);
            if(r==v) B=false; else A=false;
        }
        if(A){
            a=v;
            ll D=T^v;
            ll p=pairSearch(D,x+1,t0,a);
            b=p; c=p^D;
        } else {
            c=cc;
            ll p=pairSearch(v,z0+1,x,c);
            a=p; b=p^v;
        }
    } else {
        ll x=0,v=0;
        for(int j=1;j<62;j++){
            x=(1LL<<j)-1;
            if(x>n) break;
            v=Q(x);
            if(v!=0) break;
        }
        a=v;
        ll p=pairSearch(a,x+1,n,a);
        b=p; c=p^a;
    }
    printf("ans %lld %lld %lld\n",a,b,c);
    fflush(stdout);
    return 0;
}

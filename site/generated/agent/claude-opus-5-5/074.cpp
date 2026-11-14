#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

ll n, h; int pi_;
int qcnt;
#ifdef LOCAL
ll HA, HB; // hidden 0-indexed
ll cdist(ll x, ll y){ ll d = llabs(x-y)%n; return min(d, n-d);}
ll rawq(ll x){ qcnt++; return cdist(x,HA)+cdist(x,HB);}
#else
ll rawq(ll x){
    qcnt++;
    printf("? %lld\n", x+1); fflush(stdout);
    ll r; if(scanf("%lld",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}
#endif
ll ask(ll x){ x%=n; if(x<0)x+=n; ll r=rawq(x); return pi_? n-r : r; }
ll fdiv(ll a, ll b){ return a/b - ((a%b!=0)&&((a<0)!=(b<0))); }

// candidates M = lo + 2j, j in [0,K). Keep those with clamp(D(X,M),L,n-L)==r
bool filt(ll &lo, ll &K, ll X, ll r, ll L){
    ll e0 = lo - X, e1 = e0 + 2*(K-1);
    vector<pair<ll,ll>> T, pieces;
    for(int k=-3;k<=3;k++){
        ll s=2*n*k;
        if(r==L) T.push_back({s-L,s+L});
        else if(r==n-L) T.push_back({s+n-L,s+n+L});
        else { T.push_back({s+r,s+r}); T.push_back({s-r,s-r}); }
    }
    for(auto [A,B]:T){
        ll a=max(A,e0), b=min(B,e1);
        if(a>b) continue;
        ll ja = fdiv(a-e0+1,2), jb = fdiv(b-e0,2);
        if(ja>jb) continue;
        pieces.push_back({ja,jb});
    }
    if(pieces.empty()) return false;
    sort(pieces.begin(),pieces.end());
    vector<pair<ll,ll>> mg;
    for(auto p:pieces){
        if(!mg.empty() && p.first<=mg.back().second+1) mg.back().second=max(mg.back().second,p.second);
        else mg.push_back(p);
    }
    if(mg.size()!=1) return false;
    lo = lo + 2*mg[0].first; K = mg[0].second-mg[0].first+1;
    return true;
}

pair<ll,ll> finish(ll M, ll L){
    if(pi_) M+=n;
    M%=2*n; if(M<0) M+=2*n;
    ll a=((M-L)/2%n+n)%n, b=((M+L)/2%n+n)%n;
    if((M-L)<0){ a = fdiv(M-L,2); a%=n; if(a<0)a+=n; }
    return {a,b};
}

pair<ll,ll> solve(){
    h=(n-1)/2; pi_=0; qcnt=0;
    ll v0=rawq(0);
    if(v0>h){ pi_=1; v0=n-v0; }
    ll t=(v0+1)/2;
    ll r=ask(t);
    if(r<v0) return finish(v0, r);
    if(r>n-v0 || r==2*t+v0){
        ll M=((-v0)%(2*n)+2*n)%(2*n);
        ll L=ask(M/2);
        return finish(-v0, L);
    }
    ll L=v0;
    ll lo=-v0+(pi_?1:0), K=(v0-lo)/2+1;
    if(!filt(lo,K,2*t,r,L)) { fprintf(stderr,"FAIL1\n"); exit(1);} 
    while(K>1){
        ll c=(K+1)/2;
        ll X = ((lo-L)%2==0) ? lo+2*c-2-L : lo+2*c-1-L;
        ll rr=ask(fdiv(X,2));
        if(!filt(lo,K,X,rr,L)) { fprintf(stderr,"FAIL2\n"); exit(1);} 
    }
    return finish(lo, L);
}

int main(){
#ifdef LOCAL
    int T; scanf("%d",&T); int mx=0; mt19937_64 rng(T);
    // modes
    while(T--){
        ll a,b; scanf("%lld %lld %lld",&n,&a,&b);
        HA=a; HB=b;
        auto [x,y]=solve();
        mx=max(mx,qcnt);
        if(!((x==a&&y==b)||(x==b&&y==a))||qcnt>30){ printf("WRONG n=%lld a=%lld b=%lld got %lld %lld q=%d\n",n,a,b,x,y,qcnt); return 0;}
    }
    printf("OK maxq=%d\n",mx);
#else
    int T; if(scanf("%d",&T)!=1) return 0;
    while(T--){
        if(scanf("%lld",&n)!=1) return 0;
        auto [x,y]=solve();
        printf("! %lld %lld\n",x+1,y+1); fflush(stdout);
    }
#endif
}

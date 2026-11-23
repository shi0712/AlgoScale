#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static ll n;
#ifdef LOCAL
static ll Ji,Jj,Jk; static int qcnt=0;
#endif
static ll C2(ll x){ return x<=1?0:x*(x-1)/2; }
static ll ov(ll l,ll r,ll x,ll y){ ll lo=max(l,x),hi=min(r,y); return hi>=lo?hi-lo+1:0; }
static ll hval(ll l,ll r,ll a,ll b,ll s){ return C2(ov(l,r,s,s+a-1))+C2(ov(l,r,s+a,s+a+b-1)); }
static ll ask(ll l,ll r){
#ifdef LOCAL
    qcnt++; return hval(l,r,Jj-Ji,Jk-Jj+1,Ji);
#else
    printf("? %lld %lld\n",l,r); fflush(stdout);
    ll x; if(scanf("%lld",&x)!=1) exit(0); if(x<0) exit(0); return x;
#endif
}
struct Hyp{ ll a,b,lo,hi; };
static vector<Hyp> H;
static ll totalS(){ ll s=0; for(auto&h:H) s+=h.hi-h.lo+1; return s; }
static void segs(const Hyp&h,ll l,ll r,vector<pair<ll,ll>>&out){
    out.clear();
    ll L=h.a+h.b;
    ll k[6]={l,r-h.a+1,l-h.a,r-L+1,l-L,r+1};
    ll pts[16]; int c=0;
    pts[c++]=h.lo; pts[c++]=h.hi+1;
    for(int i=0;i<6;i++) for(int d=0;d<2;d++){ ll p=k[i]+d; if(p>h.lo&&p<=h.hi) pts[c++]=p; }
    sort(pts,pts+c); c=unique(pts,pts+c)-pts;
    for(int i=0;i+1<c;i++) out.push_back({pts[i],pts[i+1]-1});
}
// first t in [x,y] with pred true (pred monotone false..true), y+1 if none
template<class F> static ll firstTrue(ll x,ll y,F f){ ll lo=x,hi=y+1; while(lo<hi){ ll m=lo+(hi-lo)/2; if(f(m)) hi=m; else lo=m+1;} return lo; }
static void update(ll l,ll r,ll ans){
    vector<Hyp> NH; vector<pair<ll,ll>> sg;
    for(auto&h:H){
        segs(h,l,r,sg);
        auto f=[&](ll s){return hval(l,r,h.a,h.b,s);};
        vector<pair<ll,ll>> got;
        for(auto [x0,x1]:sg){
            ll m=firstTrue(x0,x1,[&](ll t){return t==x1||f(t+1)>=f(t);});
            // left nonincreasing [x0,m]
            ll p=firstTrue(x0,m,[&](ll t){return f(t)<=ans;});
            ll q=firstTrue(x0,m,[&](ll t){return f(t)<ans;})-1;
            if(p<=q) got.push_back({p,q});
            if(m+1<=x1){
                p=firstTrue(m+1,x1,[&](ll t){return f(t)>=ans;});
                q=firstTrue(m+1,x1,[&](ll t){return f(t)>ans;})-1;
                if(p<=q) got.push_back({p,q});
            }
        }
        sort(got.begin(),got.end());
        for(auto [p,q]:got){
            if(!NH.empty()&&NH.back().a==h.a&&NH.back().b==h.b&&NH.back().hi+1>=p) NH.back().hi=max(NH.back().hi,q);
            else NH.push_back({h.a,h.b,p,q});
        }
    }
    H=NH;
}
static const ll LIM=2000;
static chrono::steady_clock::time_point T0;
static double elapsed(){ return chrono::duration<double>(chrono::steady_clock::now()-T0).count(); }
struct Metric{ long double mx, sq; bool operator<(const Metric&o)const{ if(mx!=o.mx) return mx<o.mx; return sq<o.sq; } };
static Metric evalQ(ll l,ll r,ll S){
    unordered_map<ll,ll> mp; mp.reserve(64);
    if(S<=LIM && elapsed()<7){
        for(auto&h:H) for(ll s=h.lo;s<=h.hi;s++) mp[hval(l,r,h.a,h.b,s)]++;
        long double mx=0,sq=0; for(auto&kv:mp){ mx=max(mx,(long double)kv.second); sq+=(long double)kv.second*kv.second; }
        return {mx,sq};
    }
    vector<pair<ll,ll>> sg; long double ramp=0;
    for(auto&h:H){
        segs(h,l,r,sg);
        for(auto [x0,x1]:sg){
            ll v0=hval(l,r,h.a,h.b,x0);
            if(x0==x1){ mp[v0]++; continue; }
            ll v1=hval(l,r,h.a,h.b,x1);
            if(v0==v1 && hval(l,r,h.a,h.b,x0+1)==v0 && hval(l,r,h.a,h.b,x1-1)==v1) mp[v0]+=x1-x0+1;
            else ramp+=x1-x0+1;
        }
    }
    long double mx=ramp>0?2:0,sq=ramp*2;
    for(auto&kv:mp){ mx=max(mx,(long double)kv.second); sq+=(long double)kv.second*kv.second; }
    return {mx,sq};
}
static pair<ll,ll> choose(){
    ll S=totalS();
    vector<ll> P={1,n};
    for(auto&h:H){
        ll L=h.a+h.b, span=h.hi-h.lo+L;
        int K=16;
        for(int k=0;k<=K;k++){
            ll base=h.lo+(ll)((long double)span*k/K);
            for(ll off: {0LL,-L+1,-1LL,1LL}){ ll p=base+off; if(p>=1&&p<=n) P.push_back(p); }
        }
        P.push_back(min(n,h.hi+L-1)); P.push_back(max(1LL,h.lo));
    }
    sort(P.begin(),P.end()); P.erase(unique(P.begin(),P.end()),P.end());
    size_t cap = S<=LIM ? 110 : 200;
    cap=min(cap,(size_t)max(20.0,sqrt(2.5e6/max<size_t>(1,H.size()))));
    { double e=elapsed(); if(e>4) cap=60; if(e>7) cap=25; }
    if(P.size()>cap){ // subsample
        vector<ll> Q; for(size_t i=0;i<cap;i++) Q.push_back(P[i*P.size()/cap]); Q.push_back(1); Q.push_back(n);
        sort(Q.begin(),Q.end()); Q.erase(unique(Q.begin(),Q.end()),Q.end()); P=Q;
    }
    Metric best{1e30L,1e30L}; ll bl=1,br=n;
    for(size_t i=0;i<P.size();i++) for(size_t j=i;j<P.size();j++){
        Metric m=evalQ(P[i],P[j],S);
        if(m<best){best=m;bl=P[i];br=P[j];}
    }
    // hill climb
    for(ll step=1LL<<30; step>=1 && elapsed()<8.5; step>>=1){
        bool imp=true; int it=0;
        while(imp && it<20){ imp=false; it++;
            ll dl[]={-step,step,0,0,-step,step,-step,step}, dr[]={0,0,-step,step,-step,step,step,-step};
            for(int d=0;d<8;d++){
                ll l=bl+dl[d], r=br+dr[d];
                if(l<1||r>n||l>r) continue;
                Metric m=evalQ(l,r,S);
                if(m<best){best=m;bl=l;br=r;imp=true;}
            }
        }
    }
    return {bl,br};
}
int main(){
    T0=chrono::steady_clock::now();
#ifdef LOCAL
    { ll a,b,c,d; if(scanf("%lld %lld %lld %lld",&a,&b,&c,&d)!=4) return 1; n=a; Ji=b; Jj=c; Jk=d; }
#else
    if(scanf("%lld",&n)!=1) return 0;
#endif
    if(n==4){
#ifdef LOCAL
        printf("%s %d\n",(Ji==1&&Jj==3&&Jk==4)?"OK":"WRONG",qcnt);
#else
        printf("! 1 3 4\n"); fflush(stdout);
#endif
        return 0;
    }
    ll T=ask(1,n);
    // find all a<=b with C2(a)+C2(b)=T, a,b>=2
    {
        ll a=2,b=2; ll Ca=1,Cb=1;
        // b max: largest with C2(b)<=T-1
        b=(ll)sqrtl(2.0L*T)+2; while(C2(b)>T-1) b--; Cb=C2(b);
        while(a<=b){
            ll s=Ca+Cb;
            if(s<T){ Ca+=a; a++; }
            else if(s>T){ b--; Cb-=b; }
            else {
                if(a+b<=n){ H.push_back({a,b,1,n-a-b+1}); if(a!=b) H.push_back({b,a,1,n-a-b+1}); }
                Ca+=a; a++; b--; Cb-=b;
            }
        }
    }
    while(totalS()>1){
        auto [l,r]=choose();
        ll ans=ask(l,r);
        update(l,r,ans);
    }
    Hyp h=H[0];
#ifdef LOCAL
    bool ok=(h.lo==Ji&&h.lo+h.a==Jj&&h.lo+h.a+h.b-1==Jk);
    printf("%s %d\n",ok?"OK":"WRONG",qcnt);
#else
    printf("! %lld %lld %lld\n",h.lo,h.lo+h.a,h.lo+h.a+h.b-1); fflush(stdout);
#endif
}

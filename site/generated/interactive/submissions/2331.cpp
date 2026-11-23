#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

ll n, T;
static inline ll C2(ll m){ return m<2?0:m*(m-1)/2; }

ll ask(ll l, ll r){
    printf("? %lld %lld\n", l, r); fflush(stdout);
    ll v; if(scanf("%lld",&v)!=1) exit(0);
    if(v<0) exit(0);
    return v;
}

struct P{ ll L1,L2,S; vector<pair<ll,ll>> iv; };
vector<P> ps;

ll f(const P&p, ll i, ll x){
    ll o1 = min(x, i+p.L1-1) - i + 1; if(o1<0) o1=0;
    ll o2 = min(x, i+p.S-1) - (i+p.L1) + 1; if(o2<0) o2=0;
    return C2(o1)+C2(o2);
}
// solve C2(m)=w, m>=2 ; return -1 if none
ll solveM(ll w){
    if(w<1) return -1;
    ull d = 1 + 8ULL*(ull)w;
    ull s = (ull)sqrtl((long double)d);
    while(s*s>d) s--;
    while((s+1)*(s+1)<=d) s++;
    if(s*s!=d || (s%2)==0) return -1;
    return (ll)((1+s)/2);
}

ll Zc(ll x){ ll t=0; for(auto&p:ps) for(auto&[lo,hi]:p.iv){ ll a=max(lo,x); if(hi>=a) t+=hi-a+1;} return t;}
ll Tcc(ll x){ ll t=0; for(auto&p:ps) for(auto&[lo,hi]:p.iv){ ll b=min(hi,x-p.S); if(b>=lo) t+=b-lo+1;} return t;}

int main(){
    if(scanf("%lld",&n)!=1) return 0;
    T = ask(1,n);
    // enumerate pairs
    {
        ll b=2; ll cb=1;
        // a largest with C2(a) <= T-1
        ll a = (ll)sqrtl((long double)2*(T-1)) + 2;
        while(C2(a) > T-1) a--;
        ll ca = C2(a);
        while(2*cb <= T){
            ll rem = T - cb;
            while(ca > rem){ ca -= (a-1); a--; }
            if(ca==rem && a>=2){
                if(a+b<=n){
                    ps.push_back({a,b,a+b,{}});
                    if(a!=b) ps.push_back({b,a,a+b,{}});
                }
            }
            cb += b; b++;
        }
    }
    for(auto&p:ps) p.iv.push_back({1, n-p.S+1});
    while(true){
        // remove empty
        vector<P> np;
        for(auto&p:ps){ vector<pair<ll,ll>> v; for(auto&pr:p.iv) if(pr.first<=pr.second) v.push_back(pr); if(!v.empty()){ p.iv=v; np.push_back(p);} }
        ps=np;
        ll N=0; for(auto&p:ps) for(auto&[lo,hi]:p.iv) N+=hi-lo+1;
        if(N<=1){
            if(N==1){
                for(auto&p:ps) for(auto&[lo,hi]:p.iv){
                    ll i=lo; printf("! %lld %lld %lld\n", i, i+p.L1, i+p.S-1); fflush(stdout); return 0;
                }
            }
            return 0;
        }
        // balanced x
        ll lo=2, hi=n, fx=-1;
        while(lo<=hi){ ll m=(lo+hi)/2; if(Tcc(m)>=Zc(m)){fx=m;hi=m-1;} else lo=m+1; }
        ll bx;
        if(fx==-1) bx=n;
        else{
            bx=fx;
            if(fx-1>=2){
                ll v1=max(Zc(fx),Tcc(fx)), v2=max(Zc(fx-1),Tcc(fx-1));
                if(v2<v1) bx=fx-1;
            }
        }
        ll LIM = max<ll>(300, 3*(ll)ps.size());
        ll x=bx;
        if(N<=LIM){
            vector<pair<int,ll>> cs;
            for(int q=0;q<(int)ps.size();q++) for(auto&[l,h]:ps[q].iv) for(ll i=l;i<=h;i++) cs.push_back({q,i});
            vector<ll> xs; xs.push_back(bx);
            for(auto&[q,i]:cs){ xs.push_back(i+1); xs.push_back(i+ps[q].L1); }
            sort(xs.begin(),xs.end()); xs.erase(unique(xs.begin(),xs.end()),xs.end());
            ll best=LLONG_MAX; vector<ll> vals(cs.size());
            for(ll cx:xs){
                if(cx<1||cx>n) continue;
                for(size_t t=0;t<cs.size();t++) vals[t]=f(ps[cs[t].first],cs[t].second,cx);
                sort(vals.begin(),vals.end());
                ll mb=0,run=0;
                for(size_t t=0;t<vals.size();t++){ if(t>0&&vals[t]==vals[t-1]) run++; else run=1; mb=max(mb,run);}
                if(mb<best){best=mb;x=cx;}
            }
        }
        ll v = ask(1,x);
        for(auto&p:ps){
            vector<pair<ll,ll>> nv;
            for(auto&[l,h]:p.iv){
                if(v==0){ nv.push_back({max(l,x),h}); }
                else if(v==T){ nv.push_back({l,min(h,x-p.S)}); }
                else{
                    vector<ll> cand;
                    ll m=solveM(v); if(m>=2) cand.push_back(x-m+1);
                    ll w=v-C2(p.L1);
                    if(w==0) cand.push_back(x-p.L1);
                    else if(w>0){ ll m2=solveM(w); if(m2>=2) cand.push_back(x-p.L1-m2+1); }
                    sort(cand.begin(),cand.end()); cand.erase(unique(cand.begin(),cand.end()),cand.end());
                    for(ll i:cand) if(i>=l&&i<=h&&f(p,i,x)==v) nv.push_back({i,i});
                }
            }
            p.iv=nv;
        }
    }
}

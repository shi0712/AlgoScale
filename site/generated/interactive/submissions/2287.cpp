#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
ll n,S; vector<pair<ll,ll>> o1,o2;
static inline ll tri(ll x){ return x<2?0:x*(x-1)/2; }
ll froot(ll v){ if(v<=0) return 1; ll x=(ll)((1.0L+sqrtl(1.0L+8.0L*(long double)v))/2.0L); if(x<1)x=1;
    while(x>1&&tri(x)>v)--x; while(tri(x+1)<=v)++x; return x; }
ll croot(ll t){ if(t<=0) return 0; ll x=froot(t); if(tri(x)<t)++x; return x; }
ll isq(ll v){ if(v<0) return -1; ll x=(ll)sqrtl((long double)v); if(x<0)x=0;
    while(x>0&&x*x>v)--x; while((x+1)*(x+1)<=v)++x; return x; }
ll q1(ll r){ for(auto&p:o1) if(p.first==r) return p.second;
    printf("? 1 %lld\n",r); fflush(stdout); ll v; if(scanf("%lld",&v)!=1) exit(0); if(v<0) exit(0);
    o1.push_back(make_pair(r,v)); return v; }
ll q2(ll l){ for(auto&p:o2) if(p.first==l) return p.second;
    printf("? %lld %lld\n",l,n); fflush(stdout); ll v; if(scanf("%lld",&v)!=1) exit(0); if(v<0) exit(0);
    o2.push_back(make_pair(l,v)); return v; }
bool consistent(ll i,ll A,ll B){ if(i<1||A<2||B<2) return false; ll j=i+A,k=j+B-1;
    if(k>n||j>=k||i>=j) return false; if(tri(A)+tri(B)!=S) return false;
    for(auto&p:o1){ ll r=p.first; ll x=max(0LL,min(r,j-1)-i+1), y=max(0LL,min(r,k)-j+1);
        if(tri(x)+tri(y)!=p.second) return false; }
    for(auto&p:o2){ ll l=p.first; ll x=max(0LL,(j-1)-max(l,i)+1), y=max(0LL,k-max(l,j)+1);
        if(tri(x)+tri(y)!=p.second) return false; }
    return true; }
pair<ll,ll> findAB(ll i){ ll Alo=2,Ahi=min(froot(S-1),n-i-1); vector<ll> ex;
    for(auto&p:o1){ ll r=p.first,v=p.second; if(v==0) continue; ll m=r-i+1; if(m<2) continue;
        if(v==tri(m)) Alo=max(Alo,m); else if(v==S) Ahi=min(Ahi,m-2);
        else { ll d=4*v+2*m-m*m; if(d>=0){ ll s=isq(d); if(s*s==d){ if(((m+s)&1LL)==0) ex.push_back((m+s)/2);
            if(s>0&&((m-s)&1LL)==0) ex.push_back((m-s)/2);} } } }
    auto chk=[&](ll A)->ll{ if(A<2||A<Alo||A>Ahi) return -1; ll tb=S-tri(A); if(tb<1) return -1;
        ll B=froot(tb); if(tri(B)!=tb||B<2) return -1; if(!consistent(i,A,B)) return -1; return B; };
    sort(ex.begin(),ex.end()); ex.erase(unique(ex.begin(),ex.end()),ex.end());
    vector<pair<ll,ll>> cs; for(ll A:ex){ ll B=chk(A); if(B>0) cs.push_back(make_pair(A,B)); }
    if(cs.size()==1) return cs[0];
    if(cs.empty()&&Ahi>=Alo&&Ahi-Alo<=3000000){ for(ll A=Alo;A<=Ahi;++A){ ll B=chk(A); if(B>0) cs.push_back(make_pair(A,B)); }
        if(cs.size()==1) return cs[0]; }
    ll w=q2(i+1); ll A=S-w+1; ll B=chk(A); if(B>0) return make_pair(A,B);
    ll tb=S-tri(A); return make_pair(A,froot(tb)); }
int main(){ if(scanf("%lld",&n)!=1) return 0; S=q1(n);
    ll Mmin=max(4LL,2+croot(S-1)); ll L=1,R=n-Mmin+1; if(R<L)R=L;
    ll ai=-1,aA=-1,aB=-1;
    auto apply1=[&](ll r,ll v){ if(v==0) L=max(L,r); else if(v==S) R=min(R,r-Mmin+1);
        else { R=min(R,r-froot(v)+1); ll mm=1+isq(1+4*v); L=max(L,r-mm+1);} };
    while(true){
        if(L>=R){ ai=L; break; }
        ll r; if(R-L<=2) r=L+1; else { r=(L+R+Mmin-1)/2; if(r<L+1)r=L+1; if(r>R)r=R; }
        ll v=q1(r); apply1(r,v);
        if(L>=R){ ai=L; break; }
        if(v>0&&v<S){
            ll u=r, v2=q1(u+1); apply1(u+1,v2);
            if(L>=R){ ai=L; break; }
            bool ok2=false,ok1=false; ll i2=0,A2=0,B2=0,i1=0;
            ll y=v2-v;
            if(y>=0&&tri(y)<=v){ ll t=v-tri(y); ll A=froot(t);
                if(tri(A)==t&&A>=2){ ll ic=u-A-y+1; ll tb=S-tri(A);
                    if(tb>=1){ ll B=froot(tb);
                        if(tri(B)==tb&&B>=2&&consistent(ic,A,B)){ ok2=true;i2=ic;A2=A;B2=B; } } } }
            { ll m=froot(v); if(tri(m)==v&&m>=2){ ll ic=u-m+1;
                if(ic>=L&&ic<=R&&tri(m+1)==v2){ ok1=true;i1=ic; } } }
            if(ok2&&!ok1){ ai=i2;aA=A2;aB=B2; break; }
            if(ok1&&!ok2){ ai=i1; break; }
            if(ok1&&ok2){ if(i1==i2){ ai=i1; break; }
                ll w=q1(i1); apply1(i1,w);
                if(w==0) ai=i1; else { ai=i2;aA=A2;aB=B2; } break; }
            while(true){ if(L>=R){ ai=L; break; } ll w=q1(R); apply1(R,w); if(w==0){ ai=R; break; } }
            break;
        }
    }
    if(aA<0){ pair<ll,ll> pr=findAB(ai); aA=pr.first; aB=pr.second; }
    printf("! %lld %lld %lld\n",ai,ai+aA,ai+aA+aB-1); fflush(stdout);
    return 0; }

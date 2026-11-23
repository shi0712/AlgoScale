#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static ll N,X0,Y0,Mmax;
static vector<array<ll,3>> hist;
static map<pair<ll,ll>,int> memo;
static bool ask(ll x,ll y){
    if(x<1||x>N||y<1||y>N) return false;
    pair<ll,ll> k(x,y); auto it=memo.find(k);
    if(it!=memo.end()) return it->second!=0;
    cout<<"? "<<x<<" "<<y<<endl; string s;
    if(!(cin>>s)) exit(0); if(s=="-1") exit(0);
    int r=(s=="true"||s=="1")?1:0; memo[k]=r; hist.push_back({x,y,(ll)r}); return r!=0;
}
static bool rowOK(ll L,ll M){ ll R=L+5*M-1; if(L<1||R>N||X0<L||X0>R) return false;
    ll p0=((X0-L)/M)&1;
    for(auto&h:hist){ if(h[1]!=Y0) continue; ll x=h[0]; int r=(int)h[2],p=0;
        if(x>=L&&x<=R) p=((((x-L)/M)&1)==p0)?1:0; if(p!=r) return false; } return true; }
static bool fullOK(ll L,ll B,ll M){ ll R=L+5*M-1,T=B+5*M-1;
    if(L<1||R>N||B<1||T>N) return false;
    for(auto&h:hist){ ll x=h[0],y=h[1]; int r=(int)h[2],p=0;
        if(x>=L&&x<=R&&y>=B&&y<=T){ ll u=(x-L)/M,v=(y-B)/M; p=(((u+v)&1)==0)?1:0; }
        if(p!=r) return false; } return true; }
static ll findStart(ll c,ll M,bool xa,ll S,int pm){
    ll wl=max(1LL,c-M+1),wh=c; vector<pair<ll,ll>> sg;
    for(ll j=0;j<5;j++){ if(!((pm>>(j&1))&1)) continue; ll l=max(wl,j*M+1),r=min(wh,j*M+S); if(l<=r) sg.push_back({l,r}); }
    sort(sg.begin(),sg.end()); vector<pair<ll,ll>> ms;
    for(auto&p:sg){ if(!ms.empty()&&p.first<=ms.back().second+1) ms.back().second=max(ms.back().second,p.second); else ms.push_back(p); }
    if(ms.empty()) return wl; ll tot=0; for(auto&p:ms) tot+=p.second-p.first+1;
    auto kth=[&](ll k)->ll{ for(auto&p:ms){ ll c2=p.second-p.first+1; if(k<=c2) return p.first+k-1; k-=c2; } return ms.back().second; };
    ll lo=1,hi=tot; while(lo<hi){ ll mid=lo+(hi-lo)/2,c2=kth(mid); bool r=xa?ask(c2,Y0):ask(X0,c2); if(r) hi=mid; else lo=mid+1; }
    return kth(lo);
}
int main(){ ios_base::sync_with_stdio(false); cin.tie(nullptr);
    if(!(cin>>N>>X0>>Y0)) return 0; Mmax=N/5; ll M=-1,aX=-1; if(Mmax<5) M=3;
    if(M<0){ ll lo=X0,hi=0,st=1;
        while(true){ ll x=X0+st; if(x>N||st>Mmax){ hi=min(min(x,N+1),X0+Mmax); break; }
            if(ask(x,Y0)){ lo=x; st*=2; } else { hi=x; break; } }
        while(hi-lo>1){ ll mid=lo+(hi-lo)/2; if(ask(mid,Y0)) lo=mid; else hi=mid; }
        ll eX=lo,Dx=eX-X0+1, cap=min(min(Mmax,eX),max(eX/5,N-eX));
        ll loV=Dx-1,hiV=0,v=Dx;
        while(true){ if(v>cap){ hiV=cap; break; } ll x=eX-v; if(x<1){ hiV=v; break; }
            if(ask(x,Y0)){ loV=v; v*=2; } else { hiV=v; break; } }
        while(hiV-loV>64){ ll mid=loV+(hiV-loV)/2; if(mid%2==0) mid++; if(ask(eX-mid,Y0)) loV=mid; else hiV=mid; }
        vector<ll> cs;
        for(ll t=loV+1;t<=hiV;t++){ if(t<3||t>Mmax||(t&1)==0) continue;
            for(int k=1;k<=5;k++){ ll L=eX-(ll)k*t+1; if(L>=1&&L+5*t-1<=N&&rowOK(L,t)){ cs.push_back(t); break; } } }
        if(cs.empty()) for(ll t=loV+1;t<=hiV;t++) if(t>=3&&(t&1)) cs.push_back(t);
        ll l0=0,h0=(ll)cs.size()-1;
        while(l0<h0){ ll mid=(l0+h0)/2; if(ask(eX-cs[mid],Y0)) l0=mid+1; else h0=mid; }
        M=cs[l0]; aX=eX-M+1; }
    ll S=N-5*M+1; if(aX<0) aX=findStart(X0,M,true,S,3);
    int pm=0; for(int i=0;i<5;i++){ ll L=aX-(ll)i*M; if(L>=1&&L+5*M-1<=N&&rowOK(L,M)) pm|=1<<(i&1); } if(!pm) pm=3;
    ll bY=findStart(Y0,M,false,S,pm);
    vector<pair<int,int>> cs;
    for(int i=0;i<5;i++){ ll L=aX-(ll)i*M; if(L<1||L+5*M-1>N) continue;
        for(int j=0;j<5;j++){ if((i+j)&1) continue; ll B=bY-(ll)j*M; if(B<1||B+5*M-1>N) continue; if(fullOK(L,B,M)) cs.push_back({i,j}); } }
    if(cs.empty()) for(int i=0;i<5;i++){ ll L=aX-(ll)i*M; if(L<1||L+5*M-1>N) continue;
        for(int j=0;j<5;j++){ if((i+j)&1) continue; ll B=bY-(ll)j*M; if(B<1||B+5*M-1>N) continue; cs.push_back({i,j}); } }
    int n=cs.size(); if(n==0){ cout<<"! "<<X0<<" "<<Y0<<endl; return 0; }
    unsigned mask=(1u<<n)-1;
    if(n>1){
        vector<pair<unsigned,pair<ll,ll>>> qs;
        for(int k=-4;k<=4;k++)for(int l=-4;l<=4;l++){ if((k+l)&1) continue; if(k==0&&l==0) continue;
            ll bx=aX+(ll)k*M,by=bY+(ll)l*M; ll xl=max(1LL,bx),xr=min(N,bx+M-1),yl=max(1LL,by),yr=min(N,by+M-1);
            if(xl>xr||yl>yr) continue; unsigned m=0;
            for(int c=0;c<n;c++){ ll L=aX-(ll)cs[c].first*M,B=bY-(ll)cs[c].second*M; bool pr=false;
                if(xl>=L&&xl<=L+5*M-1&&yl>=B&&yl<=B+5*M-1){ ll u=(xl-L)/M,v=(yl-B)/M; pr=(((u+v)&1)==0); }
                if(pr) m|=1u<<c; }
            if(m==0||m==((1u<<n)-1)) continue; qs.push_back({m,{xl,yl}}); }
        sort(qs.begin(),qs.end());
        qs.erase(unique(qs.begin(),qs.end(),[](const pair<unsigned,pair<ll,ll>>&a,const pair<unsigned,pair<ll,ll>>&b){return a.first==b.first;}),qs.end());
        int FULL=1<<n; vector<int> mw(FULL,-1); vector<ll> msu(FULL,0);
        function<pair<int,ll>(unsigned)> rec=[&](unsigned mk)->pair<int,ll>{
            int pc=__builtin_popcount(mk); if(pc<=1) return {0,0};
            if(mw[mk]>=0) return {mw[mk],msu[mk]};
            int bw=1000; ll bs=LLONG_MAX;
            for(auto&q:qs){ unsigned t=mk&q.first,f=mk&~q.first; if(!t||!f) continue;
                auto a=rec(t),b=rec(f); int w=1+max(a.first,b.first); ll s=(ll)pc+a.second+b.second;
                if(w<bw||(w==bw&&s<bs)){bw=w;bs=s;} }
            mw[mk]=bw; msu[mk]=bs; return {bw,bs}; };
        while(__builtin_popcount(mask)>1){
            int pc=__builtin_popcount(mask),bw=1000,best=-1; ll bs=LLONG_MAX;
            for(int qi=0;qi<(int)qs.size();qi++){ unsigned t=mask&qs[qi].first,f=mask&~qs[qi].first; if(!t||!f) continue;
                auto a=rec(t),b=rec(f); int w=1+max(a.first,b.first); ll s=(ll)pc+a.second+b.second;
                if(w<bw||(w==bw&&s<bs)){bw=w;bs=s;best=qi;} }
            if(best<0) break;
            bool r=ask(qs[best].second.first,qs[best].second.second);
            mask=r?(mask&qs[best].first):(mask&~qs[best].first);
        }
    }
    int idx=__builtin_ctz(mask);
    ll L=aX-(ll)cs[idx].first*M,B=bY-(ll)cs[idx].second*M;
    ll cx=L+(5*M-1)/2, cy=B+(5*M-1)/2;
    cout<<"! "<<cx<<" "<<cy<<endl;
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef unsigned __int128 u128; typedef long long ll; typedef unsigned long long ull;
static ll n,A,B,K,c1,c2,minval; static u128 maxval,CAP;
static inline u128 Av(ll d){return (u128)d*((u128)A*(u128)d+(u128)c1);}
static inline u128 fv(ll d){return (u128)d*((u128)c2-(u128)A*(u128)d);}
static u128 isq(u128 x){if(!x)return 0;u128 r=(u128)sqrtl((long double)x);while(r&&r*r>x)--r;while((r+1)*(r+1)<=x)++r;return r;}
static ll cD(u128 Y){u128 dc=(u128)c1*(u128)c1+(u128)4*(u128)A*Y;u128 s=isq(dc);u128 dd=(Y<<1)/((u128)c1+s);ll d=(dd>=(u128)n)?(n-1):(ll)dd;while(d>0&&Av(d)>Y)--d;while(d<n-1&&Av(d+1)<=Y)++d;return d;}
static ll cF(u128 Y){if(fv(n-1)<=Y)return n-1;u128 dc=(u128)c2*(u128)c2-(u128)4*(u128)A*Y;u128 s=isq(dc);u128 dd=(Y<<1)/((u128)c2+s);ll d=(dd>=(u128)n)?(n-1):(ll)dd;while(d>0&&fv(d)>Y)--d;while(d<n-1&&fv(d+1)<=Y)++d;return d;}
static inline ull fd(u128 num,ull den,double dd){ll h=(ll)(ull)(num>>32),l=(ll)(ull)(num&(u128)0xFFFFFFFFULL);double nd=(double)h*4294967296.0+(double)l;ull q=(ull)(nd/dd);u128 pr=(u128)q*(u128)den;if(pr>num)--q;else if(num-pr>=(u128)den)++q;return q;}
static u128 cnt_le(u128 Y){
    if(Y<(u128)minval)return 0; if(Y>=maxval)return CAP;
    ll D=cD(Y),Df=cF(Y);
    u128 cnt=(u128)Df*(u128)n-(((u128)Df*(u128)(Df+1))>>1);
    if(cnt>=CAP)return CAP; if(D<=Df)return cnt;
    ll d=Df+1; ull tA=2ULL*(ull)A,den=tA*(ull)d; double dd=(double)den,ds=(double)tA;
    u128 del=(u128)A*(u128)(2*d+1)+(u128)c1;
    if(Y<(((u128)1)<<63)){
        ull num=(ull)(Y-Av(d)),dl=(ull)del,cl=(ull)cnt,CP=(ull)CAP;
        for(;d<=D;++d){ull q=(ull)((double)(ll)num/dd);ll r=(ll)(num-q*den);if(r<0)--q;else if(r>=(ll)den)++q;cl+=q+1;if(cl>=CP)return CAP;num-=dl;dl+=tA;den+=tA;dd+=ds;}
        return (u128)cl;
    }
    u128 num=Y-Av(d);
    for(;d<=D;++d){ull q=fd(num,den,dd);cnt+=q+1;if(cnt>=CAP)return CAP;num-=del;del+=tA;den+=tA;dd+=ds;}
    return cnt;
}
static u128 nxt(u128 lo){ll D1=cF(lo),D2=cD(lo);u128 best=maxval;if(D2<n-1)best=Av(D2+1);
    for(ll d=D1+1;d<=D2;++d){u128 ad=Av(d);ull den=2ULL*(ull)A*(ull)d;ull q=fd(lo-ad,den,(double)den);u128 v=ad+(u128)den*(u128)(q+1);if(v<best)best=v;} return best;}
static u128 mxle(u128 hi){ll Dh=cD(hi),Df=cF(hi);u128 best=0;if(Df>=1)best=fv(Df);
    for(ll d=Df+1;d<=Dh;++d){u128 ad=Av(d);ull den=2ULL*(ull)A*(ull)d;ull q=fd(hi-ad,den,(double)den);u128 v=ad+(u128)den*(u128)q;if(v>best)best=v;} return best;}
static u128 enr(u128 lo,u128 hi,u128 clo,u128 chi){
    ll need=(ll)((u128)K-clo);ll Dh=cD(hi),Dl=cF(lo);
    vector<u128> v; v.reserve((size_t)(chi-clo)+1);
    for(ll d=Dl+1;d<=Dh;++d){u128 ad=Av(d);ull den=2ULL*(ull)A*(ull)d;double dd=(double)den;
        ll js=0; if(lo>=ad){js=(ll)fd(lo-ad,den,dd)+1;}
        ll je; if(hi>=fv(d))je=n-d-1; else je=(ll)fd(hi-ad,den,dd);
        for(ll j=js;j<=je;++j)v.push_back(ad+(u128)den*(u128)j);}
    nth_element(v.begin(),v.begin()+(need-1),v.end()); return v[need-1];
}
static void pr(u128 v){char b[64];int l=0;if(!v){putchar('0');return;}while(v){b[l++]=(char)('0'+(int)(v%10));v/=10;}while(l)putchar(b[--l]);}
int main(){
    ll a,b; if(scanf("%lld %lld %lld %lld",&n,&K,&a,&b)!=4)return 0;
    ll g0=__gcd(a,2*b); A=a/g0; B=(2*b)/g0; c1=2*A+B; c2=2*A*n+B; minval=3*A+B; maxval=fv(n-1);
    u128 tot=((u128)n*(u128)(n-1))>>1;
    CAP=(tot<(u128)200000000000000ULL)?tot:(u128)200000000000000ULL;
    ll cc=2*n-1; u128 dc=(u128)cc*(u128)cc-(u128)8*(u128)K; u128 s=isq(dc);
    u128 dd=((u128)4*(u128)K)/((u128)cc+s); ll d0=(dd>=(u128)n)?(n-1):(ll)dd; if(d0<1)d0=1;
    auto gg=[&](ll d)->u128{return (u128)d*(u128)n-(((u128)d*(u128)(d+1))>>1);};
    while(d0>1&&gg(d0-1)>=(u128)K)--d0; while(d0<n-1&&gg(d0)<(u128)K)++d0;
    u128 U=fv(d0);
    u128 lo=(u128)minval-1,hi=U,clo=0,chi=cnt_le(hi),Y=hi;
    const u128 LIM=1000000;
    long double flo=-(long double)K,fhi=(long double)chi-(long double)K; int last=0,stall=0;
    while(true){
        if(hi-lo<=1){Y=hi;break;}
        if(clo+1==(u128)K){Y=nxt(lo);break;}
        if(chi==(u128)K){Y=mxle(hi);break;}
        if(chi-clo<=LIM){Y=enr(lo,hi,clo,chi);break;}
        u128 w=hi-lo,mid;
        if(stall>=8||fhi<=0.0L||flo>=0.0L)mid=lo+(w>>1);
        else{long double t=(-flo)/(fhi-flo);long double m=(long double)lo+(long double)w*t;
            if(m<=(long double)lo)mid=lo+1;else if(m>=(long double)hi)mid=hi-1;
            else{mid=(u128)m;if(mid<=lo)mid=lo+1;if(mid>=hi)mid=hi-1;}}
        u128 cm=cnt_le(mid);
        if(cm<(u128)K){lo=mid;clo=cm;if(last==-1)fhi*=0.5L;flo=(long double)cm-(long double)K;last=-1;}
        else{hi=mid;chi=cm;if(last==1)flo*=0.5L;fhi=(long double)cm-(long double)K;last=1;}
        if((hi-lo)*2>w)++stall;else stall=0;
    }
    ll rem=(ll)(Y%(u128)A); ll g=__gcd(rem,A); if(g==0)g=A;
    u128 p=Y/(u128)g; ll q=A/g;
    pr(p); putchar(' '); printf("%lld\n",q);
    return 0;
}

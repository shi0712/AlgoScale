#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64; typedef __uint128_t u128;
static const u64 MOD = 998244353;

struct Mont {
    u64 n, ninv, r2, one;
    Mont() {} explicit Mont(u64 mod){ init(mod); }
    void init(u64 mod){
        n=mod; ninv=mod;
        for(int i=0;i<6;++i) ninv *= 2 - n*ninv;
        ninv = 0ULL - ninv;
        r2 = (u128)(0ULL - (u128)n) % n;
        one = reduce((u128)r2);
    }
    inline u64 reduce(u128 t) const {
        u64 m = (u64)t * ninv;
        u64 res = (u64)((t + (u128)m*n) >> 64);
        return res >= n ? res-n : res;
    }
    inline u64 tr(u64 x) const { return reduce((u128)x*r2); }
    inline u64 mul(u64 a,u64 b) const { return reduce((u128)a*b); }
    u64 power(u64 a,u64 e) const { u64 r=one; while(e){ if(e&1) r=mul(r,a); a=mul(a,a); e>>=1;} return r; }
};
static bool isPrime(u64 n){
    if(n<2) return false;
    for(u64 p:{2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL,19ULL,23ULL,29ULL,31ULL,37ULL}) if(n%p==0) return n==p;
    Mont mt(n); u64 d=n-1; int s=__builtin_ctzll(d); d>>=s; u64 nm1=mt.tr(n-1);
    for(u64 a:{2ULL,3ULL,5ULL,7ULL,11ULL,13ULL,17ULL}){
        u64 x=mt.power(mt.tr(a),d);
        if(x==mt.one||x==nm1) continue;
        bool comp=true;
        for(int i=1;i<s;++i){ x=mt.mul(x,x); if(x==nm1){comp=false;break;} }
        if(comp) return false;
    }
    return true;
}
static u64 rs=88172645463325252ULL;
static inline u64 rnd(){ rs^=rs<<13; rs^=rs>>7; rs^=rs<<17; return rs; }
static u64 pollard(u64 n){
    if(n%2==0) return 2;
    Mont mt(n);
    while(true){
        u64 c=mt.tr(rnd()%(n-1)+1), y=mt.tr(rnd()%n);
        u64 x=0,ys=0,q=mt.one,g=1,r=1; const u64 B=128;
        auto f=[&](u64 v){ u64 t=mt.mul(v,v)+c; if(t>=n) t-=n; return t; };
        while(g==1){
            x=y;
            for(u64 i=0;i<r;++i) y=f(y);
            for(u64 k=0;k<r&&g==1;k+=B){
                ys=y; u64 lim=min(B,r-k);
                for(u64 i=0;i<lim;++i){ y=f(y); u64 df=x>y?x-y:y-x; q=mt.mul(q,df); }
                g=std::__gcd(q,n);
            }
            r<<=1; if(r>(1ULL<<22)) break;
        }
        if(g==n){ g=1; while(g==1){ ys=f(ys); u64 df=x>ys?x-ys:ys-x; g=std::__gcd(df,n);} }
        if(g!=n&&g!=1) return g;
    }
}
static u64 gp[24]; static int ge[24]; static int gk;
static void addf(u64 p){ for(int i=0;i<gk;++i) if(gp[i]==p){ge[i]++;return;} gp[gk]=p; ge[gk]=1; gk++; }
static void recf(u64 n){
    if(n==1) return;
    if(isPrime(n)){ addf(n); return; }
    u64 s=(u64)sqrtl((long double)n);
    while(s>0&&s*s>n) s--;
    while((s+1)*(s+1)<=n) s++;
    if(s*s==n){ recf(s); recf(s); return; }
    u64 d=pollard(n); recf(d); recf(n/d);
}
static u64 ts[16384], cs[16384];
int main(){
    int T; if(scanf("%d",&T)!=1) return 0;
    static const u64 small[]={2,3,5,7,11,13,17,19,23,29,31,37,41,43,47};
    while(T--){
        u64 n,m; if(scanf("%llu %llu",&n,&m)!=2) return 0;
        gk=0; u64 nn=n;
        for(u64 p:small){ if(p*p>nn) break; while(nn%p==0){ nn/=p; addf(p);} }
        recf(nn);
        int cnt=1; ts[0]=1; cs[0]=1;
        for(int idx=0;idx<gk;++idx){
            u64 p=gp[idx]; int e=ge[idx]; int F=(e+1)/2;
            u64 pw[64],pm[64]; pw[0]=1; pm[0]=1;
            u64 pmod=p%MOD, pm1=(p-1)%MOD;
            for(int i=1;i<=e;++i){ pw[i]=pw[i-1]*p; pm[i]=pm[i-1]*pmod%MOD; }
            auto phif=[&](int j)->u64{ return j==0?1ULL: pm[j-1]*pm1%MOD; };
            u64 lt[40],lc[40];
            lt[0]=1; lc[0]=phif(e);
            for(int f=1;f<=F;++f){
                lt[f]=pw[f];
                int i1=2*f-1;
                u64 v=pm[i1]*phif(e-i1)%MOD;
                if(2*f<=e) v=(v+pm[2*f]*phif(e-2*f))%MOD;
                lc[f]=v;
            }
            int old=cnt;
            for(int f=1;f<=F;++f){
                u64 a=lt[f], b=lc[f];
                u64* dt=ts+(size_t)f*old; u64* dc=cs+(size_t)f*old;
                for(int i=0;i<old;++i){ dt[i]=ts[i]*a; dc[i]=cs[i]*b%MOD; }
            }
            u64 b0=lc[0];
            for(int i=0;i<old;++i) cs[i]=cs[i]*b0%MOD;
            cnt=old*(F+1);
        }
        u64 ans=0;
        for(int i=0;i<cnt;++i){
            u64 t=ts[i];
            u64 q = (t>m)?1ULL:(m/t+1);
            if(q>=MOD) q%=MOD;
            ans=(ans+cs[i]*q)%MOD;
        }
        printf("%llu\n",ans);
    }
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull; typedef unsigned u32;
const u32 MOD=998244353;
ll pw(ll b,ll e){ll r=1;b%=MOD;if(b<0)b+=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
// Montgomery
const u32 NINV=[]{u32 x=MOD;for(int i=0;i<5;i++)x*=2-MOD*x;return -x;}(); // -MOD^{-1} mod 2^32
const u32 R2=(u32)((((unsigned __int128)1)<<64)%MOD);
static inline u32 mmul(u32 a,u32 b){ull t=(ull)a*b;u32 m=(u32)t*NINV;u32 r=(t+(ull)m*MOD)>>32;return r>=MOD?r-MOD:r;}
static inline u32 toM(u32 a){return mmul(a,R2);}
vector<u32> rt,irt; int rtn=0;
void prep(int n){
    if(rtn>=n)return; rtn=n; rt.assign(n,0);irt.assign(n,0);
    for(int len=1;len<n;len<<=1){
        ll w=pw(3,(MOD-1)/(2*len)),iw=pw(w,MOD-2),c=1,ic=1;
        for(int j=0;j<len;j++){rt[len+j]=toM(c);irt[len+j]=toM(ic);c=c*w%MOD;ic=ic*iw%MOD;}
    }
}
#include <immintrin.h>
#define AVX __attribute__((target("avx2")))
AVX static inline __m256i vmul(__m256i a,__m256i b){
    const __m256i P=_mm256_set1_epi32(MOD),NI=_mm256_set1_epi32(NINV);
    __m256i pe=_mm256_mul_epu32(a,b);
    __m256i po=_mm256_mul_epu32(_mm256_srli_epi64(a,32),_mm256_srli_epi64(b,32));
    __m256i me=_mm256_mul_epu32(pe,NI),mo=_mm256_mul_epu32(po,NI);
    __m256i te=_mm256_add_epi64(pe,_mm256_mul_epu32(me,P));
    __m256i to=_mm256_add_epi64(po,_mm256_mul_epu32(mo,P));
    __m256i r=_mm256_blend_epi32(_mm256_srli_epi64(te,32),to,0xAA);
    return _mm256_min_epu32(r,_mm256_sub_epi32(r,P));
}
AVX static inline __m256i vadd(__m256i a,__m256i b){const __m256i P=_mm256_set1_epi32(MOD);__m256i s=_mm256_add_epi32(a,b);return _mm256_min_epu32(s,_mm256_sub_epi32(s,P));}
AVX static inline __m256i vsub(__m256i a,__m256i b){const __m256i P=_mm256_set1_epi32(MOD);__m256i s=_mm256_add_epi32(_mm256_sub_epi32(a,b),P);return _mm256_min_epu32(s,_mm256_sub_epi32(s,P));}
static bool HAS_AVX2=__builtin_cpu_supports("avx2");
AVX void dif_avx(u32*a,int n){
    for(int len=n>>1;len>=8;len>>=1){
        const u32*w=&rt[len];
        for(int i=0;i<n;i+=2*len){u32*x=a+i,*y=a+i+len;
            for(int j=0;j<len;j+=8){
                __m256i u=_mm256_loadu_si256((__m256i*)(x+j)),v=_mm256_loadu_si256((__m256i*)(y+j)),ww=_mm256_loadu_si256((const __m256i*)(w+j));
                _mm256_storeu_si256((__m256i*)(x+j),vadd(u,v));
                _mm256_storeu_si256((__m256i*)(y+j),vmul(vsub(u,v),ww));
            }}
    }
}
AVX void dit_avx(u32*a,int n){
    for(int len=8;len<n;len<<=1){
        const u32*w=&irt[len];
        for(int i=0;i<n;i+=2*len){u32*x=a+i,*y=a+i+len;
            for(int j=0;j<len;j+=8){
                __m256i u=_mm256_loadu_si256((__m256i*)(x+j)),v=vmul(_mm256_loadu_si256((__m256i*)(y+j)),_mm256_loadu_si256((const __m256i*)(w+j)));
                _mm256_storeu_si256((__m256i*)(x+j),vadd(u,v));
                _mm256_storeu_si256((__m256i*)(y+j),vsub(u,v));
            }}
    }
}
AVX void pmul_avx(u32*a,const u32*b,int n){for(int i=0;i<n;i+=8)_mm256_storeu_si256((__m256i*)(a+i),vmul(_mm256_loadu_si256((__m256i*)(a+i)),_mm256_loadu_si256((const __m256i*)(b+i))));}
void dif_sc(u32*a,int n,int lo){ // stages with len from n/2 down to lo... scalar for len<lo? handles len in [1, top]
    for(int len=lo;len>=1;len>>=1){
        const u32*w=&rt[len];
        for(int i=0;i<n;i+=2*len){u32*x=a+i,*y=a+i+len;
            for(int j=0;j<len;j++){u32 u=x[j],v=y[j];u32 s=u+v;x[j]=s>=MOD?s-MOD:s;y[j]=mmul(u+MOD-v,w[j]);}}
    }
}
void dit_sc(u32*a,int n,int hi){
    for(int len=1;len<n&&len<=hi;len<<=1){
        const u32*w=&irt[len];
        for(int i=0;i<n;i+=2*len){u32*x=a+i,*y=a+i+len;
            for(int j=0;j<len;j++){u32 u=x[j],v=mmul(y[j],w[j]);u32 s=u+v;x[j]=s>=MOD?s-MOD:s;y[j]=u>=v?u-v:u+MOD-v;}}
    }
}
void dif(u32*a,int n){
    if(HAS_AVX2&&n>=16){dif_avx(a,n);dif_sc(a,n,4);}
    else dif_sc(a,n,n>>1);
}
void dit(u32*a,int n){
    if(HAS_AVX2&&n>=16){dit_sc(a,n,4);dit_avx(a,n);}
    else dit_sc(a,n,n);
}
// pointwise a*=b (b already transformed), then inverse; result normal
void finish(vector<u32>&A,const vector<u32>&B){
    int n=A.size();
    if(HAS_AVX2&&n>=8)pmul_avx(A.data(),B.data(),n);else for(int i=0;i<n;i++)A[i]=mmul(A[i],B[i]);
    dit(A.data(),n);u32 c=(u32)(pw(n,MOD-2)*(ull)R2%MOD);
    if(HAS_AVX2&&n>=8){vector<u32> C(8,c);for(int i=0;i<n;i+=8)pmul_avx(A.data()+i,C.data(),8);}
    else for(auto&x:A)x=mmul(x,c);
}
vector<u32> mul(const vector<u32>&a,const vector<u32>&b,int keep){
    int need=min<int>(keep,a.size()+b.size()-1);
    if(min(a.size(),b.size())<=48){
        const vector<u32>&p=a.size()<=b.size()?b:a; const vector<u32>&q=a.size()<=b.size()?a:b;
        vector<ull> r(need,0);
        for(size_t i=0;i<q.size()&&(int)i<need;i++){
            ull qi=q[i]; int lim=min<int>(p.size(),need-i); ull*rr=&r[i];
            for(int j=0;j<lim;j++){rr[j]+=qi*p[j];if(rr[j]>=(ull)8e18)rr[j]%=MOD;}
        }
        vector<u32> o(need);for(int i=0;i<need;i++)o[i]=r[i]%MOD;return o;
    }
    int la=min<int>(a.size(),need), lb=min<int>(b.size(),need);
    int sz=1;while(sz<la+lb-1)sz<<=1; prep(sz);
    vector<u32> A(a.begin(),a.begin()+la),B(b.begin(),b.begin()+lb);A.resize(sz);B.resize(sz);
    dif(A.data(),sz);dif(B.data(),sz);finish(A,B);
    A.resize(need);return A;
}
int n; ll K; vector<ll> a,b; vector<ll> fct,ifc;
map<int,vector<u32>> Ecache;
struct Res{vector<u32> f;int mode;int s,t;}; // mode 1=G,0=H,-1 trivial
vector<u32> convert(const vector<u32>&f,int s,int t){
    int m=min(s,t);
    vector<u32> g(m+1);
    if(f.size()==1){
        for(int x=0;x<=m;x++)g[x]=fct[s]*ifc[x]%MOD*ifc[s-x]%MOD*fct[t]%MOD*ifc[t-x]%MOD;
        return g;
    }
    int L=min<int>(f.size(),m+1);
    vector<u32> P(L);
    for(int j=0;j<L;j++){ll v=(ll)f[j]*fct[s-j]%MOD*fct[t-j]%MOD;if(j&1)v=(MOD-v)%MOD;P[j]=v;}
    vector<u32> c;
    int N=1;while(N<L+m)N<<=1;
    if(L<=48||m<=48){
        vector<u32> E(m+1);for(int i=0;i<=m;i++)E[i]=ifc[i];
        c=mul(P,E,m+1);
    }else if(2*L>N||true){
        // cached E of length N/2 (requires L<=N/2 and m<=N/2-1)
        if(L<=N/2&&m<=N/2-1){
            prep(N);
            auto it=Ecache.find(N);
            if(it==Ecache.end()){vector<u32> E(N,0);for(int i=0;i<N/2&&i<=n;i++)E[i]=ifc[i];dif(E.data(),N);it=Ecache.emplace(N,move(E)).first;}
            P.resize(N);dif(P.data(),N);finish(P,it->second);c=move(P);
        }else{
            vector<u32> E(m+1);for(int i=0;i<=m;i++)E[i]=ifc[i];
            c=mul(P,E,m+1);
        }
    }
    c.resize(m+1);
    for(int x=0;x<=m;x++)g[x]=(ll)c[x]*ifc[s-x]%MOD*ifc[t-x]%MOD;
    return g;
}
// big poly f in mode X with sizes (s1,t1); small poly e in mode X with sizes (s2,t2);
// in mode X, all cross edges present. returns X-poly with sizes (s1+s2,t1+t2)
vector<u32> joinCross(const vector<u32>&f,int s1,int t1,const vector<u32>&e,int s2,int t2){
    int S=s1+s2,T=t1+t2,M=min(S,T);
    vector<ull> out(M+1,0);
    int F=f.size();
    for(int i=0;i<(int)e.size();i++){ if(!e[i])continue;
        for(int c1=0;c1<=t2-i;c1++)for(int c2=0;c2<=s2-i;c2++){
            ll Kc=(ll)e[i]*fct[t2-i]%MOD*ifc[t2-i-c1]%MOD*fct[s2-i]%MOD*ifc[s2-i-c2]%MOD*ifc[c1]%MOD*ifc[c2]%MOD;
            int off=i+c1+c2;
            for(int j=0;j<F&&j+off<=M;j++){
                if(s1-j<c1||t1-j<c2)break;
                ull v=(ull)f[j]*(fct[s1-j]*ifc[s1-j-c1]%MOD)%MOD*(fct[t1-j]*ifc[t1-j-c2]%MOD)%MOD;
                out[j+off]+=v*Kc%MOD;
            }
        }
        for(auto&x:out)x%=MOD;
    }
    vector<u32> r(M+1);for(int i=0;i<=M;i++)r[i]=out[i]%MOD;
    while(r.size()>1&&r.back()==0)r.pop_back();
    return r;
}
static int crossTerms(int s2,int t2){if(s2+t2>8)return 1000;int c=0;for(int i=0;i<=min(s2,t2);i++)c+=(s2-i+1)*(t2-i+1);return c;}
Res solve(int al,int ar,int bl,int br,int d){
    int s=ar-al,t=br-bl;
    if(s==0||t==0)return {{1},-1,s,t};
    if(d<0)return {{1},0,s,t};
    int kb=(K>>d)&1; int pm=kb;
    int am=partition_point(a.begin()+al,a.begin()+ar,[&](ll v){return !((v>>d)&1);})-a.begin();
    int bm=partition_point(b.begin()+bl,b.begin()+br,[&](ll v){return !((v>>d)&1);})-b.begin();
    Res r1,r2;
    if(kb){r1=solve(al,am,bm,br,d-1);r2=solve(am,ar,bl,bm,d-1);}
    else{r1=solve(al,am,bl,bm,d-1);r2=solve(am,ar,bm,br,d-1);}
    auto triv=[&](const Res&r){return r.mode==-1||(r.f.size()==1&&r.mode==pm);};
    if(triv(r2))swap(r1,r2);
    if(triv(r1)){
        if(triv(r2))return {{1},pm,s,t};
        if(r2.s==s&&r2.t==t)return r2;
    }
    // make r1 the smaller (by vertex count)
    if(r1.s+r1.t>r2.s+r2.t)swap(r1,r2);
    if(r2.mode!=pm&&crossTerms(r1.s,r1.t)<=8&&r2.s+r2.t>64){
        int X=r2.mode;
        vector<u32> e=r1.f;
        if(r1.mode!=X&&r1.mode!=-1)e=convert(e,r1.s,r1.t);
        // r1 is in pm mode and disconnected from r2 in pm; in X all cross edges present
        return {joinCross(r2.f,r2.s,r2.t,e,r1.s,r1.t),X,s,t};
    }
    if(r1.mode!=pm&&r1.mode!=-1)r1.f=convert(r1.f,r1.s,r1.t);
    if(r2.mode!=pm&&r2.mode!=-1)r2.f=convert(r2.f,r2.s,r2.t);
    vector<u32> f=mul(r1.f,r2.f,min(s,t)+1);
    while(f.size()>1&&f.back()==0)f.pop_back();
    return {move(f),pm,s,t};
}
static char buf[1<<25];
int main(){
    size_t len=fread(buf,1,sizeof(buf)-1,stdin);buf[len]=0;char*p=buf;
    auto rd=[&]()->ll{while(*p&&(*p<'0'||*p>'9'))p++;ll v=0;while(*p>='0'&&*p<='9')v=v*10+(*p++-'0');return v;};
    n=rd();K=rd();a.resize(n);b.resize(n);
    for(auto&x:a)x=rd();for(auto&x:b)x=rd();
    sort(a.begin(),a.end());sort(b.begin(),b.end());
    fct.assign(n+1,1);ifc.assign(n+1,1);for(int i=1;i<=n;i++)fct[i]=fct[i-1]*i%MOD;
    ifc[n]=pw(fct[n],MOD-2);for(int i=n;i>0;i--)ifc[i-1]=ifc[i]*i%MOD;
    Res r=solve(0,n,0,n,59);
    vector<u32> g;
    if(r.mode==1||r.mode==-1)g=r.f; else g=convert(r.f,n,n);
    g.resize(n+1,0);
    string out;
    for(int x=1;x<=n;x++){out+=to_string(g[x]);out+=(x==n?'\n':' ');}
    fputs(out.c_str(),stdout);
}

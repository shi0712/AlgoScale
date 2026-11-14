#include <bits/stdc++.h>
#if defined(__x86_64__) || defined(__i386__)
#pragma GCC target("popcnt,pclmul,sse4.2")
#include <immintrin.h>
#define HAVE_CLMUL 1
#endif
using namespace std;
typedef unsigned long long u64; typedef unsigned u32;
static char buf[1<<25]; int bl=0,bp=0;
inline int gc(){ if(bp==bl){ bl=fread(buf,1,sizeof buf,stdin); bp=0; if(bl<=0) return -1;} return buf[bp++]; }
inline u64 rd(){ int c=gc(); while(c<'0') c=gc(); u64 x=0; while(c>='0'){ x=x*10+c-'0'; c=gc(); } return x; }

struct H128{u64 lo,hi;};
static inline H128 clmul(u64 a,u64 b){
#ifdef HAVE_CLMUL
    __m128i x=_mm_clmulepi64_si128(_mm_cvtsi64_si128((long long)a),_mm_cvtsi64_si128((long long)b),0);
    return {(u64)_mm_cvtsi128_si64(x),(u64)_mm_extract_epi64(x,1)};
#else
    u64 lo=0,hi=0;
    for(int i=0;i<64;i++) if(b>>i&1){ lo^=a<<i; if(i) hi^=a>>(64-i); }
    return {lo,hi};
#endif
}
static u64 TU[4][256],TL[4][256];
static inline u64 tab(u64 T[4][256],u32 x){ return T[0][x&255]^T[1][(x>>8)&255]^T[2][(x>>16)&255]^T[3][x>>24]; }
vector<u64> A,Bf;
void rsort(size_t m){
    u64*a=A.data(),*b=Bf.data();
    static size_t cnt[65536];
    for(int s=0;s<64;s+=16){
        memset(cnt,0,sizeof cnt);
        for(size_t i=0;i<m;i++) cnt[(a[i]>>s)&65535]++;
        size_t t=0; for(int i=0;i<65536;i++){ size_t c=cnt[i]; cnt[i]=t; t+=c; }
        for(size_t i=0;i<m;i++) b[cnt[(a[i]>>s)&65535]++]=a[i];
        swap(a,b);
    }
}
int main(){
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count()*1000003ULL+12345);
    u64 ub[32],vb[32]; for(int i=0;i<32;i++){ub[i]=rng();vb[i]=rng();}
    for(int t=0;t<4;t++) for(int x=0;x<256;x++){ u64 a=0,b=0; for(int i=0;i<8;i++) if(x>>i&1){a^=ub[t*8+i];b^=vb[t*8+i];} TU[t][x]=a; TL[t][x]=b; }
    const u64 K=0x9E3779B97F4A7C15ULL;
    int T=rd(); string out;
    while(T--){
        int n=rd();
        size_t cap=(size_t)33*n+5; if(A.size()<cap){A.resize(cap);Bf.resize(cap);}
        size_t m=0;
        u32 bv[32]; int bpos[32]; for(int i=0;i<32;i++){bv[i]=0;bpos[i]=0;}
        for(int r=1;r<=n;r++){
            u32 x=(u32)rd(); int p=r;
            for(int b=31;b>=0&&x;b--) if(x>>b&1){
                if(!bv[b]){bv[b]=x;bpos[b]=p;x=0;break;}
                if(bpos[b]<p){swap(bv[b],x);swap(bpos[b],p);}
                x^=bv[b];
            }
            pair<int,u32> it[32]; int k=0;
            for(int b=0;b<32;b++) if(bv[b]) it[k++]={bpos[b],bv[b]};
            sort(it,it+k,[](auto&a,auto&b){return a.first>b.first;});
            if(k==0||it[0].first<r) A[m++]=0x1234567ULL; // span {0}
            u32 col[32],row[32]; for(int b=0;b<32;b++) col[b]=0;
            u32 pm=0; u64 hlo=0,hhi=0;
            for(int j=0;j<k;j++){
                u32 v=it[j].second, s=v&pm, w=v;
                while(s){ w^=row[__builtin_ctz(s)]; s&=s-1; }
                int q=31-__builtin_clz(w);
                u32 R0=col[q], R=R0|(1u<<q);
                while(R0){ row[__builtin_ctz(R0)]^=w; R0&=R0-1; }
                row[q]=w;
                H128 h=clmul(tab(TU,R),tab(TL,w)); hlo^=h.lo; hhi^=h.hi;
                for(int b=0;b<32;b++) col[b]^=R&(0u-((w>>b)&1u));
                pm|=1u<<q;
                A[m++]=hlo^(hhi*K+(u64)(j+1));
            }
        }
        if(m<(1u<<16)) sort(A.begin(),A.begin()+m); else rsort(m);
        long long c=0; for(size_t i=0;i<m;i++) if(i==0||A[i]!=A[i-1]) c++;
        out+=to_string(c); out+='\n';
    }
    fwrite(out.data(),1,out.size(),stdout);
}

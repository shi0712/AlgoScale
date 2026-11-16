#include <bits/stdc++.h>
using namespace std;
typedef uint32_t u32; typedef uint64_t u64;
static char ibuf[1<<16]; static size_t ipos=0, ilen=0;
static inline int gc(){ if(ipos==ilen){ ilen=fread(ibuf,1,sizeof(ibuf),stdin); ipos=0; if(ilen==0) return -1;} return (unsigned char)ibuf[ipos++]; }
static inline u64 readU(){ int c=gc(); while(c!=-1&&(c<'0'||c>'9')) c=gc(); u64 x=0; while(c>='0'&&c<='9'){x=x*10+(u64)(c-'0');c=gc();} return x; }
static inline u64 sm64(u64 x){ x+=0x9e3779b97f4a7c15ULL; x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL; x=(x^(x>>27))*0x94d049bb133111ebULL; return x^(x>>31); }
static u32 cntbuf[65536];
static size_t countDistinct(u64*a,u64*b,size_t n){
    if(n==0) return 0; u64*src=a;
    if(n<=1024) sort(a,a+n);
    else{
        int bits=(n>262144)?16:8; int nb=1<<bits; u64 mask=(u64)(nb-1); u64*dst=b;
        for(int shift=0;shift<64;shift+=bits){
            memset(cntbuf,0,(size_t)nb*sizeof(u32));
            for(size_t i=0;i<n;i++) cntbuf[(src[i]>>shift)&mask]++;
            if(cntbuf[(src[0]>>shift)&mask]==n) continue;
            u32 sum=0; for(int i=0;i<nb;i++){u32 c=cntbuf[i];cntbuf[i]=sum;sum+=c;}
            for(size_t i=0;i<n;i++) dst[cntbuf[(src[i]>>shift)&mask]++]=src[i];
            u64*t=src;src=dst;dst=t;
        }
    }
    size_t res=1; for(size_t i=1;i<n;i++) res+=(src[i]!=src[i-1]); return res;
}
static u32 pool[40][32]; static int LvIdx[40]; static u32 Pm[40]; static u64 Hh1[40],Hh2[40];
int main(){
    mt19937_64 rng((u64)chrono::steady_clock::now().time_since_epoch().count()*0x9e3779b97f4a7c15ULL+12345);
    u64 C[32],D[32]; for(int i=0;i<32;i++){C[i]=rng()|1ULL;D[i]=rng()|1ULL;}
    u64 *keys=nullptr,*tmpb=nullptr; size_t cap=0; string out; out.reserve(1<<20);
    int T=(int)readU();
    while(T-->0){
        int n=(int)readU();
        size_t need=(n<=32)?((size_t)n*(n+1)/2):((size_t)32*n-496);
        if(need>cap){ free(keys);free(tmpb); cap=need; keys=(u64*)malloc(cap*sizeof(u64)); tmpb=(u64*)malloc(cap*sizeof(u64)); if(!keys||!tmpb) return 1; }
        int k=0; bool zeroSeen=false, fullSeen=false; size_t m=0;
        LvIdx[0]=0;Pm[0]=0;Hh1[0]=0;Hh2[0]=0; memset(pool[0],0,128); u64 used=1ULL;
        for(int i=0;i<n;i++){
            u32 a=(u32)readU();
            if(a==0){ zeroSeen=true; continue; }
            u64 fm=~used&((1ULL<<40)-1);
            int curI=__builtin_ctzll(fm); fm&=fm-1; int spareI=__builtin_ctzll(fm);
            u32*cur=pool[curI],*spare=pool[spareI]; memset(cur,0,128);
            u32 curP=0; u64 curH1=0,curH2=0; u32 x=a;
            for(int d=1;;++d){
                if(x==0){ if(d>=2 && d-1!=32) m--; break; }
                bool last=(d>k);
                int nxtI=-1; u32*nxt=nullptr; u32 nxtP=0; u64 nxtH1=0,nxtH2=0;
                if(!last){ nxtI=LvIdx[d]; nxt=pool[nxtI]; nxtP=Pm[d]; nxtH1=Hh1[d]; nxtH2=Hh2[d]; }
                int c=31-__builtin_clz(x);
                memcpy(spare,cur,128);
                u64 h1=curH1+(u64)x*C[c], h2=curH2+(u64)x*D[c];
                u32 mm=curP;
                while(mm){
                    int b2=__builtin_ctz(mm); mm&=mm-1;
                    u32 ov=spare[b2];
                    u32 nv=ov^(x&(0u-((ov>>c)&1u)));
                    spare[b2]=nv;
                    u64 delta=(u64)nv-(u64)ov;
                    h1+=delta*C[b2]; h2+=delta*D[b2];
                }
                spare[c]=x;
                if(!last) used&=~(1ULL<<nxtI);
                used|=1ULL<<spareI;
                LvIdx[d]=spareI; Pm[d]=curP|(1u<<c); Hh1[d]=h1; Hh2[d]=h2;
                if(d==32) fullSeen=true; else keys[m++]=sm64(h1)^sm64(h2);
                if(last){ k=d; break; }
                u32 diff=nxtP&~curP; int b=31-__builtin_clz(diff);
                x^=nxt[b]&(0u-((x>>b)&1u));
                spareI=curI; spare=cur;
                curI=nxtI; cur=nxt; curP=nxtP; curH1=nxtH1; curH2=nxtH2;
            }
        }
        size_t ans=countDistinct(keys,tmpb,m)+(zeroSeen?1:0)+(fullSeen?1:0);
        char tb[24]; int tl=0; if(ans==0) tb[tl++]='0';
        while(ans){ tb[tl++]=(char)('0'+ans%10); ans/=10; }
        while(tl) out.push_back(tb[--tl]);
        out.push_back('\n');
    }
    fwrite(out.data(),1,out.size(),stdout);
    return 0;
}

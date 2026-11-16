#include <bits/stdc++.h>
using namespace std;
typedef uint32_t u32; typedef uint64_t u64;
static const u32 MOD=998244353;
static u32 pw(u32 b,u64 e){u64 r=1,x=b;while(e){if(e&1)r=r*x%MOD;x=x*x%MOD;e>>=1;}return (u32)r;}
static vector<u32> W; static int Wmax=0;
static void ensure_roots(int n){ if(n<=Wmax)return; W.assign((size_t)n,0);
  for(int len=2;len<=n;len<<=1){int half=len>>1;u32 w=pw(3,(MOD-1)/len);W[half]=1;
    for(int j=1;j<half;j++)W[half+j]=(u64)W[half+j-1]*w%MOD;} Wmax=n; }
static void ntt(u32*a,int n){
  for(int i=1,j=0;i<n;i++){int bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j){u32 t=a[i];a[i]=a[j];a[j]=t;}}
  for(int len=2;len<=n;len<<=1){int half=len>>1;const u32*w=W.data()+half;
    for(int i=0;i<n;i+=len){u32*x=a+i,*y=a+i+half;
      for(int j=0;j<half;j++){u32 u=x[j],v=(u32)((u64)y[j]*w[j]%MOD);
        u32 s=u+v;if(s>=MOD)s-=MOD;u32 d=u+MOD-v;if(d>=MOD)d-=MOD;x[j]=s;y[j]=d;}}} }
static void intt(u32*a,int n){ ntt(a,n);
  for(int i=1,j=n-1;i<j;i++,j--){u32 t=a[i];a[i]=a[j];a[j]=t;}
  u32 ni=pw((u32)n,MOD-2); for(int i=0;i<n;i++)a[i]=(u64)a[i]*ni%MOD; }
static vector<u32> poly_inv(const vector<u32>&f,int n){
  vector<u32> h(1,pw(f[0],MOD-2)); int m=1;
  while(m<n){ int L=m<<1; ensure_roots(L);
    vector<u32> fa(L,0),hb(L,0); int fs=min((int)f.size(),L);
    for(int i=0;i<fs;i++)fa[i]=f[i]; for(int i=0;i<m;i++)hb[i]=h[i];
    ntt(fa.data(),L);ntt(hb.data(),L);
    for(int i=0;i<L;i++)fa[i]=(u64)fa[i]*hb[i]%MOD; intt(fa.data(),L);
    vector<u32> ub(L,0); for(int i=0;i<m;i++)ub[i]=fa[m+i];
    ntt(ub.data(),L); for(int i=0;i<L;i++)ub[i]=(u64)ub[i]*hb[i]%MOD; intt(ub.data(),L);
    h.resize(L); for(int i=0;i<m;i++){u32 v=ub[i];h[m+i]=v?MOD-v:0;} m=L; }
  h.resize(n); return h; }
static vector<u32> poly_exp(const vector<u32>&f,int n,const vector<u32>&iv){
  vector<u32> g(1,1); int m=1;
  while(m<n){ int L=m<<1; ensure_roots(L);
    vector<u32> h=poly_inv(g,L);
    vector<u32> d(L,0),hh(L,0);
    for(int i=1;i<m;i++)d[i-1]=(u64)g[i]*(u32)i%MOD;
    for(int i=0;i<L;i++)hh[i]=h[i];
    ntt(d.data(),L);ntt(hh.data(),L);
    for(int i=0;i<L;i++)d[i]=(u64)d[i]*hh[i]%MOD; intt(d.data(),L);
    vector<u32> e(L,0);
    for(int i=0;i<m;i++){int k=m+i;u32 lg=(u64)d[k-1]*iv[k]%MOD;
      u32 fv=(k<(int)f.size())?f[k]:0; e[i]=fv>=lg?fv-lg:fv+MOD-lg;}
    vector<u32> gg(L,0); for(int i=0;i<m;i++)gg[i]=g[i];
    ntt(gg.data(),L);ntt(e.data(),L);
    for(int i=0;i<L;i++)e[i]=(u64)e[i]*gg[i]%MOD; intt(e.data(),L);
    g.resize(L); for(int i=0;i<m;i++)g[m+i]=e[i]; m=L; }
  g.resize(n); return g; }
static char ibuf[1<<16]; static int ilen=0,ipos=0;
static inline int gc(){ if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;} return ibuf[ipos++]; }
static inline long long readInt(){ int c=gc(); while(c!=-1&&(c<'0'||c>'9')&&c!='-')c=gc();
  bool neg=false; if(c=='-'){neg=true;c=gc();} long long x=0;
  while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();} return neg?-x:x; }
int main(){
  int n=(int)readInt();
  vector<int> cnt(n+2,0); long long ms=0; int np=0;
  for(int i=0;i<n;i++){int a=(int)readInt(); ms+=a; if(a>0){cnt[a+1]++;np++;}}
  int M=(int)ms,K=M/2,nn=K+1;
  int N=1; while(N<nn)N<<=1;
  int isz=max(N,nn);
  vector<u32> iv(isz+1); iv[1]=1;
  for(int i=2;i<=isz;i++)iv[i]=(u64)(MOD-MOD/i)*iv[MOD%i]%MOD;
  vector<u32> G(nn,0);
  { vector<u32> gj(nn,(u32)np);
    int tmax=min(n+1,K);
    for(int t=2;t<=tmax;t++)if(cnt[t]){
      u32 v=(u32)((u64)t*cnt[t]%MOD);
      if(v){u32 nv=MOD-v; for(int j=t;j<=K;j+=t){u32 x=gj[j]+nv;if(x>=MOD)x-=MOD;gj[j]=x;}} }
    for(int j=1;j<=K;j++)G[j]=(u64)gj[j]*iv[j]%MOD; }
  vector<u32> P=poly_exp(G,nn,iv);
  vector<u32> fac(M+1),ifac(M+1);
  fac[0]=1; for(int i=1;i<=M;i++)fac[i]=(u64)fac[i-1]*i%MOD;
  ifac[M]=pw(fac[M],MOD-2); for(int i=M;i>=1;i--)ifac[i-1]=(u64)ifac[i]*i%MOD;
  u64 ans=0; u32 pc=0,pp=0;
  for(int k=0;k<=K;k++){
    u32 c=(u64)fac[M]*ifac[k]%MOD; c=(u64)c*ifac[M-k]%MOD;
    u32 d1=c>=pc?c-pc:c+MOD-pc;
    u32 d2=P[k]>=pp?P[k]-pp:P[k]+MOD-pp;
    ans=(ans+(u64)d1*d2)%MOD; pc=c; pp=P[k]; }
  printf("%llu\n",(unsigned long long)ans);
  return 0;
}

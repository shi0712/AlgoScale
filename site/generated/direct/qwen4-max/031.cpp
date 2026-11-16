#include <bits/stdc++.h>
using namespace std;
using u32=unsigned; using u64=unsigned long long;
static const u32 MOD=998244353;
static inline u32 addm(u32 a,u32 b){a+=b;if(a>=MOD)a-=MOD;return a;}
static inline u32 mulm(u32 a,u32 b){return (u32)((u64)a*b%MOD);}
struct Mat{u32 a,b,c,d;}; struct Dual{u32 r,e;};
static inline Mat mmul(const Mat&X,const Mat&Y){Mat Z;
 Z.a=(u32)(((u64)X.a*Y.a+(u64)X.b*Y.c)%MOD); Z.b=(u32)(((u64)X.a*Y.b+(u64)X.b*Y.d)%MOD);
 Z.c=(u32)(((u64)X.c*Y.a+(u64)X.d*Y.c)%MOD); Z.d=(u32)(((u64)X.c*Y.b+(u64)X.d*Y.d)%MOD); return Z;}
static inline Dual dmul(const Dual&X,const Dual&Y){Dual Z;Z.r=mulm(X.r,Y.r);
 Z.e=(u32)(((u64)X.r*Y.e+(u64)X.e*Y.r)%MOD); return Z;}
static char ibuf[1<<16]; static int ipos=0,ilen=0;
static inline int gc(){if(ipos==ilen){ilen=(int)fread(ibuf,1,sizeof(ibuf),stdin);ipos=0;if(ilen<=0)return -1;}return (unsigned char)ibuf[ipos++];}
static inline int readInt(){int c=gc();while(c!=-1&&(c<'0'||c>'9'))c=gc();int x=0;while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}return x;}
int main(){
 int n=readInt(); if(n<=1)return 0; int m=n-1;
 vector<int> eu(m),ev(m),st(n+2,0);
 for(int i=0;i<m;i++){int u=readInt(),v=readInt();eu[i]=u;ev[i]=v;st[u+1]++;st[v+1]++;}
 vector<int> bq(m); vector<char> inb(n+1,0);
 for(int i=0;i<m;i++){bq[i]=readInt();inb[bq[i]]=1;}
 int r=1; while(inb[r])++r;
 for(int v=1;v<=n;v++)st[v+1]+=st[v];
 vector<int> adj(2*m);
 {vector<int> fp(st);for(int i=0;i<m;i++){adj[fp[eu[i]]++]=ev[i];adj[fp[ev[i]]++]=eu[i];}}
 vector<int>().swap(eu); vector<int>().swap(ev);
 vector<int> par(n+1,0),order; order.reserve(n); order.push_back(r);
 for(size_t i=0;i<order.size();i++){int v=order[i];for(int j=st[v];j<st[v+1];j++){int u=adj[j];if(u!=par[v]){par[u]=v;order.push_back(u);}}}
 vector<int> sz(n+1,1),heavy(n+1,0);
 for(int i=n-1;i>=1;i--){int v=order[i],p=par[v];sz[p]+=sz[v];if(!heavy[p]||sz[v]>sz[heavy[p]])heavy[p]=v;}
 vector<int> top(n+1,0),pos(n+1,0),plen(n+1,0),pB(n+1,0),pbase(n+1,0); vector<int> tops;
 for(int v:order){ if(v==r||heavy[par[v]]!=v){int len=0;for(int x=v;x;x=heavy[x]){top[x]=v;pos[x]=len++;}plen[v]=len;tops.push_back(v);} }
 size_t mtot=0;
 for(int t:tops){int B=1;while(B<plen[t])B<<=1;pB[t]=B;pbase[t]=(int)mtot;mtot+=2*(size_t)B;}
 vector<Mat> mats(mtot);
 vector<int> lcnt(n+1,0),lidx(n+1,0),lbase(n+1,0),uc(n+1,0);
 for(int v=1;v<=n;v++){int k=0,ch=0;for(int j=st[v];j<st[v+1];j++){int u=adj[j];if(u==par[v])continue;ch++;if(u!=heavy[v])lidx[u]=k++;}lcnt[v]=k;uc[v]=ch;}
 size_t ltot=0;
 for(int v=1;v<=n;v++)if(lcnt[v]){lbase[v]=(int)ltot;ltot+=2*(size_t)lcnt[v];}
 vector<Dual> duals(ltot?ltot:1);
 for(int v=1;v<=n;v++){int k=lcnt[v];if(!k)continue;int base=lbase[v];
  for(int i=0;i<k;i++)duals[base+k+i]=Dual{1,1};
  for(int i=k-1;i>=1;i--)duals[base+i]=dmul(duals[base+2*i],duals[base+2*i+1]);}
 for(int t:tops){int B=pB[t],base=pbase[t],len=plen[t];int x=t;
  for(int i=0;i<len;i++,x=heavy[x])mats[base+B+i]=Mat{0,0,1,1};
  for(int i=len;i<B;i++)mats[base+B+i]=Mat{1,0,0,1};
  for(int i=B-1;i>=1;i--)mats[base+i]=mmul(mats[base+2*i],mats[base+2*i+1]);}
 vector<u32> pw(n+1); pw[0]=1; for(int i=1;i<=n;i++)pw[i]=addm(pw[i-1],pw[i-1]);
 const u32 inv2=(MOD+1)/2;
 vector<char> proc(n+1,0);
 u32 cpow=1,curPhi=1;
 string out; out.reserve((size_t)m*11); char tmp[16];
 for(int i=0;i<m;i++){
  int u=bq[i];
  cpow=mulm(cpow,pw[uc[u]]);
  int p=par[u];
  if(p){if(proc[p])cpow=mulm(cpow,inv2);uc[p]--;}
  proc[u]=1;
  int cur=u; u32 ans=0;
  while(true){
   int t=top[cur];
   Dual Q=lcnt[cur]?duals[lbase[cur]+1]:Dual{1,0};
   Mat M;
   if(!proc[cur])M=Mat{0,0,Q.r,Q.r};
   else M=Mat{Q.r,Q.r,Q.e,addm(Q.r,Q.e)};
   int B=pB[t],base=pbase[t]; int li=B+pos[cur];
   mats[base+li]=M;
   for(int j=li>>1;j>=1;j>>=1)mats[base+j]=mmul(mats[base+2*j],mats[base+2*j+1]);
   u32 bb=mats[base+1].a,aa=mats[base+1].c;
   int pp=par[t];
   if(!pp){curPhi=addm(bb,aa);ans=mulm(cpow,curPhi);break;}
   Dual psi{addm(bb,aa),aa};
   int k=lcnt[pp],lb=lbase[pp]; int di=k+lidx[t];
   if(duals[lb+di].r==psi.r&&duals[lb+di].e==psi.e){ans=mulm(cpow,curPhi);break;}
   duals[lb+di]=psi;
   for(int j=di>>1;j>=1;j>>=1)duals[lb+j]=dmul(duals[lb+2*j],duals[lb+2*j+1]);
   cur=pp;
  }
  int len=0; if(!ans)tmp[len++]='0';
  while(ans){tmp[len++]=char('0'+ans%10);ans/=10;}
  while(len)out.push_back(tmp[--len]);
  out.push_back('\n');
 }
 fwrite(out.data(),1,out.size(),stdout);
 return 0;
}

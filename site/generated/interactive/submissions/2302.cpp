#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
static int n,k,NW; static u64 unc[1024][16]; static bool alv[1024];
static long long remPairs; static vector<int> prevPos; static mt19937 rng(998244353u);
static inline bool hasUnc(int i){for(int w=0;w<NW;w++) if(unc[i][w]) return true; return false;}
static inline bool getb(int i,int j){return (unc[i][j>>6]>>(j&63))&1ULL;}
static inline void clrb(int i,int j){unc[i][j>>6]&=~(1ULL<<(j&63));}
static int ask(int x){printf("? %d\n",x+1);fflush(stdout);char buf[64];if(scanf("%63s",buf)!=1)exit(0);if(buf[0]=='Y')return 1;if(buf[0]=='N')return 0;exit(0);}
static void executePass(const vector<int>&ord){printf("R\n");fflush(stdout);deque<int>win;vector<int>seq;seq.reserve(ord.size());
 for(int x:ord){ if(!alv[x]||!hasUnc(x))continue; int r=ask(x); seq.push_back(x);
  if(r){int c=0;for(int w=0;w<NW;w++)c+=__builtin_popcountll(unc[x][w]);remPairs-=c;alv[x]=false;for(int w=0;w<NW;w++)unc[x][w]=0;for(int y=0;y<n;y++)clrb(y,x);}
  else{for(int y:win) if(getb(x,y)){clrb(x,y);clrb(y,x);remPairs--;}}
  win.push_back(x); if((int)win.size()>k)win.pop_front(); }
 for(int i=0;i<n;i++)prevPos[i]=n+i; for(int p=0;p<(int)seq.size();p++)prevPos[seq[p]]=p;}
static vector<vector<int>> getComponents(const vector<int>&A){u64 rem[16];for(int w=0;w<16;w++)rem[w]=0;for(int x:A)rem[x>>6]|=1ULL<<(x&63);
 vector<vector<int>>comps;vector<int>stk;
 for(int s:A){ if(!((rem[s>>6]>>(s&63))&1ULL))continue; rem[s>>6]&=~(1ULL<<(s&63)); vector<int>C; stk.clear();stk.push_back(s);
  while(!stk.empty()){int x=stk.back();stk.pop_back();C.push_back(x);
   for(int w=0;w<NW;w++){u64 bits=unc[x][w]&rem[w]; if(!bits)continue; rem[w]&=~bits; while(bits){int j=w*64+__builtin_ctzll(bits);bits&=bits-1;stk.push_back(j);}}}
  comps.push_back(move(C)); } return comps;}
static vector<int> optimizeOrder(const vector<vector<int>>&cnt,int g,int D,long long&covOut){
 auto covOf=[&](const vector<int>&p)->long long{long long s=0;for(int i=0;i<g;i++){int hi=min(g-1,i+D);const vector<int>&row=cnt[p[i]];for(int j=i+1;j<=hi;j++)s+=row[p[j]];}return s;};
 vector<int>best;long long bestCov=-1;int starts=(g<=32?6:3); if(starts>g)starts=g;
 for(int t=0;t<starts;t++){int start=(int)((long long)t*g/starts);vector<char>used(g,0);vector<int>p;p.reserve(g);int cur=start;
  for(int step=0;step<g;step++){used[cur]=1;p.push_back(cur);int lo=max(0,(int)p.size()-D);int nxt=-1;long long bv=-1;
   for(int y=0;y<g;y++)if(!used[y]){long long v=0;const vector<int>&row=cnt[y];for(int q=lo;q<(int)p.size();q++)v+=row[p[q]];if(v>bv){bv=v;nxt=y;}}
   cur=nxt;}
  long long c=covOf(p); if(c>bestCov){bestCov=c;best=p;}}
 {vector<int>p=best;long long cur=bestCov;int iters=40000;
  for(int it=0;it<iters;it++){int i=(int)(rng()%g),j=(int)(rng()%g);if(i==j)continue;if(i>j)swap(i,j);int u=p[i],v=p[j];long long oc=0,nc=0;
   int lo=max(0,i-D),hi=min(g-1,i+D);for(int r=lo;r<=hi;r++){if(r==i||r==j)continue;oc+=cnt[u][p[r]];nc+=cnt[v][p[r]];}
   lo=max(0,j-D);hi=min(g-1,j+D);for(int r=lo;r<=hi;r++){if(r==i||r==j)continue;oc+=cnt[v][p[r]];nc+=cnt[u][p[r]];}
   if(nc>=oc){swap(p[i],p[j]);cur+=nc-oc;if(cur>bestCov){bestCov=cur;best=p;}}}}
 covOut=bestCov;return best;}
static vector<int> buildOrder(const vector<int>&A){int M=A.size();auto comps=getComponents(A);int maxc=0;for(auto&C:comps)maxc=max(maxc,(int)C.size());
 if(maxc<=k+1){vector<int>ord;ord.reserve(M);for(auto&C:comps)for(int x:C)ord.push_back(x);return ord;}
 int b=max(1,k/16);int D=(k+1)/b-1;if(D<1)D=1;long long bestTotal=-1;vector<int>bestOrd;
 for(int mode=0;mode<2;mode++){vector<vector<int>>groups;
  for(auto&C0:comps){vector<int>C=C0;sort(C.begin(),C.end(),[](int x,int y){return prevPos[x]<prevPos[y];});int c=(int)C.size();int ng=(c+b-1)/b;
   if(mode==0){for(int i=0;i<c;i+=b){vector<int>G;for(int j=i;j<min(c,i+b);j++)G.push_back(C[j]);groups.push_back(move(G));}}
   else{vector<vector<int>>gg(ng);for(int i=0;i<c;i++)gg[i%ng].push_back(C[i]);for(auto&G:gg)groups.push_back(move(G));}}
  int g=(int)groups.size();vector<array<u64,16>>mask(g);
  for(int i=0;i<g;i++){mask[i].fill(0ULL);for(int x:groups[i])mask[i][x>>6]|=1ULL<<(x&63);}
  vector<vector<int>>cnt(g,vector<int>(g,0));long long internal=0;
  for(int i=0;i<g;i++){for(int x:groups[i]){int c=0;for(int w=0;w<NW;w++)c+=__builtin_popcountll(unc[x][w]&mask[i][w]);internal+=c;}
   for(int j=i+1;j<g;j++){int c=0;for(int x:groups[i])for(int w=0;w<NW;w++)c+=__builtin_popcountll(unc[x][w]&mask[j][w]);cnt[i][j]=cnt[j][i]=c;}}
  internal/=2;long long cov=0;vector<int>perm;
  if(g<=D+1){perm.resize(g);iota(perm.begin(),perm.end(),0);for(int i=0;i<g;i++)for(int j=i+1;j<g;j++)cov+=cnt[i][j];}
  else perm=optimizeOrder(cnt,g,D,cov);
  long long total=internal+cov;
  if(total>bestTotal){bestTotal=total;bestOrd.clear();for(int idx:perm)for(int x:groups[idx])bestOrd.push_back(x);}}
 return bestOrd;}
int main(){ if(scanf("%d %d",&n,&k)!=2)return 0; NW=(n+63)/64;
 for(int i=0;i<n;i++){alv[i]=true;for(int w=0;w<NW;w++)unc[i][w]=~0ULL;for(int j=n;j<NW*64;j++)clrb(i,j);clrb(i,i);}
 remPairs=(long long)n*(n-1)/2; prevPos.assign(n,0); for(int i=0;i<n;i++)prevPos[i]=i;
 bool first=true; int guard=0;
 while(remPairs>0 && ++guard<100000){
  vector<int>A; for(int i=0;i<n;i++) if(alv[i]&&hasUnc(i)) A.push_back(i);
  if(A.empty())break; vector<int>ord;
  if(first){ord=A;first=false;} else if((int)A.size()<=k+1)ord=A; else ord=buildOrder(A);
  long long before=remPairs; executePass(ord);
  if(remPairs==before){ int x=-1,y=-1;
   for(int i=0;i<n&&x<0;i++){ if(!alv[i])continue; for(int w=0;w<NW;w++) if(unc[i][w]){y=w*64+__builtin_ctzll(unc[i][w]);x=i;break;} }
   if(x<0)break; vector<int>o2; o2.push_back(x); o2.push_back(y); long long b2=remPairs; executePass(o2); if(remPairs==b2)break; } }
 int d=0; for(int i=0;i<n;i++) if(alv[i]) d++;
 printf("! %d\n",d); fflush(stdout); return 0;}

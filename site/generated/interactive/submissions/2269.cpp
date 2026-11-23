#include <bits/stdc++.h>
using namespace std;
static int N,A,B,D,F;static int dst[120][120];static bool kn[120][120];static int mpos[120];
static int ask(int i,int j){if(i==j)return 0;if(kn[i][j])return dst[i][j];printf("? %d %d\n",i,j);fflush(stdout);int v;if(scanf("%d",&v)!=1)exit(0);if(v<=0)exit(0);kn[i][j]=kn[j][i]=true;dst[i][j]=dst[j][i]=v;return v;}
static bool sameBranch(int s,int t,int z){if(s==t)return true;int hs=dst[A][s]-z,ht=dst[A][t]-z;if(hs<2||ht<2)return false;return ask(s,t)<hs+ht;}
static bool bigBranch(const vector<int>&T,int z){int K=(int)T.size();if(K<=F)return false;struct G{int rep,w;vector<int> mem;};vector<G> g;for(int t:T)g.push_back(G{t,1,vector<int>{t}});
 while(g.size()>1){sort(g.begin(),g.end(),[](const G&x,const G&y){return x.w<y.w;});vector<G> ng;bool any=false;size_t i=0;
  while(i<g.size()){size_t j=i;while(j<g.size()&&g[j].w==g[i].w)++j;size_t k=i;
   for(;k+1<j;k+=2){any=true;if(sameBranch(g[k].rep,g[k+1].rep,z)){G gg;gg.rep=g[k].rep;gg.w=g[k].w*2;gg.mem=g[k].mem;gg.mem.insert(gg.mem.end(),g[k+1].mem.begin(),g[k+1].mem.end());ng.push_back(move(gg));}}
   if(k<j)ng.push_back(g[k]);i=j;}
  g=move(ng);if(!any)break;}
 if(g.empty())return false;size_t bi=0;for(size_t i=1;i<g.size();++i)if(g[i].w>g[bi].w)bi=i;int cnt=g[bi].w;if(cnt>F)return true;int rep=g[bi].rep;vector<char> in(N,0);for(int t:g[bi].mem)in[t]=1;int rem=K-cnt;
 for(int t:T){if(in[t])continue;if(cnt+rem<=F)return false;if(sameBranch(rep,t,z)){++cnt;if(cnt>F)return true;}--rem;}
 return cnt>F;}
int main(){if(scanf("%d",&N)!=1)return 0;for(int i=0;i<N;++i){kn[i][i]=true;dst[i][i]=0;}
 for(int t=1;t<N;++t)ask(0,t);A=1;for(int t=2;t<N;++t)if(dst[0][t]>dst[0][A])A=t;
 for(int t=1;t<N;++t)if(t!=A)ask(A,t);B=-1;for(int t=0;t<N;++t)if(t!=A){if(B<0||dst[A][t]>dst[A][B]||(dst[A][t]==dst[A][B]&&t==0))B=t;}
 D=dst[A][B];F=N/2;vector<int> m0(N,-1);for(int t=0;t<N;++t)if(t!=A)m0[t]=(dst[A][t]+dst[A][0]-dst[0][t])/2;
 int x=INT_MAX;for(int t=0;t<N;++t)if(t!=A)x=min(x,m0[t]);
 if(2*x>=D){int C1=0;vector<int> T;for(int t=0;t<N;++t)if(t!=A){if(m0[t]>x)++C1;else T.push_back(t);}bool bal=(C1<=F)&&!bigBranch(T,x);printf("! %d\n",bal?x:-x);fflush(stdout);return 0;}
 int w=(D+dst[A][0]-dst[0][B])/2;vector<int> lo(N,0),hi(N,0);for(int t=0;t<N;++t)mpos[t]=-1;mpos[A]=0;mpos[B]=D;vector<int> amb;
 for(int t=0;t<N;++t){if(t==A||t==B)continue;if(m0[t]<w)mpos[t]=m0[t];else if(m0[t]>w)mpos[t]=w;else{amb.push_back(t);lo[t]=w;hi[t]=min(dst[A][t],D)-1;}}
 int R=INT_MAX;for(int t=0;t<N;++t)if(t!=A&&t!=B&&mpos[t]>=0)R=min(R,max(mpos[t],D-mpos[t]));
 auto bp=[&](int t){int l=lo[t],h=hi[t];if(2*h<D)return D-h;if(2*l>D)return l;return (D+1)/2;};
 sort(amb.begin(),amb.end(),[&](int p,int q){return bp(p)<bp(q);});
 for(int t:amb){if(bp(t)>=R)break;ask(B,t);mpos[t]=(dst[A][t]+D-dst[B][t])/2;R=min(R,max(mpos[t],D-mpos[t]));}
 int z1=D-R,z2=R;vector<int> cls(N,-1);
 for(int t:amb){if(mpos[t]>=0)continue;int p1l=lo[t],p1h=min(hi[t],z1),p2l=max(lo[t],z2),p2h=hi[t];bool h1=(p1l<=p1h),h2=(p2l<=p2h);
  if(!h1&&!h2){ask(B,t);mpos[t]=(dst[A][t]+D-dst[B][t])/2;continue;}
  int pmn=h1?p1l:p2l,pmx=h2?p2h:p1h;
  if(pmn==pmx){mpos[t]=pmn;continue;}if(pmx<z1){cls[t]=0;continue;}if(pmn>z2){cls[t]=1;continue;}
  ask(B,t);mpos[t]=(dst[A][t]+D-dst[B][t])/2;}
 bool bal=false;vector<int> zs{z1};if(z2!=z1)zs.push_back(z2);
 for(int z:zs){bool inS=false;for(int t=0;t<N;++t)if(t!=A&&t!=B&&mpos[t]==z)inS=true;if(!inS)continue;
  int L=0,G=0,K=0;vector<int> T;
  for(int t=0;t<N;++t){if(mpos[t]<0&&cls[t]<0){ask(B,t);mpos[t]=(dst[A][t]+D-dst[B][t])/2;}
   if(mpos[t]>=0){if(mpos[t]<z)++L;else if(mpos[t]>z)++G;else{++K;T.push_back(t);}}else if(cls[t]==0)++L;else ++G;}
  if(L>F||G>F)continue;if(K<=F){bal=true;break;}if(!bigBranch(T,z)){bal=true;break;}}
 printf("! %d\n",bal?R:-R);fflush(stdout);return 0;}

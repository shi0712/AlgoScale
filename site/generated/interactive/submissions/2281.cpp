#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull; typedef long long ll;
static inline ull smix(ull x){x+=0x9e3779b97f4a7c15ULL;x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL;x=(x^(x>>27))*0x94d049bb133111ebULL;return x^(x>>31);}
static int R,C,N=0,S=0; static vector<string> G; static vector<int> cr,cc,OBS,TR[3],CLS; static int nCls=0;
static bool treeOK=false; static vector<int> TPAR,TLVL,TTIN,TTOUT,LEAF; static vector<vector<int>> UPT; static int LOGT=1;
static inline bool isAnc(int u,int v){return TTIN[u]<=TTIN[v]&&TTOUT[v]<=TTOUT[u];}
static int lcaF(int u,int v){if(isAnc(u,v))return u;if(isAnc(v,u))return v;for(int k=LOGT-1;k>=0;k--){int w=UPT[k][u];if(!isAnc(w,v))u=w;}return UPT[0][u];}
static inline int pairDist(int s,int t){if(LEAF[s]==LEAF[t])return 1000000;return TLVL[lcaF(LEAF[s],LEAF[t])]+1;}
static void precompute(){vector<vector<int>> cid(R,vector<int>(C,-1));for(int i=0;i<R;i++)for(int j=0;j<C;j++)if(G[i][j]=='.'){cid[i][j]=N++;cr.push_back(i);cc.push_back(j);}S=4*N;vector<array<int,4>> dst(N);
 for(int i=0;i<R;i++)for(int j=0;j<C;j++)if(cid[i][j]>=0){int id=cid[i][j];dst[id][0]=(i>0&&cid[i-1][j]>=0)?dst[cid[i-1][j]][0]+1:0;}
 for(int i=R-1;i>=0;i--)for(int j=0;j<C;j++)if(cid[i][j]>=0){int id=cid[i][j];dst[id][2]=(i+1<R&&cid[i+1][j]>=0)?dst[cid[i+1][j]][2]+1:0;}
 for(int j=0;j<C;j++)for(int i=0;i<R;i++)if(cid[i][j]>=0){int id=cid[i][j];dst[id][3]=(j>0&&cid[i][j-1]>=0)?dst[cid[i][j-1]][3]+1:0;}
 for(int j=C-1;j>=0;j--)for(int i=0;i<R;i++)if(cid[i][j]>=0){int id=cid[i][j];dst[id][1]=(j+1<C&&cid[i][j+1]>=0)?dst[cid[i][j+1]][1]+1:0;}
 int dr[4]={-1,0,1,0},dc[4]={0,1,0,-1};OBS.assign(S,0);for(int a=0;a<3;a++)TR[a].assign(S,0);
 for(int id=0;id<N;id++)for(int d=0;d<4;d++){int s=id*4+d;OBS[s]=dst[id][d];TR[0][s]=id*4+((d+3)&3);TR[1][s]=id*4+((d+1)&3);if(dst[id][d]>0){TR[2][s]=cid[cr[id]+dr[d]][cc[id]+dc[d]]*4+d;}else TR[2][s]=s;}}
struct HT{int mask;vector<ull> k;vector<int> v,u;int st=0;void init(int n){int sz=1;while(sz<2*n)sz<<=1;mask=sz-1;k.assign(sz,0);v.assign(sz,0);u.assign(sz,0);}inline int ga(ull key,int nid){int h=(int)(smix(key)&(ull)mask);while(true){if(u[h]!=st){u[h]=st;k[h]=key;v[h]=nid;return nid;}if(k[h]==key)return v[h];h=(h+1)&mask;}}};
static void buildLCA(){int n=TPAR.size();vector<vector<int>> ch(n);for(int v=1;v<n;v++)ch[TPAR[v]].push_back(v);TTIN.assign(n,0);TTOUT.assign(n,0);vector<int> it(n,0),stk;int tm=0;stk.push_back(0);TTIN[0]=++tm;while(!stk.empty()){int v=stk.back();if(it[v]<(int)ch[v].size()){int u=ch[v][it[v]++];TTIN[u]=++tm;stk.push_back(u);}else{TTOUT[v]=tm;stk.pop_back();}}LOGT=1;while((1<<LOGT)<n)LOGT++;UPT.assign(LOGT,vector<int>(n));for(int v=0;v<n;v++)UPT[0][v]=TPAR[v]<0?v:TPAR[v];for(int k=1;k<LOGT;k++)for(int v=0;v<n;v++)UPT[k][v]=UPT[k-1][UPT[k-1][v]];}
static void hopcroft(){vector<vector<int>> inv[3];for(int a=0;a<3;a++){inv[a].assign(S,{});for(int s=0;s<S;s++)inv[a][TR[a][s]].push_back(s);}vector<int> bo(S);vector<vector<int>> bl;int mx=0;for(int s=0;s<S;s++)mx=max(mx,OBS[s]);vector<int> id(mx+1,-1);for(int s=0;s<S;s++){int v=OBS[s];if(id[v]<0){id[v]=bl.size();bl.push_back({});}bo[s]=id[v];bl[id[v]].push_back(s);}vector<char> inW(bl.size(),1);deque<int> W;for(int b=0;b<(int)bl.size();b++)W.push_back(b);vector<int> mk(S,0),bm(bl.size(),0);int st=0;vector<int> X,af;while(!W.empty()){int A=W.front();W.pop_front();inW[A]=0;vector<int> As=bl[A];for(int a=0;a<3;a++){++st;X.clear();af.clear();for(int t:As)for(int s:inv[a][t])if(mk[s]!=st){mk[s]=st;X.push_back(s);}if((int)bm.size()<(int)bl.size())bm.resize(bl.size(),0);for(int s:X){int b=bo[s];if(bm[b]!=st){bm[b]=st;af.push_back(b);}}for(int b:af){int sz=bl[b].size(),cx=0;for(int s:bl[b])if(mk[s]==st)cx++;if(cx==0||cx==sz)continue;bool mm=(cx<=sz-cx);int nB=bl.size();bl.push_back(vector<int>());inW.push_back(0);vector<int> kept;kept.reserve(sz-cx);for(int s:bl[b]){bool m=(mk[s]==st);if(m==mm){bl[nB].push_back(s);bo[s]=nB;}else kept.push_back(s);}bl[b].swap(kept);inW[nB]=1;W.push_back(nB);}}}CLS=bo;nCls=bl.size();}
static void buildClasses(){vector<int> cur(S),nxt(S);int mx=0;for(int s=0;s<S;s++)mx=max(mx,OBS[s]);vector<int> id(mx+1,-1);int cnt=0;for(int s=0;s<S;s++){int v=OBS[s];if(id[v]<0)id[v]=cnt++;cur[s]=id[v];}TPAR.clear();TLVL.clear();TPAR.push_back(-1);TLVL.push_back(-1);vector<int> nof(cnt);for(int k=0;k<cnt;k++){nof[k]=TPAR.size();TPAR.push_back(0);TLVL.push_back(0);}HT ht;ht.init(S);ll work=0;const ll CAP=50000000LL;bool conv=false;int lev=0;
 while(true){lev++;work+=S;if(work>CAP)break;ht.st++;int nc=0;vector<int> pn;for(int s=0;s<S;s++){ull key=((ull)cur[s]<<48)|((ull)cur[TR[0][s]]<<32)|((ull)cur[TR[1][s]]<<16)|(ull)cur[TR[2][s]];int g=ht.ga(key,nc);if(g==nc){nc++;pn.push_back(cur[s]);}nxt[s]=g;}if(nc==cnt){conv=true;break;}vector<int> cc2(cnt,0);for(int k=0;k<nc;k++)cc2[pn[k]]++;vector<int> nn(nc);for(int k=0;k<nc;k++){if(cc2[pn[k]]==1)nn[k]=nof[pn[k]];else{nn[k]=TPAR.size();TPAR.push_back(nof[pn[k]]);TLVL.push_back(lev);}}nof.swap(nn);cur.swap(nxt);cnt=nc;}
 if(conv){treeOK=true;CLS=cur;nCls=cnt;LEAF.assign(S,0);for(int s=0;s<S;s++)LEAF[s]=nof[cur[s]];buildLCA();}else{hopcroft();treeOK=false;}}
static const int MAXD=16; static vector<int> buf[MAXD+2]; static int cntb[MAXD+2][100],posb[MAXD+2][100];
static ll workCnt=0,workLimit=3000000,gwork=0; struct ME{int d;double v;int a;}; static unordered_map<ull,ME> memo; static unordered_set<ull> vis;
static vector<int> tst,tfir,touched; static vector<char> tmul; static int tcur=0;
static int term(const int*B,int n){int c0=B[0]>>2;bool uq=true;++tcur;touched.clear();for(int i=0;i<n;i++){int s=B[i],ce=s>>2;if(ce!=c0)uq=false;int k=CLS[s];if(tst[k]!=tcur){tst[k]=tcur;tfir[k]=ce;tmul[k]=0;touched.push_back(k);}else if(tfir[k]!=ce)tmul[k]=1;}if(uq)return 1;for(int k:touched)if(!tmul[k])return 0;return 2;}
static inline ull hb(const int*B,int n){ull h=0;for(int i=0;i<n;i++)h+=smix((ull)B[i]+1);return h;}
static double heur(const int*B,int n){if(!treeOK)return 2.0+log2((double)n+1.0);int mn=-1,mx=-1,ti=INT_MAX,ta=-1;for(int i=0;i<n;i++){int lf=LEAF[B[i]],t=TTIN[lf];if(t<ti){ti=t;mn=lf;}if(t>ta){ta=t;mx=lf;}}if(mn==mx)return 1.0;return (double)TLVL[lcaF(mn,mx)]+2.0;}
static int buildCh(const int*B,int n,int a,int idx,int*st,int*sz){int*cb=cntb[idx];int*pb=posb[idx];memset(cb,0,100*sizeof(int));const vector<int>&T=TR[a];for(int i=0;i<n;i++)cb[OBS[T[B[i]]]]++;int off=0,nb=0;for(int v=0;v<100;v++){pb[v]=off;if(cb[v]){st[nb]=off;sz[nb]=cb[v];nb++;}off+=cb[v];}int tp[100];memcpy(tp,pb,100*sizeof(int));buf[idx].resize(n);for(int i=0;i<n;i++){int s=T[B[i]];buf[idx][tp[OBS[s]]++]=s;}return nb;}
static double rec(const int*B,int n,int dep,int last,int prev,int*oa){*oa=-1;if(term(B,n))return 1.0;workCnt+=2LL*n;gwork+=2LL*n;if(dep<=0||workCnt>workLimit)return heur(B,n);ull h=hb(B,n);auto it=memo.find(h);if(it!=memo.end()&&it->second.d>=dep){*oa=it->second.a;return it->second.v;}workCnt+=6LL*n+40;gwork+=6LL*n+40;int d=OBS[B[0]];double best=1e18;int ba=-1;int st[100],sz[100];
 for(int a=0;a<3;a++){if(a==2&&d==0)continue;if(a==0&&last==1)continue;if(a==1&&last==0)continue;if(a<2&&a==last&&a==prev)continue;int nb=buildCh(B,n,a,dep,st,sz);double v=1.0;int ca;if(nb==1)v+=rec(buf[dep].data(),n,dep-1,a,last,&ca);else for(int i=0;i<nb;i++)v+=((double)sz[i]/n)*rec(buf[dep].data()+st[i],sz[i],dep-1,a,last,&ca);if(v<best){best=v;ba=a;}}
 memo[h]=ME{dep,best,ba};*oa=ba;return best;}
static int chooseAction(const vector<int>&B){int n=B.size(),d=OBS[B[0]];memo.clear();if(gwork>130000000LL)workLimit=100000;else if(gwork>80000000LL)workLimit=500000;else workLimit=3000000;int bestA=-1;
 for(int D=1;D<=MAXD;D++){workCnt=0;double cv=1e18;int ca=-1;bool ab=false;int st[100],sz[100];
  for(int a=0;a<3;a++){if(a==2&&d==0)continue;int nb=buildCh(B.data(),n,a,D,st,sz);double v=1.0;int dum;for(int i=0;i<nb;i++){const int*p=buf[D].data()+st[i];int len=sz[i];double x=rec(p,len,D-1,a,-1,&dum);if(nb==1&&vis.count(hb(p,len)))x+=4.0;v+=((double)len/n)*x;}if(v<cv){cv=v;ca=a;}if(workCnt>workLimit){ab=true;break;}}
  if(!ab||bestA==-1)bestA=ca;if(ab)break;}
 if(bestA<0)bestA=1;return bestA;}
static int forcedAction(const vector<int>&B){int n=B.size(),d=OBS[B[0]];int u=B[0],v=-1;
 if(treeOK){int bd=INT_MAX;for(int i=1;i<n;i++)if(CLS[B[i]]!=CLS[u]){int dd=pairDist(u,B[i]);if(dd<bd){bd=dd;v=B[i];}}if(v>=0){int ba=-1,bv=INT_MAX;for(int a=0;a<3;a++){if(a==2&&d==0)continue;int dd=pairDist(TR[a][u],TR[a][v]);if(dd<bv){bv=dd;ba=a;}}if(ba>=0)return ba;}}
 for(int i=1;i<n;i++)if(CLS[B[i]]!=CLS[u]){v=B[i];break;}if(v<0)return 1;
 unordered_set<ull> seen;deque<array<int,3>> q;seen.insert((ull)u*S+v);
 for(int a=0;a<3;a++){if(a==2&&d==0)continue;int nu=TR[a][u],nv=TR[a][v];if(OBS[nu]!=OBS[nv])return a;seen.insert((ull)nu*S+nv);q.push_back({nu,nv,a});}
 int c=0;while(!q.empty()&&c<200000){auto cu=q.front();q.pop_front();c++;int dd=OBS[cu[0]];for(int a=0;a<3;a++){if(a==2&&dd==0)continue;int nu=TR[a][cu[0]],nv=TR[a][cu[1]];ull key=(ull)nu*S+nv;if(seen.count(key))continue;seen.insert(key);if(OBS[nu]!=OBS[nv])return cu[2];q.push_back({nu,nv,cu[2]});}}
 return 1;}
int main(){ios::sync_with_stdio(true);if(!(cin>>R>>C))return 0;G.resize(R);for(int i=0;i<R;i++)if(!(cin>>G[i]))return 0;precompute();buildClasses();tst.assign(nCls,0);tfir.assign(nCls,0);tmul.assign(nCls,0);
 vector<int> B(S);for(int i=0;i<S;i++)B[i]=i;int lastSize=S+1,since=0;bool forced=false;
 while(true){int d;if(!(cin>>d))return 0;if(d<0)return 0;int m=0;for(int i=0;i<(int)B.size();i++)if(OBS[B[i]]==d)B[m++]=B[i];B.resize(m);if(m==0)return 0;
  int t=term(B.data(),m);
  if(t==1){cout<<"yes "<<cr[B[0]>>2]+1<<" "<<cc[B[0]>>2]+1<<endl;return 0;}
  if(t==2){cout<<"no"<<endl;return 0;}
  vis.insert(hb(B.data(),m));
  if(m<lastSize){lastSize=m;since=0;forced=false;}else since++;
  int md=treeOK?(int)(heur(B.data(),m)-1.0):20;
  if(since>md+15)forced=true;
  int a=forced?forcedAction(B):chooseAction(B);
  if(a==0)cout<<"left"<<endl;else if(a==1)cout<<"right"<<endl;else cout<<"step"<<endl;
  for(int i=0;i<m;i++)B[i]=TR[a][B[i]];}
 return 0;}

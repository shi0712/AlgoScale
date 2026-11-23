#include <bits/stdc++.h>
using namespace std;
static const int MAXN=1024;
static int n,k,gsize,rc;
struct Item{int type,id;};
static deque<Item> mem;
static vector<bitset<MAXN>> poss;
static vector<char> resolvedC,ambigG,usedG;
static vector<int> cntG;
static void resetMem(){fputs("R\n",stdout);fflush(stdout);mem.clear();}
static char rawQuery(int idx){printf("? %d\n",idx);fflush(stdout);char b[64];if(scanf("%63s",b)!=1)exit(0);if(b[0]!='Y'&&b[0]!='N')exit(0);return b[0];}
static inline void setPoss(int c,int g,bool v){if(poss[c].test(g)!=v){poss[c].set(g,v);cntG[g]+=v?1:-1;}}
static char queryItem(int idx,int type,int id){char a=rawQuery(idx);
 if(type==0){ if(a=='Y'){resolvedC[id]=1;bitset<MAXN> w;for(const auto&it:mem)if(it.type==1)w.set(it.id);
   for(int g=(int)poss[id]._Find_first();g<MAXN;g=(int)poss[id]._Find_next(g))if(!w.test(g))setPoss(id,g,false);}
  else{for(const auto&it:mem)if(it.type==1)setPoss(id,it.id,false);} }
 else{ if(a=='Y')ambigG[id]=1; else{for(const auto&it:mem)if(it.type==0)setPoss(it.id,id,false);} }
 mem.push_back(Item{type,id}); if((int)mem.size()>k)mem.pop_front(); return a;}
static void propagate(){bool ch=true;while(ch){ch=false;
 for(int c=0;c<rc;c++)if(resolvedC[c]&&poss[c].count()==1){int g=(int)poss[c]._Find_first();
  if(g<gsize&&!usedG[g]){usedG[g]=1;for(int c2=0;c2<rc;c2++)if(c2!=c&&poss[c2].test(g)){setPoss(c2,g,false);ch=true;}}}
 for(int g=0;g<gsize;g++)if(ambigG[g]&&!usedG[g]&&cntG[g]==1){int c=-1;
  for(int c2=0;c2<rc;c2++)if(poss[c2].test(g)){c=c2;break;}
  if(c>=0){usedG[g]=1;if(!resolvedC[c]){resolvedC[c]=1;ch=true;}
   for(int g2=(int)poss[c]._Find_first();g2<MAXN;g2=(int)poss[c]._Find_next(g2))if(g2!=g){setPoss(c,g2,false);ch=true;}}}}}
struct DSU{vector<int> p;DSU(int n):p(n+1){iota(p.begin(),p.end(),0);}int f(int x){while(p[x]!=x){p[x]=p[p[x]];x=p[x];}return x;}void u(int a,int b){a=f(a);b=f(b);if(a!=b)p[a]=b;}};
int main(){ if(scanf("%d %d",&n,&k)!=2)return 0;
 if(k==1){ if(n==1){resetMem();rawQuery(1);printf("! 1\n");fflush(stdout);return 0;}
  vector<vector<pair<int,int>>> adj(n+1);int ec=0;
  for(int i=1;i<=n;i++)for(int j=i+1;j<=n;j++){adj[i].push_back({j,ec});adj[j].push_back({i,ec});ec++;}
  for(int i=1;i+1<=n-2;i+=2){adj[i].push_back({i+1,ec});adj[i+1].push_back({i,ec});ec++;}
  vector<char> us(ec,0);vector<int> ptr(n+1,0),stk,path;stk.push_back(n-1);
  while(!stk.empty()){int v=stk.back();while(ptr[v]<(int)adj[v].size()&&us[adj[v][ptr[v]].second])ptr[v]++;
   if(ptr[v]==(int)adj[v].size()){path.push_back(v);stk.pop_back();}
   else{int to=adj[v][ptr[v]].first,id=adj[v][ptr[v]].second;us[id]=1;ptr[v]++;stk.push_back(to);}}
  reverse(path.begin(),path.end());resetMem();DSU d(n);int prev=-1;
  for(int v:path){char a=rawQuery(v);if(a=='Y'&&prev>0)d.u(prev,v);prev=v;}
  int cnt=0;for(int i=1;i<=n;i++)if(d.f(i)==i)cnt++;printf("! %d\n",cnt);fflush(stdout);return 0;}
 vector<int> G;
 for(int start=1;start<=n;start+=k){ resetMem(); vector<int> C;
  for(int i=0;i<k;i++)if(rawQuery(start+i)=='N')C.push_back(start+i);
  rc=(int)C.size(); if(G.empty()){for(int x:C)G.push_back(x);continue;}
  gsize=(int)G.size(); bitset<MAXN> full; for(int g=0;g<gsize;g++)full.set(g);
  poss.assign(rc,full);cntG.assign(gsize,rc);resolvedC.assign(rc,0);ambigG.assign(gsize,0);usedG.assign(gsize,0);
  bool memHasC; if(rc==k){mem.clear();for(int c=0;c<rc;c++)mem.push_back(Item{0,c});memHasC=true;}else{resetMem();memHasC=false;}
  vector<pair<int,int>> ch;int rem=gsize%k,p=0;
  if(rem){ch.push_back({0,rem});p=rem;} while(p<gsize){ch.push_back({p,k});p+=k;}
  vector<int> act(rc);iota(act.begin(),act.end(),0);
  for(size_t ci=0;ci<ch.size();ci++){ if(act.empty())break; int cs=ch[ci].first,sz=ch[ci].second,rr=(int)act.size();
   if(sz+rr<=k+1){ resetMem(); for(int j=0;j<sz;j++)queryItem(G[cs+j],1,cs+j);
    for(int c:act){if(resolvedC[c]||!poss[c].any())continue;if(queryItem(C[c],0,c)=='Y')propagate();} memHasC=true; }
   else{ if(!memHasC){resetMem();for(int c:act){rawQuery(C[c]);mem.push_back(Item{0,c});}memHasC=true;}
    for(int j=0;j<sz;j++)queryItem(G[cs+j],1,cs+j);
    for(int c:act){if(resolvedC[c]||!poss[c].any())continue;if(queryItem(C[c],0,c)=='Y')propagate();} }
   propagate(); vector<int> na;for(int c:act)if(!resolvedC[c]&&poss[c].any())na.push_back(c);act.swap(na);}
  propagate(); vector<int> U;for(int c=0;c<rc;c++)if(!resolvedC[c]&&poss[c].any())U.push_back(c);
  if(!U.empty()){ bitset<MAXN> Ab;for(int c:U)Ab|=poss[c]; vector<int> A;
   for(int g=0;g<gsize;g++)if(Ab.test(g))A.push_back(g);
   int a=(int)A.size(),u=(int)U.size();
   resetMem(); for(int g:A)queryItem(G[g],1,g);
   for(int c:U){if(resolvedC[c]||!poss[c].any())continue;if(queryItem(C[c],0,c)=='Y')propagate();}
   propagate();
   if(a+u>k+1){ vector<int> U2;for(int c:U)if(!resolvedC[c]&&poss[c].any())U2.push_back(c);
    if(!U2.empty()){ resetMem(); for(int i=a-1;i>=0;i--)queryItem(G[A[i]],1,A[i]);
     for(int i=(int)U2.size()-1;i>=0;i--){int c=U2[i];if(resolvedC[c]||!poss[c].any())continue;if(queryItem(C[c],0,c)=='Y')propagate();}
     propagate(); } }
   for(int c=0;c<rc;c++){ while(!resolvedC[c]&&poss[c].any()){ vector<int> gs;
     for(int g=(int)poss[c]._Find_first();g<MAXN&&(int)gs.size()<k;g=(int)poss[c]._Find_next(g))gs.push_back(g);
     if(gs.empty())break; resetMem(); for(int g:gs)queryItem(G[g],1,g); queryItem(C[c],0,c); propagate(); } } }
  for(int c=0;c<rc;c++)if(!resolvedC[c])G.push_back(C[c]); }
 printf("! %d\n",(int)G.size());fflush(stdout);return 0;}

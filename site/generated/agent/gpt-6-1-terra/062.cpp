#include <bits/stdc++.h>
using namespace std;
struct DSU {
 vector<int> p,sz;
 DSU(int n):p(n),sz(n,1){iota(p.begin(),p.end(),0);}
 int find(int x){while(x!=p[x]){p[x]=p[p[x]];x=p[x];}return x;}
 bool unite(int a,int b){a=find(a);b=find(b);if(a==b)return false;if(sz[a]<sz[b])swap(a,b);p[b]=a;sz[a]+=sz[b];return true;}
};
struct Edge {int a,b,c,d;};
struct Tree {
 vector<vector<pair<int,int>>> g;
 vector<int> par,dep,pe;
 Tree(int n):g(n),par(n,-1),dep(n),pe(n,-1){}
 void add(int u,int v,int id){g[u].push_back({v,id});g[v].push_back({u,id});}
 void build(){vector<int> q={0};par[0]=0;for(size_t i=0;i<q.size();++i){int u=q[i];for(auto [v,id]:g[u])if(par[v]<0){par[v]=u;dep[v]=dep[u]+1;pe[v]=id;q.push_back(v);}}}
 template<class F> void path(int u,int v,F fn) const {
  while(dep[u]>dep[v]){fn(pe[u]);u=par[u];}
  while(dep[v]>dep[u]){fn(pe[v]);v=par[v];}
  while(u!=v){fn(pe[u]);fn(pe[v]);u=par[u];v=par[v];}
 }
};
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int t;cin>>t;while(t--){
  int n,m;cin>>n>>m;vector<Edge> e(m);for(auto &x:e){cin>>x.a>>x.b>>x.c>>x.d;--x.a;--x.b;--x.c;--x.d;}
  DSU da(n),db(n);vector<char> in(m);Tree A(n),B(n);bool bad=false;
  for(int i=0;i<m;i++)if(da.unite(e[i].a,e[i].b)){
   in[i]=1;A.add(e[i].a,e[i].b,i);B.add(e[i].c,e[i].d,i);
   if(!db.unite(e[i].c,e[i].d))bad=true;
  }
  if(bad){cout<<"YES\n";continue;}
  A.build();B.build();DSU comp(m);
  for(int i=0;i<m;i++)if(!in[i]) A.path(e[i].a,e[i].b,[&](int j){comp.unite(i,j);});
  vector<int> head(m,-1),indeg(m),mark(m,0),to,nxt;int stamp=0;
  for(int i=0;i<m&&!bad;i++)if(!in[i]){
   ++stamp;int count=0,match=0;
   A.path(e[i].a,e[i].b,[&](int j){mark[j]=stamp;++count;});
   int from=comp.find(i);
   B.path(e[i].c,e[i].d,[&](int j){
    if(mark[j]==stamp){++match;return;}
    int v=comp.find(j);if(v==from){bad=true;return;}
    to.push_back(v);nxt.push_back(head[from]);head[from]=(int)to.size()-1;++indeg[v];
   });
   if(match!=count)bad=true;
  }
  if(!bad){
   vector<int> q;int cnt=0;for(int i=0;i<m;i++)if(comp.find(i)==i){++cnt;if(!indeg[i])q.push_back(i);}
   for(size_t k=0;k<q.size();++k)for(int j=head[q[k]];j!=-1;j=nxt[j])if(--indeg[to[j]]==0)q.push_back(to[j]);
   if((int)q.size()!=cnt)bad=true;
  }
  cout<<(bad?"YES\n":"NO\n");
 }
}

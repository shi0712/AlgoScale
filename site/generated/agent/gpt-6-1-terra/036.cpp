#include <bits/stdc++.h>
using namespace std;
using ll = long long;
struct Edge { int u,v,w; unsigned short town; };
struct DSU {
 vector<int> p, sz;
 DSU(int n):p(n),sz(n,1){iota(p.begin(),p.end(),0);}
 int find(int a){while(p[a]!=a){p[a]=p[p[a]];a=p[a];}return a;}
 bool unite(int a,int b){a=find(a);b=find(b);if(a==b)return false;if(sz[a]<sz[b])swap(a,b);p[b]=a;sz[a]+=sz[b];return true;}
};
int n,m,k; vector<Edge> edges; ll cost[16], mn[16], gain[16]; ll best, original; int allmask;
struct Res {ll weight; ll lower; int deg[16]; ll leafcost[16];};
Res calc(int available,int required,ll paid){
 DSU d(n+k); Res r{}; int need=n+__builtin_popcount((unsigned)available)-1;
 for(const auto &e:edges){
  if(e.town && !(available & (1<<(e.town-1)))) continue;
  if(d.unite(e.u,e.v)){
   r.weight+=e.w;
   if(e.town){int j=e.town-1;r.deg[j]++;r.leafcost[j]=e.w;}
   if(!--need)break;
  }
 }
 ll lower=r.weight+paid;
 for(int j=0;j<k;j++)if((available & (1<<j)) && !(required & (1<<j))) lower-=mn[j];
 r.lower=lower;
 ll candidate=r.weight;
 for(int j=0;j<k;j++)if(available&(1<<j)){
   if(r.deg[j]==1)candidate-=r.leafcost[j];
   else if(r.deg[j]>=2)candidate+=cost[j];
 }
 best=min(best,candidate);
 return r;
}
void dfs(int available,int required,ll paid){
 if(paid>=best)return;
 Res r=calc(available,required,paid);
 ll additive=original+paid;
 for(int j=0;j<k;j++)if(available&(1<<j)){
  additive-=gain[j];
  if(!(required&(1<<j)))additive+=min(cost[j],gain[j]);
 }
 if(max(r.lower,additive)>=best)return;
 int undecided=available & ~required;
 if(!undecided)return;
 int choose=-1; ll score=-1;
 for(int j=0;j<k;j++)if(undecided&(1<<j)){
   ll s=cost[j]+(r.deg[j]>=2 ? (1LL<<40) : 0);
   if(s>score)score=s,choose=j;
 }
 int bit=1<<choose;
 // A town that appears in the current relaxed tree is usually promising.
 if(r.deg[choose]>=2){dfs(available,required|bit,paid+cost[choose]);dfs(available^bit,required,paid);}
 else {dfs(available^bit,required,paid);dfs(available,required|bit,paid+cost[choose]);}
}
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 if(!(cin>>n>>m>>k))return 0;
 vector<Edge> old;old.reserve(m);
 for(int i=0,u,v,w;i<m;i++){cin>>u>>v>>w;old.push_back({u-1,v-1,w,0});}
 sort(old.begin(),old.end(),[](const Edge&a,const Edge&b){return a.w<b.w;});
 DSU d(n);best=0;edges.reserve(n-1+(size_t)n*k);
 for(auto &e:old)if(d.unite(e.u,e.v)){edges.push_back(e);best+=e.w;}
 vector<Edge>().swap(old);
 // Free old roads may always be used, so contract their components.
 int original_n=n;
 DSU zero(n);
 for(auto &e:edges)if(e.w==0)zero.unite(e.u,e.v);
 vector<int> id(n,-1), component(n);
 int count=0;
 for(int i=0;i<n;i++){
  int r=zero.find(i);
  if(id[r]<0)id[r]=count++;
  component[i]=id[r];
 }
 vector<Edge> reduced; reduced.reserve(edges.size());
 for(auto e:edges)if(e.w){e.u=component[e.u];e.v=component[e.v];reduced.push_back(e);}
 n=count; edges.swap(reduced);
 edges.reserve(n-1+(size_t)n*k);
 for(int j=0;j<k;j++){
  cin>>cost[j];mn[j]=LLONG_MAX;
  vector<int> lowest(n,INT_MAX);
  for(int i=0,w;i<original_n;i++){cin>>w;lowest[component[i]]=min(lowest[component[i]],w);}
  for(int i=0;i<n;i++){int w=lowest[i];mn[j]=min(mn[j],(ll)w);edges.push_back({i,n+j,w,(unsigned short)(j+1)});}
 }
 if(n==1||best==0){cout<<0<<'\n';return 0;}
 sort(edges.begin(),edges.end(),[](const Edge&a,const Edge&b){return a.w<b.w;});
 original=best;
 allmask=(1<<k)-1;
 for(int j=0;j<k;j++){
  Res r=calc(1<<j,0,0);
  gain[j]=original-(r.weight-mn[j]);
  if(gain[j]<=cost[j]) allmask^=1<<j;
 }
 dfs(allmask,0,0);
 cout<<best<<'\n';
}

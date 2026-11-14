#include <bits/stdc++.h>
using namespace std;
using ld=long double;
struct Edge{int u,v,w;};
struct Line{ld mu,q; ld val(ld x)const{return q-2*mu*x;} };
int n,m; vector<Edge> es; vector<vector<int>> adj,radj; vector<int> valid; ld best=1e100L;
struct State{ld cost, sum, sq;};
const ld INF=1e80L;
Line oracle(ld t){
 vector<ld> cost(es.size()); for(int i=0;i<(int)es.size();i++) {ld z=es[i].w-t; cost[i]=z*z;}
 vector<vector<ld>> d(n+1,vector<ld>(n,INF)); for(int i=0;i<n;i++) if(valid[i]) d[0][i]=0;
 vector<State> p(n,{INF,0,0}),np(n); p[0]={0,0,0};
 Line ans{0,INF}; ld av=INF;
 for(int k=1;k<=n;k++){
  fill(np.begin(),np.end(),State{INF,0,0});
  for(int i=0;i<(int)es.size();i++){auto e=es[i]; ld c=cost[i]; if(d[k-1][e.u]+c<d[k][e.v]) d[k][e.v]=d[k-1][e.u]+c;
   if(p[e.u].cost+c<np[e.v].cost){np[e.v]={p[e.u].cost+c,p[e.u].sum+e.w,p[e.u].sq+e.w*e.w};}}
  p.swap(np); if(k<n && p[n-1].cost<INF/2 && p[n-1].cost/k<av){av=p[n-1].cost/k; ans={p[n-1].sum/k,p[n-1].sq/k};}
 }
 ld lam=INF;
 for(int v=0;v<n;v++) if(d[n][v]<INF/2){ld mx=-INF; for(int k=0;k<n;k++) if(d[k][v]<INF/2) mx=max(mx,(d[n][v]-d[k][v])/(n-k)); lam=min(lam,mx);}
 if(lam<av+1e-13L && lam<INF/2){
  vector<ld> h(n,INF); for(int v=0;v<n;v++) if(valid[v]) for(int k=0;k<n;k++) if(d[k][v]<INF/2) h[v]=min(h[v],d[k][v]-k*lam);
  vector<vector<int>> tight(n); for(int i=0;i<(int)es.size();i++){auto e=es[i]; if(h[e.u]<INF/2 && h[e.v]<INF/2 && fabsl(h[e.u]+cost[i]-lam-h[e.v])<1e-10L) tight[e.u].push_back(i);}
  vector<int> col(n),par(n,-1); bool found=false; Line cyc{};
  function<void(int)> dfs=[&](int u){col[u]=1; for(int id:tight[u]){if(found) break; int v=es[id].v; if(!col[v]){par[v]=id;dfs(v);}else if(col[v]==1){ld s=es[id].w,q=es[id].w*es[id].w;int len=1; for(int x=u;x!=v;){int j=par[x]; if(j<0) break; s+=es[j].w;q+=es[j].w*es[j].w;len++;x=es[j].u;}cyc={s/len,q/len};found=true;}}col[u]=2;};
  for(int v=0;v<n&&!found;v++)if(valid[v]&&!col[v])dfs(v);
  if(found && cyc.val(t)+t*t < av+1e-9L){ans=cyc;av=cyc.val(t)+t*t;}
 }
 if(ans.q<INF/2) best=min(best,max((ld)0,ans.q-ans.mu*ans.mu));
 return ans;
}
void solve(ld l,Line a,ld r,Line b,int depth){
 if(depth>100 || r-l<1e-14L || best<1e-18L)return;
 // The secant of the concave lower envelope is a lower bound.
 ld ga=a.val(l),gb=b.val(r), slope=(gb-ga)/(r-l);
 ld x=max(l,min(r,-slope/2)); ld lower=x*x+ga+slope*(x-l);
 if(lower>=best-1e-13L)return;
 ld den=2*(a.mu-b.mu); if(fabsl(den)<1e-22L)return;
 ld mid=(a.q-b.q)/den;
 if(mid<=l+1e-15L || mid>=r-1e-15L)return;
 Line c=oracle(mid);
 if(c.val(mid)>=a.val(mid)-1e-12L)return;
 solve(l,a,mid,c,depth+1); solve(mid,c,r,b,depth+1);
}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);cin>>n>>m;vector<Edge> all;adj.resize(n);radj.resize(n); for(int i=0;i<m;i++){int u,v,w;cin>>u>>v>>w;--u;--v;all.push_back({u,v,w});adj[u].push_back(v);radj[v].push_back(u);} vector<int> a(n),b(n);auto dfs=[&](auto&&self,int u,vector<int>&vis,vector<vector<int>>&g)->void{vis[u]=1;for(int v:g[u])if(!vis[v])self(self,v,vis,g);};dfs(dfs,0,a,adj);dfs(dfs,n-1,b,radj);if(!a[n-1]){cout<<"-1\n";return 0;} valid.resize(n);for(int i=0;i<n;i++)valid[i]=a[i]&&b[i];for(auto e:all)if(valid[e.u]&&valid[e.v])es.push_back(e);
 Line l=oracle(0),r=oracle(20); solve(0,l,20,r,0);cout<<fixed<<setprecision(15)<<(double)best<<'\n';}

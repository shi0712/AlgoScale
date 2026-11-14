#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const ll INF = (1LL<<62), BONUS = 1000000000000LL;
struct Choice { int to, value; };
struct Edge { int to, rev, cap; ll cost; };
struct Solver {
 int n,m,S,T,own,none,V;
 vector<vector<Choice>> choices;
 vector<vector<Edge>> g;
 vector<vector<pair<int,int>>> refs;
 void add(int u,int v,int cap,ll cost){ Edge a{v,(int)g[v].size(),cap,cost},b{u,(int)g[u].size(),0,-cost};g[u].push_back(a);g[v].push_back(b); }
 pair<ll,vector<int>> solve(int k,bool get=false){
  S=0; own=1+n+m; none=own+1; T=none+1; V=T+1;
  g.assign(V,{}); refs.assign(n,{});
  for(int i=0;i<n;i++){
   int u=1+i; add(S,u,1,0);
   for(auto c:choices[i]){int v=c.to==-1?none:(c.to==m?own:1+n+c.to); refs[i].push_back({(int)g[u].size(),c.value});add(u,v,1,c.value);}
  }
  for(int j=0;j<m;j++) add(1+n+j,T,k,BONUS);
  add(none,T,n,BONUS);add(own,T,k,0);
  vector<ll> pot(V),dist(V); vector<int> pv(V),pe(V); ll total=0;
  for(int flow=0;flow<n;flow++){
   fill(dist.begin(),dist.end(),INF);dist[S]=0;
   priority_queue<pair<ll,int>,vector<pair<ll,int>>,greater<pair<ll,int>>> pq; pq.push({0,S});
   while(!pq.empty()){
    auto [d,u]=pq.top();pq.pop();if(d!=dist[u])continue;
    for(int e=0;e<(int)g[u].size();e++){auto &x=g[u][e];if(!x.cap)continue; ll nd=d+x.cost+pot[u]-pot[x.to];if(nd<dist[x.to]){dist[x.to]=nd;pv[x.to]=u;pe[x.to]=e;pq.push({nd,x.to});}}
   }
   if(dist[T]==INF)return {INF,{}};
   for(int v=0;v<V;v++)if(dist[v]<INF)pot[v]+=dist[v];
   for(int v=T;v!=S;v=pv[v]){auto &e=g[pv[v]][pe[v]];total+=e.cost;e.cap--;g[v][e.rev].cap++;}
  }
  int used=0;for(auto &e:g[own])if(e.to==T)used=k-e.cap;
  if(used!=k)return {INF,{}};
  vector<int> ans;
  if(get){ans.resize(n);for(int i=0;i<n;i++)for(auto [idx,val]:refs[i])if(g[1+i][idx].cap==0){ans[i]=val;break;}}
  return {total-BONUS*(n-k),ans};
 }
};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,m;if(!(cin>>n>>m))return 0;Solver s;s.n=n;s.m=m;s.choices.resize(n);
 for(int i=0;i<n;i++){
  vector<pair<int,int>> a(m);for(int j=0;j<m;j++){cin>>a[j].first;a[j].second=j;}sort(a.begin(),a.end());
  vector<pair<int,int>> unique;
  for(int l=0;l<m;){int r=l+1;while(r<m&&a[r].first==a[l].first)r++;if(r==l+1&&a[l].first>0)unique.push_back(a[l]);l=r;}
  int top=unique.empty()?0:unique.back().first;
  if(unique.empty())s.choices[i].push_back({-1,0});
  else {s.choices[i].push_back({unique.back().second,0});s.choices[i].push_back({unique.size()>1?unique[unique.size()-2].second:-1,top});}
  int win=top+1;for(auto [v,j]:a)if(v==win)win++;else if(v>win)break;
  s.choices[i].push_back({m,win});
 }
 vector<ll> cache(n+1,-1);auto cost=[&](int k){if(cache[k]==-1)cache[k]=s.solve(k).first;return cache[k];};
 int l=0,r=n;while(l<r){int mid=(l+r)/2;ll a=cost(mid),b=cost(mid+1);if(a==INF)l=mid+1;else if(a<=b)r=mid;else l=mid+1;}
 auto result=s.solve(l,true);for(int i=0;i<n;i++)cout<<result.second[i]<<(i+1==n?'\n':' ');
}

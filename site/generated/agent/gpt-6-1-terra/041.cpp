#include <bits/stdc++.h>
using namespace std;
struct Op {array<int,4> v; array<pair<int,int>,3> e;};
Op op(int x,int y,int z,int w, pair<int,int>a,pair<int,int>b,pair<int,int>c){return {{x,y,z,w},{a,b,c}};}
struct Builder {
 int n,r; bool g[105][105]{}; vector<Op> ops; vector<vector<Op>> undo;
 Builder(int N,int R, vector<pair<int,int>> es):n(N),r(R){for(auto [u,v]:es)g[u][v]=g[v][u]=1;}
 void apply(Op o){auto [x,y,z,w]=o.v; g[x][y]=g[y][x]=g[y][z]=g[z][y]=g[z][w]=g[w][z]=0; for(auto [a,b]:o.e)g[a][b]=g[b][a]=1; ops.push_back(o);}
 void step(Op o,vector<Op> rev){apply(o);undo.push_back(rev);}
 vector<int> parents(){vector<int> p(n+1,-1);p[r]=0;queue<int>q;q.push(r);while(q.size()){int u=q.front();q.pop();for(int v=1;v<=n;v++)if(g[u][v]&&p[v]<0)p[v]=u,q.push(v);}return p;}
 void build(){
  auto par=parents(); int q=-1; for(int i=1;i<=n;i++){int deg=0;for(int j=1;j<=n;j++)deg+=g[i][j];if(deg==1&&par[i]&&par[i]!=r){q=i;break;}}
  while(true){par=parents();vector<int> path;for(int v=q;v;v=par[v])path.push_back(v);reverse(path.begin(),path.end());if(path.size()<=3)break;int a=path[1],b=path[2],c=path[3];
   step(op(r,a,b,c,{r,a},{r,b},{b,c}),{op(a,r,b,c,{r,a},{a,b},{b,c})});
  }
  par=parents();int p=par[q];
  while(true){par=parents();int c=0;for(int i=1;i<=n;i++)if(par[i]>0 && par[par[i]]>0 && par[par[par[i]]]==r){c=i;break;} if(!c)break;
   int b=par[c],a=par[b];vector<Op> rev;
   if(a==p){rev={op(b,r,p,q,{r,b},{p,b},{p,q}),op(c,r,b,p,{r,p},{p,b},{b,c})};}
   else {rev={op(a,r,p,q,{r,a},{a,p},{p,q}),op(b,r,a,p,{r,b},{r,p},{a,b}),op(c,r,b,a,{r,a},{a,b},{b,c})};}
   step(op(r,a,b,c,{r,a},{r,b},{r,c}),rev);
  }
  while(true){par=parents();int b=0;for(int i=1;i<=n;i++)if(i!=q&&par[i]>0&&par[i]!=r){b=i;break;}if(!b)break;int a=par[b],d=0;for(int i=1;i<=n;i++)if(g[r][i]&&i!=a){d=i;break;}assert(d);
   vector<Op> rev;
   if(a==p) rev={op(b,r,p,q,{r,p},{p,b},{p,q})};
   else rev={op(a,r,p,q,{r,a},{a,p},{p,q}),op(b,r,a,p,{r,a},{r,p},{a,b})};
   step(op(b,a,r,d,{r,b},{r,a},{r,d}),rev);
  }
  // Reconfigure the remaining length-two branch to r-2-3.
  int tp=2,tq=3; if(r!=1){return;}
  int idx[105][105];memset(idx,-1,sizeof idx);struct Node{int p,q,pr,c;};vector<Node> ns;queue<int> qu;ns.push_back({p,q,-1,0});idx[p][q]=0;qu.push(0);int goal=-1;
  while(!qu.empty()){int id=qu.front();qu.pop();auto cur=ns[id];if(cur.p==tp&&cur.q==tq){goal=id;break;}for(int c=1;c<=n;c++)if(c!=r&&c!=cur.p&&c!=cur.q){int ar[3]={cur.p,cur.q,c};for(int i=0;i<3;i++)for(int j=0;j<3;j++)if(i!=j&&idx[ar[i]][ar[j]]<0){idx[ar[i]][ar[j]]=ns.size();ns.push_back({ar[i],ar[j],id,c});qu.push((int)ns.size()-1);}}}
  assert(goal>=0);vector<int> seq;for(int id=goal;ns[id].pr!=-1;id=ns[id].pr)seq.push_back(id);reverse(seq.begin(),seq.end());
  for(int id:seq){auto v=ns[id];int c=v.c, other=0;for(int x:{p,q,c})if(x!=v.p&&x!=v.q)other=x;
   Op f=op(q,p,r,c,{r,v.p},{v.p,v.q},{r,other});
   Op back=op(v.q,v.p,r,other,{r,p},{p,q},{r,c});step(f,{back});p=v.p;q=v.q;
  }
 }
};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int n;if(!(cin>>n))return 0;vector<pair<int,int>> s,t;vector<int>ds(n+1),dt(n+1);for(int i=1,u,v;i<n;i++){cin>>u>>v;s.push_back({u,v});ds[u]++;ds[v]++;}for(int i=1,u,v;i<n;i++){cin>>u>>v;t.push_back({u,v});dt[u]++;dt[v]++;}auto norm=[](vector<pair<int,int>> a){for(auto &e:a)if(e.first>e.second)swap(e.first,e.second);sort(a.begin(),a.end());return a;};if(norm(s)==norm(t)){cout<<"YES\n0\n";return 0;}int cs=0,ct=0;for(int i=1;i<=n;i++){if(ds[i]==n-1)cs=i;if(dt[i]==n-1)ct=i;}if(cs){cout<<"NO\n";return 0;}vector<Op> ans;
 if(ct){Builder a(n,ct,s);a.build();ans=a.ops;auto par=a.parents();int q=0,p=0;for(int i=1;i<=n;i++)if(par[i]&&par[i]!=ct){q=i;p=par[i];break;}int d=0;for(int i=1;i<=n;i++)if(a.g[ct][i]&&i!=p){d=i;break;}ans.push_back(op(q,p,ct,d,{ct,q},{ct,p},{ct,d}));}
 else {Builder a(n,1,s),b(n,1,t);a.build();b.build();ans=a.ops;for(int i=(int)b.undo.size()-1;i>=0;i--)for(auto o:b.undo[i])ans.push_back(o);}
 assert(ans.size()<=400);cout<<"YES\n"<<ans.size()<<'\n';for(auto o:ans){for(int x:o.v)cout<<x<<' ';cout<<'\n';for(auto [u,v]:o.e)cout<<u<<' '<<v<<' ';cout<<'\n';}
}

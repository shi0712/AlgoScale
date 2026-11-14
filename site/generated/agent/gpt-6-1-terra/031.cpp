#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const int MOD=998244353;
struct Mat { int a,b,c,d; };
Mat mul(const Mat &x,const Mat &y){
 return {(int)(((ll)x.a*y.a+(ll)x.b*y.c)%MOD), (int)(((ll)x.a*y.b+(ll)x.b*y.d)%MOD), (int)(((ll)x.c*y.a+(ll)x.d*y.c)%MOD), (int)(((ll)x.c*y.b+(ll)x.d*y.d)%MOD)};
}
struct Pair {int p,s;};
Pair combine(Pair x,Pair y){return {(int)((ll)x.p*y.p%MOD),(int)(((ll)x.s*y.p+(ll)x.p*y.s)%MOD)};}
int main(){
 ios::sync_with_stdio(false); cin.tie(nullptr);
 int n; if(!(cin>>n)) return 0;
 vector<vector<int>> adj(n);
 for(int i=1,u,v;i<n;i++){cin>>u>>v;--u;--v;adj[u].push_back(v);adj[v].push_back(u);}
 vector<int> parent(n,-1),order; order.reserve(n); order.push_back(0);
 for(int i=0;i<n;i++){int u=order[i];for(int v:adj[u]) if(v!=parent[u]) parent[v]=u,order.push_back(v);}
 vector<int> sz(n,1),heavy(n,-1);
 for(int i=n-1;i>0;i--){int u=order[i],p=parent[u];sz[p]+=sz[u]; if(heavy[p]<0||sz[u]>sz[heavy[p]]) heavy[p]=u;}
 vector<int> head(n),pos(n),tail(n),lightIndex(n,-1); int timer=0;
 for(int u:order) if(parent[u]<0||heavy[parent[u]]!=u){int h=u; for(int v=u;v!=-1;v=heavy[v]) head[v]=h,pos[v]=timer++; tail[h]=timer-1;}
 vector<int> base(n),lsize(n); int total=0;
 for(int u=0;u<n;u++){int cnt=0;for(int v:adj[u]) if(parent[v]==u&&v!=heavy[u]) lightIndex[v]=cnt++;
 if(cnt){int z=1;while(z<cnt)z*=2;lsize[u]=z;base[u]=total;total+=2*z;}}
 vector<Pair> light(total,{1,0});
 int N=1;while(N<n)N*=2;
 Mat id{1,0,0,1}, inactive{1,0,0,0}; vector<Mat> seg(2*N,id);
 for(int u=0;u<n;u++)seg[N+pos[u]]=inactive;
 for(int i=N-1;i;i--)seg[i]=mul(seg[i*2],seg[i*2+1]);
 vector<char> active(n,0); vector<int> f(n,1),g(n,0);
 auto matrixFor=[&](int u){Pair q=lsize[u]?light[base[u]+1]:Pair{1,0};if(!active[u])return Mat{q.p,0,0,0};int a=((ll)adj[u].size()*q.p-q.s)%MOD;if(a<0)a+=MOD;return Mat{a,q.p?MOD-q.p:0,q.p,0};};
 auto setmat=[&](int p,Mat x){p+=N;seg[p]=x;for(p/=2;p;p/=2)seg[p]=mul(seg[p*2],seg[p*2+1]);};
 auto query=[&](int l,int r){Mat left=id,right=id;for(l+=N,r+=N+1;l<r;l/=2,r/=2){if(l&1)left=mul(left,seg[l++]);if(r&1)right=mul(seg[--r],right);}return mul(left,right);};
 int factor=1; const int inv2=(MOD+1)/2;
 for(int step=0;step<n-1;step++){
  int x;cin>>x;--x; int already=0; for(int v:adj[x]) already+=active[v]; int delta=(int)adj[x].size()-already-1; if(delta<0) factor=(ll)factor*inv2%MOD; else while(delta--) factor=(ll)factor*2%MOD; active[x]=1;int u=x;
  while(true){setmat(pos[u],matrixFor(u));int h=head[u];Mat z=query(pos[h],tail[h]);int nf=z.a,ng=z.c;if(nf==f[h]&&ng==g[h])break;f[h]=nf;g[h]=ng;int p=parent[h];if(p<0)break;
   int k=base[p]+lsize[p]+lightIndex[h];light[k]={nf,ng}; k=(k-base[p])/2+base[p];while(k>base[p]){light[k]=combine(light[base[p]+2*(k-base[p])],light[base[p]+2*(k-base[p])+1]);k=(k-base[p])/2+base[p];} u=p;
  }
  cout<<(ll)f[0]*factor%MOD<<'\n';
 }
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll; const ll MOD=998244353;
struct M{ll a,b,c,d;};
static inline M mul(const M&x,const M&y){return {(x.a*y.a+x.b*y.c)%MOD,(x.a*y.b+x.b*y.d)%MOD,(x.c*y.a+x.d*y.c)%MOD,(x.c*y.b+x.d*y.d)%MOD};}
struct P{ll A,B;};
static inline P comb(const P&x,const P&y){return {x.A*y.A%MOD,(x.A*y.B+x.B*y.A)%MOD};}
int n; vector<vector<int>> adj; vector<int> par,heavy,top,pos,endp,sz,deg,lid,loff,lcnt; vector<char> inS;
vector<P> lt; // light segtrees pool: for vertex v, nodes [loff, loff+2*lcnt)
vector<M> seg; int SN; vector<int> at;
M mat(int v){P p=lcnt[v]?lt[loff[v]+1]:P{1,0};
  if(lcnt[v]==1) p=lt[loff[v]+1];
  if(inS[v]) return {((ll)deg[v]*p.A-p.B+MOD)%MOD,(MOD-p.A)%MOD,p.A,0};
  return {p.A,0,0,0};}
void supd(int i,const M&m){i+=SN;seg[i]=m;for(i>>=1;i;i>>=1)seg[i]=mul(seg[2*i],seg[2*i+1]);}
M squery(int l,int r){ // inclusive, ordered
  M L{1,0,0,1},R{1,0,0,1};
  for(l+=SN,r+=SN+1;l<r;l>>=1,r>>=1){if(l&1)L=mul(L,seg[l++]);if(r&1)R=mul(seg[--r],R);}
  return mul(L,R);}
void lupd(int v,int idx,P val){int base=loff[v],c=lcnt[v];int i=idx+c;lt[base+i]=val;for(i>>=1;i>=1;i>>=1)lt[base+i]=comb(lt[base+2*i],lt[base+2*i+1]);}
// note: for c==1, node 1 is leaf itself
int main(){scanf("%d",&n);adj.resize(n+1);for(int i=0;i<n-1;i++){int u,v;scanf("%d %d",&u,&v);adj[u].push_back(v);adj[v].push_back(u);}
  if(n==1)return 0;
  par.assign(n+1,0);heavy.assign(n+1,0);top.assign(n+1,0);pos.assign(n+1,0);endp.assign(n+1,0);sz.assign(n+1,1);deg.assign(n+1,0);lid.assign(n+1,-1);loff.assign(n+1,0);lcnt.assign(n+1,0);inS.assign(n+1,0);
  for(int i=1;i<=n;i++)deg[i]=adj[i].size();
  vector<int> order;order.reserve(n);{vector<int> st={1};par[1]=0;while(!st.empty()){int v=st.back();st.pop_back();order.push_back(v);for(int u:adj[v])if(u!=par[v]){par[u]=v;st.push_back(u);}}}
  for(int i=n-1;i>=0;i--){int v=order[i];int best=0;for(int u:adj[v])if(u!=par[v]){sz[v]+=sz[u];if(!best||sz[u]>sz[best])best=u;}heavy[v]=best;}
  // light children
  int tot=0;for(int v=1;v<=n;v++){for(int u:adj[v])if(u!=par[v]&&u!=heavy[v])lid[u]=lcnt[v]++;loff[v]=tot;tot+=2*max(lcnt[v],1);}
  lt.assign(tot,P{1,0});
  // hld positions
  {int cur=0;vector<int> st={1};while(!st.empty()){int h=st.back();st.pop_back();for(int v=h;v;v=heavy[v]){top[v]=h;pos[v]=cur++;for(int u:adj[v])if(u!=par[v]&&u!=heavy[v])st.push_back(u);}
     int e=h;while(heavy[e])e=heavy[e];for(int v=h;v;v=heavy[v])endp[v]=pos[e];}}
  SN=1;while(SN<n)SN<<=1;seg.assign(2*SN,M{1,0,0,1});
  // initial: all grounded, all g=1,h=0 -> matrices [[1,0],[0,0]]
  for(int v=1;v<=n;v++)seg[SN+pos[v]]=M{1,0,0,0};
  for(int i=SN-1;i>=1;i--)seg[i]=mul(seg[2*i],seg[2*i+1]);
  vector<ll> pw(2*n+5,1);for(size_t i=1;i<pw.size();i++)pw[i]=pw[i-1]*2%MOD;
  ll eSU=0,eSS=0,cS=0;
  string out;
  for(int step=0;step<n-1;step++){int x;scanf("%d",&x);
    for(int y:adj[x]){if(inS[y]){eSU--;eSS++;}else eSU++;}cS++;
    inS[x]=1;
    int v=x;
    while(v){supd(pos[v],mat(v));int t=top[v];M pr=squery(pos[t],endp[t]);int p=par[t];if(!p)break;lupd(p,lid[t],P{pr.a,pr.c});v=p;}
    M pr=squery(pos[1],endp[1]);ll det=pr.a;
    ll ans=det*pw[eSU-cS+eSS]%MOD;
    out+=to_string(ans);out+='\n';}
  fputs(out.c_str(),stdout);}

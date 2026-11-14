#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static char buf[1<<25];int bl=0,bp=0;
inline int gc(){if(bp==bl){bl=fread(buf,1,sizeof(buf),stdin);bp=0;if(bl<=0)return -1;}return buf[bp++];}
inline ll rd(){int c=gc();while(c!='-'&&(c<'0'||c>'9'))c=gc();ll x=0;while(c>='0'&&c<='9'){x=x*10+c-'0';c=gc();}return x;}
int n,m,V,S,Tn;bool finalArc=false;ll Lg;
struct E{int u,v;ll w;int f;};
vector<E> es; vector<vector<int>> adj; vector<int> fS,fT;
vector<ll> h,dist; vector<char> vis;
template<class F> inline void arcs(int x,F cb){
    // cb(y,cost,type,idx) type:0 grid,1 S->s,2 s->S,3 t->T,4 T->t
    if(x==S){for(int i=0;i<n;i++)cb(i*m,0LL,1,i*m);if(finalArc)cb(Tn,Lg,5,0);return;}
    if(x==Tn){for(int i=0;i<n;i++){int t=i*m+m-1;if(fT[t]>0)cb(t,0LL,4,t);}return;}
    for(int id:adj[x]){E&e=es[id];int y;int g;if(e.u==x){y=e.v;g=e.f;}else{y=e.u;g=-e.f;}
        if(g==0)cb(y,e.w,0,id);else if(g==-1)cb(y,-e.w,0,id);}
    if(x%m==0&&fS[x]>0)cb(S,0LL,2,x);
    if(x%m==m-1)cb(Tn,0LL,3,x);
}
const ll INF=LLONG_MAX/4;
static int hp[1024],pos[1024];int hs;
inline bool lessD(int a,int b){return dist[a]<dist[b];}
inline void up(int i){int x=hp[i];while(i>0){int p=(i-1)>>1;if(dist[hp[p]]<=dist[x])break;hp[i]=hp[p];pos[hp[i]]=i;i=p;}hp[i]=x;pos[x]=i;}
inline void down(int i){int x=hp[i];while(true){int c=2*i+1;if(c>=hs)break;if(c+1<hs&&dist[hp[c+1]]<dist[hp[c]])c++;if(dist[hp[c]]>=dist[x])break;hp[i]=hp[c];pos[hp[i]]=i;i=c;}hp[i]=x;pos[x]=i;}
void dijkstra(){
    dist.assign(V,INF);dist[S]=0;
    for(int v=0;v<V;v++)pos[v]=-1;
    hs=0;hp[hs++]=S;pos[S]=0;
    while(hs){int x=hp[0];pos[x]=-2;hs--;if(hs){hp[0]=hp[hs];pos[hp[0]]=0;down(0);}
        ll d=dist[x];
        arcs(x,[&](int y,ll c,int,int){if(pos[y]==-2)return;ll nd=d+c+h[x]-h[y];if(nd<dist[y]){dist[y]=nd;if(pos[y]==-1){hp[hs]=y;pos[y]=hs;hs++;}up(pos[y]);}});}
}
bool dfs(int x){
    if(x==Tn)return true;
    vis[x]=1;bool ok=false;
    arcs(x,[&](int y,ll c,int type,int idx){
        if(ok||vis[y])return; if(c+h[x]-h[y]!=0)return;
        if(dfs(y)){ok=true;
            if(type==0){E&e=es[idx];if(e.u==x)e.f++;else e.f--;}
            else if(type==1)fS[idx]++;else if(type==2)fS[idx]--;
            else if(type==3)fT[idx]++;else fT[idx]--;}
    });
    return ok;
}
int main(){
    int T=rd();string out;out.reserve(1<<24);
    while(T--){
        n=rd();m=rd();ll k=rd();
        V=n*m+2;S=n*m;Tn=n*m+1;
        es.clear();adj.assign(V,{});fS.assign(V,0);fT.assign(V,0);
        for(int i=0;i<n;i++)for(int j=0;j+1<m;j++){ll w=rd();es.push_back({i*m+j,i*m+j+1,w,0});}
        for(int i=0;i+1<n;i++)for(int j=0;j<m;j++){ll w=rd();es.push_back({i*m+j,(i+1)*m+j,w,0});}
        for(int i=0;i<(int)es.size();i++){adj[es[i].u].push_back(i);adj[es[i].v].push_back(i);}
        h.assign(V,0);vis.assign(V,0);
        ll L=-1;
        while(true){
            dijkstra();
            if(L<0)L=dist[Tn]+k;
            if(dist[Tn]+h[Tn]>=L){Lg=L;finalArc=true;dijkstra();finalArc=false;break;}
            for(int v=0;v<V;v++)h[v]+=dist[v];
            while(true){fill(vis.begin(),vis.end(),0);if(!dfs(S))break;}
        }
        vector<ll> d(V);for(int v=0;v<V;v++)d[v]=dist[v]+h[v];
        ll ans=0;vector<ll> nw(es.size());
        for(size_t i=0;i<es.size();i++){E&e=es[i];ll x=0;
            if(e.f==1)x=d[e.v]-d[e.u]-e.w;else if(e.f==-1)x=d[e.u]-d[e.v]-e.w;
            nw[i]=e.w+x;ans+=x;}
        out+=to_string(ans);out+='\n';
        size_t p=0;
        for(int i=0;i<n;i++){for(int j=0;j+1<m;j++){if(j)out+=' ';out+=to_string(nw[p++]);}out+='\n';}
        for(int i=0;i+1<n;i++){for(int j=0;j<m;j++){if(j)out+=' ';out+=to_string(nw[p++]);}out+='\n';}
    }
    fwrite(out.data(),1,out.size(),stdout);
}

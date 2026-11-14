#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int n,m,N;
vector<array<int,3>> E; // useful-subgraph edges with relabeled vertices
struct Pt{ll l,S,Q;};
const ll INF=LLONG_MAX/4;
vector<ll> cst;
// returns false if no cycle; else fills cycle point
bool query(ll a2, ll a1, Pt&res, ll&p, ll&q){ // cost = a2*w^2 + a1*w
    int M=E.size();
    cst.assign(M,0);
    for(int i=0;i<M;i++){ll w=E[i][2];cst[i]=a2*w*w+a1*w;}
    vector<vector<ll>> D(N+1,vector<ll>(N,INF));
    for(int v=0;v<N;v++)D[0][v]=0;
    for(int k=1;k<=N;k++)for(int i=0;i<M;i++){int u=E[i][0],v=E[i][1];if(D[k-1][u]<INF)D[k][v]=min(D[k][v],D[k-1][u]+cst[i]);}
    bool found=false; ll bp=0,bq=1;
    for(int v=0;v<N;v++){ if(D[N][v]>=INF)continue;
        ll mp=0,mq=0;bool h=false;
        for(int k=0;k<N;k++){ if(D[k][v]>=INF)continue; ll pp=D[N][v]-D[k][v],qq=N-k;
            if(!h||(__int128)pp*mq>(__int128)mp*qq){mp=pp;mq=qq;h=true;}}
        if(!found||(__int128)mp*bq<(__int128)bp*mq){bp=mp;bq=mq;found=true;}
    }
    if(!found)return false;
    p=bp;q=bq;
    vector<ll> c2(M),h(N,0);
    for(int i=0;i<M;i++)c2[i]=cst[i]*q-p;
    for(int it=0;it<N+1;it++){bool ch=false;for(int i=0;i<M;i++){int u=E[i][0],v=E[i][1];if(h[u]+c2[i]<h[v]){h[v]=h[u]+c2[i];ch=true;}}if(!ch)break;}
    vector<vector<int>> adj(N);
    for(int i=0;i<M;i++){int u=E[i][0],v=E[i][1];if(h[u]+c2[i]==h[v])adj[u].push_back(i);}
    vector<int> col(N,0),pe(N,-1);
    vector<int> cyc;
    function<bool(int)> dfs=[&](int u)->bool{
        col[u]=1;
        for(int i:adj[u]){int v=E[i][1];
            if(col[v]==1){ cyc.push_back(i); int x=u; while(x!=v){cyc.push_back(pe[x]);x=E[pe[x]][0];} return true;}
            if(col[v]==0){pe[v]=i;if(dfs(v))return true;}
        }
        col[u]=2;return false;};
    for(int s=0;s<N&&cyc.empty();s++)if(!col[s])dfs(s);
    assert(!cyc.empty());
    res={0,0,0};
    for(int i:cyc){ll w=E[i][2];res.l++;res.S+=w;res.Q+=w*w;}
    return true;
}
double best=1e18;
void consider(const Pt&P){ double v=(double)(P.Q*P.l-P.S*P.S)/((double)P.l*P.l); best=min(best,v);}
void rec(Pt A, Pt B, int depth){
    ll dmu=B.S*A.l-A.S*B.l, dq=B.Q*A.l-A.Q*B.l; // dmu>0
    Pt P;ll p,q;
    if(!query(dmu,-dq,P,p,q))return;
    // value at A: (Q1*dmu - S1*dq)/l1
    __int128 va=(__int128)A.Q*dmu-(__int128)A.S*dq;
    if((__int128)p*A.l<va*q){ consider(P); rec(A,P,depth+1); rec(P,B,depth+1);}
}
int main(){
    scanf("%d %d",&n,&m);
    vector<array<int,3>> e(m);
    vector<vector<int>> g(n+1),rg(n+1);
    for(auto&x:e){scanf("%d %d %d",&x[0],&x[1],&x[2]);g[x[0]].push_back(x[1]);rg[x[1]].push_back(x[0]);}
    auto bfs=[&](int s,vector<vector<int>>&G){vector<char> vis(n+1,0);vis[s]=1;vector<int> st{s};while(!st.empty()){int u=st.back();st.pop_back();for(int v:G[u])if(!vis[v]){vis[v]=1;st.push_back(v);}}return vis;};
    auto r1=bfs(1,g),r2=bfs(n,rg);
    if(!r1[n]){printf("-1\n");return 0;}
    // paths
    {
        int L=n-1; 
        vector<vector<ll>> dp(n+1,vector<ll>(20*L+1,INF)),nd;
        dp[1][0]=0;
        for(int k=1;k<=L;k++){
            nd.assign(n+1,vector<ll>(20*L+1,INF));
            int maxS=20*(k-1);
            for(auto&x:e){int u=x[0],v=x[1];ll w=x[2],ww=w*w;
                if(!r1[u]||!r2[v])continue;
                ll*a=dp[u].data();ll*b=nd[v].data()+w;
                for(int S=0;S<=maxS;S++){ if(a[S]+ww<b[S])b[S]=a[S]+ww;}
            }
            dp.swap(nd);
            for(int S=0;S<=20*k;S++)if(dp[n][S]<INF/2)consider({k,S,dp[n][S]});
        }
    }
    // cycles
    vector<int> id(n+1,-1);N=0;
    for(int v=1;v<=n;v++)if(r1[v]&&r2[v])id[v]=N++;
    for(auto&x:e)if(id[x[0]]>=0&&id[x[1]]>=0)E.push_back({id[x[0]],id[x[1]],x[2]});
    {
        const ll K=10000000;
        Pt A,B;ll p,q;
        if(query(1,K,A,p,q)){
            query(1,-K,B,p,q);
            consider(A);consider(B);
            if(B.S*A.l-A.S*B.l>0)rec(A,B,0);
        }
    }
    printf("%.15f\n",best);
}

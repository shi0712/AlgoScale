#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
int n,m;
vector<int> au,av,bu,bv;
vector<vector<pair<int,int>>> adj;
vector<int> tin,low,blk; int timer_,nb; vector<int> estk;
void dfs(int v,int pe){
    tin[v]=low[v]=++timer_;
    for(auto [to,id]:adj[v]){
        if(id==pe) continue;
        if(tin[to]){ if(tin[to]<tin[v]){low[v]=min(low[v],tin[to]);estk.push_back(id);} }
        else{
            estk.push_back(id);
            dfs(to,id);
            low[v]=min(low[v],low[to]);
            if(low[to]>=tin[v]){
                while(true){int x=estk.back();estk.pop_back();blk[x]=nb;if(x==id)break;}
                nb++;
            }
        }
    }
}
vector<int> dsu;
int fd(int x){while(dsu[x]!=x){dsu[x]=dsu[dsu[x]];x=dsu[x];}return x;}
vector<int> tdsu,tstamp; int curst=0;
inline void ti(int x){if(tstamp[x]!=curst){tstamp[x]=curst;tdsu[x]=x;}}
int tf(int x){ti(x);while(tdsu[x]!=x){tdsu[x]=tdsu[tdsu[x]];x=tdsu[x];}return x;}
int main(){
    int t;scanf("%d",&t);
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    while(t--){
        scanf("%d %d",&n,&m);
        au.resize(m);av.resize(m);bu.resize(m);bv.resize(m);
        adj.assign(n+1,{});
        for(int i=0;i<m;i++){scanf("%d %d %d %d",&au[i],&av[i],&bu[i],&bv[i]);adj[au[i]].push_back({av[i],i});adj[av[i]].push_back({au[i],i});}
        tin.assign(n+1,0);low.assign(n+1,0);blk.assign(m,-1);timer_=0;nb=0;estk.clear();
        dfs(1,-1);
        // per block: edges; build spanning tree (BFS order) in A
        vector<vector<int>> bedges(nb);
        for(int i=0;i<m;i++) bedges[blk[i]].push_back(i);
        // tree structure per block: order of (vertex, parent vertex, tree edge); nontree edges
        vector<vector<array<int,3>>> border(nb);
        vector<vector<int>> bnon(nb);
        {
            vector<vector<pair<int,int>>> ladj(n+1);
            vector<int> vis(n+1,-1);
            for(int b=0;b<nb;b++){
                vector<int> verts;
                for(int e:bedges[b]){ladj[au[e]].push_back({av[e],e});ladj[av[e]].push_back({au[e],e});verts.push_back(au[e]);verts.push_back(av[e]);}
                vector<char> used(bedges[b].size(),0);
                int r=au[bedges[b][0]];
                vis[r]=b; border[b].push_back({r,-1,-1});
                vector<char> istree;
                set<int> treeE;
                for(size_t q=0;q<border[b].size();q++){
                    int v=border[b][q][0];
                    for(auto [to,id]:ladj[v]) if(vis[to]!=b){vis[to]=b;border[b].push_back({to,v,id});treeE.insert(id);}
                }
                for(int e:bedges[b]) if(!treeE.count(e)) bnon[b].push_back(e);
                for(int v:verts) ladj[v].clear();
            }
        }
        dsu.resize(n+1);iota(dsu.begin(),dsu.end(),0);
        tdsu.assign(n+1,0);tstamp.assign(n+1,0);curst=0;
        vector<ull> R(n+1);for(auto&x:R)x=rng();
        vector<ull> P(n+1);
        vector<char> done(nb,0);int left=nb;
        bool prog=true;
        while(left&&prog){
            prog=false;
            for(int b=0;b<nb;b++){
                if(done[b])continue;
                auto hB=[&](int e){return R[fd(bu[e])]^R[fd(bv[e])];};
                bool ok=true;
                curst++;
                for(auto&[v,p,e]:border[b]){
                    if(p<0){P[v]=0;continue;}
                    P[v]=P[p]^hB(e);
                    int x=tf(fd(bu[e])),y=tf(fd(bv[e]));
                    if(x==y){ok=false;break;}
                    tdsu[x]=y;
                }
                if(ok) for(int e:bnon[b]) if((P[au[e]]^P[av[e]]^hB(e))!=0){ok=false;break;}
                if(ok){
                    done[b]=1;left--;prog=true;
                    for(int e:bedges[b]){int x=fd(bu[e]),y=fd(bv[e]);if(x!=y)dsu[x]=y;}
                }
            }
        }
        puts(left?"YES":"NO");
    }
}

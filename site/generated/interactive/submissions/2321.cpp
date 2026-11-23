#include <bits/stdc++.h>
using namespace std;

int n,m;
vector<int> eu,ev;
vector<vector<pair<int,int>>> adj;
vector<int> par,pe,dep,ord_;
vector<char> isTree,vis;
vector<int> status_;
vector<int> T;
int baseVal=-1;

int ask(const vector<int>& es){
    printf("?");
    for(int e:es) printf(" %d",e);
    printf("\n");
    fflush(stdout);
    int r;
    if(scanf("%d",&r)!=1) exit(0);
    if(r<0) exit(0);
    return r;
}
int getBase(){
    if(baseVal<0) baseVal=ask(T);
    return baseVal;
}
int queryReplace(int rem,int add){
    vector<int> es;
    for(int t:T) if(t!=rem) es.push_back(t);
    es.push_back(add);
    return ask(es);
}
vector<int> dsu;
int fnd(int x){while(dsu[x]!=x){dsu[x]=dsu[dsu[x]];x=dsu[x];}return x;}
bool uni(int a,int b){a=fnd(a);b=fnd(b);if(a==b)return false;dsu[a]=b;return true;}
int countS(const vector<int>& S){
    dsu.assign(n,0);
    for(int i=0;i<n;i++)dsu[i]=i;
    vector<int> es=S;
    for(int e:S) uni(eu[e],ev[e]);
    int sum=0;
    for(int t:T){
        if(uni(eu[t],ev[t])){es.push_back(t);sum+=status_[t];}
    }
    return ask(es)-sum;
}
void dfs(int v){
    vis[v]=1;ord_.push_back(v);
    for(auto [w,e]:adj[v]){
        if(!vis[w]){
            par[w]=v;pe[w]=e;dep[w]=dep[v]+1;isTree[e]=1;
            dfs(w);
        }
    }
}
void finish(){
    printf("!");
    for(int e=0;e<m;e++) if(status_[e]==1) printf(" %d",e);
    printf("\n");
    fflush(stdout);
    exit(0);
}
int main(){
    if(scanf("%d %d",&n,&m)!=2) return 0;
    eu.resize(m);ev.resize(m);adj.assign(n,{});
    for(int i=0;i<m;i++){
        scanf("%d %d",&eu[i],&ev[i]);
        adj[eu[i]].push_back({ev[i],i});
        adj[ev[i]].push_back({eu[i],i});
    }
    status_.assign(m,-1);
    if(m==n-1){
        for(int i=0;i<m;i++)status_[i]=1;
        finish();
    }
    par.assign(n,-1);pe.assign(n,-1);dep.assign(n,0);isTree.assign(m,0);vis.assign(n,0);
    dfs(0);
    for(int v=1;v<n;v++) T.push_back(pe[v]);
    const int INF=INT_MAX;
    vector<int> lowD(n,INF),lowE(n,-1);
    for(int e=0;e<m;e++){
        if(isTree[e]) continue;
        int u=eu[e],a=ev[e];
        if(dep[u]<dep[a]) swap(u,a);
        if(dep[a]<lowD[u]){lowD[u]=dep[a];lowE[u]=e;}
    }
    for(int i=n-1;i>=1;i--){
        int c=ord_[i];int p=par[c];
        if(lowD[c]<lowD[p]){lowD[p]=lowD[c];lowE[p]=lowE[c];}
    }
    for(int c=1;c<n;c++){
        if(lowD[c]>=dep[c]) status_[pe[c]]=1;
    }
    for(int idx=0;idx<n;idx++){
        int c=ord_[idx];
        if(c==0) continue;
        if(status_[pe[c]]!=-1) continue;
        int b=lowE[c];
        int u=eu[b],a=ev[b];
        if(dep[u]<dep[a]) swap(u,a);
        vector<int> U,K;
        int x=u;
        while(x!=a){
            int e=pe[x];
            if(status_[e]==-1)U.push_back(e);else K.push_back(e);
            x=par[x];
        }
        int base=getBase();
        vector<int> d(U.size());
        bool has1=false,hasm1=false;
        for(size_t i=0;i<U.size();i++){
            d[i]=queryReplace(U[i],b)-base;
            if(d[i]==1)has1=true;
            if(d[i]==-1)hasm1=true;
        }
        if(status_[b]==-1){
            if(has1) status_[b]=1;
            else if(hasm1) status_[b]=0;
            else{
                int k0=-1;
                for(int k:K) if(status_[k]==0){k0=k;break;}
                if(k0==-1) status_[b]=0;
                else{
                    int q=queryReplace(k0,b);
                    status_[b]=q-base+status_[k0];
                }
            }
        }
        for(size_t i=0;i<U.size();i++) status_[U[i]]=status_[b]-d[i];
    }
    // Phase 2: degrees
    vector<int> deg(n,0);
    vector<int> needQ;
    vector<int> knownCnt(n,0);
    vector<vector<int>> unkInc(n);
    for(int v=0;v<n;v++){
        for(auto [w,e]:adj[v]){
            if(status_[e]==1) knownCnt[v]++;
            else if(status_[e]==-1) unkInc[v].push_back(e);
        }
        if(unkInc[v].empty()) deg[v]=knownCnt[v];
        else needQ.push_back(v);
    }
    {
        long long sumKnown=0;
        for(int v=0;v<n;v++) if(unkInc[v].empty()) sumKnown+=deg[v];
        for(size_t i=0;i<needQ.size();i++){
            int v=needQ[i];
            if(i+1==needQ.size()){
                deg[v]=2*(n-1)-sumKnown;
            } else {
                deg[v]=knownCnt[v]+countS(unkInc[v]);
                sumKnown+=deg[v];
            }
        }
    }
    vector<char> alive(n,1);
    vector<int> rd=deg;
    deque<int> q;
    for(int v=0;v<n;v++) if(rd[v]==1) q.push_back(v);
    while(!q.empty()){
        int v=q.front();q.pop_front();
        if(!alive[v]||rd[v]!=1) continue;
        int found=-1;
        vector<int> C;
        for(auto [w,e]:adj[v]){
            if(!alive[w]) continue;
            if(status_[e]==1){found=e;break;}
            if(status_[e]==-1) C.push_back(e);
        }
        if(found==-1){
            while(C.size()>1){
                size_t h=C.size()/2;
                vector<int> A(C.begin(),C.begin()+h),B(C.begin()+h,C.end());
                int cnt=countS(A);
                if(cnt>0) C=A; else C=B;
            }
            if(C.empty()) continue;
            found=C[0];
            status_[found]=1;
        }
        int w=eu[found]==v?ev[found]:eu[found];
        alive[v]=0;
        rd[v]--;
        rd[w]--;
        if(rd[w]==1) q.push_back(w);
    }
    finish();
}

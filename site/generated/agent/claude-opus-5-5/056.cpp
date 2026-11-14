#include <bits/stdc++.h>
using namespace std;
int N;
vector<int> adjStart, adjList;
vector<int> mt, par, base_, q;
vector<char> used, blos;
vector<int> lcaMark; int lcaStamp=0;
vector<int> touched;
inline void touch(int v){ touched.push_back(v);}
int lca(int a,int b){
    ++lcaStamp;
    while(true){ a=base_[a]; lcaMark[a]=lcaStamp; if(mt[a]==-1)break; a=par[mt[a]]; }
    while(true){ b=base_[b]; if(lcaMark[b]==lcaStamp)return b; b=par[mt[b]]; }
}
vector<int> blosList;
void markPath(int v,int b,int children){
    while(base_[v]!=b){
        int bv=base_[v], bm=base_[mt[v]];
        if(!blos[bv]){blos[bv]=1;blosList.push_back(bv);}
        if(!blos[bm]){blos[bm]=1;blosList.push_back(bm);}
        par[v]=children; children=mt[v]; v=par[mt[v]];
    }
}
int qh,qt;
int findPath(int root){
    for(int v:touched){used[v]=0;par[v]=-1;base_[v]=v;}
    touched.clear();
    used[root]=1; touch(root); qh=qt=0; q[qt++]=root;
    while(qh<qt){
        int v=q[qh++];
        for(int e=adjStart[v];e<adjStart[v+1];e++){
            int to=adjList[e];
            if(base_[v]==base_[to]||mt[v]==to)continue;
            if(to==root||(mt[to]!=-1&&par[mt[to]]!=-1)){
                int cb=lca(v,to);
                blosList.clear();
                markPath(v,cb,to); markPath(to,cb,v);
                // all vertices whose base is in blos -> set to cb
                // iterate touched vertices (all in tree) 
                for(int i:touched){
                    if(blos[base_[i]]){
                        base_[i]=cb;
                        if(!used[i]){used[i]=1;q[qt++]=i;}
                    }
                }
                for(int x:blosList)blos[x]=0;
            } else if(par[to]==-1){
                par[to]=v; touch(to);
                if(mt[to]==-1)return to;
                int w=mt[to]; touch(w);
                used[w]=1; q[qt++]=w;
            }
        }
    }
    return -1;
}
int main(){
    int n,m; if(scanf("%d %d",&n,&m)!=2)return 0;
    vector<string> g(n);
    {static char buf[1024]; for(int i=0;i<n;i++){scanf("%s",buf); g[i]=buf;}}
    // vertex ids
    vector<int> id1(n*m,-1), id2(n*m,-1);
    int cnt=0, centers=0;
    for(int i=0;i<n;i++)for(int j=0;j<m;j++){
        int c=i*m+j;
        if(g[i][j]=='.') id1[c]=cnt++;
        else {id1[c]=cnt++; id2[c]=cnt++; centers++;}
    }
    N=cnt;
    vector<pair<int,int>> edges;
    int dx[4]={0,0,1,-1}, dy[4]={1,-1,0,0}; // 0,1 horizontal; 2,3 vertical
    for(int i=0;i<n;i++)for(int j=0;j<m;j++){
        if(g[i][j]=='.')continue;
        int c=i*m+j;
        edges.push_back({id1[c],id2[c]});
        for(int d=0;d<4;d++){
            int x=i+dx[d], y=j+dy[d];
            if(x<0||y<0||x>=n||y>=m||g[x][y]!='.')continue;
            int dt=id1[x*m+y];
            if(g[i][j]=='*'){
                edges.push_back({d<2?id1[c]:id2[c],dt});
            } else {
                edges.push_back({id1[c],dt}); edges.push_back({id2[c],dt});
            }
        }
    }
    adjStart.assign(N+1,0);
    for(auto&e:edges){adjStart[e.first+1]++;adjStart[e.second+1]++;}
    for(int i=0;i<N;i++)adjStart[i+1]+=adjStart[i];
    adjList.resize(adjStart[N]);
    {vector<int> pos(adjStart.begin(),adjStart.end()-1);
     for(auto&e:edges){adjList[pos[e.first]++]=e.second;adjList[pos[e.second]++]=e.first;}}
    mt.assign(N,-1);par.assign(N,-1);base_.resize(N);for(int i=0;i<N;i++)base_[i]=i;
    q.resize(N+5);used.assign(N,0);blos.assign(N,0);lcaMark.assign(N,0);
    // greedy: match center-dots first preferring low degree
    {
        vector<int> order(N); iota(order.begin(),order.end(),0);
        sort(order.begin(),order.end(),[&](int a,int b){return adjStart[a+1]-adjStart[a]<adjStart[b+1]-adjStart[b];});
        for(int v:order){ if(mt[v]!=-1)continue;
            int best=-1,bd=INT_MAX;
            for(int e=adjStart[v];e<adjStart[v+1];e++){int to=adjList[e]; if(mt[to]==-1){int d=adjStart[to+1]-adjStart[to]; if(d<bd){bd=d;best=to;}}}
            if(best!=-1){mt[v]=best;mt[best]=v;}
        }
    }
    for(int v=0;v<N;v++){
        if(mt[v]!=-1)continue;
        int u=findPath(v);
        while(u!=-1){
            int pv=par[u], ppv=mt[pv];
            mt[u]=pv; mt[pv]=u; u=ppv;
        }
    }
    // build trominoes
    vector<int> owner(n*m,-1); int T=0;
    vector<array<int,3>> tr;
    vector<int> dotCell(N,-1);
    for(int i=0;i<n;i++)for(int j=0;j<m;j++) if(g[i][j]=='.') dotCell[id1[i*m+j]]=i*m+j;
    for(int c=0;c<n*m;c++){
        if(id2[c]<0)continue;
        int a=mt[id1[c]], b=mt[id2[c]];
        if(a>=0&&b>=0&&dotCell[a]>=0&&dotCell[b]>=0){
            tr.push_back({c,dotCell[a],dotCell[b]});
            owner[c]=owner[dotCell[a]]=owner[dotCell[b]]=T++;
        }
    }
    vector<char> col(T,0);
    for(int t=0;t<T;t++){
        unsigned mask=0;
        for(int c:tr[t]){int i=c/m,j=c%m; for(int d=0;d<4;d++){int x=i+dx[d],y=j+dy[d]; if(x<0||y<0||x>=n||y>=m)continue; int o=owner[x*m+y]; if(o>=0&&o!=t&&col[o])mask|=1u<<(col[o]-'a');}}
        int k=0; while(mask>>k&1)k++;
        col[t]='a'+k;
        for(int c:tr[t]) g[c/m][c%m]=col[t];
    }
    string out; out.reserve(n*(m+1));
    for(int i=0;i<n;i++){out+=g[i];out+='\n';}
    fwrite(out.data(),1,out.size(),stdout);
}

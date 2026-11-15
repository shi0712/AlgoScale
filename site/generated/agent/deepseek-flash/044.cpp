// ==== World Map ====
// Build a K x K map (K = floor(5N/4)) of N countries whose map-adjacency graph is G.
//
// Layers: cell (i,j) (0-based) lies in layer s = i+j.  Cells of one layer are never
// side-adjacent; layer s is side-adjacent only to layers s-1 and s+1 (brick wall).
//
// Base map: colour whole layer s with W[s] for a walk W of G (W[s]==W[s+1] or
// W[s]~W[s+1]).  The resulting map is valid and realises exactly the edges W walks.
//
// Missing edges: paint one cell of layer s with colour c.  Every neighbour of that
// cell has colour W[s-1] or W[s+1], so painting is legal iff c~W[s-1] and c~W[s+1],
// and then the edges (c,W[s-1]), (c,W[s+1]) are realised.  Several cells of a layer
// may be painted (same-layer cells are not adjacent).  We only paint layers of one
// parity and never paint index 0 of a painted layer, so painted cells are never
// adjacent to each other and no base adjacency is destroyed.
//
// A greedy walk (steered by how many still-missing edges a move can realise) plus
// greedy painting covers all edges; randomised restarts are used when needed.
#include <bits/stdc++.h>
using namespace std;

static const int MAXN = 45;
int N, M;
vector<pair<int,int>> edges;
vector<vector<int>> adj;
static bool E[MAXN][MAXN];
static unsigned long long compMask[MAXN][MAXN];

int K, L;
vector<pair<int,int>> layerCells[130];
int layerLen[130];
int g[60][60];
static bool cellUsed[130][MAXN];

static inline int eid(int u,int v){ if(u>v) swap(u,v); return u*N+v; }

mt19937 rng(0x9E3779B9u);
static int paintParity;
static clock_t gStart;
static inline bool timeLeft(){ return (double)(clock()-gStart) < 0.38*CLOCKS_PER_SEC; }

// one randomised construction attempt; true on success
bool attempt(int mode, vector<vector<int>> &outGrid)
{
    static vector<int> W; W.clear(); W.reserve(L);
    static vector<char> cov, visited;
    cov.assign(N*N,0); visited.assign(N,0);
    for(int s=0;s<L;s++) for(int c=0;c<N;c++) cellUsed[s][c]=false;

    int cur = rng()%N;
    W.push_back(cur); visited[cur]=1;
    int dist[MAXN];

    auto computeDist = [&](){          // BFS distance to nearest missing edge
        for(int i=0;i<N;i++) dist[i]=1000;
        static deque<int> q; q.clear();
        for(auto &e: edges){
            if(!cov[eid(e.first,e.second)]){
                if(dist[e.first]>0){ dist[e.first]=0; q.push_back(e.first); }
                if(dist[e.second]>0){ dist[e.second]=0; q.push_back(e.second); }
            }
        }
        while(!q.empty()){
            int u=q.front(); q.pop_front();
            for(int v: adj[u]) if(dist[v]>dist[u]+1){ dist[v]=dist[u]+1; q.push_back(v); }
        }
    };

    // paint up to cap cells of layer s (neighbour colours x,y), greedily
    auto paintLayer = [&](int s,int x,int y,int cap){
        if(cap<=0) return;
        int bc[MAXN], bg[MAXN], cnt=0;
        unsigned long long m = compMask[x][y];
        while(m){
            int c=__builtin_ctzll(m); m&=m-1;
            int gain=0;
            if(c!=x && !cov[eid(c,x)]) gain++;
            if(c!=y && !cov[eid(c,y)]) gain++;
            if(gain>0){ bc[cnt]=c; bg[cnt]=gain; cnt++; }
        }
        int take = min(cap,cnt);
        for(int it=0;it<take;it++){
            int bi=it;
            for(int j=it+1;j<cnt;j++) if(bg[j]>bg[bi]) bi=j;
            swap(bg[it],bg[bi]); swap(bc[it],bc[bi]);
        }
        for(int it=0;it<take;it++){
            int c=bc[it];
            if(c!=x) cov[eid(c,x)]=1;
            if(c!=y) cov[eid(c,y)]=1;
            cellUsed[s][c]=true;
        }
    };

    long long distW  = (mode==0?3:(mode==1?5:2));
    long long spikeW = (mode==2?16:10);
    while((int)W.size()<L){
        int i=(int)W.size();
        int prev = (i>=2 ? W[i-2] : -1);
        bool canPaint = (i-1>=1 && i-1<=L-2 && ((i-1)&1)==paintParity);
        int cap = canPaint ? layerLen[i-1]-1 : 0;
        if((i&3)==0) computeDist();
        int best=-1; long long bestScore=LLONG_MIN;
        for(int v: adj[cur]){
            long long sc=0;
            if(!cov[eid(cur,v)]) sc+=10;                       // base edge
            if(canPaint && prev>=0){                           // pair (prev,v) coverage
                int est=0;
                unsigned long long m=compMask[prev][v];
                while(m){
                    int c=__builtin_ctzll(m); m&=m-1;
                    if(c!=prev && !cov[eid(c,prev)]) est++;
                    if(c!=v && !cov[eid(c,v)]) est++;
                }
                sc += (long long)min(est,2*cap)*10;
            }
            if(!visited[v]) sc+=2;
            sc -= distW*min(dist[v],50);
            sc += (long long)(rng()%5);
            if(sc>bestScore){ bestScore=sc; best=v; }
        }
        if(canPaint && prev>=0){               // (u,*,u) covers every edge at u
            int v=cur; int est=0;
            unsigned long long m=compMask[prev][v];
            while(m){
                int c=__builtin_ctzll(m); m&=m-1;
                if(c!=prev && !cov[eid(c,prev)]) est++;
                if(c!=v && !cov[eid(c,v)]) est++;
            }
            long long sc=(long long)min(est,2*cap)*spikeW;
            sc -= distW*min(dist[v],50);
            if(sc>bestScore){ bestScore=sc; best=v; }
        }
        if(best<0) best=cur;
        if(canPaint && prev>=0) paintLayer(i-1,prev,best,cap);
        if(best!=cur) cov[eid(cur,best)]=1;
        visited[best]=1;
        W.push_back(best); cur=best;
    }

    for(int s=0;s<L;s++) for(auto &pr: layerCells[s]) g[pr.first][pr.second]=W[s];
    for(int s=1;s<=L-2;s++){
        int idx=0;
        for(int c=0;c<N;c++) if(cellUsed[s][c]){
            if(idx>=(int)layerCells[s].size()-1) break;
            auto pr=layerCells[s][idx+1]; idx++;
            g[pr.first][pr.second]=c;
        }
    }

    // verify: all required edges realised, no forbidden adjacency, all colours used
    static bool eok[MAXN][MAXN];
    memset(eok,0,sizeof eok);
    for(int i=0;i<K;i++) for(int j=0;j<K;j++){
        if(i+1<K && g[i][j]!=g[i+1][j]){
            if(!E[g[i][j]][g[i+1][j]]) return false;
            eok[g[i][j]][g[i+1][j]]=eok[g[i+1][j]][g[i][j]]=true;
        }
        if(j+1<K && g[i][j]!=g[i][j+1]){
            if(!E[g[i][j]][g[i][j+1]]) return false;
            eok[g[i][j]][g[i][j+1]]=eok[g[i][j+1]][g[i][j]]=true;
        }
    }
    for(auto &e: edges) if(!eok[e.first][e.second]) return false;
    static bool seen[MAXN];
    memset(seen,0,sizeof seen);
    for(int i=0;i<K;i++) for(int j=0;j<K;j++) seen[g[i][j]]=true;
    for(int v=0;v<N;v++) if(!seen[v]) return false;

    outGrid.assign(K, vector<int>(K));
    for(int i=0;i<K;i++) for(int j=0;j<K;j++) outGrid[i][j]=g[i][j];
    return true;
}

static char outbuf[1<<22];
static int outpos;
static inline void wint(int x){
    char t[8]; int n=0;
    if(x==0){ outbuf[outpos++]='0'; return; }
    while(x){ t[n++]='0'+x%10; x/=10; }
    while(n) outbuf[outpos++]=t[--n];
}
static inline void wch(char c){ outbuf[outpos++]=c; }
static void flushOut(){ fwrite(outbuf,1,outpos,stdout); outpos=0; }

int main(){
    gStart = clock();
    int T;
    if(scanf("%d",&T)!=1) return 0;
    while(T--){
        if(scanf("%d %d",&N,&M)!=2) return 0;
        memset(E,0,sizeof E);
        adj.assign(N,{});
        edges.clear();
        for(int i=0;i<M;i++){
            int a,b; if(scanf("%d %d",&a,&b)!=2) return 0;
            --a; --b;
            if(a<0||a>=N||b<0||b>=N||a==b) continue;
            edges.push_back({a,b});
            E[a][b]=E[b][a]=true;
            adj[a].push_back(b); adj[b].push_back(a);
        }
        if(N==1){ wch('1'); wch('\n'); wch('1'); wch('\n'); flushOut(); continue; }
        K = 5*N/4;
        L = 2*K-1;
        for(int s=0;s<L;s++){
            layerCells[s].clear();
            for(int i=0;i<K;i++){ int j=s-i; if(j>=0&&j<K) layerCells[s].push_back({i,j}); }
            layerLen[s]=(int)layerCells[s].size();
        }
        for(int x=0;x<N;x++) for(int y=0;y<N;y++){
            unsigned long long m=0;
            for(int c=0;c<N;c++) if((c==x||E[c][x])&&(c==y||E[c][y])) m|=1ULL<<c;
            compMask[x][y]=m;
        }
        vector<vector<int>> grid;
        bool ok=false;
        for(int att=0; att<1000 && !ok; att++){
            paintParity = (att/3)&1;
            ok = attempt(att&3, grid);
            if((att&15)==15 && !timeLeft()) break;
        }
        if(!ok){
            int extra=0;
            while(!ok && extra<20000 && timeLeft()){
                paintParity = (int)(rng()&1);
                ok = attempt((int)(rng()&3), grid);
                extra++;
            }
        }
        if(!ok){                       // should not happen for valid inputs
            grid.assign(K, vector<int>(K,0));
            for(int i=0;i<K;i++) for(int j=0;j<K;j++) grid[i][j]=g[i][j];
        }
        wint(K); wch('\n');
        for(int i=0;i<K;i++)
            for(int j=0;j<K;j++){
                wint(grid[i][j]+1);
                wch(j+1==K?'\n':' ');
            }
        flushOut();
    }
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const ll NEG = -(ll)4e18;

static int n;
static unordered_map<long long,ll> qcache;
static long long queryIO=0;
static vector<char> isPos;
static vector<int> prefNonpos;

static inline long long qkey(int l,int r){ return ((long long)l<<32) | (unsigned)r; }

static ll ask(int l,int r){
    long long k=qkey(l,r);
    auto it=qcache.find(k);
    if(it!=qcache.end()) return it->second;
    printf("? %d %d\n",l,r);
    fflush(stdout);
    ll x;
    if(scanf("%lld",&x)!=1){ exit(0); }
    qcache.emplace(k,x);
    queryIO++;
    return x;
}
static inline ll getM(int l,int r){ return ask(l,r); }

// ---------- constraints ----------
struct Edge { int u,v; ll w; };
static vector<Edge> edges;

// weighted DSU: P_x - P_par[x] = dsuDiff[x]
static vector<int> par;
static vector<ll> dsuDiff;

static void dsuInit(int N){
    par.resize(N); dsuDiff.assign(N,0);
    for(int i=0;i<N;i++) par[i]=i;
}
static pair<int,ll> dsuFind(int x){
    if(par[x]==x) return {x,0};
    auto r=dsuFind(par[x]);
    dsuDiff[x]+=r.second;
    par[x]=r.first;
    return {par[x], dsuDiff[x]};
}
static void dsuUnite(int a,int b,ll c){ // P_b - P_a = c
    auto ra=dsuFind(a), rb=dsuFind(b);
    if(ra.first==rb.first) return;
    // P_{ra} = P_{rb} + (rb.second - ra.second - c)
    par[ra.first]=rb.first;
    dsuDiff[ra.first]=rb.second-ra.second-c;
}

static inline void addUpper(int i,int j,ll w){ edges.push_back({i,j,w}); }
static inline void addEq(int i,int j,ll w){
    edges.push_back({i,j,w});
    edges.push_back({j,i,-w});
    dsuUnite(i,j,w);
}

static int scanMode; // 0 = only top-level good blocks, 1 = all levels

static void shortestMax(int L,int R,ll v,int &s,int &e){
    int lo=L,hi=R;
    while(lo<hi){ int mid=(lo+hi)>>1; if(getM(L,mid)>=v) hi=mid; else lo=mid+1; }
    e=lo;
    lo=L; hi=e;
    while(lo<hi){ int mid=(lo+hi+1)>>1; if(getM(mid,e)>=v) lo=mid; else hi=mid-1; }
    s=lo;
}

static inline bool allPositive(int L,int R){
    return (prefNonpos[R]-prefNonpos[L-1])==0;
}

static void good(int L,int R,bool top);
static void freeRec(int L,int R);

static void addScans(int L,int R){
    if(scanMode==2){
        for(int x=L;x<R;x++){
            addUpper(L-1,x,getM(L,x));
            addUpper(x,R,getM(x+1,R));
        }
        return;
    }
    int i=L;
    while(i<=R){
        if(isPos[i]){ i++; continue; }
        int u=i;
        while(i+1<=R && !isPos[i+1]) i++;
        int w=i;
        if(w+1<=R){
            int x=w+1;
            while(x+1<=R && isPos[x+1]) x++;
            addUpper(L-1,x,getM(L,x));
        }
        if(u-1>=L){
            int x=u-1;
            while(x-1>=L && isPos[x-1]) x--;
            addUpper(x-1,R,getM(x,R));
        }
        i++;
    }
}

static void good(int L,int R,bool top){
    ll v=getM(L,R);
    addUpper(L-1,R,v);
    if(L==R) return;
    if(allPositive(L,R)) return;
    if(scanMode!=0 || top) addScans(L,R);
    ll ml=getM(L,R-1), mr=getM(L+1,R);
    if(ml>=mr){
        ll W=ml;
        int lo=L,hi=R-1;
        while(lo<hi){ int mid=(lo+hi)>>1; if(getM(L,mid)>=W) hi=mid; else lo=mid+1; }
        int j=lo;
        addEq(L-1,j,W);
        good(L,j,false);
        freeRec(j+1,R);
    } else {
        ll W=mr;
        int lo=L+1,hi=R;
        while(lo<hi){ int mid=(lo+hi+1)>>1; if(getM(mid,R)>=W) lo=mid; else hi=mid-1; }
        int idx=lo;
        addEq(idx-1,R,W);
        freeRec(L,idx-1);
        good(idx,R,false);
    }
}

static void freeRec(int L,int R){
    if(L>R) return;
    if(allPositive(L,R)) return;
    ll v=getM(L,R);
    addUpper(L-1,R,v);
    if(v==0) return;
    int s,e;
    shortestMax(L,R,v,s,e);
    addEq(s-1,e,v);
    freeRec(L,s-1);
    good(s,e,true);
    freeRec(e+1,R);
}

static vector<ll> solveDist(){
    int V=n+1;
    vector<vector<pair<int,ll>>> adj(V);
    for(auto &e:edges) adj[e.u].push_back({e.v,e.w});
    vector<ll> dist(V,0);
    vector<char> inq(V,1);
    deque<int> dq;
    for(int i=0;i<V;i++) dq.push_back(i);
    while(!dq.empty()){
        int u=dq.front(); dq.pop_front(); inq[u]=0;
        ll du=dist[u];
        for(auto &pr:adj[u]){
            int v=pr.first;
            if(dist[v]>du+pr.second){
                dist[v]=du+pr.second;
                if(!inq[v]){ inq[v]=1; dq.push_back(v); }
            }
        }
    }
    ll sh=dist[0];
    vector<ll> P(V);
    for(int i=0;i<V;i++) P[i]=dist[i]-sh;
    return P;
}

// returns true if b (from P) has M_b == LB(components)
static bool certificate(const vector<ll>& P, const vector<int>& roots){
    int W=n+1;
    vector<ll> dp((size_t)W*W, NEG);
    for(int i=0;i<n;i++){
        int ri=roots[i];
        for(int j=i+1;j<=n;j++){
            if(roots[j]==ri){
                dp[(size_t)(i+1)*W + j] = P[j]-P[i];
            }
        }
    }
    // DP by increasing length
    for(int len=1;len<=n;len++){
        for(int l=1;l+len-1<=n;l++){
            int r=l+len-1;
            size_t idx=(size_t)l*W + r;
            ll val=dp[idx];
            if(len>1){
                ll a=dp[(size_t)l*W + (r-1)];
                ll b=dp[(size_t)(l+1)*W + r];
                if(a>val) val=a;
                if(b>val) val=b;
            }
            if(val<0) val=0;
            dp[idx]=val;
        }
    }
    // compute b and M_b, compare
    vector<ll> b(n+1);
    for(int i=1;i<=n;i++) b[i]=P[i]-P[i-1];
    for(int l=1;l<=n;l++){
        ll cur=0,best=0;
        for(int r=l;r<=n;r++){
            cur+=b[r];
            if(cur<0) cur=0;
            if(cur>best) best=cur;
            if(best!=dp[(size_t)l*W + r]) return false;
        }
    }
    return true;
}

int main(){
    if(scanf("%d",&n)!=1) return 0;
    qcache.reserve(1<<20);
    isPos.assign(n+1,0);
    prefNonpos.assign(n+1,0);
    // phase A: top-level scans
    scanMode=0;
    edges.clear();
    dsuInit(n+1);
    for(int i=1;i<=n;i++){
        ll x=getM(i,i);
        isPos[i]=(x>0);
        prefNonpos[i]=prefNonpos[i-1]+(isPos[i]?0:1);
        if(x>0) addEq(i-1,i,x);
        else addUpper(i-1,i,0);
    }
    freeRec(1,n);
    vector<ll> P=solveDist();
    vector<int> roots(n+1);
    {
        for(int i=0;i<=n;i++){ auto r=dsuFind(i); roots[i]=r.first; }
    }
    if(certificate(P,roots)){
        printf("!");
        for(int i=1;i<=n;i++) printf(" %lld",P[i]-P[i-1]);
        printf("\n"); fflush(stdout);
        return 0;
    }
    // phase B: all-level scans, reusing cache
    scanMode=1;
    edges.clear();
    dsuInit(n+1);
    for(int i=1;i<=n;i++){
        ll x=getM(i,i);
        if(x>0) addEq(i-1,i,x);
        else addUpper(i-1,i,0);
    }
    freeRec(1,n);
    P=solveDist();
    for(int i=0;i<=n;i++){ auto r=dsuFind(i); roots[i]=r.first; }
    if(certificate(P,roots)){
        printf("!");
        for(int i=1;i<=n;i++) printf(" %lld",P[i]-P[i-1]);
        printf("\n"); fflush(stdout);
        return 0;
    }
    // phase C: full prefix/suffix scans (last-resort tightening)
    if(queryIO > 150000){
        printf("!");
        for(int i=1;i<=n;i++) printf(" %lld",P[i]-P[i-1]);
        printf("\n"); fflush(stdout);
        return 0;
    }
    scanMode=2;
    edges.clear();
    dsuInit(n+1);
    for(int i=1;i<=n;i++){
        ll x=getM(i,i);
        if(x>0) addEq(i-1,i,x);
        else addUpper(i-1,i,0);
    }
    freeRec(1,n);
    P=solveDist();
    printf("!");
    for(int i=1;i<=n;i++) printf(" %lld",P[i]-P[i-1]);
    printf("\n"); fflush(stdout);
    return 0;
}

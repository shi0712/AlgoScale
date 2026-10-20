#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned int u32;
static const ll MOD = 1000000007;

static ll pw(ll b, ll e){ ll r=1; b%=MOD; while(e){ if(e&1) r=r*b%MOD; b=b*b%MOD; e>>=1; } return r; }
static ll inv2, inv4;

/* ---------- problem data ---------- */
int n, m;
struct Edge { int u,v,w; };
vector<Edge> es;

/* ---------- hierarchy ---------- */
struct Node {
    vector<int> verts;   // global vertices
    vector<int> ch;      // children
    int level;
    vector<int> cross;   // edge indices that are cross edges here
    vector<int> vlist;   // sorted verts
    vector<u32> dist;    // local mask -> probability
    int loc[20];         // global vertex -> local index
    bool leaf;
};
vector<Node> nodes;

int cnv[20]; // vertex -> current node id during construction

static int dsu_f(vector<int>&p,int x){ while(p[x]!=x){p[x]=p[p[x]];x=p[x];} return x; }

void build_hierarchy(){
    // singletons
    for(int v=0;v<n;v++){
        Node nd; nd.verts={v}; nd.level=0; nd.leaf=true;
        nd.vlist={v}; nd.loc[v]=0;
        nd.dist.assign(2,0); nd.dist[1]=1;
        nodes.push_back(nd);
        cnv[v]=(int)nodes.size()-1;
    }
    vector<int> ord(m); for(int i=0;i<m;i++) ord[i]=i;
    sort(ord.begin(),ord.end(),[&](int a,int b){ return es[a].w<es[b].w; });
    int p=0;
    while(p<m){
        int q=p; while(q<m && es[ord[q]].w==es[ord[p]].w) q++;
        int lam=es[ord[p]].w;
        // map involved old nodes -> temp index
        vector<int> involved;
        for(int t=p;t<q;t++){ int e=ord[t]; involved.push_back(cnv[es[e].u]); involved.push_back(cnv[es[e].v]); }
        sort(involved.begin(),involved.end());
        involved.erase(unique(involved.begin(),involved.end()),involved.end());
        int kk=involved.size();
        vector<int> tmp(kk); // dsu over involved
        for(int i=0;i<kk;i++) tmp[i]=i;
        auto idxof=[&](int nodeid)->int{ return lower_bound(involved.begin(),involved.end(),nodeid)-involved.begin(); };
        // union by edges (ignore self)
        for(int t=p;t<q;t++){
            int e=ord[t]; int a=idxof(cnv[es[e].u]), b=idxof(cnv[es[e].v]);
            if(a!=b){ int ra=dsu_f(tmp,a), rb=dsu_f(tmp,b); if(ra!=rb) tmp[ra]=rb; }
        }
        // groups
        map<int,vector<int>> grp;
        for(int i=0;i<kk;i++) grp[dsu_f(tmp,i)].push_back(involved[i]);
        // for each group with >=2 nodes, create new node
        map<int,int> newnode; // root -> new node id
        for(auto &g : grp){
            if(g.second.size()<2) continue;
            Node nd; nd.level=lam; nd.leaf=false; nd.ch=g.second;
            vector<int> vs;
            for(int cid : nd.ch) for(int v : nodes[cid].verts) vs.push_back(v);
            sort(vs.begin(),vs.end());
            nd.verts=vs; nd.vlist=vs;
            for(int i=0;i<(int)vs.size();i++) nd.loc[vs[i]]=i;
            nodes.push_back(nd);
            newnode[g.first]=(int)nodes.size()-1;
        }
        // assign cross edges
        for(int t=p;t<q;t++){
            int e=ord[t]; int a=cnv[es[e].u], b=cnv[es[e].v];
            if(a==b) continue; // ineligible / internal
            int r=dsu_f(tmp,idxof(a));
            auto it=newnode.find(r);
            if(it!=newnode.end()) nodes[it->second].cross.push_back(e);
        }
        // update cnv
        for(auto &g : grp){
            if(g.second.size()<2) continue;
            int nid=newnode[g.first];
            for(int cid : g.second) for(int v : nodes[cid].verts) cnv[v]=nid;
        }
        p=q;
    }
}

/* ---------- helpers ---------- */
static inline void addmod(ll &a, ll b){ a+=b; if(a>=MOD) a%=MOD; }

/* translation: child's local mask -> parent's local mask */
static void build_trans(const Node &par, const Node &chi, vector<u32> &tr){
    int nc = chi.verts.size(), np = par.verts.size();
    tr.assign(1u<<nc, 0);
    // map each child local index to parent local bit
    u32 bit[20];
    for(int i=0;i<nc;i++) bit[i] = 1u<<par.loc[chi.vlist[i]];
    tr[0]=0;
    for(u32 s=1; s<(1u<<nc); s++){
        u32 lb = s & (~s+1);
        int i = __builtin_ctz(lb);
        tr[s] = tr[s^lb] | bit[i];
    }
}

/* ---------- exact recursive probability that induced digraph on S has a root reaching all ---------- */
/* w_S(A) = probability no arc enters A from S\A ; arcs independent with p=1/2 (equal weight level) */
static void compute_P0(int nv, const vector<pair<int,int>>& arcs, vector<ll>& P0){
    // arcs: directed (i,j) present with prob 1/2, independent (single-level case)
    // P0[S] = P( no proper nonempty in-isolated subset in induced digraph on S )
    int N=1<<nv;
    // precompute cross[S][A]? too big; compute w(A,S) on the fly
    // w(A) with S: product over arcs (i,j), i in S\A, j in A of 1/2 -> 2^{-cnt}
    vector<vector<ll>> wfin; // not used
    P0.assign(N,0); P0[0]=1;
    // We need, for each S, sum over A subset of S: w_S(A)*P0[S\A]
    // do O(3^nv)
    vector<int> cntarc(nv*nv,0);
    for(auto&a:arcs) cntarc[a.first*nv+a.second]=1;
    // precompute inmask[j] = set of i with arc i->j
    vector<int> inmask(nv,0);
    for(int j=0;j<nv;j++) for(int i=0;i<nv;i++) if(cntarc[i*nv+j]) inmask[j]|=1<<i;
    for(int S=1;S<N;S++){
        ll sum=0;
        // enumerate nonempty proper A subset of S
        for(int A=(S-1)&S; A; A=(A-1)&S){
            // cnt arcs from S\A into A
            int cnt=0;
            for(int j=0;j<nv;j++) if(A>>j&1) cnt += __builtin_popcount((unsigned)(inmask[j] & S & ~A));
            ll w = pw(inv2,cnt);
            sum = (sum + w*P0[S^A])%MOD;
        }
        P0[S] = ((1-sum)%MOD+MOD)%MOD;
    }
}

/* ---------- node DP ---------- */
static ll BUDGET = 60000000;

void solve_node(int id){
    Node &nd = nodes[id];
    if(nd.leaf){ return; }
    int k = nd.ch.size();
    int nc = nd.verts.size();
    u32 full = (1u<<nc)-1;
    nd.dist.assign(1u<<nc, 0);

    if(k==2){
        Node &L = nodes[nd.ch[0]], &R = nodes[nd.ch[1]];
        int nl=L.verts.size(), nr=R.verts.size();
        vector<u32> trL, trR;
        build_trans(nd,L,trL); build_trans(nd,R,trR);
        // av[v] for v in R's local: inv2^{#edges L->v}
        vector<ll> avLocal(1u<<nr,1), bvLocal(1u<<nl,1);
        // count edges
        vector<int> cntR(nr,0), cntL(nl,0);
        for(int e : nd.cross){
            int u=es[e].u, v=es[e].v;
            int lu=L.loc[u], lr=R.loc[v];
            bool uin = (lu>=0 && L.vlist[lu]==u);
            int cu = (L.loc[u]>=0 && L.vlist[L.loc[u]]==u) ? L.loc[u] : -1;
            int cv = (R.loc[v]>=0 && R.vlist[R.loc[v]]==v) ? R.loc[v] : -1;
            int cv2= (R.loc[u]>=0 && R.vlist[R.loc[u]]==u) ? R.loc[u] : -1;
            int cu2= (L.loc[v]>=0 && L.vlist[L.loc[v]]==v) ? L.loc[v] : -1;
            (void)uin;(void)lr;
            if(cu>=0 && cv>=0) cntR[cv]++;          // arc L->R head v
            if(cu2>=0 && cv2>=0) cntL[cu2]++;       // arc R->L head u
        }
        // subset products
        for(u32 s=1;s<(1u<<nr);s++){ u32 lb=s&(~s+1); int i=__builtin_ctz(lb); avLocal[s]=avLocal[s^lb]*pw(inv2,cntR[i])%MOD; }
        for(u32 s=1;s<(1u<<nl);s++){ u32 lb=s&(~s+1); int i=__builtin_ctz(lb); bvLocal[s]=bvLocal[s^lb]*pw(inv2,cntL[i])%MOD; }
        for(u32 sL=0;sL<(1u<<nl);sL++){
            ll wL=L.dist[sL]; if(!wL) continue;
            ll p2 = (1 - bvLocal[sL] + MOD)%MOD;
            u32 gL = trL[sL];
            for(u32 sR=0;sR<(1u<<nr);sR++){
                ll wR=R.dist[sR]; if(!wR) continue;
                ll p1 = (1 - avLocal[sR] + MOD)%MOD;
                ll ww = wL*wR%MOD;
                ll q1=(1-p1+MOD)%MOD, q2=(1-p2+MOD)%MOD;
                nd.dist[gL|trR[sR]] = (nd.dist[gL|trR[sR]] + ww%MOD*p1%MOD*p2)%MOD;
                nd.dist[gL]          = (nd.dist[gL]          + ww%MOD*p1%MOD*q2)%MOD;
                nd.dist[trR[sR]]     = (nd.dist[trR[sR]]     + ww%MOD*q1%MOD*p2)%MOD;
                nd.dist[0]           = (nd.dist[0]           + ww%MOD*q1%MOD*q2)%MOD;
            }
        }
        return;
    }

    /* k >= 3 : cases */
    int t = nd.cross.size();
    // special case: all children singletons and this is the root -> use P0 recursion
    bool allsing = true;
    for(int c : nd.ch) if(nodes[c].verts.size()!=1) { allsing=false; break; }
    bool isroot = (nc==n);

    // decide exact cross-enumeration feasibility
    double tupleCount = 1; for(int c: nd.ch) tupleCount *= (double)(1ull<<nodes[c].verts.size());
    double crossCount = 1; for(int i=0;i<t;i++){ crossCount *= 4.0; if(crossCount>1e18) break; }
    bool canEnum = (tupleCount * crossCount <= (double)BUDGET) && t<=10;

    if(!canEnum && allsing && isroot && n<=17){
        // exact: single-level-like root; arcs of cross edges, independent directions with p=1/2
        // local index mapping = nd.loc
        vector<pair<int,int>> arcs;
        for(int e: nd.cross){
            int u=es[e].u, v=es[e].v;
            arcs.push_back({nd.loc[u],nd.loc[v]});
            arcs.push_back({nd.loc[v],nd.loc[u]});
        }
        vector<ll> P0;
        compute_P0(nc, arcs, P0);
        ll pnone = P0[(1<<nc)-1]; // probability that R==empty : no root reaching all
        nd.dist.assign(1u<<nc,0);
        nd.dist[0]=pnone;
        nd.dist[(1u<<nc)-1] = ((1-pnone)%MOD+MOD)%MOD; // placeholder for nonempty mass
        // mark special: store nonempty mass at full mask
        nd.special_p0 = true;
        return;
    }

    if(!canEnum){
        // crude fallback: independent-block approximation (never exact in general)
        // compute per-block p_i = 1 - prod_{j!=i} P(no arc block i -> S_j) under head-set uses
        // Use a simple factorized approximation ignoring path structure.
        // We'll approximate by: treat each block independently reaching-all with p_i,
        // then enumerate good sets only if k small.
        // For safety: compute via product over blocks for union distribution using
        // P(all blocks good) etc. -> use "all or nothing" fallback.
        nd.dist.assign(1u<<nc,0);
        // gather: with prob prod p_i all blocks good -> union of all child sets; we need per-tuple anyway
        // do a (possibly slow) tuple enumeration with approximation
        vector<u32> trs;
        vector<vector<u32>> tr(k);
        for(int i=0;i<k;i++) build_trans(nd, nodes[nd.ch[i]], tr[i]);
        vector<u32> cur(k,0);
        vector<ll> probs(k,0);
        // per-edge arc absences
        // enumerate tuples recursively
        function<void(int,ll)> rec = [&](int ci, ll w){
            if(ci==k){
                // compute p_i
                ll allgood=1, allbad=1;
                for(int i=0;i<k;i++){
                    // p_i = 1 - prod_j P(no arc from block i into S_j)
                    ll q=1;
                    for(int e: nd.cross){
                        int u=es[e].u, v=es[e].v;
                        int bu=-1,bv=-1;
                        for(int c=0;c<k;c++){ if(nodes[nd.ch[c]].loc[u]>=0 && nodes[nd.ch[c]].vlist[nodes[nd.ch[c]].loc[u]]==u) bu=c; if(nodes[nd.ch[c]].loc[v]>=0 && nodes[nd.ch[c]].vlist[nodes[nd.ch[c]].loc[v]]==v) bv=c; }
                        if(bu<0||bv<0) continue;
                        // arc bu->bv usable if head v in S_bv
                        bool hv = (cur[bv] >> nodes[nd.ch[bv]].loc[v]) & 1;
                        if(bu!=bv && hv) q = q*inv2%MOD;
                    }
                    ll pi=(1-q+MOD)%MOD;
                    allgood = allgood*pi%MOD;
                    allbad  = allbad*(1-pi+MOD)%MOD;
                }
                u32 U=0; for(int i=0;i<k;i++) if(cur[i]) U|=tr[i][cur[i]];
                nd.dist[U] = (nd.dist[U] + w*allgood)%MOD;
                nd.dist[0] = (nd.dist[0] + w*allbad)%MOD;
                return;
            }
            for(u32 s=0;s<(1u<<nodes[nd.ch[ci]].verts.size());s++){
                if(!nodes[nd.ch[ci]].dist[s]) continue;
                cur[ci]=s;
                rec(ci+1, w*nodes[nd.ch[ci]].dist[s]%MOD);
            }
        };
        rec(0,1);
        return;
    }

    /* exact cross-enumeration */
    // map each vertex to block index
    vector<int> blk(n,-1);
    for(int i=0;i<k;i++) for(int v : nodes[nd.ch[i]].verts) blk[v]=i;
    // precompute for each cross edge and each state the arcs (from,to) with head vertex
    // state 0: both; 1: u->v ; 2: v->u ; 3: none
    int t2=t;
    vector<array<pair<int,int>,2>> arcOf(t2); // (blockfrom,blockto) and head vertex packed
    vector<array<int,2>> headOf(t2);
    vector<array<int,2>> tailOf(t2);
    for(int i=0;i<t2;i++){
        int e=nd.cross[i], u=es[e].u, v=es[e].v;
        arcOf[i][0]={blk[u],blk[v]}; headOf[i][0]=v; // arc u->v
        arcOf[i][1]={blk[v],blk[u]}; headOf[i][1]=u; // arc v->u
    }
    // iterate over tuples
    vector<u32> cur(k,0);
    vector<vector<u32>> tr(k);
    for(int i=0;i<k;i++) build_trans(nd, nodes[nd.ch[i]], tr[i]);
    vector<int> statev(t,0);
    function<void(int,ll)> rec = [&](int ci, ll w){
        if(ci==k){
            // enumerate 4^t states
            int tot=1; for(int i=0;i<t;i++) tot*=4;
            u32 Uaccum=0;
            for(int code=0;code<tot;code++){
                int c2=code; u32 adj[20]; for(int i=0;i<k;i++) adj[i]=0;
                for(int i=0;i<t;i++){
                    int st=c2&3; c2>>=2;
                    if(st==0||st==1){
                        int from=arcOf[i][0].first, to=arcOf[i][0].second, hv=headOf[i][0];
                        if((cur[to]>>nodes[nd.ch[to]].loc[hv])&1) adj[from]|=1u<<to;
                    }
                    if(st==0||st==2){
                        int from=arcOf[i][1].first, to=arcOf[i][1].second, hv=headOf[i][1];
                        if((cur[to]>>nodes[nd.ch[to]].loc[hv])&1) adj[from]|=1u<<to;
                    }
                }
                // transitive closure
                u32 rch[20]; for(int i=0;i<k;i++) rch[i]=adj[i]|(1u<<i);
                for(int i=0;i<k;i++) for(int j=0;j<k;j++) if(rch[i]>>j&1) rch[i]|=rch[j];
                u32 fullk=(k==32)?~0u:((1u<<k)-1);
                u32 good=0;
                for(int i=0;i<k;i++) if((rch[i]&fullk)==fullk) good|=1u<<i;
                u32 U=0; for(int i=0;i<k;i++) if(good>>i&1) U|=tr[i][cur[i]];
                Uaccum |= U; // NOTE: only correct for accumulation with weights; handled below
                (void)Uaccum;
                // add weight w * (1/4)^t to dist[U]
                ll wt = w % MOD;
                nd.dist[U] = (nd.dist[U] + wt) % MOD;
            }
            // multiply all added by (1/4)^t : instead, add w*(inv4)^t
            return;
        }
        for(u32 s=0;s<(1u<<nodes[nd.ch[ci]].verts.size());s++){
            if(!nodes[nd.ch[ci]].dist[s]) continue;
            cur[ci]=s;
            rec(ci+1, w*nodes[nd.ch[ci]].dist[s]%MOD);
        }
    };
    // Note: weights need the inv4^t factor; recompute properly below
    nd.dist.assign(1u<<nc,0);
    // redo with correct weighting: we will accumulate with factor inv4^t
    ll stateW = 1; for(int i=0;i<t;i++) stateW = stateW*inv4%MOD;
    vector<int> stateIdx(0);
    function<void(int,ll)> rec2 = [&](int ci, ll w){
        if(ci==k){
            int tot=1; for(int i=0;i<t;i++) tot*=4;
            for(int code=0;code<tot;code++){
                int c2=code; u32 adj[20]; for(int i=0;i<k;i++) adj[i]=0;
                for(int i=0;i<t;i++){
                    int st=c2&3; c2>>=2;
                    if(st==0||st==1){
                        int from=arcOf[i][0].first, to=arcOf[i][0].second, hv=headOf[i][0];
                        if((cur[to]>>nodes[nd.ch[to]].loc[hv])&1) adj[from]|=1u<<to;
                    }
                    if(st==0||st==2){
                        int from=arcOf[i][1].first, to=arcOf[i][1].second, hv=headOf[i][1];
                        if((cur[to]>>nodes[nd.ch[to]].loc[hv])&1) adj[from]|=1u<<to;
                    }
                }
                u32 rch[20]; for(int i=0;i<k;i++) rch[i]=adj[i]|(1u<<i);
                for(int i=0;i<k;i++) for(int j=0;j<k;j++) if(rch[i]>>j&1) rch[i]|=rch[j];
                u32 fullk=(k==32)?~0u:((1u<<k)-1);
                u32 good=0;
                for(int i=0;i<k;i++) if((rch[i]&fullk)==fullk) good|=1u<<i;
                u32 U=0; for(int i=0;i<k;i++) if(good>>i&1) U|=tr[i][cur[i]];
                ll wt = w%MOD*stateW%MOD;
                nd.dist[U]=(nd.dist[U]+wt)%MOD;
            }
            return;
        }
        for(u32 s=0;s<(1u<<nodes[nd.ch[ci]].verts.size());s++){
            if(!nodes[nd.ch[ci]].dist[s]) continue;
            cur[ci]=s;
            rec2(ci+1, w*nodes[nd.ch[ci]].dist[s]%MOD);
        }
    };
    rec2(0,1);
    (void)rec;
}

int main(){
    inv2 = pw(2,MOD-2);
    inv4 = inv2*inv2%MOD;
    int c,T; if(scanf("%d %d",&c,&T)!=2) return 0;
    while(T--){
        scanf("%d %d",&n,&m);
        es.clear(); es.resize(m);
        for(int i=0;i<m;i++){ scanf("%d %d %d",&es[i].u,&es[i].v,&es[i].w); es[i].u--; es[i].v--; }
        nodes.clear();
        build_hierarchy();
        // process nodes in order (children before parents since created later)
        for(int i=0;i<(int)nodes.size();i++) solve_node(i);
        // find root
        int root=-1;
        for(int i=0;i<(int)nodes.size();i++) if((int)nodes[i].verts.size()==n) root=i;
        ll ans;
        if(nodes[root].special_p0){
            // nonempty mass stored at full mask (for root, single-level case)
            ans = nodes[root].dist[(1<<nodes[root].verts.size())-1];
        } else {
            ll s=0;
            for(u32 mask=1; mask<nodes[root].dist.size(); mask++) s=(s+nodes[root].dist[mask])%MOD;
            ans=s;
        }
        printf("%lld\n", ans);
    }
    return 0;
}

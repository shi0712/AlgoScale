#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
static chrono::steady_clock::time_point T_START;
static double elapsedMs(){ return chrono::duration<double,milli>(chrono::steady_clock::now()-T_START).count(); }
static u64 mix64(u64 x){ x+=0x9E3779B97F4A7C15ULL; x=(x^(x>>30))*0xBF58476D1CE4E5B9ULL; x=(x^(x>>27))*0x94D049BB133111EBULL; return x^(x>>31);}

struct Tree {
    int n; vector<vector<int>> adj;
    // LCA
    vector<int> dep, first, par; vector<vector<int>> sp; vector<int> lg;
    void buildLCA(){
        dep.assign(n+1,0); first.assign(n+1,0); par.assign(n+1,0); vector<int> eul;
        vector<int> it(n+1,0); vector<int> st{1}; eul.push_back(1);
        while(!st.empty()){int v=st.back(); if(it[v]<(int)adj[v].size()){int w=adj[v][it[v]++]; if(w==par[v]) continue; par[w]=v; dep[w]=dep[v]+1; first[w]=eul.size(); eul.push_back(w); st.push_back(w);} else {st.pop_back(); if(!st.empty()) eul.push_back(st.back());}}
        int E=eul.size(); lg.assign(E+1,0); for(int i=2;i<=E;i++) lg[i]=lg[i/2]+1;
        sp.assign(lg[E]+1, vector<int>(E)); sp[0]=eul;
        for(int j=1;j<=lg[E];j++) for(int i=0;i+(1<<j)<=E;i++){int a=sp[j-1][i],b=sp[j-1][i+(1<<(j-1))]; sp[j][i]=dep[a]<dep[b]?a:b;}
    }
    inline int lca(int a,int b) const {int l=first[a],r=first[b]; if(l>r) swap(l,r); int j=lg[r-l+1]; int x=sp[j][l],y=sp[j][r-(1<<j)+1]; return dep[x]<dep[y]?x:y;}
    inline int dist(int a,int b) const {return dep[a]+dep[b]-2*dep[lca(a,b)];}
    vector<int> bfsDist(int u) const {
        vector<int> d(n+1,-1); vector<int> q; q.reserve(n); q.push_back(u); d[u]=0;
        for(size_t i=0;i<q.size();i++){int v=q[i]; for(int w:adj[v]) if(d[w]<0){d[w]=d[v]+1;q.push_back(w);}}
        return d;
    }
};

static int ceilLog(long long p){int d=0; while((1LL<<d)<p) d++; return d;}

template<int W>
struct Exact {
    struct BS{ u64 w[W]; bool operator==(const BS&o)const{return memcmp(w,o.w,sizeof w)==0;} };
    struct BH{ size_t operator()(const BS&b)const{ u64 h=0; for(int i=0;i<W;i++) h=mix64(h^b.w[i])+i; return h;} };
    static int pc(const BS&b){int s=0; for(int i=0;i<W;i++) s+=__builtin_popcountll(b.w[i]); return s;}
    static BS band(const BS&a,const BS&b){BS r; for(int i=0;i<W;i++) r.w[i]=a.w[i]&b.w[i]; return r;}
    static BS bandn(const BS&a,const BS&b){BS r; for(int i=0;i<W;i++) r.w[i]=a.w[i]&~b.w[i]; return r;}
    static bool bless(const BS&a,const BS&b){ for(int i=W-1;i>=0;i--) if(a.w[i]!=b.w[i]) return a.w[i]<b.w[i]; return false;}

    const Tree& T; int m; vector<int> idxV;
    vector<BS> F; vector<int> Fu,Fk;
    unordered_map<BS,int,BH> memoBall[64];
    size_t memoCount=0;
    long long work=0, WORKLIM=0; bool timeout=false;
    mt19937_64 rng{987654321};
    Exact(const Tree&t):T(t){}

    // returns false if family too large
    bool buildFamily(const vector<int>&S, size_t capWords){
        m=S.size(); idxV=S; int n=T.n;
        vector<u64> zob(m); for(auto&z:zob) z=rng();
        int L=idxV[0]; for(int x:idxV) L=T.lca(L,x);
        vector<char> mk(n+1,0); vector<int> stv; mk[L]=1; stv.push_back(L);
        for(int x:idxV){int v=x; while(!mk[v]){mk[v]=1; stv.push_back(v); v=T.par[v];}}
        unordered_set<u64> seen; F.clear(); Fu.clear(); Fk.clear();
        vector<int> dd(m), ord(m), cnt;
        for(int v:stv){
            int mx=0; for(int i=0;i<m;i++){dd[i]=T.dist(v,idxV[i]); if(dd[i]>mx) mx=dd[i];}
            cnt.assign(mx+2,0); for(int i=0;i<m;i++) cnt[dd[i]+1]++; for(int i=1;i<=mx+1;i++) cnt[i]+=cnt[i-1];
            for(int i=0;i<m;i++) ord[cnt[dd[i]]++]=i;
            u64 h=0;
            for(int i=0;i<m-1;i++){ h^=zob[ord[i]]; if(dd[ord[i+1]]==dd[ord[i]]) continue;
                if(seen.insert(h).second){
                    if((F.size()+1)*W>capWords) return false;
                    BS b; memset(b.w,0,sizeof b.w); for(int j=0;j<=i;j++) b.w[ord[j]>>6]|=1ULL<<(ord[j]&63);
                    F.push_back(b); Fu.push_back(v); Fk.push_back(dd[ord[i]]);} }
        }
        return true;
    }
    vector<int> filt(const BS&mask,int p,const vector<int>&U){
        vector<int> R; R.reserve(U.size()); unordered_set<BS,BH> sn; sn.reserve(U.size()*2);
        for(int i:U){ BS a=band(F[i],mask); int pa=pc(a); if(pa==0||pa==p) continue;
            BS c=bandn(mask,F[i]); if(!sn.insert(bless(a,c)?a:c).second) continue; R.push_back(i);}
        work+=(long long)U.size()*(W+4)+R.size()*(W+8)+50;
        if(work>WORKLIM) timeout=true;
        return R;
    }
    int quick2(const BS&mask,const vector<int>&U){ for(int i:U){int pa=pc(band(F[i],mask)); if(pa==1) return i;} return -1; }
    // -2 trivially solved, -1 fail, >=0 ball index
    int solve(const BS& mask,int d,const vector<int>&Upar){
        int p=pc(mask);
        if(p<=1) return -2;
        if(d<=0 || p>(1LL<<min(d,40))) return -1;
        if(p==2) return quick2(mask,Upar);
        auto it=memoBall[d].find(mask); if(it!=memoBall[d].end()) return it->second;
        if(timeout) return -1;
        vector<int> U=filt(mask,p,Upar);
        long long lim=1LL<<(d-1);
        vector<pair<int,int>> cand;
        for(int i:U){ int pa=pc(band(F[i],mask)); if(pa>lim||p-pa>lim) continue; cand.push_back({abs(2*pa-p),i}); }
        sort(cand.begin(),cand.end());
        int res=-1;
        for(auto&c:cand){
            if(timeout) break;
            if(solve(band(F[c.second],mask),d-1,U)==-1) continue;
            if(solve(bandn(mask,F[c.second]),d-1,U)==-1) continue;
            res=c.second; break;
        }
        if(!timeout||res!=-1){ memoBall[d][mask]=res; if(++memoCount>MEMOCAP) timeout=true; }
        return res;
    }
    size_t MEMOCAP=400000;
    // run interaction; returns final vertex or -1 on read failure
    template<class Ask> int run(Ask&& ask, double deadlineMs){
        BS mask; memset(mask.w,0,sizeof mask.w); for(int i=0;i<m;i++) mask.w[i>>6]|=1ULL<<(i&63);
        vector<int> ALL(F.size()); iota(ALL.begin(),ALL.end(),0);
        while(pc(mask)>1){
            int p=pc(mask);
            int d=ceilLog(p); int bi=-1;
            for(int tries=0;tries<6;tries++,d++){
                double rem=deadlineMs-elapsedMs();
                if(rem<=0) break;
                timeout=false; work=0; WORKLIM=(long long)(min(rem*0.5,250.0)*1.5e5);
                if(memoCount>MEMOCAP){ for(auto&mm:memoBall) mm.clear(); memoCount=0; }
                bi=solve(mask,d,ALL);
                if(bi>=0) break;
            }
            if(bi<0){
                int bs=INT_MAX; for(size_t i=0;i<F.size();i++){int pa=pc(band(F[i],mask)); if(pa==0||pa==p) continue; int sc=max(pa,p-pa); if(sc<bs){bs=sc;bi=i;}}
            }
            int r=ask(Fu[bi],Fk[bi]);
            if(r<0) return -1;
            mask = r? band(mask,F[bi]) : bandn(mask,F[bi]);
        }
        for(int i=0;i<m;i++) if(mask.w[i>>6]>>(i&63)&1) return idxV[i];
        return -1;
    }
};

struct Solver {
    Tree T; int n;
    vector<vector<int>> ancC, ancD;
    vector<int> offA, lenA, offB, lenB;
    vector<int> PA, PB, totA, totB;
    vector<int> S;
    mt19937 rng{12345};
    double DEADLINE=600; int MAXM=2048; size_t CAPW=2500000;

    void buildCD(){
        auto&adj=T.adj;
        vector<char> rem(n+1,0); vector<int> sz(n+1), par(n+1), dist(n+1);
        ancC.assign(n+1,{}); ancD.assign(n+1,{});
        offA.assign(n+1,0); lenA.assign(n+1,0); offB.assign(n+1,0); lenB.assign(n+1,0);
        vector<pair<int,int>> st; st.push_back({1,0});
        int totalA=0,totalB=0; vector<int> order;
        while(!st.empty()){
            auto [r,pc]=st.back(); st.pop_back();
            order.clear(); order.push_back(r); par[r]=0;
            for(size_t i=0;i<order.size();i++){int v=order[i]; for(int w:adj[v]) if(!rem[w]&&w!=par[v]){par[w]=v;order.push_back(w);}}
            for(int i=order.size()-1;i>=0;i--){int v=order[i]; sz[v]=1; for(int w:adj[v]) if(!rem[w]&&w!=par[v]) sz[v]+=sz[w];}
            int tot=order.size(); int c=r;
            while(true){int nx=-1; for(int w:adj[c]) if(!rem[w]&&w!=par[c]&&sz[w]*2>tot){nx=w;break;} if(nx<0)break; c=nx;}
            int mb=0; if(pc){ for(int v:order) mb=max(mb, ancD[v].back()); }
            lenB[c]=pc? mb+1:0; offB[c]=totalB; totalB+=lenB[c];
            order.clear(); order.push_back(c); par[c]=0; dist[c]=0;
            for(size_t i=0;i<order.size();i++){int v=order[i]; for(int w:adj[v]) if(!rem[w]&&w!=par[v]){par[w]=v;dist[w]=dist[v]+1;order.push_back(w);}}
            int ma=0; for(int v:order){ancC[v].push_back(c); ancD[v].push_back(dist[v]); ma=max(ma,dist[v]);}
            lenA[c]=ma+1; offA[c]=totalA; totalA+=lenA[c];
            rem[c]=1;
            for(int w:adj[c]) if(!rem[w]) st.push_back({w,c});
        }
        PA.assign(totalA,0); PB.assign(totalB,0); totA.assign(n+1,0); totB.assign(n+1,0);
    }
    void rebuildCounts(){
        fill(PA.begin(),PA.end(),0); fill(PB.begin(),PB.end(),0);
        fill(totA.begin(),totA.end(),0); fill(totB.begin(),totB.end(),0);
        for(int v:S){
            int L=ancC[v].size();
            for(int l=0;l<L;l++){int c=ancC[v][l]; PA[offA[c]+ancD[v][l]]++; totA[c]++;
                if(l+1<L){int c2=ancC[v][l+1]; PB[offB[c2]+ancD[v][l]]++; totB[c2]++;}}
        }
        for(int c=1;c<=n;c++){
            for(int i=1;i<lenA[c];i++) PA[offA[c]+i]+=PA[offA[c]+i-1];
            for(int i=1;i<lenB[c];i++) PB[offB[c]+i]+=PB[offB[c]+i-1];
        }
    }
    inline int f(int u,int k){
        int L=ancC[u].size(); int s=0;
        for(int l=0;l<L;l++){int c=ancC[u][l]; int dd=k-ancD[u][l];
            if(dd<0) continue;
            s+= dd>=lenA[c]? totA[c] : PA[offA[c]+dd];
            if(l+1<L){int c2=ancC[u][l+1]; s-= dd>=lenB[c2]? totB[c2] : PB[offB[c2]+dd];}
        }
        return s;
    }
    template<int W, class Ask> int tryExact(Ask&ask, bool &built){
        auto E=make_unique<Exact<W>>(T);
        built=E->buildFamily(S,CAPW);
        if(!built) return 0;
        return E->run(ask, DEADLINE);
    }
    template<class Ask> int run(Ask&& ask){
        n=T.n; T.buildLCA(); buildCD();
        S.resize(n); iota(S.begin(),S.end(),1);
        bool famFailed=false;
        while(S.size()>1){
            int m=S.size();
            if(m<=MAXM && !famFailed){
                bool built=false; int r;
                if(m<=64) r=tryExact<1>(ask,built);
                else if(m<=128) r=tryExact<2>(ask,built);
                else if(m<=256) r=tryExact<4>(ask,built);
                else if(m<=512) r=tryExact<8>(ask,built);
                else if(m<=1024) r=tryExact<16>(ask,built);
                else if(m<=2048) r=tryExact<32>(ask,built);
                else r=tryExact<64>(ask,built);
                if(built) return r;
                famFailed=true;
            }
            // greedy step
            rebuildCounts();
            int p=S.size(); int best=INT_MAX, bu=-1,bk=-1, cnt=0;
            for(int u=1;u<=n;u++){
                int lo=0,hi=n;
                while(lo<hi){int mid=(lo+hi)/2; if(2*f(u,mid)>=p) hi=mid; else lo=mid+1;}
                for(int k=lo-1;k<=lo;k++){ if(k<0) continue; int fv=f(u,k); if(fv==0||fv==p) continue;
                    int sc=max(fv,p-fv);
                    if(sc<best){best=sc;bu=u;bk=k;cnt=1;}
                    else if(sc==best){cnt++; if(rng()%cnt==0){bu=u;bk=k;}}
                }
            }
            int r=ask(bu,bk);
            if(r<0) return -1;
            auto d=T.bfsDist(bu);
            vector<int> NS; for(int x:S){ bool in=d[x]<=bk; if(in==(r==1)) NS.push_back(x); }
            S.swap(NS);
            famFailed=false;
        }
        return S[0];
    }
};
int main(){
    T_START=chrono::steady_clock::now();
    int n; if(scanf("%d",&n)!=1) return 0;
    Solver s; s.T.n=n; s.T.adj.assign(n+1,{});
    for(int i=2;i<=n;i++){int p; if(scanf("%d",&p)!=1) return 0; s.T.adj[i].push_back(p); s.T.adj[p].push_back(i);}
    auto ask=[&](int u,int k)->int{
        printf("? %d %d\n",u,k); fflush(stdout);
        int r; if(scanf("%d",&r)!=1) exit(0);
        if(r!=0&&r!=1) exit(0);
        return r;
    };
    int ans=s.run(ask);
    if(ans<1) return 0;
    printf("! %d\n",ans); fflush(stdout);
    return 0;
}

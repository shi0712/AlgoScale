#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll NEG = -(ll)4e18;

// Sparse representation of G_v: sorted vector of (level, delta>0),
// G(L) = sum of delta over entries with level <= L.
struct Map {
    vector<pair<int,ll>> v;
    void clear(){ v.clear(); v.shrink_to_fit(); }
};

static vector<pair<int,ll>> combine(const vector<Map*> &ms){
    size_t tot=0; for(auto m:ms) tot+=m->v.size();
    vector<pair<int,ll>> all; all.reserve(tot);
    for(auto m: ms) for(auto &e: m->v) all.push_back(e);
    sort(all.begin(), all.end());
    vector<pair<int,ll>> res; res.reserve(all.size());
    for(auto &e: all){
        if(!res.empty() && res.back().first==e.first) res.back().second += e.second;
        else res.push_back(e);
    }
    return res;
}

// Compute G_v increments from children's combined events (sorted).
static Map transform(const vector<pair<int,ll>> &events, ll w, ll p){
    Map out;
    ll S=0, H=NEG, Gprev=0;
    int prev=0;
    auto reg=[&](int a,int b){
        if(a>b) return;
        int cand=a;
        ll hv = S - (ll)a*p + ((a%2==0)?w:0);
        if(a+1<=b){
            ll hv2 = S - (ll)(a+1)*p + (((a+1)%2==0)?w:0);
            if(hv2>hv){ hv=hv2; cand=a+1; }
        }
        if(hv>H){
            H=hv;
            ll G=H-p; if(G<0) G=0;
            if(G>Gprev){ out.v.push_back({cand, G-Gprev}); Gprev=G; }
        }
    };
    size_t i=0;
    while(i<events.size()){
        int e=events[i].first;
        if(prev<=e-1) reg(prev,e-1);
        ll d=0;
        while(i<events.size() && events[i].first==e){ d+=events[i].second; ++i; }
        S+=d;
        reg(e,e);
        prev=e+1;
    }
    reg(prev,prev+1);
    return out;
}

struct PrefMap {
    vector<int> lv;
    vector<ll> pre;
    void build(const vector<pair<int,ll>> &ev){
        lv.clear(); pre.clear(); ll s=0;
        for(auto &e: ev){ lv.push_back(e.first); s+=e.second; pre.push_back(s); }
    }
    ll query(int L) const {
        int lo=0, hi=(int)lv.size();
        while(lo<hi){ int mid=(lo+hi)>>1; if(lv[mid]<=L) lo=mid+1; else hi=mid; }
        if(lo==0) return 0;
        return pre[lo-1];
    }
};

int main(){
    int n,s;
    if(scanf("%d %d",&n,&s)!=2) return 0;
    vector<ll> w(n+1),p(n+1); vector<int> a(n+1);
    for(int i=1;i<=n;i++) if(scanf("%lld",&w[i])!=1) return 0;
    for(int i=1;i<=n;i++) if(scanf("%lld",&p[i])!=1) return 0;
    for(int i=1;i<=n;i++) if(scanf("%d",&a[i])!=1) return 0;

    vector<vector<int>> rev(n+1);
    for(int i=1;i<=n;i++) rev[a[i]].push_back(i);

    vector<int> dist(n+1,-1), order; order.reserve(n);
    { queue<int> q; dist[s]=0; q.push(s);
      while(!q.empty()){ int v=q.front(); q.pop(); order.push_back(v);
        for(int u:rev[v]) if(dist[u]<0){ dist[u]=dist[v]+1; q.push(u); } } }

    vector<int> cyc;
    if(dist[a[s]]>=0){ int cur=s; do{ cyc.push_back(cur); cur=a[cur]; }while(cur!=s); }
    vector<char> onC(n+1,0); for(int v:cyc) onC[v]=1;

    ll ans=NEG;
    if(cyc.empty()){
        // s is not on a cycle: it can only stay wet (label 0) or be dried (label 1),
        // so every node's label is in {-1,0,1}.  Simple 2-state DP.
        vector<ll> dS(n+1,0), dU(n+1,0);
        for(int oi=(int)order.size()-1; oi>=0; --oi){
            int v=order[oi];
            ll sS=0, sU=0;
            for(int u: rev[v]){
                if(dS[u]>0) sS+=dS[u];
                ll m1=max(dU[u],dS[u]); if(m1>0) sU+=m1;
            }
            dS[v] = w[v]-p[v]+sS;
            dU[v] = -2*p[v]+sU;
        }
        ll sS=0, sU=0;
        for(int u: rev[s]){ if(dS[u]>0) sS+=dS[u]; ll m1=max(dU[u],dS[u]); if(m1>0) sU+=m1; }
        ans=max(w[s]+sS, -p[s]+sU);
        printf("%lld\n",ans);
        return 0;
    }

    // s is on a cycle: compute tree DP for non-cycle nodes of R (sparse label DP).
    vector<Map> dp(n+1);
    for(int oi=(int)order.size()-1; oi>=0; --oi){
        int v=order[oi];
        if(onC[v]) continue;
        vector<Map*> ch;
        for(int u: rev[v]) if(!onC[u]) ch.push_back(&dp[u]);
        auto ev = combine(ch);
        dp[v] = transform(ev, w[v], p[v]);
        for(int u: rev[v]) if(!onC[u]) dp[u].clear();
    }

    int m=(int)cyc.size();
    vector<int> e(m);
    for(int pp=0;pp<m;pp++) e[pp]=cyc[(m-pp)%m];

    vector<PrefMap> treeM(m);
    vector<int> maxLvl(m,0);
    for(int pp=0;pp<m;pp++){
        int v=e[pp];
        vector<Map*> ch;
        for(int u: rev[v]) if(!onC[u]) ch.push_back(&dp[u]);
        auto ev=combine(ch);
        treeM[pp].build(ev);
        if(!ev.empty()) maxLvl[pp]=ev.back().first;
        // nofree
    }
    auto Aval=[&](int pp,ll j)->ll{
        ll v = treeM[pp].query((int)j);
        if(pp==0) v += (j%2==0? w[e[pp]]:0) - j*p[e[pp]];
        else      v += (j%2==0? w[e[pp]]:0) - (j+1)*p[e[pp]];
        return v;
    };

    if(m==2){
        ll best=NEG;
        best=max(best, Aval(0,0));                  // s S, other never colored
        best=max(best, Aval(0,0) + Aval(1,0));      // both S
        best=max(best, Aval(0,1));                  // s dried, other never colored
        printf("%lld\n",best);
        return 0;
    }

    // Small-label case: chain labels in {-1,0,1}, non-increasing, s in {0,1}.
    for(int js=0; js<=1; ++js){
        ll sVal = Aval(0,js);
        ll f[3];
        for(int t=0;t<3;t++){
            int j=t-1;
            f[t] = (j<=js) ? (j==-1?0:Aval(1,j)) : NEG;
        }
        for(int pp=2;pp<m;pp++){
            ll g[3]={NEG,NEG,NEG};
            for(int tp=0;tp<3;tp++){
                if(f[tp]==NEG) continue;
                int jp=tp-1;
                for(int tc=0;tc<3;tc++){
                    int jc=tc-1; if(jc>jp) continue;
                    ll add=(jc==-1?0:Aval(pp,jc));
                    g[tc]=max(g[tc], f[tp]+add);
                }
            }
            for(int t=0;t<3;t++) f[t]=g[t];
        }
        for(int tc=0;tc<3;tc++){
            int jl=tc-1;
            if(f[tc]==NEG) continue;
            if(js > jl+2) continue;
            ans=max(ans, sVal + f[tc]);
        }
    }

    // High-label case: all cycle labels in [b,b+2], non-increasing along the chain.
    int Bmax=2;
    for(int pp=0;pp<m;pp++) Bmax=max(Bmax, maxLvl[pp]+3);
    for(int b=0;b<=Bmax;++b){
        ll f[3];
        for(int d=0;d<3;d++) f[d]=Aval(0,(ll)b+d);
        for(int pp=1;pp<m;pp++){
            ll g[3]={NEG,NEG,NEG};
            ll a0=Aval(pp,(ll)b), a1=Aval(pp,(ll)b+1), a2=Aval(pp,(ll)b+2);
            ll av[3]={a0,a1,a2};
            for(int tp=0;tp<3;tp++){
                if(f[tp]==NEG) continue;
                for(int tc=0;tc<=tp;tc++) g[tc]=max(g[tc], f[tp]+av[tc]);
            }
            for(int d=0;d<3;d++) f[d]=g[d];
        }
        for(int d=0;d<3;d++) ans=max(ans,f[d]);
    }

    printf("%lld\n",ans);
    return 0;
}

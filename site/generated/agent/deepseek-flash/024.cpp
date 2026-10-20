#include <bits/stdc++.h>
using namespace std;
static const int MOD = 1000000007;

static vector<long long> fact_, invfact_;
static vector<vector<int>> gp; // gp[s][j] for j=1..s

static inline long long pw(long long b, long long e){ long long r=1; b%=MOD; while(e){ if(e&1) r=r*b%MOD; b=b*b%MOD; e>>=1; } return r; }

// multiply two polynomials mod
static vector<int> mulpoly(const vector<int>&a, const vector<int>&b){
    if(a.empty()||b.empty()) return {};
    vector<int> c(a.size()+b.size()-1, 0);
    for(size_t i=0;i<a.size();i++){
        if(!a[i]) continue;
        long long ai=a[i];
        for(size_t j=0;j<b.size();j++){
            if(!b[j]) continue;
            c[i+j] = (c[i+j] + ai*b[j]) % MOD;
        }
    }
    return c;
}

// A-value: prod fact[s_i] * sum_B fact[B]*P[B]
static long long evalA(const vector<int>& P, long long prodFact){
    long long s=0;
    for(size_t b=0;b<P.size();b++){
        if(!P[b]) continue;
        s = (s + fact_[b]*P[b]) % MOD;
    }
    return s*prodFact%MOD;
}

int main(){
    int n;
    if(scanf("%d",&n)!=1) return 0;
    vector<vector<int>> adj(n);
    for(int i=0;i<n-1;i++){
        int u,v; scanf("%d %d",&u,&v); --u;--v;
        adj[u].push_back(v); adj[v].push_back(u);
    }
    vector<int> par(n,-1), order; order.reserve(n);
    {
        vector<int> st; st.push_back(0); par[0]=-2;
        while(!st.empty()){
            int x=st.back(); st.pop_back(); order.push_back(x);
            for(int y:adj[x]) if(par[y]==-1){ par[y]=x; st.push_back(y); }
        }
    }
    vector<int> sz(n,1);
    for(int i=n-1;i>=0;i--){ int x=order[i]; if(par[x]>=0) sz[par[x]]+=sz[x]; }

    fact_.assign(n+1,1); invfact_.assign(n+1,1);
    for(int i=1;i<=n;i++) fact_[i]=fact_[i-1]*i%MOD;
    invfact_[n]=pw(fact_[n],MOD-2);
    for(int i=n;i>=1;i--) invfact_[i-1]=invfact_[i]*i%MOD;

    gp.assign(n+1,{});
    for(int s=1;s<=n;s++){
        gp[s].assign(s+1,0);
        for(int j=1;j<=s;j++){
            long long c = fact_[s-1]*invfact_[j-1]%MOD*invfact_[s-j]%MOD;
            if((s-j)&1) c = (MOD-c)%MOD;
            c = c*invfact_[j]%MOD;
            gp[s][j] = (int)c;
        }
    }

    vector<long long> f(n,0);
    vector<int> ch;
    vector<vector<int>> suf;
    for(int oi=n-1; oi>=0; oi--){
        int x = order[oi];
        ch.clear();
        for(int y: adj[x]) if(par[y]==x) ch.push_back(y);
        int m = ch.size();
        suf.assign(m+2, {});
        suf[m+1] = vector<int>{1};
        for(int i=m;i>=0;i--){
            const vector<int>& p = (i==0)? gp[1] : gp[sz[ch[i-1]]];
            suf[i] = mulpoly(p, suf[i+1]);
        }
        long long pfAll = 1;
        for(int i=1;i<=m;i++) pfAll = pfAll*fact_[sz[ch[i-1]]]%MOD;
        long long tot = evalA(suf[0], pfAll); // all-d-zero case
        // prefix running product
        vector<int> pre = gp[1];
        for(int i=1;i<=m;i++){
            int y = ch[i-1];
            if(sz[y]>=2){
                vector<int> q = mulpoly(pre, suf[i+1]);
                long long pf = pfAll * invfact_[sz[y]] % MOD;
                long long ae = evalA(q, pf);
                tot = (tot + ae*f[y]) % MOD;
            }
            pre = mulpoly(pre, gp[sz[y]]);
        }
        f[x] = tot;
    }
    printf("%lld\n", f[0]);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const int MOD=1000000007;

/*
For each prime, the generating function of tau(p^(a+b+c)) is

 (1 - xy - xz - yz + 2xyz) / ((1-x)^2 (1-y)^2 (1-z)^2).

Consequently the answer is the sum of
  2^omega(t) mu(u) mu(v) mu(w)
    D(A/(tuv)) D(B/(tuw)) D(C/(tvw)),
where t,u,v,w are squarefree and pairwise coprime, and D is the
summatory divisor function.

Sort u,v,w. Except for repeated 1's, they are strictly increasing;
the six assignments of the bounds account for their permutations.
For fixed t,u,v, group w by the two constant integer quotients.
The remaining range sum of mu(w), restricted to gcd(w,tuv)=1,
is evaluated using bitsets and popcount.
*/
struct Solver {
    int N, W;
    vector<int> spf, mu, omega, D, primeIndex;
    vector<uint64_t> positive, negative, masks;
    vector<int> primes;
    struct Filter { int k=0; const uint64_t* p[8]; };
    Solver(int n):N(n),W((n+64)/64),spf(n+1),mu(n+1),omega(n+1),D(n+1),primeIndex(n+1,-1),positive(W),negative(W) {
        mu[1]=1;
        vector<int> tau(n+1), exponent(n+1); tau[1]=1;
        for(int i=2;i<=N;i++) {
            if(!spf[i]) { spf[i]=i; primes.push_back(i); mu[i]=-1; omega[i]=1; tau[i]=2; exponent[i]=1; }
            for(int p:primes) {
                if(p>spf[i] || (ll)i*p>N) break;
                int x=i*p; spf[x]=p;
                if(p==spf[i]) { mu[x]=0; omega[x]=omega[i]; exponent[x]=exponent[i]+1; tau[x]=tau[i]/(exponent[i]+1)*(exponent[x]+1); break; }
                mu[x]=-mu[i]; omega[x]=omega[i]+1; exponent[x]=1; tau[x]=tau[i]*2;
            }
        }
        for(int i=1;i<=N;i++) {
            D[i]=D[i-1]+tau[i];
            if(mu[i]==1) positive[i>>6]|=1ULL<<(i&63);
            if(mu[i]==-1) negative[i>>6]|=1ULL<<(i&63);
        }
        int pc=0; for(int p:primes) if((ll)p*p<=N) primeIndex[p]=pc++;
        masks.resize((size_t)pc*W);
        for(int p:primes) {
            if((ll)p*p>N) break;
            uint64_t* a=masks.data()+(size_t)primeIndex[p]*W;
            for(int i=p;i<=N;i+=p) a[i>>6]|=1ULL<<(i&63);
        }
    }
    Filter filter(int q) const {
        Filter f;
        while(q>1) {
            int p=spf[q]; q/=p;
            // A prime > sqrt(N) in t forces w <= N/t < p.
            // Neither u nor v can contain such a prime.
            if(primeIndex[p]>=0) f.p[f.k++]=masks.data()+(size_t)primeIndex[p]*W;
        }
        return f;
    }
    inline int wordSum(int w, uint64_t bits, const Filter& f) const {
        uint64_t bad=0;
        for(int i=0;i<f.k;i++) bad|=f.p[i][w];
        bits &= ~bad;
        return __builtin_popcountll(positive[w]&bits)-__builtin_popcountll(negative[w]&bits);
    }
    inline int rangeSum(int l,int r,const Filter& f) const {
        int a=l>>6,b=r>>6;
        uint64_t lm=~0ULL<<(l&63), rm=~0ULL>>(63-(r&63));
        if(a==b) return wordSum(a,lm&rm,f);
        int ans=wordSum(a,lm,f)+wordSum(b,rm,f);
        for(int w=a+1;w<b;w++) ans+=wordSum(w,~0ULL,f);
        return ans;
    }
    ll inner(int P,int Q,int low,const Filter& f) const {
        int hi=min(P,Q);
        ll ans=0;
        for(int l=low;l<=hi;) {
            int a=P/l,b=Q/l;
            int r=min(P/a,Q/b);
            int s=rangeSum(l,r,f);
            ans+=(ll)s*D[a]*D[b];
            l=r+1;
        }
        return ans;
    }
    int solve(int A,int B,int C) const {
        array<int,3> bounds={A,B,C}; sort(bounds.begin(),bounds.end());
        A=bounds[0]; B=bounds[1]; C=bounds[2];
        vector<pair<array<int,3>,int>> perms;
        // Each distinct assignment occurs with the multiplicity of equal bounds.
        int mult=(A==B&&B==C)?6:((A==B||B==C)?2:1);
        do {
            perms.push_back({bounds,mult});
        } while(next_permutation(bounds.begin(),bounds.end()));
        ll ans=0;
        for(int t=1;t<=A;t++) if(mu[t]) {
            int weight=1<<omega[t];
            // u = v = w = 1.
            ll base=(ll)D[A/t]*D[B/t]%MOD*D[C/t]%MOD;
            ans=(ans+base*weight)%MOD;
            Filter ft=filter(t);
            // u = v = 1, w > 1: choose the coordinate equal to t.
            ll sum=0;
            for(int i=0;i<3;i++) {
                int x=bounds[i], y=bounds[(i+1)%3], z=bounds[(i+2)%3];
                ll val=inner(y/t,z/t,2,ft)%MOD;
                sum=(sum+val*D[x/t])%MOD;
            }
            ans=(ans+sum*weight)%MOD;
            // u < v < w. Necessary bounds: tuv <= A, tv(v+1) <= C.
            for(int v=2;(ll)t*v*(v+1)<=C;v++) if(mu[v] && gcd(t,v)==1) {
                int tv=t*v;
                int umax=min(v-1,A/tv);
                for(int u=1;u<=umax;u++) if(mu[u] && gcd(tv,u)==1) {
                    int q=tv*u;
                    Filter f=filter(q);
                    ll cur=0;
                    for(auto &pr:perms) {
                        auto p=pr.first;
                        if(q>p[0]) continue;
                        int P=p[1]/(t*u),Q=p[2]/tv;
                        if(min(P,Q)<=v) continue;
                        ll val=inner(P,Q,v+1,f)%MOD;
                        cur=(cur+val*D[p[0]/q]%MOD*pr.second)%MOD;
                    }
                    ans=(ans+cur*(weight*mu[u]*mu[v]))%MOD;
                }
            }
        }
        if(ans<0) ans+=MOD;
        return ans;
    }
};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int T;cin>>T;vector<array<int,3>>cases(T);int n=1;for(auto &a:cases)for(int &x:a){cin>>x;n=max(n,x);}Solver solver(n);for(auto a:cases)cout<<solver.solve(a[0],a[1],a[2])<<'\n';}

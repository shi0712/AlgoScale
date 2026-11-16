#include <bits/stdc++.h>
using namespace std;
using ll = long long;
static const ll MOD = 998244353;

ll pw(ll b, ll e){ ll r=1; b%=MOD; while(e){ if(e&1) r=r*b%MOD; b=b*b%MOD; e>>=1;} return r; }

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    if(!(cin >> n >> m)) return 0;
    vector<int> a(m), b(m);
    for(auto &x : a) cin >> x;
    for(auto &x : b) cin >> x;
    ll B = 0; for(int i=0;i<m;i++) B += b[i];

    int N = n + 2;
    vector<ll> inv(N+1), fact(N+1), ifact(N+1), H(N+1);
    inv[1]=1;
    for(int i=2;i<=N;i++) inv[i] = (MOD - (MOD/i) * inv[MOD%i] % MOD) % MOD;
    fact[0]=1; for(int i=1;i<=N;i++) fact[i]=fact[i-1]*i%MOD;
    ifact[N]=pw(fact[N],MOD-2); for(int i=N;i>=1;i--) ifact[i-1]=ifact[i]*i%MOD;
    H[0]=0; for(int i=1;i<=N;i++) H[i]=(H[i-1]+inv[i])%MOD;
    auto C=[&](int nn,int kk)->ll{
        if(kk<0||kk>nn||nn<0) return 0;
        return fact[nn]*ifact[kk]%MOD*ifact[nn-kk]%MOD;
    };

    // product polynomial P(u) = prod F_i(u)
    vector<ll> P(1,1);
    for(int i=0;i<m;i++){
        if(b[i]==0) continue;                 // F_i == 1
        int ai=a[i], bi=b[i], di=ai-bi;
        vector<ll> F(ai+1, 0);
        ll coef = (ll)bi % MOD * C(ai,bi) % MOD;
        for(int j=0;j<=di;j++){
            ll t = coef * C(di,j) % MOD * inv[bi+j] % MOD;
            if(j&1) t = (MOD - t)%MOD;
            F[bi+j] = t;
        }
        vector<ll> np(P.size()+F.size()-1, 0);
        for(size_t x=0;x<P.size();x++){
            ll px = P[x];
            if(!px) continue;
            for(size_t y=0;y<F.size();y++){
                if(!F[y]) continue;
                np[x+y] = (np[x+y] + px*F[y])%MOD;
            }
        }
        P.swap(np);
    }

    // E[max] = sum_{k} (1 - sum_{j<=k} p_j) / (k+1)
    ll Emax = 0;
    ll s = 1;
    int D = (int)P.size()-1;
    for(int k=0;k<D;k++){
        s = (s - P[k] + MOD)%MOD;
        Emax = (Emax + s * inv[k+1])%MOD;
    }

    ll ans = B % MOD;
    for(int i=0;i<m;i++){
        ll Etau = (H[a[i]] - H[a[i]-b[i]] + MOD)%MOD;
        ll d = a[i]-b[i];
        if(d){
            ll diff = (Emax - Etau + MOD)%MOD;
            ans = (ans + (d % MOD) * diff)%MOD;
        }
    }
    cout << ans << "\n";
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int main(){
    int n,m; if(scanf("%d %d",&n,&m)!=2) return 0;
    vector<int> t(n); for(auto&x:t) scanf("%d",&x);
    vector<vector<ll>> a(n,vector<ll>(m)), d(n,vector<ll>(m));
    for(auto&r:a) for(auto&x:r) scanf("%lld",&x);
    for(auto&r:d) for(auto&x:r) scanf("%lld",&x);
    int F=1<<m;
    vector<ll> A(F),D(F),G(F),val(F,0);
    vector<char> bad(F,0);
    for(int s=0;s<n;s++){
        A[0]=D[0]=G[0]=0;
        const ll* as=a[s].data(); const ll* ds=d[s].data();
        for(int S=1;S<F;S++){
            int l=__builtin_ctz(S); int P=S&(S-1);
            A[S]=as[l]+A[P]; D[S]=ds[l]+D[P];
            G[S]=max(G[P],A[S]-D[P]);
        }
        if(t[s]){
            for(int S=1;S<F;S++){ if(G[S]>0) bad[S]=1; else val[S]+=D[S]-A[S]; }
        } else {
            for(int S=1;S<F;S++) val[S]-=G[S];
        }
    }
    int cover=0;
    for(int S=1;S<F;S++) if(!bad[S]&&val[S]>=0) cover|=S;
    ll ans=0;
    for(int r=0;r<m;r++) if(!(cover>>r&1)) for(int s=0;s<n;s++) ans+=a[s][r];
    printf("%lld\n",ans);
}

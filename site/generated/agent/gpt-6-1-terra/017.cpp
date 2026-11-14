#include <bits/stdc++.h>
using namespace std;
using ll = long long;
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n,m; cin>>n>>m;
    vector<int> tr(n); for(int &x:tr) cin>>x;
    vector<vector<ll>> a(n,vector<ll>(m)),d(n,vector<ll>(m));
    vector<ll> rankA(m), trumpDelta(m);
    for(int i=0;i<n;i++) for(int j=0;j<m;j++){cin>>a[i][j]; rankA[j]+=a[i][j];}
    for(int i=0;i<n;i++) for(int j=0;j<m;j++) cin>>d[i][j];
    int N=1<<m;
    vector<ll> diff(N), deficit(N), regular(N,0);
    vector<unsigned char> bad(N,0);
    for(int i=0;i<n;i++){
        if(tr[i]) for(int j=0;j<m;j++) trumpDelta[j]+=d[i][j]-a[i][j];
        for(int mask=1;mask<N;mask++){
            int bit=__builtin_ctz((unsigned)mask);
            int rest=mask&(mask-1);
            diff[mask]=diff[rest]+a[i][bit]-d[i][bit];
            deficit[mask]=max(deficit[rest],diff[rest]+a[i][bit]);
            if(tr[i]) bad[mask] |= deficit[mask]>0;
            else regular[mask]+=deficit[mask];
        }
    }
    diff[0]=0;
    int safe=0;
    for(int mask=1;mask<N;mask++){
        int bit=__builtin_ctz((unsigned)mask);
        diff[mask]=diff[mask&(mask-1)]+trumpDelta[bit];
        if(!bad[mask] && regular[mask]<=diff[mask]) safe|=mask;
    }
    ll ans=0;
    for(int j=0;j<m;j++) if(!(safe>>j&1)) ans+=rankA[j];
    cout<<ans<<'\n';
}

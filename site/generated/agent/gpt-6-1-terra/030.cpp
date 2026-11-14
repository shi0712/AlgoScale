#include <bits/stdc++.h>
using namespace std;
using int64 = long long;
const int MOD = 998244353;
int modpow(int a, int e) { int64 r=1; for (;e;e>>=1,a=(int64)a*a%MOD) if(e&1) r=r*a%MOD; return r; }
int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    int n,m; cin>>n>>m;
    vector<int>a(m),b(m); for(int &x:a) cin>>x; for(int &x:b) cin>>x;
    int B=accumulate(b.begin(),b.end(),0), C=n-B;
    if(C==0 || B==0){cout<<B<<'\n'; return 0;}
    vector<int> fact(n+1), ifact(n+1), inv(n+1), H(n+1);
    fact[0]=1; for(int i=1;i<=n;i++) fact[i]=(int64)fact[i-1]*i%MOD;
    ifact[n]=modpow(fact[n],MOD-2); for(int i=n;i>0;i--) ifact[i-1]=(int64)ifact[i]*i%MOD;
    inv[1]=1; for(int i=2;i<=n;i++) inv[i]=MOD-(int64)(MOD/i)*inv[MOD%i]%MOD;
    for(int i=1;i<=n;i++) {H[i]=H[i-1]+inv[i]; if(H[i]>=MOD) H[i]-=MOD;}
    auto choose=[&](int x,int y)->int {return (int64)fact[x]*ifact[y]%MOD*ifact[x-y]%MOD;};
    vector<int> poly(1,1);
    int sub=0;
    for(int i=0;i<m;i++){
        int ai=a[i], bi=b[i], r=ai-bi;
        int h=H[ai]-H[r]; if(h<0) h+=MOD;
        sub=(sub+(int64)r*h)%MOD;
        if(!bi) continue;
        vector<int> f(ai+1); f[0]=1;
        for(int k=r+1;k<=ai;k++){
            int v=(int64)choose(ai,k)*choose(k-1,r)%MOD;
            if((k-r)&1) v=MOD-v;
            f[k]=v;
        }
        vector<int> next(poly.size()+ai);
        for(int j=0;j<(int)poly.size();j++){
            next[j]+=poly[j]; if(next[j]>=MOD) next[j]-=MOD;
            for(int k=r+1;k<=ai;k++) next[j+k]=(next[j+k]+(int64)poly[j]*f[k])%MOD;
        }
        poly.swap(next);
    }
    int et=0;
    for(int k=1;k<(int)poly.size();k++) et=(et+(int64)poly[k]*inv[k])%MOD;
    if(et) et=MOD-et;
    int ans=(B+(int64)C*et-sub)%MOD; if(ans<0) ans+=MOD;
    cout<<ans<<'\n';
}

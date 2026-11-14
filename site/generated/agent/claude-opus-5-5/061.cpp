#include <bits/stdc++.h>
using namespace std;
typedef long long ll; const ll MOD=998244353;
ll pw(ll b,ll e){b%=MOD;if(b<0)b+=MOD;ll r=1;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
ll F[8105],IF[8105];
ll C(int n,int k){if(k<0||k>n||n<0)return 0;return F[n]*IF[k]%MOD*IF[n-k]%MOD;}
int main(){
    F[0]=1;for(int i=1;i<8105;i++)F[i]=F[i-1]*i%MOD;
    IF[8104]=pw(F[8104],MOD-2);for(int i=8104;i>0;i--)IF[i-1]=IF[i]*i%MOD;
    int T;scanf("%d",&T);
    while(T--){
        int n,m;scanf("%d %d",&n,&m);int N=2*m;
        vector<int>a(n);bool one=false;for(auto&x:a){scanf("%d",&x);if(x==N)one=true;}
        if(one){puts("0");continue;}
        if(m==1){puts("1");continue;}
        // d[r][s]: (2j)^{fall r} = sum d[r][s] j^{fall s}
        vector<vector<ll>> d(m+1,vector<ll>(m+1,0));
        d[0][0]=1;
        for(int r=0;r<m;r++)for(int s=0;s<=r;s++){
            ll v=d[r][s];if(!v)continue;
            d[r+1][s+1]=(d[r+1][s+1]+2*v)%MOD;
            d[r+1][s]=(d[r+1][s]+((2*s-r)%MOD+MOD)%MOD*v)%MOD;
        }
        // coef[r] = m^{fall r}/N^{fall r}
        vector<ll> co(m+1);co[0]=1;
        for(int r=1;r<=m;r++)co[r]=co[r-1]*(m-r+1)%MOD*pw(N-r+1,MOD-2)%MOD;
        vector<ll> b(m+1,0);b[m]=1;
        for(int s=m-1;s>=2;s--){
            ll sum=0;
            for(int r=s+1;r<=m;r++)sum=(sum+b[r]*co[r]%MOD*d[r][s])%MOD;
            ll css=co[s]*d[s][s]%MOD;
            b[s]=sum*pw((1-css+MOD)%MOD,MOD-2)%MOD;
        }
        ll S1=0;for(int r=2;r<=m;r++)S1=(S1+b[r]*co[r]%MOD*d[r][1])%MOD;
        ll sc=(MOD-pw(m,MOD-2))%MOD*pw(S1,MOD-2)%MOD;
        for(auto&x:b)x=x*sc%MOD;
        vector<ll> g(m+1,0);
        for(int k=0;k<=m;k++){
            ll s=0,ff=1;
            for(int r=0;r<=k;r++){s=(s+b[r]*ff)%MOD;ff=ff*(k-r)%MOD;}
            g[k]=s;
        }
        ll invCN=pw(C(N,m),MOD-2),invN=pw(N,MOD-2);
        map<int,ll> memo;
        auto f=[&](int x)->ll{
            if(x%2==0)return g[x/2];
            auto it=memo.find(x);if(it!=memo.end())return it->second;
            ll s=0;for(int k=0;k<=min(x,m);k++)s=(s+C(x,k)*C(N-x,m-k)%MOD*g[k])%MOD;
            s=(s*invCN+(ll)x*invN)%MOD;memo[x]=s;return s;
        };
        ll ans=0;for(int x:a)ans=(ans+f(x))%MOD;
        ans=(ans-g[m]+MOD)%MOD;
        printf("%lld\n",ans);
    }
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll MOD=998244353;
ll pw(ll b,ll e){ll r=1;b%=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
const int MX=4005;
ll fact[MX],ifact[MX],p2[MX];
inline ll C(int n,int k){if(k<0||n<0||k>n)return 0;return fact[n]*ifact[k]%MOD*ifact[n-k]%MOD;}
int main(){
    fact[0]=1;for(int i=1;i<MX;i++)fact[i]=fact[i-1]*i%MOD;
    ifact[MX-1]=pw(fact[MX-1],MOD-2);
    for(int i=MX-1;i>=1;i--)ifact[i-1]=ifact[i]*i%MOD;
    p2[0]=1;for(int i=1;i<MX;i++)p2[i]=p2[i-1]*2%MOD;
    int T;if(scanf("%d",&T)!=1)return 0;
    while(T--){
        int n,m;scanf("%d %d",&n,&m);
        vector<int>a(n);for(int i=0;i<n;i++)scanf("%d",&a[i]);
        int N=2*m;
        if(n==1){puts("0");continue;}
        if(m==1){puts("1");continue;}
        vector<ll>q(m+1,0),al(m+1,0),be(m+1,0);
        for(int i=0;i<n;i++){
            int hi=min(a[i],m);
            for(int k=2;k<=hi;k++){q[k]+=C(a[i],k);if(q[k]>=MOD)q[k]-=MOD;}
        }
        for(int k=1;k<=m;k++)
            al[k]=fact[m]*ifact[m-k]%MOD*fact[N-k]%MOD*ifact[N]%MOD;
        be[m]=1;
        for(int j=m-1;j>=2;j--){
            ll s=0;int hi=min(2*j,m);
            for(int k=j+1;k<=hi;k++){
                ll t=be[k]*al[k]%MOD*C(j,k-j)%MOD*p2[2*j-k]%MOD;
                s+=t;if(s>=MOD)s-=MOD;
            }
            ll den=(p2[j]*al[j]%MOD+MOD-1)%MOD;
            be[j]=(MOD-s)%MOD*pw(den,MOD-2)%MOD;
        }
        ll g1=al[2]*be[2]%MOD;
        ll sc=(MOD-pw((ll)m*g1%MOD,MOD-2))%MOD;   // -1/(m*gamma_1)
        ll ans=1;
        for(int k=2;k<=m;k++){
            ll b=be[k]*sc%MOD;
            ll t=(al[k]*q[k]%MOD+MOD-C(m,k))%MOD;
            ans=(ans+b*t)%MOD;
        }
        printf("%lld\n",ans);
    }
    return 0;
}

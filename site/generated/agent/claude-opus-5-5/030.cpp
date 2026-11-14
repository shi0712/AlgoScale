#include <bits/stdc++.h>
using namespace std;
typedef long long ll; const ll MOD=998244353;
ll pw(ll b,ll e){ll r=1;b%=MOD;if(b<0)b+=MOD;while(e){if(e&1)r=r*b%MOD;b=b*b%MOD;e>>=1;}return r;}
int main(){int n,m;cin>>n>>m;vector<int>a(m),b(m);for(auto&x:a)cin>>x;for(auto&x:b)cin>>x;
 vector<vector<ll>> C(n+1,vector<ll>(n+1,0));for(int i=0;i<=n;i++){C[i][0]=1;for(int j=1;j<=i;j++)C[i][j]=(C[i-1][j-1]+C[i-1][j])%MOD;}
 vector<ll> inv(n+2,1);for(int i=2;i<=n+1;i++)inv[i]=(MOD-(MOD/i)*inv[MOD%i]%MOD)%MOD;
 ll ans=0,D=0;
 for(int j=0;j<m;j++){for(int c=0;c<b[j];c++)ans=(ans+(ll)(b[j]-c)*inv[a[j]-c])%MOD;D+=a[j]-b[j];}
 vector<ll> Q(1,1);
 for(int j=0;j<m;j++){if(b[j]==0)continue;int A=a[j],lim=A-b[j];vector<ll> f(A+1);
  for(int k=0;k<=A;k++){ll s=0;for(int e=0;e<=min(k,lim);e++){ll v=C[k][e];if((k-e)&1)s-=v;else s+=v;}s%=MOD;if(s<0)s+=MOD;f[k]=C[A][k]*s%MOD;}
  vector<ll> R(Q.size()+A,0);for(size_t i=0;i<Q.size();i++){if(!Q[i])continue;for(int k=0;k<=A;k++)R[i+k]=(R[i+k]+Q[i]*f[k])%MOD;}Q=R;}
 ll s=0;for(size_t k=1;k<Q.size();k++)s=(s+Q[k]*inv[k])%MOD;
 ans=(ans+(D%MOD)*((MOD-s)%MOD))%MOD;printf("%lld\n",ans);}

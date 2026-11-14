#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull; typedef __uint128_t u128;
const ll MOD=998244353;
ull mulm(ull a,ull b,ull m){return (u128)a*b%m;}
ull pw(ull a,ull e,ull m){ull r=1;a%=m;while(e){if(e&1)r=mulm(r,a,m);a=mulm(a,a,m);e>>=1;}return r;}
bool isPrime(ull n){if(n<2)return false;for(ull p:{2,3,5,7,11,13,17,19,23,29,31,37}){if(n%p==0)return n==p;}
 ull d=n-1;int s=0;while(!(d&1)){d>>=1;s++;}
 for(ull a:{2,3,5,7,11,13,17,19,23,29,31,37}){ull x=pw(a,d,n);if(x==1||x==n-1)continue;bool ok=false;for(int r=1;r<s;r++){x=mulm(x,x,n);if(x==n-1){ok=true;break;}}if(!ok)return false;}return true;}
mt19937_64 rng(12345);
ull rho(ull n){if(n%2==0)return 2;while(true){ull x=rng()%n,y=x,c=rng()%(n-1)+1,d=1;auto f=[&](ull v){return (mulm(v,v,n)+c)%n;};
 while(d==1){x=f(x);y=f(f(y));d=__gcd(x>y?x-y:y-x,n);}if(d!=n)return d;}}
void fac(ull n,vector<ull>&v){if(n==1)return;if(isPrime(n)){v.push_back(n);return;}ull d=rho(n);fac(d,v);fac(n/d,v);}
int main(){int T;scanf("%d",&T);while(T--){ll n,m;scanf("%lld %lld",&n,&m);
 vector<ull> ps; ll r=n; for(ll p=2;p<=1000&&p*p<=r;p++) while(r%p==0){ps.push_back(p);r/=p;}
 fac(r,ps); sort(ps.begin(),ps.end());
 vector<pair<ll,ll>> divs={{1,1}}; // (d, h(d) mod)
 for(size_t i=0;i<ps.size();){size_t j=i;while(j<ps.size()&&ps[j]==ps[i])j++;int e=j-i;ll p=ps[i];i=j;
  ll pm=p%MOD; ll pe1=1; for(int t=0;t<e-1;t++)pe1=pe1*pm%MOD; ll base=pe1*((pm-1+MOD)%MOD)%MOD;
  vector<ll> F(e+1);
  for(int jj=0;jj<=e;jj++){ if(2*jj>=e) F[jj]=((ll)e%MOD*base+pe1*pm)%MOD; else F[jj]=(ll)(2*jj+1)*base%MOD;}
  vector<pair<ll,ll>> nd; nd.reserve(divs.size()*(e+1));
  for(auto [d,h]:divs){ll pp=1;for(int jj=0;jj<=e;jj++){ll hv= jj==0?F[0]:(F[jj]-F[jj-1]+MOD)%MOD; nd.push_back({d*pp,h*hv%MOD}); pp*=p;}}
  divs.swap(nd);}
 ll ans=0; for(auto [d,h]:divs) ans=(ans+h*((m/d+1)%MOD))%MOD;
 printf("%lld\n",ans);}}

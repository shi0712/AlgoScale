#include <bits/stdc++.h>
using namespace std;
const long long MOD=998244353;
long long pw(long long a,long long e){long long r=1;for(;e;e>>=1,a=a*a%MOD)if(e&1)r=r*a%MOD;return r;}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);
 int T; if(!(cin>>T)) return 0;
 vector<pair<int,vector<int>>> inp;
 for(int tc=0;tc<T;++tc){int n,m;cin>>n>>m;vector<int>a(n);for(int&x:a)cin>>x;inp.push_back({m,a});}
 int M=0;for(auto &z:inp)M=max(M,z.first*2);
 vector<long long> fac(M+1),ifac(M+1),pow2(M+1);fac[0]=pow2[0]=1;for(int i=1;i<=M;++i){fac[i]=fac[i-1]*i%MOD;pow2[i]=pow2[i-1]*2%MOD;}ifac[M]=pw(fac[M],MOD-2);for(int i=M;i;--i)ifac[i-1]=ifac[i]*i%MOD;
 auto C=[&](int n,int k)->long long{if(k<0||k>n)return 0;return fac[n]*ifac[k]%MOD*ifac[n-k]%MOD;};
 for(auto &cs:inp){int m=cs.first,N=2*m;auto &a=cs.second;vector<long long>b(N+1),w(N+1);b[N]=1;long long ans=0;
   for(int r=N;r>=2;--r){long long p0=fac[r]*fac[N-r]%MOD*pow2[r]%MOD*fac[m]%MOD;
     p0=p0*ifac[r]%MOD*ifac[0]%MOD*ifac[m-r]%MOD*ifac[N]%MOD;
     if(m-r<0)p0=0;
     w[r]=b[r]*pw((1-p0+MOD)%MOD,MOD-2)%MOD;
     long long common=fac[r]*fac[N-r]%MOD*fac[m]%MOD*ifac[N]%MOD;
     for(int x=max(1,r-m);x<=r/2;++x){long long q=common*pow2[r-2*x]%MOD*ifac[r-2*x]%MOD*ifac[x]%MOD*ifac[m-r+x]%MOD; b[r-x]=(b[r-x]+w[r]*q)%MOD;}
   }
   for(int r=2;r<=N;++r){long long hr=0;for(int x:a)if(x>=r)hr=(hr+fac[x]*fac[N-r]%MOD*ifac[x-r]%MOD*ifac[N]%MOD)%MOD;ans=(ans+w[r]*(1-hr+MOD))%MOD;}
   cout<<ans%MOD<<'\n';
 }
}

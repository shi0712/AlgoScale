#include <bits/stdc++.h>
using namespace std;
using ll=long long;
const int MOD=1000000007;
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int T;cin>>T; vector<array<int,3>> cases(T); int M=0; for(auto &v:cases){cin>>v[0]>>v[1]>>v[2]; M=max({M,v[0],v[1],v[2]});}
 vector<int> mu(M+1),spf(M+1),pr,tw(M+1),divs(M+1); mu[1]=tw[1]=1;
 for(int i=2;i<=M;i++){if(!spf[i]){spf[i]=i;pr.push_back(i);mu[i]=-1;tw[i]=2;} for(int p:pr){if(p>spf[i]||1LL*i*p>M)break;spf[i*p]=p;if(i%p==0){mu[i*p]=0;tw[i*p]=0;break;}mu[i*p]=-mu[i];tw[i*p]=2*tw[i];}}
 for(int i=1;i<=M;i++)for(int j=i;j<=M;j+=i) divs[j]++;
 for(int i=1;i<=M;i++) divs[i]+=divs[i-1];
 int W=(M+64)/64; vector<unsigned long long> pos(W),neg(W); for(int i=1;i<=M;i++) {if(mu[i]>0)pos[i>>6]|=1ULL<<(i&63);else if(mu[i]<0)neg[i>>6]|=1ULL<<(i&63);}
 vector<int> small;for(int p:pr)if(p<=1000)small.push_back(p);else break;
 vector<vector<unsigned long long>> masks(small.size(),vector<unsigned long long>(W)); vector<int> pidx(1001,-1); for(int k=0;k<(int)small.size();k++){int p=small[k];pidx[p]=k;for(int j=p;j<=M;j+=p)masks[k][j>>6]|=1ULL<<(j&63);}
 vector<unsigned long long> pp(W),nn(W);vector<int> pref(W+1); 
 auto factors=[&](int v, vector<int>& f){while(v>1){int p=spf[v];f.push_back(p);while(v%p==0)v/=p;}};
 // The Euler factor of the three-variable generating function is
 // (1 - XY - XZ - YZ + 2XYZ) / ((1-X)^2(1-Y)^2(1-Z)^2).
 // Thus pair variables x,y,z and triple variable t are squarefree and
 // pairwise coprime, with weight mu(x)mu(y)mu(z)2^omega(t).
 // D(n)=sum_{i<=n} d(i), stored in divs, supplies the denominator.
 for(auto v:cases){int N=max({v[0],v[1],v[2]}); ll ans=0;
  for(int t=1;t<=N;t++) if(mu[t]){int n=N/t; ll wt=tw[t];
   // The three pair variables are all one.
   ans=(ans+wt*divs[v[0]/t]%MOD*divs[v[1]/t]%MOD*divs[v[2]/t])%MOD;
   for(int y=1;1LL*y*y<=n;y++) if(mu[y]&&gcd(y,t)==1){
    for(int x=1;x<=y;x++) if(mu[x] && (x!=y || x==1) && gcd(x,y)==1 && gcd(x,t)==1){
     int zlo=y+1,zmax=n/y;if(zlo>zmax)continue;
     vector<int> fac; factors(t,fac);factors(x,fac);factors(y,fac);
     int last=zmax>>6; for(int w=0;w<=last;w++){unsigned long long mask=0;for(int p:fac)if(p<=1000)mask|=masks[pidx[p]][w];pp[w]=pos[w]&~mask;nn[w]=neg[w]&~mask;}
     for(int p:fac)if(p>1000)for(int z=p;z<=zmax;z+=p){auto bit=~(1ULL<<(z&63));pp[z>>6]&=bit;nn[z>>6]&=bit;}
     pref[0]=0;for(int w=0;w<=last;w++)pref[w+1]=pref[w]+__builtin_popcountll(pp[w])-__builtin_popcountll(nn[w]);
     auto prefix=[&](int z)->int{if(z<0)return 0;int w=z>>6,b=z&63;unsigned long long mask=~0ULL>>(63-b);return pref[w]+__builtin_popcountll(pp[w]&mask)-__builtin_popcountll(nn[w]&mask);};
     // Each permutation assigns the pair variables to AB, AC, BC.
     vector<pair<int,int>> placements; if(x==y)placements={{x,y},{x,0},{0,x}};else placements={{x,y},{y,x},{x,0},{0,x},{y,0},{0,y}};
     for(auto [u0,u1]:placements){ // zero denotes z; remaining pair is determined
      int u2; if(u0&&u1)u2=0;else if(x==y)u2=x;else if(u0==x||u1==x)u2=y;else u2=x;
      int fixed=0, c1=0,c2=0; int p[3]={u0,u1,u2}; int denom[3];
      // AB, AC, BC: dimension 0 uses pair 0,1; dimension 1 uses 0,2; dimension 2 uses 1,2.
      int ii[3]={0,0,1}, jj[3]={1,2,2}; bool valid=true;
      for(int k=0;k<3;k++){int r=p[ii[k]],s=p[jj[k]];if(r&&s){int d=t*r*s;if(d>v[k]){valid=false;break;}fixed=divs[v[k]/d];}else {int d=t*(r?r:s);denom[k]=d; if(!c1)c1=k+1;else c2=k+1;}}
      if(!valid)continue;int k1=c1-1,k2=c2-1;int lim=min({zmax,v[k1]/denom[k1],v[k2]/denom[k2]});
      for(int l=zlo;l<=lim;){int q1=v[k1]/(denom[k1]*l),q2=v[k2]/(denom[k2]*l);int r=min({lim,v[k1]/(denom[k1]*q1),v[k2]/(denom[k2]*q2)});int sm=prefix(r)-prefix(l-1);if(sm){ll term=wt*fixed%MOD*divs[q1]%MOD*divs[q2]%MOD*sm%MOD*mu[x]*mu[y];ans+=term;if(ans>=MOD||ans<=-MOD)ans%=MOD;}l=r+1;}
     }
    }
   }
  }
  cout<<(ans%MOD+MOD)%MOD<<'\n';
 }
}

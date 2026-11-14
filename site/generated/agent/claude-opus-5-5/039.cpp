#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
#ifndef BB
#define BB 40
#endif
#ifndef DD
#define DD 1000
#endif
int main(){int n,k;if(scanf("%d %d",&n,&k)!=2)return 0;
 vector<vector<ll>> g(14);for(int i=0;i<n;i++){ll v;int t;scanf("%lld %d",&v,&t);g[t].push_back(v);}
 struct C{double key;ll v;int t;};vector<C> cs;
 for(int t=1;t<=13;t++){sort(g[t].rbegin(),g[t].rend());int pos=0;for(ll v:g[t])if(v>0)pos++;
  for(int r=0;r<(int)g[t].size();r++){double key= r<pos ? (r+0.5)/pos : 1.0+(r-pos)+t*1e-3;cs.push_back({key,g[t][r],t});}}
 sort(cs.begin(),cs.end(),[](const C&a,const C&b){return a.key<b.key;});
 int B=min(k,BB);int D=DD;int W=2*D+1;const ll NEG=LLONG_MIN/4;
 vector<ll> dp((B+1)*W,NEG),nd;dp[D]=0;
 for(auto&c:cs){nd=dp;int t=c.t;ll v=c.v;
  for(int j=0;j<=B;j++){ll*o=&nd[j*W];const ll*p=&dp[j*W];
   for(int d=t;d<W;d++)o[d]=max(o[d],p[d-t]+v);
   for(int d=0;d+t<W;d++)o[d]=max(o[d],p[d+t]+v);
   if(j>0){const ll*q=&dp[(j-1)*W];int t2=2*t;
    for(int d=t2;d<W;d++)o[d]=max(o[d],q[d-t2]+v);
    for(int d=0;d+t2<W;d++)o[d]=max(o[d],q[d+t2]+v);}
  }
  swap(dp,nd);}
 ll best=0;for(int j=0;j<=B;j++)best=max(best,dp[j*W+D]);
 printf("%lld\n",best);}

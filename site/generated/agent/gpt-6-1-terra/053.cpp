#include <bits/stdc++.h>
using namespace std;
using ll=long long;
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,c;if(!(cin>>n>>c))return 0;
 vector<ll> t(n+1),pre(c+1);for(int i=1;i<=n;i++)cin>>t[i];sort(t.begin()+1,t.end());
 if(n<=c){cout<<t[n]<<'\n';return 0;}
 if(c==2){vector<ll>d(n+1);d[1]=t[1];d[2]=t[2];for(int i=3;i<=n;i++){d[i]=d[i-1]+t[1]+t[i];if(i>=4)d[i]=min(d[i],d[i-2]+t[1]+2*t[2]+t[i]);}cout<<d[n]<<'\n';return 0;}
 for(int i=1;i<=c;i++)pre[i]=pre[i-1]+t[i];
 const ll INF=LLONG_MAX/4;
 vector<vector<ll>> dp(c+1);
 for(int x=2;x<=n;x++){
  int E=(n-x)/c; auto &a=dp[x%(c+1)];a.assign(E+1,INF);
  if(x<=c){
   a[0]=t[x];
   for(int e=1;e<=E;e++)for(int i=2;i<=min(x,e+1);i++){
    auto &b=dp[i%(c+1)]; int k=e-i+1;
    a[e]=min(a[e],b[k]+t[x]+pre[i]);
   }
  }else{
   for(int e=0;e<=E;e++)for(int i=0;i<=min(c-1,e+1);i++){
    int y=x-c+i;if(y<2)continue;
    auto &b=dp[y%(c+1)];int k=e+1-i;
    a[e]=min(a[e],b[k]+t[x]+pre[i]);
   }
  }
 }
 cout<<dp[n%(c+1)][0]<<'\n';
}

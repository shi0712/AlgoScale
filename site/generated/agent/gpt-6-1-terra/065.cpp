#include <bits/stdc++.h>
using namespace std;
const int INF=100000000;
struct Mat { int a,b,c,d; };
inline Mat mergeM(const Mat &x,const Mat &y){
 return {min(x.a+y.a,x.b+y.c),min(x.a+y.b,x.b+y.d),min(x.c+y.a,x.d+y.c),min(x.c+y.b,x.d+y.d)};
}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);
 int T;cin>>T;while(T--){int n,m;cin>>n>>m;vector<int>a(n);int mn=2001,mx=0;for(int &v:a){cin>>v;mn=min(mn,v);mx=max(mx,v);}for(int &v:a)v-=mn;int V=mx-mn;
 if(m==1){long long ans=V-a[0];for(int i=1;i<n;i++)ans+=max(0,a[i-1]-a[i]);cout<<ans<<'\n';continue;}
 if(V==0){cout<<0<<'\n';continue;}
 vector<vector<int>> pos(V+1);for(int i=0;i<n;i++)pos[a[i]].push_back(i);
 int sz=1;while(sz<n)sz<<=1;vector<Mat> st(2*sz);Mat identity{0,INF,INF,0};int ans=INF;
 for(int gap=0;gap<=V && ans;gap++){
  int x=V-gap;
  auto leaf=[&](int i,bool low)->Mat{
   if(i==0){int lo=low?-a[0]:INF;int hi=gap-a[0];return {lo,hi,lo,hi};}
   int diff=a[i-1]-a[i];int same=max(0,diff),up=max(0,diff+gap),down=max(0,diff-gap);
   return {low?same:INF,up,low?down:INF,same};
  };
  for(int i=0;i<n;i++)st[sz+i]=leaf(i,a[i]<=x);
  for(int i=n;i<sz;i++)st[sz+i]=identity;
  for(int i=sz-1;i;i--)st[i]=mergeM(st[i*2],st[i*2+1]);
  ans=min(ans,x+min(st[1].a,st[1].b));
  for(int val=x+1;val<=V && ans;val++){
   if(pos[val].empty())continue;
   for(int idx:pos[val]){int p=sz+idx;st[p]=leaf(idx,true);for(p>>=1;p;p>>=1)st[p]=mergeM(st[p*2],st[p*2+1]);}
   ans=min(ans,val+min(st[1].a,st[1].b));
  }
 }
 cout<<ans<<'\n';
 }}

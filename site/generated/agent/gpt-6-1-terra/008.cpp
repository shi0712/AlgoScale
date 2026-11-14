#include <bits/stdc++.h>
using namespace std;
using ll=long long;
const int M=2000000;
int freq[M+1],rt[M+1], val[M+1], cnt[M+1];
int hc[M+1];
ll hd[M+1];
int n,P,Q,mx;
struct R{ll cost,ops;};
// Charge t for each halving, then minimize the charged total using at most P
// square roots.  The marginal gains of successive square roots decrease.
R eval(int t){
 int lim=2*t, bits=32-__builtin_clz((unsigned)lim);
 for(int x=1;x<=mx;x++){
  int k=max(0, (32-__builtin_clz((unsigned)x))-bits);
  k+= (x>>k)>lim;
  cnt[x]=k; val[x]=(x>>k)+t*k;
 }
 int top=0; ll base=0, ops=0; static int fsmall[1416]; memset(fsmall,0,sizeof(fsmall));
 for(int x=1;x<=mx;x++) if(freq[x]){
  int f=freq[x]; base+=(ll)f*val[x]; ops+=(ll)f*cnt[x];
  int y=rt[x]; if(y==x)continue;
  int d=val[x]-val[y]; hc[d]+=f; hd[d]+=(ll)f*(cnt[x]-cnt[y]); top=max(top,d); fsmall[y]+=f;
 }
 for(int x=1414;x>=2;x--) if(fsmall[x]){
  int f=fsmall[x],y=rt[x],d=val[x]-val[y];
  hc[d]+=f; hd[d]+=(ll)f*(cnt[x]-cnt[y]); top=max(top,d); fsmall[y]+=f;
 }
 // Take the largest square-root gains, in bulk.  The weighted average at a
 // tied gain gives a valid subgradient for the search over halving prices.
 int rem=P;
 for(int d=top;d>=0;d--){
  if(hc[d]){
   int take=min(rem,hc[d]); base-=(ll)take*d;
   if(take==hc[d])ops-=hd[d]; else ops-=hd[d]*take/hc[d];
   rem-=take; hc[d]=0; hd[d]=0;
  }
 }
 return {base,ops};
}
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 cin>>n>>P>>Q;
 ll sum=0;
 for(int i=0,x;i<n;i++){cin>>x;freq[x]++;mx=max(mx,x);sum+=x;}
 if(!mx){cout<<0<<'\n';return 0;}
 for(int i=1,r=1;i<=mx;i++){while((r+1)*(r+1)<=i)r++;rt[i]=r;}
 if(Q==0){
  static int gains[M+1];
  for(int x=2;x<=mx;x++) if(freq[x]) {int y=x;while(y>1){int z=rt[y];gains[y-z]+=freq[x];y=z;}}
  for(int d=mx;d>0 && P;d--){int k=min(P,gains[d]);sum-=(ll)d*k;P-=k;}
  cout<<sum<<'\n';return 0;
 }
 // The number of halvings in a charged optimum decreases with the price.
 int lo=1,hi=mx;
 while(lo<hi){int mid=(lo+hi)/2;auto r=eval(mid);if(r.ops<=Q)hi=mid;else lo=mid+1;}
 auto r=eval(lo);ll ans=r.cost-(ll)lo*Q;
 if(lo>1){auto s=eval(lo-1);ans=max(ans,s.cost-(ll)(lo-1)*Q);}
 cout<<max(0LL,ans)<<'\n';
}

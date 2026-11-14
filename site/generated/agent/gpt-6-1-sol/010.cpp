#include <bits/stdc++.h>
using namespace std;
/*
Let R[x] be 1 for a card initially in hand, or j+1 for a card at
position j in the hidden queue. Optimal quest completion times satisfy
  t[i] = max(t[i-1]+1, t[previous equal card]+n+1, R[a[i]]).
While waiting, play the hand card whose next quest is furthest away.
This realizes the lower bound: among the n hand cards, at least one has
no quest in the next n turns (the unavailable active card occupies one
of those turns). That filler returns before its next required play.

Compute longest-path distances w[i] from quest i to the final quest.
Only the prefix with d[i] = w[0]-w[i] < n affects the random part of the
answer. Its labels are distinct, since equal labels have distance at
least n+1. For X=R-1 and Z=max(0,max_i(X[a[i]]-d[i])), the answer is
w[0]+1+E[Z]. For 0 <= s < n, nested allowable initial positions give
  Pr(Z <= s) = product_{i: d[i]+s<n} (n+s+d[i]-i)/(2*n-i).
Thus E[Z] = n - sum_s Pr(Z <= s).

The numerators are prefix products of linear factors x+(d[i]-i),
evaluated at x=n+s. Divide the factors into halves; evaluate the left
product at queries needing the right half, and recurse. A shared
subproduct tree performs these multipoint evaluations using NTT.
Complexity: O(m + n log^3 n) time, O(m + n log n) memory.
*/
const int MOD=998244353;
int power(int a,int b){int r=1;for(;b;b>>=1,a=(long long)a*a%MOD)if(b&1)r=(long long)r*a%MOD;return r;}
namespace Poly {
vector<int> roots{0,1}, rev;
void ntt(vector<int>& a){
 int n=a.size(); if(n==1)return;
 int lg=__builtin_ctz((unsigned)n);
 if((int)rev.size()!=n){rev.resize(n);for(int i=0;i<n;i++)rev[i]=(rev[i>>1]>>1)|((i&1)<<(lg-1));}
 if((int)roots.size()<n){int k=__builtin_ctz((unsigned)roots.size());roots.resize(n);while((1<<k)<n){int z=power(3,(MOD-1)>>(k+1));for(int i=1<<(k-1);i<(1<<k);i++){roots[2*i]=roots[i];roots[2*i+1]=(long long)roots[i]*z%MOD;}k++;}}
 for(int i=0;i<n;i++)if(i<rev[i])swap(a[i],a[rev[i]]);
 for(int len=1;len<n;len<<=1)for(int i=0;i<n;i+=2*len)for(int j=0;j<len;j++){
  int z=(long long)a[i+j+len]*roots[len+j]%MOD;
  int x=a[i+j]+z;if(x>=MOD)x-=MOD;
  int y=a[i+j]-z;if(y<0)y+=MOD;
  a[i+j]=x;a[i+j+len]=y;
 }
}
void intt(vector<int>& a){reverse(a.begin()+1,a.end());ntt(a);int z=power(a.size(),MOD-2);for(int &x:a)x=(long long)x*z%MOD;}
vector<int> mul(const vector<int>&a,const vector<int>&b){
 if(a.empty()||b.empty())return {};
 if(min(a.size(),b.size())<=32){vector<int> c(a.size()+b.size()-1);for(int i=0;i<(int)a.size();i++)for(int j=0;j<(int)b.size();j++)c[i+j]=(c[i+j]+(long long)a[i]*b[j])%MOD;return c;}
 int n=1;while(n<(int)(a.size()+b.size()-1))n<<=1;
 vector<int>x(a.begin(),a.end()),y(b.begin(),b.end());x.resize(n);y.resize(n);ntt(x);ntt(y);for(int i=0;i<n;i++)x[i]=(long long)x[i]*y[i]%MOD;intt(x);x.resize(a.size()+b.size()-1);return x;
}
void extend_inverse(const vector<int>&a, vector<int>&r,int need){
 if(r.empty())r.push_back(power(a[0],MOD-2));
 while((int)r.size()<need){int m=r.size(), len=2*m;
  vector<int> f(len),g(len);for(int i=0;i<len&&i<(int)a.size();i++)f[i]=a[i];copy(r.begin(),r.end(),g.begin());
  ntt(f);ntt(g);for(int i=0;i<len;i++)f[i]=(long long)f[i]*g[i]%MOD;intt(f);
  // Newton iteration r <- r - r*(a*r-1), using cyclic convolutions.
  fill(f.begin(),f.begin()+m,0);ntt(f);for(int i=0;i<len;i++)f[i]=(long long)f[i]*g[i]%MOD;intt(f);
  r.resize(len);for(int i=m;i<len;i++)r[i]=f[i]?MOD-f[i]:0;
 }
}
}
struct QNode{int l,r,lc=-1,rc=-1;vector<int> p, inv;};
struct FNode{int l,r,lc=-1,rc=-1,q;vector<int> p;};
struct Solver {
 int n,K; vector<int>d,e,ans;vector<QNode> qs;vector<FNode>fs;
 int build_plain(int l,int r){
  int id=qs.size();qs.push_back({l,r});
  if(r-l==1){qs[id].p={MOD-(n+l),1};return id;}
  int mid=(l+r)/2,a=build_plain(l,mid),b=build_plain(mid,r);
  qs[id].lc=a;qs[id].rc=b;qs[id].p=Poly::mul(qs[a].p,qs[b].p);return id;
 }
 int build(int l,int r){
  int id=fs.size();fs.push_back({l,r});
  // Leaf l contains exactly the queries whose active prefix has l+1 factors.
  if(r-l==1){int lo=r==K?0:n-d[r],hi=n-d[l];fs[id].q=build_plain(lo,hi);fs[id].p={e[l],1};return id;}
  int mid=(l+r)/2,a=build(l,mid),b=build(mid,r);fs[id].lc=a;fs[id].rc=b;
  int qa=fs[a].q,qb=fs[b].q,q=qs.size();qs.push_back({qs[qb].l,qs[qa].r,qb,qa});
  qs[q].p=Poly::mul(qs[qa].p,qs[qb].p);fs[id].q=q;
  fs[id].p=Poly::mul(fs[a].p,fs[b].p);return id;
 }
 vector<int> remainder(const vector<int>&f,int id){
  auto &node=qs[id];int dg=node.p.size()-1,k=(int)f.size()-dg;
  vector<int> rg(node.p.rbegin(),node.p.rend());Poly::extend_inverse(rg,node.inv,k);
  vector<int> rf(k),iv(node.inv.begin(),node.inv.begin()+k);for(int i=0;i<k;i++)rf[i]=f[f.size()-1-i];
  vector<int> q=Poly::mul(rf,iv);q.resize(k);reverse(q.begin(),q.end());q=Poly::mul(q,node.p);
  vector<int> out(dg);for(int i=0;i<dg;i++){out[i]=f[i]-q[i];if(out[i]<0)out[i]+=MOD;}return out;
 }
 void eval(const vector<int>&p,int id){
  auto &q=qs[id];int deg=p.size()-1;
  if(deg<=40 || (long long)(q.r-q.l)*(deg+1)<=4096){
   for(int s=q.l;s<q.r;s++){int v=0,x=n+s;for(int i=deg;i>=0;i--)v=((long long)v*x+p[i])%MOD;ans[s]=(long long)ans[s]*v%MOD;}return;
  }
  if(deg>=q.r-q.l){auto rem=remainder(p,id);eval(rem,id);return;}
  eval(p,q.lc);eval(p,q.rc);
 }
 void run(int id){
  auto &f=fs[id];if(f.r-f.l==1){eval(f.p,f.q);return;}
  // Every query in the right half includes every factor in the left half.
  eval(fs[f.lc].p,fs[f.rc].q);run(f.lc);run(f.rc);
 }
 int compute(int nn,vector<int> dd){
  n=nn;d=move(dd);K=d.size();e.resize(K);for(int i=0;i<K;i++)e[i]=d[i]-i;
  ans.assign(n,1);qs.reserve(2*n+2*K);fs.reserve(2*K);int root=build(0,K);run(root);
  vector<int>den(K+1,1);for(int k=1;k<=K;k++)den[k]=(long long)den[k-1]*(2*n-k+1)%MOD;
  int iv=power(den[K],MOD-2),sum=0;
  for(int k=K;k>=1;k--){int lo=k==K?0:n-d[k],hi=n-d[k-1];for(int s=lo;s<hi;s++){sum=(sum+(long long)ans[s]*iv)%MOD;}iv=(long long)iv*(2*n-k+1)%MOD;}
  int res=n-sum;if(res<0)res+=MOD;return res;
 }
};
int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int T;cin>>T;while(T--){
  int n,m;cin>>n>>m;vector<int>a(m),nxt(2*n+1,-1);for(int &x:a)cin>>x;
  vector<long long>w(m);for(int i=m-1;i>=0;i--){if(i+1<m)w[i]=w[i+1]+1;if(nxt[a[i]]!=-1)w[i]=max(w[i],w[nxt[a[i]]]+n+1);nxt[a[i]]=i;}
  vector<int>d;for(int i=0;i<m && w[0]-w[i]<n;i++)d.push_back(w[0]-w[i]);
  Solver solver;int z=solver.compute(n,move(d));cout<<(w[0]+1+z)%MOD<<'\n';
 }
 return 0;
}

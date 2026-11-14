#include <bits/stdc++.h>
using namespace std;
using ll = long long;
const int MOD=998244353, N=6000;
int power(int a,int b){int r=1; for(;b;b>>=1,a=(ll)a*a%MOD) if(b&1) r=(ll)r*a%MOD; return r;}
struct Group {int p,k; vector<int> val, inv;};
int main(){
 ios::sync_with_stdio(false); cin.tie(nullptr);
 int n; if(!(cin>>n)) return 0;
 vector<int> cnt(N+1); for(int i=0,x;i<n;i++){cin>>x;cnt[x]++;}
 vector<int> primes; vector<int> spf(N+1);
 for(int i=2;i<=N;i++){if(!spf[i]){spf[i]=i; primes.push_back(i);} for(int p:primes){if(p>spf[i] || p*i>N) break; spf[p*i]=p;}}
 vector<int> small; for(int p:primes) if(p<=77) { bool used=false; for(int x=p;x<=N;x+=p) if(cnt[x]) {used=true; break;} if(used) small.push_back(p); }
 int m=small.size(); vector<int> bit(N+1,-1); for(int i=0;i<m;i++) bit[small[i]]=i;
 vector<vector<pair<int,int>>> terms(N+1);
 for(int x=1;x<=N;x++) if(cnt[x]){
  int y=x, mask=0, large=0;
  while(y>1){int p=spf[y]; if(p<=77) mask|=1<<bit[p]; else large=p; while(y%p==0)y/=p;}
  terms[large].push_back({mask,power(x+1,cnt[x])});
 }
 vector<Group> groups;
 auto make_group=[&](int p,int k,const vector<pair<int,int>>& t){
  int sz=1<<k; Group g; g.p=p;g.k=k;g.val.assign(sz,1);
  for(auto [mask,v]:t) g.val[mask]=(ll)g.val[mask]*v%MOD;
  for(int b=0;b<k;b++) for(int base=0;base<sz;base+=2<<b) for(int j=0;j<(1<<b);j++){
    int pos=base+(1<<b)+j; g.val[pos]=(ll)g.val[pos]*g.val[base+j]%MOD;
  }
  if(p) for(int &v:g.val) v=(1LL+(ll)(p-1)*v)%MOD;
  groups.push_back(move(g));
 };
 make_group(0,m,terms[0]);
 int norm=1;
 for(int p:primes) if(p>77 && !terms[p].empty()){
  int mask=0; for(auto [s,v]:terms[p])mask|=s;
  int k=mask ? 32-__builtin_clz((unsigned)mask):0;
  make_group(p,k,terms[p]); norm=(ll)norm*power(p,MOD-2)%MOD;
 }
 for(int p:small) norm=(ll)norm*power(p,MOD-2)%MOD;
 // Batch inversion handles zero table entries, which are tracked separately.
 for(auto &g:groups){
  int sz=g.val.size(); g.inv.resize(sz); int prod=1;
  for(int i=0;i<sz;i++){g.inv[i]=prod; if(g.val[i]) prod=(ll)prod*g.val[i]%MOD;}
  prod=power(prod,MOD-2);
  for(int i=sz-1;i>=0;i--){if(g.val[i]){g.inv[i]=(ll)prod*g.inv[i]%MOD; prod=(ll)prod*g.val[i]%MOD;}else g.inv[i]=0;}
 }
 vector<vector<int>> affected(m);
 for(int i=0;i<(int)groups.size();i++) for(int j=0;j<groups[i].k;j++) affected[j].push_back(i);
 int total=1, zeros=0; for(auto &g:groups){int v=g.val[0]; if(v)total=(ll)total*v%MOD;else zeros++;}
 vector<int> invcoef(m); for(int i=0;i<m;i++) invcoef[i]=power(small[i]-1,MOD-2);
 int coef=1, mask=0, answer=0;
 int states=1<<m;
 for(int step=0;step<states;step++){
  if(!zeros) answer=(answer+(ll)total*coef)%MOD;
  if(step+1==states) break;
  int b=m-1-__builtin_ctz((unsigned)(step+1));
  int next=mask^(1<<b);
  if(next&(1<<b))coef=(ll)coef*(small[b]-1)%MOD;else coef=(ll)coef*invcoef[b]%MOD;
  for(int idx:affected[b]){
   auto &g=groups[idx]; int filter=(1<<g.k)-1;
   int old=mask&filter, now=next&filter;
   int x=g.val[old], y=g.val[now];
   if(x)total=(ll)total*g.inv[old]%MOD;else zeros--;
   if(y)total=(ll)total*y%MOD;else zeros++;
  }
  mask=next;
 }
 cout<<(ll)answer*norm%MOD<<'\n';
}

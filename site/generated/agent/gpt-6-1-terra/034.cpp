#include <bits/stdc++.h>
using namespace std;
const int MOD=998244353, G=3;
int add(int a,int b){a+=b; if(a>=MOD)a-=MOD; return a;}
int subm(int a,int b){a-=b; if(a<0)a+=MOD; return a;}
int pw(int a,int b){int r=1; for(;b;b>>=1,a=(long long)a*a%MOD)if(b&1)r=(long long)r*a%MOD;return r;}
void ntt(vector<int>& a,bool inv){
 int n=a.size();
 for(int i=1,j=0;i<n;i++){int bit=n>>1;for(;j&bit;bit>>=1)j^=bit;j^=bit;if(i<j)swap(a[i],a[j]);}
 for(int len=2;len<=n;len<<=1){int wlen=pw(G,(MOD-1)/len);if(inv)wlen=pw(wlen,MOD-2);for(int i=0;i<n;i+=len){long long w=1;for(int j=0;j<len/2;j++){int u=a[i+j],v=w*a[i+j+len/2]%MOD;a[i+j]=add(u,v);a[i+j+len/2]=subm(u,v);w=w*wlen%MOD;}}}
 if(inv){int z=pw(n,MOD-2);for(int &x:a)x=(long long)x*z%MOD;}
}
vector<int> mul(const vector<int>&a,const vector<int>&b){
 if(a.empty()||b.empty())return {};
 int sz=a.size()+b.size()-1;
 if((long long)a.size()*b.size()<=4096){vector<int> c(sz);for(int i=0;i<(int)a.size();i++)for(int j=0;j<(int)b.size();j++)c[i+j]=(c[i+j]+(long long)a[i]*b[j])%MOD;return c;}
 int n=1;while(n<sz)n<<=1;vector<int>x(a.begin(),a.end()),y(b.begin(),b.end());x.resize(n);y.resize(n);ntt(x,0);ntt(y,0);for(int i=0;i<n;i++)x[i]=(long long)x[i]*y[i]%MOD;ntt(x,1);x.resize(sz);return x;
}
int n; unsigned long long k; vector<unsigned long long>a,b;vector<int>fac,ifac;
struct Poly { int kind; vector<int> p; }; // 0 empty, 1 complete, 2 explicit
vector<int> materialize(Poly &p,int A,int B){
 if(p.kind==0)return {1};
 if(p.kind==2)return move(p.p);
 int m=min(A,B);vector<int> v(m+1);for(int i=0;i<=m;i++)v[i]=(long long)fac[A]*fac[B]%MOD*ifac[A-i]%MOD*ifac[B-i]%MOD*ifac[i]%MOD;return v;
}
Poly complement(Poly p,int A,int B){
 if(p.kind<2){p.kind^=1;return p;}
 int m=min(A,B); vector<int>x(p.p.size()),y(m+1);
 for(int i=0;i<(int)x.size();i++){x[i]=(long long)p.p[i]*fac[A-i]%MOD*fac[B-i]%MOD;if(i&1&&x[i])x[i]=MOD-x[i];}
 for(int i=0;i<=m;i++)y[i]=ifac[i];
 vector<int>z=mul(x,y);z.resize(m+1);
 for(int i=0;i<=m;i++)z[i]=(long long)z[i]*ifac[A-i]%MOD*ifac[B-i]%MOD;
 return {2,move(z)};
}
int split(const vector<unsigned long long>&v,int l,int r,int bit){unsigned long long mask=1ULL<<bit;int lo=l,hi=r;while(lo<hi){int md=(lo+hi)/2;if(v[md]&mask)hi=md;else lo=md+1;}return lo;}
Poly solve(int bit,int al,int ar,int bl,int br,bool inv){
 int A=ar-al,B=br-bl;if(!A||!B)return {0,{}};
 if(bit<0)return {inv?0:1,{}};
 int am=split(a,al,ar,bit),bm=split(b,bl,br,bit);int ka=(k>>bit)&1;
 // A common fixed XOR bit determines the answer immediately or leaves a smaller problem.
 if((am==al||am==ar)&&(bm==bl||bm==br)){
  int xa=(am==al),xb=(bm==bl),v=xa^xb;
  if(v!=ka)return {(v>ka)^inv?1:0,{}};
  return solve(bit-1,al,ar,bl,br,inv);
 }
 bool natural=!ka; // k bit 0: complement splits; k bit 1: graph splits
 int aL[2]={al,am},aR[2]={am,ar},bL[2]={bl,bm},bR[2]={bm,br};
 Poly parts[2];int aa[2],bb[2];
 for(int t=0;t<2;t++){int u=t^ka;aa[t]=aR[t]-aL[t];bb[t]=bR[u]-bL[u];parts[t]=solve(bit-1,aL[t],aR[t],bL[u],bR[u],natural);}
 Poly res;
 if(parts[0].kind==0&&parts[1].kind==0)res={0,{}};
 else if(parts[0].kind==0||parts[1].kind==0){int t=parts[0].kind==0?1:0;res=move(parts[t]);if(res.kind==1&&(aa[t]!=A||bb[t]!=B)){res.p=materialize(res,aa[t],bb[t]);res.kind=2;}}
 else {auto x=materialize(parts[0],aa[0],bb[0]);auto y=materialize(parts[1],aa[1],bb[1]);res={2,mul(x,y)};}
 if(inv!=natural)res=complement(move(res),A,B);
 return res;
}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);cin>>n>>k;a.resize(n);b.resize(n);for(auto &x:a)cin>>x;for(auto &x:b)cin>>x;sort(a.begin(),a.end());sort(b.begin(),b.end());fac.resize(n+1);ifac.resize(n+1);fac[0]=1;for(int i=1;i<=n;i++)fac[i]=(long long)fac[i-1]*i%MOD;ifac[n]=pw(fac[n],MOD-2);for(int i=n;i;i--)ifac[i-1]=(long long)ifac[i]*i%MOD;Poly ans=solve(59,0,n,0,n,0);vector<int>p=materialize(ans,n,n);for(int i=1;i<=n;i++)cout<<(i<(int)p.size()?p[i]:0)<<(i==n?'\n':' ');}

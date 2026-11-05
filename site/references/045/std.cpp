#include <bits/stdc++.h>
using namespace std;
using int64 = long long;
using i128 = __int128_t;

template<int MOD, int G>
struct NTT {
    static int qpow(int a, int64 e) { int64 r=1; while(e){ if(e&1) r=r*a%MOD; a=(int64)a*a%MOD; e>>=1;} return (int)r; }
    static void ntt(vector<int>& a, bool inv){
        int n=a.size();
        for(int i=1,j=0;i<n;++i){ int bit=n>>1; while(j&bit){j^=bit;bit>>=1;} j^=bit; if(i<j) swap(a[i],a[j]); }
        for(int len=2;len<=n;len<<=1){ int wn=qpow(G,(MOD-1)/len); if(inv) wn=qpow(wn,MOD-2); for(int i=0;i<n;i+=len){int64 w=1; for(int j=0;j<len/2;++j){int x=a[i+j]; int y=(int)(a[i+j+len/2]*w%MOD); int u=x+y; if(u>=MOD)u-=MOD; int v=x-y; if(v<0)v+=MOD; a[i+j]=u; a[i+j+len/2]=v; w=w*wn%MOD;}}}
        if(inv){ int invn=qpow(n,MOD-2); for(int&x:a)x=(int64)x*invn%MOD; }
    }
    static vector<int> multiply(const vector<int>& a,const vector<int>&b){ if(a.empty()||b.empty())return{}; int need=a.size()+b.size()-1,sz=1; while(sz<need)sz<<=1; vector<int>A(sz),B(sz); for(int i=0;i<(int)a.size();++i)A[i]=a[i]%MOD; for(int i=0;i<(int)b.size();++i)B[i]=b[i]%MOD; ntt(A,0); ntt(B,0); for(int i=0;i<sz;++i)A[i]=(int64)A[i]*B[i]%MOD; ntt(A,1); A.resize(need); return A; }
};
constexpr int P1=998244353,P2=1004535809,P3=469762049; using N1=NTT<P1,3>;using N2=NTT<P2,3>;using N3=NTT<P3,3>;
struct Solver{
 int n,MOD; vector<int>a,q,A1,Q1,R,T,S,cp,cr,ct,cs,inv;
 static int powmod(int64 a,int64 e,int mod){int64 r=1;a%=mod;while(e){if(e&1)r=r*a%mod;a=a*a%mod;e>>=1;}return(int)r;}
 int norm(int64 x)const{x%=MOD;if(x<0)x+=MOD;return(int)x;}
 vector<int> multiply(const vector<int>&x,const vector<int>&y,int limit=-1)const{
  if(x.empty()||y.empty())return{}; int need=x.size()+y.size()-1;if(limit>=0)need=min(need,limit);if(need<=0)return{};
  if(1LL*x.size()*y.size()<=4096){vector<int>z(need);for(int i=0;i<(int)x.size();++i){int lim=min((int)y.size()-1,need-1-i);for(int j=0;j<=lim;++j)z[i+j]=(z[i+j]+(int64)x[i]*y[j])%MOD;}return z;}
  if(MOD==P1){auto z=N1::multiply(x,y);if((int)z.size()>need)z.resize(need);return z;}
  if(MOD==P3){auto z=N3::multiply(x,y);if((int)z.size()>need)z.resize(need);return z;}
  vector<int>x1(x.size()),x2(x.size()),x3(x.size()),y1(y.size()),y2(y.size()),y3(y.size());
  for(int i=0;i<(int)x.size();++i)x1[i]=x[i]%P1,x2[i]=x[i]%P2,x3[i]=x[i]%P3;
  for(int i=0;i<(int)y.size();++i)y1[i]=y[i]%P1,y2[i]=y[i]%P2,y3[i]=y[i]%P3;
  auto z1=N1::multiply(x1,y1),z2=N2::multiply(x2,y2),z3=N3::multiply(x3,y3);z1.resize(need);z2.resize(need);z3.resize(need);
  static const int invP1P2=powmod(P1,P2-2LL,P2); static const int64 P12=1LL*P1*P2; static const int invP12P3=powmod(P12%P3,P3-2LL,P3);
  vector<int>z(need);for(int i=0;i<need;++i){int64 r1=z1[i],r2=z2[i],r3=z3[i];int64 d2=(r2-r1)%P2;if(d2<0)d2+=P2;int64 t2=d2*invP1P2%P2;int64 x12=r1+(int64)P1*t2;int64 d3=(r3-x12%P3)%P3;if(d3<0)d3+=P3;int64 t3=d3*invP12P3%P3;i128 real=(i128)x12+(i128)P12*t3;z[i]=(int)(real%MOD);}return z;
 }
 vector<int>cut(const vector<int>&v,int l,int r)const{return l>r?vector<int>{}:vector<int>(v.begin()+l,v.begin()+r+1);} void addTo(vector<int>&a,int p,int x){a[p]+=x;if(a[p]>=MOD)a[p]-=MOD;}
 void addCross(const vector<int>&X,const vector<int>&Y,vector<int>&dst,int l,int mid,int r){int width=r-l;{int yr=min(mid,width);if(yr>=1){auto u=cut(X,l,mid),v=cut(Y,1,yr),w=multiply(u,v,r-l);int off=l+1,L=max(mid+1,off),Rr=min(r,off+(int)w.size()-1);for(int k=L;k<=Rr;++k)addTo(dst,k,w[k-off]);}}{int xr=min(l-1,width);if(xr>=1){auto u=cut(X,1,xr),v=cut(Y,l,mid),w=multiply(u,v,r-l);int off=l+1,L=max(mid+1,off),Rr=min(r,off+(int)w.size()-1);for(int k=L;k<=Rr;++k)addTo(dst,k,w[k-off]);}}}
 void cdq(int l,int r){if(l==r){int m=l,P=cp[m];int K=norm((int64)cr[m]-16LL*S[m-1]-4LL*a[m-1]+16LL*T[m-1]);int am=norm((int64)K-(int64)(m+2)*P);am=(int64)am*inv[m+1]%MOD*inv[m+1]%MOD;int qm=norm(-(int64)P-(int64)m*am);a[m]=am;q[m]=qm;A1[m]=(int64)(m+1)*am%MOD;Q1[m]=(int64)(m+1)*qm%MOD;R[m]=norm((int64)(m+2)*qm+cr[m]);T[m]=norm((int64)am+qm+ct[m]);S[m]=norm((int64)am+R[m]+cs[m]);return;}int mid=(l+r)>>1;cdq(l,mid);addCross(q,A1,cp,l,mid,r);addCross(q,Q1,cr,l,mid,r);addCross(a,q,ct,l,mid,r);addCross(a,R,cs,l,mid,r);cdq(mid+1,r);}
 vector<int>polyInv(const vector<int>&f,int need)const{vector<int>g{powmod(f[0],MOD-2LL,MOD)};int len=1;while(len<need){int nxt=min(len<<1,need);vector<int>F(f.begin(),f.begin()+min((int)f.size(),nxt));auto t=multiply(F,g,nxt);t.resize(nxt);for(int&x:t)if(x)x=MOD-x;t[0]+=2;if(t[0]>=MOD)t[0]-=MOD;auto ng=multiply(g,t,nxt);ng.resize(nxt);g.swap(ng);len=nxt;}return g;}
 vector<int>polyPow(vector<int>a,int e,int need)const{a.resize(min((int)a.size(),need));vector<int>r(1,1);while(e){if(e&1){r=multiply(r,a,need);r.resize(min((int)r.size(),need));}e>>=1;if(e){a=multiply(a,a,need);a.resize(min((int)a.size(),need));}}r.resize(need);return r;}
 vector<int> solve(){a.assign(n+1,0);q.assign(n+1,0);A1.assign(n+1,0);Q1.assign(n+1,0);R.assign(n+1,0);T.assign(n+1,0);S.assign(n+1,0);cp.assign(n+1,0);cr.assign(n+1,0);ct.assign(n+1,0);cs.assign(n+1,0);inv.assign(n+2,0);a[0]=q[0]=1;R[0]=T[0]=S[0]=1;inv[1]=1;for(int i=2;i<=n+1;++i)inv[i]=MOD-(int64)(MOD/i)*inv[MOD%i]%MOD;if(n>=1)cdq(1,n);vector<int>Y=polyInv(a,n+1);vector<int>G=polyPow(Y,n+1,n+1);vector<int>fac(2*n+1),ifac(2*n+1);fac[0]=1;for(int i=1;i<=2*n;++i)fac[i]=(int64)fac[i-1]*i%MOD;ifac[2*n]=powmod(fac[2*n],MOD-2LL,MOD);for(int i=2*n;i>=1;--i)ifac[i-1]=(int64)ifac[i]*i%MOD;vector<int>B(n+1);int invN=inv[n+1];for(int k=0;k<=n;++k){int C=(int64)fac[2*n]*ifac[k]%MOD*ifac[2*n-k]%MOD;B[k]=(int64)C*invN%MOD*G[n-k]%MOD;}vector<int>lhs(n+1),rhs(n+1);for(int i=0;i<=n;++i){lhs[n-i]=(int64)B[i]*fac[i]%MOD;rhs[i]=(i&1)?(ifac[i]?MOD-ifac[i]:0):ifac[i];}auto cv=multiply(lhs,rhs,n+1);cv.resize(n+1);vector<int>ans(n+1);for(int k=0;k<=n;++k)ans[k]=(int64)cv[n-k]*ifac[k]%MOD;return ans;}
};
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);int tc;cin>>tc;while(tc--){Solver s;cin>>s.n>>s.MOD;auto ans=s.solve();for(int i=0;i<=s.n;++i){if(i)cout<<' ';cout<<ans[i];}cout<<'\n';}}

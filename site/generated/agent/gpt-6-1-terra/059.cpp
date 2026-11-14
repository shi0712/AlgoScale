#include <bits/stdc++.h>
#include <dlfcn.h>
using namespace std;
// Use the GMP runtime for enough precision to withstand inclusion-exclusion.
struct __mpf_struct {int prec; int size; long exp; unsigned long *d;};
using mpf=__mpf_struct[1];
void* lib=dlopen("libgmp.so.10",RTLD_NOW);
template<class T> T sym(const char*s){return reinterpret_cast<T>(dlsym(lib,s));}
auto initf=sym<void(*)(mpf,unsigned long)>("__gmpf_init2");
auto clearf=sym<void(*)(mpf)>("__gmpf_clear");
auto setf=sym<void(*)(mpf,const __mpf_struct*)>("__gmpf_set");
auto setsi=sym<void(*)(mpf,long)>("__gmpf_set_si");
auto addf=sym<void(*)(mpf,const __mpf_struct*,const __mpf_struct*)>("__gmpf_add");
auto subf=sym<void(*)(mpf,const __mpf_struct*,const __mpf_struct*)>("__gmpf_sub");
auto mulf=sym<void(*)(mpf,const __mpf_struct*,const __mpf_struct*)>("__gmpf_mul");
auto divf=sym<void(*)(mpf,const __mpf_struct*,const __mpf_struct*)>("__gmpf_div");
auto mului=sym<void(*)(mpf,const __mpf_struct*,unsigned long)>("__gmpf_mul_ui");
auto divui=sym<void(*)(mpf,const __mpf_struct*,unsigned long)>("__gmpf_div_ui");
auto getd=sym<double(*)(const __mpf_struct*)>("__gmpf_get_d");
struct Real { mpf x;
 Real(){initf(x,512);} Real(long long v){initf(x,512);setsi(x,v);} Real(const Real& v){initf(x,512);setf(x,v.x);} ~Real(){clearf(x);}
 Real& operator=(const Real&v){setf(x,v.x);return *this;} Real& operator=(long long v){setsi(x,v);return *this;}
 Real operator-() const { Real z(0); subf(z.x,z.x,x);return z; }
 Real& operator+=(const Real&b){addf(x,x,b.x);return *this;} Real& operator-=(const Real&b){subf(x,x,b.x);return *this;}
 Real& operator*=(const Real&b){mulf(x,x,b.x);return *this;} Real& operator/=(const Real&b){divf(x,x,b.x);return *this;}
 friend Real operator+(Real a,const Real&b){return a+=b;} friend Real operator-(Real a,const Real&b){return a-=b;}
 friend Real operator*(Real a,const Real&b){return a*=b;} friend Real operator/(Real a,const Real&b){return a/=b;}
 Real& operator*=(long long b){mului(x,x,b);return *this;} Real& operator/=(long long b){divui(x,x,b);return *this;}
 friend Real operator*(Real a,long long b){return a*=b;} friend Real operator/(Real a,long long b){return a/=b;}
 friend ostream& operator<<(ostream& o,const Real&a){return o<<getd(a.x);}
};
int main(){
 ios::sync_with_stdio(false); cin.tie(nullptr);
 int n; long long m; if(!(cin>>n>>m)) return 0;
 vector<Real> fac(n+2), invfac(n+2), bern(n+1); fac[0]=1;
 for(int i=1;i<=n+1;i++) fac[i]=fac[i-1]*i;
 for(int i=0;i<=n+1;i++) invfac[i]=1/fac[i];
 bern[0]=1;
 for(int k=1;k<=n;k++){ Real s=0; for(int j=0;j<k;j++) s+=bern[j]*invfac[k-j+1]; bern[k]=-s; }
 // F[j] is C(n,j) times the sum, over positive t, of the probability
 // that a specified j gaps all exceed t. There are C(m-j*t,n)
 // corresponding subsets.
 vector<Real> F(n+1), choose(n+1); choose[0]=1;
 for(int j=1;j<=n;j++) choose[j]=choose[j-1]*(n-j+1)/j;
 for(int j=1;j<=n;j++){
  long long L=(m-n)/j;
  if(!L) continue;
  Real sum=0;
  if(L<n){
   for(long long t=1;t<=L;t++){
    Real p=1; long long shift=j*t;
    for(int r=0;r<n;r++){ p*=m-r-shift; p/=m-r; }
    sum+=p;
   }
  }else{
   // The summand is a polynomial of degree n in t. Store it in the
   // Bernstein basis on [0,L], whose coefficients are nonnegative.
   // Integrate the polynomial exactly and apply Euler-Maclaurin; endpoint
   // derivatives are differences of its Bernstein coefficients.
   vector<Real> b(n+1); b[0]=1;
   Real tmp;
   for(int d=1;d<=n;d++){
    Real end(m-(d-1)-j*L); end/=m-(d-1);
    for(int k=d;k>=0;k--){
     if(k<d) b[k]*=d-k; else b[k]=0;
     if(k>0){ mulf(tmp.x,b[k-1].x,end.x); tmp*=k; addf(b[k].x,b[k].x,tmp.x); }
     b[k]/=d;
    }
   }
   for(int k=0;k<=n;k++) sum+=b[k];
   sum*=Real(L)/Real(n+1);
   sum+=(b[n]-b[0])/2;
   Real multiplier=1;
   for(int d=1;d<=n;d++){
    for(int k=0;k<=n-d;k++) b[k]=b[k+1]-b[k];
    multiplier*=Real(n-d+1)/Real(L);
    if(d%2==1 && d+1<=n) sum+=bern[d+1]*multiplier*(b[n-d]-b[0]);
   }
  }
  F[j]=sum*choose[j];
 }
 cout<<fixed<<setprecision(12);
 // If K gaps exceed t, their contribution to the smallest i gaps is
 // max(0,K-(n-i)). Expand this function in binomial coefficients of K.
 for(int i=1;i<=n;i++){
  Real ans=i;
  if(i==n) ans+=F[1];
  else {
   int q=n-i; Real coeff=1;
   for(int j=q+1;j<=n;j++){
    if(j>q+1) coeff*=Real(j-2)/Real(j-q-1);
    if((j-q-1)%2==0) ans+=coeff*F[j]; else ans-=coeff*F[j];
   }
  }
  cout<<ans<<'\n';
 }
}

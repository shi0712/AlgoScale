#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
static const int Fv[11]={0,0,8,7,3,7,6,3,7,2,5};
static u64 P11[19]; static int BIN[12][12];
template<int P> struct Solver{
 int L; u64 u,pw[64]; int cd[64],sd[64],hk[64][P]; bool special; int coef[12];
 vector<int> needE; int nprim,h; u64 Ph,Uh,lamTot;
 vector<unsigned char> Whi,Y; vector<int> PZ; u64 w,dhi[64],dlo[64],rho_hi,rho_lo; long long acc;
 inline u64 modU(u64 x)const{if constexpr(P==2)return x&(u-1);else return x%u;}
 inline u64 modPh(u64 x)const{if constexpr(P==2)return x&(Ph-1);else return x%Ph;}
 inline u64 divPh(u64 x)const{if constexpr(P==2)return x>>h;else return x/Ph;}
 inline u64 modUh(u64 x)const{if constexpr(P==2)return x&(Uh-1);else return x%Uh;}
 static u64 inv(u64 a,u64 m){long long o_r=(long long)a,r=(long long)m,o_s=1,s=0;
  while(r){long long q=o_r/r,t=o_r-q*r;o_r=r;r=t;t=o_s-q*s;o_s=s;s=t;}
  long long res=o_s%(long long)m; if(res<0)res+=(long long)m; return (u64)res;}
 inline int Weval(u64 z)const{int r=1;for(int k=0;k<L;k++){int e=(int)(z%(u64)P);z/=(u64)P;r=r*hk[k][e]%P;if(!r)return 0;}return r;}
 void init(u64 c){L=0;u=1;while(u<=c){u*=(u64)P;L++;} pw[0]=1;for(int k=1;k<=L;k++)pw[k]=pw[k-1]*(u64)P;
  u64 t=c;for(int k=0;k<L;k++){cd[k]=(int)(t%(u64)P);t/=(u64)P;sd[k]=P-1-cd[k];}
  for(int k=0;k<L;k++)for(int e=0;e<P;e++)hk[k][e]=(e+cd[k]<P)?(BIN[e+cd[k]][cd[k]]%P):0;
  special=(c==u-1);
  for(int e=0;e<=11;e++){int fe=(e<=10?Fv[e]:0),fm=(e>=1?Fv[e-1]:0);coef[e]=((fe-fm)%P+P)%P;}
  needE.clear();for(int e=0;e<=10;e++)if(coef[e])needE.push_back(e); if(coef[11])needE.push_back(0);
  nprim=(int)needE.size(); if(!nprim)return;
  double best=1e300;h=0;u64 lam=1;
  for(int hh=0;hh<=L;hh++){double cost=3.0*(double)pw[L-hh]+8.0*(double)nprim*(double)lam;
   if(cost<best){best=cost;h=hh;} if(hh<L)lam*=(u64)(P-cd[hh]);}
  Ph=pw[h];Uh=pw[L-h];lamTot=1;for(int k=0;k<h;k++)lamTot*=(u64)(P-cd[k]);
  Whi.assign((size_t)Uh,1);
  {vector<unsigned char> tmp((size_t)Uh,0);u64 sz=1;
   for(int j=0;j<L-h;j++){for(int d=0;d<P;d++)for(u64 z=0;z<sz;z++)tmp[(size_t)((u64)d*sz+z)]=(unsigned char)((int)Whi[(size_t)z]*hk[h+j][d]%P);
    u64 ns=sz*(u64)P;for(u64 z=0;z<ns;z++)Whi[(size_t)z]=tmp[(size_t)z];sz=ns;}}
  Y.assign((size_t)Uh,0);PZ.assign((size_t)(2*Uh+1),0);}
 void setupK(u64 v){u64 vm=modU(v);w=inv(vm,u);u64 st=modUh(vm),z=0;
  for(u64 j=0;j<Uh;j++){Y[(size_t)j]=Whi[(size_t)z];z+=st;if(z>=Uh)z-=Uh;}
  int s=0;PZ[0]=0;
  for(u64 i=0;i<Uh;i++){s+=Y[(size_t)i];if(s>=P)s-=P;PZ[(size_t)i+1]=s;}
  for(u64 i=0;i<Uh;i++){s+=Y[(size_t)i];if(s>=P)s-=P;PZ[(size_t)(Uh+i+1)]=s;}
  for(int k=0;k<h;k++){u64 d=modU(pw[k]*w);dhi[k]=divPh(d);dlo[k]=modPh(d);}}
 void dfs(int k,u64 bhi,u64 blo,int wt){
  if(k==h){u64 idx=bhi+Uh+1;int val=PZ[(size_t)idx]-PZ[(size_t)(idx-rho_hi)];if(val<0)val+=P;
   if(blo<rho_lo){u64 t=bhi+Uh-rho_hi;if(t>=Uh)t-=Uh;val+=Y[(size_t)t];}
   acc+=(long long)wt*val;return;}
  u64 dh=dhi[k],dl=dlo[k];const int*H=hk[k];int lim=sd[k];
  for(int e=0;e<=lim;e++){int nw; if constexpr(P==2)nw=wt; else nw=wt*H[e]%P;
   dfs(k+1,bhi,blo,nw);
   if(blo>=dl)blo-=dl;else{blo+=Ph-dl;bhi=(bhi?bhi-1:Uh-1);}
   bhi=(bhi>=dh?bhi-dh:bhi+Uh-dh);}}
 int psi(u64 A,u64 v,u64 vm,bool direct){u64 Q=A/v;
  if(special){u64 q0=modU(modU(A)*w);if(q0>Q)return 0;return (int)(((Q-q0)/u+1)%(u64)P);}
  if(direct){u64 z=modU(A);int s=0;for(u64 q=0;q<=Q;q++){s+=Weval(z);if(s>=P)s-=P;z=(z>=vm?z-vm:z+u-vm);}return s;}
  u64 rho=modU(Q+1);if(!rho)return 0;rho_hi=divPh(rho);rho_lo=modPh(rho);
  u64 B=modU(modU(A)*w);acc=0;dfs(0,divPh(B),modPh(B),1);return (int)(acc%P);}
 int solve(u64 n,u64 c){init(c);if(!nprim)return 0;int total=0,phn=Weval(modU(n));
  for(int k=0;k<=17&&P11[k]<=n;k++){u64 v=P11[k+1],vm=modU(v);w=inv(vm,u);
   u64 Qmax=n/v;bool direct;
   if(special)direct=true;else{double dc=(double)nprim*(double)(Qmax+1)*(double)(L+2);
    double mc=3.0*(double)Uh+8.0*(double)nprim*(double)lamTot;direct=(dc<mc)&&(Qmax<=1000000ULL);}
   if(!direct)setupK(v);
   int vals[12];for(int i=0;i<12;i++)vals[i]=0;
   for(size_t i=0;i<needE.size();i++){int e=needE[i];u64 ek=(u64)e*P11[k];
    if(ek>n){vals[e]=0;continue;} vals[e]=psi(n-ek,v,vm,direct);}
   int contrib=0;for(int e=0;e<=10;e++)if(coef[e])contrib+=coef[e]*vals[e];
   if(coef[11]){int d=vals[0]-phn;if(d<0)d+=P;contrib+=coef[11]*d;}
   total=(total+contrib)%P;}
  return total;}
};
int main(){
 P11[0]=1;for(int i=1;i<=18;i++)P11[i]=P11[i-1]*11ULL;
 for(int i=0;i<12;i++){BIN[i][0]=1;for(int j=1;j<=i;j++)BIN[i][j]=BIN[i-1][j-1]+BIN[i-1][j];}
 int T;if(scanf("%d",&T)!=1)return 0;
 while(T--){unsigned long long n,c;if(scanf("%llu %llu",&n,&c)!=2)return 0;
  Solver<2> s2;Solver<5> s5;int a2=s2.solve(n,c),a5=s5.solve(n,c);
  int x=0;for(int i=0;i<10;i++)if(i%2==a2&&i%5==a5){x=i;break;}
  int ans=1;for(int i=0;i<x;i++)ans=ans*2%11;printf("%d\n",ans);}
 return 0;}

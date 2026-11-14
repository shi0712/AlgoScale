#include <bits/stdc++.h>
using namespace std;
static constexpr int MOD=1000000007;
inline int add(int a,int b){ int s=a+b; return s>=MOD?s-MOD:s; }
int main(){
 ios::sync_with_stdio(false); cin.tie(nullptr);
 int n,Q; cin>>n>>Q; string s; cin>>s;
 int m=count(s.begin(),s.end(),'?'), W=m+2, size=W*W;
 vector<int> A(size),B(size),C(size), NA(size),NB(size),NC(size);
 int k=0; bool first=true;
 for(char ch:s){
  if(first){
   if(ch=='?'){A[W]=1; B[1]=1; C[0]=1; k=1;}
   else if(ch=='a') A[0]=1; else if(ch=='b') B[0]=1; else C[0]=1;
   first=false; continue;
  }
  if(ch=='?'){
   ++k;
   for(int a=0;a<=k;++a){
    int base=a*W, limit=k-a;
    for(int b=0;b<=limit;++b){
     int i=base+b;
     NA[i]=a ? add(B[i-W],C[i-W]) : 0;
     NB[i]=b ? add(A[i-1],C[i-1]) : 0;
     NC[i]=(a+b<k) ? add(A[i],B[i]) : 0;
    }
   }
   A.swap(NA); B.swap(NB); C.swap(NC);
  }else{
   for(int a=0;a<=k;++a){
    int base=a*W;
    for(int b=0;b<=k-a;++b){
     int i=base+b;
     if(ch=='a'){A[i]=add(B[i],C[i]); B[i]=C[i]=0;}
     else if(ch=='b'){B[i]=add(A[i],C[i]); A[i]=C[i]=0;}
     else {C[i]=add(A[i],B[i]); A[i]=B[i]=0;}
    }
   }
  }
 }
 vector<int> ab(size),ac(size),bc(size),sa(W),sb(W),sc(W);
 for(int a=0;a<=m;++a) for(int b=0;b<=m-a;++b){
  int c=m-a-b, v=add(add(A[a*W+b],B[a*W+b]),C[a*W+b]);
  ab[a*W+b]=v; ac[a*W+c]=v; bc[b*W+c]=v;
  sa[a]=add(sa[a],v); sb[b]=add(sb[b],v); sc[c]=add(sc[c],v);
 }
 for(int i=m-1;i>=0;--i){sa[i]=add(sa[i],sa[i+1]); sb[i]=add(sb[i],sb[i+1]); sc[i]=add(sc[i],sc[i+1]);}
 auto suffix=[&](vector<int>& p){
  for(int a=m;a>=0;--a){
   int row=0;
   for(int b=m;b>=0;--b){
    row=add(row,p[a*W+b]);
    p[a*W+b]=add(row,p[(a+1)*W+b]);
   }
  }
 };
 suffix(ab); suffix(ac); suffix(bc);
 int total=sa[0];
 while(Q--){
  int x,y,z; cin>>x>>y>>z;
  x=min(x+1,m+1); y=min(y+1,m+1); z=min(z+1,m+1);
  long long ans=(long long)total-sa[x]-sb[y]-sc[z]+ab[x*W+y]+ac[x*W+z]+bc[y*W+z];
  ans%=MOD; if(ans<0)ans+=MOD;
  cout<<ans<<'\n';
 }
}

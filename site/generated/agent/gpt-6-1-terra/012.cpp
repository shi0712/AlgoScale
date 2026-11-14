#include <bits/stdc++.h>
using namespace std;
using ll=long long;
using ull=unsigned long long;
int main(){
 ios::sync_with_stdio(false); cin.tie(nullptr);
 int n,P;cin>>n>>P;
 // Removing the first vertex gives the reversal/complement of a
 // stack-sorting output on n-1 elements.
 --n;
 if(n==0){cout<<1<<'\n';return 0;}
 vector<int> inv(n+2); inv[1]=1;
 for(int i=2;i<=n;i++) inv[i]=P-(ll)(P/i)*inv[P%i]%P;
 // Let A(x) be the EGF of stack-sorting outputs. Its associated
 // bivariate series G obeys
 //   y G_x = (1-y)(1+y A)G - 1,   G(x,0)=1,
 // and [y]G = 1-A. We store ordinary x coefficients of G and A.
 vector<vector<int>> g(n+1); vector<int> a(n+1); a[0]=1;
 g[0]={1}; g[1]={0,P-1}; a[1]=1;
 vector<ull> h(n+3);
 for(int t=1;t<n;t++){
   // Only coefficients that can still reach [x^n y] are needed.
   int lim=min(t+1,n-t);
   fill(h.begin(),h.begin()+lim+2,0);
   int batch=0;
   for(int k=0;k<=t;k++){
      int c=a[k]; const auto &row=g[t-k];
      int sz=min((int)row.size(),lim+2);
      if(c) for(int j=0;j<sz;j++) h[j]+=(ull)c*row[j];
      if(++batch==8){for(int j=0;j<=lim+1;j++) h[j]%=P;batch=0;}
   }
   for(int j=0;j<=lim+1;j++) h[j]%=P;
   const auto &r=g[t];
   g[t+1].assign(lim+1,0);
   for(int j=1;j<=lim;j++){
      ll v=(j+1<(int)r.size()?r[j+1]:0)-(ll)(j<(int)r.size()?r[j]:0)+(ll)h[j]-(ll)h[j-1];
      v%=P; if(v<0)v+=P;
      g[t+1][j]=v*inv[t+1]%P;
   }
   a[t+1]=g[t+1][1]?P-g[t+1][1]:0;
 }
 ll ans=a[n];for(int i=2;i<=n;i++)ans=ans*i%P;
 cout<<ans<<'\n';
}

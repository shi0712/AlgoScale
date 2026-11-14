#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int main(){int T;scanf("%d",&T);while(T--){int n,m;scanf("%d %d",&n,&m);vector<ll>a(n+2,0);for(int i=1;i<=n;i++)scanf("%lld",&a[i]);
 ll M=*max_element(a.begin()+1,a.begin()+n+1), mn=*min_element(a.begin()+1,a.begin()+n+1);
 ll base=M-a[1];for(int i=2;i<=n;i++)base+=max(0LL,a[i-1]-a[i]);
 if(m==1||M==mn){printf("%lld\n",base);continue;}
 ll V=M-mn; ll best=0;
 vector<int> ord(n);iota(ord.begin(),ord.end(),1);sort(ord.begin(),ord.end(),[&](int x,int y){return a[x]<a[y];});
 vector<ll>Q(n+2),P(n+2); vector<array<ll,4>> S(n+2); vector<int> lof(n+2),rof(n+2); vector<char> fr(n+2);
 for(ll D=1;D<=V+1;D++){
  Q[1]=D; for(int j=2;j<=n;j++)Q[j]=min(D,max(0LL,a[j-1]-a[j]));
  P[n+1]=D; for(int i=2;i<=n;i++)P[i]=min(D,max(0LL,a[i]-a[i-1]));
  fill(fr.begin(),fr.end(),0);
  ll H=0;
  auto add=[&](int x){
   // S index: l*2+r
   array<ll,4> s; s[0]=min(0LL,D-Q[x]-P[x+1]); s[2]=-P[x+1]; s[1]=D-Q[x]; s[3]=0;
   int L=x,R=x;
   auto merge=[&](const array<ll,4>&A,const array<ll,4>&B){array<ll,4> C; for(int l=0;l<2;l++)for(int r=0;r<2;r++)C[l*2+r]=min(A[l*2+0]+B[0*2+r],A[l*2+1]+B[1*2+r]);return C;};
   if(x>1&&fr[x-1]){int L1=lof[x-1]; H-=S[L1][0]; s=merge(S[L1],s); L=L1;}
   if(x<n&&fr[x+1]){int R2=rof[x+1]; H-=S[x+1][0]; s=merge(s,S[x+1]); R=R2;}
   fr[x]=1; S[L]=s; rof[L]=R; lof[R]=L; H+=s[0];
  };
  ll X0=M-D; size_t p=0;
  while(p<ord.size()&&a[ord[p]]<=X0)add(ord[p++]);
  best=min(best,H);
  while(p<ord.size()&&a[ord[p]]<M){ll v=a[ord[p]]; while(p<ord.size()&&a[ord[p]]==v)add(ord[p++]); best=min(best,v+D-M+H);}
 }
 printf("%lld\n",base+best);
}}

#include <bits/stdc++.h>
using namespace std;
static int n; static vector<char> alv; static int mode_=0;
static long long cntMult(long long a){long long c=0;for(long long m=a;m<=(long long)n;m+=a)if(alv[(size_t)m])++c;return c;}
static long long readR(){long long r;if(!(cin>>r))exit(0);if(r<0)exit(0);return r;}
static int qA(long long a){long long pred=cntMult(a);cout<<"A "<<a<<"\n";cout.flush();long long r=readR();return r>pred?1:0;}
static int qB(long long a){long long pred=cntMult(a);cout<<"B "<<a<<"\n";cout.flush();long long r=readR();
 for(long long m=a;m<=(long long)n;m+=a)if(alv[(size_t)m])alv[(size_t)m]=0;
 if(mode_==0&&pred>=2)mode_=(r==pred)?1:2;
 if(mode_==2)return r==1?1:0; return r>pred?1:0;}
static void ans(long long x){cout<<"C "<<x<<"\n";cout.flush();exit(0);}
static long long pw(long long p,int e){long long v=1;while(e-->0)v*=p;return v;}
static int gallop(long long p,long long lim){int K=0;long long pk=1;while(pk<=lim/p){pk*=p;++K;}if(K<=1)return 1;
 int lo=1,hi=K+1,step=2;
 while(step<=K){if(qA(pw(p,step))){lo=step;step<<=1;}else{hi=step;break;}}
 while(hi-lo>1){int mid=lo+(hi-lo)/2;if(qA(pw(p,mid)))lo=mid;else hi=mid;} return lo;}
int main(){ios::sync_with_stdio(false);cin.tie(nullptr);
 if(!(cin>>n))return 0; if(n<=1)ans(1); alv.assign(n+1,1);
 vector<int> primes;{vector<char> comp(n+1,0);for(int i=2;i<=n;++i)if(!comp[i]){primes.push_back(i);
  if((long long)i*i<=n)for(long long j=(long long)i*i;j<=n;j+=i)comp[(size_t)j]=1;}}
 vector<int> small,large;for(int p:primes){if((long long)p*p<=n)small.push_back(p);else large.push_back(p);}
 vector<int> S;for(int p:small)if(qB(p))S.push_back(p);
 bool dead=false; if(!small.empty())dead=(qA(1)==1);
 if(!dead){int m=(int)large.size(); if(m==0)ans(1);
  if(mode_==2){for(int q:large)if(qB(q))ans(q);ans(1);}
  long long bestC=m+20,bestCost=LLONG_MAX;
  for(long long C=m+1;C<=m+40;++C){long long en=0,b=0,mc=0;bool ok=true;
   while(en<m){++b;long long kb=(C-b+1-en)/2;if(kb<1)kb=1;long long e2=min((long long)m,en+kb);
    mc=max(mc,e2+(e2-en)+b-1);en=e2;if(b>60){ok=false;break;}}
   mc=max(mc,(long long)m+b); if(ok&&mc<bestCost){bestCost=mc;bestC=C;}}
  long long C=bestC;int end=0,b=0;
  while(end<m){++b;long long kb=(C-b+1-end)/2;if(kb<1)kb=1;int st=end,en=(int)min((long long)m,end+kb);
   for(int i=st;i<en;++i)qB(large[i]); end=en;
   if(qA(1)){for(int i=st;i+1<en;++i)if(qA(large[i]))ans(large[i]);ans(large[en-1]);}}
  ans(1);}
 long long s=1;
 if(mode_==1){vector<int> cand;
  if(!S.empty()){for(int p:small)if(p<S[0])cand.push_back(p);}else cand=small;
  int p0=cand.empty()?2:cand.back();
  for(size_t i=0;i+1<cand.size();++i)if(qA(cand[i])){p0=cand[i];break;}
  vector<int> P=S;P.push_back(p0);sort(P.begin(),P.end());
  long long det=1,rest=1;for(int p:P)rest*=p;
  for(int p:P){rest/=p;long long lim=(long long)n/(det*rest);int e=gallop(p,lim);det*=pw(p,e);} s=det;
 }else{vector<char> inS(n+1,0);for(int p:S)inS[p]=1;
  for(int p:small){if((long long)p>(long long)n/s)break;bool d=inS[p]||qA(p);
   if(d){int e=gallop(p,(long long)n/s);s*=pw(p,e);}}}
 long long lim=(long long)n/s;
 for(int q:large){if((long long)q>lim)break;if(qA(q))ans(s*q);}
 ans(s);return 0;}

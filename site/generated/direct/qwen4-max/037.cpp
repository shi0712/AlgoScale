#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
static inline int gc(){static char buf[1<<16];static int len=0,pos=0;if(pos==len){len=(int)fread(buf,1,sizeof(buf),stdin);pos=0;if(len<=0)return -1;}return (unsigned char)buf[pos++];}
static inline ll readLL(){int c=gc();while(c!=-1&&(c<'0'||c>'9'))c=gc();ll x=0;while(c>='0'&&c<='9'){x=x*10+(c-'0');c=gc();}return x;}
static inline void writeNum(string&s,ll x){char t[24];int k=0;if(x==0)t[k++]='0';while(x>0){t[k++]=char('0'+(int)(x%10));x/=10;}while(k)s.push_back(t[--k]);}
static vector<int> PL,PR,PC,PRF,freeList; static vector<ull> PH,Rnd;
static inline int newNode(){if(!freeList.empty()){int x=freeList.back();freeList.pop_back();return x;}PL.push_back(0);PR.push_back(0);PC.push_back(0);PRF.push_back(0);PH.push_back(0);return (int)PL.size()-1;}
static inline void incRef(int x){if(x)++PRF[x];}
static void releaseNode(int x){while(x){if(--PRF[x]!=0)return;int l=PL[x],r=PR[x];freeList.push_back(x);releaseNode(l);x=r;}}
static int insertVal(int old,int lo,int hi,int pos){int cur=newNode();PC[cur]=PC[old]+1;PH[cur]=PH[old]+Rnd[pos];PRF[cur]=1;
 if(lo==hi){PL[cur]=0;PR[cur]=0;return cur;} int mid=(lo+hi)>>1;
 if(pos<=mid){int keep=PR[old];int nl=insertVal(PL[old],lo,mid,pos);PL[cur]=nl;PR[cur]=keep;incRef(keep);}
 else{int keep=PL[old];int nr=insertVal(PR[old],mid+1,hi,pos);PL[cur]=keep;incRef(keep);PR[cur]=nr;} return cur;}
static bool isGreater(int A,int B,int pa,int pb,int V){ if(PH[A]+Rnd[pa]==PH[B]+Rnd[pb])return false;
 int lo=0,hi=V-1,a=A,b=B;
 while(lo<hi){int mid=(lo+hi)>>1;int ra=PR[a],rb=PR[b];
  ull ha=PH[ra]+((pa>mid&&pa<=hi)?Rnd[pa]:0ULL), hb=PH[rb]+((pb>mid&&pb<=hi)?Rnd[pb]:0ULL);
  if(ha!=hb){a=ra;b=rb;lo=mid+1;}else{a=PL[a];b=PL[b];hi=mid;}}
 return PC[a]+(pa==lo?1:0)>PC[b]+(pb==lo?1:0);}
static void collectVals(int x,int lo,int hi,const vector<ll>&vals,vector<ll>&out){if(!x)return;
 if(lo==hi){ll v=vals[lo];for(int k=0;k<PC[x];++k)out.push_back(v);return;} int mid=(lo+hi)>>1;
 collectVals(PL[x],lo,mid,vals,out);collectVals(PR[x],mid+1,hi,vals,out);}
static void solveChain(const vector<pair<ll,ll>>&ch,vector<ll>&out){int q=(int)ch.size();
 if(q==1){out.push_back(ch[0].second-ch[0].first+1);return;}
 vector<ll> b(q),g(q,0);
 for(int i=0;i+1<q;i++)g[i]=ch[i].second-ch[i+1].first+1;
 b[0]=ch[1].first-ch[0].first; b[q-1]=ch[q-1].second-ch[q-2].second;
 for(int i=1;i+1<q;i++)b[i]=ch[i+1].first-ch[i-1].second-1;
 if(q==2){if(b[0]>=b[1]){out.push_back(b[0]+g[0]);out.push_back(b[1]);}else{out.push_back(b[1]+g[0]);out.push_back(b[0]);}return;}
 vector<ll> vals; vals.reserve(4*q);
 for(int i=0;i<q;i++){ll gl=(i>0?g[i-1]:0),gr=g[i];vals.push_back(b[i]);if(gl)vals.push_back(b[i]+gl);if(gr)vals.push_back(b[i]+gr);if(gl&&gr)vals.push_back(b[i]+gl+gr);}
 sort(vals.begin(),vals.end()); vals.erase(unique(vals.begin(),vals.end()),vals.end());
 int V=(int)vals.size();
 while((int)Rnd.size()<V)Rnd.push_back((((ull)rand())<<32)^(ull)rand());
 auto idx=[&](ll v)->int{return (int)(lower_bound(vals.begin(),vals.end(),v)-vals.begin());};
 int dp0=insertVal(0,0,V-1,idx(b[0])), dp1=insertVal(0,0,V-1,idx(b[0]+g[0]));
 for(int i=1;i<q;i++){int nd0=0,nd1=0;
  for(int sp=0;sp<2;sp++){ll add=sp?g[i]:0;int pa=idx(b[i]+g[i-1]+add),pb=idx(b[i]+add);
   bool takeA=isGreater(dp0,dp1,pa,pb,V);int nn=insertVal(takeA?dp0:dp1,0,V-1,takeA?pa:pb);
   if(sp==0)nd0=nn;else nd1=nn;}
  releaseNode(dp0);releaseNode(dp1);dp0=nd0;dp1=nd1;}
 collectVals(dp0,0,V-1,vals,out); releaseNode(dp0);releaseNode(dp1);}
int main(){
 PL.push_back(0);PR.push_back(0);PC.push_back(0);PRF.push_back(0);PH.push_back(0);
 PL.reserve(1<<16);PR.reserve(1<<16);PC.reserve(1<<16);PRF.reserve(1<<16);PH.reserve(1<<16);
 mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count()^0xdeadbeefcafeULL);
 Rnd.resize(1<<20); for(auto&x:Rnd)x=rng();
 string outbuf; outbuf.reserve(1<<20);
 int t=(int)readLL();
 while(t--){
  int n=(int)readLL(),m=(int)readLL();
  vector<ll> L1(n),R1(n),L2(m),R2(m);
  for(int i=0;i<n;i++){L1[i]=readLL();R1[i]=readLL();}
  for(int j=0;j<m;j++){L2[j]=readLL();R2[j]=readLL();}
  vector<pair<ll,ll>> all; all.reserve(n+m);
  int i=0,j=0;
  while(i<n||j<m){
   if(j>=m||(i<n&&(L1[i]<L2[j]||(L1[i]==L2[j]&&R1[i]>=R2[j])))){all.emplace_back(L1[i],R1[i]);++i;}
   else{all.emplace_back(L2[j],R2[j]);++j;}}
  vector<ll> res; res.reserve(all.size());
  vector<pair<ll,ll>> chain; ll curMaxR=-1;
  for(auto&I:all){
   if(chain.empty()||I.first>curMaxR){
    if(!chain.empty())solveChain(chain,res);
    chain.clear(); chain.push_back(I); curMaxR=I.second;
   } else if(I.second>curMaxR){ chain.push_back(I); curMaxR=I.second; }
  }
  if(!chain.empty())solveChain(chain,res);
  sort(res.begin(),res.end(),greater<ll>());
  writeNum(outbuf,(ll)res.size()); outbuf.push_back('\n');
  for(size_t k=0;k<res.size();k++){if(k)outbuf.push_back(' ');writeNum(outbuf,res[k]);}
  outbuf.push_back('\n');
  if(outbuf.size()>(1<<20)){fwrite(outbuf.data(),1,outbuf.size(),stdout);outbuf.clear();}
 }
 fwrite(outbuf.data(),1,outbuf.size(),stdout);
 return 0;
}

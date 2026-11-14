#include <bits/stdc++.h>
#include <immintrin.h>
using namespace std;
using ll=long long;
const ll NEG=-(1LL<<60);
struct Player {ll e;int c;};

// All arrays have eight guard entries on either side of each row.
__attribute__((target("avx512f")))
void update512(ll* out,const ll* old,const ll* x,const ll* y,ll wx,ll wy,int first,int last,int mode){
 __m512i vx=_mm512_set1_epi64(wx),vy=_mm512_set1_epi64(wy);
 int i=first&~7;
 if(mode==3)for(;i<=last;i+=8){auto v=_mm512_loadu_si512(old+i);auto a=_mm512_add_epi64(_mm512_loadu_si512(x+i),vx);auto b=_mm512_add_epi64(_mm512_loadu_si512(y+i),vy);v=_mm512_max_epi64(v,_mm512_max_epi64(a,b));_mm512_storeu_si512(out+i,v);}
 else if(mode==1)for(;i<=last;i+=8){auto v=_mm512_max_epi64(_mm512_loadu_si512(old+i),_mm512_add_epi64(_mm512_loadu_si512(x+i),vx));_mm512_storeu_si512(out+i,v);}
 else for(;i<=last;i+=8){auto v=_mm512_max_epi64(_mm512_loadu_si512(old+i),_mm512_add_epi64(_mm512_loadu_si512(y+i),vy));_mm512_storeu_si512(out+i,v);}
}
template<int Mode>
__attribute__((target("avx512f")))
void rows512(ll* out,const ll* old,const ll* x,const ll* y,const size_t* off,int low,int high,int c,ll wx,ll wy,int juniors,int seniors){
 auto vx=_mm512_set1_epi64(wx),vy=_mm512_set1_epi64(wy);
 for(int k=low;k<=high;k++){
  size_t at=off[k],before=off[k-1];
  const ll *xx=x+before-c,*yy=y+before-c-1;
  int first=max(0,2*k-seniors)&~7,last=min(2*k,juniors);
  for(int t=first;t<=last;t+=8){
   auto v=_mm512_loadu_si512(old+at+t);
   if constexpr(Mode&1)v=_mm512_max_epi64(v,_mm512_add_epi64(_mm512_loadu_si512(xx+t),vx));
   if constexpr(Mode&2)v=_mm512_max_epi64(v,_mm512_add_epi64(_mm512_loadu_si512(yy+t),vy));
   _mm512_storeu_si512(out+at+t,v);
  }
 }
}
__attribute__((target("avx2")))
void update256(ll* out,const ll* old,const ll* x,const ll* y,ll wx,ll wy,int first,int last,int mode){
 auto vx=_mm256_set1_epi64x(wx),vy=_mm256_set1_epi64x(wy);
 for(int i=first&~3;i<=last;i+=4){auto v=_mm256_loadu_si256((const __m256i*)(old+i));if(mode&1){auto z=_mm256_add_epi64(_mm256_loadu_si256((const __m256i*)(x+i)),vx);v=_mm256_blendv_epi8(v,z,_mm256_cmpgt_epi64(z,v));}if(mode&2){auto z=_mm256_add_epi64(_mm256_loadu_si256((const __m256i*)(y+i)),vy);v=_mm256_blendv_epi8(v,z,_mm256_cmpgt_epi64(z,v));}_mm256_storeu_si256((__m256i*)(out+i),v);}
}
void updateScalar(ll* out,const ll* old,const ll* x,const ll* y,ll wx,ll wy,int first,int last,int mode){
 for(int i=first;i<=last;i++){ll v=old[i];if(mode&1)v=max(v,x[i]+wx);if(mode&2)v=max(v,y[i]+wy);out[i]=v;}
}

int main(){
 ios::sync_with_stdio(false);cin.tie(nullptr);
 int n,m;ll d;if(!(cin>>n>>m>>d))return 0;
 vector<ll>l(n);for(auto&x:l)cin>>x;sort(l.begin(),l.end());
 vector<Player>a;for(int i=0;i<m;i++){ll e;int t;cin>>e>>t;if(e<=l.back())a.push_back({e,t==1});}
 sort(a.begin(),a.end(),[](Player x,Player y){return x.e<y.e;});m=a.size();
 // A player without any compatible neighbor can never be selected.
 vector<Player> usable;usable.reserve(m);
 for(int i=0;i<m;i++)if((i&&a[i].e-a[i-1].e<=d)||(i+1<m&&a[i+1].e-a[i].e<=d))usable.push_back(a[i]);
 a.swap(usable);m=a.size();
 int K=2*n;
 auto impossible=[&](){for(int t=0;t<=K;t++)cout<<-1<<(t==K?'\n':' ');};
 // Greedily taking the largest possible last pair also tests feasibility.
 int pos=m-1;for(int k=n-1;k>=0;k--){while(pos>=0&&a[pos].e>l[k])pos--;while(pos>=1&&a[pos].e-a[pos-1].e>d)pos--;if(pos<1){impossible();return 0;}pos-=2;}

 // First solve the relaxation without the difference restriction. Its bases
 // form a nested-capacity matroid. If its optimal bases satisfy the original
 // restriction, they certify all answers immediately.
 vector<int>cat[2],parent[2],cap[2];
 for(int i=0;i<m;i++)cat[a[i].c].push_back(i);
 for(int c=0;c<2;c++){
  parent[c].resize(cat[c].size()+1);iota(parent[c].begin(),parent[c].end(),0);cap[c].resize(K);
  int j=0;for(int k=0;k<K;k++){while(j<(int)cat[c].size()&&a[cat[c][j]].e<=l[k/2])j++;cap[c][k]=j;}
 }
 auto findp=[&](int c,int x){int r=x;while(parent[c][r]!=r)r=parent[c][r];while(parent[c][x]!=x){int z=parent[c][x];parent[c][x]=r;x=z;}return r;};
 vector<int>b;ll sum=0;int countJ=0;
 for(int k=0;k<K;k++){
  int s=findp(0,cap[0][k]),j=findp(1,cap[1][k]);int c=(s==0);
  int x=c?j:s;int id=cat[c][x-1];b.push_back(id);sum+=a[id].e;countJ+=c;parent[c][x]=findp(c,x-1);
 }
 sort(b.begin(),b.end());vector<ll>relax(K+1,-1);bool allValid=true;
 vector<unsigned char>selected(m,0);for(int x:b)selected[x]=1;
 while(true){
  bool ok=true;for(int k=0;k<n;k++)if(a[b[2*k+1]].e-a[b[2*k]].e>d){ok=false;break;}
  if(!ok){allValid=false;break;}relax[countJ]=sum;
  // A new junior can replace a senior above the last tight prefix below it.
  vector<pair<ll,int>>tight;int j=0;
  for(int k=0;k<n;){int h=k;while(h<n&&l[h]==l[k])h++;while(j<K&&a[b[j]].e<=l[k])j++;if(j==2*h)tight.push_back({l[k],j});k=h;}
  vector<int>minSenior(K+1,-1);for(int k=K-1;k>=0;k--){minSenior[k]=minSenior[k+1];if(a[b[k]].c==0)minSenior[k]=b[k];}
  int ti=0,lo=0,add=-1,remove=-1;ll gain=NEG;
  for(int x:cat[1])if(!selected[x]){
   while(ti<(int)tight.size()&&tight[ti].first<a[x].e)lo=tight[ti++].second;
   if(lo==K||minSenior[lo]<0)continue;
   int y=minSenior[lo];ll g=a[x].e-a[y].e;
   if(g>gain){gain=g;add=x;remove=y;}
  }
  if(add<0)break;
  selected[add]=1;selected[remove]=0;*find(b.begin(),b.end(),remove)=add;sort(b.begin(),b.end());sum+=gain;countJ++;
 }
 if(allValid){for(int t=0;t<=K;t++)cout<<relax[t]<<(t==K?'\n':' ');return 0;}

 // F[k][t] at a prefix: maximum sum using k pairs and t juniors.
 // For a fixed right endpoint and partner category, the most recent possible
 // partner dominates every earlier one. Consequently only the current prefix
 // and the two prefixes immediately before the last player of each category
 // need to be retained.
 vector<size_t>off(n+1);size_t total=0;
 for(int k=0;k<=n;k++){off[k]=total+8;total+=((2*k+1+7)/8)*8+16;}
 vector<ll>buf[4];for(auto&v:buf)v.assign(total,NEG);
 vector<ll>finished(total,NEG);
 for(auto&v:buf)v[off[0]]=0;
 finished[off[0]]=0;
 int cur=0,snapshot[2]={0,0};int last[2]={-1,-1};int juniors=0,seniors=0;
 int frozen=0;
 bool wide=false;
 auto update=updateScalar;
 #if defined(__x86_64__)
 if(__builtin_cpu_supports("avx512f")){update=update512;wide=true;}
 else if(__builtin_cpu_supports("avx2"))update=update256;
 #endif
 for(int i=0;i<m;i++){
  while(frozen<n&&l[frozen]<a[i].e){int k=++frozen;copy_n(buf[cur].data()+off[k],2*k+1,finished.data()+off[k]);}
  int c=a[i].c;if(c)juniors++;else seniors++;
  int mode=0;ll weight[2]={0,0};for(int h=0;h<2;h++)if(last[h]>=0&&a[i].e-a[last[h]].e<=d){mode|=1<<h;weight[h]=a[i].e+a[last[h]].e;}
  int upper=min(n,(i+1)/2);
  if(mode&&upper>frozen){
   int out=0;while(out==cur||out==snapshot[0]||out==snapshot[1])out++;
   int rowEnd=wide?frozen+1:upper;
   for(int k=frozen+1;k<=rowEnd;k++){
    const ll *src[2]={nullptr,nullptr};
    for(int h=0;h<2;h++)if(mode&(1<<h)){
     bool fixed=k==1||(last[h]>=0&&a[last[h]].e>l[k-2]);
     src[h]=(fixed?finished.data():buf[snapshot[h]].data())+off[k-1]-c-h;
    }
    int first=max(0,2*k-seniors),lastT=min(2*k,juniors);
    if(first<=lastT)update(buf[out].data()+off[k],buf[cur].data()+off[k],src[0],src[1],weight[0],weight[1],first,lastT,mode);
   }
   if(wide&&frozen+2<=upper){
    auto args=make_tuple(buf[out].data(),buf[cur].data(),buf[snapshot[0]].data(),buf[snapshot[1]].data(),off.data(),frozen+2,upper,c,weight[0],weight[1],juniors,seniors);
    if(mode==3)apply(rows512<3>,args);
    else if(mode==1)apply(rows512<1>,args);
    else apply(rows512<2>,args);
   }
   snapshot[c]=cur;cur=out;
  }else snapshot[c]=cur;
  last[c]=i;
 }
 const ll*ans=(frozen==n?finished.data():buf[cur].data())+off[n];
 for(int t=0;t<=K;t++)cout<<(ans[t]<0?-1:ans[t])<<(t==K?'\n':' ');
}

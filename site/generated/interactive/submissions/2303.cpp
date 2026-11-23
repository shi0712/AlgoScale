#include <bits/stdc++.h>
using namespace std;

typedef array<unsigned long long,4> Set;

static int n;
static map<Set,int> memo;
static Set S;
static int sz, q;
static vector<int> ordv;
static vector<Set> segs;
static vector<int> slo, shi;
static mt19937 rng;

static inline void setb(Set &s,int i){ s[i>>6] |= 1ULL<<(i&63); }
static inline bool getb(const Set &s,int i){ return (s[i>>6]>>(i&63))&1ULL; }
static inline int pc(const Set &s){
  return __builtin_popcountll(s[0])+__builtin_popcountll(s[1])+__builtin_popcountll(s[2])+__builtin_popcountll(s[3]);
}
static inline Set dif(const Set &a,const Set &b){ Set r{}; for(int i=0;i<4;i++) r[i]=a[i]&~b[i]; return r; }
static inline Set uni(const Set &a,const Set &b){ Set r{}; for(int i=0;i<4;i++) r[i]=a[i]|b[i]; return r; }

static int ask(const Set &s){
  int c=pc(s);
  if(c==0||c==n) return 0;
  auto it=memo.find(s);
  if(it!=memo.end()) return it->second;
  string t(n,'0');
  for(int i=0;i<n;i++) if(getb(s,i)) t[i]='1';
  cout<<"? "<<t<<'\n'; cout.flush();
  int x;
  if(!(cin>>x)) exit(0);
  if(x<0) exit(0);
  memo.emplace(s,x);
  if(x==0){ cout<<"! 0\n"; cout.flush(); exit(0); }
  return x;
}

static void build(int id,int lo,int hi){
  slo[id]=lo; shi[id]=hi;
  Set st{};
  for(int i=lo;i<hi;i++) setb(st,ordv[i]);
  segs[id]=st;
  if(hi-lo>1){ int mid=(lo+hi)>>1; build(id<<1,lo,mid); build(id<<1|1,mid,hi); }
}

static void collect(int id,int depth,int d,vector<int>&out){
  if(depth>=d || shi[id]-slo[id]==1){ out.push_back(id); return; }
  collect(id<<1,depth+1,d,out); collect(id<<1|1,depth+1,d,out);
}

static void descend(int id){
  while(shi[id]-slo[id]>1){
    int c1=id<<1, c2=c1|1;
    if(rng()&1u) swap(c1,c2);
    Set T=dif(segs[c1],S);
    int t=pc(T);
    if(t>0){
      int v1=ask(uni(S,segs[c1]));
      if(v1==q-t){ S=uni(S,T); sz+=t; q=v1; return; }
      if(v1<q || q+ask(T)-v1>0){ id=c1; continue; }
    }
    id=c2;
  }
  int v=ordv[slo[id]];
  Set S2=S; setb(S2,v);
  int v1=ask(S2);
  S=S2; sz++; q=v1;
}

int main(){
  ios::sync_with_stdio(false);
  cin.tie(nullptr);
  if(!(cin>>n)) return 0;
  if(n<=1){ cout<<"! 1\n"; cout.flush(); return 0; }
  rng.seed((unsigned)chrono::steady_clock::now().time_since_epoch().count());
  ordv.resize(n);
  iota(ordv.begin(),ordv.end(),0);
  shuffle(ordv.begin(),ordv.end(),rng);
  segs.assign(4*n+4,Set{});
  slo.assign(4*n+4,0);
  shi.assign(4*n+4,0);
  build(1,0,n);
  S=Set{}; setb(S,ordv[0]); sz=1;
  q=ask(S);
  for(;;){
    if(q==0){ cout<<"! "<<(sz==n?1:0)<<'\n'; cout.flush(); return 0; }
    if(q==n-sz){ cout<<"! 1\n"; cout.flush(); return 0; }
    int d=0;
    while((1<<(d+1))<=q && (1<<(d+1))<n) ++d;
    vector<int> nodes;
    collect(1,0,d,nodes);
    shuffle(nodes.begin(),nodes.end(),rng);
    bool progressed=false;
    for(int id: nodes){
      Set T=dif(segs[id],S);
      int t=pc(T);
      if(t==0) continue;
      int v1=ask(uni(S,segs[id]));
      if(v1==q-t){ S=uni(S,T); sz+=t; q=v1; progressed=true; break; }
      if(v1<q || q+ask(T)-v1>0){ descend(id); progressed=true; break; }
    }
    if(!progressed) descend(1);
  }
}

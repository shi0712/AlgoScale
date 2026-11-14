#include <bits/stdc++.h>
using namespace std;
using U = uint32_t; using L = uint64_t;
struct Key { L a,b; bool operator<(const Key& o) const {return a!=o.a ? a<o.a : b<o.b;} bool operator==(const Key& o) const {return a==o.a && b==o.b;} };
static inline L mix(L x) { x += 0x9e3779b97f4a7c15ULL; x=(x^(x>>30))*0xbf58476d1ce4e5b9ULL; x=(x^(x>>27))*0x94d049bb133111ebULL; return x^(x>>31); }
static const L seed = chrono::steady_clock::now().time_since_epoch().count();
static inline Key h(int p, U x) { L v=(((L)p<<32)|x)^seed; return {mix(v),mix(v^0xd6e8feb86659fd93ULL)}; }
int main(){
 ios::sync_with_stdio(false); cin.tie(nullptr);
 int T; cin>>T;
 while(T--){
  int n; cin>>n; vector<Key> all; all.reserve((size_t)n*33);
  U lb[32]={}; int pos[32]={};
  for(int r=1;r<=n;r++){
   uint64_t input; cin>>input; U x=(U)input; int at=r;
   for(int b=31;b>=0 && x;--b) if((x>>b)&1U){
    if(!lb[b]) {lb[b]=x;pos[b]=at;break;}
    if(pos[b]<at){swap(pos[b],at);swap(lb[b],x);} x^=lb[b];
   }
   pair<int,U> order[32]; int k=0;
   for(int b=0;b<32;b++) if(lb[b]) order[k++]={pos[b],lb[b]};
   sort(order,order+k,[](auto &a,auto &b){return a.first>b.first;});
   U bas[32]={}; Key hv[32]={}; Key cur{0,0}; U pivots=0;
   // A zero suffix contributes the zero subspace, but only if the last entry is zero.
   if(input==0) all.push_back(cur);
   for(int j=0;j<k;j++){
    U v=order[j].second;
    U remaining=v & pivots;
    while(remaining){ int b=31-__builtin_clz(remaining); v^=bas[b]; remaining=v & pivots & ((1U<<b)-1); }
    int p=31-__builtin_clz(v);
    for(int b=p+1;b<32;b++) if((bas[b]>>p)&1U){
      cur.a^=hv[b].a; cur.b^=hv[b].b;
      bas[b]^=v; hv[b]=h(b,bas[b]);
      cur.a^=hv[b].a; cur.b^=hv[b].b;
    }
    pivots |= 1U<<p; bas[p]=v; hv[p]=h(p,v); cur.a^=hv[p].a; cur.b^=hv[p].b;
    all.push_back(cur);
   }
  }
  sort(all.begin(),all.end()); auto e=unique(all.begin(),all.end()); cout<<(e-all.begin())<<'\n';
 }
}

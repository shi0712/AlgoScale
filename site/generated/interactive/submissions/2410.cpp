#include <bits/stdc++.h>
using namespace std;
// codes: 0=T1 1=T2 2=L1 3=L2 ; 4 = fresh
static inline int updc(int c,bool truth){
  if(c==4) return truth?0:2;
  if(truth){ if(c==0) return 1; if(c==1) return -1; return 0;}
  else { if(c==2) return 3; if(c==3) return -1; return 2;}
}
struct Endgame {
  int K;
  vector<vector<uint8_t>> lo, hi; // lo: known not solvable within lo ; hi: solvable within hi
  Endgame(int K_):K(K_){ lo.resize(K+1); hi.resize(K+1); for(int k=0;k<=K;k++){lo[k].assign(1u<<(2*k),0); hi[k].assign(1u<<(2*k),255);} }
  static pair<int,uint32_t> apply(int k,uint32_t s,int i,int j,bool yes){
    int nk=0; uint32_t ns=0;
    for(int p=0;p<k;p++){int c=(s>>(2*p))&3; bool in=(p>=i&&p<=j); bool truth= in?yes:!yes; int d=updc(c,truth); if(d>=0){ns|=(uint32_t)d<<(2*nk);nk++;}}
    return {nk,ns};
  }
  bool can(int k,uint32_t s,int d){
    if(k<=2) return true;
    if(d<=0) return false;
    if(hi[k][s]<=d) return true;
    if(lo[k][s]>=d) return false;
    bool ok=false;
    for(int i=0;i<k&&!ok;i++)for(int j=i;j<k&&!ok;j++){
      auto a=apply(k,s,i,j,true); auto b=apply(k,s,i,j,false);
      if(a.first==0||b.first==0) continue; // adversary must pick other; handle: if one empty, only other matters
      if(can(a.first,a.second,d-1)&&can(b.first,b.second,d-1)) ok=true;
    }
    // queries where one outcome kills all: adversary forced to the other
    for(int i=0;i<k&&!ok;i++)for(int j=i;j<k&&!ok;j++){
      auto a=apply(k,s,i,j,true); auto b=apply(k,s,i,j,false);
      if(a.first==0&&b.first==0) {ok=true;break;}
      if(a.first==0){ if(can(b.first,b.second,d-1)) ok=true;}
      else if(b.first==0){ if(can(a.first,a.second,d-1)) ok=true;}
    }
    if(ok) hi[k][s]=d; else lo[k][s]=d;
    return ok;
  }
  int value(int k,uint32_t s){ int d=0; while(!can(k,s,d)) d++; return d; }
  pair<int,int> choose(int k,uint32_t s){
    int d=value(k,s); int bs=1<<30; pair<int,int> bi{0,k-1};
    for(int i=0;i<k;i++)for(int j=i;j<k;j++){
      auto a=apply(k,s,i,j,true); auto b=apply(k,s,i,j,false);
      bool oa = a.first==0 || can(a.first,a.second,d-1);
      bool ob = b.first==0 || can(b.first,b.second,d-1);
      if(oa&&ob){ int sc=(a.first?value(a.first,a.second):0)+(b.first?value(b.first,b.second):0); if(sc<bs){bs=sc;bi={i,j};} }
    }
    return bi;
  }
};

struct Strategy {
  double a2; int K; Endgame *eg;
  vector<int> ids; vector<int> st;
  Strategy(int n,double a2_,int K_,Endgame*e):a2(a2_),K(K_),eg(e){ ids.resize(n); iota(ids.begin(),ids.end(),1); st.assign(n,4);}
  double w(int c){ if(c==4) return 2.0/1.618; return (c&1)?a2:1.0; }
  double gT(int c){ int d=updc(c,true); return d<0?0:w(d);} 
  double gL(int c){ int d=updc(c,false); return d<0?0:w(d);} 
  // returns interval in candidate index [i,j]
  pair<int,int> choose(){
    int k=ids.size();
    if(k<=K){ uint32_t s=0; bool fresh=false; for(int p=0;p<k;p++){ if(st[p]==4) fresh=true; s|=(uint32_t)st[p]<<(2*p);} 
      if(!fresh) return eg->choose(k,s);
      // fresh small: brute
      int best=1e9; pair<int,int> bi{0,0};
      for(int i=0;i<k;i++)for(int j=i;j<k;j++){ uint32_t a=0,b=0; for(int p=0;p<k;p++){bool in=p>=i&&p<=j; a|=(uint32_t)(in?0:2)<<(2*p); b|=(uint32_t)(in?2:0)<<(2*p);} int v=max(eg->value(k,a),eg->value(k,b)); if(v<best){best=v;bi={i,j};}}
      return bi;
    }
    // weight balancing
    double GT=0,GL=0; vector<double> P(k+1,0);
    for(int p=0;p<k;p++){ double t=gT(st[p]),l=gL(st[p]); GT+=t; GL+=l; P[p+1]=P[p]+(t-l);} 
    // W_yes = GL + sum_S d ; W_no = GT - sum_S d ; target sum_S d = (GT-GL)/2
    double target=(GT-GL)/2;
    // for each j, find i-1 with P[i-1] closest to P[j+1]-target, i-1<=j
    set<pair<double,int>> S; double bestv=1e18; pair<int,int> bi{0,0};
    for(int j=0;j<k;j++){ S.insert({P[j],j});
      double want=P[j+1]-target; auto it=S.lower_bound({want,-1});
      for(int z=0;z<2;z++){ if(z==1){ if(it==S.begin()) break; --it;} else if(it==S.end()) continue;
        double sd=P[j+1]-it->first; double v=max(GL+sd,GT-sd); if(v<bestv-1e-12){bestv=v;bi={it->second,j};}}
    }
    return bi;
  }
  void apply(int i,int j,bool yes){ vector<int> ni,ns; for(int p=0;p<(int)ids.size();p++){ bool in=p>=i&&p<=j; int d=updc(st[p],in?yes:!yes); if(d>=0){ni.push_back(ids[p]);ns.push_back(d);} } ids=ni; st=ns; }
};

int main(){
  int n; if(!(cin>>n)) return 0;
  Endgame eg(8); Strategy S(n,0.618,8,&eg);
  while(S.ids.size()>2){
    auto [i,j]=S.choose(); int l=S.ids[i], r=S.ids[j];
    printf("? %d %d\n",l,r); fflush(stdout);
    long long c; if(!(cin>>c)) return 0; if(c<0) return 0;
    bool yes = (c==r-l);
    S.apply(i,j,yes);
  }
  for(int id:S.ids){ printf("! %d\n",id); fflush(stdout); int x; if(!(cin>>x)) return 0; if(x<0) return 0; if(x==1) break; }
  printf("#\n"); fflush(stdout);
  return 0;
}

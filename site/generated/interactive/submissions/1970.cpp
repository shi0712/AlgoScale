#include <bits/stdc++.h>
#include <chrono>
using namespace std;

static const int DR[4]={-1,0,1,0}, DC[4]={0,1,0,-1};

/* ----------------------------- DFA minimization ----------------------------- */
struct Hopcroft {
  int n;
  vector<int> *lft,*rgt,*fwd,*lb;
  vector<int> revL, revR;
  vector<vector<int>> revF;
  vector<vector<int>> members;
  vector<int> blockOf, pos;
  vector<char> inQueue;
  vector<vector<int>> tmp;
  vector<int> touched;
  vector<int> mark; int token=0;
  deque<int> q;

  void init(int n_, vector<int>& lb_, vector<int>& l_, vector<int>& r_, vector<int>& f_){
     n=n_; lb=&lb_; lft=&l_; rgt=&r_; fwd=&f_;
     revL.assign(n,-1);revR.assign(n,-1);revF.assign(n,{});
     for(int s=0;s<n;s++){revL[(*lft)[s]]=s; revR[(*rgt)[s]]=s; revF[(*fwd)[s]].push_back(s);}
     blockOf.assign(n,-1); pos.assign(n,-1); mark.assign(n,0);
     vector<int> labBlock(128,-1);
     for(int s=0;s<n;s++){
        int L=(*lb)[s];
        if(labBlock[L]==-1){ labBlock[L]=members.size(); members.push_back({}); }
        int b=labBlock[L]; blockOf[s]=b; pos[s]=members[b].size(); members[b].push_back(s);
     }
     int nb=members.size();
     inQueue.assign(nb,1);
     for(int b=0;b<nb;b++) q.push_back(b);
     tmp.assign(n+1, {});
  }
  void split(int Y, vector<int>& S){
     int ysize=members[Y].size(); int ssize=S.size();
     int nb=members.size();
     vector<int> moved;
     if(ssize <= ysize-ssize){ moved.assign(S.begin(),S.end()); }
     else {
        ++token;
        for(int p:S) mark[p]=token;
        moved.reserve(ysize-ssize);
        for(int p:members[Y]) if(mark[p]!=token) moved.push_back(p);
     }
     for(int p:moved){
        int idx=pos[p];
        int last=members[Y].back();
        members[Y][idx]=last; pos[last]=idx;
        members[Y].pop_back();
     }
     members.push_back(moved);
     for(int k=0;k<(int)moved.size();k++){blockOf[moved[k]]=nb; pos[moved[k]]=k;}
     inQueue.push_back(0);
     bool was = inQueue[Y];
     if(was){ inQueue[nb]=1; q.push_back(nb); }
     else {
        if(moved.size() <= members[Y].size()){ inQueue[nb]=1; q.push_back(nb); }
        else { inQueue[Y]=1; q.push_back(Y); }
     }
  }
  vector<int> run(){
     while(!q.empty()){
        int A=q.front(); q.pop_front();
        if(!inQueue[A]) continue;
        inQueue[A]=0;
        vector<int> Asnap = members[A];
        for(int c=0;c<3;c++){
           touched.clear();
           if(c<2){
              const vector<int>& rev = (c==0?revL:revR);
              for(int t: Asnap){
                 int p = rev[t];
                 if(p<0) continue;
                 int Y=blockOf[p];
                 if(tmp[Y].empty()) touched.push_back(Y);
                 tmp[Y].push_back(p);
              }
           } else {
              for(int t: Asnap){
                 for(int p: revF[t]){
                    int Y=blockOf[p];
                    if(tmp[Y].empty()) touched.push_back(Y);
                    tmp[Y].push_back(p);
                 }
              }
           }
           for(int Y: touched){
              auto &S=tmp[Y];
              if((int)S.size()==(int)members[Y].size()){ S.clear(); continue; }
              split(Y,S);
              S.clear();
           }
        }
     }
     map<int,int> mp;
     vector<int> out(n);
     for(int s=0;s<n;s++){ int b=blockOf[s]; if(!mp.count(b))mp[b]=mp.size(); out[s]=mp[b]; }
     return out;
  }
};

/* ------------------------------- The problem -------------------------------- */
struct Problem {
  int r,c;
  vector<string> g;
  vector<int> cid;       // open-cell index -> i*c+j
  vector<int> openIdx;   // i*c+j -> open-cell index or -1
  int nstates;
  vector<int> sp, sd, Lst, fwd, lft, rgt;
  int ncls;
  vector<int> clabel, csize, cmulti, cpos, cposcount;
  vector<array<int,3>> ct; // 0 left, 1 right, 2 forward

  void build(int r_, int c_, vector<string> g_){
    r=r_;c=c_;g=g_;
    openIdx.assign(r*c,-1); cid.clear();
    for(int i=0;i<r;i++)for(int j=0;j<c;j++)if(g[i][j]=='.'){openIdx[i*c+j]=cid.size();cid.push_back(i*c+j);}
    nstates=cid.size()*4;
    sp.assign(nstates,0);sd.assign(nstates,0);Lst.assign(nstates,0);
    for(int oi=0;oi<(int)cid.size();oi++){
      int i=cid[oi]/c,j=cid[oi]%c;
      for(int d=0;d<4;d++){
        int s=oi*4+d; sp[s]=oi; sd[s]=d;
        int k=0,x=i+DR[d],y=j+DC[d];
        while(x>=0&&x<r&&y>=0&&y<c&&g[x][y]=='.'){k++;x+=DR[d];y+=DC[d];}
        Lst[s]=k;
      }
    }
    fwd.assign(nstates,0);lft.assign(nstates,0);rgt.assign(nstates,0);
    for(int s=0;s<nstates;s++){
      int oi=sp[s],d=sd[s];
      lft[s]=oi*4+((d+3)&3);
      rgt[s]=oi*4+((d+1)&3);
      if(Lst[s]>0){
        int i=cid[oi]/c+DR[d], j=cid[oi]%c+DC[d];
        fwd[s]=openIdx[i*c+j]*4+d;
      } else fwd[s]=s;
    }
    if(nstates==0){ ncls=0; return; }
    Hopcroft hp; hp.init(nstates,Lst,lft,rgt,fwd);
    vector<int> blk=hp.run();
    ncls=*max_element(blk.begin(),blk.end())+1;
    clabel.assign(ncls,0); csize.assign(ncls,0); cmulti.assign(ncls,0); cpos.assign(ncls,-1);
    vector<int> firstpos(ncls,-1); vector<char> seen(ncls,0);
    vector<int> rep(ncls,-1);
    for(int s=0;s<nstates;s++){
      int b=blk[s]; csize[b]++;
      if(rep[b]<0) rep[b]=s;
      int cell=cid[sp[s]];
      if(!seen[b]){seen[b]=1; firstpos[b]=cell; cpos[b]=cell; clabel[b]=Lst[s];}
      else if(cell!=firstpos[b]) cmulti[b]=1;
    }
    // distinct positions per class
    vector<vector<int>> cellsOf(ncls);
    for(int s=0;s<nstates;s++) cellsOf[blk[s]].push_back(cid[sp[s]]);
    cposcount.assign(ncls,1);
    for(int b=0;b<ncls;b++){
      auto&v=cellsOf[b]; sort(v.begin(),v.end()); v.erase(unique(v.begin(),v.end()),v.end());
      cposcount[b]=v.size();
    }
    ct.assign(ncls,{0,0,0});
    for(int b=0;b<ncls;b++){
      int s=rep[b];
      ct[b][0]=blk[lft[s]];
      ct[b][1]=blk[rgt[s]];
      if(Lst[s]>0) ct[b][2]=blk[fwd[s]];
      else ct[b][2]=b;
    }
  }
};

/* ------------------------------- Exact solver ------------------------------- */
struct ExactSolver {
  const Problem* P;
  int ncls;
  vector<int> allB;
  vector<uint32_t> bOff, bLen;
  vector<int> W;
  vector<char> term;
  vector<uint32_t> eOff, eCnt;
  vector<pair<int,int>> edges;
  unordered_map<uint64_t,int> hmap;
  vector<int> nextHash;
  vector<double> values;

  void init(const Problem& p){ P=&p; ncls=p.ncls; }
  static uint64_t hashSeg(const int* a, int len){
    uint64_t h=1469598103934665603ULL;
    for(int i=0;i<len;i++){ h^=(uint64_t)(uint32_t)a[i]; h*=1099511628211ULL; }
    return h;
  }
  int addNode(const int* a, int len){
    int id=bLen.size();
    bOff.push_back((uint32_t)allB.size());
    bLen.push_back((uint32_t)len);
    allB.insert(allB.end(), a, a+len);
    W.push_back(0); term.push_back(0);
    eOff.push_back(0); eCnt.push_back(0);
    eOff.push_back(0); eCnt.push_back(0);
    eOff.push_back(0); eCnt.push_back(0);
    nextHash.push_back(-1);
    return id;
  }
  int findOrAdd(const int* a, int len, uint64_t h){
    auto it=hmap.find(h);
    int head = (it==hmap.end()? -1 : it->second);
    for(int id=head; id!=-1; id=nextHash[id]){
      if((int)bLen[id]==len && equal(a,a+len, allB.begin()+bOff[id])) return id;
    }
    int id=addNode(a,len);
    if(it==hmap.end()) hmap.emplace(h,id);
    else { nextHash[id]=it->second; it->second=id; }
    return id;
  }
  bool isTermSeg(const int* b, int sz, bool& isNo) const {
    if(sz==0){isNo=true;return true;}
    bool allMulti=true; int fp=-1; bool same=true;
    for(int i=0;i<sz;i++){ int C=b[i];
      if(P->cmulti[C]) same=false;
      else { allMulti=false; if(fp<0)fp=P->cpos[C]; else if(P->cpos[C]!=fp)same=false; }
    }
    if(allMulti){isNo=true;return true;}
    if(same&&fp>=0){isNo=false;return true;}
    return false;
  }
  bool build(const vector<int>& B0, long long cap, double timeLimit){
    auto t0=chrono::steady_clock::now();
    hmap.reserve((size_t)min<long long>(cap+10, 650010));
    hmap.max_load_factor(0.7f);
    findOrAdd(B0.data(),(int)B0.size(),hashSeg(B0.data(),(int)B0.size()));
    vector<int> tmp(ncls);
    size_t head=0;
    while(head<bLen.size()){
      if((long long)bLen.size()>cap) return false;
      if((head&255)==0){
        if(chrono::duration<double>(chrono::steady_clock::now()-t0).count()>timeLimit) return false;
        if((long long)allB.size()*4 + (long long)edges.size()*8 > 110LL*1024*1024) return false;
      }
      int u=head++;
      int sz=bLen[u];
      vector<int> Bcopy(allB.begin()+bOff[u], allB.begin()+bOff[u]+sz);
      const int* B=Bcopy.data();
      int w=0; for(int i=0;i<sz;i++) w+=P->csize[B[i]];
      W[u]=w;
      bool isNo;
      if(isTermSeg(B,sz,isNo)){ term[u]=1; continue; }
      int d=P->clabel[B[0]];
      for(int a=0;a<3;a++){
        if(a==2 && d==0) continue;
        eOff[u*3+a]=(uint32_t)edges.size();
        int hist[128]={0};
        for(int i=0;i<sz;i++){ int t=P->ct[B[i]][a]; hist[P->clabel[t]]++; }
        int start[128]; int acc=0;
        for(int L=0;L<128;L++){ start[L]=acc; acc+=hist[L]; }
        int posArr[128]; memcpy(posArr,start,sizeof(start));
        for(int i=0;i<sz;i++){ int C=B[i]; int t=P->ct[C][a]; int L=P->clabel[t]; tmp[posArr[L]++]=t; }
        for(int L=0;L<128;L++) if(hist[L]){
          const int* seg=tmp.data()+start[L]; int cnt=hist[L];
          uint64_t h=hashSeg(seg,cnt);
          int id=findOrAdd(seg,cnt,h);
          int pw=0; for(int i=0;i<cnt;i++) pw+=P->csize[seg[i]];
          edges.push_back({id,pw});
          eCnt[u*3+a]++;
        }
      }
    }
    return true;
  }
  bool solve(double timeLimit){
    auto t0=chrono::steady_clock::now();
    int n=bLen.size();
    values.assign(n,0.0);
    for(int i=0;i<n;i++) if(term[i])values[i]=1.0;
    for(int iter=0;iter<100000;iter++){
      double maxdiff=0;
      for(int i=0;i<n;i++){
        if(term[i])continue;
        double best=1e18;
        int w=W[i];
        for(int a=0;a<3;a++){
          int cnt=eCnt[i*3+a]; if(!cnt)continue;
          uint32_t st=eOff[i*3+a];
          double s=0;
          for(int k=0;k<cnt;k++){ auto&pr=edges[st+k]; s += (double)pr.second/w * values[pr.first]; }
          double val=1.0+s;
          if(val<best)best=val;
        }
        double diff=best-values[i]; if(diff<0)diff=-diff;
        if(diff>maxdiff)maxdiff=diff;
        values[i]=best;
      }
      if(maxdiff<1e-12) return true;
      if((iter&63)==0){
        if(chrono::duration<double>(chrono::steady_clock::now()-t0).count()>timeLimit) return false;
      }
    }
    return false;
  }
  int bestAction(int node) const {
    double best=1e18; int ba=-1;
    int w=W[node];
    for(int a=0;a<3;a++){
      int cnt=eCnt[node*3+a]; if(!cnt)continue;
      uint32_t st=eOff[node*3+a];
      double s=0;
      for(int k=0;k<cnt;k++){ auto&pr=edges[st+k]; s += (double)pr.second/w * values[pr.first]; }
      double val=1.0+s;
      if(val<best){best=val;ba=a;}
    }
    return ba;
  }
  int childForObs(int node, int a, int obs) const {
    int cnt=eCnt[node*3+a]; uint32_t st=eOff[node*3+a];
    for(int k=0;k<cnt;k++){
      int id=edges[st+k].first;
      if(P->clabel[allB[bOff[id]]]==obs) return id;
    }
    return -1;
  }
  void clearAll(){
    vector<int>().swap(allB); vector<uint32_t>().swap(bOff); vector<uint32_t>().swap(bLen);
    vector<int>().swap(W); vector<char>().swap(term); vector<uint32_t>().swap(eOff); vector<uint32_t>().swap(eCnt);
    vector<pair<int,int>>().swap(edges); unordered_map<uint64_t,int>().swap(hmap); vector<int>().swap(nextHash); vector<double>().swap(values);
  }
};

/* ------------------------------ Fallback policy ----------------------------- */
struct Policy {
  const Problem* P;
  int ncls;
  vector<int> clabel, csize, cmulti, cpos, cposcount;
  vector<array<int,3>> ct;
  vector<vector<int>> scratch;
  int H;
  long long nodeBudget;
  long long nodes;
  bool useTaboo;
  unordered_set<uint64_t> taboo;
  static constexpr double TABOO_VAL = 1e8;

  void init(const Problem& p, int h, long long budget){
    P=&p; ncls=p.ncls; H=h; nodeBudget=budget;
    clabel=p.clabel; csize=p.csize; cmulti=p.cmulti; cpos=p.cpos; cposcount=p.cposcount; ct=p.ct;
    scratch.assign(H+2, vector<int>(ncls));
    useTaboo=false;
  }
  inline uint64_t hashBelief(const int* b, int sz) const {
    uint64_t h=1469598103934665603ULL;
    for(int i=0;i<sz;i++){ h^=(uint64_t)(uint32_t)b[i]; h*=1099511628211ULL; }
    return h;
  }
  bool terminal(const int* b, int sz, bool& isNo) const {
    if(sz==0){ isNo=true; return true; }
    bool allMulti=true; int fp=-1; bool same=true;
    for(int i=0;i<sz;i++){
      int C=b[i];
      if(cmulti[C]) same=false;
      else { allMulti=false; if(fp<0) fp=cpos[C]; else if(cpos[C]!=fp) same=false; }
    }
    if(allMulti){ isNo=true; return true; }
    if(same && fp>=0){ isNo=false; return true; }
    return false;
  }
  double heuristic(const int* b, int sz) const {
    double Psum=0;
    for(int i=0;i<sz;i++) Psum+=cposcount[b[i]];
    if(Psum<=1) return 1.0;
    int D=2;
    int d=clabel[b[0]];
    for(int a=0;a<3;a++){
      if(a==2 && d==0) continue;
      bool seen[128]={false}; int cnt=0;
      for(int i=0;i<sz;i++){
        int t=ct[b[i]][a]; int L=clabel[t];
        if(!seen[L]){seen[L]=true;cnt++;}
      }
      if(cnt>D) D=cnt;
    }
    return 1.0 + log(Psum)/log((double)D);
  }
  double eval(const int* b, int sz, int depth, int* outAct){
    bool isNo;
    if(terminal(b,sz,isNo)) return 1.0;
    if(depth==0) return heuristic(b,sz);
    if(useTaboo && depth < H){
      if(taboo.count(hashBelief(b,sz))) return TABOO_VAL;
    }
    ++nodes;
    if(nodes>nodeBudget) return heuristic(b,sz);
    int d = clabel[b[0]];
    double best=1e18; int bestA=-1;
    int* buf = scratch[depth].data();
    for(int a=0;a<3;a++){
      if(a==2 && d==0) continue;
      int hist[128]={0};
      for(int i=0;i<sz;i++){ int t=ct[b[i]][a]; hist[clabel[t]]++; }
      int start[128]; int acc=0;
      for(int L=0;L<128;L++){ start[L]=acc; acc+=hist[L]; }
      int posArr[128]; memcpy(posArr,start,sizeof(start));
      for(int i=0;i<sz;i++){ int C=b[i]; int t=ct[C][a]; int L=clabel[t]; buf[posArr[L]++]=t; }
      double W=0; for(int i=0;i<sz;i++) W+=csize[b[i]];
      double sum=0;
      for(int L=0;L<128;L++) if(hist[L]){
        int* seg=buf+start[L]; int segsz=hist[L];
        double w=0; for(int i=0;i<segsz;i++) w+=csize[seg[i]];
        double v=eval(seg,segsz,depth-1,nullptr);
        sum += (w/W)*v;
      }
      double val=1.0+sum;
      if(val<best){best=val;bestA=a;}
    }
    if(outAct) *outAct=bestA;
    return best;
  }
  bool findWord(const vector<int>& B, vector<int>& word, int maxPairTries=6, long long bfsCap=300000){
    int n=B.size();
    if(n<2) return false;
    vector<pair<int,int>> pairs;
    for(int i=0;i<n;i++)for(int j=i+1;j<n;j++){
      if(cpos[B[i]]!=cpos[B[j]] || cmulti[B[i]] || cmulti[B[j]]){ pairs.push_back({B[i],B[j]}); if((int)pairs.size()>=maxPairTries*3)break; }
      if((int)pairs.size()>=maxPairTries*3)break;
    }
    for(int i=0;i<n;i++)for(int j=i+1;j<n;j++){
      pairs.push_back({B[i],B[j]});
      if((int)pairs.size()>=maxPairTries*6)break;
      if((int)pairs.size()>=maxPairTries*6)break;
    }
    int tries=0; vector<int> bestWord;
    for(auto&pr:pairs){
      if(tries>=maxPairTries) break;
      tries++;
      int c0=pr.first,d0=pr.second;
      if(c0==d0) continue;
      unordered_map<long long,int> parState;
      unordered_map<long long,int> parAction;
      vector<pair<int,int>> nodes;
      auto key=[&](int u,int v){ return (long long)u*ncls+v; };
      long long st=key(c0,d0);
      parState[st]=-1; parAction[st]=-1;
      nodes.push_back({c0,d0});
      size_t head=0; bool found=false; int fu=-1,fv=-1,fa=-1;
      while(head<nodes.size() && !found){
        auto pr2=nodes[head]; head++;
        int u=pr2.first,v=pr2.second;
        for(int a=0;a<3;a++){
          if(a==2 && clabel[u]==0) continue;
          int tu=ct[u][a], tv=ct[v][a];
          if(clabel[tu]!=clabel[tv]){ found=true; fu=u; fv=v; fa=a; break; }
          long long nk=key(tu,tv);
          if(parState.find(nk)==parState.end()){
            parState[nk]=key(u,v); parAction[nk]=a;
            nodes.push_back({tu,tv});
            if((long long)nodes.size()>bfsCap) break;
          }
        }
        if((long long)nodes.size()>bfsCap) break;
      }
      if(found){
        vector<int> w;
        long long k=key(fu,fv);
        while(parAction[k]!=-1){
          w.push_back(parAction[k]);
          k=parState[k];
        }
        reverse(w.begin(),w.end());
        w.push_back(fa);
        if(bestWord.empty() || w.size()<bestWord.size()) bestWord=w;
        if(bestWord.size()<=1) break;
      }
    }
    if(bestWord.empty()) return false;
    word=bestWord;
    return true;
  }
  int choose(const vector<int>& B){
    bool isNo;
    if(terminal(B.data(),(int)B.size(),isNo)) return -1;
    nodes=0;
    int act=-1;
    eval(B.data(),(int)B.size(),H,&act);
    if(act<0) act=0;
    return act;
  }
};

/* ----------------------------------- Main ----------------------------------- */
static int rdInt(){ int x; if(scanf("%d",&x)!=1) exit(0); return x; }
static void outCmd(int a){
  if(a==0) printf("left\n");
  else if(a==1) printf("right\n");
  else printf("step\n");
  fflush(stdout);
}

int main(){
  int r=rdInt(), c=rdInt();
  vector<string> g(r);
  {
    char buf[128];
    for(int i=0;i<r;i++){
      if(scanf("%127s",buf)!=1) return 0;
      g[i]=string(buf);
    }
  }
  Problem P; P.build(r,c,g);
  int d0=rdInt();
  vector<int> B0;
  for(int b=0;b<P.ncls;b++) if(P.clabel[b]==d0) B0.push_back(b);

  // Try exact solution first (optimal expected number of rounds).
  ExactSolver ex; ex.init(P);
  bool exactOk = false;
  bool exactFreed = false;
  {
    auto tstart=chrono::steady_clock::now();
    long long cap = 600000;
    double buildLim = 1.8;
    if(ex.build(B0,cap,buildLim)){
      double rem = buildLim - chrono::duration<double>(chrono::steady_clock::now()-tstart).count();
      double solveLim = 0.9;
      if(rem < solveLim) solveLim = rem;
      if(solveLim > 0.02 && ex.solve(solveLim)) exactOk=true;
    }
  }
  if(exactOk){
    int node=0; // B0 was inserted first by build
    while(true){
      bool isNo;
      const int* b=ex.allB.data()+ex.bOff[node];
      int sz=ex.bLen[node];
      if(ex.isTermSeg(b,sz,isNo)){
        if(isNo) printf("no\n");
        else printf("yes %d %d\n", ex.P->cpos[b[0]]/c+1, ex.P->cpos[b[0]]%c+1);
        fflush(stdout);
        return 0;
      }
      int a=ex.bestAction(node);
      if(a<0) a=0;
      outCmd(a);
      int obs=rdInt();
      int nn=ex.childForObs(node,a,obs);
      if(nn<0) return 0;
      node=nn;
    }
  }

  // Fallback: depth-limited expected-value policy with taboo-cycle avoidance
  // and a guaranteed-progress forced distinguishing word.
  ex.clearAll(); (void)exactFreed;
  Policy pol; pol.init(P,8,3000000LL);
  vector<int> B=B0;
  pol.useTaboo=true;
  pol.taboo.insert(pol.hashBelief(B.data(),B.size()));
  vector<int> word; size_t wi=0; int forcedStartSize=-1;
  int sinceDec=0;
  while(true){
    bool isNo;
    if(pol.terminal(B.data(),B.size(),isNo)){
      if(isNo) printf("no\n");
      else {
        int pos=-1;
        for(int C:B){ if(!P.cmulti[C]){ pos=P.cpos[C]; break; } }
        printf("yes %d %d\n", pos/c+1, pos%c+1);
      }
      fflush(stdout);
      return 0;
    }
    int a;
    if(!word.empty()) a=word[wi++];
    else {
      if(sinceDec>=3 && pol.findWord(B,word)){
        wi=0; a=word[wi++]; forcedStartSize=B.size();
      } else a=pol.choose(B);
    }
    if(a<0) a=0;
    outCmd(a);
    int obs=rdInt();
    int oldSize=B.size();
    vector<int> nb;
    nb.reserve(B.size());
    for(int C:B){ int t=P.ct[C][a]; if(P.clabel[t]==obs) nb.push_back(t); }
    if(nb.empty()){ B.swap(nb); continue; }
    B.swap(nb);
    if((int)B.size()<oldSize) sinceDec=0; else sinceDec++;
    if(!word.empty()){
      if((int)B.size()<forcedStartSize){ word.clear(); wi=0; }
      else if(wi==word.size()){
        word.clear(); wi=0;
        if((int)B.size()==oldSize){
          if(!pol.findWord(B,word)) return 0;
          forcedStartSize=B.size(); wi=0;
        }
      }
    } else {
      uint64_t h=pol.hashBelief(B.data(),B.size());
      if((int)B.size()==oldSize && pol.taboo.count(h)){
        if(!pol.findWord(B,word)) return 0;
        forcedStartSize=B.size(); wi=0;
      } else {
        pol.taboo.insert(h);
      }
    }
  }
  return 0;
}

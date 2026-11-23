#include <bits/stdc++.h>
using namespace std;

static int n;
static int queriesUsed = 0;

static string ask(const vector<int>& d) {
    // d is 0-indexed, size n
    string s;
    s.reserve(n * 4 + 4);
    s += "?";
    for (int i = 0; i < n; i++) { s += ' '; s += to_string(d[i]); }
    s += '\n';
    fwrite(s.data(), 1, s.size(), stdout);
    fflush(stdout);
    queriesUsed++;
    string r;
    if (!(cin >> r)) exit(0);
    if ((int)r.size() != n) exit(0);
    return r;
}

// ---------------- depth planner on abstract model ----------------
typedef vector<pair<int,int>> Pcs; // pieces incl. root (0,0)
typedef vector<pair<int,int>> Act; // (piece index, h)
static vector<long long> AA;
static int needA(long long L){ int r=0; while(AA[r]<L) r++; return r; }
static mt19937 prng(12345);

static double Wt(int L,int h,int var){
    double l=log2((double)L), mx=log2((double)max(h,L-h));
    switch(var){
        case 0: return pow(l,3)-pow(mx,3);
        case 1: return l*l-mx*mx;
        case 2: return pow(l,4)-pow(mx,4);
        case 3: return l*l;
        default: return pow(l,3);
    }
}
static int FORCEI=-1, FORCEH=0;
static double NOISE=0;
static Act dpAct(const Pcs&p,int var,int hmode){
    int k=p.size();
    vector<double> best(k,-1); vector<int> prv(k,-1), bh(k,0);
    vector<double> pref(k,0); vector<int> prefi(k,-1);
    uniform_real_distribution<double> U01(0,1);
    for(int i=0;i<k;i++){
        int s=p[i].first,e=p[i].second,L=e-s+1;
        if(i>=1 && L>=2){
            int Ts=p[i-1].first;
            vector<int> hs;
            if(i==FORCEI) hs={FORCEH};
            else if(hmode==0) hs={L/2};
            else { for(int h=1;h<L;h++) hs.push_back(h); }
            double nz = NOISE>0 ? (1+NOISE*(U01(prng)-0.5)) : 1.0;
            for(int h:hs){
                double w=Wt(L,h,var)*nz; if(i==FORCEI) w+=1e9;
                int lim=Ts-h;
                int lo=0,hi=i-2,J=-1;
                while(lo<=hi){int md=(lo+hi)/2; if(p[md].second<lim){J=md;lo=md+1;}else hi=md-1;}
                double bs=0;int bp=-1;
                if(J>=0 && pref[J]>0){bs=pref[J];bp=prefi[J];}
                if(bs+w>best[i]){best[i]=bs+w;prv[i]=bp;bh[i]=h;}
            }
        }
        pref[i]= i? pref[i-1]:0; prefi[i]= i?prefi[i-1]:-1;
        if(best[i]>pref[i]){pref[i]=best[i];prefi[i]=i;}
    }
    Act M; int i=prefi[k-1];
    while(i>=0){M.push_back({i,bh[i]}); i=prv[i];}
    return M;
}
static Act frontAct(const Pcs&p,int var,int hmode){
    int k=p.size()-1; int L=p[k].second-p[k].first+1;
    if(L>=2){ int r=needA(L); int h=(int)(L-AA[r-1]); if(h<1)h=1; if(h>L-1)h=L-1; FORCEI=k;FORCEH=h; Act M=dpAct(p,var,hmode); FORCEI=-1; return M; }
    return dpAct(p,var,hmode);
}
static Pcs applyAct(const Pcs&p,const Act&M){
    vector<int> hh(p.size(),0); for(auto&x:M) hh[x.first]=x.second;
    Pcs r;
    for(int i=0;i<(int)p.size();i++){
        if(hh[i]){int s=p[i].first,e=p[i].second,h=hh[i]; r.push_back({s,s+h-1}); r.push_back({s+h,e});}
        else r.push_back(p[i]);
    }
    return r;
}
static bool doneP(const Pcs&p){for(auto&x:p) if(x.second>x.first) return false; return true;}
static int baseRun(Pcs p){int q=0; while(!doneP(p)){p=applyAct(p,frontAct(p,0,1));q++;} return q;}
static double HYPW=0.4; static int HYPMODE=1; static int CTOP=0, NTOT=0; static double WU=0.3, WF=0.5;
static int truncRun(Pcs p,int Dh){
    while(p.size()>1 && p.back().first>Dh) p.pop_back();
    if(p.back().second>Dh) p.back().second=Dh;
    return baseRun(p);
}
static Act planDepth(const Pcs&p){
    vector<Act> cands;
    for(int v=0;v<5;v++) for(int hm=0;hm<2;hm++){ cands.push_back(dpAct(p,v,hm)); cands.push_back(frontAct(p,v,hm)); }
    NOISE=0.8;
    for(int c=0;c<20;c++){ int v=prng()%5, hm=prng()%2; if(prng()%2) cands.push_back(dpAct(p,v,hm)); else cands.push_back(frontAct(p,v,hm)); }
    NOISE=0;
    // hypotheses on true max depth within top piece
    int s=p.back().first, U=p.back().second, L=U-s+1;
    vector<pair<int,double>> hyps; hyps.push_back({U,1.0});
    if(HYPMODE==1){ for(int j=1; ; j++){ int Lh=(L+(1<<j)-1)>>j; if(Lh<1||j>20) break; int Dh=s-1+Lh; if(Dh<hyps.back().first) hyps.push_back({Dh,pow(HYPW,j)}); if(Lh==1) break; } }
    else if(HYPMODE==4 && CTOP>0){
        double w=max(1.0,(double)(NTOT-CTOP)/s);
        double est=CTOP/w;
        hyps.clear(); hyps.push_back({U,WU});
        for(double f: {0.5,1.0,2.0}){ int Dh=s-1+(int)ceil(est*f); Dh=max(Dh,s); if(Dh<U) hyps.push_back({Dh, f==1.0?1.0:WF}); }
    }
    double bestv=1e18; Act ba;
    if(HYPMODE==2||HYPMODE==3) for(auto&h:hyps) h.second=1.0;
    for(auto&a:cands){ if(a.empty()) continue; Pcs np=applyAct(p,a); double v=0,ws=0,mx=-1e9;
        for(auto&h:hyps){ double c=truncRun(np,h.first); if(HYPMODE==2||HYPMODE==3) c-=needA(h.first); v+=h.second*c; ws+=h.second; mx=max(mx,c);} v/=ws;
        if(HYPMODE==3) v=mx+1e-3*v;
        if(v<bestv-1e-9){bestv=v;ba=a;} }
    return ba;
}

// ---------------- exact solver for small states ----------------
namespace EX {
typedef unsigned long long u64;
int D; long long nodes, LIMIT; bool aborted;
unordered_set<u64> failed[80];
struct Pc{int s,e;};
inline int need(int L){ return needA(L); }
vector<Pc> pieces(u64 m){vector<Pc> v; v.push_back({0,0}); int s=1; for(int b=1;b<=D;b++){ if(b==D || (m>>b&1)){v.push_back({s,b}); s=b+1;} } return v;}
int Vlb(const vector<Pc>&p){int k=p.size(),best=0; for(int i=1;i<k;i++){int s=0; for(int j=i;j<k&&j<i+40;j++){s+=p[j].e-p[j].s+1; best=max(best,need(s)-(j-i));}} return best;}
vector<int> suf;
bool sufok(int r,int added){ int k=suf.size(); for(int a=k-added;a<k;a++){ int s=0; for(int j=a;j>=0&&a-j<40;j--){ s+=suf[j]; if(need(s)-(a-j)>r-1) return false; } } return true; }
bool solve(u64 m,int r);
vector<u64> path;
bool rec(const vector<Pc>&p,int i,u64 nm,int r,vector<array<int,3>>&M,bool any){
    if(aborted) return false;
    if(i==0){ if(!any) return false; return solve(nm,r-1);}
    int s=p[i].s,e=p[i].e,L=e-s+1;
    bool forced = L>=2 && need(L)>=r;
    bool blocked=false; for(auto&z:M) if(!(e<z[1]||s>z[2])){blocked=true;break;}
    if(L>=2&&!blocked){
        int Ts=p[i-1].s, Te=p[i-1].e;
        int hs[64]; int nh=0; for(int h=1;h<L;h++) if(need(h)<=r-1&&need(L-h)<=r-1) hs[nh++]=h;
        sort(hs,hs+nh,[&](int a,int b){return abs(2*a-L)<abs(2*b-L);});
        for(int q=0;q<nh;q++){ int h=hs[q];
            M.push_back({i,Ts-h,Te});
            suf.push_back(L-h); suf.push_back(h);
            bool ok=sufok(r,2) && rec(p,i-1,nm|(1ULL<<(s+h-1)),r,M,true);
            suf.pop_back(); suf.pop_back(); M.pop_back();
            if(ok) return true;
            if(aborted) return false;
        }
    }
    if(!forced){ suf.push_back(L); bool ok=sufok(r,1)&&rec(p,i-1,nm,r,M,any); suf.pop_back(); if(ok) return true; }
    return false;
}
bool solve(u64 m,int r){
    if(++nodes>LIMIT){aborted=true;return false;}
    auto p=pieces(m);
    if((int)p.size()==D+1) return true;
    if(r<=0) return false;
    if(Vlb(p)>r) return false;
    if(failed[r].count(m)) return false;
    vector<array<int,3>> M; vector<int> saved; saved.swap(suf);
    bool ok=rec(p,p.size()-1,m,r,M,false);
    suf.swap(saved);
    if(ok){ path.push_back(m); return true;}
    if(!aborted) failed[r].insert(m);
    return false;
}
// returns plan as list of states (cut masks) from next state to final; empty if fail
// rmax: only try budgets < rmax
vector<u64> plan(const Pcs&p,int rmax,long long limit){
    D=p.back().second; if(D>62) return {};
    u64 m=0; for(size_t i=1;i+1<p.size();i++) m|=1ULL<<p[i].second;
    for(auto&f:failed) f.clear();
    vector<Pc> pp=pieces(m); int lb=Vlb(pp);
    nodes=0; LIMIT=limit; aborted=false;
    for(int r=lb;r<rmax;r++){
        path.clear(); suf.clear();
        if(solve(m,r)){ reverse(path.begin(),path.end()); vector<u64> res(path.begin()+1,path.end()); u64 full=0; for(int b=1;b<D;b++) full|=1ULL<<b; res.push_back(full); return res; }
        if(aborted) break;
    }
    return {};
}
}
static vector<unsigned long long> exPlan; static int exD=-1; static int EXL=3;
static vector<int> PROBES={2};
static Act planForcedTop(const Pcs&p,int fh){
    int fi=p.size()-1; vector<Act> cands;
    for(int v=0;v<5;v++) for(int hm=0;hm<2;hm++){ FORCEI=fi;FORCEH=fh; cands.push_back(dpAct(p,v,hm)); FORCEI=-1; }
    int bestv=INT_MAX; Act ba;
    for(auto&a:cands){ if(a.empty()) continue; int v=baseRun(applyAct(p,a)); if(v<bestv){bestv=v;ba=a;} }
    return ba;
}
static Act planDepthMain(const Pcs&p){
    { int s=p.back().first, U=p.back().second;
      for(size_t i=0;i<PROBES.size();i++){ int t=PROBES[i]; if(t>=s && t<U){ PROBES.erase(PROBES.begin(),PROBES.begin()+i+1); exPlan.clear(); exD=-1; return planForcedTop(p,t-s+1);} } }
    int D=p.back().second;
    unsigned long long m=0; for(size_t i=1;i+1<p.size();i++) m|=1ULL<<p[i].second;
    // follow existing plan if consistent
    if(D==exD && !exPlan.empty()){
        unsigned long long nx=exPlan.front();
        if((nx&m)==m){
            exPlan.erase(exPlan.begin());
            Act M; for(size_t i=1;i<p.size();i++){ int s=p[i].first,e=p[i].second; for(int b=s;b<e;b++) if(nx>>b&1){ M.push_back({(int)i,b-s+1}); break; } }
            return M;
        }
    }
    Act ra=planDepth(p);
    int rv=1+baseRun(applyAct(p,ra));
    if(D<=62 && (p.back().second-p.back().first+1<=EXL)){
        auto pl=EX::plan(p,rv,400000);
        if(!pl.empty()){ exPlan=pl; exD=D; return planDepthMain(p); }
    }
    exPlan.clear(); exD=-1;
    return ra;
}

int main(){
    AA={1,2,3}; while(AA.size()<80) AA.push_back(AA[AA.size()-2]+AA[AA.size()-3]+1);
    if(getenv("HYPMODE")) HYPMODE=atoi(getenv("HYPMODE")); if(getenv("WU")) WU=atof(getenv("WU")); if(getenv("WF")) WF=atof(getenv("WF")); if(getenv("HYPW")) HYPW=atof(getenv("HYPW"));
    if(getenv("PROBES")){ PROBES.clear(); string s=getenv("PROBES"); stringstream ss(s); int x; while(ss>>x) PROBES.push_back(x); }
    if(!(cin>>n)) return 0;
    if(n==2){ printf("!\n1 2\n"); fflush(stdout); return 0; }
    vector<int> lo(n,1), hi(n,n-1);
    lo[0]=hi[0]=0;
    // per-layer parent decoding state (layers indexed by depth)
    vector<vector<int>> S(n+1);       // exact layer members (only when exact)
    vector<char> exact(n+1,0);
    vector<int> idx(n,0), pacc(n,0);
    vector<int> need(n+2,0), got(n+2,0);
    auto refreshExact=[&](){
        for(int k=0;k<=n;k++){ if(exact[k]) continue; }
        vector<vector<int>> tmp(n+1);
        for(int v=0;v<n;v++) if(lo[v]==hi[v]) tmp[lo[v]].push_back(v);
        for(int k=0;k<=n;k++) if(!exact[k] && !tmp[k].empty()){
            exact[k]=1; S[k]=tmp[k];
            for(int i=0;i<(int)S[k].size();i++) idx[S[k][i]]=i;
        }
        for(int k=2;k<=n;k++) if(exact[k-1] && need[k]==0){ int w=S[k-1].size(); int b=0; while((1<<b)<w) b++; need[k]=b; }
    };
    refreshExact();
    // ---------- depth phase with interleaved parent bits ----------
    while(true){
        set<pair<int,int>> ps; for(int v=0;v<n;v++) ps.insert({lo[v],hi[v]});
        Pcs p(ps.begin(),ps.end());
        {
            auto &tp=p.back(); int cnt=0; for(int v=0;v<n;v++) if(lo[v]==tp.first) cnt++;
            int ne=min(tp.second,tp.first-1+cnt);
            if(ne!=tp.second){ for(int v=0;v<n;v++) if(lo[v]==tp.first) hi[v]=ne; tp.second=ne; }
        }
        if(doneP(p)) break;
        refreshExact();
        { int cnt=0; for(int v=0;v<n;v++) if(lo[v]==p.back().first) cnt++; CTOP=cnt; NTOT=n; }
        Act M=planDepthMain(p);
        vector<int> d(n,0);
        map<pair<int,int>,int> measured; map<pair<int,int>,int> srcR;
        vector<array<int,3>> reach; // depth source reach intervals [a,b]
        vector<pair<int,int>> measIv;
        for(auto&x:M){ measured[p[x.first]]=x.second; srcR[p[x.first-1]]=x.second;
            auto T=p[x.first-1]; reach.push_back({T.first-x.second,T.second+x.second,0}); measIv.push_back(p[x.first]); }
        for(int v=0;v<n;v++){ auto it=srcR.find({lo[v],hi[v]}); if(it!=srcR.end()) d[v]=it->second; }
        // choose parent-measured layers
        int Dmax=p.back().second;
        vector<int> cand;
        for(int k=2;k<=Dmax;k++) if(exact[k]&&exact[k-1]&&got[k]<need[k]) cand.push_back(k);
        sort(cand.begin(),cand.end(),[&](int a,int b){int ra=need[a]-got[a], rb=need[b]-got[b]; if(ra!=rb) return ra>rb; return a<b;});
        vector<int> C; set<int> Cs;
        for(int k:cand){
            bool ok=true;
            if(srcR.count({k-1,k-1})) ok=false;
            for(auto&r:reach) if(k>=r[0]&&k<=r[1]) ok=false;
            for(auto&iv:measIv) if(k-2>=iv.first&&k-2<=iv.second) ok=false;
            if(Cs.count(k-2)||Cs.count(k+2)) ok=false;
            if(ok){C.push_back(k);Cs.insert(k);}
        }
        for(int k:C){ int b=got[k]; for(int u:S[k-1]) if(idx[u]>>b&1) d[u]=1; }
        string r=ask(d);
        for(int k:C){ int b=got[k]; for(int v:S[k]) if(r[v]=='1') pacc[v]|=1<<b; got[k]++; }
        for(int v=0;v<n;v++){
            auto it=measured.find({lo[v],hi[v]});
            if(it==measured.end()) continue;
            int s=lo[v],h=it->second;
            if(r[v]=='1') hi[v]=s+h-1; else lo[v]=s+h;
        }
    }
    refreshExact();
    int D=0; for(int v=0;v<n;v++) D=max(D,lo[v]);
    { int sg=0,sn=0; for(int k=0;k<=D;k++){sg+=got[k]; sn+=need[k];} int Q0=0; for(int k=1;k<=D;k++){ int rk=need[k]-got[k]; Q0=max(Q0,rk); if(k+2<=D) Q0=max(Q0,rk+need[k+2]-got[k+2]); } int Qf=0; for(int k=1;k<=D;k++){ Qf=max(Qf,need[k]); if(k+2<=D) Qf=max(Qf,need[k]+need[k+2]); } fprintf(stderr,"depthq=%d bitsDuring=%d/%d Qrem=%d Qfull=%d ",queriesUsed,sg,sn,Q0,Qf); }
    // ---------- parent phase ----------
    int Q=0; for(int k=1;k<=D;k++){ int rk=need[k]-got[k]; Q=max(Q,rk); if(k+2<=D) Q=max(Q,rk+need[k+2]-got[k+2]); }
    vector<vector<int>> slots(Q);
    for(int par=0;par<2;par++){
        int pos=0;
        for(int k=par;k<=D;k+=2){
            int rk=need[k]-got[k];
            if(rk==0){pos=0;continue;}
            if(pos%2==0) for(int t=0;t<rk;t++) slots[t].push_back(k);
            else for(int t=Q-rk;t<Q;t++) slots[t].push_back(k);
            pos++;
        }
    }
    for(int t=0;t<Q;t++){
        if(slots[t].empty()) continue;
        vector<int> d(n,0);
        for(int k:slots[t]){ int b=got[k]; for(int u:S[k-1]) if(idx[u]>>b&1) d[u]=1; }
        string r=ask(d);
        for(int k:slots[t]){ int b=got[k]; for(int v:S[k]) if(r[v]=='1') pacc[v]|=1<<b; got[k]++; }
    }
    string out="!\n";
    for(int k=1;k<=D;k++) for(int v:S[k]){
        int pidx = (k==1)?0:pacc[v];
        int u=S[k-1][pidx];
        out+=to_string(v+1)+" "+to_string(u+1)+"\n";
    }
    fwrite(out.data(),1,out.size(),stdout); fflush(stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
// Right-to-left rolling-window solver.
struct Key { vector<uint64_t> w; int d; bool operator==(const Key&o)const{return d==o.d&&w==o.w;} };
struct KH { size_t operator()(const Key&k)const{ uint64_t h=k.d*0x9E3779B97F4A7C15ULL; for(auto x:k.w){h^=x+0x9E3779B97F4A7C15ULL+(h<<6)+(h>>2);} return h;} };
struct Universe { // for window size w and scap
    int w, sc, U, words;
    vector<vector<int>> idx; // per code: merged index of window elems (0..w-1) then s0,s1
    vector<char> valid;
    struct Qd{int x,y;}; vector<Qd> qd; vector<vector<uint64_t>> qb;
};
struct Roll {
    int n, K; int TIEMODE=0; long long added=0; int curOpt=0; int CC=2; bool dbg=false; int SLACK=0; int LOOK=0; int LW=2; int WMODE=0; double THR=9; double CDEC=1; double codelen=1.585; double DECAY=0.01; int curSlack=0; double BETA=2; vector<int> hist; double cnt2[9][3]={}, cnt0[3]={};
    function<bool(int,int)> ask; long long queries=0;
    map<pair<int,int>,Universe> unis;
    unordered_map<Key,char,KH> memo;
    long long feasCalls=0;
    Universe& uni(int w,int sc){
        auto it=unis.find({w,sc}); if(it!=unis.end()) return it->second;
        Universe&u=unis[{w,sc}]; u.w=w;u.sc=sc; u.U=1; for(int t=0;t<w;t++)u.U*=3; u.words=(u.U+63)/64;
        u.idx.assign(u.U,{}); u.valid.assign(u.U,0);
        for(int c=0;c<u.U;c++){
            // list of ids: window t -> t, s_j -> w+j ; S elements available: sc (cap 2) plus "rest" unlimited if sc==2? need s2 for ranks: element can have rank 2 with S of size>=... 
            // we use sc = min(|S|,3); ids w..w+sc-1
            vector<int> L; for(int j=0;j<sc;j++) L.push_back(w+j);
            int cc=c; bool ok=true;
            for(int t=0;t<w;t++){ int r=cc%3; cc/=3; if(r>(int)L.size()){ok=false;break;} L.insert(L.begin()+r,t);} 
            if(!ok) continue;
            u.valid[c]=1; vector<int> ix(w+sc); for(int j=0;j<(int)L.size();j++) ix[L[j]]=j; u.idx[c]=ix;
        }
        auto addq=[&](int x,int y){ vector<uint64_t> b(u.words,0); int cnt=0,tot=0; for(int c=0;c<u.U;c++) if(u.valid[c]){tot++; if(u.idx[c][x]<u.idx[c][y]){b[c>>6]|=1ULL<<(c&63);cnt++;}}
            if(cnt==0||cnt==tot) return; u.qd.push_back({x,y}); u.qb.push_back(b);} ;
        for(int x=0;x<w;x++) for(int y=x+1;y<w;y++) addq(x,y);
        for(int x=0;x<w;x++) for(int j=0;j<min(sc,2);j++) addq(x,w+j);
        return u;
    }
    static int pc(const vector<uint64_t>&a){int c=0; for(auto x:a)c+=__builtin_popcountll(x); return c;}
    bool feas(Universe&u,const vector<uint64_t>&s,int c,int d){
        if(c<=1) return true;
        if(d>=31 || c>(1LL<<d)) return d>=31? true:false;
        Key key{s,d}; auto it=memo.find(key); if(it!=memo.end()) return it->second;
        feasCalls++;
        bool ok=false; int Q=u.qb.size();
        vector<pair<int,int>> ord; vector<uint64_t> a(u.words),b(u.words);
        for(int q=0;q<Q;q++){ int ca=0; for(int j=0;j<u.words;j++) ca+=__builtin_popcountll(s[j]&u.qb[q][j]); int cb=c-ca; if(!ca||!cb) continue; if(max(ca,cb)>(1LL<<(d-1))) continue; ord.push_back({abs(ca-cb),q}); }
        sort(ord.begin(),ord.end());
        for(auto [_,q]:ord){ int ca=0; for(int j=0;j<u.words;j++){a[j]=s[j]&u.qb[q][j]; b[j]=s[j]&~u.qb[q][j]; ca+=__builtin_popcountll(a[j]);}
            if(feas(u,a,ca,d-1)&&feas(u,b,c-ca,d-1)){ok=true;break;} }
        memo[key]=ok; return ok;
    }
    int opt(Universe&u,const vector<uint64_t>&s,int c){ int d=0; while((1LL<<d)<c) d++; while(!feas(u,s,c,d)) d++; return d; }
    // state
    vector<int> sorted; // positions of S sorted by value ascending (S = positions > R)
    int R, L; // window = positions L..R (t=0 is R, t=R-L is L)
    vector<int> codes;
    vector<int> res; // final array
    double predp(int pos,int r){ auto rk=[&](int p2)->int{return p2<=n?hist[p2]:0;}; int ctx=rk(pos+1)*3+rk(pos+2); double t0=cnt0[0]+cnt0[1]+cnt0[2]+3; double p0=(cnt0[r]+1)/t0;
        return WMODE==2? (cnt2[ctx][r]+BETA*p0)/(cnt2[ctx][0]+cnt2[ctx][1]+cnt2[ctx][2]+BETA) : p0; }
    void record(int pos,int r){ codelen = (1-DECAY)*codelen + DECAY*(-log2(predp(pos,r))); hist[pos]=r; if(CDEC<1){ for(int a=0;a<9;a++)for(int b=0;b<3;b++)cnt2[a][b]*=CDEC; for(int b=0;b<3;b++)cnt0[b]*=CDEC; } cnt0[r]+=1; auto rk=[&](int p2)->int{return p2<=n?hist[p2]:0;}; cnt2[rk(pos+1)*3+rk(pos+2)][r]+=1; }
    void run(){
        hist.assign(n+3,0);
        sorted.clear(); R=n; L=n+1; codes={0};
        while(R>=1){
            int w=R-L+1;
            int sc=min((int)sorted.size(),3);
            auto filt=[&](Universe&uu){ vector<int> nc; for(int c:codes) if(uu.valid[c]) nc.push_back(c); codes.swap(nc); };
            filt(uni(w,sc));
            if(w>0){ bool same=true; for(int c:codes) if(c%3!=codes[0]%3){same=false;break;}
                if(same){ int r=codes[0]%3; record(R,r); sorted.insert(sorted.begin()+r,R); R--; for(int&c:codes)c/=3; continue; } }
            bool wasResolved = codes.size()==1;
            while(w<K && L>1){ // try extend left
                int pw=1; for(int t=0;t<w;t++)pw*=3; vector<int> nc; for(int c:codes) for(int r=0;r<3;r++) nc.push_back(c+r*pw);
                Universe&u2=uni(w+1,sc); { vector<int> t2; for(int c:nc) if(u2.valid[c]) t2.push_back(c); nc.swap(t2);} 
                vector<uint64_t> s2(u2.words,0); for(int c:nc) s2[c>>6]|=1ULL<<(c&63);
                int o2=opt(u2,s2,nc.size());
                long long cr=5LL*(added+1)-3LL*(queries+o2);
                bool forced = codes.size()==1 && (w==0 || true) && false;
                if(dbg) fprintf(stderr,"try w=%d N=%zu o2=%d cr=%lld curOpt=%d q=%lld\n",w,codes.size(),o2,cr,curOpt,queries);
                if(cr>=-CC || (wasResolved && w<3)){ codes.swap(nc); L--; w++; added++; curOpt=o2; }
                else break;
            }
            Universe&u=uni(w,sc);
            // resolve rightmost
            bool same=true; for(int c:codes) if(c%3!=codes[0]%3){same=false;break;}
            if(same){
                int r=codes[0]%3; record(R,r); sorted.insert(sorted.begin()+r,R); R--; for(int&c:codes)c/=3; continue;
            }
            vector<uint64_t> s(u.words,0); for(int c:codes) s[c>>6]|=1ULL<<(c&63);
            int N=codes.size();
            vector<double> wt(N,1.0); double WT=N;
            bool useW = WMODE && codelen<THR;
            if(useW){ WT=0;
                for(int h=0;h<N;h++){ int c=codes[h]; int dg[16]; for(int t=0;t<w;t++){dg[t]=c%3;c/=3;}
                    double pw=1; for(int t=0;t<w;t++){ int pos=R-t; auto rk=[&](int tt)->int{ if(tt>=0) return dg[tt]; int p2=R-tt; return p2<=n? hist[p2]:0; };
                        int ctx = WMODE==2? rk(t-1)*3+rk(t-2) : 0; double t0=cnt0[0]+cnt0[1]+cnt0[2]+3; double p0=(cnt0[dg[t]]+1)/t0;
                        double p = WMODE==2? (cnt2[ctx][dg[t]]+BETA*p0)/(cnt2[ctx][0]+cnt2[ctx][1]+cnt2[ctx][2]+BETA) : p0; pw*=p; }
                    wt[h]=pw; WT+=pw; }
            }
            int bestq=-1; int bestD=INT_MAX; double bestBal=1e300;
            vector<uint64_t> a(u.words),b(u.words);
            for(int q=0;q<(int)u.qb.size();q++){
                int ca=0; for(int j=0;j<u.words;j++){a[j]=s[j]&u.qb[q][j]; b[j]=s[j]&~u.qb[q][j]; ca+=__builtin_popcountll(a[j]);}
                int cb=N-ca; if(!ca||!cb) continue;
                int oa=opt(u,a,ca), ob=opt(u,b,cb);
                int D=max(oa,ob); double bal;
                if(useW){ double wa=0; for(int h=0;h<N;h++){int c=codes[h]; if(a[c>>6]>>(c&63)&1) wa+=wt[h];} bal=fabs(2*wa-WT); }
                else bal = abs(ca-cb);
                int SL = (SLACK>0 && codelen<THR)? SLACK:0;
                if(SL>0){ long long cr=5LL*added-3LL*(queries+1+D); int lim=curOpt-1+SL; int Dc = (D<=lim && cr>=-CC)? 0 : D; 
                    if(Dc<bestD||(Dc==bestD&&bal<bestBal)){bestD=Dc;bestBal=bal;bestq=q;} }
                else if(D<bestD||(D==bestD&&bal<bestBal)){bestD=D;bestBal=bal;bestq=q;}
            }
            assert(bestq>=0);
            if(LOOK && N>2 && !useW){
                // among queries with D==bestD (non-slack), evaluate 2-step: sum over children of best child balance
                auto childBest=[&](vector<uint64_t>&cs,int cn)->long long{
                    if(cn<=1) return 0; int od=opt(u,cs,cn); long long bb=LLONG_MAX; vector<uint64_t> x(u.words),y(u.words);
                    for(int q2=0;q2<(int)u.qb.size();q2++){ int cx=0; for(int j=0;j<u.words;j++){x[j]=cs[j]&u.qb[q2][j]; cx+=__builtin_popcountll(x[j]);} int cy=cn-cx; if(!cx||!cy) continue;
                        if(max(cx,cy)>(1LL<<(od-1))) continue; for(int j=0;j<u.words;j++) y[j]=cs[j]&~u.qb[q2][j];
                        if(max(opt(u,x,cx),opt(u,y,cy))!=od-1) continue; bb=min(bb,(long long)abs(cx-cy)); }
                    return bb==LLONG_MAX? 1000000: bb; };
                long long bestS=LLONG_MAX; int bq=bestq;
                for(int q=0;q<(int)u.qb.size();q++){
                    int ca=0; for(int j=0;j<u.words;j++){a[j]=s[j]&u.qb[q][j]; b[j]=s[j]&~u.qb[q][j]; ca+=__builtin_popcountll(a[j]);}
                    int cb=N-ca; if(!ca||!cb) continue; int oa=opt(u,a,ca), ob=opt(u,b,cb); if(max(oa,ob)!=bestD) continue;
                    if(abs(ca-cb)>bestBal+LOOK-1) continue;
                    long long sc=(long long)abs(ca-cb)*LW + childBest(a,ca)+childBest(b,cb);
                    if(sc<bestS){bestS=sc;bq=q;}
                }
                bestq=bq;
            }
            auto qd=u.qd[bestq];
            auto posOf=[&](int id){ return id<w? R-id : sorted[id-w]; };
            if(dbg) fprintf(stderr,"bestD=%d curOpt=%d w=%d N=%d\n",bestD,curOpt,w,N);
            bool lt=ask(posOf(qd.x),posOf(qd.y)); queries++;
            vector<int> nc; for(int c:codes) if((u.idx[c][qd.x]<u.idx[c][qd.y])==lt) nc.push_back(c); codes.swap(nc);
            { vector<uint64_t> s3(u.words,0); for(int c:codes) s3[c>>6]|=1ULL<<(c&63); curOpt=opt(u,s3,codes.size()); }
        }
        res.assign(n+1,0); for(int j=0;j<n;j++) res[sorted[j]]=j+1;
    }
};

int main(){
    int n; if(!(cin>>n)) return 0;
    Roll S; S.n=n;
    S.K = n<=12 ? 7 : 6; S.CC = n<=12 ? 100 : 2;
    S.TIEMODE=0; S.SLACK=1; S.LOOK=1; S.LW=1; S.WMODE=2; S.BETA=4; S.THR=1.55; S.CDEC=0.999;
    S.ask=[&](int x,int y)->bool{
        printf("? %d %d\n",x,y); fflush(stdout);
        string t; if(!(cin>>t)) exit(0);
        if(t=="<") return true; if(t==">") return false; exit(0);
    };
    S.run();
    printf("!"); for(int i=1;i<=n;i++) printf(" %d",S.res[i]); printf("\n"); fflush(stdout);
    return 0;
}

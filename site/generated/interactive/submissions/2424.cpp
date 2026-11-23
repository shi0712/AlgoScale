#include <bits/stdc++.h>
using namespace std;
#pragma GCC target("popcnt")
typedef unsigned long long u64;
#ifndef TCARRY
#define TCARRY 12
#endif
#ifndef TOL
#define TOL 0.04
#endif
#ifndef EXPL
#define EXPL 64
#endif
#ifndef HWIN
#define HWIN 24
#endif
#ifndef HMIN
#define HMIN 2
#endif
#ifndef PEN
#define PEN 6.0
#endif
#ifndef USEPRIOR
#define USEPRIOR 1
#endif

#ifdef LOCALSIM
static vector<int> HP; static long QC=0, STUCKS=0; static vector<vector<char>> FT;
static int ask(int l,int r){ QC++; return FT[l][r]; }
static void simInit(int n){ HP.assign(n+1,0); for(int i=1;i<=n;i++) if(scanf("%d",&HP[i])!=1) exit(1);
  FT.assign(n+2,vector<char>(n+2,0));
  for(int len=2;len<=n;len++) for(int l=1;l+len-1<=n;l++){int r=l+len-1; FT[l][r]=FT[l+1][r]^FT[l][r-1]^(len>2?FT[l+1][r-1]:0)^(HP[l]>HP[r]);} }
#else
static int ask(int l,int r){
    printf("? %d %d\n",l,r); fflush(stdout);
    int x; if(scanf("%d",&x)!=1||x<0) exit(0); return x;
}
#endif

static const int NS=14;
// special types: anchor kinds 0: rank of p_{k-1}, 1: rank of p_{k-2} (offsets -2..2, 0 = just above),
// 2: top (k-1-d), 3: bottom (d), d in 0..1
static inline void specialCands(const vector<int>&rk,int k,int*cs){
    int idx=0;
    for(int a=1;a<=2;a++){
        int base = (k-a>=1)? rk[k-a] : INT_MIN/2;
        for(int d=-2;d<=2;d++) cs[idx++]= base==INT_MIN/2 ? -1000 : base+d;
    }
    for(int d=0;d<2;d++) cs[idx++]=k-1-d;
    for(int d=0;d<2;d++) cs[idx++]=d;
}
struct Group{
    vector<int> rk;   // rk[j], j in 1..k-1 : rank among prefix (1..k-1)
    vector<char> F;   // F[i] = inversion parity of [i,k-1]
    vector<int> C;    // sorted candidates r (= #prefix elements smaller than p_k)
    double W=1;       // posterior weight of this prefix hypothesis
    unsigned hist[NS]={};
    // per-element prior: weight u per candidate plus extra on special values
    double u=1; int ns=0; int sc[NS]; double se[NS]; int st[NS];
    vector<u64> cur, fw, best;
    int ones=0, fones=0;
};

static int n;
static inline u64 sufx(u64 x){ x^=x>>1; x^=x>>2; x^=x>>4; x^=x>>8; x^=x>>16; x^=x>>32; return x; }

int main(){
    if(scanf("%d",&n)!=1) return 0;
#ifdef LOCALSIM
    simInit(n); auto t0=clock();
#endif
    if(n==1){ printf("! 1\n"); fflush(stdout); return 0; }
    vector<Group> G(1);
    G[0].rk.assign(n+2,0); G[0].F.assign(n+2,0); G[0].rk[1]=1;
    bool uniformW=true; // all group weights equal
    int slowMode=0; clock_t cstart=clock();
    for(int k=2;k<=n;k++){
        // ----- set up candidates and priors -----
        bool weighted=!uniformW;
        for(auto&g:G){
            g.C.resize(k); iota(g.C.begin(),g.C.end(),0);
            g.ns=0;
            double sp=0;
            if(USEPRIOR && k>=3){
                int cs[NS]; specialCands(g.rk,k,cs);
                double ps[NS];
                for(int s=0;s<NS;s++){
                    int h=__builtin_popcount(g.hist[s]&((1u<<HWIN)-1));
                    { double v=(h-HMIN+1) - PEN*(double)HWIN/k; ps[s]= (h>=HMIN && v>0) ? v/HWIN : 0; }
                    sp+=ps[s];
                }
                double scale = sp>0.98 ? 0.98/sp : 1.0;
                for(int s=0;s<NS;s++) if(ps[s]>0){
                    int c=cs[s]; if(c<0||c>k-1) continue;
                    double e=ps[s]*scale; bool merged=false;
                    for(int a=0;a<g.ns;a++) if(g.sc[a]==c){ g.se[a]+=e; merged=true; }
                    if(!merged){ g.sc[g.ns]=c; g.se[g.ns]=e; g.ns++; }
                }
                sp*=scale;
            }
            g.u=(1-sp)/k;
            if(g.ns) weighted=true;
        }
        if(!weighted) for(auto&g:G){ g.W=1; g.u=1; }
        auto total=[&](){ long s=0; for(auto&g:G) s+=g.C.size(); return s; };
        long M=total();
        long Tc=TCARRY;
        if((k&7)==0){
            double el=(double)(clock()-cstart)/CLOCKS_PER_SEC;
            if(el>0.5) slowMode=2; else if(el>0.3 && slowMode<1) slowMode=1;
        }
        if(slowMode==1) Tc=min<long>(Tc,4); else if(slowMode==2) Tc=1;
        long T = (k==n||weighted)?1:Tc;
        bool stuck=false;
        // ---------- scanning phase ----------
        while(M>T && (weighted || M>EXPL)){
            double Wtot=0, wmax=0;
            for(auto&g:G){
                int m=g.C.size(), W=(m+63)/64;
                g.cur.assign(W,0); g.ones=0;
                g.fw.assign(W,0); g.fones=0;
                for(int t=0;t<m;t++) if((k-1-g.C[t])&1){ g.fw[t>>6]|=1ULL<<(t&63); g.fones++; }
                double wt=g.u*m;
                for(int a=0;a<g.ns;a++){
                    int t=lower_bound(g.C.begin(),g.C.end(),g.sc[a])-g.C.begin();
                    g.st[a]=(t<m&&g.C[t]==g.sc[a])?t:-1;
                    if(g.st[a]>=0) wt+=g.se[a];
                }
                Wtot+=g.W*wt;
                if(weighted){
                    double wm=g.u; for(int a=0;a<g.ns;a++) if(g.st[a]>=0) wm=max(wm,g.u+g.se[a]);
                    wmax=max(wmax,g.W*wm);
                }
            }
            auto flipPre=[&](const Group&g,vector<u64>&cv,int&ones,int i){
                int L=lower_bound(g.C.begin(),g.C.end(),g.rk[i])-g.C.begin();
                int full=L>>6, rem=L&63, inpre=0;
                u64* c=cv.data();
                for(int w=0;w<full;w++){ inpre+=__builtin_popcountll(c[w]); c[w]=~c[w]; }
                if(rem){ u64 mk=(1ULL<<rem)-1; inpre+=__builtin_popcountll(c[full]&mk); c[full]^=mk; }
                ones+=L-2*inpre;
            };
            double good = weighted ? max(TOL*Wtot, 2*wmax-Wtot+1e-9*Wtot) : (double)max<long>(M&1,(long)(TOL*M));
            int bestI=-1; double bestScore=1e300;
            auto evalAt=[&](int i,bool back){
                double wones=0; long cones=0;
                for(auto&g:G){
                    int o= back? g.ones : g.fones;
                    int f=g.F[i];
                    long co= f ? (long)g.C.size()-o : o;
                    cones+=co;
                    if(weighted){
                        const vector<u64>&bv = back? g.cur : g.fw;
                        double wo=g.u*co;
                        for(int a=0;a<g.ns;a++) if(g.st[a]>=0){
                            int t=g.st[a];
                            if((int)((bv[t>>6]>>(t&63))&1)^f) wo+=g.se[a];
                        }
                        wones+=g.W*wo;
                    }
                }
                if(cones<=0||cones>=M) return;
                double sc= weighted ? fabs(2*wones-Wtot) : (double)labs(2*cones-M);
                if(sc<bestScore){
                    bestScore=sc; bestI=i;
                    for(auto&g:G) g.best = back? g.cur : g.fw;
                }
            };
            for(int lo=1,hi=k-1; lo<=hi; ){
                for(auto&g:G) flipPre(g,g.cur,g.ones,hi);
                evalAt(hi,true); hi--;
                if(bestScore<=good||lo>hi) break;
                evalAt(lo,false);
                if(bestScore<=good) break;
                for(auto&g:G) flipPre(g,g.fw,g.fones,lo);
                lo++;
            }
            if(bestI<0){ stuck=true; break; }
            int q=ask(bestI,k);
            vector<Group> NG;
            for(auto&g:G){
                int m=g.C.size();
                int s=q^g.F[bestI];
                vector<int> NC;
                for(int t=0;t<m;t++) if((int)((g.best[t>>6]>>(t&63))&1)==s) NC.push_back(g.C[t]);
                if(!NC.empty()){ g.C.swap(NC); NG.push_back(std::move(g)); }
            }
            G.swap(NG);
            M=total();
        }
        // ---------- explicit phase (small M, uniform weights) ----------
        if(M>T && !stuck){
            int W=(k+63)/64;
            struct Hyp{ int g, c; vector<u64> v; };
            vector<Hyp> H;
            vector<int> posOf(k+1);
            vector<u64> X(W), S(W);
            for(int gi=0;gi<(int)G.size();gi++){
                auto&g=G[gi];
                for(int j=1;j<k;j++) posOf[g.rk[j]]=j;
                vector<u64> Fb(W,0);
                for(int i=1;i<k;i++) if(g.F[i]) Fb[i>>6]|=1ULL<<(i&63);
                int m=g.C.size();
                fill(S.begin(),S.end(),0);
                int hiR=k-1;
                for(int t=m-1;t>=0;t--){
                    int c=g.C[t];
                    if(c<hiR){
                        fill(X.begin(),X.end(),0);
                        for(int a=c+1;a<=hiR;a++){ int j=posOf[a]; X[j>>6]|=1ULL<<(j&63); }
                        u64 carry=0;
                        for(int w=W-1;w>=0;w--){
                            u64 y=sufx(X[w]); if(carry) y=~y;
                            S[w]^=y; carry^=(u64)(__builtin_popcountll(X[w])&1);
                        }
                        hiR=c;
                    }
                    Hyp h; h.g=gi; h.c=c; h.v.resize(W);
                    for(int w=0;w<W;w++) h.v[w]=S[w]^Fb[w];
                    H.push_back(std::move(h));
                }
            }
            const int B=8;
            vector<u64> cnt(B*W);
            while((long)H.size()>T){
                int Mh=H.size();
                fill(cnt.begin(),cnt.end(),0);
                for(auto&h:H){
                    for(int w=0;w<W;w++){
                        u64 x=h.v[w];
                        for(int b=0;x&&b<B;b++){ u64 t=cnt[b*W+w]&x; cnt[b*W+w]^=x; x=t; }
                    }
                }
                int bestI=-1, bestScore=INT_MAX;
                for(int i=1;i<k;i++){
                    int w=i>>6, s=i&63, c=0;
                    for(int b=0;b<B;b++) c|=(int)((cnt[b*W+w]>>s)&1)<<b;
                    if(c==0||c==Mh) continue;
                    int sc=abs(2*c-Mh);
                    if(sc<bestScore){ bestScore=sc; bestI=i; if(sc<=(Mh&1)) break; }
                }
                if(bestI<0){ stuck=true; break; }
                int q=ask(bestI,k);
                vector<Hyp> NH;
                for(auto&h:H) if((int)((h.v[bestI>>6]>>(bestI&63))&1)==q) NH.push_back(std::move(h));
                H.swap(NH);
            }
            for(auto&g:G) g.C.clear();
            for(auto&h:H) G[h.g].C.push_back(h.c);
            vector<Group> NG;
            for(auto&g:G) if(!g.C.empty()){ sort(g.C.begin(),g.C.end()); NG.push_back(std::move(g)); }
            G.swap(NG);
            M=total();
        }
        if(stuck || k==n){
            // remaining hypotheses (one candidate per group) are indistinguishable by (.,k)
            // queries: separate them with arbitrary intervals inside [1,k]
#ifdef LOCALSIM
            if(M>1) STUCKS++;
#endif
            while(M>1){
                int ng=G.size();
                vector<vector<int>> P(ng,vector<int>(k+1));
                for(int t=0;t<ng;t++){
                    auto&g=G[t]; int r=g.C[0];
                    for(int j=1;j<k;j++) P[t][j]=g.rk[j]+(g.rk[j]>r);
                    P[t][k]=r+1;
                }
                vector<vector<char>> prv(ng,vector<char>(k+2,0)), cr(ng,vector<char>(k+2,0));
                int bl=-1,br=-1; long bs=LONG_MAX;
                for(int l=k;l>=1&&bs>(M&1);l--){
                    for(int t=0;t<ng;t++){
                        cr[t][l]=0;
                        for(int r=l+1;r<=k;r++)
                            cr[t][r]=prv[t][r]^cr[t][r-1]^(r-1>=l+1?prv[t][r-1]:0)^(P[t][l]>P[t][r]);
                    }
                    for(int r=l+1;r<=k;r++){
                        long ones=0; for(int t=0;t<ng;t++) ones+=cr[t][r];
                        if(ones==0||ones==M) continue;
                        long sc=labs(2*ones-M);
                        if(sc<bs){bs=sc;bl=l;br=r;}
                    }
                    swap(prv,cr);
                }
                if(bl<0) break; // identical permutations (cannot happen)
                int q=ask(bl,br);
                vector<Group> NG;
                for(int t=0;t<ng;t++){
                    int c=0; for(int a=bl;a<=br;a++) for(int b=a+1;b<=br;b++) c^=(P[t][a]>P[t][b]);
                    if(c==q) NG.push_back(std::move(G[t]));
                }
                G.swap(NG); M=total();
            }
            if(k==n) break;
        }
        // ----- expand: each hypothesis becomes a group with element k placed -----
        vector<Group> NG;
        bool anyPrior=false; for(auto&g:G) if(g.ns) anyPrior=true;
        double mx=0;
        for(auto&g:G){
            int cs[NS]; specialCands(g.rk,k,cs);
            for(int r:g.C){
                Group h;
                h.rk=g.rk; h.F=g.F;
                int par=0;
                for(int i=k-1;i>=1;i--){ if(h.rk[i]>r) par^=1; h.F[i]^=par; }
                h.F[k]=0;
                for(int j=1;j<k;j++) if(h.rk[j]>r) h.rk[j]++;
                h.rk[k]=r+1;
                for(int s=0;s<NS;s++) h.hist[s]=(g.hist[s]<<1)|(unsigned)(r==cs[s]);
                double q=g.u; for(int a=0;a<g.ns;a++) if(g.sc[a]==r) q+=g.se[a];
                h.W=g.W*q; mx=max(mx,h.W);
                NG.push_back(std::move(h));
            }
        }
        for(auto&h:NG) h.W/=mx;
        uniformW = (NG.size()==1) || (!anyPrior && uniformW);
        G.swap(NG);
    }
    auto&g=G[0]; int r=g.C[0];
    printf("!");
    for(int i=1;i<n;i++) printf(" %d",g.rk[i]+(g.rk[i]>r));
    printf(" %d\n",r+1); fflush(stdout);
#ifdef LOCALSIM
    bool ok=true; for(int i=1;i<n;i++) ok&=(HP[i]==g.rk[i]+(g.rk[i]>r)); ok&=(HP[n]==r+1);
    double lb=0; for(int k=2;k<=n;k++) lb+=log2(k);
    fprintf(stderr,"stucks=%ld ",STUCKS);fprintf(stderr,"ok=%d q=%ld lb=%.0f diff=%.0f time=%.3f\n",ok,QC,lb,QC-lb,(double)(clock()-t0)/CLOCKS_PER_SEC);
#endif
}

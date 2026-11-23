// Flatten or Concatenate (interactive)
//
// Structure used:
//  * Every valid array of sum s (a power of two) and length >= 2 splits at the
//    unique prefix with sum s/2 into two halves that are both "flatten
//    descendants" of one common earlier state; the shorter half therefore
//    contains the maximum (equal lengths => equal multisets).
//  * Every element of value v starts at a prefix sum that is a multiple of v
//    (dyadic alignment), which gives tight bounds on split positions.
//  * We descend into the shorter half lazily: split positions are only
//    narrowed as much as needed to decide which half is shorter.
//  * When the current interval is short and its ends are known exactly, an
//    endgame enumerates all valid patterns and picks queries by search.
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

#ifndef LOCAL_TEST
static ll ask(int l, int r){
    printf("? %d %d\n", l, r); fflush(stdout);
    ll x; if(scanf("%lld",&x)!=1) exit(0);
    if(x<0) exit(0);
    return x;
}
#else
ll ask(int l, int r);
#endif

int LIM=22;          // endgame max interval length
ll EXACTW=3000;      // exact search if |C|*L*L <= EXACTW
double RHO=0.5, TAU=2.0;
int FALLBACK_K=4;    // bisection fallback if bracket did not halve in K steps

struct Endgame {
    vector<vector<vector<int>>> pats;   // depth vectors by length
    vector<vector<double>> pw;          // prior weights
    void build(int M){
        vector<map<vector<int>,double>> valid(M+1);
        valid[1][vector<int>{0}]=1.0;
        for(int n=2;n<=M;n++){
            for(int m=1;2*m<=n;m++) for(auto& [X,wx]: valid[m]){
                vector<int> X1=X; for(int&e:X1) e++;
                vector<set<vector<int>>> lv(n-2*m+1); lv[0].insert(X1);
                for(int t=1;t<=n-2*m;t++) for(auto& Y: lv[t-1]){
                    int mn=*min_element(Y.begin(),Y.end());
                    for(size_t i=0;i<Y.size();i++) if(Y[i]==mn){
                        vector<int> Z=Y; Z[i]++; Z.insert(Z.begin()+i, Z[i]); lv[t].insert(Z);
                    }
                }
                for(int t1=0;t1<=n-2*m;t1++){
                    int t2=n-2*m-t1;
                    double w = wx*pow(RHO,t1+t2)*(t1==t2?TAU:1.0)/((double)lv[t1].size()*lv[t2].size());
                    for(auto& H1: lv[t1]) for(auto& H2: lv[t2]){
                        vector<int> F=H1; F.insert(F.end(),H2.begin(),H2.end()); valid[n][F]+=w;
                    }
                }
            }
            double Z=0; for(auto& [F,w]: valid[n]) Z+=w;
            for(auto& [F,w]: valid[n]) w/=Z;
        }
        pats.assign(M+1,{}); pw.assign(M+1,{});
        for(int n=1;n<=M;n++) for(auto& [F,w]: valid[n]){ pats[n].push_back(F); pw[n].push_back(w); }
    }
    int L=0; vector<vector<ll>> pre; vector<ll> ansv; vector<double> cw;
    struct Res{ int w; double tot; int a,b; };
    struct VH{ size_t operator()(const vector<int>& v) const { size_t h=v.size(); for(int x:v) h=h*1000003u+x; return h; } };
    unordered_map<vector<int>,Res,VH> memo;
    Res solve(const vector<int>& C){
        bool same=true; for(int c:C) if(ansv[c]!=ansv[C[0]]){same=false;break;}
        if(same) return {0,0,0,0};
        auto it=memo.find(C); if(it!=memo.end()) return it->second;
        Res best{INT_MAX,1e300,1,1};
        vector<pair<ll,int>> buf(C.size());
        for(int a=1;a<=L;a++) for(int b=a;b<=L;b++){
            for(size_t i=0;i<C.size();i++) buf[i]={pre[C[i]][b]-pre[C[i]][a-1], C[i]};
            sort(buf.begin(),buf.end());
            if(buf.front().first==buf.back().first) continue;
            int w=0; double tot=0; size_t i=0;
            while(i<buf.size()){
                size_t j=i; vector<int> part; double pws=0;
                while(j<buf.size() && buf[j].first==buf[i].first){ part.push_back(buf[j].second); pws+=cw[buf[j].second]; j++; }
                sort(part.begin(),part.end());
                Res r=solve(part);
                w=max(w,r.w+1); tot+=r.tot+pws;
                i=j;
            }
            if(make_pair(w,tot)<make_pair(best.w,best.tot)) best={w,tot,a,b};
        }
        memo[C]=best; return best;
    }
    Res greedy(const vector<int>& C){
        Res best{0,0,1,1}; double bs=1e300;
        vector<pair<ll,int>> buf(C.size());
        for(int a=1;a<=L;a++) for(int b=a;b<=L;b++){
            for(size_t i=0;i<C.size();i++) buf[i]={pre[C[i]][b]-pre[C[i]][a-1], C[i]};
            sort(buf.begin(),buf.end());
            if(buf.front().first==buf.back().first) continue;
            double sc=0; size_t i=0;
            while(i<buf.size()){
                size_t j=i; bool same=true; ll a0=ansv[buf[i].second]; double W=0;
                while(j<buf.size() && buf[j].first==buf[i].first){ if(ansv[buf[j].second]!=a0) same=false; W+=cw[buf[j].second]; j++; }
                if(!same) sc+=W*log2(1.0+(double)(j-i));
                i=j;
            }
            if(sc<bs){ bs=sc; best={0,0,a,b}; }
        }
        return best;
    }
};
static Endgame EG; static bool EGbuilt=false;

struct Br { ll A,B; int lo,hi; ll Plo,Phi; bool exact; };

struct Solver {
    int n=0; ll S=0; ll CAP=0;
    map<int,ll> K;   // pos -> prefix sum
    map<ll,int> V;   // prefix sum -> pos
    map<ll,vector<ll>> widths; map<ll,vector<int>> lastq;
    ll getP(int m){
        auto it=K.find(m); if(it!=K.end()) return it->second;
        ll v = ask(1,m); K[m]=v; V[v]=m; return v;
    }
    void setExact(ll T, int pos){ if(!K.count(pos)){ K[pos]=T; V[T]=pos; } }
    static int lg2(ll x){ return 63-__builtin_clzll(x); }
    // min number of aligned dyadic blocks (size <= CAP) tiling [a,b)
    ll dyad(ll a, ll b){
        ll c=0;
        while(a<b){
            ll v = a==0 ? (1LL<<62) : (a & -a);
            if(v>CAP) v=CAP;
            while(a+v>b) v>>=1;
            a+=v; c++;
        }
        return c;
    }
    Br bracket(ll T){
        Br r; r.exact=false;
        auto it=V.lower_bound(T);
        if(it!=V.end() && it->first==T){ r.A=r.B=it->second; r.lo=r.hi=it->second; r.Plo=r.Phi=T; r.exact=true; return r; }
        auto hi=it; auto lo=prev(it);
        r.lo=lo->second; r.hi=hi->second; r.Plo=lo->first; r.Phi=hi->first;
        r.A = max<ll>(r.lo + dyad(r.Plo,T), r.hi - (r.Phi - T));
        r.B = min<ll>(r.lo + (T - r.Plo), r.hi - dyad(T,r.Phi));
        if(r.A==r.B){ r.exact=true; setExact(T,(int)r.A); }
        return r;
    }
    ll estimate(ll T, const Br& b){
        ll lp=b.A, hp=b.B;
        auto& w=widths[T];
        bool bis=false;
        w.push_back(hp-lp+1);
        if(FALLBACK_K>0 && (int)w.size()>=FALLBACK_K+1){
            if(w.back()*2 > w[w.size()-1-FALLBACK_K]) bis=true;
        }
        ll m;
        if(bis) m=(lp+hp)/2;
        else {
            auto& lq=lastq[T];
            double est = b.lo + (double)(T-b.Plo)*(b.hi-b.lo)/(double)(b.Phi-b.Plo);
            if(lq.size()>=2){
                int q1=lq[lq.size()-1], q2=lq[lq.size()-2];
                ll P1=K[q1],P2=K[q2];
                if(P1!=P2){
                    double sec=q1+(double)(T-P1)*(q1-q2)/(double)(P1-P2);
                    if(sec>b.lo && sec<b.hi) est=sec;
                }
            }
            m=llround(est);
        }
        m=max(m,lp); m=min(m,hp);
        return m;
    }
    void stepTarget(ll T){
        Br b=bracket(T);
        if(b.exact) return;
        int m=(int)estimate(T,b);
        getP(m); lastq[T].push_back(m);
    }
    struct Rng{ ll mn,mx; };
    // range of f = 2pm - pl - pr under bracket and difference constraints
    Rng frange(ll Al,ll Bl,ll Am,ll Bm,ll Ar,ll Brr,ll c1,ll d1,ll c2,ll d2){
        ll lo=max({Am, Al+c1, Ar-d2}), hi=min({Bm, Bl+d1, Brr-c2});
        Rng r{LLONG_MAX,LLONG_MIN};
        if(lo<=hi){
            ll pts[6]={lo,hi,Al+d1,Ar-c2,Bl+c1,Brr-d2};
            for(ll pm: pts){
                if(pm<lo||pm>hi) continue;
                ll xl=max(c1, pm-Bl), xh=min(d1, pm-Al);
                ll yl=max(c2, Ar-pm), yh=min(d2, Brr-pm);
                if(xl>xh||yl>yh) continue;
                r.mn=min(r.mn,xl-yh); r.mx=max(r.mx,xh-yl);
            }
        }
        if(r.mn>r.mx){ r.mn=LLONG_MIN; r.mx=LLONG_MAX; } // should not happen
        return r;
    }
    // exhaustive endgame on interval (pl, pr] with P(pl)=Tl, sum s
    ll endgame(int pl, int pr, ll Tl, ll s){
        int L=pr-pl;
        if(L<1 || L>LIM) return -1;
        if(!EGbuilt){ EG.build(LIM); EGbuilt=true; }
        int e=lg2(s);
        EG.L=L; EG.pre.clear(); EG.ansv.clear(); EG.memo.clear(); EG.cw.clear();
        vector<int> C;
        const auto& P=EG.pats[L];
        for(size_t i=0;i<P.size();i++){
            const auto& p=P[i];
            bool ok=true; int mnd=INT_MAX;
            for(int d:p){ if(d>e){ok=false;break;} mnd=min(mnd,d); }
            if(!ok) continue;
            ll mx=s>>mnd; if(mx>(1LL<<30) || mx>CAP) continue;
            vector<ll> pr_(L+1,0); for(int j=0;j<L;j++) pr_[j+1]=pr_[j]+(s>>p[j]);
            for(auto it=K.upper_bound(pl); it!=K.end() && it->first<pr; ++it){
                if(pr_[it->first-pl]!=it->second-Tl){ok=false;break;}
            }
            if(!ok) continue;
            C.push_back((int)EG.pre.size()); EG.pre.push_back(pr_); EG.ansv.push_back(mx); EG.cw.push_back(EG.pw[L][i]);
        }
        if(C.empty()) return -1;
        while(true){
            bool same=true; for(int c:C) if(EG.ansv[c]!=EG.ansv[C[0]]){same=false;break;}
            if(same) return EG.ansv[C[0]];
            Endgame::Res r;
            if((ll)C.size()*L*L <= EXACTW) r=EG.solve(C);
            else r=EG.greedy(C);
            ll v=ask(pl+r.a, pl+r.b);
            if(r.a==1){ // prefix information, keep it for a possible fallback
                int pos=pl+r.b; if(!K.count(pos)){ K[pos]=Tl+v; V[Tl+v]=pos; }
            }
            vector<int> C2; for(int c:C) if(EG.pre[c][r.b]-EG.pre[c][r.a-1]==v) C2.push_back(c);
            C.swap(C2);
            if(C.empty()) return -1;
        }
    }
    ll run(){
        if(n==1) return ask(1,1);
        K[0]=0; V[0]=0;
        S=getP(n);
        CAP=min<ll>(1LL<<30, S/2);
        ll Tl=0, Tr=S;
        ll egS=-1;
        while(true){
            ll s=Tr-Tl;
            Br bl=bracket(Tl), br=bracket(Tr);
            ll Lmin=max<ll>(br.A-bl.B, dyad(Tl,Tr)), Lmax=min<ll>(br.B-bl.A, s);
            // feasible answers for this interval
            set<ll> ansset;
            if(Lmin<=1) ansset.insert(s);
            int e=lg2(s);
            for(int d=1; d<=e && (1LL<<d)<=Lmax; d++){
                ll M=s>>d; if(M>(1LL<<30)) continue;
                ll lo=max<ll>(Lmin,1LL<<d), hi=min<ll>(Lmax,1+s-M);
                if(lo<=hi) ansset.insert(M);
            }
            if(ansset.size()==1) return *ansset.begin();
            if(bl.exact && br.exact && Lmax<=LIM && LIM>0 && egS!=s){
                egS=s;
                ll r=endgame((int)bl.A, (int)br.A, Tl, s);
                if(r>0) return r;
                continue;
            }
            if(Lmin<=1){
                if(bl.B-bl.A >= br.B-br.A) stepTarget(Tl); else stepTarget(Tr);
                continue;
            }
            ll Tm=Tl+s/2;
            CAP=min(CAP,s/2);
            Br bm=bracket(Tm);
            ll c1=dyad(Tl,Tm), d1=Tm-Tl, c2=dyad(Tm,Tr), d2=Tr-Tm;
            Rng fr=frange(bl.A,bl.B,bm.A,bm.B,br.A,br.B,c1,d1,c2,d2);
            ll mn=fr.mn, mx=fr.mx;
            ll wl=bl.B-bl.A, wm=bm.B-bm.A, wr=br.B-br.A;
            if(mx<=0 || (wl==0 && wm==0 && wr==0 && mn<0)){
                if(mn==0 && mx==0){
                    Br b1=bracket(Tl+s/4), b2=bracket(Tm+s/4);
                    if(b1.B-b1.A <= b2.B-b2.A) Tr=Tm; else Tl=Tm;
                } else Tr=Tm;
                continue;
            }
            if(mn>=0){ Tl=Tm; continue; }
            if(wm>0 && 2*wm>=wl && 2*wm>=wr) stepTarget(Tm);
            else if(wl>0 && wl>=wr) stepTarget(Tl);
            else stepTarget(Tr);
        }
    }
};

#ifndef LOCAL_TEST
int main(){
    int n; if(scanf("%d",&n)!=1) return 0;
    if(n<0) return 0;
    Solver s; s.n=n;
    ll ans=s.run();
    printf("! %lld\n", ans); fflush(stdout);
    return 0;
}
#endif

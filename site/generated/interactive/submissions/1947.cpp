#include <bits/stdc++.h>
using namespace std;

static inline int beatsOf(int a){ return (a+2)%3; }   // opponent move that our move a beats
static inline int counterOf(int b){ return (b+1)%3; } // our move that beats opponent move b

/* ===========================================================================
   PHASE 1 — exact minimax-optimal play against the most likely strategy set:
   3 constants (always R/P/S) plus 3 reactive strategies (copy / beat / lose to
   the player's previous move).  Two models are solved:
      P6  : reactive strategies open with Rock
      P12 : arbitrary (unknown) opening move of the reactive strategies
   Belief state is (mask of surviving hypotheses, previous own move, streak);
   feedback is binary so a round carries at most one bit.  Value iteration
   gives the optimal worst-case number of rounds.  If a model's belief dies,
   play switches to the wider model and finally to the broad fallback.
   =========================================================================== */
struct Policy {
    int n, masks;
    vector<array<int,3>> F;
    vector<int> INIT;
    vector<int> V;
    vector<signed char> POL;
    vector<int> SPLIT;
    int idx(int mask,int la,int s) const { return (mask*4+la)*10+s; }
    int si(int mask,int la,int a) const { return (mask*4+la)*3+a; }
    void build(){
        V.assign((size_t)masks*4*10, 1000000000);
        POL.assign((size_t)masks*4*10, 0);
        SPLIT.assign((size_t)masks*4*3, 0);
        for(int mask=1; mask<masks; mask++)
            for(int la=0; la<4; la++)
                for(int a=0; a<3; a++){
                    int wm=0;
                    for(int h=0; h<n; h++) if(mask>>h&1){
                        int p = (la==3) ? INIT[h] : F[h][la];
                        if(p==beatsOf(a)) wm |= (1<<h);
                    }
                    SPLIT[si(mask,la,a)]=wm;
                }
        const int INF=1000000000;
        for(int it=0; it<400; it++){
            bool changed=false;
            for(int mask=1; mask<masks; mask++)
                for(int la=0; la<4; la++)
                    for(int s=0; s<10; s++){
                        int id=idx(mask,la,s);
                        int best=INF;
                        for(int a=0;a<3;a++){
                            int wm=SPLIT[si(mask,la,a)];
                            int lm=mask^wm;
                            int worst=0;
                            if(wm){
                                int ns=s+1; int c;
                                if(ns>=10) c=1;
                                else { int nx=idx(wm,a,ns); c=(V[nx]>=INF/2)?INF:1+V[nx]; }
                                worst=c;
                            }
                            if(lm && worst<INF){
                                int nx=idx(lm,a,0);
                                int c=(V[nx]>=INF/2)?INF:1+V[nx];
                                if(c>worst) worst=c;
                            }
                            if(worst<best) best=worst;
                        }
                        if(best<V[id]){ V[id]=best; changed=true; }
                    }
            if(!changed) break;
        }
        for(int mask=1; mask<masks; mask++)
            for(int la=0; la<4; la++)
                for(int s=0; s<10; s++){
                    int id=idx(mask,la,s);
                    int best=INF, ba=0;
                    for(int a=0;a<3;a++){
                        int wm=SPLIT[si(mask,la,a)];
                        int lm=mask^wm;
                        int worst=0;
                        if(wm){
                            int ns=s+1; int c;
                            if(ns>=10) c=1;
                            else { int nx=idx(wm,a,ns); c=(V[nx]>=INF/2)?INF:1+V[nx]; }
                            worst=c;
                        }
                        if(lm && worst<INF){
                            int nx=idx(lm,a,0);
                            int c=(V[nx]>=INF/2)?INF:1+V[nx];
                            if(c>worst) worst=c;
                        }
                        if(worst<best){ best=worst; ba=a; }
                    }
                    POL[id]=(signed char)ba;
                }
    }
    int action(int mask,int la,int s) const { return POL[idx(mask,la,s)]; }
    int winPart(int mask,int la,int a) const { return SPLIT[si(mask,la,a)]; }
};

static Policy P6, P12;

static void buildPhase1(){
    int fam[3][3]={{0,1,2},{1,2,0},{2,0,1}}; // copy, beat, lose to player's last move
    {
        P6.n=6; P6.masks=64;
        P6.F.resize(6); P6.INIT.resize(6);
        for(int c=0;c<3;c++){ P6.F[c]={c,c,c}; P6.INIT[c]=c; }
        for(int t=0;t<3;t++){ P6.F[3+t]={fam[t][0],fam[t][1],fam[t][2]}; P6.INIT[3+t]=0; }
        P6.build();
    }
    {
        P12.n=12; P12.masks=4096;
        P12.F.resize(12); P12.INIT.resize(12);
        for(int c=0;c<3;c++){ P12.F[c]={c,c,c}; P12.INIT[c]=c; }
        for(int t=0;t<3;t++) for(int i=0;i<3;i++){
            int h=3+t*3+i;
            P12.F[h]={fam[t][0],fam[t][1],fam[t][2]};
            P12.INIT[h]=i;
        }
        P12.build();
    }
}

/* ===========================================================================
   PHASE 2 — broad fallback.

   Hypothesis families (all simple deterministic programs):
     type 0: b_t = f(player's move k rounds ago)          k = 1,2
     type 1: b_t = f(own move k rounds ago)               k = 1,2
     type 2: fixed periodic sequence, period 1..4
     type 3: majority of player's moves, relation + tie-break
     type 4: b_t = F(player's previous move, own previous move)  (full table)

   Strategy: if every surviving hypothesis predicts the same next move and the
   whole remaining winning streak is forced, take the win; otherwise play the
   move that splits the surviving set as evenly as possible (ties prefer the
   move that wins for more hypotheses).
   =========================================================================== */
struct Hyp {
    unsigned char type, k;
    unsigned char f[9];
    unsigned char init[2];
    unsigned char lb0, lb1;   // predicted own previous move, and the one before
    bool alive;
};

static vector<Hyp> H2;
static vector<int> A2;   // our actions so far

static inline int predH(const Hyp &h, int t, const vector<int> &A, int lb0, int lb1){
    switch(h.type){
        case 0:
            if(t<=h.k) return h.init[t-1];
            return h.f[A[t-1-h.k]];
        case 1:
            if(t<=h.k) return h.init[t-1];
            return h.f[h.k==1 ? lb0 : lb1];
        case 2:
            return h.f[(t-1)%h.k];
        case 3: {
            if(t<=1) return h.init[0];
            int c0=0,c1=0,c2=0;
            for(int i=0;i<t-1;i++){ int x=A[i]; if(x==0)c0++; else if(x==1)c1++; else c2++; }
            int mx=max(c0,max(c1,c2));
            int pick;
            if(h.k==0) pick = (c0==mx)?0:((c1==mx)?1:2);
            else if(h.k==1) pick = (c2==mx)?2:((c1==mx)?1:0);
            else{
                pick=0;
                for(int i=t-2;i>=0;i--){
                    int x=A[i];
                    int cx=(x==0?c0:(x==1?c1:c2));
                    if(cx==mx){ pick=x; break; }
                }
            }
            return (pick + h.f[0])%3;
        }
        case 4: {
            if(t<=1) return h.init[0];
            return h.f[A[t-2]*3 + lb0];
        }
        default: {
            if(t<=2) return h.init[t-1];
            return h.f[A[t-3]*3 + A[t-2]];
        }
    }
}

static void addFa(int k){
    for(int f0=0;f0<3;f0++)for(int f1=0;f1<3;f1++)for(int f2=0;f2<3;f2++){
        int cnt=(k==1)?3:9;
        for(int ii=0; ii<cnt; ii++){
            Hyp h{}; h.type=0; h.k=k; h.f[0]=f0; h.f[1]=f1; h.f[2]=f2;
            if(k==1){ h.init[0]=ii; h.init[1]=0; }
            else { h.init[0]=ii%3; h.init[1]=ii/3; }
            h.alive=true; H2.push_back(h);
        }
    }
}
static void addGb(int k){
    for(int f0=0;f0<3;f0++)for(int f1=0;f1<3;f1++)for(int f2=0;f2<3;f2++){
        int cnt=(k==1)?3:9;
        for(int ii=0; ii<cnt; ii++){
            Hyp h{}; h.type=1; h.k=k; h.f[0]=f0; h.f[1]=f1; h.f[2]=f2;
            if(k==1){ h.init[0]=ii; h.init[1]=0; }
            else { h.init[0]=ii%3; h.init[1]=ii/3; }
            h.alive=true; H2.push_back(h);
        }
    }
}
static void addPer(int p){
    int total=1; for(int i=0;i<p;i++) total*=3;
    for(int code=0; code<total; code++){
        Hyp h{}; h.type=2; h.k=p;
        int c=code;
        for(int i=0;i<p;i++){ h.f[i]=c%3; c/=3; }
        h.alive=true; H2.push_back(h);
    }
}
static void addMaj(){
    for(int rel=0;rel<3;rel++) for(int tb=0;tb<3;tb++) for(int ini=0;ini<3;ini++){
        Hyp h{}; h.type=3; h.k=tb; h.f[0]=rel; h.init[0]=ini;
        h.alive=true; H2.push_back(h);
    }
}
static void addF(){
    // full table F(player prev, own prev) + first move
    for(int code=0; code<19683; code++){
        unsigned char tab[9];
        int c=code;
        for(int i=0;i<9;i++){ tab[i]=c%3; c/=3; }
        for(int ini=0; ini<3; ini++){
            Hyp h{}; h.type=4; h.k=1;
            for(int i=0;i<9;i++) h.f[i]=tab[i];
            h.init[0]=ini;
            h.alive=true; H2.push_back(h);
        }
    }
}
static void addF2(){
    // full table F2(player's move t-1, player's move t-2) + first two moves
    for(int code=0; code<19683; code++){
        unsigned char tab[9];
        int c=code;
        for(int i=0;i<9;i++){ tab[i]=c%3; c/=3; }
        for(int i0=0;i0<3;i0++) for(int i1=0;i1<3;i1++){
            Hyp h{}; h.type=5; h.k=1;
            for(int i=0;i<9;i++) h.f[i]=tab[i];
            h.init[0]=i0; h.init[1]=i1;
            h.alive=true; H2.push_back(h);
        }
    }
}
static void buildPhase2(){
    addFa(1); addFa(2);
    addGb(1); addGb(2);
    addPer(1); addPer(2); addPer(3); addPer(4);
    addMaj();
    addF();
    addF2();
}

static bool canWinPhase2(int need, int aliveCount){
    if(aliveCount==0) return false;
    static vector<int> simA, idxs;
    static vector<unsigned char> s0, s1;
    simA = A2;
    int t0=(int)A2.size()+1;
    idxs.clear(); s0.clear(); s1.clear();
    for(int i=0;i<(int)H2.size();i++) if(H2[i].alive){
        idxs.push_back(i); s0.push_back(H2[i].lb0); s1.push_back(H2[i].lb1);
    }
    for(int step=0; step<need; step++){
        int t=t0+step;
        int b0=-1;
        for(size_t j=0;j<idxs.size();j++){
            int b=predH(H2[idxs[j]],t,simA,s0[j],s1[j]);
            if(b0<0) b0=b;
            else if(b!=b0) return false;
        }
        simA.push_back(counterOf(b0));
        for(size_t j=0;j<idxs.size();j++){ s1[j]=s0[j]; s0[j]=(unsigned char)b0; }
    }
    return true;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    buildPhase1();
    buildPhase2();

    int m6  = 63;
    int m12 = 4095;
    int la  = 3;
    int streak = 0;
    bool phase1 = true;
    int totalRounds = 0;
    const int PHASE1_CAP = 30;

    while(true){
        int aliveCount = 0;
        for(auto &h : H2) if(h.alive) aliveCount++;
        if(phase1 && totalRounds>=PHASE1_CAP) phase1=false;

        int a;
        if(phase1 && m6!=0){
            a = P6.action(m6, la, streak);
        } else if(phase1 && m12!=0){
            a = P12.action(m12, la, streak);
        } else {
            phase1=false;
            int need = 10-streak;
            if(aliveCount==0){
                a = counterOf(la);
            } else if(canWinPhase2(need, aliveCount)){
                int t=(int)A2.size()+1;
                int b=-1;
                for(auto &h:H2) if(h.alive){ b=predH(h,t,A2,h.lb0,h.lb1); break; }
                a = counterOf(b);
            } else {
                int t=(int)A2.size()+1;
                int bestA=-1, bestMax=INT_MAX, bestW=-1;
                for(int cand=0; cand<3; cand++){
                    int w=0;
                    for(auto &h:H2) if(h.alive){
                        if(predH(h,t,A2,h.lb0,h.lb1)==beatsOf(cand)) w++;
                    }
                    int l=aliveCount-w;
                    int mx=max(w,l);
                    bool better = (mx<bestMax) || (mx==bestMax && w>bestW) ||
                                  (mx==bestMax && w==bestW && cand>bestA);
                    if(better){ bestMax=mx; bestW=w; bestA=cand; }
                }
                a=bestA;
            }
        }

        cout << "RPS"[a] << "\n" << flush;
        string resp;
        if(!(cin>>resp)) return 0;
        for(char &ch : resp) ch = (char)tolower((unsigned char)ch);
        bool win = (resp=="player");
        int t=(int)A2.size()+1;

        for(auto &h : H2) if(h.alive){
            int b=predH(h,t,A2,h.lb0,h.lb1);
            bool pw = (b==beatsOf(a));
            if(pw==win){
                h.lb1=h.lb0; h.lb0=(unsigned char)b;
            } else h.alive=false;
        }
        A2.push_back(a);

        if(phase1){
            int w6=P6.winPart(m6,la,a);
            m6 = win ? w6 : (m6^w6);
            int w12=P12.winPart(m12,la,a);
            m12 = win ? w12 : (m12^w12);
        }
        la = a;
        streak = win ? streak+1 : 0;
        totalRounds++;
        if(streak>=10) return 0;
    }
    return 0;
}

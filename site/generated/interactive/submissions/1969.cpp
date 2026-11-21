// Slot Machine - interactive solution
#include <bits/stdc++.h>
using namespace std;

static int n;
static int curK;
static long long nMoves = 0;
static inline double elapsedSec(){ return (double)std::clock() / (double)CLOCKS_PER_SEC; }

#ifdef DEBUGLOG
#define DBG(x) do{ cerr << x << "\n"; }while(0)
#else
#define DBG(x) do{}while(0)
#endif

static void sendMove(int i, long long j){
    long long jj = ((j % n) + n) % n;
    cout << i << " " << jj << "\n";
    cout.flush();
    ++nMoves;
    DBG("move " << i << " " << jj << "  (moves=" << nMoves << ")");
}
static bool readCount(){
    if(!(cin >> curK)) return false;
    DBG("   k=" << curK);
    return true;
}

static int findEmpty(const set<int>& S){
    for(int v=0; v<n; ++v) if(!S.count(v)) return v;
    return -1;
}
static bool isPeriodic(const set<int>& S){
    int sz = (int)S.size();
    if(sz == 0 || sz == n) return true;
    for(int u=1; u<n; ++u){
        bool ok = true;
        for(int v : S){ if(!S.count((v+u)%n)){ ok=false; break; } }
        if(ok) return true;
    }
    return false;
}

struct Hyp { int x, c; };

static inline int predCount(const Hyp& h, int rot, int j, const set<int>& S){
    int dest = (h.x + rot + j) % n;
    if(h.c == 0){
        return (int)S.size() + (S.count(dest) ? 0 : 1);
    } else {
        int ks = (int)S.size() - 1;
        bool inK = (dest != h.x) && (S.count(dest) != 0);
        return ks + (inK ? 0 : 1);
    }
}

// Build hypothesis set consistent with knowledge.
static vector<Hyp> buildHyps(const set<int>& S, const vector<int>& knownAt, int r, bool allDistinct){
    vector<int> U;
    for(int x : S) if(knownAt[x] == 0) U.push_back(x);
    bool tight = ((int)U.size() == r);
    vector<Hyp> H;
    if(tight){
        for(int x : U) H.push_back({x, 1});
    } else if(allDistinct){
        for(int x : S){
            if(knownAt[x] == 0) H.push_back({x, 1});
            else H.push_back({x, 0});
        }
    } else {
        for(int x : S){
            bool canC1 = (knownAt[x] == 0);
            bool canC0 = (knownAt[x] >= 1) || (r >= 2);
            if(canC0) H.push_back({x, 0});
            if(canC1) H.push_back({x, 1});
        }
    }
    if(H.empty()){
        for(int x : S){ H.push_back({x,0}); if(knownAt[x]==0) H.push_back({x,1}); }
    }
    return H;
}

// score a candidate action j: returns key (6 ints, lexicographic, minimise)
static inline void evalKey(const vector<Hyp>& H, int rot, int j, const set<int>& S, int base, int* key){
    // base = |S| ; possible preds: base-1, base, base+1 -> groups 0,1,2
    int dx[3] = {0,0,0};
    int hc[3] = {0,0,0};
    static int tag[3][64];
    static int stamp = 0;
    ++stamp;
    for(auto& h : H){
        int pc = predCount(h, rot, j, S);
        int g = pc - (base-1);
        if(g<0) g=0; if(g>2) g=2;
        if(tag[g][h.x] != stamp){ tag[g][h.x] = stamp; dx[g]++; }
        hc[g]++;
    }
    // sort desc
    for(int a=0;a<2;++a) for(int b=a+1;b<3;++b){
        if(dx[b]>dx[a]) swap(dx[a],dx[b]);
        if(hc[b]>hc[a]) swap(hc[a],hc[b]);
    }
    key[0]=dx[0]; key[1]=dx[1]; key[2]=dx[2];
    key[3]=hc[0]; key[4]=hc[1]; key[5]=hc[2];
}

static double estTree(const vector<Hyp>& H, int rot, const set<int>& S, int depth, long long& budget){
    if(budget <= 0) return 60.0;
    --budget;
    int m = (int)H.size();
    if(m <= 1) return 0.0;
    bool same = true;
    for(auto& h : H) if(h.x != H[0].x){ same=false; break; }
    if(same) return 0.0;
    if(depth > 45) return 45.0;
    int base = (int)S.size();
    int bestJ=-1; int bestKey[6];
    for(int j=1;j<n;++j){
        int key[6];
        evalKey(H, rot, j, S, base, key);
        bool better=false;
        if(bestJ<0) better=true;
        else for(int t=0;t<6;++t){ if(key[t]<bestKey[t]){better=true;break;} if(key[t]>bestKey[t])break; }
        if(better){ bestJ=j; memcpy(bestKey,key,sizeof(key)); }
    }
    if(bestJ<0) return 0.0;
    vector<Hyp> g[3];
    for(auto& h : H){
        int pc = predCount(h, rot, bestJ, S);
        int gi = pc - (base-1);
        if(gi<0) gi=0; if(gi>2) gi=2;
        g[gi].push_back(h);
    }
    int nrot = (rot + bestJ) % n;
    double tot = 0;
    for(int t=0;t<3;++t){
        if(g[t].empty()) continue;
        tot += (double)g[t].size() * estTree(g[t], nrot, S, depth+1, budget);
    }
    return 1.0 + tot / (double)m;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if(!(cin >> n)) return 0;
    if(!readCount()) return 0;
    if(curK == 1) return 0;

    // ---------------- learn the support ----------------
    set<int> S;                     // support, wheel1 is at 0
    if(curK == n){
        // every position occupied => all wheels distinct, support = everything
        for(int v=0; v<n; ++v) S.insert(v);
    } else {
        vector<int> cnt(n);
        cnt[0] = curK;
        for(int t=1; t<n; ++t){
            sendMove(1, 1);
            if(!readCount()) return 0;
            cnt[t] = curK;
            if(curK == 1) return 0;
        }
        int minC = *min_element(cnt.begin(), cnt.end());
        // move wheel 1 back to its original position (target = original grouping)
        for(int t=0; t<n; ++t) if(cnt[t] == minC) S.insert((t+1)%n);
        S.insert(0);
        if(((int)S.size()*2 < n)){
            sendMove(1, 1);
            if(!readCount()) return 0;
            if(curK == 1) return 0;
            set<int> S2;
            for(int t=0; t<n; ++t) if(cnt[t] == minC) S2.insert(t);
            S2.insert(0);
            S = S2;
        }
    }

    bool allDistinct = ((int)S.size() == n);   // currently every position holds exactly one wheel
    vector<int> knownAt(n, 0);
    knownAt[0] = 1;
    vector<int> wpos(n+1, -1);
    wpos[1] = 0;
    vector<int> gathered;
    vector<int> strays;
    int r = n-1;

    DBG("after sweep support=" << (int)S.size() << " curK=" << curK);

    vector<int> order;
    for(int i=2;i<=n;++i) order.push_back(i);
    {
        unsigned long long rs = 12345678901234567ULL ^ (unsigned long long)n;
        for(int i=(int)order.size()-1;i>0;--i){
            rs ^= rs<<13; rs ^= rs>>7; rs ^= rs<<17;
            int j = (int)(rs % (unsigned long long)(i+1));
            swap(order[i], order[j]);
        }
    }
    for(int oi=0; oi<(int)order.size(); ++oi){
        int i = order[oi];
        // ---- ensure S is aperiodic ----
        for(int attempt=0; attempt<8 && isPeriodic(S); ++attempt){
            if(!gathered.empty()){
                int w = gathered.back(); gathered.pop_back();
                int e = findEmpty(S);
                if(e < 0) break;
                sendMove(w, e);
                if(!readCount()) return 0;
                if(curK == 1) return 0;
                S.insert(e);
                knownAt[e] = 1;
                if(knownAt[0] > 0) knownAt[0]--;
                wpos[w] = e;
                strays.push_back(w);
                DBG("symbreak stray wheel " << w << " -> " << e);
            } else {
                if((int)S.size() < n){
                    int e = findEmpty(S);
                    if(e < 0) break;
                    int prev = curK;
                    sendMove(1, e);
                    if(!readCount()) return 0;
                    if(curK == 1) return 0;
                    bool alone = ((curK - prev) == 0);
                    set<int> Sa = S;
                    if(alone) Sa.erase(0);
                    Sa.insert(e);
                    set<int> S2;
                    for(int v : Sa) S2.insert(((v - e) % n + n) % n);
                    S = S2;
                    knownAt.assign(n, 0);
                    knownAt[0] = alone ? 1 : 2;
                    for(int w : strays){
                        if(wpos[w] < 0) continue;
                        wpos[w] = ((wpos[w] - e) % n + n) % n;
                        knownAt[wpos[w]]++;
                    }
                    DBG("symbreak wheel1 -> " << e << " alone=" << alone);
                } else {
                    sendMove(1, 1);
                    if(!readCount()) return 0;
                    if(curK == 1) return 0;
                    set<int> Sa;
                    for(int v=0; v<n; ++v) if(v != 0) Sa.insert(v);
                    Sa.insert(1);
                    set<int> S2;
                    for(int v : Sa) S2.insert(((v - 1) % n + n) % n);
                    S = S2;
                    knownAt.assign(n, 0);
                    knownAt[0] = 2;
                    DBG("symbreak wheel1 +1 (full support)");
                }
            }
        }

        vector<Hyp> H = buildHyps(S, knownAt, r, allDistinct);

        // ---- adaptive landmark parking ----
        const double PARK_THRESH = 3;
        const int PARK_LIMIT = 30;
        while(true){
            if((int)strays.size() >= PARK_LIMIT) break;
            if(gathered.empty()) break;
            int nEmpty = n - (int)S.size();
            if(nEmpty < 3) break;
            if(elapsedSec() > 0.22) break;
            long long budget = 300000;
            double cur = estTree(H, 0, S, 0, budget);
            if(cur <= PARK_THRESH) break;
            vector<int> empties;
            for(int v=0; v<n; ++v) if(!S.count(v)) empties.push_back(v);
            int step = max(1, (int)empties.size()/12);
            double best = cur; int bestPos = -1;
            for(int idx = 0; idx < (int)empties.size(); idx += step){
                if(elapsedSec() > 0.40) break;
                int e = empties[idx];
                set<int> S2 = S; S2.insert(e);
                vector<int> kA2 = knownAt; kA2[e] = 1; if(kA2[0]>0) kA2[0]--;
                vector<Hyp> H2 = buildHyps(S2, kA2, r, allDistinct);
                long long bud2 = 300000;
                double c2 = estTree(H2, 0, S2, 0, bud2);
                if(c2 < best - 0.25){ best = c2; bestPos = e; }
            }
            if(bestPos < 0) break;
            int w = gathered.back(); gathered.pop_back();
            sendMove(w, bestPos);
            if(!readCount()) return 0;
            if(curK == 1) return 0;
            S.insert(bestPos);
            knownAt[bestPos] = 1;
            if(knownAt[0] > 0) knownAt[0]--;
            wpos[w] = bestPos;
            strays.push_back(w);
            H = buildHyps(S, knownAt, r, allDistinct);
            DBG("park wheel " << w << " at " << bestPos << " est " << cur << " -> " << best);
        }

        // ---- identify x ----
        int rot = 0;
        int noProg = 0;
        while(true){
            bool same = true;
            for(auto& h : H) if(h.x != H[0].x){ same = false; break; }
            if(same) break;
            int base = (int)S.size();
            int bestJ = -1; int bestKey[6];
            for(int j=1; j<n; ++j){
                int key[6];
                evalKey(H, rot, j, S, base, key);
                bool better=false;
                if(bestJ<0) better=true;
                else for(int t=0;t<6;++t){ if(key[t]<bestKey[t]){better=true;break;} if(key[t]>bestKey[t])break; }
                if(better){ bestJ=j; memcpy(bestKey,key,sizeof(key)); }
            }
            if(bestJ < 0) break;
            sendMove(i, bestJ);
            if(!readCount()) return 0;
            if(curK == 1) return 0;
            vector<Hyp> H2;
            for(auto& h : H){
                int pc = predCount(h, rot, bestJ, S);
                if(pc == curK) H2.push_back(h);
            }
            rot = (rot + bestJ) % n;
            if(H2.empty()){
                H = buildHyps(S, knownAt, r, allDistinct);
                ++noProg;
                if(noProg > 2*n+5) break;
            } else if(H2.size() == H.size()){
                H = H2;
                ++noProg;
                if(noProg > 2*n+5) break;
            } else {
                H = H2;
                noProg = 0;
            }
        }

        int x = H[0].x;
        int p = (x + rot) % n;
        bool cKnown = true;
        for(auto& h : H) if(h.c != H[0].c){ cKnown = false; break; }
        int c = H[0].c;

        if(p != 0){
            int j = (n - p) % n;
            sendMove(i, j);
            if(!readCount()) return 0;
            c = (int)S.size() - curK;
            if(curK == 1) return 0;
        } else if(!cKnown){
            if((int)S.size() < n){
                int e = findEmpty(S);
                sendMove(i, e);
                if(!readCount()) return 0;
                if(curK == 1) return 0;
                c = (int)S.size() - (curK - 1);
                sendMove(i, (n - e) % n);
                if(!readCount()) return 0;
                if(curK == 1) return 0;
            } else {
                c = 1;
            }
        }
        if(c != 0 && c != 1) c = (c > 1) ? 0 : 1;
        if(c == 1) S.erase(x);
        knownAt[0]++;
        wpos[i] = 0;
        gathered.push_back(i);
        --r;
        DBG("wheel " << i << " at " << x << " c=" << c << " support=" << (int)S.size() << " moves=" << nMoves);
        if(curK == 1) return 0;
    }

    // ---------------- re-gather strays -----------------
    for(int w : strays){
        int p = wpos[w];
        if(p == 0) continue;
        int j = (n - p) % n;
        int prevSize = (int)S.size();
        sendMove(w, j);
        if(!readCount()) return 0;
        if(curK == 1) return 0;
        if(curK < prevSize){
            S.erase(p);
            knownAt[p] = 0;
        }
        knownAt[0]++;
        wpos[w] = 0;
    }

    // ---------------- last resort ----------------
    int guard = 0;
    while(curK != 1 && guard < 8*n){
        int w = 2 + (guard % (n-1));
        int prev = curK;
        sendMove(w, 1);
        ++guard;
        if(!readCount()) return 0;
        if(curK == 1) return 0;
        if(curK > prev){ sendMove(w, n-1); ++guard; if(!readCount()) return 0; if(curK == 1) return 0; }
    }
    return 0;
}

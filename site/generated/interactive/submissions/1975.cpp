#include <bits/stdc++.h>
using namespace std;

#ifdef TEST_MODE
extern int g_n;
extern int g_dbg;
extern int g_attemptUsed;
extern long long g_failProbes;
extern long long g_earlyTried, g_earlyOK, g_removals, g_earlyBuildOK;
extern pair<int,int> g_ask(int d);
extern void g_output(const vector<int>& path);
#define GET_N() g_n
#define ASK(d) g_ask(d)
#define OUTPUT(p) g_output(p)
#else
static int g_n_real = 0;
static pair<int,int> askReal(int d) {
    printf("? %d\n", d);
    fflush(stdout);
    int a, b;
    if (scanf("%d %d", &a, &b) != 2) exit(0);
    return {a, b};
}
static void outputReal(const vector<int>& path) {
    printf("!");
    for (int v : path) printf(" %d", v);
    printf("\n");
    fflush(stdout);
}
#define GET_N() g_n_real
#define ASK(d) askReal(d)
#define OUTPUT(p) outputReal(p)
#endif

static const int INF_TIME = 1000000000;

struct Info {
    int n = 0;
    vector<char> isBlind;
    vector<int> Nbr, Uval, rtime;   // rtime == INF_TIME means unqueried
};

static inline bool certFn(const Info& I, int a, int b) {
    int ta = I.rtime[a], tb = I.rtime[b];
    if (ta == INF_TIME && tb == INF_TIME) return false;
    if (ta < tb) {
        if (!I.isBlind[a]) return b != I.Nbr[a];
        return b < I.Uval[a];
    } else {
        if (!I.isBlind[b]) return a != I.Nbr[b];
        return a < I.Uval[b];
    }
}

struct Builder {
    const Info& I;
    int n;
    vector<int> nxt, prv;
    vector<char> inPath;
    int head = 0, tail = 0;
    vector<int> gl, gr;
    vector<char> galive;
    vector<int> gapPos;
    vector<int> aliveList;
    vector<int> gapAfter;
    priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> hp;
    int scanBudget = 0;
    vector<int> rSlots;   // gaps usable for unqueried vertices
    vector<int> rsPos;
    vector<int> ffSlots;  // subset: gaps whose both endpoints are FULL queried vertices
    vector<int> ffPos;

    Builder(const Info& inf) : I(inf) {
        n = inf.n;
        nxt.assign(n+1, 0); prv.assign(n+1, 0); inPath.assign(n+1, 0);
        gapAfter.assign(n+1, -1);
        gl.push_back(0); gr.push_back(0);
        galive.push_back(false); gapPos.push_back(-1);
    }
    void addGap(int l, int r) {
        int g = (int)gl.size();
        gl.push_back(l); gr.push_back(r); galive.push_back(true);
        gapPos.push_back((int)aliveList.size());
        aliveList.push_back(g);
        int mk = max(l, r);
        hp.push({mk, g});
    }
    void removeGap(int g) {
        galive[g] = false;
        int p = gapPos[g];
        int last = aliveList.back();
        aliveList[p] = last; gapPos[last] = p;
        aliveList.pop_back();
    }
    void placeFirst(int v) {
        head = tail = v; prv[v] = nxt[v] = 0; inPath[v] = 1;
        addGap(0, v); gapAfter[0] = (int)gl.size()-1;
        addGap(v, 0); gapAfter[v] = (int)gl.size()-1;
    }
    void insertAtGap(int g, int v) {
        int l = gl[g], r = gr[g];
        removeGap(g);
        if (l == 0) {
            prv[r] = v; nxt[v] = r; prv[v] = 0; head = v;
            addGap(0, v); gapAfter[0] = (int)gl.size()-1;
            addGap(v, r); gapAfter[v] = (int)gl.size()-1;
        } else if (r == 0) {
            nxt[l] = v; prv[v] = l; nxt[v] = 0; tail = v;
            addGap(l, v); gapAfter[l] = (int)gl.size()-1;
            addGap(v, 0); gapAfter[v] = (int)gl.size()-1;
        } else {
            nxt[l] = v; prv[v] = l; nxt[v] = r; prv[r] = v;
            addGap(l, v); gapAfter[l] = (int)gl.size()-1;
            addGap(v, r); gapAfter[v] = (int)gl.size()-1;
        }
        inPath[v] = 1;
    }
    int findFull(int v, bool preferAppend) {
        int x = I.Nbr[v];
        if (preferAppend) {
            if (tail != 0 && tail != x) return gapAfter[tail];
            if (head != 0 && head != x) return gapAfter[0];
        }
        int cnt = 0;
        for (int i = (int)aliveList.size()-1; i >= 0; --i) {
            int g = aliveList[i];
            int l = gl[g], r = gr[g];
            if (l == x || r == x) { if (++cnt > 2) return -1; continue; }
            return g;
        }
        return -1;
    }
    int findBlind(int v) {
        int u = I.Uval[v];
        while (!hp.empty()) {
            auto pr = hp.top();
            int g = pr.second;
            if (!galive[g]) { hp.pop(); continue; }
            if (pr.first < u) return g;
            return -1;
        }
        return -1;
    }
    int findGeneralR(int v) {
        // Prefer gaps whose both endpoints are FULL queried vertices: for such a gap
        // at most two slots can be unusable for v, so this is constant time.
        for (int i = (int)ffSlots.size()-1; i >= 0; --i) {
            int g = ffSlots[i];
            int l = gl[g], r = gr[g];
            if (l && !certFn(I, v, l)) continue;
            if (r && !certFn(I, v, r)) continue;
            return g;
        }
        int budget = 600;
        for (int i = (int)rSlots.size()-1; i >= 0; --i) {
            int g = rSlots[i];
            int l = gl[g], r = gr[g];
            bool ok = true;
            if (l && !certFn(I, v, l)) ok = false;
            if (ok && r && !certFn(I, v, r)) ok = false;
            if (ok) return g;
            if (--budget == 0) return -1;
        }
        return -1;
    }
    void removeRSlot(int g) {
        int p = rsPos[g];
        if (p >= 0) {
            rsPos[g] = -1;
            int last = rSlots.back();
            rSlots[p] = last; rsPos[last] = p;
            rSlots.pop_back();
        }
        int q2 = ffPos[g];
        if (q2 >= 0) {
            ffPos[g] = -1;
            int last = ffSlots.back();
            ffSlots[q2] = last; ffPos[last] = q2;
            ffSlots.pop_back();
        }
    }
    int findGeneral(int v) {
        int budget = scanBudget;
        for (int i = (int)aliveList.size()-1; i >= 0; --i) {
            int g = aliveList[i];
            int l = gl[g], r = gr[g];
            bool ok = true;
            if (l && !certFn(I, v, l)) ok = false;
            if (ok && r && !certFn(I, v, r)) ok = false;
            if (ok) return g;
            if (budget > 0) { if (--budget == 0) return -1; }
        }
        return -1;
    }
    // Try to place vertices that could not be placed with their own (tight) rule.
    // Fast path uses the tight rule again (cheap), then a bounded general scan.
    int placeDeferred(vector<int>& deferred, bool allowBig) {
        int rounds = 0;
        int maxRounds = allowBig ? 60 : 3;
        while (!deferred.empty()) {
            if (++rounds > maxRounds) return (int)deferred.size();
            bool progress = false;
            vector<int> nd;
            for (int v : deferred) {
                int g = -1;
                bool usedHeapTop = false;
                if (I.rtime[v] != INF_TIME) {
                    if (!I.isBlind[v]) g = findFull(v, false);
                    else { g = findBlind(v); usedHeapTop = (g >= 0); }
                    if (g >= 0) {
                        // the tight rules only certify adjacency to later vertices,
                        // so double-check with the full test
                        int l = gl[g], r = gr[g];
                        bool ok = true;
                        if (l && !certFn(I, v, l)) ok = false;
                        if (ok && r && !certFn(I, v, r)) ok = false;
                        if (!ok) { g = -1; usedHeapTop = false; }
                    }
                }
                if (g < 0) {
                    int save = scanBudget;
                    if (save == 0) scanBudget = 3000;
                    g = findGeneral(v);
                    scanBudget = save;
                }
                if (g >= 0) {
                    if (usedHeapTop) hp.pop();
                    insertAtGap(g, v); progress = true;
                }
                else nd.push_back(v);
            }
            deferred.swap(nd);
            if (!progress) break;
        }
        return (int)deferred.size();
    }
    bool run(const vector<int>& qOrder, const vector<int>& unq, bool preferAppend) {
        vector<int> deferredQ;
        bool first = true;
        for (int v : qOrder) {
            if (first) { placeFirst(v); first = false; continue; }
            int g = -1;
            if (I.rtime[v] != INF_TIME && !I.isBlind[v]) {
                g = findFull(v, preferAppend);
                if (g >= 0) insertAtGap(g, v);
                else deferredQ.push_back(v);
            } else if (I.rtime[v] != INF_TIME) {
                g = findBlind(v);
                if (g >= 0) { hp.pop(); insertAtGap(g, v); }
                else deferredQ.push_back(v);
            } else {
                deferredQ.push_back(v);
            }
        }
        // put back the queried vertices that could not be placed with their tight rule
        sort(deferredQ.begin(), deferredQ.end(), [&](int a, int b) {
            bool ba = I.isBlind[a], bb = I.isBlind[b];
            if (ba != bb) return (int)ba > (int)bb;
            if (ba) return I.Uval[a] < I.Uval[b];
            return a < b;
        });
        if (placeDeferred(deferredQ, true) != 0) return false;
        if (unq.empty()) return true;
        // Insert unqueried vertices.  They have no certified edges between them, so
        // each must go into a gap between two queried vertices.  Order by number of
        // certified partners (most constrained first).
        vector<int> pointBad(n+1, 0), diff(n+2, 0), suffix(n+1, 0);
        for (int v : qOrder) {
            if (I.rtime[v] == INF_TIME) continue;
            if (!I.isBlind[v]) { int x = I.Nbr[v]; if (x) pointBad[x]++; }
            else diff[I.Uval[v]]++;
        }
        int acc = 0;
        for (int x = 1; x <= n; ++x) { acc += diff[x]; suffix[x] = acc; }
        int placed = 0;
        for (int v : qOrder) if (inPath[v]) placed++;
        vector<pair<int,int>> ordR;
        ordR.reserve(unq.size());
        for (int v : unq) ordR.push_back({placed - suffix[v] - pointBad[v], v});
        sort(ordR.begin(), ordR.end());
        rSlots.clear(); ffSlots.clear();
        rsPos.assign(gl.size(), -1); ffPos.assign(gl.size(), -1);
        for (size_t k = 0; k < aliveList.size(); ++k) {
            int g = aliveList[k];
            rsPos[g] = (int)rSlots.size();
            rSlots.push_back(g);
            int l = gl[g], r2 = gr[g];
            if (l && r2 && I.rtime[l] != INF_TIME && I.rtime[r2] != INF_TIME
                && !I.isBlind[l] && !I.isBlind[r2]) {
                ffPos[g] = (int)ffSlots.size();
                ffSlots.push_back(g);
            }
        }
#ifdef TEST_MODE
        if (g_dbg) {
            int ff=0, fb=0, bb=0; int cur=head, prev=0;
            while (cur) { if (prev) { bool bp = I.isBlind[prev], bc = I.isBlind[cur]; if(!bp&&!bc)ff++; else if(!bp||!bc)fb++; else bb++; } prev=cur; cur=nxt[cur]; }
            fprintf(stderr, "  Q path: FF=%d FB=%d BB=%d alive=%d ordR0..3:", ff, fb, bb, (int)aliveList.size());
            for (int kk = 0; kk < 4 && kk < (int)ordR.size(); ++kk) fprintf(stderr, " (%d,%d)", ordR[kk].first, ordR[kk].second);
            fprintf(stderr, "\n");
        }
#endif
        vector<int> leftR;
        for (size_t k = 0; k < ordR.size(); ++k) {
            int v = ordR[k].second;
            int g = findGeneralR(v);
            if (g >= 0) { removeRSlot(g); insertAtGap(g, v); }
            else {
                leftR.push_back(v);
#ifdef TEST_MODE
                if (g_dbg && leftR.size() <= 5) {
                    int validSlots = 0, validAlive = 0;
                    for (int gg : rSlots) { int l2 = gl[gg], r2 = gr[gg]; if ((!l2 || certFn(I,v,l2)) && (!r2 || certFn(I,v,r2))) validSlots++; }
                    for (int gg : aliveList) { int l2 = gl[gg], r2 = gr[gg]; if ((!l2 || certFn(I,v,l2)) && (!r2 || certFn(I,v,r2))) validAlive++; }
                    fprintf(stderr, "  R fail: v=%d cnt=%d slots=%d validSlots=%d aliveValid=%d alive=%d\n", v, ordR[k].first, (int)rSlots.size(), validSlots, validAlive, (int)aliveList.size());
                }
#endif
            }
        }
#ifdef TEST_MODE
        if (g_dbg) fprintf(stderr, "  R pass: %d of %d failed, slots left=%d\n", (int)leftR.size(), (int)ordR.size(), (int)rSlots.size());
#endif
        if (leftR.empty()) return true;
        return placeDeferred(leftR, true) == 0;
    }
    vector<int> extract() const {
        vector<int> p;
        p.reserve(n);
        int cur = head;
        while (cur) { p.push_back(cur); cur = nxt[cur]; }
        return p;
    }
};

static bool verifyPath(const Info& I, const vector<int>& path) {
    int n = I.n;
    if ((int)path.size() != n) return false;
    vector<char> seen(n+1, 0);
    for (int v : path) {
        if (v < 1 || v > n || seen[v]) return false;
        seen[v] = 1;
    }
    for (int i = 0; i + 1 < n; ++i) {
        if (!certFn(I, path[i], path[i+1])) return false;
    }
    return true;
}

// Necessary condition for stopping early: every remaining (unqueried) vertex must
// have at least 2 certified neighbours among the queried ones.
static bool cheapFeasible(const Info& I, const vector<int>& queried, const vector<char>& isQ) {
    int n = I.n;
    int q = (int)queried.size();
    vector<int> pointBad(n+2, 0), diff(n+2, 0);
    for (int v : queried) {
        if (!I.isBlind[v]) {
            int x = I.Nbr[v];
            if (x && !isQ[x]) pointBad[x]++;
        } else {
            int u = I.Uval[v];
            if (u >= 1) diff[u]++;
        }
    }
    int run = 0;
    for (int x = 1; x <= n; ++x) {
        run += diff[x];
        if (!isQ[x]) {
            if (q - run - pointBad[x] < 2) return false;
        }
    }
    return true;
}

static void solve() {
    int n = GET_N();
    Info I;
    I.n = n;
    I.isBlind.assign(n+1, 0);
    I.Nbr.assign(n+1, 0);
    I.Uval.assign(n+1, 0);
    I.rtime.assign(n+1, INF_TIME);
    vector<int> queried;
    queried.reserve(n);
    vector<char> isQ(n+1, 0);

    vector<int> finalPath;
    bool done = false;
    if (n <= 3) {
        // tiny cases: just query everything (cheap) -- handled by generic code
    }

    int qmin = n/2 + 1;
    if (qmin < 1) qmin = 1;
    int step = max(1, (n - qmin) / 8);
    int nextTry = qmin;

    int r = n;
    while (r > 0) {
        int dmax = (r >= 2) ? (r - 2) : 0;
        int d = dmax;
        int usedD = d;
        pair<int,int> res = ASK(d);
        if (res.first == 0 && d > 0) {
#ifdef TEST_MODE
            ++g_failProbes;
#endif
            // core state: all remaining have h >= 2; remove the min-degree (max h) vertex
            usedD = 0;
            res = ASK(0);
        }
        int v = res.first, u = res.second;
        if (v == 0) exit(0);
#ifdef TEST_MODE
        ++g_removals;
#endif
        bool full = (usedD >= r - 2);
        I.rtime[v] = (int)queried.size();
        queried.push_back(v);
        isQ[v] = 1;
        if (full) { I.Nbr[v] = u; I.isBlind[v] = 0; }
        else { I.Uval[v] = u; I.isBlind[v] = 1; }
        --r;

        int q = (int)queried.size();
        if (!done && q >= nextTry && q >= n - q) {
            nextTry = q + step;
            bool ch = cheapFeasible(I, queried, isQ);
#ifdef TEST_MODE
            ++g_earlyTried;
            if (!ch) {} else ++g_earlyOK;
#endif
#ifdef TEST_MODE
            if (g_dbg) fprintf(stderr, "early-stop check at q=%d (n=%d) cheap=%d\n", q, n, (int)ch);
#endif
            if (ch) {
                vector<int> revQ(queried.rbegin(), queried.rend());
                vector<int> unq;
                for (int x = 1; x <= n; ++x) if (!isQ[x]) unq.push_back(x);
                for (int pass = 0; pass < 2 && !done; ++pass) {
                    Builder B(I);
                    B.scanBudget = 5000;
                    bool rr = B.run(revQ, unq, pass == 0);
#ifdef TEST_MODE
                    if (g_dbg) { auto pp = B.extract(); fprintf(stderr, "  early pass %d: run=%d placed=%d verified=%d\n", pass, (int)rr, (int)pp.size(), (int)(rr && verifyPath(I,pp))); }
#endif
                    if (rr) {
                        vector<int> p = B.extract();
                        if (verifyPath(I, p)) {
                            finalPath = p; done = true;
#ifdef TEST_MODE
                            g_attemptUsed = 1;
                            ++g_earlyBuildOK;
#endif
                        }
                    }
                }
            }
        }
        if (done) break;
    }

    if (!done) {
        vector<int> revOrder(queried.rbegin(), queried.rend());
        vector<int> noUnq;
        for (int pass = 0; pass < 3 && !done; ++pass) {
            Builder B(I);
            B.scanBudget = 0;
            bool rr = B.run(pass == 0 ? revOrder : queried, noUnq, pass != 2);
#ifdef TEST_MODE
            if (g_dbg) { auto pp = B.extract(); fprintf(stderr, "full pass %d: run=%d placed=%d verified=%d\n", pass, (int)rr, (int)pp.size(), (int)(rr && verifyPath(I,pp))); }
#endif
            if (rr) {
                vector<int> p = B.extract();
                if (verifyPath(I, p)) {
                    finalPath = p; done = true;
#ifdef TEST_MODE
                    g_attemptUsed = (pass == 0 ? 11 : (pass == 1 ? 12 : 13));
#endif
                }
            }
        }
    }
    if (!done) {
        vector<int> revOrder(queried.rbegin(), queried.rend());
        mt19937 rng(987654321);
        for (int attempt = 0; attempt < 30 && !done; ++attempt) {
            vector<int> ord = revOrder;
            shuffle(ord.begin(), ord.end(), rng);
            Builder B(I);
            B.scanBudget = 0;
            vector<int> noUnq;
            if (B.run(ord, noUnq, (attempt & 1) == 0)) {
                vector<int> p = B.extract();
                if (verifyPath(I, p)) {
                    finalPath = p; done = true;
#ifdef TEST_MODE
                    g_attemptUsed = 14;
#endif
                }
            }
        }
    }
    if (!done) {
        finalPath.assign(queried.rbegin(), queried.rend());
#ifdef TEST_MODE
        g_attemptUsed = 15;
#endif
    }
    OUTPUT(finalPath);
}

#ifndef TEST_MODE
int main() {
    if (scanf("%d", &g_n_real) != 1) return 0;
    solve();
    return 0;
}
#endif

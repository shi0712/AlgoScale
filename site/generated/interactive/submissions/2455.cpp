#include <bits/stdc++.h>
using namespace std;

static int n, m;
static int SMALLN = 6;
static int SAMPLEK = 12;
static long long qcnt = 0;

#ifdef LOCAL
#include "judge_local.h"
#else
static int ask(int v, const vector<char>& b) {
    string s(m, '0');
    for (int i = 0; i < m; i++) s[i] = b[i] ? '1' : '0';
    printf("? %d %s\n", v, s.c_str());
    fflush(stdout);
    qcnt++;
    int a;
    if (scanf("%d", &a) != 1) exit(0);
    if (a < 0) exit(0);
    return a;
}
#endif

static vector<char> X;  // X[i] = 1 iff f_i is closer to root r
static int r = 1;
static vector<int> dep;

// returns |T ∩ P(v)| where P(v) = edges on path root->v
static int storeV = -1;
static vector<vector<pair<vector<int>, int>>> stq;  // stored (T, raw) per vertex, raw = 2|T∩P| - depth
static const int BADPAR = INT_MIN;
static int askT(int v, const vector<char>& inT) {
    vector<char> b(m);
    int cntT = 0;
    for (int i = 0; i < m; i++) {
        if (inT[i]) { b[i] = !X[i]; cntT++; }
        else b[i] = X[i];
    }
    int a = ask(v, b);
    int raw = a - (m - cntT);
    if (storeV == v) {
        vector<int> T;
        for (int i = 0; i < m; i++) if (inT[i]) T.push_back(i);
        stq[v].push_back({T, raw});
    }
    int val = raw + dep[v];
    if ((val & 1) || val < 0) return BADPAR;
    return val / 2;
}

// ---------------- Phase 1: Lindstrom coin weighing at root ----------------
// ---------------- generic Lindstrom plan ----------------
struct Coin { int S, A, B; };
struct LindPlan { int N; vector<Coin> coins; };
static LindPlan makePlan(int c) {
    LindPlan P;
    int N = 1; long long cap = 0;
    while (cap < c) { cap += __builtin_popcount(N); N++; }
    P.N = N;
    for (int S = 1; S < N && (int)P.coins.size() < c; S++) {
        int pc = __builtin_popcount(S), A = 0, cnt = 0;
        for (int bit = 0; bit < 30 && (int)P.coins.size() < c; bit++) {
            if (S >> bit & 1) {
                A |= 1 << bit; cnt++;
                P.coins.push_back({S, A, S & ~A});
                if (cnt == pc) break;
            }
        }
    }
    return P;
}
static int lindCost(int c) {
    if (c <= 0) return 0;
    int N = 1; long long cap = 0;
    while (cap < c) { cap += __builtin_popcount(N); N++; }
    return N - 1;
}
static inline bool coinIn(const Coin& c, int T) {
    return ((c.B & ~T) == 0) && (__builtin_popcount(T & c.A) & 1);
}
// val[T] = sum of x over coins in query T (val[0]=0)
static vector<int> decodePlan(const LindPlan& P, const vector<long long>& val) {
    int N = P.N, c = P.coins.size();
    vector<int> order;
    for (int S = 1; S < N; S++) order.push_back(S);
    sort(order.begin(), order.end(), [](int a, int b2) {
        int pa = __builtin_popcount(a), pb = __builtin_popcount(b2);
        if (pa != pb) return pa > pb;
        return a < b2;
    });
    vector<vector<int>> coinsOf(N);
    for (int i = 0; i < c; i++) coinsOf[P.coins[i].S].push_back(i);
    vector<int> xs(c, 0);
    vector<int> ones;  // decoded coins with x=1
    for (int S : order) {
        long long g = 0;
        int pS = __builtin_popcount(S);
        for (int T = S;; T = (T - 1) & S) {
            int sgn = ((pS - __builtin_popcount(T)) & 1) ? -1 : 1;
            g += sgn * val[T];
            if (T == 0) break;
        }
        for (int i : ones) {
            const Coin& cc = P.coins[i];
            if (cc.S == S) continue;
            if ((S & ~cc.S) != 0) continue;
            if ((cc.B & ~S) != 0) continue;
            int k = __builtin_popcount(S & cc.A);
            if (k == 0) continue;
            long long mu = (long long)1 << (k - 1);
            if ((k + 1) & 1) mu = -mu;
            g -= mu;
        }
        long long R = g;
        for (int i : coinsOf[S]) {
            int d = (int)(((R % 2) + 2) % 2);
            xs[i] = d;
            if (d) ones.push_back(i);
            R = (R - d) / (-2);
        }
    }
    return xs;
}

// learn P(v) ∩ coins (coins = edge list); if knownCount>=0, last coin inferred
static vector<char> lindLearn(int v, const vector<int>& coinsE, int knownCount) {
    int c = coinsE.size();
    vector<char> res(c, 0);
    if (c == 0) return res;
    int cc = (knownCount >= 0) ? c - 1 : c;
    if (cc > 0) {
        LindPlan P = makePlan(cc);
        vector<long long> val(P.N, 0);
        vector<char> inT(m, 0);
        for (int T = 1; T < P.N; T++) {
            for (int i = 0; i < cc; i++) inT[coinsE[i]] = coinIn(P.coins[i], T);
            val[T] = askT(v, inT);
            for (int i = 0; i < cc; i++) inT[coinsE[i]] = 0;
        }
        vector<int> xs = decodePlan(P, val);
        for (int i = 0; i < cc; i++) res[i] = xs[i];
    }
    if (knownCount >= 0) {
        int s = 0;
        for (int i = 0; i < cc; i++) s += res[i];
        res[c - 1] = knownCount - s;
    }
    return res;
}

// ---------------- Phase 3 ----------------
static vector<int> par, pe, childOf, kids;
static double statA = 0, statC = 0, statQ = 0, statP = 0, statR = 0, statT = 0;
static vector<char> assigned, processedV;
static vector<char> sEdge, relMark;
static vector<vector<int>> below;
static vector<double> wgt, wE;
static bool shallowEnd = false;
static double PENK = 0.0;
static int RESTARTS = 3;
static int SAITER = 0;
static double PROBEDISC = 0.85, PROBEMARGIN = 10;
static vector<vector<int>> lv;
static int maxD;
static mt19937 rng(12345);
static const int NAL = 9;  // 0..7 CRP alphas, 8 = hazard model
static const double alphas[8] = {0.02, 0.05, 0.15, 0.4, 1, 3, 10, 1e9};
static double alphaLoss[NAL], levelLoss[NAL];
static int curLevel = -1;
static double GW = 0.3;
static vector<double> ccHist;  // child-count histogram of completed vertices
static int ccDoneLevel = -1;

static double modelW(int mi, int u) {
    if (mi < 8) { double al = alphas[mi]; return (al > 1e8) ? 1.0 : kids[u] + al; }
    int k = kids[u];
    double S0 = 0, S1 = 0, tot = 0, tot1 = 0;
    for (size_t c = 0; c < ccHist.size(); c++) {
        tot += ccHist[c]; tot1 += ccHist[c] * c;
        if ((int)c >= k) { S0 += ccHist[c]; S1 += ccHist[c] * (c - k); }
    }
    double mu = tot > 0 ? tot1 / tot : 1.0;
    double beta = 0.5;
    return (S1 + beta * mu) / (S0 + beta) + 1e-3;
}
static void updateCC(int d) {
    // vertices at depth <= d-2 have complete child counts
    while (ccDoneLevel < d - 2) {
        ccDoneLevel++;
        if (ccDoneLevel < 0 || ccDoneLevel > maxD) continue;
        for (int x : lv[ccDoneLevel]) {
            int k = kids[x];
            if ((int)ccHist.size() <= k) ccHist.resize(k + 1, 0);
            ccHist[k] += 1;
        }
    }
}
static void setCRP(const vector<int>& A, int level) {
    if (A.size() <= 1) { for (int u : A) wgt[u] = 1.0; return; }
    if (level != curLevel) { curLevel = level; for (int i = 0; i < NAL; i++) levelLoss[i] = 0; }
    int bi = 0;
    auto sc = [&](int i) { return levelLoss[i] + GW * alphaLoss[i]; };
    for (int i = 1; i < NAL; i++) if (sc(i) < sc(bi) - 1e-9) bi = i;
    if (getenv("MODEL")) bi = atoi(getenv("MODEL"));
    for (int u : A) wgt[u] = modelW(bi, u);
}
static void updCRP(const vector<int>& A, int u) {
    if (A.size() <= 1) return;
    for (int i = 0; i < NAL; i++) {
        double num = modelW(i, u), den = 0;
        for (int x : A) den += modelW(i, x);
        alphaLoss[i] += -log2(num / den);
        levelLoss[i] += -log2(num / den);
    }
}

struct Block { vector<int> A, C; double mu; };

// candidate pairs (u,e) in blocks; weight mu*wgt[u]. C={-1} means no edge part.
static pair<int, int> searchBlocks(int v, vector<Block> bl) {
    auto ew = [&](int e) { return e < 0 ? 1.0 : wE[e]; };
    for (auto& b : bl) {
        shuffle(b.C.begin(), b.C.end(), rng);
        stable_sort(b.C.begin(), b.C.end(), [&](int x, int y) { return ew(x) > ew(y); });
    }
    vector<int> entU, entB, entF;
    while (true) {
        long long cnt = 0;
        for (auto& b : bl) cnt += (long long)b.A.size() * b.C.size();
        if (cnt == 1) break;
        if (cnt == 0) return {-1, -1};
        int B = bl.size();
        double tot = 0;
        set<int> distinctU;
        vector<vector<double>> pre(bl.size());
        for (size_t bi = 0; bi < bl.size(); bi++) {
            auto& b = bl[bi];
            double w = 0; for (int u : b.A) { w += wgt[u]; distinctU.insert(u); }
            pre[bi].assign(b.C.size() + 1, 0.0);
            for (size_t k = 0; k < b.C.size(); k++) pre[bi][k + 1] = pre[bi][k] + ew(b.C[k]);
            tot += b.mu * w * pre[bi].back();
        }
        entU.clear(); entB.clear(); entF.clear();
        for (int bi = 0; bi < B; bi++) for (int u : bl[bi].A) { entU.push_back(u); entB.push_back(bi); entF.push_back(0); }
        int NE = entU.size();
        vector<int> rel;
        if (distinctU.size() > 1) {
            for (int i = 0; i < NE; i++) {
                int w = entU[i];
                while (w != r) {
                    int e = pe[w];
                    if (!relMark[e]) { relMark[e] = 1; rel.push_back(e); below[e].clear(); }
                    below[e].push_back(i);
                    w = par[w];
                }
            }
        }
        for (int e : rel) sEdge[e] = 0;
        int FM = dep[v] + 2;
        vector<vector<double>> hist(B, vector<double>(FM, 0));
        vector<vector<int>> cntH(B, vector<int>(FM, 0));
        auto toggle = [&](int e) {
            int delta = sEdge[e] ? -1 : 1;
            for (int i : below[e]) {
                int bi = entB[i];
                double w = bl[bi].mu * wgt[entU[i]];
                hist[bi][entF[i]] -= w; cntH[bi][entF[i]]--; entF[i] += delta; hist[bi][entF[i]] += w; cntH[bi][entF[i]]++;
            }
            sEdge[e] ^= 1;
        };
        for (int i = 0; i < NE; i++) { hist[entB[i]][0] += bl[entB[i]].mu * wgt[entU[i]]; cntH[entB[i]][0]++; }
        if (distinctU.size() > 1) {
            for (auto& b : bl) {
                vector<int> ord = b.A;
                double wsum = 0; for (int u : ord) wsum += wgt[u];
                sort(ord.begin(), ord.end(), [&](int x, int y) { return wgt[x] > wgt[y]; });
                double acc = 0;
                for (int u : ord) {
                    if (acc + wgt[u] / 2 > wsum / 2) continue;
                    acc += wgt[u];
                    if (u == r) continue;
                    if (!sEdge[pe[u]]) toggle(pe[u]);
                }
            }
        }
        vector<long long> c1(B), csz(B);
        vector<char> dummy(B, 0);
        for (int bi = 0; bi < B; bi++) {
            csz[bi] = bl[bi].C.size();
            dummy[bi] = (bl[bi].C[0] == -1);
            c1[bi] = dummy[bi] ? 0 : csz[bi] / 2;
        }
        vector<double> P(FM + 1);
        vector<int> ccnt(FM + 1), lnA(FM + 1), lnC(FM + 1);
        auto entropy = [&]() {
            fill(P.begin(), P.end(), 0.0);
            fill(ccnt.begin(), ccnt.end(), 0);
            for (int bi = 0; bi < B; bi++) {
                double w1 = pre[bi][c1[bi]], w0 = pre[bi][csz[bi]] - w1;
                int n1 = c1[bi], n0 = csz[bi] - c1[bi];
                for (int k = 0; k < FM; k++) {
                    double h = hist[bi][k];
                    if (cntH[bi][k] <= 0) continue;
                    if (n0 > 0) { P[k] += h * w0; ccnt[k]++; lnA[k] = cntH[bi][k]; lnC[k] = n0; }
                    if (n1 > 0) { P[k + 1] += h * w1; ccnt[k + 1]++; lnA[k + 1] = cntH[bi][k]; lnC[k + 1] = n1; }
                }
            }
            double H = 0, pen = 0;
            for (int a2 = 0; a2 <= FM; a2++) {
                double p = P[a2];
                if (p <= 1e-300) continue;
                double q = p / tot; H -= q * log2(q);
                if (PENK > 0 && ccnt[a2] == 1) {
                    if (lnA[a2] == 1 && lnC[a2] > 1) pen += q * PENK * log2((double)lnC[a2]);
                    else if (lnC[a2] == 1 && lnA[a2] > 1) pen += q * PENK * log2((double)lnA[a2]);
                }
            }
            return H - pen;
        };
        auto localSearch = [&]() {
            double best = entropy();
            for (int pass = 0; pass < 4; pass++) {
                bool imp = false;
                for (int bi = 0; bi < B; bi++) {
                    if (dummy[bi]) continue;
                    long long orig = c1[bi];
                    long long lo = 0, hi = csz[bi];
                    while (hi - lo > 2) {
                        long long m1 = lo + (hi - lo) / 3, m2 = hi - (hi - lo) / 3;
                        c1[bi] = m1; double h1 = entropy();
                        c1[bi] = m2; double h2 = entropy();
                        if (h1 < h2) lo = m1 + 1; else hi = m2 - 1;
                    }
                    long long bc = orig; double bh = best;
                    for (long long o = lo; o <= hi; o++) {
                        c1[bi] = o; double h = entropy();
                        if (h > bh + 1e-12) { bh = h; bc = o; }
                    }
                    c1[bi] = bc;
                    if (bh > best + 1e-12) { best = bh; imp = true; }
                }
                for (int e : rel) {
                    toggle(e);
                    double h = entropy();
                    if (h > best + 1e-12) { best = h; imp = true; }
                    else toggle(e);
                }
                if (!imp) break;
            }
            return best;
        };
        double best = localSearch();
        if (SAITER > 0 && rel.size() > 1) {
            vector<char> bestS(rel.size());
            for (size_t i = 0; i < rel.size(); i++) bestS[i] = sEdge[rel[i]];
            vector<long long> bestC1 = c1;
            double curH = best, T0 = 0.05;
            int iters = SAITER;
            for (int it = 0; it < iters; it++) {
                double temp = T0 * (1.0 - (double)it / iters) + 1e-4;
                int k = rng() % rel.size();
                int e1 = rel[k];
                bool pair2 = (rng() & 1) && e1 >= 0;
                int x = childOf[e1];
                int e2 = (pair2 && x != 0 && par[x] != r && relMark[pe[par[x]]]) ? pe[par[x]] : -1;
                toggle(e1); if (e2 >= 0) toggle(e2);
                double h = entropy();
                if (h > curH || exp((h - curH) / temp) > (double)(rng() % 1000000) / 1e6) {
                    curH = h;
                    if (h > best + 1e-9) { best = h; for (size_t i = 0; i < rel.size(); i++) bestS[i] = sEdge[rel[i]]; bestC1 = c1; }
                } else { if (e2 >= 0) toggle(e2); toggle(e1); }
            }
            for (size_t i = 0; i < rel.size(); i++) if (sEdge[rel[i]] != bestS[i]) toggle(rel[i]);
            c1 = bestC1;
            best = localSearch();
        }
#ifdef LOCAL
        static double sumH = 0; static long long nH = 0;
#endif
        if (RESTARTS > 0 && rel.size() > 1) {
            vector<char> bestS(rel.size());
            for (size_t i = 0; i < rel.size(); i++) bestS[i] = sEdge[rel[i]];
            vector<long long> bestC1 = c1;
            for (int rs = 0; rs < RESTARTS; rs++) {
                for (size_t i = 0; i < rel.size(); i++) if ((int)(rng() & 1) != sEdge[rel[i]]) toggle(rel[i]);
                for (int bi = 0; bi < B; bi++) if (!dummy[bi]) c1[bi] = csz[bi] / 2;
                double h = localSearch();
                if (h > best + 1e-9) {
                    best = h;
                    for (size_t i = 0; i < rel.size(); i++) bestS[i] = sEdge[rel[i]];
                    bestC1 = c1;
                }
            }
            for (size_t i = 0; i < rel.size(); i++) if (sEdge[rel[i]] != bestS[i]) toggle(rel[i]);
            c1 = bestC1;
        }
#ifdef LOCAL
        sumH += best; nH++;
        if (getenv("DBGH") && nH % 500 == 0) fprintf(stderr, "avgH %.3f over %lld (last tot=%lld B=%d rel=%zu)\n", sumH / nH, nH, cnt, B, rel.size());
#endif
        vector<char> inT(m, 0);
        for (int e : rel) if (sEdge[e]) inT[e] = 1;
        for (int bi = 0; bi < B; bi++) for (int k = 0; k < c1[bi]; k++) inT[bl[bi].C[k]] = 1;
        if (shallowEnd && cnt == 2) {
            vector<pair<int, int>> prs;
            for (auto& b : bl) for (int u : b.A) for (int e : b.C) prs.push_back({u, e});
            if (prs[0].second != prs[1].second && prs[0].second >= 0) {
                for (int e : rel) { relMark[e] = 0; sEdge[e] = 0; }
                rel.clear();
                fill(inT.begin(), inT.end(), 0);
                inT[prs[0].second] = 1;
                for (int i = 0; i < NE; i++) entF[i] = 0;
                for (int bi = 0; bi < B; bi++) {
                    // reorder C so that the chosen edge is first, c1 = 1 if present
                    auto& C = bl[bi].C;
                    auto it = find(C.begin(), C.end(), prs[0].second);
                    if (it != C.end()) { iter_swap(C.begin(), it); c1[bi] = 1; } else c1[bi] = 0;
                }
            }
        }
        int a = askT(v, inT);
        if (a == BADPAR) { for (int e : rel) { relMark[e] = 0; sEdge[e] = 0; } return {-1, -1}; }
        vector<Block> nb;
        int ei = 0;
        for (int bi = 0; bi < B; bi++) {
            Block b0, b1;
            b0.mu = b1.mu = bl[bi].mu;
            for (size_t k = 0; k < bl[bi].A.size(); k++, ei++) {
                int u = bl[bi].A[k];
                int g = a - entF[ei];
                if (g == 0) b0.A.push_back(u);
                else if (g == 1) b1.A.push_back(u);
            }
            if (!b0.A.empty()) for (int k = c1[bi]; k < csz[bi]; k++) b0.C.push_back(bl[bi].C[k]);
            if (!b1.A.empty()) for (int k = 0; k < c1[bi]; k++) b1.C.push_back(bl[bi].C[k]);
            if (!b0.A.empty() && !b0.C.empty()) nb.push_back(move(b0));
            if (!b1.A.empty() && !b1.C.empty()) nb.push_back(move(b1));
        }
        for (int e : rel) { relMark[e] = 0; sEdge[e] = 0; }
        bl = move(nb);
        if (bl.empty()) return {-1, -1};
    }
    return {bl[0].A[0], bl[0].C[0]};
}

// filter candidate blocks with a known answer a = |T ∩ P(v)|
static void applyFilter(vector<Block>& bl, const vector<int>& Tl, int a) {
    vector<char> inT(m, 0);
    for (int e : Tl) inT[e] = 1;
    vector<Block> nb;
    for (auto& b : bl) {
        Block b0, b1;
        b0.mu = b1.mu = b.mu;
        for (int u : b.A) {
            int f = 0, w = u;
            while (w != r) { f += inT[pe[w]]; w = par[w]; }
            int g = a - f;
            if (g == 0) b0.A.push_back(u);
            else if (g == 1) b1.A.push_back(u);
        }
        for (int e : b.C) {
            if (e >= 0 && inT[e]) { if (!b1.A.empty()) b1.C.push_back(e); }
            else { if (!b0.A.empty()) b0.C.push_back(e); }
        }
        if (!b0.A.empty() && !b0.C.empty()) nb.push_back(move(b0));
        if (!b1.A.empty() && !b1.C.empty()) nb.push_back(move(b1));
    }
    bl = move(nb);
}

// ---------------- chunks ----------------
struct Chunk { vector<int> E; int par; int attachV; int top; int botV; vector<int> kids; bool alive; };
static vector<Chunk> chs;
static vector<int> chunkOf, botChunk;
static int botLevel(int c) { return chs[c].top + (int)chs[c].E.size() - 1; }

static int newChunk(const vector<int>& E, int parC, int attachV, int top, int botV) {
    int id = chs.size();
    chs.push_back({E, parC, attachV, top, botV, {}, true});
    for (int e : E) chunkOf[e] = id;
    if (botV) botChunk[botV] = id;
    if (parC >= 0) chs[parC].kids.push_back(id);
    return id;
}
// split chunk c: top part = edges with inJ; returns bottom chunk id
static int splitChunk(int c, const vector<char>& onP) {
    vector<int> J, Rr;
    for (int e : chs[c].E) (onP[e] ? J : Rr).push_back(e);
    int id = chs.size();
    Chunk B;
    B.E = Rr; B.par = c; B.attachV = 0; B.top = chs[c].top + (int)J.size();
    B.botV = chs[c].botV; B.kids = chs[c].kids; B.alive = true;
    chs.push_back(B);
    for (int k : chs[id].kids) chs[k].par = id;
    chs[c].E = J; chs[c].kids = {id}; chs[c].botV = 0;
    for (int e : Rr) chunkOf[e] = id;
    if (chs[id].botV) botChunk[chs[id].botV] = id;
    return id;
}

static void assignV(int v, int u, int e) {
    par[v] = u; pe[v] = e; childOf[e] = v; assigned[e] = 1; kids[u]++; processedV[v] = 1;
    int c = chunkOf[e];
    if (c >= 0) {
        auto& E = chs[c].E;
        E.erase(find(E.begin(), E.end(), e));
        chunkOf[e] = -1;
        if (E.empty()) {
            for (int k : chs[c].kids) { chs[k].par = -1; chs[k].attachV = v; }
            chs[c].alive = false;
            if (chs[c].botV) botChunk[chs[c].botV] = -1;
        } else {
            chs[c].attachV = v; chs[c].top = dep[v] + 1;
        }
    }
}

static int BFMAX = 7;
static double bfCostTab[9] = {0, 0, 1, 2.63, 3.92, 5.17, 7.65, 9.55, 12.0};
static double tdCost(int s) { double c = 0; for (int k = 2; k <= s; k++) c += log2((double)k); return c; }
static double idcCost(int c) { if (c <= 1) return 0; return (double)(c - 1) * (c + 2) / (2.0 * c); }
static map<pair<int, int>, double> ccMemo;
static double chunkCost(int s, int idq) {  // idq: identification cost (x10) per split
    if (s <= 1) return 0;
    auto key = make_pair(s, idq);
    auto it = ccMemo.find(key);
    if (it != ccMemo.end()) return it->second;
    double best = tdCost(s);
    if (idq == 0 && s <= BFMAX) best = min(best, bfCostTab[s]);
    int a = s / 2;
    double alt = idq / 10.0 + lindCost(s - 1) + chunkCost(a, idq) + chunkCost(s - a, idq);
    if (alt < best) best = alt;
    return ccMemo[key] = best;
}

static vector<int> levelCands(int k);
// brute force the order of a small chunk attached at a processed vertex
static bool bruteChunk(int c) {
    int s = chs[c].E.size();
    if (s < 2 || s > BFMAX || chs[c].par != -1) return false;
    int t = chs[c].top;
    vector<int> vs(s);
    for (int k = 0; k < s; k++) {
        int lvl = t + k;
        if (k == s - 1 && chs[c].botV) { vs[k] = chs[c].botV; continue; }
        vector<int> cd = levelCands(lvl);
        if (cd.size() != 1) return false;
        vs[k] = cd[0];
    }
    vector<int> E = chs[c].E;
#ifdef LOCAL
    long long q0 = qcnt;
    struct G { long long q0; int s; ~G(){ if (getenv("DBGBF")) fprintf(stderr, "bfq s=%d q=%lld\n", s, qcnt - q0);} } g{q0, s};
#endif
    vector<int> perm(s);
    iota(perm.begin(), perm.end(), 0);
    vector<vector<uint8_t>> pm;  // pm[h][j] = mask of edges (indices into E) at positions 0..j
    vector<vector<int>> perms;
    do {
        vector<uint8_t> row(s);
        uint8_t msk = 0;
        for (int j = 0; j < s; j++) { msk |= 1 << perm[j]; row[j] = msk; }
        pm.push_back(row); perms.push_back(perm);
    } while (next_permutation(perm.begin(), perm.end()));
    vector<int> cand(pm.size());
    iota(cand.begin(), cand.end(), 0);
    int full = 1 << s;
    while (cand.size() > 1) {
        double bestH = -1; int bj = 0, bT = 1;
        vector<int> hist(s + 1);
        for (int j = 0; j < s - 1; j++) for (int T = 1; T < full; T++) {
            fill(hist.begin(), hist.end(), 0);
            for (int h : cand) hist[__builtin_popcount(T & pm[h][j])]++;
            double H = 0;
            for (int x : hist) if (x) { double q = (double)x / cand.size(); H -= q * log2(q); }
            if (H > bestH + 1e-12) { bestH = H; bj = j; bT = T; }
        }
        vector<char> inT(m, 0);
        for (int i = 0; i < s; i++) if (bT >> i & 1) inT[E[i]] = 1;
        int a = askT(vs[bj], inT);
        vector<int> nc;
        for (int h : cand) if (__builtin_popcount(bT & pm[h][bj]) == a) nc.push_back(h);
        cand.swap(nc);
        if (cand.empty()) exit(0);
    }
#ifdef LOCAL
    if (getenv("DBGBF")) fprintf(stderr, "bf s=%d\n", s);
#endif
    vector<int> pr = perms[cand[0]];
    // split into single-edge chunks with known bottoms (assigned lazily level by level)
    int cur = c;
    vector<char> onP(m, 0);
    for (int k = 0; k < s - 1; k++) {
        onP[E[pr[k]]] = 1;
        int b = splitChunk(cur, onP);
        onP[E[pr[k]]] = 0;
        chs[cur].botV = vs[k]; botChunk[vs[k]] = cur;
        cur = b;
    }
    chs[cur].botV = vs[s - 1]; botChunk[vs[s - 1]] = cur;
    return true;
}

static vector<int> levelCands(int k) {
    vector<int> res;
    if (k < 0 || k > maxD) return res;
    for (int x : lv[k]) if (!processedV[x] && botChunk[x] == -1) res.push_back(x);
    return res;
}

static void refineChunk(int c) {
#ifdef LOCAL
    if (getenv("DBG")) fprintf(stderr, "refine chunk size %zu top %d\n", chs[c].E.size(), chs[c].top);
#endif
    int s = chs[c].E.size();
    if (s < 3) return;
    int t = chs[c].top;
    double td = tdCost(s);
    if (s <= BFMAX) {
        bool uniq = true;
        for (int k = t; k <= t + s - 2 && uniq; k++) if (levelCands(k).size() != 1) uniq = false;
        if (uniq) td = min(td, bfCostTab[s]);
    }
    int bestA = -1; double bestCost = 1e18;
    int lo = max(1, s / 2 - s / 6), hi = min(s - 1, s / 2 + (s + 5) / 6);
    for (int a = lo; a <= hi; a++) {
        int k = t + a - 1;
        int cc = levelCands(k).size();
        if (cc == 0) continue;
        double cost = idcCost(cc) + lindCost(s - 1) + chunkCost(a, 0) + chunkCost(s - a, 0);
        if (cost < bestCost) { bestCost = cost; bestA = a; }
    }
    if (bestA < 0 || bestCost >= td) return;
    int a = bestA, k = t + a - 1;
    vector<int> cand = levelCands(k);
    shuffle(cand.begin(), cand.end(), rng);
    int x = -1;
    vector<char> inT(m, 0);
    for (int e : chs[c].E) inT[e] = 1;
    for (size_t i = 0; i < cand.size(); i++) {
        if (i + 1 == cand.size()) { x = cand[i]; break; }
        int j = askT(cand[i], inT);
        if (j == a) { x = cand[i]; break; }
    }
    vector<int> E = chs[c].E;
    shuffle(E.begin(), E.end(), rng);
#ifdef LOCAL
    long long q0 = qcnt;
#endif
    vector<char> in = lindLearn(x, E, a);
#ifdef LOCAL
    statR += qcnt - q0;
    if (getenv("DBG")) fprintf(stderr, "  split s=%d a=%d cands=%zu lind=%lld\n", s, a, cand.size(), qcnt-q0);
#endif
    vector<char> onP(m, 0);
    for (size_t i = 0; i < E.size(); i++) if (in[i]) onP[E[i]] = 1;
    int b = splitChunk(c, onP);
    chs[c].botV = x; botChunk[x] = c;
    refineChunk(c);
    refineChunk(b);
}

static void probe(int w, int d, bool noChunkPart) {
    vector<int> Uv;
    for (int i = 0; i < m; i++) if (!assigned[i] && (!noChunkPart || chunkOf[i] == -1)) Uv.push_back(i);
    int Lp = dep[w] - d + 1;
    vector<char> onP(m, 0);
    if (Lp == (int)Uv.size()) { for (int e : Uv) onP[e] = 1; }
    else {
        shuffle(Uv.begin(), Uv.end(), rng);
        vector<char> in = lindLearn(w, Uv, Lp);
        for (size_t i = 0; i < Uv.size(); i++) if (in[i]) onP[Uv[i]] = 1;
    }
    int cur = -1;
    int nch = chs.size();
    for (int c = 0; c < nch; c++) {
        if (!chs[c].alive || chs[c].par != -1) continue;
        for (int e : chs[c].E) if (onP[e]) { cur = c; break; }
        if (cur >= 0) break;
    }
    int deepest = -1;
    while (cur != -1) {
        int J = 0;
        for (int e : chs[cur].E) J += onP[e];
        if (J == (int)chs[cur].E.size()) {
            deepest = cur;
            int nxt = -1;
            for (int k : chs[cur].kids) {
                for (int e : chs[k].E) if (onP[e]) { nxt = k; break; }
                if (nxt >= 0) break;
            }
            cur = nxt;
        } else {
            splitChunk(cur, onP);
            deepest = cur;
            break;
        }
    }
    vector<int> N;
    for (int e : Uv) if (onP[e] && chunkOf[e] == -1) N.push_back(e);
#ifdef LOCAL
    if (getenv("DBG")) { int cntP = 0; for (int e : Uv) cntP += onP[e]; fprintf(stderr, "  probe result: onP=%d N=%zu deepest=%d Lp=%d Uv=%zu\n", cntP, N.size(), deepest, Lp, Uv.size()); }
#endif
    if (!N.empty()) {
        if (deepest == -1) {
            vector<int> A = lv[d - 1];
            for (int u : A) wgt[u] = 1.0;
            int u = A[0];
            if (A.size() > 1) u = searchBlocks(w, {Block{A, {-1}, 1.0}}).first;
            newChunk(N, -1, u, d, w);
        } else {
            newChunk(N, deepest, 0, botLevel(deepest) + 1, w);
        }
    } else if (deepest >= 0) {
        chs[deepest].botV = w; botChunk[w] = deepest;
    }
}

static double H2(double p) { if (p <= 0 || p >= 1) return 0; return -p * log2(p) - (1 - p) * log2(1 - p); }

static int probeFail = 0, probeSucc = 0;
static vector<char> skipG;
static void probes(int d) {
    vector<char> skip(n + 1, 0);
    int testsHere = 0;
    for (int it = 0; it < 6; it++) {
        vector<int> cover(maxD + 2, 0);
        for (auto& c : chs) if (c.alive) for (int k = c.top; k <= c.top + (int)c.E.size() - 1 && k <= maxD; k++) cover[k]++;
        int w = -1;
        {
            int kmax = d - 1;
            while (kmax + 1 <= maxD && (int)lv[kmax + 1].size() > cover[kmax + 1]) kmax++;
            for (int k = kmax; k >= d && w == -1; k--) {
                vector<int> cs;
                for (int x : lv[k]) if (!processedV[x] && botChunk[x] == -1 && !skip[x] && !skipG[x]) cs.push_back(x);
                if (!cs.empty()) w = cs[rng() % cs.size()];
            }
        }
        if (w == -1) return;
        int Lp = dep[w] - d + 1;
        if (Lp < 3) return;
        int Usz = 0, F = 0;
        for (int i = 0; i < m; i++) if (!assigned[i]) { Usz++; if (chunkOf[i] == -1) F++; }
        if (F == 0) return;
        bool anyChunk = false;
        for (auto& c : chs) if (c.alive) { anyChunk = true; break; }
        // coverage per level
        auto estimate = [&](int fresh, bool fromRoot, bool noChunk) {
            double ben = 0;
            double idsum = 0; int cntLv = 0;
            int k0 = dep[w] - fresh + 1;
            for (int k = k0; k <= dep[w]; k++) {
                int R = (int)lv[k].size() - cover[k];
                if (R < 1) R = 1;
                int Rp = max(1, (int)lv[k - 1].size());
                double lF = log2((double)max(2, F - (k - k0)));
                double lS = log2((double)max(1, dep[w] - k + 1));
                double cTD = max(lF, (log2((double)Rp) + lF) / 1.58);
                double cCh = max(lS, (log2((double)R) + lS) / 1.58);
                ben += cTD - cCh;
                idsum += idcCost(R); cntLv++;
            }
            ben *= PROBEDISC;
            int idq = cntLv ? (int)lround(10 * idsum / cntLv) : 0;
            ben += max(0.0, tdCost(fresh) - chunkCost(fresh, idq));
            double cost;
            if (noChunk) cost = (fresh == F ? 0 : lindCost(F - 1));
            else cost = (Lp == Usz ? 0 : lindCost(Usz - 1));
            cost += (fromRoot ? log2((double)lv[d - 1].size()) / 1.5 : 0);
            return ben - cost - PROBEMARGIN;
        };
        int fresh = min(Lp, F);
        if (estimate(fresh, !anyChunk, true) <= 0) return;
        bool noChunk = true;
        if (anyChunk) {
            if (testsHere >= 1 || probeFail >= 3 + 3 * probeSucc) return;
            testsHere++;
            vector<char> inT(m, 0);
            for (int i = 0; i < m; i++) if (!assigned[i] && chunkOf[i] != -1) inT[i] = 1;
            int j = askT(w, inT);
            fresh = Lp - j;
#ifdef LOCAL
            statT++;
#endif
            noChunk = (j == 0);
            if (fresh <= 0 || estimate(fresh, j == 0, noChunk) <= 0) { skip[w] = 1; skipG[w] = 1; probeFail++; continue; }
            probeSucc++;
        }
#ifdef LOCAL
        long long q0 = qcnt;
        if (getenv("DBG")) fprintf(stderr, "probe w depth %d level %d Lp %d fresh %d F %d U %d\n", dep[w], d, Lp, fresh, F, Usz);
#endif
        probe(w, d, noChunk);
#ifdef LOCAL
        statP += qcnt - q0;
#endif
        int nch = chs.size();
        for (int c = 0; c < nch; c++) if (chs[c].alive) refineChunk(c);
    }
}

// ---------------- edge-label prior (adaptive mixture) ----------------
static const int NEM = 4;
static double emLoss[NEM];
static vector<double> cntOff;           // offset e - v + n
static vector<vector<double>> cntLB;    // level x bucket
static int bucketSz = 8;
static vector<vector<double>> emProb;   // per model, prob over F (index aligned with F)
static vector<pair<int,int>> pastVE;
static void initEdgeModel() {
    for (int k = 0; k < NEM; k++) emLoss[k] = 0;
    emLoss[0] = -2.0;  // slight preference to uniform initially
    cntOff.assign(2 * n + 2, 0);
    pastVE.clear();
    bucketSz = max(4, m / 32);
    cntLB.assign(maxD + 2, vector<double>(m / bucketSz + 2, 0));
}
static void computeEdgeModel(int v, int d, const vector<int>& F) {
    int K = F.size();
    emProb.assign(NEM, vector<double>(K, 0));
    for (int i = 0; i < K; i++) emProb[0][i] = 1.0 / K;
    double s1 = 0, s2 = 0;
    for (int i = 0; i < K; i++) {
        int e = F[i];
        double p1 = cntOff[e - v + n] + 0.02;
        int b = e / bucketSz;
        double p2 = cntLB[d][b] + 0.5 * ((d >= 1 ? cntLB[d - 1][b] : 0) + cntLB[d + 1][b]) + 0.1;
        emProb[1][i] = p1; emProb[2][i] = p2; s1 += p1; s2 += p2;
    }
    for (int i = 0; i < K; i++) { emProb[1][i] /= s1; emProb[2][i] /= s2; }
    {
        double hv = max(2.0, n / 16.0), he = max(2.0, m / 16.0);
        vector<double> kv;  // weights of past points by vertex distance
        vector<pair<int, double>> pts;
        for (auto [pv, pe2] : pastVE) {
            double w = 1.0 - fabs((double)(pv - v)) / hv;
            if (w > 0) pts.push_back({pe2, w});
        }
        double s3 = 0;
        for (int i = 0; i < K; i++) {
            int e = F[i];
            double p = 0.05;
            for (auto [pe2, w] : pts) {
                double w2 = 1.0 - fabs((double)(pe2 - e)) / he;
                if (w2 > 0) p += w * w2;
            }
            emProb[3][i] = p; s3 += p;
        }
        for (int i = 0; i < K; i++) emProb[3][i] /= s3;
    }
    double mn = 1e18;
    for (int k = 0; k < NEM; k++) mn = min(mn, emLoss[k]);
    double pi[NEM], ps = 0;
    for (int k = 0; k < NEM; k++) { pi[k] = exp2(-(emLoss[k] - mn)); ps += pi[k]; }
    for (int k = 0; k < NEM; k++) pi[k] /= ps;
    if (getenv("NOEM")) { pi[0] = 1; for (int k = 1; k < NEM; k++) pi[k] = 0; }
    for (int i = 0; i < K; i++) {
        double p = 0;
        for (int k = 0; k < NEM; k++) p += pi[k] * emProb[k][i];
        wE[F[i]] = p * K;
    }
}
static void updateEdgeModel(int v, int d, const vector<int>& F, int e) {
    int idx = -1;
    for (size_t i = 0; i < F.size(); i++) if (F[i] == e) { idx = i; break; }
    if (idx < 0) return;
    for (int k = 0; k < NEM; k++) emLoss[k] += -log2(max(emProb[k][idx], 1e-300));
    cntOff[e - v + n] += 1;
    cntLB[d][e / bucketSz] += 1;
    pastVE.push_back({v, e});
}

static void solveStructure() {
    maxD = 0;
    for (int v = 1; v <= n; v++) maxD = max(maxD, dep[v]);
    lv.assign(maxD + 1, {});
    for (int v = 1; v <= n; v++) lv[dep[v]].push_back(v);
    par.assign(n + 1, 0); pe.assign(n + 1, -1); childOf.assign(m, 0); kids.assign(n + 1, 0);
    assigned.assign(m, 0); processedV.assign(n + 1, 0); processedV[r] = 1;
    sEdge.assign(m, 0); relMark.assign(m, 0); below.assign(m, {}); wgt.assign(n + 1, 1.0); wE.assign(m, 1.0);
    initEdgeModel();
    chs.clear(); chunkOf.assign(m, -1); botChunk.assign(n + 1, -1); ccMemo.clear();
    for (int i = 0; i < NAL; i++) alphaLoss[i] = 0;
    alphaLoss[7] = -1e-6;
    ccHist.clear(); ccDoneLevel = -1;
    curLevel = -1; probeFail = probeSucc = 0; skipG.assign(n + 1, 0);
    if (getenv("GW")) GW = atof(getenv("GW"));
    if (getenv("PENK")) PENK = atof(getenv("PENK"));
    if (getenv("RESTARTS")) RESTARTS = atoi(getenv("RESTARTS"));
    if (getenv("SAITER")) SAITER = atoi(getenv("SAITER"));
    if (getenv("PDISC")) PROBEDISC = atof(getenv("PDISC"));
    if (getenv("PMARGIN")) PROBEMARGIN = atof(getenv("PMARGIN"));
    bool useProbe = !getenv("NOPROBE");
    bool useBF = !getenv("NOBF");
    for (int d = 1; d <= maxD; d++) {
        if (useProbe) probes(d);
        updateCC(d);
        if (useBF) {
            for (int c = 0; c < (int)chs.size(); c++)
                if (chs[c].alive && chs[c].par == -1 && chs[c].top == d) bruteChunk(c);
        }
        for (int v : lv[d]) {
            if (botChunk[v] == -1) continue;
            int c = botChunk[v];
            if (chs[c].par != -1 || chs[c].E.size() != 1 || chs[c].top != d) continue;
            assignV(v, chs[c].attachV, chs[c].E[0]);
        }
        for (int v : lv[d]) {
            if (processedV[v]) continue;
            int R = 0;
            for (int x : lv[d]) if (!processedV[x]) R++;
            vector<Block> bl;
            vector<int> elig;
            for (int c = 0; c < (int)chs.size(); c++)
                if (chs[c].alive && chs[c].par == -1 && chs[c].top == d) elig.push_back(c);
            vector<int> A = lv[d - 1];
            vector<int> F;
            for (int i = 0; i < m; i++) if (!assigned[i] && chunkOf[i] == -1) F.push_back(i);
            setCRP(A, d);
            for (int c : elig) for (int e : chs[c].E) wE[e] = 1.0;
            if (!F.empty()) computeEdgeModel(v, d, F);
            double rest = 1.0 - (double)elig.size() / R;
            if (!F.empty() && rest > 1e-9) {
                double WA = 0; for (int u : A) WA += wgt[u];
                bl.push_back(Block{A, F, rest / (WA * F.size())});
            }
            for (int c : elig) {
                int u = chs[c].attachV;
                bl.push_back(Block{{u}, chs[c].E, (1.0 / R) / (chs[c].E.size() * wgt[u])});
            }
#ifdef LOCAL
            long long q0 = qcnt;
#endif
            auto [u, e] = searchBlocks(v, bl);
#ifdef LOCAL
            statQ += qcnt - q0;
            if (e < 0) { fprintf(stderr, "search failed v=%d d=%d R=%d elig=%zu F=%zu A=%zu\n", v, d, R, elig.size(), F.size(), A.size()); }
#endif
            if (e < 0) exit(0);
            if (chunkOf[e] == -1) { updCRP(A, u); updateEdgeModel(v, d, F, e); }
            assignV(v, u, e);
        }
    }
}

// ---------------- shallow mode (no depth queries) ----------------
static vector<int> procOrder;
static bool certified(int v, int d, int u, int e) {
    (void)u; (void)e;
    int Ucnt = 0;
    for (int i = 0; i < m; i++) if (!assigned[i]) Ucnt++;
    vector<int> f(n + 1, 0);
    vector<char> inT(m, 0);
    for (auto& [T, raw] : stq[v]) {
        int val = raw + d;
        if ((val & 1) || val < 0) continue;
        int k = val / 2;
        for (int x : T) inT[x] = 1;
        int tU = 0;
        for (int x : T) if (!assigned[x]) tU++;
        int nU = Ucnt - tU;
        set<pair<int, int>> prs;
        for (int a : procOrder) {
            f[a] = (a == r) ? 0 : f[par[a]] + inT[pe[a]];
            prs.insert({dep[a], f[a]});
        }
        for (int x : T) inT[x] = 0;
        bool excluded = true;
        for (int j = 1; d + 2 * j <= m && excluded; j++) {
            int dp = d + 2 * j;
            for (auto [da, fa] : prs) {
                if (da > dp - 1) continue;
                int need = dp - da;
                if (need > Ucnt) continue;
                int t = k + j - fa;
                int lo = max(0, need - nU), hi = min(need, tU);
                if (t >= lo && t <= hi) { excluded = false; break; }
            }
        }
        if (excluded) return true;
    }
    return false;
}

static void initPhase3() {
    par.assign(n + 1, 0); pe.assign(n + 1, -1); childOf.assign(m, 0); kids.assign(n + 1, 0);
    assigned.assign(m, 0); processedV.assign(n + 1, 0); processedV[r] = 1;
    sEdge.assign(m, 0); relMark.assign(m, 0); below.assign(m, {}); wgt.assign(n + 1, 1.0); wE.assign(m, 1.0);
    chs.clear(); chunkOf.assign(m, -1); botChunk.assign(n + 1, -1); ccMemo.clear();
    for (int i = 0; i < NAL; i++) alphaLoss[i] = 0;
    alphaLoss[7] = -1e-6;
    ccHist.clear(); ccDoneLevel = -1;
    curLevel = -1; probeFail = probeSucc = 0; skipG.assign(n + 1, 0);
    procOrder.assign(1, r);
}

static int depthQuery(int v) {
    vector<char> b(m);
    for (int i = 0; i < m; i++) b[i] = X[i];
    int a = ask(v, b);
    return m - a;
}

static void solveShallow() {
    maxD = n;  // for edge model arrays
    initPhase3();
    initEdgeModel();
    vector<vector<int>> lvP(1, vector<int>{r});
    vector<int> parity(n + 1, -1);
    for (int v = 1; v <= n; v++) if (dep[v] >= 0) parity[v] = dep[v] & 1;
    int failures = 0;
    bool fallback = false;
    int remaining = n - 1;
    {
        mt19937 r0(4242);
        vector<int> idx(m);
        iota(idx.begin(), idx.end(), 0);
        for (int v = 1; v <= n; v++) {
            if (v == r || dep[v] != -1) continue;
            shuffle(idx.begin(), idx.end(), r0);
            vector<char> inT(m, 0);
            for (int i = 0; i < m / 2; i++) inT[idx[i]] = 1;
            dep[v] = 1;
            storeV = v;
            askT(v, inT);
            storeV = -1;
            dep[v] = -1;
            parity[v] = (stq[v][0].second & 1) ? 1 : 0;
        }
    }
    for (int d = 1; remaining > 0; d++) {
        if ((int)lvP.size() <= d) lvP.resize(d + 1);
        if (lvP[d - 1].empty()) exit(0);
        vector<int> cands;
        for (int v = 1; v <= n; v++)
            if (!processedV[v] && (dep[v] == d || (dep[v] == -1 && (parity[v] == -1 || parity[v] == (d & 1))))) cands.push_back(v);
        // known-depth first
        stable_sort(cands.begin(), cands.end(), [&](int a, int b2) { return (dep[a] == d) > (dep[b2] == d); });
        for (int v : cands) {
            if (processedV[v]) continue;
            bool known = (dep[v] == d);
            if (!known && dep[v] != -1) continue;
            if (!known && parity[v] != -1 && parity[v] != (d & 1)) continue;
            if (!known) dep[v] = d;
            bool needCert = false;
            for (int x = 1; x <= n && !needCert; x++) if (!processedV[x] && parity[x] == ((d + 1) & 1)) needCert = true;
            vector<int> A = lvP[d - 1];
            vector<int> F;
            for (int i = 0; i < m; i++) if (!assigned[i]) F.push_back(i);
            setCRP(A, d);
            computeEdgeModel(v, d, F);
            double WA = 0; for (int u : A) WA += wgt[u];
            vector<Block> bl{Block{A, F, 1.0 / (WA * F.size())}};
            bool ok = true;
            for (auto& [T, raw] : stq[v]) {
                int val = raw + d;
                if ((val & 1) || val < 0) { ok = false; break; }
                applyFilter(bl, T, val / 2);
                if (bl.empty()) { ok = false; break; }
            }
            pair<int, int> res = {-1, -1};
            if (ok) {
                storeV = v; shallowEnd = !known && needCert;
                res = searchBlocks(v, bl);
                shallowEnd = false; storeV = -1;
                if (res.first < 0) ok = false;
            }
            if (ok && !known && needCert && !certified(v, d, res.first, res.second)) {
                vector<char> inT(m, 0);
                inT[res.second] = 1;
                storeV = v;
                int a = askT(v, inT);
                storeV = -1;
                if (a != 1) ok = false;
            }
            if (!stq[v].empty()) parity[v] = (stq[v][0].second & 1) ? 1 : 0;
            if (ok) {
                updCRP(A, res.first);
                updateEdgeModel(v, d, F, res.second);
                assignV(v, res.first, res.second);
                lvP[d].push_back(v);
                procOrder.push_back(v);
                remaining--;
            } else {
                if (known) exit(0);
                bool parityFail = (parity[v] != (d & 1));
                dep[v] = -1;
                if (!parityFail) failures++;
                if (failures > 6 && !fallback) {
                    fallback = true;
                    for (int x = 1; x <= n; x++) if (!processedV[x] && dep[x] == -1) { dep[x] = depthQuery(x); parity[x] = dep[x] & 1; }
                }
            }
        }
    }
}

// ---------------- brute force for tiny n ----------------
static void smallSolve() {
    int mm = n - 1;
    // enumerate trees via Prufer
    vector<vector<pair<int, int>>> trees;
    if (n == 2) trees.push_back({{1, 2}});
    else {
        int L = n - 2;
        vector<int> pr(L, 1);
        while (true) {
            vector<int> deg(n + 1, 1);
            for (int x : pr) deg[x]++;
            vector<pair<int, int>> E;
            vector<int> p2 = pr;
            vector<int> dg = deg;
            for (int i = 0; i < L; i++) {
                int leaf = 1;
                while (dg[leaf] != 1) leaf++;
                E.push_back({leaf, p2[i]});
                dg[leaf]--; dg[p2[i]]--;
            }
            int a = -1, b = -1;
            for (int x = 1; x <= n; x++) if (dg[x] == 1) { if (a < 0) a = x; else b = x; }
            E.push_back({a, b});
            trees.push_back(E);
            int k = L - 1;
            while (k >= 0 && pr[k] == n) { pr[k] = 1; k--; }
            if (k < 0) break;
            pr[k]++;
        }
    }
    vector<int> perm(mm);
    iota(perm.begin(), perm.end(), 0);
    vector<vector<int>> perms;
    do perms.push_back(perm); while (next_permutation(perm.begin(), perm.end()));
    int NO = 1 << mm;
    // hypothesis id = (t * perms + p) * NO + o
    long long H = (long long)trees.size() * perms.size() * NO;
    vector<uint8_t> xm((size_t)H * n);
    vector<vector<int>> distT(trees.size());
    for (size_t t = 0; t < trees.size(); t++) {
        vector<vector<int>> D(n + 1, vector<int>(n + 1, 99));
        for (int i = 1; i <= n; i++) D[i][i] = 0;
        for (auto [a, b] : trees[t]) D[a][b] = D[b][a] = 1;
        for (int k = 1; k <= n; k++) for (int i = 1; i <= n; i++) for (int j = 1; j <= n; j++) D[i][j] = min(D[i][j], D[i][k] + D[k][j]);
        for (size_t p = 0; p < perms.size(); p++) {
            // edge i of hypothesis = trees[t][perms[p][i]] with orientation bit
            for (int o = 0; o < NO; o++) {
                size_t id = ((size_t)t * perms.size() + p) * NO + o;
                for (int v = 1; v <= n; v++) {
                    uint8_t msk = 0;
                    for (int i = 0; i < mm; i++) {
                        auto [a, b] = trees[t][perms[p][i]];
                        int sv = a, fv = b;
                        if (o >> i & 1) swap(sv, fv);
                        if (D[v][fv] < D[v][sv]) msk |= 1 << i;
                    }
                    xm[id * n + (v - 1)] = msk;
                }
            }
        }
    }
    vector<uint32_t> cand(H);
    iota(cand.begin(), cand.end(), 0);
    bool first = true;
    while (cand.size() > 1) {
        int bv = 1, bb = NO - 1;
        if (!first) {
            double bestH = -1;
            vector<long long> cntA(mm + 1);
            for (int v = 1; v <= n; v++) for (int b = 0; b < NO; b++) {
                fill(cntA.begin(), cntA.end(), 0);
                for (uint32_t id : cand) cntA[mm - __builtin_popcount((unsigned)(b ^ xm[(size_t)id * n + v - 1]))]++;
                double Hh = 0;
                for (long long c : cntA) if (c) { double q = (double)c / cand.size(); Hh -= q * log2(q); }
                if (Hh > bestH + 1e-12) { bestH = Hh; bv = v; bb = b; }
            }
        }
        first = false;
        vector<char> bq(mm);
        for (int i = 0; i < mm; i++) bq[i] = bb >> i & 1;
        int a = ask(bv, bq);
        vector<uint32_t> nc;
        for (uint32_t id : cand) if (mm - __builtin_popcount((unsigned)(bb ^ xm[(size_t)id * n + bv - 1])) == a) nc.push_back(id);
        cand.swap(nc);
        if (cand.empty()) exit(0);
    }
    size_t id = cand[0];
    int o = id % NO; size_t rest = id / NO;
    int p = rest % perms.size(); int t = rest / perms.size();
    string out = "!";
    for (int i = 0; i < mm; i++) {
        auto [a, b] = trees[t][perms[p][i]];
        int sv = a, fv = b;
        if (o >> i & 1) swap(sv, fv);
        out += " " + to_string(sv) + " " + to_string(fv);
    }
#ifdef LOCAL
    localSubmit(out);
#else
    printf("%s\n", out.c_str());
    fflush(stdout);
#endif
}

int main() {
#ifdef LOCAL
    localInit();
    while (localNext()) {
#endif
    qcnt = 0;
#ifdef LOCAL
    n = localN();
#else
    if (scanf("%d", &n) != 1) return 0;
#endif
    m = n - 1;
    if (n <= SMALLN) { smallSolve();
#ifdef LOCAL
        continue;
#else
        return 0;
#endif
    }
    dep.assign(n + 1, 0);
    {
        vector<char> b0(m, 1);
        int Xtot = ask(r, b0);
        X.assign(m, 0);
        int minority = min(Xtot, m - Xtot);
        bool flip = (m - Xtot) < Xtot;  // search for zeros instead
        double sparseCost = minority == 0 ? 0 : minority * (log2((double)m / minority) + 1.2);
        if (minority == 0 || sparseCost < lindCost(m)) {
            // adaptive splitting; sumq(T) = number of x=1 in T
            auto sumq = [&](const vector<int>& T) {
                vector<char> bb(m, 0);
                for (int i : T) bb[i] = 1;
                int a = ask(r, bb);
                return (a + Xtot - m + (int)T.size()) / 2;
            };
            vector<pair<vector<int>, int>> st;
            vector<int> all(m); iota(all.begin(), all.end(), 0);
            st.push_back({all, Xtot});
            while (!st.empty()) {
                auto [G, c] = st.back(); st.pop_back();
                if (c == 0) continue;
                if (c == (int)G.size()) { for (int i : G) X[i] = 1; continue; }
                int cc = flip ? (int)G.size() - c : c;  // minority count in G
                (void)cc;
                vector<int> G1(G.begin(), G.begin() + G.size() / 2), G2(G.begin() + G.size() / 2, G.end());
                int c1 = sumq(G1);
                st.push_back({G1, c1});
                st.push_back({G2, c - c1});
            }
        } else {
            LindPlan P = makePlan(m);
            vector<long long> val(P.N, 0);
            for (int T = 1; T < P.N; T++) {
                int c1 = 0;
                for (int i = 0; i < m; i++) { b0[i] = coinIn(P.coins[i], T); c1 += b0[i]; }
                int a = ask(r, b0);
                val[T] = (a - (m - c1) + Xtot) / 2;
            }
            vector<int> xs = decodePlan(P, val);
            for (int i = 0; i < m; i++) X[i] = xs[i];
        }
    }
    stq.assign(n + 1, {});
    for (int v = 1; v <= n; v++) dep[v] = -1;
    dep[r] = 0;
    bool shallow = false;
    {
        vector<int> others;
        for (int v = 1; v <= n; v++) if (v != r) others.push_back(v);
        mt19937 srng(987654);
        shuffle(others.begin(), others.end(), srng);
        int K = min((int)others.size(), SAMPLEK);
        int mx = 0;
        for (int i = 0; i < K; i++) { dep[others[i]] = depthQuery(others[i]); mx = max(mx, dep[others[i]]); }
        shallow = (n >= 20 && mx <= 2 && !getenv("NOSHALLOW"));
    }
    if (shallow) solveShallow();
    else {
        for (int v = 1; v <= n; v++) if (dep[v] < 0) dep[v] = depthQuery(v);
        solveStructure();
    }
    // output
    string out = "!";
    for (int i = 0; i < m; i++) {
        int c = childOf[i], p = par[c];
        int s, f;
        if (X[i]) { s = c; f = p; } else { s = p; f = c; }
        out += " " + to_string(s) + " " + to_string(f);
    }
#ifdef LOCAL
    localSubmit(out);
#else
    printf("%s\n", out.c_str());
    fflush(stdout);
#endif
#ifdef LOCAL
    }
    fprintf(stderr,"phase3 search Q %.1f probe %.1f refineLind %.1f tests %.1f\n",statQ/Lcases,statP/Lcases,statR/Lcases,statT/Lcases);
    localFinish();
#endif
    return 0;
}

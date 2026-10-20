#include <bits/stdc++.h>
using namespace std;

static const int INFI = 1000000000;
static const int MAXN = 100005;

int n;
vector<vector<int>> adj_;
vector<int> offv, degv;          // per-vertex offset into relation/DSU arrays
vector<int> relS, relP;          // relation: arrival edge pos -> departure edge pos
vector<int> dPv, dSv;            // per-vertex DSU (parent abs index / component size)
vector<int> fPos, lPos;          // first edge pos (departure) / last edge pos (arrival terminal)
vector<char> tgt;                // vertex already used as a target
vector<int> parv, depv, tinv, toutv, szv, posPar, posInPar;
vector<int> childOff, childList, childCnt;
vector<int> segv; int Sseg;
vector<int> pathBuf, tmpBuf;

int lgRelS[MAXN], lgRelP[MAXN], lgUA[MAXN], lgUB[MAXN], lgUC[MAXN];
int lgFv[MAXN], lgFo[MAXN], lgLv[MAXN], lgLo[MAXN];
int cRel = 0, cU = 0, cF = 0, cL = 0;

static inline int findr(int v, int i) {
    int e = offv[v] + i;
    while (dPv[e] >= 0) e = dPv[e];
    return e;
}
static inline int posInAdj(int v, int w) {   // position of neighbour w in adj_[v]
    return (w == parv[v]) ? posPar[v] : posInPar[w];
}
static inline void rollbackRel(int sRel, int sU) {
    while (cRel > sRel) { cRel--; relS[lgRelS[cRel]] = -1; relP[lgRelP[cRel]] = -1; }
    while (cU > sU) { cU--; dPv[lgUB[cU]] = -1; dSv[lgUA[cU]] = lgUC[cU]; }
}
static inline void rollbackAll(int sRel, int sU, int sF, int sL) {
    rollbackRel(sRel, sU);
    while (cF > sF) { cF--; fPos[lgFv[cF]] = lgFo[cF]; }
    while (cL > sL) { cL--; lPos[lgLv[cL]] = lgLo[cL]; }
}

// ---------- segment tree over Euler order: minimum unused label in a range ----------
void segInit() {
    Sseg = 1; while (Sseg < n) Sseg <<= 1;
    segv.assign(2 * Sseg, INFI);
    for (int v = 0; v < n; v++) segv[Sseg + tinv[v]] = tgt[v] ? INFI : v;
    for (int i = Sseg - 1; i >= 1; i--) segv[i] = min(segv[2 * i], segv[2 * i + 1]);
}
static inline void segSet(int pos, int val) {
    int i = Sseg + pos; segv[i] = val;
    for (i >>= 1; i; i >>= 1) segv[i] = min(segv[2 * i], segv[2 * i + 1]);
}
static inline int segQuery(int l, int r) {
    if (l > r) return INFI;
    int res = INFI;
    for (l += Sseg, r += Sseg + 1; l < r; l >>= 1, r >>= 1) {
        if (l & 1) res = min(res, segv[l++]);
        if (r & 1) res = min(res, segv[--r]);
    }
    return res;
}
// minimum unused label in the component of T-v containing neighbour w
static inline int subtreeMinFree(int v, int w) {
    if (w == parv[v]) return min(segQuery(0, tinv[v] - 1), segQuery(toutv[v] + 1, n - 1));
    return segQuery(tinv[w], toutv[w]);
}

// ---------- commit: append the walk x -> y, checking every rule ----------
bool testAssign(int x, int y) {
    int k = 0;
    int a = x, b = y;
    while (depv[a] > depv[b]) { pathBuf[k++] = a; a = parv[a]; }
    int t = 0;
    while (depv[b] > depv[a]) { tmpBuf[t++] = b; b = parv[b]; }
    while (a != b) { pathBuf[k++] = a; a = parv[a]; tmpBuf[t++] = b; b = parv[b]; }
    pathBuf[k++] = a;
    for (int i = t - 1; i >= 0; i--) pathBuf[k++] = tmpBuf[i];

    cRel = cU = cF = cL = 0;
    bool ok = true;

    if (degv[x] >= 2) {
        int d = (pathBuf[1] == parv[x]) ? posPar[x] : posInPar[pathBuf[1]];
        if (relP[offv[x] + d] != -1) return false;
        lgFv[cF] = x; lgFo[cF] = fPos[x]; cF++;
        fPos[x] = d;
        int lp = lPos[x];
        if (lp != -1) {
            int rf = findr(x, d), rl = findr(x, lp);
            if (rf == rl && dSv[rf] != degv[x]) ok = false;
        }
    }

    for (int i = 1; ok && i + 1 < k; i++) {
        int v = pathBuf[i];
        int pv = pathBuf[i - 1], nv = pathBuf[i + 1];
        int pa = (pv == parv[v]) ? posPar[v] : posInPar[pv];
        int pb = (nv == parv[v]) ? posPar[v] : posInPar[nv];
        if (lPos[v] == pa) { ok = false; break; }
        int cur = relS[offv[v] + pa];
        if (cur != -1) {
            if (cur != pb) ok = false;
        } else {
            if (relP[offv[v] + pb] != -1) { ok = false; break; }
            if (fPos[v] == pb) { ok = false; break; }
            int ea = offv[v] + pa, eb = offv[v] + pb;
            int ra = ea; while (dPv[ra] >= 0) ra = dPv[ra];
            int rb = eb; while (dPv[rb] >= 0) rb = dPv[rb];
            if (ra == rb) { ok = false; break; }
            relS[ea] = pb; relP[eb] = pa;
            lgRelS[cRel] = ea; lgRelP[cRel] = eb; cRel++;
            int sa = dSv[ra], sb = dSv[rb];
            if (sa < sb) { swap(ra, rb); swap(sa, sb); }
            dPv[rb] = ra; dSv[ra] = sa + sb;
            lgUA[cU] = ra; lgUB[cU] = rb; lgUC[cU] = sa; cU++;
            int f = fPos[v], ll = lPos[v];
            if (f != -1 && ll != -1) {
                int rf = findr(v, f), rl = findr(v, ll);
                if (rf == rl && dSv[rf] != degv[v]) { ok = false; break; }
            }
        }
    }

    if (ok) {
        int a2 = pathBuf[k - 2];
        int pa = (a2 == parv[y]) ? posPar[y] : posInPar[a2];
        if (degv[y] >= 2) {
            if (relS[offv[y] + pa] != -1 || lPos[y] != -1 || fPos[y] == pa) ok = false;
            else {
                lgLv[cL] = y; lgLo[cL] = lPos[y]; cL++;
                lPos[y] = pa;
                int f = fPos[y];
                if (f != -1) {
                    int rf = findr(y, f), rl = findr(y, pa);
                    if (rf == rl && dSv[rf] != degv[y]) ok = false;
                }
            }
        }
    }

    if (!ok) { rollbackAll(0, 0, 0, 0); return false; }
    return true;
}

// ---------- pruned search for the minimum valid target of token x ----------
// Each frame keeps a set of Euler intervals ("region") that are still worth exploring.
// The direction taken at the frame's vertex is determined by the smallest unused label
// in the region; intervals belonging to already explored / blocked directions are cut out.
struct Frame { int v, posa, best, ivBase, niv, pendRel, pendU, pendDir; };
vector<Frame> fst;
vector<int> ivM, ivL, ivR;   // interval scratch: three parallel-ish arrays are avoided; use one
int ivTop = 0;

static inline void frameAddIv(Frame &f, int mnf, int l, int r) {
    int p = f.ivBase + f.niv;
    ivM[p] = mnf; ivL[p] = l; ivR[p] = r;
    f.niv++;
    ivTop = f.ivBase + f.niv;
}
static inline void frameDelIv(Frame &f, int i) {
    int last = f.niv - 1;
    if (i != last) { ivM[f.ivBase + i] = ivM[f.ivBase + last]; ivL[f.ivBase + i] = ivL[f.ivBase + last]; ivR[f.ivBase + i] = ivR[f.ivBase + last]; }
    f.niv--;
    ivTop = f.ivBase + f.niv;
}
// cut [a,b] out of the region
static void frameCut(Frame &f, int a, int b) {
    if (a > b) return;
    for (int i = 0; i < f.niv; i++) {
        int p = f.ivBase + i;
        if (ivL[p] <= a && b <= ivR[p]) {
            int ml = ivL[p], mr = ivR[p];
            frameDelIv(f, i);
            if (ml <= a - 1) frameAddIv(f, segQuery(ml, a - 1), ml, a - 1);
            if (b + 1 <= mr) frameAddIv(f, segQuery(b + 1, mr), b + 1, mr);
            return;
        }
    }
}
// cut the component of T-v reached through edge position dirPos
static void frameCutDir(Frame &f, int dirPos) {
    int v = f.v;
    if (dirPos == posPar[v]) { frameCut(f, 0, tinv[v] - 1); frameCut(f, toutv[v] + 1, n - 1); }
    else { int c = adj_[v][dirPos]; frameCut(f, tinv[c], toutv[c]); }
}

static void frameInit(Frame &f, int v, int posa) {
    f.v = v; f.posa = posa; f.best = INFI; f.niv = 0;
    f.pendRel = -1; f.pendU = -1; f.pendDir = -1;
    ivTop = f.ivBase;
    if (lPos[v] == posa) return;                 // the walk stops here (v is a used target)
    if (relS[offv[v] + posa] != -1) return;      // arrival arc already used
    if (!tgt[v] && fPos[v] != posa) {            // option: stop at v
        bool okE = true;
        if (fPos[v] != -1) {
            int rf = findr(v, fPos[v]), rl = findr(v, posa);
            if (rf == rl && dSv[rf] != degv[v]) okE = false;
        }
        if (okE) f.best = v;
    }
    if (posa == posPar[v]) {
        frameAddIv(f, segQuery(tinv[v] + 1, toutv[v]), tinv[v] + 1, toutv[v]);
    } else {
        int c = adj_[v][posa];
        frameAddIv(f, segQuery(0, tinv[v] - 1), 0, tinv[v] - 1);
        frameAddIv(f, segQuery(tinv[v] + 1, tinv[c] - 1), tinv[v] + 1, tinv[c] - 1);
        frameAddIv(f, segQuery(toutv[c] + 1, n - 1), toutv[c] + 1, n - 1);
    }
    // drop intervals with no unused label
    for (int i = f.niv - 1; i >= 0; i--) if (ivM[f.ivBase + i] >= INFI) frameDelIv(f, i);
    ivTop = f.ivBase + f.niv;
}

// minimum valid target when the walk leaves x towards n0 (INFI if none)
int searchSide(int x, int n0) {
    fst.clear(); ivTop = 0;
    fst.push_back(Frame());
    fst.back().ivBase = 0;
    frameInit(fst.back(), n0, posInAdj(n0, x));
    int res = INFI;
    while (!fst.empty()) {
        Frame &f = fst.back();
        // pick the interval with the smallest unused label
        int bi = -1, bmnf = INFI;
        for (int i = 0; i < f.niv; i++) {
            int m = ivM[f.ivBase + i];
            if (m < bmnf) { bmnf = m; bi = i; }
        }
        if (bi < 0 || bmnf >= f.best) {
            int fres = f.best;
            fst.pop_back();
            ivTop = fst.empty() ? 0 : (fst.back().ivBase + fst.back().niv);
            if (!fst.empty()) {
                Frame &pf = fst.back();
                rollbackRel(pf.pendRel, pf.pendU);
                if (fres < pf.best) pf.best = fres;
                if (pf.pendDir >= 0) frameCutDir(pf, pf.pendDir);
            } else if (fres < res) res = fres;
            continue;
        }
        int y = bmnf;
        int v = f.v;
        int dirPos;
        if (tinv[y] < tinv[v] || tinv[y] > toutv[v]) {
            dirPos = posPar[v];                       // parent direction
        } else {
            int lo = childOff[v], hi = childOff[v] + childCnt[v] - 1;   // child containing tin[y]
            while (lo < hi) {
                int m = (lo + hi + 1) >> 1;
                if (tinv[childList[m]] <= tinv[y]) lo = m; else hi = m - 1;
            }
            dirPos = posInPar[childList[lo]];
        }
        bool allowed = true;
        if (relP[offv[v] + dirPos] != -1) allowed = false;
        else if (fPos[v] == dirPos) allowed = false;
        else if (findr(v, dirPos) == findr(v, f.posa)) allowed = false;
        if (!allowed) { frameCutDir(f, dirPos); continue; }
        // descend through this direction
        int ra = findr(v, f.posa), rb = findr(v, dirPos);
        int sRel = cRel, sU = cU;
        int ea = offv[v] + f.posa, eb = offv[v] + dirPos;
        relS[ea] = dirPos; relP[eb] = f.posa;
        lgRelS[cRel] = ea; lgRelP[cRel] = eb; cRel++;
        int sa = dSv[ra], sb = dSv[rb];
        if (sa < sb) { swap(ra, rb); swap(sa, sb); }
        dPv[rb] = ra; dSv[ra] = sa + sb;
        lgUA[cU] = ra; lgUB[cU] = rb; lgUC[cU] = sa; cU++;
        bool okE = true;
        int fp = fPos[v], lq = lPos[v];
        if (fp != -1 && lq != -1) {
            int rf = findr(v, fp), rl = findr(v, lq);
            if (rf == rl && dSv[rf] != degv[v]) okE = false;
        }
        if (!okE) { rollbackRel(sRel, sU); frameCutDir(f, dirPos); continue; }
        int w = adj_[v][dirPos];
        f.pendRel = sRel; f.pendU = sU; f.pendDir = dirPos;
        int newBase = f.ivBase + f.niv;
        int posa2 = posInAdj(w, v);
        fst.push_back(Frame());
        fst.back().ivBase = newBase;
        frameInit(fst.back(), w, posa2);
    }
    return res;
}

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        scanf("%d", &n);
        vector<int> p(n);
        for (int i = 0; i < n; i++) { scanf("%d", &p[i]); p[i]--; }
        adj_.assign(n, {});
        for (int i = 0; i + 1 < n; i++) {
            int u, v; scanf("%d %d", &u, &v); u--; v--;
            adj_[u].push_back(v); adj_[v].push_back(u);
        }
        if (n == 1) { printf("1\n"); continue; }

        for (int v = 0; v < n; v++) sort(adj_[v].begin(), adj_[v].end());
        offv.assign(n, 0); degv.assign(n, 0);
        int tot = 0;
        for (int v = 0; v < n; v++) { offv[v] = tot; degv[v] = (int)adj_[v].size(); tot += degv[v]; }
        relS.assign(tot, -1); relP.assign(tot, -1);
        dPv.assign(tot, -1); dSv.assign(tot, 1);
        fPos.assign(n, -1); lPos.assign(n, -1); tgt.assign(n, 0);

        parv.assign(n, -1); depv.assign(n, 0);
        tinv.assign(n, 0); toutv.assign(n, 0); szv.assign(n, 1);
        posPar.assign(n, -1); posInPar.assign(n, -1);
        childOff.assign(n + 1, 0); childList.assign(n + 1, 0); childCnt.assign(n, 0);
        {
            vector<int> stk; stk.reserve(n); stk.push_back(0);
            vector<int> order; order.reserve(n);
            vector<char> seen(n, 0); seen[0] = 1;
            while (!stk.empty()) {
                int v = stk.back(); stk.pop_back();
                order.push_back(v);
                for (int w : adj_[v]) if (!seen[w]) { seen[w] = 1; parv[w] = v; depv[w] = depv[v] + 1; stk.push_back(w); }
            }
            for (int i = 0; i < n; i++) tinv[order[i]] = i;
            for (int i = n - 1; i >= 1; i--) szv[parv[order[i]]] += szv[order[i]];
            for (int v = 0; v < n; v++) toutv[v] = tinv[v] + szv[v] - 1;
            for (int v = 1; v < n; v++) {
                int lo = 0, hi = degv[v] - 1, t = parv[v];
                while (lo < hi) { int m = (lo + hi) >> 1; if (adj_[v][m] < t) lo = m + 1; else hi = m; }
                posPar[v] = lo;
                lo = 0; hi = degv[t] - 1;
                while (lo < hi) { int m = (lo + hi) >> 1; if (adj_[t][m] < v) lo = m + 1; else hi = m; }
                posInPar[v] = lo;
            }
            // children lists in preorder (i.e. sorted by tin)
            for (int v = 0; v < n; v++) childCnt[v] = 0;
            for (int v = 1; v < n; v++) childCnt[parv[v]]++;
            int acc = 0;
            for (int v = 0; v < n; v++) { childOff[v] = acc; acc += childCnt[v]; }
            childOff[n] = acc;
            vector<int> ptr(n);
            for (int v = 0; v < n; v++) ptr[v] = childOff[v];
            for (int i = 1; i < n; i++) { int v = order[i]; childList[ptr[parv[v]]++] = v; }
        }
        pathBuf.assign(n + 2, 0); tmpBuf.assign(n + 2, 0);
        fst.reserve(n + 2);
        ivM.assign(24 * (size_t)n + 100, INFI);
        ivL.assign(24 * (size_t)n + 100, 0);
        ivR.assign(24 * (size_t)n + 100, 0);

        segInit();

        vector<int> ans(n);
        vector<pair<int,int>> sideList;
        for (int step = 0; step < n; step++) {
            int x = p[step];
            int best = INFI;
            sideList.clear();
            if (x != 0) {
                int mf = subtreeMinFree(x, parv[x]);
                if (mf < INFI) sideList.push_back({mf, parv[x]});
            }
            for (int w : adj_[x]) if (w != parv[x]) {
                int mf = subtreeMinFree(x, w);
                if (mf < INFI) sideList.push_back({mf, w});
            }
            sort(sideList.begin(), sideList.end());
            for (size_t si = 0; si < sideList.size(); si++) {
                int mf = sideList[si].first, n0 = sideList[si].second;
                if (mf >= best) break;
                int depPos = posInAdj(x, n0);
                if (degv[x] >= 2) {
                    if (relP[offv[x] + depPos] != -1) continue;
                    int lp = lPos[x];
                    if (lp != -1) {
                        int rf = findr(x, depPos), rl = findr(x, lp);
                        if (rf == rl && dSv[rf] != degv[x]) continue;
                    }
                    lgFv[cF] = x; lgFo[cF] = fPos[x]; cF++;
                    fPos[x] = depPos;
                }
                int r = searchSide(x, n0);
                if (degv[x] >= 2) { cF--; fPos[lgFv[cF]] = lgFo[cF]; }
                if (r < best) best = r;
            }
            int chosen = best;
            if (chosen >= INFI || !testAssign(x, chosen)) {
                chosen = -1;
                for (int y = 0; y < n; y++) {
                    if (tgt[y] || y == x) continue;
                    if (testAssign(x, y)) { chosen = y; break; }
                }
            }
            tgt[chosen] = 1;
            segSet(tinv[chosen], INFI);
            ans[step] = chosen;
        }
        for (int i = 0; i < n; i++) printf("%d%c", ans[i] + 1, " \n"[i + 1 == n]);
    }
    return 0;
}

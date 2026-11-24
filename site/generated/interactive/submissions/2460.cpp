#include <bits/stdc++.h>
using namespace std;
typedef bitset<512> BS;

static int N;
static long long opsCount = 0;
static long long trivP = 0, trivQ = 0, predP = 0, predQ = 0;
static double KTH = 35;
static double PEELR = 15;
static double DECX = 100, KSMALL = 12, GROWTH = 1000;
static int SKIPON = 1, DPSKIP = 0;
static double SKIPM = 0;
static int SKIPC = -1000;
static double SKIPTAU = 0;
#ifdef SIM
static vector<int> simLab;
static deque<int> simQ;
static vector<int> simCnt;
static int simDistinct = 0;
static int doOp(char t, int x) {
    opsCount++;
    if (t == '+') {
        simQ.push_back(x);
        if (simCnt[simLab[x]]++ == 0) simDistinct++;
    } else {
        int y = simQ.front();
        simQ.pop_front();
        if (--simCnt[simLab[y]] == 0) simDistinct--;
    }
    return simDistinct;
}
#else
static int doOp(char t, int x) {
    opsCount++;
    if (t == '+') printf("+ %d\n", x + 1);
    else printf("-\n");
    fflush(stdout);
    int c;
    if (scanf("%d", &c) != 1) exit(0);
    if (c < 0) exit(0);
    return c;
}
#endif

static void reportAnswer(const vector<vector<int>> &cls) {
#ifdef SIM
    bool ok = true;
    for (auto &c : cls)
        for (int v : c) if (simLab[v] != simLab[c[0]]) ok = false;
    set<int> labs; for (int v : simLab) labs.insert(v);
    if ((int)labs.size() != (int)cls.size()) ok = false;
    if (!ok) { fprintf(stderr, "WRONG ANSWER\n"); exit(1); }
#else
    printf("! %d\n", (int)cls.size());
    for (auto &c : cls) {
        printf("%d", (int)c.size());
        for (int v : c) printf(" %d", v + 1);
        printf("\n");
    }
    fflush(stdout);
#endif
}

struct ChainSolver {
    int n = 0;
    vector<int> el;
    double kHint = -1;
    bool decideMode = false;
    int switchTo = 0;
    vector<char> prefNew;
    vector<int> groupOf;

    vector<BS> M, Mt;  // M[t]: possible heads of next chain; Mt[h]: possible tails of prev chain
    BS full;
    vector<int> par, headOf, tailOf;
    vector<char> closedR;
    vector<int> qcnt;
    vector<char> isTail, isHead;
    deque<int> Q;
    int cur = 0;
    int lastPushed = -1;
    double r = 0.9;
    double kEst = -1;
    bool firstLap = true;
    vector<char> pushedOnce;
    int closedCnt = 0;
    vector<int> work;
    vector<char> inWork;
    vector<vector<double>> cumIn, cumOut;
    vector<vector<short>> cntIn, cntOut;
    long long lastProgress = 0;
    vector<char> dirtyRow, dirtyCol;
    bool globalDirty = true;
    vector<int> tailPref;
    int openTails = 0;

    int find(int x) { return par[x] == x ? x : par[x] = find(par[x]); }
    int fdist(int a, int b) const { return ((b - a) % n + n) % n; }
    int tailsBetween(int a, int b) const {
        int fd = fdist(a, b);
        if (a == b) fd = n;
        if (fd <= 1) return 0;
        int lo = (a + 1) % n;
        int cnt = fd - 1;
        int hi = lo + cnt - 1;
        if (hi < n) return tailPref[hi + 1] - tailPref[lo];
        return (tailPref[n] - tailPref[lo]) + tailPref[hi - n + 1];
    }
    void rebuildTailPref() {
        tailPref.assign(n + 1, 0);
        for (int i = 0; i < n; i++) tailPref[i + 1] = tailPref[i] + (isTail[i] ? 1 : 0);
        openTails = tailPref[n];
    }
    void removeArc(int t, int h) {
        if (!M[t][h]) return;
        lastProgress = opsCount;
        M[t].reset(h);
        Mt[h].reset(t);
        dirtyRow[t] = 1;
        dirtyCol[h] = 1;
        if (!inWork[t]) { inWork[t] = 1; work.push_back(t); }
        if (!inWork[n + h]) { inWork[n + h] = 1; work.push_back(n + h); }
    }
    void keepCol(int h, const BS &S) {
        BS rem = Mt[h] & ~S;
        for (int t = rem._Find_first(); t < 512; t = rem._Find_next(t)) removeArc(t, h);
    }
    void keepRow(int t, const BS &S) {
        BS rem = M[t] & ~S;
        for (int h = rem._Find_first(); h < 512; h = rem._Find_next(h)) removeArc(t, h);
    }
    BS openRange(int a, int b) const {
        BS m;
        int cnt = fdist(a, b) - 1;
        if (a == b) cnt = n - 1;
        if (cnt <= 0) return m;
        int lo = (a + 1) % n;
        int hi = lo + cnt - 1;
        if (hi < n) {
            BS x = full; x >>= (n - cnt); x <<= lo; return x;
        } else {
            BS x = full; x >>= lo; x <<= lo;
            BS y = full; y >>= (n - (hi - n + 1));
            return x | y;
        }
    }
    void fixArc(int t, int h) {
        BS kr; kr.set(h);
        keepRow(t, kr);
        BS kc; kc.set(t);
        keepCol(h, kc);
        int A = find(t), B = find(h);
        isTail[t] = 0;
        isHead[h] = 0;
        openTails--;
        tailsChanged = true;
        if (A == B) {
            closedR[A] = 1;
            closedCnt++;
            return;
        }
        par[B] = A;
        tailOf[A] = tailOf[B];
        qcnt[A] += qcnt[B];
        int hh = headOf[A], tt = tailOf[A];
        BS inside = openRange(hh, tt);
        keepCol(hh, ~inside & full);
        keepRow(tt, ~inside & full);
    }
    void propagate() {
        while (!work.empty()) {
            int v = work.back();
            work.pop_back();
            inWork[v] = 0;
            if (v >= n) {
                int h = v - n;
                if (isHead[h] && Mt[h].count() == 1) fixArc(Mt[h]._Find_first(), h);
            } else {
                int t = v;
                if (isTail[t] && M[t].count() == 1) fixArc(t, M[t]._Find_first());
            }
        }
    }
    double scaleUsed = 1.0;
    int wdist(int a, int b) const {
        int d = fdist(a, b); if (d == 0) d = n;
        return (int)(d * scaleUsed + 0.5);
    }
    void recomputeCol(int h) {
        auto &c = cumIn[h];
        static vector<double> w;
        w.assign(n + 1, 0.0);
        static vector<short> cw;
        cw.assign(n + 1, 0);
        int dmin = INT_MAX;
        for (int t = Mt[h]._Find_first(); t < 512; t = Mt[h]._Find_next(t)) dmin = min(dmin, wdist(t, h));
        for (int t = Mt[h]._Find_first(); t < 512; t = Mt[h]._Find_next(t)) {
            int d = fdist(t, h);
            if (d == 0) d = n;
            w[d] += powT[wdist(t, h) - dmin];
            cw[d]++;
        }
        c[0] = 0;
        auto &cc = cntIn[h];
        cc[0] = 0;
        for (int d = 1; d <= n; d++) { c[d] = c[d - 1] + w[d]; cc[d] = cc[d - 1] + cw[d]; }
    }
    void recomputeRow(int t) {
        auto &c = cumOut[t];
        static vector<double> w;
        w.assign(n + 1, 0.0);
        static vector<short> cw;
        cw.assign(n + 1, 0);
        int dmin = INT_MAX;
        for (int h = M[t]._Find_first(); h < 512; h = M[t]._Find_next(h)) dmin = min(dmin, wdist(t, h));
        for (int h = M[t]._Find_first(); h < 512; h = M[t]._Find_next(h)) {
            int d = fdist(t, h);
            if (d == 0) d = n;
            w[d] += powT[wdist(t, h) - dmin];
            cw[d]++;
        }
        c[0] = 0;
        auto &cc = cntOut[t];
        cc[0] = 0;
        for (int d = 1; d <= n; d++) { c[d] = c[d - 1] + w[d]; cc[d] = cc[d - 1] + cw[d]; }
    }
    vector<double> powT;
    bool tailsChanged = true;
    double powR = -1;
    void refreshCum() {
        if (powR != r || (int)powT.size() < n + 2) { powR = r; powT.assign(n + 2, 1.0); for (int i = 1; i < n + 2; i++) powT[i] = powT[i - 1] * r; globalDirty = true; }
        {
            double sc = max(1, openTails) / (double)n;
            if (fabs(sc - scaleUsed) > 0.1 * scaleUsed) { scaleUsed = sc; globalDirty = true; }
        }
        if (tailsChanged) { rebuildTailPref(); tailsChanged = false; }
        if (globalDirty) { globalVer++; globalDirty = false; }
        for (int i = 0; i < n; i++) {
            if (dirtyCol[i]) { verCol[i] = -1; dirtyCol[i] = 0; }
            if (dirtyRow[i]) { verRow[i] = -1; dirtyRow[i] = 0; }
        }
    }
    int globalVer = 0;
    vector<int> verRow, verCol;
    void ensureCol(int h) { if (verCol[h] != globalVer) { recomputeCol(h); verCol[h] = globalVer; } }
    void ensureRow(int t) { if (verRow[t] != globalVer) { recomputeRow(t); verRow[t] = globalVer; } }
    bool done() const { return openTails == 0; }
    static double H2(double p) {
        static vector<double> tab;
        if (tab.empty()) { tab.resize(4097); for (int i = 0; i <= 4096; i++) { double q = i / 4096.0; tab[i] = (q <= 0 || q >= 1) ? 0 : -(q * log2(q) + (1 - q) * log2(1 - q)); } }
        if (p <= 1e-12 || p >= 1 - 1e-12) return 0;
        double f = p * 4096; int i = (int)f; if (i >= 4096) return tab[4096];
        double a = f - i;
        double v = tab[i] * (1 - a) + tab[i + 1] * a;
        if (i == 0) v = max(v, 1e-9);
        if (i >= 4095) v = max(v, 1e-9);
        return v;
    }
    BS presentTails() {
        BS s;
        for (int e : Q) s.set(tailOf[find(e)]);
        return s;
    }
    BS presentHeads() {
        BS s;
        for (int e : Q) s.set(headOf[find(e)]);
        return s;
    }
    void push(int x) {
        int X = find(x);
        bool triv = qcnt[X] > 0;
        BS pt = presentTails();
        int frontPos = Q.empty() ? -1 : Q.front();
        int c = doOp('+', el[x]);
        int delta = c - cur;
        cur = c;
        Q.push_back(x);
        qcnt[X]++;
        lastPushed = x;
        skippedBS.reset(x);
        if (pushedOnce[x]) firstLap = false;
        pushedOnce[x] = 1;
        if (triv) {
#ifdef SIM
            trivP++;
#endif
            return;
        }
        int h = headOf[X];
        if (!isHead[h]) return;
#ifdef SIM
        if ((Mt[h] & pt).none() || (Mt[h] & ~pt).none()) predP++;
#endif
        BS rng = pt;
        if (frontPos >= 0) rng |= skippedBS & openRange((frontPos - 1 + n) % n, x);
        if (delta == 0) keepCol(h, rng);
        else keepCol(h, ~pt & full);
        propagate();
    }
    void pop() {
        int y = Q.front();
        int Z = find(y);
        int c = doOp('-', 0);
        int delta = c - cur;
        cur = c;
        Q.pop_front();
        qcnt[Z]--;
        if (qcnt[Z] > 0) {
#ifdef SIM
            trivQ++;
#endif
            return;
        }
        int t = tailOf[Z];
        if (!isTail[t]) return;
        if (Q.empty()) return;
        BS ph = presentHeads();
        BS rng = ph;
        int b = Q.back();
        if (fdist(y, t) < fdist(y, b) || t == y) {
            if (t != b) {
                BS sk = skippedBS & openRange(t, (b + 1) % n);
                for (int q = sk._Find_first(); q < 512; q = sk._Find_next(q)) if (isTail[q]) rng.set(headOf[find(q)]);
            }
        }
#ifdef SIM
        if ((M[t] & ph).none() || (M[t] & ~ph).none()) predQ++;
#endif
        if (delta == 0) keepRow(t, rng);
        else keepRow(t, ~ph & full);
        propagate();
    }
    void updateK() {
        if (!firstLap) return;
        int L = Q.size();
        if (L < 4) return;
        int c = cur;
        double k;
        if (c >= L) k = 4.0 * n;
        else if (c <= 1) k = 1;
        else {
            double lo = c, hi = 4.0 * n;
            for (int it = 0; it < 60; it++) {
                double mid = (lo + hi) / 2;
                double e = mid * (1 - pow(1 - 1 / mid, L));
                if (e < c) lo = mid; else hi = mid;
            }
            k = (lo + hi) / 2;
        }
        kEst = k;
    }
    void updateR() {
        double kopen = max(1.0, kEst - closedCnt);
        double nr = 1 - 1 / kopen;
        nr = max(nr, 0.15);
        if (fabs(log((1 - nr) / (1 - r))) > 0.1) {
            r = nr;
            globalDirty = true;
        }
    }
    int nextPushable(int p, int frontPos) {
        for (int s = 1; s <= n; s++) {
            int q = (p + s) % n;
            if (frontPos >= 0 && q == frontPos) return -1;
            if (!isTail[q]) continue;
            if (qcnt[find(q)] > 0) continue;
            return q;
        }
        return -1;
    }

    void init() {
        M.assign(n, BS());
        Mt.assign(n, BS());
        full.reset();
        for (int i = 0; i < n; i++) full.set(i);
        for (int i = 0; i < n; i++) { M[i] = full; Mt[i] = full; }
        par.resize(n); iota(par.begin(), par.end(), 0);
        headOf.resize(n); iota(headOf.begin(), headOf.end(), 0);
        tailOf.resize(n); iota(tailOf.begin(), tailOf.end(), 0);
        closedR.assign(n, 0);
        qcnt.assign(n, 0);
        isTail.assign(n, 1); isHead.assign(n, 1);
        pushedOnce.assign(n, 0);
        inWork.assign(2 * n, 0);
        dirtyRow.assign(n, 0); dirtyCol.assign(n, 0);
        cumIn.assign(n, vector<double>(n + 1));
        cumOut.assign(n, vector<double>(n + 1));
        cntIn.assign(n, vector<short>(n + 1));
        cntOut.assign(n, vector<short>(n + 1));
        verRow.assign(n, -1); verCol.assign(n, -1);
        rebuildTailPref();
    }
    static double invertK(int L, int c, int nmax) {
        if (c >= L) return 4.0 * nmax;
        if (c <= 1) return 1;
        double lo = c, hi = 4.0 * nmax;
        for (int it = 0; it < 60; it++) {
            double mid = (lo + hi) / 2;
            double e = mid * (1 - pow(1 - 1 / mid, L));
            if (e < c) lo = mid; else hi = mid;
        }
        return (lo + hi) / 2;
    }
    // returns 0 if solved (groupOf filled), 1 if decided to switch (decideMode)
    int run() {
        init();
        if (n == 1) { finish(); return 0; }
        kEst = kHint > 0 ? kHint : sqrt((double)n);
        r = max(0.15, 1 - 1 / kEst);
        globalDirty = true;
        if (decideMode) {
            int x = 0;
            vector<int> cAt(1, 0);
            int lim = min(n, (int)DECX);
            while (x < n) {
                int before = cur;
                push(x);
                prefNew.push_back(cur > before);
                x++;
                cAt.push_back(cur);
                if (done()) { finish(); return 0; }
                int rep = x - cur;
                double ke = invertK(x, cur, n);
                if (rep >= 10 && ke < KSMALL) { kEst = ke; break; }
                if (x == lim) {
                    int growth = cur - cAt[x / 2];
                    if ((ke >= KTH || growth >= GROWTH) && x < n) { switchTo = 1; return 1; }
                    if (x == n && ke >= KTH) { switchTo = 1; return 1; }
                    kEst = ke;
                    break;
                }
            }
        } else push(0);
        long long iters = 0;
        while (!done()) {
#ifdef SIM
            if (++iters > 200000) { fprintf(stderr, "CHAINLOOP n=%d open=%d Q=%zu ops=%lld\n", n, openTails, Q.size(), opsCount); exit(6); }
#endif
            updateK();
            updateR();
            refreshCum();
            if (opsCount - lastProgress > 3LL * n + 60) { stallFallback(); lastProgress = opsCount; continue; }
            int act = decide();
            if (act == 0) {
                int fp = Q.empty() ? -1 : Q.front();
                int s = nextPushable(lastPushed, fp);
                if (s < 0) { pop(); continue; }
                if (SKIPON && !Q.empty() && skipStreak < n && canSkip(s)) { lastPushed = s; skippedBS.set(s); skipStreak++; continue; }
                skipStreak = 0;
                push(s);
            } else pop();
        }
        finish();
        return 0;
    }
    void flushQ() { while (!Q.empty()) { Q.pop_front(); cur = doOp('-', 0); } }
    int skipStreak = 0;
    BS skippedBS;
    bool canSkip(int s) {
        int X = find(s);
        int h = headOf[X];
        int fp = Q.front();
        if (isHead[h]) {
            BS pt = presentTails();
            BS rng = pt | openRange((fp - 1 + n) % n, s);
            if ((Mt[h] & rng).any()) {
                if (SKIPTAU <= 0) return false;
                ensureCol(h);
                int d; int fh = fdist(h, fp);
                if (h != s && fh > 0 && fh <= fdist(h, s)) d = 0; else d = fdist(fp, h);
                const auto &c = cumIn[h];
                if (c[n] <= 0) return false;
                double P = c[d] / c[n];
                if (P >= SKIPTAU) return false;
            }
        }
        if (!isTail[s]) return false;
        int Lw = fdist(fp, s);
        int lim = (int)(Lw * SKIPM) + SKIPC;
        for (int hh = M[s]._Find_first(); hh < 512; hh = M[s]._Find_next(hh)) {
            int d = fdist(s, hh); if (d == 0) d = n;
            if (d <= lim) return false;
        }
        return true;
    }
    void stallFallback() {
#ifdef SIM
        if (getenv("MTRACE")) fprintf(stderr, "stall at ops %lld open %d\n", opsCount, openTails);
#endif
        // pick unfixed head with fewest candidates
        int bh = -1, bc = INT_MAX;
        for (int h = 0; h < n; h++) if (isHead[h]) { int c = Mt[h].count(); if (c >= 2 && c < bc) { bc = c; bh = h; } }
        if (bh < 0) { if (!Q.empty()) pop(); else push(nextPushable(lastPushed, -1)); return; }
        int h = bh, x = tailOf[find(h)];
        vector<pair<int,int>> cs;
        for (int t = Mt[h]._Find_first(); t < 512; t = Mt[h]._Find_next(t)) { int d = fdist(t, h); if (d == 0) d = n; cs.push_back({d, t}); }
        sort(cs.begin(), cs.end());
        int tm = cs[(cs.size() - 1) / 2].second;
        if (cs[(cs.size() - 1) / 2].first >= n) tm = cs[0].second;
        while (!Q.empty()) pop();
        lastPushed = (tm - 1 + n) % n;
        // push from tm up to before x, then x
        int guard = 0;
        while (guard++ < 2 * n) {
            int fp = Q.empty() ? -1 : Q.front();
            int s = nextPushable(lastPushed, fp);
            if (s < 0) break;
            push(s);
            if (find(s) == find(x) || done()) break;
        }
    }

    double entPush(int x, int front) {
        int h = headOf[find(x)];
        if (!isHead[h]) return 0;
        int d;
        int fh = fdist(h, front);
        if (h != x && fh > 0 && fh <= fdist(h, x)) d = 0;
        else d = fdist(front, h);
        ensureCol(h);
        const auto &c = cumIn[h];
        const auto &cc = cntIn[h];
        double e = c[n] > 0 ? H2(c[d] / c[n]) : 0;
        if (cc[d] > 0 && cc[d] < cc[n]) e = max(e, 1e-4);
        return e;
    }
    double entPop(int y, int back) {
        int t = tailOf[find(y)];
        if (t != y || !isTail[t]) return 0;
        int d = fdist(t, back);
        ensureRow(t);
        const auto &c = cumOut[t];
        const auto &cc = cntOut[t];
        double e = c[n] > 0 ? H2(c[d] / c[n]) : 0;
        if (cc[d] > 0 && cc[d] < cc[n]) e = max(e, 1e-4);
        return e;
    }

    int decide() {
        int len = Q.size();
        if (len == 0) return 0;
        int base = lastPushed;
        int H = 2 * (int)ceil(max(1.0, kEst - closedCnt)) + 10;
        H = min(H, 120);
        H = max(H, 8);
        vector<int> popseq(Q.begin(), Q.end());
        vector<int> pushseq, prevOcc;
        unordered_map<int, int> lastIdx;
        for (int i = 0; i < len; i++) lastIdx[find(popseq[i])] = i;
        int p = base;
        for (int j = 0; j < H; j++) {
            int s = -1;
            for (int st = 1; st <= n; st++) {
                int q = (p + st) % n;
                if (isTail[q]) {
                    if (DPSKIP && j < n && SKIPON && !Q.empty() && qcnt[find(q)] == 0 && canSkip(q)) continue;
                    s = q; break;
                }
            }
            if (s < 0) break;
            int R = find(s);
            pushseq.push_back(s);
            auto it = lastIdx.find(R);
            prevOcc.push_back(it == lastIdx.end() ? -1 : it->second);
            lastIdx[R] = len + j;
            popseq.push_back(s);
            p = s;
        }
        int P = pushseq.size();
        int W = H + 1;
        static vector<double> V;
        static vector<signed char> choice;
        V.assign((size_t)W * W, 0);
        choice.assign((size_t)W * W, -1);
        auto id = [&](int i, int j) { return (size_t)i * W + j; };
        for (int hh = H - 1; hh >= 0; hh--) {
            for (int i = 0; i <= hh; i++) {
                int j = hh - i;
                if (i > len + j) continue;
                double best = -1; int bc = -1;
                if (j < P && prevOcc[j] < i) {
                    double e = 0;
                    if (i < len + j) e = entPush(pushseq[j], popseq[i]);
                    double v = e + V[id(i, j + 1)];
                    if (v > best) { best = v; bc = 0; }
                }
                if (i < len + j) {
                    double e = 0;
                    if (i + 1 < len + j) {
                        int back = j > 0 ? pushseq[j - 1] : Q.back();
                        e = entPop(popseq[i], back);
                    }
                    double v = e + V[id(i + 1, j)];
                    if (v > best + 1e-12) { best = v; bc = 1; }
                }
                if (bc < 0) best = 0;
                V[id(i, j)] = best;
                choice[id(i, j)] = bc;
            }
        }
        int c = choice[id(0, 0)];
        if (c < 0) c = 1;
        return c;
    }

    void finish() {
        groupOf.assign(n, -1);
        map<int,int> id;
        for (int i = 0; i < n; i++) {
            int r = find(i);
            if (!id.count(r)) { int s = id.size(); id[r] = s; }
            groupOf[i] = id[r];
        }
    }
};


struct Matcher {
    // children (distinct classes) matched injectively into parents (class reps)
    vector<int> ch, pa;           // element ids
    vector<BS> row, col;          // row[child] over parent idx ; col[parent] over child idx
    vector<char> mustMatch;       // parent must have a child
    vector<int> assign;           // child -> parent idx
    vector<char> rowDone, colDone;
    vector<int> work;
    int unresolved;
    deque<pair<int,int>> &Q;      // (side, idx) side 0 child 1 parent ; shared physical queue mirror
    int &cur;
    deque<int> *PE = nullptr;
    Matcher(deque<pair<int,int>> &q, int &c) : Q(q), cur(c) {}

    void removeE(int c, int p) {
        if (!row[c][p]) return;
        row[c].reset(p); col[p].reset(c);
        work.push_back(c); work.push_back(10000 + p);
    }
    void fix(int c, int p) {
        BS r = row[c]; r.reset(p);
        for (int q = r._Find_first(); q < 512; q = r._Find_next(q)) removeE(c, q);
        BS cc = col[p]; cc.reset(c);
        for (int d = cc._Find_first(); d < 512; d = cc._Find_next(d)) removeE(d, p);
        if (!rowDone[c]) { rowDone[c] = 1; assign[c] = p; unresolved--; }
        colDone[p] = 1;
    }
    void propagate() {
        while (!work.empty()) {
            int v = work.back(); work.pop_back();
            if (v >= 10000) {
                int p = v - 10000;
                if (colDone[p]) continue;
                int cnt = col[p].count();
                if (cnt == 0) { if (mustMatch[p]) { fprintf(stderr, "MCONTRA1\n"); exit(3); } colDone[p] = 1; }
                else if (cnt == 1 && mustMatch[p]) fix(col[p]._Find_first(), p);
            } else {
                int c = v;
                if (rowDone[c]) continue;
                int cnt = row[c].count();
                if (cnt == 0) { fprintf(stderr, "MCONTRA2\n"); exit(3); }
                if (cnt == 1) fix(c, row[c]._Find_first());
            }
        }
    }
    // physical ops
    void pushE(int side, int idx) {
        // window content before push
        BS other; // indices of opposite side in window
        for (auto &pr : Q) if (pr.first == 1 - side) other.set(pr.second);
        int e = side == 0 ? ch[idx] : pa[idx];
        int c = doOp('+', e);
        bool in = (c == cur);
        cur = c;
        Q.push_back({side, idx});
        if (PE) PE->push_back(e);
        applyTest(side, idx, other, in);
    }
    void popE() {
        auto pr = Q.front();
        Q.pop_front();
        if (PE) PE->pop_front();
        int c = doOp('-', 0);
        bool in = (c == cur);
        cur = c;
        if (pr.first == 2) return;
        // if same class still in window via duplicate entry, test is trivial
        for (auto &q : Q) if (q.first == pr.first && q.second == pr.second) return;
        BS other;
        for (auto &q : Q) if (q.first == 1 - pr.first) other.set(q.second);
        applyTest(pr.first, pr.second, other, in);
    }
    void applyTest(int side, int idx, const BS &other, bool in) {
        if (side == 0) {
            int c = idx;
            if (rowDone[c]) return;
            if (in) { BS r = row[c] & ~other; for (int p = r._Find_first(); p < 512; p = r._Find_next(p)) removeE(c, p); }
            else { BS r = row[c] & other; for (int p = r._Find_first(); p < 512; p = r._Find_next(p)) removeE(c, p); }
        } else {
            int p = idx;
            if (colDone[p]) return;
            if (in) { mustMatch[p] = 1; BS r = col[p] & ~other; for (int c = r._Find_first(); c < 512; c = r._Find_next(c)) removeE(c, p); work.push_back(10000 + p); }
            else { BS r = col[p] & other; for (int c = r._Find_first(); c < 512; c = r._Find_next(c)) removeE(c, p); }
        }
        propagate();
    }
    vector<vector<double>> A;
    void sinkhorn() {
        int nc = ch.size(), np = pa.size();
        A.assign(nc, vector<double>(np, 0.0));
        for (int c = 0; c < nc; c++) for (int p = row[c]._Find_first(); p < 512; p = row[c]._Find_next(p)) A[c][p] = 1;
        vector<double> cs(np);
        for (int it = 0; it < 60; it++) {
            for (int c = 0; c < nc; c++) {
                double s = 0; for (int p = 0; p < np; p++) s += A[c][p];
                if (s > 0) for (int p = 0; p < np; p++) A[c][p] /= s;
            }
            fill(cs.begin(), cs.end(), 0.0);
            for (int c = 0; c < nc; c++) for (int p = 0; p < np; p++) cs[p] += A[c][p];
            for (int p = 0; p < np; p++) {
                if (cs[p] <= 0) continue;
                double target = mustMatch[p] ? 1.0 : min(1.0, cs[p]);
                double f = target / cs[p];
                if (f != 1.0) for (int c = 0; c < nc; c++) A[c][p] *= f;
            }
        }
        for (int c = 0; c < nc; c++) {
            double s = 0; for (int p = 0; p < np; p++) s += A[c][p];
            if (s > 0) for (int p = 0; p < np; p++) A[c][p] /= s;
        }
    }
    bool useSink = true;
    double probIn(int side, int idx, const vector<int> &suffixSet /*unused*/, const BS &S) {
        if (useSink && !A.empty()) {
            double r = 0;
            if (side == 0) { for (int p = S._Find_first(); p < 512; p = S._Find_next(p)) r += A[idx][p]; }
            else { for (int c = S._Find_first(); c < 512; c = S._Find_next(c)) r += A[c][idx]; }
            return min(1.0, max(0.0, r));
        }
        if (side == 0) {
            int n = row[idx].count();
            if (n == 0) return 0;
            return (double)(row[idx] & S).count() / n;
        } else {
            // expected matched mass from children in S
            double tot = 0, inS = 0;
            for (int c = col[idx]._Find_first(); c < 512; c = col[idx]._Find_next(c)) {
                double w = 1.0 / max<size_t>(1, row[c].count());
                tot += w;
                if (S[c]) inS += w;
            }
            double pm = mustMatch[idx] ? 1.0 : min(1.0, tot);
            if (tot <= 0) return 0;
            return pm * inS / tot;
        }
    }
    void run() {
        int nc = ch.size(), np = pa.size();
        assign.assign(nc, -1);
        rowDone.assign(nc, 0); colDone.assign(np, 0);
        unresolved = nc;
        for (int c = 0; c < nc; c++) work.push_back(c);
        for (int p = 0; p < np; p++) work.push_back(10000 + p);
        propagate();
        if (unresolved == 0) return;
        int curSide = -1;
        for (auto &pr : Q) if (pr.first == 1) curSide = 1;
        if (curSide < 0) {
            while (!Q.empty()) popE();
            vector<int> ac, ap;
            for (int c = 0; c < nc; c++) if (!rowDone[c]) ac.push_back(c);
            BS up; for (int c : ac) up |= row[c];
            for (int p = up._Find_first(); p < 512; p = up._Find_next(p)) ap.push_back(p);
            if (ac.size() <= ap.size()) { for (int c : ac) pushE(0, c); curSide = 0; }
            else { for (int p : ap) pushE(1, p); curSide = 1; }
        }
        int guard = 0;
        while (unresolved > 0) {
            if (++guard > 200) { fprintf(stderr, "MGUARD\n"); exit(5); }
            int ySide = 1 - curSide;
            sinkhorn();
            // X order in window
            vector<pair<int,int>> xs(Q.begin(), Q.end());
            int X = xs.size();
            // candidates to push
            vector<int> ys;
            if (ySide == 0) { for (int c = 0; c < nc; c++) if (!rowDone[c]) ys.push_back(c); }
            else {
                BS up; for (int c = 0; c < nc; c++) if (!rowDone[c]) up |= row[c];
                for (int p = up._Find_first(); p < 512; p = up._Find_next(p)) if (!colDone[p]) ys.push_back(p);
            }
            vector<pair<int,int>> plan;  // (threshold, y)
            vector<BS> suf(X + 1);
            for (int i = X - 1; i >= 0; i--) { suf[i] = suf[i + 1]; if (xs[i].first == curSide) suf[i].set(xs[i].second); }
            for (int y : ys) {
                double bestD = 2; int bestI = -1;
                for (int i = 0; i <= X; i++) {
                    if (i > 0 && suf[i] == suf[i - 1]) continue;
                    double p = probIn(ySide, y, {}, suf[i]);
                    if (fabs(p - 0.5) < bestD - 1e-12) { bestD = fabs(p - 0.5); bestI = i; }
                }
                if (bestD < 0.5 - 1e-9) plan.push_back({bestI, y});
            }
            if (plan.empty()) {
                // nothing informative from this side; flush and switch
                while (!Q.empty()) popE();
                if (unresolved == 0) break;
                // push other side fully as new fill
                vector<int> ac;
                if (ySide == 0) { for (int c = 0; c < nc; c++) if (!rowDone[c]) ac.push_back(c); }
                else { BS up; for (int c = 0; c < nc; c++) if (!rowDone[c]) up |= row[c]; for (int p = up._Find_first(); p < 512; p = up._Find_next(p)) ac.push_back(p); }
                for (int v : ac) pushE(ySide, v);
                curSide = ySide;
                continue;
            }
            stable_sort(plan.begin(), plan.end());
            int popped = 0;
            for (auto &pl : plan) {
                while (popped < pl.first && unresolved > 0) { popE(); popped++; }
                if (unresolved == 0) break;
                pushE(ySide, pl.second);
            }
            if (unresolved == 0) break;
            while (popped < X && unresolved > 0) { popE(); popped++; }
            if (unresolved == 0) break;
            curSide = ySide;
        }
    }
};


struct PeelSolver {
    deque<pair<int,int>> mq;
    deque<int> physE;
    int cur = 0;
    vector<int> cls;
    int k = 0;
    // prefix handover
    int T = 0;
    vector<char> prefNew;
    int prefCur = 0;
    void flush() { while (!mq.empty()) { mq.pop_front(); physE.pop_front(); cur = doOp('-', 0); } }
    void run() {
        cls.assign(N, -1);
        vector<int> S(N);
        iota(S.begin(), S.end(), 0);
        vector<vector<int>> Fl, Ll;
        vector<char> inF(N, 0), inL(N, 0);
        int pj = 0;
        vector<int> chainRest;
        while (!S.empty()) {
            if (pj > 0) {
                // alive estimate: F of previous peel not in L of previous peel
                int A = 0;
                for (int f : Fl[pj - 1]) if (!inL[f]) A++;
                if ((double)S.size() > PEELR * max(1, A)) { chainRest = S; break; }
            }
            vector<int> F, L;
            int startIdx = 0;
            if (pj == 0) {
                cur = prefCur;
                for (int x = 0; x < T; x++) if (prefNew[x]) { F.push_back(x); inF[x] = 1; }
                startIdx = T;
            } else cur = 0;
            for (size_t i = startIdx; i < S.size(); i++) { int x = S[i]; int c = doOp('+', x); if (c > cur) { F.push_back(x); inF[x] = 1; } cur = c; }
            for (int x : S) { int c = doOp('-', 0); if (c < cur) { L.push_back(x); inL[x] = 1; } cur = c; (void)x; }
            vector<int> S2;
            for (int x : S) if (!inF[x] && !inL[x]) S2.push_back(x);
            Fl.push_back(F); Ll.push_back(L);
            S = S2;
            pj++;
        }
        cur = 0;
#ifdef SIM
        if (getenv("MTRACE")) fprintf(stderr, "peels %d ops %lld\n", pj, opsCount);
#endif
        k = Fl[0].size();
        vector<int> rep(k), lo(k), hi(k);
        vector<char> alive(k, 1);
        for (int c = 0; c < k; c++) { rep[c] = Fl[0][c]; cls[rep[c]] = c; lo[c] = rep[c]; hi[c] = -1; }
        for (int j = 0; j < pj; j++) {
            if (j > 0) {
                vector<int> children = Fl[j];
                vector<int> parents;
                for (int c = 0; c < k; c++) if (alive[c]) parents.push_back(c);
                runMerge(children, children, parents, rep, lo, hi, false);
                vector<char> got(k, 0);
                for (int x : children) { got[cls[x]] = 1; lo[cls[x]] = x; }
                for (int c = 0; c < k; c++) if (!got[c]) alive[c] = 0;
            }
            set<int> Fset(Fl[j].begin(), Fl[j].end()), Lset(Ll[j].begin(), Ll[j].end());
            vector<int> children;
            for (int x : Ll[j]) if (!Fset.count(x)) children.push_back(x);
            vector<int> parents;
            for (int f : Fl[j]) if (!Lset.count(f)) parents.push_back(cls[f]);
            for (int f : Fl[j]) if (Lset.count(f)) alive[cls[f]] = 0;
            if (!children.empty()) runMerge(children, children, parents, rep, lo, hi, true);
            for (int x : children) hi[cls[x]] = x;
        }
        if (!chainRest.empty()) {
            flush();
            int A = 0;
            for (int c = 0; c < k; c++) if (alive[c]) A++;
            ChainSolver cs;
            cs.n = chainRest.size();
            cs.el = chainRest;
            cs.kHint = max(1, A);
            cs.run();
            cs.flushQ();
            int G = 0;
            for (int g : cs.groupOf) G = max(G, g + 1);
            vector<int> mn(G, INT_MAX), mx(G, -1), repG(G, -1);
            for (int i = 0; i < cs.n; i++) {
                int g = cs.groupOf[i], x = chainRest[i];
                mn[g] = min(mn[g], x); mx[g] = max(mx[g], x);
                if (repG[g] < 0) repG[g] = x;
            }
            vector<int> parents;
            for (int c = 0; c < k; c++) if (alive[c]) parents.push_back(c);
            // children reps with span info: use mn/mx for constraints
            vector<int> lowKey(G), highKey(G);
            for (int g = 0; g < G; g++) { lowKey[g] = mn[g]; highKey[g] = mx[g]; }
            runMergeSpan(repG, lowKey, highKey, parents, rep, lo, hi);
            for (int i = 0; i < cs.n; i++) cls[chainRest[i]] = cls[repG[cs.groupOf[i]]];
        }
        vector<vector<int>> out(k);
        for (int x = 0; x < N; x++) out[cls[x]].push_back(x);
        reportAnswer(out);
    }
    void runMerge(const vector<int> &children, const vector<int> &keys, const vector<int> &parents, vector<int> &rep, vector<int> &lo, vector<int> &hi, bool perfect) {
        runMergeGen(children, keys, keys, parents, rep, lo, hi, perfect);
    }
    void runMergeSpan(const vector<int> &children, const vector<int> &lk, const vector<int> &hk, const vector<int> &parents, vector<int> &rep, vector<int> &lo, vector<int> &hi) {
        runMergeGen(children, lk, hk, parents, rep, lo, hi, false);
    }
    void runMergeGen(const vector<int> &children, const vector<int> &lk, const vector<int> &hk, const vector<int> &parents, vector<int> &rep, vector<int> &lo, vector<int> &hi, bool perfect) {
        if (children.empty()) return;
        int nc = children.size(), np = parents.size();
        if (np == 1 && !perfect) {
            for (int i = 0; i < nc; i++) cls[children[i]] = parents[0];
            return;
        }
        {
            map<int,int> pidx;
            for (int i = 0; i < np; i++) pidx[parents[i]] = i;
            mq.clear();
            for (int e : physE) {
                int c = cls[e];
                auto it = pidx.find(c);
                if (c >= 0 && it != pidx.end()) mq.push_back({1, it->second}); else mq.push_back({2, -1});
            }
        }
        Matcher m(mq, cur);
        m.PE = &physE;
        m.ch = children;
        for (int c : parents) m.pa.push_back(rep[c]);
        m.row.assign(nc, BS()); m.col.assign(np, BS());
        m.mustMatch.assign(np, perfect ? 1 : 0);
        for (int i = 0; i < nc; i++) {
            for (int pIdx = 0; pIdx < np; pIdx++) {
                int c = parents[pIdx];
                if (lo[c] < lk[i] && (hi[c] < 0 || hk[i] < hi[c])) { m.row[i].set(pIdx); m.col[pIdx].set(i); }
            }
        }
        long long b0 = opsCount;
        m.run();
#ifdef SIM
        if (getenv("MTRACE")) fprintf(stderr, "merge nc %d np %d perfect %d ops %lld\n", nc, np, (int)perfect, opsCount - b0);
#endif
        for (int i = 0; i < nc; i++) cls[children[i]] = parents[m.assign[i]];
    }
};

static void solveAll() {
    if (N == 1) { reportAnswer({{0}}); return; }
    ChainSolver cs;
    cs.n = N;
    cs.el.resize(N);
    iota(cs.el.begin(), cs.el.end(), 0);
    cs.decideMode = true;
    int st = cs.run();
    if (st == 0) {
        int G = 0;
        for (int g : cs.groupOf) G = max(G, g + 1);
        vector<vector<int>> out(G);
        for (int i = 0; i < N; i++) out[cs.groupOf[i]].push_back(i);
        reportAnswer(out);
        return;
    }
    PeelSolver ps;
    ps.T = cs.prefNew.size();
    ps.prefNew = cs.prefNew;
    ps.prefCur = cs.cur;
    ps.run();
}
#ifdef SIM
int main(int argc, char **argv) {
    N = atoi(argv[1]);
    int type = atoi(argv[2]);
    double param = atof(argv[3]);
    unsigned seed = atoi(argv[4]);
    mt19937 rng(seed);
    simLab.assign(N, 0);
    int k = max(1, (int)param);
    if (type == 0) {
        for (int i = 0; i < N; i++) simLab[i] = rng() % k;
    } else if (type == 1) {
        for (int i = 0; i < N; i++) simLab[i] = i % k;
    } else if (type == 2) {
        for (int i = 0; i < N; i++) simLab[i] = (long long)i * k / N;
    } else if (type == 3) {
        vector<double> w(k);
        for (int i = 0; i < k; i++) w[i] = 1.0 / (i + 1);
        discrete_distribution<int> dd(w.begin(), w.end());
        for (int i = 0; i < N; i++) simLab[i] = dd(rng);
    } else if (type == 4) {
        for (int i = 0; i < N; i++) simLab[i] = (rng() % 100 < (unsigned)k) ? 0 : i + 1;
    } else if (type == 5) {
        vector<int> perm(N);
        iota(perm.begin(), perm.end(), 0);
        shuffle(perm.begin(), perm.end(), rng);
        for (int i = 0; i < N; i++) simLab[perm[i]] = i / k;
    }
    simCnt.assign(*max_element(simLab.begin(), simLab.end()) + 2, 0);
    if (getenv("KTH")) KTH = atof(getenv("KTH"));
    if (getenv("PEELR")) PEELR = atof(getenv("PEELR"));
    if (getenv("DECX")) DECX = atof(getenv("DECX"));
    if (getenv("SKIPON")) SKIPON = atoi(getenv("SKIPON"));
    if (getenv("DPSKIP")) DPSKIP = atoi(getenv("DPSKIP"));
    if (getenv("SKIPM")) SKIPM = atof(getenv("SKIPM"));
    if (getenv("SKIPC")) SKIPC = atoi(getenv("SKIPC"));
    if (getenv("SKIPTAU")) SKIPTAU = atof(getenv("SKIPTAU"));
    if (getenv("KSMALL")) KSMALL = atof(getenv("KSMALL"));
    if (getenv("GROWTH")) GROWTH = atof(getenv("GROWTH"));
    solveAll();
    map<int, int> sz;
    for (int v : simLab) sz[v]++;
    double lb = lgamma(N + 1);
    map<int, int> mult;
    for (auto &p : sz) { lb -= lgamma(p.second + 1); mult[p.second]++; }
    for (auto &p : mult) lb -= lgamma(p.second + 1);
    lb /= log(2);
    printf("%lld %.0f %d\n", opsCount, lb, (int)sz.size());
    if (getenv("MTRACE")) fprintf(stderr, "trivP %lld trivQ %lld predP %lld predQ %lld\n", trivP, trivQ, predP, predQ);
    return 0;
}
#endif
#ifndef SIM
int main() {
    if (scanf("%d", &N) != 1) return 0;
    solveAll();
    return 0;
}
#endif

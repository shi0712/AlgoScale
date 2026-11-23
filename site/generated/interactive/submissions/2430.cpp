#include <bits/stdc++.h>
using namespace std;

static int n, BB, R;
static vector<int> pv, bv;
static int kUnknown;
struct QR { int i, j, r; };
static vector<QR> qlog;
static mt19937 rng(12345);

#ifdef SIM
static vector<int> HP, HB;
static long long simQueries = 0;
static vector<int> outPerm; static long long outCount;
#endif

static int ask(int i, int j) {
#ifdef SIM
    int r = HP[i] ^ HB[j];
    simQueries++;
#else
    printf("? %d %d\n", i, j);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
#endif
    qlog.push_back({i, j, r});
    return r;
}

static void finish(long long cnt, const vector<int>& p) {
#ifdef SIM
    outCount = cnt; outPerm = p;
#else
    printf("!\n%lld\n", cnt);
    for (int i = 0; i < n; i++) printf("%d%c", p[i], i + 1 < n ? ' ' : '\n');
    fflush(stdout);
#endif
}

static void setP(int a, int y) {
    if (pv[a] == -1) { pv[a] = y; bv[y] = a; kUnknown--; }
}

// group of w (w < lowbit(n)) s.t. p^w equivalent to p
static vector<int> groupG(const vector<int>& p) {
    int L = n & (-n);
    vector<int> b(n);
    for (int i = 0; i < n; i++) b[p[i]] = i;
    vector<int> G;
    for (int w = 0; w < L; w++) {
        bool ok = true;
        for (int j = 0; j < n && ok; j++) if (b[j ^ w] != (b[j] ^ w)) ok = false;
        if (ok) G.push_back(w);
    }
    return G;
}

// ---------------- explicit candidate stage ----------------
struct Explicit {
    vector<int> pos, val;
    vector<int> posIdx, valIdx;
    int m = 0;                 // = pos.size() = val.size()
    size_t C = 0;
    vector<uint16_t> cp;       // cp[c*m+k] value at pos[k]
    vector<uint16_t> cb;       // cb[c*m+l] position of val[l]
    vector<int> tag;           // optional per-candidate tag (column mode: t)
    int P(size_t c, int i) const { int k = posIdx[i]; return k >= 0 ? cp[c * m + k] : pv[i]; }
    int Bv(size_t c, int j) const { int l = valIdx[j]; return l >= 0 ? cb[c * m + l] : bv[j]; }
};

static void addCand(Explicit& E, const vector<int>& vals, int tg = 0) {
    for (int k = 0; k < E.m; k++) E.cp.push_back((uint16_t)vals[k]);
    size_t base = E.cb.size();
    E.cb.resize(base + E.m);
    for (int k = 0; k < E.m; k++) E.cb[base + E.valIdx[vals[k]]] = (uint16_t)E.pos[k];
    E.tag.push_back(tg);
    E.C++;
}

static void initExplicit(Explicit& E, const vector<int>& pos, const vector<int>& val) {
    E.pos = pos; E.val = val; E.m = pos.size(); E.C = 0;
    E.cp.clear(); E.cb.clear(); E.tag.clear();
    E.posIdx.assign(n, -1); E.valIdx.assign(n, -1);
    for (size_t k = 0; k < pos.size(); k++) E.posIdx[pos[k]] = k;
    for (size_t l = 0; l < val.size(); l++) E.valIdx[val[l]] = l;
}

static vector<int> relevantQueries(const Explicit& E, size_t fromQ) {
    vector<int> rel;
    for (size_t q = fromQ; q < qlog.size(); q++)
        if (E.posIdx[qlog[q].i] >= 0 || E.valIdx[qlog[q].j] >= 0) rel.push_back(q);
    return rel;
}

static void filterExplicit(Explicit& E, size_t fromQ) {
    vector<int> rel = relevantQueries(E, fromQ);
    if (rel.empty()) return;
    size_t w = 0;
    int m = E.m;
    for (size_t c = 0; c < E.C; c++) {
        bool ok = true;
        for (int q : rel) {
            if ((E.P(c, qlog[q].i) ^ E.Bv(c, qlog[q].j)) != qlog[q].r) { ok = false; break; }
        }
        if (ok) {
            if (w != c) {
                memmove(&E.cp[w * m], &E.cp[c * m], m * sizeof(uint16_t));
                memmove(&E.cb[w * m], &E.cb[c * m], m * sizeof(uint16_t));
                E.tag[w] = E.tag[c];
            }
            w++;
        }
    }
    E.C = w; E.cp.resize(w * m); E.cb.resize(w * m); E.tag.resize(w);
}

static vector<int> fullPerm(const Explicit& E, size_t c) {
    vector<int> p(n);
    for (int i = 0; i < n; i++) p[i] = E.P(c, i);
    return p;
}

static vector<int> cntBuf;
static long long scorePair(const Explicit& E, int i, int j, vector<int>& touched) {
    long long s = 0;
    touched.clear();
    for (size_t c = 0; c < E.C; c++) {
        int a = E.P(c, i) ^ E.Bv(c, j);
        int& x = cntBuf[a];
        if (x == 0) touched.push_back(a);
        s += 2 * x + 1;
        x++;
    }
    for (int a : touched) cntBuf[a] = 0;
    return s;
}

// fast Walsh-Hadamard (xor convolution helper)
static void fwht(vector<long long>& a) {
    size_t N = a.size();
    for (size_t len = 1; len < N; len <<= 1)
        for (size_t i = 0; i < N; i += len << 1)
            for (size_t k = i; k < i + len; k++) {
                long long u = a[k], v = a[k + len];
                a[k] = u + v; a[k + len] = u - v;
            }
}

// colR: column answers r_a (or -1) when in column mode (candidates tagged by t)
static void runExplicit(Explicit& E, const vector<int>* colR = nullptr) {
    cntBuf.assign(R, 0);
    vector<int> touched;
    while (true) {
        if (E.C == 0) {
            // should not happen
            vector<int> p(n); iota(p.begin(), p.end(), 0); finish(1, p); return;
        }
        vector<int> p0 = fullPerm(E, 0);
        vector<int> G = groupG(p0);
        vector<char> inG(n & (-n), 0);
        for (int w : G) inG[w] = 1;
        bool oneClass = true;
        for (size_t c = 1; c < E.C && oneClass; c++) {
            int i0 = E.pos.empty() ? 0 : E.pos[0];
            int w = E.P(c, i0) ^ p0[i0];
            if (w >= (int)inG.size() || !inG[w]) { oneClass = false; break; }
            for (int k = 0; k < E.m; k++)
                if (E.cp[c * E.m + k] != (p0[E.pos[k]] ^ w)) { oneClass = false; break; }
            if (oneClass)
                for (int i = 0; i < n; i++)
                    if (E.posIdx[i] < 0 && (pv[i] ^ w) != pv[i]) { oneClass = false; break; }
        }
        if (oneClass) { finish((long long)G.size(), p0); return; }
        // choose query
        size_t C = E.C;
        vector<int> I = E.pos, J = E.val;
        for (int i = 0; i < n; i++) if (E.posIdx[i] < 0) { I.push_back(i); break; }
        for (int j = 0; j < n; j++) if (E.valIdx[j] < 0) { J.push_back(j); break; }
        long long budget = max<long long>(1, 40000000LL / (long long)C);
        budget = min<long long>(budget, 40000);
        vector<pair<int, int>> pairs;
        if (colR && C > 64) {
            // column family: answer(i0,j) depends on t only through f(j^t), f(u)=u^idx[u]
            const vector<int>& rr = *colR;
            vector<int> idx(R, -1);
            for (int a = 0; a < n; a++) if (rr[a] >= 0) idx[rr[a]] = a;
            vector<int> fcnt(R, 0);
            vector<int> f(R, -1);
            for (int u = 0; u < R; u++) if (idx[u] >= 0) { f[u] = u ^ idx[u]; fcnt[f[u]]++; }
            int mode = max_element(fcnt.begin(), fcnt.end()) - fcnt.begin();
            vector<long long> A(R, 0), Bs(R, 0);
            for (size_t c = 0; c < C; c++) A[E.tag[c]] = 1;
            for (int u = 0; u < R; u++) if (f[u] != mode) Bs[u] = 1;  // includes unknown (missing) u
            fwht(A); fwht(Bs);
            for (int u = 0; u < R; u++) A[u] *= Bs[u];
            fwht(A);
            vector<pair<long long, int>> sc;
            for (int j = 0; j < n; j++) sc.push_back({A[j] / R, j});
            sort(sc.rbegin(), sc.rend());
            for (size_t q = 0; q < sc.size() && (long long)pairs.size() < min<long long>(budget, 64); q++)
                pairs.push_back({I[0], sc[q].second});
        } else if ((long long)I.size() * J.size() <= budget) {
            for (int i : I) for (int j : J) pairs.push_back({i, j});
        } else {
            for (int j : J) if ((long long)pairs.size() < budget / 2) pairs.push_back({I[0], j});
            for (int i : I) if ((long long)pairs.size() < budget * 3 / 4) pairs.push_back({i, J[0]});
            while ((long long)pairs.size() < budget) pairs.push_back({I[rng() % I.size()], J[rng() % J.size()]});
        }
        long long best = LLONG_MAX; pair<int, int> bq = pairs[0];
        for (auto [i, j] : pairs) {
            long long s = scorePair(E, i, j, touched);
            if (s < best) { best = s; bq = {i, j}; }
        }
        size_t before = qlog.size();
        ask(bq.first, bq.second);
        filterExplicit(E, before);
    }
}

// ---------------- components (two-star) ----------------
static vector<int> posOff, valOff, rIdx, sIdx;
static vector<char> busyP, busyB;  // node belongs to active or pending component

struct Comp {
    int x, a0, ra0;
    vector<int> T;
    vector<int> cnt;              // passed applicable cross checks (anchor stuck detection)
    vector<pair<int, int>> Pn;    // (a, r): p_a = r ^ t
    vector<pair<int, int>> Bn;    // (j, s): b_j = s ^ t   (center x has s = 0)
};
static vector<Comp> pend;
static bool trackCnt = false;

static inline int checkP(int a, int ra, int t) {
    // returns 0 = fail, 1 = pass, 2 = pass with applicable cross check
    int y = ra ^ t;
    if (y >= n || bv[y] != -1) return 0;
    int res = 1;
    if (valOff[y] != -1) { if ((valOff[y] ^ t) != a) return 0; res = 2; }
    int j = sIdx[a ^ t];
    if (j != -1) { if (j != y) return 0; res = 2; }
    return res;
}
static inline int checkB(int j, int s, int t) {
    int z = s ^ t;
    if (z >= n || pv[z] != -1) return 0;
    int res = 1;
    if (posOff[z] != -1) { if ((posOff[z] ^ t) != j) return 0; res = 2; }
    int a = rIdx[j ^ t];
    if (a != -1) { if (a != z) return 0; res = 2; }
    return res;
}

static void addP(Comp& C, int a, int r) {
    posOff[a] = r; rIdx[r] = a; C.Pn.push_back({a, r}); busyP[a] = 1;
    size_t w = 0;
    for (size_t q = 0; q < C.T.size(); q++) {
        int c = checkP(a, r, C.T[q]);
        if (c) { C.T[w] = C.T[q]; if (trackCnt) C.cnt[w] = C.cnt[q] + (c == 2); w++; }
    }
    C.T.resize(w); if (trackCnt) C.cnt.resize(w);
}
static void addB(Comp& C, int j, int s) {
    valOff[j] = s; sIdx[s] = j; C.Bn.push_back({j, s}); busyB[j] = 1;
    size_t w = 0;
    for (size_t q = 0; q < C.T.size(); q++) {
        int c = checkB(j, s, C.T[q]);
        if (c) { C.T[w] = C.T[q]; if (trackCnt) C.cnt[w] = C.cnt[q] + (c == 2); w++; }
    }
    C.T.resize(w); if (trackCnt) C.cnt.resize(w);
}
static void unloadComp(Comp& C) {
    for (auto [a, r] : C.Pn) { posOff[a] = -1; rIdx[r] = -1; }
    for (auto [j, s] : C.Bn) { valOff[j] = -1; sIdx[s] = -1; }
}
static void loadComp(Comp& C) {
    for (auto [a, r] : C.Pn) { posOff[a] = r; rIdx[r] = a; }
    for (auto [j, s] : C.Bn) { valOff[j] = s; sIdx[s] = j; }
}
static void assignComp(Comp& C, int t) {
    for (auto [a, r] : C.Pn) { busyP[a] = 0; setP(a, r ^ t); }
    for (auto [j, s] : C.Bn) { busyB[j] = 0; setP(s ^ t, j); }
}
// unary refilter of a pending component against resolved knowledge
static void refilter(Comp& C) {
    size_t w = 0;
    for (size_t q = 0; q < C.T.size(); q++) {
        int t = C.T[q];
        bool ok = true;
        for (auto [a, r] : C.Pn) {
            int y = r ^ t;
            if (pv[a] != -1) { if (pv[a] != y) { ok = false; break; } }
            else if (bv[y] != -1) { ok = false; break; }
        }
        if (ok) for (auto [j, s] : C.Bn) {
            int z = s ^ t;
            if (bv[j] != -1) { if (bv[j] != z) { ok = false; break; } }
            else if (pv[z] != -1) { ok = false; break; }
        }
        if (ok) C.T[w++] = t;
    }
    C.T.resize(w);
}
static void cascade() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t i = 0; i < pend.size(); i++) {
            refilter(pend[i]);
            if (pend[i].T.size() == 1) {
                Comp C = std::move(pend[i]);
                pend.erase(pend.begin() + i);
                assignComp(C, C.T[0]);
                changed = true;
                break;
            }
        }
    }
}

// fresh element pools
static int freshPosCount() { int c = 0; for (int i = 0; i < n; i++) if (pv[i] == -1 && !busyP[i]) c++; return c; }
static vector<int> ordP, ordV;
static int nextFreshPosIdx(int from) { for (int k = from; k < n; k++) { int i = ordP[k]; if (pv[i] == -1 && !busyP[i]) return k; } return -1; }
static int nextFreshValIdx(int from) { for (int k = from; k < n; k++) { int j = ordV[k]; if (bv[j] == -1 && !busyB[j]) return k; } return -1; }
static int nextFreshPos(int from) { int k = nextFreshPosIdx(from); return k < 0 ? -1 : ordP[k]; }
static int nextFreshVal(int from) { int k = nextFreshValIdx(from); return k < 0 ? -1 : ordV[k]; }

static double CSTOP_K = 0.0;
static int PONLY = 0;
static int RANDORD = 1;
static int STRONGC = 0;  // lazy stop parameter
static int cstopFor(int k) {
    (void)k;
    return (int)CSTOP_K;
}

// grow active component C (loaded) until |T|<=stopAt or exhausted. returns true if |T|<=stopAt
static bool growComp(Comp& C, size_t stopAt, bool anchor, int L) {
    int fp = 0, fv = 0;
    bool turnP = true;
    while (C.T.size() > stopAt) {
        if (anchor && trackCnt && C.T.size() >= 2) {
            // stuck detection: all in one coset of [0,L) and >=2 candidates with >=3 passed checks
            (void)L;
            {
                int strong = 0;
                for (int c : C.cnt) if (c >= (STRONGC ? STRONGC : (n >= 256 ? 2 : 3))) strong++;
                if (strong >= 2) return false;
            }
            int sz = (int)(C.Pn.size() + C.Bn.size()) - 1;
            double sq = sqrt((double)n);
            if (sz >= 3 * sq && (int)C.T.size() * 4 > n) return false;
            if (sz >= 8 * sq + 10) return false;
        }
#ifdef SIM
        if (anchor && getenv("TS")) { int sq=(int)sqrt((double)n); int sz=C.Pn.size()+C.Bn.size()-1; if (sz % sq == 0) fprintf(stderr, "%d %zu\n", sz/sq, C.T.size()); }
#endif
        int ka = nextFreshPosIdx(fp), kj = nextFreshValIdx(fv);
        int a = ka < 0 ? -1 : ordP[ka], j = kj < 0 ? -1 : ordV[kj];
        if (a < 0 && j < 0) return false;
        bool doP = (turnP && a >= 0) || j < 0;
        if (!anchor && PONLY && a >= 0) doP = true;
        turnP = !turnP;
        if (doP) {
            fp = ka + 1;
            int r = ask(a, C.x);
            addP(C, a, r);
        } else {
            fv = kj + 1;
            int q = ask(C.a0, j);
            addB(C, j, q ^ C.ra0);
        }
    }
    return true;
}

static Comp startComp() {
    Comp C;
    C.x = nextFreshVal(0); C.a0 = nextFreshPos(0);
    valOff[C.x] = 0; sIdx[0] = C.x; C.Bn.push_back({C.x, 0}); busyB[C.x] = 1;
    for (int t = 0; t < n; t++) if (pv[t] == -1) C.T.push_back(t);
    if (trackCnt) C.cnt.assign(C.T.size(), 0);
    C.ra0 = ask(C.a0, C.x);
    addP(C, C.a0, C.ra0);
    return C;
}

// resolve an active (loaded) component that could not shrink: link query to a known value
static void linkResolve(Comp& C) {
    int j = -1;
    for (int v = 0; v < n; v++) if (bv[v] != -1) { j = v; break; }
    int r = ask(C.a0, j);
    int pa = r ^ bv[j];
    vector<int> T2;
    for (int t : C.T) if ((C.ra0 ^ t) == pa) T2.push_back(t);
    C.T = T2;
}

static void columnFallback(Comp& C) {
    // complete column of center x over all positions except one, then explicit candidates
    trackCnt = false;
    int x = C.x;
    vector<char> inP(n, 0);
    for (auto [a, r] : C.Pn) inP[a] = 1;
    vector<int> rest;
    for (int i = 0; i < n; i++) if (!inP[i]) rest.push_back(i);
    for (size_t k = 0; k + 1 < rest.size(); k++) {
        int r = ask(rest[k], x);
        addP(C, rest[k], r);
    }
    vector<int> all(n); iota(all.begin(), all.end(), 0);
    Explicit E; initExplicit(E, all, all);
    unloadComp(C);
    vector<int> rr(n, -1);
    for (auto [a, r] : C.Pn) rr[a] = r;
    for (int t : C.T) {
        vector<int> vals(n, -1); vector<char> used(n, 0);
        bool ok = true; int miss = -1;
        for (int i = 0; i < n; i++) {
            if (rr[i] < 0) { miss = i; continue; }
            int y = rr[i] ^ t;
            if (y >= n || used[y]) { ok = false; break; }
            vals[i] = y; used[y] = 1;
        }
        if (!ok) continue;
        if (miss >= 0) { for (int y = 0; y < n; y++) if (!used[y]) { vals[miss] = y; break; } }
        addCand(E, vals, t);
    }
    filterExplicit(E, 0);
#ifdef SIM
    if (getenv("DBG")) fprintf(stderr, "column done q=%zu cands=%zu\n", qlog.size(), E.cp.size());
#endif
    runExplicit(E, &rr);
}

static int KE = 7;
static int SMALLN = 10;

static void endgame() {
    vector<int> upos, uval;
    for (int i = 0; i < n; i++) if (pv[i] == -1) upos.push_back(i);
    for (int j = 0; j < n; j++) if (bv[j] == -1) uval.push_back(j);
    Explicit E;
    initExplicit(E, upos, uval);
    if (upos.size() >= 9 && relevantQueries(E, 0).empty()) ask(upos[0], uval[0]);
    vector<int> rel = relevantQueries(E, 0);
    int m = upos.size();
    vector<int> perm = uval, inv(n, -1);
    do {
        for (int k = 0; k < m; k++) inv[perm[k]] = upos[k];
        bool ok = true;
        for (int q : rel) {
            int i = qlog[q].i, j = qlog[q].j;
            int a = E.posIdx[i] >= 0 ? perm[E.posIdx[i]] : pv[i];
            int b = E.valIdx[j] >= 0 ? inv[j] : bv[j];
            if ((a ^ b) != qlog[q].r) { ok = false; break; }
        }
        if (ok) addCand(E, perm);
    } while (next_permutation(perm.begin(), perm.end()));
    runExplicit(E);
}


// ---------------- mid-n solver: sampling greedy + exact enumeration ----------------
static int MRVK = 8;
struct CSP {
    // components of query graph over nodes P_i (i) and B_j (n+j); node value = off ^ t(comp)
    vector<int> par, off;          // union-find
    vector<int> compOf, offOf;     // per node: component id (-1 if singleton/unconstrained), offset
    vector<vector<pair<int,int>>> mem;  // comp -> (node, off)
    vector<vector<int>> dom;       // static domains
    vector<int> tval;              // assigned constant or -1
    vector<int> p, b;
    vector<int> trailC;            // assigned comps (for undo)
    long long nodes = 0, nodeLimit = 0;
    bool bad = false;
    int ncomp = 0;
    int findr(int x) {
        if (par[x] == x) return x;
        int r = findr(par[x]);
        off[x] ^= off[par[x]];
        par[x] = r;
        return r;
    }
    void build() {
        int N2 = 2 * n;
        par.resize(N2); off.assign(N2, 0);
        iota(par.begin(), par.end(), 0);
        vector<char> touched(N2, 0);
        for (auto& q : qlog) {
            int u = q.i, v = n + q.j;
            touched[u] = touched[v] = 1;
            int ru = findr(u), rv = findr(v);
            if (ru != rv) { par[ru] = rv; off[ru] = off[u] ^ off[v] ^ q.r; }
        }
        compOf.assign(N2, -1); offOf.assign(N2, 0);
        vector<int> idOf(N2, -1);
        mem.clear(); ncomp = 0;
        for (int x = 0; x < N2; x++) if (touched[x]) {
            int r = findr(x);
            if (idOf[r] < 0) { idOf[r] = ncomp++; mem.push_back({}); }
            compOf[x] = idOf[r]; offOf[x] = off[x];
            mem[idOf[r]].push_back({x, off[x]});
        }
        dom.assign(ncomp, {});
        for (int c = 0; c < ncomp; c++) {
            for (int t = 0; t < R; t++) {
                bool ok = true;
                for (auto [x, o] : mem[c]) {
                    int v = o ^ t;
                    if (v >= n) { ok = false; break; }
                    // internal cross: P_i = v -> B_v must be i ; B_j = v -> P_v must be j
                    if (x < n) { int y = n + v; if (compOf[y] == c && (offOf[y] ^ t) != x) { ok = false; break; } }
                    else { int y = v; if (compOf[y] == c && (offOf[y] ^ t) != x - n) { ok = false; break; } }
                }
                if (ok) dom[c].push_back(t);
            }
        }
        tval.assign(ncomp, -1);
        p.assign(n, -1); b.assign(n, -1);
        trailC.clear();
    }
    // check whether comp c can take t given current p/b
    bool canTake(int c, int t) const {
        for (auto [x, o] : mem[c]) {
            int v = o ^ t;
            if (x < n) { if (p[x] != -1 ? p[x] != v : b[v] != -1) return false; }
            else { int j = x - n; if (b[j] != -1 ? b[j] != v : p[v] != -1) return false; }
        }
        return true;
    }
    vector<int> setP, qc;  // positions set (for undo), comp queue
    // assign comp c := t with propagation; returns false on conflict
    bool assignComp(int c, int t) {
        qc.clear();
        qc.push_back(c);
        if (tval[c] != -1) return tval[c] == t;
        tval[c] = t; trailC.push_back(c);
        for (size_t h = 0; h < qc.size(); h++) {
            int cc = qc[h], tt = tval[cc];
            for (auto [x, o] : mem[cc]) {
                int v = o ^ tt;
                int pos, val;
                if (x < n) { pos = x; val = v; } else { pos = v; val = x - n; }
                if (p[pos] == val) continue;
                if (p[pos] != -1 || b[val] != -1) return false;
                p[pos] = val; b[val] = pos; setP.push_back(pos);
                // cross: node P_pos and B_val now determined
                int nodes2[2] = {pos, n + val};
                int need[2] = {val, pos};
                for (int k = 0; k < 2; k++) {
                    int y = nodes2[k], c2 = compOf[y];
                    if (c2 < 0) continue;
                    int t2 = offOf[y] ^ need[k];
                    if (tval[c2] == -1) {
                        if (!binary_search(dom[c2].begin(), dom[c2].end(), t2)) return false;
                        tval[c2] = t2; trailC.push_back(c2); qc.push_back(c2);
                    } else if (tval[c2] != t2) return false;
                }
            }
        }
        return true;
    }
    void undo(size_t mc, size_t mp) {
        while (trailC.size() > mc) { tval[trailC.back()] = -1; trailC.pop_back(); }
        while (setP.size() > mp) { int i = setP.back(); setP.pop_back(); b[p[i]] = -1; p[i] = -1; }
    }
    template <class F> bool dfs(F& cb, bool randomized) {
        if (nodeLimit && ++nodes > nodeLimit) return false;
        int best = -1; size_t bestCnt = SIZE_MAX;
        static thread_local vector<int> cand;
        int looked = 0;
        for (int c = 0; c < ncomp; c++) {
            if (tval[c] != -1) continue;
            size_t cnt = 0;
            for (int t : dom[c]) if (canTake(c, t)) { cnt++; if (cnt >= bestCnt) break; }
            if (cnt == 0) return true;
            if (cnt < bestCnt) { bestCnt = cnt; best = c; if (cnt == 1) break; }
            if (++looked >= MRVK) break;
        }
        if (best < 0) return cb();
        vector<int> vals;
        for (int t : dom[best]) if (canTake(best, t)) vals.push_back(t);
        if (randomized) shuffle(vals.begin(), vals.end(), rng);
        for (int t : vals) {
            size_t mc = trailC.size(), mp = setP.size();
            bool ok = assignComp(best, t);
            if (ok) { if (!dfs(cb, randomized)) { undo(mc, mp); return false; } }
            undo(mc, mp);
        }
        return true;
    }
    void init() {}
    void sync() { build(); }
    void undoTo(size_t) { undo(0, 0); }
};

static int MIDN = 32;
static int WANT = 0;



static long long SYS_NODES = 200000;
static long long RND_NODES = 40000;
static double SYS_LOG = 20;

struct SysResult { bool complete = false; vector<uint16_t> leaves; vector<int> lfree; };
static SysResult runSystematic(CSP& S, long long budget) {
    SysResult res;
    const size_t LCAP = 100000;
    bool overflow = false;
    auto cb = [&]() -> bool {
        if (res.lfree.size() >= LCAP) { overflow = true; return false; }
        int fr = 0;
        for (int i = 0; i < n; i++) { res.leaves.push_back(S.p[i] < 0 ? 0xFFFF : S.p[i]); if (S.p[i] < 0) fr++; }
        res.lfree.push_back(fr);
        return true;
    };
    S.nodes = 0; S.nodeLimit = budget;
    S.dfs(cb, true);
    res.complete = !overflow && S.nodes <= S.nodeLimit;
    S.undoTo(0);
    return res;
}
static vector<int> leafSample(const SysResult& R0, size_t l, bool firstCompletion = false) {
    vector<int> sp(n), fv;
    vector<char> used(n, 0);
    for (int i = 0; i < n; i++) { int v = R0.leaves[l * n + i]; if (v == 0xFFFF) sp[i] = -1; else { sp[i] = v; used[v] = 1; } }
    for (int y = 0; y < n; y++) if (!used[y]) fv.push_back(y);
    if (!firstCompletion) shuffle(fv.begin(), fv.end(), rng);
    size_t k = 0;
    for (int i = 0; i < n; i++) if (sp[i] == -1) sp[i] = fv[k++];
    return sp;
}

static chrono::steady_clock::time_point T_START;
static double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T_START).count(); }
static void solveMid() {
    CSP S; S.init();
    const double CAPE = 60000;
    vector<double> lf(n + 1, 0);  // log2 factorial
    for (int k = 2; k <= n; k++) lf[k] = lf[k - 1] + log2((double)k);
    double logCount = lf[n];
    vector<vector<int>> carried;
    int want = WANT ? WANT : 300;
    // returns true if finished (explicit stage run)
    auto handleComplete = [&](SysResult& R0, vector<vector<int>>& samp) -> bool {
        size_t Lc = R0.lfree.size();
        double mx = -1e18;
        for (size_t l = 0; l < Lc; l++) mx = max(mx, lf[R0.lfree[l]]);
        vector<double> lw(Lc);
        double tot = 0;
        for (size_t l = 0; l < Lc; l++) { lw[l] = exp2(lf[R0.lfree[l]] - mx); tot += lw[l]; }
        double logTot = mx + log2(tot);
        logCount = logTot;
        if (logTot <= log2(CAPE)) {
            vector<int> all(n); iota(all.begin(), all.end(), 0);
            Explicit E; initExplicit(E, all, all);
            for (size_t l = 0; l < Lc; l++) {
                vector<int> sp(n), fp, fv;
                vector<char> used(n, 0);
                for (int i = 0; i < n; i++) { int v = R0.leaves[l * n + i]; if (v == 0xFFFF) { sp[i] = -1; fp.push_back(i); } else { sp[i] = v; used[v] = 1; } }
                for (int y = 0; y < n; y++) if (!used[y]) fv.push_back(y);
                do {
                    for (size_t k = 0; k < fp.size(); k++) sp[fp[k]] = fv[k];
                    addCand(E, sp);
                } while (next_permutation(fv.begin(), fv.end()));
            }
            runExplicit(E);
            return true;
        }
        vector<double> cum(Lc);
        double acc = 0;
        for (size_t l = 0; l < Lc; l++) { acc += lw[l]; cum[l] = acc; }
        uniform_real_distribution<double> U(0, acc);
        samp.clear();
        for (int t = 0; t < want; t++) {
            size_t l = lower_bound(cum.begin(), cum.end(), U(rng)) - cum.begin();
            if (l >= Lc) l = Lc - 1;
            samp.push_back(leafSample(R0, l));
        }
        return false;
    };
    while (true) {
        S.sync();
        vector<vector<int>> samp;
        bool exactSamples = false;
        if (logCount <= SYS_LOG) {
            SysResult R0 = runSystematic(S, logCount <= 8 ? 10 * SYS_NODES : SYS_NODES);
            if (R0.complete) {
                if (handleComplete(R0, samp)) return;
                exactSamples = true;
            }
        }
        if (!exactSamples) {
            samp = carried;
            long long used = 0;
            for (int t = 0; t < want * 3 && (int)samp.size() < want && used < RND_NODES; t++) {
                auto cb = [&]() -> bool {
                    vector<int> sp = S.p, fv;
                    for (int y = 0; y < n; y++) if (S.b[y] == -1) fv.push_back(y);
                    shuffle(fv.begin(), fv.end(), rng);
                    size_t k = 0;
                    for (int i = 0; i < n; i++) if (sp[i] == -1) sp[i] = fv[k++];
                    samp.push_back(sp); return false;
                };
                S.nodes = 0; S.nodeLimit = max<long long>(2000, (RND_NODES - used));
                S.dfs(cb, true);
                used += S.nodes;
                S.undoTo(0);
            }
            if ((int)samp.size() < want / 2) {
                // sampling struggles: systematic attempt
                SysResult R0 = runSystematic(S, 5 * SYS_NODES);
                if (R0.complete) {
                    if (handleComplete(R0, samp)) return;
                    exactSamples = true;
                } else {
                    size_t Lc = R0.lfree.size();
                    for (int t = 0; Lc && (int)samp.size() < want; t++) samp.push_back(leafSample(R0, rng() % Lc));
                }
            }
        }
        int Ns = samp.size();
        int bi = 0, bj = 0;
        long long best = LLONG_MAX;
        auto scoreAll = [&]() {
            Ns = samp.size();
            best = LLONG_MAX;
            vector<vector<int>> sb(Ns, vector<int>(n));
            for (int s2 = 0; s2 < Ns; s2++) for (int i = 0; i < n; i++) sb[s2][samp[s2][i]] = i;
            vector<int> cnt(R, 0), touched;
            for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) {
                long long sc = 0; touched.clear();
                for (int s2 = 0; s2 < Ns; s2++) {
                    int a = samp[s2][i] ^ sb[s2][j];
                    if (cnt[a] == 0) touched.push_back(a);
                    sc += 2 * cnt[a] + 1; cnt[a]++;
                }
                for (int a : touched) cnt[a] = 0;
                if (sc < best) { best = sc; bi = i; bj = j; }
            }
        };
        if (Ns > 0) scoreAll();
        if (Ns == 0 || best >= (long long)Ns * Ns) {
            // samples do not discriminate: need exact enumeration (escalating budget)
            bool done = false;
            for (long long bud = 10 * SYS_NODES; !done; bud *= 4) {
                SysResult R0 = runSystematic(S, bud);
                if (R0.complete) {
                    if (handleComplete(R0, samp)) return;
                    done = true;
                } else if (bud >= 160 * SYS_NODES || elapsed() > 5.0) {
                    size_t Lc = R0.lfree.size();
                    samp.clear();
                    for (int t = 0; Lc && t < want; t++) samp.push_back(leafSample(R0, rng() % Lc));
                    done = true;
                }
            }
            if (!samp.empty()) scoreAll();
            if (samp.empty() || best >= (long long)Ns * Ns) { bi = rng() % n; bj = rng() % n; }
        }
#ifdef SIM
        if (getenv("MDBG")) fprintf(stderr, "step q=%zu logC=%.1f samples=%d carried=%zu exact=%d t=%.0fms\n", qlog.size(), logCount, Ns, carried.size(), (int)exactSamples, 1000.0*clock()/CLOCKS_PER_SEC);
#endif
        if (Ns > 0 && best < (long long)Ns * Ns) logCount -= max(0.5, log2((double)Ns * Ns / (double)best));
        int r = ask(bi, bj);
        carried.clear();
        for (auto& sp : samp) {
            int bjv = -1;
            for (int i = 0; i < n; i++) if (sp[i] == bj) { bjv = i; break; }
            if ((sp[bi] ^ bjv) == r) carried.push_back(sp);
        }
    }
}

static void solve() {
    T_START = chrono::steady_clock::now();
    qlog.clear(); pend.clear();
    pv.assign(n, -1); bv.assign(n, -1);
    kUnknown = n;
    BB = 1; while ((1 << BB) < n) BB++;
    R = 1 << BB;
    posOff.assign(n, -1); valOff.assign(n, -1);
    rIdx.assign(R, -1); sIdx.assign(R, -1);
    busyP.assign(n, 0); busyB.assign(n, 0);
    ordP.resize(n); ordV.resize(n);
    iota(ordP.begin(), ordP.end(), 0); iota(ordV.begin(), ordV.end(), 0);
    if (RANDORD) { shuffle(ordP.begin(), ordP.end(), rng); shuffle(ordV.begin(), ordV.end(), rng); }
    if (n <= SMALLN) { endgame(); return; }
    if (n <= MIDN) { solveMid(); return; }
    int L = n & (-n);
    // anchor
    {
        trackCnt = true;
        Comp C = startComp();
        bool ok = growComp(C, 1, true, L);
#ifdef SIM
        if (getenv("DBG")) fprintf(stderr, "anchor ok=%d T=%zu P=%zu B=%zu q=%zu\n", ok, C.T.size(), C.Pn.size(), C.Bn.size(), qlog.size());
#endif
        if (!ok || C.T.size() != 1) { columnFallback(C); return; }
        trackCnt = false;
        unloadComp(C);
        assignComp(C, C.T[0]);
    }
    while (true) {
        int fresh = freshPosCount();
        if (fresh <= KE) break;
        Comp C = startComp();
        size_t stopAt = max(1, cstopFor(fresh));
        size_t q0 = qlog.size();
        growComp(C, stopAt, false, L);
        if (C.T.size() > stopAt) linkResolve(C);
#ifdef SIM
        if (getenv("COMP")) fprintf(stderr, "%d %zu\n", fresh, qlog.size() - q0);
#endif
        unloadComp(C);
        if (C.T.size() == 1) { assignComp(C, C.T[0]); cascade(); }
        else pend.push_back(std::move(C));
    }
    // resolve pending
#ifdef SIM
    if (getenv("PEND")) fprintf(stderr, "pending at end %zu queries so far %zu\n", pend.size(), qlog.size());
#endif
    while (!pend.empty()) {
        Comp C = std::move(pend.back());
        pend.pop_back();
        loadComp(C);
        growComp(C, 1, false, L);
        if (C.T.size() > 1) linkResolve(C);
        unloadComp(C);
        assignComp(C, C.T[0]);
        cascade();
    }
    endgame();
}

#ifdef SIM
int main(int argc, char** argv) {
    int N = atoi(argv[1]); int trials = atoi(argv[2]); int mode = argc > 3 ? atoi(argv[3]) : 0;
    if (argc > 4) KE = atoi(argv[4]);
    if (argc > 5) CSTOP_K = atof(argv[5]);
    if (argc > 6) PONLY = atoi(argv[6]);
    if (argc > 7) RANDORD = atoi(argv[7]);
    if (argc > 8) STRONGC = atoi(argv[8]);
    if (argc > 9) SMALLN = atoi(argv[9]);
    if (argc > 10) MIDN = atoi(argv[10]);
    if (argc > 11) WANT = atoi(argv[11]);
    if (argc > 12) MRVK = atoi(argv[12]);
    if (argc > 13) SYS_LOG = atof(argv[13]);
    if (argc > 14) RND_NODES = atoll(argv[14]);
    mt19937 g(987 + N);
    long long tot = 0, mx = 0;
    double maxT = 0;
    for (int tr = 0; tr < trials; tr++) {
        n = N;
        HP.resize(n); iota(HP.begin(), HP.end(), 0);
        if (mode == 0) shuffle(HP.begin(), HP.end(), g);
        else if (mode == 1) { int c = g() % (n & -n); for (int i = 0; i < n; i++) HP[i] = i ^ c; }
        else if (mode == 2) {
            vector<int> pr(n / 2); iota(pr.begin(), pr.end(), 0); shuffle(pr.begin(), pr.end(), g);
            vector<int> pos(n/2); iota(pos.begin(), pos.end(), 0); shuffle(pos.begin(), pos.end(), g);
            for (int k = 0; k < n / 2; k++) { int b = g() & 1; HP[2*pos[k]] = 2*pr[k] ^ b; HP[2*pos[k]+1] = 2*pr[k] ^ b ^ 1; }
            if (n & 1) HP[n-1] = n-1;
        } else if (mode == 3) { // reverse
            for (int i = 0; i < n; i++) HP[i] = n - 1 - i;
        } else if (mode == 4) { // few swaps of identity
            for (int s = 0; s < 3; s++) swap(HP[g() % n], HP[g() % n]);
        } else if (mode == 5) { // identity on half, random on other half
            vector<int> idx; for (int i = 0; i < n; i++) if (g() & 1) idx.push_back(i);
            vector<int> v = idx; shuffle(v.begin(), v.end(), g);
            for (size_t k = 0; k < idx.size(); k++) HP[idx[k]] = v[k];
        } else if (mode == 6) { // p = i ^ c on blocks, random block perm (big classes for power of 2)
            int L = n & -n; int bs = min(L, 64); int nb = n / bs; vector<int> bp(nb); iota(bp.begin(), bp.end(), 0); shuffle(bp.begin(), bp.end(), g);
            int c = g() % bs; for (int i = 0; i < n; i++) HP[i] = bp[i / bs] * bs + ((i % bs) ^ c);
        } else if (mode == 7) { // involution random
            vector<int> v(n); iota(v.begin(), v.end(), 0); shuffle(v.begin(), v.end(), g);
            for (int k = 0; k + 1 < n; k += 2) { HP[v[k]] = v[k+1]; HP[v[k+1]] = v[k]; }
        }
        HB.assign(n, 0); for (int i = 0; i < n; i++) HB[HP[i]] = i;
        simQueries = 0;
        auto T0 = chrono::steady_clock::now();
        solve();
        double el = chrono::duration<double>(chrono::steady_clock::now() - T0).count();
        maxT = max(maxT, el);
        vector<int> G = groupG(HP);
        bool ok = (long long)G.size() == outCount;
        if (ok) {
            int w = outPerm[0] ^ HP[0];
            bool f = false; for (int x : G) if (x == w) f = true;
            if (!f) ok = false;
            for (int i = 0; i < n && ok; i++) if (outPerm[i] != (HP[i] ^ w)) ok = false;
        }
        if (!ok) { printf("WRONG n=%d trial=%d\n", n, tr); return 1; }
        tot += simQueries; mx = max(mx, simQueries);
    }
    printf("n=%d avg=%.2f max=%lld ratio=%.4f maxT=%.3fs\n", N, (double)tot / trials, mx, (double)tot / trials / N, maxT);
}
#else
int main() {
    if (scanf("%d", &n) != 1) return 0;
    solve();
}
#endif

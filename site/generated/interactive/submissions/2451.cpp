#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;

static int n, m;
static vector<vector<int>> G; // 1-indexed
static vector<int> PX, PY;    // pos of value
static vector<int> PM;        // prefix max of row 1
static const int INF = INT_MAX;

#ifdef LOCAL
#include "local_judge.h"
#else
static int ask(int l, int T) {
    printf("? %d %d\n", l, T);
    fflush(stdout);
    int v;
    if (scanf("%d", &v) != 1) exit(0);
    if (v < 0) exit(0);
    return v;
}
#endif

typedef unsigned long long u64;
static int NT;   // number of diagonals (times 1..NT)
static int WD;   // words per diagonal bitset (bit index = row x)
static vector<int> XLO, XHI;
static vector<int> GV;            // GV[t*(n+2)+x] = value of cell on diag t at row x
static vector<int> ASC, DESC;     // per diag sorted rows: ASC[t*(n+2)+k], k in [0,len)
static vector<int> DLEN;
static inline int gv(int t, int x) { return GV[t * (n + 2) + x]; }

struct SparseMax {
    int N = 0, LG = 0;
    vector<int> st;
    void build(const vector<int>& a) {
        N = a.size();
        LG = 32 - __builtin_clz(N);
        st.resize((size_t)LG * N);
        copy(a.begin(), a.end(), st.begin());
        for (int k = 1; k < LG; k++)
            for (int i = 0; i + (1 << k) <= N; i++)
                st[k * N + i] = max(st[(k - 1) * N + i], st[(k - 1) * N + i + (1 << (k - 1))]);
    }
    int query(int l, int r) const {
        int k = 31 - __builtin_clz(r - l + 1);
        return max(st[k * N + l], st[k * N + r - (1 << k) + 1]);
    }
};

struct Snake {
    bool init = false;
    int ver = 1;
    vector<int> lastCh;
    vector<u64> al, fw, bw, fe; // per diag bitsets (NT+2)*WD
    vector<int> minP, maxP, topP; // pointers into ASC / DESC
    vector<int> minF, maxF, fl, fh;
    SparseMax smin, smax;
};

static vector<Snake> snakes;
static Snake defSnake;

static inline bool bit(const u64* b, int x) { return (b[x >> 6] >> (x & 63)) & 1; }

static void refreshDiag(Snake& s, int t) {
    const u64* fe = &s.fe[(size_t)t * WD];
    int base = t * (n + 2);
    int& mp = s.minP[t];
    while (!bit(fe, ASC[base + mp])) mp++;
    int& xp = s.maxP[t];
    while (!bit(fe, DESC[base + xp])) xp++;
    s.minF[t] = gv(t, ASC[base + mp]);
    s.maxF[t] = gv(t, DESC[base + xp]);
    int lo = -1, hi = -1;
    for (int w = 0; w < WD; w++) if (fe[w]) { lo = w * 64 + __builtin_ctzll(fe[w]); break; }
    for (int w = WD - 1; w >= 0; w--) if (fe[w]) { hi = w * 64 + 63 - __builtin_clzll(fe[w]); break; }
    s.fl[t] = lo;
    s.fh[t] = hi;
}

static void buildDefault() {
    Snake& s = defSnake;
    s.init = true;
    size_t SZ = (size_t)(NT + 2) * WD;
    s.al.assign(SZ, 0); s.fw.assign(SZ, 0); s.bw.assign(SZ, 0); s.fe.assign(SZ, 0);
    for (int t = 1; t <= NT; t++)
        for (int x = XLO[t]; x <= XHI[t]; x++) s.al[(size_t)t * WD + (x >> 6)] |= 1ULL << (x & 63);
    // cell (1,2) is diag 2 row 1: first move is down
    if (NT >= 2) s.al[(size_t)2 * WD] &= ~(1ULL << 1);
    for (int t = 1; t <= NT; t++) {
        u64* f = &s.fw[(size_t)t * WD];
        const u64* a = &s.al[(size_t)t * WD];
        if (t == 1) { for (int w = 0; w < WD; w++) f[w] = a[w]; continue; }
        const u64* p = &s.fw[(size_t)(t - 1) * WD];
        u64 carry = 0;
        for (int w = 0; w < WD; w++) {
            u64 sh = (p[w] << 1) | carry;
            carry = p[w] >> 63;
            f[w] = a[w] & (p[w] | sh);
        }
    }
    for (int t = NT; t >= 1; t--) {
        u64* b = &s.bw[(size_t)t * WD];
        const u64* a = &s.al[(size_t)t * WD];
        if (t == NT) { for (int w = 0; w < WD; w++) b[w] = a[w]; continue; }
        const u64* q = &s.bw[(size_t)(t + 1) * WD];
        for (int w = 0; w < WD; w++) {
            u64 sh = (q[w] >> 1) | (w + 1 < WD ? (q[w + 1] << 63) : 0);
            b[w] = a[w] & (q[w] | sh);
        }
    }
    for (size_t i = 0; i < SZ; i++) s.fe[i] = s.fw[i] & s.bw[i];
    s.minP.assign(NT + 2, 0); s.maxP.assign(NT + 2, 0); s.topP.assign(NT + 2, 0);
    s.minF.assign(NT + 1, 0); s.maxF.assign(NT + 1, 0); s.fl.assign(NT + 1, 0); s.fh.assign(NT + 1, 0);
    for (int t = 1; t <= NT; t++) refreshDiag(s, t);
    s.smin.build(s.minF);
    s.smax.build(s.maxF);
    s.lastCh.assign(NT + 1, 0);
}

static void initSnake(Snake& s) {
    if (s.init) return;
    int v = s.ver;
    s = defSnake;
    s.ver = v;
}

static void applyQuery(int l, int T, int v) {
    Snake& s = snakes[l];
    initSnake(s);
    int a = max(2, T - l + 1);
    for (int t = a; t <= T; t++) {
        int base = t * (n + 2);
        int& p = s.topP[t];
        u64* al = &s.al[(size_t)t * WD];
        while (p < DLEN[t] && gv(t, DESC[base + p]) > v) {
            int x = DESC[base + p];
            al[x >> 6] &= ~(1ULL << (x & 63));
            p++;
        }
    }
    int px = PX[v], py = PY[v];
    int lo = a, hi = T;
    if (px != 1) {
        int d = px + py - 1;
        u64* al = &s.al[(size_t)d * WD];
        for (int w = 0; w < WD; w++) al[w] &= (w == (px >> 6)) ? (1ULL << (px & 63)) : 0ULL;
        lo = min(lo, d);
        hi = max(hi, d);
    }
    int clo = lo, chi = hi;
    // forward
    for (int t = max(lo, 2); t <= NT; t++) {
        u64* f = &s.fw[(size_t)t * WD];
        const u64* al = &s.al[(size_t)t * WD];
        const u64* p = &s.fw[(size_t)(t - 1) * WD];
        u64 carry = 0, diff = 0;
        for (int w = 0; w < WD; w++) {
            u64 sh = (p[w] << 1) | carry;
            carry = p[w] >> 63;
            u64 nf = al[w] & (p[w] | sh);
            diff |= nf ^ f[w];
            f[w] = nf;
        }
        if (diff) chi = max(chi, t);
        if (t > hi && !diff) break;
    }
    // backward
    if (hi == NT) {
        u64* b = &s.bw[(size_t)NT * WD];
        const u64* al = &s.al[(size_t)NT * WD];
        for (int w = 0; w < WD; w++) b[w] = al[w];
    }
    for (int t = min(hi, NT - 1); t >= 1; t--) {
        u64* b = &s.bw[(size_t)t * WD];
        const u64* al = &s.al[(size_t)t * WD];
        const u64* q = &s.bw[(size_t)(t + 1) * WD];
        u64 diff = 0;
        for (int w = 0; w < WD; w++) {
            u64 sh = (q[w] >> 1) | (w + 1 < WD ? (q[w + 1] << 63) : 0);
            u64 nb = al[w] & (q[w] | sh);
            diff |= nb ^ b[w];
            b[w] = nb;
        }
        if (diff) clo = min(clo, t);
        if (t < lo && !diff) break;
    }
    s.ver++;
    bool any = false;
    for (int t = max(1, clo); t <= min(NT, chi); t++) {
        u64* fe = &s.fe[(size_t)t * WD];
        const u64* f = &s.fw[(size_t)t * WD];
        const u64* b = &s.bw[(size_t)t * WD];
        u64 diff = 0;
        for (int w = 0; w < WD; w++) {
            u64 x = f[w] & b[w];
            diff |= x ^ fe[w];
            fe[w] = x;
        }
        if (diff) {
            int om = s.minF[t], oM = s.maxF[t];
            refreshDiag(s, t);
            s.lastCh[t] = s.ver;
            if (om != s.minF[t] || oM != s.maxF[t]) any = true;
        }
    }
    if (any) {
        s.smin.build(s.minF);
        s.smax.build(s.maxF);
    }
}

static inline int virtMax(int l, int T) { return (T - l + 1 <= 1) ? PM[min(n, l - T + 1)] : 0; }

static inline void bounds(int l, int T, int& lb, int& ub) {
    int vm = virtMax(l, T);
    int a = max(2, T - l + 1);
    lb = ub = vm;
    if (a <= T) {
        const Snake& s = snakes[l].init ? snakes[l] : defSnake;
        lb = max(lb, s.smin.query(a, T));
        ub = max(ub, s.smax.query(a, T));
    }
}

static vector<int> bestA, bestB;

// min over paths (through feasible cells) from diag a to diag T of max value
static int bottleneck(const Snake& s, int a, int T, int* allOut) {
    int* cur = bestA.data();
    int* nxt = bestB.data();
    {
        int l0 = s.fl[a], h0 = s.fh[a];
        const u64* fe = &s.fe[(size_t)a * WD];
        int r = INF;
        for (int x = l0; x <= h0; x++) {
            int val = bit(fe, x) ? gv(a, x) : INF;
            cur[x] = val;
            r = min(r, val);
        }
        cur[l0 - 1] = INF; cur[h0 + 1] = INF;
        if (allOut) allOut[a] = r;
    }
    for (int t = a + 1; t <= T; t++) {
        int l0 = s.fl[t], h0 = s.fh[t];
        const u64* fe = &s.fe[(size_t)t * WD];
        const int* gp = &GV[t * (n + 2)];
        int r = INF;
        for (int x = l0; x <= h0; x++) {
            int p = min(cur[x - 1], cur[x]);
            int val = bit(fe, x) ? gp[x] : INF;
            int q = max(p, val);
            nxt[x] = q;
            r = min(r, q);
        }
        nxt[l0 - 1] = INF; nxt[h0 + 1] = INF;
        swap(cur, nxt);
        if (allOut) allOut[t] = r;
    }
    int r = INF;
    for (int x = s.fl[T]; x <= s.fh[T]; x++) r = min(r, cur[x]);
    return r;
}

static vector<vector<int>> defEx; // defEx[a][T]

static int exactLB(int l, int T) {
    int vm = virtMax(l, T);
    int a = max(2, T - l + 1);
    if (a > T) return vm;
    if (!snakes[l].init) {
        if (defEx[a].empty()) {
            defEx[a].assign(NT + 1, INF);
            bottleneck(defSnake, a, NT, defEx[a].data());
        }
        return max(vm, defEx[a][T]);
    }
    return max(vm, bottleneck(snakes[l], a, T, nullptr));
}

int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    G.assign(n + 2, vector<int>(n + 2, 0));
    PX.assign(n * n + 1, 0);
    PY.assign(n * n + 1, 0);
    for (int i = 1; i <= n; i++)
        for (int j = 1; j <= n; j++) {
            if (scanf("%d", &G[i][j]) != 1) return 0;
            PX[G[i][j]] = i;
            PY[G[i][j]] = j;
        }
#ifdef LOCAL
    localReadPaths();
#endif
    NT = 2 * n - 1;
    WD = (n + 2 + 63) / 64;
    XLO.assign(NT + 2, 0); XHI.assign(NT + 2, 0); DLEN.assign(NT + 2, 0);
    GV.assign((size_t)(NT + 2) * (n + 2), INF);
    ASC.assign((size_t)(NT + 2) * (n + 2), 0);
    DESC.assign((size_t)(NT + 2) * (n + 2), 0);
    for (int t = 1; t <= NT; t++) {
        XLO[t] = max(1, t - n + 1);
        XHI[t] = min(n, t);
        DLEN[t] = XHI[t] - XLO[t] + 1;
        vector<int> xs;
        for (int x = XLO[t]; x <= XHI[t]; x++) { GV[t * (n + 2) + x] = G[x][t + 1 - x]; xs.push_back(x); }
        sort(xs.begin(), xs.end(), [&](int p, int q) { return gv(t, p) < gv(t, q); });
        for (int k = 0; k < (int)xs.size(); k++) {
            ASC[t * (n + 2) + k] = xs[k];
            DESC[t * (n + 2) + k] = xs[xs.size() - 1 - k];
        }
    }
    PM.assign(n + 1, 0);
    for (int j = 1; j <= n; j++) PM[j] = max(PM[j - 1], G[1][j]);
    snakes.assign(n + 1, Snake());
    bestA.assign(n + 3, INF);
    bestB.assign(n + 3, INF);
    buildDefault();
    defEx.assign(NT + 2, {});

    vector<vector<uint8_t>> known(n + 1, vector<uint8_t>(NT + 1, 0));
    vector<int> knownVals;
    int V = n * n;
    vector<int> fwk(V + 1, 0);
    auto fadd = [&](int v) { for (; v <= V; v += v & -v) fwk[v]++; };
    auto fsum = [&](int v) { int r = 0; v = min(v, V); for (; v > 0; v -= v & -v) r += fwk[v]; return r; };
    auto markKnown = [&](int l, int T, int v) {
        known[l][T] = 1;
        knownVals.push_back(v);
        fadd(v);
    };
    // bucket queue: keys pop in nondecreasing order
    struct E { int ex, l, T; };
    vector<vector<E>> B0(V + 2), B1(V + 2);
    for (int l = 1; l <= n; l++)
        for (int T = NT; T >= 1; T--) {
            int lb, ub;
            bounds(l, T, lb, ub);
            B0[lb].push_back({0, l, T});
        }
    int k = 0;
    vector<E> cand;
    vector<int> candUB;
    while (true) {
        while (k <= V && B0[k].empty() && B1[k].empty()) k++;
        if (k > V) break;
        cand.clear();
        candUB.clear();
        while (!B0[k].empty() || !B1[k].empty()) {
            E top;
            if (!B0[k].empty()) { top = B0[k].back(); B0[k].pop_back(); }
            else { top = B1[k].back(); B1[k].pop_back(); }
            int ex = top.ex, l = top.l, T = top.T;
            if (known[l][T]) continue;
            int lb, ub;
            bounds(l, T, lb, ub);
            if (lb == ub) { markKnown(l, T, lb); continue; }
            if (lb > k) { B0[lb].push_back({0, l, T}); continue; }
            Snake& s = snakes[l];
            bool valid = ex > 0;
            if (valid && s.init) {
                for (int t = max(1, T - l + 1); t <= T; t++)
                    if (s.lastCh[t] > ex) { valid = false; break; }
            }
            if (!valid) {
                int e = exactLB(l, T);
                if (e == ub) { markKnown(l, T, e); continue; }
                if (e > k) { B1[e].push_back({s.ver, l, T}); continue; }
                top.ex = s.ver;
            }
            cand.push_back(top);
            candUB.push_back(ub);
        }
        if (cand.empty()) continue;
        if (fsum(k) >= m) break;
        int bi = 0;
        for (int i = 1; i < (int)cand.size(); i++) {
            if (candUB[i] > candUB[bi] || (candUB[i] == candUB[bi] && cand[i].l < cand[bi].l)) bi = i;
        }
        for (int i = 0; i < (int)cand.size(); i++) if (i != bi) B1[k].push_back(cand[i]);
        int l = cand[bi].l, T = cand[bi].T;
        int v = ask(l, T);
        markKnown(l, T, v);
        applyQuery(l, T, v);
    }
    sort(knownVals.begin(), knownVals.end());
#ifdef LOCAL
    localFinish(knownVals, m);
#else
    string out = "!";
    for (int i = 0; i < m; i++) { out += ' '; out += to_string(knownVals[i]); }
    out += '\n';
    fputs(out.c_str(), stdout);
    fflush(stdout);
#endif
    return 0;
}

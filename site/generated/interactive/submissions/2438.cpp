#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p;
    DSU(int n = 0) : p(n) { iota(p.begin(), p.end(), 0); }
    int f(int x) { while (p[x] != x) x = p[x] = p[p[x]]; return x; }
    bool u(int a, int b) { a = f(a); b = f(b); if (a == b) return false; p[a] = b; return true; }
};

// ---------------- interaction ----------------
int n, m;
vector<int> eu, ev;
vector<vector<pair<int,int>>> adj;
chrono::steady_clock::time_point gStart = chrono::steady_clock::now();
double gTb = 0, gTr = 0, gTh = 0; long long gSteps = 0, gPc = 0;
long long qcount = 0, gA = 0, gB = 0, gCq = 0, gF = 0, gFmax = 0; double gInfo = 0;

#ifdef LOCAL
namespace judge {
    vector<char> royal;
    bool bad = false;
}
int askRaw(const vector<int>& es) {
    qcount++;
    if ((int)es.size() != n - 1) { judge::bad = true; return 0; }
    DSU d(n);
    vector<char> used(m, 0);
    int c = 0;
    for (int e : es) {
        if (e < 0 || e >= m || used[e]) { judge::bad = true; return 0; }
        used[e] = 1;
        if (!d.u(eu[e], ev[e])) { judge::bad = true; return 0; }
        c += judge::royal[e];
    }
    return c;
}
#else
int askRaw(const vector<int>& es) {
    qcount++;
    string s = "?";
    for (int e : es) { s += ' '; s += to_string(e); }
    s += '\n';
    fputs(s.c_str(), stdout);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}
#endif

// ---------------- solver state ----------------
vector<int> st;          // -1 unknown, 0, 1
vector<int> treeEdges;
vector<char> inTree;

int queryForest(const vector<int>& S) {
    DSU d(n);
    vector<int> es;
    es.reserve(n - 1);
    for (int e : S) { bool ok = d.u(eu[e], ev[e]); (void)ok; es.push_back(e); }
    int known = 0;
    for (int e : treeEdges) if (d.u(eu[e], ev[e])) { es.push_back(e); known += st[e]; }
    return askRaw(es) - known;
}

// component structure over found royal edges
DSU comp;
vector<vector<int>> members;
vector<int> Rem, cntU;
int comps;
vector<int> checkQ;
vector<int> leafQ;

void setStatus(int e, int s);

void mergeComps(int X, int W) {
    if (members[X].size() > members[W].size()) swap(X, W);
    // internal unknown edges -> 0
    for (int a : members[X])
        for (auto [b, e2] : adj[a])
            if (st[e2] == -1 && comp.f(b) == W) {
                st[e2] = 0; cntU[X]--; cntU[W]--;
            }
    comp.p[X] = W;
    for (int a : members[X]) members[W].push_back(a);
    members[X].clear();
    cntU[W] += cntU[X];
    if (Rem[X] >= 0 && Rem[W] >= 0) Rem[W] = Rem[X] + Rem[W] - 2; else Rem[W] = -1;
    comps--;
    checkQ.push_back(W);
}

void setStatus(int e, int s) {
    if (st[e] != -1) return;
    st[e] = s;
    int X = comp.f(eu[e]), W = comp.f(ev[e]);
    cntU[X]--; cntU[W]--;
    checkQ.push_back(X); checkQ.push_back(W);
    if (s == 1) {
        for (int Y : {X, W}) if (Rem[Y] == 1)
            for (int a : members[Y]) for (auto [b, e2] : adj[a]) if (st[e2] == -1) {
                st[e2] = 0; cntU[comp.f(eu[e2])]--; cntU[comp.f(ev[e2])]--;
                checkQ.push_back(comp.f(b));
            }
        mergeComps(X, W);
    }
}

vector<int> cutEdges(int X) {
    vector<int> r;
    for (int a : members[X])
        for (auto [b, e] : adj[a])
            if (st[e] == -1) r.push_back(e);
    return r;
}

void processChecks() {
    while (!checkQ.empty()) {
        int X = comp.f(checkQ.back()); checkQ.pop_back();
        if (Rem[X] < 0 || cntU[X] == 0) continue;
        if (Rem[X] == 0) {
            for (int e : cutEdges(X)) setStatus(e, 0);
        } else if (Rem[X] == cntU[X]) {
            for (int e : cutEdges(X)) setStatus(e, 1);
        } else if (Rem[X] == 1 && comps > 2) {
            for (int e : cutEdges(X)) {
                int o = comp.f(eu[e]) == X ? comp.f(ev[e]) : comp.f(eu[e]);
                if (Rem[o] == 1) setStatus(e, 0);
            }
        }
    }
}

vector<int> answerEdges() {
    vector<int> r;
    for (int e = 0; e < m; e++) if (st[e] == 1) r.push_back(e);
    return r;
}


// ---------------- Phase C: multiplexed unit searches ----------------
struct RDSU {
    vector<int> p, sz; vector<pair<int,int>> hist;
    void init(int n) { p.resize(n); sz.assign(n, 1); iota(p.begin(), p.end(), 0); hist.clear(); }
    int f(int x) { while (p[x] != x) x = p[x]; return x; }
    bool u(int a, int b) {
        a = f(a); b = f(b); if (a == b) return false;
        if (sz[a] > sz[b]) swap(a, b);
        p[a] = b; sz[b] += sz[a]; hist.push_back({a, b}); return true;
    }
    size_t snap() { return hist.size(); }
    void rollback(size_t s) { while (hist.size() > s) { auto [a, b] = hist.back(); hist.pop_back(); p[a] = a; sz[b] -= sz[a]; } }
};

typedef unsigned __int128 u64;
struct Coin { vector<int> S, R; int k, lo, hi; vector<double> pr; int rep; int seg; };
struct Segment { vector<int> C; int k; int origin; bool alive; int pend; };
vector<Coin> coins;
vector<Segment> segs;
vector<int> pool;
vector<pair<u64, double>> feas;
int POOLMAX = getenv("PM") ? atoi(getenv("PM")) : 30;
int CANDK = getenv("CK") ? atoi(getenv("CK")) : 12;
double TGT = getenv("TG") ? atof(getenv("TG")) : 0.5;
double TGT2 = getenv("TG2") ? atof(getenv("TG2")) : 0;
int NONLEAF = getenv("NL") ? atoi(getenv("NL")) : 4;
int LAZYM = getenv("LZ") ? atoi(getenv("LZ")) : 1000;
int PREPX = getenv("PX") ? atoi(getenv("PX")) : 4;
size_t FEASMAX = getenv("FM") ? atoi(getenv("FM")) : (1 << 19);
mt19937 crng(7);

static inline int getv(u64 mk, int j) { return (mk >> (3 * j)) & 7; }

vector<double> wBtab; int wBn = 0;
double wB(int a, int b) {
    if (a + b - 3 < 0) return comps == 2 ? 1.0 : 0.0;
    if (wBn != n + 2) { wBn = n + 2; wBtab.assign((size_t)wBn * wBn, -1.0); }
    double& r = wBtab[(size_t)a * wBn + b];
    if (r < 0) r = exp(lgamma(a) + lgamma(b) - lgamma(a + b - 2));
    return r;
}
double edgeW(int e) { return wB(Rem[comp.f(eu[e])], Rem[comp.f(ev[e])]); }

int coinStatusVal(const Coin& c) {
    {
        int rs = 0, rr = 0;
        for (int e : c.S) rs += st[e] == 1;
        for (int e : c.R) rr += st[e] == 1;
        if (rs + rr == c.k) return rs;
    }
    int r = 0; bool allK = true;
    for (int e : c.S) { if (st[e] == -1) { allK = false; break; } r += st[e]; }
    if (allK) return r;
    r = 0; allK = true;
    for (int e : c.R) { if (st[e] == -1) { allK = false; break; } r += st[e]; }
    if (allK) return c.k - r;
    return -1;
}
void addSeg(const vector<int>& C, int k, int origin) {
    segs.push_back({C, k, origin, true, -1});
}
void applyCoin(const Coin& c, int v) {
    if (c.seg >= 0) segs[c.seg].alive = false;
    int origin = c.seg >= 0 ? segs[c.seg].origin : c.rep;
    addSeg(c.S, v, origin);
    addSeg(c.R, c.k - v, origin);
}
// clean segments; returns true if any status changed
bool cleanSegs() {
    bool any = false;
    for (size_t i = 0; i < segs.size(); i++) {
        Segment& s = segs[i];
        if (!s.alive) continue;
        vector<int> C; int k = s.k;
        for (int e : s.C) { if (st[e] == -1) C.push_back(e); else k -= st[e]; }
        s.C = C; s.k = k;
        if (C.empty()) { s.alive = false; continue; }
        if (k == 0) { s.alive = false; for (int e : C) setStatus(e, 0); any = true; }
        else if (k == (int)C.size()) { s.alive = false; for (int e : C) setStatus(e, 1); any = true; }
    }
    return any;
}
void fixVar(int j, int v) {
    vector<pair<u64, double>> nf;
    for (auto [mk, w] : feas) if (getv(mk, j) == v) {
        u64 lo = mk & (((u64)1 << (3 * j)) - 1), hi = (mk >> (3 * j + 3)) << (3 * j);
        nf.push_back({lo | hi, w});
    }
    feas.swap(nf);
    int id = pool[j];
    pool.erase(pool.begin() + j);
    if (coins[id].seg >= 0) segs[coins[id].seg].pend = -1;
    applyCoin(coins[id], v);
}
void resolveAll() {
    bool changed = true;
    while (changed) {
        changed = false;
        processChecks();
        if (cleanSegs()) { changed = true; continue; }
        for (int j = 0; j < (int)pool.size(); j++) {
            int v = coinStatusVal(coins[pool[j]]);
            if (v != -1) { fixVar(j, v); changed = true; break; }
        }
        if (changed) continue;
        for (int j = 0; j < (int)pool.size() && !changed; j++) {
            int mn = 8, mx = -1;
            for (auto& [mk, w] : feas) { int g = getv(mk, j); mn = min(mn, g); mx = max(mx, g); if (mn != mx) break; }
            if (mn == mx) { fixVar(j, mn); changed = true; }
        }
    }
}
static double entropyOf(const vector<double>& d) {
    double t = 0, h = 0;
    for (double x : d) t += x;
    for (double x : d) if (x > 0) { double q = x / t; h -= q * log2(q); }
    return h;
}

struct Unit { vector<int> C; int k; int rep; int seg; };

void phaseC() {
    coins.clear(); pool.clear(); feas.assign(1, {0ULL, 1.0});
    RDSU qd; qd.init(n);
    while (true) {
        {
            double el = chrono::duration<double>(chrono::steady_clock::now() - gStart).count();
            if (el > 7.0) { POOLMAX = min(POOLMAX, 8); FEASMAX = min<size_t>(FEASMAX, 4096); }
            else if (el > 5.0) { POOLMAX = min(POOLMAX, 18); FEASMAX = min<size_t>(FEASMAX, 1 << 16); }
        }
        auto tR0 = chrono::steady_clock::now();
        resolveAll();
        gTr += chrono::duration<double>(chrono::steady_clock::now() - tR0).count();
        if (comps <= 1) break;
        // candidate units
        vector<char> pendComp(n, 0), segComp(n, 0);
        for (int id : pool) if (coins[id].seg < 0) pendComp[comp.f(coins[id].rep)] = 1;
        for (auto& s : segs) if (s.alive) segComp[comp.f(s.origin)] = 1;
        vector<Unit> units;
        bool anyLeafUnit = false;
        for (size_t i = 0; i < segs.size(); i++) if (segs[i].alive && segs[i].pend < 0)
            units.push_back({segs[i].C, segs[i].k, -1, (int)i});
        for (int v = 0; v < n; v++) if (comp.f(v) == v && Rem[v] >= 1 && cntU[v] > Rem[v] && !pendComp[v] && !segComp[v]) {
            if (Rem[v] > 1 && NONLEAF == 0) continue;
            if (comps == 2 && pendComp[comp.f(v)]) continue;
            units.push_back({{}, Rem[v], v, -1});
            if (Rem[v] == 1) anyLeafUnit = true;
        }
        if (comps == 2) {
            // only one search allowed on the final edge
            bool pending = !pool.empty();
            if (pending) units.clear();
            else if (units.size() > 1) units.resize(1);
        }
        if (NONLEAF >= 2) {
            int leafU = 0;
            for (auto& U : units) if (U.k == 1) leafU++;
            if (leafU >= NONLEAF) { vector<Unit> u2; for (auto& U : units) if (U.k == 1 || U.seg >= 0) u2.push_back(U); units.swap(u2); }
        }
        shuffle(units.begin(), units.end(), crng);
        auto tB0 = chrono::steady_clock::now();
        // build query
        qd.init(n);
        vector<int> Q;
        vector<int> incl;           // pool indices included
        vector<vector<double>> newPr;
        vector<Coin> newC;
        vector<char> usedU(units.size(), 0), usedPool(pool.size(), 0);
        // per-assignment partial sums for included pool vars
        size_t F = feas.size();
        vector<unsigned char> sumA(F, 0);
        size_t P = pool.size();
        vector<unsigned char> vals(F * max<size_t>(P, 1));
        for (size_t a = 0; a < F; a++) { u64 mk = feas[a].first; for (size_t j = 0; j < P; j++) vals[a * P + j] = getv(mk, j); }
        int sumMax = 0;
        auto baseDist = [&]() {
            vector<double> d(sumMax + 1, 0.0);
            for (size_t a = 0; a < F; a++) d[sumA[a]] += feas[a].second;
            return d;
        };
        auto conv = [&](vector<double> d, const vector<double>& p) {
            vector<double> d2(d.size() + p.size(), 0.0);
            for (size_t x = 0; x < d.size(); x++) if (d[x] > 0) for (size_t y = 0; y < p.size(); y++) d2[x + y] += d[x] * p[y];
            return d2;
        };
        // precompute sorted weights for units (sampled)
        vector<vector<pair<double,int>>> uw(units.size());
        vector<double> utot(units.size(), 0);
        int prepared = 0;
        for (size_t i = 0; i < units.size() && prepared < CANDK + PREPX; i++) {
            if (units[i].seg < 0) units[i].C = cutEdges(units[i].rep);
            for (int e : units[i].C) if (st[e] == -1) { double wt = edgeW(e); uw[i].push_back({wt, e}); utot[i] += wt; }
            if (uw[i].size() < 2 || utot[i] <= 0) { uw[i].clear(); continue; }
            sort(uw[i].begin(), uw[i].end(), greater<>());
            prepared++;
        }
        double curH = 0;
        size_t expand = feas.size();
        int stepNo = 0;
        vector<double> lastScore(pool.size(), 0.0);
        while (true) {
            double bestH = curH + 1e-9; int bestType = -1, bestIdx = -1;
            Coin bestCoin;
            vector<double> bd = baseDist();
            vector<double> bdN = bd;
            for (auto& p : newPr) bdN = conv(bdN, p);
            // pool candidates
            vector<int> pc;
            for (int j = 0; j < (int)pool.size(); j++) if (!usedPool[j]) {
                const Coin& c = coins[pool[j]];
                size_t sn = qd.snap(); bool ok = true;
                for (int e : c.S) if (st[e] == -1 && !qd.u(eu[e], ev[e])) { ok = false; break; }
                qd.rollback(sn);
                if (ok) pc.push_back(j);
            }
            if (stepNo > 0 && (int)pc.size() > LAZYM) {
                sort(pc.begin(), pc.end(), [&](int x, int y) { return lastScore[x] > lastScore[y]; });
                pc.resize(LAZYM);
            }
            stepNo++;
            auto tH0 = chrono::steady_clock::now();
            gSteps++; gPc += pc.size();
            if (!pc.empty()) {
                int W = sumMax + 8;
                vector<double> hist(pc.size() * W, 0.0);
                for (size_t a = 0; a < F; a++) {
                    const unsigned char* va = &vals[a * P];
                    double w = feas[a].second; int s0 = sumA[a];
                    double* hp = hist.data() + s0;
                    for (size_t t = 0; t < pc.size(); t++) hp[t * W + va[pc[t]]] += w;
                }
                for (size_t t = 0; t < pc.size(); t++) {
                    vector<double> d(hist.begin() + t * W, hist.begin() + (t + 1) * W);
                    for (auto& p : newPr) d = conv(d, p);
                    double h = entropyOf(d);
                    lastScore[pc[t]] = h;
                    if (h > bestH) { bestH = h; bestType = 0; bestIdx = pc[t]; }
                }
            }
            gTh += chrono::duration<double>(chrono::steady_clock::now() - tH0).count();
            int tried = 0;
            if ((int)(pool.size() + newC.size()) < POOLMAX)
            for (int i = 0; i < (int)units.size() && tried < CANDK; i++) if (!usedU[i] && !uw[i].empty()) {
                Unit& U = units[i];
                tried++;
                double tot = utot[i];
                for (double tg : {TGT, TGT2}) { if (tg <= 0) continue;
                size_t sn = qd.snap();
                Coin c; c.k = U.k; c.rep = U.rep; c.seg = U.seg; double sw = 0;
                for (auto [wt, e] : uw[i]) {
                    if ((c.S.empty() || sw + wt <= tot * tg + 1e-9) && qd.u(eu[e], ev[e])) { c.S.push_back(e); sw += wt; }
                    else c.R.push_back(e);
                }
                qd.rollback(sn);
                if (c.S.empty() || c.R.empty()) continue;
                c.lo = max(0, c.k - (int)c.R.size()); c.hi = min(c.k, (int)c.S.size());
                if (c.hi > 7 || c.hi <= c.lo) continue;
                if (expand * (c.hi - c.lo + 1) > FEASMAX) continue;
                double q = sw / tot;
                c.pr.assign(c.hi + 1, 0.0);
                double t = 0;
                for (int v = c.lo; v <= c.hi; v++) { c.pr[v] = exp(lgamma(c.k + 1) - lgamma(v + 1) - lgamma(c.k - v + 1)) * pow(q, v) * pow(1 - q, c.k - v); t += c.pr[v]; }
                for (auto& x : c.pr) x /= t;
                double h = entropyOf(conv(bdN, c.pr));
                if (h > bestH) { bestH = h; bestType = 1; bestIdx = i; bestCoin = c; }
                }
            }
            if (bestType < 0) break;
            curH = bestH;
            if (bestType == 0) {
                usedPool[bestIdx] = 1; incl.push_back(bestIdx);
                for (size_t a = 0; a < F; a++) sumA[a] += vals[a * P + bestIdx];
                sumMax += 7;
                for (int e : coins[pool[bestIdx]].S) if (st[e] == -1) { qd.u(eu[e], ev[e]); Q.push_back(e); }
            } else {
                usedU[bestIdx] = 1;
                for (int e : bestCoin.S) { qd.u(eu[e], ev[e]); Q.push_back(e); }
                newPr.push_back(bestCoin.pr);
                expand *= bestCoin.hi - bestCoin.lo + 1;
                newC.push_back(bestCoin);
            }
        }
        if (Q.empty()) {
#ifdef LOCAL
            fprintf(stderr, "EMPTY QUERY units=%zu pool=%zu feas=%zu comps=%d\n", units.size(), pool.size(), feas.size(), comps);
            for (auto& U : units) fprintf(stderr, " unit k=%d C=%zu seg=%d\n", U.k, U.C.size(), U.seg);
            exit(1);
#endif
            break;
        }
        auto tB1 = chrono::steady_clock::now(); gTb += chrono::duration<double>(tB1 - tB0).count();
        int r = queryForest(Q);
        for (int j : incl) for (int e : coins[pool[j]].S) if (st[e] == 1) r++;
        // combos of new coins grouped by sum
        vector<vector<pair<u64, double>>> bySum(1, vector<pair<u64, double>>{{(u64)0, 1.0}});
        for (size_t i = 0; i < newC.size(); i++) {
            int j = pool.size();
            coins.push_back(newC[i]); pool.push_back(coins.size() - 1);
            if (newC[i].seg >= 0) segs[newC[i].seg].pend = coins.size() - 1;
            vector<vector<pair<u64, double>>> nb(bySum.size() + newC[i].hi);
            for (size_t t = 0; t < bySum.size(); t++)
                for (auto [mk, w] : bySum[t])
                    for (int v = newC[i].lo; v <= newC[i].hi; v++)
                        nb[t + v].push_back({mk | ((u64)v << (3 * j)), w * newC[i].pr[v]});
            bySum.swap(nb);
        }
        vector<pair<u64, double>> nf;
        double tw = 0;
        for (size_t a2 = 0; a2 < F; a2++) {
            int t = r - (int)sumA[a2];
            if (t < 0 || t >= (int)bySum.size()) continue;
            auto [mk0, w0] = feas[a2];
            for (auto [mk, w] : bySum[t]) { nf.push_back({mk0 | mk, w0 * w}); tw += w0 * w; }
        }
        for (auto& [mk, w] : nf) w /= tw;
        feas.swap(nf);
        gInfo += -log2(tw); gCq++; gF += feas.size(); gFmax = max(gFmax, (long long)feas.size());
#ifdef LOCAL
        if (feas.empty()) { fprintf(stderr, "EMPTY FEAS\n"); exit(1); }
#endif
    }
}

double log2STgen() {
    int N = n - 1;
    if (N <= 0) return 0;
    vector<vector<double>> L(N, vector<double>(N, 0));
    for (int e = 0; e < m; e++) {
        int a = eu[e], b = ev[e];
        if (a < N) L[a][a] += 1;
        if (b < N) L[b][b] += 1;
        if (a < N && b < N) { L[a][b] -= 1; L[b][a] -= 1; }
    }
    double ld = 0;
    for (int i = 0; i < N; i++) {
        int p = i;
        for (int r = i + 1; r < N; r++) if (fabs(L[r][i]) > fabs(L[p][i])) p = r;
        swap(L[i], L[p]);
        if (fabs(L[i][i]) < 1e-12) return 0;
        ld += log2(fabs(L[i][i]));
        for (int r = i + 1; r < N; r++) {
            double f = L[r][i] / L[i][i];
            if (f != 0) for (int c = i; c < N; c++) L[r][c] -= f * L[i][c];
        }
    }
    return ld;
}

// exact version-space search for small instances
bool smallSolve(vector<int>& ans, size_t LIM) {
    if (m > 64) return false;
    if (log2STgen() > log2((double)LIM) + 0.5) return false;
    vector<unsigned long long> trees;
    bool overflow = false;
    vector<int> p(n);
    function<int(int)> fnd = [&](int x) { return p[x] == x ? x : fnd(p[x]); };
    vector<pair<int,int>> hist;
    auto unite = [&](int a, int b) { a = fnd(a); b = fnd(b); if (a == b) return false; p[a] = b; hist.push_back({a, b}); return true; };
    auto rollback = [&](size_t sz) { while (hist.size() > sz) { p[hist.back().first] = hist.back().first; hist.pop_back(); } };
    iota(p.begin(), p.end(), 0);
    function<void(int, int, unsigned long long)> rec = [&](int idx, int cnt, unsigned long long mask) {
        if (overflow) return;
        if (cnt == n - 1) { trees.push_back(mask); if (trees.size() > LIM) overflow = true; return; }
        if (idx == m) return;
        if (cnt + (m - idx) < n - 1) return;
        size_t sz = hist.size();
        if (unite(eu[idx], ev[idx])) { rec(idx + 1, cnt + 1, mask | (1ULL << idx)); rollback(sz); }
        // exclude: check connectivity with remaining edges
        {
            size_t s2 = hist.size();
            int comps = 0;
            for (int v = 0; v < n; v++) if (fnd(v) == v) comps++;
            for (int e = idx + 1; e < m && comps > 1; e++) if (unite(eu[e], ev[e])) comps--;
            rollback(s2);
            if (comps == 1) rec(idx + 1, cnt, mask);
        }
    };
    rec(0, 0, 0);
    if (overflow || trees.empty()) return false;
    vector<unsigned long long> H = trees;
    mt19937 srng(99);
    while (H.size() > 1) {
        vector<unsigned long long> C = trees;
        static double SB = getenv("SB") ? atof(getenv("SB")) : 4e7; size_t maxC = max<size_t>(50, (size_t)(SB / H.size()));
        if (C.size() > maxC) { shuffle(C.begin(), C.end(), srng); C.resize(maxC); }
        double bestH = -1; unsigned long long best = C[0];
        vector<double> cnt(n + 1);
        for (auto c : C) {
            fill(cnt.begin(), cnt.end(), 0);
            for (auto h : H) cnt[__builtin_popcountll(c & h)] += 1;
            double e = 0;
            for (double x : cnt) if (x > 0) { double q = x / H.size(); e -= q * log2(q); }
            if (e > bestH + 1e-12) { bestH = e; best = c; }
        }
        vector<int> q;
        for (int e = 0; e < m; e++) if (best >> e & 1) q.push_back(e);
        int r = askRaw(q);
        vector<unsigned long long> H2;
        for (auto h : H) if (__builtin_popcountll(best & h) == r) H2.push_back(h);
        H.swap(H2);
        if (H.empty()) return false;
    }
    ans.clear();
    for (int e = 0; e < m; e++) if (H[0] >> e & 1) ans.push_back(e);
    return true;
}

vector<int> solve() {
    st.assign(m, -1);
    inTree.assign(m, 0);
    treeEdges.clear();
    if (m == n - 1) { vector<int> r(m); iota(r.begin(), r.end(), 0); return r; }
    {
        static size_t SLIM = getenv("SLIM") ? atol(getenv("SLIM")) : 300000;
        vector<int> ans;
        if (smallSolve(ans, SLIM)) return ans;
    }
    // DFS tree
    vector<int> par(n, -1), pe(n, -1), dep(n, 0), tin(n, 0), it(n, 0);
    vector<char> vis(n, 0);
    {
        vector<int> stk = {0}; vis[0] = 1; int timer = 0; tin[0] = timer++;
        while (!stk.empty()) {
            int v = stk.back();
            if (it[v] < (int)adj[v].size()) {
                auto [w, e] = adj[v][it[v]++];
                if (!vis[w]) {
                    vis[w] = 1; par[w] = v; pe[w] = e; dep[w] = dep[v] + 1; tin[w] = timer++;
                    inTree[e] = 1; treeEdges.push_back(e); stk.push_back(w);
                }
            } else stk.pop_back();
        }
    }
    int base = askRaw(treeEdges);
    if (base == n - 1) return treeEdges;
    vector<int> pos(m, -1);
    for (int i = 0; i < (int)treeEdges.size(); i++) pos[treeEdges[i]] = i;
    if (base == 0) {
        for (int e : treeEdges) st[e] = 0;
    } else {
        vector<int> nt;
        for (int e = 0; e < m; e++) if (!inTree[e]) nt.push_back(e);
        auto ancOf = [&](int e) { return dep[eu[e]] < dep[ev[e]] ? eu[e] : ev[e]; };
        auto descOf = [&](int e) { return dep[eu[e]] < dep[ev[e]] ? ev[e] : eu[e]; };
        sort(nt.begin(), nt.end(), [&](int a, int b) {
            int aa = ancOf(a), ab = ancOf(b);
            if (tin[aa] != tin[ab]) return tin[aa] < tin[ab];
            return dep[descOf(a)] > dep[descOf(b)];
        });
        vector<int> T = treeEdges;
        auto swapAsk = [&](int te, int f) {
            int i = pos[te];
            T[i] = f;
            int r = askRaw(T);
            T[i] = te;
            return r - base;
        };
        auto swapCycle = [&](int f) {
            int a = eu[f], b = ev[f];
            vector<int> U; int K = -1; bool allKroyal = true;
            while (a != b) {
                if (dep[a] < dep[b]) swap(a, b);
                int e = pe[a];
                if (st[e] == -1) U.push_back(e);
                else { if (K == -1 || st[e] == 0) K = e; if (st[e] == 0) allKroyal = false; }
                a = par[a];
            }
            if (U.empty()) return;
            vector<int> d(U.size());
            int sf = st[f];
            for (size_t i = 0; i < U.size(); i++) {
                d[i] = swapAsk(U[i], f);
                if (d[i] == 1) sf = 1;
                else if (d[i] == -1) sf = 0;
            }
            if (sf == -1) {
                if (K == -1) sf = 0;
                else if (allKroyal) sf = 0;
                else sf = st[K] + swapAsk(K, f);
            }
            st[f] = sf;
            for (size_t i = 0; i < U.size(); i++) st[U[i]] = sf - d[i];
        };
        static double RFAC = getenv("RF") ? atof(getenv("RF")) : 2;
        static int PAmode = getenv("PA") ? atoi(getenv("PA")) : 1;
        if (PAmode == 0) {
            for (int f : nt) swapCycle(f);
            for (int e : treeEdges) if (st[e] == -1) st[e] = 1;  // bridges
        } else {
            // bridges via low-link
            vector<int> low(n);
            for (int v = 0; v < n; v++) low[v] = tin[v];
            for (int f : nt) { int d = descOf(f), a = ancOf(f); low[d] = min(low[d], tin[a]); }
            vector<int> byTin(n);
            for (int v = 0; v < n; v++) byTin[tin[v]] = v;
            for (int i = n - 1; i > 0; i--) { int v = byTin[i]; low[par[v]] = min(low[par[v]], low[v]); }
            for (int v = 0; v < n; v++) if (par[v] >= 0 && low[v] >= tin[v]) st[pe[v]] = 1;
            auto kTnow = [&]() { int k = base; for (int e : treeEdges) if (st[e] == 1) k--; return k; };
            auto unknownTree = [&]() { vector<int> r; for (int e : treeEdges) if (st[e] == -1) r.push_back(e); return r; };
            // measure x(N) - x(E)
            auto measure = [&](const vector<int>& E, const vector<int>& N) {
                vector<char> inE(m, 0);
                for (int e : E) inE[e] = 1;
                DSU d(n);
                vector<int> Q;
                int kT = kTnow();
                for (int e : treeEdges) if (st[e] == -1 && !inE[e]) { d.u(eu[e], ev[e]); Q.push_back(e); }
                for (int e : N) { d.u(eu[e], ev[e]); Q.push_back(e); }
                int kn = 0;
                for (int e : treeEdges) if (st[e] != -1 && d.u(eu[e], ev[e])) { Q.push_back(e); kn += st[e]; }
                for (int e = 0; e < m && (int)Q.size() < n - 1; e++) if (!inTree[e] && st[e] != -1 && d.u(eu[e], ev[e])) { Q.push_back(e); kn += st[e]; }
                int r = askRaw(Q);
                return r - kT - kn;
            };
            // resolve segment G with count c; sign: tree -> measure returns -x(E)
            function<void(vector<int>, int, bool)> resolveSeg = [&](vector<int> G, int c, bool isTree) {
                vector<int> G2; for (int e : G) if (st[e] == -1) G2.push_back(e); else c -= st[e];
                G = G2;
                if (G.empty()) return;
                if (c == 0) { for (int e : G) st[e] = 0; return; }
                if (c == (int)G.size()) { for (int e : G) st[e] = 1; return; }
                size_t h = G.size() / 2;
                if (c == 1) h = (G.size() + 1) / 2;
                vector<int> A(G.begin(), G.begin() + h), B(G.begin() + h, G.end());
                int ca = isTree ? -measure(A, {}) : measure({}, A);
                resolveSeg(A, ca, isTree);
                resolveSeg(B, c - ca, isTree);
            };
            mt19937 arng(12345);
            if (kTnow() == 0) { for (int e : treeEdges) if (st[e] == -1) st[e] = 0; }
            {
                vector<int> UT = unknownTree();
                if (!UT.empty() && kTnow() == (int)UT.size()) for (int e : UT) st[e] = 1;
            }
            // bootstrap: shortest cycle
            {
                vector<int> UT = unknownTree();
                if (!UT.empty()) {
                    int best = -1, bl = INT_MAX;
                    for (int f : nt) { int L = dep[descOf(f)] - dep[ancOf(f)]; if (L < bl) { bl = L; best = f; } }
                    if (best >= 0) swapCycle(best);
                }
            }
            while (true) {
                vector<int> UT = unknownTree();
                if (UT.empty()) break;
                int kT = kTnow();
                if (kT == 0) { for (int e : UT) st[e] = 0; break; }
                if (kT == (int)UT.size()) { for (int e : UT) st[e] = 1; break; }
                // redundant set R
                DSU dk(n);
                for (int e = 0; e < m; e++) if (st[e] != -1) dk.u(eu[e], ev[e]);
                shuffle(UT.begin(), UT.end(), arng);
                vector<int> R;
                {
                    DSU d2 = dk;
                    for (int e : UT) if (!d2.u(eu[e], ev[e])) R.push_back(e);
                }
                // choose N
                DSU du(n);
                for (int e : UT) du.u(eu[e], ev[e]);
                vector<int> cand;
                for (int f : nt) if (st[f] == -1) cand.push_back(f);
                shuffle(cand.begin(), cand.end(), arng);
                vector<int> N;
                {
                    DSU dk2 = dk;
                    for (int f : cand) {
                        if (du.f(eu[f]) != du.f(ev[f]) && dk2.f(eu[f]) != dk2.f(ev[f])) { du.u(eu[f], ev[f]); dk2.u(eu[f], ev[f]); N.push_back(f); }
                    }
                }
                if (!R.empty() && (R.size() * RFAC >= N.size() || R.size() == UT.size())) {
#ifdef LOCAL
                    if(getenv("DBG"))fprintf(stderr, "[R %zu kT=%d UT=%zu q=%lld] ", R.size(), kT, UT.size(), qcount);
#endif
                    int c = -measure(R, {});
                    resolveSeg(R, c, true);
                    continue;
                }
                if (!N.empty()) {
#ifdef LOCAL
                    if(getenv("DBG"))fprintf(stderr, "[N %zu q=%lld] ", N.size(), qcount);
#endif
                    int c = measure({}, N);
                    resolveSeg(N, c, false);
                    continue;
                }
                if (!R.empty()) {
                    int c = -measure(R, {});
                    resolveSeg(R, c, true);
                    continue;
                }
                // fallback: swap cycle on some edge covering unknown tree edge
                int pick = -1;
                for (int f : cand) {
                    int a = eu[f], b = ev[f]; bool has = false;
                    while (a != b) { if (dep[a] < dep[b]) swap(a, b); if (st[pe[a]] == -1) { has = true; break; } a = par[a]; }
                    if (has) { pick = f; break; }
                }
                if (pick < 0) { for (int e : UT) st[e] = 1; break; }
#ifdef LOCAL
                if(getenv("DBG"))fprintf(stderr, "[S q=%lld] ", qcount);
#endif
                swapCycle(pick);
            }
        }
    }

    long long qA = qcount;
    // components over found royal edges
    comp = DSU(n);
    members.assign(n, {});
    for (int v = 0; v < n; v++) members[v] = {v};
    Rem.assign(n, -1); cntU.assign(n, 0);
    comps = n;
    checkQ.clear(); leafQ.clear();
    for (int e = 0; e < m; e++) if (st[e] == -1) { cntU[eu[e]]++; cntU[ev[e]]++; }
    for (int e = 0; e < m; e++) if (st[e] == 1) {
        int X = comp.f(eu[e]), W = comp.f(ev[e]);
        if (X != W) mergeComps(X, W);
    }
    checkQ.clear();
    if (comps == 1) return answerEdges();

    segs.clear();
    // Phase B: degrees of components
    {
        vector<int> roots;
        for (int v = 0; v < n; v++) if (comp.f(v) == v) roots.push_back(v);
        // leave out the one with largest cut
        int skip = roots[0];
        for (int r : roots) {
            bool better = members[r].size() < members[skip].size() || (members[r].size() == members[skip].size() && cntU[r] > cntU[skip]);
            if (better) skip = r;
        }
        int sum = 0;
        for (int X : roots) {
            if (X == skip) continue;
            vector<int> cut = cutEdges(X);
            int tot = 0;
            vector<char> taken(cut.size(), 0);
            size_t done = 0;
            int pieces = 0;
            {
                vector<char> tk(cut.size(), 0); size_t dn = 0;
                while (dn < cut.size()) {
                    DSU d(n); pieces++;
                    for (size_t i = 0; i < cut.size(); i++) if (!tk[i] && d.u(eu[cut[i]], ev[cut[i]])) { tk[i] = 1; dn++; }
                }
            }
            if (members[X].size() > 1 && pieces >= (int)members[X].size()) {
                for (int a : members[X]) {
                    vector<int> S;
                    for (auto [b, e] : adj[a]) if (st[e] == -1) S.push_back(e);
                    if (S.empty()) continue;
                    int c = queryForest(S);
                    tot += c;
                    segs.push_back({S, c, a, true, -1});
                }
                done = cut.size();
            }
            while (done < cut.size()) {
                DSU d(n);
                vector<int> S;
                for (size_t i = 0; i < cut.size(); i++) if (!taken[i] && d.u(eu[cut[i]], ev[cut[i]])) {
                    taken[i] = 1; done++; S.push_back(cut[i]);
                }
                tot += queryForest(S);
            }
            Rem[X] = tot; sum += tot;
        }
        Rem[skip] = 2 * (comps - 1) - sum;
        for (int X : roots) checkQ.push_back(X);
        processChecks();
    }

    long long qB = qcount;
#ifdef LOCAL
    gA=qA; gB=qB-qA;
#endif
    phaseC();
    return answerEdges();
}

#ifdef LOCAL
mt19937_64 rng(12345);
int rnd(int a, int b) { return uniform_int_distribution<int>(a, b)(rng); }

void buildAdj() {
    adj.assign(n, {});
    for (int e = 0; e < m; e++) { adj[eu[e]].push_back({ev[e], e}); adj[ev[e]].push_back({eu[e], e}); }
}
// uniform spanning tree via Wilson
vector<char> wilson() {
    vector<char> in(n, 0), r(m, 0);
    vector<int> nxt(n, -1), nxtE(n, -1);
    in[rnd(0, n - 1)] = 1;
    for (int i = 0; i < n; i++) {
        int u = i;
        while (!in[u]) { auto [w, e] = adj[u][rnd(0, adj[u].size() - 1)]; nxt[u] = w; nxtE[u] = e; u = w; }
        u = i;
        while (!in[u]) { in[u] = 1; r[nxtE[u]] = 1; u = nxt[u]; }
    }
    return r;
}
void genGraph(int N, int M) {
    n = N; set<pair<int,int>> s; eu.clear(); ev.clear();
    vector<int> perm(n); iota(perm.begin(), perm.end(), 0); shuffle(perm.begin(), perm.end(), rng);
    for (int i = 1; i < n; i++) { int a = perm[i], b = perm[rnd(0, i - 1)]; s.insert({min(a,b), max(a,b)}); }
    long long maxM = 1LL * n * (n - 1) / 2;
    M = min<long long>(M, maxM);
    if (M > maxM / 2) {
        vector<pair<int,int>> all;
        for (int a = 0; a < n; a++) for (int b = a + 1; b < n; b++) if (!s.count({a,b})) all.push_back({a,b});
        shuffle(all.begin(), all.end(), rng);
        for (auto p : all) { if ((int)s.size() >= M) break; s.insert(p); }
    } else {
        while ((int)s.size() < M) { int a = rnd(0, n - 1), b = rnd(0, n - 1); if (a == b) continue; s.insert({min(a,b), max(a,b)}); }
    }
    vector<pair<int,int>> v(s.begin(), s.end()); shuffle(v.begin(), v.end(), rng);
    m = v.size();
    for (auto [a, b] : v) { if (rng() & 1) swap(a, b); eu.push_back(a); ev.push_back(b); }
    buildAdj();
}
double log2ST() {
    int N = n - 1;
    vector<vector<double>> L(N, vector<double>(N, 0));
    for (int e = 0; e < m; e++) {
        int a = eu[e], b = ev[e];
        if (a < N) L[a][a] += 1;
        if (b < N) L[b][b] += 1;
        if (a < N && b < N) { L[a][b] -= 1; L[b][a] -= 1; }
    }
    double ld = 0;
    for (int i = 0; i < N; i++) {
        int p = i;
        for (int r = i + 1; r < N; r++) if (fabs(L[r][i]) > fabs(L[p][i])) p = r;
        swap(L[i], L[p]);
        ld += log2(fabs(L[i][i]));
        for (int r = i + 1; r < N; r++) {
            double f = L[r][i] / L[i][i];
            if (f != 0) for (int c = i; c < N; c++) L[r][c] -= f * L[i][c];
        }
    }
    return ld;
}
int main(int argc, char** argv) {
    // tests: type n m
    vector<tuple<string,int,int>> tests;
    int seed = argc > 1 ? atoi(argv[1]) : 1;
    rng.seed(seed);
    string mode = argc > 2 ? argv[2] : "all";
    auto add = [&](string t, int N, int M) { tests.push_back({t, N, M}); };
    if (mode == "all" || mode == "complete") { add("uni", 500, 124750); add("uni", 100, 4950); add("path", 500, 124750); add("star", 500, 124750); }
    if (mode == "all" || mode == "dense") { add("uni", 500, 60000); add("uni", 500, 20000); add("uni", 240, 20000); }
    if (mode == "stress") {
        int cnt = argc > 3 ? atoi(argv[3]) : 300;
        for (int i = 0; i < cnt; i++) {
            int N = rnd(2, rnd(0, 3) == 0 ? 120 : 25);
            long long mx = 1LL * N * (N - 1) / 2;
            int M = rnd(N - 1, (int)min<long long>(mx, rnd(0, 1) ? mx : 3LL * N));
            add("uni", N, M);
        }
    }
    if (mode == "small") { for (int i = 0; i < 6; i++) { add("uni", 7, 21); add("uni", 7, 12); add("uni", 8, 28); add("uni", 6, 10); add("uni", 12, 20); add("uni",30,40);} }
    if (mode == "all" || mode == "sparse") { add("uni", 500, 5000); add("uni", 500, 1500); add("uni", 500, 700); add("uni", 50, 100); add("uni", 7, 15); }
    long long total = 0;
    for (auto [t, N, M] : tests) {
        genGraph(N, M);
        if (t == "uni") judge::royal = wilson();
        else {
            judge::royal.assign(m, 0);
            // path or star over random permutation where edges exist (complete graph)
            map<pair<int,int>,int> id;
            for (int e = 0; e < m; e++) id[{min(eu[e],ev[e]), max(eu[e],ev[e])}] = e;
            vector<int> p(n); iota(p.begin(), p.end(), 0); shuffle(p.begin(), p.end(), rng);
            for (int i = 1; i < n; i++) {
                int a = t == "path" ? p[i - 1] : p[0], b = p[i];
                judge::royal[id[{min(a,b), max(a,b)}]] = 1;
            }
        }
        qcount = 0; judge::bad = false;
        if (getenv("DBG")) fprintf(stderr, "\n=== %s n=%d m=%d\n", t.c_str(), n, m);
        auto t0 = chrono::steady_clock::now(); gStart = t0;
        { static int pm0 = POOLMAX; static size_t fm0 = FEASMAX; POOLMAX = pm0; FEASMAX = fm0; }
        vector<int> ans = solve();
        double el = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
        bool ok = !judge::bad && (int)ans.size() == n - 1;
        for (int e : ans) if (!judge::royal[e]) ok = false;
        if (getenv("ST")) printf("log2ST=%.0f ", log2ST());
        printf("%-5s n=%3d m=%6d A=%4lld B=%4lld C=%4lld q=%5lld %s %.2fs bits/q=%.2f F=%lld Fmax=%lld tb=%.2f tr=%.2f th=%.2f steps=%lld pc=%lld\n", t.c_str(), n, m, gA, gB, qcount-gA-gB, qcount, ok ? "OK" : "FAIL", el, gInfo/max(1LL,gCq), gF/max(1LL,gCq), gFmax, gTb, gTr, gTh, gSteps, gPc); gTb=gTr=gTh=0; gSteps=gPc=0; gA=gB=0; gInfo=0; gCq=0; gF=gFmax=0;
        total += qcount;
    }
    printf("total %lld\n", total);
}
#else
int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    eu.resize(m); ev.resize(m);
    for (int i = 0; i < m; i++) if (scanf("%d %d", &eu[i], &ev[i]) != 2) return 0;
    adj.assign(n, {});
    for (int e = 0; e < m; e++) { adj[eu[e]].push_back({ev[e], e}); adj[ev[e]].push_back({eu[e], e}); }
    vector<int> ans = solve();
    string s = "!";
    for (int e : ans) { s += ' '; s += to_string(e); }
    s += '\n';
    fputs(s.c_str(), stdout);
    fflush(stdout);
    return 0;
}
#endif

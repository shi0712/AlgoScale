#include <bits/stdc++.h>
using namespace std;

// ---------------- interaction ----------------
#ifdef LOCAL_SIM
#include "sim.h"
#else
static long long g_queries = 0;
static int ask(const vector<int>& seq) {
    string s = "? " + to_string(seq.size());
    for (int v : seq) { s += ' '; s += to_string(v); }
    s += '\n';
    fputs(s.c_str(), stdout);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    g_queries++;
    return r;
}
static void answer(int c) {
    printf("! %d\n", c);
    fflush(stdout);
    exit(0);
}
static int readN() {
    int n;
    if (scanf("%d", &n) != 1) exit(0);
    return n;
}
#endif

// ---------------- detecting matrices ----------------
const int KMAX = 7;
#ifndef SMALL_N
#define SMALL_N 8
#endif
#ifndef GAMMA
#define GAMMA 1.0
#endif
#ifndef TRUNC_MIN
#define TRUNC_MIN 8
#endif
typedef vector<vector<int8_t>> Mat;
struct Block {
    int fam, k;                     // fam 0: signed (Cantor-Mills), 1: unsigned
    int m, n;
    Mat M;
    vector<int> flip;
    vector<int> order;
    vector<vector<short>> preP, preQ;
    vector<pair<int,int>> fullRows; // (p,q) in execution order
    vector<pair<int, vector<pair<int,int>>>> opts; // (j, rows) options
};
static vector<Block> blocks;
static Mat SA[KMAX + 1], UB[KMAX + 1];

static Mat buildSigned(int k) {
    if (k == 0) return {{1}};
    const Mat& P = SA[k - 1];
    int m = P.size(), n = P[0].size();
    Mat R(2 * m, vector<int8_t>(2 * n + m, 0));
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            R[i][j] = P[i][j]; R[i][n + j] = P[i][j];
            R[m + i][j] = P[i][j]; R[m + i][n + j] = -P[i][j];
        }
        R[i][2 * n + i] = 1;
    }
    return R;
}
static Mat buildUnsigned(int k) {
    if (k == 0) return {{1}};
    const Mat& P = UB[k - 1];
    int m = P.size(), n = P[0].size();
    Mat R(2 * m + 1, vector<int8_t>(2 * n + m, 0));
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            R[i][j] = P[i][j]; R[i][n + j] = P[i][j];
            R[m + i][j] = P[i][j]; R[m + i][n + j] = 1 - P[i][j];
        }
        R[i][2 * n + i] = 1;
    }
    for (int j = 0; j < n; j++) R[2 * m][n + j] = 1;
    return R;
}
static vector<int> mulMat(const Mat& P, const vector<int>& x) {
    vector<int> r(P.size(), 0);
    for (size_t i = 0; i < P.size(); i++)
        for (size_t j = 0; j < x.size(); j++) r[i] += P[i][j] * x[j];
    return r;
}
// decode y = SA[k] x
static vector<int> decodeSigned(int k, const vector<int>& y) {
    if (k == 0) return {y[0]};
    int m = y.size() / 2;
    vector<int> z(m), t2(m), t1(m);
    for (int i = 0; i < m; i++) {
        int d = y[i] - y[m + i];
        z[i] = ((d % 2) + 2) % 2;
        t2[i] = (d - z[i]) / 2;
    }
    vector<int> x2 = decodeSigned(k - 1, t2);
    vector<int> bx2 = mulMat(SA[k - 1], x2);
    for (int i = 0; i < m; i++) t1[i] = y[m + i] + bx2[i];
    vector<int> x1 = decodeSigned(k - 1, t1);
    vector<int> x = x1;
    x.insert(x.end(), x2.begin(), x2.end());
    x.insert(x.end(), z.begin(), z.end());
    return x;
}
static vector<int> decodeUnsigned(int k, const vector<int>& y) {
    if (k == 0) return {y[0]};
    int m = (y.size() - 1) / 2;
    int sigma = y[2 * m];
    vector<int> z(m), t2(m), t1(m);
    for (int i = 0; i < m; i++) {
        int v = y[m + i] - sigma;          // B x1 - B x2
        int d = y[i] - v;                  // 2 B x2 + z
        z[i] = ((d % 2) + 2) % 2;
        t2[i] = (d - z[i]) / 2;
    }
    vector<int> x2 = decodeUnsigned(k - 1, t2);
    for (int i = 0; i < m; i++) t1[i] = y[i] - t2[i] - z[i];
    vector<int> x1 = decodeUnsigned(k - 1, t1);
    vector<int> x = x1;
    x.insert(x.end(), x2.begin(), x2.end());
    x.insert(x.end(), z.begin(), z.end());
    return x;
}

static double scoreFlip(const Mat& M, const vector<int>& flip, double rho) {
    vector<pair<int,int>> nd;
    for (size_t r = 0; r < M.size(); r++) {
        int p = 0, q = 0;
        for (size_t j = 0; j < flip.size(); j++) {
            int v = M[r][j] * flip[j];
            if (v == 1) p++; else if (v == -1) q++;
        }
        nd.push_back({max(p, q), min(p, q)});
    }
    sort(nd.begin(), nd.end());
    double s = 0, ga = 1.0 / (1 + rho), gb = rho / (1 + rho);
    for (int j = 0; j < (int)nd.size(); j++) {
        s = max(s, nd[j].first + 1 - j * ga);
        if (nd[j].second > 0) s = max(s, (nd[j].second + 1 - j * gb) / rho);
    }
    return s;
}

static pair<int,int> rowKey(const Block& b, int r, int j) {
    int p = b.preP[r][j], q = b.preQ[r][j];
    return {max(p, q), min(p, q)};
}

static void prepBlocks() {
    mt19937 rng(12345);
    for (int k = 0; k <= KMAX; k++) { SA[k] = buildSigned(k); UB[k] = buildUnsigned(k); }
    const double rhos[] = {1.0, 0.7, 0.5, 0.33, 0.2};
    for (int fam = 0; fam < 2; fam++)
        for (int k = (fam ? 1 : 0); k <= KMAX; k++)
        for (int vi = 0; vi < (fam || k == 0 ? 1 : 5); vi++) {
            Block b;
            b.fam = fam; b.k = k;
            b.M = fam ? UB[k] : SA[k];
            b.m = b.M.size(); b.n = b.M[0].size();
            b.flip.assign(b.n, 1);
            if (fam == 0 && k >= 1) {
                double rho = rhos[vi];
                double best = scoreFlip(b.M, b.flip, rho);
                int iters = k <= 4 ? 3000 : (k == 5 ? 1500 : (k == 6 ? 700 : 250));
                for (int it = 0; it < iters; it++) {
                    int i = rng() % b.n;
                    b.flip[i] = -b.flip[i];
                    double sc = scoreFlip(b.M, b.flip, rho);
                    if (sc <= best) best = sc;
                    else b.flip[i] = -b.flip[i];
                }
            }
            b.preP.assign(b.m, vector<short>(b.n + 1, 0));
            b.preQ.assign(b.m, vector<short>(b.n + 1, 0));
            for (int r = 0; r < b.m; r++)
                for (int j = 0; j < b.n; j++) {
                    int v = b.M[r][j] * b.flip[j];
                    b.preP[r][j + 1] = b.preP[r][j] + (v == 1);
                    b.preQ[r][j + 1] = b.preQ[r][j] + (v == -1);
                }
            b.order.resize(b.m);
            iota(b.order.begin(), b.order.end(), 0);
            stable_sort(b.order.begin(), b.order.end(), [&](int x, int y) {
                return rowKey(b, x, b.n) < rowKey(b, y, b.n);
            });
            for (int r : b.order) b.fullRows.push_back({b.preP[r][b.n], b.preQ[r][b.n]});
            for (int t = TRUNC_MIN; t <= 8; t++) {
                int j = b.n * t / 8;
                if (j < 1 || (t < 8 && b.k < 3)) continue;
                vector<pair<int,int>> rows;
                for (int r : b.order) if (b.preP[r][j] + b.preQ[r][j] > 0) rows.push_back({b.preP[r][j], b.preQ[r][j]});
                sort(rows.begin(), rows.end(), [](auto& u, auto& v){ return make_pair(max(u.first,u.second), min(u.first,u.second)) < make_pair(max(v.first,v.second), min(v.first,v.second)); });
                b.opts.push_back({j, rows});
            }
            blocks.push_back(b);
        }
}

static vector<pair<int,int>> blockRows(const Block& b, int j) {
    vector<pair<int,int>> rows;
    for (int r : b.order) if (b.preP[r][j] + b.preQ[r][j] > 0) rows.push_back({b.preP[r][j], b.preQ[r][j]});
    sort(rows.begin(), rows.end(), [](auto& u, auto& v){ return make_pair(max(u.first,u.second), min(u.first,u.second)) < make_pair(max(v.first,v.second), min(v.first,v.second)); });
    return rows;
}

const int INF = 1e9;
// number of queries needed to measure rows (greedy as in measure()), growth by fraction f to B
static int simulateRows(const vector<pair<int,int>>& rows, double a, double b, double f) {
    int cnt = 0;
    for (auto [p, q] : rows) {
        while (p > 0 || q > 0) {
            int capA = max(0, (int)floor(a + 1e-9) - 1), capB = max(0, (int)floor(b + 1e-9) - 1);
            bool full1 = p <= capA && q <= capB, full2 = p <= capB && q <= capA;
            if (full1 || full2) { p = q = 0; }
            else {
                int t1 = min(p, capA) + min(q, capB);
                int t2 = min(p, capB) + min(q, capA);
                if (max(t1, t2) == 0) return INF;
                if (t1 >= t2) { p -= min(p, capA); q -= min(q, capB); }
                else { p -= min(p, capB); q -= min(q, capA); }
            }
            cnt++;
            if (a >= b) { b += f * GAMMA; a += 1 - f * GAMMA; }
            else { a += (1 - f) * GAMMA; b += 1 - (1 - f) * GAMMA; }
            if (cnt > 100000) return INF;
        }
    }
    return cnt;
}

// ---------------- state ----------------
static int n;
static vector<int> As, Bs;     // known labels
static deque<int> pool;         // unprocessed labels
static long long countedA = 0;  // A's counted in counting phase (not identified)

static void addKnown(int lbl, int isB) {
    if (isB) Bs.push_back(lbl); else As.push_back(lbl);
}

// One query: coins in A-sandwich (inA), coins in B-sandwich (inB), optional parity coin.
// Returns D = sum_{inA} x - sum_{inB} x  (x=1 means B). Parity coin is resolved & recorded.
static int doQuery(const vector<int>& inA, const vector<int>& inB, int par) {
    vector<int> seq;
    int lastType = -1;
    if (!inA.empty()) {
        for (size_t i = 0; i < inA.size(); i++) { seq.push_back(As[i]); seq.push_back(inA[i]); }
        seq.push_back(As[inA.size()]);
        lastType = 0;
    }
    if (!inB.empty()) {
        for (size_t i = 0; i < inB.size(); i++) { seq.push_back(Bs[i]); seq.push_back(inB[i]); }
        seq.push_back(Bs[inB.size()]);
        lastType = 1;
    }
    if (par >= 0) {
        if (lastType < 0) {
            if (!As.empty()) { seq.push_back(As[0]); lastType = 0; }
            else { seq.push_back(Bs[0]); lastType = 1; }
        }
        seq.push_back(par);
    }
    int V = ask(seq);
    int junction = (!inA.empty() && !inB.empty()) ? 1 : 0;
    int Vp = V - 2 * (int)inB.size() - junction;
    int e = ((Vp % 2) + 2) % 2;
    int D = (Vp - e) / 2;
    if (par >= 0) {
        int isB = lastType == 0 ? e : 1 - e;
        addKnown(par, isB);
    } else {
        // e should be 0
    }
    return D;
}

static int takeParity() {
    if (pool.empty()) return -1;
    int p = pool.front();
    pool.pop_front();
    return p;
}

// Measure sum_i sign_i * x_i for given signed coins, possibly with several queries.
static int measure(vector<pair<int,int>> coins) {
    vector<int> plus, minus;
    for (auto [c, s] : coins) (s > 0 ? plus : minus).push_back(c);
    int total = 0;
    while (!plus.empty() || !minus.empty()) {
        int capA = max(0, (int)As.size() - 1), capB = max(0, (int)Bs.size() - 1);
        // mu=+1: plus->A, minus->B ; mu=-1: plus->B, minus->A
        int t1 = min((int)plus.size(), capA) + min((int)minus.size(), capB);
        int t2 = min((int)plus.size(), capB) + min((int)minus.size(), capA);
        bool full1 = (int)plus.size() <= capA && (int)minus.size() <= capB;
        bool full2 = (int)plus.size() <= capB && (int)minus.size() <= capA;
        int mu;
        if (full1) mu = 1;
        else if (full2) mu = -1;
        else mu = (t1 >= t2) ? 1 : -1;
        vector<int> inA, inB;
        if (mu == 1) {
            int ta = min((int)plus.size(), capA), tb = min((int)minus.size(), capB);
            inA.assign(plus.end() - ta, plus.end()); plus.resize(plus.size() - ta);
            inB.assign(minus.end() - tb, minus.end()); minus.resize(minus.size() - tb);
        } else {
            int ta = min((int)minus.size(), capA), tb = min((int)plus.size(), capB);
            inA.assign(minus.end() - ta, minus.end()); minus.resize(minus.size() - ta);
            inB.assign(plus.end() - tb, plus.end()); plus.resize(plus.size() - tb);
        }
        if (inA.empty() && inB.empty()) exit(0); // should not happen
        int par = takeParity();
        int D = doQuery(inA, inB, par);
        total += mu * D;
#ifdef LOCAL_SIM
        if (!plus.empty() || !minus.empty()) fprintf(stderr, "SPLIT\n");
#endif
    }
    return total;
}

// execute block bi using first j columns
static void runBlock(int bi, int j) {
    const Block& b = blocks[bi];
    vector<int> lbl(j);
    for (int c = 0; c < j; c++) { lbl[c] = pool.front(); pool.pop_front(); }
    vector<int> y(b.m, 0);
    vector<int> ord = b.order;
    stable_sort(ord.begin(), ord.end(), [&](int u, int v) {
        return rowKey(b, u, j) < rowKey(b, v, j);
    });
    for (int r : ord) {
        vector<pair<int,int>> coins;
        int corr = 0;
        for (int c = 0; c < j; c++) {
            int v = b.M[r][c];
            if (!v) continue;
            coins.push_back({lbl[c], v * b.flip[c]});
            if (b.flip[c] < 0) corr += v;
        }
        if (coins.empty()) { y[r] = 0; continue; }
        int R = measure(coins);
        y[r] = R + corr;
    }
    vector<int> xt = b.fam ? decodeUnsigned(b.k, y) : decodeSigned(b.k, y);
    for (int c = 0; c < j; c++) {
        int x = xt[c];
        if (b.flip[c] < 0) x = 1 - x;
        addKnown(lbl[c], x);
    }
}

static void countStep() {
    bool useA = As.size() >= Bs.size();
    const vector<int>& K = useA ? As : Bs;
    int s = K.size();
    int take = min<int>(s, pool.size());
    // take-1 middle coins, 1 parity coin
    vector<int> mid;
    for (int i = 0; i < take - 1; i++) { mid.push_back(pool.front()); pool.pop_front(); }
    int par = pool.front(); pool.pop_front();
    int D;
    if (useA) D = doQuery(mid, {}, par);
    else D = -doQuery({}, mid, par);
    int numB = D;
    countedA += (int)mid.size() - numB;
}

// ---------------- planning ----------------
static int countCost(double a, double b, int U, double f) {
    int q = 0;
    while (U > 0) {
        int k = max(1, (int)floor(max(a, b) + 1e-9));
        U -= k; q++;
        a += 1 - f; b += f;
    }
    return q;
}

struct Action { int type; int bi; int j; }; // type 0=count,1=block

// returns all first actions with their estimated total cost under growth fraction f
static vector<pair<Action,int>> evalActions(double f) {
    int a = As.size(), b = Bs.size();
    int U = pool.size();
    int X = min(U, 1500);
    vector<int> D(X + 1, INF);
    vector<pair<Action,int>> out;
    for (int x = X; x >= 0; x--) {
        double na = a + (1 - f) * x, nb = b + f * x;
        int rem = U - x;
        if (rem == 0) { D[x] = 0; continue; }
        int best = countCost(na, nb, rem, f);
        if (x == 0) out.push_back({{0, 0, 0}, best});
        for (int bi = 0; bi < (int)blocks.size(); bi++) {
            const Block& B = blocks[bi];
            if (B.n + B.m <= rem) {
                for (auto& [j, rows] : B.opts) {
                    int cc = j + (int)rows.size();
                    if (x + cc > X || D[x + cc] >= INF) continue;
                    if (x > 0 && (int)rows.size() + D[x + cc] >= best) continue;
                    int q = simulateRows(rows, na, nb, f);
                    if (q >= INF) continue;
                    int tot = q + D[x + cc];
                    if (x == 0) out.push_back({{1, bi, j}, tot});
                    best = min(best, tot);
                }
            } else {
                auto covered = [&](int jj) {
                    int nz = 0;
                    for (int r = 0; r < B.m; r++) if (B.preP[r][jj] + B.preQ[r][jj] > 0) nz++;
                    return make_pair(jj + nz, nz);
                };
                int lo = 1, hi = B.n;
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    if (covered(mid).first >= rem) hi = mid; else lo = mid + 1;
                }
                auto cv = covered(lo);
                if (x > 0 && cv.second >= best) continue;
                int q = simulateRows(blockRows(B, lo), na, nb, f);
                if (q >= INF) continue;
                int extra = max(0, rem - (lo + q));
                int tot = q + (extra ? countCost(na + lo + q, nb, extra, f) : 0);
                if (x == 0) out.push_back({{1, bi, lo}, tot});
                best = min(best, tot);
            }
        }
        D[x] = best;
    }
    return out;
}

#ifndef SIGMA_W
#define SIGMA_W 1.0
#endif
static Action decide() {
    int a = As.size(), b = Bs.size();
    double f0 = (b + 1.0) / (a + b + 1.0);
    double sd = sqrt(f0 * (1 - f0) / (a + b + 1.0)) * SIGMA_W;
    double fs[3] = {max(0.01, f0 - sd), f0, min(0.99, f0 + sd)};
    double ws[3] = {0.25, 0.5, 0.25};
    map<tuple<int,int,int>, pair<double,int>> acc;
    for (int i = 0; i < 3; i++) {
        auto v = evalActions(fs[i]);
        for (auto& [ac, c] : v) {
            auto& e = acc[{ac.type, ac.bi, ac.j}];
            e.first += ws[i] * c; e.second++;
        }
    }
    Action best{0, 0, 0}; double bc = 1e18;
    for (auto& [key, e] : acc) {
        if (e.second < 3) continue;
        if (e.first < bc - 1e-9) { bc = e.first; best = {get<0>(key), get<1>(key), get<2>(key)}; }
    }
    return best;
}

// ---------------- exact search for small n ----------------
namespace small {
int n, M;
bool identMode = false;
vector<vector<int>> qv;
vector<vector<int>> qseq;
vector<int> cntA;
unordered_map<string, int> memo[8];
string key(const vector<int>& S) { string s; for (int x : S) { s += char(x & 255); s += char(x >> 8); } return s; }
bool allSame(const vector<int>& S) { if (identMode) return S.size() == 1; for (int x : S) if (cntA[x] != cntA[S[0]]) return false; return true; }
bool solve(const vector<int>& S, int d, int* pick) {
    if (allSame(S)) return true;
    if (d == 0) return false;
    string k = key(S);
    if (!pick) { auto it = memo[d].find(k); if (it != memo[d].end()) return it->second; }
    set<vector<int>> seen;
    vector<pair<int,int>> cand;
    for (size_t q = 0; q < qv.size(); q++) {
        vector<int> sig; sig.reserve(S.size());
        for (int x : S) sig.push_back(qv[q][x]);
        { map<int,int> rn; for (int& v : sig) { auto it = rn.find(v); if (it == rn.end()) { int t = rn.size(); rn[v] = t; v = t; } else v = it->second; } }
        if (!seen.insert(sig).second) continue;
        map<int,int> cnt; for (int x : S) cnt[qv[q][x]]++;
        if (cnt.size() == 1) continue;
        int mx = 0; for (auto& p : cnt) mx = max(mx, p.second);
        cand.push_back({mx, (int)q});
    }
    sort(cand.begin(), cand.end());
    bool ok = false;
    for (auto [mx, q] : cand) {
        map<int, vector<int>> parts; for (int x : S) parts[qv[q][x]].push_back(x);
        bool good = true;
        for (auto& p : parts) if (!solve(p.second, d - 1, nullptr)) { good = false; break; }
        if (good) { ok = true; if (pick) *pick = q; break; }
    }
    memo[d][k] = ok;
    return ok;
}
void build(int nn) {
    n = nn; M = 1 << (n - 1);
    cntA.resize(M);
    for (int a = 0; a < M; a++) cntA[a] = n - __builtin_popcount(a);
    vector<int> seq;
    function<void(int)> rec = [&](int used) {
        if (seq.size() >= 2 && seq.front() < seq.back()) {
            vector<int> v(M);
            for (int a = 0; a < M; a++) {
                int r = 0;
                for (size_t i = 0; i + 1 < seq.size(); i++) {
                    int t1 = seq[i] ? (a >> (seq[i] - 1) & 1) : 0, t2 = seq[i + 1] ? (a >> (seq[i + 1] - 1) & 1) : 0;
                    r += t1 != t2;
                }
                v[a] = r;
            }
            qv.push_back(v); qseq.push_back(seq);
        }
        for (int i = 0; i < n; i++) if (!(used >> i & 1)) { seq.push_back(i); rec(used | 1 << i); seq.pop_back(); }
    };
    rec(0);
    { // dedup
        map<vector<int>, int> u; vector<vector<int>> q2, s2;
        for (size_t i = 0; i < qv.size(); i++) if (!u.count(qv[i])) { u[qv[i]] = 1; q2.push_back(qv[i]); s2.push_back(qseq[i]); }
        qv.swap(q2); qseq.swap(s2);
    }
}
int play(int d) {
    vector<int> S(M); iota(S.begin(), S.end(), 0);
    while (!solve(S, d, nullptr)) d++;
    while (!allSame(S)) {
        int q = -1;
        solve(S, d, &q);
        int r = ask(qseq[q]);
        vector<int> S2;
        for (int x : S) if (qv[q][x] == r) S2.push_back(x);
        if (S2.empty()) exit(0);
        S.swap(S2); d--;
    }
    return S[0];
}
void run(int nn) {
    build(nn);
    static const int optD[9] = {0, 0, 1, 1, 2, 2, 3, 3, 3};
    int x = play(optD[n]);
    answer(cntA[x]);
}
void opening(int m, int d) {
    identMode = true;
    build(m);
    int x = play(d);
    for (int i = 1; i < m; i++) addKnown(i, x >> (i - 1) & 1);
}
}

int main() {
#ifdef LOCAL_SIM
    simInit();
#endif
    n = readN();
    if (n <= SMALL_N) small::run(n);
    prepBlocks();
    As.push_back(0);
    int start = 1;
#ifndef OPEN_M
#define OPEN_M 7
#endif
    if (n >= OPEN_M + 2 && OPEN_M > 1) { small::opening(OPEN_M, 3); start = OPEN_M; }
    for (int i = start; i < n; i++) pool.push_back(i);
    bool committed = false;
    while (!pool.empty()) {
        if (committed) { countStep(); continue; }
        Action ac = decide();
        if (ac.type == 1) {
#ifdef LOCAL_SIM
            fprintf(stderr, "block fam=%d k=%d j=%d a=%zu b=%zu q=%lld\n", blocks[ac.bi].fam, blocks[ac.bi].k, ac.j, As.size(), Bs.size(), g_queries);
#endif
            runBlock(ac.bi, ac.j);
#ifdef LOCAL_SIM
            fprintf(stderr, "  -> after a=%zu b=%zu q=%lld\n", As.size(), Bs.size(), g_queries);
#endif
        } else {
            countStep();
            if (As.size() + Bs.size() >= 40) committed = true;
        }
    }
    answer((int)As.size() + (int)countedA);
}

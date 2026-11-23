// Coffee Varieties (hard version) - interactive
// Strategy: precomputed structured query plan (block-circulant passes + redundant
// query deletion + greedy cleanup), executed adaptively. Removed (duplicate)
// elements are skipped; an online greedy cleanup guarantees correctness.
#include <bits/stdc++.h>
using namespace std;
typedef vector<vector<int>> Plan;

static int N, K;

// ---------------------------------------------------------------- plan building
struct Cov {
    int n; vector<int> cnt;
    void init(int n_) { n = n_; cnt.assign((size_t)n * n, 0); }
    inline int& at(int a, int b) { return a < b ? cnt[(size_t)a * n + b] : cnt[(size_t)b * n + a]; }
};
static void computeCov(const Plan& P, int n, int k, Cov& C) {
    C.init(n);
    for (auto& s : P)
        for (size_t i = 0; i < s.size(); i++)
            for (size_t j = i + 1; j < s.size() && j <= i + (size_t)k; j++)
                if (s[i] != s[j]) C.at(s[i], s[j])++;
}
static long long planLen(const Plan& P) { long long t = 0; for (auto& s : P) t += s.size(); return t; }

static Plan buildBlock(int n, int k, int m, const vector<int>& S) {
    vector<vector<int>> B(m);
    for (int i = 0; i < n; i++) B[(long long)i * m / n].push_back(i);
    Plan P;
    for (int s : S) {
        vector<int> seg;
        for (int j = 0; j < m; j++) { int u = (int)((long long)j * s % m); for (int x : B[u]) seg.push_back(x); }
        int base = seg.size(); int need = min(k, base - 1);
        for (int t = 0; t < need; t++) seg.push_back(seg[t]);
        P.push_back(seg);
    }
    return P;
}
static void deleteRedundant(Plan& P, int n, int k, Cov& C, int rounds = 4) {
    for (int r = 0; r < rounds; r++) {
        bool changed = false;
        for (auto& s : P) {
            for (int p = (int)s.size() - 1; p >= 0; p--) {
                int L = s.size(); bool ok = true; int x = s[p];
                for (int q = max(0, p - k); q <= min(L - 1, p + k) && ok; q++) if (q != p && C.at(x, s[q]) < 2) ok = false;
                if (!ok) continue;
                for (int i = max(0, p - k); i < p && ok; i++) { int j = i + k + 1; if (j < L && s[i] == s[j]) ok = false; }
                if (!ok) continue;
                for (int q = max(0, p - k); q <= min(L - 1, p + k); q++) if (q != p) C.at(x, s[q])--;
                for (int i = max(0, p - k); i < p; i++) { int j = i + k + 1; if (j < L) C.at(s[i], s[j])++; }
                s.erase(s.begin() + p); changed = true;
            }
        }
        P.erase(remove_if(P.begin(), P.end(), [](const vector<int>& v) { return v.empty(); }), P.end());
        if (!changed) break;
    }
}
static void cleanupPlan(Plan& P, int n, int k, Cov& C) {
    int W = (n + 63) / 64;
    vector<vector<uint64_t>> U(n, vector<uint64_t>(W, 0)); vector<int> deg(n, 0); long long rem = 0;
    for (int a = 0; a < n; a++) for (int b = a + 1; b < n; b++)
        if (C.at(a, b) == 0) { U[a][b >> 6] |= 1ULL << (b & 63); U[b][a >> 6] |= 1ULL << (a & 63); deg[a]++; deg[b]++; rem++; }
    if (!rem) return;
    vector<int> seg; deque<int> win; vector<uint64_t> wm(W, 0);
    while (rem > 0) {
        int best = -1; long long bs = -1;
        for (int x = 0; x < n; x++) {
            if (wm[x >> 6] >> (x & 63) & 1) continue;
            long long g = 0; for (int w = 0; w < W; w++) g += __builtin_popcountll(U[x][w] & wm[w]);
            long long sc = g * 4096 + deg[x]; if (sc > bs) { bs = sc; best = x; }
        }
        if (best < 0) break;
        int x = best; seg.push_back(x);
        for (int y : win) C.at(x, y)++;
        for (int w = 0; w < W; w++) { uint64_t mm = U[x][w] & wm[w]; while (mm) { int y = w * 64 + __builtin_ctzll(mm); mm &= mm - 1; U[x][y >> 6] &= ~(1ULL << (y & 63)); U[y][x >> 6] &= ~(1ULL << (x & 63)); deg[x]--; deg[y]--; rem--; } }
        win.push_back(x); wm[x >> 6] |= 1ULL << (x & 63);
        if ((int)win.size() > k) { int y = win.front(); win.pop_front(); wm[y >> 6] &= ~(1ULL << (y & 63)); }
    }
    P.push_back(seg);
}

// table of block-circulant parameters {n,k,m,{steps}}
struct Entry { int n, k, m; vector<int> S; };
static const vector<Entry> TABLE = {
{4,2,4,{1}}, // L=5
{8,2,4,{3}}, // L=20
{8,4,8,{1}}, // L=11
{16,2,17,{3,1,5,4}}, // L=71
{16,4,16,{11}}, // L=50
{16,8,16,{5}}, // L=23
{32,2,37,{18,2,5,17,14,6,8,13,15}}, // L=299
{32,4,11,{7,6,10,2,8}}, // L=179
{32,8,8,{5,7}}, // L=109
{32,16,32,{13}}, // L=47
{64,2,67,{33,8,10,32,6,30,14,26,27,9,2,11,19,23,31,24,25}}, // L=1117
{64,4,67,{20,11,8,33,3,19,18,30,13,21}}, // L=671
{64,8,16,{5,7,11,9,13,1}}, // L=381
{64,16,8,{3,1}}, // L=221
{64,32,64,{25}}, // L=95
{128,2,131,{20,7,59,47,38,31,34,32,5,27,45,3,30,18,2,39,11,44,58,21,49,57,23,65,12,8,14,48,61,51,56,25,26}}, // L=4284
{128,4,131,{48,3,60,7,23,43,19,25,42,65,45,29,18,49,26,10,17,8}}, // L=2365
{128,8,64,{43,5,55,35,3,25,45,17,31}}, // L=1392
{128,16,16,{7,13,13,3,1,5}}, // L=760
{128,32,13,{3,12,4}}, // L=443
{128,64,128,{15}}, // L=191
{256,8,289,{5,182,90,216,226,46,233,12,148,158,260,39,267,192,80,165,114}}, // L=4809
{256,16,95,{49,74,51,24,86,39,64,6,4,54}}, // L=2782
{256,32,16,{11,13,7,13,1,1}}, // L=1528
{256,64,13,{3,9,1}}, // L=886
{256,128,256,{59}}, // L=383
{512,32,111,{10,49,77,70,7,16,11,1,65}}, // L=5386
{512,64,37,{17,15,16,2,12,9}}, // L=3251
{512,128,13,{5,6,2}}, // L=1775
{512,256,512,{105}}, // L=767
{1024,128,37,{4,3,7,13,5,18}}, // L=6524
{1024,256,13,{4,3,1}}, // L=3545
{1024,512,1024,{79}}, // L=1535
};

static Plan zigzagK1(int n) {
    Plan P;
    for (int c = 0; c < n / 2; c++) {
        vector<int> seg; seg.push_back(c);
        for (int i = 1; (int)seg.size() < n; i++) {
            seg.push_back(((c + i) % n + n) % n);
            if ((int)seg.size() < n) seg.push_back(((c - i) % n + n) % n);
        }
        P.push_back(seg);
    }
    return P;
}

// ---------------------------------------------------------------- SA for small cases
static chrono::steady_clock::time_point T_START;
static double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T_START).count(); }

struct SASolver {
    int n, k, L; vector<int> seq, cnt, uncPos, uncList; mt19937_64 rng;
    inline void markCov(int id) { int p = uncPos[id]; int last = uncList.back(); uncList[p] = last; uncPos[last] = p; uncList.pop_back(); uncPos[id] = -1; }
    inline void markUnc(int id) { uncPos[id] = uncList.size(); uncList.push_back(id); }
    inline void inc(int x, int y) { int id = x < y ? x * n + y : y * n + x; if (cnt[id]++ == 0) markCov(id); }
    inline void dec(int x, int y) { int id = x < y ? x * n + y : y * n + x; if (--cnt[id] == 0) markUnc(id); }
    void addpos(int p, int s) { int x = seq[p]; for (int q = max(0, p - k); q <= min(L - 1, p + k); q++) if (q != p) { if (s > 0) inc(x, seq[q]); else dec(x, seq[q]); } }
    int deltaChange(int p, int y) {
        int x = seq[p]; int d = 0;
        for (int q = max(0, p - k); q <= min(L - 1, p + k); q++) if (q != p) {
            int z = seq[q]; if (z == y) return INT_MAX;
            int a = min(x, z), b = max(x, z); if (cnt[a * n + b] == 1) d++;
            a = min(y, z); b = max(y, z); if (cnt[a * n + b] == 0) d--;
        }
        return d;
    }
    // try to find a covering sequence of length L within time limit
    bool run(int n_, int k_, int L_, long long iters, double deadline, uint64_t seed) {
        n = n_; k = k_; L = L_; rng.seed(seed);
        cnt.assign(n * n, 0); uncPos.assign(n * n, -1); uncList.clear();
        for (int x = 0; x < n; x++) for (int y = x + 1; y < n; y++) markUnc(x * n + y);
        seq.resize(L);
        for (int i = 0; i < L; i++) seq[i] = (int)((long long)i * (2 * (i / n) + 1) % n);
        for (int i = 0; i < L; i++) {
            for (int q = max(0, i - k); q < i; q++) if (seq[q] == seq[i]) { seq[i] = rng() % n; q = max(0, i - k) - 1; }
        }
        for (int p = 0; p < L; p++) { int x = seq[p]; for (int q = p + 1; q <= min(L - 1, p + k); q++) inc(x, seq[q]); }
        double T0 = 1.0, T1 = 0.01;
        for (long long it = 0; it < iters && !uncList.empty(); it++) {
            if ((it & 4095) == 0 && elapsed() > deadline) return false;
            double T = T0 * pow(T1 / T0, (double)it / iters);
            int p, y;
            if (rng() % 2) { p = rng() % L; y = rng() % n; }
            else {
                int id = uncList[rng() % uncList.size()]; int a = id / n, b = id % n; if (rng() % 2) swap(a, b);
                int tries = 0; do { p = rng() % L; tries++; } while (seq[p] != a && tries < 4 * n);
                if (seq[p] != a) continue;
                int off = (int)(rng() % (2 * k)) - k; if (off >= 0) off++;
                p += off; if (p < 0 || p >= L) continue; y = b;
            }
            if (seq[p] == y) continue;
            int d = deltaChange(p, y); if (d == INT_MAX) continue;
            if (d <= 0 || exp(-d / T) > (double)(rng() % 1000000) / 1e6) { addpos(p, -1); seq[p] = y; addpos(p, 1); }
        }
        return uncList.empty();
    }
};

// block-level designs (symbol sequences), blown up by replacing each symbol by a block.
// Oriented designs encode symbol*2+reversedFlag.
struct Design { bool oriented; vector<int> seq; };
static const vector<Design> DESIGNS = {
{false,{0,7,6,1,4,2,5,1,4,7,3,2,4,0,6,3,1,0,5,2,6,7,5,3}}, // m=8 w=1 L=24
{false,{11,4,0,5,11,12,3,5,7,6,0,3,10,11,8,6,2,9,12,4,1,14,5,8,15,1,0,8,7,11,6,15,7,4,3,5,6,13,4,5,9,1,6,3,7,1,2,11,13,1,10,12,0,2,15,10,5,2,4,8,15,13,12,8,15,14,11,9,15,3,8,9,10,7,13,9,0,14,13,3,2,14,7,12,6,14,10,4,6}}, // m=16 w=1 L=89
{false,{14,2,9,13,8,15,10,0,12,5,11,14,15,7,8,1,2,9,0,6,13,7,11,12,10,5,9,2,3,4,11,12,8,1,6,5,3,13,4,1,15,6,10,14,3,4,7,0}}, // m=16 w=3 L=48
};
static int designM(const Design& d) {
    int mx = *max_element(d.seq.begin(), d.seq.end());
    return d.oriented ? mx / 2 + 1 : mx + 1;
}
static Plan blowUp(int n, const Design& d) {
    int m = designM(d);
    vector<vector<int>> B(m);
    for (int i = 0; i < n; i++) B[(long long)i * m / n].push_back(i);
    Plan P(1);
    for (int v : d.seq) {
        int u = d.oriented ? v / 2 : v; bool rev = d.oriented && (v & 1);
        if (!rev) for (int e : B[u]) P[0].push_back(e);
        else for (int i = (int)B[u].size() - 1; i >= 0; i--) P[0].push_back(B[u][i]);
    }
    return P;
}
static void finishPlan(Plan& P, int n, int k) {
    Cov C; computeCov(P, n, k, C);
    deleteRedundant(P, n, k, C);
    cleanupPlan(P, n, k, C);
    deleteRedundant(P, n, k, C);
}

static Plan makePlan(int n, int k) {
    Plan P;
    if (n <= 1) return P;
    if (k >= n) { vector<int> s(n); iota(s.begin(), s.end(), 0); P.push_back(s); return P; }
    if (k == 1) return zigzagK1(n);
    const Entry* e = nullptr;
    for (auto& t : TABLE) if (t.n == n && t.k == k) { e = &t; break; }
    Cov C;
    if (e) {
        P = buildBlock(n, k, e->m, e->S);
        computeCov(P, n, k, C);
        deleteRedundant(P, n, k, C);
        cleanupPlan(P, n, k, C);
        deleteRedundant(P, n, k, C);
    } else {
        // generic fallback: block circulant with step 1..; then cleanup
        int m = max(3, 2 * n / k + 1);
        vector<int> S;
        for (int s = 1; s <= m / 2; s++) if (__gcd(s, m) == 1) S.push_back(s);
        P = buildBlock(n, k, m, S);
        computeCov(P, n, k, C);
        deleteRedundant(P, n, k, C);
        cleanupPlan(P, n, k, C);
        deleteRedundant(P, n, k, C);
    }
    // blow-up candidates
    for (auto& d : DESIGNS) {
        int m = designM(d);
        if (m > n || n % m) continue;
        int b = n / m; int t = (long long)m * k / n;
        if (t < 2 || t * b != k) continue;
        if ((long long)d.seq.size() * b >= planLen(P)) continue;
        Plan Q = blowUp(n, d);
        finishPlan(Q, n, k);
        if (planLen(Q) < planLen(P)) P = Q;
    }
    // SA improvement for small instances
    if (n <= 64 && 2 * k < n) {
        long long best = planLen(P);
        double deadline = 4.0;
        SASolver sa;
        int L = (int)best - 1;
        int fails = 0;
        while (elapsed() < deadline && L > 0 && fails < 3) {
            if (sa.run(n, k, L, 6000000, deadline, 1234567ULL + L * 7919ULL + fails)) {
                P.clear(); P.push_back(sa.seq); best = L; L--; fails = 0;
            } else fails++;
        }
    }
    return P;
}

// ---------------------------------------------------------------- interaction
static int W;
static vector<char> alive;
static vector<vector<uint64_t>> certified; // certified distinct pairs
static vector<int> remUnc;                 // #alive partners not certified
struct MemEntry { int x; bool removed; vector<uint64_t> M; };
static deque<MemEntry> mem;
static vector<int> inMemCnt;
static long long queries = 0;

static bool isCert(int a, int b) { return certified[a][b >> 6] >> (b & 63) & 1; }
static void setCert(int a, int b) {
    if (isCert(a, b)) return;
    certified[a][b >> 6] |= 1ULL << (b & 63); certified[b][a >> 6] |= 1ULL << (a & 63);
    if (alive[a] && alive[b]) { remUnc[a]--; remUnc[b]--; }
}
static void doReset() {
    if (mem.empty()) return;
    printf("R\n"); fflush(stdout);
    for (auto& e : mem) inMemCnt[e.x]--;
    mem.clear();
}
// is x blocked (a "Y" for it would be ambiguous)?
static bool blocked(int x) {
    for (auto& e : mem) if (e.removed && (e.M[x >> 6] >> (x & 63) & 1)) return true;
    return false;
}
// may we query x now? (strict: also not blocked)
static bool allowed(int x) {
    if (!alive[x]) return false;
    if (inMemCnt[x] > 0) return false;
    return !blocked(x);
}
static void doQuery(int x) {
    bool amb = blocked(x);
    printf("? %d\n", x + 1); fflush(stdout);
    char buf[16];
    if (scanf("%15s", buf) != 1) exit(0);
    queries++;
    if (buf[0] == 'E') exit(0);
    MemEntry e; e.x = x; e.removed = false;
    if (buf[0] == 'N') {
        for (auto& m : mem) setCert(x, m.x);
    } else if (amb) {
        // ambiguous: x may equal a removed element whose only alive twin is x; keep x alive
    } else {
        // duplicate of something in memory -> remove x
        e.removed = true;
        e.M.assign(W, 0);
        for (auto& m : mem) {
            if (!m.removed) e.M[m.x >> 6] |= 1ULL << (m.x & 63);
            else for (int w = 0; w < W; w++) e.M[w] |= m.M[w];
        }
        alive[x] = false;
        for (int y = 0; y < N; y++) if (y != x && alive[y] && !isCert(x, y)) remUnc[y]--;
        remUnc[x] = 0;
    }
    mem.push_back(std::move(e)); inMemCnt[x]++;
    if ((int)mem.size() > K) { inMemCnt[mem.front().x]--; mem.pop_front(); }
}
static bool hasUncInMem(int x) {
    for (auto& m : mem) if (!m.removed && alive[m.x] && m.x != x && !isCert(x, m.x)) return true;
    return false;
}

int main() {
    T_START = chrono::steady_clock::now();
    if (scanf("%d %d", &N, &K) != 2) return 0;
    if (N == 1) { printf("! 1\n"); fflush(stdout); return 0; }
    Plan P = makePlan(N, K);
    W = (N + 63) / 64;
    alive.assign(N, 1); certified.assign(N, vector<uint64_t>(W, 0)); remUnc.assign(N, N - 1); inMemCnt.assign(N, 0);

    for (auto& seg : P) {
        doReset();
        int L = seg.size();
        for (int p = 0; p < L; p++) {
            int x = seg[p];
            if (!alive[x]) continue;
            if (remUnc[x] == 0) continue;
            if (inMemCnt[x] > 0) continue;
            if (!allowed(x)) continue;
            // usefulness: an uncertified alive partner in memory or among next K planned alive elements
            bool useful = hasUncInMem(x);
            if (!useful) {
                int seen = 0;
                for (int q = p + 1; q < L && seen < K; q++) {
                    int y = seg[q]; if (!alive[y]) continue; seen++;
                    if (y != x && !isCert(x, y)) { useful = true; break; }
                }
            }
            if (!useful) continue;
            doQuery(x);
        }
    }
    // online greedy cleanup
    while (true) {
        long long remPairs = 0; for (int x = 0; x < N; x++) if (alive[x]) remPairs += remUnc[x];
        if (remPairs == 0) break;
        // choose next element maximizing uncertified alive partners in memory
        int best = -1; long long bs = -1;
        for (int x = 0; x < N; x++) {
            if (!alive[x] || remUnc[x] == 0 || !allowed(x)) continue;
            long long g = 0;
            for (auto& m : mem) if (!m.removed && alive[m.x] && !isCert(x, m.x)) g++;
            long long sc = g * 4096 + remUnc[x];
            if (sc > bs) { bs = sc; best = x; }
        }
        if (best < 0 || bs < 4096) {
            // nothing gains from current memory: if memory nonempty, reset and retry
            if (!mem.empty()) { doReset(); continue; }
            if (best < 0) break; // should not happen
        }
        doQuery(best);
    }
    int d = 0; for (int x = 0; x < N; x++) if (alive[x]) d++;
    printf("! %d\n", d); fflush(stdout);
    return 0;
}

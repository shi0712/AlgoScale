#include <bits/stdc++.h>
using namespace std;

#ifdef SIM
// ---------------- local simulator ----------------
static vector<int> SP;   // secret permutation, 1-based
static long long SQ = 0; // query count
static bool SDONE = false, SOK = false;
struct Done {};
#endif

static int n, N;
static vector<vector<pair<int, int>>> adj;
static vector<long long> dv;     // lcm of results on element (capped at N+1)
static vector<char> nz;          // proven nonzero
static vector<char> touched;
static vector<int> pool;         // touched elements
static mt19937_64 rng;

static long long lcmcap(long long a, long long b) {
    long long g = std::gcd(a, b);
    long long r = a / g * b;
    if (r > N) r = N + 1;
    return r;
}

[[noreturn]] static void answer(int x, int y) {
#ifdef SIM
    bool ok = (SP[x] == 0 || SP[y] == 0);
    SDONE = true; SOK = ok;
    throw Done{};
#else
    printf("! %d %d\n", x, y);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    exit(0);
#endif
}

static void touch(int e) {
    if (!touched[e]) { touched[e] = 1; pool.push_back(e); }
}

static double logLR(int x);
static vector<double> lrc;
static set<pair<long long, int>, greater<pair<long long, int>>> byD;
static set<pair<double, int>, greater<pair<double, int>>> byLR;
static int ask(int i, int j) {
    int g;
#ifdef SIM
    SQ++;
    g = std::gcd(SP[i], SP[j]);
#else
    printf("? %d %d\n", i, j);
    fflush(stdout);
    if (scanf("%d", &g) != 1) exit(0);
    if (g == -1) exit(0);
#endif
    adj[i].push_back({j, g});
    adj[j].push_back({i, g});
    touch(i); touch(j);
    for (int e : {i, j}) {
        byD.erase({dv[e], e});
        byLR.erase({lrc[e], e});
        dv[e] = lcmcap(dv[e], g);
        lrc[e] = logLR(e);
        byD.insert({dv[e], e});
        if (!nz[e]) byLR.insert({lrc[e], e});
    }
    if (2LL * g > N) answer(i, j);
    if (dv[i] > N) answer(i, i);
    if (dv[j] > N) answer(j, j);
    return g;
}

// Sound check: returns true if x is proven nonzero.
static bool checkNZ(int x) {
    if (nz[x]) return true;
    auto &A = adj[x];
    // if x == 0, results with distinct partners are distinct values p_r
    if (A.size() > 1) {
        vector<int> gs;
        gs.reserve(A.size());
        for (auto &pr : A) gs.push_back(pr.second);
        sort(gs.begin(), gs.end());
        for (size_t i = 1; i < gs.size(); i++)
            if (gs[i] == gs[i - 1]) { nz[x] = 1; return true; }
    }
    for (auto &pr : A) {
        int r = pr.first, g = pr.second;
        // if x == 0 then p_r == g, so every other result on r divides g
        for (auto &q : adj[r]) {
            if (q.first == x) continue;
            if (g % q.second != 0) { nz[x] = 1; return true; }
        }
    }
    return false;
}

// approximate log likelihood ratio of "x is zero"
static vector<double> sumLogG;
static vector<int> qcnt;
static double LOG06N;
static double logLR(int x) {
    // LR = prod(g_i) * d_x / (0.6N)^k
    double s = 0;
    int k = 0;
    long long D = 1;
    for (auto &pr : adj[x]) { s += log((double)pr.second); k++; D = lcmcap(D, pr.second); }
    return s + log((double)D) - k * LOG06N;
}

static double pbig(long long e) {
    long long tot = N / e;
    long long big = tot - N / (2 * e);
    if (tot <= 0) return 0;
    return (double)big / tot;
}

static vector<int> mark;
static int markStamp = 0;

// choose a reference r for testing x; -1 if none

static double postOf(int e) {
    if (nz[e]) return 0;
    double LR = exp(min(lrc[e], 50.0));
    return LR / (LR + n);
}
static double LAMBDA = 0.5;
static int chooseRef(int x, double q, double R) {
    markStamp++;
    mark[x] = markStamp;
    for (auto &pr : adj[x]) mark[pr.first] = markStamp;
    long long D = dv[x];
    double pbD = pbig(D);
    double best = -1e18;
    int bestR = -1;
    double cnt = 0;
    static vector<int> cand;
    cand.clear();
    {
        int c = 0;
        for (auto it = byD.begin(); it != byD.end() && c < 100; ++it) {
            int r = it->second;
            if (mark[r] == markStamp) continue;
            cand.push_back(r); c++;
        }
        c = 0;
        for (auto it = byLR.begin(); it != byLR.end() && c < 40;) {
            int r = it->second;
            if (nz[r]) { it = byLR.erase(it); continue; }
            ++it;
            if (mark[r] == markStamp) continue;
            cand.push_back(r); c++;
        }
    }
    for (int r : cand) {
        long long e = dv[r];
        long long L = lcmcap(D, e);
        double val;
        if (L > N) val = q * R + (1 - q);
        else {
            double amb = (double)D / (double)L;
            val = q * pbig(e) * R + (1 - q) * (1 - amb);
        }
        double qr = postOf(r);
        if (qr > 0) {
            if (L > N) val += LAMBDA * qr * R;
            else val += LAMBDA * qr * pbD * R;
        }
        if (val > best + 1e-12) { best = val; bestR = r; cnt = 1; }
        else if (val > best - 1e-12) {
            cnt += 1;
            if ((double)(rng() % 1000000) / 1000000.0 * cnt < 1.0) bestR = r;
        }
    }
    return bestR;
}

// one test query on x. returns false if no reference available.
static bool testOnce(int x, double q, double R) {
    int r = chooseRef(x, q, R);
    if (r < 0) return false;
    ask(x, r);
    return true;
}

static double BETA = -1;
#ifdef SIM
static long long ZPOS=0, PH1=0, SF=0, STZ=0, SMISS=0, SMC=0, SMS=-1;
#endif

// ---------- exact-state solver for small n ----------
static double HK = 0.5;
static int SMALLN = 10;
static double leafH(int c) {
    if (c <= 2) return 0;
    return max(1.0, HK * log2(c / 2.0));
}
static double evalD1(const vector<uint8_t> &S, const vector<uint8_t> &zp, const vector<int> &idx, int i, int j, int G[16][16]) {
    long long c[16] = {0};
    int m[16] = {0};
    for (int k : idx) {
        const uint8_t *q = S.data() + (size_t)k * n;
        int g = G[q[i]][q[j]];
        c[g]++;
        m[g] |= 1 << zp[k];
    }
    double v = 1;
    for (int g = 0; g < 16; g++)
        if (c[g]) v += (double)c[g] / idx.size() * leafH(__builtin_popcount(m[g]));
    return v;
}
static void smallSolve() {
    // S: flat list of consistent permutations (values of positions 0..n-1)
    vector<uint8_t> S;
    vector<uint8_t> zp; // zero position per perm
    {
        vector<uint8_t> p(n);
        for (int i = 0; i < n; i++) p[i] = i;
        do {
            S.insert(S.end(), p.begin(), p.end());
            for (int i = 0; i < n; i++) if (p[i] == 0) { zp.push_back(i); break; }
        } while (next_permutation(p.begin(), p.end()));
    }
    int G[16][16];
    for (int a = 0; a < n; a++) for (int b = 0; b < n; b++) G[a][b] = std::gcd(a, b);
    bool first = true;
    while (true) {
        size_t M = zp.size();
        int zm = 0;
        for (size_t k = 0; k < M; k++) zm |= 1 << zp[k];
        if (__builtin_popcount(zm) <= 2) {
            int a = __builtin_ctz(zm);
            int rest = zm & (zm - 1);
            int b = rest ? __builtin_ctz(rest) : a;
            answer(a + 1, b + 1);
        }
        int bi = 0, bj = 1;
        if (!first) {
            int pairs = n * (n - 1) / 2;
            bool deep = (double)pairs * pairs * M <= 6e7;
            double best = 1e18;
            int cnt = 0;
            vector<int> all(M);
            iota(all.begin(), all.end(), 0);
            for (int i = 0; i < n; i++)
                for (int j = i + 1; j < n; j++) {
                    double v;
                    if (!deep) v = evalD1(S, zp, all, i, j, G);
                    else {
                        vector<int> grp[16];
                        for (int k : all) grp[G[S[(size_t)k * n + i]][S[(size_t)k * n + j]]].push_back(k);
                        v = 1;
                        for (int g = 0; g < 16; g++) {
                            if (grp[g].empty()) continue;
                            int gm = 0;
                            for (int k : grp[g]) gm |= 1 << zp[k];
                            if (__builtin_popcount(gm) <= 2) continue;
                            double b2 = 1e18;
                            for (int a = 0; a < n; a++)
                                for (int b = a + 1; b < n; b++)
                                    b2 = min(b2, evalD1(S, zp, grp[g], a, b, G));
                            v += (double)grp[g].size() / M * b2;
                        }
                    }
                    if (v < best - 1e-9) { best = v; bi = i; bj = j; cnt = 1; }
                    else if (v < best + 1e-9) { cnt++; if (rng() % cnt == 0) { bi = i; bj = j; } }
                }
        }
        first = false;
        int g = ask(bi + 1, bj + 1);
        size_t w = 0;
        for (size_t k = 0; k < M; k++) {
            const uint8_t *q = S.data() + k * n;
            if (G[q[bi]][q[bj]] == g) {
                if (w != k) { memmove(S.data() + w * n, q, n); zp[w] = zp[k]; }
                w++;
            }
        }
        S.resize(w * n);
        zp.resize(w);
    }
}


static void solve() {
    N = n - 1;
    if (n == 2) answer(1, 2);
    double beta = BETA > 0 ? BETA : (n <= 300 ? 0.4 : 0.7);
    adj.assign(n + 1, {});
    dv.assign(n + 1, 1);
    nz.assign(n + 1, 0);
    touched.assign(n + 1, 0);
    mark.assign(n + 1, 0);
    pool.clear();
    byD.clear(); byLR.clear();
    lrc.assign(n + 1, 0);
    LOG06N = log(0.6 * N);
    if (n <= SMALLN) { smallSolve(); }
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 1);
    shuffle(ord.begin(), ord.end(), rng);
    int P = n / 2;
#ifdef SIM
    if (getenv("FORCEW")) {
        int w = atoi(getenv("FORCEW"));
        int zi = -1; for (int i = 0; i < n; i++) if (SP[ord[i]] == 0) zi = i;
        if (zi < 2 * (n / 2)) {
            int part = ord[zi ^ 1];
            int wpos = -1; for (int i = 1; i <= n; i++) if (SP[i] == w) wpos = i;
            swap(SP[part], SP[wpos]);
        }
    }
    for(int i=0;i<n;i++) if(SP[ord[i]]==0) ZPOS=i/2+1;
    PH1=-1;
#endif
    for (int k = 0; k < P; k++) {
        int x = ord[2 * k], y = ord[2 * k + 1];
        ask(x, y);
        double R = P - 1 - k; // remaining pair queries
        // test candidates in this pair while worthwhile
        for (int it = 0; it < 64; it++) {
            int c = -1;
            double bl = -1e18;
            for (int z : {x, y}) {
                if (checkNZ(z)) continue;
                double l = logLR(z);
                if (l > bl) { bl = l; c = z; }
            }
            if (c < 0) break;
            double LR = exp(min(bl, 50.0));
            double post = LR / (LR + n);
            if (post * R < beta) break;
            if (!testOnce(c, post, R)) break;
#ifdef SIM
            if (k + 1 < ZPOS) SF++; else if (k + 1 == ZPOS) STZ++;
#endif
        }
#ifdef SIM
        if (k + 1 == ZPOS) { SMISS++; SMS = SQ; if(getenv("DBG")) fprintf(stderr,"miss k=%d R=%.0f w=%d tests=%d\n",k,R,max(SP[x],SP[y]),(int)adj[x].size()+(int)adj[y].size()-2); }
#endif
    }
#ifdef SIM
    PH1=SQ;
#endif
    touch(ord[n - 1]);
    // phase 2: best-first over all candidates
    // lazy priority queue
    priority_queue<pair<double, int>> pq;
    vector<double> cur(n + 1, 0);
    for (int e = 1; e <= n; e++) {
        if (checkNZ(e)) continue;
        cur[e] = logLR(e);
        pq.push({cur[e], e});
    }
    vector<char> exhausted(n + 1, 0);
    while (!pq.empty()) {
        auto [l, x] = pq.top();
        pq.pop();
        if (exhausted[x] || nz[x]) continue;
        if (checkNZ(x)) continue;
        double nl = logLR(x);
        if (fabs(nl - l) > 1e-9) { pq.push({nl, x}); continue; }
        double LR = exp(min(nl, 50.0));
        double post = LR / (LR + n);
        if (!testOnce(x, post, 1.0 * n)) { exhausted[x] = 1; continue; }
        // re-push x and the reference's neighbors' states may change; re-push x
        int r = adj[x].back().first;
        pq.push({logLR(x), x});
        if (!nz[r] && !checkNZ(r)) pq.push({logLR(r), r});
    }
    // should never get here; fallback
    answer(1, 1);
}

#ifdef SIM
int main(int argc, char **argv) {
    int nn = argc > 1 ? atoi(argv[1]) : 20000;
    int T = argc > 2 ? atoi(argv[2]) : 100;
    if (argc > 3) BETA = atof(argv[3]);
    if (argc > 5) LAMBDA = atof(argv[5]);
    if (argc > 6) SMALLN = atoi(argv[6]);
    if (argc > 7) HK = atof(argv[7]);
    unsigned long long seed = argc > 4 ? atoll(argv[4]) : 12345;
    mt19937_64 jr(seed * 7 + 1);
    rng.seed(seed);
    double tot = 0; long long mx = 0; int bad = 0; double ovs=0; double tmax=0;
    for (int t = 0; t < T; t++) {
        n = nn;
        SP.assign(n + 1, 0);
        vector<int> v(n);
        iota(v.begin(), v.end(), 0);
        shuffle(v.begin(), v.end(), jr);
        for (int i = 1; i <= n; i++) SP[i] = v[i - 1];
        SQ = 0;
        auto t0 = chrono::steady_clock::now();
        try { solve(); } catch (Done &) {}
        tmax = max(tmax, chrono::duration<double>(chrono::steady_clock::now() - t0).count());
        if (!SOK) bad++;
        if (SMS >= 0) SMC += SQ - SMS; SMS = -1;
        long long over = SQ - min<long long>(ZPOS, n/2); ovs += over; if (over > 300 && 0) printf("  big over=%lld zpos=%lld ph1=%lld q=%lld\n", over, ZPOS, PH1, SQ);
        tot += SQ; mx = max(mx, SQ);
    }
    printf("F=%.2f TZ=%.2f miss=%.4f misscost=%.1f\n", (double)SF/T,(double)STZ/T,(double)SMISS/T,(double)SMC/T);
    printf("tmax=%.3fs\n", tmax);
    printf("n=%d avg=%.2f (n/4=%.1f) max=%lld bad=%d over=%.2f\n", nn, tot / T, nn / 4.0, mx, bad, ovs/T);
}
#else
int main() {
    rng.seed(chrono::steady_clock::now().time_since_epoch().count() ^ (unsigned long long)random_device{}());
    if (scanf("%d", &n) != 1) return 0;
    solve();
}
#endif

#include <bits/stdc++.h>
using namespace std;

// ---------------- parameters ----------------
static vector<double> PK = {0.02, 0.07, 0.2, 0.5};
static vector<double> PIW = {1.0, 1.0, 1.0, 1.0};
static double EPS = 1e-3;
static double ALPHA = 1.0;
static double PRUNE = 1e-12;
static double PRUNE_EXACT = 1e-25;
static int TWO = 1;
static double GAMMA = 1.7;
static int SIDEMODE = 0;
static int SHARE = 1;
static int XFER = 1;
static double DENSETH = 2;
static bool traceOn = false;

// ---------------- interaction ----------------
struct Interactor {
    virtual int flip(int i, int j) = 0;  // returns new count, or -1 => terminate
    virtual ~Interactor() {}
};

struct RealIO : Interactor {
    int flip(int i, int j) override {
        printf("%d %d\n", i, j);
        fflush(stdout);
        int c;
        if (scanf("%d", &c) != 1) return -1;
        return c;
    }
};

static int K;
static vector<vector<vector<double>>> vpri;  // [k][len][v]
static vector<vector<double>> powp, powq;    // powp[k][c] = p^c, powq[k][c] = (1-p)^c

static double coverProb(int t, double p) {
    if (t == 0) return 1.0;
    vector<double> dp(t + 1, 0.0), nd(t + 1);
    dp[0] = 1.0;
    for (int a = 1; a <= t; a++) {
        fill(nd.begin(), nd.end(), 0.0);
        double none = pow(1 - p, t - a + 1);
        for (int r = 0; r <= t; r++) {
            if (dp[r] == 0) continue;
            if (r >= a) nd[r] += dp[r] * none;
            for (int b = a; b <= t; b++) nd[max(r, b)] += dp[r] * p * pow(1 - p, t - b);
        }
        swap(dp, nd);
    }
    return dp[t];
}

static void precompute() {
    K = PK.size();
    vpri.assign(K, vector<vector<double>>(11));
    powp.assign(K, vector<double>(12));
    powq.assign(K, vector<double>(12));
    for (int k = 0; k < K; k++) {
        double p = PK[k];
        for (int c = 0; c < 12; c++) powp[k][c] = pow(p, c), powq[k][c] = pow(1 - p, c);
        vector<double> cov(11);
        for (int t = 0; t <= 10; t++) cov[t] = coverProb(t, p);
        for (int len = 0; len <= 9; len++) {
            vector<double>& V = vpri[k][len];
            V.assign(1 << len, 0.0);
            int total = len * (len + 1) / 2;
            for (int v = 0; v < (1 << len); v++) {
                int inner = 0;
                double pr = 1;
                int run = 0;
                for (int i = 0; i <= len; i++) {
                    bool covered = (i < len) && !((v >> i) & 1);
                    if (covered) run++;
                    else {
                        if (run) { inner += run * (run + 1) / 2; pr *= cov[run]; }
                        run = 0;
                    }
                }
                pr *= pow(1 - p, total - inner);
                V[v] = pr;
            }
        }
    }
}

static inline double tprior(int k, int s, int T) {
    int c = __builtin_popcount(T);
    return powp[k][c] * powq[k][s - c];
}

// One-sided model. Coordinates are relative to its own end:
// T bit b <-> barrier covering own end cell up to distance b (bit s-1 = whole interval, shared "e")
// v bit i <-> cell at distance i+1 from own end (coverage by barriers not in T, at reference state)
struct Model {
    int s = 0, S = 0;
    vector<uint8_t> ek;
    vector<uint16_t> eZ, ev;
    vector<double> ew;
    vector<double> Q;
    double tot = 0;

    static inline int obsv(int off, int Tcur, int v) {
        if (Tcur == 0) return off + 1 + __builtin_popcount(v);
        int m = 31 - __builtin_clz(Tcur);
        return off + __builtin_popcount(v >> m);
    }

    void clear() { ek.clear(); eZ.clear(); ev.clear(); ew.clear(); }
    void push(int k, int Z, int v, double w) {
        ek.push_back(k); eZ.push_back(Z); ev.push_back(v); ew.push_back(w);
    }

    void prune(bool exact) {
        double mx = 0;
        for (double w : ew) mx = max(mx, w);
        double thr = mx * (exact ? PRUNE_EXACT : PRUNE);
        size_t j = 0;
        double t = 0;
        for (size_t i = 0; i < ew.size(); i++) {
            if (ew[i] > thr && ew[i] > 0) {
                ek[j] = ek[i]; eZ[j] = eZ[i]; ev[j] = ev[i]; ew[j] = ew[i];
                t += ew[i];
                j++;
            }
        }
        ek.resize(j); eZ.resize(j); ev.resize(j); ew.resize(j);
        if (t > 0) for (double& w : ew) w /= t;
    }

    void initFresh(int sz, int off, int c, bool exact) {
        s = sz; S = 0;
        clear();
        int vb = s - 1;
        for (int k = 0; k < K; k++)
            for (int T = 0; T < (1 << s); T++) {
                double tp = PIW[k] * tprior(k, s, T);
                for (int v = 0; v < (1 << vb); v++) {
                    if (obsv(off, T, v) != c) continue;
                    double w = tp * vpri[k][vb][v];
                    if (w > 0) push(k, T, v, w);
                }
            }
        prune(exact);
    }

    // returns false on contradiction
    double ratio[8];
    bool filter(int off, int c, bool& changed, bool exact) {
        size_t j = 0;
        double before[8] = {0}, after[8] = {0};
        for (size_t i = 0; i < ew.size(); i++) {
            before[ek[i]] += ew[i];
            if (obsv(off, eZ[i] ^ S, ev[i]) == c) {
                ek[j] = ek[i]; eZ[j] = eZ[i]; ev[j] = ev[i]; ew[j] = ew[i];
                after[ek[i]] += ew[i];
                j++;
            } else changed = true;
        }
        for (int k = 0; k < K; k++) ratio[k] = before[k] > 0 ? max(after[k] / before[k], 1e-12) : 1.0;
        ek.resize(j); eZ.resize(j); ev.resize(j); ew.resize(j);
        if (j == 0) return false;
        prune(exact);
        return true;
    }
    void scaleComp(const double* r, bool exact) {
        for (size_t i = 0; i < ew.size(); i++) ew[i] *= r[ek[i]];
        prune(exact);
    }

    void computeQ() {
        Q.assign(1 << s, 0.0);
        tot = 0;
        for (size_t i = 0; i < ew.size(); i++) { Q[eZ[i]] += ew[i]; tot += ew[i]; }
    }

    // M[k][v] for hypotheses with T_init == S (own end certified)
    vector<vector<double>> extractM() const {
        vector<vector<double>> M(K, vector<double>(1 << (s - 1), 0.0));
        for (size_t i = 0; i < ew.size(); i++)
            if (eZ[i] == S) M[ek[i]][ev[i]] += ew[i];
        return M;
    }
    // build model of size ns from coverage-likelihood M (over ns bits, in certified side coords).
    // rev: whether this model's coords are reversed w.r.t. M. ind: optional indicator over (T,v).
    void buildFromM(const vector<vector<double>>& M, int ns, int newS, bool rev, const vector<char>* ind, bool exact) {
        s = ns; S = newS;
        int vb = s - 1, vbPrev = s;
        clear();
        for (int k = 0; k < K; k++)
            for (int T = 0; T < (1 << s); T++) {
                double tp = tprior(k, s, T);
                int m = T ? 31 - __builtin_clz(T) : -1;
                for (int v = 0; v < (1 << vb); v++) {
                    if (ind && !(*ind)[((size_t)T << vb) | v]) continue;
                    int f = T == 0 ? (1 | (v << 1)) : ((v >> m) << (m + 1));
                    if (rev) {
                        int r = 0;
                        for (int i = 0; i < vbPrev; i++) if ((f >> i) & 1) r |= 1 << (vbPrev - 1 - i);
                        f = r;
                    }
                    double mm = M[k][f];
                    if (mm <= 0) continue;
                    double w = mm * tp * vpri[k][vb][v] / vpri[k][vbPrev][f];
                    if (w > 0) push(k, T, v, w);
                }
            }
        prune(exact);
    }

    // own end certified empty (T_init == S). New interval shrinks at own end.
    // newS: toggles of the new T (own end barriers of the new interval), supplied by caller.
    void advanceOwn(int newS, bool exact) {
        int vbPrev = s - 1;
        vector<vector<double>> M(K, vector<double>(1 << vbPrev, 0.0));
        for (size_t i = 0; i < ew.size(); i++)
            if (eZ[i] == S) M[ek[i]][ev[i]] += ew[i];
        s--;
        S = newS;
        int vb = s - 1;
        clear();
        for (int k = 0; k < K; k++)
            for (int T = 0; T < (1 << s); T++) {
                double tp = tprior(k, s, T);
                int m = T ? 31 - __builtin_clz(T) : -1;
                for (int v = 0; v < (1 << vb); v++) {
                    int f = T == 0 ? (1 | (v << 1)) : ((v >> m) << (m + 1));
                    double mm = M[k][f];
                    if (mm <= 0) continue;
                    double w = mm * tp * vpri[k][vb][v] / vpri[k][vbPrev][f];
                    if (w > 0) push(k, T, v, w);
                }
            }
        prune(exact);
    }

    // other end certified: its original T (in other's coords) = SO. Interval shrinks at far end.
    void advanceOther(int SO, bool exact) {
        int e = 1 << (s - 1);
        int eOrig = (SO & e) ? 1 : 0;
        int rest = SO & ~e;
        int vbPrev = s - 1;
        int NT = 1 << s, NV = 1 << vbPrev;
        vector<double> W((size_t)K * NT * NV, 0.0);
        for (size_t i = 0; i < ew.size(); i++) W[((size_t)ek[i] * NT + eZ[i]) * NV + ev[i]] += ew[i];
        int far = s - 2;  // own v bit of far cell
        int lim;          // own v bits >= lim are covered by rest
        if (rest) lim = far - (31 - __builtin_clz(rest));
        else lim = far + 1;
        int ns = s - 1, nvb = ns - 1;
        clear();
        for (int k = 0; k < K; k++) {
            double prest = tprior(k, s - 1, rest);
            for (int T = 0; T < (1 << ns); T++) {
                int Told = T | (eOrig ? e : 0);
                for (int v = 0; v < (1 << nvb); v++) {
                    int vold;
                    if (lim > nvb) vold = v; else vold = v & ((1 << lim) - 1);
                    if (!rest) vold |= 1 << far;
                    double wo = W[((size_t)k * NT + Told) * NV + vold];
                    if (wo <= 0) continue;
                    double w = wo * vpri[k][nvb][v] * prest / vpri[k][vbPrev][vold];
                    if (w > 0) push(k, T, v, w);
                }
            }
        }
        s = ns;
        S = S & ~e;
        prune(exact);
    }
};

struct Solver {
    int n;
    Interactor* io;
    bool finished = false;
    bool exactMode = false;
    int L, R;
    Model M[2];  // 0 = left, 1 = right
    long long flips = 0;
    int restarts = 0;

    int sz() const { return R - L + 1; }
    int off() const { return n - sz(); }
    pair<int, int> barrier(int side, int b) const {
        if (side == 0) return {L, L + b};
        return {R - b, R};
    }
    int ebit() const { return 1 << (sz() - 1); }
    bool usable(int side) const { return (M[1 - side].S & ~ebit()) == 0; }

    int doFlip(int side, int b) {
        flips++;
        auto [i, j] = barrier(side, b);
        int c = io->flip(i, j);
        if (traceOn) fprintf(stderr, "flip %d %d -> %d  [%zu %zu] side %d\n", i, j, c, M[0].ew.size(), M[1].ew.size(), side);
        if (c < 0 || c == n) { finished = true; return -1; }
        if (b == sz() - 1) { M[0].S ^= ebit(); M[1].S ^= ebit(); }
        else M[side].S ^= 1 << b;
        return c;
    }

    void run(int c0) {
        precompute();
        PRUNE_EXACT = 1e-25;
        int cur = c0;
        if (cur == n) return;
        while (!finished) {
            L = 1; R = n;
            for (int sd = 0; sd < 2; sd++) M[sd].initFresh(n, 0, cur, exactMode);
            bool ok = loop(cur);
            if (finished) return;
            if (!ok) { restarts++; exactMode = true; if (restarts >= 2) PRUNE_EXACT = 0.0; }
        }
    }

    void advance(int side) {
        int s = sz();
        int SO = M[side].S;  // original T of certified side (== toggles)
        int other = 1 - side;
        // new shared barrier e' belongs to other side's T at bit s-2
        int eNewTog = (M[other].S >> (s - 2)) & 1;
        if (side == 0) L++; else R--;
        int ns = s - 1;
        if (!SHARE) {
            M[side].advanceOwn(eNewTog ? (1 << (ns - 1)) : 0, exactMode);
            M[other].advanceOther(SO, exactMode);
            return;
        }
        auto MM = M[side].extractM();
        M[other].advanceOther(SO, exactMode);
        int oS = M[other].S;
        vector<char> ind((size_t)1 << (2 * ns - 1), 0);
        for (size_t i = 0; i < M[other].ew.size(); i++) ind[((size_t)M[other].eZ[i] << (ns - 1)) | M[other].ev[i]] = 1;
        M[side].buildFromM(MM, ns, eNewTog ? (1 << (ns - 1)) : 0, false, nullptr, exactMode);
        M[other].buildFromM(MM, ns, oS, true, &ind, exactMode);
    }

    bool loop(int& cur) {
        bool haveTarget = false;
        int tside = 0, target = 0;
        bool lastChanged = true;
        long long sinceChange = 0;
        while (true) {
            int s = sz();
            for (int sd = 0; sd < 2; sd++) {
                M[sd].computeQ();
                if (M[sd].tot <= 0 || M[sd].ew.empty()) return false;
            }
            double eps = exactMode ? 0.0 : EPS;
            bool adv = false;
            for (int sd = 0; sd < 2 && !adv; sd++) {
                if (M[sd].Q[M[sd].S] >= (1 - eps) * M[sd].tot) {
                    if (s == 1) return false;
                    advance(sd);
                    adv = true;
                }
            }
            if (adv) { haveTarget = false; lastChanged = true; continue; }
            if (sinceChange > 4 * (1 << s) + 50) return false;  // safety

            int eb = ebit();
            int fside = -1, fbit = -1;
            if (haveTarget && M[tside].S == target) {
                haveTarget = false;
                if (!lastChanged && M[tside].Q[M[tside].S] > 0 && usable(tside)) { fside = tside; fbit = 0; }
            }
            if (fside < 0) {
                if (!haveTarget || lastChanged) {
                    double best = -1; int bs = -1, bz = -1;
                    int nsides = (TWO && s > 1) ? 2 : 1;
                    if (nsides > 1 && DENSETH < 1) {
                        double dw = 0, tw = 0;
                        Model& X = M[tside];
                        for (size_t i = 0; i < X.ew.size(); i++) { tw += X.ew[i]; if (X.ek[i] == K - 1) dw += X.ew[i]; }
                        if (dw > DENSETH * tw) nsides = 1;
                    }
                    int allowed[2] = {1, nsides > 1};
                    if (nsides == 1) { allowed[0] = (tside == 0); allowed[1] = (tside == 1); nsides = 2; }
                    if (nsides > 1 && SIDEMODE == 1) {
                        double idx[2];
                        for (int sd = 0; sd < 2; sd++) {
                            Model& X = M[sd];
                            int undo = __builtin_popcount(M[1 - sd].S & ~eb);
                            vector<double> qs;
                            qs.reserve(1 << s);
                            for (int Z = 0; Z < (1 << s); Z++) if (Z != X.S && X.Q[Z] > 0) qs.push_back(X.Q[Z] / X.tot);
                            sort(qs.rbegin(), qs.rend());
                            double F = 0, cost = undo, bi = 0;
                            for (double q : qs) {
                                cost += 1 - F;
                                F += q;
                                bi = max(bi, F / cost);
                            }
                            idx[sd] = bi;
                        }
                        if (idx[1 - tside] > idx[tside] * GAMMA) allowed[tside] = 0; else allowed[1 - tside] = 0;
                    }
                    for (int sd = 0; sd < nsides; sd++) {
                        if (!allowed[sd]) continue;
                        Model& X = M[sd];
                        int undo = __builtin_popcount(M[1 - sd].S & ~eb);
                        for (int Z = 0; Z < (1 << s); Z++) {
                            if (Z == X.S || X.Q[Z] <= 0) continue;
                            int d = __builtin_popcount(Z ^ X.S) + undo;
                            double sc = X.Q[Z] / X.tot / pow((double)d, ALPHA);
                            if (SIDEMODE == 0 && sd != tside) sc /= GAMMA;
                            if (sc > best * (1 + 1e-9)) { best = sc; bs = sd; bz = Z; }
                        }
                    }
                    if (bs < 0) { fside = 0; fbit = 0; }
                    else { tside = bs; target = bz; haveTarget = true; }
                }
                if (fside < 0) {
                    int oth = 1 - tside;
                    int und = M[oth].S & ~eb;
                    if (und) {
                        fside = oth;
                        fbit = 31 - __builtin_clz(und);
                    } else {
                        Model& X = M[tside];
                        int diff = target ^ X.S;
                        double bq = -1;
                        for (int b = s - 1; b >= 0; b--) if ((diff >> b) & 1) {
                            double q = X.Q[X.S ^ (1 << b)];
                            if (q > bq * (1 + 1e-9) + 1e-300) { bq = q; fbit = b; }
                        }
                        fside = tside;
                    }
                }
            }
            int c = doFlip(fside, fbit);
            if (c < 0) return true;
            cur = c;
            bool changed = false;
            bool us[2] = {usable(0), usable(1)};
            for (int sd = 0; sd < 2; sd++) {
                if (!us[sd]) continue;
                if (!M[sd].filter(off(), c, changed, exactMode)) return false;
            }
            if (XFER) for (int sd = 0; sd < 2; sd++)
                if (us[sd] && !us[1 - sd]) M[1 - sd].scaleComp(M[sd].ratio, exactMode);
            lastChanged = changed;
            if (changed) sinceChange = 0; else sinceChange++;
        }
    }
};

#ifdef SIM
struct SimIO : Interactor {
    int n;
    vector<vector<int>> act;
    int count() {
        int c = 0;
        for (int k = 1; k <= n; k++) {
            bool cov = false;
            for (int i = 1; i <= k && !cov; i++)
                for (int j = k; j <= n; j++) if (act[i][j]) { cov = true; break; }
            if (!cov) c++;
        }
        return c;
    }
    long long fl = 0;
    int flip(int i, int j) override {
        fl++;
        if (i < 1 || j > n || i > j) { fprintf(stderr, "bad flip\n"); exit(1); }
        act[i][j] ^= 1;
        if (fl > 100000) { fprintf(stderr, "too many\n"); exit(1); }
        return count();
    }
};

int main(int argc, char** argv) {
    int n = atoi(argv[1]);
    double p = atof(argv[2]);
    int trials = atoi(argv[3]);
    int seed = argc > 4 ? atoi(argv[4]) : 1;
    if (argc > 5) TWO = atoi(argv[5]);
    if (argc > 6) ALPHA = atof(argv[6]);
    if (argc > 7) EPS = atof(argv[7]);
    if (getenv("GAMMA")) GAMMA = atof(getenv("GAMMA"));
    if (getenv("SIDEMODE")) SIDEMODE = atoi(getenv("SIDEMODE"));
    if (getenv("SHARE")) SHARE = atoi(getenv("SHARE"));
    if (getenv("XFER")) XFER = atoi(getenv("XFER"));
    if (getenv("PIWS")) { PIW.clear(); string t=getenv("PIWS"); stringstream ss(t); double x; while (ss>>x) PIW.push_back(x); }
    if (getenv("EPS")) EPS = atof(getenv("EPS"));
    if (getenv("ALPHA")) ALPHA = atof(getenv("ALPHA"));
    if (getenv("DENSETH")) DENSETH = atof(getenv("DENSETH"));
    if (getenv("PKS")) { PK.clear(); PIW.clear(); string t=getenv("PKS"); stringstream ss(t); double x; while (ss>>x) { PK.push_back(x); PIW.push_back(1.0);} }
    mt19937 rng(seed);
    double totalFl = 0;
    long long mx = 0;
    double totalAct = 0; long long totalRs = 0;
    for (int t = 0; t < trials; t++) {
        SimIO sim; sim.n = n;
        int c0, na;
        do {
            sim.act.assign(n + 2, vector<int>(n + 2, 0));
            na = 0;
            if (p >= 0) {
                for (int i = 1; i <= n; i++) for (int j = i; j <= n; j++)
                    if (uniform_real_distribution<double>(0, 1)(rng) < p) { sim.act[i][j] = 1; na++; }
            } else {
                int kk = (int)(-p);
                vector<pair<int,int>> all;
                for (int i = 1; i <= n; i++) for (int j = i; j <= n; j++) all.push_back({i,j});
                shuffle(all.begin(), all.end(), rng);
                for (int q = 0; q < kk && q < (int)all.size(); q++) { sim.act[all[q].first][all[q].second] = 1; na++; }
            }
            c0 = sim.count();
        } while (c0 == n);
        Solver sol; sol.n = n; sol.io = &sim;
        traceOn = getenv("TT") && atoi(getenv("TT")) == t;
        if (traceOn) { for (int i=1;i<=n;i++) for (int j=i;j<=n;j++) if (sim.act[i][j]) fprintf(stderr,"[%d,%d] ",i,j); fprintf(stderr,"\n"); }
        sol.run(c0);
        if (sim.count() != n) { fprintf(stderr, "FAIL\n"); return 1; }
        if (getenv("PT")) fprintf(stderr, "trial %d flips %lld active %d c0 %d\n", t, sim.fl, na, c0);
        totalRs += sol.restarts;
        totalFl += sim.fl; mx = max(mx, sim.fl); totalAct += na;
    }
    printf("n=%d p=%.3f avgflips=%.2f max=%lld avgactive=%.2f restarts=%lld\n", n, p, totalFl / trials, mx, totalAct / trials, totalRs);
}
#else
int main() {
    int n, c0;
    if (scanf("%d", &n) != 1) return 0;
    if (scanf("%d", &c0) != 1) return 0;
    RealIO io;
    Solver sol; sol.n = n; sol.io = &io;
    sol.run(c0);
    return 0;
}
#endif

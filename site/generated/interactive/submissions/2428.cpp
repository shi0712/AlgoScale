#include <bits/stdc++.h>
using namespace std;

// ---------------- interaction layer ----------------
struct Done {};
static int N;
static int curK;
#ifdef SIM
static vector<int> simPos;
static long long simMoves, simLimit = 100000;
static long long simThin, simScan, simSym;
static double statLog, statMoves, statWheels;
static int endgameMark; static long long endgameStart, endgameW; static double ENDLIMIT = 3e5; static double endTot, endWTot;
static int simCount() {
    vector<char> s(N, 0);
    int c = 0;
    for (int p : simPos) if (!s[p]) s[p] = 1, c++;
    return c;
}
struct SimFail {};
#endif
static int ask(int wheel, int shift) {  // wheel 0-based
    shift %= N;
    if (shift < 0) shift += N;
    if (shift > N / 2) shift -= N;
#ifdef SIM
    simMoves++;
    if (simMoves > simLimit) throw SimFail();
    simPos[wheel] = ((simPos[wheel] + shift) % N + N) % N;
    curK = simCount();
    if (getenv("TRACE")) { fprintf(stderr, "ask %d %d -> %d  [", wheel, shift, curK); for (int p : simPos) fprintf(stderr, "%d ", p); fprintf(stderr, "]\n"); }
#else
    printf("%d %d\n", wheel + 1, shift);
    fflush(stdout);
    if (scanf("%d", &curK) != 1) exit(0);
#endif
    if (curK == 1) throw Done();
    return curK;
}

static double BETA = 1.0;
static double GAMMA = 1.0;
static int SMALLDEPTH = 3;
static double SMALLBUDGET = 3e6;
static int DEPTH = 2, TOPK = 8, TOPK3 = 4;
static double Sst[64][64], fact[64], binom[64][64];
static void initComb() {
    Sst[0][0] = 1;
    for (int m = 1; m < 64; m++) for (int k = 1; k <= m; k++) Sst[m][k] = k * Sst[m - 1][k] + Sst[m - 1][k - 1];
    fact[0] = 1; for (int i = 1; i < 64; i++) fact[i] = fact[i - 1] * i;
    for (int i = 0; i < 64; i++) { binom[i][0] = 1; for (int j = 1; j <= i; j++) binom[i][j] = binom[i - 1][j - 1] + (j <= i - 1 ? binom[i - 1][j] : 0); }
}
// number of maps of M balls into r required (nonempty) + m optional boxes
static double Fcount(int M, int r, int m) {
    if (r < 0 || M < r) return 0;
    double s = 0;
    for (int t = r; t <= M; t++) s += binom[M][t] * fact[r] * Sst[t][r] * pow((double)m, M - t);
    return s;
}
static double THIN = 0.85;

struct Hyp { int x; int s; double w; };  // s=1: single among unknown, s=0: shared, s=2: x is known-occupied pos

static unsigned rngState = 88172645u;
static unsigned rnd() { rngState ^= rngState << 13; rngState ^= rngState >> 17; rngState ^= rngState << 5; return rngState; }


// ---------------- exact belief solver for small n ----------------
static double ALPHA = 0.6;
static int SMALLN = 7;
static int FULLN = 8;
static double ENDLIM = 3e4;
static void solveSmall() {
    const int n = N;
    int S = 1;
    for (int i = 0; i < n - 1; i++) S *= n;
    const int A = n * (n - 1);
    vector<unsigned char> cntS(S), Ls(S);
    vector<int> trans((size_t)S * A);
    vector<int> a(n), pw(n);
    pw[0] = 1;
    for (int i = 1; i < n; i++) pw[i] = pw[i - 1] * n;
    for (int s = 0; s < S; s++) {
        a[0] = 0;
        int x = s;
        for (int i = 1; i < n; i++) a[i] = x % n, x /= n;
        int mult[8] = {0};
        int c = 0, mm = 0;
        for (int i = 0; i < n; i++) { if (!mult[a[i]]) c++; mult[a[i]]++; mm = max(mm, mult[a[i]]); }
        cntS[s] = c;
        Ls[s] = n - mm;
        for (int i = 0; i < n; i++) for (int j = 1; j < n; j++) {
            int code = 0;
            if (i == 0) {
                // wheel 0 moves by j: others relative shift by -j
                for (int t = 1; t < n; t++) code += ((a[t] - j + n) % n) * pw[t - 1];
            } else {
                for (int t = 1; t < n; t++) code += ((t == i ? (a[t] + j) % n : a[t])) * pw[t - 1];
            }
            trans[(size_t)s * A + i * (n - 1) + (j - 1)] = code;
        }
    }
    vector<int> B;
    for (int s = 0; s < S; s++) if (cntS[s] == curK) B.push_back(s);
    auto h = [&](const vector<int>& Bv) -> double {
        double l = 0;
        for (int s : Bv) l += Ls[s];
        l /= Bv.size();
        return l + ALPHA * log2((double)Bv.size());
    };
    function<double(const vector<int>&, int, int*)> V = [&](const vector<int>& Bv, int d, int* best) -> double {
        if (d == 0) return h(Bv);
        double bestV = 1e18;
        vector<int> parts[9];
        double mL = 0;
        for (int s : Bv) mL += Ls[s];
        mL /= Bv.size();
        for (int pass = 0; pass < 2 && bestV > 1e17; pass++)
        for (int act = 0; act < A; act++) {
            for (int o = 0; o <= n; o++) parts[o].clear();
            for (int s : Bv) { int t = trans[(size_t)s * A + act]; parts[cntS[t]].push_back(t); }
            if (pass == 0) {
                int ne = 0, last = 0;
                for (int o = 1; o <= n; o++) if (!parts[o].empty()) ne++, last = o;
                if (ne == 1 && last != 1) {
                    double ml2 = 0;
                    for (int t : parts[last]) ml2 += Ls[t];
                    ml2 /= parts[last].size();
                    if (ml2 >= mL - 1e-9) continue;
                }
            }
            double v = 1;
            for (int o = 2; o <= n; o++) if (!parts[o].empty()) {
                v += (double)parts[o].size() / Bv.size() * V(parts[o], d - 1, nullptr);
                if (v >= bestV) break;
            }
            if (v < bestV) { bestV = v; if (best) *best = act; }
        }
        return bestV;
    };
    while (true) {
        int d = 1;
        double work = (double)B.size() * A;
        while (d < SMALLDEPTH && work * A < SMALLBUDGET) work *= A, d++;
        int act = 0;
        V(B, d, &act);
        int i = act / (n - 1), j = act % (n - 1) + 1;
        int k = ask(i, j);
        vector<int> B2;
        for (int s : B) { int t = trans[(size_t)s * A + act]; if (cntS[t] == k) B2.push_back(t); }
        B.swap(B2);
    }
}


// ---------------- generic explicit belief solver ----------------
static double GALPHA = 0.6;
static double GBUDGET = 2e7;
static int GTOPK = 8;
static int GMAXD = 3;
struct BeliefSet {
    int n;
    size_t B;
    vector<uint8_t> st;     // positions, stride n
    vector<uint8_t> M;      // multiplicities, stride n
    vector<uint8_t> cnt, mx, nmx;
    void build(int n_, vector<uint8_t>&& s) {
        n = n_; st = std::move(s); B = st.size() / n;
        M.assign(B * n, 0); cnt.resize(B); mx.resize(B); nmx.resize(B);
        for (size_t b = 0; b < B; b++) {
            uint8_t* m = &M[b * n];
            const uint8_t* q = &st[b * n];
            for (int i = 0; i < n; i++) m[q[i]]++;
            int c = 0, x = 0, nx = 0;
            for (int p = 0; p < n; p++) if (m[p]) { c++; if (m[p] > x) x = m[p], nx = 1; else if (m[p] == x) nx++; }
            cnt[b] = c; mx[b] = x; nmx[b] = nx;
        }
    }
    inline void eff(size_t b, int i, int d, int& nc, int& L) const {
        const uint8_t* m = &M[b * n];
        int p = st[b * n + i], q = p + d; if (q >= n) q -= n;
        nc = cnt[b] - (m[p] == 1) + (m[q] == 0);
        int x = mx[b];
        int nm = (m[p] == x && nmx[b] == 1) ? x - 1 : x;
        nm = max(nm, m[q] + 1);
        L = n - nm;
    }
    double meanL() const { double s = 0; for (size_t b = 0; b < B; b++) s += n - mx[b]; return s / B; }
};
static vector<int> distinctWheels(const BeliefSet& S) {
    int n = S.n;
    vector<int> res;
    vector<uint64_t> hs(n);
    for (int i = 0; i < n; i++) {
        uint64_t h = 1469598103934665603ULL;
        for (size_t b = 0; b < S.B; b++) h = (h ^ S.st[b * n + i]) * 1099511628211ULL;
        hs[i] = h;
    }
    for (int i = 0; i < n; i++) {
        bool dup = false;
        for (int r : res) if (hs[r] == hs[i]) {
            bool same = true;
            for (size_t b = 0; b < S.B && same; b++) if (S.st[b * n + i] != S.st[b * n + r]) same = false;
            if (same) { dup = true; break; }
        }
        if (!dup) res.push_back(i);
    }
    return res;
}
// depth-1 evaluation of all actions; returns sorted (value, action)
static vector<pair<double,int>> evalD1(const BeliefSet& S, const vector<int>& W) {
    int n = S.n;
    vector<pair<double,int>> out;
    vector<double> sumL(n + 2);
    vector<int> cn(n + 2);
    double invB = 1.0 / S.B;
    const double mL0 = S.meanL();
    for (int wi = 0; wi < (int)W.size(); wi++) {
        int i = W[wi];
        for (int d = 1; d < n; d++) {
            fill(sumL.begin(), sumL.end(), 0.0);
            fill(cn.begin(), cn.end(), 0);
            for (size_t b = 0; b < S.B; b++) {
                int nc, L;
                S.eff(b, i, d, nc, L);
                cn[nc]++; sumL[nc] += L;
            }
            double v = 1;
            for (int o = 2; o <= n; o++) if (cn[o]) v += invB * (sumL[o] + GALPHA * cn[o] * log2((double)cn[o]));
            int ne = 0, last = 0;
            for (int o = 1; o <= n; o++) if (cn[o]) ne++, last = o;
            if (ne == 1 && last != 1 && sumL[last] / cn[last] >= mL0 - 1e-9) v += 1e6;  // no progress
            out.push_back({v, i * n + d});
        }
    }
    sort(out.begin(), out.end());
    return out;
}
static void childStates(const BeliefSet& S, int act, int outcome, vector<uint8_t>& res) {
    int n = S.n, i = act / n, d = act % n;
    res.clear();
    for (size_t b = 0; b < S.B; b++) {
        int nc, L;
        S.eff(b, i, d, nc, L);
        if (nc != outcome) continue;
        size_t o = res.size();
        res.insert(res.end(), S.st.begin() + b * n, S.st.begin() + (b + 1) * n);
        int q = res[o + i] + d; if (q >= n) q -= n;
        res[o + i] = q;
    }
}
static double Vrec(const BeliefSet& S, int depth, int* bestOut) {
    vector<int> W = distinctWheels(S);
    auto c1 = evalD1(S, W);
    if (depth <= 1 || S.B == 1) {
        if (bestOut) *bestOut = c1[0].second;
        return S.B == 1 && !bestOut ? S.meanL() : c1[0].first;
    }
    int n = S.n;
    int K = min(GTOPK, (int)c1.size());
    double best = 1e18; int bestA = c1[0].second;
    vector<uint8_t> tmp;
    vector<int> cn(n + 2);
    for (int t = 0; t < K; t++) {
        int act = c1[t].second, i = act / n, d = act % n;
        fill(cn.begin(), cn.end(), 0);
        for (size_t b = 0; b < S.B; b++) { int nc, L; S.eff(b, i, d, nc, L); cn[nc]++; }
        double v = 1 + (c1[t].first >= 1e6 ? 1e6 : 0);
        for (int o = 2; o <= n; o++) if (cn[o]) {
            childStates(S, act, o, tmp);
            BeliefSet C;
            C.build(n, std::move(tmp));
            tmp = vector<uint8_t>();
            double cv = (C.B == 1) ? C.meanL() : Vrec(C, depth - 1, nullptr);
            v += (double)cn[o] / S.B * cv;
            if (v >= best) break;
        }
        if (v < best) best = v, bestA = act;
    }
    if (bestOut) *bestOut = bestA;
    return best;
}
static int chooseAction(const BeliefSet& S) {
    vector<int> W = distinctWheels(S);
    double work = (double)W.size() * (S.n - 1) * S.B;
    int d = 1;
    while (d < GMAXD && work * GTOPK <= GBUDGET) work *= GTOPK, d++;
    int act;
    Vrec(S, d, &act);
    return act;
}
static void solveBelief(int n, vector<uint8_t> st0) {
    BeliefSet S;
    S.build(n, std::move(st0));
#ifdef SIM
    long long guard = 0;
#endif
    while (true) {
#ifdef SIM
        if (++guard > 20 * n + 50) throw SimFail();
        if (getenv("BDUMP") && guard > 12 && guard < 15) { fprintf(stderr, "B=%zu\n", S.B); for (size_t b = 0; b < min<size_t>(S.B, 20); b++) { for (int i = 0; i < n; i++) fprintf(stderr, "%d ", S.st[b*n+i]); fprintf(stderr, "\n"); } }
        if (S.B == 0) throw SimFail();
#endif
        int act = chooseAction(S);
        int i = act / n, d = act % n;
        int k = ask(i, d);
        vector<uint8_t> nx;
        childStates(S, act, k, nx);
        S.build(n, std::move(nx));
    }
}
static void solveFullBelief() {
    int n = N;
    // enumerate all configs with wheel 0 at 0 and count == curK
    vector<uint8_t> st;
    vector<int> a(n, 0);
    long long tot = 1;
    for (int i = 0; i < n - 1; i++) tot *= n;
    vector<int> m(n);
    for (long long code = 0; code < tot; code++) {
        long long x = code;
        fill(m.begin(), m.end(), 0);
        m[0] = 1;
        int c = 1;
        for (int i = 1; i < n; i++) { a[i] = x % n; x /= n; if (!m[a[i]]++) c++; }
        if (c == curK) for (int i = 0; i < n; i++) st.push_back((uint8_t)a[i]);
    }
    solveBelief(n, std::move(st));
}

static void solve() {
    if (N <= SMALLN) { solveSmall(); return; }
    if (N <= FULLN) { solveFullBelief(); return; }
    const int n = N;
    initComb();
    // ---------- Phase 0: thinning ----------
    {
        int target = max(2, (int)floor(THIN * n));
        int w = n - 1, guard = 0;
        while (curK > target && guard < 4 * n) {
            int sh = 1 + rnd() % (n - 1);
            ask(w, sh);
            w--; if (w < 1) w = n - 1;
            guard++;
#ifdef SIM
            simThin++;
#endif
        }
    }
    int k0 = curK;
    // ---------- Phase 1: scan wheel 0 ----------
    // u(x) = [x not in O1], count = |O1| + u(pos of wheel 0)
    vector<int> v(n, INT_MIN);
    v[0] = 0;
    int pos1 = 0, kprev = k0;
    vector<int> occ(n, 0);  // occupancy by others (O1)
    int visited = 1;
    while (true) {
        int goodA = -1, nGood = 0;
        for (int a = 0; a < 2; a++) {
            bool ok = true;
            int occVis = 0;
            for (int x = 0; x < n; x++) if (v[x] != INT_MIN) {
                int u = v[x] + a;
                if (u < 0 || u > 1) { ok = false; break; }
                if (u == 0) occVis++;
            }
            if (!ok) continue;
            int T = k0 - a;
            int unv = n - visited;
            if (T < occVis || T > occVis + unv || T < 1 || T > n - 1) continue;
            nGood++;
            if (T == occVis || T == occVis + unv) goodA = a; else goodA = -2;
        }
        if (nGood == 1 && goodA >= 0) {
            int a = goodA;
            int occVis = 0;
            for (int x = 0; x < n; x++) if (v[x] != INT_MIN && v[x] + a == 0) occVis++;
            int T = k0 - a;
            int restOcc = (T == occVis) ? 0 : 1;
            for (int x = 0; x < n; x++) {
                if (v[x] != INT_MIN) occ[x] = (v[x] + a == 0);
                else occ[x] = restOcc;
            }
            break;
        }
        int nx = (pos1 + 1) % n;
#ifdef SIM
        simScan++;
#endif
        int k = ask(0, 1);
        v[nx] = v[pos1] + (k - kprev);
        kprev = k;
        pos1 = nx;
        visited++;
    }
    // ---------- State ----------
    int c = pos1;                       // gathering point
    vector<int> pl(n, 0);               // known wheels count per position
    vector<int> kpos(n, -1);            // position of known wheels (-1 unknown)
    kpos[0] = c; pl[c] = 1;
    vector<char> Rk(n, 0);              // unknown-wheel occupancy at positions with pl==0 (exact)
    vector<char> maybe(n, 0);           // positions with pl>0 that may hold unknown wheels
    vector<char> maybeReq(n, 0);        // maybe positions known to hold unknown wheels
    for (int x = 0; x < n; x++) if (occ[x]) { if (x == c) maybe[x] = maybeReq[x] = 1; else Rk[x] = 1; }
    vector<int> U;
    for (int j = 1; j < n; j++) U.push_back(j);
    size_t uIdx = 0;

    auto rkCount = [&]() { int r = 0; for (int x = 0; x < n; x++) r += Rk[x]; return r; };
    auto occSet = [&]() { vector<char> o(n); for (int x = 0; x < n; x++) o[x] = Rk[x] || pl[x] > 0; return o; };
    auto periods = [&](const vector<char>& o) {
        vector<int> ps;
        for (int d = 1; d < n; d++) {
            bool ok = true;
            for (int x = 0; x < n && ok; x++) if (o[x] != o[(x + d) % n]) ok = false;
            if (ok) ps.push_back(d);
        }
        return ps;
    };
    // move known wheel w to position m, updating state
    auto moveKnown = [&](int w, int m) {
        int p = kpos[w];
        pl[p]--;
        kpos[w] = m;
        bool wasMaybe = (pl[p] == 0 && maybe[p]);
        bool mNew = (pl[m] == 0);
        pl[m]++;
        if (mNew && Rk[m]) { Rk[m] = 0; maybe[m] = 1; maybeReq[m] = 1; }
        int k = ask(w, m - p);
        if (pl[p] == 0 && wasMaybe) {
            maybe[p] = 0; maybeReq[p] = 0;
            int cnt = 0;
            for (int x = 0; x < n; x++) if (Rk[x] || pl[x] > 0) cnt++;
            Rk[p] = (k == cnt + 1);
        }
        return k;
    };

    vector<pair<int,int>> markers;  // (wheel, pos)

    while (true) {
        int rc = rkCount();
        int Wleft = (int)(U.size() - uIdx);
        if (Wleft == 0) break;
        if (rc == 0) {
            bool anyMaybeNonC = false;
            for (int x = 0; x < n; x++) if (x != c && maybe[x]) anyMaybeNonC = true;
            if (!anyMaybeNonC) break;
        }
        int j = U[uIdx];
#ifdef SIM
        if (!endgameMark) {
            int r = 0, mOpt = 0;
            for (int x = 0; x < n; x++) { r += Rk[x] + (maybe[x] && maybeReq[x]); mOpt += (maybe[x] && !maybeReq[x]); }
            if (Fcount(Wleft, r, mOpt) <= ENDLIMIT) { endgameMark = 1; endgameStart = simMoves; endgameW = Wleft; }
        }
#endif
        {
            int r = 0, mOpt = 0;
            for (int x = 0; x < n; x++) { r += Rk[x] + (maybe[x] && maybeReq[x]); mOpt += (maybe[x] && !maybeReq[x]); }
            if (Fcount(Wleft, r, mOpt) <= ENDLIM) {
                vector<int> allowed, req;
                for (int x = 0; x < n; x++) if (Rk[x] || maybe[x]) allowed.push_back(x);
                vector<char> isReq(n, 0);
                for (int x = 0; x < n; x++) if (Rk[x] || (maybe[x] && maybeReq[x])) isReq[x] = 1;
                vector<int> rem(U.begin() + uIdx, U.end());
                vector<uint8_t> base(n);
                for (int w = 0; w < n; w++) if (kpos[w] >= 0) base[w] = kpos[w];
                vector<uint8_t> st;
                vector<int> cover(n, 0);
                int uncovered = 0;
                for (int x = 0; x < n; x++) uncovered += isReq[x];
                // DFS
                function<void(int)> dfs = [&](int t) {
                    if ((int)rem.size() - t < uncovered) return;
                    if (t == (int)rem.size()) { st.insert(st.end(), base.begin(), base.end()); return; }
                    for (int x : allowed) {
                        base[rem[t]] = x;
                        if (isReq[x] && cover[x]++ == 0) uncovered--;
                        dfs(t + 1);
                        if (isReq[x] && --cover[x] == 0) uncovered++;
                    }
                };
                dfs(0);
                solveBelief(n, std::move(st));
                return;
            }
        }
        auto buildH = [&]() {
            vector<Hyp> H;
            int r = 0, mOpt = 0;
            for (int x = 0; x < n; x++) { r += Rk[x] + (maybe[x] && maybeReq[x]); mOpt += (maybe[x] && !maybeReq[x]); }
            int M = Wleft - 1;
            double w1 = Fcount(M, r - 1, mOpt), w0 = Fcount(M, r, mOpt), wReq = Fcount(M, r - 1, mOpt + 1);
            double mx = max({w1, w0, wReq, 1e-300});
            w1 /= mx; w0 /= mx; wReq /= mx;
            for (int x = 0; x < n; x++) if (Rk[x]) {
                if (w1 > 0) H.push_back({x, 1, w1});
                if (w0 > 0) H.push_back({x, 0, w0});
            }
            for (int x = 0; x < n; x++) if (maybe[x]) {
                double w = maybeReq[x] ? wReq : w0;
                if (w > 0) H.push_back({x, 2, w});
            }
            return H;
        };
        vector<Hyp> H = buildH();
        // symmetry check
        {
            vector<char> o = occSet();
            auto ps = periods(o);
            bool conflict = false;
            if (!ps.empty()) {
                vector<char> hx(n, 0);
                for (auto& h : H) hx[h.x] = 1;
                for (int d : ps) for (int x = 0; x < n; x++) if (hx[x] && hx[(x + d) % n]) conflict = true;
            }
            if (conflict) {
#ifdef SIM
                simSym++;
#endif
                int bestW = -1, bestM = -1;
                bool movesC = (pl[c] == 1);
                int cw = -1;
                for (int w = 0; w < n; w++) if (kpos[w] == c) { cw = w; break; }
                for (int m = 0; m < n && bestW < 0; m++) {
                    if (movesC ? (m == c) : (bool)o[m]) continue;
                    bool good = true;
                    for (int stay = 0; stay < 2 && good; stay++) {
                        vector<char> o2 = o;
                        o2[m] = 1;
                        if (movesC) {
                            if (!maybe[c] && stay == 1) continue;
                            o2[c] = (char)stay;
                        }
                        if (!periods(o2).empty()) good = false;
                    }
                    if (good) { bestW = cw; bestM = m; }
                }
                if (bestW < 0) {
                    for (int m = 0; m < n; m++) if (movesC ? (m != c) : !o[m]) { bestW = cw; bestM = m; break; }
                }
                if (bestW >= 0) {
                    moveKnown(bestW, bestM);
                    if (movesC) c = bestM;
                    else markers.push_back({bestW, bestM});
                    H = buildH();
                }
            }
        }
        int D = 0;
#ifdef SIM
        long long mv0 = simMoves;
        { set<int> xs; for (auto& h : H) xs.insert(h.x); statLog += log2((double)xs.size()); statWheels++; }
#endif
        vector<char> occA(n);
        int base0 = 0;
        auto refreshOcc = [&]() { base0 = 0; for (int y = 0; y < n; y++) { occA[y] = Rk[y] || pl[y] > 0; base0 += occA[y]; } };
        refreshOcc();
        auto predict = [&](const Hyp& h, int Dp) {
            bool removed = (h.s == 1);
            int y = h.x + Dp; if (y >= n) y -= n;
            bool inOcc = occA[y] && !(removed && y == h.x);
            return base0 - (removed ? 1 : 0) + (inOcc ? 0 : 1);
        };
        // value of a hypothesis set (indices into Hs), j currently at shift D
        vector<Hyp> Hs;
        vector<double> wxBuf(n);
        auto heur = [&](const vector<int>& idx, int D) -> double {
            int x0 = Hs[idx[0]].x;
            bool uq = true;
            for (int i : idx) if (Hs[i].x != x0) { uq = false; break; }
            if (uq) return ((x0 + D) % n == c) ? 0.0 : 1.0;
            fill(wxBuf.begin(), wxBuf.end(), 0.0);
            double tot = 0;
            for (int i : idx) wxBuf[Hs[i].x] += Hs[i].w, tot += Hs[i].w;
            double ent = 0;
            for (int x = 0; x < n; x++) if (wxBuf[x] > 0) { double p = wxBuf[x] / tot; ent -= p * log2(p); }
            return GAMMA + BETA * ent;
        };
        // returns value; bestOut gets argmin shift
        function<double(const vector<int>&, int, int, int*)> V = [&](const vector<int>& idx, int D, int depth, int* bestOut) -> double {
            int x0 = Hs[idx[0]].x;
            bool uq = true;
            for (int i : idx) if (Hs[i].x != x0) { uq = false; break; }
            if (uq) {
                if (bestOut) *bestOut = ((c - x0) % n + n) % n;
                return ((x0 + D) % n == c) ? 0.0 : 1.0;
            }
            if (depth == 0) return heur(idx, D);
            double tot = 0;
            for (int i : idx) tot += Hs[i].w;
            vector<pair<double,int>> cand;
            vector<int> g[3];
            for (int Dp = 0; Dp < n; Dp++) {
                if (Dp == D) continue;
                for (auto& v : g) v.clear();
                for (int i : idx) g[predict(Hs[i], Dp) - base0 + 1].push_back(i);
                int ng = 0;
                for (auto& v : g) ng += !v.empty();
                if (ng <= 1) continue;
                double val = 1;
                for (auto& v : g) if (!v.empty()) {
                    double w = 0;
                    for (int i : v) w += Hs[i].w;
                    val += w / tot * heur(v, Dp);
                }
                cand.push_back({val, Dp});
            }
            if (cand.empty()) { if (bestOut) *bestOut = -1; return 1e9; }
            sort(cand.begin(), cand.end());
            if (depth >= 2) {
                int K = min((int)cand.size(), depth >= 3 ? TOPK3 : TOPK);
                for (int t = 0; t < K; t++) {
                    int Dp = cand[t].second;
                    for (auto& v : g) v.clear();
                    for (int i : idx) g[predict(Hs[i], Dp) - base0 + 1].push_back(i);
                    double val = 1;
                    for (auto& v : g) if (!v.empty()) {
                        double w = 0;
                        for (int i : v) w += Hs[i].w;
                        val += w / tot * V(v, Dp, depth - 1, nullptr);
                    }
                    cand[t].first = val;
                }
                sort(cand.begin(), cand.begin() + K);
            }
            if (bestOut) *bestOut = cand[0].second;
            return cand[0].first;
        };
        while (true) {
            bool uq = true;
            for (auto& h : H) if (h.x != H[0].x) uq = false;
            if (uq && (H[0].x + D) % n == c) break;
            int bestD = -1;
            Hs = H;
            vector<int> idx(H.size());
            iota(idx.begin(), idx.end(), 0);
            V(idx, D, DEPTH, &bestD);
            if (bestD < 0) {
#ifdef SIM
                throw SimFail();
#endif
                bestD = ((c - H[0].x) % n + n) % n;  // should not happen
            }
            int k = ask(j, bestD - D);
            D = bestD;
            vector<Hyp> H2;
            for (auto& h : H) if (predict(h, D) == k) H2.push_back(h);
            H = H2;
            if (H.empty()) {
#ifdef SIM
                throw SimFail();
#endif
                break;
            }
        }
        if (H.empty()) break;
        int x = H[0].x;
        if (H[0].s != 2) {
            if (curK == base0 - 1) Rk[x] = 0;
        } else {
            maybeReq[x] = 0;  // unknown whether others remain at x
        }
#ifdef SIM
        statMoves += simMoves - mv0;
#endif
        kpos[j] = c;
        pl[c]++;
        uIdx++;
    }
    for (auto& [w, m] : markers) moveKnown(w, c);
}

#ifdef SIM
int main(int argc, char** argv) {
    int T = argc > 1 ? atoi(argv[1]) : 200;
    int nn = argc > 2 ? atoi(argv[2]) : 50;
    int mode = argc > 3 ? atoi(argv[3]) : 0;
    if (argc > 4) THIN = atof(argv[4]);
    if (argc > 5) BETA = atof(argv[5]);
    if (getenv("ALPHA")) ALPHA = atof(getenv("ALPHA"));
    if (getenv("ENDLIM")) ENDLIM = atof(getenv("ENDLIM"));
    if (getenv("FULLN")) FULLN = atoi(getenv("FULLN"));
    if (getenv("GALPHA")) GALPHA = atof(getenv("GALPHA"));
    if (getenv("GBUDGET")) GBUDGET = atof(getenv("GBUDGET"));
    if (getenv("GMAXD")) GMAXD = atoi(getenv("GMAXD"));
    if (getenv("GTOPK")) GTOPK = atoi(getenv("GTOPK"));
    if (getenv("SMALLN")) SMALLN = atoi(getenv("SMALLN"));
    if (getenv("SDEPTH")) SMALLDEPTH = atoi(getenv("SDEPTH"));
    if (getenv("SBUDGET")) SMALLBUDGET = atof(getenv("SBUDGET"));
    if (getenv("GAMMA")) GAMMA = atof(getenv("GAMMA"));
    if (getenv("DEPTH")) DEPTH = atoi(getenv("DEPTH"));
    if (getenv("TOPK")) TOPK = atoi(getenv("TOPK"));
    if (getenv("TOPK3")) TOPK3 = atoi(getenv("TOPK3"));
    long long totThin = 0, totScan = 0, totSym = 0;
    mt19937 rng(getenv("SEED") ? atoi(getenv("SEED")) : 12345);
    long long total = 0, mx = 0;
    if (mode == 98) {
        int n = nn; N = n; simPos.resize(n);
        for (int i = 0; i < n; i++) simPos[i] = atoi(argv[6 + i]);
        simMoves = 0; curK = simCount(); bool fail = false;
        try { solve(); } catch (Done&) {} catch (SimFail&) { fail = true; }
        printf("fail=%d final=%d moves=%lld\n", fail, simCount(), simMoves);
        return 0;
    }
    if (mode == 99) {
        int n = nn; N = n; long long cnt = 0;
        vector<int> cur(n, 0);
        while (true) {
            simPos = cur;
            if (simCount() > 1) {
                simMoves = 0; curK = simCount(); bool fail = false;
                try { solve(); } catch (Done&) {} catch (SimFail&) { fail = true; }
                if (fail || simCount() != 1) { printf("FAIL n=%d: ", n); for (int p : cur) printf("%d ", p); puts(""); return 1; }
                if (getenv("SHOWBAD") && simMoves >= atoi(getenv("SHOWBAD"))) { printf("bad %lld: ", simMoves); for (int p : cur) printf("%d ", p); puts(""); }
                total += simMoves; mx = max(mx, simMoves); cnt++;
            }
            int i = 0;
            while (i < n && ++cur[i] == n) cur[i++] = 0;
            if (i == n) break;
        }
        printf("n=%d configs %lld avg %.3f max %lld\n", n, cnt, (double)total / cnt, mx);
        return 0;
    }
    for (int t = 0; t < T; t++) {
        int n = nn ? nn : 3 + rng() % 48;
        N = n;
        simPos.assign(n, 0);
        do {
            if (mode == 0) for (auto& p : simPos) p = rng() % n;
            else if (mode == 1) { iota(simPos.begin(), simPos.end(), 0); shuffle(simPos.begin(), simPos.end(), rng); }
            else if (mode == 2) { int m = 1 + rng() % 4; vector<int> cs(m); for (auto& x : cs) x = rng() % n; for (auto& p : simPos) p = cs[rng() % m]; }
            else if (mode == 3) { int m = 2 + rng() % (n/2); vector<int> cs(m); for (auto& x : cs) x = rng() % n; for (auto& p : simPos) p = cs[rng() % m]; }
            else if (mode == 4) { for (auto& p : simPos) p = 0; simPos[rng() % n] = rng() % n; }
            else if (mode == 5) {
                vector<int> ds; for (int d = 1; d < n; d++) if (n % d == 0) ds.push_back(d);
                int d = ds[rng() % ds.size()]; int off = rng() % n;
                for (auto& p : simPos) p = (off + d * (rng() % (n / d))) % n;
            }
            else if (mode == 6) { for (int i = 0; i < n; i++) simPos[i] = i; }
            else if (mode == 7) { int m = 1 + rng() % n; for (auto& p : simPos) p = rng() % m; }
        } while (simCount() == 1);
        vector<int> orig = simPos;
        simMoves = 0; simThin = 0; simScan = 0; simSym = 0; endgameMark = 0; endgameStart = 0; endgameW = 0;
        curK = simCount();
        bool fail = false;
        try { solve(); } catch (Done&) {} catch (SimFail&) { fail = true; }
        if (fail || simCount() != 1) { printf("FAIL t=%d n=%d\n", t, n); for (int p : orig) printf("%d ", p); puts(""); return 1; }
        if (endgameMark) endTot += simMoves - endgameStart, endWTot += endgameW;
        totThin += simThin; totScan += simScan; totSym += simSym;
        total += simMoves; mx = max(mx, simMoves);
    }
    printf("endgame moves %.2f endgame wheels %.2f\n", endTot / T, endWTot / T);
    printf("perwheel moves %.3f log2hyps %.3f\n", statMoves / statWheels, statLog / statWheels);
    printf("avg %.2f max %lld  thin %.2f scan %.2f sym %.2f\n", (double)total / T, mx, (double)totThin / T, (double)totScan / T, (double)totSym / T);
}
#else
int main() {
    if (scanf("%d", &N) != 1) return 0;
    if (scanf("%d", &curK) != 1) return 0;
    if (curK == 1) return 0;
    try { solve(); } catch (Done&) {}
    return 0;
}
#endif

// Poetic Tournament - top-K selection with few comparisons
#include <bits/stdc++.h>
using namespace std;

// ---------------- comparator layer ----------------
struct Cmp {
    bool sim = true;
    const vector<int>* val = nullptr; // simulation values
    bool flip = false;
    long long cnt = 0;
    long long limit = LLONG_MAX; // abort sim if exceeded
    bool aborted = false;
    vector<pair<int,int>>* log = nullptr; // (winner, loser) in true orientation
    long long recov = 0; bool failed = false; int failTime = -1;
    // returns true if a beats b (a "greater" in current orientation). 0 is -inf sentinel.
    bool gt(int a, int b) {
        if (b == 0) return true;
        if (a == 0) return false;
        cnt++;
        bool r;
        if (sim) {
            if (cnt > limit) aborted = true;
            r = (*val)[a] > (*val)[b];
            if (log) log->push_back(r ? make_pair(a, b) : make_pair(b, a));
        } else {
            printf("? %d %d\n", a, b);
            fflush(stdout);
            int w;
            if (scanf("%d", &w) != 1) exit(0);
            if (w != a && w != b) exit(0);
            r = (w == a);
        }
        return flip ? !r : r;
    }
};

// ---------------- generic helpers ----------------
// Hadian-Sobel style: top-t of X (unstructured). Returns the set.
static vector<int> hsTop(Cmp& C, const vector<int>& X, int t) {
    int n = X.size();
    if (t >= n) return X;
    if (t <= 0) return {};
    if (t == 1) {
        int m = X[0];
        for (int i = 1; i < n; i++) if (C.gt(X[i], m)) m = X[i];
        return {m};
    }
    int m = n - t + 2;
    int sz = 1; while (sz < m) sz <<= 1;
    vector<int> tr(2 * sz, 0);
    for (int i = 0; i < m; i++) tr[sz + i] = X[i];
    // build: pair adjacent leaves
    for (int i = sz - 1; i >= 1; i--) {
        int a = tr[2 * i], b = tr[2 * i + 1];
        if (a == 0) tr[i] = b; else if (b == 0) tr[i] = a; else tr[i] = C.gt(a, b) ? a : b;
        if (C.aborted) return {};
    }
    vector<int> res;
    int nxt = m;
    for (int e = 0; e < t - 1; e++) {
        int w = tr[1];
        res.push_back(w);
        // find leaf
        int pos = -1;
        for (int i = sz; i < 2 * sz; i++) if (tr[i] == w) { pos = i; break; }
        tr[pos] = (nxt < n) ? X[nxt++] : 0;
        for (int i = pos >> 1; i >= 1; i >>= 1) {
            int a = tr[2 * i], b = tr[2 * i + 1];
            if (a == 0) tr[i] = b; else if (b == 0) tr[i] = a; else tr[i] = C.gt(a, b) ? a : b;
        }
        if (C.aborted) return {};
    }
    res.push_back(tr[1]);
    return res;
}

// select top-t of unstructured X choosing orientation that is cheaper
static vector<int> selTop(Cmp& C, const vector<int>& X, int t) {
    int n = X.size();
    if (t >= n) return X;
    if (t <= 0) return {};
    auto lg = [](int m) { int c = 0; while ((1 << c) < m) c++; return c; };
    long long c1 = (long long)(n - t) + (long long)(t - 1) * lg(n - t + 2);
    int d = n - t;
    long long c2 = (long long)(n - d) + (long long)(d - 1) * lg(n - d + 2);
    if (c1 <= c2) return hsTop(C, X, t);
    C.flip = !C.flip;
    vector<int> bot = hsTop(C, X, d);
    C.flip = !C.flip;
    unordered_set<int> bs(bot.begin(), bot.end());
    vector<int> res;
    for (int x : X) if (!bs.count(x)) res.push_back(x);
    return res;
}

// ---------------- chain strategy ----------------
struct ChainParams { double epsq, epsu, gam; };

struct HypTable {
    int n = 0, k = 0;
    vector<vector<double>> pC, pD; // pC[i][j] = P(C_i=j) for top-k count; pD for top-(k-1)
    static void fill(int n, int k, vector<vector<double>>& P) {
        P.assign(n + 1, vector<double>(k + 1, 0));
        vector<double> lf(n + 1); lf[0] = 0;
        for (int i = 1; i <= n; i++) lf[i] = lf[i - 1] + log((double)i);
        auto lC = [&](int a, int b) { return (b < 0 || b > a) ? -1e300 : lf[a] - lf[b] - lf[a - b]; };
        for (int i = 0; i <= n; i++) {
            int jmax = min(k, i), jmin = max(0, i - (n - k));
            // start at mode-ish: compute log at jmin then iterate ratios
            double lp = lC(k, jmin) + lC(n - k, i - jmin) - lC(n, i);
            double p = exp(lp);
            for (int j = jmin; j <= jmax; j++) {
                P[i][j] = p;
                if (j < jmax) {
                    // p(j+1)/p(j) = (k-j)(i-j) / ((j+1)(n-k-i+j+1))
                    double ratio = (double)(k - j) * (i - j) / ((double)(j + 1) * (n - k - i + j + 1));
                    p *= ratio;
                }
            }
        }
    }
    void build(int n_, int k_) { n = n_; k = k_; fill(n, k, pC); fill(n, k - 1, pD); }
};

struct Quant {
    vector<int> hiT, loT;
    double failCost = 0, failProb = 0;
    void build(const HypTable& H, double epsq, double epsu, double gam = 0) {
        int n = H.n, k = H.k, k1 = k - 1;
        hiT.assign(n + 1, 0); loT.assign(n + 1, 0);
        for (int i = 1; i <= n; i++) {
            {
                int jmax = min(k1, i), jmin = max(0, i - (n - k1));
                double eq = epsq * pow((double)n / (n - i + 1), gam); if (eq > 0.5) eq = 0.5;
                double tail = 0; int a = jmin;
                for (int j = jmax; j >= jmin; j--) { if (tail + H.pD[i][j] > eq) { a = j; break; } tail += H.pD[i][j]; }
                hiT[i] = a;
            }
            int jmax = min(k, i), jmin = max(0, i - (n - k));
            double h = 0; int r = jmax;
            for (int j = jmin; j <= jmax; j++) { if (h + H.pC[i][j] > epsu) { r = j; break; } h += H.pC[i][j]; }
            loT[i] = r;
        }
        vector<double> pr(k1 + 2, 0), nx(k1 + 2, 0);
        pr[0] = 1; failCost = 0; failProb = 0;
        for (int i = 0; i < n - 1; i++) {
            std::fill(nx.begin(), nx.end(), 0);
            for (int d = 0; d <= min(k1, i); d++) {
                if (pr[d] == 0) continue;
                double ps = (double)(k1 - d) / (n - i);
                nx[d] += pr[d] * (1 - ps);
                if (d + 1 <= k1) nx[d + 1] += pr[d] * ps;
            }
            int h = hiT[i + 1];
            for (int d = h + 1; d <= k1; d++) if (nx[d] > 0) { failProb += nx[d]; failCost += nx[d] * (n - i - 1); nx[d] = 0; }
            swap(pr, nx);
        }
    }
};

// min-forest (lazy heap) for unsorted sets where we may need minima
struct MinForest {
    vector<int> roots; vector<vector<int>>* ch; int sz = 0;
    void add(int x) { roots.push_back(x); sz++; }
    int popMin(Cmp& C) {
        while (roots.size() > 1) {
            vector<int> nr; nr.reserve(roots.size() / 2 + 1);
            for (size_t j = 0; j + 1 < roots.size(); j += 2) {
                int a = roots[j], b = roots[j + 1];
                if (C.gt(a, b)) { (*ch)[b].push_back(a); nr.push_back(b); }
                else { (*ch)[a].push_back(b); nr.push_back(a); }
            }
            if (roots.size() % 2) nr.push_back(roots.back());
            roots.swap(nr);
        }
        int m = roots[0];
        roots.swap((*ch)[m]);
        (*ch)[m].clear();
        sz--;
        return m;
    }
    void collect(vector<int>& out) const {
        vector<int> st = roots;
        while (!st.empty()) { int x = st.back(); st.pop_back(); out.push_back(x); for (int c : (*ch)[x]) st.push_back(c); }
    }
};

static vector<int> chainRun(Cmp& C, const vector<int>& X, int K, const Quant& Q, int maxLabel) {
    int n = X.size();
    int q = 0;
    vector<int> W; // ascending sorted chain
    static vector<vector<int>> chs;
    static vector<int> par;
    if ((int)chs.size() < maxLabel + 1) { chs.assign(maxLabel + 1, {}); par.assign(maxLabel + 1, 0); }
    for (int x : X) chs[x].clear();
    MinForest U; U.ch = &chs; // above W.back()
    vector<double> c; vector<int> rt; vector<double> S;
    auto locate = [&](int x) -> int {
        int L = W.size();
        if (L == 0) return 0;
        int m = L + 1;
        vector<double> w(m, 1.0);
        w[L] = U.sz == 0 ? 1.0 : (double)U.sz + 1.0;
        c.assign(m * m, 0); rt.assign(m * m, -1); S.assign(m * m, 0);
        for (int i = 0; i < m; i++) S[i * m + i] = w[i];
        for (int len = 2; len <= m; len++)
            for (int i = 0; i + len - 1 < m; i++) {
                int j = i + len - 1;
                S[i * m + j] = S[i * m + j - 1] + w[j];
                double b = 1e300; int br = -1;
                for (int r = i; r < j; r++) {
                    double v = c[i * m + r] + c[(r + 1) * m + j];
                    if (v < b - 1e-12) { b = v; br = r; }
                }
                c[i * m + j] = b + S[i * m + j]; rt[i * m + j] = br;
            }
        int lo = 0, hi = m - 1;
        while (lo < hi) {
            int r = rt[lo * m + hi];
            if (C.gt(x, W[r])) lo = r + 1; else hi = r;
        }
        return lo;
    };
    int curI = 0, lastMove = 0;
    auto dropQ = [&](int nq) { par[q] = nq; q = nq; lastMove = curI; };
    auto adjust = [&](int i, bool fin) {
        int hq = fin ? K : Q.hiT[i], lu = fin ? K : Q.loT[i];
        while ((int)W.size() + U.sz > hq) {
            if (!W.empty()) { dropQ(W[0]); W.erase(W.begin()); continue; }
            int m = U.popMin(C);
            dropQ(m);
            if (C.aborted) return;
        }
        while (W.size() >= 2 && U.sz + 2 <= lu) { U.add(W.back()); W.pop_back(); }
    };
    for (int i = 0; i < n; i++) {
        int x = X[i]; curI = i;
        if (!C.gt(x, q)) par[x] = q;
        else {
            int g = locate(x);
            if (g == (int)W.size() && U.sz > 0) U.add(x);
            else W.insert(W.begin() + g, x);
        }
        adjust(i + 1, false);
        if (C.aborted) return {};
    }
    adjust(n, true);
    vector<int> A = W; U.collect(A);
    if ((int)A.size() == K) return A;
    // a < K: q too high; recovery via max-extraction over below-forest
    long long c0 = C.cnt; C.failed = true; C.failTime = lastMove;
    vector<char> in(maxLabel + 1, 0);
    for (int x : A) in[x] = 1;
    in[q] = 1;
    vector<vector<int>> kids(maxLabel + 1);
    for (int x : X) if (!in[x]) kids[par[x]].push_back(x);
    A.push_back(q);
    // max-forest extraction using flipped min-forest
    C.flip = !C.flip;
    vector<vector<int>> ch2(maxLabel + 1);
    MinForest F; F.ch = &ch2;
    for (int x : kids[q]) F.add(x);
    while ((int)A.size() < K) {
        int mm = F.popMin(C);
        A.push_back(mm);
        for (int y : kids[mm]) F.add(y);
        if (C.aborted) break;
    }
    C.flip = !C.flip;
    C.recov = C.cnt - c0;
    return A;
}

// ---------------- DP-optimal sampled quickselect (small n) ----------------
struct QSDP {
    int NM = 0, SM = 25;
    vector<vector<double>> F, G;
    vector<vector<short>> Fs, Fr, Gs, Gr; // s=0 means HS
    static int lg2c(int m) { int c = 0; while ((1 << c) < m) c++; return c; }
    static double hs(int n, int t) { if (t >= n) return 0; if (t == 1) return n - 1; return (n - t) + (double)(t - 1) * lg2c(n - t + 2); }
    int TM = 0;
    void build(int nmax, int tmax) {
        NM = nmax; TM = tmax;
        int GM = min(SM, NM);
        vector<double> lf(NM + 2, 0);
        for (int i = 1; i <= NM + 1; i++) lf[i] = lf[i - 1] + log((double)i);
        auto lC = [&](int a, int b) { return (b < 0 || b > a) ? -1e300 : lf[a] - lf[b] - lf[a - b]; };
        F.assign(NM + 1, vector<double>(TM + 2, 0));
        Fs.assign(NM + 1, vector<short>(TM + 2, 0)); Fr = Fs;
        G.assign(GM + 1, vector<double>(GM + 2, 0));
        Gs.assign(GM + 1, vector<short>(GM + 2, 0)); Gr = Gs;
        for (int n = 1; n <= GM; n++) {
            for (int t = 1; t <= n; t++) {
                if (n == 1) { G[n][t] = 0; continue; }
                if (t > (n + 1) / 2) { G[n][t] = G[n][n + 1 - t]; Gs[n][t] = Gs[n][n + 1 - t]; Gr[n][t] = Gr[n][n + 1 - t]; continue; }
                double best = (t == 1) ? n - 1 : min(hs(n, t), hs(n, n + 1 - t));
                int bs = 0, br = 0;
                for (int s = 1; s <= n - 1; s++) {
                    int id = (int)llround((double)t * (s + 1) / (n + 1));
                    for (int r = max(1, id - 4); r <= min(s, id + 4); r++) {
                        double base = G[s][r] + (n - s);
                        if (base >= best) continue;
                        double e = 0;
                        double l0 = -lC(n, s);
                        for (int j = r; j <= n - s + r; j++) {
                            double pj = exp(lC(j - 1, r - 1) + lC(n - j, s - r) + l0);
                            if (pj < 1e-14) continue;
                            double c = (j == t) ? 0 : (j > t ? G[j - 1][t] : G[n - j][t - j]);
                            e += pj * c;
                            if (base + e >= best) break;
                        }
                        if (base + e < best) { best = base + e; bs = s; br = r; }
                    }
                }
                G[n][t] = best; Gs[n][t] = bs; Gr[n][t] = br;
            }
        }
        auto getF = [&](int n, int t) -> double { if (t <= 0 || t >= n) return 0; if (t > n - t) t = n - t; return F[n][t]; };
        for (int n = 1; n <= NM; n++) {
            for (int t = 1; t <= min(TM, n / 2); t++) {
                double best = min(hs(n, t), hs(n, n - t));
                int bs = 0, br = 0;
                for (int s = 1; s <= min(GM, n - 1); s++) {
                    int id = (int)llround((double)t * (s + 1) / (n + 1));
                    for (int r = max(1, id - 4); r <= min(s, id + 5); r++) {
                        double base = G[s][r] + (n - s);
                        if (base >= best) continue;
                        double e = 0;
                        double l0 = -lC(n, s);
                        for (int j = r; j <= n - s + r; j++) {
                            double pj = exp(lC(j - 1, r - 1) + lC(n - j, s - r) + l0);
                            if (pj < 1e-14) continue;
                            double c = (j == t || j == t + 1) ? 0 : (j > t + 1 ? getF(j - 1, t) : getF(n - j, t - j));
                            e += pj * c;
                            if (base + e >= best) break;
                        }
                        if (base + e < best) { best = base + e; bs = s; br = r; }
                    }
                }
                F[n][t] = best; Fs[n][t] = bs; Fr[n][t] = br;
            }
        }
    }
    // select t-th largest: returns pivot, fills above/below
    int runG(Cmp& C, const vector<int>& X, int t, vector<int>& ab, vector<int>& be) {
        int n = X.size();
        ab.clear(); be.clear();
        if (n == 1) return X[0];
        if (t > (n + 1) / 2) {
            C.flip = !C.flip;
            int p = runG(C, X, n + 1 - t, be, ab);
            C.flip = !C.flip;
            return p;
        }
        int s = Gs[n][t], r = Gr[n][t];
        if (s == 0) {
            vector<int> res;
            int m = t;
            bool rev = false;
            if (t != 1 && hs(n, n + 1 - t) < hs(n, t)) { rev = true; m = n + 1 - t; }
            if (rev) C.flip = !C.flip;
            res = hsTop(C, X, m);
            if (rev) C.flip = !C.flip;
            int p = res.back();
            unordered_set<int> st(res.begin(), res.end());
            vector<int>& A = rev ? be : ab; vector<int>& B = rev ? ab : be;
            for (int i = 0; i + 1 < (int)res.size(); i++) A.push_back(res[i]);
            for (int x : X) if (!st.count(x)) B.push_back(x);
            return p;
        }
        vector<int> S(X.begin(), X.begin() + s);
        vector<int> sa, sb;
        int piv = runG(C, S, r, sa, sb);
        for (int i = s; i < n; i++) { if (C.gt(X[i], piv)) sa.push_back(X[i]); else sb.push_back(X[i]); }
        int j = sa.size() + 1;
        if (j == t) { ab = sa; be = sb; return piv; }
        if (j > t) {
            vector<int> a2, b2;
            int p2 = runG(C, sa, t, a2, b2);
            ab = a2; be = b2; be.push_back(piv); be.insert(be.end(), sb.begin(), sb.end());
            return p2;
        }
        vector<int> a2, b2;
        int p2 = runG(C, sb, t - j, a2, b2);
        ab = sa; ab.push_back(piv); ab.insert(ab.end(), a2.begin(), a2.end()); be = b2;
        return p2;
    }
    vector<int> runF(Cmp& C, const vector<int>& X, int t) {
        int n = X.size();
        if (t <= 0) return {};
        if (t >= n) return X;
        if (t > n - t) {
            C.flip = !C.flip;
            vector<int> bot = runF(C, X, n - t);
            C.flip = !C.flip;
            unordered_set<int> st(bot.begin(), bot.end());
            vector<int> res; for (int x : X) if (!st.count(x)) res.push_back(x);
            return res;
        }
        int s = Fs[n][t], r = Fr[n][t];
        if (s == 0 || t > TM) return selTop(C, X, t);
        vector<int> S(X.begin(), X.begin() + s);
        vector<int> sa, sb;
        int piv = runG(C, S, r, sa, sb);
        for (int i = s; i < n; i++) { if (C.gt(X[i], piv)) sa.push_back(X[i]); else sb.push_back(X[i]); }
        int j = sa.size() + 1;
        if (j == t) { sa.push_back(piv); return sa; }
        if (j == t + 1) return sa;
        if (j > t + 1) return runF(C, sa, t);
        vector<int> r2 = runF(C, sb, t - j);
        sa.push_back(piv); sa.insert(sa.end(), r2.begin(), r2.end());
        return sa;
    }
};
static QSDP qsdp;

// ---------------- strategy dispatch ----------------
struct Strategy {
    int type; // 1 = Hadian-Sobel, 2 = chain, 4 = DP quickselect
    ChainParams cp;
    Quant Q;
};

static vector<int> runStrategy(Cmp& C, const vector<int>& X, int t, Strategy& s, int maxLabel) {
    if (t <= 0) return {};
    if (t >= (int)X.size()) return X;
    if (s.type == 1) return hsTop(C, X, t);
    if (s.type == 4) return qsdp.runF(C, X, t);
    return chainRun(C, X, t, s.Q, maxLabel);
}

int main(int argc, char** argv) {
    auto T0 = chrono::steady_clock::now();
    int N, K;
    bool simMode = (argc > 1 && string(argv[1]) == "sim");
    unsigned long long seed = chrono::steady_clock::now().time_since_epoch().count();
    if (simMode) { N = atoi(argv[2]); K = atoi(argv[3]); seed = argc > 5 ? atoll(argv[5]) : 1; }
    else { if (scanf("%d %d", &N, &K) != 2) return 0; }
    mt19937_64 rng(seed);

    if (K >= N) {
        if (!simMode) {
            printf("!"); for (int i = 1; i <= N; i++) printf(" %d", i); printf("\n"); fflush(stdout);
        }
        return 0;
    }
    bool flip = false;
    int t = K;
    if (N - K < K) { flip = true; t = N - K; }

    // candidate strategies
    vector<Strategy> strats;
    { Strategy s; s.type = 1; strats.push_back(s); }
    if (t >= 2) {
        HypTable H; H.build(N, t);
        vector<double> eqs = {3e-2, 1e-2, 3e-3, 1e-3, 3e-4, 1e-4, 3e-5, 1e-5, 3e-6};
        vector<double> eus = {3e-1, 1e-1, 3e-2};
        vector<double> gams = {0, 1.0};
        if (getenv("EQ")) eqs = {atof(getenv("EQ"))};
        if (getenv("EU")) eus = {atof(getenv("EU"))};
        for (double eq : eqs) for (double eu : eus) for (double gm : gams) {
            Strategy s; s.type = 2; s.cp = {eq, eu, gm};
            s.Q.build(H, eq, eu, gm);
            strats.push_back(s);
        }
    }
    if (N <= (getenv("DPN") ? atoi(getenv("DPN")) : 400) && t >= 2 && !getenv("NODP")) {
        qsdp.SM = getenv("SM") ? atoi(getenv("SM")) : 41;
        qsdp.build(N, t);
        Strategy s; s.type = 4; strats.push_back(s);
    }
    if (simMode) fprintf(stderr, "setup time %.3f\n", chrono::duration<double>(chrono::steady_clock::now() - T0).count());
    // evaluate by simulation
    int best = 0;
    if (strats.size() > 1) {
        auto t0 = T0;
        double budget = simMode ? 3.0 : 3.5;
        vector<double> sum(strats.size(), 0); vector<int> runs(strats.size(), 0);
        vector<int> vals(N + 1);
        vector<int> perm(N);
        int round = 0;
        while (true) {
            double el = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
            if (round >= 3 && el > budget) break;
            if (round >= 400) break;
            iota(perm.begin(), perm.end(), 1);
            shuffle(perm.begin(), perm.end(), rng);
            for (int i = 0; i < N; i++) vals[i + 1] = perm[i];
            vector<int> X(N); iota(X.begin(), X.end(), 1); shuffle(X.begin(), X.end(), rng);
            for (size_t si = 0; si < strats.size(); si++) {
                Cmp C; C.sim = true; C.val = &vals; C.flip = flip;
                vector<int> r = runStrategy(C, X, t, strats[si], N);
                sum[si] += C.cnt - C.recov; runs[si]++;
            }
            round++;
        }
        for (size_t si = 0; si < strats.size(); si++) if (strats[si].type == 2) sum[si] += 1.6 * strats[si].Q.failCost * runs[si];
        for (size_t si = 0; si < strats.size(); si++) if (strats[si].type == 4) sum[si] = qsdp.F[N][min(t, N - t)] * runs[si];
        for (size_t si = 0; si < strats.size(); si++) if (sum[si] / runs[si] < sum[best] / runs[best]) best = si;
        if (getenv("FORCE")) best = atoi(getenv("FORCE"));
        if (simMode) fprintf(stderr, "select done %.3f rounds %d\n", chrono::duration<double>(chrono::steady_clock::now() - T0).count(), round);
        if (simMode) {
            for (size_t si = 0; si < strats.size(); si++)
                fprintf(stderr, "strat %zu type %d fc %.2f fp %.5f eq %.0e eu %.0e g %.1f avg %.2f (runs %d)\n", si, strats[si].type, strats[si].Q.failCost, strats[si].Q.failProb, strats[si].cp.epsq, strats[si].cp.epsu, strats[si].cp.gam, sum[si] / runs[si], runs[si]);
        }
    }

    vector<int> X(N); iota(X.begin(), X.end(), 1); shuffle(X.begin(), X.end(), rng);
    if (simMode) {
        // evaluate chosen strategy on fresh permutations
        int T = argc > 4 ? atoi(argv[4]) : 100;
        double tot = 0; long long mx = 0;
        vector<int> vals(N + 1), perm(N);
        for (int it = 0; it < T; it++) {
            iota(perm.begin(), perm.end(), 1); shuffle(perm.begin(), perm.end(), rng);
            for (int i = 0; i < N; i++) vals[i + 1] = perm[i];
            shuffle(X.begin(), X.end(), rng);
            Cmp C; C.sim = true; C.val = &vals; C.flip = flip;
            vector<pair<int,int>> lg; C.log = &lg;
            vector<int> r = runStrategy(C, X, t, strats[best], N);
            vector<int> ans;
            if (flip) { vector<char> in(N + 1, 0); for (int x : r) in[x] = 1; for (int i = 1; i <= N; i++) if (!in[i]) ans.push_back(i); }
            else ans = r;
            sort(ans.begin(), ans.end(), [&](int a, int b) { return vals[a] > vals[b]; });
            bool ok = (int)ans.size() == K;
            for (int i = 0; ok && i < K; i++) if (vals[ans[i]] != N - i) ok = false;
            if (!ok) { printf("WRONG N=%d K=%d\n", N, K); return 1; }
            {
                // certification: every non-answer element must have all K answer elements as ancestors
                vector<int> bit(N + 1, -1); for (int i = 0; i < K; i++) bit[ans[i]] = i;
                vector<vector<int>> out(N + 1); vector<int> indeg(N + 1, 0);
                for (auto& e : lg) { out[e.first].push_back(e.second); indeg[e.second]++; }
                vector<unsigned long long> anc(N + 1, 0);
                vector<int> st; for (int i = 1; i <= N; i++) if (!indeg[i]) st.push_back(i);
                while (!st.empty()) { int x = st.back(); st.pop_back(); unsigned long long m = anc[x] | (bit[x] >= 0 ? (1ULL << bit[x]) : 0ULL);
                    for (int y : out[x]) { anc[y] |= m; if (--indeg[y] == 0) st.push_back(y); } }
                for (int i = 1; i <= N; i++) if (bit[i] < 0 && __builtin_popcountll(anc[i]) != K) { printf("NOT CERTIFIED N=%d K=%d elem %d\n", N, K, i); return 1; }
            }
            tot += C.cnt; mx = max(mx, C.cnt);
        }
        printf("N=%d K=%d best=%d avg=%.2f max=%lld\n", N, K, best, tot / T, mx);
        return 0;
    }
    Cmp C; C.sim = false; C.flip = flip;
    vector<int> r = runStrategy(C, X, t, strats[best], N);
    vector<int> ans;
    if (flip) { vector<char> in(N + 1, 0); for (int x : r) in[x] = 1; for (int i = 1; i <= N; i++) if (!in[i]) ans.push_back(i); }
    else ans = r;
    printf("!");
    for (int x : ans) printf(" %d", x);
    printf("\n");
    fflush(stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

static int N;

[[noreturn]] static void die() { exit(0); }

#ifndef SIM
static long long readInt() {
    long long x;
    if (scanf("%lld", &x) != 1) die();
    if (x == -1) die();
    return x;
}

// returns true iff X_i < X_j
static bool lessQ(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    return readInt() == 1;
}

static int addQ(int i, int j) {
    printf("+ %d %d\n", i, j);
    fflush(stdout);
    return (int)readInt();
}
#else
bool lessQ(int i, int j);
int addQ(int i, int j);
#endif

// ---------------- Ford-Johnson merge-insertion sort ----------------
static vector<int> fjSort(const vector<int>& v) {
    int n = v.size();
    if (n <= 1) return v;
    int k = n / 2;
    vector<int> big(k);
    unordered_map<int, int> partner;
    partner.reserve(k * 2);
    for (int i = 0; i < k; i++) {
        int x = v[2 * i], y = v[2 * i + 1];
        int b, s;
        if (lessQ(x, y)) { b = y; s = x; } else { b = x; s = y; }
        big[i] = b;
        partner[b] = s;
    }
    vector<int> sb = fjSort(big);
    vector<int> pend;  // pend[j] pairs with sb[j]; last may be the odd one
    for (int j = 0; j < k; j++) pend.push_back(partner[sb[j]]);
    if (n & 1) pend.push_back(v[n - 1]);
    int m = pend.size();
    vector<int> chain;
    chain.push_back(pend[0]);
    for (int j = 0; j < k; j++) chain.push_back(sb[j]);
    // insertion order (1-indexed): groups by Jacobsthal numbers
    vector<int> order;
    {
        long long a = 1, b = 3;
        long long lastDone = 1;
        while (lastDone < m) {
            long long hi = min<long long>(b, m);
            for (long long j = hi; j > lastDone; j--) order.push_back((int)j);
            lastDone = hi;
            long long c = b + 2 * a;
            a = b;
            b = c;
        }
    }
    for (int jj : order) {
        int j = jj - 1;  // 0-indexed
        int x = pend[j];
        int hi;
        if (j < k) {
            int a = sb[j];
            hi = find(chain.begin(), chain.end(), a) - chain.begin();
        } else {
            hi = chain.size();
        }
        int lo = 0;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (lessQ(x, chain[mid])) hi = mid; else lo = mid + 1;
        }
        chain.insert(chain.begin() + lo, x);
    }
    return chain;
}

// ---------------- slot predictor for carries ----------------
static double gp_minsc = 0.7, gp_wide = 10.0, gp_hsm = 0.8, gp_q = 0.3, gp_mixf = 0.01, gp_hdec = 0.98;
static int gp_win = 8;
static double gp_af = 0.25, gp_as = 0.06;
static int gp_nexp = 8;
static double gp_hsmooth = 0.002, gp_hdec2 = 0.97;
static double gp_aE = 0.05, gp_minE = 0.3;
struct Predictor {
    int prev = -1;
    map<int, double> hist;
    deque<int> recent;
    static const int K = 10;
    double w[K] = {1.0, 1.0, 0.1, 0.3, 0.2, 0.5, 0.5, 0.3, 0.3, 0.3};
    vector<double> hf, hb;  // histograms of distance from front / back
    double emaF = 0, emaS = 0, devF = 3, devS = 3;
    vector<double> E[K];

    static void lap(vector<double>& e, double mu, double scale) {
        int S = e.size();
        double t = 0;
        for (int i = 0; i < S; i++) { e[i] = exp(-fabs(i - mu) / scale); t += e[i]; }
        for (int i = 0; i < S; i++) e[i] /= t;
    }
    const vector<double>* estp = nullptr;
    double prevEst = 0, driftE = 0, scaleE = 2.0;
    bool haveEst = false;
    void updateEst(double estC) {
        if (haveEst) {
            double d = estC - prevEst;
            scaleE = (1 - gp_aE) * scaleE + gp_aE * fabs(d - driftE);
            driftE = (1 - gp_aE) * driftE + gp_aE * d;
        }
        prevEst = estC;
        haveEst = true;
    }
    void predict(int n, vector<double>& p, int ppIdx = -1) {
        int S = n + 1;
        for (int k = 0; k < K; k++) E[k].assign(S, 0.0);
        if (prev < 0) {
            for (int k = 0; k < K; k++)
                for (int i = 0; i < S; i++) E[k][i] = 1.0 / S;
        } else {
            int c = min(max(prev - 1, 0), n);
            double sc = 3.0;
            if (!recent.empty()) {
                double s = 0;
                for (int d : recent) s += abs(d);
                sc = max(gp_minsc, s / recent.size());
            }
            lap(E[1], c, sc);
            lap(E[3], c, sc * gp_wide);
            double t = 0;
            for (auto& [d, wt] : hist) {
                int i = c + d;
                if (i >= 0 && i < S) { E[0][i] += wt; t += wt; }
            }
            if (t > 0) {
                for (int i = 0; i < S; i++) E[0][i] = gp_hsm * E[0][i] / t + (1 - gp_hsm) * E[1][i];
            } else {
                E[0] = E[1];
            }
            for (int i = 0; i < S; i++) E[2][i] = 1.0 / S;
            {
                double q = gp_q, tt = 0;
                for (int i = 0; i < S; i++) {
                    int dd = min(i, n - i);
                    E[4][i] = pow(q, dd) + 1e-6;
                    tt += E[4][i];
                }
                for (int i = 0; i < S; i++) E[4][i] /= tt;
            }
            lap(E[5], min<double>(n, max<double>(0, c + emaF)), max(gp_minsc, devF));
            lap(E[6], min<double>(n, max<double>(0, c + emaS)), max(gp_minsc, devS));
            if (estp && !estp->empty() && haveEst) {
                const vector<double>& e = *estp;
                double mu = prevEst + driftE, b = max(gp_minE, scaleE);
                auto F = [&](double x) { return x < mu ? 0.5 * exp((x - mu) / b) : 1 - 0.5 * exp(-(x - mu) / b); };
                double prevF = 0, tt = 0;
                for (int i = 0; i < S; i++) {
                    double cur = (i < n) ? F(e[i]) : 1.0;
                    E[7][i] = max(0.0, cur - prevF) + 1e-9;
                    prevF = cur;
                    tt += E[7][i];
                }
                for (int i = 0; i < S; i++) E[7][i] /= tt;
            } else E[7] = E[3];
            {
                double tf = 0, tb = 0;
                for (int i = 0; i < S; i++) {
                    double a = (i < (int)hf.size() ? hf[i] : 0) + gp_hsmooth;
                    double b = ((n - i) < (int)hb.size() ? hb[n - i] : 0) + gp_hsmooth;
                    E[8][i] = a; E[9][i] = b; tf += a; tb += b;
                }
                for (int i = 0; i < S; i++) { E[8][i] /= tf; E[9][i] /= tb; }
            }
        }
        double W = 0;
        int KK = gp_nexp;
        for (int k = 0; k < KK; k++) W += w[k];
        p.assign(S, 0.0);
        for (int i = 0; i < S; i++) {
            double s = 0;
            for (int k = 0; k < KK; k++) s += w[k] * E[k][i];
            p[i] = s / W;
        }
    }
    void update(int slot, int n) {
        for (auto& x : hf) x *= gp_hdec2;
        for (auto& x : hb) x *= gp_hdec2;
        if ((int)hf.size() <= slot) hf.resize(slot + 1, 0.0);
        hf[slot] += 1;
        if ((int)hb.size() <= n - slot) hb.resize(n - slot + 1, 0.0);
        hb[n - slot] += 1;
        double W = 0;
        int KK = gp_nexp;
        for (int k = 0; k < KK; k++) { w[k] *= max(E[k][slot], 1e-12); W += w[k]; }
        for (int k = 0; k < KK; k++) w[k] = (1 - gp_mixf) * w[k] / W + gp_mixf / KK;
        if (prev >= 0) {
            int d = slot - (prev - 1);
            recent.push_back(d);
            if ((int)recent.size() > gp_win) recent.pop_front();
            for (auto it = hist.begin(); it != hist.end();) {
                it->second *= gp_hdec;
                if (it->second < 1e-4) it = hist.erase(it); else ++it;
            }
            hist[d] += 1.0;
            devF = (1 - gp_af) * devF + gp_af * fabs(d - emaF);
            devS = (1 - gp_as) * devS + gp_as * fabs(d - emaS);
            emaF = (1 - gp_af) * emaF + gp_af * d;
            emaS = (1 - gp_as) * emaS + gp_as * d;
        }
        prev = slot;
    }
};

static double g_tau = 0.04;
static double g_adjFrac = 0.05;
// ---------------- generic ordered-outcome search ----------------
// Outcomes 0..m-1 ordered; boundary i (between outcome i and i+1) is resolved by goLeftFn(i).
// Terminal region: single outcome, or (allowPair) two adjacent outcomes.
static int g_dp = 1;

static auto noStop = [](int, int) { return false; };
template <class F, class G = decltype(noStop)>
static pair<int,int> searchOutcomes(const vector<double>& w, F goLeftFn, bool allowPair, double theta = 1e9, G stopFn = noStop) {
    int m = w.size();
    vector<double> pref(m + 1, 0.0), podd(m + 1, 0.0);
    for (int i = 0; i < m; i++) { pref[i + 1] = pref[i] + w[i]; podd[i + 1] = podd[i] + ((i & 1) ? w[i] : 0.0); }
    // outcome o even = gap, odd = class. With allowPair, any region with <= 1 gap is terminal.
    auto terminal = [&](int a, int b) {
        if (a == b) return true;
        if (!allowPair) return false;
        int gaps = (b / 2) - ((a + 1) / 2) + 1;  // count even numbers in [a,b]
        if (gaps > 1) return false;
        if (a == 0 || b == m - 1) return true;  // end placement: equality knowledge is useless
        double cm = podd[b + 1] - podd[a], tm = pref[b + 1] - pref[a];
        return cm <= theta * tm;
    };
    auto medianSplit = [&](int a, int b) {
        // boundary k in [a, b-1], left = a..k
        double tot = pref[b + 1] - pref[a];
        double target = pref[a] + tot / 2;
        int k = int(lower_bound(pref.begin() + a + 1, pref.begin() + b + 1, target) - pref.begin()) - 1;
        int best = -1; double bd = 1e300;
        for (int kk = k - 1; kk <= k + 1; kk++) {
            if (kk < a || kk > b - 1) continue;
            double d = fabs(pref[kk + 1] - target);
            if (d < bd) { bd = d; best = kk; }
        }
        return best;
    };
    // adjust boundary k inside [a,b] so that an adjacent class outcome (ties) falls on the heavier side
    auto adjust = [&](int a, int b, int k) {
        if (!allowPair) return k;
        int cls = (k & 1) ? k : k + 1;  // the class adjacent to boundary k
        // boundary options: cls-1 (class on right), cls (class on left)
        int k1 = cls - 1, k2 = cls;
        if (k1 < a || k2 > b - 1) return k;
        if (w[cls] > g_adjFrac * (pref[b + 1] - pref[a])) return k;
        double left = pref[cls] - pref[a], right = pref[b + 1] - pref[cls + 1];
        return left >= right ? k2 : k1;
    };
    auto bisect = [&](int a, int b) {
        while (!terminal(a, b)) {
            int k = medianSplit(a, b);
            k = adjust(a, b, k);
            if (goLeftFn(k)) b = k; else a = k + 1;
            if (stopFn(a, b)) break;
        }
        return make_pair(a, b);
    };
    if (!g_dp || m <= 2) return bisect(0, m - 1);
    // leaves via recursive splitting
    vector<pair<int,int>> leaves;
    double tau = g_tau * pref[m];
    vector<pair<int,int>> st;
    st.push_back({0, m - 1});
    while (!st.empty()) {
        auto [a, b] = st.back();
        st.pop_back();
        double mass = pref[b + 1] - pref[a];
        if (terminal(a, b) || mass <= tau) { leaves.push_back({a, b}); continue; }
        int k = medianSplit(a, b);
        st.push_back({k + 1, b});
        st.push_back({a, k});
    }
    int M = leaves.size();
    vector<double> lw(M), lc(M);
    for (int i = 0; i < M; i++) {
        auto [a, b] = leaves[i];
        lw[i] = pref[b + 1] - pref[a];
        int sz = b - a + 1;
        double eff = allowPair ? (sz + 1) / 2.0 : sz;
        lc[i] = (terminal(a, b) || eff <= 1) ? 0.0 : lw[i] * log2(eff);
    }
    vector<double> pw(M + 1, 0.0);
    for (int i = 0; i < M; i++) pw[i + 1] = pw[i] + lw[i];
    static vector<double> C;
    static vector<int> K;
    C.assign(M * M, 0.0);
    K.assign(M * M, -1);
    for (int i = 0; i < M; i++) C[i * M + i] = lc[i];
    for (int len = 2; len <= M; len++) {
        for (int i = 0; i + len - 1 < M; i++) {
            int j = i + len - 1;
            int klo = i, khi = j - 1;
            if (len > 2) { klo = max(i, K[i * M + j - 1] - 0); khi = min(j - 1, K[(i + 1) * M + j] + 0); if (klo > khi) { klo = i; khi = j - 1; } }
            double best = 1e300; int bk = i;
            for (int k = klo; k <= khi; k++) {
                double v = C[i * M + k] + C[(k + 1) * M + j];
                if (v < best) { best = v; bk = k; }
            }
            C[i * M + j] = best + (pw[j + 1] - pw[i]);
            K[i * M + j] = bk;
        }
    }
    int i = 0, j = M - 1;
    int A = 0, B = m - 1;
    while (i < j && !terminal(A, B)) {
        int k = K[i * M + j];
        int bnd = adjust(A, B, leaves[k].second);
        if (goLeftFn(bnd)) { j = k; B = bnd; } else { i = k + 1; A = bnd + 1; }
        if (stopFn(A, B)) return make_pair(A, B);
    }
    return bisect(A, B);
}

// ---------------- sorted list with known-equal classes ----------------
struct SList {
    vector<int> el;        // labels, nondecreasing
    vector<char> eqPrev;   // el[i] known equal to el[i-1]
    // class boundaries
    vector<int> cs;        // class start indices, plus el.size() at end
    void buildClasses() {
        cs.clear();
        for (int i = 0; i < (int)el.size(); i++) if (i == 0 || !eqPrev[i]) cs.push_back(i);
        cs.push_back(el.size());
    }
    int numClasses() const { return (int)cs.size() - 1; }
    // insert x: outcome region [a,b] over outcomes (gap_j = 2j, class_j = 2j+1). returns insertion index
    int place(int x, int a, int b) {
        int pos; bool eq = false;
        if (a == b && (a & 1)) {  // class
            int j = a / 2;
            pos = cs[j + 1];
            eq = true;
        } else if (a == b) {
            pos = cs[a / 2];
        } else {
            // region containing exactly one gap: place at that gap
            int g = (a & 1) ? a + 1 : a;
            pos = cs[g / 2];
        }
        el.insert(el.begin() + pos, x);
        eqPrev.insert(eqPrev.begin() + pos, eq ? 1 : 0);
        return pos;
    }
    void popFront() {
        el.erase(el.begin()); eqPrev.erase(eqPrev.begin());
        if (!eqPrev.empty()) eqPrev[0] = 0;
    }
    void popBack() { el.pop_back(); eqPrev.pop_back(); }
};

// comparison at boundary i for inserting x into class structure
static bool boundaryGoLeft(const SList& S, int x, int i) {
    if ((i & 1) == 0) {  // gap_j | class_j : ask x < rep_j
        int j = i / 2;
        return lessQ(x, S.el[S.cs[j]]);
    } else {  // class_j | gap_{j+1} : ask rep_j < x ; true -> right
        int j = i / 2;
        return !lessQ(S.el[S.cs[j]], x);
    }
}

static double g_pNewMax = 0.95;
// insert x into S using class-aware CRP model (sorting phase, exact outcomes)
static void insertSortTie(SList& S, int x, int inserted) {
    S.buildClasses();
    int k = S.numClasses();
    int m = 2 * k + 1;
    vector<double> w(m);
    double pNew = (k + 0.5) / (inserted + 1.0);
    pNew = min(g_pNewMax, max(0.02, pNew));
    for (int j = 0; j <= k; j++) w[2 * j] = pNew / (k + 1);
    for (int j = 0; j < k; j++) w[2 * j + 1] = (1 - pNew) * (S.cs[j + 1] - S.cs[j]) / (double)inserted;
    auto r = searchOutcomes(w, [&](int i) { return boundaryGoLeft(S, x, i); }, false);
    S.place(x, r.first, r.second);
}

// merge sorted A (with classes) and sorted B (distinct, singletons) by Hwang-Lin
static SList hwangLin(const SList& A, const vector<int>& B) {
    // work from the top
    int m = A.el.size(), n = B.size();
    vector<int> outEl; vector<char> outEq;  // built reversed
    // A elements: process classes as units (compare rep, move whole class)
    // build class list of A
    vector<pair<int,int>> ac;  // [start,end)
    for (int i = 0; i < m; i++) if (i == 0 || !A.eqPrev[i]) ac.push_back({i, i + 1}); else ac.back().second = i + 1;
    int ma = ac.size();
    auto emitB = [&](int idx) { outEl.push_back(B[idx]); outEq.push_back(0); };
    auto emitAClass = [&](int c) {
        for (int i = ac[c].second - 1; i >= ac[c].first; i--) {
            outEl.push_back(A.el[i]);
            outEq.push_back(i > ac[c].first ? 1 : 0);
        }
    };
    while (ma > 0 && n > 0) {
        if (ma <= n) {
            int t = 0;
            while ((1LL << (t + 1)) * ma <= n) t++;
            int len = 1 << t;
            int x = A.el[ac[ma - 1].first];
            int pos = n - len;  // compare with B[pos]
            if (lessQ(x, B[pos])) {
                for (int q = n - 1; q >= pos; q--) emitB(q);
                n = pos;
            } else {
                // x >= B[pos]; binary search in B[pos+1 .. n-1]
                int lo = pos + 1, hi = n;
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    if (lessQ(x, B[mid])) hi = mid; else lo = mid + 1;
                }
                for (int q = n - 1; q >= lo; q--) emitB(q);
                n = lo;
                emitAClass(ma - 1);
                ma--;
            }
        } else {
            int t = 0;
            while ((1LL << (t + 1)) * n <= ma) t++;
            int len = 1 << t;
            int y = B[n - 1];
            int pc = ma - len;
            int ypc = A.el[ac[pc].first];
            if (lessQ(y, ypc)) {
                for (int q = ma - 1; q >= pc; q--) emitAClass(q);
                ma = pc;
            } else {
                int lo = pc + 1, hi = ma;
                while (lo < hi) {
                    int mid = (lo + hi) / 2;
                    if (lessQ(y, A.el[ac[mid].first])) hi = mid; else lo = mid + 1;
                }
                for (int q = ma - 1; q >= lo; q--) emitAClass(q);
                ma = lo;
                emitB(n - 1);
                n--;
            }
        }
    }
    while (n > 0) emitB(--n);
    while (ma > 0) emitAClass(--ma);
    SList R;
    R.el.assign(outEl.rbegin(), outEl.rend());
    // eq flags: outEq[i] refers to "equal to the element emitted after it in reversed order" -> i.e. equal to previous in ascending
    vector<char> eqr(outEq.rbegin(), outEq.rend());
    // eqr[i] = 1 means element i (ascending) is not the first of its class -> equal to previous
    R.eqPrev = eqr;
    if (!R.eqPrev.empty()) R.eqPrev[0] = 0;
    return R;
}

static double g_tiePrior = 0.05;
static int g_sample = 48;
static int g_dupThresh = 2;
static int g_checks = 16;
static int g_checkMin = 6;

static int g_runThresh = 16;
static int g_runClasses = 1;
static int g_runMinN = 64;

// maximal monotone run starting at label i (non-decreasing or strictly decreasing), returned ascending
static vector<int> getRun(int i) {
    vector<int> r{i};
    if (i == N) return r;
    bool desc = lessQ(i + 1, i);
    int j = i + 1;
    r.push_back(j);
    while (j < N) {
        bool cont = desc ? !lessQ(j, j + 1) : !lessQ(j + 1, j);
        if (!cont) break;
        j++;
        r.push_back(j);
    }
    if (desc) reverse(r.begin(), r.end());
    return r;
}

// spread-out equality checks on adjacent pairs of sorted S; optionally resolve all classes
static void detectTies(SList& S, bool& tieMode) {
    int M = S.el.size();
    vector<int> known(M, -1);
    int eq = 0, t = 0;
    int P = M - 1;
    int T = min(g_checks, P);
    vector<int> order;
    {
        vector<char> used(M, 0);
        for (int step = P; step >= 1 && (int)order.size() < T; step /= 2) {
            for (int i = 1 + step / 2; i < M && (int)order.size() < T; i += step) {
                if (!used[i]) { used[i] = 1; order.push_back(i); }
            }
        }
        for (int i = 1; i < M && (int)order.size() < T; i++) if (!used[i]) { used[i] = 1; order.push_back(i); }
    }
    for (int i : order) {
        if (t >= g_checkMin && eq == 0) break;
        if (eq >= g_dupThresh) break;
        bool lt = lessQ(S.el[i - 1], S.el[i]);
        known[i] = lt ? 0 : 1;
        t++;
        if (!lt) eq++;
    }
    g_tiePrior = (eq + 0.1) / (t + 1.0);
    tieMode = eq >= g_dupThresh;
    if (tieMode) g_tiePrior = max(g_tiePrior, 0.5);
    for (int i = 1; i < M; i++) {
        if (known[i] < 0 && tieMode) known[i] = lessQ(S.el[i - 1], S.el[i]) ? 0 : 1;
        S.eqPrev[i] = known[i] == 1;
    }
    if (M > 0) S.eqPrev[0] = 0;
}

static SList sortAll() {
    // run detection: cheap on random input, near-linear on presorted input
    if (N >= g_runMinN) {
        vector<vector<int>> runs{getRun(1)};
        int thr = min(N, g_runThresh);
        bool runMode = (int)runs[0].size() >= thr;
        if (!runMode && (int)runs[0].size() < N) {
            // a tie-prefix can hide a long descending run: probe one more run
            runs.push_back(getRun(runs[0].size() + 1));
            runMode = (int)runs[1].size() >= thr;
        }
        if (runMode) {
            int i = 1;
            for (auto& r : runs) i += r.size();
            while (i <= N) { runs.push_back(getRun(i)); i += runs.back().size(); }
#ifdef SIM
            if (getenv("DBGRUN")) { fprintf(stderr, "runs:"); for (auto& r : runs) fprintf(stderr, " %d", (int)r.size()); fprintf(stderr, "\n"); }
#endif
            // merge smallest first
            multimap<int, int> pq;
            for (int k = 0; k < (int)runs.size(); k++) pq.insert({(int)runs[k].size(), k});
            while (pq.size() > 1) {
                int a = pq.begin()->second; pq.erase(pq.begin());
                int b = pq.begin()->second; pq.erase(pq.begin());
                SList A; A.el = runs[a]; A.eqPrev.assign(A.el.size(), 0);
                SList R = hwangLin(A, runs[b]);
                runs[a] = R.el; runs[b].clear();
                pq.insert({(int)runs[a].size(), a});
            }
            SList S;
            S.el = runs[pq.begin()->second];
            S.eqPrev.assign(S.el.size(), 0);
            if (g_runClasses) { bool tm; detectTies(S, tm); }
            return S;
        }
    }
    int M = min(N, g_sample);
    vector<int> samp;
    for (int i = 1; i <= M; i++) samp.push_back(i);
    samp = fjSort(samp);
    SList S;
    S.el = samp;
    S.eqPrev.assign(M, 0);
    if (M == N) return S;
#ifdef SIM
    { extern long long cnt, simA; simA += cnt; }
#endif
    bool tieMode;
    detectTies(S, tieMode);
    if (tieMode) {
        for (int i = M + 1; i <= N; i++) insertSortTie(S, i, S.el.size());
        return S;
    }
    vector<int> rest;
    for (int i = M + 1; i <= N; i++) rest.push_back(i);
    vector<int> B = fjSort(rest);
#ifdef SIM
    { extern long long cnt, simB; simB += cnt; }
#endif
    return hwangLin(S, B);
}

static double g_eqAlpha = 4.0, g_theta = 0.05;
static int g_explore = 6;
static int g_signCert = 1;
static int g_allowPair = 1;

static void solve() {
    if (N == 1) return;
    if (N == 2) { addQ(1, 2); return; }
    SList S = sortAll();
#ifdef SIM
    { extern long long cnt, simSort; simSort += cnt; }
#endif
    Predictor P;
    vector<double> p;
    double eqObs = 0, resolved = 0;
    int prevc = -1, prevprev = -1;
    vector<double> est, rankOf(2 * N + 2, 0.0);
    for (int i = 0; i < (int)S.el.size(); i++) rankOf[S.el[i]] = i;
    while (S.el.size() >= 2) {
        if (S.el.size() == 2) {
            addQ(S.el[0], S.el[1]);
            break;
        }
        int a = S.el.front(), b = S.el.back();
        bool yTwin = S.eqPrev.back() != 0;              // b known equal to new last element
        bool xTwin = S.el.size() >= 2 && S.eqPrev[1];   // a known equal to new first element
        S.popBack();
        S.popFront();
        int c = addQ(a, b);
        int n = S.el.size();
        if (n == 1) {
            S.el.push_back(c); S.eqPrev.push_back(0);
            continue;
        }
#ifdef SIM
        extern long long cnt; long long simC0 = cnt;
#endif
        int ppIdx = -1;
        if (prevprev > 0) {
            auto it = find(S.el.begin(), S.el.end(), prevprev);
            if (it != S.el.end()) ppIdx = it - S.el.begin();
        }
        // value proxy: original ranks, carries spread evenly between neighbouring originals
        {
            est.assign(n, 0.0);
            int last = -1;  // index of last original seen
            for (int i = 0; i <= n; i++) {
                if (i < n && S.el[i] > N) continue;
                // carries in (last, i)
                int m = i - last - 1;
                double L = last >= 0 ? rankOf[S.el[last]] : -1e18, Rr = i < n ? rankOf[S.el[i]] : 1e18;
                for (int j = 0; j < m; j++) {
                    int idx = last + 1 + j;
                    if (last >= 0 && i < n) est[idx] = L + (Rr - L) * (j + 1) / (m + 1);
                    else if (last >= 0) est[idx] = L + (j + 1);
                    else if (i < n) est[idx] = Rr - (m - j);
                    else est[idx] = idx;
                }
                if (i < n) est[i] = rankOf[S.el[i]];
                last = i;
            }
            P.estp = &est;
        }
        P.predict(n, p, ppIdx);
        S.buildClasses();
        int k = S.numClasses();
        int m = 2 * k + 1;
        vector<double> w(m, 0.0);
        double eqRate = (eqObs + g_eqAlpha * g_tiePrior) / (resolved + g_eqAlpha);
        for (int j = 0; j < k; j++) {
            int s0 = S.cs[j], e0 = S.cs[j + 1];
            w[2 * j] = p[s0];
            double interior = 0;
            for (int t = s0 + 1; t < e0; t++) interior += p[t];
            w[2 * j + 1] = interior + eqRate * 0.5 * (p[s0] + p[e0]);
        }
        w[2 * k] = p[n];
        bool explore = resolved < g_explore;
        // sign certificate: c = a + b >= twin(b) => a >= 0 => all live values >= 0 (symmetric for <= 0)
        auto certified = [&](int lo, int hi) {
            return g_signCert && ((yTwin && lo >= 2 * k - 1) || (xTwin && hi <= 1));
        };
        auto r = searchOutcomes(w, [&](int i) { return boundaryGoLeft(S, c, i); }, (bool)g_allowPair, explore ? -1.0 : g_theta, certified);
        if (certified(r.first, r.second)) {
            // every remaining addition is safe in any order
            S.el.push_back(c);
            while (S.el.size() >= 2) {
                int u = S.el.back(); S.el.pop_back();
                int v = S.el.back(); S.el.pop_back();
                S.el.push_back(addQ(u, v));
            }
            return;
        }
        int pos = S.place(c, r.first, r.second);
        if (r.first == r.second) { resolved += 1; if (r.first & 1) eqObs += 1; }
#ifdef SIM
        extern double simEnt, simCost[3];
        { extern double simFloor; simFloor += (pos==0||pos==n)?1:2; }
        simCost[(a <= N) + (b <= N)] += cnt - simC0;
#endif
        {
            double l = pos > 0 ? est[pos - 1] : 0, r = pos < n ? est[pos] : 0, ec;
            if (pos > 0 && pos < n) ec = (l + r) / 2;
            else if (pos > 0) ec = l + 0.5;
            else ec = r - 0.5;
            P.updateEst(ec);
        }
        P.update(pos, n);
        prevprev = prevc; prevc = c;
    }
#ifdef SIM
    if (getenv("DBG2")) fprintf(stderr, "eqObs %.0f resolved %.0f prior %.3f\n", eqObs, resolved, g_tiePrior);
#endif
}

#ifndef SIM
int main() {
    if (scanf("%d", &N) != 1) return 0;
    if (N <= 0) return 0;
    solve();
    printf("!\n");
    fflush(stdout);
    return 0;
}
#endif

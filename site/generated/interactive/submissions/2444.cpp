#include <bits/stdc++.h>
using namespace std;

static int m;
static long long qcnt = 0;

#ifdef LOCAL
static vector<int> trueCls;
static vector<int> qpar, qedge, qans;
static int ask(int p, int e) {
    qcnt++;
    int a;
    if (p != 0 && qans[p] == 0) a = 0;
    else {
        int c = trueCls[e];
        if (c < 0) a = 0;
        else {
            a = 1;
            for (int v = p; v; v = qpar[v])
                if (trueCls[qedge[v]] == c) { a = 0; break; }
        }
    }
    qpar.push_back(p); qedge.push_back(e); qans.push_back(a);
    return a;
}
#else
static int ask(int p, int e) {
    printf("? %d %d\n", p, e);
    fflush(stdout);
    qcnt++;
    int a;
    if (scanf("%d", &a) != 1) exit(0);
    if (a < 0) exit(0);
    return a;
}
#endif

static const int BIG = 15;
static double modelBits = 0, floorBits = 0;
static double REB_H = 0.1, TRUST_MIN = 0.9;
static int SORT_SMALL = 1;
#ifdef TUNE
struct TuneInit{TuneInit(){if(getenv("SORT_SMALL"))SORT_SMALL=atoi(getenv("SORT_SMALL"));if(getenv("REB_H"))REB_H=atof(getenv("REB_H"));if(getenv("TRUST_MIN"))TRUST_MIN=atof(getenv("TRUST_MIN"));}} tuneInit;
#endif
static int REB_MAXNL = 300, REB_MAXCOST = 400;

// ---------------- exchangeable (shuffled labels) model ----------------
struct ExchModel {
    vector<double> Lv;      // grid of cycle lengths
    vector<double> nL;      // estimated number of classes per grid length
    vector<double> lgL;     // lgamma(L+1)
    void init(int M) {
        for (int L = 1; L <= 64 && L <= M; L++) Lv.push_back(L);
        double x = 64;
        while (true) {
            x *= 1.12;
            if (x > M + 0.5) break;
            Lv.push_back(floor(x));
        }
        nL.assign(Lv.size(), 0.05);
        for (double L : Lv) lgL.push_back(lgamma(L + 1));
    }
    // sVals: distinct s values, sCnt: number of classes with that s.
    // returns g(s) for each distinct s, and new weight
    void compute(int t, const vector<int>& sVals, const vector<int>& sCnt, vector<double>& g, double& newW, int iters) {
        double f = max((double)t, 0.5) / m;
        if (f > 0.999999) f = 0.999999;
        double lf = log(f), l1f = log1p(-f);
        int G = Lv.size(), S = sVals.size();
        vector<double> P((size_t)G * S, 0.0);
        for (int j = 0; j < S; j++) {
            int s = sVals[j];
            double lgs = lgamma(s + 1.0);
            for (int i = 0; i < G; i++) {
                double L = Lv[i];
                if (L < s) continue;
                double lp = lgL[i] - lgs - lgamma(L - s + 1) + s * lf + (L - s) * l1f;
                P[(size_t)i * S + j] = exp(lp);
            }
        }
        vector<double> det(G);
        for (int i = 0; i < G; i++) det[i] = max(1e-12, -expm1(Lv[i] * l1f));
        for (int it = 0; it < iters; it++) {
            vector<double> acc(G, 0.0);
            for (int j = 0; j < S; j++) {
                double den = 0;
                for (int i = 0; i < G; i++) den += nL[i] * P[(size_t)i * S + j];
                if (den <= 0) continue;
                double k = sCnt[j] / den;
                for (int i = 0; i < G; i++) acc[i] += nL[i] * P[(size_t)i * S + j] * k;
            }
            for (int i = 0; i < G; i++) nL[i] = (acc[i] + 0.02) / det[i];
        }
        g.assign(S, 0.0);
        for (int j = 0; j < S; j++) {
            double num = 0, den = 0;
            for (int i = 0; i < G; i++) {
                double w = nL[i] * P[(size_t)i * S + j];
                num += w * (Lv[i] - sVals[j]);
                den += w;
            }
            g[j] = den > 0 ? num / den : (sVals[j] + 1) * (1 - f) / f;
        }
        newW = 0;
        for (int i = 0; i < G; i++) newW += nL[i] * exp(Lv[i] * l1f) * Lv[i];
    }
};

// optimal alphabetic search tree (Knuth DP). returns expected cost (sum w * depth) and fills root.
static double alphaDP(const vector<double>& pre, int Lo, int n, vector<vector<int>>* rootOut) {
    vector<vector<double>> cost(n, vector<double>(n, 0.0));
    vector<vector<int>> root(n, vector<int>(n, 0));
    for (int i = 0; i < n; i++) root[i][i] = i;
    for (int len = 2; len <= n; len++)
        for (int i = 0; i + len - 1 < n; i++) {
            int j = i + len - 1;
            double W = pre[Lo + j + 1] - pre[Lo + i];
            int rl = (len == 2) ? i : root[i][j - 1];
            int rh = (len == 2) ? i : min(root[i + 1][j], j - 1);
            double best = 1e300; int br = rl;
            for (int r = rl; r <= rh; r++) {
                double cc = cost[i][r] + cost[r + 1][j];
                if (cc < best) { best = cc; br = r; }
            }
            cost[i][j] = best + W;
            root[i][j] = br;
        }
    if (rootOut) *rootOut = std::move(root);
    return cost[0][n - 1];
}

struct Leaf { int lo, hi; };
// chainCls: class at depth j+1. Builds leaves: bridge, classes (big runs merged), new.
static void buildLeaves(const vector<int>& chainCls, const vector<int>& csize, vector<Leaf>& leaves) {
    int k = chainCls.size();
    leaves.clear();
    leaves.push_back({0, 0});
    for (int c = 0; c < k;) {
        if (csize[chainCls[c]] >= BIG) {
            int d = c;
            while (d + 1 < k && csize[chainCls[d + 1]] >= BIG) d++;
            leaves.push_back({c + 1, d + 1});
            c = d + 1;
        } else {
            leaves.push_back({c + 1, c + 1});
            c++;
        }
    }
    leaves.push_back({k + 1, k + 1});
}

int main() {
#ifdef LOCAL
    if (scanf("%d", &m) != 1) return 0;
    trueCls.assign(m + 1, -1);
    for (int i = 1; i <= m; i++) if (scanf("%d", &trueCls[i]) != 1) return 0;
    qpar.push_back(0); qedge.push_back(0); qans.push_back(1);
#ifdef CHEAT
    { map<int,int> cc; for (int i=1;i<=m;i++) if(trueCls[i]>=0) cc[trueCls[i]]++; for (auto [a,b]:cc) cheatHist[b]++; }
#endif
#else
    if (scanf("%d", &m) != 1) return 0;
#endif

    vector<int> cls(m + 1, -2);
    vector<int> csize;            // class sizes by class id
    vector<int> crep;             // representative edge per class
    vector<int> chainCls;         // class at chain depth j+1
    vector<int> node;             // node[j] = query id of chain prefix j
    node.push_back(0);
    vector<int> mtf;              // classes, most recent first
    vector<int> sHist(m + 2, 0);  // number of classes with given size

    ExchModel ex;
    ex.init(m);

    double catR[3] = {1, 1, 1};   // decayed counts: bridge, new, existing
    const double DEC = 0.97;
    vector<double> rankCnt(20, 0.3);

    const int NM = 3;
    double mw[NM];
    for (int i = 0; i < NM; i++) mw[i] = 1.0 / NM;
    int nb = 0;

    vector<double> gS;
    vector<double> gBy(m + 2, 0.0);

    for (int e = 1; e <= m; e++) {
        int t = e - 1;
        int k = csize.size();
        int R = m - t;
        // ---- exchangeable model
        vector<int> sVals, sCnt;
        for (int s = 1; s <= m; s++) if (sHist[s]) { sVals.push_back(s); sCnt.push_back(sHist[s]); }
        double newW = 0;
        ex.compute(t, sVals, sCnt, gS, newW, 3);
        for (size_t j = 0; j < sVals.size(); j++) gBy[sVals[j]] = gS[j];
        double bW = (nb + 0.5) * (double)R / (t + 1.0);

        // ---- per-model probabilities: index 0 bridge, 1..k class id+1, k+1 new
        vector<vector<double>> pc(NM, vector<double>(k + 2, 0.0));
        {
            double tot = bW + newW;
            for (int c = 0; c < k; c++) tot += gBy[csize[c]];
            pc[0][0] = bW / tot;
            pc[0][k + 1] = newW / tot;
            for (int c = 0; c < k; c++) pc[0][c + 1] = gBy[csize[c]] / tot;
            double ct = catR[0] + catR[1] + (k > 0 ? catR[2] : 0);
            double pB = catR[0] / ct, pN = catR[1] / ct, pE = (k > 0 ? catR[2] / ct : 0);
            double gs = 0;
            for (int c = 0; c < k; c++) gs += gBy[csize[c]];
            pc[1][0] = pB; pc[1][k + 1] = pN;
            if (k > 0 && gs > 0)
                for (int c = 0; c < k; c++) pc[1][c + 1] = pE * gBy[csize[c]] / gs;
            pc[2][0] = pB; pc[2][k + 1] = pN;
            if (k > 0) {
                vector<double> rw(k);
                double rs = 0;
                for (int r = 0; r < k; r++) {
                    int b = r == 0 ? 0 : 32 - __builtin_clz(r);
                    double bsz = b == 0 ? 1 : (1 << (b - 1));
                    rw[r] = rankCnt[b] / bsz;
                    rs += rw[r];
                }
                for (int r = 0; r < k; r++) pc[2][mtf[r] + 1] = pE * rw[r] / rs;
            }
        }
        vector<double> wc(k + 2, 0.0);   // mixed weight per outcome id
        for (int i = 0; i < k + 2; i++) {
            for (int j = 0; j < NM; j++) wc[i] += mw[j] * pc[j][i];
            wc[i] = max(wc[i], 1e-9);
        }
        auto leafWeights = [&](const vector<int>& order, vector<Leaf>& lv, vector<double>& pre) {
            buildLeaves(order, csize, lv);
            int nl = lv.size();
            pre.assign(nl + 1, 0.0);
            for (int i = 0; i < nl; i++) {
                double s = 0;
                for (int x = lv[i].lo; x <= lv[i].hi; x++) {
                    int id = (x == 0) ? 0 : (x == (int)order.size() + 1 ? k + 1 : order[x - 1] + 1);
                    s += wc[id];
                }
                pre[i + 1] = pre[i] + s;
            }
        };

        vector<Leaf> leaves;
        vector<double> pre;
        leafWeights(chainCls, leaves, pre);
        int NL = leaves.size();

        // ---- consider chain rebuild: bigs first, then smalls
        {
            int fs = -1;
            for (int j = 0; j < k; j++) if (csize[chainCls[j]] < BIG) { fs = j; break; }
            bool need = false;
            if (fs >= 0)
                for (int j = fs; j < k; j++) if (csize[chainCls[j]] >= BIG) { need = true; break; }
            if (need && NL <= REB_MAXNL && k - fs <= REB_MAXCOST && mw[0] + mw[1] >= TRUST_MIN) {
                vector<int> nord(chainCls.begin(), chainCls.begin() + fs);
                for (int j = fs; j < k; j++) if (csize[chainCls[j]] >= BIG) nord.push_back(chainCls[j]);
                {
                    vector<int> sm;
                    for (int j = fs; j < k; j++) if (csize[chainCls[j]] < BIG) sm.push_back(chainCls[j]);
                    if (SORT_SMALL) stable_sort(sm.begin(), sm.end(), [&](int a, int b) { return csize[a] > csize[b]; });
                    for (int c : sm) nord.push_back(c);
                }
                vector<Leaf> lv2; vector<double> pre2;
                vector<double> wsave = wc;
                for (int i = 0; i < k + 2; i++) wc[i] = max(pc[0][i], 1e-9);
                vector<Leaf> lv1; vector<double> pre1;
                leafWeights(chainCls, lv1, pre1);
                leafWeights(nord, lv2, pre2);
                double Ecur = alphaDP(pre1, 0, NL, nullptr) / pre1[NL];
                double Enew = alphaDP(pre2, 0, lv2.size(), nullptr) / pre2[lv2.size()];
                wc = wsave;
                double cost = k - fs;
                double trust = mw[0] + mw[1];
                trust = trust < TRUST_MIN ? 0 : trust;
                if ((Ecur - Enew) * R * REB_H * trust > cost) {
                    leafWeights(nord, lv2, pre2);
                    node.resize(fs + 1);
                    for (int j = fs; j < k; j++) {
                        int a = ask(node[j], crep[nord[j]]);
                        (void)a;
                        node.push_back(qcnt);
                    }
                    chainCls = nord;
                    leaves = lv2; pre = pre2; NL = leaves.size();
                }
            }
        }

        // ---- search
        int Lo = 0, Hi = NL - 1;
        int lastNewNode = -1;
        const int DPN = 200;
        vector<vector<int>> root;
        int dpBase = -1;
        auto doTest = [&](int r) {
            int j = leaves[r].hi;
            int a = ask(node[j], e);
            if (a == 0) Hi = r;
            else {
                Lo = r + 1;
                if (j == k) lastNewNode = qcnt;
            }
        };
        while (Lo < Hi) {
            int n = Hi - Lo + 1;
            if (n <= DPN) {
                if (dpBase < 0) { dpBase = Lo; alphaDP(pre, Lo, n, &root); }
                doTest(root[Lo - dpBase][Hi - dpBase] + dpBase);
            } else {
                double half = (pre[Hi + 1] + pre[Lo]) / 2;
                int r = upper_bound(pre.begin() + Lo + 1, pre.begin() + Hi + 1, half) - pre.begin() - 1;
                int best = -1; double bd = 1e300;
                for (int c = r - 2; c <= r + 1; c++) {
                    if (c < Lo || c >= Hi) continue;
                    double d = fabs(pre[c + 1] - half);
                    if (d < bd) { bd = d; best = c; }
                }
                doTest(best);
            }
        }
        int leaf = Lo;
#ifdef LOCAL
        { double b = -log2((pre[leaf + 1] - pre[leaf]) / pre[NL]); modelBits += b; floorBits += max(b, (leaf == 0 || leaf == NL - 1) ? 1.0 : 2.0); }
#endif
        // ---- mixture update (per-model leaf probability)
        {
            double z = 0;
            for (int j = 0; j < NM; j++) {
                double p = 0;
                for (int x = leaves[leaf].lo; x <= leaves[leaf].hi; x++) {
                    int id = (x == 0) ? 0 : (x == k + 1 ? k + 1 : chainCls[x - 1] + 1);
                    p += pc[j][id];
                }
                mw[j] *= max(p, 1e-12); z += mw[j];
            }
            const double alpha = 0.01;
            for (int j = 0; j < NM; j++) mw[j] = (1 - alpha) * mw[j] / z + alpha / NM;
        }
        for (int i = 0; i < 3; i++) catR[i] *= DEC;
        for (auto& x : rankCnt) x = x * 0.995 + 0.0005;
        if (leaf == 0) {
            cls[e] = -1; nb++;
            catR[0] += 1;
        } else if (leaf == NL - 1) {
            int c = k;
            cls[e] = c;
            csize.push_back(1);
            crep.push_back(e);
            sHist[1]++;
            chainCls.push_back(c);
            node.push_back(lastNewNode);
            mtf.insert(mtf.begin(), c);
            catR[1] += 1;
        } else {
            catR[2] += 1;
            if (leaves[leaf].lo == leaves[leaf].hi) {
                int c = chainCls[leaves[leaf].lo - 1];
                cls[e] = c;
                sHist[csize[c]]--;
                csize[c]++;
                sHist[csize[c]]++;
                int pos = find(mtf.begin(), mtf.end(), c) - mtf.begin();
                int b = pos == 0 ? 0 : 32 - __builtin_clz(pos);
                rankCnt[b] += 1;
                mtf.erase(mtf.begin() + pos);
                mtf.insert(mtf.begin(), c);
            } else {
                cls[e] = -3;  // some big class
            }
        }
    }
    // output
    string out = "!";
    auto val = [&](int e) {
        if (cls[e] == -1) return -1;
        if (cls[e] == -3) return 0;
        return csize[cls[e]] >= BIG ? 0 : csize[cls[e]];
    };
    for (int e = 1; e <= m; e++) out += " " + to_string(val(e));
#ifdef LOCAL
    map<int, int> cnt;
    for (int e = 1; e <= m; e++) if (trueCls[e] >= 0) cnt[trueCls[e]]++;
    bool ok = true;
    for (int e = 1; e <= m; e++) {
        int v = trueCls[e] < 0 ? -1 : (cnt[trueCls[e]] >= BIG ? 0 : cnt[trueCls[e]]);
        if (v != val(e)) ok = false;
    }
    printf("%s %lld %.3f model=%.3f floor=%.3f\n", ok ? "OK" : "WRONG", qcnt, (double)qcnt / m, modelBits / m, floorBits / m);
#else
    printf("%s\n", out.c_str());
    fflush(stdout);
    int verdict;
    if (scanf("%d", &verdict) != 1) return 0;
#endif
    return 0;
}

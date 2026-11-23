#include <bits/stdc++.h>
using namespace std;

// Coins: types of components are unordered pairs {lo,hi} of side sizes.
static int N, M;
static double LAMBDA = 0.02;

struct Solver {
    vector<int> par, rel;      // rel: parity to parent
    vector<int> s0, s1;        // for roots: size of side with parity 0 (root side) and 1
    map<pair<int,int>, set<int>> byType;  // (lo,hi) -> roots
    int queries = 0;
    int FF = 0, FK = 0;  // fresh-fresh queries and how many answered "different"
    bool modeA = false;
    int chainRoot = -1;
    double PTHR = 1e-4; int FMAX = 333;

    static double lchoose(int n, int k) { return lgamma(n + 1.0) - lgamma(k + 1.0) - lgamma(n - k + 1.0); }
    double hyperCdf(int n, int k) { // P(X<=k), X~Hypergeo(N,M,n)
        double s = 0;
        for (int i = 0; i <= k && i <= M; i++) {
            if (n - i < 0 || n - i > N - M) continue;
            s += exp(lchoose(M, i) + lchoose(N - M, n - i) - lchoose(N, n));
        }
        return s;
    }
    int freshCount() { auto it = byType.find({0, 1}); return it == byType.end() ? 0 : it->second.size(); }
    int sumMin() { int s = 0; for (auto &kv : byType) s += kv.first.first * (int)kv.second.size(); return s; }
    static int groupSize(int r) {
        if (r % 2 == 1) return 2;
        if (r % 3 != 0) return 3;
        if (r % 4 != 0) return 4;
        return 5;
    }

    int find(int x, int &p) {
        p = 0;
        int r = x;
        while (par[r] != r) { p ^= rel[r]; r = par[r]; }
        // path compression
        int cur = x, cp = p;
        while (par[cur] != cur) {
            int nx = par[cur], np = cp ^ rel[cur];
            par[cur] = r; rel[cur] = cp;
            cur = nx; cp = np;
        }
        return r;
    }
    static pair<int,int> key(int a, int b) { return a < b ? make_pair(a, b) : make_pair(b, a); }

    void init() {
        par.resize(N + 1); rel.assign(N + 1, 0); s0.assign(N + 1, 1); s1.assign(N + 1, 0);
        for (int i = 1; i <= N; i++) { par[i] = i; byType[{0, 1}].insert(i); }
    }

    // polynomial ops truncated at degree M
    typedef array<double, 11> Poly;
    Poly one() { Poly p{}; p[0] = 1; return p; }
    Poly mul(const Poly &a, const Poly &b) {
        Poly c{};
        for (int i = 0; i <= M; i++) if (a[i] != 0)
            for (int j = 0; i + j <= M; j++) c[i + j] += a[i] * b[j];
        return c;
    }
    Poly powType(int lo, int hi, int m) {
        Poly base{};
        if (lo <= M) base[lo] += 1;
        if (hi <= M) base[hi] += 1;
        Poly r = one();
        Poly b = base;
        while (m) { if (m & 1) r = mul(r, b); m >>= 1; if (m) b = mul(b, b); }
        return r;
    }

    struct TypeInfo { int lo, hi, m; };

    vector<TypeInfo> types() {
        vector<TypeInfo> t;
        for (auto &kv : byType) if (!kv.second.empty()) t.push_back({kv.first.first, kv.first.second, (int)kv.second.size()});
        return t;
    }

    double total(const vector<TypeInfo> &t) {
        Poly p = one();
        for (auto &x : t) p = mul(p, powType(x.lo, x.hi, x.m));
        return p[M];
    }

    // pick best query: returns pair of type indices
    // Returns false if solved.
    bool choose(int &ra, int &rb) {
        vector<TypeInfo> t = types();
        int T = t.size();
        vector<Poly> full(T), red1(T), red2(T);
        for (int i = 0; i < T; i++) {
            full[i] = powType(t[i].lo, t[i].hi, t[i].m);
            red1[i] = powType(t[i].lo, t[i].hi, t[i].m - 1);
            red2[i] = t[i].m >= 2 ? powType(t[i].lo, t[i].hi, t[i].m - 2) : Poly{};
        }
        vector<Poly> pre(T + 1), suf(T + 1);
        pre[0] = one();
        for (int i = 0; i < T; i++) pre[i + 1] = mul(pre[i], full[i]);
        suf[T] = one();
        for (int i = T - 1; i >= 0; i--) suf[i] = mul(full[i], suf[i + 1]);
        double tot = pre[T][M];
        if (tot <= 1.5) return false;
        if (modeA && freshCount() >= 1) {
            int r = M - sumMin();
            int u = freshCount();
            if (u <= r) {
                auto &fs = byType[{0, 1}];
                if (chainRoot < 0) {
                    if (u >= 2) { ra = *fs.begin(); rb = *next(fs.begin()); chainRoot = ra; return true; }
                } else {
                    int p; ra = find(chainRoot, p); rb = *fs.begin(); return true;
                }
            }
            int g = groupSize(r);
            // extend largest uniform group smaller than g
            for (int sz = g - 1; sz >= 2; sz--) {
                auto it = byType.find({0, sz});
                if (it != byType.end() && !it->second.empty()) {
                    ra = *it->second.begin();
                    rb = *byType[{0, 1}].begin();
                    return true;
                }
            }
        }
        double best = -1; int bi = -1, bj = -1;
        auto eval = [&](int i, int j, const Poly &Q) {
            // Q = product excluding one comp of type i and one of type j
            int a1 = t[i].lo, a2 = t[i].hi, b1 = t[j].lo, b2 = t[j].hi;
            auto coef = [&](int c, int d) {
                double v = 0;
                if (c <= M) v += Q[M - c];
                if (d <= M) v += Q[M - d];
                return v;
            };
            double n0 = coef(a1 + b1, a2 + b2);
            double n1 = coef(a1 + b2, a2 + b1);
            double s = n0 + n1;
            if (s <= 0) return;
            double p = n0 / s;
            double H = 0;
            if (p > 0 && p < 1) H = -(p * log2(p) + (1 - p) * log2(1 - p));
            int fresh = (a1 + a2 == 1) + (b1 + b2 == 1);
            double score = H + LAMBDA * fresh;
            if (score > best) { best = score; bi = i; bj = j; }
        };
        for (int i = 0; i < T; i++) {
            if (t[i].m >= 2) {
                Poly Q = mul(mul(pre[i], red2[i]), suf[i + 1]);
                eval(i, i, Q);
            }
            Poly L = mul(pre[i], red1[i]);
            for (int j = i + 1; j < T; j++) {
                Poly Q = mul(mul(L, red1[j]), suf[j + 1]);
                eval(i, j, Q);
                L = mul(L, full[j]);
            }
        }
        auto &si = byType[{t[bi].lo, t[bi].hi}];
        auto &sj = byType[{t[bj].lo, t[bj].hi}];
        ra = *si.begin();
        if (bi == bj) rb = *next(si.begin()); else rb = *sj.begin();
        return true;
    }

    void apply(int a, int b, int ans) {
        // a,b are roots; ans 0: same type
        queries++;
        if (s0[a] + s1[a] == 1 && s0[b] + s1[b] == 1) {
            FF++; FK += ans;
            if (!modeA && FF <= FMAX && hyperCdf(2 * FF, FK) < PTHR) modeA = true;
        }
        auto &sa = byType[key(s0[a], s1[a])]; sa.erase(a);
        auto &sb = byType[key(s0[b], s1[b])]; sb.erase(b);
        // attach b under a
        par[b] = a; rel[b] = ans;
        if (ans == 0) { s0[a] += s0[b]; s1[a] += s1[b]; }
        else { s0[a] += s1[b]; s1[a] += s0[b]; }
        byType[key(s0[a], s1[a])].insert(a);
    }

    vector<int> answer() {
        vector<TypeInfo> t = types();
        int T = t.size();
        // DP over types choose k (#hi) per type
        vector<vector<double>> dp(T + 1, vector<double>(M + 1, 0));
        dp[T][0] = 1;
        for (int i = T - 1; i >= 0; i--)
            for (int s = 0; s <= M; s++) {
                double v = 0;
                for (int k = 0; k <= t[i].m; k++) {
                    long long c = (long long)k * t[i].hi + (long long)(t[i].m - k) * t[i].lo;
                    if (c <= s) v += dp[i + 1][s - c];
                }
                dp[i][s] = v;
            }
        vector<int> hiType(T, 0);
        int s = M;
        for (int i = 0; i < T; i++) {
            for (int k = 0; k <= t[i].m; k++) {
                long long c = (long long)k * t[i].hi + (long long)(t[i].m - k) * t[i].lo;
                if (c <= s && dp[i + 1][s - c] > 0) { hiType[i] = k; s -= c; break; }
            }
        }
        // for each root, the counterfeit side
        map<pair<int,int>, int> hiOf;
        for (int i = 0; i < T; i++) hiOf[{t[i].lo, t[i].hi}] = hiType[i] ? 1 : 0;
        vector<int> res;
        for (int x = 1; x <= N; x++) {
            int p; int r = find(x, p);
            int lo = min(s0[r], s1[r]), hi = max(s0[r], s1[r]);
            int chooseHi = hiOf[{lo, hi}];
            int cnt = chooseHi ? hi : lo;
            // which side of root is counterfeit: side 0 has s0[r]
            int cside;
            if (s0[r] == cnt && s1[r] != cnt) cside = 0;
            else if (s1[r] == cnt && s0[r] != cnt) cside = 1;
            else cside = 0;  // shouldn't happen
            if (cnt > 0 && p == cside) res.push_back(x);
        }
        return res;
    }
};

#ifndef SIM
int main() {
    if (!(cin >> N >> M)) return 0;
    // random relabeling so that structured hidden instances look random
    vector<int> lab(N + 1);
    iota(lab.begin(), lab.end(), 0);
    mt19937_64 rng((uint64_t)chrono::steady_clock::now().time_since_epoch().count() ^ ((uint64_t)random_device{}() << 1));
    shuffle(lab.begin() + 1, lab.end(), rng);
    Solver S; S.init();
    while (true) {
        int a, b;
        if (!S.choose(a, b)) break;
        cout << "? " << lab[a] << " " << lab[b] << "\n" << flush;
        int r;
        if (!(cin >> r) || (r != 0 && r != 1)) return 0;
        S.apply(a, b, r);
    }
    vector<int> res = S.answer();
    for (int &x : res) x = lab[x];
    sort(res.begin(), res.end());
    cout << "!";
    for (int x : res) cout << " " << x;
    cout << "\n" << flush;
    return 0;
}
#endif

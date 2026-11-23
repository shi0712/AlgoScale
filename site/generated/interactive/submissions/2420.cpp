// core solver, parameterized by judge
#include <bits/stdc++.h>
using namespace std;

struct Params {
    double cs = 1.0;   // block size coefficient for small primes
    double cl = 1.0;   // block size coefficient for large primes
    int smallSingles = 4; // check after each of the first k primes
    int tail = 1;
    int dp = 1;
    double alpha = 0.5;
    vector<pair<int,int>> pre; // (block size, check a)
};

template <class J>
struct Solver {
    J &judge;
    int n;
    Params P;
    vector<char> inSim, cand;
    int simSize = 0, candCnt = 0, uCnt = 0;
    vector<int> primes;
    long long ops = 0;
    vector<int> removedLog;
    bool dead = false;
    Solver(J &j, int n_, Params p) : judge(j), n(n_), P(p) {}

    int ask(char t, int a) {
        ops++;
        int r = judge.query(t, a);
        if (r < 0) { dead = true; return r; }
        int sc;
        if (a == 1) sc = simSize;
        else { sc = 0; for (int y = a; y <= n; y += a) sc += inSim[y]; }
        int bit = r - sc;
        if (bit == 1) {
            for (int y = 1; y <= n; y++) if (cand[y]) {
                bool keep = (y % a == 0) && !inSim[y];
                if (!keep) { cand[y] = 0; candCnt--; if (inSim[y]) uCnt--; removedLog.push_back(y); }
            }
        } else {
            for (int y = a; y <= n; y += a) if (cand[y] && !inSim[y]) { cand[y] = 0; candCnt--; removedLog.push_back(y); }
        }
        if (t == 'B') {
            for (int y = a; y <= n; y += a) if (inSim[y]) {
                inSim[y] = 0; simSize--;
                if (cand[y]) uCnt--;
            }
        }
        return r;
    }

    int finish() {
        int ans = 1;
        for (int y = 1; y <= n; y++) if (cand[y]) { ans = y; break; }
        ops++;
        judge.answer(ans);
        return ans;
    }

    int run() {
        inSim.assign(n + 1, 1); inSim[0] = 0; simSize = n;
        cand.assign(n + 1, 1); cand[0] = 0; candCnt = n; uCnt = n;
        vector<char> comp(n + 1, 0);
        for (int i = 2; i <= n; i++) if (!comp[i]) { primes.push_back(i); for (long long j = 1LL * i * i; j <= n; j += i) comp[j] = 1; }
        int np = primes.size();
        int nsmall = 0; while (nsmall < np && 1LL * primes[nsmall] * primes[nsmall] <= n) nsmall++;
        // Phase 1
        int i = 0;
        int blockEnd = 0;
        int preIdx = 0, curCheck = 1;
        vector<int> dpBlock(np + 2, 1);
        if (P.dp && nsmall < np) {
            vector<double> f(np + 2, 0), w(np + 2, 0), W(np + 3, 0);
            for (int k = 0; k < np; k++) w[k] = pow((double)(k + 1), -P.alpha);
            // W[k] = sum_{j>=k} w[j] + weight of x=1
            W[np] = pow((double)(np + 1), -P.alpha);
            for (int k = np - 1; k >= 0; k--) W[k] = W[k + 1] + w[k];
            for (int st = np - 1; st >= nsmall; st--) {
                double R = W[st];
                double best = 1e18; int bs = 1;
                double inb = 0; // sum_{j=1..b} w[st+j-1]*j  (adjusted for last)
                for (int b = 1; st + b <= np; b++) {
                    inb += w[st + b - 1] * b;
                    double v = R * (b + 1) + (inb - w[st + b - 1]) + f[st + b];
                    if (v < best) { best = v; bs = b; }
                    if (R * (b + 1) > best) break;
                }
                f[st] = best; dpBlock[st] = bs;
            }
        }
        auto nextBlock = [&](int start) {
            int b;
            curCheck = 1;
            if (preIdx < (int)P.pre.size()) { b = P.pre[preIdx].first; curCheck = P.pre[preIdx].second; preIdx++; }
            else if (start < P.smallSingles) b = 1;
            else if (start < nsmall) b = max(1, (int)floor(P.cs * sqrt((double)start)));
            else if (P.dp) b = dpBlock[start];
            else b = max(1, (int)floor(P.cl * sqrt((double)(start))));
            if (P.tail && !(P.dp && start >= nsmall)) b = min(b, max(1, (np - start + 1) / 2));
            if (P.dp && start < nsmall && start + b > nsmall) b = nsmall - start;
            return start + b;
        };
        blockEnd = nextBlock(0);
        while (uCnt > 0 && candCnt > 1) {
            if (i >= np) {
                // only x=1 remains unprotected; protected ones pending
                ask('A', 1); if (dead) return 0;
                break;
            }
            ask('B', primes[i]); if (dead) return 0;
            i++;
            if (uCnt == 0 || candCnt <= 1) break;
            if (i >= blockEnd) {
                if (candCnt - uCnt > 0) { ask('A', curCheck); if (dead) return 0; }
                blockEnd = nextBlock(i);
            }
        }
        if (candCnt > 1) phase2();
        if (dead) return 0;
        return finish();
    }

    void phase2() {
        // all candidates protected
        vector<int> cnt(n + 1, 0);
        int maxc = 0;
        for (int y = 1; y <= n; y++) if (cand[y]) maxc = y;
        for (int a = 1; a <= maxc; a++) for (int y = a; y <= maxc; y += a) cnt[a] += cand[y];
        vector<int> spf(maxc + 1, 0);
        for (int i2 = 2; i2 <= maxc; i2++) if (!spf[i2]) for (int j = i2; j <= maxc; j += i2) if (!spf[j]) spf[j] = i2;
        vector<int> divs;
        auto removeY = [&](int y) {
            divs.assign(1, 1);
            int t = y;
            while (t > 1) {
                int p = spf[t], e = 0;
                while (t % p == 0) { t /= p; e++; }
                size_t sz = divs.size();
                int pw = 1;
                for (int k = 0; k < e; k++) { pw *= p; for (size_t q = 0; q < sz; q++) divs.push_back(divs[q] * pw); }
            }
            for (int d : divs) cnt[d]--;
        };
        auto doAsk = [&](int a) {
            removedLog.clear();
            ask('A', a);
            for (int y : removedLog) removeY(y);
        };
        vector<int> act;
        for (int a = 2; a <= maxc; a++) if (cnt[a] >= 2) act.push_back(a);
        while (candCnt > 1) {
            int m = candCnt;
            int best = -1, bestv = 0;
            size_t w = 0;
            for (size_t r = 0; r < act.size(); r++) {
                int a = act[r];
                if (cnt[a] < 2) continue;
                act[w++] = a;
                int v = min(cnt[a], m - cnt[a]);
                if (v > bestv) { bestv = v; best = a; }
            }
            act.resize(w);
            if (bestv < 2) break;
            doAsk(best); if (dead) return;
        }
        while (candCnt > 1) {
            for (int y = 1; y <= maxc && candCnt > 1; y++) if (cand[y]) {
                int a = -1;
                for (int d = 2; 1LL * d * d <= y && a < 0; d++) if (y % d == 0 && cnt[d] == 1) a = d;
                if (a < 0) {
                    vector<int> ds;
                    for (int d = 1; 1LL * d * d <= y; d++) if (y % d == 0) ds.push_back(y / d);
                    reverse(ds.begin(), ds.end());
                    for (int d : ds) if (d >= 2 && cnt[d] == 1) { a = d; break; }
                }
                if (a < 0) continue;
                doAsk(a); if (dead) return;
            }
        }
    }
};

static const char* TREES[] = {"",
"L1",
"B2{}{A1{L2}{L1}}",
"B2{}{A1{L2}{B3{}{A1{L3}{L1}}}}",
"B2{}{A1{A4{L4}{L2}}{B3{}{A1{L3}{L1}}}}",
"B2{}{A1{A4{L4}{L2}}{B3{}{A1{L3}{B5{}{A1{L5}{L1}}}}}}",
"B2{}{B3{L6}{A1{A2{A4{L4}{L2}}{L3}}{B5{}{A1{L5}{L1}}}}}",
"B2{}{B3{L6}{A1{A2{A4{L4}{L2}}{L3}}{B5{}{A1{L5}{B7{}{A1{L7}{L1}}}}}}}",
"B2{}{A4{A8{L8}{L4}}{B3{L6}{A1{A2{L2}{L3}}{B5{}{A1{L5}{B7{}{A1{L7}{L1}}}}}}}}",
"B2{}{A1{A4{A8{L8}{L4}}{A3{L6}{L2}}}{B3{}{A1{A9{L9}{L3}}{B5{}{A1{L5}{B7{}{A1{L7}{L1}}}}}}}}",
"B2{}{A1{A4{A8{L8}{L4}}{A3{L6}{A5{L10}{L2}}}}{B3{}{A1{A9{L9}{L3}}{B5{}{A1{L5}{B7{}{A1{L7}{L1}}}}}}}}",
"B2{}{A1{A4{A8{L8}{L4}}{A3{L6}{A5{L10}{L2}}}}{B3{}{A1{A9{L9}{L3}}{B5{}{A1{L5}{B7{}{A1{L7}{B11{}{A1{L11}{L1}}}}}}}}}}",
"B2{}{B3{A4{L12}{L6}}{A1{A2{A4{A8{L8}{L4}}{A5{L10}{L2}}}{A9{L9}{L3}}}{B5{}{A1{L5}{B7{}{A1{L7}{B11{}{A1{L11}{L1}}}}}}}}}",
"B2{}{B3{A4{L12}{L6}}{A1{A2{A4{A8{L8}{L4}}{A5{L10}{L2}}}{A9{L9}{L3}}}{B5{}{B7{}{A1{A5{L5}{L7}}{B11{}{A1{L11}{B13{}{A1{L13}{L1}}}}}}}}}}",
"B2{}{B3{A4{L12}{L6}}{A1{A2{A4{A8{L8}{L4}}{A5{L10}{A7{L14}{L2}}}}{A9{L9}{L3}}}{B5{}{B7{}{A1{A5{L5}{L7}}{B11{}{A1{L11}{B13{}{A1{L13}{L1}}}}}}}}}}",
"B2{}{B3{A4{L12}{L6}}{A2{A4{A8{L8}{L4}}{A5{L10}{A7{L14}{L2}}}}{B5{L15}{A1{A3{A9{L9}{L3}}{L5}}{B7{}{A1{L7}{B11{}{A1{L11}{B13{}{A1{L13}{L1}}}}}}}}}}}",
"B2{}{B3{A4{L12}{L6}}{A2{A4{A8{A16{L16}{L8}}{L4}}{A5{L10}{A7{L14}{L2}}}}{B5{L15}{A1{A3{A9{L9}{L3}}{L5}}{B7{}{A1{L7}{B11{}{A1{L11}{B13{}{A1{L13}{L1}}}}}}}}}}}",
"B2{}{B3{A4{L12}{L6}}{A2{A4{A8{A16{L16}{L8}}{L4}}{A5{L10}{A7{L14}{L2}}}}{B5{L15}{A1{A3{A9{L9}{L3}}{L5}}{B7{}{B11{}{A1{A7{L7}{L11}}{B13{}{A1{L13}{B17{}{A1{L17}{L1}}}}}}}}}}}}",
"B2{}{B3{A4{L12}{A9{L18}{L6}}}{A2{A4{A8{A16{L16}{L8}}{L4}}{A5{L10}{A7{L14}{L2}}}}{B5{L15}{A1{A3{A9{L9}{L3}}{L5}}{B7{}{B11{}{A1{A7{L7}{L11}}{B13{}{A1{L13}{B17{}{A1{L17}{L1}}}}}}}}}}}}",
"B2{}{B3{A4{L12}{A9{L18}{L6}}}{A2{A4{A8{A16{L16}{L8}}{L4}}{A5{L10}{A7{L14}{L2}}}}{B5{L15}{A1{A3{A9{L9}{L3}}{L5}}{B7{}{B11{}{A1{A7{L7}{L11}}{B13{}{A1{L13}{B17{}{A1{L17}{B19{}{A1{L19}{L1}}}}}}}}}}}}}}",
"B2{}{A4{A8{A16{L16}{L8}}{A3{L12}{A5{L20}{L4}}}}{B3{A9{L18}{L6}}{B5{A2{L10}{L15}}{B7{L14}{A1{A3{A9{L9}{L3}}{A2{L2}{A5{L5}{L7}}}}{B11{}{B13{}{A1{A11{L11}{L13}}{B17{}{A1{L17}{B19{}{A1{L19}{L1}}}}}}}}}}}}}",
"B2{}{B3{A4{L12}{A9{L18}{L6}}}{A2{A4{A8{A16{L16}{L8}}{A5{L20}{L4}}}{A5{L10}{A7{L14}{L2}}}}{B5{L15}{B7{L21}{A1{A3{A9{L9}{L3}}{A5{L5}{L7}}}{B11{}{B13{}{A1{A11{L11}{L13}}{B17{}{A1{L17}{B19{}{A1{L19}{L1}}}}}}}}}}}}}",
"B2{}{B3{A4{L12}{A9{L18}{L6}}}{A2{A4{A8{A16{L16}{L8}}{A5{L20}{L4}}}{A5{L10}{A7{L14}{A11{L22}{L2}}}}}{B5{L15}{B7{L21}{A1{A3{A9{L9}{L3}}{A5{L5}{L7}}}{B11{}{B13{}{A1{A11{L11}{L13}}{B17{}{A1{L17}{B19{}{A1{L19}{L1}}}}}}}}}}}}}",
"B2{}{B3{A4{L12}{A9{L18}{L6}}}{A2{A4{A8{A16{L16}{L8}}{A5{L20}{L4}}}{A5{L10}{A7{L14}{A11{L22}{L2}}}}}{B5{L15}{B7{L21}{A1{A3{A9{L9}{L3}}{A5{L5}{L7}}}{B11{}{B13{}{A1{A11{L11}{L13}}{B17{}{A1{L17}{B19{}{A1{L19}{B23{}{A1{L23}{L1}}}}}}}}}}}}}}}",
"B2{}{B3{A4{A8{L24}{L12}}{A9{L18}{L6}}}{A2{A4{A8{A16{L16}{L8}}{A5{L20}{L4}}}{A5{L10}{A7{L14}{A11{L22}{L2}}}}}{B5{L15}{B7{L21}{A1{A3{A9{L9}{L3}}{A5{L5}{L7}}}{B11{}{B13{}{A1{A11{L11}{L13}}{B17{}{A1{L17}{B19{}{A1{L19}{B23{}{A1{L23}{L1}}}}}}}}}}}}}}}"};
static const int NTREES = 24;

struct IOJudge {
    int query(char t, int a) {
        printf("%c %d\n", t, a);
        fflush(stdout);
        int r;
        if (scanf("%d", &r) != 1) exit(0);
        if (r < 0) exit(0);
        return r;
    }
    void answer(int a) { printf("C %d\n", a); fflush(stdout); }
};

// optimal precomputed decision tree for tiny n
static void runTree(int n, const char* s) {
    IOJudge J;
    vector<char> inSim(n + 1, 1);
    size_t p = 0;
    string t(s);
    auto readNum = [&]() { int v = 0; while (p < t.size() && isdigit((unsigned char)t[p])) v = v * 10 + (t[p++] - '0'); return v; };
    // skip a balanced {...} block starting at p (t[p]=='{'), return position after it
    auto skipBlock = [&](size_t q) { int d = 0; do { if (t[q] == '{') d++; else if (t[q] == '}') d--; q++; } while (d > 0); return q; };
    while (true) {
        char c = t[p++];
        if (c == 'L') { int x = readNum(); J.answer(x); return; }
        int a = readNum();
        int sc = 0; for (int y = a; y <= n; y += a) sc += inSim[y];
        int r = J.query(c, a);
        int bit = r - sc;
        if (c == 'B') for (int y = a; y <= n; y += a) inSim[y] = 0;
        // t[p] == '{' : positive child
        if (bit == 1) { p++; }
        else { p = skipBlock(p); p++; }
    }
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    if (n < NTREES) { runTree(n, TREES[n]); return 0; }
    IOJudge J;
    Params P;
    P.cs = 1.5; P.smallSingles = 1; P.tail = 1; P.dp = 1; P.alpha = 0.5;
    P.pre = {{2, 2}, {2, 1}, {2, 1}};
    Solver<IOJudge> S(J, n, P);
    S.run();
    return 0;
}

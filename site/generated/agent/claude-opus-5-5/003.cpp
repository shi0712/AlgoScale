#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef unsigned int ui;
static const ui MOD = 998244353;
static inline ui addm(ui a, ui b) { ui c = a + b; return c >= MOD ? c - MOD : c; }
static inline ui subm(ui a, ui b) { return a >= b ? a - b : a + MOD - b; }
static inline ui mulm(ui a, ui b) { return (ull)a * b % MOD; }

static int n, M, p0;
static vector<int> a; // 1-indexed

// ---------------- per-period direct computation ----------------
static inline int Uof(long long pos) { // pos >= 1
    long long l = (pos - 1) / n; int i = (int)((pos - 1) % n) + 1;
    return a[i] - (int)l;
}
static int sufOnesStart; // smallest i with a_i..a_n all 1 (n+1 if a_n != 1)
static int lastIdx2;     // last index with a>=2 (0 if none)

static ui periodCount(int k) {
    long long P = (long long)k * n + p0;
    long long base = P - n + 1; // idx 0
    int N2 = 2 * n;
    vector<int> u(N2);
    for (int i = 0; i < N2; i++) u[i] = Uof(base + i);
    vector<ui> L(n, 0), S(n, 0);
    {
        vector<int> st; st.reserve(n);
        L[n - 1] = 1; S[n - 1] = 1; st.push_back(n - 1);
        int nextPos = n - 1;
        for (int i = n - 2; i >= 0; i--) {
            if (u[i] <= 0) continue;
            L[i] = S[nextPos];
            while (!st.empty() && u[st.back()] <= u[i]) st.pop_back();
            S[i] = addm(L[i], st.empty() ? 0 : S[st.back()]);
            st.push_back(i);
            nextPos = i;
        }
    }
    // R over idx [n-1, 2n-1]
    vector<ui> R(N2, 0), PRs(N2 + 1, 0), PWs(N2 + 1, 0);
    vector<int> pg(N2, -1);
    {
        vector<int> st; st.reserve(n);
        R[n - 1] = 1; st.push_back(n - 1);
        PRs[n - 1] = 0; PRs[n] = 1;
        for (int i = n; i < N2; i++) {
            if (u[i] >= 1) {
                while (!st.empty() && u[st.back()] < u[i]) st.pop_back();
                pg[i] = st.back();
                R[i] = subm(PRs[i], PRs[pg[i]]);
                st.push_back(i);
            }
            PRs[i + 1] = addm(PRs[i], R[i]);
        }
    }
    // g
    vector<ui> g(N2, 0);
    {
        int last = -1; int zc = 0; int firstZero = -1;
        auto flush = [&]() {
            if (last >= 0 && zc > 0) {
                long long orig = base + firstZero - n;
                long long lp = (orig - 1) / n; int idx = (int)((orig - 1) % n) + 1;
                bool ok = true;
                if (lp == 0 && idx < sufOnesStart) ok = false;
                if (ok) g[last] = zc;
            }
        };
        for (int i = n - 1; i < N2; i++) {
            if (u[i] >= 1) { flush(); last = i; zc = 0; firstZero = -1; }
            else if (u[i] == 0) { if (zc == 0) firstZero = i; zc++; }
        }
    }
    PWs[n - 1] = 0;
    for (int i = n - 1; i < N2; i++) PWs[i + 1] = addm(PWs[i], mulm(R[i], g[i]));
    ui res = 0;
    int lapP = (int)((P - 1) / n);
    long long omega = lastIdx2 ? (long long)n + lastIdx2 : -1;
    for (int i = 0; i < n; i++) {
        if (u[i] < 2) continue;
        long long pos = base + i;
        int lp = (int)((pos - 1) / n);
        if (lp < 1) continue;
        int y = i + n;
        int p = pg[y];
        ui A = subm(PRs[y], PRs[p]);
        if (lp == 1 && lapP == 1 && omega >= 0) {
            long long oi = omega - base; // idx of omega
            long long hiE = min<long long>(oi, y);
            if (hiE > p) A = subm(A, subm(PRs[hiE], PRs[p]));
        }
        ui B = subm(PWs[y], PWs[p]);
        res = addm(res, mulm(L[i], addm(A, B)));
    }
    return res;
}

// ---------------- segment tree over threshold t ----------------
struct Tag { ui p, q, r, w, z, k1, k2, k3; };
static vector<Tag> T;          // tag or value (p,q,r = alpha,beta,gamma for leaves)
static vector<int> LC, RC;     // LC==0 => leaf
static vector<int> freeList;
static int lo_, hi_;

static inline Tag idTag() { return Tag{1, 0, 1, 0, 1, 0, 0, 0}; }
static inline bool isId(const Tag& t) { return t.p == 1 && t.q == 0 && t.r == 1 && t.w == 0 && t.z == 1 && t.k1 == 0 && t.k2 == 0 && t.k3 == 0; }
// apply t2 after t1
static inline Tag compose(const Tag& t1, const Tag& t2) {
    Tag o;
    o.p = mulm(t2.p, t1.p);
    o.k1 = addm(mulm(t2.p, t1.k1), t2.k1);
    o.q = (ui)(((ull)t2.q * t1.p + (ull)t2.r * t1.q) % MOD);
    o.r = mulm(t2.r, t1.r);
    o.w = (ui)(((ull)t2.r * t1.w + (ull)t2.w * t1.z) % MOD);
    o.k2 = (ui)(((ull)t2.q * t1.k1 + (ull)t2.r * t1.k2 + (ull)t2.w * t1.k3 + t2.k2) % MOD);
    o.z = mulm(t2.z, t1.z);
    o.k3 = addm(mulm(t2.z, t1.k3), t2.k3);
    return o;
}
static inline void applyVal(Tag& v, const Tag& t) { // v.p=alpha,v.q=beta,v.r=gamma
    ui al = v.p, be = v.q, ga = v.r;
    v.p = addm(mulm(t.p, al), t.k1);
    v.q = (ui)(((ull)t.q * al + (ull)t.r * be + (ull)t.w * ga + t.k2) % MOD);
    v.r = addm(mulm(t.z, ga), t.k3);
}
static inline int newNode() {
    int x;
    if (!freeList.empty()) { x = freeList.back(); freeList.pop_back(); }
    else { x = (int)T.size(); T.push_back(Tag()); LC.push_back(0); RC.push_back(0); }
    LC[x] = RC[x] = 0;
    return x;
}
static inline int newLeaf(ui al, ui be, ui ga) {
    int x = newNode(); T[x].p = al; T[x].q = be; T[x].r = ga; return x;
}
static void freeTree(int x) {
    // iterative
    static vector<int> st; st.clear(); st.push_back(x);
    while (!st.empty()) { int y = st.back(); st.pop_back(); if (LC[y]) { st.push_back(LC[y]); st.push_back(RC[y]); } freeList.push_back(y); }
}
static inline void applyTag(int x, const Tag& t) {
    if (!LC[x]) applyVal(T[x], t);
    else T[x] = compose(T[x], t);
}
static inline void pushdown(int x) {
    if (!isId(T[x])) { Tag t = T[x]; applyTag(LC[x], t); applyTag(RC[x], t); T[x] = idTag(); }
}
static inline void splitLeaf(int x) {
    Tag v = T[x];
    int l = newLeaf(v.p, v.q, v.r), r = newLeaf(v.p, v.q, v.r);
    LC[x] = l; RC[x] = r; T[x] = idTag();
}
static void rangeApply(int x, int L, int R, int ql, int qr, const Tag& t) {
    if (ql <= L && R <= qr) { applyTag(x, t); return; }
    if (!LC[x]) splitLeaf(x); else pushdown(x);
    int mid = (L + R) >> 1;
    if (ql <= mid) rangeApply(LC[x], L, mid, ql, qr, t);
    if (qr > mid) rangeApply(RC[x], mid + 1, R, ql, qr, t);
}
static void rangeAssign(int x, int L, int R, int ql, int qr, ui al, ui be, ui ga) {
    if (ql <= L && R <= qr) {
        if (LC[x]) { freeTree(LC[x]); freeTree(RC[x]); LC[x] = RC[x] = 0; }
        T[x].p = al; T[x].q = be; T[x].r = ga; return;
    }
    if (!LC[x]) splitLeaf(x); else pushdown(x);
    int mid = (L + R) >> 1;
    if (ql <= mid) rangeAssign(LC[x], L, mid, ql, qr, al, be, ga);
    if (qr > mid) rangeAssign(RC[x], mid + 1, R, ql, qr, al, be, ga);
}
static inline Tag leftMul(ui l1, ui l2, ui l3) { return Tag{l1, 0, l1, l2, l3, 0, 0, 0}; }
static inline Tag rightMul(ui r1, ui r2, ui r3) { return Tag{r1, r2, r3, 0, r3, 0, 0, 0}; }
// pointwise x*y, consumes both
static int mulTree(int x, int y) {
    if (!LC[x]) { Tag v = T[x]; applyTag(y, leftMul(v.p, v.q, v.r)); freeList.push_back(x); return y; }
    if (!LC[y]) { Tag v = T[y]; applyTag(x, rightMul(v.p, v.q, v.r)); freeList.push_back(y); return x; }
    pushdown(x); pushdown(y);
    int l = mulTree(LC[x], LC[y]);
    int r = mulTree(RC[x], RC[y]);
    LC[x] = l; RC[x] = r;
    freeList.push_back(y);
    if (!LC[l] && !LC[r] && T[l].p == T[r].p && T[l].q == T[r].q && T[l].r == T[r].r) {
        T[x] = T[l]; LC[x] = RC[x] = 0; freeList.push_back(l); freeList.push_back(r);
    }
    return x;
}
static ui sumBeta(int x, int L, int R) {
    if (!LC[x]) return mulm(T[x].q, (ui)(R - L + 1));
    pushdown(x);
    int mid = (L + R) >> 1;
    return addm(sumBeta(LC[x], L, mid), sumBeta(RC[x], mid + 1, R));
}

static ui treeGeneral(int lo, int hi) {
    // period sequence: idx p0+1..n (V=a), 1..p0-1 (V=a-1), then b (V=M-1); root 0 with V=M
    int cnt = n; // elements 1..n-1 plus b at position n
    vector<int> V(cnt + 1);
    V[0] = M;
    int c = 0;
    for (int i = p0 + 1; i <= n; i++) V[++c] = a[i];
    for (int i = 1; i < p0; i++) V[++c] = a[i] - 1;
    V[++c] = M - 1; // b at position cnt
    // pge tree
    vector<int> par(cnt + 1, -1);
    {
        vector<int> st; st.reserve(cnt + 1); st.push_back(0);
        for (int i = 1; i <= cnt; i++) {
            while (V[st.back()] < V[i]) st.pop_back();
            par[i] = st.back(); st.push_back(i);
        }
    }
    vector<char> spine(cnt + 1, 0);
    for (int x = cnt; x != -1; x = par[x]) spine[x] = 1;
    // children CSR (only active: V>=lo), in increasing position
    vector<int> deg(cnt + 2, 0);
    for (int i = 1; i <= cnt; i++) if (V[i] >= lo || spine[i]) deg[par[i]]++;
    vector<int> start(cnt + 2, 0);
    for (int i = 0; i <= cnt; i++) start[i + 1] = start[i] + deg[i];
    vector<int> ch(start[cnt + 1]);
    {
        vector<int> fill(start.begin(), start.end() - 1);
        for (int i = 1; i <= cnt; i++) if (V[i] >= lo || spine[i]) ch[fill[par[i]]++] = i;
    }
    // subtree sizes (active)
    vector<int> sz(cnt + 1, 1);
    for (int i = cnt; i >= 1; i--) if (V[i] >= lo || spine[i]) sz[par[i]] += sz[i];
    vector<int> heavy(cnt + 1, -1);
    for (int x = 0; x <= cnt; x++) {
        int best = -1, bs = -1;
        for (int j = start[x]; j < start[x + 1]; j++) if (sz[ch[j]] > bs) { bs = sz[ch[j]]; best = j; }
        heavy[x] = best;
    }
    vector<int> H(cnt + 1, 0), Lf(cnt + 1, 0), Rt(cnt + 1, 0), res(cnt + 1, 0);
    vector<int> stage(cnt + 1, 0), ptr(cnt + 1, 0);
    // Iterative DFS
    vector<int> stk; stk.reserve(cnt + 1); stk.push_back(0);
    const ui ONE = 1;
    auto finishNode = [&](int x) {
        int N;
        if (H[x] == 0) N = newLeaf(1, 0, 1);
        else {
            N = H[x];
            if (Lf[x]) N = mulTree(Lf[x], N);
            if (Rt[x]) N = mulTree(N, Rt[x]);
        }
        H[x] = Lf[x] = Rt[x] = 0;
        if (x == 0) { res[x] = N; return; }
        int v = V[x];
        if (spine[x]) {
            // F = Lm * N; v = M-1 > hi always
            Tag t1 = leftMul(1, 1, 1);
            if (v - 1 >= hi) applyTag(N, t1);
            else {
                if (v - 1 >= lo) rangeApply(N, lo, hi, lo, v - 1, t1);
                if (v >= lo && v <= hi) rangeApply(N, lo, hi, v, v, leftMul(0, 1, 1));
                if (v + 1 <= hi) rangeAssign(N, lo, hi, max(v + 1, lo), hi, 0, 0, 0);
            }
        } else {
            Tag t1 = leftMul(1, 1, 1); t1.k1 = ONE; t1.k3 = ONE;
            if (v - 1 >= hi) applyTag(N, t1);
            else {
                if (v - 1 >= lo) rangeApply(N, lo, hi, lo, v - 1, t1);
                if (v >= lo && v <= hi) { Tag t2 = leftMul(0, 1, 1); t2.k1 = ONE; t2.k3 = ONE; rangeApply(N, lo, hi, v, v, t2); }
                if (v + 1 <= hi) rangeAssign(N, lo, hi, max(v + 1, lo), hi, 1, 0, 1);
            }
        }
        res[x] = N;
    };
    while (!stk.empty()) {
        int x = stk.back();
        int hj = heavy[x];
        if (stage[x] == 0) {
            if (hj < 0) { finishNode(x); stk.pop_back(); continue; }
            stage[x] = 1; stk.push_back(ch[hj]); continue;
        }
        if (stage[x] == 1) { // heavy returned
            H[x] = res[ch[hj]]; res[ch[hj]] = 0;
            stage[x] = 2; ptr[x] = start[x];
        }
        // collect returned light child
        if (stage[x] == 3) {
            int cj = ptr[x]; int r = res[ch[cj]]; res[ch[cj]] = 0;
            if (cj < hj) Lf[x] = Lf[x] ? mulTree(Lf[x], r) : r;
            else Rt[x] = Rt[x] ? mulTree(Rt[x], r) : r;
            ptr[x]++; stage[x] = 2;
        }
        if (stage[x] == 2) {
            if (ptr[x] == hj) ptr[x]++;
            if (ptr[x] < start[x + 1]) { stage[x] = 3; stk.push_back(ch[ptr[x]]); continue; }
            finishNode(x); stk.pop_back(); continue;
        }
    }
    ui ans = sumBeta(res[0], lo, hi);
    freeTree(res[0]);
    return ans;
}

static bool SLOW = false;

static ui solveCase() {
    M = 0; for (int i = 1; i <= n; i++) M = max(M, a[i]);
    p0 = 0; for (int i = 1; i <= n; i++) if (a[i] == M) p0 = i;
    sufOnesStart = n + 1; while (sufOnesStart > 1 && a[sufOnesStart - 1] == 1) sufOnesStart--;
    lastIdx2 = 0; for (int i = 1; i <= n; i++) if (a[i] >= 2) lastIdx2 = i;
    ui ans = 0;
    // T1
    {
        int T1 = 0;
        for (int l = max(0, M - 2); l <= M - 1; l++) {
            // prefix: for i<idx, lap l+1 values a-l-1 ; suffix: i>=idx, values a-l
            vector<int> preMax(n + 2, INT_MIN), preCnt(n + 2, 0), sufMax(n + 2, INT_MIN), sufCnt(n + 2, 0);
            for (int i = 1; i <= n; i++) { int v = a[i] - l - 1; preMax[i] = max(preMax[i - 1], v); preCnt[i] = preCnt[i - 1] + (v >= 1); }
            for (int i = n; i >= 1; i--) { int v = a[i] - l; sufMax[i] = max(sufMax[i + 1], v); sufCnt[i] = sufCnt[i + 1] + (v >= 1); }
            for (int i = 1; i <= n; i++) {
                if (a[i] - l != 1) continue;
                if (preMax[i - 1] > 1 || sufMax[i] > 1) continue;
                T1 = max(T1, preCnt[i - 1] + sufCnt[i]);
            }
        }
        ans = addm(ans, (ui)T1);
    }
    // Z part
    {
        vector<ui> Z(n + 1, 0), PZ(n + 1, 0);
        vector<int> st; st.reserve(n);
        for (int i = 1; i <= n; i++) {
            if (a[i] >= 2) {
                while (!st.empty() && a[st.back()] < a[i]) st.pop_back();
                int pg = st.empty() ? 0 : st.back();
                ui s = pg ? 0 : 1;
                int lo = max(pg, 1);
                s = addm(s, subm(PZ[i - 1], PZ[lo - 1]));
                Z[i] = s; st.push_back(i);
            }
            PZ[i] = addm(PZ[i - 1], Z[i]);
        }
        int lastPos = 0, ones = 0;
        for (int j = 1; j <= n; j++) {
            if (a[j] >= 2) {
                ui cz = lastPos ? Z[lastPos] : 1;
                ans = addm(ans, mulm(cz, (ui)(1 + ones)));
                lastPos = j; ones = 0;
            } else ones++;
        }
    }
    if (SLOW) {
        for (int k = 1; k <= M - 2; k++) ans = addm(ans, periodCount(k));
    } else {
        for (int k = 1; k <= min(2, M - 2); k++) ans = addm(ans, periodCount(k));
        if (M - 2 >= 3) ans = addm(ans, treeGeneral(3, M - 2));
    }
    return ans;
}

int main(int argc, char** argv) {
    if (argc > 1 && string(argv[1]) == "slow") SLOW = true;
    int c, Tc;
    if (scanf("%d %d", &c, &Tc) != 2) return 0;
    T.reserve(1 << 16); LC.reserve(1 << 16); RC.reserve(1 << 16);
    T.push_back(Tag()); LC.push_back(0); RC.push_back(0);
    while (Tc--) {
        int m;
        if (scanf("%d %d", &n, &m) != 2) return 0;
        a.assign(n + 1, 0);
        for (int i = 1; i <= n; i++) scanf("%d", &a[i]);
        printf("%u\n", solveCase());
    }
    return 0;
}

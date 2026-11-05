#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const int MOD = 998244353;

struct Mat { int a, b, c, d; };

static inline Mat mmul(const Mat &x, const Mat &y) {
    return Mat{
        (int)(((ll)x.a * y.a + (ll)x.b * y.c) % MOD),
        (int)(((ll)x.a * y.b + (ll)x.b * y.d) % MOD),
        (int)(((ll)x.c * y.a + (ll)x.d * y.c) % MOD),
        (int)(((ll)x.c * y.b + (ll)x.d * y.d) % MOD)};
}
static const Mat IDM{1, 0, 0, 1};

static int n;
static vector<int> adjHead, adjNext, adjTo; // CSR adjacency
static vector<int> par_, heavy, szs, ordr;
static vector<char> mem_;
static vector<int> kL;
static vector<int> lightOff, lightPow, lidx;
static vector<Mat> ltree;
static vector<int> chainId, cpos, chainOff, chainPow;
static vector<Mat> ctree;
static vector<vector<int> > chains;
static vector<int> inv2p;
static vector<Mat> nodeM;
static int root_;
static int answer;

static inline int nextpow2(int x) { int s = 1; while (s < x) s <<= 1; return s; }

static inline void lightAgg(int x, int &AL, int &BL) {
    if (lightPow[x] == 0) { AL = 1; BL = 0; return; }
    const Mat &m = ltree[lightOff[x] + 1];
    AL = m.a; BL = m.c;
}

static inline void computeNodeM(int x) {
    int AL, BL;
    lightAgg(x, AL, BL);
    if (mem_[x]) {
        int t = (int)(((ll)2 * BL + AL) % MOD);
        nodeM[x] = Mat{AL, (int)(2LL * AL % MOD), BL, t};
    } else {
        int k = kL[x];
        int h = heavy[x];
        if (h != -1 && !mem_[h]) k++;
        int coef = inv2p[k];
        int v1 = (int)((ll)AL * coef % MOD);
        nodeM[x] = Mat{0, 0, v1, (int)(2LL * v1 % MOD)};
    }
}

static inline void updateNode(int x) {
    computeNodeM(x);
    int c = chainId[x], base = chainOff[c], S = chainPow[c];
    int j = S + cpos[x];
    ctree[base + j] = nodeM[x];
    for (j >>= 1; j >= 1; j >>= 1)
        ctree[base + j] = mmul(ctree[base + 2 * j], ctree[base + 2 * j + 1]);
}

static inline void setLightLeaf(int q, int y, int u, int J) {
    int base = lightOff[q], S = lightPow[q];
    int j = S + lidx[y];
    ltree[base + j] = Mat{u, 0, J, u};
    for (j >>= 1; j >= 1; j >>= 1)
        ltree[base + j] = mmul(ltree[base + 2 * j], ltree[base + 2 * j + 1]);
}

static inline void setLightLeafRaw(int q, int y, int u, int J) {
    int base = lightOff[q], S = lightPow[q];
    ltree[base + S + lidx[y]] = Mat{u, 0, J, u};
}

static void flip(int x) {
    mem_[x] = 1;
    updateNode(x);
    int p = par_[x];
    if (p != -1 && heavy[p] == x) updateNode(p);
    int c = chainId[x];
    while (true) {
        const Mat &pr = ctree[chainOff[c] + 1];
        int P = pr.a, J = pr.c;
        int top = chains[c][0];
        if (top == root_) { answer = J; return; }
        int q = par_[top];
        int u = (int)((2LL * J + P) % MOD);
        setLightLeaf(q, top, u, J);
        if (top == x) kL[q]--;
        updateNode(q);
        c = chainId[q];
    }
}

int main() {
    // ---- fast input ----
    static string sbuf;
    {
        sbuf.reserve(1 << 22);
        char tmp[1 << 16];
        size_t r;
        while ((r = fread(tmp, 1, sizeof(tmp), stdin)) > 0) sbuf.append(tmp, r);
    }
    const char *buf = sbuf.data();
    size_t bufLen = sbuf.size();
    size_t bufPos = 0;
    auto readInt = [&]() -> int {
        while (bufPos < bufLen && (buf[bufPos] < '0' || buf[bufPos] > '9')) bufPos++;
        int v = 0;
        while (bufPos < bufLen && buf[bufPos] >= '0' && buf[bufPos] <= '9') {
            v = v * 10 + (buf[bufPos] - '0');
            bufPos++;
        }
        return v;
    };

    n = readInt();

    adjHead.assign(n, -1);
    adjTo.reserve(2 * max(0, n - 1));
    adjNext.reserve(2 * max(0, n - 1));
    for (int i = 0; i < n - 1; i++) {
        int u = readInt() - 1, v = readInt() - 1;

        adjTo.push_back(v); adjNext.push_back(adjHead[u]); adjHead[u] = (int)adjTo.size() - 1;
        adjTo.push_back(u); adjNext.push_back(adjHead[v]); adjHead[v] = (int)adjTo.size() - 1;
    }
    vector<int> ops(n > 1 ? n - 1 : 0);
    vector<char> isOp(n, 0);
    for (int i = 0; i < n - 1; i++) { ops[i] = readInt() - 1; isOp[ops[i]] = 1; }
    if (n == 1) return 0;
    int r = 0;
    while (isOp[r]) r++;
    root_ = r;

    // ---- root the tree ----
    par_.assign(n, -1);
    ordr.reserve(n);
    vector<int> stk;
    stk.push_back(r);
    vector<char> vis(n, 0);
    vis[r] = 1;
    while (!stk.empty()) {
        int x = stk.back(); stk.pop_back();
        ordr.push_back(x);
        for (int e = adjHead[x]; e != -1; e = adjNext[e]) {
            int y = adjTo[e];
            if (!vis[y]) { vis[y] = 1; par_[y] = x; stk.push_back(y); }
        }
    }

    // ---- sizes and heavy child ----
    szs.assign(n, 1);
    heavy.assign(n, -1);
    for (int i = n - 1; i >= 0; i--) {
        int x = ordr[i];
        int best = -1, bestSz = 0;
        for (int e = adjHead[x]; e != -1; e = adjNext[e]) {
            int y = adjTo[e];
            if (par_[y] == x) {
                szs[x] += szs[y];
                if (szs[y] > bestSz) { bestSz = szs[y]; best = y; }
            }
        }
        heavy[x] = best;
    }

    // ---- HLD chains ----
    chainId.assign(n, -1);
    cpos.assign(n, 0);
    chains.clear();
    for (int i = 0; i < n; i++) {
        int x = ordr[i];
        if (par_[x] == -1 || heavy[par_[x]] != x) {
            // x is chain head
            int cid = (int)chains.size();
            chains.push_back(vector<int>());
            int y = x;
            while (y != -1) {
                chainId[y] = cid;
                cpos[y] = (int)chains.back().size();
                chains.back().push_back(y);
                y = heavy[y];
            }
        }
    }

    // ---- light children blocks ----
    // count light children
    lightOff.assign(n, 0);
    lightPow.assign(n, 0);
    lidx.assign(n, 0);
    {
        vector<int> cnt(n, 0);
        for (int x = 0; x < n; x++) {
            int p = par_[x];
            if (p != -1 && heavy[p] != x) cnt[p]++;
        }
        // assign indices in a stable order: iterate over children in reverse-order list
        vector<int> fill(n, 0);
        for (int i = n - 1; i >= 0; i--) {
            int x = ordr[i];
            int p = par_[x];
            if (p != -1 && heavy[p] != x) lidx[x] = fill[p]++;
        }
        long long total = 0;
        for (int x = 0; x < n; x++) {
            if (cnt[x] > 0) {
                lightPow[x] = nextpow2(cnt[x]);
                total += 2LL * lightPow[x];
            }
        }
        ltree.assign(total, IDM);
        long long off = 0;
        for (int x = 0; x < n; x++) {
            if (lightPow[x] > 0) { lightOff[x] = (int)off; off += 2LL * lightPow[x]; }
        }
    }

    // ---- chain segment trees ----
    {
        int nc = (int)chains.size();
        chainOff.assign(nc, 0);
        chainPow.assign(nc, 0);
        long long total = 0;
        for (int c = 0; c < nc; c++) {
            chainPow[c] = nextpow2((int)chains[c].size());
            total += 2LL * chainPow[c];
        }
        ctree.assign(total, IDM);
        long long off = 0;
        for (int c = 0; c < nc; c++) { chainOff[c] = (int)off; off += 2LL * chainPow[c]; }
    }

    // ---- inverse powers of 2 ----
    inv2p.assign(n + 2, 1);
    {
        int inv2 = (MOD + 1) / 2;
        for (int i = 1; i <= n + 1; i++) inv2p[i] = (int)((ll)inv2p[i - 1] * inv2 % MOD);
    }

    // ---- initial bottom-up DP ----
    mem_.assign(n, 0);
    kL.assign(n, 0);
    nodeM.assign(n, IDM);
    {
        vector<int> Pv(n, 0), Jv(n, 0), Uv(n, 0);
        // kL init: count light children (none in S yet)
        for (int x = 0; x < n; x++) {
            int p = par_[x];
            if (p != -1 && heavy[p] != x) kL[p]++;
        }
        for (int i = n - 1; i >= 0; i--) {
            int x = ordr[i];
            if (lightPow[x] > 0) {
                int base = lightOff[x], S = lightPow[x];
                for (int j = S - 1; j >= 1; j--)
                    ltree[base + j] = mmul(ltree[base + 2 * j], ltree[base + 2 * j + 1]);
            }
            computeNodeM(x);
            int h = heavy[x];
            int Px, Jx;
            if (h == -1) { Px = nodeM[x].a; Jx = nodeM[x].c; }
            else {
                Px = (int)(((ll)nodeM[x].a * Pv[h] + (ll)nodeM[x].b * Jv[h]) % MOD);
                Jx = (int)(((ll)nodeM[x].c * Pv[h] + (ll)nodeM[x].d * Jv[h]) % MOD);
            }
            Pv[x] = Px; Jv[x] = Jx;
            Uv[x] = (int)((2LL * Jx + Px) % MOD);
            int p = par_[x];
            if (p != -1 && heavy[p] != x) setLightLeafRaw(p, x, Uv[x], Jv[x]);
        }
        answer = Jv[r];
    }

    // ---- build chain segment trees ----
    {
        int nc = (int)chains.size();
        for (int c = 0; c < nc; c++) {
            int base = chainOff[c], S = chainPow[c];
            int L = (int)chains[c].size();
            for (int i = 0; i < L; i++) ctree[base + S + i] = nodeM[chains[c][i]];
            for (int j = S + L; j < 2 * S; j++) ctree[base + j] = IDM;
            for (int j = S - 1; j >= 1; j--)
                ctree[base + j] = mmul(ctree[base + 2 * j], ctree[base + 2 * j + 1]);
        }
    }

    // ---- process operations ----
    string out;
    out.reserve((size_t)(n - 1) * 11);
    char tmp[16];
    for (int i = 0; i < n - 1; i++) {
        flip(ops[i]);
        int v = answer;
        int len = 0;
        if (v == 0) tmp[len++] = '0';
        while (v > 0) { tmp[len++] = char('0' + v % 10); v /= 10; }
        while (len > 0) out.push_back(tmp[--len]);
        out.push_back('\n');
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

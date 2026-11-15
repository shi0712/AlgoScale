#include <bits/stdc++.h>
using namespace std;

static int N, M, K, W;
static vector<uint64_t> adjb;            // M x W bitset adjacency of remaining graph
static vector<int> mt;
static vector<vector<int>> colorMatrix;  // [M][M]
static int colorsDone = 0;
static vector<string> A;                 // input adjacency

static inline bool hasEdge(int u, int v) { return (adjb[(size_t)u * W + (v >> 6)] >> (v & 63)) & 1; }
static inline void clearBit(int u, int v) { adjb[(size_t)u * W + (v >> 6)] &= ~(1ULL << (v & 63)); }

static uint64_t rngState = 0x243F6A8885A308D3ULL;
static inline uint64_t rnd() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 7;
    rngState ^= rngState << 17;
    return rngState;
}

// working graph description: for every pair (i,j) in [0,M) we know if edge exists in G'
static inline bool workingEdge(int i, int j) {
    if (N % 2 == 0) return (i != j) && A[i][j] == '1';
    // odd: M=N+1, dummy w=N inserted into H-edge (hu,hv)
    static int hu = -2, hv = -2;
    if (hu == -2) {
        hu = -1; hv = -1;
        for (int a = 0; a < N && hu < 0; a++)
            for (int b = a + 1; b < N; b++)
                if (A[a][b] == '0') { hu = a; hv = b; break; }
    }
    int w = N;
    if (i == j) return false;
    if (i == w) return j != hu && j != hv;
    if (j == w) return i != hu && i != hv;
    if ((i == hu && j == hv) || (i == hv && j == hu)) return true;   // (hu,hv) becomes a G'-edge
    return A[i][j] == '1';
}

static void buildWorking() {
    adjb.assign((size_t)M * W, 0);
    for (int i = 0; i < M; i++)
        for (int j = 0; j < M; j++)
            if (workingEdge(i, j)) adjb[(size_t)i * W + (j >> 6)] |= 1ULL << (j & 63);
}

// ---------------- blossom augmenting path (single root) ----------------
static vector<int> bmatch, bp, bbase, bq;
static vector<char> bused, bblossom, lcaMark;

static void markPath(int v, int b, int children) {
    while (bbase[v] != b) {
        bblossom[bbase[v]] = bblossom[bbase[bmatch[v]]] = 1;
        bp[v] = children;
        children = bmatch[v];
        v = bp[bmatch[v]];
    }
}
static int lcaFn(int a, int b) {
    fill(lcaMark.begin(), lcaMark.end(), 0);
    while (true) {
        a = bbase[a]; lcaMark[a] = 1;
        if (bmatch[a] == -1) break;
        a = bp[bmatch[a]];
    }
    while (true) {
        b = bbase[b];
        if (lcaMark[b]) return b;
        b = bp[bmatch[b]];
    }
}
static bool findPath(int root) {
    fill(bused.begin(), bused.end(), 0);
    fill(bp.begin(), bp.end(), -1);
    for (int i = 0; i < M; i++) bbase[i] = i;
    bused[root] = 1;
    int qh = 0, qt = 0; bq[qt++] = root;
    while (qh < qt) {
        int v = bq[qh++];
        const uint64_t* row = &adjb[(size_t)v * W];
        for (int w = 0; w < W; w++) {
            uint64_t x = row[w];
            while (x) {
                int to = (w << 6) + __builtin_ctzll(x);
                x &= x - 1;
                if (to >= M) break;
                if (bbase[v] == bbase[to] || bmatch[v] == to) continue;
                if (to == root || (bmatch[to] != -1 && bp[bmatch[to]] != -1)) {
                    int curbase = lcaFn(v, to);
                    fill(bblossom.begin(), bblossom.end(), 0);
                    markPath(v, curbase, to);
                    markPath(to, curbase, v);
                    for (int i = 0; i < M; i++)
                        if (bblossom[bbase[i]]) {
                            bbase[i] = curbase;
                            if (!bused[i]) { bused[i] = 1; bq[qt++] = i; }
                        }
                } else if (bp[to] == -1) {
                    bp[to] = v;
                    if (bmatch[to] == -1) {
                        int u = to;
                        while (u != -1) {
                            int pv = bp[u], nv = (pv == -1 ? -1 : bmatch[pv]);
                            bmatch[u] = pv;
                            if (pv != -1) bmatch[pv] = u;
                            u = nv;
                        }
                        return true;
                    } else {
                        bused[bmatch[to]] = 1;
                        bq[qt++] = bmatch[to];
                    }
                }
            }
        }
    }
    return false;
}

static bool findPerfectMatching() {
    mt.assign(M, -1);
    static vector<int> ord;
    ord.resize(M);
    for (int i = 0; i < M; i++) ord[i] = i;
    for (int i = M - 1; i > 0; i--) { int j = (int)(rnd() % (uint64_t)(i + 1)); swap(ord[i], ord[j]); }
    static vector<uint64_t> avail;
    avail.assign(W, ~0ULL);
    if (M % 64) avail[W - 1] = (1ULL << (M % 64)) - 1;
    for (int oi = 0; oi < M; oi++) {
        int u = ord[oi];
        if (mt[u] != -1) continue;
        int found = -1;
        const uint64_t* row = &adjb[(size_t)u * W];
        for (int w = 0; w < W; w++) {
            uint64_t x = row[w] & avail[w];
            if (x) { found = (w << 6) + __builtin_ctzll(x); break; }
        }
        if (found >= 0) {
            mt[u] = found; mt[found] = u;
            avail[u >> 6] &= ~(1ULL << (u & 63));
            avail[found >> 6] &= ~(1ULL << (found & 63));
        }
    }
    // cheap length-3 augmentations
    bool changed = true;
    while (changed) {
        changed = false;
        // recompute avail
        avail.assign(W, ~0ULL);
        if (M % 64) avail[W - 1] = (1ULL << (M % 64)) - 1;
        for (int i = 0; i < M; i++) if (mt[i] != -1) avail[i >> 6] &= ~(1ULL << (i & 63));
        for (int u = 0; u < M && !changed; u++) {
            if (mt[u] != -1) continue;
            const uint64_t* ru = &adjb[(size_t)u * W];
            for (int w = 0; w < W && !changed; w++) {
                uint64_t x = ru[w];
                while (x) {
                    int a = (w << 6) + __builtin_ctzll(x);
                    x &= x - 1;
                    if (a >= M) break;
                    int b = mt[a];
                    if (b < 0) continue;
                    const uint64_t* rb = &adjb[(size_t)b * W];
                    int c = -1;
                    for (int w2 = 0; w2 < W; w2++) {
                        uint64_t y = rb[w2] & avail[w2];
                        if (y) { c = (w2 << 6) + __builtin_ctzll(y); break; }
                    }
                    if (c >= 0 && c != u && c != a && mt[c] == -1) {
                        mt[u] = a; mt[a] = u; mt[b] = c; mt[c] = b;
                        avail[c >> 6] &= ~(1ULL << (c & 63));
                        changed = true; break;
                    }
                }
            }
        }
    }
    bool any = false;
    for (int i = 0; i < M; i++) if (mt[i] == -1) { any = true; break; }
    if (!any) return true;
    bmatch = mt;
    for (int i = 0; i < M; i++) if (bmatch[i] == -1) findPath(i);
    for (int i = 0; i < M; i++) if (bmatch[i] == -1) return false;
    mt = bmatch;
    return true;
}

static bool validMatching() {
    for (int u = 0; u < M; u++) {
        if (mt[u] < 0 || mt[u] >= M) return false;
        if (mt[mt[u]] != u) return false;
        if (!hasEdge(u, mt[u])) return false;
    }
    return true;
}

static bool tailFactor(int deg, int triesLeft) {
    if (deg == 0) return true;
    if (deg == 1) {
        if (!findPerfectMatching() || !validMatching()) return false;
        for (int u = 0; u < M; u++) { int v = mt[u]; colorMatrix[u][v] = colorMatrix[v][u] = colorsDone; }
        colorsDone++;
        return true;
    }
    for (int t = 0; t < triesLeft; t++) {
        if (!findPerfectMatching() || !validMatching()) return false;
        vector<uint64_t> saved = adjb;
        int col = colorsDone;
        for (int u = 0; u < M; u++) {
            int v = mt[u];
            if (v > u) { clearBit(u, v); clearBit(v, u); }
            colorMatrix[u][v] = colorMatrix[v][u] = col;
        }
        colorsDone++;
        if (tailFactor(deg - 1, triesLeft)) return true;
        colorsDone = col;
        adjb = saved;
    }
    return false;
}

static chrono::steady_clock::time_point gStart;
static bool solveEven() {
    for (int attempt = 0; attempt < 6; attempt++) {
        if (attempt > 0 && chrono::duration<double>(chrono::steady_clock::now() - gStart).count() > 2.1) break;
        buildWorking();
        colorsDone = 0;
        colorMatrix.assign(M, vector<int>(M, -1));
        int deg = M - 3;
        bool ok = true;
        while (deg > 4) {
            if (!findPerfectMatching() || !validMatching()) { ok = false; break; }
            for (int u = 0; u < M; u++) {
                int v = mt[u];
                if (v > u) { clearBit(u, v); clearBit(v, u); }
                colorMatrix[u][v] = colorMatrix[v][u] = colorsDone;
            }
            colorsDone++;
            deg--;
        }
        if (ok && deg > 0) ok = tailFactor(deg, 4);
        if (ok) {
            vector<vector<char>> used(M, vector<char>(K, 0));
            bool good = true;
            for (int u = 0; u < M && good; u++)
                for (int v = u + 1; v < M && good; v++)
                    if (workingEdge(u, v)) {
                        int c = colorMatrix[u][v];
                        if (c < 0 || c >= K) { good = false; break; }
                        if (used[u][c] || used[v][c]) { good = false; break; }
                        used[u][c] = used[v][c] = 1;
                    }
            if (good) return true;
        }
    }
    return false;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> N)) return 0;
    A.resize(N);
    for (int i = 0; i < N; i++) cin >> A[i];

    gStart = chrono::steady_clock::now();
    rngState = 0x9E3779B97F4A7C15ULL ^ ((uint64_t)N * 0x2545F4914F6CDD1DULL);
    for (int i = 0; i < 20; i++) rnd();

    int C = (N % 2 == 0) ? N - 3 : N - 2;
    if (N % 2 == 0) { M = N; K = N - 3; }
    else { M = N + 1; K = N - 2; }
    W = (M + 63) / 64;

    bbase.resize(M); bp.resize(M); bq.resize(2 * M + 8); bused.resize(M); bblossom.resize(M); lcaMark.resize(M);

    solveEven();

    string out;
    out.reserve((size_t)N * N * 3 + 16);
    out += to_string(C); out += '\n';
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++) {
            int val = -1;
            if (i != j && A[i][j] == '1') val = colorMatrix[i][j] + 1;
            out += to_string(val);
            out += (j + 1 == N) ? '\n' : ' ';
        }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

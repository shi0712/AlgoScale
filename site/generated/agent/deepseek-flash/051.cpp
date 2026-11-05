// 密图调色板 — edge colouring of K_n minus a 2-factor
//
// The complement F = K_n - G of the given graph is 2-regular, i.e. a disjoint
// union of cycles (length >= 3).  Hence deg_G = n-3.
//   * n even: chi' = n-3 and the edges split into n-3 perfect matchings.
//   * n odd : chi' = n-2 (counting bound + Vizing).
// For odd n we delete an edge {a,b} of F, insert a dummy vertex d joined to a
// and b, obtaining a graph G' on n+1 (even) vertices that is again K_{n+1}
// minus a 2-factor.  A 1-factorization of G' with n-2 perfect matchings turns
// into a colouring of G with n-2 colours once the dummy edges (one per class)
// and the edge {a,b} are deleted.
//
// The 1-factorization itself is built by repeatedly extracting a perfect
// matching (randomized greedy + short augmenting paths + Edmonds' blossom as a
// fallback).  Extraction failures only happen in the last few rounds, so the
// state right before those rounds is snapshotted and only the tail is retried.

#include <bits/stdc++.h>
using namespace std;

static int N;                 // number of vertices of the working graph (even)
static int WORIG;             // original n
static bool ODD;              // original n odd?
static int DUMMY;             // dummy vertex index (= WORIG) when ODD
static int PAIR_A = -1, PAIR_B = -1; // F-edge removed when ODD

static vector<vector<int>> nb;         // alive neighbours
static vector<uint16_t> posIn;         // position of v inside nb[u]
static vector<uint64_t> aliveBits;     // alive adjacency bit matrix
static int BW;                         // words per row of aliveBits

static vector<int16_t> colMat;         // answer matrix, flat

static mt19937_64 rng;

static inline bool aliveQ(int u, int v) {
    return (aliveBits[(size_t)u * BW + (v >> 6)] >> (v & 63)) & 1ULL;
}
static inline void clearBit(int u, int v) {
    aliveBits[(size_t)u * BW + (v >> 6)] &= ~(1ULL << (v & 63));
}
static inline void delEdge(int u, int v) {
    clearBit(u, v); clearBit(v, u);
    int pu = posIn[(size_t)u * N + v], pv = posIn[(size_t)v * N + u];
    int lu = (int)nb[u].size() - 1, lv = (int)nb[v].size() - 1;
    nb[u][pu] = nb[u][lu]; posIn[(size_t)u * N + nb[u][lu]] = (uint16_t)pu; nb[u].pop_back();
    nb[v][pv] = nb[v][lv]; posIn[(size_t)v * N + nb[v][lv]] = (uint16_t)pv; nb[v].pop_back();
}

// ------------------------------------------------------------------ blossom
struct Blossom {
    int n;
    vector<int> match, p, base;
    vector<char> used, bl;
    vector<char> *u2;
    Blossom(int n_) : n(n_) {
        match.assign(n, -1); p.assign(n, -1); base.resize(n);
        used.resize(n); bl.resize(n);
    }
    int lca(int a, int b) {
        static vector<char> u; u.assign(n, 0);
        for (;;) { a = base[a]; u[a] = 1; if (match[a] == -1) break; a = p[match[a]]; }
        for (;;) { b = base[b]; if (u[b]) return b; b = p[match[b]]; }
    }
    void markPath(int v, int b, int child) {
        while (base[v] != b) {
            bl[base[v]] = bl[base[match[v]]] = 1;
            p[v] = child; child = match[v]; v = p[match[v]];
        }
    }
    int findPath(int root) {
        fill(used.begin(), used.end(), 0);
        fill(p.begin(), p.end(), -1);
        for (int i = 0; i < n; i++) base[i] = i;
        used[root] = 1;
        queue<int> q; q.push(root);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int to : nb[v]) {
                if (match[v] == to || base[v] == base[to]) continue;
                if (to == root || (match[to] != -1 && p[match[to]] != -1)) {
                    int cb = lca(v, to);
                    fill(bl.begin(), bl.end(), 0);
                    markPath(v, cb, to); markPath(to, cb, v);
                    for (int i = 0; i < n; i++)
                        if (bl[base[i]]) { base[i] = cb; if (!used[i]) { used[i] = 1; q.push(i); } }
                } else if (p[to] == -1) {
                    p[to] = v;
                    if (match[to] == -1) {
                        int cur = to;
                        while (cur != -1) {
                            int pv = p[cur];
                            int nx = (pv == -1) ? -1 : match[pv];
                            if (pv != -1) match[pv] = cur;
                            match[cur] = pv;
                            cur = nx;
                        }
                        return 1;
                    }
                    used[match[to]] = 1;
                    q.push(match[to]);
                }
            }
        }
        return 0;
    }
};

// try to complete `match` (restricted to verts) with short augmenting paths
static int repair(vector<int>& match, const vector<int>& verts, vector<int>& U) {
    U.clear();
    for (int v : verts) if (match[v] < 0) U.push_back(v);
    bool prog = true;
    while (prog && !U.empty()) {
        prog = false;
        for (size_t i = 0; i < U.size(); i++) {
            if (match[U[i]] >= 0) continue;
            for (size_t j = i + 1; j < U.size(); j++) {
                if (match[U[j]] >= 0) continue;
                if (aliveQ(U[i], U[j])) { match[U[i]] = U[j]; match[U[j]] = U[i]; prog = true; break; }
            }
        }
        if (prog) { U.clear(); for (int v : verts) if (match[v] < 0) U.push_back(v); continue; }
        for (int u : U) {
            if (match[u] >= 0) continue;
            bool done = false;
            for (int a : nb[u]) {
                int b = match[a];
                if (b < 0) continue;
                for (int w : U) {
                    if (w == u || match[w] >= 0 || w == b) continue;
                    if (aliveQ(b, w)) {
                        match[u] = a; match[a] = u; match[b] = w; match[w] = b;
                        prog = true; done = true; break;
                    }
                }
                if (done) break;
            }
        }
        if (prog) { U.clear(); for (int v : verts) if (match[v] < 0) U.push_back(v); }
    }
    int c = 0;
    for (int v : verts) if (match[v] < 0) c++;
    return c;
}

// build the working graph (K_N minus 2-factor F')
static void buildGraph(const vector<vector<char>>& isF) {
    nb.assign(N, {});
    posIn.assign((size_t)N * N, 0);
    BW = (N + 63) / 64;
    aliveBits.assign((size_t)N * BW, 0);
    for (int i = 0; i < N; i++) {
        vector<int>& L = nb[i];
        L.reserve(N);
        for (int j = 0; j < N; j++)
            if (i != j && !isF[i][j]) {
                posIn[(size_t)i * N + j] = (uint16_t)L.size();
                L.push_back(j);
            }
        for (int j : L) aliveBits[(size_t)i * BW + (j >> 6)] |= 1ULL << (j & 63);
    }
}

// restore a snapshot of the graph
static void restoreGraph(const vector<int>& snapOff, const vector<int>& snapVals) {
    for (int i = 0; i < N; i++) {
        nb[i].clear();
        for (int k = snapOff[i]; k < snapOff[i + 1]; k++) nb[i].push_back(snapVals[k]);
        for (size_t k = 0; k < nb[i].size(); k++)
            posIn[(size_t)i * N + nb[i][k]] = (uint16_t)k;
    }
    fill(aliveBits.begin(), aliveBits.end(), 0ULL);
    for (int i = 0; i < N; i++)
        for (int j : nb[i]) aliveBits[(size_t)i * BW + (j >> 6)] |= 1ULL << (j & 63);
}
// compact snapshot: flat copy of the neighbour lists
static void takeSnapshot(vector<int>& snapOff, vector<int>& snapVals) {
    snapOff.assign(N + 1, 0);
    for (int i = 0; i < N; i++) snapOff[i + 1] = snapOff[i] + (int)nb[i].size();
    snapVals.clear();
    snapVals.reserve(snapOff[N]);
    for (int i = 0; i < N; i++) snapVals.insert(snapVals.end(), nb[i].begin(), nb[i].end());
}

// one extraction round; returns false if no perfect matching could be found
// colour = the colour index to write (0 = do not write)
static bool oneRound(vector<int>& match, vector<int>& order, vector<int>& U, int colour,
                     vector<pair<int,int>>* written) {
    int m = (int)order.size();
    for (int i = 0; i < m; i++) match[order[i]] = -1;
    shuffle(order.begin(), order.end(), rng);
    for (int u : order) {
        if (match[u] != -1) continue;
        vector<int>& L = nb[u];
        int d = (int)L.size();
        if (!d) continue;
        int st = (int)(rng() % (uint64_t)d);
        for (int t = 0; t < d; t++) {
            int idx = st + t; if (idx >= d) idx -= d;
            int v = L[idx];
            if (match[v] == -1) { match[u] = v; match[v] = u; break; }
        }
    }
    int left = repair(match, order, U);
    if (left > 0) {
        Blossom B(N);
        B.match = match;
        for (int v : order) if (B.match[v] == -1) B.findPath(v);
        match = B.match;
        int l2 = 0;
        for (int v : order) if (match[v] < 0) l2++;
        if (l2) return false;
    }
    for (int v : order) {
        int w = match[v];
        if (w >= 0 && v < w) {
            if (colour > 0 && v < WORIG && w < WORIG &&
                !(ODD && ((v == PAIR_A && w == PAIR_B) || (v == PAIR_B && w == PAIR_A)))) {
                colMat[(size_t)v * WORIG + w] = (int16_t)colour;
                colMat[(size_t)w * WORIG + v] = (int16_t)colour;
                if (written) written->push_back({v, w});
            }
            delEdge(v, w);
        }
    }
    return true;
}

int main() {
    // ---------------------------------------------------------------- input
    vector<char> buf;
    {
        FILE* f = stdin;
        fseek(f, 0, SEEK_END);
        long sz = ftell(f);
        fseek(f, 0, SEEK_SET);
        buf.resize(sz + 1);
        size_t rd = fread(buf.data(), 1, sz, f);
        buf[rd] = 0;
    }
    int pos = 0;
    int n = 0;
    while (pos < (int)buf.size() && (buf[pos] < '0' || buf[pos] > '9')) pos++;
    while (pos < (int)buf.size() && buf[pos] >= '0' && buf[pos] <= '9') n = n * 10 + (buf[pos++] - '0');
    WORIG = n;
    vector<vector<char>> isF(n, vector<char>(n, 0));
    {
        // read n rows of n characters
        int i = 0;
        while (i < n) {
            while (pos < (int)buf.size() && (buf[pos] != '0' && buf[pos] != '1')) pos++;
            if (pos + n > (int)buf.size()) break;
            const char* row = buf.data() + pos;
            for (int j = 0; j < n; j++) isF[i][j] = (i != j && row[j] == '0');
            pos += n;
            i++;
        }
    }
    // free input buffer memory
    vector<char>().swap(buf);

    int C = (n % 2 == 0) ? n - 3 : n - 2;
    colMat.assign((size_t)n * n, (int16_t)-1);

    // Build working graph ---------------------------------------------------
    // For odd n: split an F-edge {a,b} through a dummy vertex d.
    ODD = (n % 2 == 1);
    N = ODD ? n + 1 : n;
    DUMMY = n;
    static vector<vector<char>> isF2;
    const vector<vector<char>>* pF = &isF;
    if (ODD) {
        isF2 = isF;
        isF2.resize(N);
        for (int i = 0; i < N; i++) isF2[i].resize(N, 0);
        // find an F-edge
        int a = -1, b = -1;
        for (int i = 0; i < n && a < 0; i++)
            for (int j = 0; j < n; j++)
                if (isF[i][j]) { a = i; b = j; break; }
        PAIR_A = a; PAIR_B = b;
        isF2[a][b] = isF2[b][a] = 0;   // {a,b} becomes a G-edge
        for (int i = 0; i < n; i++) isF2[DUMMY][i] = isF2[i][DUMMY] = 0;
        isF2[DUMMY][a] = isF2[a][DUMMY] = 1;   // dummy keeps the 2-factor
        isF2[DUMMY][b] = isF2[b][DUMMY] = 1;
        isF2[DUMMY][DUMMY] = 0;
        vector<vector<char>>().swap(isF);
        pF = &isF2;
    }

    rng.seed(0x9E3779B97F4A7C15ULL ^ (unsigned long long)n);

    // -------------------------------------------------------------- solving
    const int TAIL = 48;
    int totalRounds = N - 3;             // number of matchings to extract
    vector<int> match(N, -1), U;
    vector<int> order(N);

    bool done = false;
    int outerAttempts = 0, innerAttempts = 0, midFail = 0, tailFail = 0;
    for (int outer = 0; outer < 60 && !done; outer++) {
        outerAttempts++;
        buildGraph(*pF);
        bool fail = false;
        int R1 = totalRounds - TAIL;
        if (R1 < 0) R1 = 0;
        for (int r = 0; r < R1 && !fail; r++) {
            for (int i = 0; i < N; i++) order[i] = i;
            if (!oneRound(match, order, U, r + 1, nullptr)) fail = true;
        }
        if (fail) { midFail++; continue; }
        vector<int> snapOff, snapVals;
        takeSnapshot(snapOff, snapVals);
        for (int inner = 0; inner < 40 && !done && !fail; inner++) {
            innerAttempts++;
            vector<pair<int,int>> written;
            bool f2 = false;
            int r = R1;
            for (; r < totalRounds && !f2; r++) {
                for (int i = 0; i < N; i++) order[i] = i;
                if (!oneRound(match, order, U, r + 1, &written)) f2 = true;
            }
            if (f2) tailFail++;
            if (!f2) {
                bool clean = true;
                for (int i = 0; i < N && clean; i++) if (!nb[i].empty()) clean = false;
                if (clean) done = true;
            }
            if (!done) {
                // undo colours written in the failed tail and restore snapshot
                for (auto& pr : written) {
                    colMat[(size_t)pr.first * WORIG + pr.second] = -1;
                    colMat[(size_t)pr.second * WORIG + pr.first] = -1;
                }
                restoreGraph(snapOff, snapVals);
            }
        }
    }

    if (getenv("DBG"))
        fprintf(stderr, "n=%d outer=%d inner=%d midFail=%d tailFail=%d done=%d\n",
                n, outerAttempts, innerAttempts, midFail, tailFail, (int)done);

    // --------------------------------------------------------------- output
    {
        string out;
        out.reserve(1 << 18);
        char tmp[16];
        auto flushOut = [&]() { fwrite(out.data(), 1, out.size(), stdout); out.clear(); };
        auto putInt = [&](int v) {
            if (v < 0) { out.push_back('-'); out.push_back('1'); out.push_back(' '); return; }
            int len = 0;
            if (v == 0) tmp[len++] = '0';
            while (v > 0) { tmp[len++] = char('0' + v % 10); v /= 10; }
            while (len > 0) out.push_back(tmp[--len]);
            out.push_back(' ');
        };
        putInt(C); out.back() = '\n';
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int v = (i == j) ? -1 : (int)colMat[(size_t)i * n + j];
                putInt(v);
            }
            out.back() = '\n';
            if (out.size() > (1 << 17)) flushOut();
        }
        flushOut();
    }
    return 0;
}

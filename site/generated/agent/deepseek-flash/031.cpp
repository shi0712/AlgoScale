#include <bits/stdc++.h>
using namespace std;

static constexpr uint32_t MOD = 998244353;

inline uint32_t mulmod(uint32_t a, uint32_t b) {
    return (uint32_t)((uint64_t)a * b % MOD);
}
inline uint32_t addmod(uint32_t a, uint32_t b) {
    uint32_t s = a + b;
    if (s >= MOD) s -= MOD;
    return s;
}

uint32_t modpow(uint32_t a, uint64_t e) {
    uint32_t r = 1;
    while (e) {
        if (e & 1) r = mulmod(r, a);
        a = mulmod(a, a);
        e >>= 1;
    }
    return r;
}
inline uint32_t modinv(uint32_t a) { return modpow(a, MOD - 2); }

struct Mat {
    uint32_t a00, a01, a10, a11;
};
inline Mat operator*(const Mat& A, const Mat& B) {
    Mat C;
    C.a00 = (uint32_t)(((uint64_t)A.a00 * B.a00 + (uint64_t)A.a01 * B.a10) % MOD);
    C.a01 = (uint32_t)(((uint64_t)A.a00 * B.a01 + (uint64_t)A.a01 * B.a11) % MOD);
    C.a10 = (uint32_t)(((uint64_t)A.a10 * B.a00 + (uint64_t)A.a11 * B.a10) % MOD);
    C.a11 = (uint32_t)(((uint64_t)A.a10 * B.a01 + (uint64_t)A.a11 * B.a11) % MOD);
    return C;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<vector<int>> adj(n);
    for (int i = 0; i < n - 1; i++) {
        int u, v;
        cin >> u >> v;
        --u; --v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    if (n == 1) return 0;
    vector<int> ord(n - 1);
    for (int i = 0; i < n - 1; i++) { cin >> ord[i]; --ord[i]; }

    vector<int> deg(n);
    for (int i = 0; i < n; i++) deg[i] = (int)adj[i].size();

    // root at 0, iterative DFS order
    vector<int> parent(n, -1), bfs;
    bfs.reserve(n);
    {
        vector<char> vis(n, 0);
        vector<int> st;
        st.reserve(n);
        st.push_back(0); vis[0] = 1;
        while (!st.empty()) {
            int u = st.back(); st.pop_back();
            bfs.push_back(u);
            for (int v : adj[u]) if (!vis[v]) { vis[v] = 1; parent[v] = u; st.push_back(v); }
        }
    }
    vector<int> sz(n, 1), heavy(n, -1);
    for (int i = n - 1; i >= 0; i--) {
        int u = bfs[i];
        int best = 0;
        for (int v : adj[u]) {
            if (v == parent[u]) continue;
            sz[u] += sz[v];
            if (sz[v] > best) { best = sz[v]; heavy[u] = v; }
        }
    }

    // HLD: assign positions chain by chain
    vector<int> head(n), pos(n);
    vector<int> chainHeads;
    chainHeads.reserve(n);
    {
        vector<int> stNode, stHead;
        stNode.push_back(0); stHead.push_back(0);
        int curPos = 0;
        while (!stNode.empty()) {
            int u = stNode.back(); stNode.pop_back();
            int h = stHead.back(); stHead.pop_back();
            chainHeads.push_back(h);
            for (int v = u; v != -1; v = heavy[v]) {
                head[v] = h;
                pos[v] = curPos++;
                for (int w : adj[v]) {
                    if (w != parent[v] && w != heavy[v]) {
                        stNode.push_back(w);
                        stHead.push_back(w);
                    }
                }
            }
        }
    }
    vector<int> clen(n, 0);
    for (int u = 0; u < n; u++) clen[head[u]]++;
    vector<int> offOfHead(n, -1), pszOfHead(n, -1);
    int poolSize = 0;
    for (int h : chainHeads) {
        int psz = 1;
        while (psz < clen[h]) psz <<= 1;
        offOfHead[h] = poolSize;
        pszOfHead[h] = psz;
        poolSize += 2 * psz;
    }
    vector<Mat> pool(poolSize);
    Mat initMat{1, 0, 0, 0};
    for (int h : chainHeads) {
        int off = offOfHead[h], psz = pszOfHead[h];
        for (int i = 0; i < psz; i++) pool[off + psz + i] = initMat;
        for (int i = psz - 1; i >= 1; i--) pool[off + i] = pool[off + 2*i] * pool[off + 2*i + 1];
    }

    vector<uint32_t> curF(n, 1), curE(n, 0);
    vector<int> zc(n, 0);
    vector<uint32_t> pn(n, 1), Ssum(n, 0), sumE(n, 0);
    vector<char> active(n, 0);

    auto getP = [&](int u) -> uint32_t { return zc[u] > 0 ? 0u : pn[u]; };
    auto getQ = [&](int u) -> uint32_t {
        if (zc[u] == 0) return mulmod(pn[u], Ssum[u]);
        if (zc[u] == 1) return mulmod((MOD - sumE[u]) % MOD, pn[u]);
        return 0u;
    };
    auto setLeaf = [&](int u, const Mat& m) {
        int h = head[u];
        int off = offOfHead[h], psz = pszOfHead[h];
        int i = psz + (pos[u] - pos[h]);
        pool[off + i] = m;
        for (i >>= 1; i >= 1; i >>= 1) pool[off + i] = pool[off + 2*i] * pool[off + 2*i + 1];
    };
    auto recomputeMat = [&](int u) {
        uint32_t P = getP(u), Q = getQ(u);
        Mat m;
        if (active[u]) {
            m.a00 = addmod(mulmod((uint32_t)deg[u], P), Q);
            m.a01 = (P == 0 ? 0u : MOD - P);
            m.a10 = P;
            m.a11 = 0;
        } else {
            m.a00 = P; m.a01 = 0; m.a10 = 0; m.a11 = 0;
        }
        setLeaf(u, m);
    };

    vector<uint32_t> pow2(n + 2, 1);
    for (int i = 1; i <= n + 1; i++) pow2[i] = addmod(pow2[i-1], pow2[i-1]);

    long long K = 1;
    string out;
    out.reserve((size_t)(n - 1) * 11);

    for (int step = 0; step < n - 1; step++) {
        int x = ord[step];
        int d = 0;
        for (int v : adj[x]) if (!active[v]) d++;
        K += (long long)d - 1;
        active[x] = 1;
        recomputeMat(x);

        int u = x;
        while (true) {
            int h = head[u];
            Mat pr = pool[offOfHead[h] + 1];
            uint32_t nf = pr.a00, ne = pr.a10;
            if (nf == curF[h] && ne == curE[h]) break;
            uint32_t of = curF[h], oe = curE[h];
            curF[h] = nf; curE[h] = ne;
            int p = parent[h];
            if (p < 0) break;
            if (of == 0) {
                zc[p]--;
                sumE[p] = addmod(sumE[p], (MOD - oe) % MOD);
            } else {
                uint32_t iv = modinv(of);
                pn[p] = mulmod(pn[p], iv);
                Ssum[p] = addmod(Ssum[p], mulmod(oe, iv));
            }
            if (nf == 0) {
                zc[p]++;
                sumE[p] = addmod(sumE[p], ne);
            } else {
                uint32_t iv = modinv(nf);
                pn[p] = mulmod(pn[p], nf);
                Ssum[p] = addmod(Ssum[p], (MOD - mulmod(ne, iv)) % MOD);
            }
            recomputeMat(p);
            u = p;
        }

        uint32_t ans = mulmod(curF[0], pow2[(size_t)(K - 1)]);
        out += to_string(ans);
        out += '\n';
    }
    cout << out;
    return 0;
}

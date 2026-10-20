#include <bits/stdc++.h>
using namespace std;
using u32 = unsigned int;
using u64 = unsigned long long;

constexpr u32 MOD = 1000000007u;

static int N, K, Q;

// ---------------- input graph ----------------
static vector<u32> adj; // N*N adjacency, mod MOD

// ---------------- char poly ----------------
static int nn;             // degree of charpoly == N
static vector<u32> fcoef;  // monic charpoly, size nn+1, low->high
static vector<u32> Rtab;   // (nn-1) * nn : z^{nn+i} mod fcoef

static inline u32 modpow(u32 a, u64 e) {
    u64 r = 1, b = a;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return (u32)r;
}

static void computeCharPoly() {
    vector<u32> H = adj;
    // Hessenberg reduction
    for (int j = 0; j + 2 < nn; j++) {
        int piv = -1;
        for (int i = j + 2; i < nn; i++) if (H[(size_t)i * nn + j]) { piv = i; break; }
        if (piv < 0) continue;
        if (piv != j + 1) {
            for (int t = 0; t < nn; t++) swap(H[(size_t)(j + 1) * nn + t], H[(size_t)piv * nn + t]);
            for (int t = 0; t < nn; t++) swap(H[(size_t)t * nn + (j + 1)], H[(size_t)t * nn + piv]);
        }
        u32 inv = modpow(H[(size_t)(j + 1) * nn + j], MOD - 2);
        for (int i = j + 2; i < nn; i++) {
            u32 h = H[(size_t)i * nn + j];
            if (!h) continue;
            u32 cf = (u64)h * inv % MOD;
            u32 ncf = cf ? MOD - cf : 0;
            u32* ri = &H[(size_t)i * nn];
            const u32* rj = &H[(size_t)(j + 1) * nn];
            for (int t = 0; t < nn; t++) ri[t] = (u32)((ri[t] + (u64)ncf * rj[t]) % MOD);
            for (int t = 0; t < nn; t++)
                H[(size_t)t * nn + (j + 1)] = (u32)((H[(size_t)t * nn + (j + 1)] + (u64)cf * H[(size_t)t * nn + i]) % MOD);
        }
    }
    vector<vector<u32>> p(nn + 1);
    p[0].assign(1, 1);
    for (int k = 1; k <= nn; k++) {
        vector<u32> cur(k + 1, 0);
        const vector<u32>& pv = p[k - 1];
        u32 d = H[(size_t)(k - 1) * nn + (k - 1)];
        u32 md = d ? MOD - d : 0;
        for (int t = 0; t < (int)pv.size(); t++) {
            cur[t + 1] = (u32)((cur[t + 1] + pv[t]) % MOD);
            cur[t] = (u32)((cur[t] + (u64)md * pv[t]) % MOD);
        }
        u32 prod = 1;
        for (int i = k - 1; i >= 1; i--) {
            prod = (u32)((u64)prod * H[(size_t)i * nn + (i - 1)] % MOD);
            u32 coeff = (u32)((u64)H[(size_t)(i - 1) * nn + (k - 1)] * prod % MOD);
            if (coeff) {
                u32 mc = MOD - coeff;
                const vector<u32>& pi = p[i - 1];
                for (int t = 0; t < (int)pi.size(); t++)
                    cur[t] = (u32)((cur[t] + (u64)mc * pi[t]) % MOD);
            }
        }
        p[k] = move(cur);
    }
    fcoef = p[nn];
    fcoef.resize(nn + 1, 0);
    fcoef[nn] = 1; // should already be 1
}

static void buildRtab() {
    Rtab.assign((size_t)max(0, nn - 1) * nn, 0);
    if (nn <= 1) return;
    vector<u32> cur(nn);
    for (int t = 0; t < nn; t++) cur[t] = fcoef[t] ? MOD - fcoef[t] : 0;
    for (int i = 0; i < nn - 1; i++) {
        memcpy(&Rtab[(size_t)i * nn], cur.data(), sizeof(u32) * nn);
        if (i + 1 < nn - 1) {
            u32 top = cur[nn - 1];
            for (int t = nn - 1; t >= 1; t--) cur[t] = cur[t - 1];
            cur[0] = 0;
            if (top) {
                u32 mt = MOD - top;
                for (int t = 0; t < nn; t++) cur[t] = (u32)((cur[t] + (u64)mt * fcoef[t]) % MOD);
            }
        }
    }
}

static u64 tmpbuf[2105], accbuf[1060];

static void mulR(const u32* a, const u32* b, u32* out) {
    int n = nn;
    if (n == 1) { out[0] = (u32)((u64)a[0] * b[0] % MOD); return; }
    int L = 2 * n - 1;
    for (int i = 0; i < L; i++) tmpbuf[i] = 0;
    for (int i = 0; i < n; i++) {
        u64 ai = a[i];
        if (ai) {
            u64* tp = tmpbuf + i;
            const u32* bp = b;
            for (int j = 0; j < n; j++) tp[j] += ai * bp[j];
        }
        if ((i & 15) == 15) for (int t = 0; t < L; t++) tmpbuf[t] %= MOD;
    }
    for (int t = 0; t < L; t++) tmpbuf[t] %= MOD;
    for (int t = 0; t < n; t++) accbuf[t] = tmpbuf[t];
    for (int i = 0; i < n - 1; i++) {
        u64 c = tmpbuf[n + i];
        if (c) {
            const u32* r = &Rtab[(size_t)i * n];
            for (int t = 0; t < n; t++) accbuf[t] += c * r[t];
        }
        if ((i & 15) == 15) for (int t = 0; t < n; t++) accbuf[t] %= MOD;
    }
    for (int t = 0; t < n; t++) out[t] = (u32)(accbuf[t] % MOD);
}

static void addmod(u32* out, const u32* a) { for (int t = 0; t < nn; t++) { u32 s = out[t] + a[t]; out[t] = s >= MOD ? s - MOD : s; } }

static vector<u32> valOne() { vector<u32> v(nn, 0); v[0] = 1; return v; }

// shift by z:  out = z * a  (in R)
static void shiftPoly(const u32* a, u32* out) {
    // out = z*a mod f  (alias-safe)
    u32 top = a[nn - 1];
    for (int t = nn - 1; t >= 1; t--) out[t] = a[t - 1];
    out[0] = 0;
    {
        if (top) {
            u32 mt = MOD - top;
            for (int t = 0; t < nn; t++) out[t] = (u32)((out[t] + (u64)mt * fcoef[t]) % MOD);
        }
    }
}

int main() {
    scanf("%d %d %d", &N, &K, &Q);
    adj.assign((size_t)N * N, 0);
    {
        vector<char> line(N + 2);
        for (int i = 0; i < N; i++) {
            scanf("%s", line.data());
            for (int j = 0; j < N; j++) adj[(size_t)i * N + j] = (line[j] == '1');
        }
    }
    nn = N;
    computeCharPoly();
    buildRtab();

    // V[j], j=0..K   ; D[j] for j=0..K-1 ; Suff[b] b=0..K
    vector<vector<u32>> V(K + 1), D(K > 0 ? K : 1), Suff(K + 2);
    V[0] = valOne();
    vector<u32> sq(nn), t1(nn), t2(nn);
    for (int j = 0; j < K; j++) {
        mulR(V[j].data(), V[j].data(), sq.data());     // sq = V_j^2
        // D_j = 1 + z*V_j
        vector<u32> dd(nn, 0);
        dd[0] = 1;
        {
            vector<u32> sh(nn);
            shiftPoly(V[j].data(), sh.data());
            addmod(dd.data(), sh.data());
        }
        D[j] = move(dd);
        // V_{j+1} = V_j + z*sq
        vector<u32> nv = V[j];
        {
            vector<u32> sh(nn);
            shiftPoly(sq.data(), sh.data());
            addmod(nv.data(), sh.data());
        }
        V[j + 1] = move(nv);
    }
    // Suff
    Suff[K].assign(nn, 0);
    for (int b = K - 1; b >= 0; b--) {
        vector<u32> s(nn, 0);
        s[0] = 1;
        if (b < (int)D.size()) {
            mulR(D[b].data(), D[b].data(), t1.data());       // D_b^2
            mulR(t1.data(), Suff[b + 1].data(), t2.data());   // * Suff_{b+1}
            addmod(s.data(), t2.data());
        }
        Suff[b] = move(s);
    }

    // segment tree over D_j (j = 0..K-1) products, commutative ring
    int segN = 1; while (segN < max(1, K)) segN <<= 1;
    vector<u32> seg((size_t)2 * segN * nn, 0);
    auto segAt = [&](int i) -> u32* { return &seg[(size_t)i * nn]; };
    for (int i = 0; i < segN; i++) {
        u32* p = segAt(segN + i);
        if (i < K) memcpy(p, D[i].data(), sizeof(u32) * nn);
        else p[0] = 1;
    }
    for (int i = segN - 1; i >= 1; i--) {
        mulR(segAt(2 * i), segAt(2 * i + 1), segAt(i));
    }
    auto rangeProd = [&](int l, int r, vector<u32>& res) {
        // product D_l * ... * D_r  (inclusive); if l>r -> 1
        res.assign(nn, 0); res[0] = 1;
        if (l > r) return;
        l += segN; r += segN + 1;
        vector<u32> acc(nn, 0), tmpv(nn);
        bool have = false;
        while (l < r) {
            if (l & 1) { if (!have) { memcpy(acc.data(), segAt(l), sizeof(u32) * nn); have = true; } else { mulR(acc.data(), segAt(l), tmpv.data()); acc.swap(tmpv); } ++l; }
            if (r & 1) { --r; if (!have) { memcpy(acc.data(), segAt(r), sizeof(u32) * nn); have = true; } else { mulR(acc.data(), segAt(r), tmpv.data()); acc.swap(tmpv); } }
            l >>= 1; r >>= 1;
        }
        if (have) res = acc;
    };

    // read queries
    struct Query { int a, b, s, t; };
    vector<Query> qs(Q);
    vector<pair<int,int>> pairs;
    for (int i = 0; i < Q; i++) {
        int bs, s, bt, t;
        scanf("%d %d %d %d", &bs, &s, &bt, &t);
        --s; --t;
        int a = min(bs, bt), b = max(bs, bt);
        qs[i] = {a, b, s, t};
        pairs.push_back({a, b});
    }
    sort(pairs.begin(), pairs.end());
    pairs.erase(unique(pairs.begin(), pairs.end()), pairs.end());
    map<pair<int,int>, vector<u32>> Hs;

    for (auto& pr : pairs) {
        int a = pr.first, b = pr.second;
        vector<u32> H(nn, 0), X(nn), u1(nn), u2(nn), u3(nn), u4(nn);
        if (a == b) {
            // H = 1 + z^2 V_{a-1}^2 Suff_a
            mulR(V[a - 1].data(), V[a - 1].data(), u1.data());
            mulR(u1.data(), Suff[a].data(), u2.data());
            // z^2 u2
            vector<u32> sh1(nn), sh2(nn);
            shiftPoly(u2.data(), sh1.data());
            shiftPoly(sh1.data(), sh2.data());
            H[0] = 1;
            addmod(H.data(), sh2.data());
        } else {
            // X = prod_{j=a}^{b-2} D_j
            rangeProd(a, b - 2, X);
            // u1 = V_{a-1} * X
            mulR(V[a - 1].data(), X.data(), u1.data());
            // u2 = u1 * D_{b-1}  (= V_{a-1} X_{a,b})
            mulR(u1.data(), D[b - 1].data(), u2.data());
            // u3 = u2 * V_{b-1}
            mulR(u2.data(), V[b - 1].data(), u3.data());
            // u4 = u3 * Suff_b
            mulR(u3.data(), Suff[b].data(), u4.data());
            vector<u32> sh1(nn), sh2(nn), sh3(nn);
            shiftPoly(u1.data(), sh1.data());       // z * u1
            shiftPoly(u4.data(), sh2.data());
            shiftPoly(sh2.data(), sh3.data());      // z^2 * u4
            H = sh1;
            addmod(H.data(), sh3.data());
        }
        Hs[pr] = move(H);
    }

    // ---- power tables of M for evaluation ----
    int B = max(1, (int)(sqrt((double)nn) + 0.5));
    if (B > nn) B = nn;
    int nb = (nn + B - 1) / B;
    // Pt[j][t*n+k] = (M^j)_{k,t}
    vector<u32> Pt((size_t)B * nn * nn, 0);
    vector<u32> curMat((size_t)nn * nn, 0), nxt((size_t)nn * nn, 0);
    // curMat = identity
    for (int i = 0; i < nn; i++) curMat[(size_t)i * nn + i] = 1;
    auto matMulInto = [&](const vector<u32>& A, const vector<u32>& Bm, vector<u32>& C) {
        // C = A*B
        vector<u64> tmp(nn);
        for (int i = 0; i < nn; i++) {
            for (int t = 0; t < nn; t++) tmp[t] = 0;
            const u32* Ai = &A[(size_t)i * nn];
            for (int k = 0; k < nn; k++) {
                u64 a = Ai[k];
                if (a) {
                    const u32* Bk = &Bm[(size_t)k * nn];
                    for (int j = 0; j < nn; j++) tmp[j] += a * Bk[j];
                }
                if ((k & 15) == 15) for (int j = 0; j < nn; j++) tmp[j] %= MOD;
            }
            u32* Ci = &C[(size_t)i * nn];
            for (int j = 0; j < nn; j++) Ci[j] = (u32)(tmp[j] % MOD);
        }
    };
    // P[0..B-1] transposed stored; roll curMat
    for (int j = 0; j < B; j++) {
        u32* dst = &Pt[(size_t)j * nn * nn];
        for (int r = 0; r < nn; r++) {
            const u32* src = &curMat[(size_t)r * nn];
            for (int c = 0; c < nn; c++) dst[(size_t)c * nn + r] = src[c];
        }
        if (j + 1 < B) { matMulInto(curMat, adj, nxt); curMat.swap(nxt); }
    }
    // MB = M^B = curMat * adj
    vector<u32> MB((size_t)nn * nn, 0);
    matMulInto(curMat, adj, MB);
    // Q[i] = M^{i*B}
    vector<u32> Qtab((size_t)nb * nn * nn, 0);
    {
        vector<u32> q((size_t)nn * nn, 0), qn((size_t)nn * nn, 0);
        for (int i = 0; i < nn; i++) q[(size_t)i * nn + i] = 1;
        for (int i = 0; i < nb; i++) {
            memcpy(&Qtab[(size_t)i * nn * nn], q.data(), sizeof(u32) * nn * nn);
            if (i + 1 < nb) { matMulInto(q, MB, qn); q.swap(qn); }
        }
    }

    string outbuf;
    outbuf.reserve(Q * 12);
    char buf[32];
    for (int qi = 0; qi < Q; qi++) {
        const Query& qu = qs[qi];
        const vector<u32>& H = Hs[{qu.a, qu.b}];
        u64 ans = 0;
        for (int i = 0; i < nn; i++) {
            u32 h = H[i];
            if (!h) continue;
            int blk = i / B, j = i % B;
            const u32* Qs = &Qtab[(size_t)blk * nn * nn + (size_t)qu.s * nn];
            const u32* Pj = &Pt[(size_t)j * nn * nn + (size_t)qu.t * nn];
            u64 acc = 0;
            for (int k = 0; k < nn; k++) {
                acc += (u64)Qs[k] * Pj[k];
                if ((k & 15) == 15) acc %= MOD;
            }
            acc %= MOD;
            ans = (ans + acc * h) % MOD;
        }
        int len = snprintf(buf, sizeof(buf), "%llu\n", (unsigned long long)ans);
        outbuf.append(buf, len);
    }
    fwrite(outbuf.data(), 1, outbuf.size(), stdout);
    return 0;
}

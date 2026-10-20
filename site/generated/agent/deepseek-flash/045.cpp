// 交通枢纽复建
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long u64;
typedef unsigned int u32;

static const int BUFSZ = 1 << 22;
static char ibuf[BUFSZ];
static int ipos = 0, ilen = 0;
static inline int gc() {
    if (ipos >= ilen) { ilen = (int)fread(ibuf, 1, BUFSZ, stdin); ipos = 0; if (ilen <= 0) return -1; }
    return ibuf[ipos++];
}
static inline bool readInt(ll& out) {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9') && c != '-') c = gc();
    if (c == -1) return false;
    bool neg = false;
    if (c == '-') { neg = true; c = gc(); }
    ll v = 0;
    while (c >= '0' && c <= '9') { v = v * 10 + (c - '0'); c = gc(); }
    out = neg ? -v : v;
    return true;
}

struct DSU {
    vector<int> p, sz;
    void init(int n) { p.resize(n); iota(p.begin(), p.end(), 0); sz.assign(n, 1); }
    int find(int x) { while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; } return x; }
    int uni(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return a;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a; sz[a] += sz[b];
        return a;
    }
};

int main() {
    ll n_, m_, k_;
    if (!readInt(n_)) return 0;
    readInt(m_); readInt(k_);
    int n = (int)n_, m = (int)m_, k = (int)k_;

    vector<u64> ce(m);
    for (int i = 0; i < m; i++) {
        ll u, v, w;
        readInt(u); readInt(v); readInt(w);
        u--; v--;
        ce[i] = ((u64)w << 34) | ((u64)u << 17) | (u64)v;
    }
    vector<ll> c(k);
    vector<u32> av((size_t)k * n);
    for (int j = 0; j < k; j++) {
        ll cj; readInt(cj); c[j] = cj;
        for (int i = 0; i < n; i++) { ll x; readInt(x); av[(size_t)j * n + i] = (u32)x; }
    }

    if (n == 1) { printf("0\n"); return 0; }

    // ---------- 1. MST of city graph ----------
    sort(ce.begin(), ce.end());
    DSU d1; d1.init(n);
    vector<u64> t0; t0.reserve(n - 1);
    ll W0 = 0;
    for (int i = 0; i < m && (int)t0.size() < n - 1; i++) {
        u64 e = ce[i];
        int u = (int)((e >> 17) & 0x1FFFF), v = (int)(e & 0x1FFFF);
        if (d1.find(u) != d1.find(v)) {
            d1.uni(u, v);
            t0.push_back(e);
            W0 += (ll)(e >> 34);
        }
    }
    vector<u64>().swap(ce);

    if (k == 0) { printf("%lld\n", W0); return 0; }

    // ---------- 2. Kruskal reconstruction tree of T0 ----------
    int numNodes = n + (int)t0.size();
    vector<int> formN(numNodes, 0), mergeN(numNodes, INT_MAX);
    vector<int> cl(numNodes, -1), cr(numNodes, -1);
    vector<int> nodeOf(n);
    for (int i = 0; i < n; i++) nodeOf[i] = i;
    {
        DSU d2; d2.init(n);
        int nxt = n;
        for (auto& e : t0) {
            int u = (int)((e >> 17) & 0x1FFFF), v = (int)(e & 0x1FFFF);
            int w = (int)(e >> 34);
            int ru = d2.find(u), rv = d2.find(v);
            int x = nxt++;
            formN[x] = w;
            cl[x] = nodeOf[ru]; cr[x] = nodeOf[rv];
            mergeN[nodeOf[ru]] = w; mergeN[nodeOf[rv]] = w;
            int nr = d2.uni(ru, rv);
            nodeOf[nr] = x;
        }
        mergeN[nodeOf[d2.find(0)]] = INT_MAX;
    }
    vector<int>().swap(nodeOf);

    int full = (1 << k) - 1;

    // ---------- 3. Phi(S) preprocessing ----------
    vector<u32> V((size_t)numNodes * k);
    {
        for (int i = 0; i < n; i++) {
            u32* dst = &V[(size_t)i * k];
            for (int j = 0; j < k; j++) dst[j] = av[(size_t)j * n + i];
        }
        for (int x = n; x < numNodes; x++) {
            const u32* Vl = &V[(size_t)cl[x] * k];
            const u32* Vr = &V[(size_t)cr[x] * k];
            u32* Vx = &V[(size_t)x * k];
            for (int j = 0; j < k; j++) Vx[j] = min(Vl[j], Vr[j]);
        }
    }

    vector<ll> T((size_t)(full + 1) * k, 0);
    {
        u32 vals[16]; int idx[16];
        for (int x = 0; x < numNodes; x++) {
            ll f = formN[x], mg = mergeN[x];
            const u32* Vx = &V[(size_t)x * k];
            for (int j = 0; j < k; j++) { vals[j] = Vx[j]; idx[j] = j; }
            for (int a = 1; a < k; a++) {
                u32 vv = vals[a]; int ii = idx[a];
                int b = a - 1;
                while (b >= 0 && vals[b] > vv) { vals[b + 1] = vals[b]; idx[b + 1] = idx[b]; b--; }
                vals[b + 1] = vv; idx[b + 1] = ii;
            }
            int mask = 0;
            for (int t = 0; t < k; t++) {
                ll v = vals[t]; int j = idx[t];
                ll mm = (mg < v) ? mg : v;
                if (mm > f) T[(size_t)mask * k + j] += mm - f;
                mask |= (1 << j);
            }
        }
    }
    vector<u32>().swap(V);
    vector<int>().swap(formN); vector<int>().swap(mergeN);
    vector<int>().swap(cl); vector<int>().swap(cr);

    // subset zeta transform over the mask index: T[S][j] = sum_{E subset S} T0[E][j]
    for (int b = 0; b < k; b++) {
        for (int mask = 0; mask <= full; mask++) {
            if ((mask >> b) & 1) {
                ll* A = &T[(size_t)mask * k];
                ll* B = &T[(size_t)(mask ^ (1 << b)) * k];
                for (int j = 0; j < k; j++) A[j] += B[j];
            }
        }
    }

    // ---------- 4. delta(j,j') sweep ----------
    vector<int> delta((size_t)k * k, INT_MAX);
    {
        // Event ids: [0,nC) are the MST city edges, the rest are star edges (j,i) with
        // j = (id-nC)/n, i = (id-nC)%n.  Sort key = (weight << 21) | id.
        int nC = (int)t0.size();
        size_t nStar = (size_t)n * k;
        vector<u64> ev((size_t)nC + nStar);
        vector<u64> cityUV(nC);
        for (int idx = 0; idx < nC; idx++) {
            u64 e = t0[idx];
            cityUV[idx] = (e & ((1ULL << 34) - 1)) | (1ULL << 34);  // marks a valid payload
            ev[idx] = (((e >> 34) << 21) | (u64)idx);
        }
        vector<u64>().swap(t0);
        for (size_t j = 0; j < (size_t)k; j++) {
            const u32* aj = &av[j * n];
            u64* dst = &ev[nC + j * n];
            for (int i = 0; i < n; i++) dst[i] = ((u64)aj[i] << 21) | (u64)(nC + j * n + i);
        }
        vector<u32>().swap(av);
        sort(ev.begin(), ev.end());

        DSU d3; d3.init(n);
        vector<int> mask3(n, 0);
        vector<int> R(k, 0);
        for (int j = 0; j < k; j++) R[j] = (1 << j);
        for (auto& e : ev) {
            u32 id = (u32)(e & ((1u << 21) - 1));
            u32 w = (u32)(e >> 21);
            if (id < (u32)nC) {
                u64 uv = cityUV[id];
                int u = (int)((uv >> 17) & 0x1FFFF), v = (int)(uv & 0x1FFFF);
                int ru = d3.find(u), rv = d3.find(v);
                if (ru == rv) continue;
                int m1 = mask3[ru], m2 = mask3[rv];
                if (__builtin_popcount(m1) > __builtin_popcount(m2)) swap(m1, m2);
                int tmp = m1;
                while (tmp) {
                    int j = __builtin_ctz(tmp); tmp &= tmp - 1;
                    int other = m2 & ~R[j];
                    while (other) {
                        int j2 = __builtin_ctz(other); other &= other - 1;
                        delta[(size_t)j * k + j2] = (int)w;
                        delta[(size_t)j2 * k + j] = (int)w;
                        R[j] |= 1 << j2; R[j2] |= 1 << j;
                    }
                }
                int nr = d3.uni(ru, rv);
                mask3[nr] = m1 | m2;
            } else {
                u32 t = id - (u32)nC;
                int j = (int)(t / (u32)n), i = (int)(t % (u32)n);
                int ri = d3.find(i);
                int other = mask3[ri] & ~R[j];
                while (other) {
                    int j2 = __builtin_ctz(other); other &= other - 1;
                    delta[(size_t)j * k + j2] = (int)w;
                    delta[(size_t)j2 * k + j] = (int)w;
                    R[j] |= 1 << j2; R[j2] |= 1 << j;
                }
                mask3[ri] |= 1 << j;
            }
        }
    }

    // ---------- 5. enumerate subsets ----------
    vector<ll> cs(full + 1, 0), mh(full + 1, 0);
    for (int S = 1; S <= full; S++) {
        int lb = __builtin_ctz(S);
        cs[S] = cs[S ^ (1 << lb)] + c[lb];
    }
    ll best = W0;
    for (int S = 1; S <= full; S++) {
        // HubMST(S)
        if ((S & (S - 1)) == 0) mh[S] = 0;
        else {
            ll bv = LLONG_MAX;
            for (int t = S; t; t &= t - 1) {
                int j = __builtin_ctz(t);
                int rest = S ^ (1 << j);
                if (rest == 0) continue;
                int cm = INT_MAX;
                for (int t2 = rest; t2; t2 &= t2 - 1) {
                    int j2 = __builtin_ctz(t2);
                    int d = delta[(size_t)j * k + j2];
                    if (d < cm) cm = d;
                }
                if (cm == INT_MAX) continue;
                ll cand = mh[rest] + (ll)cm;
                if (cand < bv) bv = cand;
            }
            mh[S] = (bv == LLONG_MAX) ? (LLONG_MAX / 4) : bv;
        }
        // Phi(S)
        int comp = full ^ S;
        ll phi = 0;
        for (int t = S; t; t &= t - 1) {
            int j = __builtin_ctz(t);
            phi += T[(size_t)comp * k + j];
        }
        ll tot = cs[S] + phi + mh[S];
        if (tot < best) best = tot;
    }
    printf("%lld\n", best);
    return 0;
}

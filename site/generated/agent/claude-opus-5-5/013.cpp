#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef __int128 lll;

// ---------------- treap of breakpoints (key = position, dec = slope decrease) ----------------
struct TNode { int key, lz; ll dec, sum; unsigned pri; int l, r; };
static vector<TNode> T;
static unsigned rngState = 2463534242u;
static unsigned rnd() { rngState ^= rngState << 13; rngState ^= rngState >> 17; rngState ^= rngState << 5; return rngState; }
static int newNode(int key, ll dec) {
    T.push_back({key, 0, dec, dec, rnd(), 0, 0});
    return (int)T.size() - 1;
}
static inline void applyShift(int t, int d) { if (t) { T[t].key += d; T[t].lz += d; } }
static inline void push(int t) {
    if (T[t].lz) { applyShift(T[t].l, T[t].lz); applyShift(T[t].r, T[t].lz); T[t].lz = 0; }
}
static inline void pull(int t) { T[t].sum = T[t].dec + T[T[t].l].sum + T[T[t].r].sum; }
// a: keys < k, b: keys >= k
static void split(int t, int k, int &a, int &b) {
    if (!t) { a = b = 0; return; }
    push(t);
    if (T[t].key < k) { split(T[t].r, k, T[t].r, b); a = t; pull(t); }
    else { split(T[t].l, k, a, T[t].l); b = t; pull(t); }
}
static int join(int a, int b) {
    if (!a) return b;
    if (!b) return a;
    if (T[a].pri > T[b].pri) { push(a); T[a].r = join(T[a].r, b); pull(a); return a; }
    push(b); T[b].l = join(a, T[b].l); pull(b); return b;
}
static int unite(int a, int b) {
    if (!a) return b;
    if (!b) return a;
    if (T[a].pri < T[b].pri) swap(a, b);
    push(a);
    int bl, br, mid;
    split(b, T[a].key, bl, br);
    split(br, T[a].key + 1, mid, br);
    if (mid) T[a].dec += T[mid].sum; // merge equal keys (nodes discarded)
    T[a].l = unite(T[a].l, bl);
    T[a].r = unite(T[a].r, br);
    pull(a);
    return a;
}
static ll prefixSum(int t, int m) { // sum of dec with key <= m
    ll s = 0;
    while (t) {
        push(t);
        if (T[t].key <= m) { s += T[T[t].l].sum + T[t].dec; t = T[t].r; }
        else t = T[t].l;
    }
    return s;
}
static const int INFP = INT_MAX;
static int firstReach(int t, ll need) { // first key where cumulative >= need (need > 0)
    if (T[t].sum < need) return INFP;
    while (t) {
        push(t);
        ll ls = T[T[t].l].sum;
        if (ls >= need) { t = T[t].l; continue; }
        need -= ls;
        if (T[t].dec >= need) return T[t].key;
        need -= T[t].dec;
        t = T[t].r;
    }
    return INFP; // unreachable
}

struct Seq { ll init = 0; int root = 0; };
static inline ll valueAt(const Seq &s, int m) { return s.init - prefixSum(s.root, m); }
static inline int firstNonPos(const Seq &s) {
    if (s.init <= 0) return 0;
    if (!s.root) return INFP;
    return firstReach(s.root, s.init);
}
static void truncateAt(Seq &s, int c) {
    if (c == INFP) return;
    if (c == 0) { s.init = 0; s.root = 0; return; }
    ll vp = valueAt(s, c - 1);
    int a, b;
    split(s.root, c, a, b);
    s.root = join(a, newNode(c, vp));
}
static void clip(Seq &s) { truncateAt(s, firstNonPos(s)); }

// transform children-sum (D,G) by node with weight w, cost p
static void transformNode(Seq &D, Seq &G, ll w, ll p) {
    ll alpha = w - p, beta = -w - p;
    D.init += alpha; G.init += beta;
    int a = firstNonPos(D), b = firstNonPos(G);
    if (a != INFP && a <= b) {
        Seq z;
        if (a == 0) {
            z = D; D.init = 0; D.root = 0;
        } else {
            ll xa1 = valueAt(D, a - 1), xa = valueAt(D, a);
            int L, R, mid;
            split(D.root, a, L, R);
            split(R, a + 1, mid, R);
            D.root = join(L, newNode(a, xa1));
            z.init = 0;
            z.root = join(newNode(a, -xa), R);
        }
        G.init += z.init;
        G.root = unite(G.root, z.root);
        clip(G);
    } else {
        // b finite, a > b
        ll yb = valueAt(G, b);
        Seq z;
        int L, R, mid;
        if (b == 0) {
            R = G.root;
            G.init = 0; G.root = 0;
        } else {
            ll yb1 = valueAt(G, b - 1);
            split(G.root, b, L, R);
            split(R, b + 1, mid, R);
            G.root = join(L, newNode(b, yb1));
        }
        applyShift(R, 1);
        z.init = 0;
        z.root = join(newNode(b + 1, -yb), R);
        D.root = unite(D.root, z.root);
        clip(D);
    }
}

static void collect(int t, vector<pair<int, ll>> &out) {
    if (!t) return;
    push(t);
    collect(T[t].l, out);
    out.push_back({T[t].key, T[t].dec});
    collect(T[t].r, out);
}
static vector<ll> extractVals(const Seq &s) {
    vector<pair<int, ll>> bp;
    collect(s.root, bp);
    vector<ll> v;
    ll cur = s.init;
    size_t idx = 0;
    for (int m = 0;; m++) {
        while (idx < bp.size() && bp[idx].first <= m) { cur -= bp[idx].second; idx++; }
        if (cur <= 0) break;
        v.push_back(cur);
    }
    return v;
}

// ---------------- segment tree with 4x4 max-plus matrices ----------------
static const ll NEG = LLONG_MIN / 4;
struct Mat { ll a[4][4]; };
static vector<Mat> seg;
static int SZ;
static inline void combine(const Mat &L, const Mat &R, Mat &C) {
    ll Rp[4][4];
    for (int d = 0; d < 4; d++) {
        ll best = NEG;
        for (int b = 3; b >= 0; b--) {
            if (R.a[b][d] > best) best = R.a[b][d];
            Rp[b][d] = best;
        }
    }
    for (int a = 0; a < 4; a++)
        for (int d = 0; d < 4; d++) {
            ll best = NEG;
            if (d >= a)
                for (int b = a; b <= d; b++) {
                    if (L.a[a][b] == NEG || Rp[b][d] == NEG) continue;
                    ll v = L.a[a][b] + Rp[b][d];
                    if (v > best) best = v;
                }
            C.a[a][d] = best;
        }
}

int main() {
    int n, s;
    if (scanf("%d %d", &n, &s) != 2) return 0;
    s--;
    vector<ll> w(n), p(n);
    vector<int> a(n);
    for (auto &x : w) scanf("%lld", &x);
    for (auto &x : p) scanf("%lld", &x);
    for (auto &x : a) { scanf("%d", &x); x--; }

    T.reserve(4 * n + 16);
    T.push_back({0, 0, 0, 0, 0, 0, 0});

    // cycle detection from s
    vector<int> seqv;
    seqv.push_back(s);
    int x = a[s];
    while (x != s && (int)seqv.size() <= n) { seqv.push_back(x); x = a[x]; }
    bool onCyc = (x == s);
    vector<char> inCyc(n, 0);
    if (onCyc) for (int v : seqv) inCyc[v] = 1;
    else inCyc[s] = 1; // treat s as root (not a tree node)

    // reverse adjacency
    vector<int> head(n, -1), nxt(n);
    for (int i = 0; i < n; i++) { nxt[i] = head[a[i]]; head[a[i]] = i; }

    vector<int> roots;
    if (onCyc) roots = seqv; else roots.push_back(s);

    // BFS order of tree nodes
    vector<int> order;
    order.reserve(n);
    for (int r : roots)
        for (int c = head[r]; c != -1; c = nxt[c]) if (!inCyc[c]) order.push_back(c);
    for (size_t i = 0; i < order.size(); i++) {
        int u = order[i];
        for (int c = head[u]; c != -1; c = nxt[c]) order.push_back(c);
    }
    vector<Seq> D(n), G(n);
    for (int i = (int)order.size() - 1; i >= 0; i--) {
        int u = order[i];
        // children already merged into D[u],G[u]
        transformNode(D[u], G[u], w[u], p[u]);
        int par = a[u];
        D[par].init += D[u].init; D[par].root = unite(D[par].root, D[u].root);
        G[par].init += G[u].init; G[par].root = unite(G[par].root, G[u].root);
    }

    if (!onCyc) {
        ll S1 = D[s].init, S2 = D[s].init + G[s].init;
        ll ans = max(w[s] + S1, -p[s] + S2);
        printf("%lld\n", ans);
        return 0;
    }

    int L = (int)seqv.size();
    vector<int> cn(L);
    for (int k = 0; k < L; k++) cn[k] = seqv[(L - k) % L];
    vector<vector<ll>> PD(L), PG(L);
    vector<int> Kj(L);
    for (int j = 0; j < L; j++) {
        vector<ll> dv = extractVals(D[cn[j]]), gv = extractVals(G[cn[j]]);
        PD[j].assign(dv.size() + 1, 0);
        for (size_t i = 0; i < dv.size(); i++) PD[j][i + 1] = PD[j][i] + dv[i];
        PG[j].assign(gv.size() + 1, 0);
        for (size_t i = 0; i < gv.size(); i++) PG[j][i + 1] = PG[j][i] + gv[i];
        int A = (int)dv.size(), B = (int)gv.size();
        Kj[j] = max(max(2 * A - 1, 2 * B), 0);
    }
    auto Tval = [&](int j, ll k) -> ll {
        ll ha = (k + 1) / 2, hb = k / 2;
        ll A = (ll)PD[j].size() - 1, B = (ll)PG[j].size() - 1;
        return PD[j][min(ha, A)] + PG[j][min(hb, B)];
    };
    auto Fval = [&](int j, ll k) -> ll {
        int v = cn[j];
        return Tval(j, k) - p[v] * k + ((k & 1) ? w[v] : 0) + (j == 0 ? p[v] : 0);
    };

    if (L == 2) {
        ll best = LLONG_MIN;
        int cfg[3][2] = {{1, 0}, {1, 1}, {2, 0}};
        for (auto &c : cfg) best = max(best, Fval(0, c[0]) + Fval(1, c[1]));
        printf("%lld\n", best);
        return 0;
    }

    SZ = 1;
    while (SZ < L) SZ <<= 1;
    seg.assign(2 * SZ, Mat());
    Mat ident;
    for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) ident.a[i][j] = (i == j) ? 0 : NEG;
    for (int i = 0; i < 2 * SZ; i++) seg[i] = ident;
    const int off[4] = {3, 2, 1, 0};
    auto setLeaf = [&](int j, ll c) {
        Mat &M = seg[SZ + j];
        ll base = Fval(j, 2 * c);
        for (int i = 0; i < 4; i++) for (int k = 0; k < 4; k++) M.a[i][k] = NEG;
        for (int st = 0; st < 4; st++) M.a[st][st] = Fval(j, 2 * c + off[st]) - base;
    };
    ll SP = 0;
    for (int j = 0; j < L; j++) SP += p[cn[j]];
    int maxK = 0;
    for (int j = 0; j < L; j++) maxK = max(maxK, Kj[j]);
    int Cend = (maxK + 1) / 2;
    vector<ll> curT(L);
    lll sumT = 0;
    for (int j = 0; j < L; j++) { setLeaf(j, 0); curT[j] = Tval(j, 0); sumT += curT[j]; }
    for (int i = SZ - 1; i >= 1; i--) combine(seg[2 * i], seg[2 * i + 1], seg[i]);
    // nodes sorted by Kj descending for update sets
    vector<int> byK(L);
    iota(byK.begin(), byK.end(), 0);
    sort(byK.begin(), byK.end(), [&](int x1, int x2) { return Kj[x1] > Kj[x2]; });
    lll best = 0;
    bool have = false;
    for (int c = 0; c <= Cend; c++) {
        if (c > 0) {
            for (int idx = 0; idx < L && 2 * (c - 1) < Kj[byK[idx]]; idx++) {
                int j = byK[idx];
                setLeaf(j, c);
                sumT -= curT[j];
                curT[j] = Tval(j, 2 * c);
                sumT += curT[j];
                for (int pos = (SZ + j) >> 1; pos >= 1; pos >>= 1) combine(seg[2 * pos], seg[2 * pos + 1], seg[pos]);
            }
        }
        const Mat &M = seg[1];
        ll bm = NEG;
        for (int st = 0; st < 4; st++)
            for (int en = st; en < 4; en++) {
                if (st == 0 && en == 3) continue;
                if (c == 0 && st == 3) continue;
                bm = max(bm, M.a[st][en]);
            }
        lll base = sumT - (lll)2 * c * SP + p[cn[0]];
        lll val = base + bm;
        if (!have || val > best) { best = val; have = true; }
    }
    printf("%lld\n", (ll)best);
    return 0;
}

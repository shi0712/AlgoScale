#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long ull;
typedef long long ll;
const ull MOD = 998244353;

struct Mat { ull a, b, c, d; };
static inline Mat mul(const Mat& x, const Mat& y) {
    Mat r;
    r.a = (x.a * y.a + x.b * y.c) % MOD;
    r.b = (x.a * y.b + x.b * y.d) % MOD;
    r.c = (x.c * y.a + x.d * y.c) % MOD;
    r.d = (x.c * y.b + x.d * y.d) % MOD;
    return r;
}
const Mat ID = {1, 0, 0, 1};

int n, N;
vector<vector<int>> adj;
vector<int> par, heavy, topv, pos, sz, endp, lidx, koff, kcnt;
vector<char> inS;
vector<Mat> seg;
vector<ull> LP, LQ;

void segSet(int p, const Mat& m) {
    int i = p + N;
    seg[i] = m;
    for (i >>= 1; i >= 1; i >>= 1) seg[i] = mul(seg[2 * i], seg[2 * i + 1]);
}
Mat segQuery(int l, int r) {
    Mat L = ID, R = ID;
    for (l += N, r += N + 1; l < r; l >>= 1, r >>= 1) {
        if (l & 1) L = mul(L, seg[l++]);
        if (r & 1) R = mul(seg[--r], R);
    }
    return mul(L, R);
}
void lightSet(int v, int idx, ull s, ull b) {
    int k = kcnt[v];
    int o = koff[v];
    int i = k + idx;
    LP[o + i] = s;
    LQ[o + i] = b;
    for (i >>= 1; i >= 1; i >>= 1) {
        ull P1 = LP[o + 2 * i], Q1 = LQ[o + 2 * i];
        ull P2 = LP[o + 2 * i + 1], Q2 = LQ[o + 2 * i + 1];
        LP[o + i] = P1 * P2 % MOD;
        LQ[o + i] = (Q1 * P2 + P1 * Q2) % MOD;
    }
}
Mat vertexMat(int v) {
    ull P = 1, Q = 0;
    if (kcnt[v] > 0) {
        P = LP[koff[v] + 1];
        Q = LQ[koff[v] + 1];
    }
    ull a0 = inS[v] ? 1 : 0, b0 = inS[v] ? 0 : 1;
    ull a = a0 * P % MOD;
    ull b = (b0 * P + a0 * Q) % MOD;
    return Mat{a, a, b, (a + b) % MOD};
}

int main() {
    scanf("%d", &n);
    adj.assign(n + 1, {});
    for (int i = 0; i < n - 1; i++) {
        int u, v;
        scanf("%d %d", &u, &v);
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    if (n == 1) return 0;
    vector<int> b(n - 1);
    for (auto& x : b) scanf("%d", &x);

    par.assign(n + 1, 0);
    sz.assign(n + 1, 1);
    heavy.assign(n + 1, 0);
    topv.assign(n + 1, 0);
    pos.assign(n + 1, 0);
    endp.assign(n + 1, 0);
    lidx.assign(n + 1, 0);
    koff.assign(n + 1, 0);
    kcnt.assign(n + 1, 0);
    inS.assign(n + 1, 0);

    vector<int> order;
    order.reserve(n);
    order.push_back(1);
    par[1] = 0;
    for (size_t i = 0; i < order.size(); i++) {
        int v = order[i];
        for (int u : adj[v])
            if (u != par[v]) {
                par[u] = v;
                order.push_back(u);
            }
    }
    for (int i = n - 1; i >= 1; i--) sz[par[order[i]]] += sz[order[i]];
    for (int v = 1; v <= n; v++) {
        int best = 0;
        for (int u : adj[v])
            if (u != par[v] && (best == 0 || sz[u] > sz[best])) best = u;
        heavy[v] = best;
    }
    int cur = 0;
    for (int v : order) {
        if (v == 1 || heavy[par[v]] != v) {
            int x = v;
            int last = v;
            while (x) {
                topv[x] = v;
                pos[x] = cur++;
                last = x;
                x = heavy[x];
            }
            endp[v] = pos[last];
        }
    }
    int total = 0;
    for (int v = 1; v <= n; v++) {
        int ch = 0;
        for (int u : adj[v])
            if (u != par[v]) ch++;
        int k = ch > 0 ? ch - 1 : 0;
        kcnt[v] = k;
        koff[v] = total;
        total += 2 * k + 1;
        int c = 0;
        for (int u : adj[v])
            if (u != par[v] && u != heavy[v]) lidx[u] = c++;
    }
    LP.assign(total + 1, 1);
    LQ.assign(total + 1, 0);
    N = 1;
    while (N < n) N <<= 1;
    seg.assign(2 * N, ID);

    // initial build: all vertices in U
    for (int i = n - 1; i >= 0; i--) {
        int t = order[i];
        if (!(t == 1 || heavy[par[t]] != t)) continue;
        int x = t;
        while (x) {
            segSet(pos[x], vertexMat(x));
            x = heavy[x];
        }
        if (t != 1) {
            Mat r = segQuery(pos[t], endp[t]);
            ull a = r.a, bb = r.c;
            lightSet(par[t], lidx[t], (a + bb) % MOD, bb);
        }
    }

    vector<ull> pw(2 * n + 5);
    pw[0] = 1;
    for (size_t i = 1; i < pw.size(); i++) pw[i] = pw[i - 1] * 2 % MOD;

    ll eS = 0, cntS = 0;
    for (int i = 0; i < n - 1; i++) {
        int x = b[i];
        int notS = 0;
        for (int u : adj[x])
            if (!inS[u]) notS++;
        eS += notS;
        cntS++;
        inS[x] = 1;
        int v = x;
        ull rootB = 0;
        while (true) {
            segSet(pos[v], vertexMat(v));
            int t = topv[v];
            Mat r = segQuery(pos[t], endp[t]);
            ull a = r.a, bb = r.c;
            if (t == 1) {
                rootB = bb;
                break;
            }
            int p = par[t];
            lightSet(p, lidx[t], (a + bb) % MOD, bb);
            v = p;
        }
        ull ans = pw[eS - cntS] * rootB % MOD;
        printf("%llu\n", ans);
    }
    return 0;
}

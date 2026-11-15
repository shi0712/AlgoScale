#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static const ll MOD = 998244353;
static const int MX = 1000001;
static int fct[MX + 1], ifc[MX + 1];
ll pw(ll a, ll b) { ll r = 1; a %= MOD; while (b) { if (b & 1) r = r * a % MOD; a = a * a % MOD; b >>= 1; } return r; }
static char *ibuf; static size_t ipos = 0, ilen = 0;
inline int readInt() {
    while (ipos < ilen && (ibuf[ipos] < '0' || ibuf[ipos] > '9')) ipos++;
    int x = 0;
    while (ipos < ilen && ibuf[ipos] >= '0' && ibuf[ipos] <= '9') { x = x * 10 + (ibuf[ipos] - '0'); ipos++; }
    return x;
}
int R;
inline int fv(int g) {
    if (g < R) return 0;
    return (ll)fct[g] * ifc[R] % MOD * ifc[g - R] % MOD;
}
int main() {
    // read all input
    {
        size_t cap = 1 << 25; ibuf = (char*)malloc(cap);
        size_t r;
        while ((r = fread(ibuf + ilen, 1, cap - ilen, stdin)) > 0) {
            ilen += r;
            if (ilen == cap) { cap *= 2; ibuf = (char*)realloc(ibuf, cap); }
        }
    }
    fct[0] = 1;
    for (int i = 1; i <= MX; i++) fct[i] = (ll)fct[i - 1] * i % MOD;
    ifc[MX] = pw(fct[MX], MOD - 2);
    for (int i = MX; i > 0; i--) ifc[i - 1] = (ll)ifc[i] * i % MOD;
    int t = readInt();
    string out;
    while (t--) {
        int n = readInt(), k = readInt();
        vector<int> par(n + 1, 0);
        for (int i = 2; i <= n; i++) par[i] = readInt();
        if (k == 1) { out += to_string(n % MOD) + "\n"; continue; }
        if (k == 2) { out += to_string((ll)n * (n - 1) % MOD) + "\n"; continue; }
        R = k - 1;
        vector<int> h(n + 1, 0), lc(n + 1, 0), dep(n + 1, 0);
        for (int i = n; i >= 2; i--) {
            int p = par[i];
            if (h[i] + 1 > h[p] || lc[p] == 0) { if (h[i] + 1 > h[p]) h[p] = h[i] + 1; if (lc[p] == 0 || h[i] + 1 >= h[p]) lc[p] = i; }
        }
        // ensure lc is a child achieving h[p]-1
        for (int i = 2; i <= n; i++) dep[i] = dep[par[i]] + 1;
        // recompute lc exactly
        fill(lc.begin(), lc.end(), 0);
        for (int i = 2; i <= n; i++) {
            int p = par[i];
            if (h[i] + 1 == h[p] && lc[p] == 0) lc[p] = i;
        }
        // CSR children
        vector<int> st(n + 2, 0), ch(n);
        for (int i = 2; i <= n; i++) st[par[i] + 1]++;
        for (int i = 1; i <= n + 1; i++) st[i] += st[i - 1];
        {
            vector<int> fillp(st.begin(), st.end());
            for (int i = 2; i <= n; i++) ch[fillp[par[i]]++] = i;
        }
        // positions
        vector<int> pos(n + 1, 0);
        int nxt = h[1] + 1;
        pos[1] = 0;
        for (int i = 2; i <= n; i++) {
            int p = par[i];
            if (lc[p] == i) pos[i] = pos[p] + 1;
            else { pos[i] = nxt; nxt += h[i] + 1; }
        }
        // preorder with long child visited last
        vector<int> P; P.reserve(n);
        {
            vector<int> stk; stk.push_back(1);
            while (!stk.empty()) {
                int v = stk.back(); stk.pop_back();
                P.push_back(v);
                if (lc[v]) stk.push_back(lc[v]);
                for (int e = st[v]; e < st[v + 1]; e++) if (ch[e] != lc[v]) stk.push_back(ch[e]);
            }
        }
        vector<int> buf(nxt + 2, 0), off(n + 1, 0);
        vector<int> Ds; Ds.reserve(2 * n + 2);
        for (int idx = n - 1; idx >= 0; idx--) {
            int v = P[idx];
            buf[pos[v]] += 1;
            off[v] = Ds.size();
            for (int j = 0; j <= h[v]; j++) Ds.push_back(buf[pos[v] + j]);
            if (v > 1 && lc[par[v]] != v) {
                int pp = pos[par[v]];
                for (int j = 0; j <= h[v]; j++) buf[pp + j + 1] += buf[pos[v] + j];
            }
        }
        int H = h[1];
        int OFF = H + 2;
        int S = 2 * H + 8;
        vector<int> G(S + 2, 0), bit(S + 2, 0);
        ll total = 0;
        auto bitadd = [&](int x, int val) {
            total += val; if (total >= MOD) total -= MOD;
            for (int i = x + OFF + 1; i <= S; i += i & -i) { bit[i] += val; if (bit[i] >= MOD) bit[i] -= MOD; }
        };
        auto pref = [&](int x) { // sum of indices with cell idx <= x+OFF+1
            ll s = 0;
            for (int i = x + OFF + 1; i > 0; i -= i & -i) s += bit[i];
            return s % MOD;
        };
        auto change = [&](int x, int delta) {
            int &g = G[x + OFF + 1];
            int ng = g + delta;
            int fo = fv(g), fn = fv(ng);
            g = ng;
            if (fo != fn) { int d = fn - fo; if (d < 0) d += MOD; bitadd(x, d); }
        };
        ll ans = 0;
        // root init
        for (int j = 0; j <= H; j++) {
            int g = Ds[off[1] + j];
            G[j + OFF + 1] = g;
            int f = fv(g);
            if (f) bitadd(j, f);
        }
        auto query = [&](int v) {
            ll s = (total - pref(-dep[v]) + MOD) % MOD; // x >= 1-dep => exclude x <= -dep
            ans = (ans + s) % MOD;
        };
        query(1);
        // iterative DFS
        vector<int> it(n + 1, 0);
        vector<int> stk; stk.push_back(1);
        for (int i = 1; i <= n; i++) it[i] = st[i];
        auto applyV = [&](int v, int sign) {
            int hv = h[v], dv = dep[v];
            const int *D = &Ds[off[v]];
            for (int d = 1; d <= hv + 2; d++) {
                int a = d <= hv ? D[d] : 0;
                int b = (d - 2 >= 0 && d - 2 <= hv) ? D[d - 2] : 0;
                int delta = a - b;
                if (delta) change(d - dv, sign * delta);
            }
        };
        while (!stk.empty()) {
            int v = stk.back();
            if (it[v] < st[v + 1]) {
                int c = ch[it[v]++];
                applyV(c, 1);
                G[-dep[c] + OFF + 1] = 1;
                query(c);
                stk.push_back(c);
            } else {
                if (v != 1) {
                    applyV(v, -1);
                    G[-dep[v] + OFF + 1] = 0;
                }
                stk.pop_back();
            }
        }
        out += to_string(ans % MOD) + "\n";
    }
    fputs(out.c_str(), stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

typedef unsigned int u32;
typedef unsigned long long u64;

static const u32 MOD = 998244353u;

static inline u32 mul(u32 a, u32 b) { return (u32)((u64)a * b % MOD); }

static u32 pw(u32 a, u64 e) {
    u32 r = 1;
    while (e) {
        if (e & 1) r = mul(r, a);
        a = mul(a, a);
        e >>= 1;
    }
    return r;
}

struct Group {
    int j;                     // level at which this group's factor is finalized
    int q;                     // large prime (0 = no large prime)
    u32 K;                     // product of (1+a_i) over elements with empty mask
    vector<pair<int, u32>> ms; // (nonempty mask, product of (1+a_i))
};

static const int MAXV = 6000;
static const int MAXM = 1 << 21;

static u32 arr_[MAXM];
static u32 tmp_[MAXM];
static u32 wt_[MAXM];

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    static int cnt[MAXV + 1];
    for (int i = 0; i < n; i++) {
        int x;
        if (scanf("%d", &x) != 1) break;
        if (x >= 1 && x <= MAXV) cnt[x]++;
    }

    // ---- sieve primes up to 6000 ----
    static bool isp[MAXV + 1];
    for (int i = 0; i <= MAXV; i++) isp[i] = true;
    isp[0] = isp[1] = false;
    vector<int> primes;
    for (int i = 2; i <= MAXV; i++)
        if (isp[i]) {
            primes.push_back(i);
            for (long long j = 1LL * i * i; j <= MAXV; j += i) isp[(int)j] = false;
        }
    vector<int> smallp;
    for (int p : primes)
        if (p <= 73) smallp.push_back(p);
    int ns = (int)smallp.size(); // 21

    // ---- factorize present values ----
    static int vmask[MAXV + 1], vlarge[MAXV + 1];
    bool rel[32] = {false};
    for (int v = 1; v <= MAXV; v++) {
        if (!cnt[v]) continue;
        int x = v, m = 0;
        for (int i = 0; i < ns; i++) {
            int p = smallp[i];
            if (x % p == 0) {
                m |= 1 << i;
                while (x % p == 0) x /= p;
            }
        }
        vmask[v] = m;
        vlarge[v] = (x > 1 ? x : 0); // remaining factor is 1 or a prime > 73
        for (int i = 0; i < ns; i++)
            if (m >> i & 1) rel[i] = true;
    }
    int rk[32];
    int k = 0;
    for (int i = 0; i < ns; i++) rk[i] = rel[i] ? k++ : -1;

    // ---- build groups ----
    static int gidx[MAXV + 1];
    for (int i = 0; i <= MAXV; i++) gidx[i] = -1;
    vector<Group> gs;
    for (int v = 1; v <= MAXV; v++) {
        if (!cnt[v]) continue;
        int q = vlarge[v];
        if (gidx[q] < 0) {
            gidx[q] = (int)gs.size();
            gs.push_back(Group{0, q, 1u, {}});
        }
    }
    for (int v = 1; v <= MAXV; v++) {
        if (!cnt[v]) continue;
        int q = vlarge[v];
        Group &g = gs[gidx[q]];
        int r = 0;
        for (int i = 0; i < ns; i++)
            if (vmask[v] >> i & 1) r |= 1 << rk[i];
        u32 f = pw((u32)(1 + v), (u64)cnt[v]);
        if (r == 0)
            g.K = mul(g.K, f);
        else
            g.ms.push_back({r, f});
    }

    // aggregate equal masks inside each group
    for (auto &g : gs) {
        sort(g.ms.begin(), g.ms.end());
        vector<pair<int, u32>> t;
        t.reserve(g.ms.size());
        for (size_t i = 0; i < g.ms.size(); i++) {
            if (!t.empty() && t.back().first == g.ms[i].first)
                t.back().second = mul(t.back().second, g.ms[i].second);
            else
                t.push_back(g.ms[i]);
        }
        g.ms.swap(t);
        int j = 0;
        for (auto &pr : g.ms) j = max(j, 32 - __builtin_clz((unsigned)pr.first));
        g.j = j;
    }

    // ---- constant factor from groups whose factor does not depend on the mask ----
    u32 constFactor = 1;
    vector<vector<int>> byLevel(k + 1);
    for (int i = 0; i < (int)gs.size(); i++) {
        Group &g = gs[i];
        if (g.j == 0) {
            if (g.q == 0) {
                constFactor = mul(constFactor, g.K);
            } else {
                u32 iq = pw((u32)g.q, MOD - 2);
                u32 v = (u32)((1u + (u64)(g.q - 1) * g.K) % MOD);
                constFactor = mul(constFactor, mul(iq, v));
            }
        } else {
            byLevel[g.j].push_back(i);
        }
    }

    // ---- level DP over relevant small primes ----
    int sz = 1;
    arr_[0] = 1;
    for (int lev = 1; lev <= k; lev++) {
        for (int m = 0; m < sz; m++) arr_[m + sz] = arr_[m];
        sz <<= 1;
        for (int gi : byLevel[lev]) {
            Group &g = gs[gi];
            // decide method: zeta transform vs. subcube multiplication
            u64 costA = (u64)lev * (u64)(sz >> 1); // multiplications of the zeta pass
            u64 costC = (u64)sz;                   // singleton part (lowbit recurrence)
            for (auto &pr : g.ms) {
                int pc = __builtin_popcount((unsigned)pr.first);
                if (pc >= 2) costC += (u64)(sz >> pc);
            }
            if (costC < costA) {
                // --- subcube method ---
                u32 sing[21];
                for (int t = 0; t < lev; t++) sing[t] = 1;
                for (auto &pr : g.ms)
                    if (__builtin_popcount((unsigned)pr.first) == 1)
                        sing[__builtin_ctz((unsigned)pr.first)] = pr.second;
                tmp_[0] = 1;
                for (int m = 1; m < sz; m++) {
                    int lb = m & (-m);
                    tmp_[m] = mul(tmp_[m ^ lb], sing[__builtin_ctz((unsigned)lb)]);
                }
                int full = sz - 1;
                for (auto &pr : g.ms) {
                    int r = pr.first;
                    if (__builtin_popcount((unsigned)r) < 2) continue;
                    u32 val = pr.second;
                    int comp = full ^ r;
                    for (int x = comp;; x = (x - 1) & comp) {
                        tmp_[r | x] = mul(tmp_[r | x], val);
                        if (x == 0) break;
                    }
                }
            } else {
                // --- zeta (subset product) method ---
                for (int m = 0; m < sz; m++) tmp_[m] = 1;
                for (auto &pr : g.ms) tmp_[pr.first] = pr.second;
                for (int t = 0; t < lev; t++) {
                    int bit = 1 << t, step = bit << 1;
                    for (int base = 0; base < sz; base += step) {
                        u32 *hi = tmp_ + base + bit;
                        u32 *lo = tmp_ + base;
                        for (int m = 0; m < bit; m++) hi[m] = mul(hi[m], lo[m]);
                    }
                }
            }
            // combine into accumulator
            u32 c0, c1;
            if (g.q == 0) {
                c0 = 0;
                c1 = g.K;
            } else {
                u32 iq = pw((u32)g.q, MOD - 2);
                c0 = iq;
                c1 = mul(iq, mul((u32)(g.q - 1), g.K));
            }
            if (c0 == 0) {
                for (int m = 0; m < sz; m++) arr_[m] = mul(arr_[m], mul(c1, tmp_[m]));
            } else {
                for (int m = 0; m < sz; m++)
                    arr_[m] = mul(arr_[m], (u32)((c0 + (u64)c1 * tmp_[m]) % MOD));
            }
        }
    }

    // ---- weights w(M) = prod_{p in M}(1-1/p) prod_{p not in M}(1/p) ----
    u32 Cw = 1;
    for (int i = 0; i < ns; i++)
        if (rk[i] >= 0) Cw = mul(Cw, pw((u32)smallp[i], MOD - 2));

    u32 bitVal[32];
    {
        int c = 0;
        for (int i = 0; i < ns; i++)
            if (rk[i] >= 0) bitVal[c++] = (u32)(smallp[i] - 1);
    }
    wt_[0] = Cw;
    u32 ans = mul(arr_[0], wt_[0]);
    for (int m = 1; m < sz; m++) {
        int lb = m & (-m);
        int t = __builtin_ctz((unsigned)lb);
        wt_[m] = mul(wt_[m ^ lb], bitVal[t]);
        ans = (ans + mul(arr_[m], wt_[m])) % MOD;
    }

    ans = mul(ans, constFactor);
    printf("%u\n", ans);
    return 0;
}

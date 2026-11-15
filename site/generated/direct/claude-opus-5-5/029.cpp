#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef long long ll;
static const u64 MOD = 1000000007ULL;

u64 pw(u64 b, u64 e) {
    u64 r = 1; b %= MOD;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}

int n, D, sz;
vector<uint32_t> tr;
vector<u64> fct, ifct;

inline void mergeNode(int v) {
    const uint32_t* A = &tr[(size_t)(2 * v) * D * 9];
    const uint32_t* B = &tr[(size_t)(2 * v + 1) * D * 9];
    uint32_t* C = &tr[(size_t)v * D * 9];
    for (int t = 0; t < D; t++, A += 9, B += 9, C += 9) {
        for (int a = 0; a < 3; a++) {
            u64 a0 = A[a * 3], a1 = A[a * 3 + 1], a2 = A[a * 3 + 2];
            for (int c = 0; c < 3; c++) {
                C[a * 3 + c] = (uint32_t)((a0 * B[c] + a1 * B[3 + c] + a2 * B[6 + c]) % MOD);
            }
        }
    }
}

void setLeaf(int pos, ll l, ll r, ll x) {
    int node = sz + pos;
    ll len = r - l + 1;
    int region; // 0: A, 1: B, 2: C
    if (x <= l) region = 0;
    else if (x <= r) region = 1;
    else region = 2;
    uint32_t* base = &tr[(size_t)node * D * 9];
    for (int t = 0; t < D; t++) {
        uint32_t* a = base + (size_t)t * 9;
        u64 p, q;
        if (region == 0) { p = len % MOD; q = 0; }
        else if (region == 2) { p = 0; q = len % MOD; }
        else {
            ll pv = (r + 1 - t) % (ll)MOD; if (pv < 0) pv += MOD;
            ll qv = (t - l) % (ll)MOD; if (qv < 0) qv += MOD;
            p = pv; q = qv;
        }
        for (int e = 0; e < 9; e++) a[e] = 0;
        a[1] = (uint32_t)q; a[5] = (uint32_t)q; a[6] = (uint32_t)p; a[8] = (uint32_t)q;
    }
}

u64 interp(const vector<u64>& G, int m, ll N) {
    if (N <= m) return G[N];
    vector<u64> pre(m + 2), suf(m + 2);
    pre[0] = 1;
    for (int i = 0; i <= m; i++) pre[i + 1] = pre[i] * (u64)((N - i) % (ll)MOD) % MOD;
    suf[m + 1] = 1;
    for (int i = m; i >= 0; i--) suf[i] = suf[i + 1] * (u64)((N - i) % (ll)MOD) % MOD;
    u64 res = 0;
    for (int i = 0; i <= m; i++) {
        u64 term = G[i] * pre[i] % MOD * suf[i + 1] % MOD * ifct[i] % MOD * ifct[m - i] % MOD;
        if ((m - i) & 1) res = (res + MOD - term) % MOD;
        else res = (res + term) % MOD;
    }
    return res;
}

int main() {
    int T;
    scanf("%d", &T);
    fct.assign(2200, 1); ifct.assign(2200, 1);
    for (int i = 1; i < 2200; i++) fct[i] = fct[i - 1] * i % MOD;
    ifct[2199] = pw(fct[2199], MOD - 2);
    for (int i = 2199; i > 0; i--) ifct[i - 1] = ifct[i] * i % MOD;
    while (T--) {
        scanf("%d", &n);
        vector<ll> l(n), r(n);
        for (auto& v : l) scanf("%lld", &v);
        for (auto& v : r) scanf("%lld", &v);
        ll maxR = 0;
        u64 total = 1;
        for (int i = 0; i < n; i++) {
            maxR = max(maxR, r[i]);
            total = total * ((r[i] - l[i] + 1) % MOD) % MOD;
        }
        D = n + 2;
        sz = 1; while (sz < n) sz <<= 1;
        tr.assign((size_t)2 * sz * D * 9, 0);
        // identity padding
        for (int i = n; i < sz; i++) {
            uint32_t* base = &tr[(size_t)(sz + i) * D * 9];
            for (int t = 0; t < D; t++) {
                base[(size_t)t * 9 + 0] = 1; base[(size_t)t * 9 + 4] = 1; base[(size_t)t * 9 + 8] = 1;
            }
        }
        for (int i = 0; i < n; i++) setLeaf(i, l[i], r[i], 1);
        for (int v = sz - 1; v >= 1; v--) mergeNode(v);

        vector<pair<ll, int>> ev;
        for (int i = 0; i < n; i++) {
            if (l[i] + 1 <= maxR) ev.push_back({l[i] + 1, i});
            if (r[i] + 1 <= maxR) ev.push_back({r[i] + 1, i});
        }
        sort(ev.begin(), ev.end());

        u64 ans = 0;
        int m = D - 1;
        vector<u64> G(D);
        auto finishSegment = [&](ll A, ll B) {
            if (A > B) return;
            const uint32_t* root = &tr[(size_t)1 * D * 9];
            u64 run = 0;
            for (int t = 0; t < D; t++) {
                const uint32_t* P = root + (size_t)t * 9;
                u64 f = ((u64)P[6] + P[7] + P[8]) % MOD;
                run = (run + f) % MOD;
                G[t] = run;
            }
            u64 gb = interp(G, m, B);
            u64 ga = interp(G, m, A - 1);
            u64 sumBad = (gb + MOD - ga) % MOD;
            u64 L = (u64)((B - A + 1) % (ll)MOD);
            u64 good = (total * L % MOD + MOD - sumBad) % MOD;
            ans = (ans + good) % MOD;
        };

        ll curStart = 1;
        size_t idx = 0;
        while (idx < ev.size()) {
            ll e = ev[idx].first;
            finishSegment(curStart, e - 1);
            vector<int> poss;
            while (idx < ev.size() && ev[idx].first == e) {
                if (poss.empty() || poss.back() != ev[idx].second) poss.push_back(ev[idx].second);
                idx++;
            }
            sort(poss.begin(), poss.end());
            poss.erase(unique(poss.begin(), poss.end()), poss.end());
            for (int p : poss) {
                setLeaf(p, l[p], r[p], e);
                int v = (sz + p) >> 1;
                while (v >= 1) { mergeNode(v); v >>= 1; }
            }
            curStart = e;
        }
        finishSegment(curStart, maxR);
        printf("%llu\n", ans % MOD);
    }
    return 0;
}

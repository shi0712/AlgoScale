#include <bits/stdc++.h>
using namespace std;

static const long long MOD = 998244353;

static long long pw(long long b, long long e) {
    b %= MOD; if (b < 0) b += MOD;
    long long r = 1;
    while (e > 0) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}

int main() {
    int k, n, m;
    if (scanf("%d %d %d", &k, &n, &m) != 3) return 0;
    vector<pair<long long,int>> gr(n);
    for (int i = 0; i < n; i++) { long long a; int b; scanf("%lld %d", &a, &b); gr[i] = {a, b}; }
    vector<pair<long long,int>> fin(m);
    for (int i = 0; i < m; i++) { long long c; int d; scanf("%lld %d", &c, &d); fin[i] = {c, d}; }

    sort(gr.begin(), gr.end());
    sort(fin.begin(), fin.end());

    vector<long long> A(n); vector<int> Bv(n);
    for (int i = 0; i < n; i++) { A[i] = gr[i].first; Bv[i] = gr[i].second; }
    vector<long long> C(m); vector<int> Dv(m);
    for (int i = 0; i < m; i++) { C[i] = fin[i].first; Dv[i] = fin[i].second; }

    auto getD = [&](long long v) -> long long {
        auto it = lower_bound(C.begin(), C.end(), v);
        if (it != C.end() && *it == v) return Dv[(size_t)(it - C.begin())];
        return 0;
    };
    auto inA = [&](long long v) -> bool {
        auto it = lower_bound(A.begin(), A.end(), v);
        return it != A.end() && *it == v;
    };

    // ---- determine X(a): number of players with initial rating a that win ----
    vector<long long> X(n, 0);
    bool ok = true;
    for (int parity = 0; parity < 2 && ok; parity++) {
        vector<int> idx;
        idx.reserve(n);
        for (int i = 0; i < n; i++) {
            long long p = ((A[i] % 2) + 2) % 2;
            if (p == parity) idx.push_back(i);
        }
        int sz = (int)idx.size();
        for (int j = 0; j < sz; j++) {
            int i = idx[j];
            if (j == 0 || A[i] - A[idx[j-1]] != 2) {
                // start of a new chain: check top equation of the previous chain
                if (j > 0) {
                    int li = idx[j-1];
                    if (X[li] != getD(A[li] + 1)) { ok = false; break; }
                }
                X[i] = (long long)Bv[i] - getD(A[i] - 1);   // bottom equation
            } else {
                int pi = idx[j-1];
                X[i] = X[pi] - getD(A[pi] + 1) + (long long)Bv[i];  // interior equation
            }
        }
        if (ok && sz > 0) {
            int li = idx[sz-1];
            if (X[li] != getD(A[li] + 1)) ok = false;   // top equation
        }
    }

    if (ok) {
        for (int i = 0; i < n; i++) if (X[i] < 0 || X[i] > Bv[i]) { ok = false; break; }
    }
    // every final value must be produced by some player (a_i +/- 1)
    if (ok) {
        for (int j = 0; j < m; j++) if (!inA(C[j] - 1) && !inA(C[j] + 1)) { ok = false; break; }
    }
    if (!ok) { printf("0\n"); return 0; }

    // ---- count pairings ----
    // total wins must equal k
    long long sumX = 0;
    for (int i = 0; i < n; i++) sumX += X[i];
    if (sumX != (long long)k) { printf("0\n"); return 0; }

    long long maxIdx = 2LL * k;
    vector<int> fact((size_t)maxIdx + 1), invfact((size_t)maxIdx + 1);
    fact[0] = 1;
    for (long long i = 1; i <= maxIdx; i++) fact[(size_t)i] = (long long)fact[(size_t)(i-1)] * i % MOD;
    invfact[(size_t)maxIdx] = (int)pw(fact[(size_t)maxIdx], MOD - 2);
    for (long long i = maxIdx; i >= 1; i--) invfact[(size_t)(i-1)] = (long long)invfact[(size_t)i] * i % MOD;

    const long long inv2 = (MOD + 1) / 2;

    long long ans = 1, P = 0;   // P = players from lower-rated groups still awaiting a higher-rated partner
    for (int i = 0; i < n; i++) {
        long long bb = Bv[i], xx = X[i];
        long long umin = max(0LL, 2 * xx - bb);          // number of players sent up to higher groups
        long long umax = min(xx, P - bb + 2 * xx);       // t = u + bb - 2xx must be <= P
        if (umax < umin) { ans = 0; break; }

        long long s = xx - umin;                 // intra-group pairs
        long long t = umin + bb - 2 * xx;        // players matched with lower groups
        long long invpow2 = pw(inv2, s);         // 2^{-s}
        long long sum = 0;
        long long common = (long long)fact[(size_t)bb] * fact[(size_t)P] % MOD;
        for (long long u = umin; u <= umax; u++, s--, t++) {
            long long term = common * invfact[(size_t)t] % MOD;
            term = term * invfact[(size_t)u] % MOD;
            term = term * invfact[(size_t)s] % MOD;
            term = term * invfact[(size_t)(P - t)] % MOD;
            term = term * invpow2 % MOD;
            sum += term;
            if (sum >= MOD) sum -= MOD;
            invpow2 = invpow2 * 2 % MOD;
        }
        ans = ans * sum % MOD;
        P = P - bb + 2 * xx;
        if (P < 0) { ans = 0; break; }
    }
    if (P != 0) ans = 0;
    printf("%lld\n", ans % MOD);
    return 0;
}

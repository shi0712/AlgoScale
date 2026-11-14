#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;
typedef uint32_t u32;
typedef uint64_t u64;
static const u32 MOD = 998244353;

// Montgomery arithmetic, R = 2^32
static constexpr u32 calcNp() {  // -MOD^{-1} mod 2^32
    u32 x = MOD;
    for (int i = 0; i < 5; i++) x *= 2u - MOD * x;
    return (u32)(0u - x);
}
static const u32 NP = calcNp();
static inline u32 mred(u64 t) {
    u32 m = (u32)t * NP;
    u32 r = (u32)((t + (u64)m * MOD) >> 32);
    return r >= MOD ? r - MOD : r;
}
static inline u32 mmul(u32 a, u32 b) { return mred((u64)a * b); }
static u64 pw(u64 b, u64 e) {
    u64 r = 1;
    b %= MOD;
    while (e) {
        if (e & 1) r = r * b % MOD;
        b = b * b % MOD;
        e >>= 1;
    }
    return r;
}
static inline u32 toM(u64 x) { return (u32)((x % MOD) * ((1ull << 32) % MOD) % MOD); }

static char ibuf[1 << 25];
size_t ipos = 0, ilen = 0;
static inline int gc() {
    if (ipos == ilen) {
        ilen = fread(ibuf, 1, sizeof(ibuf), stdin);
        ipos = 0;
        if (!ilen) return -1;
    }
    return ibuf[ipos++];
}
static inline long long readInt() {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9')) c = gc();
    long long x = 0;
    while (c >= '0' && c <= '9') {
        x = x * 10 + (c - '0');
        c = gc();
    }
    return x;
}

__attribute__((target_clones("avx2","default"))) static void mainLoop(int N, int W, vector<u32>& tau, const vector<u32>& invM, const vector<u32>& sM, vector<u32>& f) {
    vector<u32> Lam(W + 2, 0), Lnew(W + 2, 0), PhiPrev(W + 1, 0), PhiCur(W + 1, 0);
    for (int p = 0; p <= N - 1; p++) {
        u32 *fr = &f[(size_t)(p + 1) * (N + 1)];
        if (p == N - 1) {
            for (int k = 0; k <= N; k++) {
                const u32 *t = &tau[(size_t)k * W];
                u64 acc = 0;
                for (int s = 0; s <= k; s++) acc += mmul(t[s], sM[s]);
                fr[k] = (u32)(acc % MOD);
            }
            break;
        }
       
        fill(Lam.begin(), Lam.end(), 0);
        {
            u32 *t = &tau[0];
            u32 a = t[0];
            PhiPrev[1] = mmul(a, invM[1]);
            PhiPrev[0] = PhiPrev[1] ? MOD - PhiPrev[1] : 0;
           
            fr[0] = 0;
            t[0] = 0;
        }
        for (int k = 1; k <= N; k++) {
           
           
            Lam[k + 1] = 0;
            Lam[k + 2] = 0;
            {
               
                const u32 *__restrict lp = Lam.data();
                const u32 *__restrict php = PhiPrev.data();
                u32 *__restrict ln = Lnew.data();
                ln[0] = 0;
                for (int s = 0; s <= k; s++) {
                    u32 v = lp[s] + php[s];
                    v = v >= MOD ? v - MOD : v;
                    u32 cur = lp[s + 1];
                    ln[s + 1] = v >= cur ? v - cur : v + MOD - cur;
                }
            }
            swap(Lam, Lnew);
            u32 *t = &tau[(size_t)k * W];
            u64 fsum = 0, psum = 0;
            for (int s = 0; s <= k; s++) {
                u32 a = t[s];
                u32 ph = mmul(a, invM[s + 1]);
                PhiCur[s + 1] = ph;
                psum += ph;
                u32 b = mmul(a, sM[s]);
                fsum += b;
                u32 l = Lam[s + 1];
                t[s] = b >= l ? b - l : b + MOD - l;
            }
            u32 ps = (u32)(psum % MOD);
            PhiCur[0] = ps ? MOD - ps : 0;
            fr[k] = (u32)(fsum % MOD);
            swap(PhiPrev, PhiCur);
        }
    }
}
__attribute__((target_clones("avx2","default"))) static void horner(int T, int N, const vector<int>& qn, const vector<u32>& qm, const vector<int>& order, const vector<vector<u32>>& coef, vector<u32>& ans) {
    const int LN = 16;
    int idx = 0;
    while (idx < T) {
        int n = qn[order[idx]];
        int end = idx;
        while (end < T && qn[order[end]] == n) end++;
        const u32 *c = coef[n].data();
        for (int st = idx; st < end; st += LN) {
            int cntl = min(LN, end - st);
            u32 mm[LN], acc[LN];
            for (int l = 0; l < LN; l++) {
                mm[l] = l < cntl ? toM(qm[order[st + l]]) : 0;
                acc[l] = 0;
            }
            for (int i = n; i >= 0; i--) {
                u32 ci = c[i];
                for (int l = 0; l < LN; l++) {
                    u32 v = mmul(acc[l], mm[l]) + ci;
                    acc[l] = v >= MOD ? v - MOD : v;
                }
            }
            for (int l = 0; l < cntl; l++) ans[order[st + l]] = acc[l];
        }
        idx = end;
    }
}


int main() {
    int T = (int)readInt();
    vector<int> qn(T);
    vector<u32> qm(T);
    int N = 1;
    for (int i = 0; i < T; i++) {
        qn[i] = (int)readInt();
        qm[i] = (u32)(readInt() % MOD);
        N = max(N, qn[i]);
    }
    const int W = N + 3;
    // tau[k][s], s = 0..k (normal form)
    vector<u32> tau((size_t)(N + 1) * W, 0);
    {
        // binomials
        vector<vector<u32>> C(N + 1);
        for (int k = 0; k <= N; k++) {
            C[k].assign(k + 1, 1);
            for (int s = 1; s < k; s++) C[k][s] = (C[k - 1][s - 1] + C[k - 1][s]) % MOD;
        }
        for (int k = 0; k <= N; k++)
            for (int s = 0; s <= k; s++) {
                u32 v = C[k][s];
                if ((k - s) & 1) v = v ? MOD - v : 0;
                tau[(size_t)k * W + s] = v;
            }
    }
    vector<u32> invM(W + 1), sM(W + 1);
    for (int s = 0; s <= W; s++) {
        sM[s] = toM(s);
        invM[s] = s ? toM(pw(s, MOD - 2)) : 0;
    }
    // f[n][k]
    vector<u32> f((size_t)(N + 1) * (N + 1), 0);
    f[0] = 1;
    mainLoop(N, W, tau, invM, sM, f);
    // Monomial coefficients: B[k][i] = [m^i] C(m,k)
    // Only for needed n
    vector<char> need(N + 1, 0);
    for (int i = 0; i < T; i++) need[qn[i]] = 1;
    vector<u32> B((size_t)(N + 1) * (N + 1), 0);  // B[k][i]
    {
        // falling factorial m(m-1)...(m-k+1) coefficients, divide by k!
        vector<u64> ff(N + 2, 0);
        ff[0] = 1;
        u64 fact = 1;
        for (int k = 0; k <= N; k++) {
            if (k > 0) {
                // multiply by (m - (k-1))
                u64 c = (k - 1) % MOD;
                for (int i = k; i >= 1; i--) ff[i] = (ff[i - 1] + (MOD - c) * ff[i]) % MOD;
                ff[0] = (MOD - c) * ff[0] % MOD;
                fact = fact * k % MOD;
            }
            u64 ifact = pw(fact, MOD - 2);
            for (int i = 0; i <= k; i++) B[(size_t)k * (N + 1) + i] = toM(ff[i] * ifact % MOD);
        }
    }
    vector<vector<u32>> coef(N + 1);
    for (int n = 0; n <= N; n++) {
        if (!need[n]) continue;
        vector<u64> acc(n + 1, 0);
        const u32 *fr = &f[(size_t)n * (N + 1)];
        for (int k = 0; k <= n; k++) {
            u32 fk = fr[k];
            if (!fk) continue;
            const u32 *b = &B[(size_t)k * (N + 1)];
            for (int i = 0; i <= k; i++) acc[i] += mmul(fk, b[i]);
            if ((k & 15) == 15)
                for (int i = 0; i <= n; i++) acc[i] %= MOD;
        }
        coef[n].resize(n + 1);
        for (int i = 0; i <= n; i++) coef[n][i] = (u32)(acc[i] % MOD);
    }
    // Answer queries grouped by n, Horner with 16 lanes
    vector<int> order(T);
    {
        vector<int> cnt(N + 2, 0);
        for (int i = 0; i < T; i++) cnt[qn[i] + 1]++;
        for (int i = 1; i <= N + 1; i++) cnt[i] += cnt[i - 1];
        for (int i = 0; i < T; i++) order[cnt[qn[i]]++] = i;
    }
    vector<u32> ans(T);
    horner(T, N, qn, qm, order, coef, ans);
    // output
    string out;
    out.reserve((size_t)T * 11);
    char buf[16];
    for (int i = 0; i < T; i++) {
        u32 v = ans[i];
        int L = 0;
        if (v == 0) buf[L++] = '0';
        while (v) {
            buf[L++] = '0' + v % 10;
            v /= 10;
        }
        while (L) out.push_back(buf[--L]);
        out.push_back('\n');
    }
    fwrite(out.data(), 1, out.size(), stdout);
}

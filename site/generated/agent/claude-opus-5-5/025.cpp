#pragma GCC optimize("O3")
#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef unsigned int u32;
typedef long long ll;

static u64 P;
static unsigned __int128 BAR; // floor(2^64 / P) approx for fast reduction

static inline u64 red(u64 x) { // x mod P
    u64 q = (u64)(((unsigned __int128)x * (u64)BAR) >> 64);
    u64 r = x - q * P;
    r = min(r, r - P);
    r = min(r, r - P);
    return r;
}
static inline u64 mul(u64 a, u64 b) { return red(a * b); }
static u64 pw(u64 a, u64 e) {
    u64 r = 1;
    a %= P;
    while (e) {
        if (e & 1) r = mul(r, a);
        a = mul(a, a);
        e >>= 1;
    }
    return r;
}
static inline u64 inv(u64 a) { return pw(a, P - 2); }

static u64 C32;  // 2^32 mod P
// fold: x < 2^64 -> result < 2^60 + 2^33 (congruent mod P)
#define FOLD2(x) do { x = (x >> 32) * C32 + (x & 0xffffffffULL); x = (x >> 32) * C32 + (x & 0xffffffffULL); } while (0)

// acc[a] = sum_c H[c*n+a]*z[c]  (partially reduced, < 2^61)
#define MATVEC_BODY                                                   \
    for (int a = 0; a < n; a++) acc[a] = 0;                           \
    for (int b = 0; b < n; b += 16) {                                 \
        int e = b + 16 < n ? b + 16 : n;                              \
        for (int c = b; c < e; c++) {                                 \
            const u32* h = H + (size_t)c * n;                         \
            u64 zc = z[c];                                            \
            for (int a = 0; a < n; a++) acc[a] += (u64)h[a] * zc;     \
        }                                                             \
        u64 c32 = C32;                                                \
        for (int a = 0; a < n; a++) {                                 \
            u64 x = acc[a];                                           \
            x = (x >> 32) * c32 + (x & 0xffffffffULL);                \
            x = (x >> 32) * c32 + (x & 0xffffffffULL);                \
            acc[a] = x;                                               \
        }                                                             \
    }

#define DOT_BODY                                                      \
    u64 tot = 0;                                                      \
    u64 c32 = C32;                                                    \
    for (int k = 0; k < len; k += 16) {                               \
        int e = k + 16 < len ? k + 16 : len;                          \
        u64 s = 0;                                                    \
        for (int c = k; c < e; c++) s += (u64)x[c] * y[c];            \
        tot += s;                                                     \
        tot = (tot >> 32) * c32 + (tot & 0xffffffffULL);              \
        tot = (tot >> 32) * c32 + (tot & 0xffffffffULL);              \
    }                                                                 \
    return tot;

__attribute__((target("avx2"))) static void matvec_avx2(const u32* __restrict H, const u32* __restrict z, u64* __restrict acc, int n) { MATVEC_BODY }
static void matvec_gen(const u32* __restrict H, const u32* __restrict z, u64* __restrict acc, int n) { MATVEC_BODY }
__attribute__((target("avx2"))) static u64 dot_avx2(const u32* __restrict x, const u32* __restrict y, int len) { DOT_BODY }
static u64 dot_gen(const u32* __restrict x, const u32* __restrict y, int len) { DOT_BODY }
static bool HAS_AVX2 = false;
static inline void matvec(const u32* H, const u32* z, u64* acc, int n) {
    if (HAS_AVX2) matvec_avx2(H, z, acc, n); else matvec_gen(H, z, acc, n);
}
static inline u64 dot(const u32* x, const u32* y, int len) {
    return HAS_AVX2 ? dot_avx2(x, y, len) : dot_gen(x, y, len);
}

int main() {
    int n;
    ll pp;
    if (scanf("%d %lld", &n, &pp) != 2) return 0;
    P = pp;
    BAR = (unsigned __int128)(~0ULL) / P;
    C32 = (u64)((1ULL << 32) % P);
    __builtin_cpu_init();
    HAS_AVX2 = __builtin_cpu_supports("avx2");
    if (getenv("NOAVX")) HAS_AVX2 = false;
    int N = n * (n + 1) / 2;
    if (n == 1) {
        printf("1\n");
        return 0;
    }
    // rho_k = prod_{c != k} (k+c)/|k-c|
    vector<u64> rho(n + 1), dco(n + 1);
    for (int k = 1; k <= n; k++) {
        u64 num = 1, den = 1;
        for (int c = 1; c <= n; c++)
            if (c != k) {
                num = mul(num, k + c);
                den = mul(den, abs(k - c));
            }
        rho[k] = mul(num, inv(den));
        dco[k] = mul(rho[k], 2 * k);
    }
    // Hilbert matrix H[a][b] = 1/(a+b), indices 0..n-1 represent 1..n
    vector<u64> invs(2 * n + 2);
    for (int t = 1; t <= 2 * n + 1; t++) invs[t] = inv(t);
    vector<u32> H(n * n);
    for (int a = 0; a < n; a++)
        for (int b = 0; b < n; b++) H[a * n + b] = (u32)invs[a + b + 2];

    int maxd = 0;
    vector<int> lo(n + 1), dl(n + 1);
    for (int l = 0; l <= n; l++) {
        lo[l] = l * (l + 1) / 2;
        dl[l] = l * (n - l);
        maxd = max(maxd, dl[l]);
    }
    int need = maxd + 1;          // number of interpolation points needed
    int m = (need + 1) / 2;       // number of evaluation u's
    int M = 2 * m;

    mt19937_64 rng(1234567);
    set<u64> used;
    used.insert(0);
    used.insert(1);
    used.insert(P - 1);
    vector<u64> us, uinvs;
    vector<vector<u64>> evals; // evals[k][l] = Z_l(u_k)

    vector<u32> z((n + 1) * n), y((n + 1) * n);
    vector<u64> Dv(n), Dinv(n);
    vector<u64> s(2 * n);
    vector<u64> acc(n), tot(n);
    while ((int)us.size() < m) {
        u64 u = rng() % P;
        if (used.count(u)) continue;
        u64 ui = inv(u);
        if (used.count(ui) || ui == u) continue;
        // D_a = dco[a] * u^a
        u64 up = 1;
        for (int a = 0; a < n; a++) {
            up = mul(up, u);
            Dv[a] = mul(dco[a + 1], up);
        }
        // batch inverses
        {
            vector<u64> pre(n + 1);
            pre[0] = 1;
            for (int a = 0; a < n; a++) pre[a + 1] = mul(pre[a], Dv[a]);
            u64 iv = inv(pre[n]);
            for (int a = n - 1; a >= 0; a--) {
                Dinv[a] = mul(iv, pre[a]);
                iv = mul(iv, Dv[a]);
            }
        }
        for (int a = 0; a < n; a++) z[a] = (u32)(rng() % P);
        // y_k = H z_{k-1} (reduced), z_k = D o y_k ; y_0 = Dinv o z_0
        for (int a = 0; a < n; a++) y[a] = (u32)mul(z[a], Dinv[a]);
        for (int k = 1; k <= n; k++) {
            const u32* zp = &z[(k - 1) * n];
            u32* zn = &z[k * n];
            u32* yn = &y[k * n];
            matvec(H.data(), zp, acc.data(), n);
            for (int a = 0; a < n; a++) {
                u64 t = red(acc[a]);
                yn[a] = (u32)t;
                zn[a] = (u32)mul(t, Dv[a]);
            }
        }
        // s_{2i} = z_i^T Dinv z_i = y_i . z_i, s_{2i+1} = z_i^T Dinv z_{i+1} = y_i . z_{i+1}
        for (int i = 0; i < n; i++) {
            s[2 * i] = red(dot(&y[i * n], &z[i * n], n));
            s[2 * i + 1] = red(dot(&y[i * n], &z[(i + 1) * n], n));
        }
        // Berlekamp-Massey
        vector<u64> C(1, 1), B(1, 1);
        int L = 0, mm = 1;
        u64 bbinv = 1;
        for (int i = 0; i < 2 * n; i++) {
            u64 d = s[i];
            for (int j = 1; j <= L; j++) d = red(d + C[j] * s[i - j]);
            if (d == 0) {
                mm++;
                continue;
            }
            u64 coef = mul(d, bbinv);
            if (2 * L <= i) {
                vector<u64> T = C;
                if (C.size() < B.size() + mm) C.resize(B.size() + mm, 0);
                for (size_t j = 0; j < B.size(); j++) C[j + mm] = red(C[j + mm] + P - mul(coef, B[j]));
                L = i + 1 - L;
                B.swap(T);
                bbinv = inv(d);
                mm = 1;
            } else {
                if (C.size() < B.size() + mm) C.resize(B.size() + mm, 0);
                for (size_t j = 0; j < B.size(); j++) C[j + mm] = red(C[j + mm] + P - mul(coef, B[j]));
                mm++;
            }
        }
        if (L != n) continue;  // degenerate, try another u
        C.resize(n + 1, 0);
        // connection polynomial C(x) = 1 + c1 x + ... + cn x^n ; s_i + sum c_j s_{i-j} = 0
        // minimal poly m(x) = x^n + c1 x^{n-1} + ... + cn = det(xI - L)
        // det(I + vL) = sum_l v^l e_l, e_l = (-1)^l * coef of x^{n-l} in m = (-1)^l c_l
        vector<u64> e(n + 1);
        for (int l = 0; l <= n; l++) e[l] = (l & 1) ? (C[l] ? P - C[l] : 0) : C[l];
        used.insert(u);
        used.insert(ui);
        us.push_back(u);
        uinvs.push_back(ui);
        evals.push_back(e);
    }
    // points and values
    vector<u64> X(M);
    vector<vector<u32>> val(n + 1, vector<u32>(M));
    for (int k = 0; k < m; k++) {
        u64 u = us[k], ui = uinvs[k];
        X[2 * k] = u;
        X[2 * k + 1] = ui;
        u64 uiN = pw(ui, N);
        for (int l = 0; l <= n; l++) {
            // P_l(x) = x^{-lo_l} Z_l(x)
            u64 z1 = evals[k][l];
            val[l][2 * k] = (u32)mul(z1, pw(ui, lo[l]));
            u64 z2 = mul(uiN, evals[k][n - l]);  // Z_l(1/u)
            val[l][2 * k + 1] = (u32)mul(z2, pw(u, lo[l]));
        }
    }
    // master polynomial
    vector<u64> Pm(M + 1, 0);
    Pm[0] = 1;
    for (int k = 0; k < M; k++) {
        // multiply by (x - X[k])
        u64 negx = P - X[k];
        for (int t = k + 1; t >= 1; t--) Pm[t] = red(Pm[t - 1] + Pm[t] * negx);
        Pm[0] = mul(Pm[0], negx);
    }
    // Vinv rows t=0..maxd (unnormalized Lagrange numerators), stored Vt[t*M + k];
    // the normalization 1/P'(x_k) is folded into the values.
    vector<u32> Vt((size_t)(maxd + 1) * M);
    {
        const int G = 8;
        vector<u64> qq((size_t)G * M);
        for (int k0 = 0; k0 < M; k0 += G) {
            int g = min(G, M - k0);
            u64 xs[G], cur[G], hv[G];
            for (int j = 0; j < g; j++) { xs[j] = X[k0 + j]; cur[j] = Pm[M]; hv[j] = 0; }
            for (int t = M - 1; t >= 0; t--) {
                // cur = q[t]
                for (int j = 0; j < g; j++) {
                    qq[(size_t)t * G + j] = cur[j];
                    hv[j] = red(hv[j] * xs[j] + cur[j]);
                    if (t) cur[j] = red(Pm[t] + cur[j] * xs[j]);
                }
            }
            for (int t = 0; t <= maxd; t++)
                for (int j = 0; j < g; j++) Vt[(size_t)t * M + k0 + j] = (u32)qq[(size_t)t * G + j];
            for (int j = 0; j < g; j++) {
                u64 wk = inv(hv[j]);
                for (int l = 0; l <= n; l++) val[l][k0 + j] = (u32)mul(val[l][k0 + j], wk);
            }
        }
    }
    // W[l][s]
    vector<vector<u64>> W(n + 1, vector<u64>(N + 1, 0));
    for (int l = 0; l <= n / 2; l++) {
        const u32* vl = val[l].data();
        for (int t = 0; t <= dl[l]; t++) {
            const u32* vr = &Vt[(size_t)t * M];
            u64 tot = dot(vr, vl, M);
            W[l][lo[l] + t] = red(tot);
        }
    }
    for (int l = n / 2 + 1; l <= n; l++)
        for (int s2 = 0; s2 <= N; s2++) W[l][s2] = W[n - l][N - s2];
    // base = prod 1/k! * prod_{a<b} (b-a)/(b+a)
    vector<u64> fact(N + 1);
    fact[0] = 1;
    for (int i = 1; i <= N; i++) fact[i] = mul(fact[i - 1], i);
    u64 base = 1;
    for (int k = 1; k <= n; k++) base = mul(base, inv(fact[k]));
    {
        u64 num = 1, den = 1;
        for (int a = 1; a <= n; a++)
            for (int b = a + 1; b <= n; b++) {
                num = mul(num, b - a);
                den = mul(den, b + a);
            }
        base = mul(base, mul(num, inv(den)));
    }
    u64 sf = 1;
    for (int k = 1; k < n; k++) sf = mul(sf, fact[k]);
    // R[j][l] = j!(N-j)! base W[l][j]; suffix sums over l
    // Cs[j][i] = sum_{l>=i} R[j][l]
    vector<vector<u64>> Cs(N + 1, vector<u64>(n + 2, 0));
    for (int j = 0; j <= N; j++) {
        u64 f = mul(mul(fact[j], fact[N - j]), mul(base, sf));
        for (int l = n; l >= 0; l--) Cs[j][l] = red(Cs[j][l + 1] + mul(f, W[l][j]));
    }
    string out;
    out.reserve((size_t)n * N * 6);
    char buf[24];
    for (int i = 1; i <= n; i++) {
        for (int j = 1; j <= N; j++) {
            u64 v = red(Cs[j][i] + P - Cs[j - 1][i]);
            int len = snprintf(buf, sizeof buf, "%llu", (unsigned long long)v);
            out.append(buf, len);
            out.push_back(j == N ? '\n' : ' ');
        }
    }
    fputs(out.c_str(), stdout);
    return 0;
}

// 重合括号弧 (Overlapping Parenthesis Arcs)
//
// For a prime modulus M (1e8..1e9) and n, count ordered pairs (s,t) of Dyck
// words of length 2n having exactly k common arcs, for every k = 0..n.
//
// Mathematical basis (identities verified against brute force / an exact
// rational reference for small n):
//
//   R(t)   = sum_h Cat(h)^2 t^h            (Cat = Catalan)
//   w(t)   = t R(t)
//   Y(t)   = w/(1-w)                       (Y(0)=0)
//   psi(t) = R(t) (1-w(t)) = R - t R^2
//   A(t)   = psi'(t) (1-w(t))^{2n}
//
// Then for 0 <= k <= n-1
//   ans_k = C(2n,k) * d_k / (n-k),     d_k = [t^{n-1}] A(t) Y(t)^k
// and ans_n = Cat(n).
//
// All d_k are obtained by a Graeffe / Bostan-Mori style power projection:
//   C(x) = sum_k d_k x^k = [t^N] A(t)/(1 - x Y(t)),  N = n-1,
// maintained as C(x) = [t^{N_j}] P_j(x,t)/Q_j(x,t) with Q_j(x,0)=1; each
// level halves the t-precision and doubles the x-degree.
// Bivariate polynomials are multiplied with Kronecker substitution (x -> t^W)
// and a 3-prime NTT + Garner CRT so that everything is exact modulo M.
#include <bits/stdc++.h>
#include <charconv>
using namespace std;

typedef unsigned int u32;
typedef unsigned long long u64;

static const u32 P1 = 998244353u;
static const u32 P2 = 1004535809u;
static const u32 P3 = 469762049u;
static const int MAXL = 21;      // ntt length up to 2^21

static u32 modinv32(u32 a, u32 m) { // m prime, a % m != 0
    long long aa = a, b = m, u = 1, v = 0;
    while (b) { long long t = aa / b; aa -= t * b; swap(aa, b); u -= t * v; swap(u, v); }
    u %= (long long)m; if (u < 0) u += m;
    return (u32)u;
}

template<u32 MOD, u32 G>
struct NTTImpl {
    vector<u32> fw, iw;
    int off[24];
    int maxn = 0;
    static u32 pw(u32 a, u64 b) { u32 r = 1; while (b) { if (b & 1) r = (u32)((u64)r * a % MOD); a = (u32)((u64)a * a % MOD); b >>= 1; } return r; }
    void init(int n) {
        if (n <= maxn) return;
        maxn = n;
        int tot = 0;
        for (int k = 1; (1 << k) <= n; k++) { off[k] = tot; tot += (1 << (k - 1)); }
        fw.assign(tot, 1); iw.assign(tot, 1);
        for (int k = 1; (1 << k) <= n; k++) {
            int len = 1 << k;
            u32 w = pw(G, (MOD - 1) / (u64)len), iwv = pw(w, MOD - 2);
            u32 cw = 1, ci = 1;
            int o = off[k];
            for (int j = 0; j < (len >> 1); j++) {
                fw[o + j] = cw; iw[o + j] = ci;
                cw = (u32)((u64)cw * w % MOD); ci = (u32)((u64)ci * iwv % MOD);
            }
        }
    }
    // forward DIF (output in bit reversed order)
    void fwd(u32* a, int n) const {
        for (int len = n; len >= 2; len >>= 1) {
            int h = len >> 1; int k = __builtin_ctz((unsigned)len);
            const u32* ws = fw.data() + off[k];
            for (int i = 0; i < n; i += len) {
                u32* p = a + i;
                for (int j = 0; j < h; j++) {
                    u32 u = p[j], v = p[j + h];
                    u32 s = u + v; if (s >= MOD) s -= MOD;
                    u32 d = u + MOD - v; if (d >= MOD) d -= MOD;
                    p[j] = s;
                    p[j + h] = (u32)((u64)d * ws[j] % MOD);
                }
            }
        }
    }
    // inverse DIT (input in bit reversed order), includes 1/n scaling
    void bwd(u32* a, int n) const {
        for (int len = 2; len <= n; len <<= 1) {
            int h = len >> 1; int k = __builtin_ctz((unsigned)len);
            const u32* ws = iw.data() + off[k];
            for (int i = 0; i < n; i += len) {
                u32* p = a + i;
                for (int j = 0; j < h; j++) {
                    u32 u = p[j], v = (u32)((u64)p[j + h] * ws[j] % MOD);
                    u32 s = u + v; if (s >= MOD) s -= MOD;
                    u32 d = u + MOD - v; if (d >= MOD) d -= MOD;
                    p[j] = s; p[j + h] = d;
                }
            }
        }
        u32 ni = pw((u32)n, MOD - 2);
        for (int i = 0; i < n; i++) a[i] = (u32)((u64)a[i] * ni % MOD);
    }
};

struct Engine {
    NTTImpl<P1, 3> t1;
    NTTImpl<P2, 3> t2;
    NTTImpl<P3, 3> t3;
    vector<u32> b1, b2, b3, c1, c2, c3;
    u32 M = 0, inv12 = 0, inv123 = 0, p1m = 0, p12m = 0;

    void setM(u32 m) {
        if (m == M) return;
        M = m;
        inv12 = modinv32(P1 % P2, P2);
        inv123 = modinv32((u32)((u64)(P1 % P3) * (P2 % P3) % P3), P3);
        p1m = P1 % M;
        p12m = (u32)((u64)(P1 % M) * (P2 % M) % M);
    }
    inline u32 gar(u32 x1, u32 x2, u32 x3) const {
        u32 a2 = (u32)((u64)((x2 + P2 - x1 % P2) % P2) * inv12 % P2);
        u32 a3 = (x3 + P3 - x1 % P3) % P3;
        u32 sub = (u32)((u64)(P1 % P3) * a2 % P3);
        a3 = (a3 + P3 - sub) % P3;
        a3 = (u32)((u64)a3 * inv123 % P3);
        u32 r = x1 % M;
        r = (u32)((r + (u64)p1m * a2) % M);
        r = (u32)((r + (u64)p12m * a3) % M);
        return r;
    }
    void ensure(int L) {
        if ((int)b1.size() < L) {
            b1.resize(L); b2.resize(L); b3.resize(L);
            c1.resize(L); c2.resize(L); c3.resize(L);
        }
        t1.init(L); t2.init(L); t3.init(L);
    }
    // C[0..outlen) = A*B mod M  (exact integer convolution reduced mod M)
    void conv(const vector<u32>& A, const vector<u32>& B, vector<u32>& C, int outlen) {
        int la = (int)A.size(), lb = (int)B.size();
        if ((u64)la * lb <= 4096) {
            C.assign(outlen, 0);
            for (int i = 0; i < la; i++) {
                u64 ai = A[i]; if (!ai) continue;
                if (i >= outlen) break;
                int lim = min(lb, outlen - i);
                for (int j = 0; j < lim; j++)
                    if (B[j]) C[i + j] = (u32)((C[i + j] + ai * B[j]) % M);
            }
            return;
        }
        int L = 1;
        while (L < la + lb - 1 || L < outlen) L <<= 1;
        ensure(L);
        for (int i = 0; i < L; i++) {
            u32 x = (i < la) ? A[i] : 0, y = (i < lb) ? B[i] : 0;
            b1[i] = x % P1; b2[i] = x % P2; b3[i] = x % P3;
            c1[i] = y % P1; c2[i] = y % P2; c3[i] = y % P3;
        }
        t1.fwd(b1.data(), L); t1.fwd(c1.data(), L);
        t2.fwd(b2.data(), L); t2.fwd(c2.data(), L);
        t3.fwd(b3.data(), L); t3.fwd(c3.data(), L);
        for (int i = 0; i < L; i++) {
            b1[i] = (u32)((u64)b1[i] * c1[i] % P1);
            b2[i] = (u32)((u64)b2[i] * c2[i] % P2);
            b3[i] = (u32)((u64)b3[i] * c3[i] % P3);
        }
        t1.bwd(b1.data(), L); t2.bwd(b2.data(), L); t3.bwd(b3.data(), L);
        C.assign(outlen, 0);
        for (int i = 0; i < outlen; i++) C[i] = gar(b1[i], b2[i], b3[i]);
    }
};

static Engine eng;

static vector<u32> mulN(const vector<u32>& a, const vector<u32>& b, int n) {
    vector<u32> A(a.begin(), a.begin() + min((size_t)n, a.size()));
    vector<u32> B(b.begin(), b.begin() + min((size_t)n, b.size()));
    vector<u32> C;
    eng.conv(A, B, C, n);
    return C;
}

static vector<u32> invN(const vector<u32>& a, int n) {
    vector<u32> b(1, modinv32(a[0] % eng.M, eng.M));
    int m = 1;
    while (m < n) {
        int m2 = min(2 * m, n);
        vector<u32> A(a.begin(), a.begin() + min((size_t)m2, a.size()));
        vector<u32> t;
        eng.conv(A, b, t, m2);
        for (auto& x : t) x = x ? (eng.M - x) : 0;
        t[0] = (u32)((t[0] + 2) % eng.M);
        vector<u32> nb;
        eng.conv(b, t, nb, m2);
        b.swap(nb);
        m = m2;
    }
    return b;
}

static vector<u32> powN(vector<u32> base, long long e, int n) {
    base.resize(n, 0);
    vector<u32> r(n, 0);
    r[0] = 1 % eng.M;
    while (e > 0) {
        if (e & 1) r = mulN(r, base, n);
        e >>= 1;
        if (e) base = mulN(base, base, n);
    }
    return r;
}

// d_k = [t^N] A(t) Y(t)^k for k = 0..N  (Y(0)=0)
static vector<u32> project(const vector<u32>& Ain, const vector<u32>& Yin, int N) {
    vector<u32> res(N + 1, 0);
    if (N == 0) { res[0] = Ain[0] % eng.M; return res; }
    const u32 M = eng.M;
    int W = 2 * N + 2;
    int dxp = 0, dxq = 1;
    vector<u32> P((size_t)(dxp + 1) * W, 0), Q((size_t)(dxq + 1) * W, 0);
    for (int t = 0; t <= N; t++) P[t] = Ain[t] % M;
    Q[0] = 1 % M;
    for (int t = 0; t <= N; t++) Q[(size_t)W + t] = Yin[t] ? M - Yin[t] : 0;

    vector<u32> s1, s2, s3, v1, v2, v3, tP, tQ;
    int Nj = N;
    while (Nj > 0) {
        int dxs = min(dxp + dxq, N), dxv = min(2 * dxq, N);
        int dxo = max(dxs, dxv);
        size_t need = (size_t)(dxo + 1) * W;
        int L = 1;
        while ((size_t)L < need) L <<= 1;
        vector<u32> S((size_t)(dxs + 1) * W, 0), V((size_t)(dxv + 1) * W, 0);

        if (need <= 2048) {                  // small: naive bivariate products
            // S = P * Q(-t)
            for (int x1 = 0; x1 <= dxp; x1++) {
                const u32* ar = &P[(size_t)x1 * W];
                for (int x2 = 0; x2 <= dxq; x2++) {
                    const u32* br = &Q[(size_t)x2 * W];
                    int xr = x1 + x2;
                    if (xr > dxs) break;
                    u32* rs = &S[(size_t)xr * W];
                    for (int u = 0; u <= Nj; u++) {
                        u64 av = ar[u];
                        if (!av) continue;
                        int lim = Nj - u;
                        for (int v = 0; v <= lim; v++) {
                            u32 bv = br[v];
                            if (!bv) continue;
                            if (v & 1) bv = M - bv;
                            rs[u + v] = (u32)((rs[u + v] + av * bv) % M);
                        }
                    }
                }
            }
            // V = Q * Q(-t)
            for (int x1 = 0; x1 <= dxq; x1++) {
                const u32* ar = &Q[(size_t)x1 * W];
                for (int x2 = 0; x2 <= dxq; x2++) {
                    const u32* br = &Q[(size_t)x2 * W];
                    int xr = x1 + x2;
                    if (xr > dxv) break;
                    u32* rv = &V[(size_t)xr * W];
                    for (int u = 0; u <= Nj; u++) {
                        u64 av = ar[u];
                        if (!av) continue;
                        int lim = Nj - u;
                        for (int v = 0; v <= lim; v++) {
                            u32 bv = br[v];
                            if (!bv) continue;
                            if (v & 1) bv = M - bv;
                            rv[u + v] = (u32)((rv[u + v] + av * bv) % M);
                        }
                    }
                }
            }
        } else {
            s1.assign(L, 0); s2.assign(L, 0); s3.assign(L, 0);
            v1.assign(L, 0); v2.assign(L, 0); v3.assign(L, 0);
            tP.assign(L, 0); tQ.assign(L, 0);
            auto doPrime = [&](auto& ntt, const u32 MOD, vector<u32>& S_, vector<u32>& V_) {
                for (int x = 0; x <= dxp; x++) {
                    const u32* src = &P[(size_t)x * W];
                    u32* dst = &tP[(size_t)x * W];
                    for (int t = 0; t <= Nj; t++) dst[t] = src[t] % MOD;
                }
                for (int x = 0; x <= dxq; x++) {
                    const u32* src = &Q[(size_t)x * W];
                    u32* dst = &tQ[(size_t)x * W];
                    for (int t = 0; t <= Nj; t++) dst[t] = src[t] % MOD;
                }
                // t -> -t : flip sign of odd flat indices (W is even)
                for (int i = 1; i < L; i += 2) if (tQ[i]) tQ[i] = MOD - tQ[i];
                ntt.fwd(tP.data(), L);
                ntt.fwd(tQ.data(), L);
                for (int i = 0; i < L; i++) {
                    u32 qv = tQ[i];
                    S_[i] = (u32)((u64)tP[i] * qv % MOD);
                    V_[i] = (u32)((u64)qv * qv % MOD);
                }
                ntt.bwd(S_.data(), L);
                ntt.bwd(V_.data(), L);
            };
            doPrime(eng.t1, P1, s1, v1);
            doPrime(eng.t2, P2, s2, v2);
            doPrime(eng.t3, P3, s3, v3);
            for (int x = 0; x <= dxs; x++) {
                size_t base = (size_t)x * W;
                for (int t = 0; t <= Nj; t++)
                    S[base + t] = eng.gar(s1[base + t], s2[base + t], s3[base + t]);
            }
            for (int x = 0; x <= dxv; x++) {
                size_t base = (size_t)x * W;
                for (int t = 0; t <= Nj; t += 2)
                    V[base + t] = eng.gar(v1[base + t], v2[base + t], v3[base + t]);
            }
        }
        int par = Nj & 1;
        int N2 = Nj >> 1;
        int Wn = 2 * N2 + 2;
        vector<u32> P2((size_t)(dxs + 1) * Wn, 0), Q2((size_t)(dxv + 1) * Wn, 0);
        for (int x = 0; x <= dxs; x++) {
            const u32* srow = &S[(size_t)x * W];
            u32* drow = &P2[(size_t)x * Wn];
            for (int s = par, k = 0; s <= Nj; s += 2, k++) drow[k] = srow[s];
        }
        for (int x = 0; x <= dxv; x++) {
            const u32* vrow = &V[(size_t)x * W];
            u32* drow = &Q2[(size_t)x * Wn];
            for (int s = 0, k = 0; s <= Nj; s += 2, k++) drow[k] = vrow[s];
        }
        P.swap(P2); Q.swap(Q2);
        dxp = dxs; dxq = dxv; Nj = N2; W = Wn;
    }
    for (int k = 0; k <= min(N, dxp); k++) res[k] = P[(size_t)k * W];
    return res;
}

int main() {
    static char ibuf[1 << 16];
    fread(ibuf, 1, sizeof(ibuf) - 1, stdin);
    char* ip = ibuf;
    auto readInt = [&]() -> long long {
        while (*ip && (*ip < '0' || *ip > '9')) ip++;
        long long x = 0;
        while (*ip >= '0' && *ip <= '9') { x = x * 10 + (*ip - '0'); ip++; }
        return x;
    };
    int T = (int)readInt();
    vector<int> ns(T);
    vector<u32> Ms(T);
    int maxn = 0;
    for (int i = 0; i < T; i++) { ns[i] = (int)readInt(); Ms[i] = (u32)readInt(); maxn = max(maxn, ns[i]); }
    {
        int want = (int)(4 * (size_t)(maxn + 4));
        int want2 = (int)(3 * (size_t)(2 * maxn + 2) + 8);
        int L = 1;
        while (L < want || L < want2) L <<= 1;
        if (L > (1 << MAXL)) L = 1 << MAXL;
        eng.ensure(L);
    }
    string out;
    out.reserve((size_t)maxn * 12 + T * 4);
    char tmp[16];
    for (int ti = 0; ti < T; ti++) {
        int n = ns[ti];
        u32 M = Ms[ti];
        eng.setM(M);
        int K = n + 2;
        vector<u32> inv(K + 2, 0);
        inv[1] = 1 % M;
        for (int i = 2; i <= K + 1; i++)
            inv[i] = (u32)((u64)(M - M / (u32)i) * inv[M % (u32)i] % M);
        // R[h] = Cat(h)^2
        vector<u32> R(K + 1, 0);
        {
            u32 c = 1 % M;
            R[0] = (u32)((u64)c * c % M);
            for (int h = 1; h <= K; h++) {
                c = (u32)((u64)c * (u32)((4LL * h - 2) % M) % M);
                c = (u32)((u64)c * inv[h + 1] % M);
                R[h] = (u32)((u64)c * c % M);
            }
        }
        vector<u32> Q1(K, 0), wv(K, 0);
        Q1[0] = 1 % M;
        for (int i = 1; i < K; i++) {
            wv[i] = R[i - 1];
            Q1[i] = R[i - 1] ? M - R[i - 1] : 0;
        }
        vector<u32> Q1i = invN(Q1, n);
        vector<u32> Y = mulN(wv, Q1i, n);
        vector<u32> psi = mulN(R, Q1, n + 1);
        vector<u32> dpsi(n, 0);
        for (int i = 0; i < n; i++) dpsi[i] = (u32)((u64)(i + 1) * psi[i + 1] % M);
        vector<u32> Q2n = powN(Q1, 2LL * n, n);
        vector<u32> A = mulN(dpsi, Q2n, n);
        vector<u32> dk = project(A, Y, n - 1);
        u32 binom = 1 % M;
        for (int k = 0; k < n; k++) {
            u32 val = (u32)((u64)binom * dk[k] % M * inv[n - k] % M);
            int len = (int)(to_chars(tmp, tmp + 16, val).ptr - tmp);
            out.append(tmp, len);
            out.push_back(' ');
            binom = (u32)((u64)binom * (u32)((2LL * n - k) % M) % M);
            binom = (u32)((u64)binom * inv[k + 1] % M);
        }
        u32 catn = (u32)((u64)binom * inv[n + 1] % M);
        int len = (int)(to_chars(tmp, tmp + 16, catn).ptr - tmp);
        out.append(tmp, len);
        out.push_back('\n');
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

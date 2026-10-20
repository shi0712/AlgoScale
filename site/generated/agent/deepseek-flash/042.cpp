#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using ull = unsigned long long;

static const int MOD = 998244353;
static const int RT = 3;

static inline int pw(int b, long long e) {
    int r = 1;
    while (e > 0) {
        if (e & 1) r = (int)((ll)r * b % MOD);
        b = (int)((ll)b * b % MOD);
        e >>= 1;
    }
    return r;
}

// ---------------- fast NTT (radix 2/4, ACL-style rate tables) ----------------
namespace nttimpl {
int root_[30], iroot_[30], rate2_[30], irate2_[30], rate3_[30], irate3_[30];

static void init() {
    int rank2 = __builtin_ctz(MOD - 1);
    root_[rank2] = pw(RT, (MOD - 1) >> rank2);
    iroot_[rank2] = pw(root_[rank2], MOD - 2);
    for (int i = rank2 - 1; i >= 0; i--) {
        root_[i] = (int)((ll)root_[i + 1] * root_[i + 1] % MOD);
        iroot_[i] = (int)((ll)iroot_[i + 1] * iroot_[i + 1] % MOD);
    }
    {
        ll prod = 1, iprod = 1;
        for (int i = 0; i <= rank2 - 2; i++) {
            rate2_[i] = (int)((ll)root_[i + 2] * prod % MOD);
            irate2_[i] = (int)((ll)iroot_[i + 2] * iprod % MOD);
            prod = prod * iroot_[i + 2] % MOD;
            iprod = iprod * root_[i + 2] % MOD;
        }
    }
    {
        ll prod = 1, iprod = 1;
        for (int i = 0; i <= rank2 - 3; i++) {
            rate3_[i] = (int)((ll)root_[i + 3] * prod % MOD);
            irate3_[i] = (int)((ll)iroot_[i + 3] * iprod % MOD);
            prod = prod * iroot_[i + 3] % MOD;
            iprod = iprod * root_[i + 3] % MOD;
        }
    }
}

static inline int ceil_pow2(int n) {
    int x = 0;
    while ((1u << x) < (unsigned)n) x++;
    return x;
}

void butterfly(vector<int>& a) {
    int n = (int)a.size();
    int h = ceil_pow2(n);
    int len = 0;
    while (len < h) {
        if (h - len == 1) {
            int p = 1 << (h - len - 1);
            int rot = 1;
            for (int s = 0; s < (1 << len); s++) {
                int offset = s << (h - len);
                for (int i = 0; i < p; i++) {
                    int l = a[i + offset];
                    int r = (int)((ll)a[i + offset + p] * rot % MOD);
                    int x = l + r; if (x >= MOD) x -= MOD;
                    int y = l - r; if (y < 0) y += MOD;
                    a[i + offset] = x;
                    a[i + offset + p] = y;
                }
                if (s + 1 != (1 << len)) rot = (int)((ll)rot * rate2_[__builtin_ctz(~(unsigned)s)] % MOD);
            }
            len++;
        } else {
            int p = 1 << (h - len - 2);
            int rot = 1, imag = root_[2];
            for (int s = 0; s < (1 << len); s++) {
                int rot2 = (int)((ll)rot * rot % MOD);
                int rot3 = (int)((ll)rot2 * rot % MOD);
                int offset = s << (h - len);
                for (int i = 0; i < p; i++) {
                    int a0 = a[i + offset];
                    int a1 = (int)((ll)a[i + offset + p] * rot % MOD);
                    int a2 = (int)((ll)a[i + offset + 2 * p] * rot2 % MOD);
                    int a3 = (int)((ll)a[i + offset + 3 * p] * rot3 % MOD);
                    int a1na3imag = (int)((ll)(a1 - a3 + MOD) % MOD * imag % MOD);
                    int na2 = a2 ? MOD - a2 : 0;
                    int x0 = a0 + a2; if (x0 >= MOD) x0 -= MOD;
                    int y0 = a1 + a3; if (y0 >= MOD) y0 -= MOD;
                    int x1 = x0 + y0; if (x1 >= MOD) x1 -= MOD;
                    int y1 = x0 - y0; if (y1 < 0) y1 += MOD;
                    int x2 = a0 + na2; if (x2 >= MOD) x2 -= MOD;
                    int y2 = a1na3imag;
                    int x3 = x2 + y2; if (x3 >= MOD) x3 -= MOD;
                    int y3 = x2 - y2; if (y3 < 0) y3 += MOD;
                    a[i + offset] = x1;
                    a[i + offset + p] = y1;
                    a[i + offset + 2 * p] = x3;
                    a[i + offset + 3 * p] = y3;
                }
                if (s + 1 != (1 << len)) rot = (int)((ll)rot * rate3_[__builtin_ctz(~(unsigned)s)] % MOD);
            }
            len += 2;
        }
    }
}

void butterfly_inv(vector<int>& a) {
    int n = (int)a.size();
    int h = ceil_pow2(n);
    int len = h;
    while (len) {
        if (len == 1) {
            int p = 1 << (h - len);
            int irot = 1;
            for (int s = 0; s < (1 << (len - 1)); s++) {
                int offset = s << (h - len + 1);
                for (int i = 0; i < p; i++) {
                    int l = a[i + offset];
                    int r = a[i + offset + p];
                    int x = l + r; if (x >= MOD) x -= MOD;
                    int y = l - r; if (y < 0) y += MOD;
                    a[i + offset] = x;
                    a[i + offset + p] = (int)((ll)y * irot % MOD);
                }
                if (s + 1 != (1 << (len - 1))) irot = (int)((ll)irot * irate2_[__builtin_ctz(~(unsigned)s)] % MOD);
            }
            len--;
        } else {
            int p = 1 << (h - len);
            int irot = 1, iimag = iroot_[2];
            for (int s = 0; s < (1 << (len - 2)); s++) {
                int irot2 = (int)((ll)irot * irot % MOD);
                int irot3 = (int)((ll)irot2 * irot % MOD);
                int offset = s << (h - len + 2);
                for (int i = 0; i < p; i++) {
                    int a0 = a[i + offset];
                    int a1 = a[i + offset + p];
                    int a2 = a[i + offset + 2 * p];
                    int a3 = a[i + offset + 3 * p];
                    int a2na3iimag = (int)((ll)(a2 - a3 + MOD) % MOD * iimag % MOD);
                    int x0 = a0 + a1; if (x0 >= MOD) x0 -= MOD;
                    int y0 = a0 - a1; if (y0 < 0) y0 += MOD;
                    int x1 = a2 + a3; if (x1 >= MOD) x1 -= MOD;
                    int y1 = a2na3iimag;
                    int x2 = x0 + x1; if (x2 >= MOD) x2 -= MOD;
                    int y2 = (int)((ll)(y0 + y1) % MOD * irot % MOD);
                    int x3 = x0 - x1; if (x3 < 0) x3 += MOD;
                    int y3 = (y0 - y1 + MOD) % MOD;
                    a[i + offset] = x2;
                    a[i + offset + p] = y2;
                    a[i + offset + 2 * p] = (int)((ll)x3 * irot2 % MOD);
                    a[i + offset + 3 * p] = (int)((ll)y3 * irot3 % MOD);
                }
                if (s + 1 != (1 << (len - 2))) irot = (int)((ll)irot * irate3_[__builtin_ctz(~(unsigned)s)] % MOD);
            }
            len -= 2;
        }
    }
    int n_inv = pw(n, MOD - 2);
    for (int& x : a) x = (int)((ll)x * n_inv % MOD);
}
}  // namespace nttimpl

static const int NAIVE_LIM = 20000;

static vector<int> multiply(const vector<int>& f, const vector<int>& g) {
    int n = (int)f.size(), m = (int)g.size();
    if (n == 0 || m == 0) return {};
    if (n == 1) {
        vector<int> r(m);
        int c = f[0];
        for (int j = 0; j < m; j++) r[j] = (int)((ll)c * g[j] % MOD);
        return r;
    }
    if (m == 1) {
        vector<int> r(n);
        int c = g[0];
        for (int j = 0; j < n; j++) r[j] = (int)((ll)c * f[j] % MOD);
        return r;
    }
    if ((ll)n * m <= (ll)NAIVE_LIM) {
        vector<int> r(n + m - 1, 0);
        for (int i = 0; i < n; i++) {
            if (!f[i]) continue;
            ll fi = f[i];
            for (int j = 0; j < m; j++) {
                r[i + j] = (int)((r[i + j] + fi * g[j]) % MOD);
            }
        }
        return r;
    }
    int sz = 1;
    while (sz < n + m - 1) sz <<= 1;
    vector<int> a(f.begin(), f.end()), b(g.begin(), g.end());
    a.resize(sz); b.resize(sz);
    nttimpl::butterfly(a);
    nttimpl::butterfly(b);
    for (int i = 0; i < sz; i++) a[i] = (int)((ll)a[i] * b[i] % MOD);
    nttimpl::butterfly_inv(a);
    a.resize(n + m - 1);
    return a;
}

int n;
ull K;
vector<ull> A, Bv;
vector<int> fact_, invfact_;

// convolve u with the invfact prefix, keeping first D+1 coefficients
static vector<int> convInvfact(const vector<int>& u, int D) {
    int nu = (int)u.size();
    if (nu == 0) return vector<int>(D + 1, 0);
    int kern = min(D, max(0, n)) + 1;  // invfact indices 0..min(D,n)
    if ((ll)nu * kern <= (ll)NAIVE_LIM) {
        vector<int> r(D + 1, 0);
        for (int i = 0; i < nu; i++) {
            if (!u[i]) continue;
            ll ui = u[i];
            int lim = min(D - i, n);
            for (int j = 0; j <= lim; j++) r[i + j] = (int)((r[i + j] + ui * invfact_[j]) % MOD);
        }
        return r;
    }
    vector<int> v(invfact_.begin(), invfact_.begin() + kern);
    vector<int> c = multiply(u, v);
    c.resize(D + 1);
    return c;
}

static inline int splitPointA(int l, int r, int bit) {
    ull pref = (A[l] >> (bit + 1)) << (bit + 1);
    ull thresh = pref | (1ULL << bit);
    return (int)(lower_bound(A.begin() + l, A.begin() + r, thresh) - A.begin());
}
static inline int splitPointB(int l, int r, int bit) {
    ull pref = (Bv[l] >> (bit + 1)) << (bit + 1);
    ull thresh = pref | (1ULL << bit);
    return (int)(lower_bound(Bv.begin() + l, Bv.begin() + r, thresh) - Bv.begin());
}

// X: rook numbers of a board on U x W, |U| = a, |W| = b.
// Return rook numbers of the board on U x (W + E) where |E| = e and all cells U x E are present.
static vector<int> addColsFree(const vector<int>& X, int a, int b, int e) {
    if (e == 0) return X;
    // new[k] = sum_i (e!/(e-i)!) * X[k-i] * (a-k+i)!/(a-k)!
    int D = min(a, b + e);
    int du = min((int)X.size() - 1, D);
    vector<int> u(du + 1);
    for (int p = 0; p <= du; p++) u[p] = (int)((ll)X[p] * fact_[a - p] % MOD);
    int ei = min(e, D);
    vector<int> kern(ei + 1);
    for (int i = 0; i <= ei; i++) kern[i] = (int)((ll)invfact_[i] * invfact_[e - i] % MOD);
    vector<int> c = multiply(u, kern);
    c.resize(D + 1);
    int fe = fact_[e];
    for (int k = 0; k <= D; k++) c[k] = (int)((ll)c[k] * fe % MOD * invfact_[a - k] % MOD);
    return c;
}

// Add f rows fully connected to all b columns.
static vector<int> addRowsFree(const vector<int>& X, int a, int b, int f) {
    if (f == 0) return X;
    // new[k] = sum_i (f!/(f-i)!) * X[k-i] * (b-k+i)!/(b-k)!
    int D = min(a + f, b);
    int du = min((int)X.size() - 1, D);
    vector<int> u(du + 1);
    for (int p = 0; p <= du; p++) u[p] = (int)((ll)X[p] * fact_[b - p] % MOD);
    int fi = min(f, D);
    vector<int> kern(fi + 1);
    for (int i = 0; i <= fi; i++) kern[i] = (int)((ll)invfact_[i] * invfact_[f - i] % MOD);
    vector<int> c = multiply(u, kern);
    c.resize(D + 1);
    int ff = fact_[f];
    for (int k = 0; k <= D; k++) c[k] = (int)((ll)c[k] * ff % MOD * invfact_[b - k] % MOD);
    return c;
}

// complement (inside a x b rectangle) of the bad board
static vector<int> complementTransform(const vector<int>& Bpol, int a, int b) {
    int D = min(a, b);
    int du = min((int)Bpol.size() - 1, D);
    vector<int> u(du + 1);
    for (int j = 0; j <= du; j++) {
        ll val = (ll)Bpol[j] * fact_[a - j] % MOD * fact_[b - j] % MOD;
        if (j & 1) val = MOD - val;
        u[j] = (int)(val % MOD);
    }
    vector<int> c = convInvfact(u, D);
    for (int k = 0; k <= D; k++) {
        c[k] = (int)((ll)c[k] * invfact_[a - k] % MOD * invfact_[b - k] % MOD);
    }
    return c;
}

static vector<int> completePoly(int a, int b) {
    int D = min(a, b);
    vector<int> res(D + 1);
    ll base = (ll)fact_[a] * fact_[b] % MOD;
    for (int k = 0; k <= D; k++) {
        res[k] = (int)(base * invfact_[a - k] % MOD * invfact_[b - k] % MOD * invfact_[k] % MOD);
    }
    return res;
}

// number of j in [lb, rb) with (va xor Bv[j]) >= K
static int countGoodB(ull va, int lb, int rb) {
    int cnt = 0, lo = lb, hi = rb;
    for (int t = 59; t >= 0 && lo < hi; t--) {
        ull pref = (Bv[lo] >> (t + 1)) << (t + 1);
        int mp = (int)(lower_bound(Bv.begin() + lo, Bv.begin() + hi, pref | (1ULL << t)) - Bv.begin());
        int vb = (int)((va >> t) & 1), kb = (int)((K >> t) & 1);
        if (kb == 0) {
            if (vb == 0) cnt += hi - mp; else cnt += mp - lo;
            if (vb == 0) hi = mp; else lo = mp;
        } else {
            if (vb == 0) lo = mp; else hi = mp;
        }
    }
    return cnt + (hi - lo);
}

// number of i in [la, ra) with (A[i] xor vb) >= K
static int countGoodA(ull vb, int la, int ra) {
    int cnt = 0, lo = la, hi = ra;
    for (int t = 59; t >= 0 && lo < hi; t--) {
        ull pref = (A[lo] >> (t + 1)) << (t + 1);
        int mp = (int)(lower_bound(A.begin() + lo, A.begin() + hi, pref | (1ULL << t)) - A.begin());
        int ab = (int)((vb >> t) & 1), kb = (int)((K >> t) & 1);
        if (kb == 0) {
            if (ab == 0) cnt += hi - mp; else cnt += mp - lo;
            if (ab == 0) hi = mp; else lo = mp;
        } else {
            if (ab == 0) lo = mp; else hi = mp;
        }
    }
    return cnt + (hi - lo);
}

static const vector<int> ONE = {1};

// wantG: rook numbers of good board (a xor b >= k) on U x W, else of bad board (a xor b < k).
static vector<int> solve(int la, int ra, int lb, int rb, int bit, bool wantG) {
    int a = ra - la, b = rb - lb;
    if (a == 0 || b == 0) return ONE;
    if (bit < 0) {
        if (!wantG) return ONE;
        return completePoly(a, b);
    }
    // Structural shortcut: if all rows share one value (or all columns do), the board is a
    // complete bipartite block between a rows and the good columns (or good rows and b columns).
    if ((ll)a * b >= 4096) {
        if (A[la] == A[ra - 1]) {
            int m = countGoodB(A[la], lb, rb);
            return wantG ? completePoly(a, m) : completePoly(a, b - m);
        }
        if (Bv[lb] == Bv[rb - 1]) {
            int m = countGoodA(Bv[lb], la, ra);
            return wantG ? completePoly(m, b) : completePoly(a - m, b);
        }
    }
    int ma = splitPointA(la, ra, bit);
    int mb = splitPointB(lb, rb, bit);
    int a0 = ma - la, a1 = ra - ma;
    int b0 = mb - lb, b1 = rb - mb;
    int ka = (int)((K >> bit) & 1);

    if (ka == 0) {
        if (!wantG) {
            // bad = bad(diag0) * bad(diag1)
            if (a0 == 0 || b0 == 0) {
                if (a1 == 0 || b1 == 0) return ONE;
                return solve(ma, ra, mb, rb, bit - 1, false);
            }
            if (a1 == 0 || b1 == 0) {
                return solve(la, ma, lb, mb, bit - 1, false);
            }
            vector<int> p0 = solve(la, ma, lb, mb, bit - 1, false);
            vector<int> p1 = solve(ma, ra, mb, rb, bit - 1, false);
            return multiply(p0, p1);
        } else {
            // good: join if a diagonal child is empty, else complement transform
            if (a1 == 0) {
                // U1 empty: good = G(diag0) + complete(U0 x W1)
                vector<int> x = (a0 == 0 || b0 == 0) ? ONE : solve(la, ma, lb, mb, bit - 1, true);
                return addColsFree(x, a0, b0, b1);
            }
            if (b1 == 0) {
                // W1 empty: good = G(diag0) + complete(U1 x W0)
                vector<int> x = (a0 == 0 || b0 == 0) ? ONE : solve(la, ma, lb, mb, bit - 1, true);
                return addRowsFree(x, a0, b0, a1);
            }
            if (a0 == 0) {
                // U0 empty: good = G(diag1) + complete(U1 x W0)
                vector<int> x = (a1 == 0 || b1 == 0) ? ONE : solve(ma, ra, mb, rb, bit - 1, true);
                return addColsFree(x, a1, b1, b0);
            }
            if (b0 == 0) {
                // W0 empty: good = G(diag1) + complete(U0 x W1)
                vector<int> x = (a1 == 0 || b1 == 0) ? ONE : solve(ma, ra, mb, rb, bit - 1, true);
                return addRowsFree(x, a1, b1, a0);
            }
            vector<int> p0 = solve(la, ma, lb, mb, bit - 1, false);
            vector<int> p1 = solve(ma, ra, mb, rb, bit - 1, false);
            vector<int> bp = multiply(p0, p1);
            return complementTransform(bp, a, b);
        }
    } else {
        if (wantG) {
            // good = good(anti0) * good(anti1); anti0=(U0,W1), anti1=(U1,W0)
            if (a0 == 0 || b1 == 0) {
                if (a1 == 0 || b0 == 0) return ONE;
                return solve(ma, ra, lb, mb, bit - 1, true);
            }
            if (a1 == 0 || b0 == 0) {
                return solve(la, ma, mb, rb, bit - 1, true);
            }
            vector<int> p0 = solve(la, ma, mb, rb, bit - 1, true);
            vector<int> p1 = solve(ma, ra, lb, mb, bit - 1, true);
            return multiply(p0, p1);
        } else {
            // bad = bad(anti0) * bad(anti1) [+ complete diagonal]; join if one anti child empty
            if (a1 == 0) {
                // U1 empty: bad = B(anti0) + complete(U0 x W0)
                vector<int> x = (a0 == 0 || b1 == 0) ? ONE : solve(la, ma, mb, rb, bit - 1, false);
                return addColsFree(x, a0, b1, b0);
            }
            if (b0 == 0) {
                // W0 empty: bad = B(anti0) + complete(U1 x W1)
                vector<int> x = (a0 == 0 || b1 == 0) ? ONE : solve(la, ma, mb, rb, bit - 1, false);
                return addRowsFree(x, a0, b1, a1);
            }
            if (a0 == 0) {
                // U0 empty: bad = B(anti1) + complete(U1 x W1)
                vector<int> x = (a1 == 0 || b0 == 0) ? ONE : solve(ma, ra, lb, mb, bit - 1, false);
                return addColsFree(x, a1, b0, b1);
            }
            if (b1 == 0) {
                // W1 empty: bad = B(anti1) + complete(U0 x W0)
                vector<int> x = (a1 == 0 || b0 == 0) ? ONE : solve(ma, ra, lb, mb, bit - 1, false);
                return addRowsFree(x, a1, b0, a0);
            }
            vector<int> p0 = solve(la, ma, mb, rb, bit - 1, true);
            vector<int> p1 = solve(ma, ra, lb, mb, bit - 1, true);
            vector<int> gp = multiply(p0, p1);
            return complementTransform(gp, a, b);
        }
    }
}

int main() {
    nttimpl::init();
    if (scanf("%d %llu", &n, &K) != 2) return 0;
    A.resize(n); Bv.resize(n);
    for (int i = 0; i < n; i++) {
        if (scanf("%llu", &A[i]) != 1) return 0;
    }
    for (int i = 0; i < n; i++) {
        if (scanf("%llu", &Bv[i]) != 1) return 0;
    }
    sort(A.begin(), A.end());
    sort(Bv.begin(), Bv.end());

    fact_.assign(n + 1, 1);
    invfact_.assign(n + 1, 1);
    for (int i = 1; i <= n; i++) fact_[i] = (int)((ll)fact_[i - 1] * i % MOD);
    invfact_[n] = pw(fact_[n], MOD - 2);
    for (int i = n; i >= 1; i--) invfact_[i - 1] = (int)((ll)invfact_[i] * i % MOD);

    vector<int> res = solve(0, n, 0, n, 59, true);

    string out;
    out.reserve((size_t)n * 12);
    char buf[16];
    for (int x = 1; x <= n; x++) {
        int v = (x < (int)res.size()) ? res[x] : 0;
        int len = snprintf(buf, sizeof(buf), "%d", v);
        out.append(buf, len);
        out.push_back(x == n ? '\n' : ' ');
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

static int MOD; // arbitrary prime

// ---------- NTT for three primes, arbitrary mod convolution ----------
struct NTT {
    static const int P1 = 998244353;
    static const int P2 = 1004535809;
    static const int P3 = 469762049;
    static const int G = 3;

    static int modpow(long long a, long long e, int mod) {
        long long r = 1;
        while (e) {
            if (e & 1) r = r * a % mod;
            a = a * a % mod;
            e >>= 1;
        }
        return (int)r;
    }

    static void ntt(vector<int>& a, bool invert, int mod, int root) {
        int n = (int)a.size();
        for (int i = 1, j = 0; i < n; ++i) {
            int bit = n >> 1;
            for (; j & bit; bit >>= 1) j ^= bit;
            j ^= bit;
            if (i < j) swap(a[i], a[j]);
        }
        for (int len = 2; len <= n; len <<= 1) {
            int wlen = modpow(root, (mod - 1) / len, mod);
            if (invert) wlen = modpow(wlen, mod - 2, mod);
            for (int i = 0; i < n; i += len) {
                long long w = 1;
                int half = len >> 1;
                for (int j = 0; j < half; ++j) {
                    int u = a[i + j];
                    int v = (int)(w * a[i + j + half] % mod);
                    int x = u + v; if (x >= mod) x -= mod;
                    int y = u - v; if (y < 0) y += mod;
                    a[i + j] = x;
                    a[i + j + half] = y;
                    w = w * wlen % mod;
                }
            }
        }
        if (invert) {
            int inv_n = modpow(n, mod - 2, mod);
            for (int &x : a) x = (int)((long long)x * inv_n % mod);
        }
    }

    static vector<int> convolution_mod(const vector<int>& a, const vector<int>& b, int p) {
        int n = (int)a.size(), m = (int)b.size();
        if (!n || !m) return {};
        int sz = 1;
        while (sz < n + m - 1) sz <<= 1;
        vector<int> fa(sz), fb(sz);
        for (int i = 0; i < n; ++i) fa[i] = a[i] % p;
        for (int i = 0; i < m; ++i) fb[i] = b[i] % p;
        ntt(fa, false, p, G);
        ntt(fb, false, p, G);
        for (int i = 0; i < sz; ++i) fa[i] = (int)((long long)fa[i] * fb[i] % p);
        ntt(fa, true, p, G);
        fa.resize(n + m - 1);
        return fa;
    }

    static vector<int> conv_naive(const vector<int>& a, const vector<int>& b) {
        int n = (int)a.size(), m = (int)b.size();
        if (!n || !m) return {};
        vector<int> c(n + m - 1, 0);
        if (min(n, m) <= 20) {
            for (int i = 0; i < n; ++i) {
                if (!a[i]) continue;
                long long ai = a[i];
                for (int j = 0; j < m; ++j) {
                    c[i + j] = (int)((c[i + j] + ai * b[j]) % MOD);
                }
            }
            return c;
        }
        // full arbitrary mod via three NTT primes
        vector<int> r1 = convolution_mod(a, b, P1);
        vector<int> r2 = convolution_mod(a, b, P2);
        vector<int> r3 = convolution_mod(a, b, P3);
        int len = (int)r1.size();
        c.resize(len);
        static const long long P1LL = P1, P2LL = P2, P3LL = P3;
        static const long long P12 = P1LL * P2LL;
        static const int invP1_modP2 = modpow(P1, P2 - 2, P2);
        static const int invP12_modP3 = modpow((long long)(P12 % P3), P3 - 2, P3);
        const long long p1m = P1LL % MOD;
        const long long p12m = P12 % MOD;
        for (int i = 0; i < len; ++i) {
            long long x1 = r1[i];
            long long x2 = r2[i];
            long long x3 = r3[i];
            long long t1 = (x2 - x1) % P2; if (t1 < 0) t1 += P2;
            t1 = t1 * invP1_modP2 % P2;
            long long cur = x1 + P1LL * t1; // < P1*P2
            long long cur_mod_p3 = cur % P3;
            long long t2 = (x3 - cur_mod_p3) % P3; if (t2 < 0) t2 += P3;
            t2 = t2 * invP12_modP3 % P3;
            long long res = x1 % MOD;
            res = (res + p1m * (t1 % MOD)) % MOD;
            res = (res + p12m * (t2 % MOD)) % MOD;
            c[i] = (int)res;
        }
        return c;
    }
};

// ---------- polynomial power / inverse ----------
static vector<int> poly_inv(const vector<int>& a, int n) {
    vector<int> r(1, (int)NTT::modpow(a[0], MOD - 2, MOD));
    int cur = 1;
    while (cur < n) {
        int m = min(n, cur << 1);
        vector<int> f(min((int)a.size(), m));
        for (int i = 0; i < (int)f.size(); ++i) f[i] = a[i];
        vector<int> rf = NTT::conv_naive(r, f);
        rf.resize(m, 0);
        vector<int> rr = NTT::conv_naive(r, rf);
        vector<int> nr(m, 0);
        for (int i = 0; i < m; ++i) {
            int v = (i < (int)r.size() ? 2LL * r[i] % MOD : 0);
            int sub = (i < (int)rr.size() ? rr[i] : 0);
            v -= sub; if (v < 0) v += MOD;
            nr[i] = v;
        }
        r.swap(nr);
        cur = m;
    }
    r.resize(n);
    return r;
}

static vector<int> poly_pow(vector<int> a, long long e, int n) {
    vector<int> res(1, 1);
    a.resize(min((int)a.size(), n));
    while (e > 0) {
        if (e & 1) {
            res = NTT::conv_naive(res, a);
            res.resize(n);
        }
        e >>= 1;
        if (e) {
            a = NTT::conv_naive(a, a);
            a.resize(n);
        }
    }
    res.resize(n);
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n;
        cin >> n >> MOD;
        if (n == 1) {
            cout << "0 1\n";
            continue;
        }

        // We need psi[1..n+1], where psi is the compositional inverse of z*G(z).
        // The differential equation used:
        // p(1-16p)p'' - (p')^2 + (4t+1)(p')^3 = 0, p(0)=0,p'(0)=1.
        const int maxLeaf = n;          // leaves n=2..maxLeaf compute p[n+1]
        const int maxP = n + 2;         // p[0..n+1]
        vector<int> p(maxP + 2, 0), u(maxP + 2, 0), tArr(maxP + 2, 0);
        vector<int> z(maxP + 2, 0), h(maxP + 2, 0);
        vector<int> a2(maxP + 2, 0), r(maxP + 2, 0);
        vector<int> cu(maxP + 2, 0), cz(maxP + 2, 0), ch(maxP + 2, 0), cw(maxP + 2, 0);

        p[0] = 0;
        p[1] = 1 % MOD;
        p[2] = (MOD - 1) % MOD; // -1
        tArr[0] = 0;
        tArr[1] = (int)(2LL * p[2] % MOD);
        // initial old-old (indices < 2) contributions for u = p^2 and z = t^2
        cu[2] = (int)(1LL * p[1] * p[1] % MOD);
        cz[2] = (int)(1LL * tArr[1] * tArr[1] % MOD);

        int blockSize = (int)(18.0 * sqrt((double)n));
        if (blockSize < 64) blockSize = 64;
        if (blockSize > n) blockSize = n;
        int L = 2;
        while (L <= maxLeaf) {
            int R = min(maxLeaf + 1, L + blockSize);

            for (int nn = L; nn < R; ++nn) {
                // u[nn] = [x^nn] p^2
                long long uv = cu[nn];
                for (int i = L; i <= nn - 1; ++i) {
                    int j = nn - i;
                    long long term = 1LL * p[i] * p[j] % MOD;
                    if (j < L) term = term * 2 % MOD;
                    uv += term;
                    if (uv >= (long long)MOD * MOD) uv %= MOD;
                }
                u[nn] = (int)(uv % MOD);

                // create operand index k = nn-2 for W2 (A_shift * p'')
                int k = nn - 2;
                if (k >= 0) {
                    a2[k] = (int)((p[nn] - 16LL * u[nn]) % MOD);
                    if (a2[k] < 0) a2[k] += MOD;
                    r[k] = (int)(1LL * nn * (nn - 1) % MOD * p[nn] % MOD);
                }

                // W2[k]
                long long wv;
                if (k < L) {
                    // delayed index from the previous block: add all its contributions
                    if (k >= 0) {
                        long long ak = a2[k], rk = r[k];
                        for (int j = 0; j <= k; ++j) {
                            long long val = ak * r[j] % MOD;
                            if (j < k) val = (val + 1LL * a2[j] * rk) % MOD;
                            int idx = k + j;
                            if (idx <= maxLeaf) {
                                int nv = cw[idx] + (int)val;
                                if (nv >= MOD) nv -= MOD;
                                cw[idx] = nv;
                            }
                        }
                    }
                    wv = cw[k];
                } else {
                    wv = cw[k];
                    for (int i = L; i <= k; ++i) {
                        wv += 1LL * a2[i] * r[k - i] % MOD;
                        if (wv >= (long long)MOD * MOD) wv %= MOD;
                    }
                    for (int j = L; j <= k; ++j) {
                        int i = k - j;
                        if (i < L) {
                            wv += 1LL * a2[i] * r[j] % MOD;
                            if (wv >= (long long)MOD * MOD) wv %= MOD;
                        }
                    }
                    wv %= MOD;
                }

                // z[nn] = [x^nn] t^2
                long long zv = cz[nn];
                for (int i = L; i <= nn - 1; ++i) {
                    int j = nn - i;
                    long long term = 1LL * tArr[i] * tArr[j] % MOD;
                    if (j < L) term = term * 2 % MOD;
                    zv += term;
                    if (zv >= (long long)MOD * MOD) zv %= MOD;
                }
                z[nn] = (int)(zv % MOD);

                // h[nn] = [x^nn] t^3 = t * z
                long long hv = ch[nn];
                for (int i = L; i <= nn - 1; ++i) {
                    int j = nn - i;
                    hv += 1LL * tArr[i] * z[j] % MOD;
                    if (hv >= (long long)MOD * MOD) hv %= MOD;
                }
                for (int j = L; j <= nn - 1; ++j) {
                    int i = nn - j;
                    if (i < L) {
                        hv += 1LL * tArr[i] * z[j] % MOD;
                        if (hv >= (long long)MOD * MOD) hv %= MOD;
                    }
                }
                h[nn] = (int)(hv % MOD);

                long long Rval = wv;
                Rval = (Rval + 2LL * z[nn]) % MOD;
                Rval += h[nn]; Rval %= MOD;
                Rval = (Rval + 12LL * tArr[nn - 1]) % MOD;
                Rval = (Rval + 12LL * z[nn - 1]) % MOD;
                Rval = (Rval + 4LL * h[nn - 1]) % MOD;

                long long inv = NTT::modpow((long long)(nn + 1) * (nn + 1) % MOD, MOD - 2, MOD);
                long long pv = (MOD - Rval) % MOD * inv % MOD;
                p[nn + 1] = (int)pv;
                tArr[nn] = (int)((long long)(nn + 1) * pv % MOD);
            }

            // -------- block-end updates --------
            int lenA = R - L;
            // u = p^2
            if (lenA > 0) {
                vector<int> A(lenA), B(R);
                for (int i = 0; i < lenA; ++i) A[i] = p[L + i];
                for (int j = 0; j < R; ++j) {
                    if (j < L) B[j] = (int)(2LL * p[j] % MOD);
                    else B[j] = p[j];
                }
                vector<int> C = NTT::conv_naive(A, B);
                for (int m = R; m <= maxLeaf; ++m) {
                    int idx = m - L;
                    if (idx < 0 || idx >= (int)C.size()) continue;
                    int nv = cu[m] + C[idx];
                    if (nv >= MOD) nv -= MOD;
                    cu[m] = nv;
                }
            }
            // z = t^2
            if (lenA > 0) {
                vector<int> A(lenA), B(R);
                for (int i = 0; i < lenA; ++i) A[i] = tArr[L + i];
                for (int j = 0; j < R; ++j) {
                    if (j < L) B[j] = (int)(2LL * tArr[j] % MOD);
                    else B[j] = tArr[j];
                }
                vector<int> C = NTT::conv_naive(A, B);
                for (int m = R; m <= maxLeaf; ++m) {
                    int idx = m - L;
                    if (idx < 0 || idx >= (int)C.size()) continue;
                    int nv = cz[m] + C[idx];
                    if (nv >= MOD) nv -= MOD;
                    cz[m] = nv;
                }
            }
            // h = t*z
            if (lenA > 0) {
                vector<int> At(lenA), Znew(lenA);
                for (int i = 0; i < lenA; ++i) At[i] = tArr[L + i];
                for (int i = 0; i < lenA; ++i) Znew[i] = z[L + i];
                vector<int> Zpref(R);
                for (int j = 0; j < R; ++j) Zpref[j] = z[j];
                vector<int> C1 = NTT::conv_naive(At, Zpref);
                vector<int> Tpref(L);
                for (int j = 0; j < L; ++j) Tpref[j] = tArr[j];
                vector<int> C2 = NTT::conv_naive(Tpref, Znew);
                for (int m = R; m <= maxLeaf; ++m) {
                    int idx = m - L;
                    long long add = 0;
                    if (idx >= 0 && idx < (int)C1.size()) add += C1[idx];
                    if (idx >= 0 && idx < (int)C2.size()) add += C2[idx];
                    int nv = (ch[m] + add) % MOD;
                    ch[m] = nv;
                }
            }
            // W2 = A_shift * r, operands up to R-3 (exclusive R-2)
            int Send = R - 2; // indices [L, Send)
            if (Send > L) {
                int len2 = Send - L;
                vector<int> Anew(len2), Rnew(len2);
                for (int i = 0; i < len2; ++i) {
                    Anew[i] = a2[L + i];
                    Rnew[i] = r[L + i];
                }
                vector<int> Rpref(Send);
                for (int j = 0; j < Send; ++j) Rpref[j] = r[j];
                vector<int> C1 = NTT::conv_naive(Anew, Rpref);
                vector<int> Apref(L);
                for (int j = 0; j < L; ++j) Apref[j] = a2[j];
                vector<int> C2 = NTT::conv_naive(Apref, Rnew);
                int start = R - 2;
                for (int m = start; m <= maxLeaf; ++m) {
                    int idx = m - L;
                    long long add = 0;
                    if (idx >= 0 && idx < (int)C1.size()) add += C1[idx];
                    if (idx >= 0 && idx < (int)C2.size()) add += C2[idx];
                    int nv = (cw[m] + add) % MOD;
                    cw[m] = nv;
                }
            }

            L = R;
        }

        // psi[0..n+1] known. U = psi / x, H = 1/U.
        int Lh = n + 1; // need H[0..n]
        vector<int> U(Lh);
        for (int i = 0; i < Lh; ++i) U[i] = p[i + 1];
        vector<int> H = poly_inv(U, Lh);

        // P = H^(n+1) mod x^(n+1)
        vector<int> P = poly_pow(H, n + 1, Lh);

        // factorials up to 2n
        int lim = 2 * n + 5;
        vector<int> fact(lim), invfact(lim);
        fact[0] = 1;
        for (int i = 1; i < lim; ++i) fact[i] = (int)(1LL * fact[i - 1] * i % MOD);
        invfact[lim - 1] = NTT::modpow(fact[lim - 1], MOD - 2, MOD);
        for (int i = lim - 2; i >= 0; --i) invfact[i] = (int)(1LL * invfact[i + 1] * (i + 1) % MOD);

        vector<int> A(Lh), BB(Lh);
        for (int i = 0; i < Lh; ++i) {
            A[i] = (int)(1LL * P[i] * invfact[n + i] % MOD);
            BB[i] = (int)(1LL * invfact[i] * ((i & 1) ? MOD - 1 : 1) % MOD);
        }
        vector<int> Q = NTT::conv_naive(A, BB);
        Q.resize(Lh, 0);

        vector<int> ans(n + 1, 0);
        long long inv_np1 = NTT::modpow(n + 1, MOD - 2, MOD);
        for (int j = 0; j <= n; ++j) {
            int k = n - j;
            long long comb = 1LL * fact[2 * n] * invfact[k] % MOD * invfact[n + j] % MOD;
            long long qj = 1LL * Q[j] * fact[n + j] % MOD;
            long long val = comb * inv_np1 % MOD * qj % MOD;
            ans[k] = (int)val;
        }
        for (int i = 0; i <= n; ++i) {
            if (i) cout << ' ';
            cout << ans[i];
        }
        cout << '\n';
    }
    return 0;
}

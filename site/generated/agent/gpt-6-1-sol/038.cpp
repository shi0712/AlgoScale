#include <bits/stdc++.h>
using namespace std;
using u32 = uint32_t;
using u64 = uint64_t;
using i64 = int64_t;

/*
Let H(z) = sum Catalan(m)^2 z^m, and let b[n][k] count pairs of
sequences together with a chosen set of k common matching pairs.

Remove the chosen pairs from the outermost region. Its remaining 2m
positions carry two independent balanced sequences. In each of the 2m+1
gaps there is a sequence of chosen pairs, each enclosing another region.
Consequently, with u marking chosen pairs,

    F = Q H(z Q^2),       Q = 1/(1-u z F).

Putting V=Q-1 gives V=u z(1+V)^2 H(z(1+V)^2), while
F=(1-1/(1+V))/(u z). Lagrange inversion in u now gives

    b[n][k] = binom(2n,k)/(k+1) * [z^(n-k)] H(z)^(k+1).

For f(z)=z H(z), all required coefficients are [z^(n+1)]f(z)^j.
Compute them together as the polynomial in y

    [z^(n+1)] 1/(1-y f(z)).

Bivariate Bostan--Mori halves the required z-degree in each step while
doubling the y-degree. Thus each polynomial multiplication has O(n)
coefficients. Kronecker substitution and three NTT primes implement these
multiplications modulo any input prime. Differences use centered CRT.

Finally the answer polynomial is sum_k b[n][k](x-1)^k, evaluated by a
factorial-weighted convolution. Total: O(n log^2 n) time, O(n) memory.
*/

static u32 power_mod(u32 a, u64 e, u32 mod) {
    u32 r = 1;
    for (; e; e >>= 1, a = (u64)a * a % mod)
        if (e & 1) r = (u64)r * a % mod;
    return r;
}

// NTT with lazy Montgomery residues. The forward transform has bit-reversed
// output; the inverse consumes that ordering.
template<u32 MOD, u32 ROOT> struct NTT {
    static constexpr u32 neg_inv = []() constexpr {
        u32 x = MOD;
        for (int i = 0; i < 5; ++i) x *= 2 - MOD * x;
        return -x;
    }();
    static constexpr u32 r2 = (u64(0) - MOD) % MOD;
    static u32 reduce(u64 x) {
        return (x + (u64)(u32(x) * neg_inv) * MOD) >> 32;
    }
    static u32 mul(u32 a, u32 b) { return reduce((u64)a * b); }
    static u32 add(u32 a, u32 b) {
        u32 s = a + b - 2 * MOD;
        if (int32_t(s) < 0) s += 2 * MOD;
        return s;
    }
    static u32 sub(u32 a, u32 b) {
        u32 s = a - b;
        if (int32_t(s) < 0) s += 2 * MOD;
        return s;
    }
    static u32 in(u32 x) { return reduce((u64)x * r2); }
    static u32 out(u32 x) {
        x = reduce(x);
        return x >= MOD ? x - MOD : x;
    }
    inline static vector<u32> roots{0, in(1)}, iroots{0, in(1)};
    static void prepare(int n) {
        int old = roots.size();
        if (old >= n) return;
        roots.resize(n); iroots.resize(n);
        for (int len = old; len < n; len <<= 1) {
            u32 w = in(power_mod(ROOT, (MOD - 1) / (2 * len), MOD));
            u32 iw = in(power_mod(ROOT, MOD - 1 - (MOD - 1) / (2 * len), MOD));
            for (int j = len / 2; j < len; ++j) {
                roots[2*j] = roots[j];
                roots[2*j+1] = mul(roots[j], w);
                iroots[2*j] = iroots[j];
                iroots[2*j+1] = mul(iroots[j], iw);
            }
        }
    }
    static void forward(vector<u32>& a) {
        int n = a.size(); prepare(n);
        for (int len = n / 2; len; len >>= 1) {
            for (int i = 0; i < n; i += 2 * len) {
                for (int j = 0; j < len; ++j) {
                    u32 u = a[i+j], v = a[i+j+len];
                    a[i+j] = add(u, v);
                    a[i+j+len] = mul(sub(u, v), roots[len+j]);
                }
            }
        }
    }
    static void inverse(vector<u32>& a) {
        int n = a.size(); prepare(n);
        for (int len = 1; len < n; len <<= 1) {
            for (int i = 0; i < n; i += 2 * len) {
                for (int j = 0; j < len; ++j) {
                    u32 u = a[i+j], v = mul(a[i+j+len], iroots[len+j]);
                    a[i+j] = add(u, v);
                    a[i+j+len] = sub(u, v);
                }
            }
        }
        u32 invn = in(power_mod(n, MOD-2, MOD));
        for (u32& x : a) x = out(mul(x, invn));
    }
    static vector<u32> convolution(const vector<u32>& a, const vector<u32>& b, int need) {
        int sz = a.size() + b.size() - 1, len = 1;
        while (len < sz) len <<= 1;
        vector<u32> x(len), y(len);
        for (int i = 0; i < (int)a.size(); ++i) x[i] = in(a[i]);
        for (int i = 0; i < (int)b.size(); ++i) y[i] = in(b[i]);
        forward(x); forward(y);
        for (int i = 0; i < len; ++i) x[i] = mul(x[i], y[i]);
        inverse(x); x.resize(min(sz, need));
        return x;
    }

    // One bivariate Bostan--Mori step. x is the variable whose coefficient
    // is being extracted, and y records the power of f.
    static pair<vector<u32>, vector<u32>> step(
            const vector<u32>& p, const vector<u32>& q,
            int m, int dp, int dq, int cap, bool first) {
        int b = m + 1, nm = m / 2, nb = nm + 1;
        int nq = min(2*dq, cap), np = min(dp+dq, cap);
        int len = 1;
        while (len < (2*dq+1)*b) len <<= 1;
        vector<u32> qe(len), qo(len), pe, po;
        for (int y = 0; y <= dq; ++y) {
            int off = y*b;
            for (int x = 0; x <= m; ++x)
                ((x&1) ? qo : qe)[off+x/2] = in(q[off+x]);
        }
        forward(qe); forward(qo);
        if (!first) {
            pe.resize(len); po.resize(len);
            for (int y = 0; y <= dp; ++y) {
                int off = y*b;
                for (int x = 0; x <= m; ++x)
                    ((x&1) ? po : pe)[off+x/2] = in(p[off+x]);
            }
            forward(pe); forward(po);
        }
        // Evaluation of x in the bit-reversed ordering of the NTT.
        vector<u32> z(len);
        z[0] = in(1);
        u32 w = in(power_mod(ROOT, (MOD-1)/len, MOD));
        u32 cur = in(1);
        int rev = 0;
        for (int i = 0; i < len; ++i) {
            z[rev] = cur;
            cur = mul(cur, w);
            int bit = len >> 1;
            while (rev & bit) { rev ^= bit; bit >>= 1; }
            rev ^= bit;
        }
        for (int i = 0; i < len; ++i) {
            u32 a = qe[i], bval = qo[i];
            if (!first) {
                if (m & 1) pe[i] = sub(mul(po[i], a), mul(pe[i], bval));
                else pe[i] = sub(mul(pe[i], a), mul(mul(po[i], bval), z[i]));
            }
            qe[i] = sub(mul(a, a), mul(mul(bval, bval), z[i]));
        }
        inverse(qe);
        if (!first) inverse(pe);
        vector<u32> rq((nq+1)*nb), rp;
        if (!first) rp.resize((np+1)*nb);
        for (int y = 0; y <= nq; ++y)
            for (int x = 0; x <= nm; ++x)
                rq[y*nb+x] = qe[y*b+x];
        if (!first) {
            for (int y = 0; y <= np; ++y)
                for (int x = 0; x <= nm; ++x)
                    rp[y*nb+x] = pe[y*b+x];
        }
        return {move(rp), move(rq)};
    }
};

using N1 = NTT<167772161,3>;
using N2 = NTT<469762049,3>;
using N3 = NTT<754974721,11>;

struct CRT {
    static constexpr u32 a = 167772161, b = 469762049, c = 754974721;
    u32 mod, ab, abc;
    u32 ia = power_mod(a, b-2, b);
    u32 iab = power_mod((u64)a*b%c, c-2, c);
    CRT(u32 m): mod(m), ab((u64)a*b%m), abc((u64)ab*(c%m)%m) {}
    u32 get(u32 x, u32 y, u32 z, bool signed_value) const {
        u32 v = (u64)(y + b - x) * ia % b;
        u32 w = (z + c - (x + (u64)a*v)%c) * (u64)iab % c;
        u32 ans = (x + (u64)a*v + (u64)ab*w) % mod;
        if (signed_value && w > c/2) ans = (ans + (u64)mod - abc) % mod;
        return ans;
    }
    vector<u32> join(const vector<u32>& x, const vector<u32>& y,
                     const vector<u32>& z, bool sign) const {
        vector<u32> r(x.size());
        for (int i = 0; i < (int)x.size(); ++i) r[i] = get(x[i], y[i], z[i], sign);
        return r;
    }
};

static vector<u32> convolution(const vector<u32>& a, const vector<u32>& b,
                               int need, u32 mod) {
    int sz = min<int>(a.size()+b.size()-1, need);
    if (min(a.size(), b.size()) <= 32) {
        vector<u32> r(sz);
        for (int i = 0; i < (int)a.size(); ++i)
            for (int j = 0; j < (int)b.size() && i+j < sz; ++j)
                r[i+j] = (r[i+j] + (u64)a[i]*b[j]) % mod;
        return r;
    }
    auto x = N1::convolution(a,b,need);
    auto y = N2::convolution(a,b,need);
    auto z = N3::convolution(a,b,need);
    return CRT(mod).join(x,y,z,false);
}

// Return [x^N] f(x)^j for 0 <= j <= N, with f(0)=0.
static vector<u32> project(const vector<u32>& f, int N, u32 mod) {
    int m = N, dp = 0, dq = 1;
    vector<u32> p(m+1), q(2*(m+1));
    p[0] = q[0] = 1;
    for (int i = 1; i <= N; ++i) q[m+1+i] = f[i] ? mod-f[i] : 0;
    CRT crt(mod);
    bool first = true;
    while (m > 1) {
        auto r1 = N1::step(p,q,m,dp,dq,N,first);
        auto r2 = N2::step(p,q,m,dp,dq,N,first);
        auto r3 = N3::step(p,q,m,dp,dq,N,first);
        vector<u32> np;
        int nb = m/2+1, npdeg = min(dp+dq,N);
        if (first) {
            np.resize((npdeg+1)*nb);
            for (int y = 0; y <= dq; ++y) {
                for (int x = 0; x < nb; ++x) {
                    int ix = 2*x+(m&1);
                    if (ix > m) continue;
                    u32 v = q[y*(m+1)+ix];
                    np[y*nb+x] = (m&1) && v ? mod-v : v;
                }
            }
        } else np = crt.join(r1.first,r2.first,r3.first,true);
        q = crt.join(r1.second,r2.second,r3.second,true);
        p = move(np);
        dp = npdeg; dq = min(2*dq,N); m /= 2; first = false;
    }
    // Q(0,y)=1. The final odd section is P_1(y)-P_0(y)Q_1(y).
    vector<u32> p0(dp+1), p1(dp+1), q1(dq+1);
    for (int y = 0; y <= dp; ++y) p0[y]=p[2*y], p1[y]=p[2*y+1];
    for (int y = 0; y <= dq; ++y) q1[y]=q[2*y+1];
    auto prod = convolution(p0,q1,N+1,mod);
    vector<u32> ans(N+1);
    for (int y = 0; y <= N; ++y) {
        u32 u = y <= dp ? p1[y] : 0;
        u32 v = y < (int)prod.size() ? prod[y] : 0;
        ans[y] = u >= v ? u-v : u+mod-v;
    }
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int tests; cin >> tests;
    while (tests--) {
        int n; u32 mod; cin >> n >> mod;
        vector<u32> fact(2*n+1), invfact(2*n+1);
        fact[0]=1;
        for (int i=1;i<=2*n;++i) fact[i]=(u64)fact[i-1]*i%mod;
        invfact[2*n]=power_mod(fact[2*n],mod-2,mod);
        for(int i=2*n;i;--i) invfact[i-1]=(u64)invfact[i]*i%mod;
        vector<u32> f(n+2);
        for (int i=0;i<=n;++i) {
            u32 cat=(u64)fact[2*i]*invfact[i]%mod*invfact[i+1]%mod;
            f[i+1]=(u64)cat*cat%mod;
        }
        auto a = project(f,n+1,mod);
        vector<u32> r(n+1), s(n+1);
        for(int k=0;k<=n;++k) {
            // b_k = binom(2n,k)/(k+1) * [x^(n+1)] (x H(x))^(k+1).
            // r reverses b_k*k!, ready for the binomial transform.
            u32 v=(u64)fact[2*n]*invfact[2*n-k]%mod;
            v=(u64)v*fact[k]%mod*invfact[k+1]%mod;
            r[n-k]=(u64)v*a[k+1]%mod;
            s[k]=(k&1) && invfact[k] ? mod-invfact[k] : invfact[k];
        }
        auto out=convolution(r,s,n+1,mod);
        for(int k=0;k<=n;++k) {
            if(k) cout << ' ';
            cout << (u64)out[n-k]*invfact[k]%mod;
        }
        cout << '\n';
    }
}

#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using u128 = __uint128_t;
using u32 = unsigned int;

template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
struct NTT64 {
    static u64 ninv;
    static u64 one;
    static vector<u64> roots, iroots, invs;
    static vector<vector<int>> revs;
    static bool initialized;

    static u64 reduce(u128 t) {
        u64 m = u64(t) * ninv;
        u64 r = u64((t + u128(m) * MOD) >> 64);
        return r >= MOD ? r - MOD : r;
    }

    static u64 from(u64 x) {
        return u64((u128(x % MOD) << 64) % MOD);
    }

    static u64 get(u64 x) {
        return reduce(x);
    }

    static u64 add(u64 a, u64 b) {
        u64 c = a + b;
        return c >= MOD ? c - MOD : c;
    }

    static u64 sub(u64 a, u64 b) {
        return a >= b ? a - b : a + MOD - b;
    }

    static u64 mul(u64 a, u64 b) {
        return reduce(u128(a) * b);
    }

    static u64 power(u64 a, u64 e) {
        u64 r = one;
        while (e) {
            if (e & 1) r = mul(r, a);
            a = mul(a, a);
            e >>= 1;
        }
        return r;
    }

    static void init() {
        if (initialized) return;
        u64 inv = 1;
        for (int i = 0; i < 6; ++i) inv *= 2 - MOD * inv;
        ninv = 0 - inv;
        one = from(1);
        roots.resize(MAX_LOG + 1);
        iroots.resize(MAX_LOG + 1);
        invs.resize(MAX_LOG + 1);
        revs.resize(MAX_LOG + 1);
        u64 g = from(PRIMITIVE_ROOT);
        for (int lg = 1; lg <= MAX_LOG; ++lg) {
            roots[lg] = power(g, (MOD - 1) >> lg);
            iroots[lg] = power(roots[lg], MOD - 2);
            invs[lg] = power(from(1ULL << lg), MOD - 2);
        }
        initialized = true;
    }

    static void transform(vector<u64>& a, bool inverse, int lg) {
        init();
        int n = 1 << lg;
        auto& rev = revs[lg];
        if (rev.empty()) {
            rev.resize(n);
            for (int i = 1; i < n; ++i)
                rev[i] = (rev[i >> 1] >> 1) | ((i & 1) << (lg - 1));
        }
        for (int i = 0; i < n; ++i)
            if (i < rev[i]) swap(a[i], a[rev[i]]);

        for (int len = 2, l = 1; len <= n; len <<= 1, ++l) {
            u64 wlen = inverse ? iroots[l] : roots[l];
            int half = len >> 1;
            for (int i = 0; i < n; i += len) {
                u64 w = one;
                for (int j = 0; j < half; ++j) {
                    u64 u = a[i + j];
                    u64 v = mul(a[i + j + half], w);
                    a[i + j] = add(u, v);
                    a[i + j + half] = sub(u, v);
                    w = mul(w, wlen);
                }
            }
        }
        if (inverse) {
            u64 in = invs[lg];
            for (u64& x : a) x = mul(x, in);
        }
    }

    static vector<u64> convolve(const vector<u32>& a, const vector<u32>& b, int lg) {
        init();
        int n = 1 << lg;
        vector<u64> fa(n), fb(n);
        for (size_t i = 0; i < a.size(); ++i) fa[i] = from(a[i]);
        for (size_t i = 0; i < b.size(); ++i) fb[i] = from(b[i]);
        transform(fa, false, lg);
        transform(fb, false, lg);
        for (int i = 0; i < n; ++i) fa[i] = mul(fa[i], fb[i]);
        transform(fa, true, lg);
        fa.resize(a.size() + b.size() - 1);
        for (u64& x : fa) x = get(x);
        return fa;
    }
};

template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
u64 NTT64<MOD, PRIMITIVE_ROOT, MAX_LOG>::ninv = 0;
template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
u64 NTT64<MOD, PRIMITIVE_ROOT, MAX_LOG>::one = 0;
template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
vector<u64> NTT64<MOD, PRIMITIVE_ROOT, MAX_LOG>::roots;
template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
vector<u64> NTT64<MOD, PRIMITIVE_ROOT, MAX_LOG>::iroots;
template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
vector<u64> NTT64<MOD, PRIMITIVE_ROOT, MAX_LOG>::invs;
template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
vector<vector<int>> NTT64<MOD, PRIMITIVE_ROOT, MAX_LOG>::revs;
template <u64 MOD, u64 PRIMITIVE_ROOT, int MAX_LOG>
bool NTT64<MOD, PRIMITIVE_ROOT, MAX_LOG>::initialized = false;

using NTT1 = NTT64<9223372036737335297ULL, 3, 24>;
using NTT2 = NTT64<4611686018429485057ULL, 5, 21>;

static u64 mod_pow(u64 a, u64 e, u64 mod) {
    u64 r = 1;
    while (e) {
        if (e & 1) r = u128(r) * a % mod;
        a = u128(a) * a % mod;
        e >>= 1;
    }
    return r;
}

struct ArbitraryConvolution {
    u32 mod;
    u64 p1_mod, inv_p1_p2;

    explicit ArbitraryConvolution(u32 m) : mod(m) {
        const u64 p1 = 9223372036737335297ULL;
        const u64 p2 = 4611686018429485057ULL;
        p1_mod = p1 % mod;
        inv_p1_p2 = mod_pow(p1 % p2, p2 - 2, p2);
    }

    vector<u32> operator()(const vector<u32>& a, const vector<u32>& b) const {
        if (a.empty() || b.empty()) return {};
        size_t need = a.size() + b.size() - 1;
        if (a.size() * b.size() <= 4096ULL) {
            vector<u32> c(need);
            for (size_t i = 0; i < a.size(); ++i) {
                for (size_t j = 0; j < b.size(); ++j) {
                    c[i + j] = (c[i + j] + u128(a[i]) * b[j]) % mod;
                }
            }
            return c;
        }
        int lg = 1;
        while ((1ULL << lg) < need) ++lg;
        vector<u64> r1 = NTT1::convolve(a, b, lg);
        vector<u64> r2 = NTT2::convolve(a, b, lg);
        const u64 p2 = 4611686018429485057ULL;
        vector<u32> c(need);
        for (size_t i = 0; i < need; ++i) {
            u64 x1 = r1[i];
            u64 x2 = (r2[i] + p2 - x1 % p2) % p2;
            x2 = u128(x2) * inv_p1_p2 % p2;
            c[i] = (x1 % mod + u128(p1_mod) * x2) % mod;
        }
        return c;
    }
};

struct Poly2 {
    int nx = 0, ny = 0;
    vector<u32> a;

    Poly2() = default;
    Poly2(int x, int y) : nx(x), ny(y), a(size_t(x + 1) * (y + 1)) {}

    u32& at(int x, int y) { return a[size_t(x) * (ny + 1) + y]; }
    u32 at(int x, int y) const { return a[size_t(x) * (ny + 1) + y]; }
};

static Poly2 multiply2(const Poly2& A, const Poly2& B, int max_x, int max_y,
                       const ArbitraryConvolution& conv) {
    int rx = min(max_x, A.nx + B.nx);
    int ry = min(max_y, A.ny + B.ny);
    Poly2 C(rx, ry);
    long long work = 1LL * (A.nx + 1) * (A.ny + 1) * (B.nx + 1) * (B.ny + 1);
    if (work <= 4096) {
        for (int i = 0; i <= A.nx; ++i) {
            for (int j = 0; j <= A.ny; ++j) {
                u32 x = A.at(i, j);
                if (!x) continue;
                for (int k = 0; k <= B.nx && i + k <= rx; ++k) {
                    for (int l = 0; l <= B.ny && j + l <= ry; ++l) {
                        u32& dest = C.at(i + k, j + l);
                        dest = (dest + u128(x) * B.at(k, l)) % conv.mod;
                    }
                }
            }
        }
        return C;
    }

    int K = A.nx + B.nx + 1;
    int full_y = A.ny + B.ny;
    vector<u32> fa(size_t(K) * (full_y + 1)), fb(fa.size());
    for (int i = 0; i <= A.nx; ++i)
        for (int j = 0; j <= A.ny; ++j)
            fa[i + K * j] = A.at(i, j);
    for (int i = 0; i <= B.nx; ++i)
        for (int j = 0; j <= B.ny; ++j)
            fb[i + K * j] = B.at(i, j);
    vector<u32> fc = conv(fa, fb);
    for (int i = 0; i <= rx; ++i)
        for (int j = 0; j <= ry; ++j)
            C.at(i, j) = fc[i + K * j];
    return C;
}

static vector<u32> power_projection(const vector<u32>& g, int N,
                                    const ArbitraryConvolution& conv) {
    if (N == 0) return {1};
    Poly2 P(0, 0), Q(N, 1);
    P.at(0, 0) = 1;
    Q.at(0, 0) = 1;
    for (int i = 1; i <= N; ++i)
        Q.at(i, 1) = g[i] ? conv.mod - g[i] : 0;

    int n = N;
    while (n) {
        Poly2 Qneg = Q;
        for (int i = 1; i <= Qneg.nx; i += 2)
            for (int j = 0; j <= Qneg.ny; ++j)
                if (Qneg.at(i, j)) Qneg.at(i, j) = conv.mod - Qneg.at(i, j);

        Poly2 U = multiply2(P, Qneg, n, N, conv);
        Poly2 V = multiply2(Q, Qneg, n, N, conv);
        int half = n >> 1;
        Poly2 newP(half, U.ny), newQ(half, V.ny);
        int parity = n & 1;
        for (int i = 0; i <= half; ++i) {
            for (int j = 0; j <= U.ny; ++j)
                newP.at(i, j) = U.at(2 * i + parity, j);
            for (int j = 0; j <= V.ny; ++j)
                newQ.at(i, j) = V.at(2 * i, j);
        }
        P = std::move(newP);
        Q = std::move(newQ);
        n = half;
    }

    vector<u32> result(N + 1);
    for (int j = 0; j <= N && j <= P.ny; ++j) result[j] = P.at(0, j);
    return result;
}

static vector<u32> solve_case(int n, u32 mod) {
    ArbitraryConvolution conv(mod);
    vector<u32> fact(2 * n + 1), inv_fact(2 * n + 1);
    fact[0] = 1;
    for (int i = 1; i <= 2 * n; ++i) fact[i] = u64(fact[i - 1]) * i % mod;
    inv_fact[2 * n] = mod_pow(fact[2 * n], mod - 2, mod);
    for (int i = 2 * n; i >= 1; --i)
        inv_fact[i - 1] = u64(inv_fact[i]) * i % mod;
    auto binom = [&](int a, int b) -> u32 {
        if (b < 0 || b > a) return 0;
        return u64(fact[a]) * inv_fact[b] % mod * inv_fact[a - b] % mod;
    };

    vector<u32> catalan(n + 1);
    catalan[0] = 1;
    for (int i = 1; i <= n; ++i) {
        u64 numerator = u64(2) * (2 * i - 1) % mod * catalan[i - 1] % mod;
        catalan[i] = numerator * mod_pow(i + 1, mod - 2, mod) % mod;
    }

    int N = n + 1;
    vector<u32> g(N + 1);
    for (int i = 1; i <= N; ++i)
        g[i] = u64(catalan[i - 1]) * catalan[i - 1] % mod;
    vector<u32> projection = power_projection(g, N, conv);

    vector<u32> weighted(n + 1);
    for (int k = 0; k <= n; ++k) {
        u32 A = projection[k + 1];
        u32 B = u64(binom(2 * n, k)) * mod_pow(k + 1, mod - 2, mod) % mod;
        weighted[k] = u64(A) * B % mod;
    }

    vector<u32> left(n + 1), right(n + 1);
    for (int k = 0; k <= n; ++k) {
        left[n - k] = u64(fact[k]) * weighted[k] % mod;
        right[k] = (k & 1) ? mod - inv_fact[k] : inv_fact[k];
    }
    vector<u32> product = conv(left, right);
    vector<u32> answer(n + 1);
    for (int j = 0; j <= n; ++j)
        answer[j] = u64(inv_fact[j]) * product[n - j] % mod;
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    while (t--) {
        int n;
        u32 mod;
        cin >> n >> mod;
        vector<u32> answer = solve_case(n, mod);
        for (int i = 0; i <= n; ++i) {
            if (i) cout << ' ';
            cout << answer[i];
        }
        cout << '\n';
    }
    return 0;
}

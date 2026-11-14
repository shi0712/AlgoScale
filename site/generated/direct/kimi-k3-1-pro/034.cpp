#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using Poly = vector<int>;

static constexpr int MOD = 998244353;
static constexpr int PRIMITIVE_ROOT = 3;

static int mod_pow(int a, long long e) {
    long long r = 1;
    long long x = a;
    while (e > 0) {
        if (e & 1) r = r * x % MOD;
        x = x * x % MOD;
        e >>= 1;
    }
    return static_cast<int>(r);
}

static void ntt(vector<int>& a, bool invert) {
    const int n = static_cast<int>(a.size());

    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }

    for (int len = 2; len <= n; len <<= 1) {
        int wlen = mod_pow(
            invert ? 332748118 : PRIMITIVE_ROOT,
            (MOD - 1) / len
        );

        for (int i = 0; i < n; i += len) {
            long long w = 1;
            const int half = len >> 1;
            for (int j = 0; j < half; ++j) {
                int u = a[i + j];
                int v = static_cast<int>(
                    a[i + j + half] * w % MOD
                );
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[i + j] = x;
                a[i + j + half] = y;
                w = w * wlen % MOD;
            }
        }
    }

    if (invert) {
        const int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) {
            x = static_cast<int>(1LL * x * inv_n % MOD);
        }
    }
}

// Returns the convolution truncated to at most limit coefficients.
static Poly convolution(const Poly& a, const Poly& b, int limit) {
    if (limit <= 0) return {};

    if (a.size() == 1) {
        int len = min<int>(b.size(), limit);
        Poly c(len);
        for (int i = 0; i < len; ++i) {
            c[i] = static_cast<int>(1LL * a[0] * b[i] % MOD);
        }
        return c;
    }
    if (b.size() == 1) {
        int len = min<int>(a.size(), limit);
        Poly c(len);
        for (int i = 0; i < len; ++i) {
            c[i] = static_cast<int>(1LL * b[0] * a[i] % MOD);
        }
        return c;
    }

    const long long work =
        1LL * static_cast<long long>(a.size()) * b.size();
    const long long smaller = min(a.size(), b.size());

    if (work <= 100000 || (smaller <= 4 && work <= 1000000)) {
        int full = static_cast<int>(a.size() + b.size() - 1);
        Poly c(min(full, limit), 0);

        for (int i = 0; i < static_cast<int>(a.size()); ++i) {
            if (i >= limit) break;
            int mj = min<int>(b.size(), limit - i);
            for (int j = 0; j < mj; ++j) {
                int& dest = c[i + j];
                dest = static_cast<int>(
                    (dest + 1LL * a[i] * b[j]) % MOD
                );
            }
        }
        return c;
    }

    int sa = min<int>(a.size(), limit);
    int sb = min<int>(b.size(), limit);
    int full = sa + sb - 1;
    int need = min(full, limit);

    int n = 1;
    while (n < full) n <<= 1;

    vector<int> fa(n, 0), fb(n, 0);
    for (int i = 0; i < sa; ++i) fa[i] = a[i];
    for (int i = 0; i < sb; ++i) fb[i] = b[i];

    ntt(fa, false);
    ntt(fb, false);
    for (int i = 0; i < n; ++i) {
        fa[i] = static_cast<int>(1LL * fa[i] * fb[i] % MOD);
    }
    ntt(fa, true);

    fa.resize(need);
    return fa;
}

struct Solver {
    int n;
    vector<int> fact, inv_fact;

    explicit Solver(int n_) : n(n_), fact(n_ + 1), inv_fact(n_ + 1) {
        fact[0] = 1;
        for (int i = 1; i <= n; ++i) {
            fact[i] = static_cast<int>(1LL * fact[i - 1] * i % MOD);
        }
        inv_fact[n] = mod_pow(fact[n], MOD - 2);
        for (int i = n; i >= 1; --i) {
            inv_fact[i - 1] =
                static_cast<int>(1LL * inv_fact[i] * i % MOD);
        }
    }

    static void trim(Poly& p) {
        while (p.size() > 1 && p.back() == 0) p.pop_back();
    }

    // Matching polynomial of K_{p,q}.
    Poly complete_poly(int p, int q) const {
        int d = min(p, q);
        Poly f(d + 1);
        for (int x = 0; x <= d; ++x) {
            long long v = 1LL * fact[p] * fact[q] % MOD;
            v = v * inv_fact[x] % MOD;
            v = v * inv_fact[p - x] % MOD;
            v = v * inv_fact[q - x] % MOD;
            f[x] = static_cast<int>(v);
        }
        return f;
    }

    /*
       If s_j counts j-matchings in the forbidden-edge graph, this returns
       the matching polynomial of the remaining graph.

       m_x = sum_{j=0}^x (-1)^j s_j *
             C(p-j,x-j) C(q-j,x-j) (x-j)!.
    */
    Poly transform(int p, int q, const Poly& s) const {
        const int dmax = min(p, q);
        if (dmax == 0) return {1};

        // No forbidden edges.
        if (s.size() == 1) return complete_poly(p, q);

        // All possible edges are forbidden; this also avoids some large NTTs.
        if (static_cast<int>(s.size()) == dmax + 1) {
            Poly cp = complete_poly(p, q);
            if (s == cp) return {1};
        }

        const int d = static_cast<int>(s.size()) - 1;
        Poly r(d + 1);

        for (int j = 0; j <= d; ++j) {
            long long v = 1LL * s[j] * fact[p - j] % MOD;
            v = v * fact[q - j] % MOD;
            if (j & 1) v = (MOD - v) % MOD;
            r[j] = static_cast<int>(v);
        }

        Poly e(dmax + 1);
        for (int i = 0; i <= dmax; ++i) e[i] = inv_fact[i];

        Poly c = convolution(r, e, dmax + 1);
        c.resize(dmax + 1, 0);

        Poly ans(dmax + 1);
        for (int x = 0; x <= dmax; ++x) {
            long long v = 1LL * c[x] * inv_fact[p - x] % MOD;
            v = v * inv_fact[q - x] % MOD;
            ans[x] = static_cast<int>(v);
        }
        trim(ans);
        return ans;
    }

    /*
       Current subproblem uses bits bit,...,0 and residual threshold k.

       If want_r is false, returns M(G), the matching polynomial of G.
       If want_r is true, returns M(complement of G), where complementation
       is inside K_{|A|,|B|}.
    */
    Poly solve(
        vector<u64> A,
        vector<u64> B,
        int bit,
        u64 k,
        bool want_r
    ) {
        const int p = static_cast<int>(A.size());
        const int q = static_cast<int>(B.size());

        if (p == 0 || q == 0) {
            // G has no edges; its complement is complete.
            return want_r ? complete_poly(p, q) : Poly{1};
        }

        if (k == 0) {
            // Every remaining pair is an edge.
            return want_r ? Poly{1} : complete_poly(p, q);
        }

        // A graph with only one vertex on one side has matching degree <= 1.
        if (p == 1 || q == 1) {
            const u64 mask = (1ULL << (bit + 1)) - 1;
            long long edges = 0;

            if (p == 1) {
                for (u64 y : B) {
                    edges += (((A[0] ^ y) & mask) >= k);
                }
            } else {
                for (u64 x : A) {
                    edges += (((x ^ B[0]) & mask) >= k);
                }
            }

            long long total = 1LL * p * q;
            long long ways = want_r ? total - edges : edges;
            ways %= MOD;
            if (ways == 0) return {1};
            return {1, static_cast<int>(ways)};
        }

        vector<u64> A0, A1, B0, B1;

        int ca1 = 0, cb1 = 0;
        for (u64 x : A) ca1 += ((x >> bit) & 1ULL);
        for (u64 x : B) cb1 += ((x >> bit) & 1ULL);

        A0.reserve(A.size() - ca1);
        A1.reserve(ca1);
        B0.reserve(B.size() - cb1);
        B1.reserve(cb1);

        for (u64 x : A) {
            if ((x >> bit) & 1ULL) A1.push_back(x);
            else A0.push_back(x);
        }
        for (u64 x : B) {
            if ((x >> bit) & 1ULL) B1.push_back(x);
            else B0.push_back(x);
        }

        vector<u64>().swap(A);
        vector<u64>().swap(B);

        const int dmax = min(p, q);
        const bool kbit = (k >> bit) & 1ULL;
        const u64 next_k = kbit ? (k ^ (1ULL << bit)) : k;

        Poly s;

        if (!kbit) {
            // Different current bits give a complete cross connection.
            // G = join(G00, G11), hence R(G)=R(G00)R(G11).
            bool zero0 = A0.empty() && B0.empty();
            bool zero1 = A1.empty() && B1.empty();

            if (zero0) {
                return solve(
                    std::move(A1), std::move(B1),
                    bit - 1, next_k, want_r
                );
            }
            if (zero1) {
                return solve(
                    std::move(A0), std::move(B0),
                    bit - 1, next_k, want_r
                );
            }

            Poly f0 = solve(
                std::move(A0), std::move(B0),
                bit - 1, next_k, true
            );
            Poly f1 = solve(
                std::move(A1), std::move(B1),
                bit - 1, next_k, true
            );

            s = convolution(f0, f1, dmax + 1);
            trim(s);

            if (want_r) return s;
            return transform(p, q, s);
        }

        // Equal current bits cannot produce an edge.
        // G = G01 disjoint-union G10, hence M(G)=M(G01)M(G10).
        bool zero0 = A0.empty() && B1.empty();
        bool zero1 = A1.empty() && B0.empty();

        if (zero0) {
            return solve(
                std::move(A1), std::move(B0),
                bit - 1, next_k, want_r
            );
        }
        if (zero1) {
            return solve(
                std::move(A0), std::move(B1),
                bit - 1, next_k, want_r
            );
        }

        Poly f0 = solve(
            std::move(A0), std::move(B1),
            bit - 1, next_k, false
        );
        Poly f1 = solve(
            std::move(A1), std::move(B0),
            bit - 1, next_k, false
        );

        s = convolution(f0, f1, dmax + 1);
        trim(s);

        if (!want_r) return s;
        return transform(p, q, s);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    u64 k;
    cin >> n >> k;

    vector<u64> a(n), b(n);
    for (u64& x : a) cin >> x;
    for (u64& x : b) cin >> x;

    Solver solver(n);
    Poly answer = solver.solve(
        std::move(a), std::move(b), 59, k, false
    );

    for (int x = 1; x <= n; ++x) {
        if (x > 1) cout << ' ';
        int value = x < static_cast<int>(answer.size())
                  ? answer[x] : 0;
        cout << value;
    }
    cout << '\n';

    return 0;
}

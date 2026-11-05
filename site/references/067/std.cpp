#include <bits/stdc++.h>
using namespace std;

static const uint32_t MOD = 1000000007u;
// The strengthened package allows N,K,Q up to 500.  The ring buffers are
// compile-time sized so that the hot polynomial operations remain contiguous.
static const int MAXN = 512;

using u64 = uint64_t;
using u128 = __uint128_t;

static inline uint32_t addm(uint32_t a, uint32_t b) {
    uint32_t s = a + b;
    if (s >= MOD) s -= MOD;
    return s;
}

static inline uint32_t subm(uint32_t a, uint32_t b) {
    return a >= b ? a - b : a + MOD - b;
}

static inline uint32_t mulm(uint32_t a, uint32_t b) {
    return (u64)a * b % MOD;
}

static uint32_t modpow(uint32_t a, uint64_t e) {
    uint64_t r = 1, x = a;
    while (e) {
        if (e & 1) r = r * x % MOD;
        x = x * x % MOD;
        e >>= 1;
    }
    return (uint32_t)r;
}

static inline uint32_t invm(uint32_t a) {
    return modpow(a, MOD - 2);
}

using Matrix = vector<vector<uint32_t>>;

struct SplitMix64 {
    uint64_t x;

    explicit SplitMix64(uint64_t seed) : x(seed) {}

    uint64_t next() {
        uint64_t z = (x += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }

    uint32_t mod() {
        return (uint32_t)(next() % MOD);
    }
};

static vector<uint32_t> matVec(
    const Matrix& A,
    const vector<uint32_t>& v
) {
    int n = A.size();
    vector<uint32_t> r(n);

    for (int i = 0; i < n; ++i) {
        u128 s = 0;
        for (int j = 0; j < n; ++j)
            s += (u64)A[i][j] * v[j];
        r[i] = (uint32_t)(s % MOD);
    }

    return r;
}

static vector<uint32_t> rowMat(
    const vector<uint32_t>& v,
    const Matrix& A
) {
    int n = A.size();
    vector<uint32_t> r(n);

    for (int j = 0; j < n; ++j) {
        u128 s = 0;
        for (int i = 0; i < n; ++i)
            s += (u64)v[i] * A[i][j];
        r[j] = (uint32_t)(s % MOD);
    }

    return r;
}

static vector<uint32_t> solveSquare(
    Matrix A,
    vector<uint32_t> b
) {
    int n = A.size();

    for (int col = 0; col < n; ++col) {
        int piv = col;
        while (piv < n && A[piv][col] == 0) ++piv;

        if (piv == n) return {};

        if (piv != col) {
            swap(A[piv], A[col]);
            swap(b[piv], b[col]);
        }

        uint32_t iv = invm(A[col][col]);

        for (int j = col; j < n; ++j)
            A[col][j] = mulm(A[col][j], iv);

        b[col] = mulm(b[col], iv);

        for (int i = 0; i < n; ++i) {
            if (i == col || A[i][col] == 0) continue;

            uint32_t f = A[i][col];
            A[i][col] = 0;

            for (int j = col + 1; j < n; ++j)
                A[i][j] =
                    subm(A[i][j], mulm(f, A[col][j]));

            b[i] = subm(b[i], mulm(f, b[col]));
        }
    }

    return b;
}

struct KrylovData {
    // v, Av, ..., A^{d-1}v
    vector<vector<uint32_t>> chain;

    // A^d v = sum c[j] A^j v
    vector<uint32_t> c;

    // These rows make the Krylov matrix nonsingular.
    vector<int> pivRows;
};

static KrylovData krylov(
    const Matrix& A,
    const vector<uint32_t>& v
) {
    int n = A.size();

    vector<vector<uint32_t>> ebasis;
    vector<vector<uint32_t>> erep;
    vector<int> piv;

    vector<vector<uint32_t>> chain;

    vector<uint32_t> x = v;

    for (int k = 0; k <= n; ++k) {
        vector<uint32_t> rem = x;

        vector<uint32_t> rep(n + 1, 0);
        rep[k] = 1;

        for (int q = 0; q < (int)ebasis.size(); ++q) {
            uint32_t f = rem[piv[q]];
            if (!f) continue;

            for (int j = 0; j < n; ++j)
                rem[j] =
                    subm(rem[j], mulm(f, ebasis[q][j]));

            for (int j = 0; j <= n; ++j)
                rep[j] =
                    subm(rep[j], mulm(f, erep[q][j]));
        }

        int pr = -1;

        for (int j = 0; j < n; ++j) {
            if (rem[j]) {
                pr = j;
                break;
            }
        }

        if (pr == -1) {
            vector<uint32_t> c(k);

            // rep[k] = 1 and
            // sum rep[j] A^j v = 0.
            for (int j = 0; j < k; ++j)
                c[j] = rep[j] ? MOD - rep[j] : 0;

            return {chain, c, piv};
        }

        uint32_t iv = invm(rem[pr]);

        for (int j = 0; j < n; ++j)
            rem[j] = mulm(rem[j], iv);

        for (int j = 0; j <= n; ++j)
            rep[j] = mulm(rep[j], iv);

        ebasis.push_back(move(rem));
        erep.push_back(move(rep));
        piv.push_back(pr);

        chain.push_back(x);
        x = matVec(A, x);
    }

    abort();
}

struct RrefData {
    Matrix R;
    vector<int> piv;
    vector<int> freeCols;
};

static RrefData rrefFullRowRank(Matrix R) {
    int d = R.size();
    int n = R.empty() ? 0 : R[0].size();

    vector<int> piv;
    int row = 0;

    for (int col = 0; col < n && row < d; ++col) {
        int p = row;

        while (p < d && R[p][col] == 0) ++p;

        if (p == d) continue;

        if (p != row)
            swap(R[p], R[row]);

        uint32_t iv = invm(R[row][col]);

        for (int j = col; j < n; ++j)
            R[row][j] = mulm(R[row][j], iv);

        for (int i = 0; i < d; ++i) {
            if (i == row || R[i][col] == 0) continue;

            uint32_t f = R[i][col];
            R[i][col] = 0;

            for (int j = col + 1; j < n; ++j)
                R[i][j] =
                    subm(R[i][j], mulm(f, R[row][j]));
        }

        piv.push_back(col);
        ++row;
    }

    vector<char> isP(n, 0);

    for (int x : piv)
        isP[x] = 1;

    vector<int> freeCols;

    for (int j = 0; j < n; ++j)
        if (!isP[j])
            freeCols.push_back(j);

    return {
        move(R),
        move(piv),
        move(freeCols)
    };
}

struct Block {
    int d;

    // Companion matrix:
    //
    // [0 ... 0 c0]
    // [1 ... 0 c1]
    // [      ... ]
    // [0 ... 1 c[d-1]]
    vector<uint32_t> c;

    int off;
};

struct FrobeniusDecomp {
    int N;

    // G P = P F
    Matrix P;

    Matrix Pinv;

    // Largest invariant factor first.
    vector<Block> blocks;

    static Matrix invertMatrix(Matrix A) {
        int n = A.size();

        Matrix B(
            n,
            vector<uint32_t>(2 * n)
        );

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j)
                B[i][j] = A[i][j];

            B[i][n + i] = 1;
        }

        for (int col = 0; col < n; ++col) {
            int p = col;

            while (p < n && B[p][col] == 0) ++p;

            if (p == n)
                return {};

            if (p != col)
                swap(B[p], B[col]);

            uint32_t iv = invm(B[col][col]);

            for (int j = 0; j < 2 * n; ++j)
                B[col][j] =
                    mulm(B[col][j], iv);

            for (int i = 0; i < n; ++i) {
                if (i == col || B[i][col] == 0)
                    continue;

                uint32_t f = B[i][col];

                for (int j = 0; j < 2 * n; ++j)
                    B[i][j] =
                        subm(
                            B[i][j],
                            mulm(f, B[col][j])
                        );
            }
        }

        Matrix R(
            n,
            vector<uint32_t>(n)
        );

        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                R[i][j] = B[i][n + j];

        return R;
    }

    static vector<uint32_t> blockPoly(
        const Block& b
    ) {
        // x^d - sum c_i x^i
        vector<uint32_t> f(
            b.d + 1,
            0
        );

        f[b.d] = 1;

        for (int i = 0; i < b.d; ++i)
            f[i] =
                b.c[i]
                ? MOD - b.c[i]
                : 0;

        return f;
    }

    static bool dividesMonic(
        const vector<uint32_t>& small,
        const vector<uint32_t>& big
    ) {
        int a = (int)small.size() - 1;
        int b = (int)big.size() - 1;

        if (a > b)
            return false;

        vector<uint32_t> r = big;

        for (int i = b; i >= a; --i) {
            uint32_t q = r[i];

            if (!q) continue;

            int sh = i - a;

            for (int j = 0; j <= a; ++j)
                r[sh + j] =
                    subm(
                        r[sh + j],
                        mulm(q, small[j])
                    );
        }

        for (int i = 0; i < a; ++i)
            if (r[i])
                return false;

        return true;
    }

    bool canonicalChain() const {
        // f_{i+1} | f_i.
        for (
            int i = 0;
            i + 1 < (int)blocks.size();
            ++i
        ) {
            auto big =
                blockPoly(blocks[i]);

            auto small =
                blockPoly(blocks[i + 1]);

            if (!dividesMonic(small, big))
                return false;
        }

        return true;
    }

    explicit FrobeniusDecomp(
        const Matrix& A0
    ) {
        N = A0.size();

        uint64_t seed =
            chrono::high_resolution_clock::
                now()
                .time_since_epoch()
                .count();

        seed ^= (uintptr_t)this;

        SplitMix64 rng(seed);

        for (;;) {
            Matrix A = A0;

            // E maps current coordinates back to
            // original coordinates.
            Matrix E(
                N,
                vector<uint32_t>(N)
            );

            for (int i = 0; i < N; ++i)
                E[i][i] = 1;

            vector<vector<uint32_t>> pcols;
            pcols.reserve(N);

            blocks.clear();

            int off = 0;

            while (!A.empty()) {
                int n = A.size();

                bool done = false;

                while (!done) {
                    vector<uint32_t> v(n);

                    bool nz = false;

                    for (int i = 0; i < n; ++i) {
                        v[i] = rng.mod();
                        nz |= (v[i] != 0);
                    }

                    if (!nz)
                        v[0] = 1;

                    KrylovData kd =
                        krylov(A, v);

                    int d =
                        kd.chain.size();

                    /*
                     * Whole current space is cyclic.
                     */
                    if (d == n) {
                        for (int j = 0; j < d; ++j) {
                            vector<uint32_t> gv(
                                N,
                                0
                            );

                            for (int r = 0; r < N; ++r) {
                                u128 ss = 0;

                                for (
                                    int k = 0;
                                    k < n;
                                    ++k
                                ) {
                                    ss +=
                                        (u64)E[r][k]
                                        * kd.chain[j][k];
                                }

                                gv[r] =
                                    (uint32_t)(
                                        ss % MOD
                                    );
                            }

                            pcols.push_back(
                                move(gv)
                            );
                        }

                        blocks.push_back({
                            d,
                            kd.c,
                            off
                        });

                        off += d;

                        A.clear();
                        E.clear();

                        done = true;
                        break;
                    }

                    /*
                     * Pick w such that
                     *
                     * K^T w = e_{d-1}.
                     *
                     * Then the observability matrix
                     *
                     * O =
                     * [ w^T
                     *   w^T A
                     *   ...
                     *   w^T A^{d-1} ]
                     *
                     * is nonsingular on the Krylov
                     * subspace C.
                     */
                    Matrix M(
                        d,
                        vector<uint32_t>(d)
                    );

                    for (int i = 0; i < d; ++i)
                        for (int j = 0; j < d; ++j)
                            M[i][j] =
                                kd.chain[i]
                                    [kd.pivRows[j]];

                    vector<uint32_t> rhs(
                        d,
                        0
                    );

                    rhs[d - 1] = 1;

                    vector<uint32_t> sol =
                        solveSquare(M, rhs);

                    if (sol.empty())
                        continue;

                    vector<uint32_t> w(
                        n,
                        0
                    );

                    for (int j = 0; j < d; ++j)
                        w[kd.pivRows[j]] =
                            sol[j];

                    Matrix O(
                        d,
                        vector<uint32_t>(n)
                    );

                    vector<uint32_t> rr = w;

                    for (int i = 0; i < d; ++i) {
                        O[i] = rr;

                        if (i + 1 < d)
                            rr =
                                rowMat(rr, A);
                    }

                    RrefData rd =
                        rrefFullRowRank(O);

                    if (
                        (int)rd.piv.size()
                        != d
                    ) {
                        continue;
                    }

                    int m = n - d;

                    /*
                     * W is a systematic basis of ker O.
                     *
                     * If the RREF is [ I | R ], then
                     *
                     * W[:,j]
                     * =
                     * e_free[j]
                     * - sum_r R[r][free[j]] e_piv[r].
                     *
                     * Compute Y = A W.
                     */
                    Matrix Y(
                        n,
                        vector<uint32_t>(m)
                    );

                    for (int jj = 0; jj < m; ++jj) {
                        int f =
                            rd.freeCols[jj];

                        for (int row = 0; row < n; ++row) {
                            u128 sub = 0;

                            for (
                                int r = 0;
                                r < d;
                                ++r
                            ) {
                                sub +=
                                    (u64)
                                    rd.R[r][f]
                                    *
                                    A[row]
                                     [rd.piv[r]];
                            }

                            Y[row][jj] =
                                subm(
                                    A[row][f],
                                    (uint32_t)(
                                        sub % MOD
                                    )
                                );
                        }
                    }

                    /*
                     * Exact Las Vegas certificate:
                     *
                     * A W must lie in W = ker O.
                     */
                    bool ok = true;

                    for (
                        int r = 0;
                        r < d && ok;
                        ++r
                    ) {
                        int pc =
                            rd.piv[r];

                        for (
                            int jj = 0;
                            jj < m;
                            ++jj
                        ) {
                            u128 ss =
                                Y[pc][jj];

                            for (
                                int q = 0;
                                q < m;
                                ++q
                            ) {
                                ss +=
                                    (u64)
                                    rd.R[r]
                                        [rd.freeCols[q]]
                                    *
                                    Y[rd.freeCols[q]]
                                     [jj];
                            }

                            if (
                                (uint32_t)(
                                    ss % MOD
                                ) != 0
                            ) {
                                ok = false;
                                break;
                            }
                        }
                    }

                    if (!ok)
                        continue;

                    /*
                     * The cyclic space and ker O
                     * are now exact invariant
                     * complementary subspaces.
                     *
                     * Commit the cyclic block.
                     */
                    for (int j = 0; j < d; ++j) {
                        vector<uint32_t> gv(
                            N,
                            0
                        );

                        for (int r = 0; r < N; ++r) {
                            u128 ss = 0;

                            for (
                                int k = 0;
                                k < n;
                                ++k
                            ) {
                                ss +=
                                    (u64)E[r][k]
                                    *
                                    kd.chain[j][k];
                            }

                            gv[r] =
                                (uint32_t)(
                                    ss % MOD
                                );
                        }

                        pcols.push_back(
                            move(gv)
                        );
                    }

                    blocks.push_back({
                        d,
                        kd.c,
                        off
                    });

                    off += d;

                    /*
                     * Restriction of A to W.
                     *
                     * Because W is systematic,
                     * the coordinates of a vector
                     * in W are simply its free
                     * coordinates.
                     */
                    Matrix Anew(
                        m,
                        vector<uint32_t>(m)
                    );

                    for (int i = 0; i < m; ++i) {
                        int row =
                            rd.freeCols[i];

                        for (int j = 0; j < m; ++j)
                            Anew[i][j] =
                                Y[row][j];
                    }

                    /*
                     * E <- E W.
                     *
                     * Again exploit the systematic
                     * representation rather than
                     * doing a dense N*n*m product.
                     */
                    Matrix Enew(
                        N,
                        vector<uint32_t>(m)
                    );

                    for (int jj = 0; jj < m; ++jj) {
                        int f =
                            rd.freeCols[jj];

                        for (int row = 0; row < N; ++row) {
                            u128 sub = 0;

                            for (
                                int r = 0;
                                r < d;
                                ++r
                            ) {
                                sub +=
                                    (u64)
                                    rd.R[r][f]
                                    *
                                    E[row]
                                     [rd.piv[r]];
                            }

                            Enew[row][jj] =
                                subm(
                                    E[row][f],
                                    (uint32_t)(
                                        sub % MOD
                                    )
                                );
                        }
                    }

                    A = move(Anew);
                    E = move(Enew);

                    done = true;
                }
            }

            /*
             * Any accepted split already gives an
             * exact direct sum of companion blocks.
             *
             * This final divisibility check forces
             * the blocks to be the actual Frobenius
             * invariant factors.
             *
             * If an unlucky random vector selected a
             * smaller direct summand, rebuild.
             */
            if (!canonicalChain())
                continue;

            P.assign(
                N,
                vector<uint32_t>(N)
            );

            for (int j = 0; j < N; ++j)
                for (int i = 0; i < N; ++i)
                    P[i][j] =
                        pcols[j][i];

            Pinv =
                invertMatrix(P);

            if (Pinv.empty())
                continue;

            break;
        }
    }
};

/*
 * Element of F[x] / (minimal polynomial).
 */
struct Poly {
    array<uint32_t, MAXN> a{};
};

struct PolyRing {
    int n;

    // Monic modulus of degree n.
    vector<uint32_t> modPoly;

    /*
     * red[t] =
     * x^(n+t) mod modPoly
     *
     * for 0 <= t <= n-2.
     */
    vector<array<uint32_t, MAXN>> red;

    PolyRing() = default;

    PolyRing(
        int n_,
        const vector<uint32_t>& c
    ) :
        n(n_),
        modPoly(c)
    {
        if (n >= 2) {
            red.resize(n - 1);

            /*
             * x^n =
             * -sum modPoly[j] x^j.
             */
            for (int j = 0; j < n; ++j)
                red[0][j] =
                    modPoly[j]
                    ? MOD - modPoly[j]
                    : 0;

            /*
             * Obtain x^(n+t+1) by multiplying
             * the previous remainder by x.
             */
            for (int t = 1; t < n - 1; ++t) {
                uint32_t last =
                    red[t - 1][n - 1];

                uint32_t p0 =
                    mulm(
                        last,
                        modPoly[0]
                    );

                red[t][0] =
                    p0
                    ? MOD - p0
                    : 0;

                for (int j = 1; j < n; ++j) {
                    uint32_t prod =
                        mulm(
                            last,
                            modPoly[j]
                        );

                    red[t][j] =
                        subm(
                            red[t - 1][j - 1],
                            prod
                        );
                }
            }
        }
    }

    Poly one() const {
        Poly p;
        p.a[0] = 1;
        return p;
    }

    Poly xpoly() const {
        Poly p;

        if (n == 1) {
            // x = -modPoly[0].
            p.a[0] =
                modPoly[0]
                ? MOD - modPoly[0]
                : 0;
        } else {
            p.a[1] = 1;
        }

        return p;
    }

    Poly add(
        const Poly& A,
        const Poly& B
    ) const {
        Poly C;

        for (int i = 0; i < n; ++i)
            C.a[i] =
                addm(
                    A.a[i],
                    B.a[i]
                );

        return C;
    }

    Poly sub(
        const Poly& A,
        const Poly& B
    ) const {
        Poly C;

        for (int i = 0; i < n; ++i)
            C.a[i] =
                subm(
                    A.a[i],
                    B.a[i]
                );

        return C;
    }

    /*
     * Naive O(n^2) multiplication.
     *
     * We first compute the convolution using
     * 128-bit accumulators, then use precomputed
     * reductions of x^n ... x^(2n-2).
     *
     * Only O(n) modular divisions are required
     * per multiplication.
     */
    Poly mul(
        const Poly& A,
        const Poly& B
    ) const {
        Poly C;

        if (n == 1) {
            C.a[0] =
                mulm(
                    A.a[0],
                    B.a[0]
                );

            return C;
        }

        array<u128, 2 * MAXN - 1> acc{};

        for (int i = 0; i < n; ++i) {
            if (!A.a[i]) continue;

            for (int j = 0; j < n; ++j) {
                if (!B.a[j]) continue;

                acc[i + j] +=
                    (u64)A.a[i]
                    * B.a[j];
            }
        }

        array<uint32_t, 2 * MAXN - 1> conv{};

        for (int k = 0; k <= 2 * n - 2; ++k)
            conv[k] =
                (uint32_t)(
                    acc[k] % MOD
                );

        for (int j = 0; j < n; ++j) {
            u128 s = conv[j];

            for (
                int k = n;
                k <= 2 * n - 2;
                ++k
            ) {
                if (!conv[k]) continue;

                s +=
                    (u64)conv[k]
                    *
                    red[k - n][j];
            }

            C.a[j] =
                (uint32_t)(
                    s % MOD
                );
        }

        return C;
    }
};

/*
 * Segment tree storing
 *
 * product D_l ... D_r
 *
 * inside F[x]/(m_G).
 */
struct ProdTree {
    int K;
    const PolyRing* R;

    vector<Poly> tr;
    vector<Poly> D;

    ProdTree() = default;

    ProdTree(
        const PolyRing& ring,
        const vector<Poly>& D_
    ) :
        K((int)D_.size() - 1),
        R(&ring),
        D(D_)
    {
        tr.resize(4 * K + 4);
        build(1, 1, K);
    }

    void build(
        int p,
        int l,
        int r
    ) {
        if (l == r) {
            tr[p] = D[l];
            return;
        }

        int m = (l + r) >> 1;

        build(
            p << 1,
            l,
            m
        );

        build(
            p << 1 | 1,
            m + 1,
            r
        );

        tr[p] =
            R->mul(
                tr[p << 1],
                tr[p << 1 | 1]
            );
    }

    /*
     * cur *= product_{i != a,b} D_i.
     *
     * Only O(log K) whole segment-tree
     * nodes are used.
     */
    void applyExcept(
        int p,
        int l,
        int r,
        int a,
        int b,
        Poly& cur
    ) const {
        bool ha =
            l <= a && a <= r;

        bool hb =
            l <= b && b <= r;

        if (!ha && !hb) {
            cur =
                R->mul(
                    cur,
                    tr[p]
                );

            return;
        }

        if (l == r)
            return;

        int m =
            (l + r) >> 1;

        applyExcept(
            p << 1,
            l,
            m,
            a,
            b,
            cur
        );

        applyExcept(
            p << 1 | 1,
            m + 1,
            r,
            a,
            b,
            cur
        );
    }
};

/*
 * Return [h(G)]_{s,t}.
 *
 * G = P F P^{-1}.
 *
 * Hence
 *
 * h(G) = P h(F) P^{-1}.
 *
 * On each companion block we use Horner.
 * One companion-matrix times vector costs O(d).
 *
 * h has degree < M, and total block size is N,
 * so this function costs O(MN).
 */
static uint32_t evalEntry(
    const Poly& h,
    int M,
    int s,
    int t,
    const FrobeniusDecomp& fd
) {
    u128 ans = 0;

    for (const auto& bl : fd.blocks) {
        int d = bl.d;
        int off = bl.off;

        vector<uint32_t> w(
            d,
            0
        );

        /*
         * v =
         * Pinv[off .. off+d-1][t].
         *
         * Horner:
         *
         * w <- C w + h_i v.
         */
        for (int ii = M - 1; ii >= 0; --ii) {
            uint32_t last =
                w[d - 1];

            /*
             * Companion multiplication:
             *
             * y[0] = c0 * last
             * y[j] = old_w[j-1] + cj * last
             */
            for (
                int j = d - 1;
                j >= 1;
                --j
            ) {
                uint32_t z =
                    addm(
                        w[j - 1],
                        mulm(
                            bl.c[j],
                            last
                        )
                    );

                uint32_t ad =
                    mulm(
                        h.a[ii],
                        fd.Pinv[off + j][t]
                    );

                w[j] =
                    addm(z, ad);
            }

            uint32_t z =
                mulm(
                    bl.c[0],
                    last
                );

            w[0] =
                addm(
                    z,
                    mulm(
                        h.a[ii],
                        fd.Pinv[off][t]
                    )
                );
        }

        /*
         * e_s^T P times the result.
         */
        for (int j = 0; j < d; ++j)
            ans +=
                (u64)
                fd.P[s][off + j]
                *
                w[j];
    }

    return (uint32_t)(
        ans % MOD
    );
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N, K, Q;

    cin >> N >> K >> Q;

    Matrix G(
        N,
        vector<uint32_t>(N)
    );

    string str;

    for (int i = 0; i < N; ++i) {
        cin >> str;

        for (int j = 0; j < N; ++j)
            G[i][j] =
                str[j] - '0';
    }

    struct Query {
        int bs, s, bt, t;
    };

    vector<Query> queries(Q);

    for (auto& q : queries) {
        cin
            >> q.bs
            >> q.s
            >> q.bt
            >> q.t;

        --q.s;
        --q.t;
    }

    /*
     * ------------------------------------------------
     * Frobenius decomposition.
     *
     * G = P F P^{-1}.
     * ------------------------------------------------
     */
    FrobeniusDecomp fd(G);

    /*
     * The first block is the largest invariant
     * factor, hence exactly the minimal polynomial
     * of G.
     */
    vector<uint32_t> minpoly =
        FrobeniusDecomp::
            blockPoly(fd.blocks[0]);

    int M =
        (int)minpoly.size() - 1;

    PolyRing ring(
        M,
        minpoly
    );

    /*
     * ------------------------------------------------
     * Button generating functions.
     *
     * Y_0 = x
     * Y_i = Y_{i-1} + Y_{i-1}^2
     *
     * D_i = 1 + Y_{i-1}.
     * ------------------------------------------------
     */
    vector<Poly> Y(K + 1);
    vector<Poly> D(K + 1);

    Y[0] =
        ring.xpoly();

    for (int i = 1; i <= K; ++i) {
        Y[i] =
            ring.add(
                Y[i - 1],
                ring.mul(
                    Y[i - 1],
                    Y[i - 1]
                )
            );
    }

    for (int i = 1; i <= K; ++i) {
        D[i] =
            Y[i - 1];

        D[i].a[0] =
            addm(
                D[i].a[0],
                1
            );
    }

    /*
     * Product tree for
     *
     * product_{i != a,b} D_i.
     */
    ProdTree tree(
        ring,
        D
    );

    /*
     * ------------------------------------------------
     * Equal endpoint buttons:
     *
     * R_j = 1 + D_j^2 R_{j+1}
     *
     * D_j^2 = 1 + Y_{j-1} + Y_j
     *
     * H_{a,a}
     * =
     * 1
     * +
     * (Y_a - Y_{a-1}) R_{a+1}.
     * ------------------------------------------------
     */
    vector<Poly> R(K + 2);
    vector<Poly> eq(K + 1);

    Poly one =
        ring.one();

    for (int j = K; j >= 1; --j) {
        Poly D2 =
            ring.add(
                one,
                ring.add(
                    Y[j - 1],
                    Y[j]
                )
            );

        R[j] =
            ring.add(
                one,
                ring.mul(
                    D2,
                    R[j + 1]
                )
            );
    }

    for (int a = 1; a <= K; ++a) {
        Poly diff =
            ring.sub(
                Y[a],
                Y[a - 1]
            );

        eq[a] =
            ring.add(
                one,
                ring.mul(
                    diff,
                    R[a + 1]
                )
            );
    }

    /*
     * For a != b:
     *
     * H_{a,b}
     * =
     * x * product_{i != a,b} D_i.
     */
    Poly X =
        ring.xpoly();

    for (const auto& q : queries) {
        Poly h;

        if (q.bs == q.bt) {
            h =
                eq[q.bs];
        } else {
            h = X;

            tree.applyExcept(
                1,
                1,
                K,
                q.bs,
                q.bt,
                h
            );
        }

        cout
            << evalEntry(
                h,
                M,
                q.s,
                q.t,
                fd
            )
            << '\n';
    }

    return 0;
}

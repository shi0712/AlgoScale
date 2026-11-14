#include <bits/stdc++.h>
using namespace std;

#if defined(__x86_64__) || defined(__i386__)
#include <immintrin.h>
#define HAS_X86_SIMD 1
#else
#define HAS_X86_SIMD 0
#endif

using u32 = uint32_t;
using u64 = uint64_t;
using u128 = __uint128_t;

/*
    Reduction modulo a runtime constant.

    reciprocal = floor(2^64 / mod).
    The initial quotient is either exact or one too small.
*/
struct Modulo {
    u64 mod, reciprocal;

    explicit Modulo(u64 p) : mod(p), reciprocal(UINT64_MAX / p) {}

    inline u32 reduce(u64 x) const {
        u64 q = (u64)((u128)x * reciprocal >> 64);
        u64 r = x - q * mod;
        if (r >= mod) r -= mod;
        return (u32)r;
    }
};

using DotFunction =
    u32 (*)(const u32 *, const u32 *, int, const Modulo &);

/*
    Sixteen products fit in uint64_t because P <= 1.01 * 10^9.
    Thus only one modular reduction per sixteen products is needed.
*/
static u32 dotScalar(const u32 *a, const u32 *b, int length,
                     const Modulo &md) {
    u64 result = 0;

    while (length >= 16) {
        u64 s0 = 0, s1 = 0, s2 = 0, s3 = 0;
        for (int j = 0; j < 16; j += 4) {
            s0 += (u64)a[j] * b[j];
            s1 += (u64)a[j + 1] * b[j + 1];
            s2 += (u64)a[j + 2] * b[j + 2];
            s3 += (u64)a[j + 3] * b[j + 3];
        }
        result = md.reduce(result + s0 + s1 + s2 + s3);
        a += 16;
        b += 16;
        length -= 16;
    }

    u64 sum = result;
    while (length--) {
        sum += (u64)*a++ * *b++;
    }
    return md.reduce(sum);
}

#if HAS_X86_SIMD
__attribute__((target("avx2")))
static u32 dotAVX2(const u32 *a, const u32 *b, int length,
                   const Modulo &md) {
    u64 result = 0;

    while (length >= 16) {
        __m256i a0 = _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(a));
        __m256i b0 = _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(b));
        __m256i a1 = _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(a + 8));
        __m256i b1 = _mm256_loadu_si256(
            reinterpret_cast<const __m256i *>(b + 8));

        __m256i s = _mm256_mul_epu32(a0, b0);
        s = _mm256_add_epi64(
            s, _mm256_mul_epu32(_mm256_srli_epi64(a0, 32),
                               _mm256_srli_epi64(b0, 32)));
        s = _mm256_add_epi64(s, _mm256_mul_epu32(a1, b1));
        s = _mm256_add_epi64(
            s, _mm256_mul_epu32(_mm256_srli_epi64(a1, 32),
                               _mm256_srli_epi64(b1, 32)));

        __m128i total = _mm_add_epi64(
            _mm256_castsi256_si128(s),
            _mm256_extracti128_si256(s, 1));

        u64 lo = (u64)_mm_cvtsi128_si64(total);
        u64 hi = (u64)_mm_extract_epi64(total, 1);

        result = md.reduce(result + lo + hi);

        a += 16;
        b += 16;
        length -= 16;
    }

    u64 sum = result;
    while (length--) {
        sum += (u64)*a++ * *b++;
    }
    return md.reduce(sum);
}
#endif

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    u32 P;
    cin >> n >> P;

    if (n <= 2) {
        cout << 1 << '\n';
        return 0;
    }

    /*
        Left-child / right-sibling representation:

        The original tree becomes an increasing binary tree.
        Vertex 1 has no right child, so deleting it leaves an
        increasing binary tree on n-1 vertices.

        We count distinct preorders of such binary trees.
    */
    const int N = n - 1;
    const int stride = N + 1;
    Modulo md(P);

    DotFunction dot = dotScalar;
#if HAS_X86_SIMD
    __builtin_cpu_init();
    if (__builtin_cpu_supports("avx2")) {
        dot = dotAVX2;
    }
#endif

    vector<u32> inverse(N + 1);
    inverse[1] = 1;
    for (int i = 2; i <= N; ++i) {
        u32 product = md.reduce((u64)(P / i) * inverse[P % i]);
        inverse[i] = product == 0 ? 0 : P - product;
    }

    /*
        Let c(w) be the minimum number of increasing binary trees
        whose concatenated preorders form permutation w.

        If w = A, minimum, B, then
            c(w) = c(A) + max(1, c(B)-1),
        with c(empty) = 0.

        Define
            F(x,y) = sum_w x^|w| / |w|! * y^c(w),
            A(x)   = [y] F(x,y).

        The minimum-element decomposition gives
            F_x = F * ((F-1)/y + y + (y-1)A).

        For G = 1/F:
            y G_x = (1-y^2)G + (y-y^2)A G - 1.

        Also [y]G = -A.

        Write g[m][k] = [x^m y^k]G and
              d[m][k] = g[m][k] - g[m][k-1].

        Then, for k >= 1,
          (m+1)g[m+1][k]
            = g[m][k+1] - g[m][k-1]
              + sum_{j=0}^{m-1} a[m-j] d[j][k],

        where a[i] = [x^i]A = -g[i][1].

        Store d by columns, making each convolution a contiguous
        dot product. Its nonzero range starts at j = k-1.
    */

    vector<u32> differences((size_t)(N + 2) * stride, 0);

    auto column = [&](int k) -> u32 * {
        return differences.data() + (size_t)k * stride;
    };

    // G(0,y)=1, hence d[0][1]=-1.
    column(1)[0] = P - 1;

    vector<u32> previous(N + 3, 0);
    vector<u32> current(N + 3, 0);
    previous[0] = 1;

    // reversedA[N-i] = a[i].
    vector<u32> reversedA(N + 1, 0);

    u32 factorial = 1;
    u32 answerCoefficient = 0;

    for (int m = 0; m < N; ++m) {
        int next = m + 1;
        current[0] = 0;

        for (int k = 1; k <= next; ++k) {
            u32 convolution = 0;

            int first = k - 1;
            int length = m - first;

            if (length > 0) {
                convolution = dot(
                    reversedA.data() + (N - m + first),
                    column(k) + first,
                    length,
                    md);
            }

            u64 value = (u64)convolution + previous[k + 1] + P
                        - previous[k - 1];

            current[k] = md.reduce(
                (u64)md.reduce(value) * inverse[next]);
        }

        current[next + 1] = 0;

        answerCoefficient =
            current[1] == 0 ? 0 : P - current[1];
        reversedA[N - next] = answerCoefficient;

        for (int k = 1; k <= next + 1; ++k) {
            u32 left = current[k];
            u32 right = current[k - 1];
            column(k)[next] =
                left >= right ? left - right : left + P - right;
        }

        factorial = md.reduce((u64)factorial * next);
        previous.swap(current);
    }

    // A is an exponential generating function.
    cout << md.reduce((u64)answerCoefficient * factorial) << '\n';
    return 0;
}

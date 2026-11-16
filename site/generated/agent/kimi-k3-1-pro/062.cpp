#include <bits/stdc++.h>
#include <wmmintrin.h>
using namespace std;

using u32 = uint32_t;
using u64 = uint64_t;

static constexpr u32 MOD = 0x00400007U;
static u32 reduction[4][256];

__attribute__((target("pclmul")))
static inline u32 multiply(u32 a, u32 b) {
    __m128i x = _mm_set_epi64x(0, a);
    __m128i y = _mm_set_epi64x(0, b);
    u64 product = (u64)_mm_cvtsi128_si64(_mm_clmulepi64_si128(x, y, 0));
    u32 high = product >> 32;
    return (u32)product ^ reduction[0][high & 255] ^
           reduction[1][(high >> 8) & 255] ^
           reduction[2][(high >> 16) & 255] ^ reduction[3][high >> 24];
}

static u32 slowMultiply(u32 a, u32 b) {
    u64 result = 0;
    while (b) {
        if (b & 1) result ^= a;
        a = (a << 1) ^ ((a >> 31) ? MOD : 0);
        b >>= 1;
    }
    return (u32)result;
}

static u32 power(u32 a, u64 exponent) {
    u32 result = 1;
    while (exponent) {
        if (exponent & 1) result = multiply(result, a);
        a = multiply(a, a);
        exponent >>= 1;
    }
    return result;
}

struct SparseMatroidProduct {
    int rank;
    vector<array<int, 4>> edges;
    vector<u32> weight;

    void multiplyVector(const vector<u32>& input, vector<u32>& output) const {
        fill(output.begin(), output.end(), 0);
        for (int i = 0; i < (int)edges.size(); ++i) {
            auto [aU, aV, bU, bV] = edges[i];
            u32 value = 0;
            if (bU >= 0) value ^= input[bU];
            if (bV >= 0) value ^= input[bV];
            value = multiply(weight[i], value);
            if (aU >= 0) output[aU] ^= value;
            if (aV >= 0) output[aV] ^= value;
        }
    }
};

static u32 determinant(const SparseMatroidProduct& matrix, mt19937_64& rng) {
    int n = matrix.rank;
    if (n == 0) return 1;

    for (int attempt = 0; attempt < 8; ++attempt) {
        vector<u32> u(n), v(n), w(n), next(n);
        for (u32& x : u) x = (u32)rng();
        for (u32& x : v) x = (u32)rng();
        w = v;

        vector<u32> sequence;
        sequence.reserve(2 * n);
        for (int i = 0; i < 2 * n; ++i) {
            u32 value = 0;
            for (int j = 0; j < n; ++j) value ^= multiply(u[j], w[j]);
            sequence.push_back(value);
            matrix.multiplyVector(w, next);
            w.swap(next);
        }

        vector<u32> c(1, 1), b(1, 1), previous;
        int length = 0, shift = 1;
        u32 last = 1;
        for (int i = 0; i < 2 * n; ++i) {
            u32 discrepancy = sequence[i];
            for (int j = 1; j <= length; ++j) {
                discrepancy ^= multiply(c[j], sequence[i - j]);
            }
            if (discrepancy == 0) {
                ++shift;
                continue;
            }
            u32 factor = multiply(discrepancy, power(last, (1ULL << 32) - 2));
            if (2 * length <= i) {
                previous = c;
                c.resize(max(c.size(), b.size() + shift), 0);
                for (int j = 0; j < (int)b.size(); ++j) {
                    c[j + shift] ^= multiply(factor, b[j]);
                }
                length = i + 1 - length;
                b = previous;
                last = discrepancy;
                shift = 1;
            } else {
                c.resize(max(c.size(), b.size() + shift), 0);
                for (int j = 0; j < (int)b.size(); ++j) {
                    c[j + shift] ^= multiply(factor, b[j]);
                }
                ++shift;
            }
        }
        if (c[length] == 0) return 0;
        if (length == n) return c[length];
    }
    return 0;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 256; ++j) {
            u32 value = 0;
            for (int bit = 0; bit < 8; ++bit) {
                if ((j >> bit) & 1) {
                    u32 term = 1U << (8 * i + bit);
                    value ^= slowMultiply(term, MOD);
                }
            }
            reduction[i][j] = value;
        }
    }

    int tests;
    cin >> tests;
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    while (tests--) {
        int n, m;
        cin >> n >> m;
        vector<array<int, 4>> edges(m);
        for (auto& e : edges) {
            cin >> e[0] >> e[1] >> e[2] >> e[3];
            for (int& x : e) {
                if (--x == n - 1) x = -1;
            }
        }

        SparseMatroidProduct ordinary, common;
        ordinary.rank = common.rank = n - 1;
        ordinary.edges = common.edges = edges;
        for (auto& e : ordinary.edges) {
            e[2] = e[0];
            e[3] = e[1];
        }
        ordinary.weight.resize(m);
        common.weight.resize(m);

        bool answer = false;
        for (int repetition = 0; repetition < 1 && !answer; ++repetition) {
            for (int i = 0; i < m; ++i) {
                u32 x;
                do x = (u32)rng(); while (x == 0);
                ordinary.weight[i] = common.weight[i] = x;
            }
            u32 allA = determinant(ordinary, rng);
            u32 commonTrees = determinant(common, rng);
            if (allA != commonTrees) answer = true;
        }
        cout << (answer ? "YES" : "NO") << '\n';
    }
    return 0;
}

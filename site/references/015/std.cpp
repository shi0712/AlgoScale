#include <bits/stdc++.h>
#include <wmmintrin.h>
using namespace std;

#pragma GCC target("pclmul,sse2")

using u64 = uint64_t;

struct Key {
    u64 lo, hi;
    bool operator==(const Key& other) const {
        return lo == other.lo && hi == other.hi;
    }
};
static_assert(sizeof(Key) == 16);

// GF(2^128) with modulus x^128 + x^7 + x^2 + x + 1.
// Bit i is the coefficient of x^i. Addition is xor.
struct F {
    __m128i v;
    F() : v(_mm_setzero_si128()) {}
    explicit F(u64 x) : v(_mm_cvtsi64_si128((long long)x)) {}
    F(u64 lo, u64 hi)
        : v(_mm_set_epi64x((long long)hi, (long long)lo)) {}
    explicit F(__m128i x) : v(x) {}

    friend F operator^(F a, F b) {
        return F(_mm_xor_si128(a.v, b.v));
    }
    friend bool operator==(F a, F b) {
        return _mm_movemask_epi8(_mm_cmpeq_epi8(a.v, b.v)) == 0xffff;
    }
    Key key() const {
        return {(u64)_mm_cvtsi128_si64(v),
                (u64)_mm_cvtsi128_si64(_mm_srli_si128(v, 8))};
    }
};

static inline F reduce128(__m128i low, __m128i high) {
    // Fold high * (1 + x + x^2 + x^7) into the low half.
    __m128i r = _mm_xor_si128(high, _mm_slli_epi64(high, 1));
    r = _mm_xor_si128(r, _mm_slli_epi64(high, 2));
    r = _mm_xor_si128(r, _mm_slli_epi64(high, 7));

    __m128i carry = _mm_xor_si128(_mm_srli_epi64(high, 63),
                                  _mm_srli_epi64(high, 62));
    carry = _mm_xor_si128(carry, _mm_srli_epi64(high, 57));
    r = _mm_xor_si128(r, _mm_slli_si128(carry, 8));

    // At most seven bits overflow once more.
    __m128i e = _mm_srli_si128(carry, 8);
    __m128i t = _mm_xor_si128(e, _mm_slli_epi64(e, 1));
    t = _mm_xor_si128(t, _mm_slli_epi64(e, 2));
    t = _mm_xor_si128(t, _mm_slli_epi64(e, 7));
    return F(_mm_xor_si128(low, _mm_xor_si128(r, t)));
}

static inline F mul(F a, F b) {
    __m128i p0 = _mm_clmulepi64_si128(a.v, b.v, 0x00);
    __m128i p1 = _mm_clmulepi64_si128(a.v, b.v, 0x11);

    __m128i ax = _mm_xor_si128(a.v, _mm_shuffle_epi32(a.v, 0x4e));
    __m128i bx = _mm_xor_si128(b.v, _mm_shuffle_epi32(b.v, 0x4e));
    __m128i pm = _mm_clmulepi64_si128(ax, bx, 0x00);
    pm = _mm_xor_si128(pm, _mm_xor_si128(p0, p1));

    __m128i low = _mm_xor_si128(p0, _mm_slli_si128(pm, 8));
    __m128i high = _mm_xor_si128(p1, _mm_srli_si128(pm, 8));
    return reduce128(low, high);
}

static inline F sqr(F a) {
    __m128i low = _mm_clmulepi64_si128(a.v, a.v, 0x00);
    __m128i high = _mm_clmulepi64_si128(a.v, a.v, 0x11);
    return reduce128(low, high);
}

// Stable LSD radix sort: 16 byte passes, hence O(m) in the fixed 128-bit model.
static long long count_distinct(vector<Key>& a, vector<Key>& tmp) {
    const size_t m = a.size();
    if (m <= 1) return (long long)m;
    tmp.resize(m);

    for (int pass = 0; pass < 16; ++pass) {
        size_t cnt[256] = {};
        const int shift = (pass & 7) * 8;
        auto digit = [pass, shift](const Key& x) -> unsigned {
            const u64 w = pass < 8 ? x.lo : x.hi;
            return unsigned((w >> shift) & 255U);
        };
        for (const Key& x : a) ++cnt[digit(x)];

        size_t pref = 0;
        for (size_t& c : cnt) {
            const size_t old = c;
            c = pref;
            pref += old;
        }
        for (const Key& x : a) tmp[cnt[digit(x)]++] = x;
        a.swap(tmp);
    }

    long long res = 1;
    for (size_t i = 1; i < m; ++i) {
        res += !(a[i] == a[i - 1]);
    }
    return res;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // Fixed independently chosen evaluation point for reproducible official answers.
    // Its high half is nonzero, so it is not one of the 32-bit input vectors.
#ifndef ALT_Z
    const F z(0x243f6a8885a308d3ULL, 0x13198a2e03707344ULL);
#else
    const F z(0xa4093822299f31d0ULL, 0x082efa98ec4e6c89ULL);
#endif

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n;
        cin >> n;

        F c[32];
        int d = 0;
        bool has_zero = false;
        vector<Key> hashes[32];

        for (int r = 0; r < n; ++r) {
            u64 value;
            cin >> value;
            if (value == 0) {
                has_zero = true;
                continue;
            }

            F y(value);  // P_{U_k}(value) before processing old layer k.
            F next = y; // next coefficient in the new chain.
            F h = z;    // fingerprint of the new chain prefix.
            int k = 0;

            while (true) {
                // L_{c[k]}(y) = 0 iff y == 0 or y == c[k].  y != 0 here.
                if (k < d && y == c[k]) {
                    // Dimension k+1 and above are spaces already seen at r-1.
                    c[k] = next;
                    break;
                }

                // This positive-dimensional suffix space changed and must be recorded.
                h = sqr(h) ^ mul(next, h);
                if (hashes[k].empty()) hashes[k].reserve((size_t)n);
                hashes[k].push_back(h.key());

                if (k == d) {
                    c[d++] = next;
                    break;
                }

                const F old = c[k];
                c[k] = next;
                const F p = mul(old, y);
                next = sqr(old) ^ p;
                y = sqr(y) ^ p;
                ++k;
            }
        }

        long long answer = has_zero ? 1 : 0;
        vector<Key> tmp;
        for (int k = 0; k < d; ++k) {
            answer += count_distinct(hashes[k], tmp);
        }
        cout << answer << '\n';
    }
    return 0;
}

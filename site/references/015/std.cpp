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

// The polynomial value is only a bucket key.  Equal keys are always checked
// against the actual F_2 subspaces; a key collision cannot affect the answer.
struct Record {
    u64 bucket;
    uint32_t snapshot;
};
static_assert(sizeof(Record) == 16);
using BasisSnapshot = array<uint32_t, 32>;

static void radix_sort(vector<Record>& a, vector<Record>& tmp) {
    const size_t m = a.size();
    if (m <= 1) return;
    tmp.resize(m);
    for (int pass = 0; pass < 8; ++pass) {
        size_t cnt[256] = {};
        const int shift = pass * 8;
        auto digit = [shift](const Record& x) -> unsigned {
            return unsigned((x.bucket >> shift) & 255U);
        };
        for (const Record& x : a) ++cnt[digit(x)];
        size_t pref = 0;
        for (size_t& c : cnt) {
            const size_t old = c;
            c = pref;
            pref += old;
        }
        for (const Record& x : a) tmp[cnt[digit(x)]++] = x;
        a.swap(tmp);
    }
}

// Exact row-echelon basis.  A zero remainder is a deterministic membership
// certificate, not another hash test.
struct SpaceTester {
    uint32_t row[32] = {};
    uint32_t table[4][256];
    bool table_ready = false;

    SpaceTester(const BasisSnapshot& a, int dim) {
        for (int i = 0; i < dim; ++i) {
            uint32_t x = a[i];
            while (x) {
                int bit = 31 - __builtin_clz(x);
                if (!row[bit]) {
                    row[bit] = x;
                    break;
                }
                x ^= row[bit];
            }
        }
    }

    void reduce() {
        for (int bit = 0; bit < 32; ++bit) if (row[bit]) {
            for (int j = bit + 1; j < 32; ++j) {
                if ((row[j] >> bit) & 1U) row[j] ^= row[bit];
            }
        }
    }

    void prepare_table() {
        reduce();
        uint32_t column[32];
        for (int bit = 0; bit < 32; ++bit) {
            column[bit] = (1U << bit) ^ row[bit];
        }
        for (int block = 0; block < 4; ++block) {
            table[block][0] = 0;
            for (int x = 1; x < 256; ++x) {
                int bit = __builtin_ctz((unsigned)x);
                table[block][x] = table[block][x & (x - 1)]
                                  ^ column[8 * block + bit];
            }
        }
        table_ready = true;
    }

    bool contains(const BasisSnapshot& a, int dim) const {
        if (table_ready) {
            for (int i = 0; i < dim; ++i) {
                uint32_t x = a[i];
                if (table[0][x & 255U] ^ table[1][(x >> 8) & 255U]
                    ^ table[2][(x >> 16) & 255U] ^ table[3][x >> 24]) {
                    return false;
                }
            }
        } else {
            for (int i = 0; i < dim; ++i) {
                uint32_t x = a[i];
                while (x) {
                    int bit = 31 - __builtin_clz(x);
                    if (!row[bit]) return false;
                    x ^= row[bit];
                }
            }
        }
        return true;
    }

    void append_canonical(vector<uint32_t>& out) {
        reduce();
        for (int bit = 31; bit >= 0; --bit) if (row[bit]) out.push_back(row[bit]);
    }
};

// Exact MSD radix partition of complete canonical bases.  Common prefixes are
// skipped in word-sized chunks; identical keys need only one linear scan.
static long long count_canonical(const vector<uint32_t>& keys, int dim) {
    const size_t count = keys.size() / dim;
    const int width = dim * int(sizeof(uint32_t));
    vector<uint32_t> order(count);
    iota(order.begin(), order.end(), 0U);
    const auto* data = reinterpret_cast<const unsigned char*>(keys.data());

    auto solve = [&](auto&& self, size_t left, size_t right, int offset) -> long long {
        if (right - left <= 1) return (long long)(right - left);
        const auto* first = data + (size_t)order[left] * width;
        int common = width - offset;
        for (size_t i = left + 1; i < right && common; ++i) {
            const auto* other = data + (size_t)order[i] * width;
            int j = 0;
            while (j + 8 <= common) {
                u64 a, b;
                memcpy(&a, first + offset + j, 8);
                memcpy(&b, other + offset + j, 8);
                if (a != b) {
                    j += __builtin_ctzll(a ^ b) / 8;
                    break;
                }
                j += 8;
            }
            while (j < common && first[offset + j] == other[offset + j]) ++j;
            common = j;
        }
        offset += common;
        if (offset == width) return 1;

        size_t freq[256] = {}, begin[256], next[256];
        for (size_t i = left; i < right; ++i) {
            ++freq[data[(size_t)order[i] * width + offset]];
        }
        size_t p = left;
        for (int b = 0; b < 256; ++b) {
            begin[b] = next[b] = p;
            p += freq[b];
        }
        for (int b = 0; b < 256; ++b) {
            const size_t end = begin[b] + freq[b];
            while (next[b] < end) {
                unsigned target = data[(size_t)order[next[b]] * width + offset];
                if (target == (unsigned)b) ++next[b];
                else swap(order[next[b]], order[next[target]++]);
            }
        }
        long long answer = 0;
        for (int b = 0; b < 256; ++b) if (freq[b]) {
            answer += self(self, begin[b], begin[b] + freq[b], offset + 1);
        }
        return answer;
    };
    return solve(solve, 0, count, 0);
}

static long long exact_bucket(const vector<Record>& records, size_t left, size_t right,
                              int dim, const vector<BasisSnapshot>& snapshots) {
    if (right - left <= 1) return (long long)(right - left);
    const BasisSnapshot& first = snapshots[records[left].snapshot];
    const size_t bytes = (size_t)dim * sizeof(uint32_t);
    size_t pos = left + 1;
    while (pos < right && memcmp(first.data(), snapshots[records[pos].snapshot].data(), bytes) == 0) ++pos;
    if (pos == right) return 1;

    SpaceTester tester(first, dim);
    if ((right - pos) * (size_t)dim >= 2048) tester.prepare_table();
    for (; pos < right; ++pos) {
        const BasisSnapshot& candidate = snapshots[records[pos].snapshot];
        if (memcmp(first.data(), candidate.data(), bytes) == 0 || tester.contains(candidate, dim)) continue;

        // A genuine polynomial-key collision.  Keep complete canonical bases
        // and use exact bytewise radix partitioning for the remaining classes.
        vector<uint32_t> canonical;
        canonical.reserve((right - pos + 1) * (size_t)dim);
        tester.append_canonical(canonical);
        for (size_t i = pos; i < right; ++i) {
            const BasisSnapshot& current = snapshots[records[i].snapshot];
            if (memcmp(first.data(), current.data(), bytes) == 0 || tester.contains(current, dim)) continue;
            SpaceTester other(current, dim);
            other.append_canonical(canonical);
        }
        return count_canonical(canonical, dim);
    }
    return 1;
}

static long long count_distinct(vector<Record>& records, vector<Record>& tmp,
                                int dim, const vector<BasisSnapshot>& snapshots) {
    radix_sort(records, tmp);
    long long result = 0;
    for (size_t left = 0; left < records.size();) {
        size_t right = left + 1;
        while (right < records.size() && records[left].bucket == records[right].bucket) ++right;
        result += exact_bucket(records, left, right, dim, snapshots);
        left = right;
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    // This fixed evaluation point only groups candidates.  Correctness does
    // not assume it is random, collision-free, or outside the input space.
    const F z(0x243f6a8885a308d3ULL, 0x13198a2e03707344ULL);

    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n;
        cin >> n;
        F c[32];
        int d = 0;
        bool has_zero = false, has_full = false;
        vector<Record> records[31];
        vector<BasisSnapshot> snapshots;
        snapshots.reserve((size_t)n);
        BasisSnapshot generators{};

        for (int r = 0; r < n; ++r) {
            u64 value;
            cin >> value;
            if (value == 0) {
                has_zero = true;
                continue;
            }
            F y(value), next = y, h = z;
            const uint32_t snapshot = (uint32_t)snapshots.size();
            int k = 0;
            bool changed = false;
            while (true) {
                if (k < d && y == c[k]) {
                    c[k] = next;
                    break;
                }
                h = sqr(h) ^ mul(next, h);
                changed = true;
                if (k == 31) {
                    has_full = true;  // Only one 32-dimensional subspace exists.
                } else {
                    if (records[k].empty()) records[k].reserve((size_t)n);
                    const Key key = h.key();
                    const u64 bucket = key.lo ^ (key.hi * 0x9e3779b97f4a7c15ULL);
                    records[k].push_back({bucket, snapshot});
                }
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
            if (changed) {
                // x is inserted at the front.  At the first dependent layer
                // the old k-th generator is discarded; all later ones stay.
                for (int j = k; j > 0; --j) generators[j] = generators[j - 1];
                generators[0] = (uint32_t)value;
                snapshots.push_back(generators);
            }
        }

        long long answer = (has_zero ? 1 : 0) + (has_full ? 1 : 0);
        vector<Record> tmp;
        for (int k = 0; k < min(d, 31); ++k) {
            answer += count_distinct(records[k], tmp, k + 1, snapshots);
            vector<Record>().swap(records[k]);
        }
        cout << answer << '\n';
    }
    return 0;
}

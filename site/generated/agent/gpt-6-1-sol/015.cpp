#include <bits/stdc++.h>
using namespace std;

using u32 = uint32_t;
using u64 = uint64_t;

class FastInput {
    static constexpr size_t SZ = 1 << 16;
    char buf[SZ];
    size_t at = 0, len = 0;
public:
    inline char get() {
        if (at == len) {
            len = fread(buf, 1, SZ, stdin);
            at = 0;
            if (!len) return 0;
        }
        return buf[at++];
    }
    u32 next() {
        char c;
        do { c = get(); } while (c <= ' ' && c);
        u32 x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = get();
        }
        return x;
    }
};

// A random linear functional of the 32 by 32 reduced basis matrix.
// Hashes are only an index: every matching hash is checked exactly.
alignas(64) static u64 rowHash[32][4][256];
alignas(64) static u64 colHash[32][4][256];

static u64 splitmix(u64 &s) {
    u64 z = (s += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}

static void initHashes() {
    u64 seed = chrono::steady_clock::now().time_since_epoch().count();
    u64 coeff[32][32];
    for (int i = 0; i < 32; ++i)
        for (int j = 0; j < 32; ++j) coeff[i][j] = splitmix(seed);
    for (int i = 0; i < 32; ++i) {
        for (int byte = 0; byte < 4; ++byte) {
            for (int v = 1; v < 256; ++v) {
                int bit = __builtin_ctz((unsigned)v);
                rowHash[i][byte][v] = rowHash[i][byte][v & (v - 1)]
                    ^ coeff[i][8 * byte + bit];
                colHash[i][byte][v] = colHash[i][byte][v & (v - 1)]
                    ^ coeff[8 * byte + bit][i];
            }
        }
    }
}

template<int B> inline u64 hashWord(const u64 tab[4][256], u32 x) {
    u64 h = tab[0][x & 255];
    if constexpr (B >= 2) h ^= tab[1][(x >> 8) & 255];
    if constexpr (B >= 3) h ^= tab[2][(x >> 16) & 255];
    if constexpr (B >= 4) h ^= tab[3][x >> 24];
    return h;
}

struct ReducedBasis {
    u32 row[32] = {}, col[32] = {};
    u32 pivots = 0;
    u64 hash = 0;

    inline u32 reduce(u32 x) const {
        u32 mask = x & pivots;
        while (mask) {
            int p = __builtin_ctz(mask);
            x ^= row[p];
            mask &= mask - 1;
        }
        return x;
    }

    inline void addWithoutHash(u32 x) {
        int p = 31 - __builtin_clz(x);
        u32 changed = col[p] | (u32(1) << p);
        pivots |= u32(1) << p;
        u32 m = changed;
        while (m) {
            int q = __builtin_ctz(m);
            row[q] ^= x;
            m &= m - 1;
        }
        m = x;
        while (m) {
            int q = __builtin_ctz(m);
            col[q] ^= changed;
            m &= m - 1;
        }
    }

    template<int B> inline void addByRows(u32 x, u32 changed) {
        u32 m = changed;
        while (m) {
            int q = __builtin_ctz(m);
            row[q] ^= x;
            hash ^= hashWord<B>(rowHash[q], x);
            m &= m - 1;
        }
        m = x;
        while (m) {
            int q = __builtin_ctz(m);
            col[q] ^= changed;
            m &= m - 1;
        }
    }
    template<int B> inline void addByCols(u32 x, u32 changed) {
        u32 m = changed;
        while (m) {
            int q = __builtin_ctz(m);
            row[q] ^= x;
            m &= m - 1;
        }
        m = x;
        while (m) {
            int q = __builtin_ctz(m);
            col[q] ^= changed;
            hash ^= hashWord<B>(colHash[q], changed);
            m &= m - 1;
        }
    }

    // x has already been reduced, and is nonzero.
    inline void add(u32 x, int rank) {
        int p = 31 - __builtin_clz(x);
        u32 changed = col[p] | (u32(1) << p);
        pivots |= u32(1) << p;
        int bx = (p >> 3) + 1;
        int bc = ((31 - __builtin_clz(changed)) >> 3) + 1;
        if (rank * bx <= (34 - rank) * bc) {
            switch (bx) {
                case 1: addByRows<1>(x, changed); break;
                case 2: addByRows<2>(x, changed); break;
                case 3: addByRows<3>(x, changed); break;
                default: addByRows<4>(x, changed);
            }
        } else {
            switch (bc) {
                case 1: addByCols<1>(x, changed); break;
                case 2: addByCols<2>(x, changed); break;
                case 3: addByCols<3>(x, changed); break;
                default: addByCols<4>(x, changed);
            }
        }
    }
};

class SpaceSet {
    struct CacheEntry {
        u32 row[32];
    };
    static constexpr u32 CACHED = 1u << 31;
    const vector<u32> &a;
    const u32 *history;
    int dimension;
    u32 slots;
    u64 *keys;
    u32 *reps;
    unique_ptr<CacheEntry[]> cache;
    u32 cacheCapacity, cacheUsed = 0;
    ReducedBasis verification;
    u32 verificationEnd = UINT32_MAX;
    u32 verificationCurrent = UINT32_MAX;
    int verificationRank = 0;
    int directRank = 0;
    int sameSetRank = 0;
    bool directEqual = true;
    u32 generators[64] = {};
    u32 generatorsEnd = UINT32_MAX;
    int generatorsRank = 0;

    static inline u32 generatorBucket(u32 x) {
        return ((x ^ (x >> 16)) * 0x9e3779b9u) >> 26;
    }

    bool sameGenerators(const u32 *indices, const u32 *current,
                        int right, int rank) {
        if (generatorsEnd != u32(right)) {
            memset(generators, 0, sizeof(generators));
            generatorsEnd = right;
            generatorsRank = 0;
        }
        while (generatorsRank < rank) {
            u32 x = a[current[generatorsRank++]];
            u32 at = generatorBucket(x);
            while (generators[at]) at = (at + 1) & 63;
            generators[at] = x;
        }
        while (sameSetRank < rank) {
            int j = sameSetRank;
            u32 x = a[indices[j]], at = generatorBucket(x);
            while (generators[at] != x) {
                if (!generators[at]) return false;
                at = (at + 1) & 63;
            }
            ++sameSetRank;
        }
        return true;
    }

    // Return 2 for identical generator sets, 1 for equal reduced bases.
    inline int equal(u32 rep, const ReducedBasis &basis, int right, int rank) {
        u32 code = rep - 1;
        if (int(code & 31) + 1 != rank) return false;
        u32 end = code >> 5;
        if (end != verificationEnd || u32(right) != verificationCurrent
                || rank < verificationRank || rank < directRank) {
            verification = ReducedBasis();
            verificationEnd = end;
            verificationCurrent = right;
            verificationRank = 0;
            directRank = 0;
            sameSetRank = 0;
            directEqual = true;
        }
        const u32 *indices = history + size_t(end) * dimension;
        const u32 *current = history + size_t(right) * dimension;
        while (directEqual && directRank < rank) {
            directEqual = (a[indices[directRank]] == a[current[directRank]]);
            ++directRank;
        }
        if (directEqual) {
            sameSetRank = rank;
            return 2;
        }
        bool reversed = true;
        for (int j = 0; j < rank; ++j) {
            if (a[indices[j]] != a[current[rank - 1 - j]]) {
                reversed = false;
                break;
            }
        }
        if (reversed) {
            sameSetRank = rank;
            return 2;
        }
        if (sameGenerators(indices, current, right, rank)) return 2;
        while (verificationRank < rank) {
            u32 x = verification.reduce(a[indices[verificationRank++]]);
            verification.addWithoutHash(x);
        }
        return memcmp(verification.row, basis.row, sizeof(basis.row)) == 0;
    }

    void remember(u32 slot, const ReducedBasis &basis) {
        if (cacheUsed == cacheCapacity) return;
        if (!cache) cache.reset(new CacheEntry[cacheCapacity]);
        u32 c = cacheUsed++;
        memcpy(cache[c].row, basis.row, sizeof(basis.row));
        reps[slot] = CACHED | c;
    }

public:
    SpaceSet(const vector<u32> &values, const u32 *hist, int d, u64 bound)
        : a(values), history(hist), dimension(d) {
        slots = max<u64>(16, (bound * 5 + 2) / 3 + 1);
        keys = static_cast<u64 *>(malloc(size_t(slots) * sizeof(u64)));
        reps = static_cast<u32 *>(calloc(slots, sizeof(u32)));
        cacheCapacity = min<u64>(262144, max<u64>(1, bound));
    }
    ~SpaceSet() { free(keys); free(reps); }

    inline bool insert(const ReducedBasis &basis, int right, int rank) {
        u64 h = basis.hash;
        u32 slot = (u64(u32(h >> 32)) * slots) >> 32;
        for (;;) {
            u32 rep = reps[slot];
            if (!rep) {
                keys[slot] = h;
                reps[slot] = (u32(right) << 5) + rank;
                return true;
            }
            if (keys[slot] == h) {
                if (rep & CACHED) {
                    if (memcmp(cache[rep & ~CACHED].row, basis.row,
                               sizeof(basis.row)) == 0) return false;
                } else {
                    int match = equal(rep, basis, right, rank);
                    if (match) {
                        if (match == 1) remember(slot, basis);
                        return false;
                    }
                }
            }
            if (++slot == slots) slot = 0;
        }
    }
};

static u64 solve(const vector<u32> &a) {
    int n = a.size();
    u32 global[32] = {};
    int d = 0;
    bool zero = false;
    for (u32 x : a) {
        zero |= (x == 0);
        while (x) {
            int p = 31 - __builtin_clz(x);
            if (!global[p]) { global[p] = x; ++d; break; }
            x ^= global[p];
        }
    }
    if (d == 0) return 1;
    // The full space is always attained. The zero space is attained iff a zero
    // occurs. All other spaces are enumerated below.
    u64 answer = 1 + zero;
    if (d == 1) return answer;

    int period, skipAt = 0, skipLength = 0;
    {
        vector<int> pi(n);
        for (int i = 1; i < n; ++i) {
            int j = pi[i - 1];
            while (j && a[i] != a[j]) j = pi[j - 1];
            if (a[i] == a[j]) ++j;
            pi[i] = j;
        }
        period = n - pi[n - 1];
        int end = min(n, 2 * period - 1);
        for (int length = 3; length <= end; ++length) {
            int p = length - pi[length - 1];
            if (length / p >= 3) {
                int save = (length / p - 2) * p;
                if (save > skipLength) {
                    skipLength = save;
                    skipAt = 2 * p;
                }
            }
        }
    }
    // Every period-length window spans the full space. Thus a proper space
    // comes from fewer than period entries, and can be shifted into this prefix.
    int limit = min(n, 2 * period - 1);
    unique_ptr<u32[]> history(new u32[size_t(limit) * d]);
    SpaceSet seen(a, history.get(), d,
                  u64(min(period, limit - skipLength)) * (d - 1));
    u32 latest[32];
    int len = 0;

    // latest contains the greedy independent subsequence obtained by scanning
    // backwards. Its first k vectors span the unique suffix space of rank k.
    for (int r = 0; r < limit; ++r) {
        if (skipLength && r == skipAt) {
            // Two copies already contain every space from this periodic prefix.
            // Jump whole copies; the greedy basis has the same values and its
            // selected positions simply move forward by the skipped length.
            for (int j = 0; j < len; ++j) latest[j] += skipLength;
            r += skipLength;
            if (r == limit) break;
        }
        u32 *hist = history.get() + size_t(r) * d;
        if (a[r] == 0) continue;
        ReducedBasis basis;
        u32 x = a[r];
        int rank = 0, used = 0;
        hist[0] = r;
        for (;;) {
            if (++rank == d) break;
            basis.add(x, rank);
            if (used == len) {
                answer += seen.insert(basis, r, rank);
                break;
            }
            x = basis.reduce(a[latest[used]]);
            if (!x) break;
            answer += seen.insert(basis, r, rank);
            hist[rank] = latest[used++];
        }
        // The first dependent old vector is discarded; older independent
        // vectors remain in their previous order.
        if (used == len) ++len;
        for (int j = used; j > 0; --j) latest[j] = latest[j - 1];
        latest[0] = r;
        memcpy(hist, latest, len * sizeof(u32));
    }
    return answer;
}

int main() {
    FastInput in;
    initHashes();
    u32 tests = in.next();
    string output;
    for (u32 tc = 0; tc < tests; ++tc) {
        int n = in.next();
        vector<u32> a(n);
        for (u32 &x : a) x = in.next();
        output += to_string(solve(a));
        output += '\n';
    }
    fwrite(output.data(), 1, output.size(), stdout);
}

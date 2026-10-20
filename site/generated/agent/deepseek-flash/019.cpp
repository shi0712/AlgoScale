// 区间异或空间
//
// Count the number of distinct subspaces span(a_l..a_r) over all non-empty
// intervals [l,r] (vectors are 32-bit, span over GF(2)).
//
// Observation: for fixed r, the spans of the suffixes a_l..a_r (l = r..1) form
// a nested chain, and every strict inclusion raises the dimension by at least
// one, so there are at most 33 distinct spans per r.
//
// Transition r-1 -> r:  span(a_l..a_r) = span(a_l..a_{r-1}) + span(a_r), so the
// new chain is  { span(a_r) }  union  { c + span(a_r) : c in old chain }.
// Because the chain is nested, two elements are equal iff their dimensions are
// equal, which gives a cheap de-duplication.
//
// Each chain element is kept in reduced row echelon form (canonical!) together
// with a 64-bit fingerprint  xor_p (row_p * K_p).  Adding a vector only needs
// the new pivot to be eliminated from the other rows.
//
// Globally the distinct fingerprints are counted by radix sorting them.

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <algorithm>

typedef uint32_t u32;
typedef uint64_t u64;

static const int BW = 32;      // bit width
static const int MAXC = 40;    // >= 33 chain elements + slack

struct Entry {
    u32 row[BW];   // row[p]: pivot p (highest set bit), zeros at the other pivots
    u64 hash;      // xor over pivots p of (u64)row[p] * HC[p]
    u32 mask;      // bit p set iff pivot p exists
    u32 dim;
};

static u64 HC[BW];
static inline u64 mixc(int p, u32 x) { return (u64)x * HC[p]; }

static Entry bufA[MAXC], bufB[MAXC];

// ---------------- fast input ----------------
static char *ibuf = nullptr;
static size_t ipos = 0, ilen = 0;
static const size_t IBUFSZ = 1 << 22;

static inline int gc() {
    if (ipos == ilen) {
        ilen = fread(ibuf, 1, IBUFSZ, stdin);
        ipos = 0;
        if (!ilen) return -1;
    }
    return (int)(unsigned char)ibuf[ipos++];
}
static inline u32 readUInt() {
    int c = gc();
    while (c <= ' ') { if (c < 0) return 0; c = gc(); }
    u32 r = 0;
    while (c > ' ') { r = r * 10u + (u32)(c - '0'); c = gc(); }
    return r;
}

// ---------------- candidate storage ----------------
static u64 *cand = nullptr, *scratch = nullptr;
static size_t candCap = 0;

static void ensureCap(size_t need) {
    if (need <= candCap) return;
    free(cand); free(scratch);
    candCap = need;
    cand = (u64*)malloc(candCap * sizeof(u64));
    scratch = (u64*)malloc(candCap * sizeof(u64));
    if (!cand || !scratch) { fprintf(stderr, "alloc fail\n"); exit(1); }
}

// ---------------- sorting ----------------
static void insertionSort(u64 *a, int n) {
    for (int i = 1; i < n; i++) {
        u64 v = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > v) { a[j + 1] = a[j]; j--; }
        a[j + 1] = v;
    }
}

// LSD radix sort on the low `bits` bits, 11 bits per pass.
static u64 *radixSort(u64 *a, u64 *b, int n, int bits) {
    const int D = 11, NB = 1 << D, M = NB - 1;
    static int cnt[NB];
    for (int sh = 0; sh < bits; sh += D) {
        memset(cnt, 0, sizeof(cnt));
        for (int i = 0; i < n; i++) cnt[(a[i] >> sh) & M]++;
        int s = 0;
        for (int i = 0; i < NB; i++) { int t = cnt[i]; cnt[i] = s; s += t; }
        for (int i = 0; i < n; i++) b[cnt[(a[i] >> sh) & M]++] = a[i];
        u64 *tmp = a; a = b; b = tmp;
    }
    return a;
}

// a[] is sorted by its low SORTBITS bits; count distinct 64-bit values.
static const int SORTBITS = 44;
static const u64 SORTMASK = (1ULL << SORTBITS) - 1;

static int countDistinctSorted(u64 *a, int n) {
    int c = 0, i = 0;
    while (i < n) {
        int j = i + 1;
        u64 key = a[i] & SORTMASK;
        while (j < n && (a[j] & SORTMASK) == key) j++;
        int len = j - i;
        if (len == 1) {
            c++;
        } else if (len <= 32) {
            for (int k = i; k < j; k++) {
                bool dup = false;
                for (int t = i; t < k; t++) if (a[t] == a[k]) { dup = true; break; }
                if (!dup) c++;
            }
        } else {
            // Long run: usually all entries are the same fingerprint (a genuine
            // duplicate), which a linear scan settles in one pass.
            u64 v = a[i];
            int k = i + 1;
            while (k < j && a[k] == v) k++;
            if (k == j) {
                c++;
            } else {   // mixed run: sort it properly
                std::sort(a + i, a + j);
                c++;
                for (k = i + 1; k < j; k++) if (a[k] != a[k - 1]) c++;
            }
        }
        i = j;
    }
    return c;
}

static int countDistinctSortedExact(const u64 *a, int n) {
    if (n <= 0) return 0;
    int c = 1;
    for (int i = 1; i < n; i++) if (a[i] != a[i - 1]) c++;
    return c;
}

// ---------------- output ----------------
static char obuf[1 << 16];
static int olen = 0;
static inline void outNum(u64 v) {
    char t[24];
    int k = 0;
    if (v == 0) t[k++] = '0';
    while (v) { t[k++] = char('0' + (v % 10)); v /= 10; }
    while (k) {
        if (olen == (int)sizeof(obuf)) { fwrite(obuf, 1, olen, stdout); olen = 0; }
        obuf[olen++] = t[--k];
    }
    if (olen == (int)sizeof(obuf)) { fwrite(obuf, 1, olen, stdout); olen = 0; }
    obuf[olen++] = '\n';
}

int main() {
    {   // deterministic pseudo-random odd multipliers, one per pivot
        u64 s = 0x243F6A8885A308D3ULL;
        for (int i = 0; i < BW; i++) {
            s += 0x9E3779B97F4A7C15ULL;
            u64 z = s;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
            z = z ^ (z >> 31);
            HC[i] = z | 1ULL;
        }
    }

    ibuf = (char*)malloc(IBUFSZ);
    if (!ibuf) return 1;

    u32 T = readUInt();

    for (u32 tc = 0; tc < T; tc++) {
        int n = (int)readUInt();
        ensureCap((size_t)33 * (size_t)n + 64);

        int nc = 0;
        bool hasZero = false;
        int cnt = 0;
        Entry *cur = bufA, *nxt = bufB;

        for (int r = 0; r < n; r++) {
            u32 x = readUInt();

            // smallest span for this r: span(a_r)
            Entry &e0 = nxt[0];
            if (x == 0) {
                e0.mask = 0; e0.dim = 0; e0.hash = 0;
                hasZero = true;
            } else {
                int p = 31 - __builtin_clz(x);
                e0.row[p] = x; e0.mask = 1u << p; e0.dim = 1;
                e0.hash = mixc(p, x);
                cand[nc++] = e0.hash;
            }
            int knew = 1;
            u32 res = x;

            for (int i = 0; i < cnt; i++) {
                Entry &o = cur[i];
                // reduce the residual by the old basis (RREF: the rows have
                // zeros at the other pivots, so one pass suffices)
                u32 m = res & o.mask;
                while (m) { int p = __builtin_ctz(m); m &= m - 1; res ^= o.row[p]; }

                Entry &e = nxt[knew];
                memcpy(e.row, o.row, sizeof(o.row));
                e.hash = o.hash; e.mask = o.mask; e.dim = o.dim;

                bool changed = (res != 0);
                if (changed) {
                    int p = 31 - __builtin_clz(res);
                    u32 mk = e.mask;
                    u64 h = e.hash;
                    while (mk) {
                        int q = __builtin_ctz(mk); mk &= mk - 1;
                        u32 rw = e.row[q];
                        u32 nm = (u32)(-(int)((rw >> p) & 1u));
                        u32 nw = rw ^ (res & nm);
                        h ^= mixc(q, rw) ^ mixc(q, nw);
                        e.row[q] = nw;
                    }
                    e.row[p] = res;
                    e.mask |= 1u << p;
                    e.dim++;
                    e.hash = h ^ mixc(p, res);
                }

                if (e.dim > nxt[knew - 1].dim) {
                    // unchanged subspaces were already recorded last round
                    if (changed && e.hash) cand[nc++] = e.hash;
                    else if (!e.hash) hasZero = true;
                    knew++;
                }
            }

            Entry *tmp = cur; cur = nxt; nxt = tmp;
            cnt = knew;
        }

        int distinct;
        if (nc <= 32) {
            insertionSort(cand, nc);
            distinct = countDistinctSortedExact(cand, nc);
        } else if (nc <= 2048) {
            std::sort(cand, cand + nc);
            distinct = countDistinctSortedExact(cand, nc);
        } else {
            u64 *p = radixSort(cand, scratch, nc, SORTBITS);
            distinct = countDistinctSorted(p, nc);
        }
        if (hasZero) distinct++;
        outNum((u64)distinct);
    }

    if (olen) fwrite(obuf, 1, olen, stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

static uint64_t SEED;

inline uint64_t F(uint32_t v) {
    uint64_t z = (uint64_t)v + SEED;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    z = z ^ (z >> 31);
    return z;
}

struct Basis {
    uint32_t b[32];
    uint64_t h;
    uint32_t mask;
    uint8_t rank;
};

inline bool insertBasis(Basis &B, uint32_t x) {
    uint32_t v = x;

    // Reduce by all current pivot bits.
    // Since B is kept in RREF, each b[q] has zeros at all other pivot positions.
    uint32_t t = v & B.mask;
    while (t) {
        int q = __builtin_ctz(t);
        v ^= B.b[q];
        t &= t - 1;
    }

    if (v == 0) return false;

    int p = 31 - __builtin_clz(v);
    uint32_t bit = 1u << p;

    B.h ^= F(v);

    // Clear the new pivot bit from all higher pivot rows.
    for (int q = p + 1; q < 32; ++q) {
        if (B.b[q] & bit) {
            uint64_t oldh = F(B.b[q]);
            B.b[q] ^= v;
            B.h ^= oldh ^ F(B.b[q]);
        }
    }

    B.b[p] = v;
    B.mask |= bit;
    ++B.rank;
    return true;
}

inline bool sameBasis(const Basis &A, const Basis &B) {
    if (A.h != B.h || A.rank != B.rank) return false;
    return memcmp(A.b, B.b, sizeof(A.b)) == 0;
}

void sortHashes(vector<uint64_t> &a, vector<uint64_t> &tmp) {
    size_t n = a.size();

    if (n < 200000) {
        sort(a.begin(), a.end());
        return;
    }

    tmp.resize(n);

    uint32_t cnt[256];
    uint32_t pos[256];

    for (int pass = 0; pass < 8; ++pass) {
        memset(cnt, 0, sizeof(cnt));
        int shift = pass * 8;

        for (uint64_t x : a) {
            ++cnt[(x >> shift) & 255];
        }

        uint32_t sum = 0;
        for (int i = 0; i < 256; ++i) {
            pos[i] = sum;
            sum += cnt[i];
        }

        for (uint64_t x : a) {
            int k = (x >> shift) & 255;
            tmp[pos[k]++] = x;
        }

        a.swap(tmp);
    }
}

class FastScanner {
    static constexpr int SZ = 1 << 20;
    char buf[SZ];
    int idx = 0, size = 0;

    inline char getChar() {
        if (idx >= size) {
            size = (int)fread(buf, 1, SZ, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }

public:
    uint32_t nextUInt() {
        char c;
        do {
            c = getChar();
        } while (c < '0' || c > '9');

        uint32_t x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }

    int nextInt() {
        return (int)nextUInt();
    }
};

class FastOutput {
    static constexpr int SZ = 1 << 20;
    char buf[SZ];
    int idx = 0;

    inline void putChar(char c) {
        if (idx == SZ) flush();
        buf[idx++] = c;
    }

public:
    ~FastOutput() {
        flush();
    }

    inline void flush() {
        if (idx) {
            fwrite(buf, 1, idx, stdout);
            idx = 0;
        }
    }

    void writeLongLong(long long x) {
        if (x == 0) {
            putChar('0');
            putChar('\n');
            return;
        }

        char s[24];
        int n = 0;

        while (x > 0) {
            s[n++] = char('0' + x % 10);
            x /= 10;
        }

        while (n--) putChar(s[n]);
        putChar('\n');
    }
};

int main() {
    SEED = (uint64_t)chrono::steady_clock::now().time_since_epoch().count();
    SEED ^= (uint64_t)(uintptr_t)&SEED;

    FastScanner fs;
    FastOutput out;

    int T = fs.nextInt();

    vector<uint64_t> vals;
    vector<uint64_t> tmp;
    vals.reserve(33100000);

    while (T--) {
        int n = fs.nextInt();

        vals.clear();

        Basis arr1[35], arr2[35];
        Basis *cur = arr1;
        Basis *nxt = arr2;
        int cnt = 0;

        for (int r = 0; r < n; ++r) {
            uint32_t x = fs.nextUInt();

            Basis sg;
            memset(sg.b, 0, sizeof(sg.b));
            sg.h = 0;
            sg.mask = 0;
            sg.rank = 0;

            if (x != 0) {
                int p = 31 - __builtin_clz(x);
                sg.b[p] = x;
                sg.mask = 1u << p;
                sg.rank = 1;
                sg.h = F(x);
            }

            int outCnt = 0;
            nxt[outCnt++] = sg;
            vals.push_back(sg.h);

            for (int i = 0; i < cnt; ++i) {
                insertBasis(cur[i], x);

                if (!sameBasis(cur[i], nxt[outCnt - 1])) {
                    nxt[outCnt++] = cur[i];
                    vals.push_back(cur[i].h);
                }
            }

            swap(cur, nxt);
            cnt = outCnt;
        }

        size_t sz = vals.size();

        if (sz > 1) {
            sortHashes(vals, tmp);
        }

        long long ans = 0;

        if (sz > 0) {
            ans = 1;
            for (size_t i = 1; i < sz; ++i) {
                if (vals[i] != vals[i - 1]) ++ans;
            }
        }

        out.writeLongLong(ans);
    }

    return 0;
}

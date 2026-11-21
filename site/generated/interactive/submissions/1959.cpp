#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;
using u64 = unsigned long long;

static int n;
static int rank_[2005];
static int suf[2005];
static vector<int> byRank;

static char inbuf[1 << 16];
static int inpos = 0, inlen = 0;

static inline int readInt() {
    for (;;) {
        if (inpos >= inlen) {
            inlen = (int)read(0, inbuf, sizeof(inbuf));
            inpos = 0;
            if (inlen <= 0) return -1;
        }
        char c = inbuf[inpos];
        if (c == '-') {
            ++inpos;
            int x = 0;
            while (inpos < inlen && inbuf[inpos] >= '0' && inbuf[inpos] <= '9') {
                x = x * 10 + (inbuf[inpos++] - '0');
            }
            return -x;
        }
        if (c >= '0' && c <= '9') {
            int x = 0;
            while (inpos < inlen && inbuf[inpos] >= '0' && inbuf[inpos] <= '9') {
                x = x * 10 + (inbuf[inpos++] - '0');
            }
            return x;
        }
        ++inpos;
    }
}

static inline int putUInt(char* p, int x) {
    char tmp[8];
    int t = 0;
    do { tmp[t++] = char('0' + x % 10); x /= 10; } while (x);
    for (int i = t - 1; i >= 0; --i) *p++ = tmp[i];
    return t;
}

static inline int ask(int j, int i) {
    char buf[32];
    int len = 0;
    buf[len++] = '?';
    buf[len++] = ' ';
    len += putUInt(buf + len, j);
    buf[len++] = ' ';
    len += putUInt(buf + len, i);
    buf[len++] = '\n';
    ssize_t w = write(1, buf, len);
    (void)w;
    int x = readInt();
    if (x < 0) exit(0);
    return x;
}

static inline void flipSuffix(u64* w, u64* cntw, int W, int K, int t, int& ones) {
    if (t >= K) return;
    int w0 = t >> 6, b0 = t & 63;
    u64 validLast = (K & 63) ? ((1ULL << (K & 63)) - 1) : ~0ULL;
    int start;
    if (b0) {
        u64 mask = ~((1ULL << b0) - 1);
        if (w0 == W - 1) mask &= validLast;
        int pc = __builtin_popcountll(mask);
        int old = __builtin_popcountll(w[w0] & mask);
        ones += pc - 2 * old;
        w[w0] ^= mask;
        cntw[w0] = __builtin_popcountll(w[w0]);
        start = w0 + 1;
    } else {
        start = w0;
    }
    for (int q = start; q < W - 1; ++q) {
        int old = (int)cntw[q];
        ones += 64 - 2 * old;
        cntw[q] = 64 - old;
        w[q] = ~w[q];
    }
    if (start <= W - 1) {
        int q = W - 1;
        u64 mask = validLast;
        int pc = __builtin_popcountll(mask);
        int old = __builtin_popcountll(w[q] & mask);
        ones += pc - 2 * old;
        w[q] ^= mask;
        cntw[q] = __builtin_popcountll(w[q]);
    }
}

static void solveStep(int i) {
    int m = i - 1;
    static int C[2005];
    int K = m + 1;
    for (int r = 0; r <= m; ++r) C[r] = r;
    static int cntLE[2005];
    static u64 w[40], cntw[40], bestW[40];

    while (K > 1) {
        {
            int ci = 0;
            for (int u = 0; u < m; ++u) {
                while (ci < K && C[ci] <= u) ++ci;
                cntLE[u] = ci;
            }
        }

        int W = (K + 63) >> 6;
        int ones = 0;
        for (int q = 0; q < W; ++q) { w[q] = 0; cntw[q] = 0; }
        for (int idx = 0; idx < K; ++idx) {
            if (C[idx] & 1) {
                w[idx >> 6] |= 1ULL << (idx & 63);
                ++cntw[idx >> 6];
                ++ones;
            }
        }

        int global = m & 1;
        int O = global ? K - ones : ones;
        int bestScore = max(O, K - O);
        int bestJ = 1, bestGlobal = global;
        for (int q = 0; q < W; ++q) bestW[q] = w[q];

        // A split into groups of sizes as equal as possible is optimal; no
        // need to scan further once such a split is found.
        const int perfect = (K + 1) >> 1;
        bool stop = (bestScore <= perfect);
        for (int j = 1; j <= m - 1 && !stop; ++j) {
            int u = rank_[j];
            int t = cntLE[u];
            flipSuffix(w, cntw, W, K, t, ones);
            global ^= 1;
            O = global ? K - ones : ones;
            int score = max(O, K - O);
            if (score < bestScore) {
                bestScore = score;
                bestJ = j + 1;
                bestGlobal = global;
                for (int q = 0; q < W; ++q) bestW[q] = w[q];
            }
            if (bestScore <= perfect) stop = true;
        }

        int H = ask(bestJ, i) ^ suf[bestJ];
        int newK = 0;
        for (int idx = 0; idx < K; ++idx) {
            int b = (int)((bestW[idx >> 6] >> (idx & 63)) & 1ULL) ^ bestGlobal;
            if (b == H) C[newK++] = C[idx];
        }
        K = newK;
    }

    int rstar = C[0];

    static int newSuf[2005];
    int parity = 0;
    for (int j = m; j >= 1; --j) {
        if (rank_[j] >= rstar) parity ^= 1;
        newSuf[j] = suf[j] ^ parity;
    }
    newSuf[i] = 0;
    for (int q = 0; q <= i; ++q) suf[q] = newSuf[q];

    for (int idx = rstar; idx < m; ++idx) {
        int p = byRank[idx];
        ++rank_[p];
    }
    byRank.insert(byRank.begin() + rstar, i);
    rank_[i] = rstar;
}

int main() {
    n = readInt();
    if (n <= 0) return 0;
    byRank.reserve(n + 1);
    for (int i = 1; i <= n; ++i) solveStep(i);

    static char out[20005];
    int len = 0;
    out[len++] = '!';
    for (int p = 1; p <= n; ++p) {
        out[len++] = ' ';
        len += putUInt(out + len, rank_[p] + 1);
    }
    out[len++] = '\n';
    ssize_t w = write(1, out, len);
    (void)w;
    return 0;
}

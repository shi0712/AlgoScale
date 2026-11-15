#include <bits/stdc++.h>
#include <unistd.h>
using namespace std;

static int n;

// ---------- fast interactive I/O ----------
static char inbuf[1 << 16];
static int inpos = 0, inlen = 0;

static inline int readInt() {
    int c;
    // skip non-digits (and handle '-')
    while (true) {
        if (inpos == inlen) {
            inlen = (int)read(0, inbuf, sizeof(inbuf));
            inpos = 0;
            if (inlen <= 0) return -1;
        }
        c = (unsigned char)inbuf[inpos++];
        if (c == '-') {
            int x = 0;
            while (true) {
                if (inpos == inlen) {
                    inlen = (int)read(0, inbuf, sizeof(inbuf));
                    inpos = 0;
                    if (inlen <= 0) return -x;
                }
                c = (unsigned char)inbuf[inpos++];
                if (c < '0' || c > '9') return -x;
                x = x * 10 + (c - '0');
            }
        }
        if (c >= '0' && c <= '9') {
            int x = c - '0';
            while (true) {
                if (inpos == inlen) {
                    inlen = (int)read(0, inbuf, sizeof(inbuf));
                    inpos = 0;
                    if (inlen <= 0) return x;
                }
                c = (unsigned char)inbuf[inpos++];
                if (c < '0' || c > '9') return x;
                x = x * 10 + (c - '0');
            }
        }
    }
}

static char outbuf[1 << 16];
static int outpos = 0;

static inline void flushOut() {
    int p = 0;
    while (p < outpos) {
        ssize_t w = write(1, outbuf + p, outpos - p);
        if (w <= 0) break;
        p += (int)w;
    }
    outpos = 0;
}

static inline void putChar(char c) {
    if (outpos == (int)sizeof(outbuf)) flushOut();
    outbuf[outpos++] = c;
}

static inline void putInt(int x) {
    char tmp[12];
    int len = 0;
    if (x == 0) tmp[len++] = '0';
    while (x > 0) { tmp[len++] = char('0' + x % 10); x /= 10; }
    while (len) putChar(tmp[--len]);
}

static inline int ask(int l, int r) {
    if (outpos > (int)sizeof(outbuf) - 32) flushOut();
    putChar('?'); putChar(' ');
    putInt(l); putChar(' '); putInt(r); putChar('\n');
    flushOut();
    int x = readInt();
    if (x < 0) exit(0);
    return x;
}

struct Fenwick {
    vector<int> t;
    int n;
    void init(int n_) {
        n = n_;
        t.assign(n + 2, 0);
    }
    void clear() { fill(t.begin(), t.end(), 0); }
    void add(int i, int v) {
        for (; i <= n; i += i & -i) t[i] += v;
    }
    int sum(int i) const {
        int s = 0;
        for (; i > 0; i -= i & -i) s += t[i];
        return s;
    }
};

int main() {
    n = readInt();
    if (n <= 0) return 0;

    vector<int> ord; // positions sorted by hidden value (ascending), local order
    ord.reserve(n);
    vector<int> invord(n + 1, 0); // local rank of each position (0-based)

    // suffix inversion parities of the currently known prefix
    vector<int> prefQ(n + 2, 0);
    Fenwick fw;
    fw.init(n + 2);

    const int WMAX = (n + 2 + 63) / 64;
    vector<uint64_t> H((size_t)(n + 2) * WMAX, 0); // H[l*WMAX + w]
    vector<uint64_t> C(WMAX, 0);

    for (int i = 1; i <= n; ++i) {
        int m = i - 1;
        if (m == 0) {
            ord.insert(ord.begin(), i);
            invord[i] = 0;
            continue;
        }

        // prefQ[l] = inversion parity of prefix interval [l, m]
        fw.clear();
        int inv = 0;
        prefQ[m + 1] = 0;
        for (int l = m; l >= 1; --l) {
            int r = invord[l];          // local rank, 0..m-1
            int less = fw.sum(r);       // number of later elements with rank < r
            inv ^= (less & 1);
            prefQ[l] = inv;
            fw.add(r + 1, 1);
        }

        int W = (m + 1 + 63) / 64;

        // Build row l=1: H[1][k] = (m-k) mod 2
        {
            uint64_t *row = &H[1 * WMAX];
            for (int w = 0; w < W; ++w) row[w] = 0;
            for (int k = 0; k <= m; ++k)
                if ((m - k) & 1) row[k >> 6] |= (uint64_t)1 << (k & 63);
        }
        // Rows l=2..m
        for (int l = 1; l <= m - 1; ++l) {
            uint64_t *dst = &H[(l + 1) * WMAX];
            uint64_t *src = &H[l * WMAX];
            memcpy(dst, src, (size_t)W * sizeof(uint64_t));
            int r = invord[l];
            int len = r + 1;
            int full = len >> 6, rem = len & 63;
            for (int w = 0; w < full; ++w) dst[w] ^= ~(uint64_t)0;
            if (rem) dst[full] ^= ((uint64_t)1 << rem) - 1;
        }

        // Candidate insertion slots k = 0..m
        for (int w = 0; w < W; ++w) C[w] = ~(uint64_t)0;
        if (((m + 1) & 63) != 0)
            C[W - 1] &= ((uint64_t)1 << ((m + 1) & 63)) - 1;
        int csize = m + 1;

        // narrow down the insertion slot
        while (csize > 1) {
            int bestl = -1, bestmn = -1, bestcnt = 0;

            auto evaluate = [&](int l) {
                const uint64_t *row = &H[(size_t)l * WMAX];
                int cnt = 0;
                for (int w = 0; w < W; ++w)
                    cnt += __builtin_popcountll(row[w] & C[w]);
                int mn = min(cnt, csize - cnt);
                if (mn > bestmn) {
                    bestmn = mn;
                    bestl = l;
                    bestcnt = cnt;
                }
            };

            if (m <= 256) {
                for (int l = 1; l <= m; ++l) {
                    evaluate(l);
                    if (bestmn * 2 == csize) break;
                }
            } else {
                const int K = 128;
                for (int j = 0; j < K; ++j) {
                    int l = 1 + (int)((long long)j * m / K);
                    if (l > m) l = m;
                    evaluate(l);
                    if (bestmn * 2 == csize) break;
                }
                if (bestmn * 2 != csize) evaluate(m);
                if (bestmn * 2 != csize &&
                    (bestmn == 0 || (long long)bestmn * 3 < csize)) {
                    for (int l = 1; l <= m; ++l) {
                        evaluate(l);
                        if (bestmn * 2 == csize) break;
                    }
                }
            }

            int b = ask(bestl, i) ^ prefQ[bestl];

            if (b) {
                for (int w = 0; w < W; ++w) C[w] &= H[(size_t)bestl * WMAX + w];
                csize = bestcnt;
            } else {
                for (int w = 0; w < W; ++w) C[w] &= ~H[(size_t)bestl * WMAX + w];
                csize -= bestcnt;
            }
        }

        int k = 0;
        for (int w = 0; w < W; ++w) {
            if (C[w]) {
                k = w * 64 + __builtin_ctzll(C[w]);
                break;
            }
        }

        ord.insert(ord.begin() + k, i);
        for (int r = k; r < (int)ord.size(); ++r) invord[ord[r]] = r;
    }

    vector<int> ans(n + 1);
    for (int r = 0; r < n; ++r) ans[ord[r]] = r + 1;

    putChar('!');
    for (int i = 1; i <= n; ++i) { putChar(' '); putInt(ans[i]); }
    putChar('\n');
    flushOut();
    return 0;
}

#include <cstdio>
#include <cstring>
#include <cstdint>

static const int MAXV = 1000000;
static int cnt[MAXV + 2];    // becomes suffix counts: #{a_i >= x}
static int memo[MAXV + 2];   // memo[T] = min(F(T), A) + 1, 0 = unknown

static char ibuf[1 << 16];
static int ipos = 0, ilen = 0;
static inline int gc() {
    if (ipos == ilen) {
        ilen = (int)fread(ibuf, 1, sizeof(ibuf), stdin);
        ipos = 0;
        if (ilen <= 0) return -1;
    }
    return (unsigned char)ibuf[ipos++];
}
static inline unsigned long long rd() {
    int c = gc();
    while (c < '0' || c > '9') {
        if (c == -1) return 0ULL;
        c = gc();
    }
    unsigned long long x = 0;
    while (c >= '0' && c <= '9') {
        x = x * 10ULL + (unsigned)(c - '0');
        c = gc();
    }
    return x;
}

static char obuf[4 * 1000000 + 64];

int main() {
    int N = (int)rd();
    int Q = (int)rd();
    long long S = 0;
    int A = 0;
    for (int i = 0; i < N; ++i) {
        int a = (int)rd();
        cnt[a]++;
        S += a;
        if (a > A) A = a;
    }
    // suffix counts
    for (int x = A - 1; x >= 1; --x) cnt[x] += cnt[x + 1];

    int op = 0;
    unsigned long long Au = (unsigned long long)A;
    unsigned long long Su = (unsigned long long)S;

    for (int q = 0; q < Q; ++q) {
        unsigned long long K = rd();
        bool ok;
        if (K >= Au) {
            // T = 1 : possible iff total pieces >= K (each a_i <= A <= K)
            ok = (K <= Su);
        } else {
            int k = (int)K;                 // K < A <= 1e6
            int T = (A + k - 1) / k;        // >= 2
            int v = memo[T];
            if (v == 0) {
                int s = 0;
                for (int m = T; m <= A; m += T) {
                    s += cnt[m];
                    if (s >= A) { s = A; break; }   // cap: K < A anyway
                }
                v = s + 1;
                memo[T] = v;
            }
            ok = (v - 1 >= k);
        }
        if (ok) {
            obuf[op++] = 'Y'; obuf[op++] = 'E'; obuf[op++] = 'S'; obuf[op++] = '\n';
        } else {
            obuf[op++] = 'N'; obuf[op++] = 'O'; obuf[op++] = '\n';
        }
    }
    fwrite(obuf, 1, op, stdout);
    return 0;
}

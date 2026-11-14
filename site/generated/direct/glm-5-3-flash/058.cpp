#include <cstdio>
#include <cstdlib>

static const int IBUFSZ = 1 << 22;
static char ibuf[IBUFSZ];
static int ipos = 0, ilen = 0;

static inline int gc() {
    if (ipos == ilen) {
        ilen = (int)fread(ibuf, 1, IBUFSZ, stdin);
        ipos = 0;
        if (ilen <= 0) return -1;
    }
    return (unsigned char)ibuf[ipos++];
}

static inline long long readLL() {
    int c = gc();
    while (c < '0' || c > '9') {
        if (c == -1) return -1;
        c = gc();
    }
    long long x = 0;
    while (c >= '0' && c <= '9') {
        x = x * 10 + (c - '0');
        c = gc();
    }
    return x;
}

static int cnt[1000002]; // counts of values, later suffix counts G[x] = #{a_i >= x}

int main() {
    int N = (int)readLL();
    int Q = (int)readLL();
    long long S = 0;
    int A = 0;
    for (int i = 0; i < N; ++i) {
        int a = (int)readLL();
        ++cnt[a];
        if (a > A) A = a;
        S += a;
    }
    // suffix counts
    for (int x = A; x >= 1; --x) cnt[x] += cnt[x + 1];

    // f[t] = sum_i floor(a_i / t) = sum over multiples k*t of G[k*t]
    long long *f = (long long*)malloc(sizeof(long long) * (size_t)(A + 1));
    for (int t = 1; t <= A; ++t) {
        long long s = 0;
        for (int k = t; k <= A; k += t) s += cnt[k];
        f[t] = s;
    }

    char *obuf = (char*)malloc((size_t)Q * 4 + 16);
    size_t opos = 0;
    for (int j = 0; j < Q; ++j) {
        long long K = readLL();
        bool yes;
        if (K >= (long long)A) {
            // t = 1 is optimal; f(1) = S
            yes = (K <= S);
        } else {
            long long t = (A + K - 1) / K; // ceil(A/K), smallest feasible t
            yes = (f[t] >= K);
        }
        if (yes) { obuf[opos++]='Y'; obuf[opos++]='E'; obuf[opos++]='S'; obuf[opos++]='\n'; }
        else     { obuf[opos++]='N'; obuf[opos++]='O'; obuf[opos++]='\n'; }
    }
    fwrite(obuf, 1, opos, stdout);
    return 0;
}

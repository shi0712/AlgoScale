#include <cstdio>
#include <cstring>
#include <algorithm>

// ---------------------------------------------------------------------------
// Fast input
// ---------------------------------------------------------------------------
namespace fastio {
static const int IN_SZ = 1 << 20;
static char ibuf[IN_SZ];
static int ipos = 0, ilen = 0;

inline int gc() {
    if (ipos == ilen) {
        ilen = (int)fread(ibuf, 1, IN_SZ, stdin);
        ipos = 0;
        if (ilen <= 0) return -1;
    }
    return ibuf[ipos++];
}

inline bool readInt(long long &x) {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9')) c = gc();
    if (c == -1) return false;
    long long v = 0;
    while (c >= '0' && c <= '9') { v = v * 10 + (c - '0'); c = gc(); }
    x = v;
    return true;
}

static const int OUT_SZ = 1 << 22;
static char obuf[OUT_SZ];
static int opos = 0;

inline void flush() { fwrite(obuf, 1, opos, stdout); opos = 0; }
inline void outYes() {
    if (opos + 4 > OUT_SZ) flush();
    obuf[opos++] = 'Y'; obuf[opos++] = 'E'; obuf[opos++] = 'S'; obuf[opos++] = '\n';
}
inline void outNo() {
    if (opos + 3 > OUT_SZ) flush();
    obuf[opos++] = 'N'; obuf[opos++] = 'O'; obuf[opos++] = '\n';
}
} // namespace fastio

using namespace fastio;

static const int MAXA = 1000000;

// cnt[v] = number of types with a_i == v
static int cnt[MAXA + 2];
// suf[v] = number of types with a_i >= v
static int suf[MAXA + 2];
// g[d] = sum_i floor(a_i / d)
static long long g[MAXA + 2];

int main() {
    long long N, Q;
    if (!readInt(N)) return 0;
    readInt(Q);

    int M = 0;
    for (long long i = 0; i < N; ++i) {
        long long a;
        readInt(a);
        if (a > M) M = (int)a;
        ++cnt[(int)a];
    }

    // suffix counts
    suf[M + 1] = 0;
    for (int v = M; v >= 1; --v) suf[v] = suf[v + 1] + cnt[v];

    // g[d] = sum_{t>=1} suf[t*d]
    for (int d = 1; d <= M; ++d) {
        long long s = 0;
        for (int t = d; t <= M; t += d) s += suf[t];
        g[d] = s;
    }

    for (long long qi = 0; qi < Q; ++qi) {
        long long K;
        if (!readInt(K)) break;
        // minimal feasible deliciousness d: need K*d >= M
        long long d = (K <= 0) ? (long long)M : ((long long)M + K - 1) / K;
        if (d < 1) d = 1;
        bool ok = (d <= M) && (g[d] >= K);
        if (ok) outYes(); else outNo();
    }
    flush();
    return 0;
}

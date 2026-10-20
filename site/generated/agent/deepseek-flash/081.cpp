// Doubles Lineup
//
// Sort players by ability, drop those with ability > max(l).
// An optimal solution can be taken so that pairs are consecutive blocks of the
// sorted selection (uncrossing argument).  For a pair (p,j), p<j, only the
// *latest* previous player of each type (junior / senior) can be the partner:
// among partners with the same junior flag, a later position dominates
// (prefix value function is monotone and ability is monotone).
//
// DP over positions with state k = number of completed pairs, value = array
// indexed by the number of juniors selected so far:
//   closed[k][t]  : best total with k pairs completed inside the prefix
//   carry[k][v]   : best total with k pairs completed, plus one player of
//                   junior-flag v held as the first member of the (k+1)-th pair
//                   (its junior count already included), v in {0,1}
// Closing at position j (junior flag vj) adds carry[k-1][v][t-vj] + a[j];
// the k-th pair must have max ability <= L[k], i.e. j <= J[k].
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include <vector>
using namespace std;
typedef long long ll;
static const ll NEG = -(1LL << 60);

// The DP is a long chain of small vectorisable loops and the compilation flags
// in use are fixed, so request the needed optimisations (plus a runtime
// dispatched AVX2 clone) from the compiler directly.
#pragma GCC optimize("O3")
#if defined(__GNUC__) && !defined(__clang__) && defined(__x86_64__) && defined(__linux__)
#define HOT_LOOPS __attribute__((target_clones("avx2", "default")))
#else
#define HOT_LOOPS
#endif

static char inbuf[1 << 16];
static int inpos, inlen;
static inline int gc() {
    if (inpos == inlen) {
        inlen = (int)fread(inbuf, 1, sizeof(inbuf), stdin);
        inpos = 0;
        if (inlen <= 0) return -1;
    }
    return (unsigned char)inbuf[inpos++];
}
static inline long long readInt() {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9') && c != '-') c = gc();
    int sign = 1;
    if (c == '-') { sign = -1; c = gc(); }
    long long x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return x * sign;
}

static vector<pair<ll, int> > pl;

// out[t] = best total ability with exactly t juniors, or -1 if impossible.
HOT_LOOPS
static void solveCase(int n, int m, ll d, vector<ll> &L, vector<ll> &out) {
    sort(L.begin(), L.end());
    ll Lmax = L[n - 1];
    pl.clear();
    pl.reserve(m);
    for (int i = 0; i < m; i++) {
        ll e = readInt();
        int tp = (int)readInt();
        if (e <= Lmax) pl.push_back(make_pair(e, tp));
    }
    sort(pl.begin(), pl.end());
    int M = (int)pl.size();
    out.assign(2 * n + 1, -1);
    if (M < 2 * n) return;

    vector<int> J(n + 1, 0);
    {
        int p = 0;
        for (int k = 1; k <= n; k++) {
            while (p < M && pl[p].first <= L[k - 1]) p++;
            J[k] = p;
        }
    }
    vector<int> jr(M + 1, 0), sn(M + 1, 0);
    for (int i = 1; i <= M; i++) {
        jr[i] = jr[i - 1] + (pl[i - 1].second == 1 ? 1 : 0);
        sn[i] = sn[i - 1] + (pl[i - 1].second == 2 ? 1 : 0);
    }
    vector<int> firstK(M + 2, n + 1);
    {
        int k = 1;
        for (int j = 1; j <= M; j++) {
            while (k <= n && J[k] < j) k++;
            firstK[j] = k;
        }
    }
    // pool layout: per state k, 4 arrays of S_k = 2k+6 ll, offset 2
    vector<int> base(n + 1, 0);
    int off = 0;
    for (int k = 0; k <= n; k++) { base[k] = off; off += 4 * (2 * k + 6); }
    vector<ll> pool((size_t)off, NEG);
    vector<int> curIdx(n + 1, 0), oldCur(n + 1, 0);
    vector<ll> cpos(2 * (n + 1), NEG);
    pool[base[0] + 2] = 0;  // state 0, closed buffer 0, t = 0

    for (int j = 1; j <= M; j++) {
        ll aj = pl[j - 1].first;
        int vj = (pl[j - 1].second == 1) ? 1 : 0;
        int jrj = jr[j], snj = sn[j];
        memcpy(&oldCur[0], &curIdx[0], sizeof(int) * (n + 1));
        // ---- close pairs at position j ----
        int klo = firstK[j];
        if (klo < 1) klo = 1;
        int en = n - (M - j) / 2;
        if (klo < en) klo = en;
        int khi = j / 2;
        if (khi > n) khi = n;
        for (int k = klo; k <= khi; k++) {
            int p1 = k - 1;
            ll pos0 = cpos[2 * p1], pos1 = cpos[2 * p1 + 1];
            bool v0 = (pos0 > NEG / 2) && (aj - pos0 <= d);
            bool v1 = (pos1 > NEG / 2) && (aj - pos1 <= d);
            if (!v0 && !v1) continue;
            int S = 2 * k + 6;
            int ci = curIdx[k], di = ci ^ 1;
            ll *cc = &pool[base[k] + 2];
            ll *dst = cc + (size_t)di * S;
            const ll *c = cc + (size_t)ci * S;
            int hi = 2 * k;
            if (jrj < hi) hi = jrj;
            int lo = 2 * k - snj;
            if (lo < 0) lo = 0;
            if (v0 && v1) {
                const ll *c0 = &pool[base[p1] + 2] + (size_t)2 * (2 * p1 + 6);
                const ll *c1 = c0 + (2 * p1 + 6);
                for (int t = lo; t <= hi; t++) {
                    ll b = c[t];
                    ll x = c0[t - vj] + aj;
                    if (x > b) b = x;
                    ll y = c1[t - vj] + aj;
                    if (y > b) b = y;
                    dst[t] = b;
                }
            } else {
                const ll *c0 = &pool[base[p1] + 2] + (size_t)(v0 ? 2 : 3) * (2 * p1 + 6);
                for (int t = lo; t <= hi; t++) {
                    ll b = c[t];
                    ll x = c0[t - vj] + aj;
                    if (x > b) b = x;
                    dst[t] = b;
                }
            }
            curIdx[k] = di;
        }
        // ---- hold player j as the first member of a future pair ----
        {
            int k2lo = n - 1 - (M - j - 1) / 2;
            if (k2lo < 0) k2lo = 0;
            int k2hi = (j - 1) / 2;
            if (k2hi > n - 1) k2hi = n - 1;
            for (int k = k2lo; k <= k2hi; k++) {
                int S = 2 * k + 6;
                const ll *c = &pool[base[k] + 2] + (size_t)oldCur[k] * S;
                ll *dstc = &pool[base[k] + 2] + (size_t)(2 + vj) * S;
                int hi = 2 * k + 1;
                if (jrj < hi) hi = jrj;
                int lo = 2 * k + 1 - snj;
                if (lo < 0) lo = 0;
                for (int t = lo; t <= hi; t++) dstc[t] = c[t - vj] + aj;
                cpos[2 * k + vj] = aj;
            }
        }
    }
    const ll *res = &pool[base[n] + 2] + (size_t)curIdx[n] * (2 * n + 6);
    for (int t = 0; t <= 2 * n; t++) {
        ll v = res[t];
        out[t] = (v <= NEG / 2) ? -1 : v;
    }
}

int main() {
    int T = (int)readInt();
    vector<ll> L, out;
    while (T--) {
        int n = (int)readInt();
        int m = (int)readInt();
        ll d = readInt();
        L.resize(n);
        for (int i = 0; i < n; i++) L[i] = readInt();
        solveCase(n, m, d, L, out);
        for (size_t i = 0; i < out.size(); i++)
            printf("%lld%c", out[i], i + 1 == out.size() ? '\n' : ' ');
    }
    return 0;
}

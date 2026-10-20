#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace std;
typedef long long ll;

static const int MOD = 998244353;
static const int MAXN = 1000005;

static int head_[MAXN], nxt_[MAXN], H_[MAXN];
static int fOff[MAXN], dOff[MAXN];
static int T_[MAXN];
static int fBuf[2 * MAXN + 10];
static int dBuf[4 * MAXN + 20];
static int g_[MAXN];
static int pfx[MAXN];
static int fact_[MAXN], invfact_[MAXN];

/* ---------- fast input ---------- */
static char ibuf[1 << 16];
static int ipos = 0, ilen = 0;
static inline int gc() {
    if (ipos == ilen) {
        ilen = (int)fread(ibuf, 1, sizeof(ibuf), stdin);
        ipos = 0;
        if (ilen <= 0) return -1;
    }
    return ibuf[ipos++];
}
static inline int readInt() {
    int c = gc();
    while (c < '0' || c > '9') { if (c == -1) return -1; c = gc(); }
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return x;
}

/* ---------- fast output ---------- */
static char obuf[1 << 16];
static int opos = 0;
static inline void flushOut() {
    if (opos) { fwrite(obuf, 1, opos, stdout); opos = 0; }
}
static inline void putInt(int x) {
    if (opos > (1 << 15) - 16) flushOut();
    char tmp[16]; int l = 0;
    if (x == 0) tmp[l++] = '0';
    while (x > 0) { tmp[l++] = char('0' + x % 10); x /= 10; }
    while (l) obuf[opos++] = tmp[--l];
    obuf[opos++] = '\n';
}

static ll modpow(ll b, ll e) {
    ll r = 1; b %= MOD;
    while (e) { if (e & 1) r = r * b % MOD; b = b * b % MOD; e >>= 1; }
    return r;
}

int main() {
    /* factorials up to MAXN-1 */
    fact_[0] = 1;
    for (int i = 1; i < MAXN; i++) fact_[i] = (int)((ll)fact_[i - 1] * i % MOD);
    invfact_[MAXN - 1] = (int)modpow(fact_[MAXN - 1], MOD - 2);
    for (int i = MAXN - 1; i > 0; i--) invfact_[i - 1] = (int)((ll)invfact_[i] * i % MOD);

    int t = readInt();
    while (t-- > 0) {
        int n = readInt();
        int k = readInt();
        for (int i = 1; i <= n; i++) head_[i] = 0;
        for (int i = 2; i <= n; i++) {
            int p = readInt();
            nxt_[i] = head_[p];
            head_[p] = i;
        }

        int r = k - 1;
        if (r == 0) { putInt(n % MOD); continue; }
        if (k > n) { putInt(0); continue; }   /* defensive; guaranteed not to happen */

        /* ---- subtree heights, bottom-up (p_i < i so index order is topological) ---- */
        for (int v = n; v >= 1; v--) {
            int h = 0;
            for (int c = head_[v]; c; c = nxt_[c]) {
                int hc = H_[c] + 1;
                if (hc > h) h = hc;
            }
            H_[v] = h;
        }

        /* ---- offsets for subtree distance profiles f_v (indices 0..H) ---- */
        int tot = 0;
        for (int v = 1; v <= n; v++) { fOff[v] = tot; tot += H_[v] + 1; }
        int tot2 = 0;
        for (int v = 1; v <= n; v++) { dOff[v] = tot2; tot2 += H_[v] + 3; }

        /* ---- compute f_v bottom-up ---- */
        for (int v = n; v >= 1; v--) {
            int *fv = fBuf + fOff[v];
            int h = H_[v];
            for (int d = 0; d <= h; d++) fv[d] = 0;
            fv[0] = 1;
            for (int c = head_[v]; c; c = nxt_[c]) {
                int *fc = fBuf + fOff[c];
                int hc = H_[c];
                for (int d = 0; d <= hc; d++) fv[d + 1] += fc[d];
            }
        }

        /* ---- binomial table g[m] = C(m, r) ---- */
        int cr = invfact_[r];
        g_[0] = 0;
        for (int m = 1; m <= n; m++)
            g_[m] = (m >= r) ? (int)((ll)fact_[m] * invfact_[m - r] % MOD * cr % MOD) : 0;

        /* ---- root full-profile prefix D(1, .) ---- */
        {
            int *Dv = dBuf + dOff[1];
            int h = H_[1];
            int *fv = fBuf + fOff[1];
            Dv[0] = 1;
            for (int d = 1; d <= h + 2; d++) Dv[d] = (d <= h) ? fv[d] : 0;
        }

        ll ans = 0;
        T_[1] = 0;

        for (int v = 1; v <= n; v++) {
            int *Dv = dBuf + dOff[v];
            int h = H_[v];

            /* prefix sums of g(D(v, e)) for e = 0..h+2 */
            pfx[0] = 0;
            for (int e = 1; e <= h + 2; e++) {
                int s = pfx[e - 1] + g_[Dv[e]];
                if (s >= MOD) s -= MOD;
                pfx[e] = s;
            }
            /* add the "direct" part of T(v) */
            int tv = T_[v] + pfx[h + 2];
            if (tv >= MOD) tv -= MOD;
            T_[v] = tv;
            ans += tv;

            /* children */
            for (int c = head_[v]; c; c = nxt_[c]) {
                int hc = H_[c];
                int *Dc = dBuf + dOff[c];
                int *fc = fBuf + fOff[c];
                Dc[0] = 1;
                for (int d = 1; d <= hc + 2; d++) {
                    int val = Dv[d - 1];
                    if (d <= hc) val += fc[d];
                    if (d >= 2) val -= fc[d - 2];
                    Dc[d] = val;
                }
                int t2 = T_[v] - pfx[hc + 1];
                if (t2 < 0) t2 += MOD;
                T_[c] = t2;
            }
        }

        putInt((int)(ans % MOD));
    }
    flushOut();
    return 0;
}

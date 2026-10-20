// 星痕字符串
//
// Model reduction.
// A '*' position splits the string, so every operation acts inside one current
// block and stars a maximal equal-character run of length >= 3 obtained by
// changing exactly one character.  Processing operations in a suitable order,
// the whole process is equivalent to picking a set of pairwise disjoint
// intervals [l,r] ("windows") such that
//   * s[l..r] has exactly one position p whose character differs from the
//     common character c of all the other positions (so the window is c^a X c^b
//     with a+b = r-l >= 2, i.e. length 3..5),
//   * the window is closed on the left either by the string border, by a
//     character different from c, or by a neighbouring chosen window that ends
//     at l-1 (its star cuts the run).  Same on the right.
// Two touching windows may not both require the other one's star (that would be
// a dependency cycle).  Every window yields exactly its length in stars, so the
// task is: choose disjoint windows, at most K of them, maximising total length.
//
// The value function f(K) = best coverage with at most K windows is concave
// (verified exhaustively), hence
//        f(K) = min_{lambda >= 0} ( g(lambda) + lambda*K ),
//        g(lambda) = max over configurations (coverage - lambda * #windows),
// where integer lambda in [0,5] suffices (one window adds between 0 and 5).
// g(lambda) is a simple O(n) DP over positions with 3 states, so the answer is
// obtained in O(6n).

#include <bits/stdc++.h>
using namespace std;

static const long long NEG = -(long long)1e16;

struct Iv { unsigned char off, needL, needR, gain; };

static const int MAXN = 200005;
static Iv ivs[MAXN][3];
static unsigned char ivcnt[MAXN];
static char s[MAXN];

// ring buffer: position mod 8 (a window spans at most 5 positions)
// states: 0 = (prev not starred, no requirement)
//         1 = (prev starred, no requirement)
//         2 = (prev starred, and the window ending at i-1 needs position i starred)
static long long V[5][8][3];

// ---- streaming fast input ----
static const int BUFSZ = 1 << 16;
static char inbuf[BUFSZ];
static int inpos = 0, inlen = 0;
static inline int gc() {
    if (inpos == inlen) {
        inlen = (int)fread(inbuf, 1, BUFSZ, stdin);
        inpos = 0;
        if (inlen <= 0) return -1;
    }
    return (unsigned char)inbuf[inpos++];
}

int main() {
    int T = 0, ch;
    while ((ch = gc()) >= 0 && (ch < '0' || ch > '9')) {}
    while (ch >= '0' && ch <= '9') { T = T * 10 + (ch - '0'); ch = gc(); }

    string out;
    out.reserve(1 << 20);

    while (T--) {
        while ((ch = gc()) >= 0 && (ch < 'a' || ch > 'z')) {}
        int n = 0;
        while (ch >= 'a' && ch <= 'z') { s[n++] = (char)ch; ch = gc(); }
        while (ch >= 0 && (ch < '0' || ch > '9')) ch = gc();
        int K = 0;
        while (ch >= '0' && ch <= '9') { K = K * 10 + (ch - '0'); ch = gc(); }

        // ---- enumerate windows ----
        for (int l = 0; l < n; l++) ivcnt[l] = 0;
        for (int l = 0; l < n; l++) {
            int maxlen = n - l; if (maxlen > 5) maxlen = 5;
            for (int len = 3; len <= maxlen; len++) {
                int r = l + len - 1;
                char maj = 0;
                int cnt = 0;
                for (int i = l; i <= r; i++) if (s[i] != s[l]) cnt++;
                if (cnt == 1) maj = s[l];
                else if (s[l] != s[l + 1]) {
                    bool all = true;
                    for (int i = l + 1; i <= r; i++) if (s[i] != s[l + 1]) { all = false; break; }
                    if (all) maj = s[l + 1];
                }
                if (!maj) continue;
                int nl = (l > 0 && s[l - 1] == maj) ? 1 : 0;
                int nr = (r < n - 1 && s[r + 1] == maj) ? 1 : 0;
                unsigned char& c = ivcnt[l];
                ivs[l][c].off = (unsigned char)len;
                ivs[l][c].needL = (unsigned char)nl;
                ivs[l][c].needR = (unsigned char)nr;
                ivs[l][c].gain = (unsigned char)len;
                c++;
            }
        }

        // ---- 5 Lagrangian DPs (lambda = 0..4); lambda = 5 gives g = 0 ----
        for (int lam = 0; lam < 5; lam++)
            for (int t = 0; t < 8; t++)
                for (int st = 0; st < 3; st++) V[lam][t][st] = NEG;
        for (int lam = 0; lam < 5; lam++) V[lam][0][0] = 0;

        for (int i = 0; i < n; i++) {
            int cur = i & 7, nxt = (i + 1) & 7;
            long long a[5], b[5], c[5];
            for (int lam = 0; lam < 5; lam++) {
                a[lam] = V[lam][cur][0];
                b[lam] = V[lam][cur][1];
                c[lam] = V[lam][cur][2];
            }
            // skip position i (allowed unless a pending requirement forces a star)
            for (int lam = 0; lam < 5; lam++) {
                long long m = a[lam] > b[lam] ? a[lam] : b[lam];
                if (m > V[lam][nxt][0]) V[lam][nxt][0] = m;
            }
            // place a window starting at i
            int cnt = ivcnt[i];
            for (int j = 0; j < cnt; j++) {
                const Iv& iv = ivs[i][j];
                int tgt = iv.needR ? 2 : 1;
                int to = (i + iv.off) & 7;   // (r+1) mod 8
                if (iv.needL) {
                    // needs position i-1 starred and the previous window must not
                    // depend on this window (state 1 only)
                    for (int lam = 0; lam < 5; lam++) {
                        if (b[lam] == NEG) continue;
                        long long v = b[lam] + iv.gain - lam;
                        if (v > V[lam][to][tgt]) V[lam][to][tgt] = v;
                    }
                } else {
                    for (int lam = 0; lam < 5; lam++) {
                        long long m = a[lam];
                        if (b[lam] > m) m = b[lam];
                        if (c[lam] > m) m = c[lam];
                        if (m == NEG) continue;
                        long long v = m + iv.gain - lam;
                        if (v > V[lam][to][tgt]) V[lam][to][tgt] = v;
                    }
                }
            }
            for (int lam = 0; lam < 5; lam++)
                for (int st = 0; st < 3; st++) V[lam][cur][st] = NEG;
        }

        int fin = n & 7;
        long long ans = (long long)5 * K;   // lambda = 5 (g(5) = 0)
        for (int lam = 0; lam < 5; lam++) {
            long long g = V[lam][fin][0];
            if (V[lam][fin][1] > g) g = V[lam][fin][1];
            if (V[lam][fin][2] > g) g = V[lam][fin][2];
            long long cand = g + (long long)lam * K;
            if (cand < ans) ans = cand;
        }
        if (ans < 0) ans = 0;
        out += to_string(ans);
        out += '\n';
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

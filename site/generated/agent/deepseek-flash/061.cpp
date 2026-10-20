#include <bits/stdc++.h>
using namespace std;

static const unsigned int MOD = 1000000007u;

inline unsigned int madd(unsigned int a, unsigned int b) {
    unsigned int s = a + b;
    return s >= MOD ? s - MOD : s;
}
inline unsigned int msub(unsigned int a, unsigned int b) { return a >= b ? a - b : a + MOD - b; }

static char s[1006];

int main() {
    int n, Q;
    if (scanf("%d %d", &n, &Q) != 2) return 0;
    scanf("%s", s);
    int m = 0;
    for (int i = 0; i < n; i++)
        if (s[i] == '?') m++;
    int W = m + 1;
    size_t SZ = (size_t)W * W;

    // two sets of 3 arrays (dp per last color), ping-pong
    vector<unsigned int> bufs(6 * SZ, 0);

    unsigned int *cur[3], *nxt[3];
    for (int k = 0; k < 3; k++) {
        cur[k] = bufs.data() + (size_t)k * SZ;
        nxt[k] = bufs.data() + (size_t)(3 + k) * SZ;
    }

    // index = A*W + B  (A = #a among '?' processed, B = #b among '?' processed)
    int q = 0;  // number of '?' processed so far
    {
        char c0 = s[0];
        if (c0 == '?') {
            cur[0][(size_t)W] = 1;  // color a -> (A=1,B=0)
            cur[1][1] = 1;          // color b -> (A=0,B=1)
            cur[2][0] = 1;          // color c -> (A=0,B=0)
            q = 1;
        } else if (c0 == 'a') {
            cur[0][0] = 1;
        } else if (c0 == 'b') {
            cur[1][0] = 1;
        } else {
            cur[2][0] = 1;
        }
    }

    for (int i = 1; i < n; i++) {
        char ch = s[i];
        if (ch == '?') {
            for (int ai = 0; ai <= q; ai++) {
                const unsigned int *ra = cur[0] + (size_t)ai * W;
                const unsigned int *rb = cur[1] + (size_t)ai * W;
                const unsigned int *rc = cur[2] + (size_t)ai * W;
                unsigned int *wa = nxt[0] + (size_t)ai * W + W;
                unsigned int *wb = nxt[1] + (size_t)ai * W + 1;
                unsigned int *wc = nxt[2] + (size_t)ai * W;
                int lim = q - ai;
                for (int bb = 0; bb <= lim; bb++) {
                    unsigned int x = ra[bb], y = rb[bb], z = rc[bb];
                    wa[bb] = madd(y, z);
                    wb[bb] = madd(x, z);
                    wc[bb] = madd(x, y);
                }
            }
            q++;
            // zero unwritten cells inside new triangle A+B<=q
            for (int B = 0; B <= q; B++) nxt[0][B] = 0;
            for (int A = 0; A <= q; A++) nxt[1][(size_t)A * W] = 0;
            for (int A = 0; A <= q; A++) nxt[2][(size_t)A * W + (q - A)] = 0;
            swap(cur[0], nxt[0]);
            swap(cur[1], nxt[1]);
            swap(cur[2], nxt[2]);
        } else {
            int col = ch - 'a';
            int o1 = (col + 1) % 3, o2 = (col + 2) % 3;
            for (int ai = 0; ai <= q; ai++) {
                const unsigned int *r1 = cur[o1] + (size_t)ai * W;
                const unsigned int *r2 = cur[o2] + (size_t)ai * W;
                unsigned int *w = nxt[col] + (size_t)ai * W;
                unsigned int *z1 = nxt[o1] + (size_t)ai * W;
                unsigned int *z2 = nxt[o2] + (size_t)ai * W;
                int lim = q - ai;
                for (int bb = 0; bb <= lim; bb++) {
                    w[bb] = madd(r1[bb], r2[bb]);
                    z1[bb] = 0;
                    z2[bb] = 0;
                }
            }
            swap(cur[0], nxt[0]);
            swap(cur[1], nxt[1]);
            swap(cur[2], nxt[2]);
        }
    }

    // final table cnt[A][B] = cur[0]+cur[1]+cur[2]
    vector<unsigned int> cnt(SZ, 0);
    for (int ai = 0; ai <= m; ai++) {
        const unsigned int *ra = cur[0] + (size_t)ai * W;
        const unsigned int *rb = cur[1] + (size_t)ai * W;
        const unsigned int *rc = cur[2] + (size_t)ai * W;
        unsigned int *wc = cnt.data() + (size_t)ai * W;
        for (int bb = 0; bb <= m - ai; bb++) {
            unsigned long long t = (unsigned long long)ra[bb] + rb[bb] + rc[bb];
            wc[bb] = (unsigned int)(t % MOD);
        }
    }

    // P[A][B] = sum_{a<=A,b<=B} cnt[a][b]
    vector<unsigned int> P(SZ, 0);
    for (int ai = 0; ai <= m; ai++) {
        unsigned long long rowsum = 0;
        unsigned int *wp = P.data() + (size_t)ai * W;
        const unsigned int *wc = cnt.data() + (size_t)ai * W;
        for (int bb = 0; bb <= m; bb++) {
            rowsum += wc[bb];
            unsigned long long v = rowsum;
            if (ai > 0) v += P[(size_t)(ai - 1) * W + bb];
            wp[bb] = (unsigned int)(v % MOD);
        }
    }

    // SF[A][D] = sum_{A'=0..A} rowpref[A'][D-1-A'], rowpref[A'][neg]=0, D in [1,m]
    vector<unsigned int> SF(SZ, 0);
    for (int D = 1; D <= m; D++) {
        for (int A = 0; A <= m; A++) {
            unsigned int t = 0;
            int j = D - 1 - A;
            if (j >= 0) {
                t = P[(size_t)A * W + j];
                if (A > 0) t = msub(t, P[(size_t)(A - 1) * W + j]);
            }
            unsigned int prev = (A > 0) ? SF[(size_t)(A - 1) * W + D] : 0;
            SF[(size_t)A * W + D] = madd(prev, t);
        }
    }

    string out;
    out.reserve((size_t)Q * 12);
    for (int qi = 0; qi < Q; qi++) {
        int x, y, z;
        scanf("%d %d %d", &x, &y, &z);
        unsigned int xp = (unsigned int)min(x, m), yp = (unsigned int)min(y, m);
        unsigned int ans;
        if ((long long)x + y + z < m) {
            ans = 0;
        } else if (z >= m) {
            ans = P[(size_t)xp * W + yp];
        } else {
            int D = m - z;  // >= 1
            unsigned int full = P[(size_t)xp * W + yp];
            int A1 = (int)min((long long)xp, (long long)D - 1 - (long long)yp);
            unsigned int T = (A1 >= 0) ? P[(size_t)A1 * W + yp] : 0u;
            int Ahi = (int)min((long long)xp, (long long)D - 1);
            unsigned int tri = SF[(size_t)Ahi * W + D];
            if (A1 >= 0) tri = msub(tri, SF[(size_t)A1 * W + D]);
            unsigned int sub = madd(T, tri);
            ans = msub(full, sub);
        }
        out += to_string(ans);
        out += '\n';
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

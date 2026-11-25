#include <bits/stdc++.h>
using namespace std;

const int MOD = 998244353;
const int MAXN = 1000000 + 5;

vector<int> parent(MAXN), head(MAXN), nxt(MAXN), child(MAXN);
vector<int> h(MAXN), off(MAXN + 1), S(MAXN), B(MAXN), inv(MAXN);
vector<int> Fdata, Qdata;

static const int BUFSZ = 1 << 20;
static char buf[BUFSZ];
static int bufPos = 0, bufLen = 0;

inline int readInt() {
    int x = 0;
    char c;
    do {
        if (bufPos == bufLen) {
            bufLen = fread(buf, 1, BUFSZ, stdin);
            bufPos = 0;
            if (bufLen == 0) return 0;
        }
        c = buf[bufPos++];
    } while (c < '0' || c > '9');

    while (c >= '0' && c <= '9') {
        x = x * 10 + (c - '0');
        if (bufPos == bufLen) {
            bufLen = fread(buf, 1, BUFSZ, stdin);
            bufPos = 0;
            if (bufLen == 0) break;
        }
        c = buf[bufPos++];
    }
    return x;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    inv[1] = 1;
    for (int i = 2; i < MAXN; ++i) {
        inv[i] = MOD - (long long)(MOD / i) * inv[MOD % i] % MOD;
    }

    Fdata.resize(2 * MAXN + 5);
    Qdata.resize(2 * MAXN + 5);

    int t = readInt();
    while (t--) {
        int n = readInt();
        int k = readInt();

        for (int i = 2; i <= n; ++i) parent[i] = readInt();

        if (k == 1) {
            cout << n << '\n';
            continue;
        }
        if (k == 2) {
            long long ans = (long long)n * (n - 1) % MOD;
            cout << ans << '\n';
            continue;
        }

        int r = k - 1;

        // B[c] = C(c, r) mod MOD
        for (int i = 0; i < r; ++i) B[i] = 0;
        B[r] = 1;
        for (int c = r + 1; c <= n; ++c) {
            B[c] = (long long)B[c - 1] * c % MOD * inv[c - r] % MOD;
        }

        // Build children lists
        for (int i = 1; i <= n; ++i) head[i] = -1;
        for (int i = 2; i <= n; ++i) {
            int p = parent[i];
            nxt[i - 2] = head[p];
            child[i - 2] = i;
            head[p] = i - 2;
        }

        // Heights of subtrees
        for (int v = n; v >= 1; --v) {
            int hv = 0;
            for (int e = head[v]; e != -1; e = nxt[e]) {
                int c = child[e];
                int t = h[c] + 1;
                if (t > hv) hv = t;
            }
            h[v] = hv;
        }

        // Offsets for F and Q arrays
        off[1] = 0;
        for (int v = 1; v <= n; ++v) {
            off[v + 1] = off[v] + h[v] + 1;
        }

        // Compute F[v]: distance distribution inside subtree of v
        for (int v = n; v >= 1; --v) {
            int *fv = &Fdata[off[v]];
            int len = h[v] + 1;
            for (int i = 0; i < len; ++i) fv[i] = 0;
            fv[0] = 1;

            for (int e = head[v]; e != -1; e = nxt[e]) {
                int c = child[e];
                int *fc = &Fdata[off[c]];
                int hc = h[c];
                for (int d = 0; d <= hc; ++d) {
                    fv[d + 1] += fc[d];
                }
            }
        }

        long long ans = 0;

        // Root
        {
            int *q = &Qdata[off[1]];
            int *f = &Fdata[off[1]];
            int len = h[1] + 1;
            for (int i = 0; i < len; ++i) q[i] = f[i];

            int s = 0;
            for (int d = 0; d <= h[1]; ++d) {
                s += B[f[d]];
                if (s >= MOD) s -= MOD;
            }
            S[1] = s;
            ans += s;
        }

        // Process vertices in increasing order (parents before children)
        for (int v = 2; v <= n; ++v) {
            int p = parent[v];
            int *qp = &Qdata[off[p]];
            int *fv = &Fdata[off[v]];
            int *qv = &Qdata[off[v]];

            int hv = h[v];
            int sv = S[p];
            int maxd = hv + 2;

            for (int d = 0; d <= maxd; ++d) {
                int base = (d == 0 ? 0 : qp[d - 1]);
                int fd = (d <= hv ? fv[d] : 0);
                int fd2 = (d >= 2 && d - 2 <= hv) ? fv[d - 2] : 0;

                int c = base + fd - fd2;

                int diff = B[c] - B[base];
                if (diff < 0) diff += MOD;

                sv += diff;
                if (sv >= MOD) sv -= MOD;

                if (d <= hv) qv[d] = c;
            }

            S[v] = sv;
            ans += sv;
            if (ans >= MOD) ans -= MOD;
        }

        cout << ans % MOD << '\n';
    }

    return 0;
}

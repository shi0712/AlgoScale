#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

static const int MOD = 1000000007;
static inline int addmod(int a, int b) { int s = a + b; if (s >= MOD) s -= MOD; return s; }
static inline int submod(int a, int b) { int s = a - b; if (s < 0) s += MOD; return s; }

// ---- fast input ----
static char ibuf[1 << 16];
static size_t ipos = 0, ilen = 0;
static inline int readChar() {
    if (ipos == ilen) { ilen = fread(ibuf, 1, sizeof(ibuf), stdin); ipos = 0; if (ilen == 0) return -1; }
    return ibuf[ipos++];
}
static bool readInt(int &out) {
    int c = readChar();
    while (c != -1 && (c < '0' || c > '9')) c = readChar();
    if (c == -1) return false;
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = readChar(); }
    out = x;
    return true;
}
static void readToken(string &s) {
    int c = readChar();
    while (c != -1 && c != '?' && (c < 'a' || c > 'c')) c = readChar();
    s.clear();
    while (c == '?' || (c >= 'a' && c <= 'c')) { s.push_back((char)c); c = readChar(); }
}

int main() {
    int n, Q;
    if (!readInt(n)) return 0;
    readInt(Q);
    string s;
    readToken(s);

    int m = 0;
    for (char c : s) if (c == '?') ++m;
    const int W = m + 2;
    // DP planes: f0 = last colour 'a', f1 = 'b', f2 = 'c'
    vector<int> f0((size_t)W * W, 0), f1((size_t)W * W, 0), f2((size_t)W * W, 0);

    int q = 0;  // number of '?' processed so far
    // first position
    if (s[0] == '?') {
        f0[(size_t)1 * W + 0] = 1;   // 'a', one a used
        f1[(size_t)0 * W + 1] = 1;   // 'b', one b used
        f2[(size_t)0 * W + 0] = 1;   // 'c'
        q = 1;
    } else {
        int L = s[0] - 'a';
        if (L == 0) f0[0] = 1; else if (L == 1) f1[0] = 1; else f2[0] = 1;
    }

    for (int i = 1; i < n; ++i) {
        if (s[i] == '?') {
            // in-place update, diagonals decreasing
            for (int d = q; d >= 0; --d) {
                for (int a = 0; a <= d; ++a) {
                    int b = d - a;
                    size_t idx = (size_t)a * W + b;
                    int x0 = f0[idx], x1 = f1[idx], x2 = f2[idx];
                    f2[idx] = addmod(x0, x1);                  // new last 'c'
                    f0[(size_t)(a + 1) * W + b] = addmod(x1, x2); // new last 'a'
                    f1[(size_t)a * W + (b + 1)] = addmod(x0, x2); // new last 'b'
                }
            }
            ++q;
            // stale cells: f0 needs a>=1, f1 needs b>=1
            for (int b = 0; b <= q; ++b) f0[b] = 0;
            for (int a = 0; a <= q; ++a) f1[(size_t)a * W] = 0;
        } else {
            int L = s[i] - 'a';
            int *fl, *o1, *o2;
            if (L == 0) { fl = &f0[0]; o1 = &f1[0]; o2 = &f2[0]; }
            else if (L == 1) { fl = &f1[0]; o1 = &f0[0]; o2 = &f2[0]; }
            else { fl = &f2[0]; o1 = &f0[0]; o2 = &f1[0]; }
            for (int d = q; d >= 0; --d) {
                for (int a = 0; a <= d; ++a) {
                    int b = d - a;
                    size_t idx = (size_t)a * W + b;
                    fl[idx] = addmod(o1[idx], o2[idx]);
                    o1[idx] = 0; o2[idx] = 0;
                }
            }
        }
    }

    const int SZ = m + 1;
    vector<int> cnt((size_t)SZ * SZ, 0);
    for (int a = 0; a <= m; ++a)
        for (int b = 0; b + a <= m; ++b) {
            size_t idx = (size_t)a * W + b;
            cnt[(size_t)a * SZ + b] = addmod(addmod(f0[idx], f1[idx]), f2[idx]);
        }
    vector<int>().swap(f0); vector<int>().swap(f1); vector<int>().swap(f2);

    // row prefix: RR[a][u] = sum_{b<=min(u,m-a)} cnt[a][b]
    vector<int> RR((size_t)SZ * SZ, 0), rowsum(SZ, 0);
    for (int a = 0; a <= m; ++a) {
        int acc = 0;
        for (int u = 0; u <= m; ++u) {
            if (u <= m - a) acc = addmod(acc, cnt[(size_t)a * SZ + u]);
            RR[(size_t)a * SZ + u] = acc;
        }
        rowsum[a] = acc;
    }
    // column prefix: CC[b][u] = sum_{a<=min(u,m-b)} cnt[a][b]
    vector<int> CC((size_t)SZ * SZ, 0), colsum(SZ, 0);
    for (int b = 0; b <= m; ++b) {
        int acc = 0;
        for (int u = 0; u <= m; ++u) {
            if (u <= m - b) acc = addmod(acc, cnt[(size_t)u * SZ + b]);
            CC[(size_t)b * SZ + u] = acc;
        }
        colsum[b] = acc;
    }
    // 2D prefix P[x][y] = sum_{a<=x, b<=y} cnt[a][b]
    vector<int> P((size_t)SZ * SZ, 0);
    for (int x = 0; x <= m; ++x)
        for (int y = 0; y <= m; ++y) {
            long long v = cnt[(size_t)x * SZ + y];
            if (x > 0) v += P[(size_t)(x - 1) * SZ + y];
            if (y > 0) v += P[(size_t)x * SZ + (y - 1)];
            if (x > 0 && y > 0) v -= P[(size_t)(x - 1) * SZ + (y - 1)];
            v %= MOD; if (v < 0) v += MOD;
            P[(size_t)x * SZ + y] = (int)v;
        }
    vector<int> Apre(SZ, 0), Bpre(SZ, 0);
    { int acc = 0; for (int x = 0; x <= m; ++x) { acc = addmod(acc, rowsum[x]); Apre[x] = acc; } }
    { int acc = 0; for (int y = 0; y <= m; ++y) { acc = addmod(acc, colsum[y]); Bpre[y] = acc; } }
    const int Total = Apre[m];

    // QA[x][t] = sum_{a=x}^{m} RR[a][t-a],  QB[y][t] = sum_{b=y}^{m} CC[b][t-b]
    const int TT = max(m, 1);
    vector<int> QA((size_t)(m + 2) * TT, 0), QB((size_t)(m + 2) * TT, 0);
    for (int t = 0; t <= m - 1; ++t) {
        long long acc = 0;
        QA[(size_t)(m + 1) * TT + t] = 0;
        for (int a = m; a >= 0; --a) {
            int u = t - a;
            if (u >= 0 && u <= m) acc += RR[(size_t)a * SZ + u];
            acc %= MOD;
            QA[(size_t)a * TT + t] = (int)acc;
        }
        acc = 0;
        QB[(size_t)(m + 1) * TT + t] = 0;
        for (int b = m; b >= 0; --b) {
            int u = t - b;
            if (u >= 0 && u <= m) acc += CC[(size_t)b * SZ + u];
            acc %= MOD;
            QB[(size_t)b * TT + t] = (int)acc;
        }
    }

    string out;
    out.reserve((size_t)Q * 12);
    char tmp[16];
    for (int qi = 0; qi < Q; ++qi) {
        int x, y, z;
        readInt(x); readInt(y); readInt(z);
        if (x > m) x = m;
        if (y > m) y = m;
        if (z > m) z = m;
        int t = m - z - 1;
        int ans;
        if (t < 0) {
            ans = P[(size_t)x * SZ + y];
        } else {
            // S_AB = #{a>x, b>y} = Total - Apre[x] - Bpre[y] + P[x][y]
            int SAB = addmod(submod(submod(Total, Apre[x]), Bpre[y]), P[(size_t)x * SZ + y]);
            int S = submod(Total, QA[(size_t)0 * TT + t]);
            int sx1 = (x + 1 <= m) ? submod(Total, Apre[x]) : 0;
            int sy1 = (y + 1 <= m) ? submod(Total, Bpre[y]) : 0;
            int SA = submod(sx1, QA[(size_t)(x + 1) * TT + t]);
            int SB = submod(sy1, QB[(size_t)(y + 1) * TT + t]);
            ans = submod(addmod(submod(S, SA), SAB), SB);
        }
        int len = snprintf(tmp, sizeof(tmp), "%d\n", ans);
        out.append(tmp, len);
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

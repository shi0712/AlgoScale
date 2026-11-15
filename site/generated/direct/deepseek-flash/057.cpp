#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 1000000;

struct FastScanner {
    static constexpr int SZ = 1 << 20;
    int idx = 0, sz = 0;
    char buf[SZ];

    inline char getChar() {
        if (idx >= sz) {
            sz = (int)fread(buf, 1, SZ, stdin);
            idx = 0;
            if (sz == 0) return 0;
        }
        return buf[idx++];
    }

    int nextInt() {
        char c;
        do {
            c = getChar();
        } while (c < '0' || c > '9');

        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
};

long long modPow(long long a, long long e) {
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    vector<int> fac(MAXN + 1), ifac(MAXN + 1);

    fac[0] = 1;
    for (int i = 1; i <= MAXN; ++i) {
        fac[i] = (long long)fac[i - 1] * i % MOD;
    }

    ifac[MAXN] = (int)modPow(fac[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) {
        ifac[i - 1] = (long long)ifac[i] * i % MOD;
    }

    FastScanner fs;
    int T = fs.nextInt();

    string output;
    output.reserve((size_t)T * 16);

    vector<int> parent, childCnt, off, S, P, B, comb;

    while (T--) {
        int n = fs.nextInt();
        int k = fs.nextInt();

        if (k == 1) {
            for (int i = 2; i <= n; ++i) fs.nextInt();
            int ans = n % MOD;
            output += to_string(ans);
            output.push_back('\n');
            continue;
        }

        if (k == 2) {
            for (int i = 2; i <= n; ++i) fs.nextInt();
            int ans = (long long)n * (n - 1) % MOD;
            output += to_string(ans);
            output.push_back('\n');
            continue;
        }

        parent.assign(n + 1, 0);
        childCnt.assign(n + 1, 0);

        for (int i = 2; i <= n; ++i) {
            int p = fs.nextInt();
            parent[i] = p;
            ++childCnt[p];
        }

        off.assign(n + 2, 0);
        off[1] = 0;

        for (int v = 1; v <= n; ++v) {
            off[v + 1] = off[v] + childCnt[v] + 3;
        }

        int total = off[n + 1];

        S.assign(total, 0);

        for (int v = 1; v <= n; ++v) {
            S[off[v]] = 1;
        }

        // Compute subtree distance profiles S[v].
        for (int u = n; u >= 2; --u) {
            int v = parent[u];
            int len = childCnt[u] + 3;

            int suOff = off[u];
            int svOff = off[v];

            for (int d = 0; d < len; ++d) {
                S[svOff + d + 1] += S[suOff + d];
            }
        }

        int m = k - 1;

        comb.assign(n + 1, 0);

        int base = ifac[m];

        for (int x = m; x <= n; ++x) {
            comb[x] = (long long)fac[x] * base % MOD * ifac[x - m] % MOD;
        }

        P.assign(total, 0);
        B.assign(n + 1, 0);

        // Root.
        int c1 = childCnt[1];

        int *p1 = &P[off[1]];
        int *s1 = &S[off[1]];

        p1[0] = 1;

        for (int d = 1; d <= c1 + 2; ++d) {
            p1[d] = s1[d];
        }

        int b1 = 0;

        for (int d = 1; d <= c1 + 2; ++d) {
            b1 += comb[p1[d]];
            if (b1 >= MOD) b1 -= MOD;
        }

        B[1] = b1;
        int answer = b1;

        for (int v = 2; v <= n; ++v) {
            int u = parent[v];
            int cv = childCnt[v];

            int *pv = &P[off[v]];
            int *sv = &S[off[v]];
            int *pu = &P[off[u]];

            pv[0] = 1;

            int sumV = 0;

            for (int d = 1; d <= cv + 2; ++d) {
                int val = pu[d - 1] + sv[d];

                if (d >= 2) {
                    val -= sv[d - 2];
                }

                pv[d] = val;

                sumV += comb[val];
                if (sumV >= MOD) sumV -= MOD;
            }

            int sumParent = 0;

            for (int d = 1; d <= cv + 1; ++d) {
                sumParent += comb[pu[d]];
                if (sumParent >= MOD) sumParent -= MOD;
            }

            int bv = B[u] + sumV;
            if (bv >= MOD) bv -= MOD;

            bv -= sumParent;
            if (bv < 0) bv += MOD;

            B[v] = bv;

            answer += bv;
            if (answer >= MOD) answer -= MOD;
        }

        output += to_string(answer);
        output.push_back('\n');
    }

    fwrite(output.c_str(), 1, output.size(), stdout);
    return 0;
}

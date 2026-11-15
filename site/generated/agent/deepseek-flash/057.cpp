#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;
static const int MAXN = 1000000;

static int fac[MAXN + 1], ifac[MAXN + 1];

static inline int modpow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

static inline int comb(int x, int m) {
    if (x < m || m < 0) return 0;
    return (long long)fac[x] * ifac[m] % MOD * ifac[x - m] % MOD;
}

struct FastScanner {
    static const int SZ = 1 << 20;
    int idx, size;
    char buf[SZ];
    FastScanner() : idx(0), size(0) {}
    inline char read() {
        if (idx >= size) {
            size = (int)fread(buf, 1, SZ, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }
    template <class T>
    inline bool nextInt(T &out) {
        char c;
        T sign = 1;
        T num = 0;
        c = read();
        if (!c) return false;
        while (c != '-' && (c < '0' || c > '9')) {
            c = read();
            if (!c) return false;
        }
        if (c == '-') { sign = -1; c = read(); }
        for (; c >= '0' && c <= '9'; c = read())
            num = num * 10 + (c - '0');
        out = num * sign;
        return true;
    }
};

int main() {
    fac[0] = 1;
    for (int i = 1; i <= MAXN; ++i)
        fac[i] = (long long)fac[i - 1] * i % MOD;
    ifac[MAXN] = modpow(fac[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i)
        ifac[i - 1] = (long long)ifac[i] * i % MOD;

    FastScanner fs;
    int t;
    if (!fs.nextInt(t)) return 0;

    string output;
    output.reserve(t * 12);

    for (int tc = 0; tc < t; ++tc) {
        int n, k;
        fs.nextInt(n);
        fs.nextInt(k);
        vector<int> p(n + 1), ch(n + 1, 0);
        for (int i = 2; i <= n; ++i) {
            fs.nextInt(p[i]);
            ++ch[p[i]];
        }

        long long answer;
        if (k == 1) {
            answer = n % MOD;
        } else if (k == 2) {
            answer = (long long)n * (n - 1) % MOD;
        } else {
            const int m = k - 1;

            // h[v] = height of the subtree rooted at v
            vector<int> h(n + 1, 0);
            for (int v = n; v >= 2; --v) {
                int cand = h[v] + 1;
                int pv = p[v];
                if (cand > h[pv]) h[pv] = cand;
            }

            // Flat storage for down[v][0..h[v]]
            vector<int> off(n + 2);
            off[1] = 0;
            for (int v = 1; v <= n; ++v)
                off[v + 1] = off[v] + h[v] + 1;
            const int totalEntries = off[n + 1];
            vector<int> down(totalEntries, 0);

            // down[v][d]: vertices in subtree(v) at distance d from v
            for (int v = n; v >= 1; --v) {
                int bv = off[v];
                down[bv] = 1; // d = 0
                if (v > 1) {
                    int pv = p[v];
                    int bp = off[pv] + 1;
                    int hv = h[v];
                    int *dst = &down[bp];
                    int *src = &down[bv];
                    for (int d = 0; d <= hv; ++d)
                        dst[d] += src[d];
                }
            }

            // c[v][d] = number of all vertices at distance d from v,
            // stored only for d = 0..h[v].  A(v) = sum_{d>=1} C(c[v][d], m).
            vector<int> c(totalEntries, 0);

            int b1 = off[1], h1 = h[1];
            c[b1] = 1;
            vector<int> Av(n + 1, 0);
            int rootA = 0;
            for (int d = 1; d <= h1; ++d) {
                int x = down[b1 + d];
                c[b1 + d] = x;
                rootA += comb(x, m);
                if (rootA >= MOD) rootA -= MOD;
            }
            Av[1] = rootA;
            answer = rootA;

            for (int v = 2; v <= n; ++v) {
                int pv = p[v], bv = off[v], bp = off[pv], hv = h[v];

                int A = Av[pv];
                // d = 1 of v
                A += comb(1 + ch[v], m);
                if (A >= MOD) A -= MOD;

                // Replace the first hv+1 entries of the parent's profile by
                // c[v][d] = c[p][d-1] + down[v][d] - down[v][d-2], d=e+1.
                for (int e = 1; e <= hv + 1; ++e) {
                    int orig = c[bp + e];
                    int corr = orig;
                    if (e + 1 <= hv) corr += down[bv + e + 1];
                    corr -= down[bv + e - 1];

                    A += comb(corr, m);
                    if (A >= MOD) A -= MOD;
                    A -= comb(orig, m);
                    if (A < 0) A += MOD;
                }

                c[bv] = 1;
                if (hv >= 1) c[bv + 1] = 1 + ch[v];
                for (int d = 2; d <= hv; ++d) {
                    c[bv + d] = c[bp + d - 1] + down[bv + d] - down[bv + d - 2];
                }

                Av[v] = A;
                answer += A;
                if (answer >= MOD) answer -= MOD;
            }
        }

        output += to_string(answer);
        output.push_back('\n');
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}

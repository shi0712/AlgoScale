#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 1000000;

int fact[MAXN + 1], invfact[MAXN + 1];

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

void precompute() {
    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) {
        fact[i] = (long long)fact[i - 1] * i % MOD;
    }

    invfact[MAXN] = mod_pow(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) {
        invfact[i - 1] = (long long)invfact[i] * i % MOD;
    }
}

struct FastScanner {
    static constexpr int BUFSIZE = 1 << 20;
    int idx = 0, size = 0;
    char buf[BUFSIZE];

    inline char readChar() {
        if (idx >= size) {
            size = (int)fread(buf, 1, BUFSIZE, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }

    template <class T>
    bool nextInt(T &out) {
        char c = readChar();
        if (!c) return false;

        while (c != '-' && (c < '0' || c > '9')) {
            c = readChar();
            if (!c) return false;
        }

        T sign = 1;
        if (c == '-') {
            sign = -1;
            c = readChar();
        }

        T num = 0;
        while (c >= '0' && c <= '9') {
            num = num * 10 + (c - '0');
            c = readChar();
        }

        out = num * sign;
        return true;
    }
};

int main() {
    precompute();

    FastScanner fs;
    int T;
    fs.nextInt(T);

    vector<int> answers;
    answers.reserve(T);

    while (T--) {
        int n, k;
        fs.nextInt(n);
        fs.nextInt(k);

        vector<int> parent(n + 1);
        for (int i = 2; i <= n; ++i) {
            fs.nextInt(parent[i]);
        }

        if (k == 1) {
            answers.push_back(n % MOD);
            continue;
        }

        if (k == 2) {
            int ans = (long long)n * (n - 1) % MOD;
            answers.push_back(ans);
            continue;
        }

        int r = k - 1;

        // height[v] = maximum distance from v to a node in its subtree
        vector<int> height(n + 1, 0);
        for (int i = n; i >= 2; --i) {
            int p = parent[i];
            int val = height[i] + 1;
            if (val > height[p]) height[p] = val;
        }

        // F[v][d] = number of nodes in subtree(v) at distance d from v
        vector<int> offF(n + 1);
        int totalF = 0;
        for (int i = 1; i <= n; ++i) {
            offF[i] = totalF;
            totalF += height[i] + 1;
        }

        vector<int> F(totalF, 0);
        for (int i = 1; i <= n; ++i) {
            F[offF[i]] = 1;
        }

        for (int i = n; i >= 2; --i) {
            int p = parent[i];
            int oi = offF[i];
            int op = offF[p];
            int hi = height[i];

            for (int d = 0; d <= hi; ++d) {
                F[op + d + 1] += F[oi + d];
            }
        }

        // comb[x] = C(x, r)
        vector<int> comb(n + 1, 0);
        int invR = invfact[r];
        for (int x = r; x <= n; ++x) {
            comb[x] = (long long)fact[x] * invR % MOD * invfact[x - r] % MOD;
        }

        // A[v][d] is the number of nodes at distance d from v,
        // stored only for d = 0 .. height[v] + 2.
        vector<int> offA(n + 1);
        int totalA = 0;
        for (int i = 1; i <= n; ++i) {
            offA[i] = totalA;
            totalA += height[i] + 3;
        }

        vector<int> A(totalA, 0);
        vector<int> score(n + 1, 0);

        int hRoot = height[1];
        int offRootF = offF[1];

        long long rootScore = 0;
        for (int d = 1; d <= hRoot; ++d) {
            rootScore += comb[F[offRootF + d]];
            if (rootScore >= MOD) rootScore -= MOD;
        }

        score[1] = (int)rootScore;

        int offRootA = offA[1];
        int Lroot = hRoot + 2;
        for (int d = 0; d <= Lroot; ++d) {
            A[offRootA + d] = (d <= hRoot ? F[offRootF + d] : 0);
        }

        long long totalAnswer = score[1];

        for (int v = 2; v <= n; ++v) {
            int u = parent[v];
            int hv = height[v];
            int Lv = hv + 2;

            int offV = offA[v];
            int offU = offA[u];
            int offFv = offF[v];

            long long delta = 0;

            for (int d = 0; d <= Lv; ++d) {
                int base = (d >= 1 ? A[offU + d - 1] : 0);

                int add = 0;
                if (d <= hv) add = F[offFv + d];

                int sub = 0;
                if (d >= 2) {
                    int idx = d - 2;
                    if (idx <= hv) sub = F[offFv + idx];
                }

                int cur = base + add - sub;
                A[offV + d] = cur;

                delta += comb[cur] - comb[base];
            }

            delta %= MOD;
            if (delta < 0) delta += MOD;

            int sv = score[u] + (int)delta;
            if (sv >= MOD) sv -= MOD;
            score[v] = sv;

            totalAnswer += sv;
            if (totalAnswer >= MOD) totalAnswer -= MOD;
        }

        answers.push_back((int)totalAnswer);
    }

    for (int ans : answers) {
        printf("%d\n", ans);
    }

    return 0;
}

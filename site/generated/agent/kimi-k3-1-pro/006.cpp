#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

struct ModSum {
    int p;
    int64 c;
    int64 P, B, C;
    int smallC[5][5]{};
    vector<int> low, high;
    int fullSum;

    struct Context {
        int64 M;
        int rem;
        int V, S;
        int Mp, Mc, MVc;
        int full;
        vector<int> table;
    };

    int binomSmall(int n, int k) const {
        if (k < 0 || k > n) return 0;
        return smallC[n][k];
    }

    int binom(int64 n, int64 k) const {
        if (n < 0 || k < 0 || k > n) return 0;
        int ans = 1;
        while (n > 0 || k > 0) {
            int nd = n % p, kd = k % p;
            ans = ans * binomSmall(nd, kd) % p;
            n /= p;
            k /= p;
        }
        return ans;
    }

    void init(int prime, int64 bottom) {
        p = prime;
        c = bottom;
        for (int i = 0; i < p; ++i) {
            smallC[i][0] = smallC[i][i] = 1;
            for (int j = 1; j < i; ++j)
                smallC[i][j] = (smallC[i-1][j-1] + smallC[i-1][j]) % p;
        }
        P = 1;
        while (P <= c) P *= p;
        int K = 0;
        for (int64 x = P; x > 1; x /= p) ++K;
        int lowDigits = K / 2;
        if (p == 2 && K >= 4) lowDigits = (K - 1) / 2;
        B = 1;
        for (int i = 0; i < lowDigits; ++i) B *= p;
        C = P / B;
        low.resize(B);
        high.resize(C);
        int64 cl = c % B, ch = c / B;
        for (int i = 0; i < B; ++i) low[i] = binom(i, cl);
        for (int i = 0; i < C; ++i) high[i] = binom(i, ch);
        fullSum = (c == P - 1) ? 1 : 0;
    }

    int value(int64 n) const {
        n %= P;
        if (n < 0) n += P;
        return low[n % B] * high[n / B] % p;
    }

    Context makeContext(int64 count, int64 M) const {
        Context ctx;
        ctx.M = M;
        ctx.full = int((count / P) % p * fullSum % p);
        int64 rem64 = count % P;
        ctx.rem = int(rem64);
        ctx.Mp = M % P;
        ctx.Mc = M % C;
        ctx.V = ctx.rem / B;
        ctx.S = ctx.rem % B;
        ctx.MVc = int((int64)ctx.Mc * ctx.V % C);
        if (ctx.V > 0) {
            ctx.table.resize(C);
            int cur = 0;
            int idx = 0;
            for (int v = 0; v < ctx.V; ++v) {
                cur += high[idx];
                if (cur >= p) cur -= p;
                idx -= ctx.Mc;
                if (idx < 0) idx += C;
            }
            int x = 0;
            for (int t = 0; t < C; ++t) {
                ctx.table[x] = cur;
                int nx = x + ctx.Mc;
                if (nx >= C) nx -= C;
                int last = nx - ctx.MVc;
                if (last < 0) last += C;
                cur += high[nx] - high[last];
                cur %= p;
                if (cur < 0) cur += p;
                x = nx;
            }
        }
        return ctx;
    }

    int eval(const Context &ctx, int64 A) const {
        int ans = ctx.full;
        if (ctx.rem == 0) return ans;
        A %= P;
        if (A < 0) A += P;
        int64 D = A;
        if (ctx.V == 0) {
            for (int u = 0; u < ctx.S; ++u) {
                ans += value(D);
                ans %= p;
                D -= ctx.Mp;
                if (D < 0) D += P;
            }
            return ans;
        }
        for (int u = 0; u < B; ++u) {
            int X = D / B;
            int v = ctx.table[X];
            if (u < ctx.S) {
                int last = X - ctx.MVc;
                if (last < 0) last += C;
                v += high[last];
                if (v >= p) v -= p;
            }
            ans = (ans + low[D % B] * v) % p;
            D -= ctx.Mp;
            if (D < 0) D += P;
        }
        return ans;
    }

    int evalPair(const Context &ctx, int64 A1, int64 A2) const {
        int ans1 = ctx.full, ans2 = ctx.full;
        if (ctx.rem == 0) return (ans1 - ans2) % p;
        A1 %= P; A2 %= P;
        if (A1 < 0) A1 += P;
        if (A2 < 0) A2 += P;
        int64 D1 = A1, D2 = A2;
        if (ctx.V == 0) {
            for (int u = 0; u < ctx.S; ++u) {
                ans1 += value(D1);
                ans2 += value(D2);
                D1 -= ctx.Mp; if (D1 < 0) D1 += P;
                D2 -= ctx.Mp; if (D2 < 0) D2 += P;
            }
            int ans = (ans1 - ans2) % p;
            if (ans < 0) ans += p;
            return ans;
        }
        for (int u = 0; u < B; ++u) {
            int X1 = D1 / B, X2 = D2 / B;
            int v1 = ctx.table[X1], v2 = ctx.table[X2];
            if (u < ctx.S) {
                int last1 = X1 - ctx.MVc; if (last1 < 0) last1 += C;
                int last2 = X2 - ctx.MVc; if (last2 < 0) last2 += C;
                v1 += high[last1]; if (v1 >= p) v1 -= p;
                v2 += high[last2]; if (v2 >= p) v2 -= p;
            }
            ans1 = (ans1 + low[D1 % B] * v1) % p;
            ans2 = (ans2 + low[D2 % B] * v2) % p;
            D1 -= ctx.Mp; if (D1 < 0) D1 += P;
            D2 -= ctx.Mp; if (D2 < 0) D2 += P;
        }
        int ans = (ans1 - ans2) % p;
        if (ans < 0) ans += p;
        return ans;
    }
};

int computeMod(int64 n, int64 c, int p) {
    static const int h[11] = {0,0,8,7,3,7,6,3,7,2,5};
    ModSum ms;
    ms.init(p, c);
    int ans = 0;
    for (int64 Q = 1; Q <= n; ) {
        int64 M = 11 * Q;
        vector<ModSum::Context> contexts;
        vector<int64> counts;
        for (int d = 1; d <= 10; ++d) {
            int hd = h[d] % p;
            if (hd == 0 || Q * d > n) continue;
            int64 start = Q * d;
            int64 qmax = (n - start) / M;
            int64 lastStart = start + M * qmax;
            int64 length = n - lastStart + 1;
            int64 fullCount;
            bool partial;
            if (length >= Q) {
                fullCount = qmax + 1;
                partial = false;
            } else {
                fullCount = qmax;
                partial = true;
            }
            if (fullCount > 0) {
                int pos = -1;
                for (int i = 0; i < (int)counts.size(); ++i)
                    if (counts[i] == fullCount) { pos = i; break; }
                if (pos == -1) {
                    counts.push_back(fullCount);
                    contexts.push_back(ms.makeContext(fullCount, M));
                    pos = contexts.size() - 1;
                }
                int64 A1 = n + c - start;
                int64 A2 = A1 - Q;
                int z = ms.evalPair(contexts[pos], A1, A2);
                ans = (ans + hd * z) % p;
            }
            if (partial) {
                int z = ms.binom(length - 1 + c, c);
                ans = (ans + hd * z) % p;
            }
        }
        if (Q > n / 11) break;
        Q *= 11;
    }
    if (ans < 0) ans += p;
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    while (T--) {
        int64 n, c;
        cin >> n >> c;
        int s2 = computeMod(n, c, 2);
        int s5 = computeMod(n, c, 5);
        int x = s5;
        if ((x & 1) != s2) x += 5;
        int ans = 1;
        for (int i = 0; i < x; ++i) ans = ans * 2 % 11;
        cout << ans << '\n';
    }
    return 0;
}

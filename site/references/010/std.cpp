// Dark Queue / 暗匣中的指令
// O(m + n log^2 n) time, O(m+n) working space. No answer tables.
// Constraints: 2 <= n <= 100000, 1 <= m <= 200000.
// Adapted from the algorithm finalized in the accompanying conversation.
#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
constexpr int MOD = 998244353;
constexpr int ROOT = 3;

int mod_pow(int a, int e) {
    int r = 1;
    while (e > 0) {
        if (e & 1) r = int((int64)r * a % MOD);
        a = int((int64)a * a % MOD);
        e >>= 1;
    }
    return r;
}

void ntt(vector<int>& a, bool inverse) {
    int n = (int)a.size();
    for (int i = 1, j = 0; i < n; ++i) {
        int bit = n >> 1;
        while (j & bit) { j ^= bit; bit >>= 1; }
        j ^= bit;
        if (i < j) swap(a[i], a[j]);
    }
    for (int len = 2; len <= n; len <<= 1) {
        int step = mod_pow(ROOT, (MOD - 1) / len);
        if (inverse) step = mod_pow(step, MOD - 2);
        for (int l = 0; l < n; l += len) {
            int64 w = 1;
            for (int j = 0; j < len / 2; ++j) {
                int u = a[l + j];
                int v = int(w * a[l + j + len / 2] % MOD);
                int x = u + v;
                if (x >= MOD) x -= MOD;
                int y = u - v;
                if (y < 0) y += MOD;
                a[l + j] = x;
                a[l + j + len / 2] = y;
                w = w * step % MOD;
            }
        }
    }
    if (inverse) {
        int inv_n = mod_pow(n, MOD - 2);
        for (int& x : a) x = int((int64)x * inv_n % MOD);
    }
}

vector<int> convolution(vector<int> a, vector<int> b) {
    int need = (int)a.size() + (int)b.size() - 1;
    if (min(a.size(), b.size()) <= 32) {
        vector<int> c(need, 0);
        for (int i = 0; i < (int)a.size(); ++i)
            for (int j = 0; j < (int)b.size(); ++j)
                c[i + j] = int((c[i + j] + (int64)a[i] * b[j]) % MOD);
        return c;
    }
    int len = 1;
    while (len < need) len <<= 1;
    a.resize(len);
    b.resize(len);
    ntt(a, false);
    ntt(b, false);
    for (int i = 0; i < len; ++i)
        a[i] = int((int64)a[i] * b[i] % MOD);
    ntt(a, true);
    a.resize(need);
    return a;
}

struct Solver {
    int n;
    vector<int> inv, fac, ifac;
    vector<int> q, F;

    explicit Solver(int n_) : n(n_), inv(2*n + 1), fac(2*n + 1),
                              ifac(2*n + 1), q(n, 0), F(n) {
        inv[1] = 1;
        for (int i = 2; i <= 2*n; ++i)
            inv[i] = int(MOD - (int64)(MOD / i) * inv[MOD % i] % MOD);
        fac[0] = ifac[0] = 1;
        for (int i = 1; i <= 2*n; ++i) {
            fac[i] = int((int64)fac[i - 1] * i % MOD);
            ifac[i] = int((int64)ifac[i - 1] * inv[i] % MOD);
        }
    }

    // 已知次数不超过 s-1 的多项式在 0,...,s-1 的取值。
    // 用一次卷积补出 s,...,count-1 的取值。
    vector<int> extend_right(vector<int> f, int count) {
        int s = (int)f.size();
        if (s == count) return f;
        vector<int> a(s), b(count, 0);
        for (int i = 0; i < s; ++i) {
            a[i] = int((int64)f[i] * ifac[i] % MOD * ifac[s - 1 - i] % MOD);
            if ((s - 1 - i) & 1)
                a[i] = (a[i] == 0 ? 0 : MOD - a[i]);
        }
        for (int i = 1; i < count; ++i) b[i] = inv[i];
        vector<int> c = convolution(move(a), move(b));
        f.resize(count);
        for (int i = s; i < count; ++i)
            f[i] = int((int64)c[i] * fac[i] % MOD * ifac[i - s] % MOD);
        return f;
    }

    // 返回区间乘积多项式在 l,...,r+1 的取值。
    // 同时向 F[t] 中乘入当前区间内所有下标不小于 t 的因子。
    vector<int> divide(int l, int r) {
        if (l == r) {
            int x = 1 - q[l];
            if (x < 0) x += MOD;
            F[l] = x;
            return {x, 1};
        }
        int mid = (l + r) / 2;
        vector<int> left = divide(l, mid);
        vector<int> right = divide(mid + 1, r);
        int count = r - l + 2;

        left = extend_right(move(left), count);
        // 反转样本，把向左扩展转化为向右扩展。
        reverse(right.begin(), right.end());
        right = extend_right(move(right), count);
        reverse(right.begin(), right.end());

        for (int t = l; t <= mid; ++t)
            F[t] = int((int64)F[t] * right[t - l] % MOD);
        for (int i = 0; i < count; ++i)
            left[i] = int((int64)left[i] * right[i] % MOD);
        return left;
    }

    int solve(const vector<int>& a) {
        int m = (int)a.size();
        vector<int64> b(m + 1, 0), next_value(2*n + 1, 0);
        for (int i = m - 1; i >= 0; --i) {
            b[i] = b[i + 1] + 1;
            if (next_value[a[i]] != 0)
                b[i] = max(b[i], next_value[a[i]] + n + 1);
            next_value[a[i]] = b[i];
        }
        int64 base = b[0];
        // 有贡献的任务构成一个前缀，且其中的牌互不相同。
        for (int i = 0; i < m && base - b[i] < n; ++i) {
            int c = int(base - b[i]);
            int u = n - 1 - c;
            q[u] = inv[2*n - i];
        }
        divide(0, n - 1);
        int answer = int((base + n) % MOD);
        for (int x : F) {
            answer -= x;
            if (answer < 0) answer += MOD;
        }
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<int> a(m);
        for (int& x : a) cin >> x;
        Solver solver(n);
        cout << solver.solve(a) << '\n';
    }
    return 0;
}

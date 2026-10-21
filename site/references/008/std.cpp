// Elevenfold Lattice: on-the-fly O(sqrt(c) * (1 + log_11 n)).
// c <= 1e9. This is not an answer lookup table.
// Original conversation implementation, widened arithmetic for the new range.
#include <bits/stdc++.h>
using namespace std;

using u64 = unsigned long long;
using i64 = long long;

constexpr int LOG_A[11] = {0, 0, 8, 7, 3, 7, 6, 3, 7, 2, 5};
constexpr int DELTA[12] = {0, 0, 8, -1, -4, 4, -1, -3, 4, -5, 3, -5};
constexpr int POW2[10] = {1, 2, 4, 8, 5, 10, 9, 7, 3, 6};

int inverseMod(int a, int mod) {
    i64 r0 = mod, r1 = a, x0 = 0, x1 = 1;
    while (r1 != 0) {
        i64 q = r0 / r1;
        i64 r2 = r0 - q * r1;
        r0 = r1;
        r1 = r2;
        i64 x2 = x0 - q * x1;
        x0 = x1;
        x1 = x2;
    }
    assert(r0 == 1);
    x0 %= mod;
    if (x0 < 0) x0 += mod;
    return static_cast<int>(x0);
}

// 计算 sum_{j=0}^{floor(N/Q)} C(N-Q*j+c, c) mod p。
// 每次 prepare() 指定公差 Q，之后可以连续查询多个 N。
class BinomialProgression {
    int p, c, M = 1, H = 1, K;
    int choose[5][5]{};
    vector<int> low, high, pref;
    u64 Q = 1;
    int A = 0; // A = (-Q)^(-1) mod M

    // length 是 p 的幂。利用 Lucas 拆分，在线性时间内
    // 构造所有 C(i,r) mod p，其中 0 <= i < length。
    vector<int> makeWeights(int length, int r) const {
        vector<int> a(1, 1);
        while (static_cast<int>(a.size()) < length) {
            int size = static_cast<int>(a.size());
            int digit = r % p;
            r /= p;
            vector<int> b(size * p, 0);
            for (int d = 0; d < p; ++d) {
                int w = choose[d][digit];
                if (w == 0) continue;
                for (int i = 0; i < size; ++i) {
                    b[d * size + i] = a[i] * w % p;
                }
            }
            a.swap(b);
        }
        return a;
    }

public:
    BinomialProgression(int c_, int p_) : p(p_), c(c_) {
        int digits = 0;
        // M <= 2^30 (p=2) or 5^13 (p=5), both fit int.
        while (M <= c) {
            M *= p;
            ++digits;
        }
        for (int i = 0; i < digits / 2; ++i) H *= p;
        K = M / H;

        for (int i = 0; i < p; ++i) {
            choose[i][0] = choose[i][i] = 1;
            for (int j = 1; j < i; ++j) {
                choose[i][j] = (choose[i - 1][j - 1]
                              + choose[i - 1][j]) % p;
            }
        }
        low = makeWeights(H, c % H);
        high = makeWeights(K, c / H);
        pref.resize(K + 1);
    }

    // O(K + log M)：将高位权值按乘法置换重排，再求前缀和。
    void prepare(u64 step) {
        Q = step;
        int residue = static_cast<int>(Q % M);
        A = inverseMod(M - residue, M);

        fill(pref.begin(), pref.end(), 0);
        int pos = 0, add = A % K;
        for (int v = 0; v < K; ++v) {
            pref[pos + 1] = high[v];
            pos += add;
            if (pos >= K) pos -= K;
        }
        for (int i = 1; i <= K; ++i) {
            pref[i] = (pref[i] + pref[i - 1]) % p;
        }
    }

    // O(H).
    int sum(u64 N) const {
        u64 count = N / Q + 1;

        // 整周期和为 C(M,c+1)，只有 c=M-1 时不为零。
        i64 ans = (c == M - 1)
                    ? static_cast<int>((count / M) % p) : 0;
        int len = static_cast<int>(count % M);
        if (len == 0) return static_cast<int>(ans);

        int b = static_cast<int>((N + static_cast<u64>(c)) % M);
        // z + A can exceed INT_MAX when M = 5^13.
        i64 z = (M - 1LL * A * b % M) % M;
        int whole = len / H, remain = len % H;

        for (int u = 0; u < H; ++u) {
            if (low[u] != 0) {
                int zHigh = static_cast<int>(z / H);
                int zLow = static_cast<int>(z % H);
                int take = whole + (zLow < remain);
                int left = (zHigh == 0 ? 0 : K - zHigh);
                int right = left + take;

                int part;
                if (right <= K) {
                    part = pref[right] - pref[left];
                } else {
                    part = pref[K] - pref[left] + pref[right - K];
                }
                ans += low[u] * part;
            }
            z += A;
            if (z >= M) z -= M;
        }
        ans %= p;
        if (ans < 0) ans += p;
        return static_cast<int>(ans);
    }
};

int solvePrime(u64 n, int c, int p) {
    BinomialProgression calc(c, p);
    int ans = 0;

    for (u64 q = 1; ; q *= 11) {
        calc.prepare(q * 11);
        int limit = static_cast<int>(min<u64>(11, n / q));
        for (int d = 1; d <= limit; ++d) {
            int coefficient = (DELTA[d] % p + p) % p;
            if (coefficient == 0) continue;
            ans = (ans + coefficient * calc.sum(n - q * d)) % p;
        }
        if (q > n / 11) break;
    }
    return ans;
}

int solve(u64 n, int c) {
    if (c == 0) {
        int exponent = 0;
        while (n != 0) {
            exponent += LOG_A[n % 11];
            n /= 11;
        }
        return POW2[exponent % 10];
    }

    int r2 = solvePrime(n, c, 2);
    int r5 = solvePrime(n, c, 5);
    int Jprsj = r5;
    if ((Jprsj & 1) != r2) Jprsj += 5;
    return POW2[Jprsj];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        u64 n;
        int c;
        cin >> n >> c;
        cout << solve(n, c) << '\n';
    }
    return 0;
}

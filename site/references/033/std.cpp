// Standard solution: the O(n^2) weighted DP discussed in the conversation.
// No answer table. C++17, standard input/output.
#include <bits/stdc++.h>
namespace collection_dp {
using i64 = long long;
constexpr int MOD = 998244353;
int power(int a, int b) {
    int r = 1;
    for (; b; b >>= 1, a = static_cast<i64>(a) * a % MOD)
        if (b & 1) r = static_cast<i64>(r) * a % MOD;
    return r;
}
inline void add(int &a, int b) {
    a += b;  // a+b < 2*MOD < INT_MAX.
    if (a >= MOD) a -= MOD;
}
int solve(int n, const std::vector<int>& a, const std::vector<int>& b) {
    const int m = static_cast<int>(a.size());
    std::vector<int> fac(n + 1, 1), ifac(n + 1, 1);
    for (int j = 1; j <= n; ++j) fac[j] = static_cast<i64>(fac[j-1]) * j % MOD;
    ifac[n] = power(fac[n], MOD - 2);
    for (int j = n; j >= 1; --j) ifac[j-1] = static_cast<i64>(ifac[j]) * j % MOD;

    using State = std::array<int, 2>;
    std::vector<State> f(n + 1), g(n + 1), nf(n + 1), ng(n + 1);
    f[0][0] = 1;
    int s = 0;
    for (int i = 0; i < m; ++i) {
        std::fill(nf.begin(), nf.end(), State{0, 0});
        std::fill(ng.begin(), ng.end(), State{0, 0});
        for (int x = 0; x <= a[i]; ++x) {
            const int z = static_cast<i64>(ifac[x]) * ifac[a[i]-x] % MOD;
            const int bad = (x < b[i]);
            const int remaining = a[i] - std::min(x, b[i]);
            for (int j = 0; j <= s; ++j) {
                for (int o = 0; o < 2; ++o) {
                    const int no = o | bad;
                    const int df = static_cast<i64>(f[j][o]) * z % MOD;
                    // Reduce before multiplying by z: all i64 products are < MOD^2.
                    const int dg = (g[j][o] + static_cast<i64>(f[j][o]) * remaining) % MOD
                                   * static_cast<i64>(z) % MOD;
                    add(nf[j+x][no], df);
                    add(ng[j+x][no], dg);
                }
            }
        }
        f.swap(nf);
        g.swap(ng);
        s += a[i];
    }
    int ans = 0;
    // j == n must be excluded: g[n][1] == 0, and (n-j-1)! is undefined.
    for (int j = 0; j < n; ++j) {
        const int term = static_cast<i64>(g[j][1]) * fac[j] % MOD * fac[n-j-1] % MOD;
        add(ans, term);
    }
    ans = static_cast<i64>(ans) * ifac[n] % MOD;
    for (int ai : a) ans = static_cast<i64>(ans) * fac[ai] % MOD;
    return ans;
}
} // namespace collection_dp
#ifndef COLLECTION_LIBRARY
int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    int n, m;
    if (!(std::cin >> n >> m)) return 1;
    std::vector<int> a(m), b(m);
    for (int& x : a) if (!(std::cin >> x)) return 1;
    for (int& x : b) if (!(std::cin >> x)) return 1;
    std::cout << collection_dp::solve(n, a, b) << '\n';
}
#endif

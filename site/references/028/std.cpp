#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAX_A = 6000;

int mod_pow(int a, int e) {
    int result = 1;
    while (e > 0) {
        if (e & 1) result = (long long)result * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return result;
}

// 乘法 SOS：f[S] = 原数组中所有 f[T]（T 为 S 的子集）的乘积。
void product_zeta(vector<int>& f, int bits) {
    int size = (int)f.size();
    for (int bit = 0; bit < bits; ++bit) {
        int step = 1 << bit;
        for (int base = 0; base < size; base += step * 2) {
            for (int j = 0; j < step; ++j) {
                f[base + step + j] =
                    (long long)f[base + step + j] * f[base + j] % MOD;
            }
        }
    }
}

// 原函数仅依赖低位；增加新的位时，复制原来的函数值。
void extend(vector<int>& f, int size) {
    int old_size = (int)f.size();
    if (old_size >= size) return;
    f.resize(size);
    for (int s = old_size; s < size; ++s) {
        f[s] = f[s & (old_size - 1)];
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> weight(MAX_A + 1, 1);
    vector<bool> used(MAX_A + 1, false);
    int m = 1;
    for (int i = 0; i < n; ++i) {
        int a;
        cin >> a;
        used[a] = true;
        weight[a] = (long long)weight[a] * (a + 1) % MOD;
        m = max(m, a);
    }

    // 线性筛求最小质因子。
    vector<int> spf(m + 1, 0), primes;
    for (int x = 2; x <= m; ++x) {
        if (spf[x] == 0) {
            spf[x] = x;
            primes.push_back(x);
        }
        for (int p : primes) {
            if (1LL * p * x > m) break;
            spf[p * x] = p;
            if (p == spf[x]) break;
        }
    }

    // 前 K 个质数作为小质数，使两个不同的大质数之积大于 m。
    // m = 6000 时，K = 21，小质数为 2 到 73。
    int K = 0;
    while (K + 1 < (int)primes.size() &&
           1LL * primes[K] * primes[K + 1] <= m) {
        ++K;
    }

    vector<int> id(m + 1, -1);
    for (int j = 0; j < K; ++j) id[primes[j]] = j;

    vector<int> mask(m + 1, 0), large(m + 1, 0);
    for (int x = 2; x <= m; ++x) {
        int p = spf[x], rest = x / p;
        mask[x] = mask[rest];
        large[x] = large[rest];
        if (id[p] != -1) mask[x] |= 1 << id[p];
        else large[x] = p;
    }

    // 0 组没有大质因子；p 组的大质因子为 p。
    vector<vector<pair<int, int>>> groups(m + 1);
    for (int x = 1; x <= m; ++x) {
        if (used[x]) groups[large[x]].push_back({mask[x], weight[x]});
    }

    vector<int> acc(1, 1);
    int denominator = 1;
    int r = 0;

    // 大质数从大到小处理，所依赖的小质数数量 r 单调不减。
    for (int j = (int)primes.size() - 1; j >= K; --j) {
        int p = primes[j];
        if (groups[p].empty()) continue;

        while (r < K && primes[r] <= m / p) ++r;
        int size = 1 << r;
        vector<int> f(size, 1);
        for (auto [s, w] : groups[p]) {
            f[s] = (long long)f[s] * w % MOD;
        }
        product_zeta(f, r);

        extend(acc, size);
        for (int s = 0; s < size; ++s) {
            int factor = (1LL * (p - 1) * f[s] + 1) % MOD;
            acc[s] = (long long)acc[s] * factor % MOD;
        }
        denominator = (long long)denominator * p % MOD;
    }

    int size = 1 << K;
    extend(acc, size);

    vector<int> f(size, 1);
    for (auto [s, w] : groups[0]) {
        f[s] = (long long)f[s] * w % MOD;
    }
    product_zeta(f, K);

    for (int j = 0; j < K; ++j) {
        denominator = (long long)denominator * primes[j] % MOD;
    }

    // subset_weight[S] = S 中所有小质数对应的 (p - 1) 的乘积。
    vector<int> subset_weight(size, 1);
    int answer = 0;
    for (int s = 0; s < size; ++s) {
        if (s != 0) {
            int bit = __builtin_ctz((unsigned)s);
            subset_weight[s] = (long long)subset_weight[s & (s - 1)]
                               * (primes[bit] - 1) % MOD;
        }
        int term = (long long)acc[s] * f[s] % MOD;
        term = (long long)term * subset_weight[s] % MOD;
        answer += term;
        if (answer >= MOD) answer -= MOD;
    }

    cout << (long long)answer * mod_pow(denominator, MOD - 2) % MOD << '\n';
    return 0;
}

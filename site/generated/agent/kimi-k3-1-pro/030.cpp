#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;
    vector<int> a(m), b(m);
    for (int &x : a) cin >> x;
    for (int &x : b) cin >> x;

    vector<int> fact(n + 1), inv_fact(n + 1), inv(n + 1);
    fact[0] = 1;
    for (int i = 1; i <= n; ++i) fact[i] = 1LL * fact[i - 1] * i % MOD;
    inv[1] = 1;
    for (int i = 2; i <= n; ++i)
        inv[i] = 1LL * (MOD - MOD / i) * inv[MOD % i] % MOD;
    inv_fact[0] = 1;
    for (int i = 1; i <= n; ++i)
        inv_fact[i] = 1LL * inv_fact[i - 1] * inv[i] % MOD;

    auto choose = [&](int x, int y) {
        return 1LL * fact[x] * inv_fact[y] % MOD * inv_fact[x - y] % MOD;
    };

    vector<int> complete = {1};
    vector<int> collected(n + 1);
    int target = 0;

    for (int i = 0; i < m; ++i) {
        target += b[i];
        if (b[i] == 0) continue;

        vector<int> incomplete(a[i] + 1), before_complete(a[i] + 1);
        for (int k = 0; k < b[i]; ++k) {
            for (int j = 0; j <= k; ++j) {
                int probability = 1LL * choose(a[i], k) * choose(k, j) % MOD;
                if (j & 1) probability = (MOD - probability) % MOD;
                int d = a[i] - k + j;
                incomplete[d] = (incomplete[d] + probability) % MOD;
                before_complete[d] = (before_complete[d] +
                    1LL * k * probability) % MOD;
            }
        }

        vector<int> type_complete(a[i] + 1);
        type_complete[0] = 1;
        for (int d = 0; d <= a[i]; ++d) {
            type_complete[d] = (type_complete[d] + MOD - incomplete[d]) % MOD;
            int mean = (before_complete[d] + 1LL * b[i] * type_complete[d]) % MOD;
            collected[d] = (collected[d] + mean) % MOD;
        }

        vector<int> product(complete.size() + type_complete.size() - 1);
        for (int x = 0; x < (int)complete.size(); ++x) {
            for (int y = 0; y < (int)type_complete.size(); ++y) {
                product[x + y] = (product[x + y] +
                    1LL * complete[x] * type_complete[y]) % MOD;
            }
        }
        complete.swap(product);
    }

    int answer = 0;
    for (int d = 1; d <= n; ++d) {
        int coefficient = (MOD - collected[d]) % MOD;
        if (d < (int)complete.size()) {
            coefficient = (coefficient +
                1LL * (target - n + MOD) * complete[d]) % MOD;
        }
        answer = (answer + 1LL * coefficient * inv[d]) % MOD;
    }
    cout << answer << '\n';
    return 0;
}

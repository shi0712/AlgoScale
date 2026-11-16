#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<vector<int>> tree(n + 1);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        tree[u].push_back(v);
        tree[v].push_back(u);
    }

    vector<long long> fact(n + 1), inv_fact(n + 1), inverse(n + 1);
    fact[0] = 1;
    for (int i = 1; i <= n; ++i) fact[i] = fact[i - 1] * i % MOD;
    inv_fact[n] = 1;
    long long base = fact[n], power = MOD - 2;
    while (power > 0) {
        if (power & 1) inv_fact[n] = inv_fact[n] * base % MOD;
        base = base * base % MOD;
        power >>= 1;
    }
    for (int i = n; i > 0; --i) inv_fact[i - 1] = inv_fact[i] * i % MOD;
    inverse[1] = 1;
    for (int i = 2; i <= n; ++i) inverse[i] = MOD - MOD / i * inverse[MOD % i] % MOD;

    vector<vector<int>> binom(n + 1, vector<int>(n + 1));
    vector<vector<int>> inv_binom(n + 1, vector<int>(n + 1));
    for (int a = 0; a <= n; ++a) {
        binom[a][0] = binom[a][a] = 1;
        for (int b = 1; b < a; ++b) {
            binom[a][b] = (binom[a - 1][b - 1] + binom[a - 1][b]) % MOD;
        }
        for (int b = 0; b <= a; ++b) {
            inv_binom[a][b] = fact[b] * fact[a - b] % MOD * inv_fact[a] % MOD;
        }
    }
    auto falling = [&](int a, int b) -> long long {
        return fact[a] * inv_fact[a - b] % MOD;
    };

    vector<int> parent(n + 1), order;
    order.reserve(n);
    parent[1] = -1;
    order.push_back(1);
    for (int i = 0; i < n; ++i) {
        int v = order[i];
        for (int u : tree[v]) {
            if (u == parent[v]) continue;
            parent[u] = v;
            order.push_back(u);
        }
    }
    vector<vector<int>> children(n + 1);
    for (int v = 2; v <= n; ++v) children[parent[v]].push_back(v);

    auto multiply_color = [&](const vector<vector<int>> &h, int old_size, int s) {
        int new_size = old_size + s;
        vector<vector<int>> result(new_size + 1, vector<int>(new_size + 1));
        for (int row = 0; row <= old_size; ++row) {
            vector<int> previous2(row + s + 1), previous(row + s + 1), current(row + s + 1);
            for (int j = 0; j <= row; ++j) previous[j] = h[row][j];
            for (int j = 0; j <= row; ++j) result[row][j] = (result[row][j] + h[row][j]) % MOD;
            for (int r = 1; r <= s; ++r) {
                for (int j = 0; j <= row + r; ++j) {
                    long long value = (2LL * r - 1) * previous[j];
                    if (j > 0) value -= previous[j - 1];
                    if (r > 1) value -= 1LL * (r - 1) * previous2[j];
                    current[j] = value % MOD * inverse[r] % MOD;
                    if (current[j] < 0) current[j] += MOD;
                }
                long long coefficient = falling(s, r);
                if (r & 1) coefficient = MOD - coefficient;
                for (int j = 0; j <= row + r; ++j) {
                    int difference = current[j] - previous[j];
                    if (difference < 0) difference += MOD;
                    int &target = result[row + r][j];
                    target = (target + coefficient * difference) % MOD;
                }
                previous2.swap(previous);
                previous.swap(current);
            }
        }
        return result;
    };

    auto remove_color = [&](const vector<vector<int>> &all, int s, int outside_size) {
        vector<vector<int>> result(outside_size + 1, vector<int>(outside_size + 1));
        for (int row = 0; row <= outside_size; ++row) {
            for (int j = 0; j <= row; ++j) result[row][j] = all[row][j];
        }
        for (int row = 0; row <= outside_size; ++row) {
            int limit = min(s, outside_size - row);
            vector<int> previous2(row + limit + 1), previous(row + limit + 1), current(row + limit + 1);
            for (int j = 0; j <= row; ++j) previous[j] = result[row][j];
            for (int r = 1; r <= limit; ++r) {
                for (int j = 0; j <= row + r; ++j) {
                    long long value = (2LL * r - 1) * previous[j];
                    if (j > 0) value -= previous[j - 1];
                    if (r > 1) value -= 1LL * (r - 1) * previous2[j];
                    current[j] = value % MOD * inverse[r] % MOD;
                    if (current[j] < 0) current[j] += MOD;
                }
                for (int j = 0; j <= row + r; ++j) {
                    int difference = previous[j] - current[j];
                    if (difference < 0) difference += MOD;
                    int &target = result[row + r][j];
                    long long coefficient = falling(s, r);
                    if (r & 1) coefficient = MOD - coefficient;
                    target = (target + coefficient * difference) % MOD;
                }
                previous2.swap(previous);
                previous.swap(current);
            }
        }
        return result;
    };

    auto alternating_counts = [&](const vector<vector<int>> &h, int total_size) {
        vector<int> result(total_size + 1);
        for (int row = 0; row <= total_size; ++row) {
            long long value = 0;
            for (int j = 0; j <= row; ++j) value += fact[j] * h[row][j] % MOD;
            result[row] = value % MOD;
        }
        return result;
    };

    auto ending_outside_counts = [&](const vector<vector<int>> &h, int outside_size, int s) {
        vector<vector<int>> result(outside_size + 1, vector<int>(s + 1));
        for (int row = 0; row <= outside_size; ++row) {
            vector<int> previous2(row + s + 1), previous(row + s + 1), current(row + s + 1);
            for (int j = 1; j <= row; ++j) previous[j] = 1LL * j * h[row][j] % MOD;
            for (int r = 0; r <= s; ++r) {
                if (r > 0) {
                    for (int j = 0; j <= row + r; ++j) {
                        long long value = (2LL * r - 1) * previous[j];
                        if (j > 0) value -= previous[j - 1];
                        if (r > 1) value -= 1LL * (r - 1) * previous2[j];
                        current[j] = value % MOD * inverse[r] % MOD;
                        if (current[j] < 0) current[j] += MOD;
                    }
                }
                long long value = 0;
                for (int j = 1; j <= row + r; ++j) {
                    int term = current[j];
                    if (r == 0) term = previous[j];
                    else {
                        term -= previous[j];
                        if (term < 0) term += MOD;
                    }
                    value += fact[j - 1] * term % MOD;
                }
                long long coefficient = falling(s, r);
                if (r & 1) coefficient = MOD - coefficient;
                result[row][r] = value % MOD * coefficient % MOD;
                if (r > 0) {
                    previous2.swap(previous);
                    previous.swap(current);
                }
            }
        }
        result[0][0] = 1;
        return result;
    };

    vector<int> size(n + 1, 1);
    vector<vector<int>> dp(n + 1);
    for (int oi = n - 1; oi >= 0; --oi) {
        int v = order[oi];
        for (int u : children[v]) size[v] += size[u];

        vector<vector<int>> runs(2, vector<int>(2));
        runs[0][0] = runs[1][1] = 1;
        int merged_size = 1;
        for (int u : children[v]) {
            runs = multiply_color(runs, merged_size, size[u]);
            merged_size += size[u];
        }
        dp[v] = alternating_counts(runs, size[v]);

        for (int u : children[v]) {
            int s = size[u];
            if (s < 2) continue;
            int outside_size = size[v] - s;
            vector<vector<int>> outside_runs = remove_color(runs, s, outside_size);
            vector<vector<int>> prefix = ending_outside_counts(outside_runs, outside_size, s);
            for (int outside = 0; outside <= outside_size; ++outside) {
                for (int m = 0; m <= s - 2; ++m) {
                    long long prefix_ways = prefix[outside][m];
                    if (prefix_ways == 0) continue;
                    for (int k = 2; k <= s - m; ++k) {
                        long long suffix_ways = 1LL * dp[u][k] * binom[s - m][k] % MOD
                                              * inv_binom[s][k] % MOD;
                        int &target = dp[v][outside + m + k];
                        target = (target + prefix_ways * suffix_ways) % MOD;
                    }
                }
            }
        }
    }

    cout << dp[1][n] << '\n';
    return 0;
}

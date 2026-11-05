#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

namespace {
constexpr int MOD = 998244353;

inline void add_to(int& x, int64_t y) {
    x = static_cast<int>((x + y) % MOD);
}
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<int> left(n), right(n);
    for (int i = 0; i < n; ++i) {
        cin >> left[i] >> right[i];
        --left[i];
        --right[i];
    }

    vector<vector<int>> weight(n, vector<int>(n));
    vector<int> path(n);
    for (int length = 1; length < n; ++length) {
        for (int l = 0; l + length < n; ++l) {
            const int r = l + length;
            fill(path.begin() + l, path.begin() + r + 1, 0);
            path[l] = 1;
            for (int v = l + 1; v < r; ++v) {
                if (left[v] <= l || right[v] >= r) continue;
                int64_t sum = 0;
                for (int u = l; u < v; ++u) {
                    sum += static_cast<int64_t>(path[u]) * weight[u][v] % MOD;
                    if (sum >= (INT64_C(1) << 62)) sum %= MOD;
                }
                path[v] = static_cast<int>(sum % MOD);
            }
            int64_t bad = 0;
            for (int u = l + 1; u < r; ++u) {
                bad += static_cast<int64_t>(path[u]) * weight[u][r] % MOD;
                if (bad >= (INT64_C(1) << 62)) bad %= MOD;
            }
            weight[l][r] = static_cast<int>((1 - bad % MOD + MOD) % MOD);
        }
    }

    const size_t plane = static_cast<size_t>(n) * n;
    vector<int> dp(static_cast<size_t>(n) * plane);
    auto at = [&](int i, int bound, int open) -> int& {
        return dp[static_cast<size_t>(i) * plane + static_cast<size_t>(bound) * n + open];
    };

    int answer = 0;
    for (int i = 0; i < n; ++i) {
        add_to(at(i, right[i], i), 1);
        for (int bound = i; bound < n; ++bound) {
            for (int open = 0; open <= i; ++open) {
                const int current = at(i, bound, open);
                if (current == 0) continue;
                if (open == i) {
                    answer += current;
                    if (answer >= MOD) answer -= MOD;
                }
                for (int next = i + 1; next <= bound; ++next) {
                    const int next_bound = max(bound, right[next]);
                    const int next_open = left[next] <= open ? next : open;
                    add_to(at(next, next_bound, next_open),
                           static_cast<int64_t>(current) * weight[i][next] % MOD);
                }
            }
        }
    }
    cout << answer << '\n';
    return 0;
}

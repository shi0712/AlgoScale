#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

int mod_pow(int a, int e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = (long long)a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        int n, k;
        cin >> n >> k;
        vector<int> parent(n + 1), head(n + 1, -1), nxt(n + 1, -1);
        for (int v = 2; v <= n; ++v) {
            cin >> parent[v];
            nxt[v] = head[parent[v]];
            head[parent[v]] = v;
        }

        if (k == 1) {
            cout << n << '\n';
            continue;
        }

        int m = k - 1;
        vector<int> fact(n + 1), inv_fact(n + 1);
        fact[0] = 1;
        for (int i = 1; i <= n; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
        inv_fact[n] = mod_pow(fact[n], MOD - 2);
        for (int i = n; i >= 1; --i) inv_fact[i - 1] = (long long)inv_fact[i] * i % MOD;
        auto choose = [&](int x) -> int {
            if (x < m) return 0;
            return (long long)fact[x] * inv_fact[m] % MOD * inv_fact[x - m] % MOD;
        };

        vector<int> height(n + 1, 0), offset(n + 1);
        for (int v = n; v >= 1; --v) {
            for (int c = head[v]; c != -1; c = nxt[c]) {
                height[v] = max(height[v], height[c] + 1);
            }
        }
        int total = 0;
        for (int v = 1; v <= n; ++v) {
            offset[v] = total;
            total += height[v] + 1;
        }
        vector<int> down(total, 0);
        for (int v = n; v >= 1; --v) {
            int ov = offset[v];
            down[ov] = 1;
            for (int c = head[v]; c != -1; c = nxt[c]) {
                int oc = offset[c];
                for (int d = 0; d <= height[c]; ++d) {
                    down[ov + d + 1] += down[oc + d];
                }
            }
        }

        vector<int> cur(2 * height[1] + 3, 0);
        int initial_shift = height[1] + 1;
        for (int d = 0; d <= height[1]; ++d) {
            cur[initial_shift + d] = down[offset[1] + d];
        }

        long long answer = 0;
        auto dfs = [&](auto&& self, int v, int shift, long long value) -> void {
            answer += value;
            if (answer >= MOD) answer -= MOD;
            for (int c = head[v]; c != -1; c = nxt[c]) {
                int child_shift = shift - 1;
                int old_zero = cur[child_shift];
                cur[child_shift] = 1;
                long long child_value = value + choose(1);
                if (child_value >= MOD) child_value -= MOD;
                int oc = offset[c];
                for (int d = 1; d <= height[c] + 2; ++d) {
                    int delta = (d <= height[c] ? down[oc + d] : 0)
                              - (d >= 2 ? down[oc + d - 2] : 0);
                    int pos = child_shift + d;
                    int old = cur[pos];
                    int now = old + delta;
                    cur[pos] = now;
                    child_value += choose(now) - choose(old);
                    if (child_value >= MOD) child_value -= MOD;
                    if (child_value < 0) child_value += MOD;
                }
                self(self, c, child_shift, child_value);
                cur[child_shift] = old_zero;
                for (int d = 1; d <= height[c] + 2; ++d) {
                    int delta = (d <= height[c] ? down[oc + d] : 0)
                              - (d >= 2 ? down[oc + d - 2] : 0);
                    cur[child_shift + d] -= delta;
                }
            }
        };

        long long root_value = 0;
        for (int d = 1; d <= height[1]; ++d) {
            root_value += choose(cur[initial_shift + d]);
            if (root_value >= MOD) root_value -= MOD;
        }
        dfs(dfs, 1, initial_shift, root_value);
        cout << answer << '\n';
    }
    return 0;
}

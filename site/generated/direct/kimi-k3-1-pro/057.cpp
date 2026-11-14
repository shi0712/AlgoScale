#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;
static constexpr int MAXN = 1000000;

int mod_pow(int a, int e) {
    long long r = 1, x = a;
    while (e > 0) {
        if (e & 1) r = r * x % MOD;
        x = x * x % MOD;
        e >>= 1;
    }
    return static_cast<int>(r);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    vector<int> fact(MAXN + 1), inv_fact(MAXN + 1);
    fact[0] = 1;
    for (int i = 1; i <= MAXN; ++i) {
        fact[i] = static_cast<long long>(fact[i - 1]) * i % MOD;
    }
    inv_fact[MAXN] = mod_pow(fact[MAXN], MOD - 2);
    for (int i = MAXN; i >= 1; --i) {
        inv_fact[i - 1] = static_cast<long long>(inv_fact[i]) * i % MOD;
    }

    while (T--) {
        int n, k;
        cin >> n >> k;

        vector<int> parent(n + 1);
        for (int v = 2; v <= n; ++v) {
            cin >> parent[v];
        }

        if (k == 1) {
            cout << n << '\n';
            continue;
        }

        const int m = k - 1;

        // Heights of rooted subtrees.
        vector<int> height(n + 1, 0);
        for (int v = n; v >= 2; --v) {
            int p = parent[v];
            height[p] = max(height[p], height[v] + 1);
        }

        // a[v] has length height[v]+1.
        // dist[v] and suffix[v] have length height[v]+3.
        vector<int> a_offset(n + 2, 0), d_offset(n + 2, 0);
        for (int v = 1; v <= n; ++v) {
            a_offset[v + 1] = a_offset[v] + height[v] + 1;
            d_offset[v + 1] = d_offset[v] + height[v] + 3;
        }

        vector<int> a(a_offset[n + 1], 0);
        vector<int> dist(d_offset[n + 1]);
        vector<int> suffix(d_offset[n + 1]);

        for (int v = 1; v <= n; ++v) {
            a[a_offset[v]] = 1;
        }

        // Merge each child's descendant-level profile into its parent.
        for (int v = n; v >= 2; --v) {
            int p = parent[v];
            int av = a_offset[v];
            int ap = a_offset[p];
            for (int r = 0; r <= height[v]; ++r) {
                a[ap + r + 1] += a[av + r];
            }
        }

        const int inv_fact_m = inv_fact[m];
        auto choose_m = [&](int x) -> int {
            if (x < m) return 0;
            long long result = static_cast<long long>(fact[x]) *
                               inv_fact[x - m] % MOD;
            result = result * inv_fact_m % MOD;
            return static_cast<int>(result);
        };

        int answer = 0;

        // parent[v] < v, so all parent profiles are already available.
        for (int v = 1; v <= n; ++v) {
            const int hv = height[v];
            const int L = hv + 2;
            const int bv = d_offset[v];
            const int av = a_offset[v];

            dist[bv] = 1;

            if (v == 1) {
                for (int d = 1; d <= L; ++d) {
                    dist[bv + d] = (d <= hv ? a[av + d] : 0);
                }
            } else {
                const int bp = d_offset[parent[v]];
                for (int d = 1; d <= L; ++d) {
                    int value = dist[bp + d - 1];
                    if (d <= hv) value += a[av + d];
                    if (d >= 2 && d - 2 <= hv) value -= a[av + d - 2];
                    dist[bv + d] = value;
                }
            }

            // suffix[t] = sum_{d>=max(t,1)} C(b_v(d),m).
            // Distances after L are inherited from the parent's suffix.
            int running = 0;
            if (v != 1) {
                running = suffix[d_offset[parent[v]] + L];
            }

            for (int t = L; t >= 1; --t) {
                running += choose_m(dist[bv + t]);
                if (running >= MOD) running -= MOD;
                suffix[bv + t] = running;
            }
            suffix[bv] = running;

            answer += suffix[bv + 1];
            if (answer >= MOD) answer -= MOD;
        }

        cout << answer << '\n';
    }

    return 0;
}

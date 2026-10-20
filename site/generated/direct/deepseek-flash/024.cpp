#include <bits/stdc++.h>
using namespace std;

static const int MOD = 1000000007;

long long mod_pow(long long a, long long e) {
    long long r = 1;
    while (e > 0) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<int>> adj(n);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    vector<int> parent(n, -1), order;
    order.reserve(n);

    vector<int> st;
    st.push_back(0);
    parent[0] = -2;

    while (!st.empty()) {
        int u = st.back();
        st.pop_back();
        order.push_back(u);

        for (int v : adj[u]) {
            if (v == parent[u]) continue;
            parent[v] = u;
            st.push_back(v);
        }
    }

    vector<int> fact(n + 1), ifact(n + 1);
    fact[0] = 1;
    for (int i = 1; i <= n; ++i) {
        fact[i] = 1LL * fact[i - 1] * i % MOD;
    }

    ifact[n] = mod_pow(fact[n], MOD - 2);
    for (int i = n; i >= 1; --i) {
        ifact[i - 1] = 1LL * ifact[i] * i % MOD;
    }

    auto C = [&](int N, int K) -> int {
        if (K < 0 || K > N) return 0;
        return 1LL * fact[N] * ifact[K] % MOD * ifact[N - K] % MOD;
    };

    vector<vector<int>> coef(n + 1);
    for (int a = 2; a <= n; ++a) {
        coef[a].assign(a + 1, 0);
        for (int j = 1; j <= a; ++j) {
            long long ways = 1LL * fact[a] * C(a - 1, j - 1) % MOD * ifact[j] % MOD;
            if ((a - j) & 1) ways = (MOD - ways) % MOD;
            coef[a][j] = ways;
        }
    }

    vector<int> sz(n, 0);
    vector<long long> dp(n, 0);

    for (int oi = n - 1; oi >= 0; --oi) {
        int u = order[oi];

        int s = 1;
        int m = 0;
        vector<int> restricted;
        vector<int> poly(1, 1);

        for (int v : adj[u]) {
            if (parent[v] != u) continue;

            s += sz[v];

            if (sz[v] >= 2) {
                restricted.push_back(v);
                m += sz[v];

                int a = sz[v];
                vector<int> np(poly.size() + a, 0);

                for (int i = 0; i < (int)poly.size(); ++i) {
                    if (poly[i] == 0) continue;
                    for (int j = 1; j <= a; ++j) {
                        if (coef[a][j] == 0) continue;
                        np[i + j] = (np[i + j] + 1LL * poly[i] * coef[a][j]) % MOD;
                    }
                }

                poly.swap(np);
            }
        }

        sz[u] = s;
        int freeItems = s - m;

        long long full = 0;
        for (int j = 0; j < (int)poly.size(); ++j) {
            full += 1LL * poly[j] * fact[j + freeItems] % MOD;
            full %= MOD;
        }

        long long total = full;

        for (int v : restricted) {
            int a = sz[v];
            int qdeg = m - a;

            vector<int> q(qdeg + 1, 0);

            int invA1 = mod_pow(coef[a][1], MOD - 2);

            for (int t = 0; t <= qdeg; ++t) {
                int d = t + 1;
                long long val = poly[d];

                int lim = min(a, d);
                for (int l = 2; l <= lim; ++l) {
                    int idx = d - l;
                    if (idx >= 0 && idx <= qdeg) {
                        val -= 1LL * coef[a][l] * q[idx] % MOD;
                    }
                }

                val %= MOD;
                if (val < 0) val += MOD;

                q[t] = val * invA1 % MOD;
            }

            long long other = 0;
            for (int j = 0; j < (int)q.size(); ++j) {
                other += 1LL * q[j] * fact[j + freeItems] % MOD;
                other %= MOD;
            }

            total += dp[v] * other % MOD;
            total %= MOD;
        }

        dp[u] = total % MOD;
    }

    cout << dp[0] % MOD << '\n';
    return 0;
}

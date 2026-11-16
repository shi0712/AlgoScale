#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

long long modpow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return r;
}

struct Solver {
    int n, m;
    vector<vector<int>> children, sub;
    vector<int> ways, a;
    int offset;
    long long current = 0, answer = 0;

    void update(int pos, int delta) {
        int old = a[pos];
        int now = old + delta;
        current += (now >= 0 ? ways[now] : 0) - (old >= 0 ? ways[old] : 0);
        if (current >= MOD) current -= MOD;
        if (current < 0) current += MOD;
        a[pos] = now;
    }

    void dfs(int v) {
        answer += current;
        if (answer >= MOD) answer -= MOD;
        for (int u : children[v]) {
            --offset;
            for (int d = 0; d < (int)sub[u].size(); ++d) {
                int x = sub[u][d];
                update(d + offset, x);
                update(d + 2 + offset, -x);
            }
            dfs(u);
            for (int d = 0; d < (int)sub[u].size(); ++d) {
                int x = sub[u][d];
                update(d + 2 + offset, x);
                update(d + offset, -x);
            }
            ++offset;
        }
    }

    long long run(int n_, int k, vector<vector<int>>&& ch) {
        n = n_;
        m = k - 1;
        children = std::move(ch);
        if (k == 1) return n;
        if (k == 2) return 1LL * n * (n - 1) % MOD;

        sub.resize(n);
        for (int v = 0; v < n; ++v) sub[v].push_back(1);
        for (int v = n - 1; v >= 0; --v) {
            for (int u : children[v]) {
                if (sub[v].size() <= sub[u].size()) sub[v].resize(sub[u].size() + 1, 0);
                for (int d = 0; d < (int)sub[u].size(); ++d) {
                    sub[v][d + 1] += sub[u][d];
                }
            }
        }

        vector<long long> fact(n + 1), invfact(n + 1);
        fact[0] = 1;
        for (int i = 1; i <= n; ++i) fact[i] = fact[i - 1] * i % MOD;
        invfact[n] = modpow(fact[n], MOD - 2);
        for (int i = n; i > 0; --i) invfact[i - 1] = invfact[i] * i % MOD;
        ways.assign(n + 1, 0);
        for (int c = m; c <= n; ++c) {
            ways[c] = fact[c] * invfact[m] % MOD * invfact[c - m] % MOD;
        }

        a.assign(3 * n + 5, 0);
        offset = n;
        for (int d = 0; d < (int)sub[0].size(); ++d) {
            a[d + offset] = sub[0][d];
            current += ways[sub[0][d]];
            if (current >= MOD) current -= MOD;
        }
        dfs(0);
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;
    while (t--) {
        int n, k;
        cin >> n >> k;
        vector<vector<int>> children(n);
        for (int v = 1; v < n; ++v) {
            int p;
            cin >> p;
            children[p - 1].push_back(v);
        }
        Solver solver;
        cout << solver.run(n, k, std::move(children)) << '\n';
    }
    return 0;
}

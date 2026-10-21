#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

struct Solver {
    int n, k, m;
    vector<int> head, next_sibling, parent;
    vector<vector<int>> down;
    vector<int> cur;
    vector<int> fact, ifact;
    int base = 0;
    long long score = 0;
    long long answer = 0;

    int choose(int x) const {
        if (x < m) return 0;
        return (long long)fact[x] * ifact[m] % MOD * ifact[x - m] % MOD;
    }

    void set_value(int pos, int value) {
        int old = cur[pos];
        if (old == value) return;
        score += choose(value) - choose(old);
        if (score >= MOD) score -= MOD;
        if (score < 0) score += MOD;
        cur[pos] = value;
    }

    void apply_correction(int v, int direction) {
        const auto &a = down[v];
        int len = (int)a.size();
        for (int d = 0; d <= len + 1; ++d) {
            int delta = (d < len ? a[d] : 0) - (d >= 2 && d - 2 < len ? a[d - 2] : 0);
            if (delta != 0) {
                int pos = base + d;
                set_value(pos, cur[pos] + direction * delta);
            }
        }
    }

    void dfs(int u) {
        long long add = score;
        if (m == 1) {
            --add;
            if (add < 0) add += MOD;
        }
        answer += add;
        if (answer >= MOD) answer -= MOD;

        for (int v = head[u]; v != -1; v = next_sibling[v]) {
            --base;
            apply_correction(v, +1);
            dfs(v);
            apply_correction(v, -1);
            ++base;
        }
    }

    int solve_case() {
        parent.assign(n, -1);
        head.assign(n, -1);
        next_sibling.assign(n, -1);
        for (int i = 1; i < n; ++i) {
            cin >> parent[i];
            --parent[i];
            next_sibling[i] = head[parent[i]];
            head[parent[i]] = i;
        }
        if (k == 1) return n % MOD;
        m = k - 1;

        fact.resize(n + 1);
        ifact.resize(n + 1);
        fact[0] = 1;
        for (int i = 1; i <= n; ++i) fact[i] = (long long)fact[i - 1] * i % MOD;
        auto mod_pow = [](long long a, long long e) {
            long long r = 1;
            while (e) {
                if (e & 1) r = r * a % MOD;
                a = a * a % MOD;
                e >>= 1;
            }
            return r;
        };
        ifact[n] = (int)mod_pow(fact[n], MOD - 2);
        for (int i = n; i >= 1; --i) ifact[i - 1] = (long long)ifact[i] * i % MOD;

        down.assign(n, {});
        for (int u = n - 1; u >= 0; --u) {
            int len = 1;
            for (int v = head[u]; v != -1; v = next_sibling[v])
                len = max(len, (int)down[v].size() + 1);
            down[u].assign(len, 0);
            down[u][0] = 1;
            for (int v = head[u]; v != -1; v = next_sibling[v]) {
                const auto &a = down[v];
                for (int d = 0; d < (int)a.size(); ++d) down[u][d + 1] += a[d];
            }
        }

        cur.assign(2 * n + 10, 0);
        base = n + 3;
        score = 0;
        for (int d = 0; d < (int)down[0].size(); ++d) {
            cur[base + d] = down[0][d];
            score += choose(cur[base + d]);
            if (score >= MOD) score -= MOD;
        }
        answer = 0;
        dfs(0);
        return (int)answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        Solver solver;
        cin >> solver.n >> solver.k;
        cout << solver.solve_case() << '\n';
    }
    return 0;
}

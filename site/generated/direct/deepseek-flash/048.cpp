#include <bits/stdc++.h>
using namespace std;

static const int MOD = 998244353;

int mod_pow(long long a, long long e) {
    long long r = 1;
    while (e) {
        if (e & 1) r = r * a % MOD;
        a = a * a % MOD;
        e >>= 1;
    }
    return (int)r;
}

vector<int> polyAdd(const vector<int>& a, const vector<int>& b, int sign = 1) {
    if (b.empty()) return a;
    vector<int> res(max(a.size(), b.size()), 0);
    for (int i = 0; i < (int)a.size(); i++) res[i] = a[i];
    if (sign == 1) {
        for (int i = 0; i < (int)b.size(); i++) {
            res[i] += b[i];
            if (res[i] >= MOD) res[i] -= MOD;
        }
    } else {
        for (int i = 0; i < (int)b.size(); i++) {
            res[i] -= b[i];
            if (res[i] < 0) res[i] += MOD;
        }
    }
    while (!res.empty() && res.back() == 0) res.pop_back();
    return res;
}

vector<int> polyMul(const vector<int>& a, const vector<int>& b) {
    if (a.empty() || b.empty()) return {};
    vector<int> c(a.size() + b.size() - 1, 0);
    for (int i = 0; i < (int)a.size(); i++) if (a[i]) {
        long long ai = a[i];
        for (int j = 0; j < (int)b.size(); j++) if (b[j]) {
            c[i + j] = (c[i + j] + ai * b[j]) % MOD;
        }
    }
    return c;
}

vector<int> shiftX(const vector<int>& a) {
    vector<int> r(a.size() + 1, 0);
    for (int i = 0; i < (int)a.size(); i++) r[i + 1] = a[i];
    return r;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<int> parent(n, -1);
    vector<vector<int>> children(n);

    for (int i = 1; i < n; i++) {
        int p;
        cin >> p;
        --p;
        parent[i] = p;
        children[p].push_back(i);
    }

    vector<int> depth(n, 0);
    for (int i = 1; i < n; i++) depth[i] = depth[parent[i]] + 1;

    vector<int> tin(n), tout(n);
    int timer = 0;
    function<void(int)> dfs = [&](int v) {
        tin[v] = ++timer;
        for (int c : children[v]) dfs(c);
        tout[v] = timer;
    };
    dfs(0);

    auto isAncestor = [&](int u, int v) -> bool {
        return tin[u] <= tin[v] && tout[v] <= tout[u];
    };

    vector<pair<int,int>> paths;
    paths.reserve(m);
    set<pair<int,int>> st;
    for (int i = 0; i < m; i++) {
        int a, b;
        cin >> a >> b;
        --a; --b;
        if (!st.count({a, b})) {
            st.insert({a, b});
            paths.push_back({a, b});
        }
    }

    m = (int)paths.size();

    vector<char> redundant(m, 0);

    // Remove paths containing another path.
    for (int i = 0; i < m; i++) {
        int a1 = paths[i].first, b1 = paths[i].second;
        for (int j = 0; j < m; j++) {
            if (i == j) continue;
            int a2 = paths[j].first, b2 = paths[j].second;
            // path i contains path j iff a1 ancestor a2 and b2 ancestor b1
            if (isAncestor(a1, a2) && isAncestor(b2, b1)) {
                redundant[i] = 1;
                break;
            }
        }
    }

    vector<pair<int,int>> kept;
    for (int i = 0; i < m; i++) {
        if (!redundant[i]) kept.push_back(paths[i]);
    }

    if (kept.empty()) {
        cout << 0 << '\n';
        return 0;
    }

    vector<int> req(n, -1);
    vector<char> meaningful(n, 0);

    for (auto [a, b] : kept) {
        req[b] = max(req[b], depth[a]);
        int cur = b;
        while (true) {
            meaningful[cur] = 1;
            if (cur == a) break;
            cur = parent[cur];
        }
    }

    int meaningfulCnt = 0;
    for (int i = 0; i < n; i++) meaningfulCnt += meaningful[i];
    int freeCnt = n - meaningfulCnt;

    // Factorials and combinations.
    vector<int> fact(n + 1, 1), invfact(n + 1, 1);
    for (int i = 1; i <= n; i++) fact[i] = 1LL * fact[i - 1] * i % MOD;
    invfact[n] = mod_pow(fact[n], MOD - 2);
    for (int i = n; i >= 1; i--) invfact[i - 1] = 1LL * invfact[i] * i % MOD;

    auto C = [&](int N, int K) -> int {
        if (K < 0 || K > N) return 0;
        return 1LL * fact[N] * invfact[K] % MOD * invfact[N - K] % MOD;
    };

    vector<int> H(1, 1);

    // Global factor from vertices not belonging to any minimal path.
    {
        vector<int> g(freeCnt + 1);
        for (int k = 0; k <= freeCnt; k++) g[k] = C(freeCnt, k);
        H = polyMul(H, g);
    }

    vector<int> compId(n, -1);
    vector<vector<int>> components;

    for (int start = 0; start < n; start++) {
        if (!meaningful[start] || compId[start] != -1) continue;
        if (parent[start] != -1 && meaningful[parent[start]]) continue;

        int cid = (int)components.size();
        vector<int> comp;
        vector<int> stack = {start};
        compId[start] = cid;

        while (!stack.empty()) {
            int v = stack.back();
            stack.pop_back();
            comp.push_back(v);

            for (int c : children[v]) {
                if (meaningful[c] && compId[c] == -1) {
                    compId[c] = cid;
                    stack.push_back(c);
                }
            }
        }

        components.push_back(comp);
    }

    vector<vector<vector<int>>> F(n);

    for (int cid = 0; cid < (int)components.size(); cid++) {
        vector<int> comp = components[cid];

        bool isChain = true;
        for (int v : comp) {
            int cnt = 0;
            for (int c : children[v]) {
                if (meaningful[c] && compId[c] == cid) cnt++;
            }
            if (cnt > 1) {
                isChain = false;
                break;
            }
        }

        vector<int> Hcomp;

        if (isChain) {
            int root = -1;
            for (int v : comp) if (parent[v] == -1 || !meaningful[parent[v]]) root = v;

            vector<int> chainNodes;
            int cur = root;
            while (cur != -1) {
                chainNodes.push_back(cur);
                int nxt = -1;
                for (int c : children[cur]) {
                    if (meaningful[c] && compId[c] == cid) {
                        nxt = c;
                        break;
                    }
                }
                cur = nxt;
            }

            int L = (int)chainNodes.size();
            vector<vector<int>> dp(L + 1);
            vector<int> T(1, 1);
            dp[0] = {1};

            int removedUpTo = -1;

            for (int pos = 0; pos < L; pos++) {
                int v = chainNodes[pos];

                vector<int> newLast = shiftX(T);

                if (req[v] != -1) {
                    int r = pos - (depth[v] - req[v]);
                    if (r < 0) r = 0;
                    for (int idx = removedUpTo + 1; idx <= r; idx++) {
                        if (idx < (int)dp.size() && !dp[idx].empty()) {
                            T = polyAdd(T, dp[idx], -1);
                        }
                    }
                    if (r > removedUpTo) removedUpTo = r;
                }

                dp[pos + 1] = newLast;
                T = polyAdd(T, newLast, +1);
            }

            Hcomp = T;
        } else {
            // Generic DP for a branching component.
            sort(comp.begin(), comp.end(), greater<int>());

            int root = -1;

            for (int v : comp) {
                int d = depth[v];
                F[v].resize(d + 1);

                vector<int> childComp;
                for (int c : children[v]) {
                    if (meaningful[c] && compId[c] == cid) childComp.push_back(c);
                }

                vector<int> E = {1};
                for (int c : childComp) {
                    // The child array contains p = depth(v) at index depth(v)+1.
                    E = polyMul(E, F[c][d + 1]);
                }

                vector<int> selectV = shiftX(E);

                for (int idx = 0; idx <= d; idx++) {
                    int p = idx - 1;

                    if (req[v] != -1 && p < req[v]) {
                        F[v][idx] = selectV;
                    } else {
                        vector<int> Q = {1};
                        for (int c : childComp) {
                            Q = polyMul(Q, F[c][idx]);
                        }
                        vector<int> curPoly = polyAdd(Q, selectV, +1);
                        F[v][idx] = curPoly;
                    }
                }

                bool isRoot = (parent[v] == -1 || !meaningful[parent[v]]);
                if (isRoot) root = v;

                for (int c : childComp) {
                    vector<vector<int>>().swap(F[c]);
                }
            }

            Hcomp = F[root][0];

            for (int v : comp) {
                vector<vector<int>>().swap(F[v]);
            }
        }

        H = polyMul(H, Hcomp);
    }

    H.resize(n + 1, 0);

    // Convert hitting-set polynomial H(t) to Z(y):
    // Z(y) = sum_s H_s * (-1)^s * (y-1)^s * y^(n-s)
    vector<int> Z(n + 1, 0);

    for (int s = 0; s <= n; s++) {
        if (H[s] == 0) continue;
        for (int j = 0; j <= s; j++) {
            int k = n - s + j;
            long long term = 1LL * H[s] * C(s, j) % MOD;
            if (j & 1) term = (MOD - term) % MOD;
            Z[k] += term;
            if (Z[k] >= MOD) Z[k] -= MOD;
        }
    }

    vector<int> inv(n + 1, 1);
    for (int i = 2; i <= n; i++) {
        inv[i] = MOD - 1LL * (MOD / i) * inv[MOD % i] % MOD;
    }

    long long sum = 0;
    for (int k = 1; k <= n; k++) {
        if (Z[k] == 0) continue;
        sum += 1LL * Z[k] * inv[k] % MOD;
        if (sum >= MOD) sum -= MOD;
    }

    long long ans = -1LL * n * sum % MOD;
    ans %= MOD;
    if (ans < 0) ans += MOD;

    cout << ans << '\n';
    return 0;
}

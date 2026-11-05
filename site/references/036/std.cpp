// 时间复杂度 O(n log^2 n)，额外空间 O(n log n)。
#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using i128 = __int128_t;
const i64 INF = numeric_limits<i64>::max() / 4;

static i64 isqrt(i64 v) {
    i64 s = static_cast<i64>(sqrtl(static_cast<long double>(v)));
    while ((s + 1) * (s + 1) <= v) ++s;
    while (s * s > v) --s;
    return s;
}

struct Radical {
    i64 a, b; // 表示 a + sqrt(b)，其中 b >= 0
};

// 精确比较两个带根式的数，返回 -1 / 0 / 1。
static int compareRadical(Radical u, Radical v) {
    if (u.a < v.a) return -compareRadical(v, u);
    i128 d = static_cast<i128>(u.a) - v.a;
    if (d == 0) return (u.b > v.b) - (u.b < v.b);
    i128 q = static_cast<i128>(v.b) - u.b - d * d;
    if (q < 0) return 1;
    i128 lhs = 4 * d * d * u.b;
    i128 rhs = q * q;
    return (lhs > rhs) - (lhs < rhs);
}

// 维护几何限制函数最小值的广义李超树。
class GeometryHull {
    const vector<i64>& x;
    const vector<i64>& t;
    int n;
    vector<int> tree;

    Radical value(int k, int p) const {
        i64 a = x[k] - x[p];
        // 辅助延拓：保证整个定义域上任意两函数至多交叉一次。
        // 真实查询不会用到这一段。
        if (a < 0) return {a, 0};
        return {2 * (a + t[k]), 8LL * a * t[k]};
    }

    bool better(int a, int b, int p) const {
        if (a == -1) return false;
        if (b == -1) return true;
        return compareRadical(value(a, p), value(b, p)) < 0;
    }

    void add(int v, int l, int r, int f) {
        if (tree[v] == -1) {
            tree[v] = f;
            return;
        }
        int m = (l + r) / 2;
        bool left = better(f, tree[v], l);
        bool middle = better(f, tree[v], m);
        if (middle) swap(f, tree[v]);
        if (l == r) return;
        if (left != middle) add(v * 2, l, m, f);
        else add(v * 2 + 1, m + 1, r, f);
    }

    int query(int v, int l, int r, int p) const {
        int ans = tree[v];
        if (l == r) return ans;
        int m = (l + r) / 2;
        int other = p <= m ? query(v * 2, l, m, p)
                           : query(v * 2 + 1, m + 1, r, p);
        return better(other, ans, p) ? other : ans;
    }

public:
    GeometryHull(const vector<i64>& xs, const vector<i64>& ts)
        : x(xs), t(ts), n(static_cast<int>(xs.size())), tree(4 * n, -1) {}

    void add(int k) { add(1, 0, n - 1, k); }
    int query(int p) const { return query(1, 0, n - 1, p); }
};

static vector<i64> rightLimits(const vector<i64>& x, const vector<i64>& t) {
    int n = static_cast<int>(x.size());
    vector<i64> limit(n);
    GeometryHull hull(x, t);
    for (int i = n - 1; i >= 0; --i) {
        hull.add(i);
        int k = hull.query(i);
        i64 a = x[k] - x[i];
        limit[i] = 2 * (a + t[k]) + isqrt(8LL * a * t[k]);
    }
    return limit;
}

class BridgeDP {
    int n;
    i64 alpha, beta;
    const vector<i64>& x;
    const vector<i64>& t;
    vector<int> L, R;
    vector<i64> dp, best, slope, intercept;
    vector<vector<int>> pending;

    i64 value(int k, i64 at) const {
        return slope[k] * at + intercept[k];
    }

    // a、b、c 的斜率严格递增，判断中间直线 b 是否冗余。
    bool redundant(int a, int b, int c) const {
        return (static_cast<i128>(intercept[a]) - intercept[b]) *
                   (slope[c] - slope[b]) <=
               (static_cast<i128>(intercept[b]) - intercept[c]) *
                   (slope[b] - slope[a]);
    }

    void addRange(int v, int l, int r, int ql, int qr, int id) {
        if (ql <= l && r <= qr) {
            pending[v].push_back(id);
            return;
        }
        int m = (l + r) / 2;
        if (ql <= m) addRange(v * 2, l, m, ql, qr, id);
        if (qr > m) addRange(v * 2 + 1, m + 1, r, ql, qr, id);
    }

    // 进入节点时，属于该节点的所有直线都已经加入。
    void apply(int v, int l, int r) {
        auto& ids = pending[v];
        if (ids.empty()) return;

        vector<int> order(r - l + 1);
        iota(order.begin(), order.end(), l);
        sort(order.begin(), order.end(), [&](int a, int b) {
            return L[a] > L[b];
        });

        vector<int> hull;
        hull.reserve(ids.size());
        int p = static_cast<int>(ids.size()) - 1;
        for (int j : order) {
            while (p >= 0 && ids[p] >= L[j]) {
                int k = ids[p--];
                while (hull.size() >= 2 &&
                       redundant(hull[hull.size() - 2], hull.back(), k)) {
                    hull.pop_back();
                }
                hull.push_back(k);
            }
            if (hull.empty()) continue;

            int a = 0, b = static_cast<int>(hull.size()) - 1;
            while (a < b) {
                int m = (a + b) / 2;
                if (value(hull[m], x[j]) <= value(hull[m + 1], x[j])) b = m;
                else a = m + 1;
            }
            best[j] = min(best[j], value(hull[a], x[j]));
        }
        vector<int>().swap(ids);
    }

    void solve(int v, int l, int r) {
        apply(v, l, r);
        if (l == r) {
            if (l == 0) dp[l] = alpha * t[l];
            else if (best[l] != INF) {
                dp[l] = best[l] + beta * x[l] * x[l] + alpha * t[l];
            }
            if (dp[l] != INF && l < R[l]) {
                intercept[l] = dp[l] + beta * x[l] * x[l];
                addRange(1, 0, n - 1, l + 1, R[l], l);
            }
            return;
        }
        int m = (l + r) / 2;
        solve(v * 2, l, m);
        solve(v * 2 + 1, m + 1, r);
    }

public:
    BridgeDP(const vector<i64>& xs, const vector<i64>& ts, i64 a, i64 b,
             vector<int> left, vector<int> right)
        : n(static_cast<int>(xs.size())), alpha(a), beta(b), x(xs), t(ts),
          L(move(left)), R(move(right)), dp(n, INF), best(n, INF),
          slope(n), intercept(n), pending(4 * n) {
        for (int i = 0; i < n; ++i) slope[i] = -2 * beta * x[i];
    }

    i64 run() {
        solve(1, 0, n - 1);
        return dp[n - 1];
    }
};

static i64 minimumBridge(const vector<i64>& x, const vector<i64>& t,
                         i64 alpha, i64 beta) {
    int n = static_cast<int>(x.size());
    auto right = rightLimits(x, t);
    vector<i64> rx(n), rt(n);
    for (int i = 0; i < n; ++i) {
        rx[i] = -x[n - 1 - i];
        rt[i] = t[n - 1 - i];
    }
    auto left = rightLimits(rx, rt);
    vector<int> L(n), R(n);
    for (int i = 0; i < n; ++i) {
        L[i] = static_cast<int>(lower_bound(x.begin(), x.end(),
                         x[i] - left[n - 1 - i]) - x.begin());
        R[i] = static_cast<int>(upper_bound(x.begin(), x.end(),
                         x[i] + right[i]) - x.begin()) - 1;
    }
    BridgeDP solver(x, t, alpha, beta, move(L), move(R));
    return solver.run();
}

#ifndef BRIDGE_SOLVER_NO_MAIN
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    i64 h, alpha, beta;
    if (!(cin >> n >> h >> alpha >> beta)) return 0;
    vector<i64> x(n), t(n);
    for (int i = 0; i < n; ++i) {
        i64 y;
        cin >> x[i] >> y;
        t[i] = h - y;
    }
    i64 ans = minimumBridge(x, t, alpha, beta);
    if (ans == INF) cout << "impossible\n";
    else cout << ans << '\n';
    return 0;
}
#endif

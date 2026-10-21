#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
static constexpr int64 NEG = -(1LL << 60);

struct Player {
    int e, junior;
};

vector<int64> solve(int n, int d, vector<int> limit, vector<Player> input) {
    sort(limit.begin(), limit.end());
    vector<Player> a(1, {0, 0});  // Global player indices are 1-based.
    for (const auto &p : input)
        if (p.e <= limit.back()) a.push_back(p);
    sort(a.begin() + 1, a.end(), [](const Player &x, const Player &y) {
        return x.e < y.e;
    });
    const int m = (int)a.size() - 1;
    const int W = 2 * n + 1;
    if (m < 2 * n) return vector<int64>(W, -1);

    // id[c][k]: global index of the k-th player of category c.
    // Category 1 is junior; category 0 is senior.
    vector<int> id[2] = {vector<int>(1, 0), vector<int>(1, 0)};
    vector<array<int, 2>> pre(m + 1);
    array<int, 2> last = {0, 0};
    for (int i = 1; i <= m; ++i) {
        pre[i] = last;
        id[a[i].junior].push_back(i);
        last[a[i].junior] = i;
    }
    const int sz[2] = {(int)id[0].size() - 1, (int)id[1].size() - 1};

    // same[c][k]: last feasible adjacent same-category pair ending at/before k.
    vector<int> same[2];
    for (int c = 0; c < 2; ++c) {
        same[c].assign(sz[c] + 1, 0);
        for (int k = 1; k <= sz[c]; ++k) {
            same[c][k] = same[c][k - 1];
            if (k >= 2 && a[id[c][k]].e - a[id[c][k - 1]].e <= d)
                same[c][k] = k;
        }
    }

    // reach[c][k]: number of opposite-category players with ability <= e[c][k]+d.
    vector<int> reach[2];
    for (int c = 0; c < 2; ++c) {
        reach[c].assign(sz[c] + 1, 0);
        int p = 0;
        for (int k = 1; k <= sz[c]; ++k) {
            while (p < sz[c ^ 1] &&
                   (int64)a[id[c ^ 1][p + 1]].e <= (int64)a[id[c][k]].e + d)
                ++p;
            reach[c][k] = p;
        }
    }

    // good[k]: last category-0 player at/before k having any compatible category-1 player.
    vector<int> good(sz[0] + 1, 0);
    for (int k = 1; k <= sz[0]; ++k) {
        good[k] = good[k - 1];
        int p = reach[0][k];
        if (p > 0 && (int64)a[id[1][p]].e >= (int64)a[id[0][k]].e - d)
            good[k] = k;
    }

    // Sparse prefix DP for the preceding match layer.
    vector<int> oldPos(1, 0);
    vector<int64> oldDP(W, NEG);
    oldDP[0] = 0;
    int count[2] = {0, 0};
    const int K = 2 * n - 1;

    for (int j = 1; j <= n; ++j) {
        for (int c = 0; c < 2; ++c)
            while (count[c] < sz[c] && a[id[c][count[c] + 1]].e <= limit[j - 1])
                ++count[c];

        vector<int> pos(1, 0);
        pos.reserve(18 * K + 1);
        auto add = [&](int v) {
            pos.push_back(v);
            if (pre[v][0]) pos.push_back(pre[v][0]);
            if (pre[v][1]) pos.push_back(pre[v][1]);
        };

        // At most K disjoint greedy maximum pairs of each homogeneous type.
        for (int c = 0; c < 2; ++c) {
            int p = count[c];
            for (int k = 0; k < K; ++k) {
                int r = same[c][p];
                if (!r) break;
                add(id[c][r - 1]);
                add(id[c][r]);
                p = r - 2;
            }
        }

        // At most K disjoint greedy maximum mixed-category pairs.
        int x = count[0], y = count[1];
        for (int k = 0; k < K && x > 0 && y > 0; ++k) {
            int u = good[min(x, reach[1][y])];
            if (!u) break;
            int v = min(y, reach[0][u]);
            add(id[0][u]);
            add(id[1][v]);
            x = u - 1;
            y = v - 1;
        }

        sort(pos.begin(), pos.end());
        pos.erase(unique(pos.begin(), pos.end()), pos.end());
        int s = (int)pos.size();
        vector<int64> curDP((size_t)s * W, NEG);

        // before[i]: last oldPos strictly smaller than pos[i].
        vector<int> before(s, 0);
        int p = 0;
        for (int i = 1; i < s; ++i) {
            while (p + 1 < (int)oldPos.size() && oldPos[p + 1] < pos[i]) ++p;
            before[i] = p;
        }

        int prevType[2] = {0, 0};
        for (int i = 1; i < s; ++i) {
            int64 *dst = curDP.data() + (size_t)i * W;
            copy_n(dst - W, W, dst);  // Do not use pos[i].
            int v = pos[i];
            for (int c = 0; c < 2; ++c) {
                int k = prevType[c];
                if (!k) continue;
                int u = pos[k];
                if (a[v].e - a[u].e > d) continue;
                int delta = a[u].junior + a[v].junior;
                const int64 *src = oldDP.data() + (size_t)before[k] * W;
                int64 gain = (int64)a[u].e + a[v].e;
                for (int t = 0; t <= 2 * (j - 1); ++t)
                    if (src[t] != NEG)
                        dst[t + delta] = max(dst[t + delta], src[t] + gain);
            }
            prevType[a[v].junior] = i;
        }
        oldPos.swap(pos);
        oldDP.swap(curDP);
    }

    vector<int64> ans(W);
    size_t base = (oldPos.size() - 1) * W;
    for (int t = 0; t < W; ++t)
        ans[t] = oldDP[base + t] == NEG ? -1 : oldDP[base + t];
    return ans;
}

#ifndef TENNIS_TEST
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    while (T--) {
        int n, m, d;
        cin >> n >> m >> d;
        vector<int> limit(n);
        for (int &x : limit) cin >> x;
        vector<Player> a(m);
        for (auto &p : a) {
            int type;
            cin >> p.e >> type;
            p.junior = (type == 1);
        }
        auto ans = solve(n, d, std::move(limit), std::move(a));
        for (int t = 0; t <= 2 * n; ++t)
            cout << ans[t] << (t == 2 * n ? '\n' : ' ');
    }
    return 0;
}
#endif

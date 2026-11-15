#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int main() {
    int n, m;
    ll d;
    scanf("%d %d %lld", &n, &m, &d);
    vector<ll> l(n);
    for (auto &x : l) scanf("%lld", &x);
    sort(l.begin(), l.end());
    vector<pair<ll,int>> pl(m);
    for (int i = 0; i < m; i++) {
        int t;
        scanf("%lld %d", &pl[i].first, &t);
        pl[i].second = t - 1; // 0 junior, 1 senior
    }
    sort(pl.begin(), pl.end());
    vector<ll> val(m + 2);
    vector<int> typ(m + 2);
    for (int i = 1; i <= m; i++) {
        val[i] = pl[i - 1].first;
        typ[i] = pl[i - 1].second;
    }
    vector<vector<int>> prevT(2, vector<int>(m + 2, 0));
    {
        int last[2] = {0, 0};
        for (int i = 1; i <= m; i++) {
            prevT[0][i] = last[0];
            prevT[1][i] = last[1];
            last[typ[i]] = i;
        }
    }
    // best[x][y][u]
    vector<vector<vector<int>>> best(2, vector<vector<int>>(2, vector<int>(m + 2, 0)));
    for (int x = 0; x < 2; x++)
        for (int y = 0; y < 2; y++)
            for (int i = 1; i <= m; i++) {
                bool ok = typ[i] == y && prevT[x][i] > 0 && val[i] - val[prevT[x][i]] <= d;
                best[x][y][i] = ok ? i : best[x][y][i - 1];
            }
    vector<int> c(n);
    for (int k = 0; k < n; k++) {
        c[k] = int(upper_bound(val.begin() + 1, val.begin() + m + 1, l[k]) - (val.begin() + 1));
    }
    typedef pair<int, ll> St;
    vector<vector<St>> cur(2 * n + 1), nxt(2 * n + 1);
    cur[0].push_back({m + 1, 0});
    for (int k = n - 1; k >= 0; k--) {
        for (auto &v : nxt) v.clear();
        int maxT = 2 * (n - 1 - k);
        for (int t = 0; t <= maxT; t++) {
            for (auto &s : cur[t]) {
                int u = min(c[k], s.first - 1);
                if (u <= 0) continue;
                for (int x = 0; x < 2; x++)
                    for (int y = 0; y < 2; y++) {
                        int b = best[x][y][u];
                        if (!b) continue;
                        int a = prevT[x][b];
                        int nt = t + (x == 0) + (y == 0);
                        nxt[nt].push_back({a, s.second + val[a] + val[b]});
                    }
            }
        }
        for (int t = 0; t <= 2 * n; t++) {
            auto &v = nxt[t];
            if (v.empty()) continue;
            sort(v.begin(), v.end(), [](const St &p, const St &q) {
                if (p.first != q.first) return p.first > q.first;
                return p.second > q.second;
            });
            vector<St> res;
            ll mx = -1;
            for (auto &s : v) {
                if (s.second > mx) {
                    res.push_back(s);
                    mx = s.second;
                }
            }
            v.swap(res);
        }
        swap(cur, nxt);
    }
    for (int t = 0; t <= 2 * n; t++) {
        ll ans = -1;
        for (auto &s : cur[t]) ans = max(ans, s.second);
        printf("%lld%c", ans, t == 2 * n ? '\n' : ' ');
    }
    return 0;
}

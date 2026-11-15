#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> par_, ch_[2], dist_, q_;
mt19937 rng(712367);

void bfs(int u) {
    fill(dist_.begin(), dist_.end(), -1);
    int h = 0, t = 0;
    q_[t++] = u;
    dist_[u] = 0;
    while (h < t) {
        int v = q_[h++];
        int d = dist_[v] + 1;
        int p = par_[v];
        if (p > 0 && dist_[p] < 0) { dist_[p] = d; q_[t++] = p; }
        for (int j = 0; j < 2; j++) {
            int c = ch_[j][v];
            if (c > 0 && dist_[c] < 0) { dist_[c] = d; q_[t++] = c; }
        }
    }
}

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    while (T--) {
        scanf("%d", &n);
        par_.assign(n + 1, 0);
        ch_[0].assign(n + 1, 0);
        ch_[1].assign(n + 1, 0);
        dist_.assign(n + 1, -1);
        q_.assign(n + 1, 0);
        for (int i = 2; i <= n; i++) {
            scanf("%d", &par_[i]);
            int p = par_[i];
            if (ch_[0][p] == 0) ch_[0][p] = i;
            else ch_[1][p] = i;
        }
        vector<int> S(n);
        for (int i = 0; i < n; i++) S[i] = i + 1;
        vector<int> sz(n + 1), mxc(n + 1);
        vector<int> cnt(n + 2);
        while (S.size() > 1) {
            int m = S.size();
            // weighted centroid
            fill(sz.begin(), sz.end(), 0);
            fill(mxc.begin(), mxc.end(), 0);
            for (int v : S) sz[v] = 1;
            for (int i = n; i >= 2; i--) {
                sz[par_[i]] += sz[i];
                mxc[par_[i]] = max(mxc[par_[i]], sz[i]);
            }
            int c = 1, bestc = INT_MAX;
            for (int v = 1; v <= n; v++) {
                int mx = max(mxc[v], m - sz[v]);
                if (mx < bestc) { bestc = mx; c = v; }
            }
            vector<int> cands;
            cands.push_back(c);
            int K = min(m, 40);
            for (int i = 0; i < K; i++) cands.push_back(S[rng() % m]);
            int bu = -1, bk = 0, bmax = INT_MAX;
            for (int u : cands) {
                bfs(u);
                int maxd = 0;
                for (int v : S) maxd = max(maxd, dist_[v]);
                for (int d = 0; d <= maxd; d++) cnt[d] = 0;
                for (int v : S) cnt[dist_[v]]++;
                int cum = 0;
                for (int k = 0; k <= maxd; k++) {
                    cum += cnt[k];
                    if (cum <= 0 || cum >= m) continue;
                    int mx = max(cum, m - cum);
                    if (mx < bmax) { bmax = mx; bu = u; bk = k; }
                }
                if (bmax != INT_MAX && (bmax * 100 <= 56 * m || bmax <= (m + 1) / 2)) break;
            }
            printf("? %d %d\n", bu, bk);
            fflush(stdout);
            int o;
            if (scanf("%d", &o) != 1) return 0;
            bfs(bu);
            vector<int> NS;
            for (int v : S) {
                bool in = dist_[v] <= bk;
                if ((o == 1) == in) NS.push_back(v);
            }
            S.swap(NS);
        }
        printf("! %d\n", S[0]);
        fflush(stdout);
    }
    return 0;
}

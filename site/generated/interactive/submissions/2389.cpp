#include <bits/stdc++.h>
using namespace std;

int N, M;
vector<int> par, pr, sz_;
vector<array<int,2>> cn;

int findr(int x, int &p) {
    // iterative find with parity
    vector<int> path;
    int cur = x;
    while (par[cur] != cur) { path.push_back(cur); cur = par[cur]; }
    int root = cur;
    // compress
    int acc = 0;
    for (int i = (int)path.size() - 1; i >= 0; i--) {
        int v = path[i];
        acc ^= pr[v];
        pr[v] = acc;
        par[v] = root;
    }
    p = (x == root) ? 0 : pr[x];
    return root;
}

int main() {
    scanf("%d %d", &N, &M);
    par.resize(N + 1); pr.assign(N + 1, 0); sz_.assign(N + 1, 1);
    cn.assign(N + 1, {1, 0});
    for (int i = 0; i <= N; i++) par[i] = i;
    int ptr = 1;
    while (true) {
        // gather roots
        vector<int> roots;
        for (int i = 1; i <= N; i++) if (par[i] == i) roots.push_back(i);
        int K = roots.size();
        // DP suffix counts
        vector<array<int,12>> dp(K + 1);
        for (auto &a : dp) a.fill(0);
        dp[K][0] = 1;
        for (int i = K - 1; i >= 0; i--) {
            int a = cn[roots[i]][0], b = cn[roots[i]][1];
            for (int s = 0; s <= M; s++) {
                int v = 0;
                if (s >= a) v += dp[i + 1][s - a];
                if (s >= b) v += dp[i + 1][s - b];
                dp[i][s] = min(v, 2);
            }
        }
        if (dp[0][M] == 1) {
            vector<int> choice(N + 1, 0); // class that is counterfeit
            int s = M;
            for (int i = 0; i < K; i++) {
                int a = cn[roots[i]][0], b = cn[roots[i]][1];
                if (s >= a && dp[i + 1][s - a] > 0) { choice[roots[i]] = 0; s -= a; }
                else { choice[roots[i]] = 1; s -= b; }
            }
            printf("!");
            for (int x = 1; x <= N; x++) {
                int p;
                int r = findr(x, p);
                if (p == choice[r]) printf(" %d", x);
            }
            printf("\n");
            fflush(stdout);
            return 0;
        }
        // choose query
        int bestU = -1;
        for (int r : roots) {
            if (sz_[r] > 1 && max(cn[r][0], cn[r][1]) <= M) {
                if (bestU == -1 || sz_[r] > sz_[bestU]) bestU = r;
            }
        }
        while (ptr <= N && sz_[findr(ptr, *new int) ] != 1) ptr++;
        int qx = -1, qy = -1;
        bool haveSingle = ptr <= N;
        if (bestU != -1) {
            if (haveSingle) { qx = bestU; qy = ptr; }
            else {
                int other = -1;
                for (int r : roots) if (r != bestU) {
                    if (other == -1 || max(cn[r][0], cn[r][1]) > M) other = r;
                }
                qx = bestU; qy = other;
            }
        } else {
            // find singleton(s)
            vector<int> singles;
            for (int r : roots) if (sz_[r] == 1) { singles.push_back(r); if (singles.size() >= 2) break; }
            if (singles.size() >= 2) { qx = singles[0]; qy = singles[1]; }
            else if (singles.size() == 1) {
                int other = -1;
                for (int r : roots) if (r != singles[0]) { other = r; break; }
                qx = singles[0]; qy = other;
            } else {
                qx = roots[0]; qy = roots[1];
            }
        }
        if (qx < 0 || qy < 0 || qx == qy) return 0;
        printf("? %d %d\n", qx, qy);
        fflush(stdout);
        int r;
        if (scanf("%d", &r) != 1) return 0;
        if (r < 0) return 0;
        int px, py;
        int rx = findr(qx, px), ry = findr(qy, py);
        if (rx == ry) continue;
        if (sz_[rx] > sz_[ry]) { swap(rx, ry); swap(px, py); }
        // merge rx into ry
        int rel = px ^ py ^ r;
        par[rx] = ry;
        pr[rx] = rel;
        for (int c = 0; c < 2; c++) cn[ry][c ^ rel] += cn[rx][c];
        sz_[ry] += sz_[rx];
    }
}

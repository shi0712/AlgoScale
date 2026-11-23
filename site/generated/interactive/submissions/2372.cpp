#include <bits/stdc++.h>
using namespace std;

int n;
vector<int> par_, adjStart, adjList;
vector<int> distv, qu;

void bfs(int u, vector<int>& d) {
    fill(d.begin(), d.end(), -1);
    int h = 0, t = 0;
    qu[t++] = u;
    d[u] = 0;
    while (h < t) {
        int x = qu[h++];
        for (int e = adjStart[x]; e < adjStart[x + 1]; e++) {
            int y = adjList[e];
            if (d[y] < 0) {
                d[y] = d[x] + 1;
                qu[t++] = y;
            }
        }
    }
}

int main() {
    scanf("%d", &n);
    par_.assign(n + 1, 0);
    vector<int> deg(n + 2, 0);
    for (int i = 2; i <= n; i++) {
        scanf("%d", &par_[i]);
        deg[i]++;
        deg[par_[i]]++;
    }
    adjStart.assign(n + 2, 0);
    for (int i = 1; i <= n; i++) adjStart[i + 1] = adjStart[i] + deg[i];
    adjList.assign(adjStart[n + 1], 0);
    vector<int> pos(adjStart.begin(), adjStart.end());
    for (int i = 2; i <= n; i++) {
        adjList[pos[i]++] = par_[i];
        adjList[pos[par_[i]]++] = i;
    }
    distv.assign(n + 1, -1);
    qu.assign(n + 1, 0);
    mt19937 rng(712367);
    vector<char> inS(n + 1, 1);
    inS[0] = 0;
    vector<int> S;
    for (int i = 1; i <= n; i++) S.push_back(i);
    vector<int> sub(n + 1), cnt(n + 2);
    while (S.size() > 1) {
        int m = S.size();
        // centroid of S
        for (int i = 1; i <= n; i++) sub[i] = inS[i];
        vector<int> mx(n + 1, 0);
        for (int i = n; i >= 2; i--) {
            sub[par_[i]] += sub[i];
            mx[par_[i]] = max(mx[par_[i]], sub[i]);
        }
        int c = 1, bestc = INT_MAX;
        for (int i = 1; i <= n; i++) {
            int v = max(mx[i], m - sub[i]);
            if (v < bestc) {
                bestc = v;
                c = i;
            }
        }
        vector<int> cands;
        cands.push_back(c);
        // diameter endpoints
        bfs(S[0], distv);
        int a = S[0];
        for (int s : S) if (distv[s] > distv[a]) a = s;
        bfs(a, distv);
        int b = a;
        for (int s : S) if (distv[s] > distv[b]) b = s;
        cands.push_back(a);
        cands.push_back(b);
        int R = 60;
        for (int i = 0; i < R; i++) {
            if (i % 3 == 2) cands.push_back(rng() % n + 1);
            else cands.push_back(S[rng() % m]);
        }
        int bestScore = INT_MAX, bu = -1, bk = -1;
        for (int u : cands) {
            bfs(u, distv);
            int maxd = 0;
            for (int s : S) maxd = max(maxd, distv[s]);
            for (int d = 0; d <= maxd; d++) cnt[d] = 0;
            for (int s : S) cnt[distv[s]]++;
            int pre = 0;
            for (int d = 0; d < maxd; d++) {
                pre += cnt[d];
                if (pre > 0 && pre < m) {
                    int sc = max(pre, m - pre);
                    if (sc < bestScore) {
                        bestScore = sc;
                        bu = u;
                        bk = d;
                    }
                }
            }
        }
        if (bu < 0) {
            bu = S[0];
            bk = 0;
        }
        printf("? %d %d\n", bu, bk);
        fflush(stdout);
        int o;
        if (scanf("%d", &o) != 1) return 0;
        if (o < 0) return 0;
        bfs(bu, distv);
        vector<int> NS;
        for (int s : S) {
            bool in = distv[s] <= bk;
            if (in == (o == 1)) NS.push_back(s);
            else inS[s] = 0;
        }
        S = NS;
    }
    printf("! %d\n", S[0]);
    fflush(stdout);
    return 0;
}

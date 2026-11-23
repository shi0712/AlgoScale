#include <bits/stdc++.h>
using namespace std;

int n, m;
int Xcnt;
vector<int> xb;

int askq(int v, const string &b) {
    printf("? %d %s\n", v, b.c_str());
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

void solveX(int l, int r, int c) {
    if (c == 0) {
        for (int i = l; i < r; i++) xb[i] = 0;
        return;
    }
    if (c == r - l) {
        for (int i = l; i < r; i++) xb[i] = 1;
        return;
    }
    int mid = (l + r) / 2;
    string b(m, '0');
    for (int i = l; i < mid; i++) b[i] = '1';
    int ans = askq(1, b);
    int a = (ans - m + (mid - l) + Xcnt) / 2;
    solveX(l, mid, a);
    solveX(mid, r, c - a);
}

bitset<512> P[505];

int main() {
    if (scanf("%d", &n) != 1) return 0;
    m = n - 1;
    xb.assign(m, 0);
    {
        string b(m, '1');
        Xcnt = askq(1, b);
    }
    solveX(0, m, Xcnt);
    string xs(m, '0');
    for (int i = 0; i < m; i++) xs[i] = xb[i] ? '1' : '0';

    vector<int> d(n + 1, 0);
    int maxd = 0;
    for (int v = 2; v <= n; v++) {
        if (m == 1) d[v] = 1;
        else {
            int ans = askq(v, xs);
            d[v] = m - ans;
        }
        maxd = max(maxd, d[v]);
    }
    vector<vector<int>> lvl(maxd + 2);
    lvl[0].push_back(1);
    for (int v = 2; v <= n; v++) lvl[d[v]].push_back(v);

    vector<int> par(n + 1, 0), ev(n + 1, -1);
    vector<vector<int>> ch(n + 1);
    vector<int> unassigned(m);
    iota(unassigned.begin(), unassigned.end(), 0);

    for (int dep = 1; dep <= maxd; dep++) {
        vector<int> S;
        vector<vector<int>> cm;
        int sz = 0;
        if (dep >= 2) {
            S = lvl[dep - 1];
            vector<int> tin(n + 1, 0);
            int timer = 0;
            vector<int> st;
            st.push_back(1);
            while (!st.empty()) {
                int u = st.back();
                st.pop_back();
                tin[u] = timer++;
                for (int i = (int)ch[u].size() - 1; i >= 0; i--) st.push_back(ch[u][i]);
            }
            sort(S.begin(), S.end(), [&](int a, int b) { return tin[a] < tin[b]; });
            sz = S.size();
            if (sz > 1) {
                cm.assign(sz, vector<int>(sz, 0));
                for (int i = 0; i < sz; i++)
                    for (int j = 0; j < sz; j++)
                        cm[i][j] = (int)(P[S[i]] & P[S[j]]).count();
            }
        }
        for (int v : lvl[dep]) {
            int p;
            if (dep == 1) p = 1;
            else if (sz == 1) p = S[0];
            else {
                vector<int> cur(sz);
                iota(cur.begin(), cur.end(), 0);
                while (cur.size() > 1) {
                    int s = cur.size();
                    vector<int> f(s, 0), cnt(dep + 2, 0);
                    long long bestCost = LLONG_MAX;
                    int bestK = 1;
                    for (int k = 1; k <= s - 1; k++) {
                        int g = cur[k - 1];
                        for (int j = 0; j < s; j++) f[j] = max(f[j], cm[g][cur[j]]);
                        long long cost = 0;
                        for (int j = 0; j < s; j++) cnt[f[j]]++;
                        for (int j = 0; j < s; j++) {
                            if (cnt[f[j]]) {
                                cost += (long long)cnt[f[j]] * cnt[f[j]];
                                cnt[f[j]] = 0;
                            }
                        }
                        if (cost < bestCost) {
                            bestCost = cost;
                            bestK = k;
                        }
                    }
                    fill(f.begin(), f.end(), 0);
                    bitset<512> M;
                    for (int k = 1; k <= bestK; k++) {
                        int g = cur[k - 1];
                        M |= P[S[g]];
                        for (int j = 0; j < s; j++) f[j] = max(f[j], cm[g][cur[j]]);
                    }
                    string b(m, '0');
                    for (int i = 0; i < m; i++) b[i] = ((xb[i] ^ (M[i] ? 1 : 0)) ? '1' : '0');
                    int ans = askq(v, b);
                    int c = (ans - m + (int)M.count() + dep) / 2;
                    vector<int> nc;
                    for (int j = 0; j < s; j++)
                        if (f[j] == c) nc.push_back(cur[j]);
                    if (nc.empty()) {
                        nc.push_back(cur[0]);
                    }
                    cur = nc;
                }
                p = S[cur[0]];
            }
            // edge search
            vector<int> C = unassigned;
            while (C.size() > 1) {
                int h = C.size() / 2;
                string b = xs;
                for (int i = 0; i < h; i++) b[C[i]] = (b[C[i]] == '1') ? '0' : '1';
                int ans = askq(v, b);
                int c = (ans - m + h + dep) / 2;
                if (c >= 1) {
                    C.resize(h);
                } else {
                    C.erase(C.begin(), C.begin() + h);
                }
            }
            int e = C[0];
            unassigned.erase(find(unassigned.begin(), unassigned.end(), e));
            ev[v] = e;
            par[v] = p;
            ch[p].push_back(v);
            P[v] = P[p];
            P[v].set(e);
        }
    }

    vector<int> sA(m, 0), fA(m, 0);
    for (int v = 2; v <= n; v++) {
        int i = ev[v];
        int child = v, parent = par[v];
        if (xb[i] == 1) {
            sA[i] = child;
            fA[i] = parent;
        } else {
            sA[i] = parent;
            fA[i] = child;
        }
    }
    printf("!");
    for (int i = 0; i < m; i++) printf(" %d %d", sA[i], fA[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}

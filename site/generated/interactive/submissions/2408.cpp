#include <bits/stdc++.h>
using namespace std;

int M;
static int ask(int u, int x) {
    printf("? 1 %d %d\n", u, x);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}

int main() {
    if (scanf("%d", &M) != 1) return 0;
    vector<int> par(M, -1);
    int nextId = M;
    int rootId = -1;
    vector<int> knownLeaves;
    int pos = 0;
    int Bcur = 0;
    while (pos < M) {
        int Rrem = M - pos;
        int tot = nextId;
        vector<vector<int>> ch(tot);
        vector<char> isKnown(tot, 0);
        for (int v = M; v < tot; v++) isKnown[v] = 1;
        for (int u : knownLeaves) isKnown[u] = 1;
        for (int v = 0; v < tot; v++) {
            if (isKnown[v] && par[v] >= 0) ch[par[v]].push_back(v);
        }
        vector<int> off(tot, 0);
        int mk = -1;
        if (rootId != -1) {
            mk = 0;
            for (int p = 0; p < tot; p++) {
                if (!isKnown[p]) continue;
                int cnt = 0;
                for (int x : ch[p]) if (x >= M) { off[x] = cnt++; }
                for (int x : ch[p]) if (x < M) { off[x] = cnt; }
                for (int x : ch[p]) mk = max(mk, off[x]);
            }
        }
        // choose n
        int bestN = 1;
        long long bestCost = LLONG_MAX;
        for (int n = 1; n <= Rrem; n++) {
            int Bp = max(Bcur, n + mk);
            if (Bp > 1000) continue;
            int rem = Rrem - n;
            long long cost = Bp;
            if (rem > 0) {
                int den = Bp - (mk + 1);
                if (den <= 0) continue;
                cost += (rem + den - 1) / den;
            }
            if (cost <= bestCost) {
                bestCost = cost;
                bestN = n;
            }
        }
        int n = bestN;
        Bcur = max(Bcur, n + mk);
        Bcur = max(Bcur, n - 1);
        // predicted preorder
        vector<int> P;
        if (rootId != -1) {
            function<void(int)> dfs = [&](int v) {
                P.push_back(v);
                for (int x : ch[v]) if (x >= M) dfs(x);
                for (int x : ch[v]) if (x < M) P.push_back(x);
            };
            dfs(rootId);
        }
        // rebuild known part
        if (rootId != -1) {
            vector<char> occ(tot, 0);
            for (int u : knownLeaves) {
                vector<int> lst;
                int x = u;
                while (x != -1 && !occ[x]) {
                    lst.push_back(x);
                    x = par[x];
                }
                for (int i = (int)lst.size() - 1; i >= 0; i--) {
                    int y = lst[i];
                    ask(u, n + off[y]);
                    occ[y] = 1;
                }
            }
        }
        // new runs
        vector<int> ul(n), c(n, 0);
        for (int j = 0; j < n; j++) {
            ul[j] = pos + j;
            int v = n - 1 - j;
            while (true) {
                int r = ask(ul[j], v);
                if (r == 0) break;
                c[j]++;
            }
        }
        printf("? 2\n");
        fflush(stdout);
        int s;
        if (scanf("%d", &s) != 1) return 0;
        if (s < 0) return 0;
        vector<int> seq(s);
        for (int i = 0; i < s; i++) {
            if (scanf("%d", &seq[i]) != 1) return 0;
        }
        // allocate chains
        vector<vector<int>> chain(n);
        for (int j = 0; j < n; j++) {
            for (int t = 0; t < c[j] - 1; t++) chain[j].push_back(nextId++);
            chain[j].push_back(ul[j]);
        }
        par.resize(max(nextId, M), -1);
        vector<int> run(s), kidx(s, -1), ord(s, 0);
        vector<int> topIdx(n, -1), cntRun(n, 0);
        int kc = 0;
        for (int i = 0; i < s; i++) {
            if (seq[i] < n) {
                int j = n - 1 - seq[i];
                run[i] = j;
                ord[i] = cntRun[j]++;
                if (topIdx[j] < 0) topIdx[j] = i;
            } else {
                run[i] = -1;
                kidx[i] = kc++;
            }
        }
        for (int j = n - 1; j >= 0; j--) {
            int i = topIdx[j];
            if (i < 0) continue;
            int k = i - 1;
            while (k >= 0 && run[k] > j) k--;
            int top = chain[j][0];
            if (k < 0) {
                par[top] = -1;
                rootId = top;
            } else {
                int pv;
                if (run[k] == -1) pv = P[kidx[k]];
                else pv = chain[run[k]][ord[k]];
                par[top] = pv;
            }
            for (int t = 1; t < (int)chain[j].size(); t++) par[chain[j][t]] = chain[j][t - 1];
        }
        for (int j = 0; j < n; j++) knownLeaves.push_back(ul[j]);
        pos += n;
    }
    int N = nextId;
    vector<int> label(N, -1);
    for (int v = 0; v < M; v++) label[v] = v;
    label[rootId] = N - 1;
    int cur = M;
    for (int v = M; v < N; v++) {
        if (v == rootId) continue;
        label[v] = cur++;
    }
    vector<int> R(N, 0);
    for (int v = 0; v < N; v++) {
        if (v == rootId) continue;
        R[label[v]] = label[par[v]];
    }
    printf("! %d", N);
    for (int i = 0; i < N - 1; i++) printf(" %d", R[i]);
    printf("\n");
    fflush(stdout);
    return 0;
}

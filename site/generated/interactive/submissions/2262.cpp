#include <bits/stdc++.h>
using namespace std;

static int N, K;
static vector<int> lch, rch, par, wn, leafOf;
static int nodeCnt = 0;
static bool invertCmp = false;

static inline int ask(int a, int b) {
    printf("? %d %d\n", a, b);
    fflush(stdout);
    int w;
    if (scanf("%d", &w) != 1) exit(0);
    if (w == -1) exit(0);
    if (w != a && w != b) exit(0);
    if (invertCmp) return (w == a) ? b : a;
    return w;
}

static int build(int l, int r) {
    int id = nodeCnt++;
    if (r - l == 1) {
        wn[id] = l + 1;
        leafOf[l + 1] = id;
        return id;
    }
    int m = (l + r) >> 1;
    int a = build(l, m);
    int b = build(m, r);
    lch[id] = a; rch[id] = b;
    par[a] = id; par[b] = id;
    wn[id] = ask(wn[a], wn[b]);
    return id;
}

int main() {
    if (scanf("%d %d", &N, &K) != 2) return 0;
    if (N <= 0) return 0;
    if (K >= N) {
        printf("!");
        for (int i = 1; i <= N; i++) printf(" %d", i);
        printf("\n");
        fflush(stdout);
        return 0;
    }
    int t = K;
    if (N - K < K) { invertCmp = true; t = N - K; }
    if (t < 1) t = 1;

    int cap = 2 * N + 5;
    lch.assign(cap, -1);
    rch.assign(cap, -1);
    par.assign(cap, -1);
    wn.assign(cap, 0);
    leafOf.assign(N + 1, -1);

    int root = build(0, N);

    vector<int> got;
    got.reserve(t);
    for (int i = 0; i < t; i++) {
        int m = wn[root];
        got.push_back(m);
        if (i + 1 == t) break;
        int v = leafOf[m];
        wn[v] = 0;
        for (v = par[v]; v != -1; v = par[v]) {
            int x = wn[lch[v]], y = wn[rch[v]];
            if (x == 0) wn[v] = y;
            else if (y == 0) wn[v] = x;
            else wn[v] = ask(x, y);
        }
    }

    vector<int> ans;
    if (!invertCmp) {
        ans = got;
    } else {
        vector<char> bad(N + 1, 0);
        for (int x : got) bad[x] = 1;
        for (int i = 1; i <= N; i++) if (!bad[i]) ans.push_back(i);
    }

    printf("!");
    for (int x : ans) printf(" %d", x);
    printf("\n");
    fflush(stdout);
    return 0;
}

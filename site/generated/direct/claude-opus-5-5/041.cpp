#include <bits/stdc++.h>
using namespace std;

int n;
struct Op {
    int p[4];
    int e[6];
    int qv[4];
};
typedef vector<vector<char>> Adj;

void applyOp(Adj &adj, const Op &o) {
    for (int i = 0; i < 3; i++) {
        adj[o.p[i]][o.p[i + 1]] = 0;
        adj[o.p[i + 1]][o.p[i]] = 0;
    }
    for (int i = 0; i < 3; i++) {
        int a = o.e[2 * i], b = o.e[2 * i + 1];
        adj[a][b] = 1;
        adj[b][a] = 1;
    }
}

Op mkPath(int p0, int p1, int p2, int p3, int q0, int q1, int q2, int q3) {
    Op o;
    o.p[0] = p0; o.p[1] = p1; o.p[2] = p2; o.p[3] = p3;
    o.qv[0] = q0; o.qv[1] = q1; o.qv[2] = q2; o.qv[3] = q3;
    o.e[0] = q0; o.e[1] = q1; o.e[2] = q1; o.e[3] = q2; o.e[4] = q2; o.e[5] = q3;
    return o;
}

Op mkEdges(int p0, int p1, int p2, int p3, int a1, int b1, int a2, int b2, int a3, int b3) {
    Op o;
    o.p[0] = p0; o.p[1] = p1; o.p[2] = p2; o.p[3] = p3;
    o.e[0] = a1; o.e[1] = b1; o.e[2] = a2; o.e[3] = b2; o.e[4] = a3; o.e[5] = b3;
    for (int i = 0; i < 4; i++) o.qv[i] = 0;
    return o;
}

void bfs(const Adj &adj, int root, vector<int> &par, vector<int> &dep) {
    par.assign(n + 1, 0);
    dep.assign(n + 1, -1);
    queue<int> q;
    q.push(root);
    dep[root] = 0;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v = 1; v <= n; v++) {
            if (adj[u][v] && dep[v] < 0) {
                dep[v] = dep[u] + 1;
                par[v] = u;
                q.push(v);
            }
        }
    }
}

int degOf(const Adj &adj, int v) {
    int d = 0;
    for (int u = 1; u <= n; u++) d += adj[v][u];
    return d;
}

int starCenter(const Adj &adj) {
    for (int v = 1; v <= n; v++)
        if (degOf(adj, v) == n - 1) return v;
    return 0;
}

void reduceDepth(Adj &adj, vector<Op> &ops) {
    vector<int> par, dep;
    while (true) {
        bfs(adj, 1, par, dep);
        int u = 0;
        for (int v = 1; v <= n; v++)
            if (dep[v] == 3) { u = v; break; }
        if (!u) break;
        int v = par[u], a = par[v];
        Op o = mkPath(1, a, v, u, a, 1, v, u);
        applyOp(adj, o);
        ops.push_back(o);
    }
}

int main() {
    scanf("%d", &n);
    Adj sa(n + 1, vector<char>(n + 1, 0)), ta(n + 1, vector<char>(n + 1, 0));
    for (int i = 0; i < n - 1; i++) {
        int a, b;
        scanf("%d %d", &a, &b);
        sa[a][b] = sa[b][a] = 1;
    }
    for (int i = 0; i < n - 1; i++) {
        int a, b;
        scanf("%d %d", &a, &b);
        ta[a][b] = ta[b][a] = 1;
    }
    if (sa == ta) {
        printf("YES\n0\n");
        return 0;
    }
    if (starCenter(sa)) {
        printf("NO\n");
        return 0;
    }
    vector<Op> out;
    int tc = starCenter(ta);
    if (tc) {
        int c = tc;
        vector<int> par, dep;
        while (degOf(sa, c) < n - 1) {
            bfs(sa, c, par, dep);
            int u = 0;
            for (int v = 1; v <= n; v++)
                if (dep[v] == 3) { u = v; break; }
            Op o;
            if (u) {
                int v = par[u], a = par[v];
                o = mkEdges(c, a, v, u, c, a, c, v, c, u);
            } else {
                int v = 0;
                for (int x = 1; x <= n; x++)
                    if (dep[x] == 2) { v = x; break; }
                int a = par[v];
                int b = 0;
                for (int x = 1; x <= n; x++)
                    if (sa[c][x] && x != a) { b = x; break; }
                o = mkEdges(v, a, c, b, c, v, c, a, c, b);
            }
            applyOp(sa, o);
            out.push_back(o);
        }
    } else {
        // T side
        vector<Op> tops;
        reduceDepth(ta, tops);
        vector<int> par, dep;
        bfs(ta, 1, par, dep);
        vector<int> AT;
        for (int v = 1; v <= n; v++)
            if (dep[v] == 1) AT.push_back(v);
        int bs = AT[0];
        vector<char> inA(n + 1, 0), inG(n + 1, 0);
        for (int v : AT) inA[v] = 1;
        for (int v = 2; v <= n; v++)
            if (!inA[v]) inG[v] = 1;
        for (int g = 1; g <= n; g++) {
            if (dep[g] == 2 && par[g] != bs) {
                int a = par[g];
                Op o = mkPath(g, a, 1, bs, a, 1, bs, g);
                applyOp(ta, o);
                tops.push_back(o);
            }
        }
        // S side
        vector<Op> sops;
        reduceDepth(sa, sops);
        bfs(sa, 1, par, dep);
        if (dep[bs] == 2) {
            int a = par[bs];
            int cnt = 0;
            for (int v = 1; v <= n; v++)
                if (dep[v] == 2) cnt++;
            int b = 0;
            for (int x = 1; x <= n; x++)
                if (sa[1][x] && x != a) { b = x; break; }
            Op o;
            if (cnt == 1) o = mkEdges(bs, a, 1, b, a, bs, bs, 1, 1, b);
            else o = mkEdges(bs, a, 1, b, 1, bs, 1, a, 1, b);
            applyOp(sa, o);
            sops.push_back(o);
        }
        bfs(sa, 1, par, dep);
        for (int g = 1; g <= n; g++) {
            if (dep[g] == 2 && par[g] != bs) {
                int a = par[g];
                Op o = mkPath(g, a, 1, bs, a, 1, bs, g);
                applyOp(sa, o);
                sops.push_back(o);
            }
        }
        while (true) {
            bfs(sa, 1, par, dep);
            int ga = 0, yb = 0;
            for (int v = 1; v <= n; v++) {
                if (v == 1 || v == bs) continue;
                if (par[v] == bs && inA[v]) ga = v;
                if (par[v] == 1 && inG[v]) yb = v;
            }
            if (!ga && !yb) break;
            Op o;
            if (ga && yb) {
                o = mkEdges(ga, bs, 1, yb, 1, bs, 1, ga, bs, yb);
            } else if (ga) {
                int y = 0;
                for (int x = 1; x <= n; x++)
                    if (sa[1][x] && x != bs) { y = x; break; }
                o = mkEdges(ga, bs, 1, y, 1, ga, 1, bs, 1, y);
            } else {
                int g = 0;
                for (int x = 1; x <= n; x++)
                    if (x != 1 && par[x] == bs) { g = x; break; }
                o = mkEdges(g, bs, 1, yb, bs, g, bs, 1, bs, yb);
            }
            applyOp(sa, o);
            sops.push_back(o);
        }
        out = sops;
        for (int i = (int)tops.size() - 1; i >= 0; i--) {
            const Op &t = tops[i];
            Op r = mkEdges(t.qv[0], t.qv[1], t.qv[2], t.qv[3],
                           t.p[0], t.p[1], t.p[1], t.p[2], t.p[2], t.p[3]);
            out.push_back(r);
        }
    }
    printf("YES\n%d\n", (int)out.size());
    for (auto &o : out) {
        printf("%d %d %d %d\n", o.p[0], o.p[1], o.p[2], o.p[3]);
        printf("%d %d %d %d %d %d\n", o.e[0], o.e[1], o.e[2], o.e[3], o.e[4], o.e[5]);
    }
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
int n, m;
vector<int> ex, ey, ew;
vector<int> relE;
vector<char> rel;
struct St { ll l, s, q; bool ok; };
double best = 1e18;
int queries = 0;
const double INF = 1e18;

double varOf(const St& a) {
    return (double)(a.l * a.q - a.s * a.s) / ((double)a.l * a.l);
}
double valAt(const St& a, double x) {
    return ((double)a.q - 2 * x * a.s + (double)a.l * x * x) / a.l;
}
bool same(const St& a, const St& b) {
    return a.q * b.l == b.q * a.l && a.s * b.l == b.s * a.l;
}

St query(double x) {
    queries++;
    vector<double> c(m);
    for (int e = 0; e < m; e++) c[e] = (ew[e] - x) * (ew[e] - x);
    St bestS{0, 0, 0, false};
    double bestV = INF;
    // path DP
    {
        int L = n - 1;
        vector<vector<double>> d(L + 1, vector<double>(n + 1, INF));
        vector<vector<int>> par(L + 1, vector<int>(n + 1, -1));
        d[0][1] = 0;
        for (int k = 1; k <= L; k++) {
            for (int e : relE) {
                int u = ex[e], v = ey[e];
                if (d[k - 1][u] < INF) {
                    double nv = d[k - 1][u] + c[e];
                    if (nv < d[k][v]) { d[k][v] = nv; par[k][v] = e; }
                }
            }
        }
        int bl = -1;
        double bm = INF;
        for (int k = 1; k <= L; k++) {
            if (d[k][n] < INF) {
                double mu = d[k][n] / k;
                if (mu < bm) { bm = mu; bl = k; }
            }
        }
        if (bl > 0) {
            St s{bl, 0, 0, true};
            int v = n;
            for (int k = bl; k >= 1; k--) {
                int e = par[k][v];
                s.s += ew[e];
                s.q += (ll)ew[e] * ew[e];
                v = ex[e];
            }
            best = min(best, varOf(s));
            double val = valAt(s, x);
            if (val < bestV) { bestV = val; bestS = s; }
        }
    }
    // cycle Karp
    {
        int r = 0;
        for (int v = 1; v <= n; v++) if (rel[v]) r++;
        int N = r;
        vector<vector<double>> d(N + 1, vector<double>(n + 1, INF));
        vector<vector<int>> par(N + 1, vector<int>(n + 1, -1));
        for (int v = 1; v <= n; v++) if (rel[v]) d[0][v] = 0;
        for (int k = 1; k <= N; k++) {
            for (int e : relE) {
                int u = ex[e], v = ey[e];
                if (d[k - 1][u] < INF) {
                    double nv = d[k - 1][u] + c[e];
                    if (nv < d[k][v]) { d[k][v] = nv; par[k][v] = e; }
                }
            }
        }
        int bv = -1;
        double bm = INF;
        for (int v = 1; v <= n; v++) {
            if (!rel[v] || d[N][v] >= INF) continue;
            double mx = -INF;
            for (int k = 0; k < N; k++) {
                if (d[k][v] < INF) {
                    mx = max(mx, (d[N][v] - d[k][v]) / (N - k));
                }
            }
            if (mx < bm) { bm = mx; bv = v; }
        }
        if (bv > 0) {
            vector<int> vs, es;
            int v = bv;
            vs.push_back(v);
            for (int k = N; k >= 1; k--) {
                int e = par[k][v];
                es.push_back(e);
                v = ex[e];
                vs.push_back(v);
            }
            vector<int> pos(n + 1, -1);
            vector<int> stk, stkE;
            stk.push_back(vs[0]);
            stkE.push_back(-1);
            pos[vs[0]] = 0;
            St bc{0, 0, 0, false};
            double bcv = INF;
            for (int i = 1; i <= N; i++) {
                int u = vs[i];
                int e = es[i - 1];
                if (pos[u] != -1) {
                    int p = pos[u];
                    St s{0, 0, 0, true};
                    for (int j = p + 1; j < (int)stk.size(); j++) {
                        int ee = stkE[j];
                        s.l++; s.s += ew[ee]; s.q += (ll)ew[ee] * ew[ee];
                    }
                    s.l++; s.s += ew[e]; s.q += (ll)ew[e] * ew[e];
                    while ((int)stk.size() > p + 1) {
                        pos[stk.back()] = -1;
                        stk.pop_back();
                        stkE.pop_back();
                    }
                    double val = valAt(s, x);
                    if (val < bcv) { bcv = val; bc = s; }
                } else {
                    pos[u] = stk.size();
                    stk.push_back(u);
                    stkE.push_back(e);
                }
            }
            if (bc.ok) {
                best = min(best, varOf(bc));
                if (bcv < bestV) { bestV = bcv; bestS = bc; }
            }
        }
    }
    return bestS;
}

void solve(double a, const St& Sa, double b, const St& Sb) {
    if (queries > 6000) return;
    if (same(Sa, Sb)) return;
    double Aa = (double)Sa.q / Sa.l, Ba = (double)Sa.s / Sa.l;
    double Ab = (double)Sb.q / Sb.l, Bb = (double)Sb.s / Sb.l;
    if (fabs(Ba - Bb) < 1e-15) return;
    double xc = (Aa - Ab) / (2 * (Ba - Bb));
    if (xc <= a + 1e-13 || xc >= b - 1e-13) return;
    St Sc = query(xc);
    if (!Sc.ok) return;
    double vc = valAt(Sc, xc);
    double va = valAt(Sa, xc);
    if (vc >= va - 1e-11) return;
    solve(a, Sa, xc, Sc);
    solve(xc, Sc, b, Sb);
}

int main() {
    scanf("%d %d", &n, &m);
    ex.resize(m); ey.resize(m); ew.resize(m);
    vector<vector<int>> g(n + 1), rg(n + 1);
    for (int i = 0; i < m; i++) {
        scanf("%d %d %d", &ex[i], &ey[i], &ew[i]);
        g[ex[i]].push_back(ey[i]);
        rg[ey[i]].push_back(ex[i]);
    }
    vector<char> f(n + 1, 0), b(n + 1, 0);
    {
        queue<int> qu; qu.push(1); f[1] = 1;
        while (!qu.empty()) {
            int u = qu.front(); qu.pop();
            for (int v : g[u]) if (!f[v]) { f[v] = 1; qu.push(v); }
        }
        qu.push(n); b[n] = 1;
        while (!qu.empty()) {
            int u = qu.front(); qu.pop();
            for (int v : rg[u]) if (!b[v]) { b[v] = 1; qu.push(v); }
        }
    }
    if (!f[n]) { printf("-1\n"); return 0; }
    rel.assign(n + 1, 0);
    for (int v = 1; v <= n; v++) rel[v] = f[v] && b[v];
    for (int e = 0; e < m; e++) if (rel[ex[e]] && rel[ey[e]]) relE.push_back(e);
    St S0 = query(0.0);
    St S1 = query(20.0);
    if (S0.ok && S1.ok) solve(0.0, S0, 20.0, S1);
    printf("%.15f\n", best);
    return 0;
}

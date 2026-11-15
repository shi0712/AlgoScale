#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using i128 = __int128_t;

static const ll INF_LL = (ll)4e18;
static const double INF_D = 1e300;

int n, m;
struct Edge { int u, v, w; };
vector<Edge> edges;

vector<char> inR;          // vertex belongs to relevant region
vector<int> relEdge;       // indices of edges fully inside R

// ---------------------------------------------------------------
// Cycle representation: l = length, S = sum of weights,
// Q = sum of squared weights.  A cycle corresponds to the parabola
//   phi_C(t) = (t - S/l)^2 + (Q/l - (S/l)^2) = t^2 + Led_C(t)
// with Led_C(t) = (Q - 2 t S)/l.
// ---------------------------------------------------------------
struct Cyc {
    ll l = 0, S = 0, Q = 0;
    bool valid = false;
};

static inline double varianceOf(const Cyc &c) {
    // (l*Q - S^2) / l^2
    i128 num = (i128)c.l * c.Q - (i128)c.S * c.S;
    double den = (double)c.l * (double)c.l;
    return (double)num / den;
}

// ---------------------------------------------------------------
// Oracle: minimum mean cost cycle for edge costs (w - P/Qd)^2.
// Equivalent (scaled by Qd^2 > 0) to integer costs (w*Qd - P)^2.
// Uses Karp's algorithm, then extracts one optimal cycle exactly
// via reduced costs.
// ---------------------------------------------------------------
static ll dpK[105][105];
static ll costE[1005];
static i128 pot[105];
static vector<int> tightAdj[105];
static int parV[105], parE[105];
static char col[105];
static vector<int> cycVerts, cycEdges;
static bool foundCycle;

static bool dfsTight(int u) {
    col[u] = 1;
    for (int ei : tightAdj[u]) {
        int v = edges[ei].v;
        if (col[v] == 0) {
            parV[v] = u;
            parE[v] = ei;
            if (dfsTight(v)) return true;
        } else if (col[v] == 1) {
            // cycle: v ... u -> v
            cycVerts.clear();
            cycEdges.clear();
            vector<int> tmp;
            int x = u;
            while (x != v) {
                tmp.push_back(x);
                x = parV[x];
            }
            reverse(tmp.begin(), tmp.end());
            cycVerts.push_back(v);
            for (int y : tmp) cycVerts.push_back(y);
            for (size_t i = 1; i < cycVerts.size(); i++) cycEdges.push_back(parE[cycVerts[i]]);
            cycEdges.push_back(ei);
            return true;
        }
    }
    col[u] = 2;
    return false;
}

static Cyc oracle(ll P, ll Qd, double &bestVar) {
    // Qd > 0 guaranteed by caller
    Cyc res;
    for (size_t idx = 0; idx < relEdge.size(); idx++) {
        int ei = relEdge[idx];
        i128 d = (i128)edges[ei].w * Qd - P;
        costE[idx] = (ll)(d * d);
    }
    int ne = (int)relEdge.size();

    for (int v = 0; v < n; v++) dpK[0][v] = inR[v] ? 0 : INF_LL;
    for (int k = 1; k <= n; k++) {
        for (int v = 0; v < n; v++) dpK[k][v] = INF_LL;
        for (int idx = 0; idx < ne; idx++) {
            int ei = relEdge[idx];
            int u = edges[ei].u, v = edges[ei].v;
            ll prev = dpK[k - 1][u];
            if (prev < INF_LL) {
                ll val = prev + costE[idx];
                if (val < dpK[k][v]) dpK[k][v] = val;
            }
        }
    }

    // Karp: theta = min_v max_{0<=k<n} (dp[n][v]-dp[k][v])/(n-k)
    bool got = false;
    ll A = 0, B = 1;
    for (int v = 0; v < n; v++) {
        if (!inR[v] || dpK[n][v] >= INF_LL) continue;
        bool loc = false;
        ll bn = 0, bd = 1;
        for (int k = 0; k < n; k++) {
            if (dpK[k][v] >= INF_LL) continue;
            ll num = dpK[n][v] - dpK[k][v];
            ll den = (ll)(n - k);
            if (!loc || (i128)num * bd > (i128)bn * den) {
                bn = num; bd = den; loc = true;
            }
        }
        if (loc) {
            if (!got || (i128)bn * B < (i128)A * bd) {
                A = bn; B = bd; got = true;
            }
        }
    }
    if (!got) return res;

    // reduce A/B
    {
        ll a = A < 0 ? -A : A, b = B;
        while (b) { ll t = a % b; a = b; b = t; }
        ll g = a == 0 ? 1 : a;
        A /= g; B /= g;
    }

    // potentials q[v] = min_k (dp[k][v]*B - A*k)
    for (int v = 0; v < n; v++) pot[v] = ((i128)1 << 120);
    for (int v = 0; v < n; v++) {
        if (!inR[v]) continue;
        for (int k = 0; k <= n; k++) {
            if (dpK[k][v] >= INF_LL) continue;
            i128 val = (i128)dpK[k][v] * B - (i128)A * k;
            if (val < pot[v]) pot[v] = val;
        }
    }

    for (int v = 0; v < n; v++) tightAdj[v].clear();
    for (int idx = 0; idx < ne; idx++) {
        int ei = relEdge[idx];
        int u = edges[ei].u, v = edges[ei].v;
        i128 red = (i128)costE[idx] * B - A;
        if (red + pot[u] - pot[v] == 0) tightAdj[u].push_back(ei);
    }

    for (int v = 0; v < n; v++) col[v] = 0;
    foundCycle = false;
    for (int v = 0; v < n && !foundCycle; v++) {
        if (inR[v] && col[v] == 0) {
            if (dfsTight(v)) foundCycle = true;
        }
    }
    if (!foundCycle) return res;

    ll L = 0, S = 0, Q = 0;
    for (int ei : cycEdges) {
        L++;
        S += edges[ei].w;
        Q += (ll)edges[ei].w * edges[ei].w;
    }
    res.l = L; res.S = S; res.Q = Q; res.valid = true;
    bestVar = min(bestVar, varianceOf(res));
    return res;
}

// ---------------------------------------------------------------
// Fraction helpers
// ---------------------------------------------------------------
struct Frac { i128 num, den; };  // den > 0

static bool fracLess(const Frac &a, const Frac &b) {
    return a.num * b.den < b.num * a.den;
}

static bool sameParabola(const Cyc &a, const Cyc &b) {
    return (i128)a.S * b.l == (i128)b.S * a.l &&
           (i128)a.Q * b.l == (i128)b.Q * a.l;
}

// intersection of Led_A and Led_B
static Frac intersectLine(const Cyc &A, const Cyc &B) {
    i128 num = (i128)B.Q * A.l - (i128)A.Q * B.l;
    i128 den = 2 * ((i128)B.S * A.l - (i128)A.S * B.l);
    if (den < 0) { num = -num; den = -den; }
    if (den == 0) return Frac{0, 0};  // same mean: caller must skip
    // reduce (guard against negative num)
    i128 g = 0;
    i128 an = num < 0 ? -num : num;
    i128 ad = den;
    while (ad) { i128 t = an % ad; an = ad; ad = t; }
    g = an;
    if (g > 1) { num /= g; den /= g; }
    return Frac{num, den};
}

// ---------------------------------------------------------------
// Main
// ---------------------------------------------------------------
int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    edges.resize(m);
    vector<vector<int>> fadj(n), badj(n);
    for (int i = 0; i < m; i++) {
        int x, y, w;
        scanf("%d %d %d", &x, &y, &w);
        --x; --y;
        edges[i] = {x, y, w};
        fadj[x].push_back(y);
        badj[y].push_back(x);
    }

    // reachability from vertex 0 and to vertex n-1
    vector<char> r1(n, 0), rn(n, 0);
    {
        vector<int> st;
        st.push_back(0); r1[0] = 1;
        while (!st.empty()) {
            int u = st.back(); st.pop_back();
            for (int v : fadj[u]) if (!r1[v]) { r1[v] = 1; st.push_back(v); }
        }
        st.push_back(n - 1); rn[n - 1] = 1;
        while (!st.empty()) {
            int u = st.back(); st.pop_back();
            for (int v : badj[u]) if (!rn[v]) { rn[v] = 1; st.push_back(v); }
        }
    }

    if (!r1[n - 1]) { printf("-1\n"); return 0; }

    inR.assign(n, 0);
    for (int v = 0; v < n; v++) inR[v] = (r1[v] && rn[v]);
    for (int i = 0; i < m; i++) {
        if (inR[edges[i].u] && inR[edges[i].v]) relEdge.push_back(i);
    }

    // ---------------- path part ----------------
    // min over walks 1 -> n of length <= n-1 of variance
    double ans = INF_D;
    int maxS = 20 * (n - 1);
    vector<ll> dpPrev((size_t)n * (maxS + 1), INF_LL);
    vector<ll> dpCur((size_t)n * (maxS + 1), INF_LL);
    auto at = [&](vector<ll> &d, int v, int s) -> ll& { return d[(size_t)v * (maxS + 1) + s]; };
    at(dpPrev, 0, 0) = 0;
    for (int l = 1; l <= n - 1; l++) {
        fill(dpCur.begin(), dpCur.end(), INF_LL);
        int prevMax = 20 * (l - 1);
        for (int ei : relEdge) {
            const Edge &e = edges[ei];
            ll w2 = (ll)e.w * e.w;
            const ll *src = &dpPrev[(size_t)e.u * (maxS + 1)];
            ll *dst = &dpCur[(size_t)e.v * (maxS + 1)];
            for (int s = 0; s <= prevMax; s++) {
                ll val = src[s];
                if (val < INF_LL) {
                    ll nv = val + w2;
                    if (nv < dst[s + e.w]) dst[s + e.w] = nv;
                }
            }
        }
        // check vertex n-1
        {
            const ll *src = &dpCur[(size_t)(n - 1) * (maxS + 1)];
            for (int s = 0; s <= 20 * l; s++) {
                ll q = src[s];
                if (q < INF_LL) {
                    i128 num = (i128)l * q - (i128)s * s;
                    double v = (double)num / ((double)l * l);
                    if (v < ans) ans = v;
                }
            }
        }
        dpPrev.swap(dpCur);
    }

    // ---------------- cycle part ----------------
    // Answer over cycles = min over all cycles of variance.  For a cycle C,
    // phi_C(t) = (t - mean_C)^2 + var_C, and min over cycles of var_C equals
    // min_t (min over cycles of phi_C(t)).  The inner min is a min-mean-cycle
    // problem with edge costs (w - t)^2, solved by Karp.  All parabolas have
    // the same curvature, so the lower envelope's breakpoints can be found by
    // divide and conquer on exact rational intersection points.
    double bestCycle = INF_D;
    {
        // oracle with P=0,Qd=1 returns invalid iff R contains no cycle
        Cyc c0 = oracle(0, 1, bestCycle);
        if (c0.valid) {
            Cyc c20 = oracle(20, 1, bestCycle);
            if (!sameParabola(c0, c20)) {
                // stack of tasks
                struct Task { Cyc A; Frac t1; Cyc B; Frac t2; };
                vector<Task> st;
                st.push_back(Task{c0, Frac{0, 1}, c20, Frac{20, 1}});
                while (!st.empty()) {
                    Task tk = st.back(); st.pop_back();
                    if (sameParabola(tk.A, tk.B)) continue;
                    Frac ts = intersectLine(tk.A, tk.B);
                    if (ts.den == 0) continue;
                    if (!(fracLess(tk.t1, ts) && fracLess(ts, tk.t2))) continue;
                    ll P = (ll)ts.num, Qd = (ll)ts.den;
                    Cyc C = oracle(P, Qd, bestCycle);
                    if (!C.valid) continue;
                    if (sameParabola(C, tk.A) || sameParabola(C, tk.B)) continue;
                    st.push_back(Task{tk.A, tk.t1, C, ts});
                    st.push_back(Task{C, ts, tk.B, tk.t2});
                }
            }
        }
    }

    if (bestCycle < ans) ans = bestCycle;
    if (ans >= INF_D / 2) printf("-1\n");
    else printf("%.15f\n", ans);
    return 0;
}

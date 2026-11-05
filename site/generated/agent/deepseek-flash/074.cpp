#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using i128 = __int128_t;

/*
  Walk Variance.

  Key facts:
   * For a walk W, Var(W) = min_mu (1/|W|) * sum_e (w_e - mu)^2 .
   * inf over walks from 1 to n of Var  =  min over {simple paths 1->n} U {simple cycles
     lying on a 1->n walk} of Var.   (any walk decomposes into a simple path plus closed
     sub-walks, and the average of non-negative costs is at least the minimum of the parts;
     a cycle can be traversed arbitrarily often, so its variance is approachable.)
   * Hence answer = min_{mu} h(mu), where h(mu) = min over structures of the average
     edge cost with c_e = (w_e - mu)^2.
   * Every structure p gives a parabola cost_p(mu) = (mu - x_p)^2 + V_p with
     x_p = S/k, V_p = Var.  All parabolas have the same shape, so the lower envelope
     is exactly the lower hull of the points (x_p, y_p), y_p = Q/k, and the minimum of
     the envelope equals min_p V_p.  We enumerate the hull vertices with the standard
     divide-and-conquer: given two hull vertices p,q, evaluate at the mu where their
     parabolas cross; the argmin there is a hull vertex strictly between them (if any).

  All arithmetic is exact (rational): mu = A/B, edge cost scaled by B^2:
     c_e = B^2 w^2 - 2 A B w + A^2  (integer >= 0).
*/

int n, m;
vector<array<int, 3>> Ein;

int nn_;
vector<array<int, 3>> E;   // renumbered edges among relevant vertices
int src, tgt;

const ll INF = (ll)3e18;

vector<ll> cbuf;
vector<ll> dpP;    // (N*N)
vector<int> parP;
vector<ll> Fk;     // (N+2)*N

struct Struct { ll k, S, Q; };

// compare a/b with c/d  (b,d > 0)
static inline int cmpfrac(ll a, ll b, ll c, ll d) {
    i128 l = (i128)a * d, r = (i128)c * b;
    if (l < r) return -1;
    if (l > r) return 1;
    return 0;
}

// argmin over structures of average cost at mu = A/B (B > 0)
Struct evalMu(ll A, ll B, bool &ok) {
    ok = false;
    Struct res{0, 0, 0};
    const int N = nn_;
    const int M = (int)E.size();

    cbuf.resize(M);
    for (int i = 0; i < M; i++) {
        ll w = E[i][2];
        cbuf[i] = B * B * w * w - 2 * A * B * w + A * A;
    }

    /* ---------------- simple paths / short walks 1 -> n : exactly j edges, j<=N-1 */
    dpP.assign((size_t)N * N, INF);
    parP.assign((size_t)N * N, -1);
    dpP[(size_t)0 * N + src] = 0;
    for (int j = 0; j + 1 < N; j++) {
        const ll *cur = &dpP[(size_t)j * N];
        ll *nxt = &dpP[(size_t)(j + 1) * N];
        for (int ei = 0; ei < M; ei++) {
            int x = E[ei][0], y = E[ei][1];
            ll c = cur[x];
            if (c >= INF) continue;
            ll val = c + cbuf[ei];
            if (val < nxt[y]) { nxt[y] = val; parP[(size_t)(j + 1) * N + y] = ei; }
        }
    }
    ll bestC = -1;
    int bestJ = -1;
    for (int j = 1; j < N; j++) {
        ll C = dpP[(size_t)j * N + tgt];
        if (C >= INF) continue;
        if (bestJ < 0 || cmpfrac(C, j, bestC, bestJ) < 0) { bestC = C; bestJ = j; }
    }
    Struct sp{0, 0, 0};
    bool hasPath = false;
    if (bestJ > 0) {
        hasPath = true;
        int v = tgt;
        for (int j = bestJ; j >= 1; j--) {
            int ei = parP[(size_t)j * N + v];
            int w = E[ei][2];
            sp.k++; sp.S += w; sp.Q += (ll)w * w;
            v = E[ei][0];
        }
    }

    /* ---------------- cycles : Karp's minimum mean cycle ----------------
       Virtual source: F_0(v) = 0 for all v, F_j = min cost of a j-edge walk.
       The augmented graph has N+1 vertices, so Karp's formula uses lengths 1..N+1:
         mu* = min_v max_{0<=j<=N} (F_{N+1}(v) - F_j(v)) / (N+1-j).
    */
    Fk.assign((size_t)(N + 2) * N, INF);
    for (int v = 0; v < N; v++) Fk[v] = 0;             // virtual source, j = 0
    for (int j = 1; j <= N + 1; j++) {
        const ll *cur = &Fk[(size_t)(j - 1) * N];
        ll *nxt = &Fk[(size_t)j * N];
        for (int ei = 0; ei < M; ei++) {
            int x = E[ei][0], y = E[ei][1];
            ll c = cur[x];
            if (c >= INF) continue;
            ll val = c + cbuf[ei];
            if (val < nxt[y]) nxt[y] = val;
        }
    }
    bool have = false;
    ll bnum = 0, bden = 1;
    const ll *last = &Fk[(size_t)(N + 1) * N];
    for (int v = 0; v < N; v++) {
        if (last[v] >= INF) continue;
        ll num = 0, den = 1;
        int jj = -1;
        for (int j = 0; j <= N; j++) {
            ll fj = Fk[(size_t)j * N + v];
            if (fj >= INF) continue;
            ll nu = last[v] - fj, de = N + 1 - j;
            if (jj < 0 || cmpfrac(nu, de, num, den) > 0) { num = nu; den = de; jj = j; }
        }
        if (jj < 0) continue;
        if (!have || cmpfrac(num, den, bnum, bden) < 0) { have = true; bnum = num; bden = den; }
    }
    /*
      Extraction of an actual minimum mean cycle.
      lambda* = bnum/bden = p/q.  With costs c'_e = q*c_e - p every cycle has total c' >= 0,
      and the minimum mean cycles are exactly the cycles with total c' == 0.
      Hence the graph has no negative cycle: shortest distances d[] (virtual source connected
      to every vertex with a 0-cost edge) are well defined, and the edges tight w.r.t. d[]
      (d[y] == d[x] + c'_e) contain a cycle; every such cycle has total c' == 0.
    */
    Struct sc{0, 0, 0};
    bool hasCyc = false;
    ll cycC = 0;
    if (have) {
        vector<ll> cp(M);
        for (int i = 0; i < M; i++) cp[i] = bden * cbuf[i] - bnum;
        vector<ll> d(N, 0);
        for (int it = 0; it < N; it++) {
            bool ch = false;
            for (int ei = 0; ei < M; ei++) {
                int x = E[ei][0], y = E[ei][1];
                ll nd = d[x] + cp[ei];
                if (nd < d[y]) { d[y] = nd; ch = true; }
            }
            if (!ch) break;
        }
        // tight-edge subgraph
        vector<vector<pair<int, int>>> adjT(N);   // (to, edge id)
        for (int ei = 0; ei < M; ei++) {
            int x = E[ei][0], y = E[ei][1];
            if ((i128)d[x] + cp[ei] == (i128)d[y]) adjT[x].push_back({y, ei});
        }
        vector<int> state(N, 0), parE(N, -1), it(N, 0), stk;
        int foundU = -1, foundV = -1, foundE = -1;   // back edge foundV -> foundU
        for (int s0 = 0; s0 < N && foundU < 0; s0++) {
            if (state[s0]) continue;
            stk.clear();
            stk.push_back(s0);
            state[s0] = 1;
            while (!stk.empty() && foundU < 0) {
                int u = stk.back();
                if (it[u] == (int)adjT[u].size()) { state[u] = 2; stk.pop_back(); continue; }
                int y = adjT[u][it[u]].first, ei = adjT[u][it[u]].second;
                it[u]++;
                if (state[y] == 1) { foundU = y; foundV = u; foundE = ei; break; }
                if (state[y] == 0) { state[y] = 1; parE[y] = ei; stk.push_back(y); }
            }
        }
        if (foundU >= 0) {
            hasCyc = true;
            for (int cur = foundV; cur != foundU; ) {
                int ei = parE[cur];
                int ww = E[ei][2];
                sc.k++; sc.S += ww; sc.Q += (ll)ww * ww; cycC += cbuf[ei];
                cur = E[ei][0];
            }
            int ww = E[foundE][2];
            sc.k++; sc.S += ww; sc.Q += (ll)ww * ww; cycC += cbuf[foundE];
        } else if (have) {
            // Safety net (unreachable in theory): fall back to any simple cycle.
            for (int ei0 = 0; ei0 < M && !hasCyc; ei0++) {
                int u0 = E[ei0][0], v0 = E[ei0][1];
                vector<int> par(N, -2), pe(N, -1);
                vector<int> q;
                par[v0] = -1;
                q.push_back(v0);
                for (size_t h = 0; h < q.size() && par[u0] == -2; h++) {
                    int u = q[h];
                    for (int ei2 = 0; ei2 < M; ei2++) {
                        if (E[ei2][0] != u) continue;
                        int y = E[ei2][1];
                        if (par[y] != -2) continue;
                        par[y] = u; pe[y] = ei2; q.push_back(y);
                    }
                }
                if (par[u0] == -2) continue;
                sc = {0, 0, 0}; cycC = 0;
                int ww = E[ei0][2];
                sc.k++; sc.S += ww; sc.Q += (ll)ww * ww; cycC += cbuf[ei0];
                for (int x = u0; x != v0; x = par[x]) {
                    int ei2 = pe[x]; ww = E[ei2][2];
                    sc.k++; sc.S += ww; sc.Q += (ll)ww * ww; cycC += cbuf[ei2];
                }
                hasCyc = true;
            }
        }
    }

    if (hasPath && hasCyc) {
        ok = true;
        if (cmpfrac(cycC, sc.k, bestC, bestJ) < 0) return sc;
        return sp;
    }
    if (hasPath) { ok = true; return sp; }
    if (hasCyc) { ok = true; return sc; }
    return res;
}

int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    Ein.resize(m);
    for (int i = 0; i < m; i++) {
        int x, y, w;
        scanf("%d %d %d", &x, &y, &w);
        Ein[i] = {x - 1, y - 1, w};
    }

    vector<vector<int>> fwd(n), rev(n);
    for (auto &e : Ein) { fwd[e[0]].push_back(e[1]); rev[e[1]].push_back(e[0]); }
    vector<char> r1(n, 0), rn(n, 0);
    {
        vector<int> q; q.push_back(0); r1[0] = 1;
        for (size_t h = 0; h < q.size(); h++) {
            int u = q[h];
            for (int v : fwd[u]) if (!r1[v]) { r1[v] = 1; q.push_back(v); }
        }
    }
    {
        vector<int> q; q.push_back(n - 1); rn[n - 1] = 1;
        for (size_t h = 0; h < q.size(); h++) {
            int u = q[h];
            for (int v : rev[u]) if (!rn[v]) { rn[v] = 1; q.push_back(v); }
        }
    }
    if (!r1[n - 1]) { printf("-1\n"); return 0; }

    vector<int> id(n, -1);
    nn_ = 0;
    for (int v = 0; v < n; v++) if (r1[v] && rn[v]) id[v] = nn_++;
    src = id[0];
    tgt = id[n - 1];
    E.clear();
    for (auto &e : Ein)
        if (id[e[0]] >= 0 && id[e[1]] >= 0) E.push_back({id[e[0]], id[e[1]], e[2]});

    // ---- hull recursion over mu -------------------------------------------
    const ll M = 10000000LL;   // large enough to select min/max mean structures
    bool okA, okB;
    Struct pL = evalMu(-M, 1, okA);
    Struct pR = evalMu(M, 1, okB);

    ll anum = -1, aden = 1;
    auto upd = [&](const Struct &s) {
        if (s.k == 0) return;
        ll num = s.k * s.Q - s.S * s.S;
        ll den = s.k * s.k;
        if (anum < 0 || cmpfrac(num, den, anum, aden) < 0) { anum = num; aden = den; }
    };
    upd(pL); upd(pR);

    vector<pair<Struct, Struct>> st;
    st.reserve(1024);
    st.push_back({pL, pR});
    while (!st.empty() && anum != 0) {
        Struct p = st.back().first, q = st.back().second;
        st.pop_back();
        if (p.k == 0 || q.k == 0) continue;
        // same mean?
        i128 lhs = (i128)p.S * q.k, rhs = (i128)q.S * p.k;
        if (lhs == rhs) continue;
        ll A = (ll)((i128)q.Q * p.k - (i128)p.Q * q.k);
        ll B = (ll)(2 * ((i128)q.S * p.k - (i128)p.S * q.k));
        if (B < 0) { A = -A; B = -B; }
        if (B == 0) continue;
        bool ok;
        Struct r = evalMu(A, B, ok);
        if (!ok || r.k == 0) continue;
        upd(r);            // r is a concrete structure: always a valid candidate
        // for the recursion it must lie strictly between x_p and x_q
        bool between;
        if (lhs < rhs) {
            between = ((i128)p.S * r.k < (i128)r.S * p.k) && ((i128)r.S * q.k < (i128)q.S * r.k);
        } else {
            between = ((i128)q.S * r.k < (i128)r.S * q.k) && ((i128)r.S * p.k < (i128)p.S * r.k);
        }
        if (!between) continue;
        i128 Cr = (i128)B * B * r.Q - (i128)2 * A * B * r.S + (i128)A * A * r.k;
        i128 Cp = (i128)B * B * p.Q - (i128)2 * A * B * p.S + (i128)A * A * p.k;
        if (Cr * (i128)p.k >= Cp * (i128)r.k) continue;   // no structure strictly better
        st.push_back({p, r});
        st.push_back({r, q});
    }

    if (anum < 0) { printf("-1\n"); return 0; }
    long double ans = (long double)anum / (long double)aden;
    if (ans < 0 && ans > -1e-18L) ans = 0;
    printf("%.15Lf\n", ans);
    return 0;
}

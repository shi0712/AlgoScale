// Grid Toll
//
// Objective (as evidenced by the samples): increase edge weights (only up) so that
// the shortest path from column 1 to column m becomes exactly D+k, minimising the
// TOTAL AMOUNT of increase (sum over edges of newWeight - oldWeight).  Print that
// minimum total plus the resulting weights.
//
// Solution outline
// ----------------
// Let target = D + k.  We need w' >= w with dist_{w'}(source set -> sink set) == target
// and sum(w'-w) minimal.
//
// LP duality (path packing): the minimum total increase equals
//      max over families of edge-disjoint source-sink paths of  sum_P (target - len(P))
// which is a min-cost flow: send flow from a super source to each column-1 cell,
// from each column-m cell to a super sink (arc into the sink costs -target), grid
// arcs have capacity 1 and cost = weight.  Augment while the shortest path is negative.
//
// Reconstruction of the weights uses the primal (potential) formulation
//      min sum_e max(0, |p(u)-p(v)| - w_e)   s.t.  p = 0 on used sources,
//                                                 p = target on used sinks,
//                                                 p <= 0 on unused sources,
//                                                 p >= target on unused sinks.
// This is a system of difference constraints on the residual graph of the optimal
// flow; any feasible solution costs exactly the optimum:
//   * unused edge  -> both residual directions -> |dp| <= w      (no increase)
//   * used edge u->v -> residual arc v->u of cost -w -> p(v) >= p(u)+w, and the
//     cost telescopes along each packed path to (target - len(P)).
// Then w'_e = max(w_e, |p(u)-p(v)|) satisfies every path length >= p(t)-p(s) >= target,
// and the total increase is exactly the optimum, hence the new shortest path is target.

#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
static const ll INF = (ll)4e18;

/* ------------------------------------------------------------------ */
/* fast input / output                                                 */
/* ------------------------------------------------------------------ */
static char *ibuf;
static size_t ipos = 0, ilen = 0;
static inline int gc() {
    if (ipos == ilen) {
        ilen = fread(ibuf, 1, 1 << 16, stdin);
        ipos = 0;
        if (ilen == 0) return -1;
    }
    return ibuf[ipos++];
}
static inline ll readInt() {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9') && c != '-') c = gc();
    int sgn = 1;
    if (c == '-') { sgn = -1; c = gc(); }
    ll x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return x * sgn;
}
static char *obuf;
static size_t opos = 0;
static inline void flushOut() { fwrite(obuf, 1, opos, stdout); opos = 0; }
static inline void wc(char c) {
    if (opos == (1 << 16)) flushOut();
    obuf[opos++] = c;
}
static inline void writeInt(ll x) {
    if (x < 0) { wc('-'); x = -x; }
    char tmp[24];
    int n = 0;
    do { tmp[n++] = char('0' + x % 10); x /= 10; } while (x);
    while (n) wc(tmp[--n]);
}

/* ------------------------------------------------------------------ */
/* min cost flow                                                       */
/* ------------------------------------------------------------------ */
/* radix heap: monotone integer priority queue, keys must be non-negative
   and extracted in non-decreasing order (Dijkstra). */
struct RadixHeap {
    vector<pair<ll, int>> b[65];
    ll last = 0;
    int sz = 0;
    void clear() {
        for (int i = 0; i < 65; i++) b[i].clear();
        last = 0; sz = 0;
    }
    static int bucket(ll key, ll last) {
        if (key == last) return 0;
        return 64 - __builtin_clzll((unsigned long long)(key ^ last));
    }
    void push(ll key, int val) {
        b[bucket(key, last)].push_back({key, val});
        sz++;
    }
    pair<ll, int> pop() {
        if (b[0].empty()) {
            int i = 1;
            while (b[i].empty()) i++;
            ll mn = b[i][0].first;
            for (auto &p : b[i]) if (p.first < mn) mn = p.first;
            last = mn;
            for (auto &p : b[i]) b[bucket(p.first, last)].push_back(p);
            b[i].clear();
        }
        auto p = b[0].back();
        b[0].pop_back();
        sz--;
        return p;
    }
};

struct Arc { int to, rev; ll cap, cost; };
struct MCF {
    int N;
    vector<vector<Arc>> g;
    MCF(int n) : N(n), g(n) {}
    void add(int u, int v, ll cap, ll cost) {
        g[u].push_back({v, (int)g[v].size(), cap, cost});
        g[v].push_back({u, (int)g[u].size() - 1, 0, -cost});
    }
};

int main() {
    ibuf = (char *)malloc(1 << 16);
    obuf = (char *)malloc(1 << 16);

    int Tc = (int)readInt();
    while (Tc-- > 0) {
        int n = (int)readInt(), m = (int)readInt();
        ll K = readInt();
        vector<vector<ll>> H(n, vector<ll>(m - 1)), V(n - 1, vector<ll>(m));
        for (int r = 0; r < n; r++)
            for (int j = 0; j < m - 1; j++) H[r][j] = readInt();
        for (int r = 0; r + 1 < n; r++)
            for (int j = 0; j < m; j++) V[r][j] = readInt();

        const int N = n * m;
        const int S = N, T = N + 1;

        /* ---- shortest distances from column 1 (Dijkstra) ---- */
        vector<ll> d(N, INF);
        {
            RadixHeap pq;
            auto relax = [&](int u, int v, ll w) {
                if (d[v] > d[u] + w) { d[v] = d[u] + w; pq.push(d[v], v); }
            };
            for (int r = 0; r < n; r++) { d[r * m] = 0; pq.push(0, r * m); }
            while (pq.sz) {
                auto [du, u] = pq.pop();
                if (du != d[u]) continue;
                int r = u / m, c = u % m;
                if (c + 1 < m) relax(u, u + 1, H[r][c]);
                if (c > 0) relax(u, u - 1, H[r][c - 1]);
                if (r + 1 < n) relax(u, u + m, V[r][c]);
                if (r > 0) relax(u, u - m, V[r - 1][c]);
            }
        }
        ll D = INF;
        for (int r = 0; r < n; r++) D = min(D, d[r * m + m - 1]);
        const ll target = D + K;

        /* ---- build flow network ---- */
        MCF mcf(N + 2);
        for (int r = 0; r < n; r++) mcf.add(S, r * m, (ll)1e9, 0);
        for (int r = 0; r < n; r++) mcf.add(r * m + m - 1, T, (ll)1e9, -target);

        const int EH = n * (m - 1), EV = (n - 1) * m;
        vector<int> hA(EH), hB(EH), hIAB(EH), hIBA(EH);
        vector<int> vA(EV), vB(EV), vIAB(EV), vIBA(EV);
        auto addEdge = [&](int a, int b, ll w, int &iAB, int &iBA) {
            iAB = (int)mcf.g[a].size();
            mcf.add(a, b, 1, w);
            iBA = (int)mcf.g[b].size();
            mcf.add(b, a, 1, w);
        };
        for (int r = 0; r < n; r++)
            for (int j = 0; j < m - 1; j++) {
                int id = r * (m - 1) + j;
                hA[id] = r * m + j; hB[id] = r * m + j + 1;
                addEdge(hA[id], hB[id], H[r][j], hIAB[id], hIBA[id]);
            }
        for (int r = 0; r + 1 < n; r++)
            for (int j = 0; j < m; j++) {
                int id = r * m + j;
                vA[id] = r * m + j; vB[id] = (r + 1) * m + j;
                addEdge(vA[id], vB[id], V[r][j], vIAB[id], vIBA[id]);
            }

        /* ---- live-arc masks ----
           Every cap is either 0 or positive, and only the arcs on augmented paths
           ever change it, so a 64-bit mask per vertex lets Dijkstra touch exactly
           the arcs with positive residual capacity.  A grid vertex has at most four
           incident grid edges (two entries each) plus one source/sink arc. */
        vector<unsigned long long> live(N + 2, 0);
        vector<char> masked(N + 2, 0);
        for (int v = 0; v <= T; v++) {
            if ((int)mcf.g[v].size() > 64) continue;   // S and T: plain scan
            masked[v] = 1;
            for (int i = 0; i < (int)mcf.g[v].size(); i++)
                if (mcf.g[v][i].cap > 0) live[v] |= 1ULL << i;
        }

        /* ---- successive shortest paths (Johnson potentials) ---- */
        vector<ll> pot(N + 2, INF);
        for (int v = 0; v < N; v++) pot[v] = d[v];
        pot[S] = 0;
        pot[T] = D - target;                 // shortest distance to T initially

        ll totalCost = 0;
        vector<ll> dist(N + 2);
        vector<int> cur(N + 2);
        vector<char> onStk(N + 2);
        // blocking flow on the zero-reduced-cost subgraph (primal-dual phases)
        function<ll(int)> dfs = [&](int u) -> ll {
            auto &gu = mcf.g[u];
            while (cur[u] < (int)gu.size()) {
                int i = cur[u];
                Arc &e = gu[i];
                if (e.cap <= 0 || e.cost + pot[u] - pot[e.to] != 0) { cur[u]++; continue; }
                if (e.to == T) {
                    e.cap -= 1;
                    if (e.cap <= 0 && masked[u]) live[u] &= ~(1ULL << i);
                    mcf.g[T][e.rev].cap += 1;
                    if (masked[T]) live[T] |= 1ULL << e.rev;
                    return 1;
                }
                if (onStk[e.to]) { cur[u]++; continue; }
                onStk[e.to] = 1;
                ll got = dfs(e.to);
                onStk[e.to] = 0;
                if (got > 0) {
                    e.cap -= got;
                    if (e.cap <= 0 && masked[u]) live[u] &= ~(1ULL << i);
                    mcf.g[e.to][e.rev].cap += got;
                    if (masked[e.to]) live[e.to] |= 1ULL << e.rev;
                    return got;
                }
                cur[u]++;
            }
            return 0;
        };
        RadixHeap pq;
        vector<char> settled(N + 2, 0);
        vector<int> ord;                     // vertices settled in this phase
        while (true) {
            fill(dist.begin(), dist.end(), INF);
            ord.clear();
            pq.clear();
            dist[S] = 0; pq.push(0, S);
            while (pq.sz) {
                auto [du, u] = pq.pop();
                if (settled[u] || du != dist[u]) continue;
                settled[u] = 1;
                ord.push_back(u);
                if (u == T) break;           // only what is closer than T matters
                const ll pu = pot[u] + du;
                auto &gu = mcf.g[u];
                if (masked[u]) {
                    unsigned long long msk = live[u];
                    while (msk) {
                        int i = __builtin_ctzll(msk);
                        msk &= msk - 1;
                        const Arc &e = gu[i];
                        ll nd = pu + e.cost - pot[e.to];
                        if (dist[e.to] > nd) {
                            dist[e.to] = nd;
                            pq.push(nd, e.to);
                        }
                    }
                } else {
                    for (int i = 0; i < (int)gu.size(); i++) {
                        const Arc &e = gu[i];
                        if (e.cap <= 0) continue;
                        ll nd = pu + e.cost - pot[e.to];
                        if (dist[e.to] > nd) {
                            dist[e.to] = nd;
                            pq.push(nd, e.to);
                        }
                    }
                }
            }
            if (!settled[T]) break;          // T not reachable
            ll dT = dist[T];
            ll actual = dT + pot[T] - pot[S];
            if (actual >= 0) break;          // no profitable augmenting path left
            // Johnson update.  Vertices that were not settled would all receive the
            // same shift dT, which cancels in every reduced cost, so it is skipped.
            for (int v : ord) { pot[v] += dist[v] - dT; settled[v] = 0; }
            ll unitCost = pot[T] - pot[S];
            // push a blocking flow through zero reduced cost arcs
            fill(cur.begin(), cur.end(), 0);
            fill(onStk.begin(), onStk.end(), 0);
            onStk[S] = 1;
            ll pushed = 0;
            while (ll got = dfs(S)) pushed += got;
            onStk[S] = 0;
            totalCost += unitCost * pushed;
        }
        ll answer = -totalCost;

        /* ---- which cells are endpoints of the packing? ---- */
        vector<char> srcUsed(n, 0), snkUsed(n, 0);
        for (int r = 0; r < n; r++) {
            // arcs S->s were added first, in row order
            if (mcf.g[S][r].cap < (ll)1e9) srcUsed[r] = 1;
            int t = r * m + m - 1;
            for (const Arc &e : mcf.g[t])
                if (e.to == T && e.cap < (ll)1e9) { snkUsed[r] = 1; break; }
        }

        /* ---- difference constraint system on the residual graph ---- */
        const int R = N;                     // virtual root, p(R) = 0
        vector<vector<pair<int, ll>>> cg(N + 1);
        auto addArc = [&](int u, int v, ll c) { cg[u].push_back({v, c}); };

        for (int r = 0; r < n; r++)
            for (int j = 0; j < m - 1; j++) {
                int id = r * (m - 1) + j, a = hA[id], b = hB[id];
                ll w = H[r][j];
                int f = (mcf.g[a][hIAB[id]].cap == 0) ? 1
                      : (mcf.g[b][hIBA[id]].cap == 0) ? 2 : 0;
                if (f == 1) addArc(b, a, -w);          // used a->b : p(a) <= p(b) - w
                else if (f == 2) addArc(a, b, -w);     // used b->a
                else { addArc(a, b, w); addArc(b, a, w); }
            }
        for (int r = 0; r + 1 < n; r++)
            for (int j = 0; j < m; j++) {
                int id = r * m + j, a = vA[id], b = vB[id];
                ll w = V[r][j];
                int f = (mcf.g[a][vIAB[id]].cap == 0) ? 1
                      : (mcf.g[b][vIBA[id]].cap == 0) ? 2 : 0;
                if (f == 1) addArc(b, a, -w);
                else if (f == 2) addArc(a, b, -w);
                else { addArc(a, b, w); addArc(b, a, w); }
            }
        for (int r = 0; r < n; r++) {
            int s = r * m, t = r * m + m - 1;
            if (srcUsed[r]) { addArc(R, s, 0); addArc(s, R, 0); }
            else addArc(R, s, 0);                          // p(s) <= 0
            if (snkUsed[r]) { addArc(R, t, target); addArc(t, R, -target); }
            else addArc(t, R, -target);                    // p(t) >= target
        }

        /* ---- shortest paths from R (SPFA) ---- */
        vector<ll> p(N + 1, INF);
        vector<char> inq(N + 1, 0);
        {
            deque<int> q;
            p[R] = 0; q.push_back(R); inq[R] = 1;
            while (!q.empty()) {
                int u = q.front(); q.pop_front(); inq[u] = 0;
                for (auto [v, c] : cg[u]) {
                    if (p[u] + c < p[v]) {
                        p[v] = p[u] + c;
                        if (!inq[v]) {
                            inq[v] = 1;
                            // SLF heuristic
                            if (!q.empty() && p[v] < p[q.front()]) q.push_front(v);
                            else q.push_back(v);
                        }
                    }
                }
            }
        }

        /* ---- produce new weights ---- */
        for (int r = 0; r < n; r++)
            for (int j = 0; j < m - 1; j++) {
                int id = r * (m - 1) + j;
                ll diff = p[hA[id]] - p[hB[id]];
                if (diff < 0) diff = -diff;
                if (diff > H[r][j]) H[r][j] = diff;
            }
        for (int r = 0; r + 1 < n; r++)
            for (int j = 0; j < m; j++) {
                int id = r * m + j;
                ll diff = p[vA[id]] - p[vB[id]];
                if (diff < 0) diff = -diff;
                if (diff > V[r][j]) V[r][j] = diff;
            }

        /* ---- output ---- */
        writeInt(answer); wc('\n');
        for (int r = 0; r < n; r++) {
            for (int j = 0; j < m - 1; j++) { writeInt(H[r][j]); wc(j + 1 == m - 1 ? '\n' : ' '); }
        }
        for (int r = 0; r + 1 < n; r++) {
            for (int j = 0; j < m; j++) { writeInt(V[r][j]); wc(j + 1 == m ? '\n' : ' '); }
        }
    }
    flushOut();
    return 0;
}

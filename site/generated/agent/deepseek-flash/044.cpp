#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

static inline ull splitmix(ull& x) {
    ull z = (x += 0x9e3779b97f4a7c15ULL);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    return z ^ (z >> 31);
}
static ull rngState = 0x1234567890abcdefULL;

// ---------------------------------------------------------------------------
// Persistent segment tree over compressed values with reference counting.
// Each node keeps the multiplicity count of its range and a Zobrist-style
// hash  sum(cnt[v] * R[v])  so that two ranges can be tested for *identical
// multiplicity vectors*.  Comparison of two multisets (each optionally with
// one extra value appended) walks down looking for the largest value whose
// multiplicity differs; more copies there means lexicographically larger,
// which is exactly the order induced by the score sum 100^(100^x).
// ---------------------------------------------------------------------------
struct SegPool {
    struct N { ull h; int l, r, cnt, rc; };
    vector<N> t;
    int head;
    SegPool() { t.reserve(1 << 16); t.push_back({0, 0, 0, 0, 0x3fffffff}); head = -1; }
    void reset() { t.clear(); t.push_back({0, 0, 0, 0, 0x3fffffff}); head = -1; }
    int alloc() {
        if (head >= 0) { int id = head; head = t[id].l; return id; }
        t.push_back({0, 0, 0, 0, 0}); return (int)t.size() - 1;
    }
    void freeNode(int id) { t[id].l = head; head = id; }
    int add(int root, int lo, int hi, int pos, ull rv) {
        int id = alloc();
        t[id].l = t[root].l; t[id].r = t[root].r;
        t[id].cnt = t[root].cnt + 1; t[id].h = t[root].h + rv; t[id].rc = 1;
        if (hi - lo > 1) {
            int mid = (lo + hi) >> 1;
            if (pos < mid) { int nl = add(t[root].l, lo, mid, pos, rv); t[t[id].r].rc++; t[id].l = nl; }
            else           { int nr = add(t[root].r, mid, hi, pos, rv); t[t[id].l].rc++; t[id].r = nr; }
        }
        return id;
    }
    void dec(int id) {
        if (id == 0) return;
        if (--t[id].rc == 0) { dec(t[id].l); dec(t[id].r); freeNode(id); }
    }
    // compare  X+{ex}  with  Y+{ey}   (ex/ey == -1 means no extra)
    int cmp(int x, int y, int hi, int ex, int ey, const vector<ull>& R) {
        int lo = 0;
        while (hi - lo > 1) {
            int mid = (lo + hi) >> 1;
            int rx = t[x].r, ry = t[y].r;
            int bx = (ex >= mid), by = (ey >= mid);
            if (t[rx].cnt + bx != t[ry].cnt + by ||
                t[rx].h + (bx ? R[ex] : 0) != t[ry].h + (by ? R[ey] : 0)) {
                if (ex != -1 && ex < mid) ex = -1;
                if (ey != -1 && ey < mid) ey = -1;
                x = rx; y = ry; lo = mid;
            } else {
                if (ex >= mid) ex = -1;
                if (ey >= mid) ey = -1;
                x = t[x].l; y = t[y].l; hi = mid;
            }
        }
        return (t[x].cnt + (ex == lo)) - (t[y].cnt + (ey == lo));
    }
    void extract(int id, int lo, int hi, vector<ll>& out, const vector<ll>& vals) {
        if (id == 0 || t[id].cnt == 0) return;
        if (hi - lo == 1) {
            for (int k = 0; k < t[id].cnt; k++) out.push_back(vals[lo]);
            return;
        }
        int mid = (lo + hi) >> 1;
        extract(t[id].l, lo, mid, out, vals);
        extract(t[id].r, mid, hi, out, vals);
    }
};

static SegPool seg;

int main() {
    int T;
    if (scanf("%d", &T) != 1) return 0;
    vector<ll> ans, vals, G, D;
    vector<ull> R;
    vector<int> compV;
    while (T--) {
        int n, m;
        if (scanf("%d %d", &n, &m) != 2) break;
        int N = n + m;
        vector<ll> L(N), C1l(n), C1r(n), C2l(m), C2r(m);
        for (int i = 0; i < n; i++) { ll l, r; if (scanf("%lld %lld", &l, &r) != 2) return 0; C1l[i] = l; C1r[i] = r; L[i] = r - l + 1; }
        for (int j = 0; j < m; j++) { ll l, r; if (scanf("%lld %lld", &l, &r) != 2) return 0; C2l[j] = l; C2r[j] = r; L[n + j] = r - l + 1; }

        // ---- overlap edges (sweep over the sorted intervals) --------------
        vector<ll> g(L.begin(), L.end());
        vector<int> eu, ev; vector<ll> ew;
        vector<vector<int>> inc(N);
        {
            int js = 0;
            for (int i = 0; i < n; i++) {
                while (js < m && C2r[js] < C1l[i]) js++;
                for (int j = js; j < m && C2l[j] <= C1r[i]; j++) {
                    ll lo = max(C1l[i], C2l[j]), hi = min(C1r[i], C2r[j]);
                    if (lo <= hi) {
                        int e = (int)eu.size();
                        eu.push_back(i); ev.push_back(n + j); ew.push_back(hi - lo + 1);
                        inc[i].push_back(e); inc[n + j].push_back(e);
                        g[i] -= (hi - lo + 1); g[n + j] -= (hi - lo + 1);
                    }
                }
            }
        }
        int E = (int)eu.size();

        // ---- absorb g==0 leaves ------------------------------------------
        vector<int> deg(N, 0);
        for (int e = 0; e < E; e++) { deg[eu[e]]++; deg[ev[e]]++; }
        vector<char> aliveEdge(E, 1), aliveV(N, 1);
        vector<int> q;
        for (int i = 0; i < N; i++) if (deg[i] == 1 && g[i] == 0) q.push_back(i);
        while (!q.empty()) {
            int u = q.back(); q.pop_back();
            if (!aliveV[u] || deg[u] != 1 || g[u] != 0) continue;
            int fe = -1;
            for (int e : inc[u]) if (aliveEdge[e]) { fe = e; break; }
            if (fe < 0) continue;
            int v = (eu[fe] == u ? ev[fe] : eu[fe]);
            aliveEdge[fe] = 0; aliveV[u] = 0;
            g[v] += ew[fe]; deg[u]--; deg[v]--;
            if (aliveV[v] && deg[v] == 1 && g[v] == 0) q.push_back(v);
        }

        // ---- walk every remaining path and run the DP --------------------
        vector<char> visV(N, 0);
        for (int s0 = 0; s0 < N; s0++) {
            if (!aliveV[s0] || visV[s0]) continue;
            compV.clear();
            compV.push_back(s0); visV[s0] = 1;
            for (size_t k = 0; k < compV.size(); k++) {
                int u = compV[k];
                for (int e : inc[u]) if (aliveEdge[e]) {
                    int v = (eu[e] == u ? ev[e] : eu[e]);
                    if (!visV[v]) { visV[v] = 1; compV.push_back(v); }
                }
            }
            if (compV.size() == 1) { if (g[compV[0]] > 0) ans.push_back(g[compV[0]]); continue; }
            int start = compV[0];
            for (int u : compV) {
                int d = 0;
                for (int e : inc[u]) if (aliveEdge[e]) d++;
                if (d <= 1) { start = u; break; }
            }
            G.clear(); D.clear();
            int cur = start, prev = -1;
            int lim = (int)compV.size() + 2;   // guards against unexpected cycles
            while ((int)G.size() < lim) {
                G.push_back(g[cur]);
                int nxt = -1; ll w = 0;
                for (int e : inc[cur]) if (aliveEdge[e]) {
                    int v = (eu[e] == cur ? ev[e] : eu[e]);
                    if (v != prev) { nxt = v; w = ew[e]; break; }
                }
                if (nxt < 0) break;
                D.push_back(w);
                prev = cur; cur = nxt;
            }
            int p = (int)G.size();

            vals.clear();
            for (int i = 0; i < p; i++) {
                ll a = (i > 0) ? D[i - 1] : 0;
                ll b = (i + 1 < p) ? D[i] : 0;
                vals.push_back(G[i]);
                vals.push_back(G[i] + a);
                vals.push_back(G[i] + b);
                vals.push_back(G[i] + a + b);
            }
            sort(vals.begin(), vals.end());
            vals.erase(unique(vals.begin(), vals.end()), vals.end());
            int V = (int)vals.size();
            R.resize(V);
            for (int i = 0; i < V; i++) R[i] = splitmix(rngState);
            auto pos = [&](ll x) { return (int)(lower_bound(vals.begin(), vals.end(), x) - vals.begin()); };

            seg.reset();
            int r0 = seg.add(0, 0, V, pos(G[0] + (p > 1 ? D[0] : 0)), R[pos(G[0] + (p > 1 ? D[0] : 0))]);
            int r1 = seg.add(0, 0, V, pos(G[0]), R[pos(G[0])]);
            for (int i = 1; i < p; i++) {
                ll dprev = D[i - 1];
                ll dcur = (i + 1 < p) ? D[i] : 0;
                int va = pos(G[i] + dcur), vb = pos(G[i] + dprev + dcur);
                int vc = pos(G[i]),          vd = pos(G[i] + dprev);
                int nd0 = (seg.cmp(r0, r1, V, va, vb, R) >= 0) ? seg.add(r0, 0, V, va, R[va])
                                                               : seg.add(r1, 0, V, vb, R[vb]);
                int nd1 = (seg.cmp(r0, r1, V, vc, vd, R) >= 0) ? seg.add(r0, 0, V, vc, R[vc])
                                                               : seg.add(r1, 0, V, vd, R[vd]);
                seg.dec(r0); seg.dec(r1);
                r0 = nd0; r1 = nd1;
            }
            int best;
            if (seg.cmp(r0, r1, V, -1, -1, R) >= 0) { best = r0; seg.dec(r1); }
            else                                     { best = r1; seg.dec(r0); }
            seg.extract(best, 0, V, ans, vals);
            seg.dec(best);
        }
        sort(ans.rbegin(), ans.rend());
        printf("%d\n", (int)ans.size());
        for (size_t i = 0; i < ans.size(); i++) printf("%lld%c", ans[i], i + 1 == ans.size() ? '\n' : ' ');
        ans.clear();
    }
    return 0;
}

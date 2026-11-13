// 围栏信标 - Fence Beacons
//
// Answer = max over the closed polygon P of f(p) = min_i dist(p, vertex_i).
// Local maxima of f over P are only at
//   (a) Voronoi vertices of the vertex set that lie inside P  (circumcenters of Delaunay triangles)
//   (b) points of dP where a Voronoi edge crosses a polygon edge
// so we build the Delaunay triangulation of the vertices, evaluate circumradii of
// triangles whose circumcenter is inside P, and walk the Voronoi cells along every
// polygon edge to get the boundary maxima.

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using i128 = __int128_t;
using ld = long double;

/* ================== Delaunay triangulation (Bowyer-Watson, infinite vertex) ================== */
struct Delaunay {
    int n;                    // real points 0..n-1 ; index n == vertex at infinity
    vector<ll> X, Y;          // size n+1
    // chHead = head of the list (in the chTri/chNext pools) of triangles that
    // replaced this one when it was removed: the "history DAG" for point location
    struct Tri { int v[3], nb[3]; int chHead; bool dead; };
    vector<int> chTri, chNext;      // child lists, grouped by parent
    vector<Tri> T;
    vector<int> mark;
    int stampC = 0;

    void init(const vector<ll>& xs, const vector<ll>& ys) {
        n = (int)xs.size();
        X = xs; Y = ys;
        X.push_back(0); Y.push_back(0);      // INF placeholder
        T.clear(); mark.clear(); stampC = 0;
    }
    inline bool isInf(int t) const { return T[t].v[2] == n; }
    inline int ori(int a, int b, int c) const {
        i128 v = (i128)(X[b]-X[a])*(Y[c]-Y[a]) - (i128)(Y[b]-Y[a])*(X[c]-X[a]);
        return (v>0) - (v<0);
    }
    // >0 iff d strictly inside circumcircle of CCW triangle (a,b,c)
    inline int inc(int a, int b, int c, int d) const {
        i128 ax=(i128)X[a]-X[d], ay=(i128)Y[a]-Y[d];
        i128 bx=(i128)X[b]-X[d], by=(i128)Y[b]-Y[d];
        i128 cx=(i128)X[c]-X[d], cy=(i128)Y[c]-Y[d];
        i128 A=ax*ax+ay*ay, B=bx*bx+by*by, C=cx*cx+cy*cy;
        i128 det = ax*(by*C - B*cy) - ay*(bx*C - B*cx) + A*(bx*cy - by*cx);
        return (det>0) - (det<0);
    }
    int newTri(int a0, int a1, int a2) {
        Tri t;
        t.v[0]=a0; t.v[1]=a1; t.v[2]=a2;
        t.nb[0]=t.nb[1]=t.nb[2]=-1;
        t.chHead=-1;
        t.dead=false;
        T.push_back(t); mark.push_back(0);
        return (int)T.size()-1;
    }
    // (u,v,w) is a CCW (spherical) triangle; canonicalise so that INF sits in slot 2
    int makeTri(int u, int v, int w) {
        if (w == n) return newTri(u, v, w);
        if (u == n) return newTri(v, w, u);
        if (v == n) return newTri(w, u, v);
        return newTri(u, v, w);
    }
    bool isBad(int t, int p) const {
        const Tri& tr = T[t];
        if (tr.v[2] == n) {
            int a = tr.v[0], b = tr.v[1];      // outer face lies left of a->b
            int o = ori(a, b, p);
            if (o > 0) return true;
            if (o == 0) {                      // on the line: strictly between a and b?
                ll d = (X[p]-X[a])*(X[p]-X[b]) + (Y[p]-Y[a])*(Y[p]-Y[b]);
                return d < 0;
            }
            return false;
        }
        return inc(tr.v[0], tr.v[1], tr.v[2], p) > 0;
    }
    // is p inside the (closed) region of triangle c ?
    inline bool inRegion(int c, int p) const {
        const Tri& tr = T[c];
        int a = tr.v[0], b = tr.v[1], d = tr.v[2];
        if (d == n) {
            int o = ori(a, b, p);
            if (o > 0) return true;
            if (o < 0) return false;
            return (X[p]-X[a])*(X[p]-X[b]) + (Y[p]-Y[a])*(Y[p]-Y[b]) < 0;
        }
        return ori(b, d, p) >= 0 && ori(d, a, p) >= 0 && ori(a, b, p) >= 0;
    }
    // locate the triangle containing p, using the history DAG to stay local
    int locate(int p) {
        int t = 0;
        for (int s = 0; s < 4 && s < (int)T.size(); s++)
            if (inRegion(s, p)) { t = s; break; }
        while (T[t].dead) {
            int found = -1;
            for (int e = T[t].chHead; e >= 0; e = chNext[e])
                if (inRegion(chTri[e], p)) { found = chTri[e]; break; }
            if (found < 0) found = chTri[T[t].chHead];   // dead => non-empty child list
            t = found;
        }
        // straight walk on the current triangulation
        int guard = 0;
        while (true) {
            const Tri& tr = T[t];
            if (tr.v[2] == n) {
                int a = tr.v[0], b = tr.v[1];
                int o = ori(a, b, p);
                if (o > 0) return t;                 // escaped the hull here
                if (o == 0) {                        // exactly on the hull line
                    ll dd = (X[p]-X[a])*(X[p]-X[b]) + (Y[p]-Y[a])*(Y[p]-Y[b]);
                    if (dd < 0) return t;            // on the hull edge itself
                }
                int u = tr.nb[2];
                if (u < 0) return t;
                t = u;
                continue;
            }
            int bd = -1;
            for (int i = 0; i < 3; i++)
                if (ori(tr.v[(i+1)%3], tr.v[(i+2)%3], p) < 0) { bd = i; break; }
            if (bd < 0) return t;
            int u = tr.nb[bd];
            if (u < 0) return t;
            t = u;
            if (++guard > 200000) return t;
        }
    }
    void insertPoint(int p) {
        int t = locate(p);
        ++stampC;
        if ((int)mark.size() < (int)T.size()) mark.resize(T.size(), 0);

        vector<int> q, par;
        q.reserve(8); par.reserve(8);
        q.push_back(t); par.push_back(-1); mark[t] = stampC;
        for (size_t qi = 0; qi < q.size(); ++qi) {
            int u = q[qi];
            for (int i = 0; i < 3; i++) {
                int w = T[u].nb[i];
                if (w < 0 || mark[w] == stampC) continue;
                if (isBad(w, p)) { mark[w] = stampC; q.push_back(w); par.push_back((int)qi); }
            }
        }
        // cavity boundary (edges of bad triangles not shared with another bad triangle)
        vector<array<int,5>> be;              // u, v, bad tri, slot, index in q
        for (size_t qi = 0; qi < q.size(); ++qi) {
            int b = q[qi];
            for (int i = 0; i < 3; i++) {
                int w = T[b].nb[i];
                if (w >= 0 && mark[w] == stampC) continue;
                be.push_back({T[b].v[(i+1)%3], T[b].v[(i+2)%3], b, i, (int)qi});
            }
        }
        int m = (int)be.size();
        vector<int> nt(m);
        for (int e = 0; e < m; e++) nt[e] = makeTri(be[e][0], be[e][1], p);

        // history-DAG children: every removed triangle remembers the new triangles
        // that replaced it (their union covers its region), so later queries can
        // walk straight down to the neighbourhood of the point.
        {
            int stride = (m > 64) ? (m + 63) / 64 : 1;
            for (size_t qi = 0; qi < q.size(); ++qi) {
                int b = q[qi], head = -1;
                for (int e = 0; e < m; e += stride) {
                    chTri.push_back(nt[e]); chNext.push_back(head);
                    head = (int)chTri.size() - 1;
                }
                T[b].chHead = head;
            }
        }

        // link new triangles to the non-bad triangles across the cavity boundary
        for (int e = 0; e < m; e++) {
            int b = be[e][2], slot = be[e][3], u = be[e][0], v = be[e][1];
            int oldn = T[b].nb[slot];
            int j = -1;
            for (int k = 0; k < 3; k++)
                if (T[nt[e]].v[(k+1)%3] == u && T[nt[e]].v[(k+2)%3] == v) { j = k; break; }
            T[nt[e]].nb[j] = oldn;
            if (oldn >= 0) {
                for (int k = 0; k < 3; k++)
                    if (T[oldn].v[(k+1)%3] == v && T[oldn].v[(k+2)%3] == u) { T[oldn].nb[k] = nt[e]; break; }
            }
        }
        // link new triangles among themselves
        if (m > 1) {
            vector<array<int,4>> de;
            de.reserve(3*m);
            for (int e = 0; e < m; e++)
                for (int k = 0; k < 3; k++)
                    de.push_back({T[nt[e]].v[(k+1)%3], T[nt[e]].v[(k+2)%3], nt[e], k});
            sort(de.begin(), de.end(), [](const array<int,4>& A, const array<int,4>& B) {
                if (A[0] != B[0]) return A[0] < B[0];
                return A[1] < B[1];
            });
            for (auto& e : de) {
                int x = e[0], y = e[1];
                if (x > y) continue;             // each pair handled once
                int lo = 0, hi = (int)de.size();
                while (lo < hi) {
                    int mid = (lo + hi) >> 1;
                    if (de[mid][0] < y || (de[mid][0] == y && de[mid][1] < x)) lo = mid + 1;
                    else hi = mid;
                }
                if (lo < (int)de.size() && de[lo][0] == y && de[lo][1] == x) {
                    T[e[2]].nb[e[3]] = de[lo][2];
                    T[de[lo][2]].nb[de[lo][3]] = e[2];
                }
            }
        }
        for (int b : q) T[b].dead = true;
    }
    void build(mt19937_64& rng) {
        // three non-collinear seeds
        int i0 = 0;
        for (int i = 1; i < n; i++)
            if (X[i] < X[i0] || (X[i] == X[i0] && Y[i] < Y[i0])) i0 = i;
        int i1 = -1; ld best = -1;
        for (int i = 0; i < n; i++) if (i != i0) {
            ld dx = (ld)X[i]-X[i0], dy = (ld)Y[i]-Y[i0];
            ld d = dx*dx + dy*dy;
            if (d > best) { best = d; i1 = i; }
        }
        int i2 = -1; ld bestc = -1;
        for (int i = 0; i < n; i++) if (i != i0 && i != i1) {
            i128 c = (i128)(X[i1]-X[i0])*(Y[i]-Y[i0]) - (i128)(Y[i1]-Y[i0])*(X[i]-X[i0]);
            ld v = (ld)(c < 0 ? -c : c);
            if (v > bestc) { bestc = v; i2 = i; }
        }
        if (i2 < 0 || bestc == 0) { /* degenerate, cannot happen for a valid polygon */ }
        if (ori(i0, i1, i2) < 0) swap(i1, i2);

        vector<char> used(n, 0);
        used[i0] = used[i1] = used[i2] = 1;

        int t0 = newTri(i0, i1, i2);
        int t1 = makeTri(i1, i0, n);
        int t2 = makeTri(i2, i1, n);
        int t3 = makeTri(i0, i2, n);
        T[t0].nb[2] = t1; T[t1].nb[2] = t0;
        T[t0].nb[0] = t2; T[t2].nb[2] = t0;
        T[t0].nb[1] = t3; T[t3].nb[2] = t0;
        T[t1].nb[0] = t3; T[t3].nb[1] = t1;
        T[t1].nb[1] = t2; T[t2].nb[0] = t1;
        T[t2].nb[1] = t3; T[t3].nb[0] = t2;

        vector<int> rest;
        rest.reserve(n-3);
        for (int i = 0; i < n; i++) if (!used[i]) rest.push_back(i);
        shuffle(rest.begin(), rest.end(), rng);
        for (int p : rest) insertPoint(p);
    }
};

/* ================== batch point-in-polygon (horizontal ray casting) ================== */
struct PointInPoly {
    int n;
    const vector<ll>* X;
    const vector<ll>* Y;
    vector<ll> ys;                       // sorted unique y of vertices
    int m;                               // number of distinct y
    int ecnt;                            // number of elementary intervals = m-1
    vector<vector<int>> nodeE;           // edges stored per node
    vector<int> nl, nr;

    void init(const vector<ll>& xs, const vector<ll>& ys_, const vector<array<int,2>>& edges) {
        n = (int)xs.size(); X = &xs; Y = &ys_;
        ys = ys_;
        sort(ys.begin(), ys.end());
        ys.erase(unique(ys.begin(), ys.end()), ys.end());
        m = (int)ys.size();
        ecnt = max(0, m-1);
        if (ecnt == 0) return;
        nodeE.assign(4*ecnt, {});
        nl.assign(4*ecnt, 0); nr.assign(4*ecnt, 0);
        build(1, 0, ecnt-1);
        for (size_t e = 0; e < edges.size(); e++) {
            int a = edges[e][0], b = edges[e][1];
            ll lo = min((*Y)[a], (*Y)[b]), hi = max((*Y)[a], (*Y)[b]);
            if (lo == hi) continue;                      // horizontal: never active
            int i0 = int(lower_bound(ys.begin(), ys.end(), lo) - ys.begin());
            int i1 = int(lower_bound(ys.begin(), ys.end(), hi) - ys.begin()) - 1;
            if (i0 > i1) continue;
            insert(1, 0, ecnt-1, i0, i1, (int)e);
        }
        for (int node = 1; node < 4*ecnt; node++) {
            if (nodeE[node].empty()) continue;
            ld midy = ((ld)ys[nl[node]] + (ld)ys[nr[node]+1]) * 0.5L;
            sort(nodeE[node].begin(), nodeE[node].end(), [&](int e1, int e2) {
                return xAt(*X, *Y, edges[e1][0], edges[e1][1], midy) <
                       xAt(*X, *Y, edges[e2][0], edges[e2][1], midy);
            });
        }
    }
    static inline ld xAt(const vector<ll>& X, const vector<ll>& Y, int a, int b, ld yy) {
        // x-coordinate of segment (a,b) at height yy  (Ya != Yb)
        ld xa = (ld)X[a], ya = (ld)Y[a], xb = (ld)X[b], yb = (ld)Y[b];
        return xa + (yy - ya) * (xb - xa) / (yb - ya);
    }
    void build(int node, int l, int r) { nl[node] = l; nr[node] = r; if (l == r) return;
        int mid = (l+r)>>1; build(node*2, l, mid); build(node*2+1, mid+1, r); }
    void insert(int node, int l, int r, int ql, int qr, int e) {
        if (ql <= l && r <= qr) { nodeE[node].push_back(e); return; }
        int mid = (l+r)>>1;
        if (ql <= mid) insert(node*2, l, mid, ql, qr, e);
        if (qr > mid) insert(node*2+1, mid+1, r, ql, qr, e);
    }
    // even-odd rule, ray towards +x
    bool inside(ld x0, ld y0, const vector<array<int,2>>& edges) const {
        if (ecnt == 0) return false;
        if (y0 < (ld)ys[0] || y0 >= (ld)ys[m-1]) return false;
        int k = int(upper_bound(ys.begin(), ys.end(), (ll)floorl(y0)) - ys.begin()) - 1;
        if (y0 < (ld)ys[0]) return false;
        if (k < 0) return false;
        if (k > ecnt-1) k = ecnt-1;
        int cnt = 0;
        int node = 1, l = 0, r = ecnt-1;
        while (true) {
            const auto& vec = nodeE[node];
            if (!vec.empty()) {
                int lo = 0, hi = (int)vec.size();
                while (lo < hi) {
                    int mid = (lo+hi)>>1;
                    int e = vec[mid];
                    ld xx = xAt(*X, *Y, edges[e][0], edges[e][1], y0);
                    if (xx <= x0) lo = mid+1; else hi = mid;
                }
                cnt += (int)vec.size() - lo;
            }
            if (l == r) break;
            int mid = (l+r)>>1;
            if (k <= mid) { node = node*2; r = mid; }
            else { node = node*2+1; l = mid+1; }
        }
        return (cnt & 1) != 0;
    }
};

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    vector<ll> X(n), Y(n);
    for (int i = 0; i < n; i++) scanf("%lld %lld", &X[i], &Y[i]);

    mt19937_64 rng(0x9e3779b97f4a7c15ULL);

    Delaunay dt;
    dt.init(X, Y);
    dt.build(rng);

    ld ans = 0;

    // ---- interior candidates: circumcentres of Delaunay triangles inside the polygon ----
    vector<array<int,2>> pedges(n);
    for (int i = 0; i < n; i++) pedges[i] = {i, (i+1)%n};

    ll minx = X[0], maxx = X[0], miny = Y[0], maxy = Y[0];
    for (int i = 1; i < n; i++) {
        minx = min(minx, X[i]); maxx = max(maxx, X[i]);
        miny = min(miny, Y[i]); maxy = max(maxy, Y[i]);
    }

    vector<array<ld,3>> cands;   // cx, cy, r
    for (auto& tr : dt.T) {
        if (tr.dead) continue;
        if (tr.v[2] == n) continue;
        int a = tr.v[0], b = tr.v[1], c = tr.v[2];
        ld ax = (ld)X[a], ay = (ld)Y[a];
        ld bx = (ld)X[b], by = (ld)Y[b];
        ld cx = (ld)X[c], cy = (ld)Y[c];
        ld d = 2.0L*(ax*(by-cy) + bx*(cy-ay) + cx*(ay-by));
        if (d == 0) continue;
        ld a2 = ax*ax + ay*ay, b2 = bx*bx + by*by, c2 = cx*cx + cy*cy;
        ld ux = (a2*(by-cy) + b2*(cy-ay) + c2*(ay-by)) / d;
        ld uy = (a2*(cx-bx) + b2*(ax-cx) + c2*(bx-ax)) / d;
        if (!(ux >= (ld)minx && ux <= (ld)maxx && uy >= (ld)miny && uy <= (ld)maxy)) continue;
        ld r = sqrtl((ux-ax)*(ux-ax) + (uy-ay)*(uy-ay));
        cands.push_back({ux, uy, r});
    }

    if (!cands.empty()) {
        PointInPoly pip;
        pip.init(X, Y, pedges);
        for (auto& c : cands)
            if (pip.inside(c[0], c[1], pedges)) ans = max(ans, c[2]);
    }

    // ---- Delaunay adjacency (for the Voronoi walk) ----
    vector<vector<int>> adj(n);
    {
        vector<pair<int,int>> ee;
        ee.reserve(3*dt.T.size());
        for (auto& tr : dt.T) {
            if (tr.dead || tr.v[2] == n) continue;
            for (int i = 0; i < 3; i++) {
                int a = tr.v[i], b = tr.v[(i+1)%3];
                ee.push_back(minmax(a,b));
            }
        }
        sort(ee.begin(), ee.end());
        ee.erase(unique(ee.begin(), ee.end()), ee.end());
        for (auto& e : ee) { adj[e.first].push_back(e.second); adj[e.second].push_back(e.first); }
    }

    // ---- boundary walk ----
    auto dist2 = [&](ld px, ld py, int i) {
        ld dx = px - (ld)X[i], dy = py - (ld)Y[i];
        return dx*dx + dy*dy;
    };
    // greedy descent in the Delaunay graph -> nearest site
    auto nearestSite = [&](ld px, ld py, int c) {
        ld bd = dist2(px, py, c);
        for (int guard = 0; guard < 200; guard++) {
            int nb = -1; ld nbv = bd;
            for (int d : adj[c]) { ld v = dist2(px, py, d); if (v < nbv) { nbv = v; nb = d; } }
            if (nb < 0) break;
            c = nb; bd = nbv;
        }
        return c;
    };

    // ---- boundary: exact branch & bound on every polygon edge ----
    // For p on an edge, f(p) = min_i |p - v_i| is 1-Lipschitz, so knowing f at the
    // midpoint of a sub-segment bounds f on it by f(mid) + halfLength.  Halving
    // until the bound cannot beat the current answer yields the exact maximum on
    // the edge without enumerating every Voronoi cell it crosses.
    vector<ld> halfLen(n);
    vector<int> ord(n);
    for (int i = 0; i < n; i++) {
        ld dx = (ld)X[(i+1)%n] - (ld)X[i], dy = (ld)Y[(i+1)%n] - (ld)Y[i];
        halfLen[i] = 0.5L*sqrtl(dx*dx + dy*dy);
        ord[i] = i;
    }
    sort(ord.begin(), ord.end(), [&](int i, int j) { return halfLen[i] > halfLen[j]; });

    vector<array<ld,4>> stk;               // t1, t2, f(t1), f(t2)
    stk.reserve(64);
    int curSite = 0;
    for (int ei : ord) {
        if (halfLen[ei] <= ans) break;     // no segment of this edge can beat ans
        int a = ei, b = (ei+1)%n;
        ld Ax = (ld)X[a], Ay = (ld)Y[a];
        ld Ex = (ld)X[b] - Ax, Ey = (ld)Y[b] - Ay;
        ld L = 2.0L*halfLen[ei];
        curSite = a;                        // f is 0 at both endpoints
        stk.clear();
        stk.push_back({0, 1, 0, 0});
        while (!stk.empty()) {
            array<ld,4> sg = stk.back(); stk.pop_back();
            ld t1 = sg[0], t2 = sg[1], f1 = sg[2], f2 = sg[3];
            ld tm = 0.5L*(t1 + t2);
            if (!(tm > t1 && tm < t2)) continue;   // interval no longer representable
            ld px = Ax + tm*Ex, py = Ay + tm*Ey;
            curSite = nearestSite(px, py, curSite);
            ld fm = sqrtl(dist2(px, py, curSite));
            if (fm > ans) ans = fm;
            ld q = (t2 - t1)*L*0.5L;       // spatial length of each half
            if (min(f1, fm) + q > ans) stk.push_back({t1, tm, f1, fm});
            if (min(fm, f2) + q > ans) stk.push_back({tm, t2, fm, f2});
        }
    }

    printf("%.12Lf\n", ans);
    return 0;
}

// 信标扫描
//
// For a fixed landing point Q the cheapest annulus scans exactly the distances
// [dmin(Q), dmax(Q)]  (r = dmin, R = dmax), so its area is pi*(dmax^2-dmin^2).
// Answer = pi/Area * ( Integral_rect dmax^2  -  Integral_rect dmin^2 ).
//
// Integral of dmin^2  : nearest-point Voronoi diagram (= dual of Delaunay).
// Integral of dmax^2  : farthest-point Voronoi diagram (= dual of the
//                       "farthest" Delaunay triangulation of the hull points).
//
// Both diagrams are obtained from a triangulation: the Voronoi cell of a site
// is the intersection of the half planes bounded by the perpendicular
// bisectors of its Delaunay neighbours.  We never build the cells explicitly;
// we clip the (convex) query rectangle against those half planes and
// integrate x^2+y^2 over the resulting convex polygon.

#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef long double ld;
typedef __int128 i128;

struct Pt { ll x, y; };

static inline i128 crossP(Pt o, Pt a, Pt b) {
    return (i128)(a.x - o.x) * (b.y - o.y) - (i128)(a.y - o.y) * (b.x - o.x);
}

// > 0 : p strictly inside circumcircle of the CCW triangle (a,b,c)
// < 0 : strictly outside, 0 : cocircular
static inline int incircleSign(Pt a, Pt b, Pt c, Pt p) {
    i128 ax = a.x - p.x, ay = a.y - p.y;
    i128 bx = b.x - p.x, by = b.y - p.y;
    i128 cx = c.x - p.x, cy = c.y - p.y;
    i128 det = (ax * ax + ay * ay) * (bx * cy - by * cx)
             - (bx * bx + by * by) * (ax * cy - ay * cx)
             + (cx * cx + cy * cy) * (ax * by - ay * bx);
    return (det > 0) - (det < 0);
}

// ---------------------------------------------------------------------------
//  Triangulation with adjacency.  nb[k] is the triangle across the directed
//  edge (v[k], v[k+1]) ;  -1 means "hull boundary".
// ---------------------------------------------------------------------------
struct Tri { int v[3]; int nb[3]; };

static vector<Pt> P;                       // all points

struct Delaunay {
    vector<Tri> TR;
    vector<char> dead;
    vector<int> redirect;

    int newTri(int a, int b, int c) {
        TR.push_back(Tri{{a, b, c}, {-1, -1, -1}});
        dead.push_back(0); redirect.push_back(-1);
        return (int)TR.size() - 1;
    }
    int findEdge(int t, int a, int b) const {   // index of directed edge (a,b)
        for (int k = 0; k < 3; k++)
            if (TR[t].v[k] == a && TR[t].v[(k + 1) % 3] == b) return k;
        return -1;
    }
    int resolve(int t) const {
        while (t >= 0 && dead[t]) t = redirect[t];
        return t;
    }

    // fan triangulation of the convex hull (hv[0..h-1] CCW, h >= 3)
    void buildFan(const vector<int>& hv) {
        int h = (int)hv.size();
        vector<int> ts;
        ts.reserve(h - 2);
        for (int k = 1; k <= h - 2; k++)
            ts.push_back(newTri(hv[0], hv[k], hv[k + 1]));
        for (int i = 1; i < (int)ts.size(); i++) {
            TR[ts[i]].nb[0]   = ts[i - 1];
            TR[ts[i - 1]].nb[2] = ts[i];
        }
    }

    void flip(int t, int k) {
        int nt = TR[t].nb[k];
        int a = TR[t].v[k], b = TR[t].v[(k + 1) % 3], c = TR[t].v[(k + 2) % 3];
        int j = findEdge(nt, b, a);
        int d = TR[nt].v[(j + 2) % 3];
        int ob = TR[t].nb[(k + 2) % 3];    // across (c,a)
        int oa = TR[t].nb[(k + 1) % 3];    // across (b,c)
        int n1 = TR[nt].nb[(j + 1) % 3];   // across (a,d)
        int n2 = TR[nt].nb[(j + 2) % 3];   // across (d,b)
        TR[t].v[0] = c; TR[t].v[1] = a; TR[t].v[2] = d;
        TR[nt].v[0] = c; TR[nt].v[1] = d; TR[nt].v[2] = b;
        TR[t].nb[0] = ob; TR[t].nb[1] = n1; TR[t].nb[2] = nt;
        TR[nt].nb[0] = t; TR[nt].nb[1] = n2; TR[nt].nb[2] = oa;
        if (ob >= 0) { int e = findEdge(ob, a, c); if (e >= 0) TR[ob].nb[e] = t; }
        if (n1 >= 0) { int e = findEdge(n1, d, a); if (e >= 0) TR[n1].nb[e] = t; }
        if (n2 >= 0) { int e = findEdge(n2, b, d); if (e >= 0) TR[n2].nb[e] = nt; }
        if (oa >= 0) { int e = findEdge(oa, c, b); if (e >= 0) TR[oa].nb[e] = nt; }
    }

    // sgn = +1 : nearest Delaunay (empty circumcircle)
    // sgn = -1 : farthest Delaunay (circumcircle contains every point)
    void runStack(vector<pair<int,int>>& st, int sgn) {
        while (!st.empty()) {
            int tt = st.back().first, kk = st.back().second;
            st.pop_back();
            int nt = TR[tt].nb[kk];
            if (nt < 0) continue;
            int a = TR[tt].v[kk], b = TR[tt].v[(kk + 1) % 3], c = TR[tt].v[(kk + 2) % 3];
            int j = findEdge(nt, b, a);
            if (j < 0) continue;
            int d = TR[nt].v[(j + 2) % 3];
            if (incircleSign(P[a], P[b], P[c], P[d]) * sgn > 0) {
                flip(tt, kk);
                st.push_back({tt, 0}); st.push_back({tt, 1});
                st.push_back({nt, 1}); st.push_back({nt, 2});
            }
        }
    }

    void legalizeAll(int sgn) {
        vector<pair<int,int>> st;
        for (int t = 0; t < (int)TR.size(); t++) {
            if (dead[t]) continue;
            for (int k = 0; k < 3; k++)
                if (TR[t].nb[k] >= 0) st.push_back({t, k});
        }
        runStack(st, sgn);
    }

    int locate(int p, int hint) const {
        int t = resolve(hint);
        if (t < 0) t = 0;
        long long guard = 0;
        while (true) {
            int moved = -1;
            for (int k = 0; k < 3; k++) {
                if (crossP(P[TR[t].v[k]], P[TR[t].v[(k + 1) % 3]], P[p]) < 0) {
                    moved = k; break;
                }
            }
            if (moved < 0) return t;
            t = resolve(TR[t].nb[moved]);
            if (t < 0) return 0;                 // should not happen
            if (++guard > 20000000LL) return 0;  // safety net
        }
    }

    // p strictly inside triangle t
    int split3(int t, int p, vector<pair<int,int>>& st) {
        int a = TR[t].v[0], b = TR[t].v[1], c = TR[t].v[2];
        int n0 = TR[t].nb[0], n1 = TR[t].nb[1], n2 = TR[t].nb[2];
        int t1 = newTri(a, b, p), t2 = newTri(b, c, p), t3 = newTri(c, a, p);
        TR[t1].nb[0] = n0; TR[t1].nb[1] = t2; TR[t1].nb[2] = t3;
        TR[t2].nb[0] = n1; TR[t2].nb[1] = t3; TR[t2].nb[2] = t1;
        TR[t3].nb[0] = n2; TR[t3].nb[1] = t1; TR[t3].nb[2] = t2;
        if (n0 >= 0) { int e = findEdge(n0, b, a); if (e >= 0) TR[n0].nb[e] = t1; }
        if (n1 >= 0) { int e = findEdge(n1, c, b); if (e >= 0) TR[n1].nb[e] = t2; }
        if (n2 >= 0) { int e = findEdge(n2, a, c); if (e >= 0) TR[n2].nb[e] = t3; }
        dead[t] = 1; redirect[t] = t1;
        st.push_back({t1, 0}); st.push_back({t2, 0}); st.push_back({t3, 0});
        return t1;
    }

    // p lies on the (interior) edge k of t -> 4 new triangles
    int split4(int t, int k, int p, vector<pair<int,int>>& st) {
        int nt = TR[t].nb[k];
        int a = TR[t].v[k], b = TR[t].v[(k + 1) % 3], c = TR[t].v[(k + 2) % 3];
        int j = findEdge(nt, b, a);
        int d = TR[nt].v[(j + 2) % 3];
        int X = TR[t].nb[(k + 1) % 3];   // across (b,c)
        int Y = TR[t].nb[(k + 2) % 3];   // across (c,a)
        int U = TR[nt].nb[(j + 1) % 3];  // across (a,d)
        int V = TR[nt].nb[(j + 2) % 3];  // across (d,b)
        int T1 = newTri(a, p, c), T2 = newTri(p, b, c);
        int T3 = newTri(b, p, d), T4 = newTri(p, a, d);
        TR[T1].nb[0] = T4; TR[T1].nb[1] = T2; TR[T1].nb[2] = Y;
        TR[T2].nb[0] = T3; TR[T2].nb[1] = X; TR[T2].nb[2] = T1;
        TR[T3].nb[0] = T2; TR[T3].nb[1] = T4; TR[T3].nb[2] = V;
        TR[T4].nb[0] = T1; TR[T4].nb[1] = U; TR[T4].nb[2] = T3;
        if (X >= 0) { int e = findEdge(X, c, b); if (e >= 0) TR[X].nb[e] = T2; }
        if (Y >= 0) { int e = findEdge(Y, a, c); if (e >= 0) TR[Y].nb[e] = T1; }
        if (U >= 0) { int e = findEdge(U, d, a); if (e >= 0) TR[U].nb[e] = T4; }
        if (V >= 0) { int e = findEdge(V, b, d); if (e >= 0) TR[V].nb[e] = T3; }
        dead[t] = 1; redirect[t] = T1;
        dead[nt] = 1; redirect[nt] = T2;
        st.push_back({T1, 2}); st.push_back({T2, 1});
        st.push_back({T3, 2}); st.push_back({T4, 1});
        return T1;
    }

    // p lies on a hull edge k of t -> 2 new triangles
    int split2(int t, int k, int p, vector<pair<int,int>>& st) {
        int a = TR[t].v[k], b = TR[t].v[(k + 1) % 3], c = TR[t].v[(k + 2) % 3];
        int X = TR[t].nb[(k + 1) % 3];   // across (b,c)
        int Y = TR[t].nb[(k + 2) % 3];   // across (c,a)
        int T1 = newTri(a, p, c), T2 = newTri(p, b, c);
        TR[T1].nb[0] = -1; TR[T1].nb[1] = T2; TR[T1].nb[2] = Y;
        TR[T2].nb[0] = -1; TR[T2].nb[1] = X; TR[T2].nb[2] = T1;
        if (X >= 0) { int e = findEdge(X, c, b); if (e >= 0) TR[X].nb[e] = T2; }
        if (Y >= 0) { int e = findEdge(Y, a, c); if (e >= 0) TR[Y].nb[e] = T1; }
        dead[t] = 1; redirect[t] = T1;
        st.push_back({T1, 2}); st.push_back({T2, 1});
        return T1;
    }

    int insertPoint(int p, int hint) {
        vector<pair<int,int>> st;
        int t = locate(p, hint);
        int onEdge = -1;
        for (int q = 0; q < 3; q++)
            if (crossP(P[TR[t].v[q]], P[TR[t].v[(q + 1) % 3]], P[p]) == 0) { onEdge = q; break; }
        int res;
        if (onEdge < 0)                          res = split3(t, p, st);
        else if (TR[t].nb[onEdge] >= 0)          res = split4(t, onEdge, p, st);
        else                                     res = split2(t, onEdge, p, st);
        runStack(st, +1);
        return res;
    }

    vector<pair<int,int>> edges() const {
        vector<pair<int,int>> E;
        for (int t = 0; t < (int)TR.size(); t++) {
            if (dead[t]) continue;
            for (int k = 0; k < 3; k++) {
                int a = TR[t].v[k], b = TR[t].v[(k + 1) % 3];
                if (a > b) swap(a, b);
                E.push_back({a, b});
            }
        }
        sort(E.begin(), E.end());
        E.erase(unique(E.begin(), E.end()), E.end());
        return E;
    }
};

// ---------------------------------------------------------------------------
//  Integration of |Q - s|^2 over a convex polygon (CCW, long double coords)
// ---------------------------------------------------------------------------
struct FPt { ld x, y; };

static ld triQuadInt(FPt A, FPt B, FPt C, ld sx, ld sy) {
    ld ux = B.x - A.x, uy = B.y - A.y;
    ld vx = C.x - A.x, vy = C.y - A.y;
    ld area2 = ux * vy - uy * vx;
    if (area2 <= 0) return 0;                 // degenerate / wrong orientation
    ld area = area2 * 0.5L;
    ld Ax = A.x, Ay = A.y;
    // trace of E[Q Q^T] for Q uniform in the triangle
    ld tr = Ax * Ax + Ay * Ay
          + (2.0L / 3.0L) * (Ax * ux + Ay * uy + Ax * vx + Ay * vy)
          + (1.0L / 6.0L) * (ux * ux + uy * uy + vx * vx + vy * vy)
          + (1.0L / 6.0L) * (ux * vx + uy * vy);
    ld ex = (A.x + B.x + C.x) / 3.0L;
    ld ey = (A.y + B.y + C.y) / 3.0L;
    ld val = tr - 2.0L * (sx * ex + sy * ey) + (sx * sx + sy * sy);
    return area * val;
}

static ld polyQuadInt(const vector<FPt>& poly, ld sx, ld sy) {
    int m = (int)poly.size();
    if (m < 3) return 0;
    ld s = 0;
    for (int i = 1; i + 1 < m; i++) s += triQuadInt(poly[0], poly[i], poly[i + 1], sx, sy);
    return s;
}

// keep A*x + B*y <= C
static void clipHalf(vector<FPt>& poly, ld A, ld B, ld C) {
    if (poly.empty()) return;
    vector<FPt> out;
    int m = (int)poly.size();
    out.reserve(m + 2);
    for (int i = 0; i < m; i++) {
        FPt p1 = poly[i], p2 = poly[(i + 1) % m];
        ld v1 = A * p1.x + B * p1.y - C;
        ld v2 = A * p2.x + B * p2.y - C;
        if (v1 <= 0) out.push_back(p1);
        if ((v1 < 0 && v2 > 0) || (v1 > 0 && v2 < 0)) {
            ld t = v1 / (v1 - v2);
            out.push_back({p1.x + t * (p2.x - p1.x), p1.y + t * (p2.y - p1.y)});
        }
    }
    poly.swap(out);
}

static ll xl_, yl_, xr_, yr_;

// integral over the rectangle of the cell of site s, given its neighbours
static ld cellIntegral(int s, const vector<int>& nbr, bool farthest) {
    vector<FPt> poly = {{(ld)xl_, (ld)yl_}, {(ld)xr_, (ld)yl_},
                        {(ld)xr_, (ld)yr_}, {(ld)xl_, (ld)yr_}};
    ld sx = (ld)P[s].x, sy = (ld)P[s].y;
    ll s2 = P[s].x * P[s].x + P[s].y * P[s].y;
    for (int j : nbr) {
        ll A = 2 * (P[j].x - P[s].x);
        ll B = 2 * (P[j].y - P[s].y);
        ll C = (P[j].x * P[j].x + P[j].y * P[j].y) - s2;
        if (!farthest) clipHalf(poly, (ld)A, (ld)B, (ld)C);
        else           clipHalf(poly, (ld)(-A), (ld)(-B), (ld)(-C));
        if ((int)poly.size() < 3) return 0;
    }
    return polyQuadInt(poly, sx, sy);
}

int main() {
    scanf("%lld %lld %lld %lld", &xl_, &yl_, &xr_, &yr_);
    int n;
    scanf("%d", &n);
    P.resize(n);
    for (int i = 0; i < n; i++) scanf("%lld %lld", &P[i].x, &P[i].y);

    // ---------------- convex hull (CCW, no collinear points) ----------------
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [](int a, int b) {
        if (P[a].x != P[b].x) return P[a].x < P[b].x;
        return P[a].y < P[b].y;
    });
    vector<int> H;
    H.reserve(2 * n);
    for (int i = 0; i < n; i++) {
        while (H.size() >= 2 && crossP(P[H[H.size() - 2]], P[H[H.size() - 1]], P[ord[i]]) <= 0)
            H.pop_back();
        H.push_back(ord[i]);
    }
    int lower = (int)H.size();
    for (int i = n - 2; i >= 0; i--) {
        while ((int)H.size() > lower &&
               crossP(P[H[H.size() - 2]], P[H[H.size() - 1]], P[ord[i]]) <= 0)
            H.pop_back();
        H.push_back(ord[i]);
    }
    H.pop_back();                          // last == first
    vector<int> hull = H;                  // CCW
    int h = (int)hull.size();

    ld Imin = 0, Imax = 0;

    if (h == 2) {
        // ---------------- all points are collinear ----------------
        Pt d{P[hull[1]].x - P[hull[0]].x, P[hull[1]].y - P[hull[0]].y};
        vector<int> s2(n);
        for (int i = 0; i < n; i++) s2[i] = i;
        sort(s2.begin(), s2.end(), [&](int a, int b) {
            ll pa = (P[a].x - P[hull[0]].x) * d.x + (P[a].y - P[hull[0]].y) * d.y;
            ll pb = (P[b].x - P[hull[0]].x) * d.x + (P[b].y - P[hull[0]].y) * d.y;
            return pa < pb;
        });
        for (int i = 0; i < n; i++) {
            vector<int> nb;
            if (i > 0) nb.push_back(s2[i - 1]);
            if (i + 1 < n) nb.push_back(s2[i + 1]);
            Imin += cellIntegral(s2[i], nb, false);
        }
        // farthest: only the two extreme points matter
        for (int e = 0; e < 2; e++) {
            int s = hull[e], o = hull[1 - e];
            vector<int> nb{o};
            Imax += cellIntegral(s, nb, true);
        }
    } else {
        // ---------------- nearest diagram: full Delaunay ----------------
        Delaunay D;
        D.buildFan(hull);
        D.legalizeAll(+1);

        // grid of location hints
        ll mnx = P[0].x, mxx = P[0].x, mny = P[0].y, mxy = P[0].y;
        for (int i = 1; i < n; i++) {
            mnx = min(mnx, P[i].x); mxx = max(mxx, P[i].x);
            mny = min(mny, P[i].y); mxy = max(mxy, P[i].y);
        }
        const int G = 128;
        ll wSpan = mxx - mnx + 1, hSpan = mxy - mny + 1;
        vector<int> grid((size_t)G * G, -1);
        auto cellOf = [&](int p) -> size_t {
            int cx = (int)((P[p].x - mnx) * (ll)G / wSpan);
            int cy = (int)((P[p].y - mny) * (ll)G / hSpan);
            cx = max(0, min(G - 1, cx)); cy = max(0, min(G - 1, cy));
            return (size_t)cy * G + cx;
        };

        vector<char> onHull(n, 0);
        for (int v : hull) onHull[v] = 1;
        vector<int> ord;
        for (int i = 0; i < n; i++) if (!onHull[i]) ord.push_back(i);
        { mt19937 rng(1234567u); shuffle(ord.begin(), ord.end(), rng); }
        int hint = 0;
        for (int i : ord) {
            size_t c = cellOf(i);
            int st = grid[c] >= 0 ? grid[c] : hint;
            int nt = D.insertPoint(i, st);
            grid[c] = nt;
            hint = nt;
            // refresh the cell of the inserted point (it moved)
            grid[cellOf(i)] = nt;
        }
        vector<int> hintUsed; (void)hintUsed;

        vector<pair<int,int>> E = D.edges();
        vector<vector<int>> nbr(n);
        for (auto& e : E) { nbr[e.first].push_back(e.second); nbr[e.second].push_back(e.first); }
        for (int i = 0; i < n; i++) Imin += cellIntegral(i, nbr[i], false);

        // ---------------- farthest diagram: on hull points only ----------------
        Delaunay F;
        F.buildFan(hull);
        F.legalizeAll(-1);
        vector<pair<int,int>> EF = F.edges();
        vector<vector<int>> nbrF(n);
        for (auto& e : EF) { nbrF[e.first].push_back(e.second); nbrF[e.second].push_back(e.first); }
        for (int v : hull) Imax += cellIntegral(v, nbrF[v], true);
    }

    ld area = (ld)(xr_ - xl_) * (ld)(yr_ - yl_);
    ld diff = Imax - Imin;
    if (diff < 0 && diff > -1e-6L * max((ld)1, fabsl(Imax))) diff = 0;
    ld ans = acosl(-1.0L) * diff / area;
    if (ans < 0) ans = 0;
    printf("%.12Lf\n", ans);
    return 0;
}

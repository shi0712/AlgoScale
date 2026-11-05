#include <bits/stdc++.h>
using namespace std;

using ld = long double;

static constexpr ld EPS = 1e-14L;
static constexpr ld PARAM_EPS = 1e-15L;

struct Point {
    ld x = 0, y = 0;
    Point() = default;
    Point(ld x_, ld y_) : x(x_), y(y_) {}
    Point operator + (const Point& o) const { return {x + o.x, y + o.y}; }
    Point operator - (const Point& o) const { return {x - o.x, y - o.y}; }
    Point operator * (ld k) const { return {x * k, y * k}; }
    Point operator / (ld k) const { return {x / k, y / k}; }
};

static inline ld cross(Point a, Point b) { return a.x * b.y - a.y * b.x; }
static inline ld cross(Point a, Point b, Point c) { return cross(b - a, c - a); }
static inline ld dot(Point a, Point b) { return a.x * b.x + a.y * b.y; }
static inline ld norm2(Point a) { return dot(a, a); }
static inline ld geom_tol(Point a, Point b) {
    return EPS * (1.0L + sqrtl(max((ld)0, norm2(a))) * sqrtl(max((ld)0, norm2(b))));
}
static inline bool same_point(Point a, Point b) { return norm2(a - b) <= 1e-26L; }

static ld polygon_area2(const vector<Point>& p) {
    ld s = 0;
    for (int i = 0, n = (int)p.size(); i < n; ++i) s += cross(p[i], p[(i + 1) % n]);
    return s;
}

static vector<Point> clean_convex(vector<Point> p) {
    if (p.empty()) return {};
    vector<Point> q;
    q.reserve(p.size());
    for (auto x : p) {
        if (q.empty() || !same_point(q.back(), x)) q.push_back(x);
    }
    if (q.size() >= 2 && same_point(q.front(), q.back())) q.pop_back();
    if (q.size() < 3) return {};

    // A clipped triangle has O(1) vertices; repeated deletion is constant-time in our model.
    bool changed = true;
    while (changed && q.size() >= 3) {
        changed = false;
        vector<Point> r;
        const int n = (int)q.size();
        r.reserve(n);
        for (int i = 0; i < n; ++i) {
            Point a = q[(i + n - 1) % n], b = q[i], c = q[(i + 1) % n];
            Point u = b - a, v = c - b;
            if (fabsl(cross(u, v)) <= geom_tol(u, v) && dot(u, v) >= -1e-24L) {
                changed = true;
            } else {
                r.push_back(b);
            }
        }
        q.swap(r);
    }
    if (q.size() < 3) return {};
    ld a2 = polygon_area2(q);
    if (fabsl(a2) <= 1e-22L) return {};
    if (a2 < 0) reverse(q.begin(), q.end());
    return q;
}

// Keep cross(dir, x-q) >= 0.
static vector<Point> clip_halfplane(const vector<Point>& poly, Point q, Point dir) {
    if (poly.empty()) return {};
    vector<Point> res;
    res.reserve(poly.size() + 1);
    const int n = (int)poly.size();
    for (int i = 0; i < n; ++i) {
        Point a = poly[i], b = poly[(i + 1) % n];
        ld va = cross(dir, a - q), vb = cross(dir, b - q);
        bool ina = va >= -geom_tol(dir, a - q);
        bool inb = vb >= -geom_tol(dir, b - q);
        if (ina) res.push_back(a);
        if (ina != inb) {
            ld den = va - vb;
            if (fabsl(den) > 1e-30L) {
                ld t = va / den;
                t = max((ld)0, min((ld)1, t));
                res.push_back(a + (b - a) * t);
            }
        }
    }
    return clean_convex(move(res));
}

// Intersection of segment a+t(b-a), 0<=t<=1, with a CCW convex polygon.
static bool segment_clip_convex(Point a, Point b, const vector<Point>& poly, ld& L, ld& R) {
    if (poly.size() < 3) return false;
    L = 0; R = 1;
    const int n = (int)poly.size();
    for (int i = 0; i < n; ++i) {
        Point p = poly[i], q = poly[(i + 1) % n], e = q - p;
        ld v0 = cross(e, a - p), v1 = cross(e, b - p);
        bool in0 = v0 >= -geom_tol(e, a - p);
        bool in1 = v1 >= -geom_tol(e, b - p);
        if (in0 && in1) continue;
        if (!in0 && !in1) return false;
        ld den = v0 - v1;
        if (fabsl(den) <= 1e-30L) return false;
        ld t = v0 / den;
        if (!in0) L = max(L, t);
        else R = min(R, t);
        if (L > R + PARAM_EPS) return false;
    }
    L = max((ld)0, L); R = min((ld)1, R);
    return L <= R + PARAM_EPS;
}

// -1 outside, 0 boundary, 1 strictly inside.
static int relation_convex(Point x, const vector<Point>& p) {
    bool boundary = false;
    for (int i = 0, n = (int)p.size(); i < n; ++i) {
        Point e = p[(i + 1) % n] - p[i], v = x - p[i];
        ld z = cross(e, v), tol = geom_tol(e, v);
        if (z < -tol) return -1;
        if (fabsl(z) <= tol) boundary = true;
    }
    return boundary ? 0 : 1;
}

static bool in_triangle_inclusive(Point p, const array<Point,3>& t) {
    for (int i = 0; i < 3; ++i) {
        Point e = t[(i + 1) % 3] - t[i], v = p - t[i];
        if (cross(e, v) < -geom_tol(e, v)) return false;
    }
    return true;
}

// O(n^2) ear clipping for a simple CCW polygon.
// Standard optimization: only reflex vertices need to be tested inside an ear triangle.
static vector<array<int,3>> triangulate(const vector<Point>& p) {
    const int n = (int)p.size();
    vector<int> prv(n), nxt(n);
    vector<char> alive(n, 1), reflex(n, 0), ear(n, 0);
    for (int i = 0; i < n; ++i) {
        prv[i] = (i + n - 1) % n;
        nxt[i] = (i + 1) % n;
    }

    auto is_reflex = [&](int i) -> bool {
        if (!alive[i]) return false;
        return cross(p[prv[i]], p[i], p[nxt[i]]) <= 0;
    };
    for (int i = 0; i < n; ++i) reflex[i] = is_reflex(i);

    auto point_in_or_on_tri = [&](Point x, Point a, Point b, Point c) -> bool {
        return cross(a,b,x) >= 0 && cross(b,c,x) >= 0 && cross(c,a,x) >= 0;
    };

    auto calc_ear = [&](int i) -> bool {
        if (!alive[i] || reflex[i]) return false;
        int a = prv[i], c = nxt[i];
        if (cross(p[a], p[i], p[c]) <= 0) return false;
        for (int j = 0; j < n; ++j) {
            if (!alive[j] || !reflex[j] || j == a || j == i || j == c) continue;
            if (point_in_or_on_tri(p[j], p[a], p[i], p[c])) return false;
        }
        return true;
    };

    deque<int> q;
    for (int i = 0; i < n; ++i) {
        ear[i] = calc_ear(i);
        if (ear[i]) q.push_back(i);
    }

    vector<array<int,3>> out;
    out.reserve(n - 2);
    int remain = n;

    auto refresh = [&](int i) {
        if (!alive[i]) return;
        reflex[i] = is_reflex(i);
        ear[i] = calc_ear(i);
        if (ear[i]) q.push_back(i);
    };

    while (remain > 3) {
        int b = -1;
        while (!q.empty()) {
            int x = q.front(); q.pop_front();
            if (alive[x] && calc_ear(x)) { b = x; break; }
        }
        if (b == -1) {
            // Every nondegenerate simple polygon has an ear; reaching this point means invalid input
            // or a violated numerical invariant. Original vertices are integral, so the ear tests are exact.
            throw runtime_error("triangulation failed");
        }
        int a = prv[b], c = nxt[b];
        out.push_back({a,b,c});
        alive[b] = 0; --remain;
        nxt[a] = c; prv[c] = a;
        refresh(a); refresh(c);
    }

    int a = -1;
    for (int i = 0; i < n; ++i) if (alive[i]) { a = i; break; }
    int b = nxt[a], c = nxt[b];
    if (cross(p[a],p[b],p[c]) > 0) out.push_back({a,b,c});
    else out.push_back({a,c,b});
    return out;
}

struct AdjEdge { int to, a, b; };

static ld union_area_convex(vector<vector<Point>> polys) {
    vector<vector<Point>> ps;
    ps.reserve(polys.size());
    for (auto &p : polys) {
        auto q = clean_convex(move(p));
        if (q.size() >= 3) ps.push_back(move(q));
    }
    const int s = (int)ps.size();
    if (s == 0) return 0;

    vector<Point> center(s);
    for (int i = 0; i < s; ++i) {
        Point c{0,0};
        for (auto x : ps[i]) c = c + x;
        center[i] = c / (ld)ps[i].size();
    }

    ld twice = 0;
    vector<pair<ld,ld>> covered;
    covered.reserve(s);

    for (int i = 0; i < s; ++i) {
        const int ni = (int)ps[i].size();
        for (int ei = 0; ei < ni; ++ei) {
            Point a = ps[i][ei], b = ps[i][(ei + 1) % ni], d = b - a;
            covered.clear();
            for (int j = 0; j < s; ++j) if (j != i) {
                ld L, R;
                if (!segment_clip_convex(a,b,ps[j],L,R)) continue;
                L = max((ld)0,L); R = min((ld)1,R);
                if (R - L <= PARAM_EPS) continue;

                ld t1 = (2*L + R)/3, t2 = (L + 2*R)/3;
                int r1 = relation_convex(a + d*t1, ps[j]);
                int r2 = relation_convex(a + d*t2, ps[j]);
                bool hide = false;
                if (r1 == 1 || r2 == 1) {
                    hide = true;
                } else {
                    // Positive-length overlap restricted to boundaries: collinear coincident edges.
                    Point mid = a + d*((L+R)/2);
                    Point v = center[j] - mid;
                    ld side = cross(d,v), tol = geom_tol(d,v);
                    if (side < -tol) hide = true;           // opposite orientation: internal seam
                    else if (j < i) hide = true;            // same-side duplicate: keep one copy
                }
                if (hide) covered.push_back({L,R});
            }

            if (covered.empty()) {
                twice += cross(a,b);
                continue;
            }
            sort(covered.begin(), covered.end());
            ld cov = 0, L = covered[0].first, R = covered[0].second;
            for (int k = 1; k < (int)covered.size(); ++k) {
                if (covered[k].first <= R + PARAM_EPS) R = max(R, covered[k].second);
                else { cov += max((ld)0, R-L); L = covered[k].first; R = covered[k].second; }
            }
            cov += max((ld)0, R-L);
            cov = min((ld)1, max((ld)0,cov));
            twice += cross(a,b) * (1-cov);
        }
    }
    ld ans = twice/2;
    if (ans < 0 && ans > -1e-9L) ans = 0;
    return ans;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<Point> poly(n);
    for (auto &p : poly) { long long x,y; cin >> x >> y; p = Point(x,y); }
    if (polygon_area2(poly) < 0) reverse(poly.begin(), poly.end());

    int m; cin >> m;
    vector<Point> cam(m);
    for (auto &p : cam) { long long x,y; cin >> x >> y; p = Point(x,y); }

    auto tri = triangulate(poly);
    const int T = (int)tri.size();
    vector<array<Point,3>> triangle(T);
    for (int t = 0; t < T; ++t)
        for (int k = 0; k < 3; ++k) triangle[t][k] = poly[tri[t][k]];

    vector<vector<AdjEdge>> g(T);
    map<pair<int,int>, int> owner;
    for (int t = 0; t < T; ++t) {
        for (int k = 0; k < 3; ++k) {
            int a = tri[t][k], b = tri[t][(k+1)%3];
            if (a > b) swap(a,b);
            auto key = make_pair(a,b);
            auto it = owner.find(key);
            if (it == owner.end()) owner[key] = t;
            else {
                int u = it->second;
                g[t].push_back({u,a,b});
                g[u].push_back({t,a,b});
            }
        }
    }

    vector<vector<vector<Point>>> pieces(T);
    for (Point q : cam) {
        vector<vector<Point>> vis(T);
        vector<char> done(T,0);
        queue<int> que;

        for (int t = 0; t < T; ++t) {
            if (in_triangle_inclusive(q, triangle[t])) {
                vis[t] = {triangle[t][0], triangle[t][1], triangle[t][2]};
                done[t] = 1;
                que.push(t);
            }
        }

        while (!que.empty()) {
            int u = que.front(); que.pop();
            for (auto e : g[u]) {
                int v = e.to;
                if (done[v]) continue;
                done[v] = 1;
                if (!vis[u].empty()) {
                    Point A = poly[e.a], B = poly[e.b];
                    ld L,R;
                    if (segment_clip_convex(A,B,vis[u],L,R) && R-L > PARAM_EPS) {
                        Point w1 = A + (B-A)*L, w2 = A + (B-A)*R;
                        Point d1 = w1-q, d2 = w2-q;
                        ld cr = cross(d1,d2);
                        if (fabsl(cr) > geom_tol(d1,d2)) {
                            if (cr < 0) swap(d1,d2);
                            vector<Point> cur = {triangle[v][0], triangle[v][1], triangle[v][2]};
                            cur = clip_halfplane(cur,q,d1);
                            cur = clip_halfplane(cur,q,Point{-d2.x,-d2.y});
                            vis[v] = move(cur);
                        }
                    }
                }
                que.push(v);
            }
        }
        for (int t = 0; t < T; ++t) if (!vis[t].empty()) pieces[t].push_back(move(vis[t]));
    }

    ld answer = 0;
    for (int t = 0; t < T; ++t) answer += union_area_convex(move(pieces[t]));
    cout << fixed << setprecision(15) << answer << '\n';
    return 0;
}

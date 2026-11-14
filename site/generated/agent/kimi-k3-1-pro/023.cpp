#include <algorithm>
#include <cmath>
#include <cstdio>
#include <deque>
#include <numeric>
#include <utility>
#include <vector>

using namespace std;
using ll = long long;
using ld = long double;

struct Point {
    ll x, y;
    int id;
};

static ll cross(const Point &a, const Point &b, const Point &c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

static ll sq(const Point &a) {
    return a.x * a.x + a.y * a.y;
}

static ll incircle(const Point &a, const Point &b, const Point &c, const Point &d) {
    Point ad{a.x - d.x, a.y - d.y, 0};
    Point bd{b.x - d.x, b.y - d.y, 0};
    Point cd{c.x - d.x, c.y - d.y, 0};
    return (ad.x * bd.y - ad.y * bd.x) * sq(cd)
         + (bd.x * cd.y - bd.y * cd.x) * sq(ad)
         + (cd.x * ad.y - cd.y * ad.x) * sq(bd);
}

struct Edge {
    Point org;
    Edge *rot, *next;
    bool alive;

    Edge *rev() const { return rot->rot; }
    const Point &dest() const { return rev()->org; }
    Edge *lnext() const { return rot->rev()->next->rot; }
    Edge *oprev() const { return rot->next->rot; }
};

struct Delaunay {
    const vector<Point> &p;
    deque<Edge> pool;
    bool reversed;

    Delaunay(const vector<Point> &p, bool reversed) : p(p), reversed(reversed) {}

    Edge *makeEdge(const Point &a, const Point &b) {
        size_t pos = pool.size();
        Point infinity{0, 0, -1};
        pool.push_back({a, nullptr, nullptr, true});
        pool.push_back({b, nullptr, nullptr, true});
        pool.push_back({infinity, nullptr, nullptr, true});
        pool.push_back({infinity, nullptr, nullptr, true});
        Edge *e = &pool[pos];
        Edge *r = &pool[pos + 1];
        Edge *dualE = &pool[pos + 2];
        Edge *dualR = &pool[pos + 3];
        e->rot = dualE;
        r->rot = dualR;
        dualE->rot = r;
        dualR->rot = e;
        e->next = e;
        r->next = r;
        dualE->next = dualR;
        dualR->next = dualE;
        return e;
    }

    static void splice(Edge *a, Edge *b) {
        swap(a->next->rot->next, b->next->rot->next);
        swap(a->next, b->next);
    }

    Edge *join(Edge *a, Edge *b) {
        Edge *e = makeEdge(a->dest(), b->org);
        splice(e, a->lnext());
        splice(e->rev(), b);
        return e;
    }

    static void removeEdge(Edge *e) {
        splice(e, e->oprev());
        splice(e->rev(), e->rev()->oprev());
        e->alive = e->rev()->alive = e->rot->alive = e->rev()->rot->alive = false;
    }

    bool leftOf(const Point &a, const Edge *e) const {
        return cross(e->org, e->dest(), a) > 0;
    }

    bool rightOf(const Point &a, const Edge *e) const {
        return cross(e->org, e->dest(), a) < 0;
    }

    bool inside(const Point &a, const Point &b, const Point &c, const Point &d) const {
        ll value = incircle(a, b, c, d);
        return reversed ? value < 0 : value > 0;
    }

    bool valid(Edge *e, Edge *base) const {
        return rightOf(e->dest(), base);
    }

    pair<Edge *, Edge *> build(int l, int r) {
        if (r - l == 1) {
            Edge *e = makeEdge(p[l], p[r]);
            return {e, e->rev()};
        }
        if (r - l == 2) {
            Edge *a = makeEdge(p[l], p[l + 1]);
            Edge *b = makeEdge(p[l + 1], p[r]);
            splice(a->rev(), b);
            ll side = cross(p[l], p[l + 1], p[r]);
            if (side == 0) return {a, b->rev()};
            Edge *c = join(b, a);
            if (side > 0) return {a, b->rev()};
            return {c->rev(), c};
        }
        int m = (l + r) / 2;
        auto [ldo, ldi] = build(l, m);
        auto [rdi, rdo] = build(m + 1, r);
        while (true) {
            if (leftOf(rdi->org, ldi)) {
                ldi = ldi->lnext();
            } else if (rightOf(ldi->org, rdi)) {
                rdi = rdi->rev()->next;
            } else {
                break;
            }
        }
        Edge *base = join(rdi->rev(), ldi);
        if (ldi->org.id == ldo->org.id) ldo = base->rev();
        if (rdo->org.id == rdi->org.id) rdo = base;
        while (true) {
            Edge *lc = base->rev()->next;
            if (valid(lc, base)) {
                while (inside(base->dest(), base->org, lc->dest(), lc->next->dest())) {
                    Edge *t = lc->next;
                    removeEdge(lc);
                    lc = t;
                }
            }
            Edge *rc = base->oprev();
            if (valid(rc, base)) {
                while (inside(base->dest(), base->org, rc->dest(), rc->oprev()->dest())) {
                    Edge *t = rc->oprev();
                    removeEdge(rc);
                    rc = t;
                }
            }
            bool lv = valid(lc, base);
            bool rv = valid(rc, base);
            if (!lv && !rv) break;
            if (!lv || (rv && inside(lc->dest(), lc->org, rc->org, rc->dest()))) {
                base = join(rc, base->rev());
            } else {
                base = join(base->rev(), lc->rev());
            }
        }
        return {ldo, rdo};
    }

    vector<vector<int>> neighbors() {
        build(0, (int)p.size() - 1);
        vector<vector<int>> adj(p.size());
        for (const Edge &e : pool) {
            if (e.alive && e.org.id >= 0) adj[e.org.id].push_back(e.dest().id);
        }
        for (auto &v : adj) {
            sort(v.begin(), v.end());
            v.erase(unique(v.begin(), v.end()), v.end());
        }
        return adj;
    }
};

struct FPoint {
    ld x, y;
};

struct Line {
    ll x, y;
    ld c;
};

static ll cross(const Line &a, const Line &b) {
    return a.x * b.y - a.y * b.x;
}

static int half(const Line &a) {
    return (a.y > 0 || (a.y == 0 && a.x >= 0)) ? 0 : 1;
}

static FPoint intersect(const Line &a, const Line &b) {
    ld det = (ld)cross(a, b);
    return {(a.c * b.y - a.y * b.c) / det,
            (a.x * b.c - a.c * b.x) / det};
}

static bool outside(const Line &l, const FPoint &p) {
    return l.x * p.x + l.y * p.y - l.c > 1e-9L;
}

static vector<FPoint> cellPolygon(const Point &site, const vector<int> &adj,
                                  const vector<Point> &sites, ll xl, ll yl, ll xr, ll yr,
                                  bool farthest) {
    vector<Line> lines = {{1, 0, (ld)xr}, {0, 1, (ld)yr},
                          {-1, 0, (ld)-xl}, {0, -1, (ld)-yl}};
    lines.reserve(adj.size() + 4);
    for (int j : adj) {
        ll dx = sites[j].x - site.x;
        ll dy = sites[j].y - site.y;
        ld c = ((ld)sq(sites[j]) - (ld)sq(site)) / 2;
        if (farthest) {
            dx = -dx;
            dy = -dy;
            c = -c;
        }
        lines.push_back({dx, dy, c});
    }
    sort(lines.begin(), lines.end(), [](const Line &a, const Line &b) {
        if (half(a) != half(b)) return half(a) < half(b);
        ll cr = cross(a, b);
        if (cr != 0) return cr > 0;
        return a.c / hypot((ld)a.x, (ld)a.y) > b.c / hypot((ld)b.x, (ld)b.y);
    });
    vector<Line> uniqueLines;
    for (const Line &l : lines) {
        if (!uniqueLines.empty() && half(uniqueLines.back()) == half(l)
            && cross(uniqueLines.back(), l) == 0) {
            uniqueLines.back() = l;
        } else {
            uniqueLines.push_back(l);
        }
    }
    deque<Line> dq;
    deque<FPoint> vertices;
    for (const Line &l : uniqueLines) {
        while (!vertices.empty() && outside(l, vertices.back())) {
            vertices.pop_back();
            dq.pop_back();
        }
        while (!vertices.empty() && outside(l, vertices.front())) {
            vertices.pop_front();
            dq.pop_front();
        }
        if (!dq.empty()) {
            if (cross(dq.back(), l) == 0) return {};
            vertices.push_back(intersect(dq.back(), l));
        }
        dq.push_back(l);
    }
    while (vertices.size() > 1 && outside(dq.back(), vertices.front())) {
        vertices.pop_front();
        dq.pop_front();
    }
    while (vertices.size() > 1 && outside(dq.front(), vertices.back())) {
        vertices.pop_back();
        dq.pop_back();
    }
    if (dq.size() < 3) return {};
    vertices.push_back(intersect(dq.back(), dq.front()));
    return vector<FPoint>(vertices.begin(), vertices.end());
}

static ld integrateDistance(const Point &site, const vector<FPoint> &poly) {
    ld twiceArea = 0, ix = 0, iy = 0, ixx = 0, iyy = 0;
    for (int i = 0; i < (int)poly.size(); ++i) {
        const FPoint &a = poly[i];
        const FPoint &b = poly[(i + 1) % poly.size()];
        ld cr = a.x * b.y - a.y * b.x;
        twiceArea += cr;
        ix += cr * (a.x + b.x);
        iy += cr * (a.y + b.y);
        ixx += cr * (a.x * a.x + a.x * b.x + b.x * b.x);
        iyy += cr * (a.y * a.y + a.y * b.y + b.y * b.y);
    }
    ld area = twiceArea / 2;
    ix /= 6;
    iy /= 6;
    ixx /= 12;
    iyy /= 12;
    return ixx + iyy - 2 * site.x * ix - 2 * site.y * iy + sq(site) * area;
}

static ld totalIntegral(const vector<Point> &sites, const vector<vector<int>> &adj,
                        ll xl, ll yl, ll xr, ll yr, bool farthest) {
    ld result = 0;
    for (int i = 0; i < (int)sites.size(); ++i) {
        vector<FPoint> poly = cellPolygon(sites[i], adj[i], sites, xl, yl, xr, yr, farthest);
        if (!poly.empty()) result += integrateDistance(sites[i], poly);
    }
    return result;
}

int main() {
    ll xl, yl, xr, yr;
    int n;
    if (scanf("%lld %lld %lld %lld %d", &xl, &yl, &xr, &yr, &n) != 5) return 0;
    vector<Point> input(n);
    for (int i = 0; i < n; ++i) {
        scanf("%lld %lld", &input[i].x, &input[i].y);
        input[i].id = i;
    }
    sort(input.begin(), input.end(), [](const Point &a, const Point &b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    for (int i = 0; i < n; ++i) input[i].id = i;
    vector<Point> nearestSites = input;
    Delaunay nearest(input, false);
    vector<vector<int>> nearestAdj = nearest.neighbors();

    vector<Point> hull;
    for (const Point &p : input) {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), p) <= 0) hull.pop_back();
        hull.push_back(p);
    }
    int lowerSize = hull.size();
    for (int i = n - 2; i >= 0; --i) {
        const Point &p = input[i];
        while ((int)hull.size() > lowerSize
               && cross(hull[hull.size() - 2], hull.back(), p) <= 0) hull.pop_back();
        hull.push_back(p);
    }
    hull.pop_back();
    for (int i = 0; i < (int)hull.size(); ++i) hull[i].id = i;
    Delaunay farthest(hull, true);
    vector<vector<int>> farthestAdj = farthest.neighbors();

    ld nearestValue = totalIntegral(nearestSites, nearestAdj, xl, yl, xr, yr, false);
    ld farthestValue = totalIntegral(hull, farthestAdj, xl, yl, xr, yr, true);
    ld rectangleArea = (ld)(xr - xl) * (yr - yl);
    ld answer = acosl(-1.0L) * (farthestValue - nearestValue) / rectangleArea;
    printf("%.20Lf\n", answer);
    return 0;
}

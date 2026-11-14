#include <bits/stdc++.h>
using namespace std;

using ld = long double;
using i128 = __int128_t;

struct IPoint {
    long long x, y;
    bool operator<(const IPoint& p) const {
        return x != p.x ? x < p.x : y < p.y;
    }
};

static long long orient(const IPoint& a, const IPoint& b,
                        const IPoint& c) {
    return (b.x - a.x) * (c.y - a.y)
         - (b.y - a.y) * (c.x - a.x);
}

/* Divide-and-conquer Delaunay triangulation, using quad-edges. */
class Delaunay {
public:
    struct Edge {
        int org, next;
    };

    vector<IPoint> p;
    vector<Edge> e;

    explicit Delaunay(vector<IPoint> points) : p(move(points)) {
        sort(p.begin(), p.end());
        e.reserve(16 * p.size());
        build(0, (int)p.size() - 1);
    }

    static int rev(int x) { return x ^ 2; }
    static int rot(int x) { return (x & ~3) | ((x + 1) & 3); }
    static int invrot(int x) { return (x & ~3) | ((x + 3) & 3); }

    int dest(int x) const { return e[rev(x)].org; }

    int lnext(int x) const {
        return rot(e[invrot(x)].next);
    }

private:
    vector<int> freeBlocks;

    int oprev(int x) const {
        return rot(e[rot(x)].next);
    }

    int makeEdge(int a, int b) {
        int z;
        if (freeBlocks.empty()) {
            z = (int)e.size();
            e.resize(z + 4);
        } else {
            z = freeBlocks.back();
            freeBlocks.pop_back();
        }
        e[z]     = {a, z};
        e[z + 1] = {-1, z + 3};
        e[z + 2] = {b, z + 2};
        e[z + 3] = {-1, z + 1};
        return z;
    }

    void splice(int a, int b) {
        int x = rot(e[a].next);
        int y = rot(e[b].next);
        swap(e[x].next, e[y].next);
        swap(e[a].next, e[b].next);
    }

    int connect(int a, int b) {
        int z = makeEdge(dest(a), e[b].org);
        splice(z, lnext(a));
        splice(rev(z), b);
        return z;
    }

    void eraseEdge(int x) {
        splice(x, oprev(x));
        splice(rev(x), oprev(rev(x)));
        int z = x & ~3;
        for (int j = 0; j < 4; ++j) e[z + j].org = -2;
        freeBlocks.push_back(z);
    }

    bool leftOf(int v, int x) const {
        return orient(p[e[x].org], p[dest(x)], p[v]) > 0;
    }

    bool rightOf(int v, int x) const {
        return orient(p[e[x].org], p[dest(x)], p[v]) < 0;
    }

    bool inCircle(int a, int b, int c, int d) const {
        long long ax = p[a].x - p[d].x;
        long long ay = p[a].y - p[d].y;
        long long bx = p[b].x - p[d].x;
        long long by = p[b].y - p[d].y;
        long long cx = p[c].x - p[d].x;
        long long cy = p[c].y - p[d].y;

        i128 aa = (i128)ax * ax + (i128)ay * ay;
        i128 bb = (i128)bx * bx + (i128)by * by;
        i128 cc = (i128)cx * cx + (i128)cy * cy;

        i128 value =
              aa * ((i128)bx * cy - (i128)by * cx)
            - bb * ((i128)ax * cy - (i128)ay * cx)
            + cc * ((i128)ax * by - (i128)ay * bx);
        return value > 0;
    }

    pair<int, int> build(int l, int r) {
        if (r - l == 1) {
            int a = makeEdge(l, r);
            return {a, rev(a)};
        }

        if (r - l == 2) {
            int a = makeEdge(l, l + 1);
            int b = makeEdge(l + 1, r);
            splice(rev(a), b);

            long long s = orient(p[l], p[l + 1], p[r]);
            if (s == 0) return {a, rev(b)};

            int c = connect(b, a);
            if (s > 0) return {a, rev(b)};
            return {rev(c), c};
        }

        int m = (l + r) / 2;
        auto [ldo, ldi] = build(l, m);
        auto [rdi, rdo] = build(m + 1, r);

        while (true) {
            if (leftOf(e[rdi].org, ldi)) {
                ldi = lnext(ldi);
            } else if (rightOf(e[ldi].org, rdi)) {
                rdi = e[rev(rdi)].next;
            } else {
                break;
            }
        }

        int base = connect(rev(rdi), ldi);
        if (e[ldi].org == e[ldo].org) ldo = rev(base);
        if (e[rdi].org == e[rdo].org) rdo = base;

        while (true) {
            int lc = e[rev(base)].next;
            if (rightOf(dest(lc), base)) {
                while (inCircle(dest(base), e[base].org,
                                dest(lc), dest(e[lc].next))) {
                    int t = e[lc].next;
                    eraseEdge(lc);
                    lc = t;
                }
            }

            int rc = oprev(base);
            if (rightOf(dest(rc), base)) {
                while (inCircle(dest(base), e[base].org,
                                dest(rc), dest(oprev(rc)))) {
                    int t = oprev(rc);
                    eraseEdge(rc);
                    rc = t;
                }
            }

            bool vl = rightOf(dest(lc), base);
            bool vr = rightOf(dest(rc), base);
            if (!vl && !vr) break;

            if (!vl || (vr && inCircle(dest(lc), e[lc].org,
                                      e[rc].org, dest(rc)))) {
                base = connect(rc, rev(base));
            } else {
                base = connect(rev(base), rev(lc));
            }
        }
        return {ldo, rdo};
    }
};

struct Point {
    double x, y;
};

// A shear avoids vertical polygon edges and vertical Voronoi edges.
static constexpr double SHEAR = 0.384423506773;

static Point transform(ld x, ld y) {
    return {(double)(x + SHEAR * y), (double)y};
}

struct Segment {
    double x1, y1, x2, y2, slope;
    int color;
    bool forward;

    Segment() = default;

    Segment(Point a, Point b, int c) : color(c) {
        forward = a.x < b.x;
        if (a.x > b.x) swap(a, b);
        x1 = a.x;
        y1 = a.y;
        x2 = b.x;
        y2 = b.y;
        slope = (y2 - y1) / (x2 - x1);
    }

    double at(double x) const {
        return y1 + slope * (x - x1);
    }
};

struct SweepCompare {
    const vector<Segment>* segments;
    const double* x;

    bool operator()(int a, int b) const {
        if (a == b) return false;
        const auto& u = (*segments)[a];
        const auto& v = (*segments)[b];
        double yu = u.at(*x), yv = v.at(*x);
        if (yu != yv) return yu < yv;
        if (u.slope != v.slope) return u.slope < v.slope;
        return a < b;
    }
};

struct Event {
    double x;
    int id;
    int type; // 0: end, 1: start, 2: query

    bool operator<(const Event& b) const {
        if (x != b.x) return x < b.x;
        if (type != b.type) return type < b.type;
        return id < b.id;
    }
};

struct Candidate {
    Point p;
    double radius;
};

/*
   Determine which Voronoi vertices belong to the polygon.

   At a query point, consider the polygon edge immediately below it.
   Since the polygon is counter-clockwise, the point is inside exactly
   when that edge is directed from left to right.
*/
static double interiorVertexBound(const vector<IPoint>& polygon,
                                  const vector<Candidate>& queries) {
    int n = (int)polygon.size();
    vector<Segment> segments;
    segments.reserve(n + 1);

    vector<Event> events;
    events.reserve(2 * n + queries.size());

    for (int i = 0; i < n; ++i) {
        Point a = transform(polygon[i].x, polygon[i].y);
        Point b = transform(polygon[(i + 1) % n].x,
                            polygon[(i + 1) % n].y);
        segments.emplace_back(a, b, 0);
        events.push_back({segments.back().x1, i, 1});
        events.push_back({segments.back().x2, i, 0});
    }

    segments.emplace_back();
    for (int i = 0; i < (int)queries.size(); ++i)
        events.push_back({queries[i].p.x, i, 2});

    sort(events.begin(), events.end());

    double sweepX = 0;
    using Set = set<int, SweepCompare>;
    Set active(SweepCompare{&segments, &sweepX});
    vector<Set::iterator> where(n);

    double answer = 0;
    for (const Event& ev : events) {
        sweepX = ev.x;
        if (ev.type == 0) {
            active.erase(where[ev.id]);
        } else if (ev.type == 1) {
            where[ev.id] = active.insert(ev.id).first;
        } else {
            double y = queries[ev.id].p.y;
            Segment& dummy = segments[n];
            dummy.x1 = sweepX;
            dummy.y1 = y;
            dummy.slope = 0;

            auto it = active.lower_bound(n);
            bool inside = false;

            if (it != active.end() &&
                abs(segments[*it].at(sweepX) - y) <= 1e-8) {
                inside = true;
            }
            if (it != active.begin()) {
                auto below = prev(it);
                if (abs(segments[*below].at(sweepX) - y) <= 1e-8)
                    inside = true;
                else if (segments[*below].forward)
                    inside = true;
            }

            if (inside) answer = max(answer, queries[ev.id].radius);
        }
    }
    return answer;
}

static bool intersects(const Segment& a, const Segment& b) {
    if (a.color == b.color) return false;

    if (max(a.x1, b.x1) > min(a.x2, b.x2)) return false;
    if (max(min(a.y1, a.y2), min(b.y1, b.y2)) >
        min(max(a.y1, a.y2), max(b.y1, b.y2)))
        return false;

    auto cross = [](double ax, double ay, double bx, double by,
                    double cx, double cy) -> ld {
        return ((ld)bx - ax) * ((ld)cy - ay)
             - ((ld)by - ay) * ((ld)cx - ax);
    };

    ld x = cross(a.x1, a.y1, a.x2, a.y2, b.x1, b.y1);
    ld y = cross(a.x1, a.y1, a.x2, a.y2, b.x2, b.y2);
    if ((x > 0 && y > 0) || (x < 0 && y < 0)) return false;

    x = cross(b.x1, b.y1, b.x2, b.y2, a.x1, a.y1);
    y = cross(b.x1, b.y1, b.x2, b.y2, a.x2, a.y2);
    return !((x > 0 && y > 0) || (x < 0 && y < 0));
}

/*
   Each color is a noncrossing segment family.
   The usual intersection-detection sweep therefore needs no
   intersection events: it stops as soon as two colors intersect.
*/
static bool hasBichromaticIntersection(const vector<Segment>& segments) {
    if (segments.empty()) return false;

    vector<Event> events;
    events.reserve(2 * segments.size());
    for (int i = 0; i < (int)segments.size(); ++i) {
        events.push_back({segments[i].x1, i, 1});
        events.push_back({segments[i].x2, i, 0});
    }
    sort(events.begin(), events.end());

    double sweepX = 0;
    using Set = set<int, SweepCompare>;
    Set active(SweepCompare{&segments, &sweepX});
    vector<Set::iterator> where(segments.size());

    for (const Event& ev : events) {
        sweepX = ev.x;
        if (ev.type == 1) {
            auto it = active.insert(ev.id).first;
            where[ev.id] = it;

            if (it != active.begin() &&
                intersects(segments[*prev(it)], segments[*it]))
                return true;

            auto nx = next(it);
            if (nx != active.end() &&
                intersects(segments[*it], segments[*nx]))
                return true;
        } else {
            auto it = where[ev.id];
            auto nx = next(it);
            if (it != active.begin() && nx != active.end()) {
                auto pr = prev(it);
                if (intersects(segments[*pr], segments[*nx]))
                    return true;
            }
            active.erase(it);
        }
    }
    return false;
}

struct Face {
    int a = -1, b = -1, c = -1;
    bool bounded = false;
};

struct VoronoiEdge {
    // Points on the edge are m + v*t, with |v| = 1.
    ld mx, my, vx, vy;
    ld lo, hi;
    ld halfSquared;
};

struct PolygonEdge {
    ld ax, ay, bx, by, length;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;
    vector<IPoint> polygon(n);
    long long xmin = LLONG_MAX, ymin = LLONG_MAX;
    long long xmax = LLONG_MIN, ymax = LLONG_MIN;

    for (auto& p : polygon) {
        cin >> p.x >> p.y;
        xmin = min(xmin, p.x);
        xmax = max(xmax, p.x);
        ymin = min(ymin, p.y);
        ymax = max(ymax, p.y);
    }

    vector<PolygonEdge> polygonEdges;
    polygonEdges.reserve(n);
    double boundaryUpper = 0;

    for (int i = 0; i < n; ++i) {
        auto a = polygon[i];
        auto b = polygon[(i + 1) % n];
        ld length = hypotl((ld)b.x - a.x, (ld)b.y - a.y);
        polygonEdges.push_back({
            (ld)a.x, (ld)a.y, (ld)b.x, (ld)b.y, length
        });
        boundaryUpper = max(boundaryUpper, (double)(length / 2));
    }

    Delaunay dt(polygon);

    vector<int> faceOf(dt.e.size(), -1);
    vector<Face> faces;
    vector<Candidate> candidates;
    faces.reserve(2 * n);
    candidates.reserve(2 * n);

    const ld boxL = (ld)xmin - 1;
    const ld boxR = (ld)xmax + 1;
    const ld boxB = (ld)ymin - 1;
    const ld boxT = (ld)ymax + 1;

    for (int start = 0; start < (int)dt.e.size(); start += 2) {
        if (dt.e[start].org < 0 || faceOf[start] != -1) continue;

        int id = (int)faces.size();
        int cur = start, count = 0;
        int vertices[3] = {-1, -1, -1};

        do {
            faceOf[cur] = id;
            if (count < 3) vertices[count] = dt.e[cur].org;
            ++count;
            cur = dt.lnext(cur);
        } while (cur != start);

        Face f;
        if (count == 3 &&
            orient(dt.p[vertices[0]], dt.p[vertices[1]],
                   dt.p[vertices[2]]) > 0) {
            f = {vertices[0], vertices[1], vertices[2], true};

            const auto& a = dt.p[f.a];
            const auto& b = dt.p[f.b];
            const auto& c = dt.p[f.c];

            ld bx = (ld)b.x - a.x, by = (ld)b.y - a.y;
            ld cx = (ld)c.x - a.x, cy = (ld)c.y - a.y;
            ld den = 2 * (bx * cy - by * cx);
            ld b2 = bx * bx + by * by;
            ld c2 = cx * cx + cy * cy;

            ld ux = (b2 * cy - c2 * by) / den;
            ld uy = (bx * c2 - cx * b2) / den;
            ld x = a.x + ux, y = a.y + uy;

            if (x >= boxL && x <= boxR && y >= boxB && y <= boxT) {
                candidates.push_back({
                    transform(x, y), (double)hypotl(ux, uy)
                });
            }
        }
        faces.push_back(f);
    }

    double lower = interiorVertexBound(polygon, candidates);

    // If all boundary points are already bounded by this value, we are done.
    if (lower >= boundaryUpper) {
        cout << fixed << setprecision(12) << lower << '\n';
        return 0;
    }

    vector<VoronoiEdge> voronoi;
    voronoi.reserve(3 * n);

    for (int e = 0; e < (int)dt.e.size(); e += 4) {
        if (dt.e[e].org < 0) continue;

        int aId = dt.e[e].org;
        int bId = dt.dest(e);
        const auto& a = dt.p[aId];
        const auto& b = dt.p[bId];

        ld dx = (ld)b.x - a.x, dy = (ld)b.y - a.y;
        ld length = hypotl(dx, dy);
        ld mx = ((ld)a.x + b.x) / 2;
        ld my = ((ld)a.y + b.y) / 2;
        ld vx = -dy / length, vy = dx / length;
        ld halfSquared = length * length / 4;

        const Face& left = faces[faceOf[e]];
        const Face& right = faces[faceOf[Delaunay::rev(e)]];

        auto parameter = [&](const Face& f) -> ld {
            int cId = f.a;
            if (cId == aId || cId == bId) cId = f.b;
            if (cId == aId || cId == bId) cId = f.c;

            ld cx = (ld)dt.p[cId].x - mx;
            ld cy = (ld)dt.p[cId].y - my;
            return (cx * cx + cy * cy - halfSquared)
                 / (2 * (cx * vx + cy * vy));
        };

        ld lo = -1e100L, hi = 1e100L;
        if (left.bounded && right.bounded) {
            ld t1 = parameter(left), t2 = parameter(right);
            lo = min(t1, t2);
            hi = max(t1, t2);
        } else if (left.bounded) {
            hi = parameter(left);
        } else if (right.bounded) {
            lo = parameter(right);
        } else {
            continue;
        }

        auto clip = [&](ld m, ld v, ld l, ld r) -> bool {
            if (v == 0) return m >= l && m <= r;
            ld t1 = (l - m) / v;
            ld t2 = (r - m) / v;
            if (t1 > t2) swap(t1, t2);
            lo = max(lo, t1);
            hi = min(hi, t2);
            return lo < hi;
        };

        if (!clip(mx, vx, boxL, boxR) ||
            !clip(my, vy, boxB, boxT) || !(lo < hi))
            continue;

        voronoi.push_back({mx, my, vx, vy, lo, hi, halfSquared});
    }

    vector<Segment> sweepSegments;
    sweepSegments.reserve(n + 2 * voronoi.size());

    auto uncoveredBoundaryExists = [&](double radius) -> bool {
        sweepSegments.clear();
        ld r = radius;
        ld r2 = r * r;

        int redCount = 0;
        for (const auto& e : polygonEdges) {
            if (e.length <= 2 * r) continue;

            ld f = r / e.length;
            ld dx = e.bx - e.ax, dy = e.by - e.ay;
            Point a = transform(e.ax + f * dx, e.ay + f * dy);
            Point b = transform(e.bx - f * dx, e.by - f * dy);
            sweepSegments.emplace_back(a, b, 0);
            ++redCount;
        }
        if (redCount == 0) return false;

        auto addBlue = [&](const VoronoiEdge& e, ld a, ld b) {
            if (!(a < b)) return;
            Point p = transform(e.mx + e.vx * a, e.my + e.vy * a);
            Point q = transform(e.mx + e.vx * b, e.my + e.vy * b);
            if (p.x != q.x)
                sweepSegments.emplace_back(p, q, 1);
        };

        for (const auto& e : voronoi) {
            if (e.halfSquared + max(e.lo * e.lo, e.hi * e.hi) <= r2)
                continue;

            if (r2 <= e.halfSquared) {
                addBlue(e, e.lo, e.hi);
            } else {
                ld cut = sqrtl(r2 - e.halfSquared);
                addBlue(e, e.lo, min(e.hi, -cut));
                addBlue(e, max(e.lo, cut), e.hi);
            }
        }

        if ((int)sweepSegments.size() == redCount) return false;
        return hasBichromaticIntersection(sweepSegments);
    };

    double upper = boundaryUpper;
    for (int iteration = 0; iteration < 45; ++iteration) {
        if (upper - lower <= 1e-8 * max(1.0, upper)) break;
        double middle = (lower + upper) / 2;
        if (uncoveredBoundaryExists(middle))
            lower = middle;
        else
            upper = middle;
    }

    cout << fixed << setprecision(12) << upper << '\n';
    return 0;
}

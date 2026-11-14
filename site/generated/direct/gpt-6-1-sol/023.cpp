#include <bits/stdc++.h>
using namespace std;

using ll = long long;
using i128 = __int128_t;
using ld = long double;

struct Point {
    ll x, y;

    bool operator<(const Point& other) const {
        return x != other.x ? x < other.x : y < other.y;
    }
};

ll cross(const Point& a, const Point& b, const Point& c) {
    return (b.x - a.x) * (c.y - a.y)
         - (b.y - a.y) * (c.x - a.x);
}

// Divide-and-conquer Delaunay triangulation, using indexed quad-edges.
class Delaunay {
    struct Node {
        int origin;
        int next;
    };

    const vector<Point>& p;
    vector<Node> e;
    vector<int> freeGroups;

    static int rot(int x) {
        return (x & ~3) | ((x + 1) & 3);
    }

    static int invRot(int x) {
        return (x & ~3) | ((x + 3) & 3);
    }

    static int sym(int x) {
        return x ^ 2;
    }

    int dest(int x) const {
        return e[sym(x)].origin;
    }

    int oPrev(int x) const {
        return rot(e[rot(x)].next);
    }

    int lNext(int x) const {
        return rot(e[invRot(x)].next);
    }

    int makeEdge(int a, int b) {
        int base;
        if (freeGroups.empty()) {
            base = (int)e.size();
            e.resize(base + 4);
        } else {
            base = freeGroups.back();
            freeGroups.pop_back();
        }

        e[base]     = {a,  base};
        e[base + 1] = {-1, base + 3};
        e[base + 2] = {b,  base + 2};
        e[base + 3] = {-1, base + 1};
        return base;
    }

    void splice(int a, int b) {
        int alpha = rot(e[a].next);
        int beta = rot(e[b].next);
        swap(e[alpha].next, e[beta].next);
        swap(e[a].next, e[b].next);
    }

    int connect(int a, int b) {
        int c = makeEdge(dest(a), e[b].origin);
        splice(c, lNext(a));
        splice(sym(c), b);
        return c;
    }

    void deleteEdge(int a) {
        splice(a, oPrev(a));
        splice(sym(a), oPrev(sym(a)));

        int base = a & ~3;
        e[base].origin = -1;
        freeGroups.push_back(base);
    }

    bool leftOf(int vertex, int edge) const {
        return cross(p[e[edge].origin], p[dest(edge)], p[vertex]) > 0;
    }

    bool rightOf(int vertex, int edge) const {
        return cross(p[e[edge].origin], p[dest(edge)], p[vertex]) < 0;
    }

    bool inCircle(int a, int b, int c, int d) const {
        ll ax = p[a].x - p[d].x;
        ll ay = p[a].y - p[d].y;
        ll bx = p[b].x - p[d].x;
        ll by = p[b].y - p[d].y;
        ll cx = p[c].x - p[d].x;
        ll cy = p[c].y - p[d].y;

        ll aa = ax * ax + ay * ay;
        ll bb = bx * bx + by * by;
        ll cc = cx * cx + cy * cy;

        i128 value =
            (i128)aa * (bx * cy - by * cx)
          - (i128)bb * (ax * cy - ay * cx)
          + (i128)cc * (ax * by - ay * bx);

        return value > 0;
    }

    pair<int, int> build(int l, int r) {
        int count = r - l;

        if (count == 2) {
            int a = makeEdge(l, l + 1);
            return {a, sym(a)};
        }

        if (count == 3) {
            int a = makeEdge(l, l + 1);
            int b = makeEdge(l + 1, l + 2);
            splice(sym(a), b);

            ll orientation = cross(p[l], p[l + 1], p[l + 2]);

            if (orientation == 0) {
                return {a, sym(b)};
            }

            int c = connect(b, a);

            if (orientation > 0) {
                return {a, sym(b)};
            }
            return {sym(c), c};
        }

        int m = (l + r) / 2;
        auto [ldoInitial, ldiInitial] = build(l, m);
        auto [rdiInitial, rdoInitial] = build(m, r);

        int ldo = ldoInitial, ldi = ldiInitial;
        int rdi = rdiInitial, rdo = rdoInitial;

        while (true) {
            if (leftOf(e[rdi].origin, ldi)) {
                ldi = lNext(ldi);
            } else if (rightOf(e[ldi].origin, rdi)) {
                rdi = e[sym(rdi)].next;
            } else {
                break;
            }
        }

        int base = connect(sym(rdi), ldi);

        if (e[ldi].origin == e[ldo].origin) {
            ldo = sym(base);
        }
        if (e[rdi].origin == e[rdo].origin) {
            rdo = base;
        }

        while (true) {
            int lcand = e[sym(base)].next;

            if (rightOf(dest(lcand), base)) {
                while (inCircle(
                    dest(base), e[base].origin,
                    dest(lcand), dest(e[lcand].next)
                )) {
                    int next = e[lcand].next;
                    deleteEdge(lcand);
                    lcand = next;
                }
            }

            int rcand = oPrev(base);

            if (rightOf(dest(rcand), base)) {
                while (inCircle(
                    dest(base), e[base].origin,
                    dest(rcand), dest(oPrev(rcand))
                )) {
                    int previous = oPrev(rcand);
                    deleteEdge(rcand);
                    rcand = previous;
                }
            }

            bool leftValid = rightOf(dest(lcand), base);
            bool rightValid = rightOf(dest(rcand), base);

            if (!leftValid && !rightValid) {
                break;
            }

            if (!leftValid ||
                (rightValid && inCircle(
                    dest(lcand), e[lcand].origin,
                    e[rcand].origin, dest(rcand)
                ))) {
                base = connect(rcand, sym(base));
            } else {
                base = connect(sym(base), sym(lcand));
            }
        }

        return {ldo, rdo};
    }

public:
    explicit Delaunay(const vector<Point>& points) : p(points) {
        e.reserve(points.size() * 24);
        build(0, (int)p.size());
    }

    vector<int> getStartingEdges() const {
        vector<int> start(p.size(), -1);

        for (int base = 0; base < (int)e.size(); base += 4) {
            if (e[base].origin == -1) continue;
            start[e[base].origin] = base;
            start[e[base + 2].origin] = base + 2;
        }

        return start;
    }

    int destination(int edge) const {
        return dest(edge);
    }

    int nextAroundOrigin(int edge) const {
        return e[edge].next;
    }
};

// Half-plane: a*x + b*y <= c.
struct Line {
    ll a, b, c;
};

ll determinant(const Line& u, const Line& v) {
    return u.a * v.b - u.b * v.a;
}

int angleHalf(const Line& l) {
    return (l.b < 0 || (l.b == 0 && l.a < 0)) ? 1 : 0;
}

bool angleLess(const Line& u, const Line& v) {
    int hu = angleHalf(u);
    int hv = angleHalf(v);
    if (hu != hv) return hu < hv;
    return determinant(u, v) > 0;
}

bool sameDirection(const Line& u, const Line& v) {
    return determinant(u, v) == 0
        && u.a * v.a + u.b * v.b > 0;
}

// Does the intersection of u and v lie strictly outside w?
// Every operation here is exact.
bool intersectionOutside(const Line& u, const Line& v, const Line& w) {
    ll d = determinant(u, v);
    ll nx = u.c * v.b - u.b * v.c;
    ll ny = u.a * v.c - u.c * v.a;

    i128 value = (i128)w.a * nx + (i128)w.b * ny - (i128)w.c * d;
    return d > 0 ? value > 0 : value < 0;
}

struct RealPoint {
    ld x, y;
};

class PolygonIntegrator {
    vector<Line> dq;
    vector<RealPoint> vertices;
    ld centerX, centerY;

public:
    PolygonIntegrator(ld cx, ld cy) : centerX(cx), centerY(cy) {}

    // Input lines must be sorted by normal-vector angle.
    // Returns the integral of |p-center|^2
    // - 2*(p-center) dot (z-center) over the intersection.
    ld integrate(vector<Line>& lines, const Point& p) {
        int uniqueCount = 0;

        for (int i = 0; i < (int)lines.size(); ++i) {
            Line current = lines[i];

            if (uniqueCount > 0 &&
                sameDirection(lines[uniqueCount - 1], current)) {
                Line& old = lines[uniqueCount - 1];

                ll oldScale = max(llabs(old.a), llabs(old.b));
                ll newScale = max(llabs(current.a), llabs(current.b));

                if ((i128)current.c * oldScale <
                    (i128)old.c * newScale) {
                    old = current;
                }
            } else {
                lines[uniqueCount++] = current;
            }
        }

        lines.resize(uniqueCount);
        dq.resize(uniqueCount);

        int front = 0, back = 0;

        for (const Line& line : lines) {
            while (back - front >= 2 &&
                   intersectionOutside(dq[back - 2], dq[back - 1], line)) {
                --back;
            }

            while (back - front >= 2 &&
                   intersectionOutside(dq[front], dq[front + 1], line)) {
                ++front;
            }

            // Equal directions were already merged. Opposite parallel
            // surviving boundaries here imply an empty intersection.
            if (back > front && determinant(dq[back - 1], line) == 0) {
                return 0;
            }

            dq[back++] = line;
        }

        while (back - front >= 3 &&
               intersectionOutside(dq[back - 2], dq[back - 1], dq[front])) {
            --back;
        }

        while (back - front >= 3 &&
               intersectionOutside(dq[front], dq[front + 1], dq[back - 1])) {
            ++front;
        }

        int count = back - front;
        if (count < 3) return 0;
        if (determinant(dq[back - 1], dq[front]) == 0) return 0;

        vertices.resize(count);

        for (int i = 0; i < count; ++i) {
            const Line& u = dq[front + i];
            const Line& v = dq[front + (i + 1) % count];

            ll d = determinant(u, v);
            if (d == 0) return 0;

            ll nx = u.c * v.b - u.b * v.c;
            ll ny = u.a * v.c - u.c * v.a;

            vertices[i] = {
                (ld)nx / d - centerX,
                (ld)ny / d - centerY
            };
        }

        ld twiceArea = 0;
        ld sixMomentX = 0;
        ld sixMomentY = 0;

        for (int i = 0; i < count; ++i) {
            const RealPoint& u = vertices[i];
            const RealPoint& v = vertices[(i + 1) % count];

            ld cr = u.x * v.y - u.y * v.x;
            twiceArea += cr;
            sixMomentX += (u.x + v.x) * cr;
            sixMomentY += (u.y + v.y) * cr;
        }

        if (twiceArea <= 0) return 0;

        ld px = p.x - centerX;
        ld py = p.y - centerY;
        ld constant = px * px + py * py;

        return constant * twiceArea / 2
             - px * sixMomentX / 3
             - py * sixMomentY / 3;
    }
};

ll squaredNorm(const Point& p) {
    return p.x * p.x + p.y * p.y;
}

// f_p <= f_q.
Line nearestConstraint(const Point& p, const Point& q) {
    return {
        2 * (q.x - p.x),
        2 * (q.y - p.y),
        squaredNorm(q) - squaredNorm(p)
    };
}

// f_p >= f_q.
Line farthestConstraint(const Point& p, const Point& q) {
    return {
        2 * (p.x - q.x),
        2 * (p.y - q.y),
        squaredNorm(p) - squaredNorm(q)
    };
}

vector<Point> convexHull(const vector<Point>& points) {
    vector<Point> hull;

    for (const Point& p : points) {
        while (hull.size() >= 2 &&
               cross(hull[hull.size() - 2], hull.back(), p) <= 0) {
            hull.pop_back();
        }
        hull.push_back(p);
    }

    size_t lowerSize = hull.size();

    for (int i = (int)points.size() - 2; i >= 0; --i) {
        while (hull.size() > lowerSize &&
               cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0) {
            hull.pop_back();
        }
        hull.push_back(points[i]);
    }

    hull.pop_back();
    return hull;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ll xl, yl, xr, yr;
    cin >> xl >> yl >> xr >> yr;

    int n;
    cin >> n;

    vector<Point> points(n);
    for (Point& p : points) {
        cin >> p.x >> p.y;
    }
    sort(points.begin(), points.end());

    // Rectangle normals in counterclockwise angular order.
    array<Line, 4> rectangle = {{
        { 1,  0,  xr},
        { 0,  1,  yr},
        {-1,  0, -xl},
        { 0, -1, -yl}
    }};

    ld centerX = ((ld)xl + xr) / 2;
    ld centerY = ((ld)yl + yr) / 2;
    PolygonIntegrator integrator(centerX, centerY);

    ld nearestIntegral = 0;
    vector<Line> lines;
    lines.reserve(64);

    {
        Delaunay triangulation(points);
        vector<int> start = triangulation.getStartingEdges();

        for (int i = 0; i < n; ++i) {
            lines.clear();

            int edge = start[i];
            int first = edge;

            do {
                int j = triangulation.destination(edge);
                lines.push_back(nearestConstraint(points[i], points[j]));
                edge = triangulation.nextAroundOrigin(edge);
            } while (edge != first);

            for (const Line& line : rectangle) {
                lines.push_back(line);
            }

            sort(lines.begin(), lines.end(), angleLess);
            nearestIntegral += integrator.integrate(lines, points[i]);
        }
    }

    vector<Point> hull = convexHull(points);
    int h = (int)hull.size();

    ld farthestIntegral = 0;
    vector<Line> ordered;
    vector<Line> merged;
    ordered.reserve(h);
    merged.reserve(h + 4);

    for (int i = 0; i < h; ++i) {
        ordered.clear();

        // For a convex polygon, these directions are cyclically sorted.
        for (int step = 1; step < h; ++step) {
            int j = (i + step) % h;
            ordered.push_back(farthestConstraint(hull[i], hull[j]));
        }

        auto minimumAngle = min_element(
            ordered.begin(), ordered.end(), angleLess
        );
        rotate(ordered.begin(), minimumAngle, ordered.end());

        merged.clear();
        int a = 0, b = 0;

        while (a < (int)ordered.size() || b < 4) {
            if (b == 4 ||
                (a < (int)ordered.size() &&
                 angleLess(ordered[a], rectangle[b]))) {
                merged.push_back(ordered[a++]);
            } else {
                merged.push_back(rectangle[b++]);
            }
        }

        farthestIntegral += integrator.integrate(merged, hull[i]);
    }

    ld rectangleArea = (ld)(xr - xl) * (yr - yl);
    ld answer = acosl(-1.0L)
              * (farthestIntegral - nearestIntegral)
              / rectangleArea;

    cout << fixed << setprecision(18) << answer << '\n';
    return 0;
}

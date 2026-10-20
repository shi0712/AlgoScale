#include <bits/stdc++.h>
using namespace std;

const long double PI = acosl(-1.0L);
const long double EPS = 1e-12L;

struct Point {
    int x, y;
};

struct Vec {
    long double x, y;
};

using Poly = vector<Vec>;

/* ---------- Polygon clipping / moments ---------- */

Poly clipPolygon(const Poly& poly, long double A, long double B, long double C) {
    // keep A*x + B*y + C >= 0
    if (poly.empty()) return {};
    Poly res;
    res.reserve(poly.size() + 2);

    int n = (int)poly.size();
    for (int i = 0; i < n; ++i) {
        Vec S = poly[i];
        Vec T = poly[(i + 1) % n];

        long double fs = A * S.x + B * S.y + C;
        long double ft = A * T.x + B * T.y + C;

        bool inS = (fs >= -EPS);
        bool inT = (ft >= -EPS);

        if (inS && inT) {
            res.push_back(T);
        } else if (inS && !inT) {
            long double t = fs / (fs - ft);
            res.push_back({S.x + (T.x - S.x) * t,
                           S.y + (T.y - S.y) * t});
        } else if (!inS && inT) {
            long double t = fs / (fs - ft);
            res.push_back({S.x + (T.x - S.x) * t,
                           S.y + (T.y - S.y) * t});
            res.push_back(T);
        }
    }
    return res;
}

struct Moments {
    long double area = 0;
    long double mx = 0, my = 0;
    long double mxx = 0, myy = 0;
};

Moments polygonMoments(const Poly& poly) {
    Moments res;
    int n = (int)poly.size();
    if (n < 3) return res;

    for (int i = 0; i < n; ++i) {
        Vec a = poly[i];
        Vec b = poly[(i + 1) % n];

        long double cr = a.x * b.y - b.x * a.y;

        res.area += cr;
        res.mx += (a.x + b.x) * cr;
        res.my += (a.y + b.y) * cr;
        res.mxx += (a.x * a.x + a.x * b.x + b.x * b.x) * cr;
        res.myy += (a.y * a.y + a.y * b.y + b.y * b.y) * cr;
    }

    res.area *= 0.5L;
    res.mx /= 6.0L;
    res.my /= 6.0L;
    res.mxx /= 12.0L;
    res.myy /= 12.0L;

    if (res.area < 0) {
        res.area = -res.area;
        res.mx = -res.mx;
        res.my = -res.my;
        res.mxx = -res.mxx;
        res.myy = -res.myy;
    }

    return res;
}

long double integrateDistanceSquared(const Poly& poly, long double px, long double py) {
    Moments m = polygonMoments(poly);
    if (m.area == 0) return 0;

    long double ans = m.mxx + m.myy;
    ans -= 2.0L * px * m.mx;
    ans -= 2.0L * py * m.my;
    ans += (px * px + py * py) * m.area;
    return ans;
}

/* ---------- Half-plane intersection for farthest Voronoi ---------- */

struct HLine {
    long double A, B, C; // A*x + B*y + C >= 0
    long double angle;
};

Vec intersectLine(const HLine& a, const HLine& b) {
    long double det = a.A * b.B - a.B * b.A;
    long double x = (a.B * b.C - a.C * b.B) / det;
    long double y = (a.C * b.A - a.A * b.C) / det;
    return {x, y};
}

bool insideLine(const HLine& l, const Vec& p) {
    return l.A * p.x + l.B * p.y + l.C >= -EPS;
}

Poly halfPlaneIntersection(vector<HLine> lines) {
    sort(lines.begin(), lines.end(), [](const HLine& a, const HLine& b) {
        return a.angle < b.angle;
    });

    vector<HLine> filtered;
    for (const auto& l : lines) {
        if (!filtered.empty()) {
            long double cr = filtered.back().A * l.B - filtered.back().B * l.A;
            long double dot = filtered.back().A * l.A + filtered.back().B * l.B;

            if (fabsl(cr) < 1e-18L && dot > 0) {
                long double len1 = hypotl(filtered.back().A, filtered.back().B);
                long double len2 = hypotl(l.A, l.B);
                long double c1 = filtered.back().C / len1;
                long double c2 = l.C / len2;

                // For the same normal direction, smaller C/|n| is stricter.
                if (c2 < c1 - 1e-18L) {
                    filtered.back() = l;
                }
                continue;
            }
        }
        filtered.push_back(l);
    }

    deque<HLine> dq;

    for (const auto& l : filtered) {
        while (dq.size() >= 2) {
            Vec p = intersectLine(dq[dq.size() - 2], dq.back());
            if (!insideLine(l, p)) dq.pop_back();
            else break;
        }

        while (dq.size() >= 2) {
            Vec p = intersectLine(dq[0], dq[1]);
            if (!insideLine(l, p)) dq.pop_front();
            else break;
        }

        dq.push_back(l);
    }

    while (dq.size() >= 2) {
        Vec p = intersectLine(dq[dq.size() - 2], dq.back());
        if (!insideLine(dq.front(), p)) dq.pop_back();
        else break;
    }

    while (dq.size() >= 2) {
        Vec p = intersectLine(dq[0], dq[1]);
        if (!insideLine(dq.back(), p)) dq.pop_front();
        else break;
    }

    if (dq.size() < 3) return {};

    int m = (int)dq.size();
    Poly poly;
    poly.reserve(m);

    for (int i = 0; i < m; ++i) {
        poly.push_back(intersectLine(dq[i], dq[(i + 1) % m]));
    }

    return poly;
}

/* ---------- Delaunay triangulation via Bowyer-Watson ---------- */

vector<Point> gpts;

struct Triangle {
    int v[3];
    int n[3];
    bool alive;
    bool bad;

    Triangle() {
        v[0] = v[1] = v[2] = -1;
        n[0] = n[1] = n[2] = -1;
        alive = true;
        bad = false;
    }

    Triangle(int a, int b, int c) {
        v[0] = a;
        v[1] = b;
        v[2] = c;
        n[0] = n[1] = n[2] = -1;
        alive = true;
        bad = false;
    }
};

__int128 incircleValue(int a, int b, int c, int p) {
    long long ax = (long long)gpts[a].x - gpts[p].x;
    long long ay = (long long)gpts[a].y - gpts[p].y;

    long long bx = (long long)gpts[b].x - gpts[p].x;
    long long by = (long long)gpts[b].y - gpts[p].y;

    long long cx = (long long)gpts[c].x - gpts[p].x;
    long long cy = (long long)gpts[c].y - gpts[p].y;

    __int128 asq = (__int128)ax * ax + (__int128)ay * ay;
    __int128 bsq = (__int128)bx * bx + (__int128)by * by;
    __int128 csq = (__int128)cx * cx + (__int128)cy * cy;

    __int128 det =
        (__int128)ax * ((__int128)by * csq - (__int128)bsq * cy)
        - (__int128)ay * ((__int128)bx * csq - (__int128)bsq * cx)
        + asq * ((__int128)bx * cy - (__int128)by * cx);

    return det;
}

int locatePoint(int idx, int start, vector<Triangle>& tris) {
    int cur = start;

    while (true) {
        Triangle& t = tris[cur];
        bool moved = false;

        for (int i = 0; i < 3; ++i) {
            int a = t.v[i];
            int b = t.v[(i + 1) % 3];

            long long cr =
                (long long)(gpts[b].x - gpts[a].x) * (gpts[idx].y - gpts[a].y)
                - (long long)(gpts[b].y - gpts[a].y) * (gpts[idx].x - gpts[a].x);

            if (cr < 0) {
                int nb = t.n[i];
                if (nb != -1) {
                    cur = nb;
                    moved = true;
                    break;
                }
            }
        }

        if (!moved) return cur;
    }
}

void buildDelaunay(int n, vector<vector<int>>& adj) {
    int S = 1000000;

    gpts.resize(n + 3);
    gpts[n] = {-S, -S};
    gpts[n + 1] = {S, -S};
    gpts[n + 2] = {0, S};

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);

    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());
    shuffle(order.begin(), order.end(), rng);

    vector<Triangle> tris;
    tris.reserve(max(1000, n * 4));

    Triangle super(n, n + 1, n + 2);
    tris.push_back(super);

    int last = 0;

    vector<int> bad;
    vector<int> nextEdge(n + 3, -1);

    struct BoundaryEdge {
        int u, v;
        int nb;
        int oldTri;
    };

    vector<BoundaryEdge> bedges;
    vector<int> newTri;

    for (int idx : order) {
        int loc = locatePoint(idx, last, tris);

        bad.clear();
        bedges.clear();
        newTri.clear();

        tris[loc].bad = true;
        bad.push_back(loc);

        for (size_t ptr = 0; ptr < bad.size(); ++ptr) {
            int id = bad[ptr];
            Triangle& tr = tris[id];

            int v0 = tr.v[0], v1 = tr.v[1], v2 = tr.v[2];

            for (int i = 0; i < 3; ++i) {
                int nb = tr.n[i];
                if (nb != -1 && !tris[nb].bad) {
                    if (incircleValue(v0, v1, v2, idx) > 0) {
                        // Wait: this checks the current triangle, not nb.
                        // Correct check is below.
                    }
                }
            }

            for (int i = 0; i < 3; ++i) {
                int nb = tr.n[i];
                if (nb != -1 && !tris[nb].bad) {
                    Triangle& nt = tris[nb];
                    if (incircleValue(nt.v[0], nt.v[1], nt.v[2], idx) > 0) {
                        tris[nb].bad = true;
                        bad.push_back(nb);
                    }
                }
            }
        }

        for (int id : bad) {
            Triangle& tr = tris[id];

            for (int i = 0; i < 3; ++i) {
                int nb = tr.n[i];

                if (nb == -1 || !tris[nb].bad) {
                    bedges.push_back({tr.v[i], tr.v[(i + 1) % 3], nb, id});
                }
            }
        }

        for (int k = 0; k < (int)bedges.size(); ++k) {
            nextEdge[bedges[k].u] = k;
        }

        newTri.resize(bedges.size());

        for (int k = 0; k < (int)bedges.size(); ++k) {
            BoundaryEdge& e = bedges[k];

            int id = (int)tris.size();
            Triangle nt(e.u, e.v, idx);
            nt.n[0] = e.nb;

            tris.push_back(nt);
            newTri[k] = id;

            if (e.nb != -1) {
                Triangle& ot = tris[e.nb];
                for (int j = 0; j < 3; ++j) {
                    if (ot.n[j] == e.oldTri) {
                        ot.n[j] = id;
                        break;
                    }
                }
            }
        }

        for (int k = 0; k < (int)bedges.size(); ++k) {
            int v = bedges[k].v;
            int k2 = nextEdge[v];

            int id = newTri[k];
            int id2 = newTri[k2];

            tris[id].n[1] = id2;
            tris[id2].n[2] = id;
        }

        for (int id : bad) {
            tris[id].alive = false;
            tris[id].bad = false;
        }

        last = newTri[0];
    }

    adj.assign(n, {});

    for (const Triangle& tr : tris) {
        if (!tr.alive) continue;

        for (int i = 0; i < 3; ++i) {
            int a = tr.v[i];
            int b = tr.v[(i + 1) % 3];

            if (a < n && b < n) {
                adj[a].push_back(b);
                adj[b].push_back(a);
            }
        }
    }

    for (int i = 0; i < n; ++i) {
        sort(adj[i].begin(), adj[i].end());
        adj[i].erase(unique(adj[i].begin(), adj[i].end()), adj[i].end());
    }
}

/* ---------- Convex hull ---------- */

vector<int> convexHull(const vector<Point>& pts) {
    int n = (int)pts.size();
    vector<int> id(n);
    iota(id.begin(), id.end(), 0);

    sort(id.begin(), id.end(), [&](int a, int b) {
        if (pts[a].x != pts[b].x) return pts[a].x < pts[b].x;
        return pts[a].y < pts[b].y;
    });

    auto cross = [&](int o, int a, int b) -> long long {
        return (long long)(pts[a].x - pts[o].x) * (pts[b].y - pts[o].y)
             - (long long)(pts[a].y - pts[o].y) * (pts[b].x - pts[o].x);
    };

    vector<int> lower, upper;

    for (int x : id) {
        while (lower.size() >= 2 &&
               cross(lower[lower.size() - 2], lower.back(), x) <= 0) {
            lower.pop_back();
        }
        lower.push_back(x);
    }

    for (int i = n - 1; i >= 0; --i) {
        int x = id[i];

        while (upper.size() >= 2 &&
               cross(upper[upper.size() - 2], upper.back(), x) <= 0) {
            upper.pop_back();
        }
        upper.push_back(x);
    }

    lower.pop_back();
    upper.pop_back();

    lower.insert(lower.end(), upper.begin(), upper.end());

    if (lower.empty()) lower.push_back(id[0]);

    return lower;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int xl, yl, xr, yr;
    cin >> xl >> yl >> xr >> yr;

    int n;
    cin >> n;

    vector<Point> pts(n);
    for (int i = 0; i < n; ++i) {
        cin >> pts[i].x >> pts[i].y;
    }

    gpts = pts;

    vector<vector<int>> adj;

    bool allCollinear = true;

    if (n >= 2) {
        long long dx = pts[1].x - pts[0].x;
        long long dy = pts[1].y - pts[0].y;

        for (int i = 2; i < n; ++i) {
            long long vx = pts[i].x - pts[0].x;
            long long vy = pts[i].y - pts[0].y;

            if (dx * vy - dy * vx != 0) {
                allCollinear = false;
                break;
            }
        }
    }

    vector<int> hull;

    if (allCollinear) {
        long long dx = pts[1].x - pts[0].x;
        long long dy = pts[1].y - pts[0].y;

        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);

        sort(ord.begin(), ord.end(), [&](int a, int b) {
            long long da = (long long)(pts[a].x - pts[0].x) * dx
                         + (long long)(pts[a].y - pts[0].y) * dy;
            long long db = (long long)(pts[b].x - pts[0].x) * dx
                         + (long long)(pts[b].y - pts[0].y) * dy;
            return da < db;
        });

        adj.assign(n, {});

        for (int i = 1; i < n; ++i) {
            int a = ord[i - 1];
            int b = ord[i];
            adj[a].push_back(b);
            adj[b].push_back(a);
        }

        hull.push_back(ord.front());
        if (n >= 2) hull.push_back(ord.back());
    } else {
        buildDelaunay(n, adj);
        hull = convexHull(pts);
    }

    Poly rect = {
        {(long double)xl, (long double)yl},
        {(long double)xr, (long double)yl},
        {(long double)xr, (long double)yr},
        {(long double)xl, (long double)yr}
    };

    long double nearestIntegral = 0;

    for (int i = 0; i < n; ++i) {
        Poly poly = rect;

        long double xi = pts[i].x;
        long double yi = pts[i].y;

        for (int j : adj[i]) {
            long double xj = pts[j].x;
            long double yj = pts[j].y;

            // |Q-p_i|^2 <= |Q-p_j|^2
            long double A = 2.0L * (xi - xj);
            long double B = 2.0L * (yi - yj);
            long double C = (xj * xj + yj * yj) - (xi * xi + yi * yi);

            poly = clipPolygon(poly, A, B, C);

            if (poly.empty()) break;
        }

        nearestIntegral += integrateDistanceSquared(poly, xi, yi);
    }

    long double farthestIntegral = 0;

    int h = (int)hull.size();

    auto makeLine = [&](long double A, long double B, long double C) -> HLine {
        long double ang = atan2l(B, A);
        if (ang < 0) ang += 2.0L * PI;
        return {A, B, C, ang};
    };

    for (int i = 0; i < h; ++i) {
        vector<HLine> lines;
        lines.reserve(h + 4);

        // Rectangle constraints.
        lines.push_back(makeLine(1.0L, 0.0L, -(long double)xl));
        lines.push_back(makeLine(-1.0L, 0.0L, (long double)xr));
        lines.push_back(makeLine(0.0L, 1.0L, -(long double)yl));
        lines.push_back(makeLine(0.0L, -1.0L, (long double)yr));

        Point pi = pts[hull[i]];
        long double xi = pi.x;
        long double yi = pi.y;
        long double pi2 = xi * xi + yi * yi;

        for (int j = 0; j < h; ++j) {
            if (i == j) continue;

            Point pj = pts[hull[j]];
            long double xj = pj.x;
            long double yj = pj.y;

            // |Q-p_i|^2 >= |Q-p_j|^2
            long double A = 2.0L * (xj - xi);
            long double B = 2.0L * (yj - yi);
            long double C = pi2 - (xj * xj + yj * yj);

            lines.push_back(makeLine(A, B, C));
        }

        Poly poly = halfPlaneIntersection(lines);

        farthestIntegral += integrateDistanceSquared(poly, xi, yi);
    }

    long double rectArea = (long double)(xr - xl) * (long double)(yr - yl);

    long double ans = PI * (farthestIntegral - nearestIntegral) / rectArea;

    if (ans < 0 && ans > -1e-9L) ans = 0;
    if (ans < 0) ans = 0;

    cout << fixed << setprecision(15) << (double)ans << '\n';

    return 0;
}

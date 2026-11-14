#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using ld = long double;

struct Pt {
    i64 x, y;
};

static inline i64 cross2(const Pt& a, const Pt& b, const Pt& c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

struct HPoint {
    i64 x, y, z;
    int id;
};

static inline i64 orient3(const HPoint& a, const HPoint& b,
                          const HPoint& c, const HPoint& d) {
    i64 ux = b.x - a.x, uy = b.y - a.y, uz = b.z - a.z;
    i64 vx = c.x - a.x, vy = c.y - a.y, vz = c.z - a.z;
    i64 wx = d.x - a.x, wy = d.y - a.y, wz = d.z - a.z;

    i64 cx = uy * vz - uz * vy;
    i64 cy = uz * vx - ux * vz;
    i64 cz = ux * vy - uy * vx;
    return cx * wx + cy * wy + cz * wz;
}

struct Face {
    array<int, 3> v{};
    array<int, 3> adj{};
    vector<int> conflict;
    int tag = 0;
    bool alive = true;
};

struct HorizonEdge {
    int a, b;
    int oldFace, oldEdge;
    int newFace;
};

// Randomized incremental 3D convex hull. If all points are coplanar,
// returns an empty face list and sets coplanar=true.
static vector<Face> buildHull3D(const vector<HPoint>& p, bool& coplanar) {
    const int n = (int)p.size();
    coplanar = false;

    int i0 = 0, i1 = 0;
    auto lexLess = [&](int a, int b) {
        if (p[a].x != p[b].x) return p[a].x < p[b].x;
        if (p[a].y != p[b].y) return p[a].y < p[b].y;
        return p[a].z < p[b].z;
    };
    for (int i = 1; i < n; ++i) {
        if (lexLess(i, i0)) i0 = i;
        if (lexLess(i1, i)) i1 = i;
    }

    int i2 = -1;
    ld bestDist = -1;
    HPoint u{p[i1].x - p[i0].x, p[i1].y - p[i0].y,
             p[i1].z - p[i0].z, 0};
    for (int i = 0; i < n; ++i) {
        if (i == i0 || i == i1) continue;
        HPoint v{p[i].x - p[i0].x, p[i].y - p[i0].y,
                 p[i].z - p[i0].z, 0};
        i64 cx = u.y * v.z - u.z * v.y;
        i64 cy = u.z * v.x - u.x * v.z;
        i64 cz = u.x * v.y - u.y * v.x;
        ld d2 = (ld)cx * cx + (ld)cy * cy + (ld)cz * cz;
        if (d2 > bestDist) {
            bestDist = d2;
            i2 = i;
        }
    }

    if (i2 == -1 || bestDist == 0) {
        coplanar = true;
        return {};
    }

    int i3 = -1;
    i64 bestOrient = 0;
    for (int i = 0; i < n; ++i) {
        if (i == i0 || i == i1 || i == i2) continue;
        i64 val = orient3(p[i0], p[i1], p[i2], p[i]);
        i64 aval = val >= 0 ? val : -val;
        if (aval > bestOrient) {
            bestOrient = aval;
            i3 = i;
        }
    }

    if (i3 == -1 || bestOrient == 0) {
        coplanar = true;
        return {};
    }

    int tv[4] = {i0, i1, i2, i3};
    vector<char> inTetra(n, false);
    for (int x : tv) inTetra[x] = true;

    vector<Face> faces;
    faces.reserve(max(16, 4 * n));

    const int defs[4][4] = {
        {0, 1, 2, 3},
        {0, 1, 3, 2},
        {0, 2, 3, 1},
        {1, 2, 3, 0}
    };

    for (auto& d : defs) {
        int a = tv[d[0]], b = tv[d[1]], c = tv[d[2]], opp = tv[d[3]];
        if (orient3(p[a], p[b], p[c], p[opp]) > 0) swap(b, c);
        Face f;
        f.v = {a, b, c};
        f.adj = {-1, -1, -1};
        faces.push_back(std::move(f));
    }

    auto sameEdge = [&](int f, int e, int a, int b) {
        int x = faces[f].v[e];
        int y = faces[f].v[(e + 1) % 3];
        return (x == a && y == b) || (x == b && y == a);
    };

    for (int f = 0; f < 4; ++f) {
        for (int g = f + 1; g < 4; ++g) {
            for (int ef = 0; ef < 3; ++ef) {
                int a = faces[f].v[ef];
                int b = faces[f].v[(ef + 1) % 3];
                for (int eg = 0; eg < 3; ++eg) {
                    if (sameEdge(g, eg, a, b)) {
                        faces[f].adj[ef] = g;
                        faces[g].adj[eg] = f;
                    }
                }
            }
        }
    }

    auto orientFace = [&](int f, int point) -> i64 {
        const Face& F = faces[f];
        return orient3(p[F.v[0]], p[F.v[1]], p[F.v[2]], p[point]);
    };

    vector<int> conflictFace(n, -1);
    for (int i = 0; i < n; ++i) {
        if (inTetra[i]) continue;
        int best = -1;
        i64 bestVal = 0;
        for (int f = 0; f < 4; ++f) {
            i64 val = orientFace(f, i);
            if (val > bestVal) {
                bestVal = val;
                best = f;
            }
        }
        if (best != -1) {
            conflictFace[i] = best;
            faces[best].conflict.push_back(i);
        }
    }

    vector<int> order;
    order.reserve(n);
    for (int i = 0; i < n; ++i) {
        if (!inTetra[i]) order.push_back(i);
    }
    mt19937 rng(0x71A3B29u);
    shuffle(order.begin(), order.end(), rng);

    vector<int> outgoing(n, -1);
    vector<int> visible, dfs, candidates;
    vector<HorizonEdge> horizon;
    candidates.reserve(n);

    int stamp = 0;

    for (int point : order) {
        int start = conflictFace[point];
        if (start < 0) continue;

        ++stamp;
        visible.clear();
        dfs.clear();
        dfs.push_back(start);
        faces[start].tag = stamp;

        while (!dfs.empty()) {
            int f = dfs.back();
            dfs.pop_back();
            visible.push_back(f);

            for (int e = 0; e < 3; ++e) {
                int g = faces[f].adj[e];
                if (g < 0 || !faces[g].alive || faces[g].tag == stamp) continue;
                if (orientFace(g, point) > 0) {
                    faces[g].tag = stamp;
                    dfs.push_back(g);
                }
            }
        }

        horizon.clear();
        for (int f : visible) {
            for (int e = 0; e < 3; ++e) {
                int g = faces[f].adj[e];
                if (g >= 0 && faces[g].tag == stamp) continue;

                int a = faces[f].v[e];
                int b = faces[f].v[(e + 1) % 3];
                int ge = -1;
                for (int k = 0; k < 3; ++k) {
                    if (sameEdge(g, k, a, b)) {
                        ge = k;
                        break;
                    }
                }
                horizon.push_back({a, b, g, ge, -1});
            }
        }

        candidates.clear();
        for (int f : visible) {
            auto& cf = faces[f].conflict;
            candidates.insert(candidates.end(), cf.begin(), cf.end());
            vector<int>().swap(cf);
            faces[f].alive = false;
        }

        for (int h = 0; h < (int)horizon.size(); ++h) {
            auto& he = horizon[h];
            Face nf;
            nf.v = {he.a, he.b, point};
            nf.adj = {he.oldFace, -1, -1};

            int nfIndex = (int)faces.size();
            faces.push_back(std::move(nf));
            he.newFace = nfIndex;
            faces[he.oldFace].adj[he.oldEdge] = nfIndex;
            outgoing[he.a] = h;
        }

        // Consecutive horizon edges (a,b), (b,c) produce new faces
        // sharing the edge (b,point).
        for (int h = 0; h < (int)horizon.size(); ++h) {
            int next = outgoing[horizon[h].b];
            int f = horizon[h].newFace;
            int g = horizon[next].newFace;
            faces[f].adj[1] = g;
            faces[g].adj[2] = f;
        }

        for (int q : candidates) {
            if (q == point) continue;
            conflictFace[q] = -1;

            int best = -1;
            i64 bestVal = 0;
            for (const auto& he : horizon) {
                i64 val = orientFace(he.newFace, q);
                if (val > bestVal) {
                    bestVal = val;
                    best = he.newFace;
                }
            }
            if (best != -1) {
                conflictFace[q] = best;
                faces[best].conflict.push_back(q);
            }
        }
        conflictFace[point] = -1;
    }

    return faces;
}

struct P2 {
    ld x = 0, y = 0;
};

struct Line {
    // Inside: a*x + b*y + c >= 0.
    ld a, b, c;
    ld angle = 0;
};

struct Integrator {
    static constexpr ld EPS = 1e-10L;

    const vector<Pt>& p;
    vector<i64> norm;
    ld xl, yl, xr, yr;
    vector<Line> rectLines;
    vector<P2> rectPoly;

    Integrator(const vector<Pt>& pts, ld xl_, ld yl_, ld xr_, ld yr_)
        : p(pts), xl(xl_), yl(yl_), xr(xr_), yr(yr_) {
        norm.resize(p.size());
        for (int i = 0; i < (int)p.size(); ++i) {
            norm[i] = p[i].x * p[i].x + p[i].y * p[i].y;
        }

        rectLines = {
            {1, 0, -xl, 0},
            {-1, 0, xr, 0},
            {0, 1, -yl, 0},
            {0, -1, yr, 0}
        };
        rectPoly = {{xl, yl}, {xr, yl}, {xr, yr}, {xl, yr}};
    }

    static bool inside(const Line& l, const P2& q) {
        return l.a * q.x + l.b * q.y + l.c >= -EPS;
    }

    static vector<P2> clipPolygon(vector<P2> cur, const vector<Line>& lines) {
        for (const Line& l : lines) {
            if (cur.empty()) break;
            vector<P2> nxt;
            int m = (int)cur.size();

            for (int i = 0; i < m; ++i) {
                P2 s = cur[i], e = cur[(i + 1) % m];
                ld fs = l.a * s.x + l.b * s.y + l.c;
                ld fe = l.a * e.x + l.b * e.y + l.c;
                bool ins = fs >= -EPS;
                bool ine = fe >= -EPS;

                if (ins) nxt.push_back(s);
                if (ins != ine) {
                    ld t = fs / (fs - fe);
                    nxt.push_back({s.x + t * (e.x - s.x),
                                   s.y + t * (e.y - s.y)});
                }
            }
            cur.swap(nxt);
        }
        return cur;
    }

    static bool sameDirection(const Line& a, const Line& b) {
        ld cr = a.a * b.b - a.b * b.a;
        ld dt = a.a * b.a + a.b * b.b;
        return fabsl(cr) < 1e-12L && dt > 0;
    }

    static P2 lineIntersection(const Line& l1, const Line& l2) {
        ld det = l1.a * l2.b - l2.a * l1.b;
        return {
            (l1.b * l2.c - l2.b * l1.c) / det,
            (l2.a * l1.c - l1.a * l2.c) / det
        };
    }

    // Returns ok=false only for a numerical/degenerate fallback case.
    static pair<bool, vector<P2>> halfplaneIntersection(vector<Line> lines) {
        const ld PI = acosl(-1.0L);
        const ld TWO_PI = 2 * PI;

        for (Line& l : lines) {
            // Direction of the boundary line is (b,-a).
            l.angle = atan2l(-l.a, l.b);
            if (l.angle < 0) l.angle += TWO_PI;
        }
        sort(lines.begin(), lines.end(), [](const Line& a, const Line& b) {
            if (a.angle != b.angle) return a.angle < b.angle;
            return a.c < b.c;
        });

        vector<Line> v;
        for (const Line& l : lines) {
            if (!v.empty() && sameDirection(v.back(), l)) {
                // With equal unit normals, smaller c is more restrictive.
                v.back().c = min(v.back().c, l.c);
            } else {
                v.push_back(l);
            }
        }
        if (v.size() >= 2 && sameDirection(v.front(), v.back())) {
            v.front().c = min(v.front().c, v.back().c);
            v.pop_back();
        }

        int m = (int)v.size();
        if (m < 3) return {true, {}};

        vector<Line> dq(m + 1);
        vector<P2> ip(m + 1);
        int head = 0, tail = 0;

        for (const Line& l : v) {
            while (tail - head >= 2 && !inside(l, ip[tail - 2])) --tail;
            while (tail - head >= 2 && !inside(l, ip[head])) ++head;

            if (tail > head) {
                ld det = dq[tail - 1].a * l.b - l.a * dq[tail - 1].b;
                if (fabsl(det) < 1e-18L) {
                    return {false, {}};
                }
                ip[tail - 1] = lineIntersection(dq[tail - 1], l);
            }
            dq[tail++] = l;
        }

        while (tail - head > 2 && !inside(dq[head], ip[tail - 2])) --tail;
        while (tail - head > 2 && !inside(dq[tail - 1], ip[head])) ++head;

        if (tail - head < 3) return {true, {}};

        ld det = dq[tail - 1].a * dq[head].b
               - dq[head].a * dq[tail - 1].b;
        if (fabsl(det) < 1e-18L) return {false, {}};

        vector<P2> poly;
        poly.reserve(tail - head);
        for (int i = head; i < tail - 1; ++i) poly.push_back(ip[i]);
        poly.push_back(lineIntersection(dq[tail - 1], dq[head]));
        return {true, std::move(poly)};
    }

    vector<P2> getCell(int i, const vector<int>& neighbors, bool upper) {
        vector<Line> bisectors;
        bisectors.reserve(neighbors.size());

        for (int j : neighbors) {
            ld a = 2.0L * (ld)(p[j].x - p[i].x);
            ld b = 2.0L * (ld)(p[j].y - p[i].y);
            ld c = (ld)(norm[i] - norm[j]);

            // Upper: ell_i-ell_j >= 0. Lower: the opposite.
            if (!upper) {
                a = -a;
                b = -b;
                c = -c;
            }

            ld len = sqrtl(a * a + b * b);
            bisectors.push_back({a / len, b / len, c / len, 0});
        }

        if (bisectors.size() <= 8) {
            return clipPolygon(rectPoly, bisectors);
        }

        vector<Line> all = rectLines;
        all.insert(all.end(), bisectors.begin(), bisectors.end());
        auto result = halfplaneIntersection(std::move(all));

        if (!result.first) {
            return clipPolygon(rectPoly, bisectors);
        }
        return std::move(result.second);
    }

    ld integrate(const vector<vector<int>>& adjacency, bool upper) {
        ld total = 0;

        for (int i = 0; i < (int)p.size(); ++i) {
            if (adjacency[i].empty()) continue;

            vector<P2> poly = getCell(i, adjacency[i], upper);
            if (poly.size() < 3) continue;

            ld a2 = 0, sx = 0, sy = 0;
            int m = (int)poly.size();
            for (int k = 0; k < m; ++k) {
                const P2& a = poly[k];
                const P2& b = poly[(k + 1) % m];
                ld cr = a.x * b.y - b.x * a.y;
                a2 += cr;
                sx += (a.x + b.x) * cr;
                sy += (a.y + b.y) * cr;
            }

            if (fabsl(a2) < 1e-18L) continue;
            if (a2 < 0) {
                a2 = -a2;
                sx = -sx;
                sy = -sy;
            }

            ld area = a2 / 2;
            ld intX = sx / 6;
            ld intY = sy / 6;

            // Subtract ell_0 from every function for better numerical stability.
            // This does not change max(ell)-min(ell).
            ld A = 2.0L * (ld)(p[0].x - p[i].x);
            ld B = 2.0L * (ld)(p[0].y - p[i].y);
            ld C = (ld)(norm[i] - norm[0]);

            total += A * intX + B * intY + C * area;
        }
        return total;
    }
};

static vector<int> convexHull2D(const vector<Pt>& p) {
    int n = (int)p.size();
    vector<int> id(n);
    iota(id.begin(), id.end(), 0);
    sort(id.begin(), id.end(), [&](int a, int b) {
        if (p[a].x != p[b].x) return p[a].x < p[b].x;
        return p[a].y < p[b].y;
    });

    vector<int> h(2 * n);
    int k = 0;
    for (int x : id) {
        while (k >= 2 && cross2(p[h[k - 2]], p[h[k - 1]], p[x]) <= 0) --k;
        h[k++] = x;
    }
    int lowerSize = k;
    for (int i = n - 2; i >= 0; --i) {
        int x = id[i];
        while (k > lowerSize &&
               cross2(p[h[k - 2]], p[h[k - 1]], p[x]) <= 0) --k;
        h[k++] = x;
    }
    h.resize(k - 1);
    return h;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ld xl, yl, xr, yr;
    if (!(cin >> xl >> yl >> xr >> yr)) return 0;

    int n;
    cin >> n;
    vector<Pt> p(n);
    for (auto& q : p) cin >> q.x >> q.y;

    vector<vector<int>> lowerAdj(n), upperAdj(n);

    Pt dir{p[1].x - p[0].x, p[1].y - p[0].y};
    bool collinear = true;
    for (int i = 1; i < n; ++i) {
        if (cross2(p[0], p[1], p[i]) != 0) {
            collinear = false;
            break;
        }
    }

    if (collinear) {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            i64 ka = p[a].x * dir.x + p[a].y * dir.y;
            i64 kb = p[b].x * dir.x + p[b].y * dir.y;
            return ka < kb;
        });

        for (int k = 0; k < n; ++k) {
            if (k > 0) lowerAdj[ord[k]].push_back(ord[k - 1]);
            if (k + 1 < n) lowerAdj[ord[k]].push_back(ord[k + 1]);
        }

        upperAdj[ord.front()].push_back(ord.back());
        upperAdj[ord.back()].push_back(ord.front());
    } else {
        vector<HPoint> lifted(n);
        for (int i = 0; i < n; ++i) {
            lifted[i] = {
                -2 * p[i].x,
                -2 * p[i].y,
                p[i].x * p[i].x + p[i].y * p[i].y,
                i
            };
        }

        bool coplanar;
        vector<Face> faces = buildHull3D(lifted, coplanar);

        if (coplanar) {
            // Noncollinear targets with coplanar lifted points are cocircular.
            // Both Voronoi diagrams consist of wedges between cyclic neighbors.
            vector<int> h = convexHull2D(p);
            int m = (int)h.size();
            for (int k = 0; k < m; ++k) {
                int prv = h[(k + m - 1) % m];
                int cur = h[k];
                int nxt = h[(k + 1) % m];
                lowerAdj[cur].push_back(prv);
                lowerAdj[cur].push_back(nxt);
                upperAdj[cur].push_back(prv);
                upperAdj[cur].push_back(nxt);
            }
        } else {
            for (const Face& f : faces) {
                if (!f.alive) continue;

                const HPoint& a = lifted[f.v[0]];
                const HPoint& b = lifted[f.v[1]];
                const HPoint& c = lifted[f.v[2]];

                i64 ux = b.x - a.x, uy = b.y - a.y;
                i64 vx = c.x - a.x, vy = c.y - a.y;
                i64 nz = ux * vy - uy * vx;
                if (nz == 0) continue;

                auto& adj = (nz > 0 ? upperAdj : lowerAdj);
                for (int e = 0; e < 3; ++e) {
                    int u = lifted[f.v[e]].id;
                    int v = lifted[f.v[(e + 1) % 3]].id;
                    adj[u].push_back(v);
                    adj[v].push_back(u);
                }
            }

            for (int type = 0; type < 2; ++type) {
                auto& adj = (type == 0 ? lowerAdj : upperAdj);
                for (auto& v : adj) {
                    sort(v.begin(), v.end());
                    v.erase(unique(v.begin(), v.end()), v.end());
                }
            }
        }
    }

    Integrator integrator(p, xl, yl, xr, yr);
    ld upperIntegral = integrator.integrate(upperAdj, true);
    ld lowerIntegral = integrator.integrate(lowerAdj, false);

    ld rectArea = (xr - xl) * (yr - yl);
    ld answer = acosl(-1.0L) * (upperIntegral - lowerIntegral) / rectArea;
    if (answer < 0 && answer > -1e-8L) answer = 0;

    cout << setprecision(20) << answer << '\n';
    return 0;
}

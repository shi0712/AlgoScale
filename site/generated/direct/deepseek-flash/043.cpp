#include <bits/stdc++.h>
using namespace std;

using ld = long double;

struct Point {
    ld x, y;
    Point() : x(0), y(0) {}
    Point(ld _x, ld _y) : x(_x), y(_y) {}

    Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
    Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
    Point operator*(ld k) const { return {x * k, y * k}; }
};

static inline ld cross(const Point& a, const Point& b) {
    return a.x * b.y - a.y * b.x;
}

static inline ld cross(const Point& a, const Point& b, const Point& c) {
    return cross(b - a, c - a);
}

static inline ld dist2(const Point& a, const Point& b) {
    ld dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

ld polygonAreaAbs(const vector<Point>& p) {
    int n = (int)p.size();
    if (n < 3) return 0;
    ld s = 0;
    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        s += cross(p[i], p[j]);
    }
    return fabsl(s) * 0.5L;
}

vector<Point> getVisibilityPolygon(const Point& S, const vector<Point>& P) {
    int n = (int)P.size();
    const ld PI = acosl(-1.0L);
    const ld TWO_PI = 2.0L * PI;

    vector<ld> angs;
    angs.reserve(n);

    for (const auto& v : P) {
        ld a = atan2l(v.y - S.y, v.x - S.x);
        if (a < 0) a += TWO_PI;
        angs.push_back(a);
    }

    sort(angs.begin(), angs.end());

    vector<ld> uniq;
    for (ld a : angs) {
        if (uniq.empty() || fabsl(a - uniq.back()) > 1e-14L) {
            uniq.push_back(a);
        }
    }
    angs.swap(uniq);

    int K = (int)angs.size();
    vector<Point> pts;

    auto addPoint = [&](const Point& p) {
        if (pts.empty() || dist2(pts.back(), p) > 1e-26L) {
            pts.push_back(p);
        }
    };

    for (int k = 0; k < K; ++k) {
        ld a1 = angs[k];
        ld a2 = angs[(k + 1) % K];
        if (k == K - 1) a2 += TWO_PI;

        ld amid = (a1 + a2) * 0.5L;
        Point dmid(cosl(amid), sinl(amid));

        int bestEdge = -1;
        ld bestT = 1e100L;

        for (int e = 0; e < n; ++e) {
            const Point& A = P[e];
            const Point& B = P[(e + 1) % n];

            Point ev = B - A;
            ld den = cross(dmid, ev);
            if (fabsl(den) < 1e-24L) continue;

            Point as = A - S;
            ld t = cross(as, ev) / den;
            ld u = cross(as, dmid) / den;

            if (t > 1e-20L && u >= -1e-12L && u <= 1.0L + 1e-12L) {
                if (t < bestT) {
                    bestT = t;
                    bestEdge = e;
                }
            }
        }

        if (bestEdge == -1) continue;

        const Point& A = P[bestEdge];
        const Point& B = P[(bestEdge + 1) % n];
        Point ev = B - A;

        auto rayIntersectLine = [&](ld ang) -> Point {
            Point d(cosl(ang), sinl(ang));
            ld den = cross(d, ev);
            Point q = A - S;
            ld t = cross(q, ev) / den;
            return Point(S.x + t * d.x, S.y + t * d.y);
        };

        Point p1 = rayIntersectLine(a1);
        Point p2 = rayIntersectLine(a2);

        addPoint(p1);
        addPoint(p2);
    }

    if (pts.size() > 1 && dist2(pts.front(), pts.back()) < 1e-26L) {
        pts.pop_back();
    }

    return pts;
}

struct Edge {
    Point a, b;
    int poly;
    ld minx, maxx, miny, maxy;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Point> polygon(n);
    for (int i = 0; i < n; ++i) {
        long long x, y;
        cin >> x >> y;
        polygon[i] = Point((ld)x, (ld)y);
    }

    int m;
    cin >> m;

    set<pair<int, int>> sourceSet;
    for (int i = 0; i < m; ++i) {
        int a, b;
        cin >> a >> b;
        sourceSet.insert({a, b});
    }

    ld polygonArea = polygonAreaAbs(polygon);

    vector<vector<Point>> visPolys;

    for (auto [sx, sy] : sourceSet) {
        Point S((ld)sx, (ld)sy);
        vector<Point> vp = getVisibilityPolygon(S, polygon);

        if (vp.size() < 3) continue;

        ld area = polygonAreaAbs(vp);
        if (fabsl(area - polygonArea) <= 1e-9L * max<ld>(1.0L, polygonArea)) {
            cout << fixed << setprecision(12) << (double)polygonArea << '\n';
            return 0;
        }

        visPolys.push_back(move(vp));
    }

    int pm = (int)visPolys.size();
    if (pm == 0) {
        cout << fixed << setprecision(12) << 0.0 << '\n';
        return 0;
    }

    vector<Edge> segs;
    vector<ld> events;

    for (int pid = 0; pid < pm; ++pid) {
        const vector<Point>& vp = visPolys[pid];
        int sz = (int)vp.size();

        for (const auto& p : vp) {
            events.push_back(p.x);
        }

        for (int i = 0; i < sz; ++i) {
            Point A = vp[i];
            Point B = vp[(i + 1) % sz];

            if (fabsl(A.x - B.x) > 1e-18L) {
                Edge e;
                e.a = A;
                e.b = B;
                e.poly = pid;
                e.minx = min(A.x, B.x);
                e.maxx = max(A.x, B.x);
                e.miny = min(A.y, B.y);
                e.maxy = max(A.y, B.y);
                segs.push_back(e);
            }
        }
    }

    int N = (int)segs.size();

    if (N > 0) {
        const ld CELL = 5.0L;
        const ld OFF = 150.0L;

        auto makeKey = [](int ix, int iy) -> unsigned long long {
            uint64_t ux = (uint32_t)ix;
            uint64_t uy = (uint32_t)iy;
            return (ux << 32) | uy;
        };

        auto getCells = [&](Point A, Point B) {
            vector<unsigned long long> cells;

            if (fabsl(B.x - A.x) < 1e-30L) {
                int ix = (int)floorl((A.x + OFF) / CELL);
                int iy1 = (int)floorl((min(A.y, B.y) + OFF) / CELL) - 1;
                int iy2 = (int)floorl((max(A.y, B.y) + OFF) / CELL) + 1;

                for (int iy = iy1; iy <= iy2; ++iy) {
                    cells.push_back(makeKey(ix, iy));
                }
            } else {
                if (A.x > B.x) swap(A, B);

                ld dx = B.x - A.x;
                ld dy = B.y - A.y;

                int ix1 = (int)floorl((A.x + OFF) / CELL);
                int ix2 = (int)floorl((B.x + OFF) / CELL);

                for (int ix = ix1; ix <= ix2; ++ix) {
                    ld xl = max(A.x, (ld)ix * CELL - OFF);
                    ld xr = min(B.x, (ld)(ix + 1) * CELL - OFF);
                    if (xl > xr + 1e-20L) continue;

                    ld yl = A.y + (xl - A.x) * dy / dx;
                    ld yr = A.y + (xr - A.x) * dy / dx;

                    ld ymin = min(yl, yr);
                    ld ymax = max(yl, yr);

                    int iy1 = (int)floorl((ymin + OFF) / CELL) - 1;
                    int iy2 = (int)floorl((ymax + OFF) / CELL) + 1;

                    for (int iy = iy1; iy <= iy2; ++iy) {
                        cells.push_back(makeKey(ix, iy));
                    }
                }
            }

            sort(cells.begin(), cells.end());
            cells.erase(unique(cells.begin(), cells.end()), cells.end());
            return cells;
        };

        unordered_map<unsigned long long, vector<int>> grid;
        grid.reserve(N * 4 + 10);

        vector<vector<unsigned long long>> edgeCells(N);

        for (int i = 0; i < N; ++i) {
            edgeCells[i] = getCells(segs[i].a, segs[i].b);
            for (auto key : edgeCells[i]) {
                grid[key].push_back(i);
            }
        }

        vector<int> mark(N, 0);
        int stamp = 0;

        const ld EPS_SEG = 1e-14L;

        for (int i = 0; i < N; ++i) {
            ++stamp;
            const Edge& ei = segs[i];

            for (auto key : edgeCells[i]) {
                auto it = grid.find(key);
                if (it == grid.end()) continue;

                for (int j : it->second) {
                    if (j <= i) continue;
                    if (mark[j] == stamp) continue;
                    mark[j] = stamp;

                    const Edge& ej = segs[j];

                    if (min(ei.maxx, ej.maxx) < max(ei.minx, ej.minx) - 1e-14L) {
                        continue;
                    }
                    if (min(ei.maxy, ej.maxy) < max(ei.miny, ej.miny) - 1e-14L) {
                        continue;
                    }

                    Point r = ei.b - ei.a;
                    Point s = ej.b - ej.a;

                    ld den = cross(r, s);
                    if (fabsl(den) < 1e-24L) continue;

                    Point q = ej.a - ei.a;

                    ld t = cross(q, s) / den;
                    ld u = cross(q, r) / den;

                    if (t > EPS_SEG && t < 1.0L - EPS_SEG &&
                        u > EPS_SEG && u < 1.0L - EPS_SEG) {
                        ld xx = ei.a.x + t * r.x;
                        events.push_back(xx);
                    }
                }
            }
        }
    }

    sort(events.begin(), events.end());

    const ld EPS_X = 1e-12L;

    vector<ld> xs;
    xs.reserve(events.size());

    for (ld x : events) {
        if (xs.empty() || fabsl(x - xs.back()) > EPS_X) {
            xs.push_back(x);
        }
    }

    if (xs.size() < 2 || N == 0) {
        cout << fixed << setprecision(12) << 0.0 << '\n';
        return 0;
    }

    vector<int> byMin(N), byMax(N);
    iota(byMin.begin(), byMin.end(), 0);
    iota(byMax.begin(), byMax.end(), 0);

    sort(byMin.begin(), byMin.end(), [&](int i, int j) {
        return segs[i].minx < segs[j].minx;
    });

    sort(byMax.begin(), byMax.end(), [&](int i, int j) {
        return segs[i].maxx < segs[j].maxx;
    });

    vector<int> active;
    active.reserve(N);

    vector<int> pos(N, -1);

    vector<vector<ld>> ysByPoly(pm);
    vector<int> usedPolys;
    usedPolys.reserve(pm);

    vector<pair<ld, ld>> intervals;
    intervals.reserve(N);

    auto calcUnionLength = [&](ld mid) -> ld {
        usedPolys.clear();
        intervals.clear();

        for (int id : active) {
            const Edge& e = segs[id];

            ld y = e.a.y + (mid - e.a.x) * (e.b.y - e.a.y) / (e.b.x - e.a.x);

            int p = e.poly;
            if (ysByPoly[p].empty()) {
                usedPolys.push_back(p);
            }
            ysByPoly[p].push_back(y);
        }

        for (int p : usedPolys) {
            auto& v = ysByPoly[p];
            sort(v.begin(), v.end());

            for (size_t k = 0; k + 1 < v.size(); k += 2) {
                ld y1 = v[k];
                ld y2 = v[k + 1];
                if (y1 > y2) swap(y1, y2);
                intervals.push_back({y1, y2});
            }

            v.clear();
        }

        if (intervals.empty()) return 0.0L;

        sort(intervals.begin(), intervals.end());

        ld total = 0.0L;
        ld curL = intervals[0].first;
        ld curR = intervals[0].second;

        for (size_t i = 1; i < intervals.size(); ++i) {
            ld l = intervals[i].first;
            ld r = intervals[i].second;

            if (l <= curR + 1e-13L) {
                if (r > curR) curR = r;
            } else {
                total += curR - curL;
                curL = l;
                curR = r;
            }
        }

        total += curR - curL;
        return total;
    };

    int sp = 0, ep = 0;
    ld answer = 0.0L;

    for (int i = 0; i + 1 < (int)xs.size(); ++i) {
        ld x = xs[i];

        while (ep < N && segs[byMax[ep]].maxx <= x + EPS_X) {
            int id = byMax[ep];
            if (pos[id] != -1) {
                int p = pos[id];
                int last = active.back();

                active[p] = last;
                pos[last] = p;

                active.pop_back();
                pos[id] = -1;
            }
            ++ep;
        }

        while (sp < N && segs[byMin[sp]].minx <= x + EPS_X) {
            int id = byMin[sp];

            if (pos[id] == -1 && segs[id].maxx > x + EPS_X) {
                pos[id] = (int)active.size();
                active.push_back(id);
            }

            ++sp;
        }

        ld nxt = xs[i + 1];

        if (nxt - x > EPS_X) {
            ld mid = (x + nxt) * 0.5L;
            ld len = calcUnionLength(mid);
            answer += len * (nxt - x);
        }
    }

    if (answer > polygonArea) answer = polygonArea;

    cout << fixed << setprecision(12) << (double)answer << '\n';

    return 0;
}

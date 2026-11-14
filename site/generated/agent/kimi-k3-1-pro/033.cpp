#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/tree_policy.hpp>
using namespace std;
using namespace __gnu_pbds;

using ll = long long;
using i128 = __int128_t;
using ld = long double;

struct Point {
    ll x, y;
    Point operator-(const Point& o) const { return {x - o.x, y - o.y}; }
};

static i128 cross(const Point& a, const Point& b, const Point& c) {
    return (i128)(b.x - a.x) * (c.y - a.y) - (i128)(b.y - a.y) * (c.x - a.x);
}

static i128 dot2(const Point& a) {
    return (i128)a.x * a.x + (i128)a.y * a.y;
}

static i128 inCircle(const Point& a, const Point& b, const Point& c, const Point& p) {
    Point aa = a - p, bb = b - p, cc = c - p;
    return dot2(aa) * ((i128)bb.x * cc.y - (i128)bb.y * cc.x)
         - dot2(bb) * ((i128)aa.x * cc.y - (i128)aa.y * cc.x)
         + dot2(cc) * ((i128)aa.x * bb.y - (i128)aa.y * bb.x);
}

struct Triangle {
    array<int, 3> v, nb;
    array<int, 3> ch;
    int nch = 0;
    bool active = true;
};

struct Delaunay {
    vector<Point> p;
    vector<Triangle> tr;
    int root;

    Delaunay(const vector<Point>& points) : p(points) {
        const ll M = 1000000LL;
        p.push_back({-M, -M});
        p.push_back({M, -M});
        p.push_back({0, M});
        root = makeTri(p.size() - 3, p.size() - 2, p.size() - 1);
    }

    int makeTri(int a, int b, int c) {
        if (cross(p[a], p[b], p[c]) < 0) swap(b, c);
        Triangle t;
        t.v = {a, b, c};
        t.nb = {-1, -1, -1};
        t.ch = {-1, -1, -1};
        tr.push_back(t);
        return tr.size() - 1;
    }

    void addChild(int t, int c) {
        tr[t].ch[tr[t].nch++] = c;
    }

    void link(int t, int i, int u, int j) {
        tr[t].nb[i] = u;
        if (u >= 0) tr[u].nb[j] = t;
    }

    int otherEdge(int u, int t) const {
        for (int j = 0; j < 3; ++j) if (tr[u].nb[j] == t) return j;
        return -1;
    }

    int locate(int id) const {
        int t = root;
        while (!tr[t].active) {
            int nt = -1;
            for (int k = 0; k < tr[t].nch; ++k) {
                int c = tr[t].ch[k];
                bool ok = true;
                for (int i = 0; i < 3; ++i) {
                    if (cross(p[tr[c].v[i]], p[tr[c].v[(i + 1) % 3]], p[id]) < 0) {
                        ok = false;
                        break;
                    }
                }
                if (ok) { nt = c; break; }
            }
            if (nt < 0) return -1;
            t = nt;
        }
        return t;
    }

    void legalize(stack<pair<int, int>>& st) {
        while (!st.empty()) {
            auto [t, i] = st.top(); st.pop();
            int u = tr[t].nb[i];
            if (u < 0) continue;
            int j = otherEdge(u, t);
            int a = tr[t].v[i];
            int b = tr[t].v[(i + 1) % 3];
            int c = tr[t].v[(i + 2) % 3];
            int d = tr[u].v[j];
            if (cross(p[a], p[b], p[d]) <= 0 || cross(p[a], p[d], p[c]) <= 0) continue;
            if (inCircle(p[a], p[b], p[c], p[d]) <= 0) continue;


            int tca = tr[t].nb[(i + 1) % 3];
            int tab = tr[t].nb[(i + 2) % 3];
            int ubd = tr[u].nb[(j + 1) % 3];
            int udc = tr[u].nb[(j + 2) % 3];

            int x = makeTri(a, b, d);
            int y = makeTri(a, d, c);
            auto attach = [&](int z, int ei, int old) {
                if (old < 0) return;
                int r1 = tr[z].v[(ei + 1) % 3], r2 = tr[z].v[(ei + 2) % 3];
                for (int k = 0; k < 3; ++k) {
                    int q1 = tr[old].v[(k + 1) % 3], q2 = tr[old].v[(k + 2) % 3];
                    if (q1 == r2 && q2 == r1) {
                        link(z, ei, old, k);
                        break;
                    }
                }
            };
            attach(x, 0, ubd);
            attach(x, 2, tab);
            attach(y, 0, udc);
            attach(y, 1, tca);
            link(x, 1, y, 2);

            tr[t].active = tr[u].active = false;
            addChild(t, x); addChild(t, y);
            addChild(u, x); addChild(u, y);
            st.push({x, 0});
            st.push({y, 0});
        }
    }

    void insert(int id) {
        int t = locate(id);
        int on = -1;
        for (int i = 0; i < 3; ++i) {
            if (cross(p[tr[t].v[(i + 1) % 3]], p[tr[t].v[(i + 2) % 3]], p[id]) == 0) {
                on = i;
                break;
            }
        }
        stack<pair<int, int>> st;

        if (on < 0) {
            int a = tr[t].v[0], b = tr[t].v[1], c = tr[t].v[2];
            int nbc = tr[t].nb[0], nca = tr[t].nb[1], nab = tr[t].nb[2];
            int x = makeTri(a, b, id);
            int y = makeTri(b, c, id);
            int z = makeTri(c, a, id);
            auto attach = [&](int nt, int ei, int old) {
                if (old >= 0) {
                    int ej = otherEdge(old, t);
                    link(nt, ei, old, ej);
                }
            };
            attach(x, 2, nab);
            attach(y, 2, nbc);
            attach(z, 2, nca);
            link(x, 0, y, 1);
            link(y, 0, z, 1);
            link(z, 0, x, 1);
            tr[t].active = false;
            addChild(t, x); addChild(t, y); addChild(t, z);
            st.push({x, 2}); st.push({y, 2}); st.push({z, 2});
        } else {
            int u = tr[t].nb[on];
            int a = tr[t].v[on];
            int b = tr[t].v[(on + 1) % 3];
            int c = tr[t].v[(on + 2) % 3];
            int nab = tr[t].nb[(on + 2) % 3];
            int nca = tr[t].nb[(on + 1) % 3];
            int x = makeTri(a, b, id);
            int y = makeTri(id, c, a);
            auto attachOld = [&](int nt, int ei, int old, int oldt) {
                if (old >= 0) link(nt, ei, old, otherEdge(old, oldt));
            };
            attachOld(x, 2, nab, t);
            attachOld(y, 0, nca, t);
            if (u >= 0) {
                int j = otherEdge(u, t);
                int d = tr[u].v[j];
                int ndc = tr[u].nb[(j + 2) % 3];
                int ndb = tr[u].nb[(j + 1) % 3];
                int z = makeTri(d, c, id);
                int w = makeTri(id, b, d);

                attachOld(z, 2, ndc, u);
                attachOld(w, 0, ndb, u);
                link(x, 0, w, 2);
                link(w, 1, z, 1);
                link(z, 0, y, 2);
                link(y, 1, x, 1);
                tr[t].active = tr[u].active = false;
                addChild(t, x); addChild(t, y);
                addChild(u, z); addChild(u, w);
                st.push({x, 2}); st.push({y, 0}); st.push({z, 2}); st.push({w, 0});
            } else {
                link(x, 1, y, 1);
                tr[t].active = false;
                addChild(t, x); addChild(t, y);
                st.push({x, 2}); st.push({y, 0});
            }
        }
        legalize(st);
    }
};

struct Query {
    ld x, y;
    int id;
};

struct SweepEdge {
    int a, b;
};

static vector<Point> poly;
static vector<SweepEdge> sweepEdges;
static ld sweepY;

static ld edgeX(int id) {
    auto [a, b] = sweepEdges[id];
    ld ya = poly[a].y, yb = poly[b].y;
    return poly[a].x + (ld)(poly[b].x - poly[a].x) * (sweepY - ya) / (yb - ya);
}

struct SweepNode {
    int id;
    ld qx;
    bool query;
};

struct SweepCmp {
    bool operator()(const SweepNode& a, const SweepNode& b) const {
        ld xa = a.query ? a.qx : edgeX(a.id);
        ld xb = b.query ? b.qx : edgeX(b.id);
        if (xa != xb) return xa < xb;
        if (a.query != b.query) return a.query < b.query;
        return a.id < b.id;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    poly.resize(n);
    for (auto& q : poly) cin >> q.x >> q.y;

    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    mt19937 rng(712367821);
    shuffle(order.begin(), order.end(), rng);
    Delaunay del(poly);
    for (int id : order) del.insert(id);

    vector<int> realTris;
    vector<vector<int>> incident(n);
    for (int t = 0; t < (int)del.tr.size(); ++t) {
        if (!del.tr[t].active) continue;
        bool real = true;
        for (int v : del.tr[t].v) if (v >= n) real = false;
        if (!real) continue;
        realTris.push_back(t);
        for (int v : del.tr[t].v) incident[v].push_back(t);
    }



    vector<Query> queries;
    vector<pair<ld, ld>> centers(del.tr.size(), {0, 0});
    queries.reserve(realTris.size());
    vector<ld> candidate(realTris.size());
    for (int k = 0; k < (int)realTris.size(); ++k) {
        auto v = del.tr[realTris[k]].v;
        Point a = poly[v[0]], b = poly[v[1]], c = poly[v[2]];
        ld ux = (ld)b.x - a.x, uy = (ld)b.y - a.y;
        ld vx = (ld)c.x - a.x, vy = (ld)c.y - a.y;
        ld d = 2 * (ux * vy - uy * vx);
        ld b2 = ux * ux + uy * uy, c2 = vx * vx + vy * vy;
        ld cx = a.x + (vy * b2 - uy * c2) / d;
        ld cy = a.y + (ux * c2 - vx * b2) / d;
        queries.push_back({cx, cy, k});
        centers[realTris[k]] = {cx, cy};
        candidate[k] = hypotl(cx - a.x, cy - a.y);
    }

    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        if (poly[i].y != poly[j].y) sweepEdges.push_back({i, j});
    }
    vector<ld> ys;
    ys.reserve(2 * sweepEdges.size() + queries.size());
    for (auto e : sweepEdges) {
        ys.push_back(poly[e.a].y);
        ys.push_back(poly[e.b].y);
    }
    for (auto& q : queries) ys.push_back(q.y);
    sort(ys.begin(), ys.end());
    ys.erase(unique(ys.begin(), ys.end()), ys.end());
    int m = ys.size();
    vector<vector<int>> rem(m), add(m), qs(m);
    for (int i = 0; i < (int)sweepEdges.size(); ++i) {
        ll lo = min(poly[sweepEdges[i].a].y, poly[sweepEdges[i].b].y);
        ll hi = max(poly[sweepEdges[i].a].y, poly[sweepEdges[i].b].y);
        add[lower_bound(ys.begin(), ys.end(), (ld)lo) - ys.begin()].push_back(i);
        rem[lower_bound(ys.begin(), ys.end(), (ld)hi) - ys.begin()].push_back(i);
    }
    for (int i = 0; i < (int)queries.size(); ++i) {
        qs[lower_bound(ys.begin(), ys.end(), queries[i].y) - ys.begin()].push_back(i);
    }

    ld answer = 0;
    tree<SweepNode, null_type, SweepCmp, rb_tree_tag, tree_order_statistics_node_update> active;
    sweepY = ys.empty() ? 0 : ys[0];
    for (int k = 0; k < m; ++k) {
        if (k > 0) sweepY = (ys[k - 1] + ys[k]) / 2;
        for (int e : rem[k]) active.erase({e, 0, false});
        if (k + 1 < m) sweepY = (ys[k] + ys[k + 1]) / 2;
        for (int e : add[k]) active.insert({e, 0, false});
        sweepY = ys[k];
        for (int qi : qs[k]) {
            auto& q = queries[qi];
            auto it = active.lower_bound({qi, q.x, true});
            if (it != active.end() && !it->query && edgeX(it->id) == q.x) {
                answer = max(answer, candidate[qi]);
                continue;
            }
            int cnt = active.size() - active.order_of_key({qi, q.x, true});
            if (cnt & 1) answer = max(answer, candidate[qi]);
        }
    }

    auto boundaryCandidate = [&](int a, int b, int c, int d) {
        Point P = poly[a];
        ld dx = (ld)poly[b].x - P.x, dy = (ld)poly[b].y - P.y;
        ld ex = (ld)poly[d].x - poly[c].x, ey = (ld)poly[d].y - poly[c].y;
        ld den = 2 * (dx * ex + dy * ey);
        if (den == 0) return;
        ld num = (ld)poly[d].x * poly[d].x + (ld)poly[d].y * poly[d].y
               - (ld)poly[c].x * poly[c].x - (ld)poly[c].y * poly[c].y
               - 2 * ((ld)P.x * ex + (ld)P.y * ey);
        ld t = num / den;
        if (t >= 0 && t <= 1) {
            ld x = P.x + t * dx - poly[c].x;
            ld y = P.y + t * dy - poly[c].y;
            answer = max(answer, hypotl(x, y));
        }
    };

    vector<vector<int>> neighbors(n);
    for (int t : realTris) {
        auto v = del.tr[t].v;
        for (int i = 0; i < 3; ++i) {
            neighbors[v[i]].push_back(v[(i + 1) % 3]);
            neighbors[v[i]].push_back(v[(i + 2) % 3]);
        }
    }
    for (auto& v : neighbors) {
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
    }

    auto voronoiWalk = [&](int a, int b) {
        int site = a;
        ld current = 0;
        for (int steps = 0; steps <= n; ++steps) {
            ld best = 2;
            int next = -1, tied = 0;
            for (int other : neighbors[site]) {
                Point P = poly[a];
                ld dx = (ld)poly[b].x - P.x, dy = (ld)poly[b].y - P.y;
                ld ex = (ld)poly[other].x - poly[site].x;
                ld ey = (ld)poly[other].y - poly[site].y;
                ld den = 2 * (dx * ex + dy * ey);
                if (den == 0) continue;
                ld num = (ld)poly[other].x * poly[other].x + (ld)poly[other].y * poly[other].y
                       - (ld)poly[site].x * poly[site].x - (ld)poly[site].y * poly[site].y
                       - 2 * ((ld)P.x * ex + (ld)P.y * ey);
                ld t = num / den;
                if (t > current + 1e-12L && t < best) {
                    if (t < best - 1e-12L) {
                        best = t;
                        next = other;
                        tied = 1;
                    } else {
                        ++tied;
                    }
                }
            }
            if (next < 0 || best > 1) break;

            ld dx = (ld)poly[b].x - poly[a].x, dy = (ld)poly[b].y - poly[a].y;
            ld px = poly[a].x + best * dx, py = poly[a].y + best * dy;
            ld x = px - poly[site].x, y = py - poly[site].y;
            ld radius = hypotl(x, y);
            answer = max(answer, radius);
            if (tied == 1) {
                for (int z : incident[site]) {
                    ld cx = centers[z].first - px, cy = centers[z].second - py;
                    if (hypotl(cx, cy) <= 1e-9L * max((ld)1, radius)) {
                        tied = 2;
                        break;
                    }
                }
            }
            if (tied > 1) {
                ld bestDerivative = numeric_limits<ld>::infinity();
                for (int q = 0; q < n; ++q) {
                    ld qx = px - poly[q].x, qy = py - poly[q].y;
                    if (hypotl(qx, qy) <= radius + 1e-9L) {
                        ld derivative = qx * dx + qy * dy;
                        if (derivative < bestDerivative) {
                            bestDerivative = derivative;
                            next = q;
                        }
                    }
                }
            }
            current = best;
            site = next;
        }
    };

    for (int a = 0; a < n; ++a) {
        int b = (a + 1) % n;
        int t = -1;
        for (int z : incident[a]) {
            auto vv = del.tr[z].v;
            if (find(vv.begin(), vv.end(), b) != vv.end()) {
                t = z;
                break;
            }
        }
        if (t < 0) {
            voronoiWalk(a, b);
            continue;
        }
        auto center = [&](int z) {
            auto w = del.tr[z].v;
            Point A = poly[w[0]], B = poly[w[1]], C = poly[w[2]];
            ld ux = (ld)B.x - A.x, uy = (ld)B.y - A.y;
            ld vx = (ld)C.x - A.x, vy = (ld)C.y - A.y;
            ld d = 2 * (ux * vy - uy * vx);
            ld b2 = ux * ux + uy * uy, c2 = vx * vx + vy * vy;
            return pair<ld, ld>{A.x + (vy * b2 - uy * c2) / d,
                                A.y + (ux * c2 - vx * b2) / d};
        };
        auto vv = del.tr[t].v;
        int ia = find(vv.begin(), vv.end(), a) - vv.begin();
        int ib = find(vv.begin(), vv.end(), b) - vv.begin();
        int ic = 3 - ia - ib;
        int other = del.tr[t].nb[ic];
        auto o1 = center(t);
        ld mx = ((ld)poly[a].x + poly[b].x) / 2;
        ld my = ((ld)poly[a].y + poly[b].y) / 2;
        bool valid;
        if (other >= 0 && del.tr[other].v[0] < n && del.tr[other].v[1] < n && del.tr[other].v[2] < n) {
            auto o2 = center(other);
            valid = (mx - o1.first) * (mx - o2.first) + (my - o1.second) * (my - o2.second) <= 0;
        } else {
            Point c = poly[vv[ic]];
            valid = (mx - o1.first) * (c.x - mx) + (my - o1.second) * (c.y - my) <= 0;
        }
        if (valid) boundaryCandidate(a, b, a, b);
        else voronoiWalk(a, b);
    }

    cout << fixed << setprecision(10) << (double)answer << '\n';
    return 0;
}

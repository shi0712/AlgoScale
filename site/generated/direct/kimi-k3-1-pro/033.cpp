#include <bits/stdc++.h>
using namespace std;

using i128 = __int128_t;

struct Point {
    long long x, y;
};

static inline i128 orient(const Point& a, const Point& b, const Point& c) {
    return (i128)(b.x - a.x) * (c.y - a.y)
         - (i128)(b.y - a.y) * (c.x - a.x);
}

static inline i128 inCircleValue(const Point& a, const Point& b,
                                 const Point& c, const Point& p) {
    i128 ax = (i128)a.x - p.x, ay = (i128)a.y - p.y;
    i128 bx = (i128)b.x - p.x, by = (i128)b.y - p.y;
    i128 cx = (i128)c.x - p.x, cy = (i128)c.y - p.y;

    i128 aa = ax * ax + ay * ay;
    i128 bb = bx * bx + by * by;
    i128 cc = cx * cx + cy * cy;

    return aa * (bx * cy - cx * by)
         - bb * (ax * cy - cx * ay)
         + cc * (ax * by - bx * ay);
}

struct Delaunay {
    struct Tri {
        array<int, 3> v;
        array<int, 3> nb;
        vector<int> children;
        int bad = 0;
        bool alive = true;

        Tri(int a, int b, int c) : v{a, b, c}, nb{-1, -1, -1} {}
    };

    int originalN;
    vector<Point> pt;
    vector<Tri> tris;
    int lastAlive = 0;

    explicit Delaunay(const vector<Point>& input) {
        originalN = (int)input.size();
        pt = input;

        const long long M = 1000000;
        pt.push_back({-M, -M});
        pt.push_back({ M, -M});
        pt.push_back({ 0,  M});

        tris.reserve((size_t)originalN * 10 + 20);
        tris.emplace_back(originalN, originalN + 1, originalN + 2);

        vector<int> order(originalN);
        iota(order.begin(), order.end(), 0);
        mt19937 rng(0x71a39b7u);
        shuffle(order.begin(), order.end(), rng);

        int stamp = 0;
        for (int id : order) insertPoint(id, ++stamp);
    }

    bool insideTriangle(int t, int p) const {
        const Tri& tr = tris[t];
        const Point& q = pt[p];
        return orient(pt[tr.v[0]], pt[tr.v[1]], q) >= 0
            && orient(pt[tr.v[1]], pt[tr.v[2]], q) >= 0
            && orient(pt[tr.v[2]], pt[tr.v[0]], q) >= 0;
    }

    int walkLocate(int p, int start) const {
        int t = start;
        while (!insideTriangle(t, p)) {
            int nxt = -1;
            const Tri& tr = tris[t];
            for (int i = 0; i < 3; ++i) {
                int a = tr.v[(i + 1) % 3];
                int b = tr.v[(i + 2) % 3];
                if (orient(pt[a], pt[b], pt[p]) < 0) {
                    nxt = tr.nb[i];
                    break;
                }
            }
            if (nxt == -1) break;
            t = nxt;
        }
        return t;
    }

    int locate(int p) const {
        int t = 0;
        while (!tris[t].children.empty()) {
            int nxt = -1;
            for (int c : tris[t].children) {
                if (insideTriangle(c, p)) {
                    nxt = c;
                    break;
                }
            }
            if (nxt == -1) return walkLocate(p, lastAlive);
            t = nxt;
        }
        return t;
    }

    static uint64_t edgeKey(int a, int b) {
        if (a > b) swap(a, b);
        return ((uint64_t)(uint32_t)a << 32) | (uint32_t)b;
    }

    void insertPoint(int p, int stamp) {
        int first = locate(p);

        vector<int> stack = {first};
        vector<int> bad;
        tris[first].bad = stamp;

        while (!stack.empty()) {
            int t = stack.back();
            stack.pop_back();
            bad.push_back(t);

            for (int i = 0; i < 3; ++i) {
                int u = tris[t].nb[i];
                if (u == -1 || tris[u].bad == stamp) continue;
                const Tri& ut = tris[u];
                if (inCircleValue(pt[ut.v[0]], pt[ut.v[1]],
                                  pt[ut.v[2]], pt[p]) > 0) {
                    tris[u].bad = stamp;
                    stack.push_back(u);
                }
            }
        }

        struct BoundaryEdge {
            int a, b, outside, outsideEdge;
        };
        vector<BoundaryEdge> boundary;

        for (int t : bad) {
            for (int i = 0; i < 3; ++i) {
                int u = tris[t].nb[i];
                if (u != -1 && tris[u].bad == stamp) continue;

                int a = tris[t].v[(i + 1) % 3];
                int b = tris[t].v[(i + 2) % 3];
                int uj = -1;
                if (u != -1) {
                    for (int j = 0; j < 3; ++j) {
                        if (tris[u].nb[j] == t) {
                            uj = j;
                            break;
                        }
                    }
                }
                boundary.push_back({a, b, u, uj});
            }
        }

        vector<int> created;
        created.reserve(boundary.size());

        for (BoundaryEdge be : boundary) {
            if (orient(pt[be.a], pt[be.b], pt[p]) < 0)
                swap(be.a, be.b);
            if (orient(pt[be.a], pt[be.b], pt[p]) == 0)
                continue;

            int nt = (int)tris.size();
            tris.emplace_back(be.a, be.b, p);
            tris[nt].nb[2] = be.outside;

            if (be.outside != -1 && be.outsideEdge != -1)
                tris[be.outside].nb[be.outsideEdge] = nt;

            created.push_back(nt);
        }

        unordered_map<uint64_t, uint64_t> openEdges;
        openEdges.reserve(created.size() * 3 + 10);

        for (int t : created) {
            for (int i = 0; i < 2; ++i) {
                int a = tris[t].v[(i + 1) % 3];
                int b = tris[t].v[(i + 2) % 3];
                uint64_t key = edgeKey(a, b);

                auto it = openEdges.find(key);
                if (it == openEdges.end()) {
                    openEdges.emplace(key, ((uint64_t)t << 2) | (uint64_t)i);
                } else {
                    uint64_t packed = it->second;
                    int u = (int)(packed >> 2);
                    int j = (int)(packed & 3);
                    tris[t].nb[i] = u;
                    tris[u].nb[j] = t;
                    openEdges.erase(it);
                }
            }
        }

        for (int t : bad) {
            tris[t].children = created;
            tris[t].alive = false;
        }

        if (!created.empty()) lastAlive = created[0];
    }
};

struct Candidate {
    long double x, y, r;
};

struct SweepEdge {
    int x1, y1, x2, y2;
    long double slope;
};

struct EdgeTreap {
    const vector<SweepEdge>& edges;
    vector<array<int, 2>> ch;
    vector<int> parent, sz;
    vector<uint64_t> priority;
    int root = -1;
    long double sweepX = 0;
    int side = 0;

    explicit EdgeTreap(const vector<SweepEdge>& e)
        : edges(e), ch(e.size()), parent(e.size(), -1),
          sz(e.size(), 1), priority(e.size()) {
        mt19937_64 rng(0x9e3779b97f4a7c15ULL);
        for (size_t i = 0; i < e.size(); ++i) {
            ch[i] = {-1, -1};
            priority[i] = rng();
        }
    }

    long double yAt(int id, long double x) const {
        const SweepEdge& e = edges[id];
        return (long double)e.y1 + e.slope * (x - (long double)e.x1);
    }

    bool lessEdge(int a, int b) const {
        long double ya = yAt(a, sweepX);
        long double yb = yAt(b, sweepX);
        if (ya < yb) return true;
        if (ya > yb) return false;

        if (side != 0 && edges[a].slope != edges[b].slope) {
            if (side > 0) return edges[a].slope < edges[b].slope;
            return edges[a].slope > edges[b].slope;
        }
        return a < b;
    }

    void pull(int x) {
        sz[x] = 1;
        if (ch[x][0] != -1) sz[x] += sz[ch[x][0]];
        if (ch[x][1] != -1) sz[x] += sz[ch[x][1]];
    }

    void rotateUp(int x) {
        int p = parent[x];
        int g = parent[p];

        if (ch[p][0] == x) {
            ch[p][0] = ch[x][1];
            if (ch[x][1] != -1) parent[ch[x][1]] = p;
            ch[x][1] = p;
        } else {
            ch[p][1] = ch[x][0];
            if (ch[x][0] != -1) parent[ch[x][0]] = p;
            ch[x][0] = p;
        }

        parent[p] = x;
        parent[x] = g;
        if (g == -1) root = x;
        else ch[g][ch[g][1] == p] = x;

        pull(p);
        pull(x);
    }

    void insertNode(int id) {
        parent[id] = -1;
        ch[id] = {-1, -1};
        sz[id] = 1;

        if (root == -1) {
            root = id;
            return;
        }

        int cur = root, p = -1;
        bool goLeft = false;
        while (cur != -1) {
            p = cur;
            goLeft = lessEdge(id, cur);
            cur = ch[cur][goLeft ? 0 : 1];
        }

        ch[p][goLeft ? 0 : 1] = id;
        parent[id] = p;
        for (int y = p; y != -1; y = parent[y]) ++sz[y];

        while (parent[id] != -1 && priority[id] > priority[parent[id]])
            rotateUp(id);
    }

    void eraseNode(int id) {
        while (ch[id][0] != -1 || ch[id][1] != -1) {
            int c;
            if (ch[id][0] == -1) c = ch[id][1];
            else if (ch[id][1] == -1) c = ch[id][0];
            else c = priority[ch[id][0]] > priority[ch[id][1]]
                   ? ch[id][0] : ch[id][1];
            rotateUp(c);
        }

        int p = parent[id];
        if (p == -1) root = -1;
        else ch[p][ch[p][1] == id] = -1;

        parent[id] = -1;
        for (int y = p; y != -1; y = parent[y]) pull(y);
    }

    bool pointInside(long double y) const {
        int below = 0;
        int cur = root;

        while (cur != -1) {
            long double ey = yAt(cur, sweepX);
            if (fabsl(ey - y) <= 1e-12L) return true;

            if (ey < y) {
                below += 1;
                if (ch[cur][0] != -1) below += sz[ch[cur][0]];
                cur = ch[cur][1];
            } else {
                cur = ch[cur][0];
            }
        }
        return (below & 1) != 0;
    }
};

static vector<char> locateCandidates(const vector<Point>& poly,
                                     const vector<Candidate>& cand) {
    vector<SweepEdge> edges;
    map<int, vector<int>> starts, ends;

    int n = (int)poly.size();
    for (int i = 0; i < n; ++i) {
        Point a = poly[i], b = poly[(i + 1) % n];
        if (a.x == b.x) continue;

        int id = (int)edges.size();
        SweepEdge e;
        e.x1 = (int)a.x; e.y1 = (int)a.y;
        e.x2 = (int)b.x; e.y2 = (int)b.y;
        e.slope = (long double)(b.y - a.y) / (long double)(b.x - a.x);
        edges.push_back(e);

        int lo = (int)min(a.x, b.x);
        int hi = (int)max(a.x, b.x);
        starts[lo].push_back(id);
        ends[hi].push_back(id);
    }

    vector<int> eventX;
    for (const auto& [x, v] : starts) eventX.push_back(x);
    for (const auto& [x, v] : ends) eventX.push_back(x);
    sort(eventX.begin(), eventX.end());
    eventX.erase(unique(eventX.begin(), eventX.end()), eventX.end());

    vector<int> queries(cand.size());
    iota(queries.begin(), queries.end(), 0);
    sort(queries.begin(), queries.end(), [&](int a, int b) {
        if (cand[a].x != cand[b].x) return cand[a].x < cand[b].x;
        return cand[a].y < cand[b].y;
    });

    EdgeTreap treap(edges);
    vector<char> answer(cand.size(), false);
    size_t ep = 0;

    auto processEvent = [&](int x) {
        treap.sweepX = x;
        treap.side = 1;
        for (int id : ends[x]) treap.eraseNode(id);
        for (int id : starts[x]) treap.insertNode(id);
        treap.side = 0;
    };

    for (int qi : queries) {
        long double x = cand[qi].x;

        while (ep < eventX.size() && (long double)eventX[ep] < x) {
            processEvent(eventX[ep]);
            ++ep;
        }
        if (ep < eventX.size() && (long double)eventX[ep] == x) {
            processEvent(eventX[ep]);
            ++ep;
        }

        treap.sweepX = x;
        treap.side = 0;
        answer[qi] = treap.pointInside(cand[qi].y);
    }

    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<Point> poly(n);
    for (Point& p : poly) cin >> p.x >> p.y;

    Delaunay delaunay(poly);
    vector<vector<int>> adj(n);

    auto addAdjacency = [&](int a, int b) {
        adj[a].push_back(b);
        adj[b].push_back(a);
    };

    vector<Candidate> candidates;
    candidates.reserve((size_t)n * 2);

    for (const auto& tr : delaunay.tris) {
        if (!tr.alive) continue;

        for (int i = 0; i < 3; ++i) {
            int a = tr.v[i];
            int b = tr.v[(i + 1) % 3];
            if (a < n && b < n) addAdjacency(a, b);
        }

        if (tr.v[0] >= n || tr.v[1] >= n || tr.v[2] >= n) continue;

        const Point& a = poly[tr.v[0]];
        const Point& b = poly[tr.v[1]];
        const Point& c = poly[tr.v[2]];

        long double bx = (long double)b.x - a.x;
        long double by = (long double)b.y - a.y;
        long double cx = (long double)c.x - a.x;
        long double cy = (long double)c.y - a.y;
        long double den = 2.0L * (bx * cy - by * cx);
        if (den == 0) continue;

        long double b2 = bx * bx + by * by;
        long double c2 = cx * cx + cy * cy;

        long double ox = (cy * b2 - by * c2) / den;
        long double oy = (bx * c2 - cx * b2) / den;

        Candidate q;
        q.x = (long double)a.x + ox;
        q.y = (long double)a.y + oy;
        q.r = hypot(ox, oy);

        if (isfinite(q.x) && isfinite(q.y) && isfinite(q.r))
            candidates.push_back(q);
    }

    for (int i = 0; i < n; ++i) {
        auto& v = adj[i];
        sort(v.begin(), v.end());
        v.erase(unique(v.begin(), v.end()), v.end());
    }

    long double best = 0;

    vector<char> inside = locateCandidates(poly, candidates);
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (inside[i]) best = max(best, candidates[i].r);
    }

    vector<long double> X(n), Y(n), N2(n);
    for (int i = 0; i < n; ++i) {
        X[i] = poly[i].x;
        Y[i] = poly[i].y;
        N2[i] = X[i] * X[i] + Y[i] * Y[i];
    }

    vector<int> edgeOrder(n);
    vector<long double> edgeLength(n);
    iota(edgeOrder.begin(), edgeOrder.end(), 0);

    for (int i = 0; i < n; ++i) {
        int j = (i + 1) % n;
        edgeLength[i] = hypot((long double)poly[j].x - poly[i].x,
                              (long double)poly[j].y - poly[i].y);
    }

    sort(edgeOrder.begin(), edgeOrder.end(), [&](int a, int b) {
        return edgeLength[a] > edgeLength[b];
    });

    const long double ROOT_EPS = 1e-13L;

    for (int ei : edgeOrder) {
        long double len = edgeLength[ei];
        if (0.5L * len <= best) break;

        int a = ei;
        int b = (ei + 1) % n;

        long double ax = X[a], ay = Y[a];
        long double vx = X[b] - ax, vy = Y[b] - ay;

        int site = a;
        long double currentT = 0;

        while (true) {
            long double nextT = 2;
            int nextSite = -1;

            for (int q : adj[site]) {
                long double sx = X[site] - X[q];
                long double sy = Y[site] - Y[q];

                long double c0 = N2[q] - N2[site]
                               + 2.0L * (ax * sx + ay * sy);
                long double c1 = 2.0L * (vx * sx + vy * sy);

                if (c1 >= 0) continue;
                long double t = -c0 / c1;

                if (t > currentT + ROOT_EPS &&
                    t < 1.0L - ROOT_EPS && t < nextT) {
                    nextT = t;
                    nextSite = q;
                }
            }

            if (nextSite == -1) break;

            long double px = ax + nextT * vx;
            long double py = ay + nextT * vy;

            auto dist2 = [&](int s) {
                long double dx = px - X[s];
                long double dy = py - Y[s];
                return dx * dx + dy * dy;
            };

            long double d0 = min(dist2(site), dist2(nextSite));

            vector<int> tied = {site, nextSite};
            for (size_t i = 0; i < tied.size(); ++i) {
                int u = tied[i];
                for (int v : adj[u]) {
                    if (find(tied.begin(), tied.end(), v) != tied.end())
                        continue;
                    long double dv = dist2(v);
                    long double tol = 1e-12L * max(1.0L, d0);
                    if (fabsl(dv - d0) <= tol) {
                        tied.push_back(v);
                        d0 = min(d0, dv);
                    }
                }
            }

            int following = site;
            long double bestDerivative = numeric_limits<long double>::infinity();

            for (int s : tied) {
                long double derivative =
                    2.0L * ((px - X[s]) * vx + (py - Y[s]) * vy);
                if (derivative < bestDerivative) {
                    bestDerivative = derivative;
                    following = s;
                }
            }

            long double value = sqrt(max(0.0L, d0));
            value = min(value, 0.5L * len);
            best = max(best, value);

            if (following == site) following = nextSite;
            if (following == site) break;

            site = following;
            currentT = nextT;
        }
    }

    cout << fixed << setprecision(15) << best << '\n';
    return 0;
}

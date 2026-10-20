#include <bits/stdc++.h>
using namespace std;

struct EKey {
    int id;   // edge id = upper vertex id, or -1 for a point
    int p;    // point vertex id when id == -1
};

vector<long long> gX, gY;
vector<int> gNextPoly;
long long gCurY;

struct ECmp {
    static __int128 numerator(int e) {
        int u = e, v = gNextPoly[u];
        long long D = gY[u] - gY[v]; // > 0
        return (__int128)gX[u] * D + (__int128)(gX[v] - gX[u]) * (gY[u] - gCurY);
    }
    static long long denominator(int e) {
        return gY[e] - gY[gNextPoly[e]];
    }

    bool operator()(const EKey& a, const EKey& b) const {
        if (a.id == -1 && b.id == -1) {
            return gX[a.p] < gX[b.p];
        }
        if (a.id != -1 && b.id != -1) {
            __int128 A = numerator(a.id);
            __int128 B = numerator(b.id);
            long long DA = denominator(a.id);
            long long DB = denominator(b.id);
            return A * DB < B * DA;
        }
        if (a.id != -1) {
            __int128 A = numerator(a.id);
            long long DA = denominator(a.id);
            return A < (__int128)gX[b.p] * DA;
        } else {
            __int128 B = numerator(b.id);
            long long DB = denominator(b.id);
            return (__int128)gX[a.p] * DB < B;
        }
    }
};

struct KDNode {
    int id, l, r;
    int minx, maxx, miny, maxy;
};

struct KDTree {
    int n;
    vector<int> xs, ys;
    vector<int> ord;
    vector<KDNode> tr;
    int root;

    KDTree() {}
    KDTree(const vector<int>& X, const vector<int>& Y) {
        init(X, Y);
    }

    void init(const vector<int>& X, const vector<int>& Y) {
        xs = X;
        ys = Y;
        n = (int)xs.size();
        ord.resize(n);
        iota(ord.begin(), ord.end(), 0);
        tr.reserve(max(1, n * 2));
        root = build(0, n - 1, 0);
    }

    int build(int l, int r, int dep) {
        if (l > r) return -1;
        int mid = (l + r) >> 1;
        if (dep & 1) {
            nth_element(ord.begin() + l, ord.begin() + mid, ord.begin() + r + 1,
                        [&](int a, int b) {
                            if (ys[a] != ys[b]) return ys[a] < ys[b];
                            return xs[a] < xs[b];
                        });
        } else {
            nth_element(ord.begin() + l, ord.begin() + mid, ord.begin() + r + 1,
                        [&](int a, int b) {
                            if (xs[a] != xs[b]) return xs[a] < xs[b];
                            return ys[a] < ys[b];
                        });
        }

        int node = (int)tr.size();
        tr.push_back({});
        int id = ord[mid];
        tr[node].id = id;
        tr[node].l = build(l, mid - 1, dep + 1);
        tr[node].r = build(mid + 1, r, dep + 1);

        int mnx = xs[id], mxx = xs[id];
        int mny = ys[id], mxy = ys[id];

        int ch[2] = {tr[node].l, tr[node].r};
        for (int c : ch) {
            if (c == -1) continue;
            mnx = min(mnx, tr[c].minx);
            mxx = max(mxx, tr[c].maxx);
            mny = min(mny, tr[c].miny);
            mxy = max(mxy, tr[c].maxy);
        }

        tr[node].minx = mnx;
        tr[node].maxx = mxx;
        tr[node].miny = mny;
        tr[node].maxy = mxy;
        return node;
    }

    double bboxDist2(int node, double qx, double qy) const {
        if (node == -1) return 1e100;
        const auto& nd = tr[node];
        double dx = 0, dy = 0;
        if (qx < nd.minx) dx = nd.minx - qx;
        else if (qx > nd.maxx) dx = qx - nd.maxx;
        if (qy < nd.miny) dy = nd.miny - qy;
        else if (qy > nd.maxy) dy = qy - nd.maxy;
        return dx * dx + dy * dy;
    }

    double query(int node, double qx, double qy, double bestD2) const {
        if (node == -1) return bestD2;
        const auto& nd = tr[node];
        double dx = qx - xs[nd.id];
        double dy = qy - ys[nd.id];
        double d2 = dx * dx + dy * dy;
        if (d2 < bestD2) bestD2 = d2;

        int a = nd.l, b = nd.r;
        double da = bboxDist2(a, qx, qy);
        double db = bboxDist2(b, qx, qy);

        if (da > db) {
            swap(a, b);
            swap(da, db);
        }

        if (da < bestD2) bestD2 = query(a, qx, qy, bestD2);
        if (db < bestD2) bestD2 = query(b, qx, qy, bestD2);
        return bestD2;
    }

    double nearestDist(double qx, double qy) const {
        double d2 = query(root, qx, qy, 1e100);
        return sqrt(max(0.0, d2));
    }
};

struct Cell {
    double x[3], y[3];
    double upper;
};

struct CellCmp {
    bool operator()(const Cell& a, const Cell& b) const {
        return a.upper < b.upper;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> xin(n), yin(n);
    for (int i = 0; i < n; ++i) {
        cin >> xin[i] >> yin[i];
    }

    // Remove collinear middle vertices of the polygon.
    vector<int> prv(n), nxt(n);
    for (int i = 0; i < n; ++i) {
        prv[i] = (i - 1 + n) % n;
        nxt[i] = (i + 1) % n;
    }

    vector<char> active(n, 1);

    auto cross_orig = [&](int a, int b, int c) -> long long {
        return 1LL * (xin[b] - xin[a]) * (yin[c] - yin[a])
             - 1LL * (yin[b] - yin[a]) * (xin[c] - xin[a]);
    };

    auto is_collinear_middle = [&](int i) -> bool {
        int p = prv[i], q = nxt[i];
        if (p == i || q == i) return false;
        if (cross_orig(p, i, q) != 0) return false;
        long long dot = 1LL * (xin[i] - xin[p]) * (xin[q] - xin[i])
                      + 1LL * (yin[i] - yin[p]) * (yin[q] - yin[i]);
        return dot > 0;
    };

    queue<int> qq;
    for (int i = 0; i < n; ++i) qq.push(i);

    while (!qq.empty()) {
        int i = qq.front();
        qq.pop();
        if (!active[i]) continue;
        if (!is_collinear_middle(i)) continue;

        active[i] = 0;
        int p = prv[i], q = nxt[i];
        nxt[p] = q;
        prv[q] = p;

        qq.push(p);
        qq.push(q);
    }

    vector<int> poly;
    int start = -1;
    for (int i = 0; i < n; ++i) {
        if (active[i]) {
            start = i;
            break;
        }
    }

    int cur = start;
    do {
        poly.push_back(cur);
        cur = nxt[cur];
    } while (cur != start);

    int m = (int)poly.size();

    vector<long long> ox(m), oy(m);
    for (int i = 0; i < m; ++i) {
        ox[i] = xin[poly[i]];
        oy[i] = yin[poly[i]];
    }

    const long long SHEAR = 1000000LL;
    gX.assign(m, 0);
    gY.assign(m, 0);
    for (int i = 0; i < m; ++i) {
        gX[i] = ox[i];
        gY[i] = oy[i] * SHEAR + ox[i];
    }

    gNextPoly.resize(m);
    for (int i = 0; i < m; ++i) gNextPoly[i] = (i + 1) % m;

    vector<int> prevPoly(m), nextPoly(m);
    for (int i = 0; i < m; ++i) {
        prevPoly[i] = (i - 1 + m) % m;
        nextPoly[i] = (i + 1) % m;
    }

    auto cross_simpl = [&](int a, int b, int c) -> long long {
        return (ox[b] - ox[a]) * (oy[c] - oy[a])
             - (oy[b] - oy[a]) * (ox[c] - ox[a]);
    };

    enum Type { START, SPLIT, END, MERGE, REGULAR };

    vector<int> typ(m);
    for (int v = 0; v < m; ++v) {
        int p = prevPoly[v], q = nextPoly[v];
        bool convex = cross_simpl(p, v, q) > 0;

        if (gY[p] < gY[v] && gY[q] < gY[v]) {
            typ[v] = convex ? START : SPLIT;
        } else if (gY[p] > gY[v] && gY[q] > gY[v]) {
            typ[v] = convex ? END : MERGE;
        } else {
            typ[v] = REGULAR;
        }
    }

    vector<int> order(m);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        if (gY[a] != gY[b]) return gY[a] > gY[b];
        return gX[a] > gX[b];
    });

    vector<int> helper(m, -1);
    set<EKey, ECmp> T;

    vector<pair<int, int>> diagonals;
    unordered_set<long long> diagSet;
    diagSet.reserve(m * 2 + 10);

    auto add_diagonal = [&](int a, int b) {
        if (a < 0 || b < 0 || a == b) return;
        if ((a + 1) % m == b || (b + 1) % m == a) return;

        int u = min(a, b), v = max(a, b);
        long long key = 1LL * u * 1000000LL + v;
        if (diagSet.insert(key).second) {
            diagonals.push_back({u, v});
        }
    };

    auto find_left = [&](int v) -> int {
        EKey key{-1, v};
        auto it = T.lower_bound(key);
        if (it == T.begin()) return -1;
        --it;
        return it->id;
    };

    for (int v : order) {
        gCurY = gY[v];

        if (typ[v] == START) {
            T.insert(EKey{v, -1});
            helper[v] = v;
        } else if (typ[v] == SPLIT) {
            int e = find_left(v);
            if (e != -1) {
                if (helper[e] != -1) add_diagonal(v, helper[e]);
                helper[e] = v;
            }
            T.insert(EKey{v, -1});
            helper[v] = v;
        } else if (typ[v] == END) {
            int ep = prevPoly[v];
            if (helper[ep] != -1 && typ[helper[ep]] == MERGE) {
                add_diagonal(v, helper[ep]);
            }
            T.erase(EKey{ep, -1});
        } else if (typ[v] == MERGE) {
            int ep = prevPoly[v];

            if (helper[ep] != -1 && typ[helper[ep]] == MERGE) {
                add_diagonal(v, helper[ep]);
            }
            T.erase(EKey{ep, -1});

            int e = find_left(v);
            if (e != -1) {
                if (helper[e] != -1 && typ[helper[e]] == MERGE) {
                    add_diagonal(v, helper[e]);
                }
                helper[e] = v;
            }
        } else { // REGULAR
            if (gY[prevPoly[v]] > gY[v]) {
                int ep = prevPoly[v];
                if (helper[ep] != -1 && typ[helper[ep]] == MERGE) {
                    add_diagonal(v, helper[ep]);
                }
                T.erase(EKey{ep, -1});

                T.insert(EKey{v, -1});
                helper[v] = v;
            } else {
                int e = find_left(v);
                if (e != -1) {
                    if (helper[e] != -1 && typ[helper[e]] == MERGE) {
                        add_diagonal(v, helper[e]);
                    }
                    helper[e] = v;
                }
            }
        }
    }

    // Build planar graph: boundary edges + diagonals.
    struct Edge {
        int u, v;
    };

    vector<Edge> edges;
    edges.reserve(m + diagonals.size());

    for (int i = 0; i < m; ++i) {
        edges.push_back({i, (i + 1) % m});
    }
    for (auto [a, b] : diagonals) {
        edges.push_back({a, b});
    }

    int E = (int)edges.size();
    int H = 2 * E;

    vector<int> from(H), to(H);
    vector<vector<int>> adj(m);

    for (int i = 0; i < E; ++i) {
        int u = edges[i].u, v = edges[i].v;
        int h = 2 * i;
        from[h] = u;
        to[h] = v;
        adj[u].push_back(h);

        h = 2 * i + 1;
        from[h] = v;
        to[h] = u;
        adj[v].push_back(h);
    }

    auto half_plane = [](long long dx, long long dy) -> int {
        return (dy > 0 || (dy == 0 && dx >= 0)) ? 0 : 1;
    };

    vector<int> posInAdj(H, -1);

    for (int v = 0; v < m; ++v) {
        sort(adj[v].begin(), adj[v].end(), [&](int h1, int h2) {
            long long dx1 = gX[to[h1]] - gX[v];
            long long dy1 = gY[to[h1]] - gY[v];
            long long dx2 = gX[to[h2]] - gX[v];
            long long dy2 = gY[to[h2]] - gY[v];

            int hp1 = half_plane(dx1, dy1);
            int hp2 = half_plane(dx2, dy2);
            if (hp1 != hp2) return hp1 < hp2;

            long long cr = dx1 * dy2 - dy1 * dx2;
            if (cr != 0) return cr > 0;

            long long d1 = dx1 * dx1 + dy1 * dy1;
            long long d2 = dx2 * dx2 + dy2 * dy2;
            return d1 < d2;
        });

        for (int i = 0; i < (int)adj[v].size(); ++i) {
            posInAdj[adj[v][i]] = i;
        }
    }

    vector<int> nextHalf(H, -1);

    for (int h = 0; h < H; ++h) {
        int v = to[h];
        int rev = h ^ 1;
        int pos = posInAdj[rev];
        int deg = (int)adj[v].size();
        int nxtPos = (pos - 1 + deg) % deg;
        nextHalf[h] = adj[v][nxtPos];
    }

    vector<char> visited(H, 0);
    vector<vector<int>> faces;

    for (int h = 0; h < H; ++h) {
        if (visited[h]) continue;

        vector<int> face;
        int curH = h;

        do {
            visited[curH] = 1;
            face.push_back(from[curH]);
            curH = nextHalf[curH];
        } while (curH != h);

        __int128 area2 = 0;
        int k = (int)face.size();

        for (int i = 0; i < k; ++i) {
            int u = face[i], v = face[(i + 1) % k];
            area2 += (__int128)gX[u] * gY[v] - (__int128)gY[u] * gX[v];
        }

        if (area2 > 0) {
            faces.push_back(face);
        }
    }

    vector<array<int, 3>> triangles;

    auto add_triangle = [&](int a, int b, int c) {
        long long cr = cross_simpl(a, b, c);
        if (cr == 0) return;
        triangles.push_back({a, b, c});
    };

    // Triangulate every y-monotone face.
    for (auto& faceRaw : faces) {
        vector<int> vlist = faceRaw;
        int k = (int)vlist.size();
        if (k < 3) continue;

        int topPos = 0, botPos = 0;
        for (int i = 1; i < k; ++i) {
            if (gY[vlist[i]] > gY[vlist[topPos]]) topPos = i;
            if (gY[vlist[i]] < gY[vlist[botPos]]) botPos = i;
        }

        vector<int> side(k, -1);

        int idx = topPos;
        side[idx] = 0;
        while (idx != botPos) {
            idx = (idx + 1) % k;
            side[idx] = 0;
        }

        idx = topPos;
        while (idx != botPos) {
            idx = (idx - 1 + k) % k;
            if (side[idx] == -1) side[idx] = 1;
        }

        vector<int> ord(k);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            return gY[vlist[a]] > gY[vlist[b]];
        });

        vector<int> st;
        st.push_back(ord[0]);
        st.push_back(ord[1]);

        for (int i = 2; i < k - 1; ++i) {
            int uPos = ord[i];
            int prevPos = ord[i - 1];
            int s = side[uPos];

            if (side[prevPos] != s) {
                while (!st.empty()) {
                    int v = st.back();
                    st.pop_back();
                    if (!st.empty()) {
                        add_triangle(vlist[uPos], vlist[v], vlist[st.back()]);
                    }
                }
                st.push_back(prevPos);
                st.push_back(uPos);
            } else {
                int v = st.back();
                st.pop_back();

                while (!st.empty()) {
                    int w = st.back();
                    long long cr = cross_simpl(vlist[w], vlist[v], vlist[uPos]);

                    bool ok;
                    if (s == 0) ok = (cr > 0);
                    else ok = (cr < 0);

                    if (ok) {
                        add_triangle(vlist[uPos], vlist[v], vlist[w]);
                        v = w;
                        st.pop_back();
                    } else {
                        break;
                    }
                }

                st.push_back(v);
                st.push_back(uPos);
            }
        }

        int lastPos = ord[k - 1];

        while ((int)st.size() > 1) {
            int v = st.back();
            st.pop_back();
            int w = st.back();
            add_triangle(vlist[lastPos], vlist[v], vlist[w]);
        }
    }

    KDTree kd(xin, yin);

    const double EPS = 1e-7;

    struct InitCell {
        double x[3], y[3];
        double upper;
    };

    vector<InitCell> init;
    init.reserve(triangles.size());

    double best = 0.0;

    for (auto tri : triangles) {
        InitCell c;
        double cx = 0, cy = 0;

        for (int i = 0; i < 3; ++i) {
            int id = tri[i];
            c.x[i] = (double)ox[id];
            c.y[i] = (double)oy[id];
            cx += c.x[i] / 3.0;
            cy += c.y[i] / 3.0;
        }

        double d = kd.nearestDist(cx, cy);
        best = max(best, d);

        double R = 0.0;
        for (int i = 0; i < 3; ++i) {
            double dx = cx - c.x[i];
            double dy = cy - c.y[i];
            R = max(R, sqrt(dx * dx + dy * dy));
        }

        c.upper = d + R;
        init.push_back(c);
    }

    priority_queue<Cell, vector<Cell>, CellCmp> pq;

    for (auto& c : init) {
        if (c.upper > best + EPS) {
            Cell cell;
            for (int i = 0; i < 3; ++i) {
                cell.x[i] = c.x[i];
                cell.y[i] = c.y[i];
            }
            cell.upper = c.upper;
            pq.push(cell);
        }
    }

    auto push_cell = [&](double x0, double y0,
                         double x1, double y1,
                         double x2, double y2) {
        double cx = (x0 + x1 + x2) / 3.0;
        double cy = (y0 + y1 + y2) / 3.0;

        double d = kd.nearestDist(cx, cy);
        if (d > best) best = d;

        double R = 0.0;
        R = max(R, hypot(cx - x0, cy - y0));
        R = max(R, hypot(cx - x1, cy - y1));
        R = max(R, hypot(cx - x2, cy - y2));

        double upper = d + R;
        if (upper > best + EPS) {
            Cell c;
            c.x[0] = x0; c.y[0] = y0;
            c.x[1] = x1; c.y[1] = y1;
            c.x[2] = x2; c.y[2] = y2;
            c.upper = upper;
            pq.push(c);
        }
    };

    while (!pq.empty()) {
        Cell c = pq.top();
        pq.pop();

        if (c.upper <= best + EPS) {
            break;
        }

        double ax = c.x[0], ay = c.y[0];
        double bx = c.x[1], by = c.y[1];
        double cx = c.x[2], cy = c.y[2];

        double m01x = (ax + bx) * 0.5, m01y = (ay + by) * 0.5;
        double m12x = (bx + cx) * 0.5, m12y = (by + cy) * 0.5;
        double m20x = (cx + ax) * 0.5, m20y = (cy + ay) * 0.5;

        push_cell(ax, ay, m01x, m01y, m20x, m20y);
        push_cell(m01x, m01y, bx, by, m12x, m12y);
        push_cell(m20x, m20y, m12x, m12y, cx, cy);
        push_cell(m01x, m01y, m12x, m12y, m20x, m20y);
    }

    cout << fixed << setprecision(12) << best << '\n';
    return 0;
}

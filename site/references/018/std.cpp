// 湖畔可达域 / adapted from QOJ 11525 - Rikka with Lake. C++17.
// Deterministic O((q+1)*n) time and O(n) auxiliary space, where q is the
// number of distinct input x coordinates (including the source's x=0).
// Integer x is restricted to [-200,200], hence q<=401 and this is O(n)
// for this problem.  This is not a coordinate-independent linear triangulator.

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <queue>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>
using namespace std;
using ld = long double;
const ld EPS = 1e-12L;
const ld PI = acosl(-1.0L), TAU = 2 * PI;

struct P {
    ld x = 0, y = 0;
    P operator+(P b) const {
        return {x + b.x, y + b.y};
    }
    P operator-(P b) const {
        return {x - b.x, y - b.y};
    }
    P operator*(ld k) const {
        return {x * k, y * k};
    }
};
ld dot(P a, P b) {
    return a.x * b.x + a.y * b.y;
}
ld cross(P a, P b) {
    return a.x * b.y - a.y * b.x;
}
ld norm2(P a) {
    return dot(a, a);
}
ld len(P a) {
    return sqrtl(norm2(a));
}
ld area2(const vector<P> &a) {
    ld z = 0;
    for (int i = 0; i < (int)a.size(); ++i)
        z += cross(a[i], a[(i + 1) % a.size()]);
    return z;
}

struct Edge {
    int a, b;
};
struct Tri {
    array<int, 3> v;
    array<int, 3> adj{-1, -1, -1}, eid{-1, -1, -1};
};
struct Link {
    int t, e, u, f;
};
struct Mesh {
    vector<P> p;
    vector<Tri> tri;
    vector<Link> links;
    int source;
};

Mesh triangulate(vector<P> poly, ld k) {
    // Integer input: lexicographic (x,y) order after an exact integer shear,
    // represented by an orthonormal rotation for Euclidean distances.
    vector<long long> column, key;
    ld M = 1009;
    for (P a : poly)
        M = max(M, 2 * fabsl(a.y) + 1);
    for (P a : poly) {
        column.push_back(llroundl(a.x));
        key.push_back(llroundl(M * a.x + a.y));
    }
    const ld W = hypotl(M, 1);
    for (P &a : poly) {
        ld x = a.x, y = a.y;
        a = {(M * x + y) / W, (-x + M * y) / W};
    }
    Mesh mesh;
    mesh.p = poly;
    auto &p = mesh.p;
    int n = poly.size();
    mesh.source = -1;
    for (int i = 0; i < n; i++)
        if (norm2(p[i]) < EPS * EPS)
            mesh.source = i;
    if (mesh.source < 0) {
        mesh.source = p.size();
        p.push_back({0, 0});
    }
    ld B = k + 1000;
    int lb = p.size();
    p.push_back({-B, -B});
    int lt = p.size();
    p.push_back({-B, B});
    int rb = p.size();
    p.push_back({B, -B});
    int rt = p.size();
    p.push_back({B, B});
    vector<Edge> ed;
    for (int i = 0; i < n; i++)
        ed.push_back({i, (i + 1) % n});
    int bottom = ed.size();
    ed.push_back({lb, rb});
    int top = ed.size();
    ed.push_back({lt, rt});
    vector<vector<int>> incoming(p.size()), outgoing(p.size());
    for (int e = 0; e < n; e++) {
        int a = ed[e].a, b = ed[e].b;
        if (p[a].x > p[b].x)
            swap(a, b);
        outgoing[a].push_back(e);
        incoming[b].push_back(e);
    }
    vector<int> events;
    for (int i = 0; i < n; i++)
        events.push_back(i);
    if (mesh.source >= n) {
        events.push_back(mesh.source);
        column.push_back(0);
        key.push_back(0);
    }
    // Fixed-width integer radix sort; no comparison sorting of the vertices.
    vector<int> eventTemp(events.size());
    for (int byte = 0; byte < 8; byte++) {
        array<int, 256> count{};
        for (int v : events)
            count[((uint64_t(key[v]) ^ (uint64_t(1) << 63)) >> (8 * byte)) & 255]++;
        int sum = 0;
        for (int &c : count) {
            int z = c;
            c = sum;
            sum += z;
        }
        for (int v : events)
            eventTemp[count[((uint64_t(key[v]) ^ (uint64_t(1) << 63)) >> (8 * byte)) & 255]++] = v;
        events.swap(eventTemp);
    }
    auto yAt = [&](int e, ld x) {
        P a = p[ed[e].a], b = p[ed[e].b];
        if (x == a.x)
            return a.y;
        if (x == b.x)
            return b.y;
        return a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x);
    };
    auto isLand = [&](int e) { return e == bottom || (e < n && p[ed[e].a].x < p[ed[e].b].x); };
    vector<int> before(ed.size(), -1), after(ed.size(), -1);
    after[bottom] = top;
    before[top] = bottom;
    vector<vector<int>> cells(ed.size());
    cells[bottom] = {lb, lt};
    int cursor = bottom;
    long long currentColumn = numeric_limits<long long>::min();

    auto emit = [&](vector<int> q) {
        vector<int> a;
        for (int x : q)
            if (a.empty() || a.back() != x)
                a.push_back(x);
        if (a.size() > 1 && a.front() == a.back())
            a.pop_back();
        while (a.size() > 3) {
            bool ok = false;
            for (int i = 0; i < (int)a.size(); i++) {
                int m = a.size(), u = a[(i + m - 1) % m], v = a[i], w = a[(i + 1) % m];
                if (cross(p[v] - p[u], p[w] - p[v]) <= EPS)
                    continue;
                bool blocked = false;
                for (int j = 0; j < m; j++) {
                    int z = a[j];
                    if (z == u || z == v || z == w)
                        continue;
                    if (cross(p[v] - p[u], p[z] - p[u]) >= -EPS &&
                        cross(p[w] - p[v], p[z] - p[v]) >= -EPS &&
                        cross(p[u] - p[w], p[z] - p[w]) >= -EPS) {
                        blocked = true;
                        break;
                    }
                }
                if (!blocked) {
                    mesh.tri.push_back({{u, v, w}});
                    a.erase(a.begin() + i);
                    ok = true;
                    break;
                }
            }
            if (!ok)
                throw runtime_error("Degenerate trapezoid triangulation");
        }
        if (a.size() == 3 && cross(p[a[1]] - p[a[0]], p[a[2]] - p[a[0]]) > EPS)
            mesh.tri.push_back({{a[0], a[1], a[2]}});
    };
    for (int ev : events) {
        ld x = p[ev].x;
        vector<int> in = incoming[ev], out = outgoing[ev];
        if (column[ev] != currentColumn) {
            currentColumn = column[ev];
            cursor = bottom;
        }
        int lowEdge, highEdge;
        if (in.empty()) {
            while (after[cursor] != top && yAt(after[cursor], x) < p[ev].y)
                cursor = after[cursor];
            lowEdge = cursor;
            highEdge = after[cursor];
        } else {
            int lo = in[0], hi = in[0];
            auto slope = [&](int e) {
                P v = p[ed[e].b] - p[ed[e].a];
                return v.y / v.x;
            };
            for (int e : in) {
                if (slope(e) > slope(lo))
                    lo = e;
                if (slope(e) < slope(hi))
                    hi = e;
            }
            lowEdge = before[lo];
            highEdge = after[hi];
        }
        if (lowEdge < 0 || highEdge < 0)
            throw runtime_error("Invalid sweep neighbors");
        vector<int> old;
        for (int e = lowEdge;; e = after[e]) {
            old.push_back(e);
            if (e == highEdge)
                break;
        }
        vector<pair<int, int>> cache;
        auto hit = [&](int e) {
            for (auto [a, b] : cache)
                if (a == e)
                    return b;
            int a = ed[e].a, b = ed[e].b, id;
            if (fabsl(p[a].x - x) < EPS)
                id = a;
            else if (fabsl(p[b].x - x) < EPS)
                id = b;
            else {
                id = p.size();
                p.push_back({x, yAt(e, x)});
            }
            cache.push_back({e, id});
            return id;
        };
        auto side = [&](int a, int b) {
            int u = hit(a), v = hit(b);
            vector<int> s{u};
            if (p[ev].y > p[u].y + EPS && p[ev].y < p[v].y - EPS)
                s.push_back(ev);
            if (v != u)
                s.push_back(v);
            return s;
        };
        for (int i = 0; i + 1 < (int)old.size(); i++) {
            auto key = make_pair(old[i], old[i + 1]);
            if (cells[key.first].empty())
                continue;
            auto left = std::move(cells[key.first]), right = side(key.first, key.second);
            vector<int> q{left.front()};
            q.insert(q.end(), right.begin(), right.end());
            for (int j = (int)left.size() - 1; j >= 1; j--)
                q.push_back(left[j]);
            emit(q);
        }
        sort(out.begin(), out.end(), [&](int a, int b) {
            P u = p[ed[a].b] - p[ed[a].a], v = p[ed[b].b] - p[ed[b].a];
            return u.y / u.x < v.y / v.x;
        });
        int link = lowEdge;
        for (int e : out) {
            after[link] = e;
            before[e] = link;
            link = e;
        }
        after[link] = highEdge;
        before[highEdge] = link;
        cursor = lowEdge;
        vector<int> local{lowEdge};
        local.insert(local.end(), out.begin(), out.end());
        local.push_back(highEdge);
        for (int i = 0; i + 1 < (int)local.size(); i++)
            if (isLand(local[i]))
                cells[local[i]] = side(local[i], local[i + 1]);
    }
    if (after[bottom] != top || cells[bottom].empty())
        throw runtime_error("Unclosed trapezoids");
    auto left = cells[bottom];
    vector<int> q{left.front(), rb, rt};
    for (int j = (int)left.size() - 1; j >= 1; j--)
        q.push_back(left[j]);
    emit(q);
    struct HalfEdge {
        int a, b, t, e;
    };
    vector<HalfEdge> half;
    for (int t = 0; t < (int)mesh.tri.size(); t++)
        for (int e = 0; e < 3; e++) {
            int a = mesh.tri[t].v[e], b = mesh.tri[t].v[(e + 1) % 3];
            if (a > b)
                swap(a, b);
            half.push_back({a, b, t, e});
        }
    vector<HalfEdge> temp(half.size());
    vector<int> count(p.size());
    for (int pass = 0; pass < 2; pass++) {
        fill(count.begin(), count.end(), 0);
        for (auto h : half)
            count[pass ? h.a : h.b]++;
        int sum = 0;
        for (int &c : count) {
            int z = c;
            c = sum;
            sum += z;
        }
        for (auto h : half)
            temp[count[pass ? h.a : h.b]++] = h;
        half.swap(temp);
    }
    for (int i = 0; i + 1 < (int)half.size(); i++)
        if (half[i].a == half[i + 1].a && half[i].b == half[i + 1].b) {
            auto a = half[i], b = half[++i];
            int id = mesh.links.size();
            mesh.links.push_back({a.t, a.e, b.t, b.e});
            mesh.tri[a.t].adj[a.e] = b.t;
            mesh.tri[b.t].adj[b.e] = a.t;
            mesh.tri[a.t].eid[a.e] = mesh.tri[b.t].eid[b.e] = id;
        }
    return mesh;
}

array<int, 2> chooseCuts(const Mesh &m) {
    int T = m.tri.size();
    vector<int> roots, g(T, -1);
    int N = 1;
    for (int t = 0; t < T; t++)
        for (int v : m.tri[t].v)
            if (v == m.source) {
                roots.push_back(t);
                g[t] = 0;
                break;
            }
    for (int t = 0; t < T; t++)
        if (g[t] < 0)
            g[t] = N++;
    vector<vector<pair<int, int>>> a(N);
    for (int e = 0; e < (int)m.links.size(); e++) {
        auto z = m.links[e];
        int u = g[z.t], v = g[z.u];
        if (u == v)
            continue;
        a[u].push_back({v, e});
        a[v].push_back({u, e});
    }
    vector<int> deg(N);
    queue<int> q;
    for (int u = 0; u < N; u++) {
        deg[u] = a[u].size();
        if (deg[u] == 1)
            q.push(u);
    }
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        deg[u] = 0;
        for (auto [v, e] : a[u])
            if (deg[v] > 0 && --deg[v] == 1)
                q.push(v);
    }
    int s = g[roots[0]], c = -1;
    vector<bool> seen(N);
    q.push(s);
    seen[s] = true;
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (deg[u] > 0) {
            c = u;
            break;
        }
        for (auto [v, e] : a[u])
            if (!seen[v])
                seen[v] = true, q.push(v);
    }
    if (c < 0)
        throw runtime_error("Missing dual cycle");
    vector<int> cut;
    for (auto [v, e] : a[c])
        if (deg[v] > 0)
            cut.push_back(e);
    if (cut.size() != 2)
        throw runtime_error("Bad dual cycle");
    return {cut[0], cut[1]};
}

struct Piece {
    P c, a, b;
    ld r;
    int boundary = -1;
};
struct SPMap {
    vector<Piece> pieces;
    vector<array<ld, 3>> vd;
    vector<int> boundary;
};
// Destructive measured 2-3 finger tree. Elements are immutable vertex leaves.
// End operations are amortized O(1); search/split O(log(min(k,n-k)+2)).
struct FingerTree {
    struct Measure {
        int n = 0, first = -1, second = -1, penult = -1, last = -1;
    };
    static Measure cat(Measure a, Measure b) {
        if (!a.n)
            return b;
        if (!b.n)
            return a;
        return {a.n + b.n, a.first, a.n > 1 ? a.second : b.first, b.n > 1 ? b.penult : a.last,
                b.last};
    }
    struct Digit {
        int n = 0;
        array<int, 4> a{};
    };
    struct Node {
        int n = 0;
        array<int, 3> a{};
        Measure m;
    };
    struct Tree {
        Digit l, r;
        int mid = 0;
        Measure m;
    };
    vector<Node> nodes{{}};
    vector<Tree> trees{{}};
    vector<int> freeNode, freeTree;
    int newNode(Node v) {
        if (freeNode.empty()) {
            nodes.push_back(v);
            return nodes.size() - 1;
        }
        int x = freeNode.back();
        freeNode.pop_back();
        nodes[x] = v;
        return x;
    }
    int leaf(int v) {
        Node q;
        q.m = {1, v, -1, -1, v};
        return newNode(q);
    }
    int branch(initializer_list<int> vs) {
        Node q;
        for (int x : vs) {
            q.a[q.n++] = x;
            q.m = cat(q.m, nodes[x].m);
        }
        return newNode(q);
    }
    Measure measure(Digit d) const {
        Measure m;
        for (int i = 0; i < d.n; i++)
            m = cat(m, nodes[d.a[i]].m);
        return m;
    }
    int make(Digit l, int mid, Digit r) {
        Tree t{l, r, mid, cat(cat(measure(l), trees[mid].m), measure(r))};
        if (freeTree.empty()) {
            trees.push_back(t);
            return trees.size() - 1;
        }
        int x = freeTree.back();
        freeTree.pop_back();
        trees[x] = t;
        return x;
    }
    static Digit digit(initializer_list<int> vs) {
        Digit d;
        for (int x : vs)
            d.a[d.n++] = x;
        return d;
    }
    static Digit slice(Digit d, int a, int b) {
        Digit z;
        for (int i = a; i < b; i++)
            z.a[z.n++] = d.a[i];
        return z;
    }
    int single(int x) {
        return make(digit({x}), 0, {});
    }
    int fromDigit(Digit d) {
        if (!d.n)
            return 0;
        if (d.n == 1)
            return single(d.a[0]);
        return make(slice(d, 0, d.n - 1), 0, slice(d, d.n - 1, d.n));
    }
    int prepend(int x, int t) {
        if (!t)
            return single(x);
        Tree z = trees[t];
        freeTree.push_back(t);
        if (!z.r.n)
            return make(digit({x}), 0, z.l);
        if (z.l.n == 4) {
            int b = branch({z.l.a[1], z.l.a[2], z.l.a[3]});
            return make(digit({x, z.l.a[0]}), prepend(b, z.mid), z.r);
        }
        Digit l = digit({x});
        for (int i = 0; i < z.l.n; i++)
            l.a[l.n++] = z.l.a[i];
        return make(l, z.mid, z.r);
    }
    int append(int t, int x) {
        if (!t)
            return single(x);
        Tree z = trees[t];
        freeTree.push_back(t);
        if (!z.r.n)
            return make(z.l, 0, digit({x}));
        if (z.r.n == 4) {
            int b = branch({z.r.a[0], z.r.a[1], z.r.a[2]});
            return make(z.l, append(z.mid, b), digit({z.r.a[3], x}));
        }
        z.r.a[z.r.n++] = x;
        return make(z.l, z.mid, z.r);
    }
    pair<int, int> popFront(int t) {
        Tree z = trees[t];
        freeTree.push_back(t);
        int x = z.l.a[0];
        if (!z.r.n)
            return {x, 0};
        if (z.l.n > 1)
            return {x, make(slice(z.l, 1, z.l.n), z.mid, z.r)};
        if (!z.mid)
            return {x, fromDigit(z.r)};
        auto [v, m] = popFront(z.mid);
        Node b = nodes[v];
        freeNode.push_back(v);
        Digit l;
        for (int i = 0; i < b.n; i++)
            l.a[l.n++] = b.a[i];
        return {x, make(l, m, z.r)};
    }
    pair<int, int> popBack(int t) {
        Tree z = trees[t];
        freeTree.push_back(t);
        if (!z.r.n)
            return {0, z.l.a[0]};
        int x = z.r.a[z.r.n - 1];
        if (z.r.n > 1)
            return {make(z.l, z.mid, slice(z.r, 0, z.r.n - 1)), x};
        if (!z.mid)
            return {fromDigit(z.l), x};
        auto [m, v] = popBack(z.mid);
        Node b = nodes[v];
        freeNode.push_back(v);
        Digit r;
        for (int i = 0; i < b.n; i++)
            r.a[r.n++] = b.a[i];
        return {make(z.l, m, r), x};
    }
    int deepL(Digit l, int m, Digit r) {
        if (l.n)
            return make(l, m, r);
        if (!m)
            return fromDigit(r);
        auto [v, mm] = popFront(m);
        Node b = nodes[v];
        freeNode.push_back(v);
        for (int i = 0; i < b.n; i++)
            l.a[l.n++] = b.a[i];
        return make(l, mm, r);
    }
    int deepR(Digit l, int m, Digit r) {
        if (r.n)
            return make(l, m, r);
        if (!m)
            return fromDigit(l);
        auto [mm, v] = popBack(m);
        Node b = nodes[v];
        freeNode.push_back(v);
        for (int i = 0; i < b.n; i++)
            r.a[r.n++] = b.a[i];
        return make(l, mm, r);
    }
    template <class Pred> int splitDigit(Digit d, Measure acc, const Pred &pred) const {
        for (int i = 0; i < d.n; i++) {
            acc = cat(acc, nodes[d.a[i]].m);
            if (pred(acc))
                return i;
        }
        throw runtime_error("finger digit split");
    }
    // Consumes t and returns the trees before/after the first pivot satisfying pred.
    template <class Pred> tuple<int, int, int> split(int t, Measure acc, const Pred &pred) {
        Tree z = trees[t];
        freeTree.push_back(t);
        if (!z.r.n)
            return {0, z.l.a[0], 0};
        Measure lm = cat(acc, measure(z.l));
        if (pred(lm)) {
            int i = splitDigit(z.l, acc, pred);
            int a = fromDigit(slice(z.l, 0, i));
            int b = deepL(slice(z.l, i + 1, z.l.n), z.mid, z.r);
            return {a, z.l.a[i], b};
        }
        Measure mm = cat(lm, trees[z.mid].m);
        if (pred(mm)) {
            auto [a, v, b] = split(z.mid, lm, pred);
            Node q = nodes[v];
            freeNode.push_back(v);
            Digit d;
            for (int i = 0; i < q.n; i++)
                d.a[d.n++] = q.a[i];
            int i = splitDigit(d, cat(lm, trees[a].m), pred);
            int l = deepR(z.l, a, slice(d, 0, i));
            int r = deepL(slice(d, i + 1, d.n), b, z.r);
            return {l, d.a[i], r};
        }
        int i = splitDigit(z.r, mm, pred);
        int a = deepR(z.l, z.mid, slice(z.r, 0, i));
        int b = fromDigit(slice(z.r, i + 1, z.r.n));
        return {a, z.r.a[i], b};
    }
    template <class Pred> int searchNode(int x, Measure acc, const Pred &pred) const {
        const Node &z = nodes[x];
        if (!z.n)
            return acc.n;
        for (int i = 0; i < z.n; i++) {
            Measure m = cat(acc, nodes[z.a[i]].m);
            if (pred(m))
                return searchNode(z.a[i], acc, pred);
            acc = m;
        }
        throw runtime_error("finger node search");
    }
    template <class Pred> int search(int t, Measure acc, const Pred &pred) const {
        const Tree &z = trees[t];
        for (int i = 0; i < z.l.n; i++) {
            Measure m = cat(acc, nodes[z.l.a[i]].m);
            if (pred(m))
                return searchNode(z.l.a[i], acc, pred);
            acc = m;
        }
        Measure m = cat(acc, trees[z.mid].m);
        if (pred(m))
            return search(z.mid, acc, pred);
        acc = m;
        for (int i = 0; i < z.r.n; i++) {
            m = cat(acc, nodes[z.r.a[i]].m);
            if (pred(m))
                return searchNode(z.r.a[i], acc, pred);
            acc = m;
        }
        throw runtime_error("finger search");
    }
    void consumeNode(int x, vector<int> &out) {
        Node q = nodes[x];
        if (!q.n) {
            out.push_back(q.m.first);
            return;
        }
        freeNode.push_back(x);
        for (int i = 0; i < q.n; i++)
            consumeNode(q.a[i], out);
    }
    void consume(int t, vector<int> &out) {
        if (!t)
            return;
        Tree q = trees[t];
        freeTree.push_back(t);
        for (int i = 0; i < q.l.n; i++)
            consumeNode(q.l.a[i], out);
        consume(q.mid, out);
        for (int i = 0; i < q.r.n; i++)
            consumeNode(q.r.a[i], out);
    }
};

SPMap makeMap(const Mesh &m, int cut, ld k, bool debug = false) {
    int T = m.tri.size();
    // Components of equal triangle corners after opening the slit.
    vector<array<int, 2>> cornerAdj(3 * T, array<int, 2>{-1, -1});
    vector<int> degree(3 * T);
    for (int e = 0; e < (int)m.links.size(); e++)
        if (e != cut) {
            auto z = m.links[e];
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    if (m.tri[z.t].v[i] == m.tri[z.u].v[j]) {
                        int a = 3 * z.t + i, b = 3 * z.u + j;
                        cornerAdj[a][degree[a]++] = b;
                        cornerAdj[b][degree[b]++] = a;
                    }
        }
    vector<int> id(3 * T, -1);
    vector<P> p;
    vector<array<int, 3>> tv(T);
    int s = -1;
    for (int c = 0; c < 3 * T; c++)
        if (id[c] < 0) {
            int v = p.size();
            p.push_back(m.p[m.tri[c / 3].v[c % 3]]);
            vector<int> stack{c};
            id[c] = v;
            while (!stack.empty()) {
                int a = stack.back();
                stack.pop_back();
                tv[a / 3][a % 3] = v;
                if (m.tri[a / 3].v[a % 3] == m.source)
                    s = v;
                for (int j = 0; j < degree[a]; j++) {
                    int b = cornerAdj[a][j];
                    if (id[b] < 0)
                        id[b] = v, stack.push_back(b);
                }
            }
        }
    int N = p.size();
    vector<ld> d(N, numeric_limits<ld>::infinity());
    d[s] = 0;
    FingerTree ft;
    vector<int> leaves(N);
    for (int v = 0; v < N; v++)
        leaves[v] = ft.leaf(v);
    vector<bool> seen(T);
    for (int t = 0; t < T; t++)
        for (int v : tv[t])
            if (v == s) {
                seen[t] = true;
                for (int x : tv[t])
                    d[x] = len(p[x] - p[s]);
                break;
            }
    vector<vector<Piece>> boundaryPieces(3 * T);
    int currentBoundary = -1;
    vector<Piece> ans;
    ld cover = 0;
    long long finalSize = 0;
    auto add = [&](int c, P a, P b) {
        ld ar = cross(a - p[c], b - p[c]);
        if (ar <= EPS)
            return;
        cover += ar / 2;
        if (k > d[c] + EPS)
            boundaryPieces[currentBoundary].push_back({p[c], a, b, k - d[c], currentBoundary});
    };
    auto emit = [&](int tree, int cusp, int boundary) {
        currentBoundary = boundary;
        vector<int> f;
        ft.consume(tree, f);
        finalSize += f.size();
        if (cusp == 0 || cusp + 1 == (int)f.size())
            return;
        int root = f[cusp], a = f.back(), b = f.front();
        P dir = p[b] - p[a];
        ld signedArea = 0;
        for (int i = 0; i < (int)f.size(); i++)
            signedArea += cross(p[f[i]] - p[root], p[f[(i + 1) % f.size()]] - p[root]);
        if (signedArea <= EPS)
            return;
        vector<int> A(f.begin() + cusp, f.end()), B;
        for (int i = cusp; i >= 0; i--)
            B.push_back(f[i]);
        auto project = [&](const vector<int> &path) {
            vector<P> out;
            for (int i = 1; i < (int)path.size(); i++) {
                P u = p[path[i - 1]], z = p[path[i]] - u;
                ld den = cross(dir, z);
                if (fabsl(den) < EPS)
                    throw runtime_error("Parallel funnel ray");
                ld t = cross(u - p[a], z) / den;
                if (debug && (t < -1e-8L || t > 1 + 1e-8L))
                    throw runtime_error("Funnel projection outside edge");
                t = max((ld)0, min((ld)1, t));
                out.push_back(p[a] + dir * t);
            }
            return out;
        };
        auto I = project(A), J = project(B);
        for (int i = (int)A.size() - 2; i >= 1; i--)
            add(A[i], I[i], I[i - 1]);
        add(root, I[0], J[0]);
        for (int i = 1; i + 1 < (int)B.size(); i++)
            add(B[i], J[i - 1], J[i]);
    };
    struct Task {
        int tri, edge, tree, cusp;
    };
    vector<Task> todo;
    auto push = [&](int t, int e, int tree, int cusp) {
        int u = m.tri[t].adj[e];
        if (u < 0 || m.tri[t].eid[e] == cut) {
            emit(tree, cusp, 3 * t + e);
            return;
        }
        if (seen[u])
            throw runtime_error("Non-tree funnel traversal");
        int f = 0;
        while (m.tri[u].adj[f] != t)
            f++;
        seen[u] = true;
        todo.push_back({u, f, tree, cusp});
    };
    // The source star is contracted before traversing the remaining dual tree.
    for (int t = 0; t < T; t++)
        if (find(tv[t].begin(), tv[t].end(), s) != tv[t].end()) {
            for (int e = 0; e < 3; e++) {
                int a = tv[t][e], b = tv[t][(e + 1) % 3];
                if (a == s || b == s)
                    continue;
                int tree = ft.append(ft.append(ft.single(leaves[b]), leaves[s]), leaves[a]);
                push(t, e, tree, 1);
            }
        }
    while (!todo.empty()) {
        auto task = todo.back();
        todo.pop_back();
        int t = task.tri, e = task.edge, tree = task.tree, cusp = task.cusp;
        int w = tv[t][(e + 2) % 3];
        int size = ft.trees[tree].m.n;
        auto pred = [&](FingerTree::Measure a) {
            if (a.n < 2)
                return false;
            ld turn = cross(p[a.last] - p[a.penult], p[w] - p[a.last]);
            return a.n <= cusp + 1 ? turn < -EPS : turn >= -EPS;
        };
        int at = pred(ft.trees[tree].m) ? ft.search(tree, {}, pred) - 1 : size - 1;
        auto [left, pivot, right] =
            ft.split(tree, {}, [&](FingerTree::Measure a) { return a.n > at; });
        int v = ft.nodes[pivot].m.first;
        ld dist = d[v] + len(p[w] - p[v]);
        if (isfinite(d[w]) && fabsl(d[w] - dist) > 1e-8L)
            throw runtime_error("Conflicting shortest distance");
        d[w] = dist;
        int f1 = ft.append(ft.append(left, pivot), leaves[w]);
        int f2 = ft.prepend(leaves[w], ft.prepend(pivot, right));
        push(t, (e + 2) % 3, f1, min(cusp, at));
        push(t, (e + 1) % 3, f2, cusp >= at ? cusp - at + 1 : 1);
    }
    vector<int> outEdge(N, -1);
    int first = -1;
    for (int t = 0; t < T; t++)
        for (int e = 0; e < 3; e++)
            if (m.tri[t].adj[e] < 0 || m.tri[t].eid[e] == cut) {
                int v = tv[t][e];
                if (outEdge[v] >= 0)
                    throw runtime_error("Non-simple slit boundary");
                outEdge[v] = 3 * t + e;
                first = 3 * t + e;
            }
    vector<int> boundaryOrder;
    int edge = first;
    do {
        boundaryOrder.push_back(edge);
        for (auto a : boundaryPieces[edge])
            ans.push_back(a);
        int t = edge / 3, e = edge % 3;
        edge = outEdge[tv[t][(e + 1) % 3]];
        if (edge < 0)
            throw runtime_error("Open slit boundary");
    } while (edge != first);
    SPMap result;
    result.pieces = std::move(ans);
    result.vd.resize(T);
    result.boundary = std::move(boundaryOrder);
    for (int t = 0; t < T; t++)
        for (int i = 0; i < 3; i++)
            result.vd[t][i] = d[tv[t][i]];
    if (debug) {
        ld area = 0;
        for (auto t : m.tri)
            area += cross(m.p[t.v[1]] - m.p[t.v[0]], m.p[t.v[2]] - m.p[t.v[0]]) / 2;
        cerr << "finger map " << cut << " vertices " << N << " pieces " << result.pieces.size()
             << " final funnel size " << finalSize << " cover difference " << setprecision(18)
             << cover - area << "\n";
    }
    return result;
}

struct Arc {
    P c;
    ld r, l, h;
    int boundary;
};
struct Curve {
    // A line, or one x-monotone branch of a circle. side=0: lower; side=1: upper.
    ld l, h;
    int side, type;
    P a, b;
    ld r;
    ld value(ld x) const {
        if (!type)
            return a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x);
        ld y = sqrtl(max((ld)0, r * r - (x - a.x) * (x - a.x)));
        return a.y + (side ? y : -y);
    }
    ld integral(ld x, ld y) const {
        if (!type)
            return value((x + y) / 2) * (y - x);
        auto primitive = [&](ld z) {
            z = max(-r, min(r, z - a.x));
            return (z * sqrtl(max((ld)0, r * r - z * z)) +
                    r * r * asinl(max((ld)-1, min((ld)1, z / r)))) /
                   2;
        };
        return a.y * (y - x) + (side ? 1 : -1) * (primitive(y) - primitive(x));
    }
};
vector<Arc> boundaryArcs(const SPMap &sp) {
    vector<Arc> out;
    for (auto p : sp.pieces) {
        ld l = atan2l(p.a.y - p.c.y, p.a.x - p.c.x), h = atan2l(p.b.y - p.c.y, p.b.x - p.c.x);
        while (l < 0)
            l += TAU;
        while (h < l)
            h += TAU;
        P v = p.b - p.a, u = p.a - p.c;
        ld aa = norm2(v), bb = dot(v, u), cc = norm2(u) - p.r * p.r;
        ld dd = bb * bb - aa * cc;
        // A mathematically tangent line can produce a tiny positive discriminant
        // after the funnel projection.  Treat such round-off as an exact tangent;
        // otherwise it creates a bogus O(1e-9)-radian arc that has no adjacent
        // triangle to cross into.
        ld discScale = bb * bb + fabsl(aa * cc);
        ld discTol = 64 * numeric_limits<ld>::epsilon() * max((ld)1, discScale);
        if (fabsl(dd) <= discTol)
            dd = 0;
        vector<ld> events{l, h};
        if (dd > 0) {
            ld z = sqrtl(dd);
            for (ld t : {(-bb - z) / aa, (-bb + z) / aa}) {
                P q = p.a + v * t;
                ld angle = atan2l(q.y - p.c.y, q.x - p.c.x);
                while (angle < l)
                    angle += TAU;
                if (angle > l + 1e-14L && angle < h - 1e-14L)
                    events.push_back(angle);
            }
        }
        sort(events.begin(), events.end()); // at most four entries
        for (int i = 0; i + 1 < (int)events.size(); i++) {
            ld a = events[i], b = events[i + 1], mid = (a + b) / 2;
            P q = p.c + P{cosl(mid), sinl(mid)} * p.r;
            if (b - a > 1e-12L && cross(v, q - p.a) >= 0)
                out.push_back({p.c, p.r, a, b, p.boundary});
        }
    }
    return out;
}

vector<vector<Curve>> splitRegions(const Mesh &m, const SPMap &sp, int cut, ld k) {
    int T = m.tri.size();
    vector<vector<Curve>> result(T);
    vector<vector<ld>> hits(3 * T);
    auto arcs = boundaryArcs(sp);

    vector<vector<int>> incident(m.p.size());
    for (int t = 0; t < T; t++)
        for (int v : m.tri[t].v)
            incident[v].push_back(t);
    auto record = [&](int t, P q) {
        for (int e = 0; e < 3; e++) {
            P a = m.p[m.tri[t].v[e]], b = m.p[m.tri[t].v[(e + 1) % 3]], v = b - a;
            ld u = dot(q - a, v) / norm2(v);
            if (u < -1e-10L || u > 1 + 1e-10L || fabsl(cross(v, q - a)) > 1e-9L * len(v))
                continue;
            u = max((ld)0, min((ld)1, u));
            hits[3 * t + e].push_back(u);
            int z = m.tri[t].adj[e];
            if (z >= 0 && m.tri[t].eid[e] != cut) {
                int f = 0;
                while (m.tri[z].adj[f] != t)
                    f++;
                hits[3 * z + f].push_back(1 - u);
            }
        }
    };
    auto afterInside = [&](int t, const Arc &a, ld ang, int dir) {
        P u{cosl(ang), sinl(ang)}, q = a.c + u * a.r, v = P{-u.y, u.x} * (a.r * dir),
                                   acc = u * (-a.r);
        for (int e = 0; e < 3; e++) {
            P A = m.p[m.tri[t].v[e]], B = m.p[m.tri[t].v[(e + 1) % 3]], edge = B - A;
            ld z = cross(edge, q - A), tol = 1e-10L * len(edge);
            if (z > tol)
                continue;
            if (z < -tol)
                return false;
            z = cross(edge, v);
            if (z > tol)
                continue;
            if (z < -tol)
                return false;
            if (cross(edge, acc) < -tol)
                return false;
        }
        return true;
    };
    auto nextTriangle = [&](int t, const Arc &a, ld ang, int dir, bool initial) {
        if (t < 0)
            throw runtime_error("Point located outside mesh");
        if (afterInside(t, a, ang, dir))
            return t;
        P q = a.c + P{cosl(ang), sinl(ang)} * a.r;
        int vertex = -1;
        for (int v : m.tri[t].v)
            if (norm2(q - m.p[v]) < 1e-16L)
                vertex = v;
        if (vertex >= 0) {
            for (int u : incident[vertex])
                if (afterInside(u, a, ang, dir))
                    return u;
        } else {
            for (int e = 0; e < 3; e++) {
                int u = m.tri[t].adj[e];
                if (u >= 0 && (initial || m.tri[t].eid[e] != cut) && afterInside(u, a, ang, dir))
                    return u;
            }
        }
        cerr << setprecision(21) << "cross failed t " << t << " c " << a.c.x << "," << a.c.y
             << " r " << a.r << " angles " << a.l << "," << a.h << " cur " << ang << " dir " << dir
             << " point " << q.x << "," << q.y << " vertex " << vertex << "\n";
        for (int e = 0; e < 3; e++) {
            int v = m.tri[t].v[e];
            P p = m.p[v];
            cerr << "v " << v << " " << p.x << "," << p.y << " adj " << m.tri[t].adj[e] << "\n";
        }
        throw runtime_error("Arc could not cross mesh edge");
    };
    auto addArc = [&](int t, const Arc &a, ld l, ld r) {
        if (l > r)
            swap(l, r);
        vector<ld> p{l, r};
        for (int i = 1; i <= 4; i++)
            if (l < i * PI && i * PI < r)
                p.push_back(i * PI);
        sort(p.begin(), p.end()); // constant size

        for (int i = 0; i + 1 < (int)p.size(); i++) {
            ld u = p[i], v = p[i + 1];
            if (v - u < 1e-14L)
                continue;
            ld x = a.c.x + a.r * cosl(u), y = a.c.x + a.r * cosl(v);
            if (x > y)
                swap(x, y);
            if (y - x > 1e-13L)
                result[t].push_back({x, y, sinl((u + v) / 2) > 0, 1, a.c, {}, a.r});
        }
    };
    ld coordinateScale = 1;
    for (P p : m.p)
        coordinateScale = max(coordinateScale, max(fabsl(p.x), fabsl(p.y)));
    long long steps = 0;
    int lastTriangle = -1;
    P lastPoint{};
    int boundaryCursor = 0, boundarySteps = 0;
    for (int id = 0; id < (int)arcs.size(); id++) {
        const Arc &a = arcs[id];
        P start = a.c + P{cosl(a.l), sinl(a.l)} * a.r;
        int t = -1;
        if (lastTriangle >= 0 && norm2(start - lastPoint) < 1e-14L)
            t = lastTriangle;
        else {
            if (id == 0) {
                for (int z = 0; z < T; z++)
                    if (afterInside(z, a, a.l, 1)) {
                        t = z;
                        break;
                    }
            } else {
                for (;;) {
                    int bd = sp.boundary[boundaryCursor], bt = bd / 3, be = bd % 3;
                    P u = m.p[m.tri[bt].v[be]], v = m.p[m.tri[bt].v[(be + 1) % 3]], d = v - u;
                    ld f = dot(start - u, d) / norm2(d);
                    if (f >= -1e-10L && f <= 1 + 1e-10L &&
                        fabsl(cross(d, start - u)) < 1e-9L * len(d)) {
                        t = bt;
                        break;
                    }
                    boundaryCursor = (boundaryCursor + 1) % sp.boundary.size();
                    if (++boundarySteps > 2 * (int)sp.boundary.size())
                        throw runtime_error("Discontinuous ordered wavefront");
                }
            }
        }
        t = nextTriangle(t, a, a.l, 1, true);
        ld cur = a.l, end = a.h;
        // An endpoint on a mesh edge can have slightly different angles when
        // reconstructed from the funnel and from a circle-line intersection.
        const ld endTolerance =
            max(1e-12L, 256 * numeric_limits<ld>::epsilon() * (1 + coordinateScale / a.r));
        record(t, start);
        while (end - cur > 1e-13L) {
            if (++steps > 100LL * (T + (long long)arcs.size() + 1))
                throw runtime_error("Too many arc crossings");
            ld next = end;
            for (int e = 0; e < 3; e++) {
                P A = m.p[m.tri[t].v[e]], B = m.p[m.tri[t].v[(e + 1) % 3]], v = B - A;
                ld vv = norm2(v), u = dot(a.c - A, v) / vv;
                P foot = A + v * u;
                ld delta = a.r * a.r - norm2(foot - a.c);
                ld tol = 64 * numeric_limits<ld>::epsilon() * max(a.r * a.r, norm2(foot - a.c));
                if (fabsl(delta) <= tol)
                    delta = 0;
                if (delta < 0)
                    continue;
                ld z = sqrtl(delta / vv);
                for (ld f : {u - z, u + z})
                    if (f >= -1e-12L && f <= 1 + 1e-12L) {
                        P q = A + v * max((ld)0, min((ld)1, f));
                        ld ang = atan2l(q.y - a.c.y, q.x - a.c.x);
                        while (ang < cur - 1e-12L)
                            ang += TAU;
                        if (ang - cur > 1e-12L && next > ang)
                            next = ang;
                    }
            }
            if (end - next <= endTolerance)
                next = end;
            addArc(t, a, cur, next);
            P q = a.c + P{cosl(next), sinl(next)} * a.r;
            record(t, q);
            cur = next;
            if (fabsl(cur - end) < 1e-12L)
                break;
            t = nextTriangle(t, a, cur, 1, false);
        }
        lastTriangle = t;
        lastPoint = a.c + P{cosl(end), sinl(end)} * a.r;
    }
    for (int t = 0; t < T; t++)
        for (int e = 0; e < 3; e++) {
            bool ia = sp.vd[t][e] <= k + 1e-11L, ib = sp.vd[t][(e + 1) % 3] <= k + 1e-11L;
            auto &z = hits[3 * t + e];
            ld l = 1, h = 0;
            if (ia)
                l = h = 0;
            if (ib)
                l = min(l, (ld)1), h = 1;
            for (ld x : z)
                l = min(l, x), h = max(h, x);
            if (h - l <= 1e-13L)
                continue;
            P A = m.p[m.tri[t].v[e]], B = m.p[m.tri[t].v[(e + 1) % 3]], a = A + (B - A) * l,
              b = A + (B - A) * h;
            if (fabsl(a.x - b.x) > 1e-13L)
                result[t].push_back({min(a.x, b.x), max(a.x, b.x), a.x > b.x, 0, a, b, 0});
        }
    return result;
}

vector<ld> crossings(const Curve &a, const Curve &b) {
    vector<ld> out;
    if (!a.type && !b.type) {
        P u = a.b - a.a, v = b.b - b.a;
        ld d = cross(u, v);
        if (fabsl(d) > 1e-24L)
            out.push_back((a.a + u * (cross(b.a - a.a, v) / d)).x);
    } else if (a.type && b.type) {
        P v = b.a - a.a;
        ld dd = norm2(v), d = sqrtl(dd);
        if (d < 1e-12L)
            return out;
        ld x = (dd + a.r * a.r - b.r * b.r) / (2 * d), h2 = a.r * a.r - x * x;
        if (h2 < -1e-10L)
            return out;
        P u = v * (1 / d), q = a.a + u * x, w = P{-u.y, u.x} * sqrtl(max((ld)0, h2));
        out.push_back((q - w).x);
        out.push_back((q + w).x);
    } else {
        const Curve &c = a.type ? a : b;
        const Curve &s = a.type ? b : a;
        P v = s.b - s.a, u = s.a - c.a;
        ld A = norm2(v), B = dot(u, v), C = norm2(u) - c.r * c.r, D = B * B - A * C;
        if (D < -1e-10L)
            return out;
        ld z = sqrtl(max((ld)0, D));
        out.push_back((s.a + v * ((-B - z) / A)).x);
        out.push_back((s.a + v * ((-B + z) / A)).x);
    }
    return out;
}
array<vector<Curve>, 2> orderedCurves(const vector<Curve> &curves) {
    array<vector<Curve>, 2> arcs, lines;
    for (auto c : curves)
        (c.type ? arcs[c.side] : lines[c.side]).push_back(c);
    for (int side = 0; side < 2; side++) {
        auto &a = arcs[side];
        if (side)
            reverse(a.begin(), a.end());
        if (!a.empty()) {
            int at = min_element(a.begin(), a.end(),
                                 [](const Curve &x, const Curve &y) { return x.l < y.l; }) -
                     a.begin();
            rotate(a.begin(), a.begin() + at, a.end());
            for (int i = 1; i < (int)a.size(); i++)
                if (a[i].l < a[i - 1].l - 1e-9L)
                    throw runtime_error("Unordered convex arc boundary");
        }
        auto &b = lines[side];
        sort(b.begin(), b.end(), [](const Curve &x, const Curve &y) { return x.l < y.l; }); // <= 3
        vector<Curve> all;
        all.reserve(a.size() + b.size());
        int i = 0, j = 0;
        while (i < (int)a.size() || j < (int)b.size()) {
            if (j == (int)b.size() || (i < (int)a.size() && a[i].l < b[j].l))
                all.push_back(a[i++]);
            else
                all.push_back(b[j++]);
        }
        a = std::move(all);
    }
    return arcs;
}
ld unionArea(const vector<Curve> &aa, const vector<Curve> &bb) {
    auto a = orderedCurves(aa), b = orderedCurves(bb);
    array<vector<Curve>, 4> lists{std::move(a[0]), std::move(a[1]), std::move(b[0]),
                                  std::move(b[1])};
    array<int, 4> at{};
    ld ans = 0, l = numeric_limits<ld>::infinity();
    for (auto &v : lists)
        if (!v.empty())
            l = min(l, v[0].l);
    while (isfinite(l)) {
        ld r = numeric_limits<ld>::infinity();
        array<const Curve *, 4> active{};
        for (int g = 0; g < 4; g++) {
            auto &v = lists[g];
            int &i = at[g];
            while (i < (int)v.size() && v[i].h <= l + 1e-12L)
                i++;
            if (i == (int)v.size())
                continue;
            if (v[i].l <= l + 1e-12L) {
                active[g] = &v[i];
                r = min(r, v[i].h);
            } else
                r = min(r, v[i].l);
        }
        if (!isfinite(r))
            break;
        bool has0 = active[0] && active[1], has1 = active[2] && active[3];
        if (r - l > 1e-13L && (has0 || has1)) {
            vector<ld> xs{l, r};
            if (has0 && has1)
                for (int i = 0; i < 4; i++)
                    for (int j = i + 1; j < 4; j++)
                        for (ld x : crossings(*active[i], *active[j]))
                            if (x > l + 1e-12L && x < r - 1e-12L)
                                xs.push_back(x);
            sort(xs.begin(), xs.end()); // <=14 entries
            for (int q = 0; q + 1 < (int)xs.size(); q++) {
                ld x = xs[q], y = xs[q + 1], mid = (x + y) / 2;
                if (y - x < 1e-13L)
                    continue;
                auto integral = [&](int hi, int lo) {
                    return active[hi]->integral(x, y) - active[lo]->integral(x, y);
                };
                if (!has1) {
                    ans += integral(1, 0);
                    continue;
                }
                if (!has0) {
                    ans += integral(3, 2);
                    continue;
                }
                ld lo0 = active[0]->value(mid), hi0 = active[1]->value(mid),
                   lo1 = active[2]->value(mid), hi1 = active[3]->value(mid);
                if (hi0 < lo1 || hi1 < lo0)
                    ans += integral(1, 0) + integral(3, 2);
                else
                    ans += integral(hi0 > hi1 ? 1 : 3, lo0 < lo1 ? 0 : 2);
            }
        }
        l = r;
    }
    return ans;
}

ld solve(vector<P> poly, ld k, bool debug = false) {
    // The source may lie in the relative interior of a lake edge.
    vector<P> p;
    for (int i = 0; i < (int)poly.size(); i++) {
        P a = poly[i], b = poly[(i + 1) % poly.size()];
        p.push_back(a);
        if (norm2(a) > EPS && norm2(b) > EPS && fabsl(cross(a, b)) < EPS && dot(a, b) < 0)
            p.push_back({0, 0});
    }
    if (area2(p) > 0)
        reverse(p.begin(), p.end());
    Mesh m = triangulate(p, k);
    auto cuts = chooseCuts(m);
    auto aa = makeMap(m, cuts[0], k, debug), bb = makeMap(m, cuts[1], k, debug);
    auto a = splitRegions(m, aa, cuts[0], k), b = splitRegions(m, bb, cuts[1], k);
    ld ans = 0;
    for (int t = 0; t < (int)m.tri.size(); t++)
        ans += unionArea(a[t], b[t]);

    return max((ld)0, ans);
}
int main(int argc, char **) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    ld k;
    bool debug = argc > 1;
    while (cin >> n >> k) {
        vector<P> p(n);
        for (P &a : p)
            cin >> a.x >> a.y;
        try {
            cout << fixed << setprecision(12) << solve(p, k, debug) << '\n';
        } catch (exception &e) {
            cerr << e.what() << '\n';
            return 1;
        }
    }
}

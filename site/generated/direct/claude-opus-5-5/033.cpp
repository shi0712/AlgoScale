#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef __int128_t lll;

struct P {
    ll x, y;
    P(ll x_ = 0, ll y_ = 0) : x(x_), y(y_) {}
    bool operator<(const P& o) const { return tie(x, y) < tie(o.x, o.y); }
    bool operator==(const P& o) const { return x == o.x && y == o.y; }
    P operator-(const P& o) const { return P(x - o.x, y - o.y); }
    ll cross(const P& b) const { return x * b.y - y * b.x; }
    ll cross(const P& a, const P& b) const { return (a - *this).cross(b - *this); }
    ll dist2() const { return x * x + y * y; }
};
P arb(LLONG_MAX, LLONG_MAX);
struct Quad;
typedef Quad* Q;
struct Quad {
    Q rot, o;
    P p = arb;
    bool mark;
    P& F() { return r()->p; }
    Q& r() { return rot->rot; }
    Q prev() { return rot->o->rot; }
    Q next() { return r()->prev(); }
};
Q HH = 0;

bool circ(P p, P a, P b, P c) {
    lll p2 = p.dist2(), A = a.dist2() - p2, B = b.dist2() - p2, C = c.dist2() - p2;
    return (lll)p.cross(a, b) * C + (lll)p.cross(b, c) * A + (lll)p.cross(c, a) * B > 0;
}
Q makeEdge(P orig, P dest) {
    Q r;
    if (HH) r = HH;
    else {
        Q q0 = new Quad{0, 0, arb, false};
        Q q1 = new Quad{q0, 0, arb, false};
        Q q2 = new Quad{q1, 0, arb, false};
        r = new Quad{q2, 0, arb, false};
    }
    HH = r->o;
    r->r()->r() = r;
    for (int i = 0; i < 4; i++) {
        r = r->rot;
        r->p = arb;
        r->o = (i & 1) ? r : r->r();
    }
    r->p = orig;
    r->F() = dest;
    return r;
}
void splice(Q a, Q b) {
    swap(a->o->rot->o, b->o->rot->o);
    swap(a->o, b->o);
}
Q connect(Q a, Q b) {
    Q q = makeEdge(a->F(), b->p);
    splice(q, a->next());
    splice(q->r(), b);
    return q;
}
#define HE(e) e->F(), e->p
#define valid(e) (e->F().cross(HE(base)) > 0)
pair<Q, Q> rec(const vector<P>& s) {
    if (s.size() <= 3) {
        Q a = makeEdge(s[0], s[1]), b = makeEdge(s[1], s.back());
        if (s.size() == 2) return {a, a->r()};
        splice(a->r(), b);
        auto side = s[0].cross(s[1], s[2]);
        Q c = side ? connect(b, a) : 0;
        return {side < 0 ? c->r() : a, side < 0 ? c : b->r()};
    }
    Q A, B, ra, rb;
    int half = s.size() / 2;
    tie(ra, A) = rec(vector<P>(s.begin(), s.end() - half));
    tie(B, rb) = rec(vector<P>(s.begin() + (s.size() - half), s.end()));
    while ((B->p.cross(HE(A)) < 0 && (A = A->next())) ||
           (A->p.cross(HE(B)) > 0 && (B = B->r()->o)));
    Q base = connect(B->r(), A);
    if (A->p == ra->p) ra = base->r();
    if (B->p == rb->p) rb = base;
#define DEL(e, init, dir) \
    Q e = init->dir; \
    if (valid(e)) \
        while (circ(e->dir->F(), HE(base), e->F())) { \
            Q t = e->dir; \
            splice(e, e->prev()); \
            splice(e->r(), e->r()->prev()); \
            e->o = HH; HH = e; e = t; \
        }
    for (;;) {
        DEL(LC, base->r(), o);
        DEL(RC, base, prev());
        if (!valid(LC) && !valid(RC)) break;
        if (!valid(LC) || (valid(RC) && circ(HE(RC), HE(LC))))
            base = connect(RC, base->r());
        else
            base = connect(base->r(), LC->r());
    }
    return {ra, rb};
}
vector<P> triangulate(vector<P> pts) {
    sort(pts.begin(), pts.end());
    if (pts.size() < 2) return {};
    Q e = rec(pts).first;
    vector<Q> q = {e};
    int qi = 0;
    while (e->o->F().cross(e->F(), e->p) < 0) e = e->o;
#define ADD { Q c = e; do { c->mark = 1; pts.push_back(c->p); \
    q.push_back(c->r()); c = c->next(); } while (c != e); }
    ADD; pts.clear();
    while (qi < (int)q.size()) if (!(e = q[qi++])->mark) ADD;
    return pts;
}

int n;
vector<ll> X, Y;
vector<double> ex1, ey1, ex2, ey2;
vector<char> leftw, active;
double curX, qyv;
double yAt(int id) {
    if (id == n) return qyv;
    return ey1[id] + (ey2[id] - ey1[id]) * (curX - ex1[id]) / (ex2[id] - ex1[id]);
}
double slopeOf(int id) {
    if (id == n) return 0;
    return (ey2[id] - ey1[id]) / (ex2[id] - ex1[id]);
}
struct Cmp {
    bool operator()(int a, int b) const {
        if (a == b) return false;
        double ya = yAt(a), yb = yAt(b);
        if (fabs(ya - yb) > 1e-9) return ya < yb;
        return slopeOf(a) < slopeOf(b);
    }
};

int main() {
    scanf("%d", &n);
    X.resize(n); Y.resize(n);
    vector<P> pts(n);
    for (int i = 0; i < n; i++) {
        scanf("%lld %lld", &X[i], &Y[i]);
        pts[i] = P(X[i], Y[i]);
    }
    vector<pair<ll, int>> keys(n);
    for (int i = 0; i < n; i++) keys[i] = {(X[i] + 10000) * 20001 + (Y[i] + 10000), i};
    sort(keys.begin(), keys.end());
    auto getId = [&](const P& p) {
        ll k = (p.x + 10000) * 20001 + (p.y + 10000);
        auto it = lower_bound(keys.begin(), keys.end(), make_pair(k, -1));
        return it->second;
    };
    vector<P> tri = triangulate(pts);
    vector<vector<int>> adj(n);
    vector<double> qx, qy, qr;
    for (size_t k = 0; k + 2 < tri.size(); k += 3) {
        int id[3];
        for (int j = 0; j < 3; j++) id[j] = getId(tri[k + j]);
        for (int j = 0; j < 3; j++) {
            int u = id[j], v = id[(j + 1) % 3];
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
        P a = tri[k], b = tri[k + 1], c = tri[k + 2];
        double bx = b.x - a.x, by = b.y - a.y, cx = c.x - a.x, cy = c.y - a.y;
        double d = 2.0 * (bx * cy - by * cx);
        if (d == 0) continue;
        double b2 = bx * bx + by * by, c2 = cx * cx + cy * cy;
        double ux = (cy * b2 - by * c2) / d, uy = (bx * c2 - cx * b2) / d;
        double px = a.x + ux, py = a.y + uy;
        if (fabs(px) > 10000.5 || fabs(py) > 10000.5) continue;
        qx.push_back(px); qy.push_back(py); qr.push_back(sqrt(ux * ux + uy * uy));
    }
    for (int i = 0; i < n; i++) {
        sort(adj[i].begin(), adj[i].end());
        adj[i].erase(unique(adj[i].begin(), adj[i].end()), adj[i].end());
    }
    double ans = 0;
    // interior candidates via sweep
    ex1.assign(n + 1, 0); ey1.assign(n + 1, 0); ex2.assign(n + 1, 0); ey2.assign(n + 1, 0);
    leftw.assign(n + 1, 0); active.assign(n + 1, 0);
    struct Ev { double x; int type; int id; };
    vector<Ev> evs;
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        if (X[i] == X[j]) continue;
        active[i] = 1;
        if (X[i] < X[j]) {
            ex1[i] = X[i]; ey1[i] = Y[i]; ex2[i] = X[j]; ey2[i] = Y[j]; leftw[i] = 0;
        } else {
            ex1[i] = X[j]; ey1[i] = Y[j]; ex2[i] = X[i]; ey2[i] = Y[i]; leftw[i] = 1;
        }
        evs.push_back({ex1[i], 1, i});
        evs.push_back({ex2[i], 0, i});
    }
    int Qn = qx.size();
    for (int i = 0; i < Qn; i++) evs.push_back({qx[i], 2, i});
    sort(evs.begin(), evs.end(), [](const Ev& a, const Ev& b) {
        if (a.x != b.x) return a.x < b.x;
        return a.type < b.type;
    });
    set<int, Cmp> S;
    vector<set<int, Cmp>::iterator> its(n);
    for (auto& e : evs) {
        if (e.type == 0) {
            S.erase(its[e.id]);
        } else if (e.type == 1) {
            curX = e.x;
            its[e.id] = S.insert(e.id).first;
        } else {
            curX = e.x;
            qyv = qy[e.id];
            auto it = S.lower_bound(n);
            if (it != S.end() && leftw[*it]) ans = max(ans, qr[e.id]);
        }
    }
    // boundary walk
    for (int i = 0; i < n; i++) {
        int j = (i + 1) % n;
        ll dx = X[j] - X[i], dy = Y[j] - Y[i];
        int s = i;
        double tcur = 0;
        long long guard = 0;
        while (true) {
            if (++guard > 4LL * n + 100) break;
            double bt = 1e18;
            int bc = -1;
            ll as = (X[i] - X[s]) * (X[i] - X[s]) + (Y[i] - Y[s]) * (Y[i] - Y[s]);
            for (int c : adj[s]) {
                ll den = 2 * (dx * (X[c] - X[s]) + dy * (Y[c] - Y[s]));
                if (den <= 0) continue;
                ll ac = (X[i] - X[c]) * (X[i] - X[c]) + (Y[i] - Y[c]) * (Y[i] - Y[c]);
                double t = (double)(ac - as) / (double)den;
                if (t < tcur) t = tcur;
                if (t < bt) { bt = t; bc = c; }
            }
            if (bc == -1 || bt > 1.0 + 1e-12) break;
            double t = min(bt, 1.0);
            double px = X[i] + t * dx, py = Y[i] + t * dy;
            double dd = hypot(px - X[s], py - Y[s]);
            ans = max(ans, dd);
            s = bc;
            tcur = bt;
        }
    }
    printf("%.9f\n", ans);
    return 0;
}

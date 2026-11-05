#include <bits/stdc++.h>
using namespace std;

struct Pt {
    double x, y;
    Pt() {}
    Pt(double x, double y) : x(x), y(y) {}
};
static inline Pt operator+(Pt a, Pt b) { return Pt(a.x + b.x, a.y + b.y); }
static inline Pt operator-(Pt a, Pt b) { return Pt(a.x - b.x, a.y - b.y); }
static inline Pt operator*(Pt a, double t) { return Pt(a.x * t, a.y * t); }
static inline double cross(Pt a, Pt b) { return a.x * b.y - a.y * b.x; }
static inline double dot(Pt a, Pt b) { return a.x * b.x + a.y * b.y; }

int n, m;
vector<Pt> poly, obs;

struct Seg {
    Pt a, b;
    int delta;   // +1 if dx > 0 (crossing the sweep line upward enters the region)
    bool isWin;  // window (radial) segment
    double xmin, xmax, ymin, ymax;
    double sl, c0;  // line: y = c0 + sl * x
};

static void addSeg(vector<Seg> &segs, Pt u, Pt v, bool isWin) {
    if (fabs(u.x - v.x) < 1e-12) return;  // vertical / degenerate -> no area effect
    Seg s;
    s.a = u;
    s.b = v;
    s.delta = (v.x > u.x) ? 1 : -1;
    s.isWin = isWin;
    s.xmin = min(u.x, v.x);
    s.xmax = max(u.x, v.x);
    s.ymin = min(u.y, v.y);
    s.ymax = max(u.y, v.y);
    s.sl = (v.y - u.y) / (v.x - u.x);
    s.c0 = u.y - s.sl * u.x;
    segs.push_back(s);
}

/* ray p + t*d (t>0) against segment a-b; returns t or -1 */
static inline double rayEdge(Pt p, Pt d, Pt a, Pt b) {
    Pt e = b - a;
    double den = cross(d, e);
    if (fabs(den) < 1e-9) return -1;
    double t = cross(a - p, e) / den;
    if (t <= 0) return -1;
    double s = cross(a - p, d) / den;
    if (s < -1e-12 || s > 1.0 + 1e-12) return -1;
    return t;
}

static inline Pt hitLine(Pt p, double th, Pt a, Pt b) {
    Pt d(cos(th), sin(th));
    Pt e = b - a;
    double den = cross(d, e);
    if (fabs(den) < 1e-12) {
        double ta = dot(a - p, d), tb = dot(b - p, d);
        return (ta < tb) ? a : b;
    }
    double t = cross(a - p, e) / den;
    return p + d * t;
}

/* Build the visibility polygon of p and append its boundary as oriented segments. */
static void buildVis(Pt p, vector<Seg> &segs) {
    vector<double> ang(n);
    for (int i = 0; i < n; i++) ang[i] = atan2(poly[i].y - p.y, poly[i].x - p.x);
    sort(ang.begin(), ang.end());
    vector<double> A;
    A.reserve(n);
    for (double a : ang)
        if (A.empty() || a - A.back() > 1e-12) A.push_back(a);
    int K = (int)A.size();
    if (K < 2) return;

    vector<int> nearE(K, -1);
    for (int k = 0; k < K; k++) {
        double a1 = A[k];
        double a2 = (k + 1 < K) ? A[k + 1] : A[0] + 2.0 * M_PI;
        double mid = 0.5 * (a1 + a2);
        Pt d(cos(mid), sin(mid));
        double best = 1e300;
        int be = -1;
        for (int i = 0; i < n; i++) {
            double t = rayEdge(p, d, poly[i], poly[(i + 1) % n]);
            if (t > 0 && t < best) { best = t; be = i; }
        }
        nearE[k] = be;
    }

    int start = 0;
    for (int k = 0; k < K; k++)
        if (nearE[k] != nearE[(k - 1 + K) % K]) { start = k; break; }

    vector<Pt> firsts, seconds;
    int k = start;
    for (int iter = 0; iter <= K; iter++) {
        int e = nearE[k];
        if (e < 0) break;
        int kend = k;
        for (int step = 0; step < K; step++) {
            int nx = (kend + 1) % K;
            if (nx == start) break;
            if (nearE[nx] != e) break;
            kend = nx;
        }
        Pt A1 = hitLine(p, A[k], poly[e], poly[(e + 1) % n]);
        Pt B1 = hitLine(p, A[(kend + 1) % K], poly[e], poly[(e + 1) % n]);
        firsts.push_back(A1);
        seconds.push_back(B1);
        k = (kend + 1) % K;
        if (k == start) break;
    }

    int R = (int)firsts.size();
    if (R == 0) return;
    for (int i = 0; i < R; i++) {
        addSeg(segs, firsts[i], seconds[i], false);           // boundary piece
        addSeg(segs, seconds[i], firsts[(i + 1) % R], true);  // window
    }
}

/* ---------------- segment intersection ---------------- */
struct Cross {
    double x;
    int i, j;
};

static inline bool segInterX(const Seg &s1, const Seg &s2, double &outX) {
    Pt d1 = s1.b - s1.a, d2 = s2.b - s2.a;
    double den = cross(d1, d2);
    if (fabs(den) < 1e-14) return false;
    Pt r = s2.a - s1.a;
    double t = cross(r, d2) / den;
    double u = cross(r, d1) / den;
    if (t < -1e-12 || t > 1.0 + 1e-12) return false;
    if (u < -1e-12 || u > 1.0 + 1e-12) return false;
    outX = s1.a.x + d1.x * t;
    return true;
}

/* ---------------- incremental vertical sweep ---------------- */
static vector<Seg> *GS;  // current segment array
static double curX;
static vector<vector<int>> blk;
static const int BLOCKSZ = 40;

struct BAgg {
    int SD;
    int mP;  // minimum prefix depth over internal gaps (INT_MAX if none)
    double mW0, mW1;
    double f0, f1, l0, l1;
};
static vector<BAgg> agg;

static inline double segYof(int id, double x) { return (*GS)[id].c0 + (*GS)[id].sl * x; }

/* Order of segments at the current sweep position.
   Comparisons closer than EPS are treated as exact ties and broken by slope:
   this reproduces the (symbolically perturbed) exact order for degenerate
   configurations, whose floating point images are only accurate to ~1e-12. */
static const double EPS = 1e-9;
static inline bool segLessId(int a, int b) {
    const Seg &A = (*GS)[a], &B = (*GS)[b];
    double ya = A.c0 + A.sl * curX, yb = B.c0 + B.sl * curX;
    if (ya < yb - EPS) return true;
    if (ya > yb + EPS) return false;
    if (A.sl != B.sl) return A.sl < B.sl;
    return a < b;
}

static void rebuild(int bi) {
    BAgg &g = agg[bi];
    const vector<int> &v = blk[bi];
    g.SD = 0;
    g.mP = INT_MAX;
    g.mW0 = g.mW1 = 0.0;
    const Seg &s0 = (*GS)[v[0]];
    g.f0 = s0.c0;
    g.f1 = s0.sl;
    double p0 = s0.c0, p1 = s0.sl;
    for (size_t p = 0; p < v.size(); p++) {
        const Seg &s = (*GS)[v[p]];
        if (p > 0) {
            double w0 = s.c0 - p0, w1 = s.sl - p1;
            if (g.SD < g.mP) { g.mP = g.SD; g.mW0 = w0; g.mW1 = w1; }
            else if (g.SD == g.mP) { g.mW0 += w0; g.mW1 += w1; }
        }
        g.SD += s.delta;
        p0 = s.c0;
        p1 = s.sl;
    }
    g.l0 = p0;
    g.l1 = p1;
}

static void splitBlock(int bi) {
    vector<int> &v = blk[bi];
    int half = (int)v.size() / 2;
    vector<int> nv(v.begin() + half, v.end());
    v.resize(half);
    blk.insert(blk.begin() + bi + 1, nv);
    agg.insert(agg.begin() + bi + 1, BAgg());
    rebuild(bi);
    rebuild(bi + 1);
}

/* first block whose last element is greater than id, or blk.size() */
static int findBlockFor(int id) {
    int lo = 0, hi = (int)blk.size() - 1, res = (int)blk.size();
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        if (segLessId(id, blk[mid].back())) { res = mid; hi = mid - 1; }
        else lo = mid + 1;
    }
    return res;
}

static void insertSeg(int id) {
    if (blk.empty()) {
        blk.push_back(vector<int>{id});
        agg.push_back(BAgg());
        rebuild(0);
        return;
    }
    int bi = findBlockFor(id);
    if (bi == (int)blk.size()) bi = (int)blk.size() - 1;
    vector<int> &v = blk[bi];
    int pos = (int)v.size();
    {
        int lo = 0, hi = (int)v.size();
        while (lo < hi) {
            int mid = (lo + hi) >> 1;
            if (segLessId(id, v[mid])) hi = mid;
            else lo = mid + 1;
        }
        pos = lo;
    }
    v.insert(v.begin() + pos, id);
    rebuild(bi);
    if ((int)v.size() > 2 * BLOCKSZ) splitBlock(bi);
}

static bool eraseSeg(int id) {
    if (blk.empty()) return false;
    int bi = findBlockFor(id);
    if (bi >= (int)blk.size()) bi = (int)blk.size() - 1;
    for (int d = 0; d <= 1; d++) {
        for (int sgn = -1; sgn <= 1; sgn += 2) {
            int b = bi + sgn * d;
            if (b < 0 || b >= (int)blk.size()) continue;
            vector<int> &v = blk[b];
            for (size_t p = 0; p < v.size(); p++) {
                if (v[p] == id) {
                    v.erase(v.begin() + p);
                    if (v.empty()) {
                        blk.erase(blk.begin() + b);
                        agg.erase(agg.begin() + b);
                    } else {
                        rebuild(b);
                    }
                    return true;
                }
            }
            if (d == 0) break;  // avoid double-checking b==bi
        }
    }
    // fallback: global scan (only reachable if the order invariant was disturbed)
    for (int b = 0; b < (int)blk.size(); b++) {
        vector<int> &v = blk[b];
        for (size_t p = 0; p < v.size(); p++) {
            if (v[p] == id) {
                v.erase(v.begin() + p);
                if (v.empty()) { blk.erase(blk.begin() + b); agg.erase(agg.begin() + b); }
                else rebuild(b);
                return true;
            }
        }
    }
    return false;
}

static double coveredLen(double x) {
    int D = 0, mP = INT_MAX;
    double mW0 = 0, mW1 = 0;
    double f0 = 0, f1 = 0, pl0 = 0, pl1 = 0;
    bool first = true;
    for (size_t i = 0; i < blk.size(); i++) {
        const BAgg &g = agg[i];
        if (first) { f0 = g.f0; f1 = g.f1; first = false; }
        else {
            double w0 = g.f0 - pl0, w1 = g.f1 - pl1;
            if (D < mP) { mP = D; mW0 = w0; mW1 = w1; }
            else if (D == mP) { mW0 += w0; mW1 += w1; }
        }
        if (g.mP != INT_MAX) {
            int cand = D + g.mP;
            if (cand < mP) { mP = cand; mW0 = g.mW0; mW1 = g.mW1; }
            else if (cand == mP) { mW0 += g.mW0; mW1 += g.mW1; }
        }
        D += g.SD;
        pl0 = g.l0;
        pl1 = g.l1;
    }
    if (first) return 0.0;
    double c0 = pl0 - f0, c1 = pl1 - f1;
    if (mP == 0) { c0 -= mW0; c1 -= mW1; }
    return c0 + c1 * x;
}

static double unionArea(vector<Seg> &segs) {
    int E = (int)segs.size();
    if (E == 0) return 0.0;

    vector<double> xs;
    xs.reserve(2 * E);
    for (auto &s : segs) {
        xs.push_back(s.xmin);
        xs.push_back(s.xmax);
    }

    vector<Cross> crs;
    {
        vector<int> ord(E);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) { return segs[a].xmin < segs[b].xmin; });
        vector<int> active;
        active.reserve(E);
        for (int idx : ord) {
            const Seg &curs = segs[idx];
            int w = 0;
            for (int j = 0; j < (int)active.size(); j++) {
                if (segs[active[j]].xmax < curs.xmin) continue;
                active[w++] = active[j];
            }
            active.resize(w);
            for (int j : active) {
                const Seg &o = segs[j];
                if (!curs.isWin && !o.isWin) continue;
                if (o.ymax < curs.ymin || curs.ymax < o.ymin) continue;
                if (o.xmax < curs.xmin || curs.xmax < o.xmin) continue;
                double x;
                if (segInterX(o, curs, x)) {
                    crs.push_back({x, j, idx});
                    xs.push_back(x);
                }
            }
            active.push_back(idx);
        }
    }

    sort(xs.begin(), xs.end());
    xs.erase(unique(xs.begin(), xs.end()), xs.end());
    int T = (int)xs.size();
    if (T < 2) return 0.0;

    sort(crs.begin(), crs.end(), [](const Cross &a, const Cross &b) { return a.x < b.x; });

    vector<vector<int>> insAt(T), delAt(T);
    for (int i = 0; i < E; i++) {
        int i0 = (int)(lower_bound(xs.begin(), xs.end(), segs[i].xmin) - xs.begin());
        int i1 = (int)(lower_bound(xs.begin(), xs.end(), segs[i].xmax) - xs.begin());
        insAt[i0].push_back(i);
        delAt[i1].push_back(i);
    }

    GS = &segs;
    blk.clear();
    agg.clear();
    blk.reserve(64);
    agg.reserve(64);
    vector<char> isActive(E, 0);
    vector<int> reinsert;

    double area = 0.0;
    size_t cptr = 0;
    for (int i = 0; i + 1 < T; i++) {
        double x = xs[i];
        curX = (i > 0) ? 0.5 * (xs[i - 1] + x) : x;

        for (int id : delAt[i])
            if (isActive[id]) { eraseSeg(id); isActive[id] = 0; }

        reinsert.clear();
        while (cptr < crs.size() && crs[cptr].x <= x) {
            if (crs[cptr].x < x) { cptr++; continue; }  // should not happen
            int a = crs[cptr].i, b = crs[cptr].j;
            if (isActive[a]) { eraseSeg(a); isActive[a] = 0; reinsert.push_back(a); }
            if (isActive[b]) { eraseSeg(b); isActive[b] = 0; reinsert.push_back(b); }
            cptr++;
        }

        curX = x;
        for (int id : reinsert)
            if (segs[id].xmax > x) { insertSeg(id); isActive[id] = 1; }
        for (int id : insAt[i]) { insertSeg(id); isActive[id] = 1; }

        double xr = xs[i + 1];
        double L1 = coveredLen(x);
        double L2 = coveredLen(xr);
        area += 0.5 * (L1 + L2) * (xr - x);
    }
    return area;
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    poly.resize(n);
    for (int i = 0; i < n; i++) {
        int x, y;
        if (scanf("%d %d", &x, &y) != 2) return 0;
        poly[i] = Pt(x, y);
    }
    if (scanf("%d", &m) != 1) return 0;
    obs.resize(m);
    for (int i = 0; i < m; i++) {
        int x, y;
        if (scanf("%d %d", &x, &y) != 2) return 0;
        obs[i] = Pt(x, y);
    }
    sort(obs.begin(), obs.end(), [](const Pt &a, const Pt &b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });
    obs.erase(unique(obs.begin(), obs.end(), [](const Pt &a, const Pt &b) {
                  return a.x == b.x && a.y == b.y;
              }),
              obs.end());
    m = (int)obs.size();

    vector<Seg> segs;
    for (auto &p : obs) buildVis(p, segs);

    double ans = unionArea(segs);
    printf("%.9f\n", ans);
    return 0;
}

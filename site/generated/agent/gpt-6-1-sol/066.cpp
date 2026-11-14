#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
const i64 INF = (1LL << 60);

struct Edge { int u, v, w; };
struct Point {
    i64 s, q, len;
    long double mean() const { return (long double)s / len; }
    long double variance() const {
        return (long double)(q * len - s * s) / (len * len);
    }
};
struct Direction { i64 num, den; };
struct Support { Point p; Direction x; };

class Solver {
    int n, source, target;
    vector<Edge> edges;
    vector<i64> cost, dc, ds, pc, ps;
public:
    long double best = 1e100L;
#ifdef LOCAL
    int queries = 0;
#endif
    Solver(int n_, int source_, int target_, vector<Edge> edges_)
        : n(n_), source(source_), target(target_), edges(move(edges_)),
          cost(edges.size()), dc((n + 1) * n), ds((n + 1) * n),
          pc((n + 1) * n), ps((n + 1) * n) {}

    // Compare pairs (cost / length, sum_of_weights / length) lexicographically.
    static bool lessRatio(i64 ac, i64 as, i64 al,
                          i64 bc, i64 bs, i64 bl) {
        if (ac * bl != bc * al) return ac * bl < bc * al;
        return as * bl < bs * al;
    }

    Point oracle(Direction x) {
#ifdef LOCAL
        ++queries;
#endif
        for (int i = 0; i < (int)edges.size(); ++i) {
            i64 w = edges[i].w;
            cost[i] = x.den * w * w - 2 * x.num * w;
        }
        fill(dc.begin(), dc.end(), INF);
        fill(pc.begin(), pc.end(), INF);
        fill(ds.begin(), ds.end(), 0);
        fill(ps.begin(), ps.end(), 0);
        // A zero-cost super-source reaches every vertex for Karp's algorithm.
        for (int v = 0; v < n; ++v) dc[v] = 0;
        pc[source] = 0;
        i64 answerC = INF, answerS = 0, answerL = 1;
        for (int k = 1; k <= n; ++k) {
            int prev = (k - 1) * n, cur = k * n;
            for (int i = 0; i < (int)edges.size(); ++i) {
                const auto &e = edges[i];
                if (dc[prev + e.u] != INF) {
                    i64 c = dc[prev + e.u] + cost[i];
                    i64 s = ds[prev + e.u] + e.w;
                    int v = cur + e.v;
                    if (c < dc[v] || (c == dc[v] && s < ds[v])) {
                        dc[v] = c; ds[v] = s;
                    }
                }
                if (k < n && pc[prev + e.u] != INF) {
                    i64 c = pc[prev + e.u] + cost[i];
                    i64 s = ps[prev + e.u] + e.w;
                    int v = cur + e.v;
                    if (c < pc[v] || (c == pc[v] && s < ps[v])) {
                        pc[v] = c; ps[v] = s;
                    }
                }
            }
            int t = cur + target;
            if (k < n && pc[t] != INF &&
                (answerC == INF || lessRatio(pc[t], ps[t], k,
                                             answerC, answerS, answerL))) {
                answerC = pc[t]; answerS = ps[t]; answerL = k;
            }
        }
        // Karp: minimum_v maximum_k (d[n][v] - d[k][v]) / (n-k).
        for (int v = 0; v < n; ++v) {
            if (dc[n * n + v] == INF) continue;
            i64 maxC = -INF, maxS = 0, maxL = 1;
            for (int k = 0; k < n; ++k) {
                if (dc[k * n + v] == INF) continue;
                i64 c = dc[n * n + v] - dc[k * n + v];
                i64 s = ds[n * n + v] - ds[k * n + v];
                i64 l = n - k;
                if (maxC == -INF || lessRatio(maxC, maxS, maxL, c, s, l)) {
                    maxC = c; maxS = s; maxL = l;
                }
            }
            if (answerC == INF || lessRatio(maxC, maxS, maxL,
                                            answerC, answerS, answerL)) {
                answerC = maxC; answerS = maxS; answerL = maxL;
            }
        }
        Point p{answerS, (answerC + 2 * x.num * answerS) / x.den, answerL};
        best = min(best, p.variance());
        return p;
    }

    static bool samePoint(const Point &a, const Point &b) {
        return a.s * b.len == b.s * a.len &&
               a.q * b.len == b.q * a.len;
    }

    bool canImprove(const Support &a, const Support &b) const {
        if (a.x.num * b.x.den >= b.x.num * a.x.den) return true;
        long double xa = (long double)a.x.num / a.x.den;
        long double xb = (long double)b.x.num / b.x.den;
        long double ma = a.p.mean(), mb = b.p.mean();
        i64 ac = a.x.den * a.p.q - 2 * a.x.num * a.p.s;
        i64 bc = b.x.den * b.p.q - 2 * b.x.num * b.p.s;
        long double ca = (long double)ac / (a.x.den * a.p.len);
        long double cb = (long double)bc / (b.x.den * b.p.len);
        // Intersect the supporting lines exactly before converting to floating
        // point; their slopes can be very close.
        __int128 numerator = (__int128)ac * b.x.den * b.p.len
                           - (__int128)bc * a.x.den * a.p.len;
        __int128 denominator = 2 * (__int128)(b.x.num * a.x.den - a.x.num * b.x.den)
                                * a.p.len * b.p.len;
        long double t = (long double)numerator / (long double)denominator;
        t = max(ma, min(mb, t));
        long double lower = max(2 * xa * t + ca, 2 * xb * t + cb) - t * t;
        lower = min({lower, a.p.variance(), b.p.variance()});
        return lower < best - 1e-12L;
    }

    void explore(const Support &a, const Support &b) {
        if (best <= 1e-12L || samePoint(a.p, b.p) || !canImprove(a, b)) return;
        // The two affine functions q/len - 2*x*s/len intersect here.
        i64 num = b.p.q * a.p.len - a.p.q * b.p.len;
        i64 den = 2 * (b.p.s * a.p.len - a.p.s * b.p.len);
        if (den == 0) return;
        if (den < 0) { den = -den; num = -num; }
        i64 g = gcd(abs(num), den);
        Direction x{num / g, den / g};
        Point p = oracle(x);
        i64 newCost = x.den * p.q - 2 * x.num * p.s;
        i64 oldCost = x.den * a.p.q - 2 * x.num * a.p.s;
        if (newCost * a.p.len == oldCost * p.len) return;
        Support mid{p, x};
        explore(a, mid);
        explore(mid, b);
    }

    long double solve() {
        Support a{oracle({0, 1}), {0, 1}};
        if (best > 1e-12L) {
            Support b{oracle({20, 1}), {20, 1}};
            explore(a, b);
        }
        return max(0.0L, best);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    cin >> n >> m;
    vector<Edge> edges(m);
    vector<vector<int>> adj(n), rev(n);
    for (auto &e : edges) {
        cin >> e.u >> e.v >> e.w;
        --e.u; --e.v;
        adj[e.u].push_back(e.v);
        rev[e.v].push_back(e.u);
    }
    auto reachable = [&](int start, const vector<vector<int>> &g) {
        vector<int> seen(n);
        vector<int> stack{start}; seen[start] = 1;
        while (!stack.empty()) {
            int v = stack.back(); stack.pop_back();
            for (int u : g[v]) if (!seen[u]) {
                seen[u] = 1; stack.push_back(u);
            }
        }
        return seen;
    };
    auto from = reachable(0, adj), to = reachable(n - 1, rev);
    if (!from[n - 1]) { cout << "-1\n"; return 0; }
    vector<int> id(n, -1);
    int count = 0;
    for (int v = 0; v < n; ++v) if (from[v] && to[v]) id[v] = count++;
    vector<Edge> useful;
    for (auto e : edges) if (id[e.u] != -1 && id[e.v] != -1) {
        e.u = id[e.u]; e.v = id[e.v]; useful.push_back(e);
    }
    Solver solver(count, id[0], id[n - 1], move(useful));
    cout << fixed << setprecision(15) << solver.solve() << '\n';
#ifdef LOCAL
    cerr << "queries " << solver.queries << '\n';
#endif
    return 0;
}

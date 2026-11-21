// Guess the Polygon
// ---------------------------------------------------------------------------
// The polygon's vertices are given shuffled; we may ask, for any rational
// p/q (0<=p/q<=1000, 1<=q<=1000), the total length of the intersection of the
// polygon with the vertical line x=p/q, and must output the area.
//
// Let X_0<...<X_{k-1} be the distinct vertex abscissae and w_i=X_{i+1}-X_i.
// On every open slab (X_i,X_{i+1}) the chord length L is affine.  Write
//   d_i = lim_{x->X_i+} L(x),  u_i = lim_{x->X_i-} L(x).
// Then
//   Area = sum_i w_i (d_i + u_{i+1})/2 .
// L can jump at X_i only because of a vertical edge lying on x=X_i, and such
// an edge needs two vertices with abscissa X_i.  Hence if X_i occurs only once
// among the vertices, L is continuous there and d_i = u_i =: v_i, gluing
// slab i-1 and slab i into one block.  (Blocks are cut at every abscissa that
// occurs at least twice - a conservative but sound criterion.)
//
// Inside a block with rows a..b, local W_p:
//   Area_block = sum_{p=a}^{b} W_p v_p,
//   W_a = w_a/2, W_p = (w_{p-1}+w_p)/2, W_b = w_{b-1}/2.
// A query inside slab i returns (1-t) v_i + t v_{i+1}, i.e. it touches only
// rows {i,i+1}; a block of s rows therefore needs at least ceil(s/2) queries,
// and this is achieved by pairing rows:
//   * pair (p,p+1): one query with forced t = W_{p+1}/(W_p+W_{p+1}) and
//     coefficient W_p+W_{p+1}.  If that abscissa cannot be written with
//     denominator <=1000, two queries inside the same slab (t=1/3, 2/3) are
//     used instead (free positions).
//   * odd s: a 3-row block (2 queries, one degree of freedom; a search over
//     representable positions is made, else 3 free-position queries), or a
//     direct query at an interior (continuous) breakpoint X_j, which yields
//     exactly v_j and therefore a free coefficient W_j.
//
// Extra shortcut: if every vertex lies on the convex hull boundary, the only
// simple polygon on that point set is the hull itself, so 0 queries suffice.
// If all abscissae coincide (or the hull is degenerate) the area is 0.
//
// The area of a lattice polygon is a half integer, so the accumulated long
// double value is rounded to the nearest half integer.  Query replies may have
// hundreds of digits (lcm of edge dx), so numbers are parsed as decimal
// strings into mantissa*10^exp.
#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using i128 = __int128_t;
using ld = long double;

// ------------------------------- fast input -------------------------------
// Line oriented on purpose: the interactor flushes complete lines, so we never
// block waiting for more bytes than a reply actually contains.
static char *lbuf = nullptr;
static size_t lcap = 0, llen = 0, lpos = 0;
static inline int gc() {
    while (lpos >= llen) {
        ssize_t r = getline(&lbuf, &lcap, stdin);
        lpos = 0;
        if (r <= 0) { llen = 0; lpos = 0; return -1; }
        llen = (size_t)r;
    }
    return (unsigned char)lbuf[lpos++];
}
static inline void pc(int c) { if (c >= 0 && lpos > 0) lpos--; }  // push one char back
static ll readInt() {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9') && c != '-') c = gc();
    int sg = 1;
    if (c == '-') { sg = -1; c = gc(); }
    ll v = 0;
    while (c >= '0' && c <= '9') { v = v * 10 + (c - '0'); c = gc(); }
    pc(c);
    return v * sg;
}
// next non-negative decimal integer as a string (skips any non-digits);
// returns empty string on EOF.
static string readNumStr() {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9')) c = gc();
    if (c == -1) return string();
    string s;
    while (c >= '0' && c <= '9') { s.push_back((char)c); c = gc(); }
    pc(c);
    return s;
}

static ll gcdll(ll a, ll b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b) { ll t = a % b; a = b; b = t; }
    return a;
}
static i128 gcd128(i128 a, i128 b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    while (b) { i128 t = a % b; a = b; b = t; }
    return a;
}

// huge decimal string -> value = m * 10^e   (m holds at most 18 digits)
static void parseNum(const string &s, ld &m, ll &e) {
    int n = (int)s.size(), i = 0;
    while (i < n && s[i] == '0') i++;
    if (i >= n) { m = 0; e = 0; return; }
    int digits = n - i;
    int take = min(digits, 18);
    ld v = 0;
    for (int j = 0; j < take; j++) v = v * 10 + (s[i + j] - '0');
    m = v;
    e = digits - take;
}

struct Plan {
    vector<ll> p, q;
    vector<ld> c;
    void add(ll pp, ll qq, ld cc) { p.push_back(pp); q.push_back(qq); c.push_back(cc); }
};

int main() {
    ll n = readInt();
    if (n <= 0) return 0;
    vector<ll> xs(n), ys(n);
    for (ll i = 0; i < n; i++) { xs[i] = readInt(); ys[i] = readInt(); }

    vector<ll> X = xs;
    sort(X.begin(), X.end());
    X.erase(unique(X.begin(), X.end()), X.end());
    int k = (int)X.size();

    auto printAns = [&](ll num, ll den) {
        printf("! %lld %lld\n", num, den);
        fflush(stdout);
    };

    if (k <= 1) { printAns(0, 1); return 0; }

    // ---------------- all vertices on the hull boundary => polygon forced ----
    {
        vector<int> ord(n);
        iota(ord.begin(), ord.end(), 0);
        sort(ord.begin(), ord.end(), [&](int a, int b) {
            if (xs[a] != xs[b]) return xs[a] < xs[b];
            return ys[a] < ys[b];
        });
        auto cr = [&](int a, int b, int c) -> ll {
            return (xs[b] - xs[a]) * (ys[c] - ys[a]) - (ys[b] - ys[a]) * (xs[c] - xs[a]);
        };
        vector<int> h;
        for (int i = 0; i < (int)n; i++) {
            while (h.size() >= 2 && cr(h[h.size() - 2], h.back(), ord[i]) <= 0) h.pop_back();
            h.push_back(ord[i]);
        }
        size_t lower = h.size();
        for (int i = (int)n - 2; i >= 0; i--) {
            while (h.size() > lower && cr(h[h.size() - 2], h.back(), ord[i]) <= 0) h.pop_back();
            h.push_back(ord[i]);
        }
        if (!h.empty()) h.pop_back();
        if (h.size() < 3) { printAns(0, 1); return 0; }  // degenerate: collinear
        bool allOn = true;
        int hs = (int)h.size();
        for (int idx = 0; idx < (int)n && allOn; idx++) {
            bool on = false;
            for (int i = 0; i < hs && !on; i++) {
                int a = h[i], b = h[(i + 1) % hs];
                if (cr(a, b, idx) != 0) continue;
                ll lox = min(xs[a], xs[b]), hix = max(xs[a], xs[b]);
                ll loy = min(ys[a], ys[b]), hiy = max(ys[a], ys[b]);
                if (xs[idx] >= lox && xs[idx] <= hix && ys[idx] >= loy && ys[idx] <= hiy)
                    on = true;
            }
            if (!on) allOn = false;
        }
        if (allOn) {
            i128 s2 = 0;
            for (int i = 0; i < hs; i++) {
                int a = h[i], b = h[(i + 1) % hs];
                s2 += (i128)xs[a] * ys[b] - (i128)xs[b] * ys[a];
            }
            if (s2 < 0) s2 = -s2;
            ll num = (ll)s2, den = 2;
            ll g = gcdll(num, den);
            printAns(num / g, den / g);
            return 0;
        }
    }

    // ---------------- blocks ----------------
    vector<ll> w(k - 1);
    for (int i = 0; i + 1 < k; i++) w[i] = X[i + 1] - X[i];

    vector<int> cnt(k, 0);
    for (int i = 0; i < (int)n; i++) {
        int idx = (int)(lower_bound(X.begin(), X.end(), xs[i]) - X.begin());
        cnt[idx]++;
    }

    struct Block { int a, b; };  // rows a..b, slabs a..b-1
    vector<Block> blocks;
    {
        int start = 0;
        for (int j = 1; j <= k - 2; j++) {
            if (cnt[j] >= 2) {  // abscissa repeated => vertical edge possible
                blocks.push_back({start, j});
                start = j;
            }
        }
        blocks.push_back({start, k - 1});
    }

    Plan plan;
#ifdef STATS
    long long st_find3_ok = 0, st_find3_fail = 0, st_pair_bad = 0, st_bp = 0;
#endif

    // A row whose abscissa is the unique minimum / maximum vertex abscissa has
    // L == 0 there (the slice is a single point and the chord shrinks to it),
    // so that row's variable is known and can be dropped from the unknowns.
    auto knownZero = [&](int j) -> bool {
        return (j == 0 && cnt[0] == 1) || (j == k - 1 && cnt[k - 1] == 1);
    };

    for (auto &blk : blocks) {
        int a = blk.a, b = blk.b;
        if (a >= b) continue;
        Plan pb;  // candidate plan for this block
        int s = b - a + 1;
        vector<ll> L2(s, 0);  // L2[t] = 2 * (block local W_{a+t})
        for (int t = 0; t < s; t++) {
            int p = a + t;
            ll v = 0;
            if (p > a) v += w[p - 1];
            if (p < b) v += w[p];
            L2[t] = v;
        }
        auto lw2 = [&](int p) -> ll { return L2[p - a]; };

        // effective (unknown) row range
        int ra = a, rb = b;
        if (a == 0 && cnt[0] == 1) ra = 1;
        if (b == k - 1 && cnt[k - 1] == 1) rb = k - 2;
        if (ra > rb) continue;  // nothing left to determine

        if (ra == rb) {
            // single unknown row: one midpoint query of an adjacent slab whose
            // other endpoint is the known zero row.
            int j = ra;
            int i = -1;
            if (j - 1 >= a && j - 1 <= b - 1 && knownZero(j - 1)) i = j - 1;
            else if (j + 1 <= b && j <= b - 1 && knownZero(j + 1)) i = j;
            if (i >= 0) {
                ll D = X[i] + X[i + 1], N = 2;
                ll g = gcdll(D, N);
                pb.add(D / g, N / g, (ld)lw2(j));
            } else {
                // defensive: one midpoint query per slab of the block
                for (int t = a; t < b; t++) {
                    ll D = X[t] + X[t + 1], N = 2;
                    ll g = gcdll(D, N);
                    pb.add(D / g, N / g, (ld)w[t]);
                }
            }
            for (size_t t = 0; t < pb.p.size(); t++) plan.add(pb.p[t], pb.q[t], pb.c[t]);
            continue;
        }

        int ns = s - 1;  // slabs a..b-1
        vector<char> ok(ns, 0);
        vector<ll> fp(ns, 0), fq(ns, 1);
        for (int t = 0; t < ns; t++) {
            int p = a + t;
            i128 D = (i128)lw2(p) + lw2(p + 1);
            i128 N = (i128)X[p] * D + (i128)w[p] * lw2(p + 1);
            i128 g = gcd128(N, D);
            i128 q = D / g, pp = N / g;
            if (q >= 1 && q <= 1000) { ok[t] = 1; fp[t] = (ll)pp; fq[t] = (ll)q; }
        }

        auto addPair = [&](int p) {
            int t = p - a;
            if (ok[t]) {
                pb.add(fp[t], fq[t], ((ld)lw2(p) + (ld)lw2(p + 1)) / 2.0L);
            } else {
#ifdef STATS
                st_pair_bad++;
#endif
                ll d = 3;
                ll n1 = 3 * X[p] + w[p], n2 = 3 * X[p] + 2 * w[p];
                ll g1 = gcdll(n1, d), g2 = gcdll(n2, d);
                pb.add(n1 / g1, d / g1, (2.0L * lw2(p) - (ld)lw2(p + 1)) / 2.0L);
                pb.add(n2 / g2, d / g2, (2.0L * lw2(p + 1) - (ld)lw2(p)) / 2.0L);
            }
        };

        // 2-query solution for rows p,p+1,p+2 (slabs p,p+1).
        auto find3 = [&](int p, ll &PP, ll &QQ, ld &ca, ll &xp, ll &xq, ld &cb) -> bool {
            i128 A2 = lw2(p), B2 = lw2(p + 1), C2 = lw2(p + 2);
            ll wa = w[p];
            for (int jj = 1; jj <= 999; jj++) {
                if ((ll)jj >= 1000LL * wa) break;
                ll P0 = 1000 * X[p] + jj, Q0 = 1000;
                ll g = gcdll(P0, Q0);
                ll pp = P0 / g, qq = Q0 / g;
                ll u = pp - X[p] * qq;  // t_p = u/v
                ll v = qq * wa;
                if (u <= 0 || u >= v) continue;
                ll gu = gcdll(u, v);
                u /= gu; v /= gu;
                i128 Rn = B2 * (i128)(v - u) - A2 * (i128)u;
                if (Rn <= 0) continue;
                i128 Rd = 2 * (i128)(v - u);
                i128 tn = C2 * Rd;                       // t_{p+1} = tn/td
                i128 td = 2 * Rn + C2 * Rd;
                i128 xn = (i128)X[p + 1] * td + (i128)w[p + 1] * tn;
                i128 gx = gcd128(xn, td);
                i128 rx = xn / gx, rq = td / gx;
                if (rq < 1 || rq > 1000) continue;
                PP = pp; QQ = qq;
                ca = ((ld)A2 / 2.0L) * ((ld)v / (ld)(v - u));
                cb = (ld)Rn / (ld)Rd + (ld)C2 / 2.0L;
                xp = (ll)rx; xq = (ll)rq;
                return true;
            }
            return false;
        };

        auto add3fallback = [&](int p) {
            ll d2 = 2, n2 = 2 * X[p + 1] + w[p + 1];
            ll g2 = gcdll(n2, d2);
            pb.add(n2 / g2, d2 / g2, (ld)lw2(p + 2));
            ll d = 3;
            ll n1 = 3 * X[p] + w[p], n3 = 3 * X[p] + 2 * w[p];
            ll g1 = gcdll(n1, d), g3 = gcdll(n3, d);
            pb.add(n1 / g1, d / g1,
                   (ld)(2 * lw2(p) - lw2(p + 1) + lw2(p + 2)) / 2.0L);
            pb.add(n3 / g3, d / g3,
                   (ld)(2 * lw2(p + 1) - 2 * lw2(p + 2) - lw2(p)) / 2.0L);
        };

        if (((rb - ra + 1) % 2) == 0) {
            for (int p = ra; p < rb; p += 2) addPair(p);
        } else {
            auto pc = [&](int p) -> int { return ok[p - a] ? 1 : 2; };
            const int INF = 1000000000;
            int bestCost = INF, bestType = -1, bestP = -1;
            for (int p = ra; p <= rb - 2; p += 2) {
                ll PP, QQ, xp, xq; ld ca, cb;
                int c3 = find3(p, PP, QQ, ca, xp, xq, cb) ? 2 : 3;
#ifdef STATS
                if (c3 == 2) st_find3_ok++; else st_find3_fail++;
#endif
                int cost = c3;
                for (int q2 = ra; q2 <= p - 2; q2 += 2) cost += pc(q2);
                for (int q2 = p + 3; q2 <= rb - 1; q2 += 2) cost += pc(q2);
                if (cost < bestCost) { bestCost = cost; bestType = 0; bestP = p; }
            }
            for (int j = ra + 2; j <= rb - 2; j += 2) {
                int cost = 1;
                for (int q2 = ra; q2 <= j - 2; q2 += 2) cost += pc(q2);
                for (int q2 = j + 1; q2 <= rb - 1; q2 += 2) cost += pc(q2);
                if (cost < bestCost) { bestCost = cost; bestType = 1; bestP = j; }
            }
            if (bestType == 0) {
                int p = bestP;
                for (int q2 = ra; q2 <= p - 2; q2 += 2) addPair(q2);
                ll PP, QQ, xp, xq; ld ca, cb;
                if (find3(p, PP, QQ, ca, xp, xq, cb)) {
                    pb.add(PP, QQ, ca);
                    pb.add(xp, xq, cb);
                } else {
                    add3fallback(p);
                }
                for (int q2 = p + 3; q2 <= rb - 1; q2 += 2) addPair(q2);
            } else {
                int j = bestP;
#ifdef STATS
                st_bp++;
#endif
                for (int q2 = ra; q2 <= j - 2; q2 += 2) addPair(q2);
                pb.add(X[j], 1, (ld)lw2(j) / 2.0L);
                for (int q2 = j + 1; q2 <= rb - 1; q2 += 2) addPair(q2);
            }
        }

        // safety net: never spend more than one midpoint query per slab
        if ((int)pb.p.size() > s - 1) {
            pb.p.clear(); pb.q.clear(); pb.c.clear();
            for (int i = a; i < b; i++) {
                ll D = X[i] + X[i + 1], N = 2;
                ll g = gcdll(D, N);
                pb.add(D / g, N / g, (ld)w[i]);
            }
        }
        for (size_t t = 0; t < pb.p.size(); t++) plan.add(pb.p[t], pb.q[t], pb.c[t]);
    }

#ifdef STATS
    fprintf(stderr, "STATS queries=%zu blocks=%zu find3ok=%lld find3fail=%lld pairbad=%lld bp=%lld\n",
            plan.p.size(), blocks.size(), st_find3_ok, st_find3_fail, st_pair_bad, st_bp);
#endif

    // ---------------- execute ----------------
    ld area = 0;
    for (size_t i = 0; i < plan.p.size(); i++) {
        printf("? %lld %lld\n", plan.p[i], plan.q[i]);
        fflush(stdout);
        string a = readNumStr();
        if (a.empty()) return 0;
        string b = readNumStr();
        if (b.empty()) return 0;
        ld L;
        if (a == "0") {
            L = 0;
        } else {
            ld ma, mb; ll ea, eb;
            parseNum(a, ma, ea);
            parseNum(b, mb, eb);
            L = (ma / mb) * powl(10.0L, (ld)(ea - eb));
        }
        area += plan.c[i] * L;
    }

    ll A2 = llroundl(area * 2.0L);
    if (A2 < 0) A2 = 0;
    if (A2 % 2 == 0) printAns(A2 / 2, 1);
    else printAns(A2, 2);
    return 0;
}

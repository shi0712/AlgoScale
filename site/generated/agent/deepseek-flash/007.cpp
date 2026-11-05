// 环轨换乘
// Circles are pairwise non-crossing (disjoint or nested, possibly tangent).
// We walk along circle arcs and transfer at tangency points.
// Build a graph: nodes = distinct tangency points (+S,T), edges = arcs between
// consecutive points on each circle; then Dijkstra.
//
// Tangency detection:
//  * internal tangency (nested) : only between parent/child in the nesting forest
//  * external tangency          : circles that are adjacent in the vertical order
//                                 of a sweep over x at some event x
//  * vertical tangent line      : two circles whose extreme-x point is shared
//                                 (one rightmost, one leftmost at same (x,y))
#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;
typedef __int128 lll;
typedef long double ld;

// ------------------ fast input ------------------
static char ibuf[1 << 20];
static int ipos_ = 0, ilen_ = 0;
static inline int gchar_() {
    if (ipos_ == ilen_) { ilen_ = (int)fread(ibuf, 1, sizeof(ibuf), stdin); ipos_ = 0; if (ilen_ <= 0) return -1; }
    return ibuf[ipos_++];
}
static inline ll readll() {
    int c = gchar_();
    while (c != -1 && c != '-' && (c < '0' || c > '9')) c = gchar_();
    if (c == -1) return 0;
    int sg = 1;
    if (c == '-') { sg = -1; c = gchar_(); }
    ll v = 0;
    while (c >= '0' && c <= '9') { v = v * 10 + (c - '0'); c = gchar_(); }
    return v * sg;
}

static int N;
static ll Sx, Sy, Tx, Ty;
static vector<ll> CX, CY, RR;
static vector<int> VV;
static ll curX;

// ================= exact sign helpers =================
static inline int sgnll(ll v) { return (v > 0) - (v < 0); }

// sign( K - 2*sqrt(A*B) )   (A,B >= 0)
static int sgnK(lll K, ll A, ll B) {
    if (K < 0) return -1;
    lll ab4 = 4 * (lll)A * B;
    if (ab4 == 0) return (K > 0) ? 1 : 0;
    lll K2 = K * K;
    if (K2 > ab4) return 1;
    if (K2 < ab4) return -1;
    return 0;
}
// sign( d + sqrt(A) - sqrt(B) )
static int coreA(ll d, ll A, ll B) {
    if (A == B) return sgnll(d);
    lll K = (lll)A + B - (lll)d * d;
    if (A > B) {
        if (d >= 0) return 1;
        return sgnK(K, A, B);
    } else {
        if (d <= 0) return -1;
        return -sgnK(K, A, B);
    }
}
// sign( d + sqrt(A) + sqrt(B) )
static int coreB(ll d, ll A, ll B) {
    if (d >= 0) return (d > 0 || A > 0 || B > 0) ? 1 : 0;
    lll M = (lll)d * d - A - B;
    if (M < 0) return 1;
    lll ab4 = 4 * (lll)A * B;
    if (ab4 == 0) return (M > 0) ? -1 : 0;
    lll M2 = M * M;
    if (ab4 > M2) return 1;
    if (ab4 < M2) return -1;
    return 0;
}
// sign( (ya + sa*sqrt(A)) - (yb + sb*sqrt(B)) ),  s in {-1,0,1}
static int sgnY(ll ya, int sa, ll A, ll yb, int sb, ll B) {
    if (sa == 0 && sb == 0) return sgnll(ya - yb);
    if (sa == 0) return -sgnY(yb, sb, B, ya, 0, 0);
    if (sb == 0) {
        ll d = ya - yb;
        if (sa > 0) {
            if (A == 0) return sgnll(d);
            if (d >= 0) return 1;
            lll dd = (lll)d * d;
            if ((lll)A > dd) return 1;
            if ((lll)A < dd) return -1;
            return 0;
        } else {
            if (A == 0) return sgnll(d);
            if (d <= 0) return -1;
            lll dd = (lll)d * d;
            if (dd > (lll)A) return 1;
            if (dd < (lll)A) return -1;
            return 0;
        }
    }
    ll d = ya - yb;
    if (sa > 0 && sb > 0) return coreA(d, A, B);
    if (sa < 0 && sb < 0) return coreA(d, B, A);
    if (sa > 0 && sb < 0) return coreB(d, A, B);
    return -coreB(-d, A, B);
}

// ------------------ 320-bit helper for second derivative tie-break ------------
struct Big5 { ull w[5]; };
static inline void bigMul(Big5 &x, ull m) {
    unsigned __int128 carry = 0;
    for (int i = 0; i < 5; i++) {
        unsigned __int128 t = (unsigned __int128)x.w[i] * m + carry;
        x.w[i] = (ull)t;
        carry = t >> 64;
    }
}
static int bigCmp(const Big5 &a, const Big5 &b) {
    for (int i = 4; i >= 0; i--) if (a.w[i] != b.w[i]) return a.w[i] > b.w[i] ? 1 : -1;
    return 0;
}
// sign( -sa*ra^2/A^{3/2} + sb*rb^2/B^{3/2} ),  A,B > 0
static int sgnSecond(int sa, ll ra, ll A, int sb, ll rb, ll B) {
    if (sa != sb) return sb > 0 ? 1 : -1;
    // cmp = sign( ra^2*B^{3/2} - rb^2*A^{3/2} )
    int cmpc;
    {
        ld v1 = (ld)ra * (ld)ra * (ld)ra * (ld)ra * (ld)B * (ld)B * (ld)B;
        ld v2 = (ld)rb * (ld)rb * (ld)rb * (ld)rb * (ld)A * (ld)A * (ld)A;
        ld diff = v1 - v2, mx = max(v1, v2);
        if (mx > 0 && fabsl(diff) > (ld)1e-12 * mx) {
            cmpc = diff > 0 ? 1 : -1;
        } else {
            Big5 x1{1, 0, 0, 0, 0};
            for (int i = 0; i < 4; i++) bigMul(x1, (ull)ra);
            for (int i = 0; i < 3; i++) bigMul(x1, (ull)B);
            Big5 x2{1, 0, 0, 0, 0};
            for (int i = 0; i < 4; i++) bigMul(x2, (ull)rb);
            for (int i = 0; i < 3; i++) bigMul(x2, (ull)A);
            cmpc = bigCmp(x1, x2);
        }
    }
    return sa > 0 ? -cmpc : cmpc;
}
// sign( p*sqrt(B) + q*sqrt(A) )
static int sgnTerms(ll p, ll A, ll q, ll B) {
    if (p == 0 && q == 0) return 0;
    if (p >= 0 && q >= 0) return 1;
    if (p <= 0 && q <= 0) return -1;
    lll lhs = (lll)p * p * B;
    lll rhs = (lll)q * q * A;
    if (lhs == rhs) return 0;
    if (p > 0) return (lhs > rhs) ? 1 : -1;
    return (lhs > rhs) ? -1 : 1;
}

// ================== sweep comparator ======================
// keys: (circle<<2) | (s+1)  with s=-1 lower arc, s=0 query point, s=+1 upper arc
struct Cmp {
    bool operator()(ull u, ull v) const {
        int cu = (int)(u >> 2), su = (int)(u & 3) - 1;
        int cv = (int)(v >> 2), sv = (int)(v & 3) - 1;
        if (cu == cv) return su < sv;
        ll A = 0, B = 0;
        if (su) { ll t = curX - CX[cu]; A = RR[cu] * RR[cu] - t * t; }
        if (sv) { ll t = curX - CX[cv]; B = RR[cv] * RR[cv] - t * t; }
        // fast path in long double
        {
            ld fa = (ld)CY[cu];
            if (su > 0) fa += sqrtl((ld)A); else if (su < 0) fa -= sqrtl((ld)A);
            ld fb = (ld)CY[cv];
            if (sv > 0) fb += sqrtl((ld)B); else if (sv < 0) fb -= sqrtl((ld)B);
            ld d = fa - fb;
            if (d > 1e-6L) return false;
            if (d < -1e-6L) return true;
        }
        int s = sgnY(CY[cu], su, A, CY[cv], sv, B);
        if (s != 0) return s < 0;
        // ---- tie at curX : use right-limit order ----
        if (su == 0 || sv == 0) {
            if (su == 0 && sv == 0) return false;
            if (su == 0) {  // query circle cu vs arc (cv,sv)  (both degenerate)
                bool smaller = RR[cu] < RR[cv];
                int sg = (sv == -1) ? (smaller ? 1 : -1) : (smaller ? -1 : 1);
                return sg < 0;
            } else {        // arc (cu,su) vs query circle cv
                bool smaller = RR[cv] < RR[cu];
                int sg = (su == -1) ? (smaller ? 1 : -1) : (smaller ? -1 : 1);
                return sg > 0;
            }
        }
        if (A == 0 && B == 0) {  // both leftmost at curX
            if (su != sv) return su < sv;
            if (su > 0) return RR[cu] < RR[cv];
            return RR[cu] > RR[cv];
        }
        if (A == 0 || B == 0) return su < sv;  // cannot happen
        // both regular: compare slopes
        ll p = -(ll)su * (curX - CX[cu]);
        ll q = (ll)sv * (curX - CX[cv]);
        int sd = sgnTerms(p, A, q, B);
        if (sd != 0) return sd < 0;
        int sdd = sgnSecond(su, RR[cu], A, sv, RR[cv], B);
        if (sdd != 0) return sdd < 0;
        return u < v;
    }
};

// ================== node table =====================
struct Key { ll nx, ny, den; };
struct KeyHash {
    size_t operator()(Key const &k) const {
        ull h = (ull)k.nx * 0x9e3779b97f4a7c15ULL;
        h ^= (ull)k.ny * 0xc2b2ae3d27d4eb4fULL + 0x165667b19e3779f9ULL;
        h ^= (ull)k.den * 0x94d049bb133111ebULL;
        h ^= h >> 29; h *= 0xbf58476d1ce4e5b9ULL; h ^= h >> 32;
        return (size_t)h;
    }
};
struct KeyEq {
    bool operator()(Key const &a, Key const &b) const {
        return a.nx == b.nx && a.ny == b.ny && a.den == b.den;
    }
};
static unordered_map<Key, int, KeyHash, KeyEq> nodeMap;
static int nodeCnt = 0;

static int getNode(ll nx, ll ny, ll den) {
    if (den < 0) { nx = -nx; ny = -ny; den = -den; }
    ll g = std::__gcd(std::__gcd(nx < 0 ? -nx : nx, ny < 0 ? -ny : ny), den);
    if (g > 1) { nx /= g; ny /= g; den /= g; }
    Key k{nx, ny, den};
    auto it = nodeMap.find(k);
    if (it != nodeMap.end()) return it->second;
    int id = nodeCnt++;
    nodeMap.emplace(k, id);
    return id;
}

struct Inc { int c, nd, dx, dy; };
static vector<Inc> incs;

// ================== main =====================
int main() {
    N = (int)readll();
    Sx = readll(); Sy = readll(); Tx = readll(); Ty = readll();
    CX.resize(N); CY.resize(N); RR.resize(N); VV.resize(N);
    for (int i = 0; i < N; i++) {
        CX[i] = readll(); CY[i] = readll(); RR[i] = readll(); VV[i] = (int)readll();
    }
    if (Sx == Tx && Sy == Ty) { printf("0.000000000000000000\n"); return 0; }

    incs.reserve((size_t)4 * N + 16);
    nodeMap.reserve((size_t)N + 16);

    vector<int> par(N, -1);

    // ---------- sweep ----------
    {
        struct Ev { ll x; int id; int typ; };  // typ 0 = delete, 1 = insert
        vector<Ev> ev((size_t)2 * N);
        for (int i = 0; i < N; i++) {
            ev[2 * i] = {CX[i] - RR[i], i, 1};      // leftmost : insert
            ev[2 * i + 1] = {CX[i] + RR[i], i, 0};  // rightmost: delete
        }
        sort(ev.begin(), ev.end(), [](const Ev &a, const Ev &b) {
            if (a.x != b.x) return a.x < b.x;
            return a.typ < b.typ;
        });

        set<ull, Cmp> st;
        vector<set<ull, Cmp>::iterator> itLo(N), itUp(N);

        auto checkExt = [&](int a, int b) {
            if (a < 0 || b < 0 || a == b) return;
            lll dx = (lll)CX[a] - CX[b], dy = (lll)CY[a] - CY[b];
            lll d2 = dx * dx + dy * dy;
            lll s = (lll)RR[a] + RR[b];
            if (d2 == s * s) {
                ll den = RR[a] + RR[b];
                ll nx = CX[a] * RR[b] + CX[b] * RR[a];
                ll ny = CY[a] * RR[b] + CY[b] * RR[a];
                int nd = getNode(nx, ny, den);
                incs.push_back({a, nd, (int)(CX[b] - CX[a]), (int)(CY[b] - CY[a])});
                incs.push_back({b, nd, (int)(CX[a] - CX[b]), (int)(CY[a] - CY[b])});
            }
        };

        vector<int> dels, ins;
        size_t ei = 0;
        while (ei < ev.size()) {
            ll x = ev[ei].x;
            size_t ej = ei;
            while (ej < ev.size() && ev[ej].x == x) ej++;
            curX = x;
            dels.clear(); ins.clear();
            size_t delStart = ei;
            // deletions first (events are sorted with typ 0 first)
            size_t k = ei;
            while (k < ej && ev[k].typ == 0) {
                int id = ev[k].id;
                auto itL = itLo[id], itU = itUp[id];
                bool hasBefore = (itL != st.begin());
                auto beforeL = itL;
                if (hasBefore) --beforeL;
                auto afterU = itU; ++afterU;
                bool hasAfter = (afterU != st.end());
                st.erase(itL);
                st.erase(itU);
                if (hasBefore && hasAfter)
                    checkExt((int)((*beforeL) >> 2), (int)((*afterU) >> 2));
                dels.push_back(id);
                k++;
            }
            (void)delStart;
            // insertions
            while (k < ej) {
                int id = ev[k].id;
                ins.push_back(id);
                k++;
            }
            if (!ins.empty()) {
                // outer circles first (nested circles can share leftmost point)
                sort(ins.begin(), ins.end(), [&](int a, int b) { return RR[a] > RR[b]; });
                for (int id : ins) {
                    ull qk = ((ull)id << 2) | 1ULL;  // s = 0 (query)
                    auto it = st.lower_bound(qk);
                    int pcircle = -1;
                    if (it != st.begin()) {
                        auto ip = it; --ip;
                        int c = (int)((*ip) >> 2);
                        int s = (int)((*ip) & 3) - 1;
                        pcircle = (s == 1) ? par[c] : c;
                    } else if (it != st.end()) {
                        int c = (int)((*it) >> 2);
                        int s = (int)((*it) & 3) - 1;
                        pcircle = (s == -1) ? c : par[c];
                    }
                    par[id] = pcircle;
                    auto iL = st.insert(it, ((ull)id << 2));
                    auto iU = st.insert(it, ((ull)id << 2) | 2ULL);
                    itLo[id] = iL; itUp[id] = iU;
                    if (iL != st.begin()) {
                        auto ip = iL; --ip;
                        checkExt((int)((*ip) >> 2), id);
                    }
                    auto in = iU; ++in;
                    if (in != st.end()) checkExt(id, (int)((*in) >> 2));
                }
            }
            // ---- vertical tangent line: circles that touch at an extreme-x point
            if (!dels.empty() && !ins.empty()) {
                sort(dels.begin(), dels.end(), [&](int a, int b) { return CY[a] < CY[b]; });
                // ins sorted by radius, re-sort by y
                sort(ins.begin(), ins.end(), [&](int a, int b) { return CY[a] < CY[b]; });
                size_t a = 0, b = 0;
                while (a < dels.size() && b < ins.size()) {
                    if (CY[dels[a]] < CY[ins[b]]) a++;
                    else if (CY[dels[a]] > CY[ins[b]]) b++;
                    else {
                        ll y = CY[dels[a]];
                        size_t a2 = a, b2 = b;
                        while (a2 < dels.size() && CY[dels[a2]] == y) a2++;
                        while (b2 < ins.size() && CY[ins[b2]] == y) b2++;
                        int nd = getNode(x, y, 1);
                        for (size_t t = a; t < a2; t++)
                            incs.push_back({dels[t], nd, (int)(x - CX[dels[t]]), (int)(y - CY[dels[t]])});
                        for (size_t t = b; t < b2; t++)
                            incs.push_back({ins[t], nd, (int)(x - CX[ins[t]]), (int)(y - CY[ins[t]])});
                        a = a2; b = b2;
                    }
                }
            }
            ei = ej;
        }
    }

    // ---------- internal tangency via nesting forest ----------
    for (int i = 0; i < N; i++) {
        int p = par[i];
        if (p < 0) continue;
        lll dx = (lll)CX[i] - CX[p], dy = (lll)CY[i] - CY[p];
        lll d2 = dx * dx + dy * dy;
        lll e = (lll)RR[p] - RR[i];
        if (d2 == e * e) {
            ll den = RR[p] - RR[i];
            ll nx = CX[i] * RR[p] - CX[p] * RR[i];
            ll ny = CY[i] * RR[p] - CY[p] * RR[i];
            int nd = getNode(nx, ny, den);
            // direction from each center towards P is a positive multiple of (c_i - c_p)
            incs.push_back({i, nd, (int)(CX[i] - CX[p]), (int)(CY[i] - CY[p])});
            incs.push_back({p, nd, (int)(CX[i] - CX[p]), (int)(CY[i] - CY[p])});
        }
    }

    // ---------- S and T ----------
    int sNode = getNode(Sx, Sy, 1);
    for (int i = 0; i < N; i++) {
        lll dx = (lll)Sx - CX[i], dy = (lll)Sy - CY[i];
        if (dx * dx + dy * dy == (lll)RR[i] * RR[i])
            incs.push_back({i, sNode, (int)(Sx - CX[i]), (int)(Sy - CY[i])});
    }
    int tNode = getNode(Tx, Ty, 1);
    for (int i = 0; i < N; i++) {
        lll dx = (lll)Tx - CX[i], dy = (lll)Ty - CY[i];
        if (dx * dx + dy * dy == (lll)RR[i] * RR[i])
            incs.push_back({i, tNode, (int)(Tx - CX[i]), (int)(Ty - CY[i])});
    }

    // ---------- build graph ----------
    {
        int M = nodeCnt;
        // the hash table is no longer needed; release it before allocating the graph
        unordered_map<Key, int, KeyHash, KeyEq>().swap(nodeMap);
        vector<int> head(M, -1);
        vector<int> nxt, to;
        vector<double> wt;
        nxt.reserve(incs.size() * 2 + 4);
        to.reserve(incs.size() * 2 + 4);
        wt.reserve(incs.size() * 2 + 4);
        auto addEdge = [&](int u, int v, double w) {
            nxt.push_back(head[u]); to.push_back(v); wt.push_back(w); head[u] = (int)to.size() - 1;
        };
        auto addUndir = [&](int u, int v, double w) {
            if (u == v) return;
            addEdge(u, v, w); addEdge(v, u, w);
        };
        // sort incidences by circle then angle
        sort(incs.begin(), incs.end(), [](const Inc &a, const Inc &b) {
            if (a.c != b.c) return a.c < b.c;
            int ha = (a.dy > 0 || (a.dy == 0 && a.dx > 0)) ? 0 : 1;
            int hb = (b.dy > 0 || (b.dy == 0 && b.dx > 0)) ? 0 : 1;
            if (ha != hb) return ha < hb;
            lll cr = (lll)a.dx * b.dy - (lll)a.dy * b.dx;
            return cr > 0;
        });
        const ld PI = acosl(-1.0L);
        vector<Inc> dl;
        size_t i = 0;
        while (i < incs.size()) {
            size_t j = i;
            int c = incs[i].c;
            while (j < incs.size() && incs[j].c == c) j++;
            dl.clear();
            for (size_t t = i; t < j; t++) {
                if (!dl.empty() && dl.back().nd == incs[t].nd) continue;
                dl.push_back(incs[t]);
            }
            size_t m = dl.size();
            if (m >= 2) {
                ld rr = (ld)RR[c];
                ld full = 2 * PI * rr;
                for (size_t t = 0; t < m; t++) {
                    if (m == 2 && t == 1) break;
                    const Inc &u = dl[t], &v = dl[(t + 1) % m];
                    lll cr = (lll)u.dx * v.dy - (lll)u.dy * v.dx;
                    lll dt = (lll)u.dx * v.dx + (lll)u.dy * v.dy;
                    ld ang = atan2l((ld)cr, (ld)dt);
                    if (ang <= 0) ang += 2 * PI;
                    ld arc = ang * rr;
                    ld half = min(arc, full - arc);
                    addUndir(u.nd, v.nd, (double)(half * (ld)VV[c]));
                }
            }
            i = j;
        }

        vector<Inc>().swap(incs);  // release incidence list before Dijkstra

        // Dijkstra
        const double INF = numeric_limits<double>::infinity();
        vector<double> dist(M, INF);
        priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
        dist[sNode] = 0;
        pq.push({0.0, sNode});
        while (!pq.empty()) {
            auto [d, u] = pq.top(); pq.pop();
            if (d > dist[u]) continue;
            if (u == tNode) break;
            for (int e = head[u]; e != -1; e = nxt[e]) {
                int v = to[e];
                double nd = d + wt[e];
                if (nd < dist[v]) { dist[v] = nd; pq.push({nd, v}); }
            }
        }
        if (dist[tNode] == INF) printf("-1\n");
        else printf("%.17Lg\n", (ld)dist[tNode]);
    }
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

typedef long long ll;
typedef unsigned long long ull;
typedef __int128 i128;
typedef unsigned __int128 u128;

/* ---------------- fast input ---------------- */
static char ibuf[1 << 16];
static int ipos = 0, ilen = 0;
static inline int gc() {
    if (ipos == ilen) { ilen = (int)fread(ibuf, 1, sizeof(ibuf), stdin); ipos = 0; if (ilen <= 0) return -1; }
    return (unsigned char)ibuf[ipos++];
}
static inline bool readInt(ll &out) {
    int c = gc();
    while (c != -1 && (c < '0' || c > '9') && c != '-') c = gc();
    if (c == -1) return false;
    bool neg = false;
    if (c == '-') { neg = true; c = gc(); }
    ll v = 0;
    while (c >= '0' && c <= '9') { v = v * 10 + (c - '0'); c = gc(); }
    out = neg ? -v : v;
    return true;
}

/* ---------------- data ---------------- */
static int N;
static vector<ll> CX, CY, CR, CSQ;
static vector<int> CV;
static ll SX, SY, TX, TY;

static ll GX;                      /* current sweep x */
static vector<pair<int,int>> TP;   /* detected tangency pairs */
static vector<char> S_ON, T_ON;

/* exact sign of  c + sqrt(A) + sqrt(B) */
static inline int sign_sum(ll c, ull A, ull B) {
    if (c > 0) return 1;
    if (c == 0) return (A == 0 && B == 0) ? 0 : 1;
    u128 k = (u128)(-(i128)c);
    u128 k2 = k * k;
    u128 s = (u128)A + (u128)B;
    if (s > k2) return 1;
    if (s == k2) return (A == 0 || B == 0) ? 0 : 1;
    u128 M = k2 - s;
    u128 lhs = (u128)4 * (u128)A * (u128)B;
    u128 rhs = M * M;
    if (lhs > rhs) return 1;
    if (lhs < rhs) return -1;
    return 0;
}

/* exact sign of  c + sqrt(A) - sqrt(B) */
static inline int sign_diff(ll c, ull A, ull B) {
    if (c >= 0) {
        u128 c2 = (u128)((i128)c * (i128)c);
        u128 base = c2 + (u128)A;
        if (base > (u128)B) return 1;
        if (base == (u128)B) return (A == 0 || c == 0) ? 0 : 1;
        if (A == 0 || c == 0) return -1;
        u128 M = (u128)B - base;
        u128 lhs = (u128)4 * c2 * (u128)A;
        u128 rhs = M * M;
        if (lhs > rhs) return 1;
        if (lhs < rhs) return -1;
        return 0;
    } else {
        u128 k = (u128)(-(i128)c);
        u128 k2 = k * k;
        if ((u128)A <= k2) return ((u128)A == k2 && B == 0) ? 0 : -1;
        u128 base2 = (u128)A + k2;
        if (base2 < (u128)B) return -1;
        if (base2 == (u128)B) return -1;
        u128 M = base2 - (u128)B;
        u128 lhs = (u128)4 * k2 * (u128)A;
        u128 rhs = M * M;
        if (lhs > rhs) return -1;
        if (lhs < rhs) return 1;
        return 0;
    }
}

/* exact sign of  c + s1*sqrt(G1) + s2*sqrt(G2) */
static inline int sign_form(ll c, int s1, ull G1, int s2, ull G2) {
    if (s1 == s2) {
        if (s1 > 0) return sign_sum(c, G1, G2);
        return -sign_sum(-c, G1, G2);
    } else {
        if (s1 > 0) return sign_diff(c, G1, G2);
        return sign_diff(c, G2, G1);
    }
}

/* ---------------- arc ordering ----------------
   arc id = 2*i     : lower arc of circle i
   arc id = 2*i + 1 : upper arc of circle i
   Ordered by y at x = GX (symbolic x+eps for exact ties).          */
static inline bool arcLess(int a, int b) {
    if (a == b) return false;
    int ca = a >> 1, cb = b >> 1;
    bool ua = (a & 1) != 0, ub = (b & 1) != 0;
    if (ca == cb) return (!ua) && ub;      /* same circle: lower arc first */

    ll da = GX - CX[ca];
    ll db = GX - CX[cb];
    ll ga = CSQ[ca] - da * da;
    ll gb = CSQ[cb] - db * db;
    if (ga < 0) ga = 0;                    /* only reachable on malformed input */
    if (gb < 0) gb = 0;

    double sa = sqrt((double)ga), sb = sqrt((double)gb);
    double ya = (double)CY[ca] + (ua ? sa : -sa);
    double yb = (double)CY[cb] + (ub ? sb : -sb);
    double dd = ya - yb;
    if (dd > 1e-4) return false;
    if (dd < -1e-4) return true;

    int sg = sign_form(CY[ca] - CY[cb], ua ? 1 : -1, (ull)ga, ub ? -1 : 1, (ull)gb);
    if (sg < 0) return true;
    if (sg > 0) return false;

    /* exact tie : both arcs pass through the same point at x = GX */
    if (ga == 0 && gb == 0) {
        if (ua && ub) return CR[ca] < CR[cb];
        if (!ua && !ub) return CR[ca] > CR[cb];
        return !ua;
    }
    {
        i128 dx = (i128)CX[ca] - CX[cb];
        i128 dy = (i128)CY[ca] - CY[cb];
        i128 d2 = dx * dx + dy * dy;
        i128 sum = (i128)CR[ca] + CR[cb];
        if (d2 == sum * sum) {
            /* external tangency: the upper-type arc is the lower one at x+eps */
            return ua;
        } else {
            /* internal tangency: larger radius arc is the outer one */
            if (ua) return CR[ca] < CR[cb];
            else return CR[ca] > CR[cb];
        }
    }
    return a < b;  /* safety net, never needed for valid input */
}

struct ArcCmp {
    inline bool operator()(int a, int b) const { return arcLess(a, b); }
};

static inline void checkPair(int a, int b) {
    if (a == b) return;
    i128 dx = (i128)CX[a] - CX[b];
    i128 dy = (i128)CY[a] - CY[b];
    i128 d2 = dx * dx + dy * dy;
    i128 s1 = (i128)CR[a] + CR[b];
    i128 s2 = (i128)CR[a] - CR[b];
    if (d2 == s1 * s1 || d2 == s2 * s2) TP.push_back({a, b});
}

/* ---------------- sweep ---------------- */
static void runSweep() {
    const ll OFF = 2000000000LL;
    vector<ull> ev(2 * (size_t)N);
    for (int i = 0; i < N; i++) {
        ev[2 * (size_t)i]     = ((ull)(uint32_t)(CX[i] + CR[i] + OFF) << 32) | (0ull << 31) | (uint32_t)i;
        ev[2 * (size_t)i + 1] = ((ull)(uint32_t)(CX[i] - CR[i] + OFF) << 32) | (1ull << 31) | (uint32_t)i;
    }
    std::sort(ev.begin(), ev.end());

    std::set<int, ArcCmp> st;
    vector<std::set<int, ArcCmp>::iterator> it(2 * (size_t)N);
    vector<pair<ll,int>> delv, insv;

    size_t p = 0, tot = ev.size();
    while (p < tot) {
        ull key = ev[p];
        ull xkey = key >> 32;
        ll x = (ll)xkey - OFF;
        GX = x;
        delv.clear(); insv.clear();
        size_t q = p;
        /* deletions at this x first */
        while (q < tot && (ev[q] >> 32) == xkey && ((ev[q] >> 31) & 1) == 0) {
            int c = (int)(ev[q] & 0x7fffffffu);
            delv.push_back({CY[c], c});
            int au = 2 * c + 1, al = 2 * c;
            {
                auto itu = it[au];
                auto nx = std::next(itu);
                bool hnx = (nx != st.end());
                bool hpv = (itu != st.begin());
                auto pv = hpv ? std::prev(itu) : st.end();
                st.erase(itu);
                if (hpv && hnx) checkPair((*pv) >> 1, (*nx) >> 1);
            }
            {
                auto itl = it[al];
                auto nx = std::next(itl);
                bool hnx = (nx != st.end());
                bool hpv = (itl != st.begin());
                auto pv = hpv ? std::prev(itl) : st.end();
                st.erase(itl);
                if (hpv && hnx) checkPair((*pv) >> 1, (*nx) >> 1);
            }
            q++;
        }
        /* then insertions */
        while (q < tot && (ev[q] >> 32) == xkey) {
            int c = (int)(ev[q] & 0x7fffffffu);
            insv.push_back({CY[c], c});
            int au = 2 * c + 1, al = 2 * c;
            it[au] = st.insert(st.lower_bound(au), au);
            it[al] = st.insert(st.lower_bound(al), al);
            for (int k = 0; k < 2; k++) {
                int a = k ? al : au;
                auto t = it[a];
                if (t != st.begin()) { auto pv = std::prev(t); checkPair((*pv) >> 1, c); }
                auto nx = std::next(t);
                if (nx != st.end()) checkPair((*nx) >> 1, c);
            }
            q++;
        }
        /* external tangency at a shared vertical tangent point:
           circles finishing at (x,y) touch circles starting at (x,y) */
        if (!delv.empty() && !insv.empty()) {
            std::sort(delv.begin(), delv.end());
            std::sort(insv.begin(), insv.end());
            size_t a2 = 0, b2 = 0;
            while (a2 < delv.size() && b2 < insv.size()) {
                if (delv[a2].first < insv[b2].first) a2++;
                else if (delv[a2].first > insv[b2].first) b2++;
                else {
                    checkPair(delv[a2].second, insv[b2].second);
                    ll yv = delv[a2].first;
                    while (a2 < delv.size() && delv[a2].first == yv) a2++;
                    while (b2 < insv.size() && insv[b2].first == yv) b2++;
                }
            }
        }
        p = q;
    }
}

/* ---------------- graph + dijkstra ---------------- */
static double solveGraph() {
    for (auto &p : TP) if (p.first > p.second) std::swap(p.first, p.second);
    std::sort(TP.begin(), TP.end());
    TP.erase(std::unique(TP.begin(), TP.end()), TP.end());
    const size_t P = TP.size();

    vector<int> cnt(N + 1, 0);
    for (auto &pr : TP) { cnt[pr.first]++; cnt[pr.second]++; }
    for (int i = 0; i < N; i++) { if (S_ON[i]) cnt[i]++; if (T_ON[i]) cnt[i]++; }
    vector<int> off(N + 1, 0);
    for (int i = 0; i < N; i++) off[i + 1] = off[i] + cnt[i];
    const int M = off[N];
    if (M == 0) return -1.0;

    struct Inc { int dx, dy, pairId; int flags; };
    vector<Inc> inc((size_t)M);
    vector<int> pos(N);
    for (int i = 0; i < N; i++) pos[i] = off[i];

    size_t pid = 0;
    for (auto &pr : TP) {
        int i = pr.first, j = pr.second;
        int idx1 = pos[i]++, idx2 = pos[j]++;
        i128 dx = (i128)CX[j] - CX[i];
        i128 dy = (i128)CY[j] - CY[i];
        i128 d2 = dx * dx + dy * dy;
        i128 s1 = (i128)CR[i] + CR[j];
        int sx, sy, tx, ty;
        if (d2 == s1 * s1) {
            sx = (int)dx; sy = (int)dy;
            tx = (int)(-dx); ty = (int)(-dy);
        } else {
            if (CR[i] > CR[j]) { sx = (int)dx; sy = (int)dy; }
            else { sx = (int)(-dx); sy = (int)(-dy); }
            tx = sx; ty = sy;
        }
        {
            int g = std::__gcd(sx < 0 ? -sx : sx, sy < 0 ? -sy : sy);
            if (g > 1) { sx /= g; sy /= g; }
        }
        {
            int g = std::__gcd(tx < 0 ? -tx : tx, ty < 0 ? -ty : ty);
            if (g > 1) { tx /= g; ty /= g; }
        }
        inc[idx1].dx = sx; inc[idx1].dy = sy; inc[idx1].pairId = (int)pid; inc[idx1].flags = 0;
        inc[idx2].dx = tx; inc[idx2].dy = ty; inc[idx2].pairId = (int)pid; inc[idx2].flags = 0;
        pid++;
    }
    for (int i = 0; i < N; i++) {
        if (S_ON[i]) {
            int k = pos[i]++;
            ll dx = SX - CX[i], dy = SY - CY[i];
            ll g = std::__gcd(llabs(dx), llabs(dy));
            if (g > 1) { dx /= g; dy /= g; }
            inc[k].dx = (int)dx; inc[k].dy = (int)dy; inc[k].pairId = -1; inc[k].flags = 1;
        }
        if (T_ON[i]) {
            int k = pos[i]++;
            ll dx = TX - CX[i], dy = TY - CY[i];
            ll g = std::__gcd(llabs(dx), llabs(dy));
            if (g > 1) { dx /= g; dy /= g; }
            inc[k].dx = (int)dx; inc[k].dy = (int)dy; inc[k].pairId = -1; inc[k].flags = 2;
        }
    }

    for (int i = 0; i < N; i++) {
        int lo = off[i], hi = off[i + 1];
        if (hi - lo <= 1) continue;
        std::sort(inc.begin() + lo, inc.begin() + hi, [](const Inc &A, const Inc &B) {
            int ha = (A.dy > 0 || (A.dy == 0 && A.dx > 0)) ? 0 : 1;
            int hb = (B.dy > 0 || (B.dy == 0 && B.dx > 0)) ? 0 : 1;
            if (ha != hb) return ha < hb;
            i128 cr = (i128)A.dx * B.dy - (i128)A.dy * B.dx;
            return cr > 0;
        });
    }

    vector<int> circ((size_t)M);
    vector<double> costNext((size_t)M);
    vector<int> prt((size_t)M);
    vector<char> flg((size_t)M);
    vector<int> pairFirst(P, -1);
    for (int i = 0; i < N; i++) {
        int lo = off[i], hi = off[i + 1];
        double w = (double)CV[i] * (double)CR[i];
        for (int k = lo; k < hi; k++) {
            const Inc &I = inc[k];
            circ[k] = i;
            int nk = (k + 1 < hi) ? k + 1 : lo;
            const Inc &J = inc[nk];
            i128 cr = (i128)I.dx * J.dy - (i128)I.dy * J.dx;   /* sin * |..| */
            i128 dt = (i128)I.dx * J.dx + (i128)I.dy * J.dy;   /* cos * |..| */
            double a = atan2((double)cr, (double)dt);
            if (a < 0) a += 2.0 * M_PI;
            costNext[k] = w * a;
            prt[k] = -1;
            flg[k] = (char)I.flags;
            if (I.pairId >= 0) {
                int pp = I.pairId;
                if (pairFirst[pp] < 0) pairFirst[pp] = k;
                else { prt[k] = pairFirst[pp]; prt[pairFirst[pp]] = k; }
            }
        }
    }
    inc.clear(); inc.shrink_to_fit();
    pairFirst.clear(); pairFirst.shrink_to_fit();

    const double INF = 1e300;
    vector<double> dist((size_t)M, INF);
    typedef pair<double,int> PDI;
    vector<PDI> hb;
    hb.reserve((size_t)M + 1);
    priority_queue<PDI, vector<PDI>, greater<PDI>> pq(greater<PDI>(), std::move(hb));
    for (int k = 0; k < M; k++) if (flg[k] & 1) { dist[k] = 0.0; pq.push({0.0, k}); }
    double best = INF;
    while (!pq.empty()) {
        PDI top = pq.top(); pq.pop();
        double d = top.first;
        int u = top.second;
        if (d > dist[u]) continue;
        if (flg[u] & 2) { best = d; break; }
        int c = circ[u];
        int lo = off[c], hi = off[c + 1];
        int nxt = (u + 1 < hi) ? u + 1 : lo;
        int prv = (u > lo) ? u - 1 : hi - 1;
        {
            double nd = d + costNext[u];
            if (nd < dist[nxt]) { dist[nxt] = nd; pq.push({nd, nxt}); }
        }
        {
            double nd = d + costNext[prv];
            if (nd < dist[prv]) { dist[prv] = nd; pq.push({nd, prv}); }
        }
        if (prt[u] >= 0) {
            int v = prt[u];
            if (d < dist[v]) { dist[v] = d; pq.push({d, v}); }
        }
    }
    return best;
}

int main() {
    ll tmp;
    if (!readInt(tmp)) return 0;
    N = (int)tmp;
    readInt(SX); readInt(SY); readInt(TX); readInt(TY);
    CX.resize(N); CY.resize(N); CR.resize(N); CSQ.resize(N); CV.resize(N);
    for (int i = 0; i < N; i++) {
        ll x, y, r, v;
        readInt(x); readInt(y); readInt(r); readInt(v);
        CX[i] = x; CY[i] = y; CR[i] = r; CV[i] = (int)v;
        CSQ[i] = r * r;
    }
    S_ON.assign(N, 0);
    T_ON.assign(N, 0);
    for (int i = 0; i < N; i++) {
        ll a = SX - CX[i], b = SY - CY[i];
        if (a * a + b * b == CSQ[i]) S_ON[i] = 1;
        ll c = TX - CX[i], d = TY - CY[i];
        if (c * c + d * d == CSQ[i]) T_ON[i] = 1;
    }
    TP.reserve(4 * (size_t)N + 16);
    runSweep();
    double ans = solveGraph();
    if (!(ans >= 0.0 && ans < 1e299)) printf("-1\n");
    else printf("%.15f\n", ans);
    return 0;
}

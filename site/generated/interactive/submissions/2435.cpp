#include <bits/stdc++.h>
using namespace std;
typedef unsigned long long u64;
typedef long double ld;

static u64 n;
static ld ALPHA = 0.9L, ALPHA2 = 0.9L;

#ifdef LOCAL
#define DIE() do{FAILED=true;throw 2;}while(0)
u64 SA, SB, SC;
int QC; int PH1;
bool FAILED;
static u64 ask(u64 l, u64 r) {
    if (l < 1 || r > n || l > r) { FAILED = true; throw 1; }
    QC++;
    u64 v = 0;
    if (l <= SA && SA <= r) v ^= SA;
    if (l <= SB && SB <= r) v ^= SB;
    if (l <= SC && SC <= r) v ^= SC;
    return v;
}
static vector<u64> RES;
static void answer(u64 a, u64 b, u64 c) { RES = {a, b, c}; throw 0; }
#else
#define DIE() exit(0)
static u64 ask(u64 l, u64 r) {
    printf("xor %llu %llu\n", l, r);
    fflush(stdout);
    long long v;
    if (scanf("%lld", &v) != 1) exit(0);
    if (v < 0) exit(0);
    return (u64)v;
}
static void answer(u64 a, u64 b, u64 c) {
    printf("ans %llu %llu %llu\n", a, b, c);
    fflush(stdout);
    exit(0);
}
#endif

static int topbit(u64 x) { return 63 - __builtin_clzll(x); }
static u64 cntIn(u64 lo, u64 hi) {  // |[lo,hi] ∩ [1,n]|
    if (lo < 1) lo = 1;
    if (hi > n) hi = n;
    if (lo > hi) return 0;
    return hi - lo + 1;
}

// ---------------- small n brute force ----------------
static void solveSmall() {
    vector<array<int, 3>> cand;
    for (int a = 1; a <= (int)n; a++)
        for (int b = a + 1; b <= (int)n; b++)
            for (int c = b + 1; c <= (int)n; c++) cand.push_back({a, b, c});
    while (cand.size() > 1) {
        double best = 1e18;
        int bl = 1, br = (int)n;
        static int cnt[128];
        for (int l = 1; l <= (int)n; l++)
            for (int r = l; r <= (int)n; r++) {
                memset(cnt, 0, sizeof cnt);
                for (auto &t : cand) {
                    int v = 0;
                    for (int x : t)
                        if (l <= x && x <= r) v ^= x;
                    cnt[v]++;
                }
                double s = 0;
                for (int v = 0; v < 128; v++)
                    if (cnt[v] > 1) s += cnt[v] * log((double)cnt[v]);
                if (s < best - 1e-9) { best = s; bl = l; br = r; }
            }
        int res = (int)ask(bl, br);
        vector<array<int, 3>> nc;
        for (auto &t : cand) {
            int v = 0;
            for (int x : t)
                if (bl <= x && x <= br) v ^= x;
            if (v == res) nc.push_back(t);
        }
        cand.swap(nc);
        if (cand.empty()) DIE();
    }
    answer(cand[0][0], cand[0][1], cand[0][2]);
}

// ---------------- pair search ----------------
// Two unknown stolen p<q with p^q=Y, h=topbit(Y). They lie in the same aligned
// block of size 2^(h+1), p in lower half, q in upper half.
struct Seg {
    u64 b0, b1;  // block index range inclusive
    ld w;        // weight per block
};
static vector<u64> known;
static u64 xorKnown(u64 l, u64 r) {
    u64 v = 0;
    for (u64 x : known)
        if (l <= x && x <= r) v ^= x;
    return v;
}

static ld BETA = 0.15L;
static ld Hn(u64 m) {  // harmonic number
    static vector<ld> tab;
    if (tab.empty()) {
        tab.resize(4097);
        tab[0] = 0;
        for (int i = 1; i <= 4096; i++) tab[i] = tab[i - 1] + 1.0L / i;
    }
    if (m <= 4096) return tab[m];
    ld x = (ld)m;
    return logl(x) + 0.57721566490153286061L + 1 / (2 * x) - 1 / (12 * x * x);
}
static ld hsum(u64 x, u64 y) {  // sum_{k=x}^{y} 1/k, x>=1
    if (x > y) return 0;
    if (y - x < 64) {
        ld s = 0;
        for (u64 k = x; k <= y; k++) s += 1.0L / (ld)k;
        return s;
    }
    if (x > 4096) return log1pl((ld)(y - x + 1) / ((ld)x - 0.5L));
    return Hn(y) - Hn(x - 1);
}
static pair<u64, u64> solvePair(u64 Y, const vector<pair<pair<u64, u64>, ld>> &regions) {
    int h = topbit(Y);
    u64 half = 1ULL << h;
    int sh = h + 1;
    auto blockCnt = [&](u64 b) -> u64 {
        u64 s = b << sh;
        u64 qlo = s + half, qhi = s + 2 * half - 1;
        u64 cq = 0;
        if (qlo <= n) cq = min(qhi, n) - qlo + 1;
        if (s == 0 && Y <= n && cq > 0) cq--;
        return cq;
    };
    vector<Seg> segs;
    u64 bn = n >> sh;
    for (auto &rg : regions) {
        u64 L = rg.first.first, R = rg.first.second;
        ld wp = rg.second;
        u64 bs = L >> sh, be = R >> sh;
        if (be > bn) be = bn;
        if (bs > be) continue;
        auto addOne = [&](u64 b) {
            u64 c = blockCnt(b);
            if (c > 0) segs.push_back({b, b, wp * (ld)c});
        };
        addOne(bs);
        if (be > bs) {
            if (be > bs + 1) segs.push_back({bs + 1, be - 1, wp * (ld)half});
            addOne(be);
        }
    }
    sort(segs.begin(), segs.end(), [](const Seg &a, const Seg &b) { return a.b0 < b.b0; });
    auto mkpq = [&](u64 p) { return make_pair(p, p ^ Y); };
    u64 Nall = bn + 1;
    ld W0 = 0;
    for (auto &s : segs) W0 += s.w * (ld)(s.b1 - s.b0 + 1);
    ld KB = BETA * W0 / (2 * (logl((ld)Nall) + 1.0L));
    auto mass = [&](const Seg &s, u64 b0, u64 b1) -> ld {
        return s.w * (ld)(b1 - b0 + 1) + KB * (hsum(b0 + 1, b1 + 1) + hsum(bn - b1 + 1, bn - b0 + 1));
    };
    while (true) {
        ld W = 0;
        u64 N = 0;
        for (auto &s : segs) { W += mass(s, s.b0, s.b1); N += s.b1 - s.b0 + 1; }
        if (N == 0) DIE();  // should not happen
        if (N == 1) {
            u64 b = segs[0].b0;
            u64 s = b << sh;
            u64 c = blockCnt(b);
            if (c == 1) {
                // unique p
                for (u64 p = s; p < s + half; p++) {
                    if (p >= 1 && (p ^ Y) <= n) return mkpq(p);
                }
            }
            u64 l = max<u64>(s, 1), r = s + half - 1;
            u64 v = ask(l, r) ^ xorKnown(l, r);
            return mkpq(v);
        }
        auto blockAt = [&](ld target) -> u64 {
            ld cum = 0;
            for (auto &s : segs) {
                ld tot = mass(s, s.b0, s.b1);
                if (cum + tot > target) {
                    ld t = target - cum;
                    u64 lo = s.b0, hi = s.b1;  // find smallest b with mass(b0..b) > t
                    while (lo < hi) {
                        u64 mid = lo + (hi - lo) / 2;
                        if (mass(s, s.b0, mid) > t) hi = mid;
                        else lo = mid + 1;
                    }
                    return lo;
                }
                cum += tot;
            }
            return segs.back().b1;
        };
        auto nextB = [&](u64 b) -> long long {
            for (auto &s : segs) {
                if (s.b1 <= b) continue;
                return (long long)max(s.b0, b + 1);
            }
            return -1;
        };
        auto prevB = [&](u64 b) -> long long {
            for (int i = (int)segs.size() - 1; i >= 0; i--) {
                auto &s = segs[i];
                if (s.b0 >= b) continue;
                return (long long)min(s.b1, b - 1);
            }
            return -1;
        };
        u64 bi = blockAt(W / 4), bk = blockAt(W * 3 / 4);
        if (bi == bk) {
            long long nx = nextB(bk);
            if (nx >= 0) bk = nx;
            else bi = prevB(bi);
        }
        u64 l = (bi << sh) + half, r = (bk << sh) + half - 1;
        u64 v = ask(l, r) ^ xorKnown(l, r);
        if (v == 0) {
            // remove blocks in [bi,bk]
            vector<Seg> ns;
            for (auto &s : segs) {
                if (s.b1 < bi || s.b0 > bk) { ns.push_back(s); continue; }
                if (s.b0 < bi) ns.push_back({s.b0, bi - 1, s.w});
                if (s.b1 > bk) ns.push_back({bk + 1, s.b1, s.w});
            }
            segs.swap(ns);
        } else if (v == Y) {
            vector<Seg> ns;
            for (auto &s : segs) {
                u64 a = max(s.b0, bi + 1), b = min(s.b1, bk - 1);
                if (a <= b) ns.push_back({a, b, s.w});
            }
            segs.swap(ns);
        } else {
            u64 blk = v >> sh;
            if (blk == bi && ((v >> h) & 1)) return mkpq(v ^ Y);  // v is q
            return mkpq(v);                                      // v is p
        }
    }
}

// ---------------- T != 0 ----------------
static u64 T;
static pair<u64, u64> blockOf(int j) {  // B_j(T) unclipped, j in [0,60]
    u64 lo = (T >> (j + 1)) << (j + 1);
    u64 hi = lo + ((1ULL << (j + 1)) - 1);
    return {lo, hi};
}
static u64 bcnt(int j) {
    if (j < 0) return cntIn(T, T);
    auto b = blockOf(j);
    return cntIn(b.first, b.second);
}

// adversarial-ish prior over (E,D): D uniform over feasible levels, E|D half
// geometric (towards D-1), half uniform.
static vector<vector<ld>> buildU(const vector<vector<char>> &feas, int L) {
    vector<vector<ld>> U(L, vector<ld>(L, 0));
    int nD = 0;
    for (int d = 0; d < L; d++) {
        bool any = false;
        for (int e = 0; e < d; e++) any |= feas[e][d];
        nD += any;
    }
    for (int d = 0; d < L; d++) {
        ld sg = 0;
        int c = 0;
        for (int e = 0; e < d; e++)
            if (feas[e][d]) { sg += powl(2.0L, e - d); c++; }
        if (!c) continue;
        for (int e = 0; e < d; e++)
            if (feas[e][d]) U[e][d] = (0.5L * powl(2.0L, e - d) / sg + 0.5L / c) / nD;
    }
    return U;
}

static void solveNonZero() {
    const int L = 61;
    vector<ld> ring(L, 0), oh(L, 0);
    for (int e = 0; e < L; e++) ring[e] = (ld)(bcnt(e) - bcnt(e - 1));
    for (int d = 1; d < L; d++) oh[d] = (ld)(bcnt(d) - bcnt(d - 1));
    vector<vector<ld>> R(L, vector<ld>(L, 0));
    vector<vector<char>> feas(L, vector<char>(L, 0));
    ld sumR = 0;
    int nf = 0;
    for (int e = 0; e < L; e++)
        for (int d = e + 1; d < L; d++)
            if (ring[e] >= 1 && oh[d] >= 2) {
                feas[e][d] = 1;
                R[e][d] = ring[e] * oh[d] / 2;
                sumR += R[e][d];
                nf++;
            }
    if (nf == 0) DIE();
    auto U = buildU(feas, L);
    vector<vector<ld>> m(L, vector<ld>(L, 0));
    for (int e = 0; e < L; e++)
        for (int d = e + 1; d < L; d++)
            if (feas[e][d]) m[e][d] = ALPHA * R[e][d] / sumR + (1 - ALPHA) * U[e][d];
    // M[a][b] = sum over e>=a, d<=b
    vector<vector<ld>> M(L + 1, vector<ld>(L, 0));
    for (int a = L - 1; a >= 0; a--)
        for (int b = 0; b < L; b++) {
            ld s = M[a + 1][b];
            for (int d = a + 1; d <= b; d++) s += m[a][d];
            M[a][b] = s;
        }
    vector<vector<ld>> f(L + 1, vector<ld>(L, 0));
    vector<vector<int>> ch(L + 1, vector<int>(L, -1));
    for (int len = 1; len < L; len++)
        for (int a = 0; a + len < L; a++) {
            int b = a + len;
            if (M[a][b] <= 0) { f[a][b] = 0; continue; }
            ld best = 1e300;
            int bj = -1;
            for (int j = a; j < b; j++) {
                ld c = M[j + 1][b] * f[j + 1][b] + M[a][j] * f[a][j];
                if (j >= 0) {}
                if (c < best) { best = c; bj = j; }
            }
            f[a][b] = 1 + best / M[a][b];
            ch[a][b] = bj;
        }
    int Elo = 0, Dhi = L - 1;
    u64 e = 0;
    int hitj = -1;
    while (true) {
        if (Elo >= Dhi) DIE();
        int j = ch[Elo][Dhi];
        if (j < 0) DIE();
        auto b = blockOf(j);
        u64 lo = max<u64>(b.first, 1), hi = min(b.second, n);
        u64 v;
        if (lo > hi) v = 0;
        else if (lo == 1 && hi == n) v = T;
        else v = ask(lo, hi);
        if (v == T) Dhi = j;
        else if (v == 0) Elo = j + 1;
        else { e = v; hitj = j; break; }
    }
#ifdef LOCAL
    PH1=QC;
#endif
    known.push_back(e);
    u64 Y = T ^ e;
    int h = topbit(Y);
    vector<pair<pair<u64, u64>, ld>> regions;
    for (int d = hitj + 1; d <= Dhi; d++) {
        if (!(oh[d] >= 2) || d <= h) continue;
        u64 lo = ((T >> d) ^ 1ULL) << d;
        u64 hi = lo + ((1ULL << d) - 1);
        ld rr = ring[h] * oh[d] / 2;
        ld wp = ALPHA2 / sumR + (1 - ALPHA2) * U[h][d] / rr;
        regions.push_back({{lo, hi}, wp});
    }
    auto pq = solvePair(Y, regions);
    answer(e, pq.first, pq.second);
}

// ---------------- T == 0 ----------------
static void solveZero() {
    const int L = 61;
    vector<ld> lvl(L, 0);
    for (int k = 0; k < L; k++) {
        u64 lo = 1ULL << k, hi = (k == 63) ? ~0ULL : ((1ULL << (k + 1)) - 1);
        lvl[k] = (ld)cntIn(lo, hi);
    }
    vector<vector<ld>> R(L, vector<ld>(L, 0));
    vector<vector<char>> alive(L, vector<char>(L, 0));
    ld sumR = 0;
    int nf = 0;
    for (int a = 0; a < L; a++)
        for (int d = a + 1; d < L; d++)
            if (lvl[a] >= 1 && lvl[d] >= 2) {
                alive[a][d] = 1;
                R[a][d] = lvl[a] * lvl[d] * lvl[d] / (2 * powl(2.0L, d));
                sumR += R[a][d];
                nf++;
            }
    if (nf == 0) DIE();
    auto U = buildU(alive, L);
    vector<vector<ld>> m(L, vector<ld>(L, 0));
    for (int a = 0; a < L; a++)
        for (int d = a + 1; d < L; d++)
            if (alive[a][d]) m[a][d] = ALPHA * R[a][d] / sumR + (1 - ALPHA) * U[a][d];
    u64 A = 0;
    int hitj = -1;
    while (true) {
        ld best = -1;
        int bj = -1;
        for (int j = 1; j < L; j++) {
            ld s = 0;
            for (int a = 0; a < j; a++)
                for (int d = j; d < L; d++)
                    if (alive[a][d]) s += m[a][d];
            if (s > best) { best = s; bj = j; }
        }
        if (bj < 0 || best <= 0) DIE();
        u64 hi = (1ULL << bj) - 1;
        u64 v;
        if (hi >= n) v = 0;
        else v = ask(1, hi);
        if (v != 0) { A = v; hitj = bj; break; }
        for (int a = 0; a < bj; a++)
            for (int d = bj; d < L; d++) alive[a][d] = 0;
    }
    known.push_back(A);
    int la = topbit(A);
    vector<pair<pair<u64, u64>, ld>> regions;
    for (int d = hitj; d < L; d++) {
        if (!alive[la][d]) continue;
        u64 lo = 1ULL << d, hi = (1ULL << (d + 1)) - 1;
        ld wp = ALPHA2 / sumR + (1 - ALPHA2) * U[la][d] / R[la][d];
        regions.push_back({{lo, hi}, wp});
    }
    auto pq = solvePair(A, regions);
    answer(A, pq.first, pq.second);
}

static void solve() {
    known.clear();
    if (n <= 60) { solveSmall(); return; }
    T = ask(1, n);
    if (T == 0) solveZero();
    else solveNonZero();
}

#ifdef LOCAL
#include "harness.h"
#else
int main() {
    unsigned long long x;
    if (scanf("%llu", &x) != 1) return 0;
    n = x;
    solve();
    return 0;
}
#endif

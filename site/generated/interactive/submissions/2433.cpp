// Robot trapping - devil strategy based on certified 1D wall defense.
#include <bits/stdc++.h>
using namespace std;

namespace dv {

const int MAXC = 1000;
const int ACT = 5; // activation distance

// W1[d][mask]: devil (to move) wins 1D wall game at distance d with window mask (bit=1 -> unblocked)
static vector<char> W1[ACT + 1];

static void buildW1() {
    for (int d = 1; d <= ACT; d++) W1[d].assign(1u << (2 * d + 1), 1);
    bool changed = true;
    while (changed) {
        changed = false;
        for (int d = 1; d <= ACT; d++) {
            int w = 2 * d + 1;
            unsigned full = (1u << w) - 1;
            for (unsigned m = 0; m <= full; m++) {
                if (!W1[d][m]) continue;
                bool ok = false;
                for (int c = -1; c < w && !ok; c++) {
                    unsigned mm = m;
                    if (c >= 0) {
                        if (!(m >> c & 1)) continue;
                        mm &= ~(1u << c);
                    }
                    bool good = true;
                    if (d == 1) {
                        if (mm) good = false;
                    } else
                        for (int s = 0; s < 3 && good; s++) {
                            unsigned nm = (mm >> s) & ((1u << (w - 2)) - 1);
                            if (!W1[d - 1][nm]) good = false;
                        }
                    if (good) { unsigned nm = (mm >> 1) | (1u << (w - 1)); if (!W1[d][nm]) good = false; }
                    if (good) { unsigned nm = ((mm << 1) | 1) & full; if (!W1[d][nm]) good = false; }
                    if (good && d < ACT) {
                        unsigned a = (mm << 2) | 3, b = (mm << 1) | 1 | (1u << (w + 1)), cc = mm | (3u << w);
                        if (!W1[d + 1][a] || !W1[d + 1][b] || !W1[d + 1][cc]) good = false;
                    }
                    if (good) ok = true;
                }
                if (!ok) { W1[d][m] = 0; changed = true; }
            }
        }
    }
}

struct Cell { int x, y; };
static inline bool operator==(const Cell& a, const Cell& b) { return a.x == b.x && a.y == b.y; }
static inline bool cellLess(const Cell& a, const Cell& b) { return a.x != b.x ? a.x < b.x : a.y < b.y; }

struct Seg {
    bool vert; // vertical line x=c (cells (c,p)), else horizontal y=c (cells (p,c))
    int c, lo, hi, side; // side=+1: king coordinate < c
};

struct Rect { int x1, x2, y1, y2; long area() const { return (long)(x2 - x1 + 1) * (y2 - y1 + 1); } };

struct Devil {
    vector<vector<char>> blk;
    int kx, ky;
    Rect rg;
    vector<Cell> pending; // must-build cells (initial corner)
    int turn = 0;
    int brokenCount = 0;

    bool isB(int x, int y) const {
        if (x < 1 || y < 1) return true;
        if (x > MAXC || y > MAXC) return false;
        return blk[x][y];
    }

    static int sdist(const Seg& s, int x, int y) { return s.vert ? s.side * (s.c - x) : s.side * (s.c - y); }
    static int salong(const Seg& s, int x, int y) { return s.vert ? y : x; }
    static Cell scell(const Seg& s, int p) { return s.vert ? Cell{s.c, p} : Cell{p, s.c}; }

    bool segU(const Seg& s, int p) const {
        if (p < s.lo || p > s.hi) return false;
        Cell q = scell(s, p);
        return !isB(q.x, q.y);
    }
    unsigned wmask(const Seg& s, int x, int y, int d) const {
        int p0 = salong(s, x, y) - d;
        unsigned m = 0;
        for (int i = 0; i <= 2 * d; i++) if (segU(s, p0 + i)) m |= 1u << i;
        return m;
    }
    // with current board, every king move from (x,y) leads to a winning state for s (no kill needed)
    bool segPass(const Seg& s, int x, int y) const {
        int d = sdist(s, x, y);
        if (d > ACT) return true;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                if (!dx && !dy) continue;
                int nx = x + dx, ny = y + dy;
                if (nx < 1 || ny < 1) continue;
                if (isB(nx, ny)) continue; // robot would explode
                int nd = sdist(s, nx, ny);
                if (nd <= 0) return false;
                if (nd > ACT) continue;
                if (!W1[nd][wmask(s, nx, ny, nd)]) return false;
            }
        return true;
    }
    bool segOKk(const Seg& s, int x, int y, Cell k) {
        if (k.x < 1) return segPass(s, x, y);
        char old = blk[k.x][k.y];
        blk[k.x][k.y] = 1;
        bool r = segPass(s, x, y);
        blk[k.x][k.y] = old;
        return r;
    }
    bool segComplete(const Seg& s) const {
        for (int p = max(s.lo, 1); p <= s.hi; p++) {
            Cell q = scell(s, p);
            if (!isB(q.x, q.y)) return false;
        }
        return true;
    }
    int segWhite(const Seg& s) const {
        int c = 0;
        for (int p = max(s.lo, 1); p <= s.hi; p++) {
            Cell q = scell(s, p);
            if (!isB(q.x, q.y)) c++;
        }
        return c;
    }

    static Seg makeBnd(int side, const Rect& r) {
        if (side == 0) return {true, r.x1 - 1, r.y1 - 1, r.y2 + 1, -1};
        if (side == 1) return {true, r.x2 + 1, r.y1 - 1, r.y2 + 1, +1};
        if (side == 2) return {false, r.y1 - 1, r.x1 - 1, r.x2 + 1, -1};
        return {false, r.y2 + 1, r.x1 - 1, r.x2 + 1, +1};
    }
    static bool isAxis(int side, const Rect& r) { return (side == 0 && r.x1 == 1) || (side == 2 && r.y1 == 1); }

    vector<pair<int, Seg>> regionSegs(const Rect& r) const {
        vector<pair<int, Seg>> v;
        for (int sd = 0; sd < 4; sd++) {
            if (isAxis(sd, r)) continue;
            Seg s = makeBnd(sd, r);
            if (!segComplete(s)) v.push_back({sd, s});
        }
        return v;
    }
    vector<Seg> activeSegs() const {
        vector<Seg> v;
        for (auto& [sd, s] : regionSegs(rg)) v.push_back(s);
        return v;
    }

    // positions where both A and B within ACT
    static void zoneRect(const Seg& A, const Seg& B, const Rect& r, Rect& z) {
        z = r;
        for (const Seg* s : {&A, &B}) {
            if (s->vert) {
                if (s->side > 0) z.x1 = max(z.x1, s->c - ACT); else z.x2 = min(z.x2, s->c + ACT);
            } else {
                if (s->side > 0) z.y1 = max(z.y1, s->c - ACT); else z.y2 = min(z.y2, s->c + ACT);
            }
        }
    }
    void pairViol(const Seg& A, const Seg& B, const Rect& z, vector<Cell>& out) const {
        out.clear();
        for (int x = z.x1; x <= z.x2; x++)
            for (int y = z.y1; y <= z.y2; y++) {
                if (isB(x, y)) continue;
                if (!segPass(A, x, y) && !segPass(B, x, y)) out.push_back({x, y});
            }
    }
    bool pairOK(const Seg& A, const Seg& B, const Rect& r) const {
        Rect z; zoneRect(A, B, r, z);
        for (int x = z.x1; x <= z.x2; x++)
            for (int y = z.y1; y <= z.y2; y++) {
                if (isB(x, y)) continue;
                if (!segPass(A, x, y) && !segPass(B, x, y)) return false;
            }
        return true;
    }
    // greedy set of cells to fix pair violations (board restored)
    vector<Cell> fixPair(const Seg& A, const Seg& B, const Rect& r) {
        Rect z; zoneRect(A, B, r, z);
        vector<Cell> V; pairViol(A, B, z, V);
        vector<Cell> added;
        int guard = 0;
        while (!V.empty() && guard++ < 80) {
            vector<Cell> cs;
            for (const Seg* s : {&A, &B}) {
                int mn = INT_MAX, mx = INT_MIN;
                for (auto& p : V) { int a = salong(*s, p.x, p.y); mn = min(mn, a); mx = max(mx, a); }
                for (int p = max(mn - ACT - 1, s->lo); p <= min(mx + ACT + 1, s->hi); p++) {
                    if (p < 1) continue;
                    Cell q = scell(*s, p);
                    if (q.x > MAXC || q.y > MAXC) continue;
                    if (!isB(q.x, q.y)) cs.push_back(q);
                }
            }
            if (cs.empty()) break;
            int best = -1; Cell bc = cs[0];
            for (auto& q : cs) {
                blk[q.x][q.y] = 1;
                int fixed = 0;
                for (auto& p : V) if (isB(p.x, p.y) || segPass(A, p.x, p.y) || segPass(B, p.x, p.y)) fixed++;
                blk[q.x][q.y] = 0;
                if (fixed > best) { best = fixed; bc = q; }
            }
            blk[bc.x][bc.y] = 1;
            added.push_back(bc);
            vector<Cell> V2;
            for (auto& p : V) if (!(isB(p.x, p.y) || segPass(A, p.x, p.y) || segPass(B, p.x, p.y))) V2.push_back(p);
            V.swap(V2);
        }
        for (auto& q : added) blk[q.x][q.y] = 0;
        return added;
    }

    void init(int sx, int sy) {
        buildW1();
        blk.assign(MAXC + 2, vector<char>(MAXC + 2, 0));
        kx = sx; ky = sy;
        t0 = chrono::steady_clock::now();
        // optimal 6-cell corner prebuild (relative to corner cell)
        const int off[6][2] = {{0, -5}, {0, -3}, {-3, 0}, {0, -2}, {0, -1}, {0, 0}};
        {
            int k = 6, Lx = sx + ACT + k + LM, Ly = sy + ACT + k + LM;
            cLx = Lx; cLy = Ly;
            rg = {1, Lx - 1, 1, Ly - 1};
            Seg R = makeBnd(1, rg), T = makeBnd(3, rg);
            pending.clear();
            for (auto& o : off) pending.push_back({Lx + o[0], Ly + o[1]});
            for (auto& q : pending) blk[q.x][q.y] = 1;
            bool ok = pairOK(R, T, rg);
            for (auto& q : pending) blk[q.x][q.y] = 0;
            if (ok) return;
        }
        auto setup = [&](int Lx, int Ly) {
            rg = {1, Lx - 1, 1, Ly - 1};
            Seg R = makeBnd(1, rg), T = makeBnd(3, rg);
            pending = fixPair(R, T, rg);
        };
        setup(sx + 40, sy + 40);
        int k = (int)pending.size();
        setup(sx + ACT + k, sy + ACT + k);
        while ((int)pending.size() > k) { k = (int)pending.size(); setup(sx + ACT + k, sy + ACT + k); }
    }
    int LM = 1;
    int cLx = 0, cLy = 0;
    chrono::steady_clock::time_point t0;
    double elapsed() const { return chrono::duration<double>(chrono::steady_clock::now() - t0).count(); }

    static Rect cutRect(const Rect& r, int side, int c) {
        Rect n = r;
        if (side == 0) n.x1 = c + 1;
        else if (side == 1) n.x2 = c - 1;
        else if (side == 2) n.y1 = c + 1;
        else n.y2 = c - 1;
        return n;
    }
    static int cutDist(int side, int c, int x, int y) {
        if (side == 0) return x - c;
        if (side == 1) return c - x;
        if (side == 2) return y - c;
        return c - y;
    }

    bool cutPairsOK(int side, int c) const {
        Rect n = cutRect(rg, side, c);
        Seg ns = makeBnd(side, n);
        for (auto& [sd, s] : regionSegs(n)) {
            if (sd == side) continue;
            if (!pairOK(ns, s, n)) return false;
        }
        return true;
    }
    // exists a kill acceptable for all segs at king pos
    bool existsKill(const vector<Seg>& ss) {
        bool allPass = true;
        for (auto& s : ss) if (!segPass(s, kx, ky)) { allPass = false; break; }
        if (allPass) return true;
        for (auto& s : ss) {
            if (segPass(s, kx, ky)) continue;
            int d = sdist(s, kx, ky);
            int p0 = salong(s, kx, ky) - d;
            for (int i = 0; i <= 2 * d; i++) {
                if (!segU(s, p0 + i)) continue;
                Cell q = scell(s, p0 + i);
                bool ok = true;
                for (auto& t : ss) if (!segOKk(t, kx, ky, q)) { ok = false; break; }
                if (ok) return true;
            }
            return false; // the non-pass seg must get the kill from its own window
        }
        return false;
    }
    bool commitValid(int side, int c) {
        int d = cutDist(side, c, kx, ky);
        if (d < 1) return false;
        if (!cutPairsOK(side, c)) return false;
        Rect n = cutRect(rg, side, c);
        vector<Seg> ss;
        for (auto& [sd, s] : regionSegs(n)) ss.push_back(s);
        return existsKill(ss);
    }

    struct Cand { int side, c, d; };
    vector<Cand> candidates() const {
        vector<Cand> v;
        for (int c = kx + 1; c <= rg.x2; c++) v.push_back({1, c, c - kx});
        for (int c = rg.x1; c <= kx - 1; c++) v.push_back({0, c, kx - c});
        for (int c = ky + 1; c <= rg.y2; c++) v.push_back({3, c, c - ky});
        for (int c = rg.y1; c <= ky - 1; c++) v.push_back({2, c, ky - c});
        return v;
    }
    long cutRed(int side, int c) const { return rg.area() - cutRect(rg, side, c).area(); }

    int cheb(Cell q) const { return max(abs(q.x - kx), abs(q.y - ky)); }

    Cell choose() {
        turn++;
        {
            vector<Cell> np;
            for (auto& q : pending) if (!isB(q.x, q.y)) np.push_back(q);
            pending.swap(np);
        }
        if (pending.empty()) {
            for (int iter = 0; iter < 8; iter++) {
                vector<pair<long, Cand>> order;
                for (auto& cd : candidates()) order.push_back({cutRed(cd.side, cd.c), cd});
                sort(order.begin(), order.end(), [](auto& a, auto& b) { return a.first > b.first; });
                bool any = false;
                for (auto& [red, cd] : order) {
                    if (red <= 0) break;
                    if (commitValid(cd.side, cd.c)) {
                        rg = cutRect(rg, cd.side, cd.c);
                        any = true;
#ifdef DBG
                        fprintf(stderr, "t=%d commit side %d c %d -> [%d,%d]x[%d,%d] king(%d,%d)\n", turn, cd.side, cd.c, rg.x1, rg.x2, rg.y1, rg.y2, kx, ky);
#endif
                        break;
                    }
                }
                if (!any) break;
            }
        }
        vector<Seg> act = activeSegs();
        bool allPass = true;
        for (auto& s : act) if (!segPass(s, kx, ky)) { allPass = false; break; }
        if (!allPass) {
            vector<Cell> cand;
            for (auto& s : act) {
                if (segPass(s, kx, ky)) continue;
                int d = sdist(s, kx, ky);
                int p0 = salong(s, kx, ky) - d;
                for (int i = 0; i <= 2 * d; i++)
                    if (segU(s, p0 + i)) cand.push_back(scell(s, p0 + i));
            }
            sort(cand.begin(), cand.end(), cellLess);
            cand.erase(unique(cand.begin(), cand.end()), cand.end());
            vector<Cell> good;
            for (auto& q : cand) {
                bool ok = true;
                for (auto& s : act) if (!segOKk(s, kx, ky, q)) { ok = false; break; }
                if (ok) good.push_back(q);
            }
            if (good.empty()) {
                brokenCount++;
                Cell bq = cand.empty() ? fallback() : cand[0];
                int bd = 1e9;
                for (auto& q : cand) { int dd = cheb(q); if (dd < bd) { bd = dd; bq = q; } }
                return place(bq);
            }
            if (good.size() == 1) return place(good[0]);
            if (pending.empty()) { Cell q = endgamePick(good, true); if (q.x > 0) return place(q); }
            vector<Cell> plan = pending;
            Cell pick = good[0];
            int bs = INT_MIN;
            for (auto& q : good) {
                int sc = 0;
                for (auto& p : plan) if (p == q) sc += 50;
                sc -= cheb(q);
                if (sc > bs) { bs = sc; pick = q; }
            }
            return place(pick);
        }
        if (!pending.empty() && cLx > 0) {
            int dmin = min(cLx - kx, cLy - ky);
            if ((int)pending.size() < dmin - ACT) { Cell q = endgamePick({}, false); if (q.x > 0) return place(q); }
        }
        if (!pending.empty()) {
            Cell pick = pending[0]; int bd = 1e9;
            for (auto& q : pending) { int dd = cheb(q); if (dd < bd) { bd = dd; pick = q; } }
            return place(pick);
        }
        { Cell q = endgamePick({}, false); if (q.x > 0) return place(q); }
        return place(fallback());
    }

    // ---------------- endgame search ----------------
    int SDEP = 2, SRAD = 2;
    vector<int> vis; int vstamp = 0;
    bool inReg(int x, int y) const { return x >= rg.x1 && x <= rg.x2 && y >= rg.y1 && y <= rg.y2; }
    bool VB(int x, int y) const { return !inReg(x, y) || isB(x, y); }
    int compSize(int sx, int sy) {
        if (vis.empty()) vis.assign((MAXC + 2) * (MAXC + 2), 0);
        vstamp++;
        static vector<int> q; q.clear();
        q.push_back(sx * (MAXC + 2) + sy); vis[q[0]] = vstamp;
        size_t h = 0;
        while (h < q.size()) {
            int id = q[h++]; int x = id / (MAXC + 2), y = id % (MAXC + 2);
            for (int dx = -1; dx <= 1; dx++)
                for (int dy = -1; dy <= 1; dy++) {
                    if (!dx && !dy) continue;
                    int a = x + dx, b = y + dy;
                    if (VB(a, b)) continue;
                    int j = a * (MAXC + 2) + b;
                    if (vis[j] == vstamp) continue;
                    vis[j] = vstamp; q.push_back(j);
                }
        }
        return (int)q.size();
    }
    int realWhiteNb(int x, int y) const {
        int c = 0;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                if (!dx && !dy) continue;
                if (!isB(x + dx, y + dy)) c++;
            }
        return c;
    }
    double WN = 3.0;
    double W2 = 0.5;
    double W3 = 0.3;
    double evalPos(int x, int y) {
        int n = realWhiteNb(x, y);
        double v = compSize(x, y) + WN * n;
        if (W2 > 0) {
            int n2 = 0;
            for (int dx = -2; dx <= 2; dx++)
                for (int dy = -2; dy <= 2; dy++) if (!VB(x + dx, y + dy)) n2++;
            v += W2 * n2;
        }
        if (W3 > 0) {
            int n3 = 0;
            for (int dx = -3; dx <= 3; dx++)
                for (int dy = -3; dy <= 3; dy++) if (!VB(x + dx, y + dy)) n3++;
            v += W3 * n3;
        }
        return v;
    }
    void searchCands(int x, int y, vector<Cell>& cs) const {
        cs.clear();
        for (int a = max(1, x - SRAD); a <= x + SRAD; a++)
            for (int b = max(1, y - SRAD); b <= y + SRAD; b++) {
                if (a > MAXC || b > MAXC) continue;
                if (isB(a, b)) continue;
                if (a < rg.x1 - 1 || a > rg.x2 + 1 || b < rg.y1 - 1 || b > rg.y2 + 1) continue;
                cs.push_back({a, b});
            }
    }
    // devil to move with king at (x,y); returns estimated cost
    double dsearch(int x, int y, int depth) {
        if (realWhiteNb(x, y) <= 1) return -1000;
        if (depth == 0) return evalPos(x, y);
        double best = 1e18;
        vector<Cell> cs; searchCands(x, y, cs);
        for (auto& k : cs) {
            blk[k.x][k.y] = 1;
            double worst = kingWorst(x, y, depth, best);
            blk[k.x][k.y] = 0;
            if (worst < best) best = worst;
        }
        return best + 1;
    }
    double kingWorst(int x, int y, int depth, double cut) {
        double worst = -1e18; bool any = false;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                if (!dx && !dy) continue;
                int nx = x + dx, ny = y + dy;
                if (VB(nx, ny)) continue;
                any = true;
                double v = dsearch(nx, ny, depth - 1);
                if (v > worst) worst = v;
                if (worst >= cut) return worst;
            }
        if (!any) return -2000;
        return worst;
    }
    Cell endgamePick(const vector<Cell>& forcedGood, bool forced) {
        double el = elapsed();
        int savedDep = SDEP;
        if (el > 8.5) { if (forced) return forcedGood[0]; return {-1, -1}; }
        if (el > 6.0) SDEP = 1;
        Cell r = endgamePick2(forcedGood, forced);
        SDEP = savedDep;
        return r;
    }
    Cell endgamePick2(const vector<Cell>& forcedGood, bool forced) {
        vector<Cell> cs;
        if (forced) cs = forcedGood; else searchCands(kx, ky, cs);
        if (cs.empty()) return {-1, -1};
        double best = 1e18; Cell bc = cs[0];
        for (auto& k : cs) {
            blk[k.x][k.y] = 1;
            double w = kingWorst(kx, ky, SDEP, best + 1e-9);
            blk[k.x][k.y] = 0;
            w += 1e-4 * cheb(k);
            if (w < best) { best = w; bc = k; }
        }
        return bc;
    }

    Cell fallback() const {
        Cell best{-1, -1}; int bd = 1e9;
        for (auto& [sd, s] : regionSegs(rg)) {
            for (int p = max(s.lo, 1); p <= s.hi; p++) {
                Cell q = scell(s, p);
                if (isB(q.x, q.y)) continue;
                int dd = cheb(q);
                if (dd < bd) { bd = dd; best = q; }
            }
        }
        if (best.x > 0) return best;
        int bf = -1;
        for (int x = rg.x1; x <= rg.x2; x++)
            for (int y = rg.y1; y <= rg.y2; y++) {
                if (isB(x, y) || (x == kx && y == ky)) continue;
                int dd = max(abs(x - kx), abs(y - ky));
                if (dd > bf) { bf = dd; best = {x, y}; }
            }
        if (best.x > 0) return best;
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++) {
                int x = kx + dx, y = ky + dy;
                if ((dx || dy) && x >= 1 && y >= 1 && !isB(x, y)) return {x, y};
            }
        return {kx, ky};
    }

    Cell place(Cell q) {
        if (q.x < 1 || q.y < 1 || q.x > MAXC || q.y > MAXC) q = {1, 1};
        blk[q.x][q.y] = 1;
        return q;
    }
    void update(int nx, int ny) { kx = nx; ky = ny; }
};

} // namespace dv

#ifndef SIM
int main() {
    int sx, sy;
    if (!(cin >> sx >> sy)) return 0;
    dv::Devil D;
    D.init(sx, sy);
    while (true) {
        dv::Cell c = D.choose();
        printf("%d %d\n", c.x, c.y);
        fflush(stdout);
        int nx, ny;
        if (!(cin >> nx >> ny)) return 0;
        if (nx == 0 && ny == 0) return 0;
        if (nx < 0) return 0;
        D.update(nx, ny);
    }
}
#endif

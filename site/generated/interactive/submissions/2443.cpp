#include <bits/stdc++.h>
using namespace std;

static int n, N;
static vector<int8_t> foot;   // -1 unvisited, 0..3 footprint dir, 4 empty
static vector<int> lab;
static int pr, pc;
static const int DR[4] = {-1, 0, 1, 0}, DC[4] = {0, -1, 0, 1};
static const char DCH[4] = {'^', '<', 'v', '>'};
static int S;
static double FOLLOW_FACTOR = 0.25;

static inline int idx(int r, int c) { return r * N + c; }
static inline bool inF(int r, int c) { return r >= 0 && r < N && c >= 0 && c < N; }

static void doMove(int d) {
    pr += DR[d];
    pc += DC[d];
    printf("%c\n", DCH[d]);
    fflush(stdout);
    char buf[16];
    if (scanf("%15s", buf) != 1) exit(0);
    char ch = buf[0];
    if (ch == 'G') exit(0);
    int v = idx(pr, pc);
    int8_t val;
    if (ch == '^') val = 0;
    else if (ch == '<') val = 1;
    else if (ch == 'v') val = 2;
    else if (ch == '>') val = 3;
    else if (ch == '.') val = 4;
    else exit(0);
    foot[v] = val;
}

static void stepTo(int r, int c) {  // adjacent
    for (int d = 0; d < 4; d++)
        if (pr + DR[d] == r && pc + DC[d] == c) { doMove(d); return; }
}

// predecessor of visited path cell v known to be a visited cell?
static bool predKnownVisited(int r, int c) {
    for (int d = 0; d < 4; d++) {
        int nr = r + DR[d], nc = c + DC[d];
        if (!inF(nr, nc)) continue;
        int8_t f = foot[idx(nr, nc)];
        if (f >= 0 && f < 4 && f == (d + 2) % 4) return true;
    }
    return false;
}

// ---------- travel: shortest path with penalty for unvisited cells ----------
static vector<uint16_t> tdist;
static vector<uint8_t> tdir;
static int TRAVEL_STEP = 2, TRAVEL_PEN = 3, TRAVEL_MARGIN = 3;
static void travel(int tr, int tc) {
    if (pr == tr && pc == tc) return;
    int r0 = max(0, min(pr, tr) - TRAVEL_MARGIN), r1 = min(N - 1, max(pr, tr) + TRAVEL_MARGIN);
    int c0 = max(0, min(pc, tc) - TRAVEL_MARGIN), c1 = min(N - 1, max(pc, tc) + TRAVEL_MARGIN);
    int h = r1 - r0 + 1, w = c1 - c0 + 1;
    size_t sz = (size_t)h * w;
    if (tdist.size() < sz) { tdist.resize(sz); tdir.resize(sz); }
    fill(tdist.begin(), tdist.begin() + sz, (uint16_t)65535);
    int maxw = TRAVEL_STEP + TRAVEL_PEN;
    vector<vector<int>> buckets(maxw + 1);
    auto L = [&](int r, int c) { return (size_t)(r - r0) * w + (c - c0); };
    // search from target to source so we can walk forward
    tdist[L(tr, tc)] = 0;
    buckets[0].push_back((int)L(tr, tc));
    int cur = 0, pending = 1;
    size_t src = L(pr, pc);
    while (pending > 0) {
        auto &bk = buckets[cur % (maxw + 1)];
        while (!bk.empty()) {
            int u = bk.back(); bk.pop_back(); pending--;
            if (tdist[u] != cur) continue;
            if ((size_t)u == src) { pending = 0; break; }
            int ur = u / w + r0, uc = u % w + c0;
            // cost of entering u (from neighbor) = step + pen(u) ; target counted too
            int cu = TRAVEL_STEP + (foot[idx(ur, uc)] < 0 ? TRAVEL_PEN : 0);
            for (int d = 0; d < 4; d++) {
                int nr = ur + DR[d], nc = uc + DC[d];
                if (nr < r0 || nr > r1 || nc < c0 || nc > c1) continue;
                size_t v = L(nr, nc);
                int nd = cur + cu;
                if (nd < (int)tdist[v]) {
                    tdist[v] = (uint16_t)nd;
                    tdir[v] = (uint8_t)((d + 2) % 4);  // direction from v to u
                    buckets[nd % (maxw + 1)].push_back((int)v);
                    pending++;
                }
            }
        }
        if (pending <= 0) break;
        cur++;
    }
    for (auto &b : buckets) b.clear();
    while (!(pr == tr && pc == tc)) {
        int d = tdir[L(pr, pc)];
        doMove(d);
    }
}

// ---------- cost table ----------
static const int M = 320;
static vector<int> ftab;
static void buildF() {
    ftab.assign((M + 1) * (M + 1), 0);
    for (int a = 1; a <= M; a++)
        for (int b = 1; b <= a; b++) {
            int best = INT_MAX;
            for (int x = 0; x < a; x++) {
                int l = ftab[x * (M + 1) + b], r = ftab[(a - 1 - x) * (M + 1) + b];
                int c = x + b + max(l, r);
                if (c < best) best = c;
            }
            for (int y = 0; y < b; y++) {
                int y1 = y, y2 = b - 1 - y;
                int l = a >= y1 ? ftab[a * (M + 1) + y1] : ftab[y1 * (M + 1) + a];
                int r = a >= y2 ? ftab[a * (M + 1) + y2] : ftab[y2 * (M + 1) + a];
                int c = y + a + max(l, r);
                if (c < best) best = c;
            }
            ftab[a * (M + 1) + b] = best;
            ftab[b * (M + 1) + a] = best;
        }
}
static double Fest(int w, int h) {
    if (w <= 0 || h <= 0) return 0;
    if (w <= M && h <= M) return ftab[w * (M + 1) + h];
    double s = (double)max(w, h) / M;
    int ww = max(1, min(M, (int)lround(w / s))), hh = max(1, min(M, (int)lround(h / s)));
    return ftab[ww * (M + 1) + hh] * s;
}

// ---------- region ----------
static int curLab = 0, nextLab = 1, roundFirst = 1;
static int R0, R1, C0, C1;
static vector<int> cells;  // flood buffer
static vector<uint8_t> mark;

struct Comp { int r0, r1, c0, c1; long long sz; int base; };

static inline bool inRround(int v) {
    return foot[v] < 0 && (lab[v] == curLab || lab[v] >= roundFirst);
}

static void computeRegion() {
    roundFirst = nextLab;
    while (true) {
        int first = nextLab;
        vector<Comp> comps;
        cells.clear();
        for (int r = R0; r <= R1; r++)
            for (int c = C0; c <= C1; c++) {
                int v = idx(r, c);
                if (!inRround(v) || lab[v] >= first) continue;
                int L = nextLab++;
                Comp cp{r, r, c, c, 0, 0};
                size_t st = cells.size();
                lab[v] = L;
                cells.push_back(v);
                size_t qi = st;
                while (qi < cells.size()) {
                    int u = cells[qi++];
                    int ur = u / N, uc = u % N;
                    cp.r0 = min(cp.r0, ur); cp.r1 = max(cp.r1, ur);
                    cp.c0 = min(cp.c0, uc); cp.c1 = max(cp.c1, uc);
                    for (int d = 0; d < 4; d++) {
                        int nr = ur + DR[d], nc = uc + DC[d];
                        if (!inF(nr, nc)) continue;
                        int w = idx(nr, nc);
                        if (inRround(w) && lab[w] < first) { lab[w] = L; cells.push_back(w); }
                    }
                }
                cp.sz = cells.size() - st;
                comps.push_back(cp);
            }
        auto labelOf = [&](int u) -> int {
            if (foot[u] < 0 && lab[u] >= first) return lab[u] - first;
            return -1;
        };
        // boundary visited cells
        vector<int> bnd;
        for (int u : cells) {
            int ur = u / N, uc = u % N;
            for (int d = 0; d < 4; d++) {
                int nr = ur + DR[d], nc = uc + DC[d];
                if (!inF(nr, nc)) continue;
                int w = idx(nr, nc);
                if (foot[w] >= 0 && !mark[w]) { mark[w] = 1; bnd.push_back(w); }
            }
        }
        vector<pair<int, vector<int>>> amb;  // cell, candidate cells
        for (int v : bnd) {
            mark[v] = 0;
            int f = foot[v];
            if (f == 4) continue;
            int vr = v / N, vc = v % N;
            int sr = vr + DR[f], sc = vc + DC[f];
            int succ = -1;
            if (inF(sr, sc)) {
                succ = idx(sr, sc);
                int l = labelOf(succ);
                if (l >= 0) comps[l].base++;
            }
            if (v == S) continue;
            if (predKnownVisited(vr, vc)) continue;
            vector<int> cand;
            int l0 = -2;
            bool multi = false;
            for (int d = 0; d < 4; d++) {
                int nr = vr + DR[d], nc = vc + DC[d];
                if (!inF(nr, nc)) continue;
                int w = idx(nr, nc);
                if (w == succ || foot[w] >= 0) continue;
                cand.push_back(w);
                int l = labelOf(w);
                if (l0 == -2) l0 = l;
                else if (l != l0) multi = true;
            }
            if (cand.empty()) continue;  // should not happen
            if (!multi) {
                if (l0 >= 0) comps[l0].base--;
            } else {
                amb.push_back({v, cand});
            }
        }
        vector<int> P;
        for (int i = 0; i < (int)comps.size(); i++)
            if (comps[i].base >= 1) P.push_back(i);
        if (P.size() == 1 || amb.empty()) {
            int ch;
            if (P.size() >= 1) ch = P[0];
            else {
                // inconsistent; choose largest component
                ch = 0;
                for (int i = 1; i < (int)comps.size(); i++)
                    if (comps[i].sz > comps[ch].sz) ch = i;
            }
            if (comps.empty()) return;  // nothing left (should not happen)
            vector<int>().swap(cells);
            curLab = first + ch;
            R0 = comps[ch].r0; R1 = comps[ch].r1; C0 = comps[ch].c0; C1 = comps[ch].c1;
            return;
        }
        // resolve relevant ambiguous cells
        vector<char> inP(comps.size(), 0);
        for (int x : P) inP[x] = 1;
        vector<int> targets;
        for (auto &a : amb) {
            bool rel = false;
            for (int w : a.second) {
                int l = labelOf(w);
                if (l >= 0 && inP[l]) rel = true;
            }
            if (P.empty()) rel = true;
            if (!rel) continue;
            // choose candidate nearest to current pos
            int bw = -1, bd = INT_MAX;
            for (int w : a.second) {
                int d = abs(w / N - pr) + abs(w % N - pc);
                if (d < bd) { bd = d; bw = w; }
            }
            targets.push_back(bw);
        }
        if (targets.empty()) {
            for (auto &a : amb) targets.push_back(a.second[0]);
        }
        // greedy nearest tour
        vector<char> done(targets.size(), 0);
        for (size_t k = 0; k < targets.size(); k++) {
            int bi = -1, bd = INT_MAX;
            for (size_t i = 0; i < targets.size(); i++) {
                if (done[i]) continue;
                int w = targets[i];
                if (foot[w] >= 0) { done[i] = 1; continue; }
                int d = abs(w / N - pr) + abs(w % N - pc);
                if (d < bd) { bd = d; bi = (int)i; }
            }
            if (bi < 0) break;
            done[bi] = 1;
            travel(targets[bi] / N, targets[bi] % N);
        }
        // update bbox to cover all round comps (unchanged R0..C1 is fine)
    }
}

// ---------- cut walking ----------
// horizontal: along = column, cross = row
static void walkCut(bool horiz, int y, int a0, int a1, int crossLo, int crossHi) {
    auto cellRC = [&](int a, int b, int &r, int &c) {
        if (horiz) { r = b; c = a; } else { r = a; c = b; }
    };
    int dir = a1 >= a0 ? 1 : -1;
    int r, c;
    cellRC(a0, y, r, c);
    travel(r, c);
    int b = y;
    int alt = 0;
    for (int a = a0;; a += dir) {
        cellRC(a, b, r, c);
        int v = idx(r, c);
        int f = foot[v];
        if (f >= 0 && f < 4 && v != S && !predKnownVisited(r, c)) {
            int succ = -1;
            {
                int sr = r + DR[f], sc = c + DC[f];
                if (inF(sr, sc)) succ = idx(sr, sc);
            }
            bool hasM = false, hasP = false;
            for (int s = -1; s <= 1; s += 2) {
                int rr, cc;
                cellRC(a, b + s, rr, cc);
                if (!inF(rr, cc)) continue;
                int w = idx(rr, cc);
                if (w == succ || foot[w] >= 0) continue;
                if (s < 0) hasM = true; else hasP = true;
            }
            if (hasM && hasP) {
                int s;
                if (b > y) s = -1;
                else if (b < y) s = 1;
                else {
                    bool okM = y - 1 >= crossLo, okP = y + 1 <= crossHi;
                    if (okM && !okP) s = -1;
                    else if (okP && !okM) s = 1;
                    else { s = alt ? 1 : -1; alt ^= 1; }
                }
                int rr, cc;
                cellRC(a, b + s, rr, cc);
                if (inF(rr, cc)) {
                    stepTo(rr, cc);
                    b += s;
                }
            }
        }
        if (a == a1) break;
        cellRC(a + dir, b, r, c);
        stepTo(r, c);
    }
}

static void planAndCut() {
    int h = R1 - R0 + 1, w = C1 - C0 + 1;
    vector<int> rmn(h, INT_MAX), rmx(h, INT_MIN), cmn(w, INT_MAX), cmx(w, INT_MIN);
    for (int r = R0; r <= R1; r++)
        for (int c = C0; c <= C1; c++) {
            int v = idx(r, c);
            if (foot[v] < 0 && lab[v] == curLab) {
                int i = r - R0, j = c - C0;
                rmn[i] = min(rmn[i], c); rmx[i] = max(rmx[i], c);
                cmn[j] = min(cmn[j], r); cmx[j] = max(cmx[j], r);
            }
        }
    double best = 1e18;
    bool bh = true;
    int by = 0, ba0 = 0, ba1 = 0;
    for (int pass = 0; pass < 2; pass++) {
        bool horiz = pass == 0;
        int len = horiz ? h : w;
        vector<int> &mn = horiz ? rmn : cmn;
        vector<int> &mx = horiz ? rmx : cmx;
        vector<int> pmn(len + 1, INT_MAX), pmx(len + 1, INT_MIN), smn(len + 2, INT_MAX), smx(len + 2, INT_MIN);
        for (int i = 0; i < len; i++) { pmn[i + 1] = min(pmn[i], mn[i]); pmx[i + 1] = max(pmx[i], mx[i]); }
        for (int i = len - 1; i >= 0; i--) { smn[i] = min(smn[i + 1], mn[i]); smx[i] = max(smx[i + 1], mx[i]); }
        int base = horiz ? R0 : C0;
        int ppos = horiz ? pr : pc, palong = horiz ? pc : pr;
        for (int y = 0; y < len; y++) {
            if (mn[y] == INT_MAX) continue;
            int lo = max(0, y - 1), hi = min(len - 1, y + 1);
            int L = INT_MAX, Rr = INT_MIN;
            for (int k = lo; k <= hi; k++) if (mn[k] != INT_MAX) { L = min(L, mn[k]); Rr = max(Rr, mx[k]); }
            double fa = 0, fb = 0;
            if (y > 0 && pmn[y] != INT_MAX) fa = Fest(pmx[y] - pmn[y] + 1, y);
            if (y < len - 1 && smn[y + 1] != INT_MAX) fb = Fest(smx[y + 1] - smn[y + 1] + 1, len - 1 - y);
            double fut = max(fa, fb);
            double cross = abs(ppos - (base + y));
            for (int e = 0; e < 2; e++) {
                int st = e == 0 ? L : Rr, en = e == 0 ? Rr : L;
                double cost = cross + abs(palong - st) + (Rr - L) + fut;
                if (cost < best) { best = cost; bh = horiz; by = base + y; ba0 = st; ba1 = en; }
            }
        }
    }
    if (bh) walkCut(true, by, ba0, ba1, R0, R1);
    else walkCut(false, by, ba0, ba1, C0, C1);
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
    N = 2 * n + 1;
    foot.assign((size_t)N * N, -1);
    lab.assign((size_t)N * N, 0);
    mark.assign((size_t)N * N, 0);
    pr = pc = n;
    S = idx(n, n);
    foot[S] = 0;
    buildF();
    // follow phase
    long long FOLLOW_ADD = 8;
    long long K = (long long)(FOLLOW_FACTOR * N) + FOLLOW_ADD;
    for (long long k = 0; k < K; k++) {
        int f = foot[idx(pr, pc)];
        if (f < 0 || f >= 4) break;
        int nr = pr + DR[f], nc = pc + DC[f];
        if (!inF(nr, nc)) break;
        doMove(f);
    }
    curLab = 0;
    R0 = 0; R1 = N - 1; C0 = 0; C1 = N - 1;
    computeRegion();
    while (true) {
        planAndCut();
        computeRegion();
    }
}

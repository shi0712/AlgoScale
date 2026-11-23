// core solver v2
#include <bits/stdc++.h>
using namespace std;
typedef bitset<200> B;
struct BH { size_t operator()(const B& b) const { return hash<B>()(b); } };
struct Done { int ans; };

#ifndef ADTH
#define ADTH 8
#endif
#ifndef SWEEP
#define SWEEP 0
#endif
#ifndef USELB
#define USELB 1
#endif
#ifndef DEGADD
#define DEGADD 0
#endif
#ifndef FINDALL
#define FINDALL 0
#endif
#ifndef HUB
#define HUB 1
#endif
#ifndef HUBF
#define HUBF 2
#endif
#ifndef FARK
#define FARK 1
#endif
#ifndef SOFTK
#define SOFTK 1000
#endif
#ifndef RECOV
#define RECOV 0
#endif
#ifndef CLOS
#define CLOS 6
#endif
#ifndef PRIORF
#define PRIORF 2.0
#endif
#ifndef SWPRUNE
#define SWPRUNE 0
#endif
#ifndef LOCAL
#define LOCAL 1
#endif
#ifndef FARMIN
#define FARMIN 2
#endif
#ifndef CMODE_R
#define CMODE_R 0.5
#endif

struct Solver {
    int n;
    function<int(const B&)> ask;
    unordered_map<B,int,BH> cache;
    B ALL;
    int queries = 0, finds = 0, tests = 0, fq = 0, useless = 0;
    int phaseQ[8] = {0};
    int Q(const B& s) {
        if (s.none() || s == ALL) return 0;
        auto it = cache.find(s);
        if (it != cache.end()) return it->second;
        int r = ask(s); queries++;
        cache[s] = r;
        if (r == 0) throw Done{0};
        return r;
    }
    bool known(const B& s) { return s.none() || s == ALL || cache.count(s); }
    B S, P, Far; int fS;
    int farAt[200]; double farScore = 0.75; int farOff = 0;
    deque<int> gainHist; int hubCool = 0; deque<int> uslHist; int addsSinceSweep = 100;
    int deg(int v) { B b; b[v] = 1; return Q(b); }

    // segment tree over labels
    vector<B> rng_;
    void build(int id, int l, int r) { B b; for (int i = l; i < r; i++) b[i] = 1; rng_[id] = b; if (r - l > 1) { int m = (l + r) / 2; build(2*id, l, m); build(2*id+1, m, r); } }

    void addSet(const B& T, int y, bool certified) {
        int c = y + (int)T.count() - fS;
        if (!certified && c == 1) P |= T;
        if (c <= 0) useless++;
        gainHist.push_back(max(c, 0)); if (gainHist.size() > 4) gainHist.pop_front();
        uslHist.push_back(c <= 0); if (uslHist.size() > 6) uslHist.pop_front(); addsSinceSweep++;
        S |= T; fS = y; Far &= ~S;
        lastAdd = T._Find_first(); if (T.count() > 1) lastAdd = -1; lastC = c;
    }
    int lastAdd = -1, lastC = 0; double locScore = 0.5; int locIdle = 0;
    // returns true if it added something
    bool localProbe() {
        if (!LOCAL || lastAdd < 0) return false;
        if (locScore < 0.3) { if (++locIdle < 25) return false; locIdle = 0; }
        int v = lastAdd;
        int offs[] = {2, -2, 1, -1};
        int tried = 0;
        for (int o : offs) {
            int u = v + o; if (u < 0 || u >= n || S[u]) continue;
            if (tried >= LOCAL) break;
            tried++;
            B b; b[u] = 1;
            int res = ptest(b, 0, 0);
            locScore = locScore * 0.75 + (res >= 1 ? 0.25 : 0);
            if (res == 2) return true;
        }
        return false;
    }

    int run() {
        if (n == 1) return 1;
        for (int i = 0; i < n; i++) ALL[i] = 1;
        rng_.resize(1024); build(1, 0, n);
        for (int i = 0; i < 200; i++) farAt[i] = -1;
        try {
            S[0] = 1; fS = Q(S);
            while (true) {
                B U = ALL & ~S;
                int m = U.count();
                if (m == 0) break;
                int r = m - fS;
                if (r == 0) break;
                if (r == 1) { addSet(U, 0, false); break; }
                int q0 = queries, ph;
                if (CLOS && wantSweep()) { ph = 0; addsSinceSweep = 0; uslHist.clear(); csweep(1); }
                else if (HUB && wantHub(m, r)) { ph = 1; hubStep(); } else if (useC(m, r)) { ph = 2; cStep(); } else if (FINDALL) { ph = 3; epoch(U); } else { ph = 4; pStep(U); }
                phaseQ[ph] += queries - q0;
            }
            { int q0 = queries; pendingCheck(P); phaseQ[5] += queries - q0; }
            return 1;
        } catch (Done d) { return d.ans; }
    }
    // verify no vertex of Pset is isolated (sufficient: each has neighbor outside the set)
    void pendingCheck(B Ps) {
        int k = Ps.count();
        if (k == 0) return;
        int y = Q(ALL & ~Ps); // throws if 0
        if (y == k) return;
        if (k == 1) return; // y>0 => not isolated
        // split
        B A1, A2; int cnt = 0;
        for (int v = 0; v < n; v++) if (Ps[v]) { if (cnt++ < k / 2) A1[v] = 1; else A2[v] = 1; }
        pendingCheck(A1); pendingCheck(A2);
    }

    // ---------- mode choice ----------
    double cSucc = 1, cFail = 0; // decayed counts
    bool useC(int m, int r) {
        if ((double)r > CMODE_R * m) return false;
        if (cFail > 1.5 * cSucc + 1) return false;
        return true;
    }

    // ---------- C-descent: find vertex in N[R] using y only ----------
    void cStep() {
        finds++;
        B U = ALL & ~S & ~Far;
        int id = 1;
        B X = U;
        if (X.none()) { recheckFar(); return; }
        // c(X) w.r.t. current S
        while (true) {
            int t = X.count();
            if (t == 1) {
                int v = X._Find_first();
                int y = Q(S | X); int c = y + 1 - fS;
                if (c <= 1) { addSet(X, y, false); cSucc = cSucc * 0.9 + 1; cFail *= 0.9; return; }
                int d = deg(v);
                int g = fS + d - y;
                if (g > 0) { addSet(X, y, true); cSucc = cSucc * 0.9 + 1; cFail *= 0.9; }
                else { Far[v] = 1; cFail = cFail * 0.9 + 1; cSucc *= 0.9; if (SWEEP) sweep(1); }
                return;
            }
            B L = X & rng_[2*id], R = X & rng_[2*id+1];
            if (L.none()) { id = 2*id+1; continue; }
            if (R.none()) { id = 2*id; continue; }
            // test smaller? test L
            B T = L; int tid = 2*id; B O = R; int oid = 2*id+1;
            if (known(S | R) && !known(S | L)) { T = R; tid = 2*id+1; O = L; oid = 2*id; }
            tests++;
            int y = Q(S | T); int c = y + (int)T.count() - fS;
            if (c <= 1) {
                addSet(T, y, false);
                if (c == 1) { cSucc = cSucc * 0.9 + 1; cFail *= 0.9; return; }
                X = O & ~S; id = oid; continue;
            }
            X = T; id = tid;
        }
    }
    void sweep(int id) {
        B T = rng_[id] & ~S & ~Far; int t = T.count(); if (!t) return;
        int m = (ALL & ~S).count(); if (m == fS) return;
        tests++;
        int y = Q(S | T); int c = y + t - fS;
        if (c <= 1) { addSet(T, y, false); return; }
        if (t == 1) { Far |= T; return; }  // heavy: treat as far-ish for C-descent
        sweep(2*id); sweep(2*id+1);
    }
    void recheckFar() {
        // all candidates outside Far have c==0? we got here with U\Far empty
        B F = Far; Far.reset();
        for (int v = 0; v < n; v++) if (F[v] && !S[v]) {
            B b; b[v] = 1;
            int y = Q(S | b); int d = deg(v);
            if (fS + d - y > 0) { addSet(b, y, true); return; }
            Far[v] = 1;
        }
        // all far and U = Far: R ⊆ Far, no vertex near => disconnected
        throw Done{0};
    }

    // ---------- find-all epoch ----------
    B marked; B certT; int certY; bool haveCert;
    // returns whether node set is positive (tested or known)
    void fa(int id, B T, int known_pos) {
        // known_pos: 1 if T known positive, 0 unknown
        int t = T.count(); if (!t) return;
        B L = T & rng_[2*id], R = T & rng_[2*id+1];
        if (t > 1 && (L.none() || R.none())) { fa(L.none() ? 2*id+1 : 2*id, T, known_pos); return; }
        int pos = known_pos;
        if (!pos) {
            tests++;
            int y = Q(S | T); int c = y + t - fS;
            if (c <= 1) { marked |= T; if (c == 1) P |= T; return; }
            int g = -1;
            if (g < 0) { int q0 = queries; int ft = Q(T); fq += queries - q0; g = fS + ft - y; }
            hist.push_back({t, 0, g > 0}); if (hist.size() > 30) hist.pop_front();
            if (g <= 0) return;
            pos = 1;
        }
        if (t == 1) { marked |= T; return; }
        // positive: split. test L; if L negative, R positive.
        // we need to know L's result to infer R
        int before = marked.count(); B mb = marked;
        // test L directly
        tests++;
        int yl = Q(S | L); int cl = yl + (int)L.count() - fS;
        int lpos;
        if (cl <= 1) { marked |= L; if (cl == 1) P |= L; lpos = 2; }
        else { int ft = Q(L); fq++; lpos = (fS + ft - yl > 0) ? 1 : 0; }
        if (lpos == 1) fa(2*id, L, 1);
        if (lpos == 0) fa(2*id+1, R, 1);
        else fa(2*id+1, R, 0);
    }
    void epoch(B U) {
        finds++;
        marked.reset();
        fa(1, U, 1);
        if (marked.none()) return; // shouldn't happen
        B NS = S | marked;
        int y = Q(NS);
        S = NS; fS = y; Far &= ~S;
    }
    bool wantSweep() {
        if ((int)uslHist.size() < 6 || addsSinceSweep < 6) return false;
        int k = 0; for (int x : uslHist) k += x;
        return k >= CLOS;
    }
    void csweep(int id) {
        B T = rng_[id] & ~S; int t = T.count(); if (!t) return;
        int m = (ALL & ~S).count(); if (m == fS) return;
        if (farRecent(T)) return;
        tests++;
        int y = Q(S | T); int c = y + t - fS;
        if (c <= 1) { addSet(T, y, false); return; }
        if (t == 1) { farAt[T._Find_first()] = S.count(); return; }
        int r = m - fS;
        if (SWPRUNE && c >= t && c < SWPRUNE * r) return;
        csweep(2*id); csweep(2*id+1);
    }
    // ---------- hub seeking ----------
    bool wantHub(int m, int r) {
        if (hubCool > 0) { hubCool--; return false; }
        if (gainHist.size() < 3 || r < 16) return false;
        double p = estP(); if (p < 0.4) return false;
        double ag = 0; for (int g : gainHist) ag += g; ag /= gainHist.size();
        double lg = log2((double)m);
        return max(ag, 0.5) * (2 * lg + 2) * HUBF < r;
    }
    void hubStep() {
        finds++;
        double ag = 0; for (int g : gainHist) ag += g; ag /= gainHist.size();
        int q0 = queries; int f0 = fS, m0 = (ALL & ~S).count();
        hubInner();
        int r0 = m0 - f0, r1 = (int)(ALL & ~S).count() - fS;
        double gain = r0 - r1, cost = queries - q0;
        if (gain < ag * cost * 0.7) hubCool = 30;
    }
    void hubInner() {
        B X = ALL & ~S & ~Far; int id = 1;
        while (true) {
            int t = X.count();
            if (t == 0) { hubCool = 5; return; }
            if (t == 1) {
                int v = X._Find_first();
                int y = Q(S | X); int c = y + 1 - fS;
                if (c <= 1) { addSet(X, y, false); return; }
                int d = deg(v);
                if (fS + d - y > 0) { addSet(X, y, true); }
                else { Far[v] = 1; hubCool = 5; }
                return;
            }
            B L = X & rng_[2*id], R = X & rng_[2*id+1];
            if (L.none()) { id = 2*id+1; continue; }
            if (R.none()) { id = 2*id; continue; }
            tests += 2;
            int yl = Q(S | L), yr = Q(S | R);
            int cl = yl + (int)L.count() - fS, cr = yr + (int)R.count() - fS;
            if (cl <= 0) { addSet(L, yl, false); X = R & ~S; id = 2*id+1; continue; }
            if (cr <= 0) { addSet(R, yr, false); X = L & ~S; id = 2*id; continue; }
            double dl = (double)(cl - (int)L.count()), dr = (double)(cr - (int)R.count());
            if (dl > dr || (dl == dr && L.count() <= R.count())) { X = L; id = 2*id; } else { X = R; id = 2*id+1; }
        }
    }
    // ---------- P-descent ----------
    deque<array<int,3>> hist;
    mt19937 rg{777};
    double estP() {
        if (hist.size() < 3) { int m = (ALL & ~S).count(); return m ? min(0.95, PRIORF * fS / m) : -1; }
        double best = 0.5, bl = -1e18;
        for (double lp = -7; lp <= -0.01; lp += 0.05) {
            double p = exp(lp), ll = 0;
            for (auto [t, tp, pos] : hist) { double q = pow(1 - p, t); double qp = tp ? pow(1 - p, tp) : 0;
                ll += pos ? log(max(1e-300, 1 - q)) - log(max(1e-300, 1 - qp)) : log(max(1e-300, q - qp)) - log(max(1e-300, 1 - qp)); }
            if (ll > bl) { bl = ll; best = p; }
        }
        return best;
    }
    // returns 0 negative, 1 positive, 2 added
    unordered_map<int, pair<B,int>> nodeF; // node id -> last known (set, f)
    int lbHits = 0;
    int ptest(const B& T, int parentSize, int nid = 0) {
        int t = T.count(); tests++;
        int y = Q(S | T);
        int c = y + t - fS;
        int res;
        if (c <= 1) { addSet(T, y, false); res = 2; }
        else {
            int g = -1;
            if (USELB && nid && !known(T) && nodeF.count(nid)) {
                auto& [Told, fold] = nodeF[nid];
                B rem = Told & ~T;
                if ((T & ~Told).none()) {
                    bool ok = true; int sd = 0;
                    for (int v = rem._Find_first(); v < 200; v = rem._Find_next(v)) { B b; b[v] = 1; if (!cache.count(b)) { ok = false; break; } sd += cache[b]; }
                    if (ok) {
                        int LB = fold - sd, UB = fold + (int)rem.count();
                        if (fS + LB - y > 0) { g = 1; lbHits++; }
                        else if (fS + UB - y <= 0) { g = 0; lbHits++; }
                    }
                }
            }
            if (g < 0) {
            int q0 = queries; int ft = Q(T); fq += queries - q0;
            if (nid) nodeF[nid] = {T, ft};
            g = fS + ft - y;
            }
            if (g > 0) { if (t == 1) { addSet(T, y, true); res = 2; } else res = 1; }
            else { res = 0; if (FARK) for (int v = T._Find_first(); v < 200; v = T._Find_next(v)) farAt[v] = S.count(); }
        }
        if (USELB && nid && known(T)) nodeF[nid] = {T, Q(T)};
        hist.push_back({t, parentSize, res > 0}); if (hist.size() > 30) hist.pop_front();
        return res;
    }
    bool farRecent(const B& T) {
        if (!SOFTK) return false;
        int sc = S.count();
        for (int v = T._Find_first(); v < 200; v = T._Find_next(v)) if (farAt[v] < 0 || sc - farAt[v] >= SOFTK) return false;
        return true;
    }
    void pStep(B U) {
        finds++;
        if (localProbe()) return;
        U = ALL & ~S;
        if ((int)U.count() == fS || U.none()) return;
        int id = 1; B cur = U;
        bool certain = true; B skipped;
        double p = estP();
        while (true) {
            int c = cur.count();
            if (c == 1) {
                if (certain) { int y = Q(S | cur); addSet(cur, y, true); return; }
                int rr = ptest(cur, 0, 0);
                if (rr == 0 && RECOV) for (int v = skipped._Find_first(); v < 200; v = skipped._Find_next(v)) farAt[v] = -1;
                return;
            }
            if (p > 0) {
                double sstar = log(2.0) / -log(1 - min(p, 0.999));
                if (c > 2.5 * sstar && p * c >= ADTH) {
                    vector<int> cand; vector<int> stk{id};
                    while (!stk.empty()) { int x = stk.back(); stk.pop_back(); B bx = rng_[x] & cur; int cx = bx.count(); if (cx == 0) continue;
                        if (cx <= 1.5 * sstar || cx == 1) cand.push_back(x); else { stk.push_back(2*x); stk.push_back(2*x+1); } }
                    if (cand.size() >= 2) {
                        shuffle(cand.begin(), cand.end(), rg);
                        stable_sort(cand.begin(), cand.end(), [&](int a, int b) { return known(rng_[a] & cur) > known(rng_[b] & cur); });
                        int found = -1;
                        for (size_t i = 0; i < cand.size(); i++) {
                            int x = cand[i]; B bx = rng_[x] & cur;
                            if (i + 1 == cand.size()) { found = x; break; }
                            int res = ptest(bx, 0, x);
                            if (res == 2) return;
                            if (res == 1) { found = x; break; }
                        }
                        id = found; cur = rng_[id] & cur; continue;
                    }
                }
            }
            B L = cur & rng_[2*id], R = cur & rng_[2*id+1];
            if (L.none()) { id = 2*id+1; continue; }
            if (R.none()) { id = 2*id; continue; }
            bool fl = farRecent(L), fr = farRecent(R);
            if (fl != fr) { certain = false; if (fl) { skipped |= L; id = 2*id+1; cur = R; } else { skipped |= R; id = 2*id; cur = L; } continue; }
            bool testL = true;
            if (!known(L) && known(R)) testL = false;
            int res = ptest(testL ? L : R, c, testL ? 2*id : 2*id+1);
            if (res == 2) return;
            bool goL = (res == 1) == testL;
            if (goL) { id = 2*id; cur = L; } else { id = 2*id+1; cur = R; }
        }
    }
};

int main() {
    int n;
    if (!(cin >> n)) return 0;
    Solver s; s.n = n;
    s.ask = [&](const B& S) -> int {
        string q(n, '0');
        for (int i = 0; i < n; i++) if (S[i]) q[i] = '1';
        cout << "? " << q << '\n' << flush;
        long long r;
        if (!(cin >> r) || r < 0) exit(0);
        return (int)r;
    };
    int a = s.run();
    cout << "! " << a << '\n' << flush;
    return 0;
}

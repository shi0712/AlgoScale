#include <bits/stdc++.h>
using namespace std;

typedef bitset<128> BS;

#ifndef GAMBLE_TH
#define GAMBLE_TH 0.1
#endif
#ifndef CSP_MAXK
#define CSP_MAXK 12
#endif
#ifndef SD_MAX
#define SD_MAX 800
#endif
#ifndef KMAX
#define KMAX 10
#endif
#ifndef PERM_TH
#define PERM_TH 0.3
#endif
#ifndef TIME_BUDGET
#define TIME_BUDGET 2.0
#endif
#ifndef SETTLE_K
#define SETTLE_K 5
#endif
#ifndef REV_MARGIN
#define REV_MARGIN 0.3
#endif

double g_earlyP = 0, g_lateP = 0;
long long g_early = 0, g_late = 0, g_cspAbort = 0;

struct Solver {
    int n;
    function<int(int)> ask;
    int tq = 0;
    vector<int> obsW, obsB;
    vector<vector<int>> mt;
    vector<int> org, st;  // st: 0 fresh, 1 amb, 2 known
    vector<BS> cand;
    vector<vector<unsigned char>> occK;
    int F;
    int T = -1;
    int cw = -1;
    bool dead = false;
    BS O0;
    mt19937 rng{12345};

    int md(int x) { x %= n; if (x < 0) x += n; return x; }
    int steps(int d, int t) { return int(upper_bound(mt[d].begin(), mt[d].end(), t) - mt[d].begin()); }
    int curPos(int d, int o) { return md(o + (int)mt[d].size()); }

    void init(int n_) {
        n = n_;
        mt.assign(n, {});
        org.assign(n, -1);
        st.assign(n, 0);
        cand.assign(n, BS());
        org[0] = 0; st[0] = 2;
        F = n - 1;
        obsW.push_back(-1); obsB.push_back(-1);
        occK.push_back(vector<unsigned char>(n, 0));
        occK[0][0] = 1;
    }

    bool doMove(int d) {
        int r = ask(d);
        if (r < 0) { dead = true; return false; }
        tq++;
        mt[d].push_back(tq);
        obsW.push_back(d); obsB.push_back(r);
        vector<unsigned char> row = occK.back();
        if (st[d] == 2) {
            int k = mt[d].size();
            row[md(org[d] + k - 1)]--;
            row[md(org[d] + k)]++;
        }
        occK.push_back(row);
        if (st[d] == 0) {
            st[d] = 1; F--;
            cand[d].reset();
            for (int i = 0; i < n; i++) cand[d][i] = 1;
        }
        return true;
    }

    void makeKnown(int d, int o) {
        st[d] = 2; org[d] = o;
        for (int t = 0; t <= tq; t++) occK[t][md(o + steps(d, t))]++;
    }

    // ---------- located info ----------
    vector<vector<int>> zeroAt;
    BS Z, Pcert;
    struct Req { int t, v; };
    vector<Req> reqs;
    vector<vector<Req>> sole;
    vector<int> A;

    bool possiblyAt(int a, int t, int v) { return cand[a][md(v - steps(a, t))]; }

    void buildLocated() {
        zeroAt.assign(n, {});
        Z.reset(); Pcert.reset();
        reqs.clear();
        A.clear();
        for (int d = 0; d < n; d++) if (st[d] == 1) A.push_back(d);
        for (int t = 1; t <= tq; t++) {
            int w = obsW[t];
            if (st[w] != 2) continue;
            int v = md(org[w] + steps(w, t));
            if (obsB[t] == 0) { zeroAt[v].push_back(t); Z[v] = 1; }
            else if (occK[t][v] - 1 == 0) reqs.push_back({t, v});
        }
        sole.assign(n, {});
        for (auto &r : reqs) {
            int cnt = 0, who = -1;
            for (int a : A) if (possiblyAt(a, r.t, r.v)) { cnt++; who = a; if (cnt > 1) break; }
            if (cnt == 0) Pcert[r.v] = 1;
            else if (cnt == 1) sole[who].push_back(r);
        }
    }

    bool unaryOK(int a, int o) {
        int len = mt[a].size();
        for (int k = 0; k <= len; k++) {
            int l = (k == 0 ? 0 : mt[a][k - 1]);
            int r = (k == len ? tq : mt[a][k] - 1);
            int p = md(o + k);
            auto &zl = zeroAt[p];
            auto it = lower_bound(zl.begin(), zl.end(), l);
            if (it != zl.end() && *it <= r) return false;
        }
        BS Zh = Z, Ph = Pcert;
        for (int k = 1; k <= len; k++) {
            int t = mt[a][k - 1], v = md(o + k), b = obsB[t];
            int ok = occK[t][v];
            if (b == 0) {
                if (ok > 0) return false;
                if (Pcert[v]) return false;
                Zh[v] = 1;
            } else {
                if (ok > 0) continue;
                bool other = false;
                for (int c : A) if (c != a && possiblyAt(c, t, v)) { other = true; break; }
                if (other) continue;
                if (Z[v]) return false;
                Ph[v] = 1;
            }
        }
        for (auto &r : sole[a]) {
            if (md(r.v - steps(a, r.t)) != o) Ph[r.v] = 1;
        }
        if ((Ph & Zh).any()) return false;
        int cp = Ph.count();
        if (cp > F) return false;
        if (F > cp && (int)Zh.count() == n) return false;
        return true;
    }

    // ---------- joint propagation (singleton consistency) ----------
    struct Clause { int L; vector<pair<int, int>> lits; };
    int K;
    vector<BS> D;
    vector<vector<vector<pair<int, int>>>> nog;  // [i][o]
    vector<vector<BS>> zm;                       // [i][o]
    vector<vector<vector<int>>> condC;           // [i][o] -> clause ids
    vector<Clause> clauses;
    vector<char> isCond;

    // probe state
    vector<BS> pd;
    vector<int> forced;
    BS Zeff, Pset;
    vector<int> activeC;
    vector<int> fq;

    bool pForce(int j, int v) {
        if (forced[j] == v) return true;
        if (forced[j] >= 0 || !pd[j][v]) return false;
        pd[j].reset(); pd[j][v] = 1; forced[j] = v;
        Zeff |= zm[j][v];
        if ((Zeff & Pset).any()) return false;
        for (int c : condC[j][v]) activeC.push_back(c);
        for (auto &p : nog[j][v]) {
            int k = p.first, w = p.second;
            if (!pd[k][w]) continue;
            if (forced[k] == w) return false;
            pd[k][w] = 0;
            int cnt = pd[k].count();
            if (cnt == 0) return false;
            if (cnt == 1 && forced[k] < 0) fq.push_back(k);
        }
        return true;
    }
    bool pDrain() {
        while (!fq.empty()) {
            int k = fq.back(); fq.pop_back();
            if (forced[k] >= 0) continue;
            if (pd[k].count() != 1) { if (pd[k].none()) return false; continue; }
            if (!pForce(k, pd[k]._Find_first())) return false;
        }
        return true;
    }
    bool probe(int i, int o) {
        pd = D;
        forced.assign(K, -1);
        Zeff = Z; Pset = Pcert;
        activeC.clear(); fq.clear();
        for (int c = 0; c < (int)clauses.size(); c++) if (!isCond[c]) activeC.push_back(c);
        if (!pForce(i, o)) return false;
        for (int k = 0; k < K; k++) if (forced[k] < 0 && pd[k].count() == 1) fq.push_back(k);
        if (!pDrain()) return false;
        bool changed = true;
        while (changed) {
            changed = false;
            int pc = Pset.count();
            if (pc > F) return false;
            for (size_t ci = 0; ci < activeC.size(); ci++) {
                Clause &c = clauses[activeC[ci]];
                if (Pset[c.L]) continue;
                bool sat = false;
                int alive = 0, lk = -1, lw = -1;
                for (auto &p : c.lits) {
                    if (forced[p.first] == p.second) { sat = true; break; }
                    if (pd[p.first][p.second]) { alive++; lk = p.first; lw = p.second; }
                }
                if (sat) continue;
                bool freshOK = !Zeff[c.L] && pc < F;
                if (alive == 0) {
                    if (!freshOK) return false;
                    Pset[c.L] = 1; pc++; changed = true;
                } else if (alive == 1 && !freshOK) {
                    if (!pForce(lk, lw)) return false;
                    if (!pDrain()) return false;
                    changed = true;
                }
            }
            if ((Zeff & Pset).any()) return false;
            if (F > (int)Pset.count() && (int)Zeff.count() == n) return false;
        }
        return true;
    }

    bool runCSP() {
        K = A.size();
        if (K < 1) return false;
        D.assign(K, BS());
        for (int i = 0; i < K; i++) D[i] = cand[A[i]];
        auto varAt = [&](int j, int t, int L) -> int {
            int o = md(L - steps(A[j], t));
            return D[j][o] ? o : -1;
        };
        nog.assign(K, vector<vector<pair<int, int>>>(n));
        zm.assign(K, vector<BS>(n));
        condC.assign(K, vector<vector<int>>(n));
        clauses.clear(); isCond.clear();
        for (int i = 0; i < K; i++) {
            int a = A[i], len = mt[a].size();
            for (int o = 0; o < n; o++) if (D[i][o]) {
                for (int k = 1; k <= len; k++) {
                    int t = mt[a][k - 1], L = md(o + k), b = obsB[t];
                    if (occK[t][L] > 0) continue;
                    if (b == 0) {
                        zm[i][o][L] = 1;
                        for (int j = 0; j < K; j++) if (j != i) {
                            int oj = varAt(j, t, L);
                            if (oj >= 0) { nog[i][o].push_back({j, oj}); nog[j][oj].push_back({i, o}); }
                        }
                    } else {
                        Clause c; c.L = L;
                        for (int j = 0; j < K; j++) if (j != i) {
                            int oj = varAt(j, t, L);
                            if (oj >= 0) c.lits.push_back({j, oj});
                        }
                        condC[i][o].push_back(clauses.size());
                        clauses.push_back(c); isCond.push_back(1);
                    }
                }
            }
        }
        {
            set<pair<int, vector<pair<int, int>>>> seen;
            for (auto &r : reqs) {
                vector<pair<int, int>> cv;
                for (int j = 0; j < K; j++) {
                    int oj = varAt(j, r.t, r.v);
                    if (oj >= 0) cv.push_back({j, oj});
                }
                if (cv.empty()) continue;
                if (seen.insert({r.v, cv}).second) { clauses.push_back({r.v, cv}); isCond.push_back(0); }
            }
        }
        bool changed = false;
        for (int i = 0; i < K; i++) {
            for (int o = 0; o < n; o++) if (D[i][o]) {
                if (!probe(i, o)) {
                    D[i][o] = 0; cand[A[i]][o] = 0; changed = true;
                }
            }
        }
        return changed;
    }

    chrono::steady_clock::time_point t0 = chrono::steady_clock::now();
    double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - t0).count(); }
    bool K_ok() {
        int k = 0, sd = 0; for (int d = 0; d < n; d++) if (st[d] == 1) { k++; sd += cand[d].count(); }
        if (k == 0) return false;
        double e = elapsed();
        if (e > TIME_BUDGET) return false;
        if (sd > SD_MAX) return false;
        return true;
    }
    // info potential: can the next reply of d distinguish hypotheses?
    bool infoPot(int d) {
        int c1 = 0, c0 = 0, cu = 0;
        int len = mt[d].size();
        for (int o = 0; o < n; o++) if (cand[d][o]) {
            int v = md(o + len + 1);
            bool knownThere = occK[tq][v] > 0;
            bool ambThere = false;
            for (int a : A) if (a != d && possiblyAt(a, tq, v)) { ambThere = true; break; }
            if (knownThere || Pcert[v]) c1++;
            else if (Z[v] && !ambThere) c0++;
            else cu++;
        }
        if (cu > 0) return true;
        return c1 > 0 && c0 > 0;
    }
    void infer(bool full) {
        while (true) {
            buildLocated();
            bool changed = false;
            for (int a : A)
                for (int o = 0; o < n; o++) if (cand[a][o])
                    if (!unaryOK(a, o)) { cand[a][o] = 0; changed = true; }
            if (!changed && full && K_ok()) changed = runCSP();
            for (int a : A)
                if (cand[a].count() == 1) { makeKnown(a, cand[a]._Find_first()); changed = true; }
            if (!changed) break;
        }
    }

    BS posSet(int d) {
        BS s;
        if (st[d] == 2) { s[curPos(d, org[d])] = 1; return s; }
        if (st[d] == 1) { for (int o = 0; o < n; o++) if (cand[d][o]) s[curPos(d, o)] = 1; return s; }
        for (int v = 0; v < n; v++) if (!Z[v]) s[v] = 1;
        return s;
    }

    // ---------- probability ----------
    double pois[64];
    double tailP(int m) { double s = 0; for (int i = m; i < 40; i++) s += pois[i]; return s; }
    double tailE(int m) { double s = 0; for (int i = m; i < 40; i++) s += pois[i] * i; return s; }
    vector<double> qv, ev;
    void computeQ() {
        vector<int> k(n, 0);
        for (int d = 1; d < n; d++) if (st[d] == 2) k[org[d]]++;
        qv.assign(n, 0); ev.assign(n, 0);
        for (int v = 0; v < n; v++) {
            if (Z[v] || !O0[v]) continue;
            int m = max(k[v], 1);
            double den = tailP(m);
            double q = tailP(k[v] + 1) / den;
            double e = (tailE(m) - k[v] * den) / den;
            if (Pcert[v]) { q = 1; if (k[v] > 0) e = e / max(1e-9, tailP(k[v] + 1) / den); }
            qv[v] = q; ev[v] = e;
        }
    }
    double pAtT(int d) {
        double wT = 0, wAll = 0;
        int len = mt[d].size();
        for (int o = 0; o < n; o++) if (cand[d][o]) {
            double w = max(ev[o], 1e-3);
            for (int k = 1; k <= len; k++) {
                int t = mt[d][k - 1], v = md(o + k), b = obsB[t];
                if (occK[t][v] > 0) continue;
                double q = min(max(qv[v], 0.02), 0.98);
                w *= b ? q : 1 - q;
            }
            wAll += w;
            if (curPos(d, o) == T) wT += w;
        }
        return wAll > 0 ? wT / wAll : 0;
    }

    void dumpP(int d) {
        int len = mt[d].size();
        fprintf(stderr, "dump d%d len=%d T=%d:", d, len, T);
        for (int o = 0; o < n; o++) if (cand[d][o]) {
            double w = max(ev[o], 1e-3);
            for (int k = 1; k <= len; k++) {
                int t = mt[d][k - 1], v = md(o + k), b = obsB[t];
                if (occK[t][v] > 0) continue;
                double q = min(max(qv[v], 0.02), 0.98);
                w *= b ? q : 1 - q;
            }
            fprintf(stderr, " o%d->%d:%.3g", o, curPos(d,o), w);
        }
        fprintf(stderr, "\n");
    }
    bool allDone() {
        for (int d = 0; d < n; d++) {
            BS s = posSet(d);
            if (!(s.count() == 1 && s[T])) return false;
        }
        return true;
    }

    bool settled = false, isPerm = false;
    int holdDial = -1, holdTarget = -1, holdUntil = 0;
    int nAmb() { int k = 0; for (int d = 0; d < n; d++) if (st[d] == 1) k++; return k; }
    int chooseSafe() {
        if (holdDial >= 0) {
            if (tq > holdUntil || st[holdDial] != 2) holdDial = -1;
            else if (curPos(holdDial, org[holdDial]) != holdTarget) return holdDial;
        }
        auto safe = [&](int d) {
            if (st[d] == 0) return false;
            if (d == holdDial) return false;
            if (st[d] == 2 && !settled) {
                int p = curPos(d, org[d]);
                if (p == T || Z[md(p - 1)]) return false;
                if (d == 0 && occK[tq][T] == 0) return false;
            }
            BS s = posSet(d); return !s[T]; };
        if (cw >= 0 && safe(cw)) return cw;
        for (int d = 0; d < n; d++) if (safe(d)) return d;
        return -1;
    }

    // ---------- target revision ----------
    double estRemaining(int Tp, const vector<double>& ef) {
        double c = 0;
        for (int d = 0; d < n; d++) {
            if (st[d] == 2) c += md(Tp - curPos(d, org[d]));
            else if (st[d] == 1) {
                double s = 0; int k = 0;
                for (int o = 0; o < n; o++) if (cand[d][o]) { s += md(Tp - curPos(d, o)); k++; }
                c += s / max(1, k);
            }
        }
        for (int v = 0; v < n; v++) if (ef[v] > 0) {
            int dist = md(Tp - v);
            if (dist == 0) c += n * ef[v] * (F - ef[v]) / (F - ef[v] + 1);
            else c += ef[v] * dist;
        }
        return c;
    }
    vector<double> estFresh() {
        vector<double> ef(n, 0);
        if (F == 0) return ef;
        vector<int> sv(n, 0);
        int S = 0;
        for (int d = 1; d < n; d++) if (st[d] == 2) { sv[org[d]]++; S++; }
        double alpha = 1.0;
        int m = 0;
        for (int v = 0; v < n; v++) if (O0[v]) m++;
        double tot = 0;
        for (int v = 0; v < n; v++) {
            if (!O0[v] || Z[v]) continue;
            double et = (n - 1) * (sv[v] + alpha) / (S + alpha * m);
            double r = max(et - sv[v], Pcert[v] ? 1.0 : 0.05);
            ef[v] = r; tot += r;
        }
        if (tot > 0) for (auto &x : ef) x *= F / tot;
        return ef;
    }
    void reviseT() {
        if (isPerm) return;
        vector<double> ef = estFresh();
        double cur = estRemaining(T, ef);
        double best = cur; int bt = T;
        for (int c = 0; c < n; c++) {
            double v = estRemaining(c, ef);
            if (v < best - 1e-9) { best = v; bt = c; }
        }
        if (bt != T && best < cur - (settled ? REV_MARGIN * n : 0.0)) T = bt;
    }

    void run() {
        for (int i = 0; i < n; i++) if (!doMove(0)) return;
        for (int i = 0; i < 64; i++) pois[i] = (i == 0 ? exp(-1.0) : pois[i - 1] / i);
        for (int t = 1; t <= n; t++) if (obsB[t]) O0[t % n] = 1;
        double e = (double)(n - 1) / max<int>(1, O0.count());
        double best = 1e18;
        for (int c = 0; c < n; c++) {
            if (O0[c]) continue;
            double cost = c;
            for (int x = 0; x < n; x++) if (O0[x]) cost += e * md(c - x);
            if (cost < best) { best = cost; T = c; }
        }
        isPerm = ((int)O0.count() == n - 1 && !O0[0]);
        if (isPerm) { T = 1 % n; settled = true; }
        vector<int> freshOrder;
        for (int d = 1; d < n; d++) freshOrder.push_back(d);
        shuffle(freshOrder.begin(), freshOrder.end(), rng);
        size_t fptr = 0;
        vector<int> lastCnt(n, 1000), sinceProg(n, 0);
        if (O0[0] && T != 0) { if (!doMove(0)) return; }
        int knownCnt = 1, picks = 0;
        if (SETTLE_K == 0) settled = true;
        if (isPerm) settled = true;
        while (true) {
            infer(false);
            {
                int kc = 0; for (int d = 0; d < n; d++) if (st[d] == 2) kc++;
                if (kc != knownCnt) { knownCnt = kc; reviseT(); }
            }
            for (int d = 0; d < n; d++) if (st[d] == 1) {
                int c = cand[d].count();
                if (c < lastCnt[d]) { lastCnt[d] = c; sinceProg[d] = 0; }
            }
            BS freshOther;
            if (F > 0) { for (int v = 0; v < n; v++) if (!Z[v] && v != T) freshOther[v] = 1; }
            bool canPick = freshOther.any();
            { int k = 0; for (int d = 0; d < n; d++) if (st[d] == 1) k++; if (k >= KMAX) canPick = false; }
            int mv = chooseSafe();
            if (mv < 0 && !settled) {
                // if nothing to do in unsettled mode, settle
                bool anyAmb = false;
                for (int d = 0; d < n; d++) if (st[d] == 1) anyAmb = true;
                if (!canPick && !anyAmb) { settled = true; reviseT(); mv = chooseSafe(); }
            }
            if (mv < 0) {
                infer(true);
                if (allDone()) return;
                mv = chooseSafe();
            }
            if (mv < 0) {
                computeQ();
                double mp = 2; int bd = -1;
                double mp2 = 2; int bd2 = -1;
                for (int d = 1; d < n; d++) if (st[d] == 1) {
                    double p = pAtT(d);
                    if (sinceProg[d] < n && p < mp && (isPerm || infoPot(d))) { mp = p; bd = d; }
                    double p2 = p + (sinceProg[d] >= n ? 1.0 : 0.0);
                    if (p2 < mp2) { mp2 = p2; bd2 = d; }
                }
                if (bd >= 0 && mp < (isPerm ? PERM_TH : GAMBLE_TH)) { mv = bd; g_early++; g_earlyP += mp; if (getenv("DBG2")) { int k=0; for (int d=0;d<n;d++) if(st[d]==1) k++; fprintf(stderr,"EARLY tq=%d F=%d K=%d d=%d p=%.3f cands=%d el=%.2f\n",tq,F,k,bd,mp,(int)cand[bd].count(),elapsed()); } }
                else if (canPick) {
                    if (getenv("DBG2")) { fprintf(stderr, "PICK tq=%d F=%d:", tq, F); for (int d=1; d<n; d++) if (st[d]==1) fprintf(stderr, " [d%d p=%.3f c=%d ip=%d sp=%d]", d, pAtT(d), (int)cand[d].count(), (int)infoPot(d), sinceProg[d]); fprintf(stderr, "\n"); for (int d=1; d<n; d++) if (st[d]==1) dumpP(d); }
                    while (st[freshOrder[fptr]] != 0) fptr++;
                    mv = freshOrder[fptr];
                    if (++picks > SETTLE_K) settled = true;
                } else if (bd2 >= 0 && mp2 >= 1.0 && F > 0 && freshOther.any() && nAmb() < KMAX) {
                    while (st[freshOrder[fptr]] != 0) fptr++;
                    mv = freshOrder[fptr];
                } else if (bd2 >= 0 && mp2 >= 1.0) {
                    // nobody makes progress: symmetry breaker - park a known dial on a known-empty point
                    int z = -1;
                    for (int d = 0; d < n; d++) if (st[d] == 2 && curPos(d, org[d]) == T) { z = d; break; }
                    int hp = -1;
                    if (z >= 0) {
                        vector<int> emp;
                        for (int k = 1; k < n; k++) {
                            int p = md(T + k);
                            if (!Z[p] || occK[tq][p] > 0) continue;
                            bool amb = false;
                            for (int a : A) if (possiblyAt(a, tq, p)) { amb = true; break; }
                            if (!amb) emp.push_back(p);
                        }
                        if (!emp.empty()) hp = emp[rng() % emp.size()];
                    }
                    if (z >= 0 && hp >= 0) { holdDial = z; holdTarget = hp; holdUntil = tq + 3 * n; mv = z; if (getenv("DBG2")) fprintf(stderr, "BREAKER tq=%d z=%d hp=%d T=%d\n", tq, z, hp, T); }
                    else if (getenv("DBG2")) fprintf(stderr, "BREAKER-FAIL tq=%d z=%d\n", tq, z);
                    else mv = bd2;
                    g_late++;
                } else if (bd2 >= 0) {
                    mv = bd2; g_late++; g_lateP += mp2; if (getenv("DBG2")) { int k=0, sd=0; for (int d=0;d<n;d++) if(st[d]==1) { k++; sd += cand[d].count(); } fprintf(stderr,"sd=%d ", sd); fprintf(stderr,"LATE tq=%d F=%d K=%d d=%d p=%.3f cands=%d el=%.2f\n",tq,F,k,bd2,mp2,(int)cand[bd2].count(),elapsed()); }
                }
            }
            if (mv < 0) {
                if (!settled) { settled = true; continue; }
                infer(true);
                if (allDone()) return;
                // fallback: any dial not certainly at T
                for (int d = 0; d < n && mv < 0; d++) if (st[d] == 2 && curPos(d, org[d]) != T) mv = d;
                if (mv < 0) {
                    computeQ();
                    double best = 2;
                    for (int d = 0; d < n; d++) if (st[d] == 1) { double p = pAtT(d); if (p < best) { best = p; mv = d; } }
                }
                if (mv < 0) for (int d = 0; d < n && mv < 0; d++) if (st[d] == 0) mv = d;
                if (mv < 0) { holdDial = -1; for (int d = 0; d < n && mv < 0; d++) if (st[d] == 2) mv = d; }
            }
            cw = mv;
            if (st[mv] == 1) sinceProg[mv]++;
            if (!doMove(mv)) return;
        }
    }
};

#ifndef SIM
int main() {
    int n;
    if (!(cin >> n)) return 0;
    Solver s;
    s.init(n);
    s.ask = [&](int d) {
        printf("? %d\n", d + 1);
        fflush(stdout);
        int r;
        if (scanf("%d", &r) != 1) return -1;
        return r;
    };
    s.run();
    if (!s.dead) { printf("!\n"); fflush(stdout); }
    return 0;
}
#endif

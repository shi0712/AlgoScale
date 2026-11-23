#include <bits/stdc++.h>
#ifndef KN
#define KN 2
#endif
#ifndef WEQ
#define WEQ 0
#endif
#ifndef SGN
#define SGN -1
#endif
#ifndef BON
#define BON 0.0
#endif
#ifndef OPT
#define OPT 1
#endif
#ifndef OPTR
#define OPTR 5
#endif
#ifndef MAXWIN
#define MAXWIN 64
#endif
#ifndef TB
#define TB 1.0
#endif
#ifndef FZ
#define FZ 20
#endif
#ifndef TRT
#define TRT 2.0
#endif
#ifndef HLIM
#define HLIM 32
#endif
#ifndef WS
#define WS 2.0
#endif
#ifndef SM
#define SM 4
#endif
#ifndef WA
#define WA 0.0
#endif
#ifndef LG
#define LG 3.0
#endif
static inline double WT(double a) { return (a <= SM ? WS : 1.0) + WA * std::max(0.0, LG - log2(a)); }
#ifndef EXN
#define EXN 7
#endif
#ifndef ITERS
#define ITERS 2
#endif
using namespace std;
typedef unsigned long long u64;

static int N, W;
static long long queryCount = 0, exclCnt = 0;

#ifdef LOCAL
static vector<int> J_door, J_val; // per switch
static int judgeQuery(const vector<u64>& x) {
    // first closed door
    vector<int> closedDoor(N, 0);
    int best = N;
    for (int s = 0; s < N; s++) {
        int b = (x[s >> 6] >> (s & 63)) & 1;
        if (b != J_val[s]) best = min(best, J_door[s]);
    }
    return best == N ? -1 : best;
}
#endif

static int ask(const vector<u64>& x) {
    queryCount++;
#ifdef LOCAL
    return judgeQuery(x);
#else
    string s(N, '0');
    for (int i = 0; i < N; i++)
        if ((x[i >> 6] >> (i & 63)) & 1) s[i] = '1';
    printf("? %s\n", s.c_str());
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < -1 || r >= N) exit(0);
    return r;
#endif
}


static void finishAnswer(const vector<int>& d, const vector<int>& val) {
#ifdef LOCAL
    for (int t = 0; t < N; t++)
        if (d[t] != J_door[t] || val[t] != J_val[t]) { fprintf(stderr, "WRONG\n"); exit(1); }
#else
    string s(N, '0');
    for (int t = 0; t < N; t++) s[t] = '0' + val[t];
    printf("! %s", s.c_str());
    for (int t = 0; t < N; t++) printf(" %d", d[t]);
    printf("\n");
    fflush(stdout);
#endif
}

// exhaustive entropy-greedy for small N
static void exactSolve() {
    struct Hy { unsigned char door[8]; unsigned char c; };
    vector<Hy> hy;
    vector<int> perm(N);
    iota(perm.begin(), perm.end(), 0);
    do {
        for (int c = 0; c < (1 << N); c++) {
            Hy h;
            for (int t = 0; t < N; t++) h.door[t] = perm[t];
            h.c = c;
            hy.push_back(h);
        }
    } while (next_permutation(perm.begin(), perm.end()));
    W = 1;
    auto resp = [&](const Hy& h, int x) {
        int mm = (x ^ h.c) & ((1 << N) - 1);
        int r = N;
        while (mm) { int b = __builtin_ctz(mm); mm &= mm - 1; if (h.door[b] < r) r = h.door[b]; }
        return r;
    };
    bool first = true;
    while (hy.size() > 1) {
        int bestx = 0;
        if (!first) {
            double bestE = -1; size_t bestMax = SIZE_MAX;
            for (int x = 0; x < (1 << N); x++) {
                vector<size_t> cnt(N + 1, 0);
                for (auto& h : hy) cnt[resp(h, x)]++;
                double e = 0; size_t mx = 0;
                for (auto c : cnt) if (c) { double q = (double)c / hy.size(); e -= q * log2(q); mx = max(mx, c); }
                if (e > bestE + 1e-12 || (e > bestE - 1e-12 && mx < bestMax)) { bestE = e; bestMax = mx; bestx = x; }
            }
        }
        first = false;
#ifdef LOCAL
        fprintf(stderr, "hy=%zu x=%d\n", hy.size(), bestx);
#endif
        vector<u64> xv(1, (u64)bestx);
        int r = ask(xv);
        if (r == -1) r = N;
        vector<Hy> nh;
        for (auto& h : hy) if (resp(h, bestx) == r) nh.push_back(h);
        hy.swap(nh);
        if (hy.empty()) exit(0);
    }
    vector<int> d(N), val(N);
    for (int t = 0; t < N; t++) { d[t] = hy[0].door[t]; val[t] = (hy[0].c >> t) & 1; }
    finishAnswer(d, val);
}

struct Solver {
    vector<vector<u64>> A, V;
    vector<int> Acnt;
    vector<char> touched, det;
    vector<int> dsw, dval, owner;
    vector<u64> U; // unknown switches
    mt19937_64 rng{12345};
    vector<int>* detListPtr;

    static inline int pc(u64 a) { return __builtin_popcountll(a); }
    static inline double H(double p) {
        if (p <= 0 || p >= 1) return 0;
        return -(p * log2(p) + (1 - p) * log2(1 - p));
    }

    void determine(int j, int s, vector<int>& stk) {
        det[j] = 1;
        dsw[j] = s;
        dval[j] = (V[j][s >> 6] >> (s & 63)) & 1;
        owner[s] = j;
        detListPtr->push_back(s);
        U[s >> 6] &= ~(1ULL << (s & 63));
        for (int l = 0; l < N; l++) {
            if (l == j || det[l] || !touched[l]) continue;
            if ((A[l][s >> 6] >> (s & 63)) & 1) {
                A[l][s >> 6] &= ~(1ULL << (s & 63));
                Acnt[l]--;
                if (Acnt[l] == 1) { stk.push_back(l); exclCnt++; }
            }
        }
    }
    void checkDet(vector<int> stk) {
        while (!stk.empty()) {
            int j = stk.back();
            stk.pop_back();
            if (det[j] || Acnt[j] != 1) continue;
            int s = -1;
            for (int w = 0; w < W; w++)
                if (A[j][w]) { s = w * 64 + __builtin_ctzll(A[j][w]); break; }
            determine(j, s, stk);
        }
    }


    vector<u64> reserved;
    vector<int> msw; // matched switch for small doors (-1)
    // Hall-type pruning on doors with small candidate sets
    void hallPrune() {
        while (true) {
            vector<int> sd;
            for (int j = 0; j < N; j++)
                if (touched[j] && !det[j] && Acnt[j] <= HLIM) sd.push_back(j);
            int nd = sd.size();
            if (nd == 0) { fill(reserved.begin(), reserved.end(), 0); return; }
            // local switch ids
            static vector<int> lid;
            lid.assign(N, -1);
            vector<int> sws;
            vector<vector<int>> adj(nd);
            for (int a = 0; a < nd; a++) {
                int j = sd[a];
                for (int w = 0; w < W; w++) {
                    u64 m = A[j][w];
                    while (m) {
                        int b = __builtin_ctzll(m); m &= m - 1;
                        int sidx = w * 64 + b;
                        if (lid[sidx] < 0) { lid[sidx] = sws.size(); sws.push_back(sidx); }
                        adj[a].push_back(lid[sidx]);
                    }
                }
            }
            int ns = sws.size();
            vector<vector<int>> radj(ns);
            for (int a = 0; a < nd; a++) for (int t : adj[a]) radj[t].push_back(a);
            vector<int> md(nd, -1), ms(ns, -1);
            for (int a = 0; a < nd; a++) {
                int j = sd[a];
                if (msw[j] >= 0 && lid[msw[j]] >= 0 && ((A[j][msw[j] >> 6] >> (msw[j] & 63)) & 1) && ms[lid[msw[j]]] < 0) {
                    md[a] = lid[msw[j]]; ms[md[a]] = a;
                }
            }
            // augment (Kuhn with visited stamps)
            vector<int> vis(ns, -1);
            int stamp = 0;
            function<bool(int)> aug = [&](int a) -> bool {
                for (int t : adj[a]) if (ms[t] < 0) { ms[t] = a; md[a] = t; return true; }
                for (int t : adj[a]) {
                    if (vis[t] == stamp) continue;
                    vis[t] = stamp;
                    if (aug(ms[t])) { ms[t] = a; md[a] = t; return true; }
                }
                return false;
            };
            for (int a = 0; a < nd; a++) if (md[a] < 0) { stamp++; aug(a); }
            for (int a = 0; a < nd; a++) msw[sd[a]] = md[a] >= 0 ? sws[md[a]] : -1;
            // good switches: can be unmatched
            vector<char> good(ns, 0);
            vector<int> q;
            for (int t = 0; t < ns; t++) if (ms[t] < 0) { good[t] = 1; q.push_back(t); }
            for (size_t h = 0; h < q.size(); h++) {
                int g = q[h];
                for (int a : radj[g]) {
                    int m = md[a];
                    if (m >= 0 && m != g && !good[m]) { good[m] = 1; q.push_back(m); }
                }
            }
            // SCC on non-good part: nodes doors [0,nd), switches [nd, nd+ns)
            int nn = nd + ns;
            vector<int> idx(nn, -1), low(nn), comp(nn, -1), st;
            vector<char> on(nn, 0);
            int cidx = 0, ccnt = 0;
            auto succ = [&](int v, vector<int>& out) {
                out.clear();
                if (v < nd) { for (int t : adj[v]) if (t != md[v] && !good[t]) out.push_back(nd + t); }
                else { int t = v - nd; if (ms[t] >= 0) out.push_back(ms[t]); }
            };
            // iterative tarjan
            vector<vector<int>> succs(nn);
            for (int v = 0; v < nn; v++) succ(v, succs[v]);
            vector<pair<int,int>> cs;
            for (int r0 = 0; r0 < nn; r0++) {
                if (idx[r0] >= 0) continue;
                if (r0 >= nd && good[r0 - nd]) continue;
                cs.push_back({r0, 0});
                idx[r0] = low[r0] = cidx++; st.push_back(r0); on[r0] = 1;
                while (!cs.empty()) {
                    int v = cs.back().first; int& it = cs.back().second;
                    if (it < (int)succs[v].size()) {
                        int u = succs[v][it++];
                        if (idx[u] < 0) { idx[u] = low[u] = cidx++; st.push_back(u); on[u] = 1; cs.push_back({u, 0}); }
                        else if (on[u]) low[v] = min(low[v], idx[u]);
                    } else {
                        if (low[v] == idx[v]) {
                            while (true) { int u = st.back(); st.pop_back(); on[u] = 0; comp[u] = ccnt; if (u == v) break; }
                            ccnt++;
                        }
                        cs.pop_back();
                        if (!cs.empty()) { int pv = cs.back().first; low[pv] = min(low[pv], low[v]); }
                    }
                }
            }
            bool changed = false;
            vector<int> stk;
            for (int a = 0; a < nd; a++) {
                int j = sd[a];
                for (int t : adj[a]) {
                    if (good[t] || t == md[a]) continue;
                    if (comp[a] == comp[nd + t]) continue;
                    int sidx = sws[t];
                    A[j][sidx >> 6] &= ~(1ULL << (sidx & 63));
                    Acnt[j]--;
                    changed = true;
                }
                if (Acnt[j] == 1) stk.push_back(j);
            }
            fill(reserved.begin(), reserved.end(), 0);
            for (int t = 0; t < ns; t++) if (!good[t]) reserved[sws[t] >> 6] |= 1ULL << (sws[t] & 63);
            // remove reserved from non-small touched doors
            for (int j = 0; j < N; j++) {
                if (!touched[j] || det[j] || Acnt[j] <= HLIM) continue;
                int c = 0; bool ch = false;
                for (int w = 0; w < W; w++) {
                    if (A[j][w] & reserved[w]) { A[j][w] &= ~reserved[w]; ch = true; }
                    c += pc(A[j][w]);
                }
                if (ch) { Acnt[j] = c; changed = true; if (c == 1) stk.push_back(j); }
            }
            if (!stk.empty()) checkDet(stk);
            if (!changed) return;
        }
    }

    void run() {
        W = (N + 63) / 64;
        A.assign(N, vector<u64>());
        V.assign(N, vector<u64>());
        Acnt.assign(N, 0);
        touched.assign(N, 0);
        det.assign(N, 0);
        dsw.assign(N, -1);
        dval.assign(N, 0);
        owner.assign(N, -1);
        U.assign(W, 0);
        reserved.assign(W, 0);
        msw.assign(N, -1);
        for (int s = 0; s < N; s++) U[s >> 6] |= 1ULL << (s & 63);
        u64 lastMask = (N % 64 == 0) ? ~0ULL : ((1ULL << (N % 64)) - 1);
        vector<double> E(N + 1);
        vector<u64> x(W), setm(W);
        int ndet = 0;
        vector<int> detList;
        detListPtr = &detList;
        while (true) {
            ndet = 0;
            for (int j = 0; j < N; j++) ndet += det[j];
            if (ndet == N) break;
            int i = 0;
            while (det[i]) i++;
            // ideal entropy-to-go
            E[N] = 0;
            for (int j = N - 1; j >= i; j--) {
                if (det[j]) E[j] = E[j + 1];
                else if (touched[j]) E[j] = log2(1 + exp2(min(E[j + 1], 60.0)));
                else E[j] = 1 + BON + E[j + 1] / 2;
            }
            vector<double> pv(N + 1, -1.0);
#ifdef DUMP
            static long qq = 0; qq++; bool dumpNow = false;
#endif
            auto build = [&](const vector<double>& Ev, bool myopic) -> double {
                fill(x.begin(), x.end(), 0);
                fill(setm.begin(), setm.end(), 0);
                for (int s : detList) {
                    setm[s >> 6] |= 1ULL << (s & 63);
                    if (dval[owner[s]]) x[s >> 6] |= 1ULL << (s & 63);
                }
                fill(pv.begin() + i, pv.end(), -1.0);
                double R = 1.0, Ent = 0;
                for (int j = i; j < N && R > 1e-7; j++) {
                    if (det[j]) { pv[j] = 1; continue; }
                    if (!touched[j]) { pv[j] = 0.5; Ent += R * (1 + BON); R *= 0.5; continue; }
                    int asz = Acnt[j];
                    int agreeSet = 0, setCnt = 0;
                    for (int w = 0; w < W; w++) {
                        u64 m = A[j][w] & setm[w];
                        setCnt += pc(m);
                        agreeSet += pc(m & ~(x[w] ^ V[j][w]));
                    }
                    int F = asz - setCnt;
                    int bestc = agreeSet;
                    double bestv = -1;
                    double En = myopic ? 0.0 : Ev[j + 1];
                    for (int c = agreeSet; c <= agreeSet + F; c++) {
                        double p = (double)c / asz;
                        double v = WT(asz) * H(p) + p * En;
                        if (v > bestv) { bestv = v; bestc = c; }
                    }
                    int need = bestc - agreeSet;
                    for (int w = 0; w < W; w++) {
                        u64 fr = A[j][w] & ~setm[w];
                        if (!fr) continue;
                        int k = pc(fr);
                        u64 agreeBits;
                        if (need >= k) { agreeBits = fr; need -= k; }
                        else if (need == 0) agreeBits = 0;
                        else {
                            agreeBits = 0;
                            u64 t = fr;
                            while (need > 0) { u64 lb = t & -t; agreeBits |= lb; t ^= lb; need--; }
                        }
                        u64 disBits = fr & ~agreeBits;
                        x[w] = (x[w] & ~fr) | (V[j][w] & agreeBits) | (~V[j][w] & disBits);
                        setm[w] |= fr;
                    }
#ifdef DUMP
                    if (dumpNow) fprintf(stderr, "  door %d |A|=%d set=%d agreeSet=%d c=%d p=%.3f E=%.2f R=%.3g\n", j, asz, setCnt, agreeSet, bestc, (double)bestc/asz, En, R);
#endif
                    double p = (double)bestc / asz;
                    pv[j] = p;
                    Ent += R * WT(asz) * H(p);
                    R *= p;
                }
                return Ent;
            };
            vector<u64> bestx;
            double bestEnt = -1;
            vector<double> Ecur = E;
            for (int it = 0; it < ITERS; it++) {
                double e = build(Ecur, false);
                if (e > bestEnt) { bestEnt = e; bestx = x; }
                // recompute E from actual p (where known), ideal beyond
                int last = N;
                while (last > i && pv[last - 1] < 0) last--;
                for (int j = last - 1; j >= i; j--) {
                    double p = pv[j];
                    Ecur[j] = (touched[j] && !det[j] ? WT(Acnt[j]) : 1.0) * H(p) + (det[j] || touched[j] ? 0.0 : BON) + p * Ecur[j + 1];
                }
            }
#ifdef DUMP
            if (qq % 2000 == 0) { dumpNow = true; fprintf(stderr, "query %ld i=%d\n", qq, i); build(Ecur, false); dumpNow = false; }
#endif
            if (bestEnt < 1e-3) { bestEnt = build(E, true); bestx = x; }
            x = bestx;
            double Ent = bestEnt;
#if OPT
            {
                vector<int> win;
                int jb = i;
                static vector<u64> Fm;
                Fm.assign(W, 0);
                for (; jb < N; jb++) {
                    if (det[jb]) continue;
                    if (!touched[jb]) break;
                    if (Acnt[jb] <= FZ) {
                        int ag = 0;
                        for (int w = 0; w < W; w++) ag += pc(A[jb][w] & ~(x[w] ^ V[jb][w]));
                        if (ag == Acnt[jb]) {
                            for (int w = 0; w < W; w++) Fm[w] |= A[jb][w];
                            continue;
                        }
                    }
                    win.push_back(jb);
                }
                int nfresh = N - jb;
                double tail = nfresh == 0 ? 0.0 : (2.0 - exp2(1.0 - nfresh)) * (1 + TB);
                if ((int)win.size() > MAXWIN) { win.resize(MAXWIN); tail = TRT; }
                int nw = win.size();
                // classes
                static vector<int> clsOf;
                clsOf.assign(N, -1);
                vector<u64> keys, keys1;
                vector<int> cn, ck;
                struct PH { size_t operator()(const pair<u64,u64>& a) const { return a.first * 0x9E3779B97F4A7C15ULL ^ (a.second + 0x12345 + (a.second << 7)); } };
                unordered_map<pair<u64,u64>, int, PH> mp;
                mp.reserve(256);
                vector<long long> cagree(nw, 0);
                for (int t = 0; t < nw; t++) {
                    int j = win[t];
                    for (int w = 0; w < W; w++) cagree[t] += pc(A[j][w] & Fm[w] & ~(x[w] ^ V[j][w]));
                }
                for (int w = 0; w < W; w++) {
                    u64 uw = U[w] & ~Fm[w];
                    while (uw) {
                        int b = __builtin_ctzll(uw);
                        uw &= uw - 1;
                        int sidx = w * 64 + b;
                        u64 a0 = 0, a1 = 0;
                        for (int t = 0; t < nw; t++) {
                            int j = win[t];
                            if ((A[j][w] >> b) & 1) {
                                if ((V[j][w] >> b) & 1) a1 |= 1ULL << t;
                                else a0 |= 1ULL << t;
                            }
                        }
                        if (!(a0 | a1)) continue;
                        pair<u64,u64> key(a0, a1);
                        auto it = mp.find(key);
                        int id;
                        if (it == mp.end()) {
                            id = keys.size();
                            mp.emplace(key, id);
                            keys.push_back(a0); keys1.push_back(a1);
                            cn.push_back(0);
                            ck.push_back(0);
                        } else id = it->second;
                        clsOf[sidx] = id;
                        cn[id]++;
                        if ((x[w] >> b) & 1) ck[id]++;
                    }
                }
                int nc = keys.size();
                vector<double> asz(nw);
                for (int t = 0; t < nw; t++) asz[t] = Acnt[win[t]];
                for (int c = 0; c < nc; c++) {
                    u64 a0 = keys[c], a1 = keys1[c];
                    for (int t = 0; t < nw; t++) {
                        if ((a1 >> t) & 1) cagree[t] += ck[c];
                        if ((a0 >> t) & 1) cagree[t] += cn[c] - ck[c];
                    }
                }
                auto objective = [&](const vector<long long>& ca) {
                    double R = 1, e = 0;
                    for (int t = 0; t < nw; t++) {
                        double p = ca[t] / asz[t];
                        e += R * WT(asz[t]) * H(p);
                        R *= p;
                        if (R < 1e-12) return e;
                    }
                    return e + R * tail;
                };
                double cur = objective(cagree);
#ifdef CHK
                { static int cc=0; if (++cc % 1000 == 0) fprintf(stderr, "greedy %.4f init %.4f nw=%d nfresh=%d nc=%d\n", bestEnt, cur, nw, nfresh, nc); }
#endif
                vector<long long> tmp(nw);
                for (int round = 0; round < OPTR; round++) {
                    bool imp = false;
                    for (int c = 0; c < nc; c++) {
                        u64 a0 = keys[c], a1 = keys1[c];
                        int n = cn[c];
                        int step = max(1, n / 2);
                        while (true) {
                            bool moved = false;
                            for (int sg = -1; sg <= 1; sg += 2) {
                                int nk = ck[c] + sg * step;
                                if (nk < 0 || nk > n) continue;
                                int d = nk - ck[c];
                                tmp = cagree;
                                for (int t = 0; t < nw; t++) {
                                    if ((a1 >> t) & 1) tmp[t] += d;
                                    if ((a0 >> t) & 1) tmp[t] -= d;
                                }
                                double v = objective(tmp);
                                if (v > cur + 1e-12) {
                                    cur = v; cagree = tmp; ck[c] = nk; moved = true; imp = true;
                                    break;
                                }
                            }
                            if (!moved) {
                                if (step == 1) break;
                                step /= 2;
                            }
                        }
                    }
                    if (!imp) break;
                }
#ifdef CHK
                { static int cc2=0; if (++cc2 % 3000 == 0) { fprintf(stderr, "Q i=%d ent=%.3f tail=%.3f\n", i, cur, tail);
                  for (int t=0;t<nw;t++) fprintf(stderr," d%d|%d|%.3f", win[t], (int)asz[t], cagree[t]/asz[t]); fprintf(stderr,"\n");
                  for (int c=0;c<nc;c++){ string m; for(int t=0;t<nw;t++){ m += ((keys[c]>>t)&1)?'0':((keys1[c]>>t)&1)?'1':'.'; } fprintf(stderr,"  %s n=%d k=%d\n", m.c_str(), cn[c], ck[c]); } } }
#endif
                // build x
                vector<int> rem = ck;
                for (int w = 0; w < W; w++) {
                    u64 uw = U[w] & ~Fm[w];
                    while (uw) {
                        int b = __builtin_ctzll(uw);
                        uw &= uw - 1;
                        int c = clsOf[w * 64 + b];
                        if (c < 0) continue;
                        if (rem[c] > 0) { x[w] |= 1ULL << b; rem[c]--; }
                        else x[w] &= ~(1ULL << b);
                    }
                }
                Ent = cur;
#ifdef BUCK
                { static double bk[16]; static long qn=0; qn++;
                  double R=1; for (int t=0;t<nw;t++){ double p=cagree[t]/asz[t]; int b=0; while((1<<(b+1))<=asz[t]) b++; bk[b]+=R*H(p); R*=p; }
                  bk[15]+=R*tail;
                  if (qn%19000==0){ for(int b=0;b<16;b++) fprintf(stderr,"b%d:%.0f ",b,bk[b]); fprintf(stderr,"\n"); } }
#endif
#ifdef LOCAL
                { static long z=0, zs=0; if (nfresh == 0) { z++; if (z % 200 == 0) fprintf(stderr, "nofresh q=%ld nw=%d ent=%.3f\n", z, nw, cur);} }
#endif
            }
#endif
            x[W - 1] &= lastMask;
            int r = ask(x);
            if (r == -1) r = N;
#ifdef LOCAL
            {static double se=0, sd=0; static long cnt=0; se+=Ent; sd+=r-i; cnt++; if(cnt%5000==0) fprintf(stderr,"q=%ld avgEnt=%.3f avgDepth=%.2f i=%d\n",cnt,se/cnt,sd/cnt,i);}
#endif
            vector<int> stk;
            for (int j = i; j <= min(r, N - 1); j++) {
                if (det[j]) continue;
                bool open = j < r;
                if (!touched[j]) {
                    touched[j] = 1;
                    A[j] = U;
                    for (int w = 0; w < W; w++) A[j][w] &= ~reserved[w];
                    V[j] = x;
                    if (!open)
                        for (int w = 0; w < W; w++) V[j][w] = ~x[w];
                    int c = 0;
                    for (int w = 0; w < W; w++) c += pc(A[j][w]);
                    Acnt[j] = c;
                } else {
                    int c = 0;
                    for (int w = 0; w < W; w++) {
                        u64 eq = ~(x[w] ^ V[j][w]);
                        A[j][w] &= open ? eq : ~eq;
                        c += pc(A[j][w]);
                    }
                    Acnt[j] = c;
                }
                if (Acnt[j] == 1) stk.push_back(j);
            }
            checkDet(stk);
            if (HLIM > 0) hallPrune();
        }
        // answer
        string s(N, '0');
        vector<int> d(N);
        for (int j = 0; j < N; j++) {
            s[dsw[j]] = '0' + dval[j];
            d[dsw[j]] = j;
        }
#ifdef LOCAL
        for (int t = 0; t < N; t++) {
            if (d[t] != J_door[t] || s[t] - '0' != J_val[t]) { fprintf(stderr, "WRONG\n"); exit(1); }
        }
#else
        printf("! %s", s.c_str());
        for (int t = 0; t < N; t++) printf(" %d", d[t]);
        printf("\n");
        fflush(stdout);
#endif
    }
};

int main(int argc, char** argv) {
#ifdef LOCAL
    N = atoi(argv[1]);
    int seed = atoi(argv[2]);
    mt19937 g(seed);
    J_door.resize(N);
    J_val.resize(N);
    iota(J_door.begin(), J_door.end(), 0);
    shuffle(J_door.begin(), J_door.end(), g);
    for (auto& v : J_val) v = g() & 1;
#else
    if (scanf("%d", &N) != 1) return 0;
#endif
    if (N <= EXN) exactSolve();
    else {
        Solver S;
        S.run();
    }
#ifdef LOCAL
    fprintf(stderr, "N=%d excl=%lld queries=%lld\n", N, exclCnt, queryCount);
#endif
    return 0;
}

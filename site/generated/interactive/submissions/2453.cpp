#include <bits/stdc++.h>
using namespace std;

static int n;
static long long qcnt = 0;
static double wasteAcc = 0;
static inline void addWaste(double p){ if(p<=0||p>=1){wasteAcc+=1;return;} wasteAcc += 1 + p*log2(p) + (1-p)*log2(1-p); }
static mt19937 rng(712367);
#ifdef LOCAL
static vector<int> hidden;
#endif

static int ask(const vector<int>& q) {  // 0-indexed positions
    qcnt++;
#ifdef LOCAL
    {
        vector<char> seen(n, 0);
        for (int i : q) { if (i < 0 || i >= n || seen[i]) { fprintf(stderr, "BAD QUERY\n"); exit(1); } seen[i] = 1; }
    }
    long long s = 0;
    for (int i : q) s += hidden[i];
    return s % (long long)q.size() == 0;
#else
    string out = "? " + to_string(q.size());
    for (int i : q) { out += ' '; out += to_string(i + 1); }
    out += '\n';
    fputs(out.c_str(), stdout);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r == -1) exit(0);
    return r;
#endif
}

static vector<int> finalAns;
static void answer(vector<int> p) {
    if (p[0] > n / 2) for (int& v : p) v = n + 1 - v;
    finalAns = p;
#ifndef LOCAL
    string out = "!";
    for (int v : p) { out += ' '; out += to_string(v); }
    out += '\n';
    fputs(out.c_str(), stdout);
    fflush(stdout);
#endif
}

// ---------------- brute force for tiny n ----------------
static void bruteGreedy(vector<vector<int>> cand) {
    while (cand.size() > 1) {
        vector<int> q;
        long long C = cand.size();
        if (n <= 12) {
            int full = 1 << n;
            vector<int> pc(full);
            for (int m = 0; m < full; m++) pc[m] = __builtin_popcount(m);
            vector<int> sums(full);
            vector<long long> yes(full, 0);
            for (auto& c : cand) {
                sums[0] = 0;
                for (int m = 1; m < full; m++) {
                    int lb = __builtin_ctz(m);
                    sums[m] = sums[m & (m - 1)] + c[lb];
                    if (sums[m] % pc[m] == 0) yes[m]++;
                }
            }
            long long best = -1; int bm = -1;
            for (int m = 1; m < full; m++) {
                if (pc[m] < 2) continue;
                long long sc = min(yes[m], C - yes[m]);
                if (sc > best) { best = sc; bm = m; }
            }
            for (int i = 0; i < n; i++) if (bm >> i & 1) q.push_back(i);
        } else {
            long long S = (long long)(6e7 / ((double)C * n / 2 + 1));
            S = max(40LL, min(S, 30000LL));
            long long best = -1;
            vector<int> idx(n);
            iota(idx.begin(), idx.end(), 0);
            for (long long t = 0; t < S; t++) {
                int sz = 2 + rng() % (n - 2);
                for (int i = 0; i < sz; i++) swap(idx[i], idx[i + rng() % (n - i)]);
                long long yes = 0;
                for (auto& c : cand) {
                    int s = 0;
                    for (int i = 0; i < sz; i++) s += c[idx[i]];
                    if (s % sz == 0) yes++;
                }
                long long sc = min(yes, C - yes);
                if (sc > best) { best = sc; q.assign(idx.begin(), idx.begin() + sz); if (2 * sc >= C - 1) break; }
            }
        }
        int a = ask(q);
        vector<vector<int>> nc;
        for (auto& c : cand) {
            int s = 0;
            for (int i : q) s += c[i];
            if ((s % (int)q.size() == 0) == (a == 1)) nc.push_back(c);
        }
        cand.swap(nc);
    }
    answer(cand[0]);
}

static void bruteSmall() {
    vector<vector<int>> cand;
    vector<int> p(n);
    iota(p.begin(), p.end(), 1);
    do {
        if (p[0] <= n / 2) cand.push_back(p);
    } while (next_permutation(p.begin(), p.end()));
    bruteGreedy(cand);
}

// ---------------- general ----------------
static int cntRes(long long R, int mod) {
    R %= mod; if (R < 0) R += mod;
    if (R == 0) R = mod;
    if (R > n) return 0;
    return (int)((n - R) / mod + 1);
}

typedef bitset<800> BS;
#ifndef PAIRCAP
#define PAIRCAP 24
#endif
#ifndef BLK
#define BLK 12
#endif
#ifndef TAILMAX
#define TAILMAX 14
#endif
#ifndef BRUTE_LIMIT
#define BRUTE_LIMIT 3e5
#endif

struct GJ {
    vector<BS> rows; vector<int> rhs; vector<int> piv;
    vector<int> pivotRow;
    GJ(int N) : pivotRow(N, -1) {}
    // reduce row; returns accumulated rhs
    int reduce(BS& r) const {
        int acc = 0;
        for (size_t t = 0; t < rows.size(); t++) if (r[piv[t]]) { r ^= rows[t]; acc ^= rhs[t]; }
        return acc;
    }
    void insertReduced(const BS& r, int h) {
        int p = (int)r._Find_first();
        for (size_t t = 0; t < rows.size(); t++) if (rows[t][p]) { rows[t] ^= r; rhs[t] ^= h; }
        rows.push_back(r); rhs.push_back(h); piv.push_back(p);
        pivotRow[p] = (int)rows.size() - 1;
    }
};

static void general() {
    int M = 0;
    while ((1LL << M) * 4 < n) M++;
    bool useBrute = false;
    if (n <= 64) {
        for (int Mb = 1; Mb <= M; Mb++) {
            int md = 1 << Mb;
            double est = 1;
            for (int R = 1; R <= md; R++) { int c = cntRes(R, md); for (int t = 2; t <= c; t++) est *= t; }
            if (n % md == 0) est *= pow(2.0, 2 * Mb - 2);
            if (est <= BRUTE_LIMIT) { M = Mb; useBrute = true; break; }
        }
    }
    vector<int> y(n, 0);
    vector<pair<int, int>> F;  // (d,u)
    int e0 = 0, e1 = -1;

    for (int j = 0; j < M; j++) {
        int m = 1 << j, k = 2 * m;
        // frames
        if (j == 0) F = {{0, 1}, {1, 1}};
        else {
            vector<pair<int, int>> NF;
            for (auto [d, u] : F)
                for (int a = 0; a < 2; a++)
                    for (int b = 0; b < 2; b++) NF.push_back({d + a * m, u + b * m});
            F.swap(NF);
        }
        vector<int> low(n);
        for (int i = 0; i < n; i++) low[i] = y[i] & (m - 1);
        vector<char> known(n, 0), bit(n, 0);
        vector<int> knownList;
        auto setKnown = [&](int i, int b) { known[i] = 1; bit[i] = b; knownList.push_back(i); };
        setKnown(e0, 0);
        if (j >= 1) setKnown(e1, 0);

        vector<vector<int>> cls(m);
        for (int i = 0; i < n; i++) cls[low[i]].push_back(i);

        if (j >= 1) {
            // LA pool
            vector<int> P = {e0, e1};
            vector<char> inP(n, 0);
            inP[e0] = inP[e1] = 1;
            vector<size_t> ptr(m, 0);
            auto addOne = [&]() -> bool {
                // add from class with fewest pool members
                vector<int> pc(m, 0);
                for (int i : P) pc[low[i]]++;
                int bestc = -1;
                for (int r = 0; r < m; r++) {
                    while (ptr[r] < cls[r].size() && inP[cls[r][ptr[r]]]) ptr[r]++;
                    if (ptr[r] >= cls[r].size()) continue;
                    if (bestc < 0 || pc[r] < pc[bestc]) bestc = r;
                }
                if (bestc < 0) return false;
                int e = cls[bestc][ptr[bestc]];
                inP[e] = 1; P.push_back(e);
                return true;
            };
            while ((int)P.size() < k + 3) addOne();
            GJ g(n);
            {
                BS r; r[e0] = 1; int h = g.reduce(r); g.insertReduced(r, h ^ 0);
                BS r2; r2[e1] = 1; h = g.reduce(r2); g.insertReduced(r2, h ^ 0);
            }
            int fails = 0;
            vector<char> mark(n, 0);
            while (g.rows.size() < P.size()) {
                int sD = (int)P.size() - k;
                long long T = 0;
                for (int i : P) T += low[i];
                // choose D
                vector<int> D;
                vector<int> perm = P;
                bool ok = false;
                for (int tr = 0; tr < 50 && !ok; tr++) {
                    D.clear();
                    for (int t = 0; t < sD - 1; t++) {
                        int idx = t + rng() % (perm.size() - t);
                        swap(perm[t], perm[idx]);
                        D.push_back(perm[t]);
                    }
                    long long s = 0;
                    for (int i : D) { s += low[i]; mark[i] = 1; }
                    int need = (int)(((T - s) % m + m) % m);
                    // find in P with low==need, not marked
                    vector<int> opts;
                    for (int i : P) if (!mark[i] && low[i] == need) opts.push_back(i);
                    for (int i : D) mark[i] = 0;
                    if (!opts.empty()) { D.push_back(opts[rng() % opts.size()]); ok = true; }
                }
                if (!ok) { fails++; if (fails > 200) { addOne(); fails = 0; } continue; }
                BS row;
                for (int i : P) row[i] = 1;
                for (int i : D) row[i] = 0;
                BS rr = row;
                int acc = g.reduce(rr);
                if (rr.none()) {
                    fails++;
                    if (fails > 400) { addOne(); fails = 0; }
                    continue;
                }
                fails = 0;
                vector<int> Q;
                for (int i : D) mark[i] = 1;
                long long sl = 0;
                for (int i : P) if (!mark[i]) { Q.push_back(i); sl += low[i]; }
                for (int i : D) mark[i] = 0;
                int s = (int)((sl / m) & 1);
                int a = ask(Q);
                int h = a ? s : (s ^ 1);
                g.insertReduced(rr, h ^ acc);
            }
            for (int i : P) {
                if (known[i]) continue;
                int t = g.pivotRow[i];
                setKnown(i, g.rhs[t]);
            }
        }

        // parity test: returns parity of sum of bits over S (S: unknown elements)
        vector<char> mark(n, 0);
        auto parityTest = [&](const vector<int>& S) -> int {
            vector<int> R;
            long long lowS = 0;
            for (int x : S) lowS += low[x];
            int sz = k - (int)S.size();
            if (j == 0 && sz == 1) R = {e0};
            else if (sz > 0 || (lowS % m) != 0) {
                vector<int> A = knownList;  // S unknown so not in list
                int Nn = (int)A.size();
                int e = Nn - sz;
                bool chooseR = sz <= e;
                int csz = chooseR ? sz : e;
                long long tot = 0;
                for (int i : A) tot += low[i];
                int target = chooseR ? (int)(((-lowS) % m + m) % m) : (int)(((tot + lowS) % m + m) % m);
                vector<int> C;
                bool ok = false;
                vector<vector<int>> buck(m);
                for (int i : A) buck[low[i]].push_back(i);
                for (int tr = 0; tr < 500 && !ok; tr++) {
                    C.clear();
                    if (csz == 0) { if (target == 0) ok = true; break; }
                    for (int t = 0; t < csz - 1; t++) {
                        int idx = t + rng() % (A.size() - t);
                        swap(A[t], A[idx]);
                        C.push_back(A[t]);
                    }
                    long long s = 0;
                    for (int i : C) { s += low[i]; mark[i] = 1; }
                    int need = (int)(((target - s) % m + m) % m);
                    auto& bk = buck[need];
                    if (!bk.empty()) {
                        int st = rng() % bk.size();
                        for (size_t z = 0; z < bk.size(); z++) {
                            int cnd = bk[(st + z) % bk.size()];
                            if (!mark[cnd]) { C.push_back(cnd); ok = true; break; }
                        }
                    }
                    for (int i : C) mark[i] = 0;
                }
                if (!ok) { fprintf(stderr, "subset fail\n"); exit(1); }
                if (chooseR) R = C;
                else {
                    for (int i : C) mark[i] = 1;
                    for (int i : knownList) if (!mark[i]) R.push_back(i);
                    for (int i : C) mark[i] = 0;
                }
            }
            long long sy = 0;
            for (int i : R) sy += low[i] + (long long)m * bit[i];
            int s = (int)(((sy + lowS) % k + k) % k);  // 0 or m
            vector<int> Q = R;
            for (int x : S) Q.push_back(x);
            int a = ask(Q);
            int b = (s == 0) ? 0 : 1;
            if (!a) b ^= 1;
            return b;
        };

        auto lC = [](int N, int K) -> double {
            if (K < 0 || K > N) return -INFINITY;
            return (lgamma(N + 1.0) - lgamma(K + 1.0) - lgamma(N - K + 1.0));
        };
        {
            vector<int> c0(m, 0), c1(m, 0), outCnt(m, 0), inWin(m, 0);
            vector<int> stream;
            for (int r = 0; r < m; r++)
                for (int i : cls[r]) {
                    if (known[i]) (bit[i] ? c1[r] : c0[r])++;
                    else { stream.push_back(i); outCnt[r]++; }
                }
            auto completeClass = [&](int r) {
                vector<pair<int, int>> NF;
                for (auto [d, u] : F)
                    if (cntRes((long long)u * r + d, k) == c0[r]) NF.push_back({d, u});
                F.swap(NF);
                if (F.empty()) { fprintf(stderr, "F empty\n"); exit(1); }
            };
            for (int r = 0; r < m; r++) if (outCnt[r] == 0) completeClass(r);
            size_t pos = 0;
            vector<int> Wl;
            vector<char> alive(1, 1);
            vector<double> w, h;
            while (true) {
                while ((int)Wl.size() < BLK && pos < stream.size()) {
                    int t = Wl.size();
                    int e = stream[pos++];
                    Wl.push_back(e);
                    outCnt[low[e]]--; inWin[low[e]]++;
                    alive.resize(1 << (t + 1));
                    for (int a2 = 0; a2 < (1 << t); a2++) alive[a2 | (1 << t)] = alive[a2];
                }
                int Wn = Wl.size();
                if (Wn == 0) break;
                int full = 1 << Wn;
                // classes in window
                vector<int> cw;
                for (int e : Wl) if (find(cw.begin(), cw.end(), low[e]) == cw.end()) cw.push_back(low[e]);
                int nc = cw.size();
                vector<int> cmask(nc, 0);
                for (int t = 0; t < Wn; t++) for (int q = 0; q < nc; q++) if (low[Wl[t]] == cw[q]) cmask[q] |= 1 << t;
                map<vector<int>, long long> vecs;
                for (auto [d, u] : F) {
                    vector<int> v(nc);
                    bool ok = true;
                    for (int q = 0; q < nc && ok; q++) {
                        int r = cw[q];
                        int A = cntRes((long long)u * r + d, k);
                        int B = (int)cls[r].size() - A;
                        if (A < c0[r] || B < c1[r]) ok = false;
                        v[q] = B - c1[r];
                    }
                    if (ok) vecs[v]++;
                }
                if (vecs.empty()) { fprintf(stderr, "no frame\n"); exit(1); }
                // log weight table per class per (vec, t)
                vector<double> lwm(full, -INFINITY);
                vector<pair<vector<int>, double>> VV;
                for (auto& [v, c] : vecs) VV.push_back({v, log((double)c)});
                double gmx = -INFINITY;
                vector<double> terms(VV.size());
                for (int a2 = 0; a2 < full; a2++) {
                    if (!alive[a2]) continue;
                    double mx = -INFINITY;
                    for (size_t z = 0; z < VV.size(); z++) {
                        double acc = VV[z].second;
                        for (int q = 0; q < nc && acc != -INFINITY; q++) {
                            int t = __builtin_popcount(a2 & cmask[q]);
                            int ones = VV[z].first[q];
                            if (t > ones) { acc = -INFINITY; break; }
                            double lc = lC(outCnt[cw[q]], ones - t);
                            acc += lc;
                        }
                        terms[z] = acc;
                        mx = max(mx, acc);
                    }
                    if (mx == -INFINITY) { alive[a2] = 0; continue; }
                    double sm = 0;
                    for (size_t z = 0; z < VV.size(); z++) if (terms[z] != -INFINITY) sm += exp(terms[z] - mx);
                    lwm[a2] = mx + log(sm);
                    gmx = max(gmx, lwm[a2]);
                }
                w.assign(full, 0.0);
                int cntAlive = 0;
                int andM = full - 1, orM = 0;
                for (int a2 = 0; a2 < full; a2++) {
                    if (!alive[a2]) continue;
                    w[a2] = max(exp(lwm[a2] - gmx), 1e-300);
                    cntAlive++; andM &= a2; orM |= a2;
                }
                if (cntAlive == 0) { fprintf(stderr, "no alive\n"); exit(1); }
                int det = andM | (~orM & (full - 1));
                if (det) {
                    int t = 31 - __builtin_clz(det);
                    int v = andM >> t & 1;
                    int e = Wl[t], r = low[e];
                    setKnown(e, v);
                    (v ? c1[r] : c0[r])++;
                    inWin[r]--;
                    vector<char> na(full >> 1);
                    for (int a2 = 0; a2 < (full >> 1); a2++) {
                        int lo = a2 & ((1 << t) - 1), hi = (a2 >> t) << (t + 1);
                        na[a2] = alive[lo | hi | (v << t)];
                    }
                    alive.swap(na);
                    Wl.erase(Wl.begin() + t);
                    if (inWin[r] == 0 && outCnt[r] == 0) completeClass(r);
                    continue;
                }
                h = w;
                for (int len = 1; len < full; len <<= 1)
                    for (int i0 = 0; i0 < full; i0 += len << 1)
                        for (int i1 = i0; i1 < i0 + len; i1++) {
                            double x = h[i1], yv = h[i1 + len];
                            h[i1] = x + yv; h[i1 + len] = x - yv;
                        }
                double tot = h[0];
                int bestS = -1; double bv = 1e18;
                for (int S = 1; S < full; S++) {
                    int pcS = __builtin_popcount(S);
                    if (pcS > k) continue;
                    if (pcS == k && nc > 1) {
                        long long ls = 0;
                        for (int q = 0; q < nc; q++) ls += (long long)cw[q] * __builtin_popcount(S & cmask[q]);
                        if (ls % m) continue;
                    }
                    double v = fabs(h[S]) / tot;
                    if (v < bv - 1e-12) { bv = v; bestS = S; }
                }
                addWaste(0.5 + bv / 2);
                vector<int> SS;
                for (int t = 0; t < Wn; t++) if (bestS >> t & 1) SS.push_back(Wl[t]);
                int par = parityTest(SS);
                for (int a2 = 0; a2 < full; a2++)
                    if (alive[a2] && (__builtin_popcount(a2 & bestS) & 1) != par) alive[a2] = 0;
            }
        }
        for (int i = 0; i < n; i++) y[i] += bit[i] * m;
#ifdef LOCAL
        fprintf(stderr, "level %d done q=%lld F=%zu waste=%.2f t=%.2f\n", j, qcnt, F.size(), wasteAcc, clock()/(double)CLOCKS_PER_SEC);
#endif
        if (j == 0) {
            for (int i = 0; i < n; i++) if (y[i] & 1) { e1 = i; break; }
        }
    }

    int mod = 1 << M;
    vector<vector<int>> cls(mod);
    for (int i = 0; i < n; i++) cls[y[i]].push_back(i);
    auto Rof = [&](int c, int d, int u) -> int {
        long long v = ((long long)u * c + d - 1) % mod;
        if (v < 0) v += mod;
        return (int)v + 1;
    };
    auto classSum = [&](int c, int d, int u) -> long long {
        long long R = Rof(c, d, u);
        long long cnt = (n - R) / mod + 1;
        return cnt * R + (long long)mod * cnt * (cnt - 1) / 2;
    };
    // canonical reps under sign
    vector<pair<int, int>> reps;
    for (auto [d, u] : F) {
        int d2 = (int)(((long long)n + 1 - d) % mod + mod) % mod;
        int u2 = (mod - u) % mod;
        if (make_pair(d, u) <= make_pair(d2, u2)) reps.push_back({d, u});
    }
    if (useBrute) {
        vector<vector<int>> cand;
        vector<int> cur(n, 0);
        for (auto [d, u] : reps) {
            // enumerate assignments class by class
            vector<vector<int>> vals(mod);
            for (int c = 0; c < mod; c++) {
                int R = Rof(c, d, u);
                for (int v = R; v <= n; v += mod) vals[c].push_back(v);
            }
            function<void(int)> rec = [&](int c) {
                if (cand.size() > 2000000) return;
                if (c == mod) { cand.push_back(cur); return; }
                vector<int> v = vals[c];
                sort(v.begin(), v.end());
                do {
                    for (size_t t = 0; t < v.size(); t++) cur[cls[c][t]] = v[t];
                    rec(c + 1);
                } while (next_permutation(v.begin(), v.end()));
            };
            rec(0);
        }
        if (cand.size() <= 2000000) { bruteGreedy(cand); return; }
    }
    // Phase 1: resolve frame
    while (reps.size() > 1) {
        long long bestSc = -1;
        vector<int> bestC;
        int CAND = 300;
        for (int tr = 0; tr < CAND; tr++) {
            int t = 1 + rng() % 6;
            if (t > mod) t = mod;
            vector<int> cs;
            set<int> used;
            while ((int)cs.size() < t) {
                int c = rng() % mod;
                if (used.count(c)) continue;
                used.insert(c); cs.push_back(c);
            }
            int ksz = 0;
            for (int c : cs) ksz += cls[c].size();
            if (ksz < 2) continue;
            long long yes = 0;
            for (auto [d, u] : reps) {
                long long s = 0;
                for (int c : cs) s += classSum(c, d, u);
                if (s % ksz == 0) yes++;
            }
            long long sc = min(yes, (long long)reps.size() - yes);
            if (sc > bestSc) { bestSc = sc; bestC = cs; }
        }
        for (int c0 = 0; c0 < mod && bestSc <= 0; c0++) {
            for (int tr = 0; tr < 12; tr++) {
                int t = 2 + tr % 5;
                if (t > mod) t = mod;
                vector<int> cs = {c0};
                set<int> used = {c0};
                while ((int)cs.size() < t) {
                    int c = rng() % mod;
                    if (used.count(c)) continue;
                    used.insert(c); cs.push_back(c);
                }
                int ksz = 0;
                for (int c : cs) ksz += cls[c].size();
                long long yes = 0;
                for (auto [d, u] : reps) {
                    long long s = 0;
                    for (int c : cs) s += classSum(c, d, u);
                    if (s % ksz == 0) yes++;
                }
                long long sc = min(yes, (long long)reps.size() - yes);
                if (sc > bestSc) { bestSc = sc; bestC = cs; }
            }
        }
        if (bestSc <= 0) { fprintf(stderr, "frame stuck %zu\n", reps.size()); for (auto [d,u]:reps) fprintf(stderr,"d=%d u=%d\n",d,u); exit(1); }
        vector<int> Q;
        for (int c : bestC) for (int i : cls[c]) Q.push_back(i);
        int ksz = Q.size();
        int a = ask(Q);
        vector<pair<int, int>> nr;
        for (auto [d, u] : reps) {
            long long s = 0;
            for (int c : bestC) s += classSum(c, d, u);
            if ((s % ksz == 0) == (a == 1)) nr.push_back({d, u});
        }
        reps.swap(nr);
    }
    int fd = reps[0].first, fu = reps[0].second;
#ifdef LOCAL
    fprintf(stderr, "phase1 done q=%lld\n", qcnt);
#endif

    // Phase 2
    vector<int> val(n, -1);
    vector<vector<int>> remVals(mod);
    vector<int> clsOf(n);
    for (int c = 0; c < mod; c++) {
        int R = Rof(c, fd, fu);
        for (int v = R; v <= n; v += mod) remVals[c].push_back(v);
        for (int i : cls[c]) clsOf[i] = c;
        if (remVals[c].size() != cls[c].size()) { fprintf(stderr, "size mismatch\n"); exit(1); }
    }
    vector<vector<int>> candOf(n);
    auto assignV = [&](int x, int v) {
        val[x] = v;
        auto& rv = remVals[clsOf[x]];
        rv.erase(find(rv.begin(), rv.end(), v));
    };
    for (int c = 0; c < mod; c++) if (cls[c].size() == 1) assignV(cls[c][0], remVals[c][0]);

    long long Stotal = (long long)n * (n + 1) / 2;
    int K2 = 2 * mod;
    // window over classes with joint candidates
    vector<int> wcls;            // active classes
    vector<int> actEl;           // unknown elements in active classes
    vector<vector<int>> cands;   // each: values for actEl
    cands.push_back({});
    size_t nextC = 0;
    const long long CANDMAX = 600;
    auto factorial = [](int x) { long long r = 1; for (int i = 2; i <= x; i++) r *= i; return r; };
    auto addClass = [&](int c) {
        vector<int> el;
        for (int i : cls[c]) if (val[i] < 0) el.push_back(i);
        vector<int> v = remVals[c];
        sort(v.begin(), v.end());
        vector<vector<int>> perms;
        do perms.push_back(v); while (next_permutation(v.begin(), v.end()));
        vector<vector<int>> nc;
        nc.reserve(cands.size() * perms.size());
        for (auto& cd : cands)
            for (auto& pm : perms) {
                vector<int> x = cd;
                x.insert(x.end(), pm.begin(), pm.end());
                nc.push_back(move(x));
            }
        cands.swap(nc);
        for (int e : el) actEl.push_back(e);
        wcls.push_back(c);
    };
    vector<int> posInAct(n, -1);
    while (true) {
        // retire determined classes
        bool changed = true;
        while (changed) {
            changed = false;
            for (size_t wi = 0; wi < wcls.size(); wi++) {
                int c = wcls[wi];
                vector<int> idx;
                for (size_t t = 0; t < actEl.size(); t++) if (clsOf[actEl[t]] == c) idx.push_back(t);
                bool det = true;
                for (int t : idx) { for (auto& cd : cands) if (cd[t] != cands[0][t]) { det = false; break; } if (!det) break; }
                if (!det) continue;
                for (int t : idx) assignV(actEl[t], cands[0][t]);
                vector<int> keep;
                for (size_t t = 0; t < actEl.size(); t++) if (clsOf[actEl[t]] != c) keep.push_back(t);
                vector<int> nae;
                for (int t : keep) nae.push_back(actEl[t]);
                for (auto& cd : cands) { vector<int> x; for (int t : keep) x.push_back(cd[t]); cd.swap(x); }
                actEl.swap(nae);
                wcls.erase(wcls.begin() + wi);
                changed = true;
                break;
            }
        }
        // fill window
        while (nextC < (size_t)mod) {
            int c = nextC;
            int un = 0;
            for (int i : cls[c]) if (val[i] < 0) un++;
            if (un == 0) { nextC++; continue; }
            if (!wcls.empty() && (long long)cands.size() * factorial(un) > CANDMAX) break;
            addClass(c);
            nextC++;
        }
        if (wcls.empty()) break;
        for (size_t t = 0; t < actEl.size(); t++) posInAct[actEl[t]] = t;
        long long C = cands.size();
        // singles available
        vector<int> singles;
        for (int i = 0; i < n; i++) if (val[i] >= 0) singles.push_back(i);
        // enumerate options
        struct Opt { double h; vector<int> S; int k; int rho; };
        vector<Opt> opts;
        int A = actEl.size();
        vector<vector<int>> subsets;
        for (int i = 0; i < A; i++) subsets.push_back({i});
        int pcap = (clock() > 4.0 * CLOCKS_PER_SEC) ? 0 : PAIRCAP;
        for (int i = 0; i < min(A, pcap); i++) for (int j2 = i + 1; j2 < min(A, pcap); j2++) subsets.push_back({i, j2});
        vector<int> Ks;
        for (int k = 3; k <= min(n - 1, 24); k++) Ks.push_back(k);
        bool bigOK = K2 <= n && K2 > 24 && (int)singles.size() >= min(K2, n - K2) + 2;
        if (bigOK) Ks.push_back(K2);
        vector<long long> hist;
        for (auto& S : subsets) {
            for (int k : Ks) {
                hist.assign(k, 0);
                for (auto& cd : cands) { long long sm = 0; for (int t : S) sm += cd[t]; hist[sm % k]++; }
                int br = -1; long long bc = -1;
                for (int r = 0; r < k; r++) { long long sc = min(hist[r], C - hist[r]); if (sc > bc) { bc = sc; br = r; } }
                if (bc <= 0) continue;
                double p = (double)hist[br] / C;
                double h = -(p * log2(p) + (1 - p) * log2(1 - p));
                opts.push_back({h, S, k, br});
            }
        }
        sort(opts.begin(), opts.end(), [](const Opt& x, const Opt& y) {
            if (fabs(x.h - y.h) > 1e-9) return x.h > y.h;
            if (x.S.size() != y.S.size()) return x.S.size() < y.S.size();
            return x.k < y.k;
        });
        bool asked = false;
#ifdef LOCAL
        static double tEval=0,tTry=0; static long long tries=0; double t0=clock();
        tEval += 0; 
#endif
        for (auto& op : opts) {
#ifdef LOCAL
            tries++;
#endif
            vector<int> Sel;
            for (int t : op.S) Sel.push_back(actEl[t]);
            int k = op.k;
            vector<int> Q;
            bool ok = false;
            if (k == K2 && bigOK) {
                // direct (refs size k-|S|, sum = -rho) or complement (T = S + refs size n-k-|S|, refs sum = Stotal - rho)
                bool direct = (k - (int)Sel.size()) <= (n - k - (int)Sel.size());
                int need = direct ? k - (int)Sel.size() : n - k - (int)Sel.size();
                long long tgt = direct ? -op.rho : Stotal - op.rho;
                tgt %= k; if (tgt < 0) tgt += k;
                if (need < 1 || (int)singles.size() < need + 1) continue;
                vector<vector<int>> buck(k);
                for (int i : singles) buck[val[i] % k].push_back(i);
                vector<int> Aa = singles;
                vector<char> mk(n, 0);
                for (int tr = 0; tr < 400 && !ok; tr++) {
                    vector<int> Cc;
                    long long sm = 0;
                    for (int t = 0; t < need - 1; t++) {
                        int id = t + rng() % (Aa.size() - t);
                        swap(Aa[t], Aa[id]);
                        Cc.push_back(Aa[t]); sm += val[Aa[t]]; mk[Aa[t]] = 1;
                    }
                    int nd = (int)(((tgt - sm) % k + k) % k);
                    auto& bk = buck[nd];
                    if (!bk.empty()) {
                        int st = rng() % bk.size();
                        for (size_t z = 0; z < bk.size(); z++) { int cnd = bk[(st + z) % bk.size()]; if (!mk[cnd]) { Cc.push_back(cnd); ok = true; break; } }
                    }
                    for (int i : Cc) mk[i] = 0;
                    if (ok) {
                        if (direct) { Q = Cc; for (int e : Sel) Q.push_back(e); }
                        else {
                            for (int i : Cc) mk[i] = 1;
                            for (int e : Sel) mk[e] = 1;
                            for (int i = 0; i < n; i++) if (!mk[i]) Q.push_back(i);
                        }
                    }
                }
                if (!ok) continue;
            } else {
                // DP over items: singles + units (unknown elements of classes not containing S elements, excluding active unknowns of other... allowed if class has no S element)
                struct It { int sz; long long sum; vector<int> el; };
                vector<It> items;
                for (int i : singles) items.push_back({1, val[i], {i}});
                for (int c = 0; c < mod; c++) {
                    vector<int> el;
                    bool hasS = false;
                    for (int i : cls[c]) if (val[i] < 0) { el.push_back(i); for (int e : Sel) if (e == i) hasS = true; }
                    if (el.empty() || hasS) continue;
                    // class must not contain any S-element's class mates? (any unknown in S's class would be fine only if not in S; but its value correlates) -> exclude classes of S
                    bool sameCls = false;
                    for (int e : Sel) if (clsOf[e] == c) sameCls = true;
                    if (sameCls) continue;
                    long long sm = 0;
                    for (int v : remVals[c]) sm += v;
                    items.push_back({(int)el.size(), sm, el});
                }
                shuffle(items.begin(), items.end(), rng);
                stable_sort(items.begin(), items.end(), [](const It& x, const It& y) { return (x.sz > 1) > (y.sz > 1); });
                if (items.size() > 100) items.resize(100);
                int L = items.size();
                int need = k - (int)Sel.size();
                int tgt = (int)(((-(long long)op.rho) % k + k) % k);
                vector<char> dp((size_t)(L + 1) * (need + 1) * k, 0);
                auto ID = [&](int i, int c, int r) { return ((size_t)i * (need + 1) + c) * k + r; };
                dp[ID(0, 0, 0)] = 1;
                for (int i = 0; i < L; i++) {
                    int sz = items[i].sz; int vm = (int)(items[i].sum % k);
                    for (int c = 0; c <= need; c++)
                        for (int r = 0; r < k; r++) {
                            if (!dp[ID(i, c, r)]) continue;
                            dp[ID(i + 1, c, r)] = 1;
                            if (c + sz <= need) dp[ID(i + 1, c + sz, (r + vm) % k)] = 1;
                        }
                }
                if (!dp[ID(L, need, tgt)]) continue;
                int c = need, r = tgt;
                for (int i = L; i >= 1; i--) {
                    if (dp[ID(i - 1, c, r)]) continue;
                    int sz = items[i - 1].sz; int vm = (int)(items[i - 1].sum % k);
                    c -= sz; r = ((r - vm) % k + k) % k;
                    for (int e : items[i - 1].el) Q.push_back(e);
                }
                for (int e : Sel) Q.push_back(e);
                ok = true;
            }
            int a = ask(Q);
            vector<vector<int>> nc;
            for (auto& cd : cands) {
                long long sm = 0;
                for (int t : op.S) sm += cd[t];
                if (((sm % k) == op.rho) == (a == 1)) nc.push_back(cd);
            }
            cands.swap(nc);
            asked = true;
            break;
        }
#ifdef LOCAL
        tTry += clock()-t0; if (qcnt%50==0) fprintf(stderr,"tries=%lld tTry=%.2f opts=%zu cands=%zu A=%d\n",tries,tTry/CLOCKS_PER_SEC,opts.size(),cands.size(),A);
#endif
        if (!asked) { fprintf(stderr, "phase2 stuck\n"); exit(1); }
    }
    vector<int> p(n);
    for (int i = 0; i < n; i++) {
        // convert: frame val is real value
        p[i] = val[i];
    }
    answer(p);
}

int main() {
    if (scanf("%d", &n) != 1) return 0;
#ifdef LOCAL
    hidden.resize(n);
    for (auto& v : hidden) if (scanf("%d", &v) != 1) return 0;
#endif
    if (n == 2) { answer({1, 2}); }
    else if (n <= 8) bruteSmall();
    else general();
#ifdef LOCAL
    vector<int> h = hidden;
    if (h[0] > n / 2) for (int& v : h) v = n + 1 - v;
    bool ok = (h == finalAns);
    printf("%d %lld %s\n", n, qcnt, ok ? "OK" : "WRONG");
#endif
    return 0;
}

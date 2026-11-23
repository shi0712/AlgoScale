#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef long double ld;

static int n;
static mt19937_64 rng;

#ifdef LOCAL
static vector<ll> hiddenC;
static long long qcount = 0;
static ll ask(int i, int j, int k) {
    qcount++;
    ll a = hiddenC[i], b = hiddenC[j], c = hiddenC[k];
    return min({a, b, c}) + max({a, b, c});
}
static long long qcountGet() { return qcount; }
static void localMain();
#else
static long long qcountGet() { return 0; }
static ll ask(int i, int j, int k) {
    printf("? %d %d %d\n", i + 1, j + 1, k + 1);
    fflush(stdout);
    ll r;
    if (scanf("%lld", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}
#endif

static const ld EPS = 1e-6;
#ifndef ALO
#define ALO 0.3
#endif
#ifndef AHI
#define AHI 10.0
#endif
#ifndef ALPHA
#define ALPHA 1.0
#endif
#ifndef LAMBDA
#define LAMBDA 0.01
#endif
#ifndef BASEM
#define BASEM 7
#endif
#ifndef FANO
#define FANO 7
#endif
#ifndef GAPMODE
#define GAPMODE 0
#endif
#ifndef PROBEMARGIN
#define PROBEMARGIN 0.25
#endif
#ifndef TIEW
#define TIEW 0.02
#endif
static const ll MAXV = 1000000000LL;

// ---------------- base solver over m elements via hypotheses (weak orders) ----------------
struct Hyp {
    int b[8];
    int k;
    double w;
    bool alive;
    bool det;
    vector<ll> sol;          // per block value when determined
    vector<vector<ld>> rr;   // RREF rows: k coeffs + rhs
    vector<int> piv;
};

struct BaseSolver {
    int m;
    vector<int> idx;  // global indices
    vector<Hyp> hs;
    vector<array<int, 3>> tri;
    vector<pair<array<int, 3>, ll>> qs;
    vector<array<int, 3>> forced;

    void genHyps(int pos, int *b, int maxb) {
        if (pos == m) {
            // must be surjective onto 0..maxb
            int k = maxb + 1;
            vector<bool> used(k, false);
            for (int i = 0; i < m; i++) used[b[i]] = true;
            for (int i = 0; i < k; i++)
                if (!used[i]) return;
            Hyp h;
            for (int i = 0; i < m; i++) h.b[i] = b[i];
            h.k = k;
            h.w = pow(TIEW, m - k);
            h.alive = true;
            h.det = false;
            hs.push_back(h);
            return;
        }
        for (int v = 0; v < m; v++) {
            b[pos] = v;
            genHyps(pos + 1, b, max(maxb, v));
        }
    }

    void rowFor(const Hyp &h, const array<int, 3> &t, vector<ld> &row) {
        row.assign(h.k, 0);
        int a = h.b[t[0]], b = h.b[t[1]], c = h.b[t[2]];
        int mn = min({a, b, c}), mx = max({a, b, c});
        row[mn] += 1;
        row[mx] += 1;
    }

    // recompute RREF and status
    void analyze(Hyp &h) {
        int k = h.k;
        vector<vector<ld>> M;
        for (auto &q : qs) {
            vector<ld> row;
            rowFor(h, q.first, row);
            row.push_back((ld)q.second);
            M.push_back(row);
        }
        int R = M.size();
        int r = 0;
        h.piv.clear();
        for (int c = 0; c < k && r < R; c++) {
            int p = -1;
            ld best = EPS;
            for (int i = r; i < R; i++)
                if (fabsl(M[i][c]) > best) best = fabsl(M[i][c]), p = i;
            if (p < 0) continue;
            swap(M[p], M[r]);
            ld d = M[r][c];
            for (int j = 0; j <= k; j++) M[r][j] /= d;
            for (int i = 0; i < R; i++)
                if (i != r && fabsl(M[i][c]) > EPS) {
                    ld f = M[i][c];
                    for (int j = 0; j <= k; j++) M[i][j] -= f * M[r][j];
                }
            h.piv.push_back(c);
            r++;
        }
        for (int i = r; i < R; i++)
            if (fabsl(M[i][k]) > 1e-3) {
                h.alive = false;
                return;
            }
        M.resize(r);
        h.rr = M;
        if (r == k) {
            h.sol.assign(k, 0);
            for (int i = 0; i < r; i++) {
                ld v = M[i][k];
                ll iv = llroundl(v);
                if (fabsl(v - iv) > 1e-3) {
                    h.alive = false;
                    return;
                }
                h.sol[h.piv[i]] = iv;
            }
            for (int i = 0; i < k; i++) {
                if (h.sol[i] < 0 || h.sol[i] > MAXV) {
                    h.alive = false;
                    return;
                }
                if (i > 0 && h.sol[i] <= h.sol[i - 1]) {
                    h.alive = false;
                    return;
                }
            }
            h.det = true;
        } else {
            h.det = false;
            if (useLP && !feasible(h)) h.alive = false;
        }
    }

    // LP feasibility (real relaxation) via Fourier-Motzkin on free vars
    bool feasible(Hyp &h) {
        int k = h.k;
        vector<bool> isPiv(k, false);
        for (int c : h.piv) isPiv[c] = true;
        vector<int> fr;
        for (int c = 0; c < k; c++)
            if (!isPiv[c]) fr.push_back(c);
        int F = fr.size();
        // express each var as affine in free vars: v_c = const + sum coef*f
        vector<vector<ld>> ex(k, vector<ld>(F + 1, 0));
        for (int fi = 0; fi < F; fi++) ex[fr[fi]][fi] = 1;
        for (size_t i = 0; i < h.rr.size(); i++) {
            int c = h.piv[i];
            ex[c][F] = h.rr[i][k];
            for (int fi = 0; fi < F; fi++) ex[c][fi] = -h.rr[i][fr[fi]];
        }
        // constraints: a.f + b >= 0
        vector<vector<ld>> cons;
        auto add = [&](vector<ld> v) { cons.push_back(v); };
        {
            vector<ld> v = ex[0];  // v0 >= 0
            add(v);
            vector<ld> u(F + 1);
            for (int j = 0; j <= F; j++) u[j] = -ex[k - 1][j];
            u[F] += MAXV;
            add(u);
            for (int i = 0; i + 1 < k; i++) {
                vector<ld> d(F + 1);
                for (int j = 0; j <= F; j++) d[j] = ex[i + 1][j] - ex[i][j];
                d[F] -= 1;
                add(d);
            }
        }
        for (int fi = F - 1; fi >= 0; fi--) {
#ifdef DBG2
            fprintf(stderr,"elim %d\n",fi); for(auto&c:cons){for(auto x:c)fprintf(stderr,"%Lg ",x);fprintf(stderr,"\n");}
#endif
            vector<vector<ld>> pos, neg, zer;
            for (auto &c : cons) {
                if (c[fi] > 1e-9) pos.push_back(c);
                else if (c[fi] < -1e-9) neg.push_back(c);
                else zer.push_back(c);
            }
            vector<vector<ld>> nc = zer;
            for (auto &p : pos)
                for (auto &q : neg) {
                    ld a = p[fi], b = -q[fi];
                    vector<ld> r(F + 1);
                    for (int j = 0; j <= F; j++) r[j] = p[j] * b + q[j] * a;
                    r[fi] = 0;
                    ld s = 0;
                    for (int j = 0; j < F; j++) s = max(s, fabsl(r[j]));
                    if (s > 1e-9) {
                        for (int j = 0; j <= F; j++) r[j] /= s;
                    } else {
                        for (int j = 0; j < F; j++) r[j] = 0;
                    }
                    nc.push_back(r);
                }
            cons = nc;
            if (cons.size() > 4000) return true;  // give up, assume feasible
        }
        for (auto &c : cons)
            if (c[F] < -1e-4) {
#ifdef DBG2
                fprintf(stderr,"infeasible const %Lg F=%d\n",c[F],F);
#endif
                return false;}
        return true;
    }

    // returns true if new; else sets pred
    bool predict(const Hyp &h, const array<int, 3> &t, ld &pred) {
        vector<ld> row;
        rowFor(h, t, row);
        ld cst = 0;
        for (size_t i = 0; i < h.rr.size(); i++) {
            int c = h.piv[i];
            ld f = row[c];
            if (fabsl(f) < EPS) continue;
            for (int j = 0; j < h.k; j++) row[j] -= f * h.rr[i][j];
            cst += f * h.rr[i][h.k];
        }
        for (int j = 0; j < h.k; j++)
            if (fabsl(row[j]) > 1e-6) return true;
        pred = cst;
        return false;
    }

    bool useLP = true;
    bool anyAlive() {
        for (auto &h : hs)
            if (h.alive) return true;
        return false;
    }
    bool asked(const array<int, 3> &t) {
        for (auto &q : qs)
            if (q.first == t) return true;
        return false;
    }
    // true if every alive hypothesis predicts the same answer for t
    bool uninformative(const array<int, 3> &t) {
        bool first = true;
        ld v0 = 0;
        for (auto &h : hs) {
            if (!h.alive) continue;
            ld p;
            if (predict(h, t, p)) return false;
            if (first) v0 = p, first = false;
            else if (fabsl(p - v0) > 1e-3) return false;
        }
        return true;
    }

    bool done(vector<ll> &vals) {
        bool first = true;
        for (auto &h : hs) {
            if (!h.alive) continue;
            if (!h.det) return false;
            vector<ll> v(m);
            for (int i = 0; i < m; i++) v[i] = h.sol[h.b[i]];
            if (first) vals = v, first = false;
            else if (v != vals) return false;
        }
        return !first;
    }

    void run(vector<int> ids, vector<ll> &out) {
        idx = ids;
        m = ids.size();
        int b[8];
        genHyps(0, b, -1);
        for (int i = 0; i < m; i++)
            for (int j = i + 1; j < m; j++)
                for (int l = j + 1; l < m; l++) tri.push_back({i, j, l});
        vector<ll> vals;
        size_t fstep = 0;
        while (!done(vals)) {
            // choose query
            double bestS = -1e18;
            int bestT = -1;
            double Wtot = 0;
            for (auto &h : hs)
                if (h.alive) Wtot += h.w;
            vector<char> bestNew;
            while (fstep < forced.size() && uninformative(forced[fstep])) fstep++;
            for (int ti = 0; ti < (int)tri.size() && fstep >= forced.size(); ti++) {
                double Wn = 0;
                vector<pair<ld, double>> pr;
                vector<char> isNew(hs.size(), 0);
                for (size_t hi = 0; hi < hs.size(); hi++) {
                    auto &h = hs[hi];
                    if (!h.alive) continue;
                    ld p;
                    if (predict(h, tri[ti], p)) Wn += h.w, isNew[hi] = 1;
                    else pr.push_back({p, h.w});
                }
                sort(pr.begin(), pr.end());
                double alpha = forced.empty() ? ALO : AHI;
                double score = Wn / Wtot * (1.0 + alpha * (Wtot - Wn) / Wtot);
                for (size_t i = 0; i < pr.size();) {
                    size_t j = i;
                    double g = 0;
                    while (j < pr.size() && fabsl(pr[j].first - pr[i].first) < 1e-3) g += pr[j].second, j++;
                    double removed = (Wtot - g - Wn) + Wn * (1 - LAMBDA);
                    score += g / Wtot * alpha * removed / Wtot;
                    i = j;
                }
                if (score > bestS) bestS = score, bestT = ti, bestNew = isNew;
            }
            while (fstep < forced.size() && uninformative(forced[fstep])) fstep++;
            if (fstep < forced.size()) {
                for (int ti = 0; ti < (int)tri.size(); ti++)
                    if (tri[ti] == forced[fstep]) bestT = ti;
                fstep++;
                bestNew.assign(hs.size(), 0);
                for (size_t hi = 0; hi < hs.size(); hi++) {
                    ld p;
                    if (hs[hi].alive && predict(hs[hi], tri[bestT], p)) bestNew[hi] = 1;
                }
            }
            if (bestT < 0 || asked(tri[bestT])) {
                bestT = -1;
                for (int ti = 0; ti < (int)tri.size(); ti++)
                    if (!asked(tri[ti])) { bestT = ti; break; }
                if (bestT < 0) break;  // cannot happen: all triples determine the values
                bestNew.assign(hs.size(), 0);
            }
            auto t = tri[bestT];
            ll r = ask(idx[t[0]], idx[t[1]], idx[t[2]]);
            qs.push_back({t, r});
            for (size_t hi = 0; hi < hs.size(); hi++) {
                auto &h = hs[hi];
                if (!h.alive) continue;
                analyze(h);
                if (bestNew[hi]) h.w *= LAMBDA;
            }
            if (!anyAlive()) {
                useLP = false;
                for (auto &h : hs) {
                    h.alive = true;
                    analyze(h);
                }
            }
            {
                double mx = 0;
                for (auto &h : hs)
                    if (h.alive) mx = max(mx, h.w);
                if (mx > 0)
                    for (auto &h : hs) h.w /= mx;
            }
        }
        if ((int)vals.size() != m) {
            vals.assign(m, 0);
            for (auto &h : hs)
                if (h.alive && h.det) {
                    for (int i = 0; i < m; i++) vals[i] = h.sol[h.b[i]];
                    break;
                }
        }
        out = vals;
    }
};

static long long baseQ = 0, uCount = 0, probeN = 0, probeCost = 0;
static vector<ll> solveAll() {
    vector<int> ord(n);
    iota(ord.begin(), ord.end(), 0);
    shuffle(ord.begin(), ord.end(), rng);
    vector<ll> c(n, -1);
    vector<bool> known(n, false);
    int m = min(n, BASEM);
    vector<int> base(ord.begin(), ord.begin() + m);
    {
        BaseSolver bs;
        if (FANO && m == 7) { bs.forced = {{0, 1, 2}, {0, 3, 4}, {0, 5, 6}, {1, 3, 5}, {1, 4, 6}, {2, 3, 6}, {2, 4, 5}}; bs.forced.resize(FANO); }
        vector<ll> vals;
        long long q0 = qcountGet();
        bs.run(base, vals);
        baseQ = qcountGet() - q0;
        for (int i = 0; i < m; i++) c[base[i]] = vals[i], known[base[i]] = true;
    }
    // incremental
    vector<pair<ll, int>> srt;
    for (int i = 0; i < m; i++) srt.push_back({c[base[i]], base[i]});
    sort(srt.begin(), srt.end());
    auto insertKnown = [&](int k, ll v) {
        c[k] = v;
        known[k] = true;
        srt.insert(lower_bound(srt.begin(), srt.end(), make_pair(v, k)), {v, k});
    };
    auto measure = [&](int k) {
        int sz = srt.size();
        int bestj = -1;
        ld bestsc = 1e30;
        for (int j = 0; j + 1 < sz; j++) {
            ll g = srt[j + 1].first - srt[j].first;
            if (g == 0) {
                bestj = j;
                break;
            }
            ld sc = (ld)g;
            if (GAPMODE > 0) {
                ld sum = 0;
                int cnt = 0;
                for (int d = -GAPMODE; d <= GAPMODE; d++) {
                    int jj = j + d;
                    if (d == 0 || jj < 0 || jj + 1 >= sz) continue;
                    sum += srt[jj + 1].first - srt[jj].first;
                    cnt++;
                }
                if (cnt > 0) sc = (ld)g / (sum / cnt);
            }
            if (sc < bestsc) bestsc = sc, bestj = j;
        }
        int x = srt[bestj].second, y = srt[bestj + 1].second;
        ll cx = c[x], cy = c[y];
        ll r = ask(x, y, k);
        ll ck;
        if (cx == cy) ck = r - cx;
        else if (r < cx + cy) ck = r - cy;
        else if (r > cx + cy) ck = r - cx;
        else {
            // c_k in [cx, cy]
            uCount++;
            if (bestj + 2 < sz) {
                int z = srt[bestj + 2].second;
                ll r2 = ask(k, y, z);
                ck = r2 - c[z];
            } else {
                int w = srt[bestj - 1].second;
                ll r2 = ask(k, x, w);
                ck = r2 - c[w];
            }
        }
        insertKnown(k, ck);
    };
    // decide whether probing three unknowns at once is worthwhile
    auto probeWorth = [&]() -> bool {
        int K = srt.size();
        int z0 = 0, z1 = 0;
        for (auto &p : srt) {
            if (p.first == 0) z0++;
            if (p.first == MAXV) z1++;
        }
        double p0 = (double)z0 / K, p1 = (double)z1 / K;
        double sP = p0 * p0 * p0 + p1 * p1 * p1;
        if (sP < 0.02) return false;
        // Monte Carlo estimate of the expected cost of a probe
        const int SMP = 1000;
        double cost = 0;
        for (int it = 0; it < SMP; it++) {
            ll v[3];
            for (int q = 0; q < 3; q++) v[q] = srt[rng() % K].first;
            ll r = min({v[0], v[1], v[2]}) + max({v[0], v[1], v[2]});
            if (r == 0 || r == 2 * MAXV) { cost += 1; continue; }
            ll lo = min(v[0], v[1]), hi = max(v[0], v[1]);
            if (r != lo + hi || lo == hi) cost += 3;
            else cost += 4;
        }
        cost /= SMP;
        return cost < 3.0 - PROBEMARGIN;
    };
    int t = m;
    while (t < n) {
        if (n - t >= 3 && probeWorth()) {
            int u1 = ord[t], u2 = ord[t + 1], u3 = ord[t + 2];
            t += 3;
            probeN++;
            long long pq0 = qcountGet();
            struct PC { long long q0; ~PC() { probeCost += qcountGet() - q0; } } pcGuard{pq0};
            ll r = ask(u1, u2, u3);
            if (r == 0 || r == 2 * MAXV) {
                insertKnown(u1, r / 2);
                insertKnown(u2, r / 2);
                insertKnown(u3, r / 2);
                continue;
            }
            measure(u1);
            measure(u2);
            ll lo = min(c[u1], c[u2]), hi = max(c[u1], c[u2]);
            if (lo == hi) insertKnown(u3, r - lo);
            else if (r < lo + hi) insertKnown(u3, r - hi);
            else if (r > lo + hi) insertKnown(u3, r - lo);
            else measure(u3);
            continue;
        }
        measure(ord[t]);
        t++;
    }
    return c;
}

int main() {
#ifdef LOCAL
    localMain();
#else
    if (scanf("%d", &n) != 1) return 0;
    rng.seed(chrono::steady_clock::now().time_since_epoch().count());
    vector<ll> c = solveAll();
    printf("!");
    for (int i = 0; i < n; i++) printf(" %lld", c[i]);
    printf("\n");
    fflush(stdout);
#endif
    return 0;
}

#ifdef LOCAL
static void localMain() {
    // args via stdin: kind n seeds
    char kind[32];
    int N, S;
    if (scanf("%s %d %d", kind, &N, &S) != 3) return;
    double tot = 0, totB = 0, totU = 0;
    int mx = -1000, bad = 0;
    for (int s = 0; s < S; s++) {
        mt19937_64 g(s * 7919 + 13);
        n = N;
        hiddenC.assign(n, 0);
        string k = kind;
        for (int i = 0; i < n; i++) {
            ll v;
            if (k == "rand") v = g() % (MAXV + 1);
            else if (k == "small") v = g() % 3;
            else if (k == "mid") v = g() % 51;
            else if (k == "eq") v = 7;
            else if (k == "exp") v = (ll)(-log((g() % 1000000 + 1) / 1e6) * 1000);
            else if (k == "two") v = (g() % 2) ? MAXV : 0;
            else if (k == "one") v = 5;
            else if (k == "perm") v = i;
            else if (k == "sq") { ll u = g() % 1001; v = u * u * u; }
            else if (k == "bin") v = g() % 2;
            else if (k == "zero") v = 0;
            else if (k == "zmost") v = (g() % 10 == 0) ? g() % 1000 : 0;
            else if (k == "maxmost") v = (g() % 5 == 0) ? g() % 1000 : MAXV;
            else if (k == "z50") v = (g() % 2) ? g() % 1000 : 0;
            else if (k == "clus") v = (g() % 3) * 300000000LL + g() % 10;
            else v = g() % 100000;
            hiddenC[i] = v;
        }
        if (k == "one") { hiddenC[g() % n] = 0; hiddenC[g() % n] = 10; }
        if (k == "perm") shuffle(hiddenC.begin(), hiddenC.end(), g);
        qcount = 0; uCount = 0;
        rng.seed(s * 31 + 7);
        vector<ll> c = solveAll();
        if (c != hiddenC) bad++;
        long long ex = qcount - n;
        tot += ex; totB += baseQ; totU += uCount; mx = max<long long>(mx, ex);
        if (getenv("VERB")) fprintf(stderr, "seed %d extra %lld base %lld\n", s, ex, baseQ);
    }
    printf("%-6s n=%5d extra avg=%.3f max=%d base=%.3f U=%.3f bad=%d\n", kind, N, tot / S, mx, totB / S, totU / S, bad);
}
#endif

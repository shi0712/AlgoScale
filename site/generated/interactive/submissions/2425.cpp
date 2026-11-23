#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

int n;
vector<vector<int>> adj;
int G;                       // virtual root
vector<int> gpar, dep, tin, tout, ord;
vector<vector<int>> ch, up;
int LOG;

static void die() { exit(0); }

int qcount=0;
ll askQ(const vector<pair<int, int>>& S) {
    qcount++;
    int k = 0;
    for (auto& p : S) k += p.second;
    string s = "? 1 " + to_string(k);
    for (auto& p : S)
        for (int i = 0; i < p.second; i++) { s += ' '; s += to_string(p.first); }
    s += '\n';
    fputs(s.c_str(), stdout);
    fflush(stdout);
    ll r;
    if (scanf("%lld", &r) != 1) die();
    return r;
}
void toggleQ(int u) {
    qcount++;
    printf("? 2 %d\n", u);
    fflush(stdout);
}

// ---------------- DSS library ----------------
vector<vector<ll>> lib[16], libX[16];
bool isDss(const vector<ll>& a) {
    int m = a.size();
    unordered_set<ll> s;
    for (int mask = 0; mask < (1 << m); mask++) {
        ll t = 0;
        for (int i = 0; i < m; i++) if (mask >> i & 1) t += a[i];
        if (!s.insert(t).second) return false;
    }
    return true;
}
void buildLib() {
    vector<ll> u = {0, 1};
    for (int i = 1; i < 16; i++) {
        int r = (int)llround(sqrt(2.0 * i));
        u.push_back(2 * u[i] - u[i - r]);
    }
    for (int m = 1; m <= 15; m++) {
        set<vector<ll>> seen;
        for (int a = 1; a <= m; a++) {
            int b = m - a;
            ll s = 1LL << b;
            vector<ll> D;
            for (int i = 0; i < a; i++) D.push_back(s * (u[a] - u[i]));
            for (int i = 0; i < b; i++) D.push_back(1LL << i);
            sort(D.rbegin(), D.rend());
            if (seen.count(D)) continue;
            if (!isDss(D)) continue;
            seen.insert(D);
            lib[m].push_back(D);
        }
    }
    // two-chain families (online only)
    for (int m = 4; m <= 15; m++) {
        for (int a = 2; a <= m; a++)
            for (int c = 2; c <= a && a + c <= m; c++) {
                int b = m - a - c;
                ll S2 = 1LL << b;
                ll sc = 0;
                for (int i = 0; i < c; i++) sc += u[c] - u[i];
                ll S1 = S2 * (sc + 1);
                vector<ll> D;
                for (int i = 0; i < a; i++) D.push_back(S1 * (u[a] - u[i]));
                for (int i = 0; i < c; i++) D.push_back(S2 * (u[c] - u[i]));
                for (int i = 0; i < b; i++) D.push_back(1LL << i);
                sort(D.rbegin(), D.rend());
                libX[m].push_back(D);
            }
    }
}

// ---------------- value learning ----------------
vector<int> known;
vector<ll> wv;  // virtual values (wv[G] = F)

struct QPlan {
    vector<int> nodes;      // T nodes (parents before children)
    vector<ll> coef;
    vector<int> parentIdx;  // index of placed parent, -1 for top
    ll k, sigma;
    bool first;
};

struct Sim {
    vector<char> kn;
    vector<int> hgt;
    vector<int> stamp;   // per-node sorted-children cache stamp
    vector<vector<int>> sch;
    vector<int> used;
    int step = 0, ustamp = 0;
    mt19937 rng;
    int newPol, scorePol, attachPol;
    Sim(int np, int sp, int ap, unsigned seed) : rng(seed), newPol(np), scorePol(sp), attachPol(ap) {
        kn.assign(n + 1, 0);
        hgt.assign(n + 1, 0);
        stamp.assign(n + 1, -1);
        sch.assign(n + 1, {});
        used.assign(n + 1, 0);
    }
    const vector<int>& sortedCh(int u) {
        if (stamp[u] != step) {
            stamp[u] = step;
            sch[u] = ch[u];
            sort(sch[u].begin(), sch[u].end(), [&](int a, int b) { return hgt[a] > hgt[b]; });
        }
        return sch[u];
    }
    void computeHeights() {
        for (int i = n - 1; i >= 0; i--) {
            int u = ord[i];
            if (kn[u]) continue;
            int h = 0;
            for (int c : ch[u]) h = max(h, hgt[c]);
            hgt[u] = h + 1;
        }
    }
    // frontier sorted by height desc
    bool embed(const vector<ll>& D, const vector<int>& fr, QPlan& P, ll& cost) {
        ustamp++;
        P.nodes.clear(); P.coef.clear(); P.parentIdx.clear();
        cost = 0;
        vector<ll> resid;
        vector<int> cptr;
        size_t fpHi = 0;            // pointer from deepest
        long fpLo = (long)fr.size() - 1;  // pointer from shallowest
        for (size_t idx = 0; idx < D.size(); idx++) {
            ll e = D[idx];
            int best = -1;
            for (int i = 0; i < (int)P.nodes.size(); i++) {
                if (resid[i] < e) continue;
                if (best != -1 && resid[i] >= resid[best]) continue;
                if (cptr[i] < (int)sortedCh(P.nodes[i]).size()) best = i;
            }
            if (best != -1 && attachPol == 1 && idx > 0) {
                while (fpLo >= 0 && used[fr[fpLo]] == ustamp) fpLo--;
                if (fpLo >= 0 && hgt[fr[fpLo]] == 1 && e <= (ll)n / 8) best = -1;
            }
            if (best != -1) {
                int bc = sortedCh(P.nodes[best])[cptr[best]++];
                used[bc] = ustamp;
                resid[best] -= e;
                P.nodes.push_back(bc); P.coef.push_back(e); P.parentIdx.push_back(best);
                resid.push_back(e); cptr.push_back(0);
            } else {
                int t = -1;
                bool deep = (newPol == 0) || idx == 0;
                if (newPol == 2) {
                    // deep if a following element could go below
                    deep = (idx + 1 < D.size() && D[idx + 1] < e);
                    deep = idx == 0 || (rng() & 1);
                }
                if (deep) {
                    while (fpHi < fr.size() && used[fr[fpHi]] == ustamp) fpHi++;
                    if (fpHi < fr.size()) t = fr[fpHi];
                } else {
                    while (fpLo >= 0 && used[fr[fpLo]] == ustamp) fpLo--;
                    if (fpLo >= 0) t = fr[fpLo];
                }
                if (t < 0) return false;
                used[t] = ustamp;
                P.nodes.push_back(t); P.coef.push_back(e); P.parentIdx.push_back(-1);
                resid.push_back(e); cptr.push_back(0);
                cost += e;
            }
        }
        return true;
    }
    vector<QPlan> run() {
        vector<QPlan> plans;
        int remaining = n;
        bool first = true;
        while (remaining > 0) {
            step++;
            computeHeights();
            vector<int> fr;
            if (first) fr = ch[G];
            else
                for (int u = 1; u <= n; u++)
                    if (!kn[u] && kn[gpar[u]]) fr.push_back(u);
            sort(fr.begin(), fr.end(), [&](int a, int b) { return hgt[a] > hgt[b]; });
            int avail = remaining - (first ? 1 : 0);
            QPlan best; bool have = false;
            double bestScore = 1e18;
            QPlan P;
            for (int m = min(13, avail); m >= 1 && !have; m--) {
                for (auto& D : lib[m]) {
                    ll cost;
                    if (!embed(D, fr, P, cost)) continue;
                    ll sigma = 0;
                    for (ll e : D) sigma += e;
                    ll k = cost;
                    if (first) { k = max(k, sigma + 1); if (k % 2 == 0) k++; }
                    if (k > n) continue;
                    double sc;
                    if (scorePol == 0) sc = k;
                    else if (scorePol == 1) {
                        // prefer consuming leaves
                        int leaves = 0;
                        for (int u : P.nodes) if (ch[u].empty()) leaves++;
                        sc = -leaves * 10000.0 + k;
                    } else if (scorePol == 2) {
                        // prefer consuming low-height tops
                        double s = 0;
                        for (size_t i = 0; i < P.nodes.size(); i++) if (P.parentIdx[i] < 0) s += hgt[P.nodes[i]];
                        sc = s * 10000.0 + k;
                    } else sc = rng() % 1000;
                    if (sc < bestScore) { bestScore = sc; best = P; best.k = k; best.sigma = sigma; best.first = first; have = true; }
                }
            }
            if (!have) {
                // m=0 only possible on first query when G has no children (n==1) - not in constraints
                best.nodes.clear(); best.coef.clear(); best.parentIdx.clear();
                best.k = 1; best.sigma = 0; best.first = first; have = true;
            }
            if (first) { kn[G] = 1; remaining--; first = false; }
            for (int u : best.nodes) { kn[u] = 1; remaining--; }
            plans.push_back(best);
        }
        return plans;
    }
};

void executePlans(const vector<QPlan>& plans) {
    known.assign(n + 1, 0);
    wv.assign(n + 1, 0);
    vector<ll> fk(n + 1, 0);
    for (auto& best : plans) {
        int m = best.nodes.size();
        vector<ll> mass(m);
        for (int i = 0; i < m; i++) mass[i] = best.coef[i];
        for (int i = 0; i < m; i++) if (best.parentIdx[i] >= 0) mass[best.parentIdx[i]] -= best.coef[i];
        vector<pair<int, int>> S;
        ll tot = 0;
        for (int i = 0; i < m; i++) if (mass[i] > 0) { S.push_back({best.nodes[i], (int)mass[i]}); tot += mass[i]; }
        if (best.first) {
            ll gmass = best.k - tot;
            if (gmass > 0) S.push_back({G, (int)gmass});
        }
        ll ans = askQ(S);
        ll D;
        if (best.first) {
            ll k = best.k;
            ll Fpar = ((ans - best.sigma) % 2 + 2) % 2;
            ll F0 = (ll)floor((double)ans / k);
            ll F = F0;
            for (ll c = F0 - 3; c <= F0 + 3; c++) {
                if (((c % 2) + 2) % 2 != Fpar) continue;
                if (llabs(ans - k * c) <= best.sigma) { F = c; break; }
            }
            wv[G] = F; fk[G] = F; known[G] = 1;
            D = ans - k * F;
        } else {
            ll base = 0;
            for (int i = 0; i < m; i++)
                if (best.parentIdx[i] < 0) base += best.coef[i] * fk[gpar[best.nodes[i]]];
            D = ans - base;
        }
        if (m > 0) {
            ll Psum = (D + best.sigma) / 2;
            int found = 0;
            for (int mask = 0; mask < (1 << m); mask++) {
                ll t = 0;
                for (int i = 0; i < m; i++) if (mask >> i & 1) t += best.coef[i];
                if (t == Psum) { found = mask; break; }
            }
            for (int i = 0; i < m; i++) wv[best.nodes[i]] = (found >> i & 1) ? 1 : -1;
            for (int i = 0; i < m; i++) {
                int u = best.nodes[i];
                fk[u] = fk[gpar[u]] + wv[u];
                known[u] = 1;
            }
        }
    }
}

int curNP, curSP, curAP;
double PLAN_BUDGET = 0.4;
vector<QPlan> planValues(bool full) {
    vector<QPlan> best;
    int it = 0;
    for (int np = 0; np < 3; np++)
        for (int sp = 0; sp < 4; sp++) {
            if (!full && (np == 2 || sp == 3)) continue;
            int reps = full ? ((np == 2 || sp == 3) ? 12 : 1) : 1;
            for (int rep = 0; rep < reps; rep++)
                for (int ap = 0; ap < 2; ap++) {
                    Sim s(np, sp, ap, 1000 + it++);
                    auto p = s.run();
                    if (best.empty() || p.size() < best.size()) { best = p; curNP = np; curSP = sp; curAP = ap; }
                }
        }
    return best;
}

// ---------------- resolution ----------------
int ancestorAtDepth(int u, int d) {
    for (int j = LOG - 1; j >= 0; j--)
        if (dep[u] - (1 << j) >= d) u = up[j][u];
    return u;
}
inline bool inSub(int x, int h) { return tin[x] <= tin[h] && tin[h] <= tout[x]; }
// neighbor of x toward h (x itself if h==x)
inline int sideOf(int x, int h) {
    if (h == x) return x;
    if (inSub(x, h)) return ancestorAtDepth(h, dep[x] + 1);
    return gpar[x];
}

// ---------------- online adaptive value learning ----------------
int polNP = 0, polSP = 0, polAP = 0;
struct HistInfo { vector<pair<ll, int>> h; double ent; int worst; ll sigma; };
map<vector<ll>, HistInfo> histCache;
const HistInfo& getHist(const vector<ll>& D) {
    auto it = histCache.find(D);
    if (it != histCache.end()) return it->second;
    int m = D.size();
    vector<ll> sums(1 << m);
    sums[0] = 0;
    for (int i = 0; i < m; i++)
        for (int s = 0; s < (1 << i); s++) sums[s | (1 << i)] = sums[s] + D[i];
    sort(sums.begin(), sums.end());
    HistInfo hi;
    hi.sigma = 0;
    for (ll e : D) hi.sigma += e;
    hi.ent = 0;
    hi.worst = 0;
    double N = 1 << m;
    for (size_t i = 0; i < sums.size();) {
        size_t j = i;
        while (j < sums.size() && sums[j] == sums[i]) j++;
        int c = j - i;
        hi.h.push_back({2 * sums[i] - hi.sigma, c});
        double p = c / N;
        hi.ent -= p * log2(p);
        hi.worst = max(hi.worst, c);
        i = j;
    }
    return histCache[D] = hi;
}

vector<char> procd;
ll Fval;
vector<vector<ll>> fhCache;     // per hypothesis: current f on processed nodes
vector<vector<char>> Rset;      // per hypothesis: allowed roots

struct Cand {
    QPlan P;
    vector<ll> D;
    vector<pair<int, ll>> res;  // resolver (node, mass)
    double ent, score, pend = -1;
    int worst;
    ll k;
};
double BETA = 0.3;

vector<ll> evalBuf;
// exact evaluation with hypotheses
void evalExact(Cand& c) {
    int HN = fhCache.size();
    vector<ll> pc = c.P.coef;
    sort(pc.rbegin(), pc.rend());
    const HistInfo& hi = getHist(pc);
    map<ll, int> shifts;
    for (int h = 0; h < HN; h++) {
        ll s = 0;
        for (size_t i = 0; i < c.P.nodes.size(); i++)
            if (c.P.parentIdx[i] < 0) s += c.P.coef[i] * fhCache[h][gpar[c.P.nodes[i]]];
        for (auto& r : c.res) s += r.second * fhCache[h][r.first];
        shifts[s]++;
    }
    if (shifts.size() == 1) {
        // entropy identical to hist but with HN multiplicity
        c.ent = hi.ent;
        c.worst = hi.worst * HN;
        return;
    }
    ll lo = LLONG_MAX, hiV = LLONG_MIN;
    for (auto& [s, cnt] : shifts) { lo = min(lo, s - hi.sigma); hiV = max(hiV, s + hi.sigma); }
    ll range = hiV - lo + 1;
    double N = (double)HN * (1 << c.D.size());
    if (range > 4000000) {
        // fallback: sort-based
        vector<pair<ll, ll>> all;
        for (auto& [s, cnt] : shifts)
            for (auto& [v, cc] : hi.h) all.push_back({s + v, (ll)cc * cnt});
        sort(all.begin(), all.end());
        double E = 0;
        int worst = 0;
        for (size_t i = 0; i < all.size();) {
            size_t j = i;
            ll t = 0;
            while (j < all.size() && all[j].first == all[i].first) t += all[j++].second;
            double p = t / N;
            E -= p * log2(p);
            worst = max<ll>(worst, t);
            i = j;
        }
        c.ent = E;
        c.worst = worst;
        return;
    }
    if ((ll)evalBuf.size() < range) evalBuf.assign(range, 0);
    for (auto& [s, cnt] : shifts)
        for (auto& [v, cc] : hi.h) evalBuf[s + v - lo] += (ll)cc * cnt;
    double E = 0;
    ll worst = 0;
    for (auto& [s, cnt] : shifts)
        for (auto& [v, cc] : hi.h) {
            ll& t = evalBuf[s + v - lo];
            if (t > 0) {
                double p = t / N;
                E -= p * log2(p);
                worst = max(worst, t);
                t = 0;
            }
        }
    c.ent = E;
    c.worst = worst;
}

const int HCAP = 48;
bool useX = false;
double PRIOR_THR = 0.12;

// learned prior: P(+1 | parent +1), P(+1 | parent -1), P(+1)
double prPP = 0.5, prPM = 0.5, prM = 0.5;
bool priorActive = false;
void estimatePrior() {
    // use hypothesis 0 values on processed nodes where all hypotheses agree
    int HN = fhCache.size();
    double cpp = 0, cpm = 0, cmp = 0, cmm = 0, cp = 0, cm = 0;
    for (int u = 1; u <= n; u++) {
        if (!procd[u] || u == G) continue;
        ll w = fhCache[0][u] - fhCache[0][gpar[u]];
        bool agree = true;
        for (int h = 1; h < HN && agree; h++) if (fhCache[h][u] - fhCache[h][gpar[u]] != w) agree = false;
        if (!agree) continue;
        if (w > 0) cp++; else cm++;
        int p = gpar[u];
        if (p == G) continue;
        ll wp = fhCache[0][p] - fhCache[0][gpar[p]];
        if (wp > 0) { if (w > 0) cpp++; else cpm++; }
        else { if (w > 0) cmp++; else cmm++; }
    }
    prPP = (cpp + 1) / (cpp + cpm + 2);
    prPM = (cmp + 1) / (cmp + cmm + 2);
    prM = (cp + 1) / (cp + cm + 2);
    double dev = max({fabs(prPP - 0.5), fabs(prPM - 0.5), fabs(prM - 0.5)});
    priorActive = (cp + cm >= 20) && dev > PRIOR_THR && !getenv("NOPRIOR");
}
vector<double> wbuf;
// weighted (prior) evaluation: sets c.ent, c.worst, c.pend
void evalWeighted(Cand& c) {
    int HN = fhCache.size();
    int m = c.P.nodes.size();
    // probabilities per mask
    static vector<double> pr;
    static vector<ll> sm;
    if ((int)pr.size() < (1 << m)) { pr.resize(1 << m); sm.resize(1 << m); }
    pr[0] = 1;
    sm[0] = 0;
    for (int i = 0; i < m; i++) {
        int u = c.P.nodes[i];
        int pi = c.P.parentIdx[i];
        double pPlusKnown = prM;
        if (pi < 0) {
            int p = gpar[u];
            if (p != G) {
                ll wp = fhCache[0][p] - fhCache[0][gpar[p]];
                pPlusKnown = wp > 0 ? prPP : prPM;
            }
        }
        for (int s = 0; s < (1 << i); s++) {
            double pp = pPlusKnown;
            if (pi >= 0) pp = (s >> pi & 1) ? prPP : prPM;
            pr[s | (1 << i)] = pr[s] * pp;
            sm[s | (1 << i)] = sm[s] + c.P.coef[i];
            pr[s] *= (1 - pp);
        }
    }
    ll sigma = 0;
    for (ll e : c.P.coef) sigma += e;
    double Hmask = 0;
    for (int s = 0; s < (1 << m); s++) if (pr[s] > 0) Hmask -= pr[s] * log2(pr[s]);
    // shifts
    map<ll, int> shifts;
    for (int h = 0; h < HN; h++) {
        ll sh = 0;
        for (int i = 0; i < m; i++)
            if (c.P.parentIdx[i] < 0) sh += c.P.coef[i] * fhCache[h][gpar[c.P.nodes[i]]];
        for (auto& r : c.res) sh += r.second * fhCache[h][r.first];
        shifts[sh]++;
    }
    // aggregate by subset sum (array indexed by sum in [0, sigma])
    static vector<double> ap;
    static vector<int> ac;
    if ((ll)ap.size() < sigma + 1) { ap.assign(sigma + 1, 0); ac.assign(sigma + 1, 0); }
    vector<int> touched;
    touched.reserve(1 << m);
    for (int s = 0; s < (1 << m); s++) {
        ll v = sm[s];
        if (ac[v] == 0) touched.push_back(v);
        ap[v] += pr[s];
        ac[v]++;
    }
    double E = 0;
    int worst = 0;
    if (shifts.size() == 1) {
        int cnt = shifts.begin()->second;
        for (int v : touched) {
            double p = ap[v];
            if (p > 0) E -= p * log2(p);
            worst = max(worst, ac[v] * cnt);
        }
    } else {
        ll lo = LLONG_MAX, hiV = LLONG_MIN;
        for (auto& [sh, cnt] : shifts) { lo = min(lo, sh - sigma); hiV = max(hiV, sh + sigma); }
        ll range = hiV - lo + 1;
        static vector<double> bp;
        static vector<int> bc;
        if (range <= 8000000) {
            if ((ll)bp.size() < range) { bp.assign(range, 0); bc.assign(range, 0); }
            for (auto& [sh, cnt] : shifts)
                for (int v : touched) {
                    ll idx = sh + 2 * v - sigma - lo;
                    bp[idx] += ap[v] * cnt / HN;
                    bc[idx] += ac[v] * cnt;
                }
            for (auto& [sh, cnt] : shifts)
                for (int v : touched) {
                    ll idx = sh + 2 * v - sigma - lo;
                    if (bc[idx]) {
                        double p = bp[idx];
                        if (p > 0) E -= p * log2(p);
                        worst = max(worst, bc[idx]);
                        bp[idx] = 0;
                        bc[idx] = 0;
                    }
                }
        } else {
            vector<pair<ll, pair<double, int>>> all;
            for (auto& [sh, cnt] : shifts)
                for (int v : touched) all.push_back({sh + 2 * v - sigma, {ap[v] * cnt / HN, ac[v] * cnt}});
            sort(all.begin(), all.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
            for (size_t i = 0; i < all.size();) {
                size_t j = i;
                double p = 0;
                int cn = 0;
                while (j < all.size() && all[j].first == all[i].first) { p += all[j].second.first; cn += all[j].second.second; j++; }
                if (p > 0) E -= p * log2(p);
                worst = max(worst, cn);
                i = j;
            }
        }
    }
    for (int v : touched) { ap[v] = 0; ac[v] = 0; }
    c.ent = E;
    c.worst = worst;
    c.pend = log2((double)HN) + Hmask - E;
}




int togglesDone = 0;
void mergeHyps(vector<vector<ll>>& NF, vector<vector<char>>& NR) {
    int N = NF.size();
    vector<int> idx(N);
    iota(idx.begin(), idx.end(), 0);
    sort(idx.begin(), idx.end(), [&](int a, int b) { return NF[a] < NF[b]; });
    for (int i = 0; i < N; i++)
        for (int u = 1; u <= n; u++)
            if (NR[i][u] && procd[u] && llabs(NF[i][u]) != 1) NR[i][u] = 0;
    fhCache.clear();
    Rset.clear();
    for (int i : idx) {
        if (!fhCache.empty() && fhCache.back() == NF[i]) {
            auto& R = Rset.back();
            for (int u = 1; u <= n; u++) R[u] |= NR[i][u];
        } else {
            fhCache.push_back(std::move(NF[i]));
            Rset.push_back(std::move(NR[i]));
        }
    }
}
double walkProb(ll a, int d) {
    // P(|a + S_d| == 1), S_d sum of d random +-1
    double p = 0;
    for (int t : {1, -1}) {
        ll need = t - a;  // S_d = need
        if ((need + d) % 2 != 0 || llabs(need) > d) continue;
        int k = (need + d) / 2;
        p += exp(lgamma(d + 1.0) - lgamma(k + 1.0) - lgamma(d - k + 1.0) - d * log(2.0));
    }
    return p;
}
double TOGGLE_SLACK = 2.0, TOGGLE_MINENT = 0.6;
int TOGGLE_MAXCL = 12;
void doToggle(int x) {
    toggleQ(x);
    togglesDone++;
    int HN = fhCache.size();
    vector<vector<ll>> NF;
    vector<vector<char>> NR;
    for (int h = 0; h < HN; h++) {
        auto& f = fhCache[h];
        auto& R = Rset[h];
        // classes: 0 = A (outside subtree x), 1 = C (x), 2.. = B_y
        vector<int> classes;
        if (x != G) classes.push_back(-1);
        classes.push_back(0);
        for (int y : ch[x]) classes.push_back(y);
        for (int cl : classes) {
            vector<char> nr(n + 1, 0);
            bool any = false;
            for (int u = 1; u <= n; u++) {
                if (!R[u]) continue;
                bool in;
                if (cl == -1) in = !inSub(x, u);
                else if (cl == 0) in = (u == x);
                else in = inSub(cl, u);
                if (in) { nr[u] = 1; any = true; }
            }
            if (!any) continue;
            // determine v_old(x) under this class, if possible
            int vold = 0;  // 0 = unknown
            if (cl == -1) { if (procd[x]) vold = (int)(f[x] - f[gpar[x]]); }
            else if (cl == 0) { if (procd[x]) vold = (int)f[x]; }
            else { if (procd[cl]) vold = (int)(f[x] - f[cl]); }
            vector<int> nvs;
            if (vold != 0) nvs.push_back(-vold);
            else nvs = {1, -1};
            for (int nv : nvs) {
                vector<ll> nf = f;
                bool changed = false;
                for (int u = 1; u <= n; u++) {
                    if (!procd[u]) continue;
                    bool in;
                    if (cl == -1) in = inSub(x, u);
                    else if (cl == 0) in = true;
                    else in = !inSub(cl, u);
                    if (in) { nf[u] += 2 * nv; changed = true; }
                }
                NF.push_back(std::move(nf));
                NR.push_back(nr);
                if (!changed) break;  // nv irrelevant
            }
        }
    }
    mergeHyps(NF, NR);
}
// candidate weights per node
vector<double> candWeights() {
    int HN = fhCache.size();
    vector<double> W(n + 1, 0);
    vector<int> anc(n + 1, 0);
    for (int u : ord) anc[u] = procd[u] ? u : anc[gpar[u]];
    for (int h = 0; h < HN; h++)
        for (int u = 1; u <= n; u++) {
            if (!Rset[h][u]) continue;
            double p;
            if (procd[u]) p = (llabs(fhCache[h][u]) == 1) ? 1.0 : 0.0;
            else {
                int a = anc[u];
                if (a == 0) p = 0.3;
                else p = walkProb(fhCache[h][a], dep[u] - dep[a]);
            }
            W[u] += p / HN;
        }
    return W;
}
void maybeToggle(int remaining) {
    if (getenv("NOTOG")) return;
    int HN = fhCache.size();
    if (HN > 2 || remaining == 0) return;
    vector<double> W = candWeights();
    double Wtot = 0;
    for (int u = 1; u <= n; u++) Wtot += W[u];
    if (Wtot < 1.5) return;
    double Rq = remaining / 11.0;
    if (getenv("DBG4")) fprintf(stderr, "mt: HN=%d rem=%d Wtot=%.1f Rq=%.1f\n", HN, remaining, Wtot, Rq);
    if (Rq > log2(Wtot) + TOGGLE_SLACK) return;
    vector<double> S(n + 1, 0);
    for (int i = n - 1; i >= 0; i--) {
        int u = ord[i];
        S[u] += W[u];
        if (gpar[u]) S[gpar[u]] += S[u];
    }
    // R-count subtree sums (union over hypotheses)
    vector<int> Rc(n + 1, 0), RS(n + 1, 0);
    for (int u = 1; u <= n; u++) {
        for (int h = 0; h < HN; h++) if (Rset[h][u]) { Rc[u] = 1; break; }
    }
    int Rtot = 0;
    for (int u = 1; u <= n; u++) Rtot += Rc[u];
    for (int i = n - 1; i >= 0; i--) {
        int u = ord[i];
        RS[u] += Rc[u];
        if (gpar[u]) RS[gpar[u]] += RS[u];
    }
    double bestE = -1;
    int bx = -1;
    for (int x = 1; x <= n; x++) {
        double E = 0;
        int ncl = 0;
        auto add = [&](double w, int rc) { if (rc > 0) ncl++; if (w > 1e-12) { double p = w / Wtot; E -= p * log2(p); } };
        if (x != G) add(Wtot - S[x], Rtot - RS[x]);
        add(W[x], Rc[x]);
        for (int y : ch[x]) add(S[y], RS[y]);
        if (ncl > TOGGLE_MAXCL) continue;
        if (E > bestE) { bestE = E; bx = x; }
    }
    if (getenv("DBG4")) fprintf(stderr, "mt: bx=%d bestE=%.2f\n", bx, bestE);
    if (bx < 0) return;
    if (bestE < TOGGLE_MINENT) return;
    doToggle(bx);
    if (getenv("DBG3")) fprintf(stderr, "toggle %d E=%.2f Wtot=%.1f H=%zu\n", bx, bestE, Wtot, fhCache.size());
}


// ---------------- high-confidence batch (HB) mode ----------------
int hbCooldown = 0;
double HB_QTHR = 0.04, HB_QSUM = 0.4;
int HB_MAXM = 32, HB_MINM = 14;
// returns true if an HB query was executed
bool tryHB(Sim& sim, int& remaining) {
    if (getenv("NOHB")) return false;
    if (!priorActive || fhCache.size() != 1 || remaining < HB_MINM) return false;
    if (hbCooldown > 0) { hbCooldown--; return false; }
    auto& f = fhCache[0];
    // predictions
    vector<int> pv(n + 1, 0);
    vector<double> qv(n + 1, 1.0);
    vector<char> certain(n + 1, 0);
    for (int u : ord) {
        if (procd[u]) continue;
        int p = gpar[u];
        double pp;
        if (procd[p]) {
            if (p == G) pp = prM;
            else pp = (f[p] - f[gpar[p]]) > 0 ? prPP : prPM;
        } else {
            if (!certain[p]) continue;
            pp = pv[p] > 0 ? prPP : prPM;
        }
        pv[u] = pp >= 0.5 ? 1 : -1;
        qv[u] = min(pp, 1 - pp);
        certain[u] = qv[u] <= HB_QTHR;
    }
    // block non-certain nodes in sim
    for (int u = 1; u <= n; u++) sim.kn[u] = procd[u] || !certain[u];
    sim.step++;
    sim.computeHeights();
    vector<int> fr;
    for (int u = 1; u <= n; u++)
        if (!sim.kn[u] && procd[gpar[u]]) fr.push_back(u);
    if (fr.empty()) return false;
    sort(fr.begin(), fr.end(), [&](int a, int b) { return sim.hgt[a] > sim.hgt[b]; });
    int availC = 0;
    for (int u = 1; u <= n; u++) if (!sim.kn[u]) availC++;
    QPlan best;
    bool have = false;
    for (int M = min(HB_MAXM, availC); M >= HB_MINM && !have; M--) {
        vector<ll> D0;
        for (int i = 0; i < M; i++) D0.push_back(2 * M - 1 - i);
        QPlan P;
        ll cost;
        if (!sim.embed(D0, fr, P, cost)) continue;
        double lam = min(4.0, (double)n / cost);
        vector<ll> D;
        for (ll e : D0) D.push_back(llround(e * lam));
        bool distinct = true;
        for (size_t i = 1; i < D.size(); i++) if (D[i] == D[i - 1]) distinct = false;
        if (!distinct || D.back() < 1) continue;
        QPlan P2;
        ll c2;
        if (!sim.embed(D, fr, P2, c2) || c2 > n) continue;
        double qs = 0;
        for (int u : P2.nodes) qs += qv[u];
        if (qs > HB_QSUM) continue;
        best = P2;
        have = true;
    }
    if (!have) return false;
    int m = best.nodes.size();
    vector<ll> mass(m);
    for (int i = 0; i < m; i++) mass[i] = best.coef[i];
    for (int i = 0; i < m; i++) if (best.parentIdx[i] >= 0) mass[best.parentIdx[i]] -= best.coef[i];
    vector<pair<int, int>> Sv;
    for (int i = 0; i < m; i++) if (mass[i] > 0) Sv.push_back({best.nodes[i], (int)mass[i]});
    ll ans = askQ(Sv);
    ll base = 0, sigma = 0;
    for (int i = 0; i < m; i++) {
        sigma += best.coef[i];
        if (best.parentIdx[i] < 0) base += best.coef[i] * f[gpar[best.nodes[i]]];
    }
    ll Dv = ans - base;
    const int CAP = 32;
    vector<ll> sols;  // masks (bit i = +1)
    if (((Dv + sigma) % 2 + 2) % 2 == 0) {
        ll target = (Dv + sigma) / 2;
        int h1 = m / 2, h2 = m - h1;
        vector<pair<ll, int>> B(1 << h2);
        for (int s2 = 0; s2 < (1 << h2); s2++) {
            ll t = 0;
            for (int i = 0; i < h2; i++) if (s2 >> i & 1) t += best.coef[h1 + i];
            B[s2] = {t, s2};
        }
        sort(B.begin(), B.end());
        for (int s1 = 0; s1 < (1 << h1) && (int)sols.size() <= CAP; s1++) {
            ll t = 0;
            for (int i = 0; i < h1; i++) if (s1 >> i & 1) t += best.coef[i];
            auto it = lower_bound(B.begin(), B.end(), make_pair(target - t, INT_MIN));
            for (; it != B.end() && it->first == target - t && (int)sols.size() <= CAP; ++it)
                sols.push_back((ll)s1 | ((ll)it->second << h1));
        }
    }
    if (getenv("DBG3")) fprintf(stderr, "HB m=%d k=%lld sols=%zu\n", m, (ll)0, sols.size());
    if (sols.empty() || (int)sols.size() > CAP) {
        hbCooldown = 3;
        return true;  // query wasted, nothing learned
    }
    vector<vector<ll>> NF;
    vector<vector<char>> NR;
    for (ll mask : sols) {
        vector<ll> nf = f;
        for (int i = 0; i < m; i++) {
            int u = best.nodes[i];
            nf[u] = nf[gpar[u]] + ((mask >> i & 1) ? 1 : -1);
        }
        NF.push_back(std::move(nf));
        NR.push_back(Rset[0]);
    }
    for (int i = 0; i < m; i++) { procd[best.nodes[i]] = 1; remaining--; }
    mergeHyps(NF, NR);
    return true;
}

void onlineValues() {
    procd.assign(n + 1, 0);
    fhCache.assign(1, vector<ll>(n + 1, 0));
    Rset.assign(1, vector<char>(n + 1, 1));
    Fval = 0;
    togglesDone = 0;
    Sim sim(polNP, polSP, polAP, 4242);
    if (getenv("BETA")) BETA = atof(getenv("BETA"));
    if (getenv("USEX")) useX = true;
    if (getenv("TSLACK")) TOGGLE_SLACK = atof(getenv("TSLACK"));
    if (getenv("TMINENT")) TOGGLE_MINENT = atof(getenv("TMINENT"));
    if (getenv("TMAXCL")) TOGGLE_MAXCL = atoi(getenv("TMAXCL"));
    int remaining = n;
    bool first = true;
    while (true) {
        int HN = fhCache.size();
        if (remaining == 0) break;
        if (!first) estimatePrior();
        if (!first && tryHB(sim, remaining)) { maybeToggle(remaining); continue; }
        // best resolver nodes: processed nodes whose f differs across hypotheses
        vector<pair<double, int>> resCands;
        if (HN > 1) {
            for (int u = 1; u <= n; u++) {
                if (!procd[u]) continue;
                bool diff = false;
                for (int h = 1; h < HN; h++) if (fhCache[h][u] != fhCache[0][u]) { diff = true; break; }
                if (!diff) continue;
                map<ll, int> cnt;
                for (int h = 0; h < HN; h++) cnt[fhCache[h][u]]++;
                if (cnt.size() <= 1) continue;
                double e = 0;
                for (auto& [x, c] : cnt) { double p = (double)c / HN; e -= p * log2(p); }
                resCands.push_back({-e, u});
            }
            sort(resCands.begin(), resCands.end());
        }
        // greedy joint resolver nodes
        vector<int> rnodes;
        if (HN > 1) {
            vector<ll> keyv(HN, 0);
            vector<char> usedR(n + 1, 0);
            for (int r = 0; r < 5; r++) {
                double bestE = -1;
                int bu = -1;
                double curE;
                {
                    map<ll, int> cnt;
                    for (int h = 0; h < HN; h++) cnt[keyv[h]]++;
                    curE = 0;
                    for (auto& [x, c] : cnt) { double p = (double)c / HN; curE -= p * log2(p); }
                }
                int lim = 0;
                for (auto& rc : resCands) {
                    if (++lim > 60) break;
                    int u = rc.second;
                    if (usedR[u]) continue;
                    map<ll, int> cnt;
                    for (int h = 0; h < HN; h++) cnt[keyv[h] * 1000003LL + fhCache[h][u]]++;
                    double e = 0;
                    for (auto& [x, c] : cnt) { double p = (double)c / HN; e -= p * log2(p); }
                    if (e > bestE) { bestE = e; bu = u; }
                }
                if (bu < 0 || bestE <= curE + 1e-9) break;
                usedR[bu] = 1;
                rnodes.push_back(bu);
                for (int h = 0; h < HN; h++) keyv[h] = keyv[h] * 1000003LL + fhCache[h][bu];
            }
        }
        Cand best;
        bool haveBest = false;
        bool bestSafe = false;  // satisfies cap
        auto consider = [&](Cand& c) {
            if (priorActive) evalWeighted(c);
            else if (HN > 1) evalExact(c);
            bool safe = c.worst <= HCAP;
            double pend = c.pend >= 0 ? c.pend : (log2((double)HN) + c.P.nodes.size() - c.ent);
            c.score = c.ent - BETA * pend;
            bool better;
            if (!haveBest) better = true;
            else if (safe != bestSafe) better = safe;
            else if (!safe) better = c.worst < best.worst;
            else better = c.score > best.score + 1e-9 ||
                          (fabs(c.score - best.score) < 1e-9 && c.P.nodes.size() > best.P.nodes.size());
            if (better) { best = c; haveBest = true; bestSafe = safe; }
        };
        double lhn = log2((double)HN);
        if (remaining > 0) {
            for (int u = 1; u <= n; u++) sim.kn[u] = procd[u];
            sim.step++;
            sim.computeHeights();
            vector<int> fr;
            if (first) fr = ch[G];
            else
                for (int u = 1; u <= n; u++)
                    if (!procd[u] && procd[gpar[u]]) fr.push_back(u);
            sort(fr.begin(), fr.end(), [&](int a, int b) { return sim.hgt[a] > sim.hgt[b]; });
            int avail = remaining - (first ? 1 : 0);
            for (int m = min(15, avail + (int)rnodes.size()); m >= 1; m--) {
                if (haveBest && bestSafe && min(m, avail) + lhn + 1e-9 < best.score) break;
                vector<vector<ll>> fams = lib[m];
                if (useX) for (auto& D : libX[m]) fams.push_back(D);
                for (auto& D0 : fams) {
                    for (int r = 0; r <= (int)rnodes.size() && r < m; r++) {
                        if (m - r > avail) continue;
                        vector<ll> D0m(D0.begin(), D0.end() - r);
                        QPlan P;
                        ll cost;
                        if (!sim.embed(D0m, fr, P, cost)) continue;
                        ll sigma0 = 0, extra0 = 0;
                        for (ll e : D0m) sigma0 += e;
                        for (int j = m - r; j < m; j++) extra0 += D0[j];
                        double lam = first ? min(1.0, (double)(n - 2) / sigma0) : min(1.0, (double)n / (cost + extra0));
                        for (int attempt = 0; attempt < 6; attempt++, lam *= 0.985) {
                            vector<ll> D;
                            for (ll e : D0) D.push_back(max<ll>(1, llround(e * lam)));
                            sort(D.rbegin(), D.rend());
                            vector<ll> Dm(D.begin(), D.end() - r);
                            QPlan P2;
                            ll c2;
                            if (!sim.embed(Dm, fr, P2, c2)) break;
                            ll sg = 0, ex = 0;
                            for (ll e : Dm) sg += e;
                            for (int j = m - r; j < m; j++) ex += D[j];
                            ll k = c2 + ex;
                            if (first) { k = max(k, sg + 1); if (k % 2 == 0) k++; }
                            if (k > n) continue;
                            Cand c;
                            c.P = P2; c.D = Dm; c.k = k;
                            for (int j = 0; j < r; j++) c.res.push_back({rnodes[j], D[m - r + j]});
                            const HistInfo& hi = getHist(Dm);
                            c.ent = hi.ent; c.worst = hi.worst * HN;
                            consider(c);
                            break;
                        }
                    }
                }
            }
        }
        if (HN > 1) {
            // resolver-only
            for (int r = 1; r <= (int)rnodes.size(); r++)
                for (auto& D0 : lib[r]) {
                    ll sg = 0;
                    for (ll e : D0) sg += e;
                    if (sg > n) continue;
                    Cand c;
                    c.D = {};
                    c.k = sg;
                    for (int i = 0; i < r; i++) c.res.push_back({rnodes[i], D0[i]});
                    c.ent = 0; c.worst = HN;
                    consider(c);
                }
        }
        if (first) {
            // always-feasible fallback: query G alone (learn F only)
            Cand c;
            c.D = {};
            c.k = 1;
            c.ent = 0;
            c.worst = 1;
            consider(c);
        }
        Cand c = best;
        // the resolver elements are not part of the plan's new nodes: D for decoding = plan coefs
        int m = c.P.nodes.size();
        vector<ll> mass(m);
        for (int i = 0; i < m; i++) mass[i] = c.P.coef[i];
        for (int i = 0; i < m; i++) if (c.P.parentIdx[i] >= 0) mass[c.P.parentIdx[i]] -= c.P.coef[i];
        map<int, ll> S;
        ll tot = 0;
        for (int i = 0; i < m; i++) if (mass[i] > 0) { S[c.P.nodes[i]] += mass[i]; tot += mass[i]; }
        for (auto& r : c.res) { S[r.first] += r.second; tot += r.second; }
        if (first && c.k - tot > 0) S[G] += c.k - tot;
        vector<pair<int, int>> Sv;
        for (auto& [u, mm] : S) Sv.push_back({u, (int)mm});
        ll ans = askQ(Sv);
        // decode
        vector<ll> coefs = c.P.coef;
        ll sigma = 0;
        for (ll e : coefs) sigma += e;
        if (first) {
            ll k = c.k;
            ll Fpar = ((ans - sigma) % 2 + 2) % 2;
            ll F0 = (ll)floor((double)ans / k);
            ll F = F0;
            for (ll cc = F0 - 3; cc <= F0 + 3; cc++) {
                if (((cc % 2) + 2) % 2 != Fpar) continue;
                if (llabs(ans - k * cc) <= sigma) { F = cc; break; }
            }
            Fval = F;
            procd[G] = 1;
            remaining--;
            fhCache[0][G] = F;
        }
        // sums -> masks
        vector<pair<ll, int>> sm(1 << m);
        for (int mask = 0; mask < (1 << m); mask++) {
            ll t = 0;
            for (int i = 0; i < m; i++) if (mask >> i & 1) t += coefs[i];
            sm[mask] = {t, mask};
        }
        sort(sm.begin(), sm.end());
        vector<vector<ll>> NF;
        vector<vector<char>> NR;
        for (int h = 0; h < HN; h++) {
            ll s = 0;
            for (int i = 0; i < m; i++)
                if (c.P.parentIdx[i] < 0) s += coefs[i] * fhCache[h][gpar[c.P.nodes[i]]];
            for (auto& r : c.res) s += r.second * fhCache[h][r.first];
            if (first) s += (c.k - tot) * Fval;  // mass on G
            ll Dv = ans - s;
            if (((Dv + sigma) % 2 + 2) % 2 != 0) continue;
            ll target = (Dv + sigma) / 2;
            auto it = lower_bound(sm.begin(), sm.end(), make_pair(target, INT_MIN));
            for (; it != sm.end() && it->first == target; ++it) {
                vector<ll> nf = fhCache[h];
                for (int i = 0; i < m; i++) {
                    int u = c.P.nodes[i];
                    nf[u] = nf[gpar[u]] + ((it->second >> i & 1) ? 1 : -1);
                }
                NF.push_back(std::move(nf));
                NR.push_back(Rset[h]);
            }
        }
        for (int i = 0; i < m; i++) {
            procd[c.P.nodes[i]] = 1;
            remaining--;
        }
        if (NF.empty()) { NF.push_back(fhCache[0]); NR.push_back(Rset[0]); }  // should not happen
        first = false;
        mergeHyps(NF, NR);
        if (getenv("DBG3")) fprintf(stderr, "q m=%d ent=%.2f H=%zu res=%zu k=%lld\n", m, c.ent, fhCache.size(), c.res.size(), c.k);
        maybeToggle(remaining);
    }
}

void rootAt(int g) {
    G = g;
    gpar.assign(n + 1, 0);
    dep.assign(n + 1, 0);
    ch.assign(n + 1, {});
    tin.assign(n + 1, 0);
    tout.assign(n + 1, 0);
    ord.clear();
    vector<int> st = {G};
    vector<size_t> it(n + 1, 0);
    int timer = 0;
    tin[G] = timer++;
    ord.push_back(G);
    while (!st.empty()) {
        int u = st.back();
        if (it[u] < adj[u].size()) {
            int v = adj[u][it[u]++];
            if (v == gpar[u]) continue;
            gpar[v] = u;
            dep[v] = dep[u] + 1;
            ch[u].push_back(v);
            tin[v] = timer++;
            ord.push_back(v);
            st.push_back(v);
        } else {
            tout[u] = timer - 1;
            st.pop_back();
        }
    }
    LOG = 1;
    while ((1 << LOG) <= n) LOG++;
    up.assign(LOG, vector<int>(n + 1, 0));
    for (int u = 1; u <= n; u++) up[0][u] = gpar[u] ? gpar[u] : u;
    for (int j = 1; j < LOG; j++)
        for (int u = 1; u <= n; u++) up[j][u] = up[j - 1][up[j - 1][u]];
}

int main() {
    auto tStart = chrono::steady_clock::now();
    if (getenv("PB")) PLAN_BUDGET = atof(getenv("PB"));
    if (scanf("%d", &n) != 1) return 0;
    adj.assign(n + 1, {});
    for (int i = 0; i < n - 1; i++) {
        int a, b;
        if (scanf("%d %d", &a, &b) != 2) return 0;
        adj[a].push_back(b);
        adj[b].push_back(a);
    }
    buildLib();
    // choose G = endpoint of diameter
    auto bfs = [&](int s) {
        vector<int> d(n + 1, -1);
        queue<int> q;
        q.push(s);
        d[s] = 0;
        int last = s;
        while (!q.empty()) {
            int u = q.front(); q.pop(); last = u;
            for (int v : adj[u]) if (d[v] < 0) { d[v] = d[u] + 1; q.push(v); }
        }
        return last;
    };
    int dA = bfs(1), dB = bfs(dA);
    vector<int> cands = {dB, dA};
    {
        vector<int> leaves;
        for (int u = 1; u <= n; u++) if (adj[u].size() == 1 && u != dA && u != dB) leaves.push_back(u);
        mt19937 r2(777);
        shuffle(leaves.begin(), leaves.end(), r2);
        for (int u : leaves) cands.push_back(u);
        // also some non-leaves
        vector<int> others;
        for (int u = 1; u <= n; u++) if (adj[u].size() > 1) others.push_back(u);
        shuffle(others.begin(), others.end(), r2);
        for (int i = 0; i < (int)others.size() && i < 3; i++) cands.push_back(others[i]);
    }
    vector<QPlan> plans;
    {
        auto t0 = chrono::steady_clock::now();
        int bestG = -1;
        for (size_t ci = 0; ci < cands.size(); ci++) {
            rootAt(cands[ci]);
            auto p = planValues(false);
            if (getenv("DBG2")) fprintf(stderr, "g=%d deg=%zu -> %zu\n", cands[ci], adj[cands[ci]].size(), p.size());
            if (bestG < 0 || p.size() < plans.size()) { plans = p; bestG = cands[ci]; polNP = curNP; polSP = curSP; polAP = curAP; }
            if (chrono::duration<double>(chrono::steady_clock::now() - t0).count() > PLAN_BUDGET) break;
        }
        rootAt(bestG);
        auto p = planValues(true);
        if (p.size() < plans.size()) { plans = p; polNP = curNP; polSP = curSP; polAP = curAP; }
        if (getenv("DBG")) {
            fprintf(stderr, "plan=%zu sizes:", plans.size());
            for (auto& q : plans) fprintf(stderr, " %zu", q.nodes.size());
            fprintf(stderr, "\n");
        }
    }
    auto tPlan = chrono::steady_clock::now();
    if (getenv("OLD")) executePlans(plans); else onlineValues();
    auto tOnline = chrono::steady_clock::now();
    if (getenv("DBG")) fprintf(stderr, "time plan=%.2f online=%.2f\n", chrono::duration<double>(tPlan - tStart).count(), chrono::duration<double>(tOnline - tPlan).count());

    int valQ=qcount;

    // ---------------- phase 2: root / hypothesis resolution ----------------
    if (getenv("OLD")) {
        fhCache.assign(1, vector<ll>(n + 1));
        for (int u : ord) fhCache[0][u] = (u == G ? wv[G] : fhCache[0][gpar[u]] + wv[u]);
        Rset.assign(1, vector<char>(n + 1, 1));
    }
    struct Hyp { int r; vector<signed char> v; vector<int> f; int grp; };
    vector<Hyp> hs;
    for (size_t hi = 0; hi < fhCache.size(); hi++) {
        auto& f0 = fhCache[hi];
        for (int r = 1; r <= n; r++) {
            if (!Rset[hi][r] || llabs(f0[r]) != 1) continue;
            Hyp h;
            h.r = r;
            h.v.assign(n + 1, 0);
            h.f.assign(n + 1, 0);
            for (int u = 1; u <= n; u++) {
                h.f[u] = (int)f0[u];
                h.v[u] = (u == r) ? (signed char)f0[u] : (signed char)(f0[u] - f0[sideOf(u, r)]);
            }
            hs.push_back(std::move(h));
        }
    }
    {
        map<vector<signed char>, int> mp;
        for (auto& h : hs) {
            auto it = mp.find(h.v);
            if (it == mp.end()) { int id = mp.size(); mp[h.v] = id; h.grp = id; }
            else h.grp = it->second;
        }
    }
    if (getenv("DBG")) fprintf(stderr, "valQ=%d hyps=%zu ", valQ, hs.size());
    vector<int> alive(hs.size());
    iota(alive.begin(), alive.end(), 0);
    mt19937_64 rng(12345);

    // objective: expected log2(#groups) after partition by key
    auto evalPart = [&](const vector<ll>& key, const vector<int>& grp) -> double {
        int A = key.size();
        vector<int> idx(A);
        iota(idx.begin(), idx.end(), 0);
        sort(idx.begin(), idx.end(), [&](int a, int b) { return key[a] != key[b] ? key[a] < key[b] : grp[a] < grp[b]; });
        double E = 0;
        int i = 0;
        while (i < A) {
            int j = i, g = 0;
            while (j < A && key[j < A ? idx[j] : 0] == key[idx[i]]) {
                if (j == i || grp[idx[j]] != grp[idx[j - 1]]) g++;
                j++;
            }
            E += (double)(j - i) * log2((double)g);
            i = j;
        }
        return E / A;
    };
    // greedy S builder; F[i] = f-vector of hypothesis i (pointer), returns S and objective
    auto buildS = [&](const vector<const int*>& F, const vector<int>& grp, double& outE) {
        int A = F.size();
        vector<int> varying;
        for (int u = 1; u <= n; u++) {
            int a = F[0][u];
            for (int i = 1; i < A; i++) if (F[i][u] != a) { varying.push_back(u); break; }
        }
        vector<pair<int, int>> S;
        vector<ll> cur(A, 0);
        double E = evalPart(cur, grp);
        int budget = n;
        static const int mus[] = {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377};
        for (int step = 0; step < 40 && budget > 0 && E > 1e-12 && !varying.empty(); step++) {
            vector<int> cand = varying;
            if (cand.size() > 50) {
                shuffle(cand.begin(), cand.end(), rng);
                cand.resize(50);
            }
            double bestE = E;
            int bu = -1, bm = 0;
            vector<ll> key(A);
            for (int u : cand)
                for (int mu : mus) {
                    if (mu > budget) break;
                    for (int i = 0; i < A; i++) key[i] = cur[i] + (ll)mu * F[i][u];
                    double e = evalPart(key, grp);
                    if (e < bestE - 1e-12 || (bu >= 0 && fabs(e - bestE) < 1e-12 && mu < bm)) { bestE = e; bu = u; bm = mu; }
                }
            if (bu < 0) break;
            for (int i = 0; i < A; i++) cur[i] += (ll)bm * F[i][bu];
            S.push_back({bu, bm});
            budget -= bm;
            E = bestE;
        }
        if (S.empty()) S.push_back({1, 1});
        outE = E;
        return S;
    };

    while (true) {
        int A = alive.size();
        vector<int> grp(A);
        {
            set<int> gs;
            for (int i = 0; i < A; i++) { grp[i] = hs[alive[i]].grp; gs.insert(grp[i]); }
            if (gs.size() <= 1) break;
        }
        double E0;
        {
            vector<ll> z(A, 0);
            E0 = evalPart(z, grp);
        }
        // option: no toggle
        double bestRate = -1;
        int bestX = 0;
        vector<pair<int, int>> bestS;
        {
            vector<const int*> F(A);
            for (int i = 0; i < A; i++) F[i] = hs[alive[i]].f.data();
            double e;
            auto S = buildS(F, grp, e);
            double rate = (E0 - e) / 1.0;
            if (rate > bestRate + 1e-9) { bestRate = rate; bestX = 0; bestS = S; }
        }
        // toggle options: rank x by side-class score
        vector<pair<double, int>> xs;
        for (int x = 1; x <= n; x++) {
            vector<ll> key(A);
            for (int i = 0; i < A; i++) {
                int h = alive[i];
                key[i] = (ll)sideOf(x, hs[h].r) * 4 + (hs[h].v[x] + 1);
            }
            xs.push_back({evalPart(key, grp), x});
        }
        sort(xs.begin(), xs.end());
        int tryX = min<int>(4, xs.size());
        vector<vector<int>> nfBest;
        for (int t = 0; t < tryX; t++) {
            int x = xs[t].second;
            if ((E0 - xs[t].first) / 2.0 <= bestRate + 1e-9) continue;
            vector<vector<int>> nf(A);
            for (int i = 0; i < A; i++) {
                auto& H = hs[alive[i]];
                nf[i] = H.f;
                int nv = -H.v[x];
                int r = H.r;
                int s = sideOf(x, r);
                for (int u = 1; u <= n; u++) {
                    bool in;
                    if (r == x) in = true;
                    else if (s == gpar[x]) in = inSub(x, u);
                    else in = !inSub(s, u);
                    if (in) nf[i][u] += 2 * nv;
                }
            }
            vector<const int*> F(A);
            for (int i = 0; i < A; i++) F[i] = nf[i].data();
            double e;
            auto S = buildS(F, grp, e);
            double rate = (E0 - e) / 2.0;
            if (rate > bestRate + 1e-9) { bestRate = rate; bestX = x; bestS = S; nfBest = std::move(nf); }
        }
        if (getenv("DBG")) fprintf(stderr, "[A=%d E0=%.2f x=%d rate=%.2f |S|=%zu] ", A, E0, bestX, bestRate, bestS.size());
        if (bestX) {
            toggleQ(bestX);
            for (int i = 0; i < A; i++) {
                auto& H = hs[alive[i]];
                H.f = nfBest[i];
                H.v[bestX] = -H.v[bestX];
            }
        }
        ll ans = askQ(bestS);
        vector<int> nal;
        for (int i = 0; i < A; i++) {
            auto& H = hs[alive[i]];
            ll q = 0;
            for (auto& pr : bestS) q += (ll)pr.second * H.f[pr.first];
            if (q == ans) nal.push_back(alive[i]);
        }
        if (nal.empty()) break;  // should not happen
        alive = nal;
    }
    if (getenv("DBG")) fprintf(stderr, "total=%d\n", qcount);
    string s = "!";
    for (int u = 1; u <= n; u++) { s += ' '; s += to_string(alive.empty() ? 1 : (int)hs[alive[0]].v[u]); }
    s += '\n';
    fputs(s.c_str(), stdout);
    fflush(stdout);
    return 0;
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef __int128 LL;
static const ll BX = 100000;
static chrono::steady_clock::time_point T0;
static double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }

static double ask(ll x1, ll y1, ll x2, ll y2) {
    printf("query %lld %lld %lld %lld\n", x1, y1, x2, y2);
    fflush(stdout);
    char buf[128];
    if (scanf("%127s", buf) != 1) exit(0);
    char* e;
    double v = strtod(buf, &e);
    if (e == buf || v < 0) exit(0);
    return v;
}

struct Cand { ll r, y0, xlo, xhi; };

// predicted chord of full line (0,p)-(BX,q)
static inline double predict(ll p, ll q, ll r, ll y0, ll x0) {
    ll a = BX, b = q - p;
    LL D = (LL)a * a + (LL)b * b;
    LL N = (LL)b * x0 - (LL)a * (y0 - p);
    LL M = D * r * r - N * N;
    if (M <= 0) return 0.0;
    return 2.0 * (double)sqrtl((long double)M / (long double)D);
}

static const double SEP = 3.2e-6;   // min separation for distinguishable predictions
static const double TOL = 1.6e-6;   // filtering tolerance

static ll totalCount(const vector<Cand>& c) {
    ll s = 0;
    for (auto& x : c) s += x.xhi - x.xlo + 1;
    return s;
}

// expected extra cost after a query (lower is better)
struct Eval { double cost; };

static double evalHorizontal(const vector<Cand>& C, ll c) {
    // classes by H'
    map<ll, pair<ll, int>> cls; // H -> (weight, number of cands)
    ll tot = 0;
    for (auto& x : C) {
        ll d = c - x.y0;
        ll H = max(0LL, x.r * x.r - d * d);
        ll w = x.xhi - x.xlo + 1;
        auto& e = cls[H];
        e.first += w; e.second++;
        tot += w;
    }
    double cost = 0;
    for (auto& [h, e] : cls) {
        double pr = (double)e.first / tot;
        if (e.second == 1) {
            if (e.first > 1) cost += pr * 1.0;
        } else cost += pr * 2.0;
    }
    return cost;
}

static vector<double> predBuf;
static const double MINL = 10.0; // avoid near-tangent chords (precision safety)
static double evalSlanted(const vector<Cand>& C, ll p, ll q, double bestSoFar) {
    predBuf.clear();
    ll small = 0;
    for (int i = 0; i < (int)C.size(); i++) {
        auto& x = C[i];
        for (ll x0 = x.xlo; x0 <= x.xhi; x0++) {
            double v = predict(p, q, x.r, x.y0, x0);
            if (v > 0 && v < MINL) small++;
            predBuf.push_back(v);
        }
    }
    ll tot = predBuf.size();
    double cost = (double)small / tot;
    if (cost >= bestSoFar) return cost;
    sort(predBuf.begin(), predBuf.end());
    size_t i = 0, n = predBuf.size();
    while (i < n) {
        size_t j = i + 1;
        while (j < n && predBuf[j] - predBuf[j - 1] < SEP) j++;
        if (j - i > 1 && predBuf[i] >= MINL) {
            cost += (double)(j - i) / tot;
            if (cost >= bestSoFar) return cost;
        } else if (j - i > 1 && predBuf[i] == 0) {
            ll z = 0; for (size_t k = i; k < j && predBuf[k] == 0; k++) z++;
            if (z > 1) cost += (double)z / tot;
            ll rest = (j - i) - z; // small ones already counted
            (void)rest;
            if (cost >= bestSoFar) return cost;
        }
        i = j;
    }
    return cost;
}

static inline long double Lof(LL N, LL D, ll r) {
    LL M = D * r * r - N * N;
    if (M <= 0) return 0.0L;
    return 2.0L * sqrtl((long double)M / (long double)D);
}
// pessimistic cost: mass of candidates not guaranteed uniquely decodable
static double fastCost(const vector<Cand>& C, ll p, ll q) {
    ll a = BX, b = q - p;
    LL D = (LL)a * a + (LL)b * b;
    LL ab = b < 0 ? -b : b;
    int n = C.size();
    static vector<long double> lo, hi; static vector<char> dirty; static vector<int> idx;
    lo.assign(n, 0); hi.assign(n, 0); dirty.assign(n, 0); idx.resize(n);
    ll tot = 0;
    for (int i = 0; i < n; i++) {
        auto& x = C[i];
        ll W = x.xhi - x.xlo + 1; tot += W;
        LL N1 = (LL)b * x.xlo - (LL)a * (x.y0 - p), N2 = (LL)b * x.xhi - (LL)a * (x.y0 - p);
        LL A1 = N1 < 0 ? -N1 : N1, A2 = N2 < 0 ? -N2 : N2;
        LL mn = min(A1, A2), mx = max(A1, A2);
        bool cross = (N1 < 0 && N2 > 0) || (N1 > 0 && N2 < 0);
        if (cross) { mn = 0; dirty[i] = 1; }
        long double Lhi = Lof(mn, D, x.r), Llo = Lof(mx, D, x.r);
        lo[i] = Llo; hi[i] = Lhi;
        if (Llo < MINL && !(W == 1 && Lhi <= 0)) dirty[i] = 1;
        if (W > 1) {
            if (Llo <= 0) dirty[i] = 1;
            else if (!cross) {
                long double sp = Lhi - Lof(mn + ab, D, x.r);
                if (sp < SEP) dirty[i] = 1;
            }
        }
    }
    for (int i = 0; i < n; i++) idx[i] = i;
    sort(idx.begin(), idx.end(), [&](int i, int j) { return lo[i] < lo[j]; });
    long double runHi = -1; int runIdx = -1;
    for (int k = 0; k < n; k++) {
        int i = idx[k];
        if (runIdx >= 0 && lo[i] - runHi < SEP) { dirty[i] = 1; dirty[runIdx] = 1; }
        if (hi[i] > runHi) { runHi = hi[i]; runIdx = i; }
    }
    // overlapping with earlier (non-max) intervals is covered since runHi tracks max
    ll bad = 0;
    for (int i = 0; i < n; i++) if (dirty[i]) bad += C[i].xhi - C[i].xlo + 1;
    return (double)bad / tot;
}

static void filterResult(vector<Cand>& C, ll p, ll q, double L) {
    vector<Cand> out;
    if (p == q) {
        ll H = (L < 1.0) ? 0 : llround(L * L / 4.0);
        for (auto& x : C) {
            ll d = p - x.y0;
            ll h = max(0LL, x.r * x.r - d * d);
            if (h == H) out.push_back(x);
        }
    } else {
        double bestDiff = 1e18; Cand bestC{0, 0, 0, 0};
        for (auto& x : C) {
            ll runStart = -1;
            for (ll x0 = x.xlo; x0 <= x.xhi + 1; x0++) {
                bool ok = false;
                if (x0 <= x.xhi) {
                    double pr = predict(p, q, x.r, x.y0, x0);
                    double df = fabs(pr - L);
                    if (df < bestDiff) { bestDiff = df; bestC = {x.r, x.y0, x0, x0}; }
                    ok = df <= TOL;
                }
                if (ok) { if (runStart < 0) runStart = x0; }
                else if (runStart >= 0) { out.push_back({x.r, x.y0, runStart, x0 - 1}); runStart = -1; }
            }
        }
        if (out.empty()) out.push_back(bestC);
    }
    if (out.empty()) out = C; // should not happen
    C = out;
}

static mt19937_64 rng(12345);

static bool validLine(ll p, ll q) { return p >= 0 && p <= BX && q >= 0 && q <= BX; }

int main() {
    T0 = chrono::steady_clock::now();
    // ---------- Phase 1: find a horizontal line hitting the disk ----------
    static const int SEQ[] = {46720,75546,24850,62426,87475,38171,12329,31014,56064,93639,68587,81113,18489,6162,43538,34195,52486,71769,49901,21670,84294,96819,9145,65407,2983,59245,15308,90259,27833,40358,79523,35787,54472,77932,92048,45129,73360,29425,10737,48309,60834,20078,82702,57654,32605,23260,85885,98410,4572,41948,70179,66999,16900,95228,7555,1391,13718,63817,26242,36979,88866,51491,76341,53677,38967,91252,74156,49105,58450,2187,16104,69383,28629,55268,20874,61630,66203,83498,9941,96024,92844,27037,70975,80317,78728,97614,77136,34991,45924,50696,14512,31809,11533,42744,22466,33399,56859,60039,99205,47515,85089,30220,94434,44334,19285,86680,5368,8349,24056,64613,67795,81909,41153,3776,72764,17496,89662,6958,796,88070,25646,63021,39562,13121,74951,36581,53083,37576,51889,17892,72366,23658,90854,80715,20476,11931,14910,49503,39960,73758,89264,38569,35389,33797,16502,78330,1789,62028,58052,63419,9543,69781,77534,42346,84692,398,2585,28231,66601,32207,81511,67397,98012,95626,91650,85487,8747,64215,46322,10339,29027,54870,18887,4970,65805,43936,15706,54075,4174,70577,36183,55666,83100,79125,48707,88468,75944,60437,92446,21272,11135,68985,50299,61232,44732,58847,3379,27435,6560,76738,96421,25248,94832,22068,34593,83896,45526,98807,31412,29822,99602,41550,56461,94036,97217,74554,14114,87078,26640,86282,22862,33003,47913,51093,30618,82305,57256,19682,79920,43142,47117,65009,7953,24454,93242,71371,5766,40756,12723,68193,59641,89861,39363,73161,53478,37377,17297,7356,75347,37775,18290,87871,90458,995,52287,25845,72167,52685,62822,13320,25447,16701,62227,61033,71570,66004,1988,48508,6361,78131,97813,81312,2784,57455,63220,98211,2386,57853,47316,4373,11334,93043,60238,30021,25049,47714,45725,97018,32406,52088,91849,83299,32008,67198,94235,64016,77335,4771,76937,31213,89065,50100,34792,21869,86481,8946,33996,37178,11732,38370,81710,52884,36382,74355,44533,23459,54671,55069,90060,72565,67994,42943,69980,40159,23857,20675,15109,55865,88269,69184,99801,85288,65208,83697,56660,33598,91451,32804,9742,63618,199,19086,51292,15507,82503,12130,28828,61829,66800,18091,69582,91053,24255,7754,29226,9344,44135,75745,49702,15905,14313,28032,12922,53876,40557,46123,13519,59046,61431,20277,7157,39761,80914,84493,80516,50895,71968,30419,66402,10936,16303,40954,38768,1590,95825,48906,87673,86083,35190,49304,78529,51690,92247,10140,27236,44931,64414,35588,85686,27634,89463,28430,597,42147,73957,70378,14711,79324,21073,88667,84095,45327,50498,5567,46521,68786,76142,73559,18688,6759,70776,99006,92645,22267,94633,34394,29623,41749,87276,86879,57058,42545,3975,1193,3578,55467,10538,36780,5169,33200,60635,64811,41351,65606,96620,82901,98609,26441,59840,17693,95030,95427,58251,58649,30817,43737,77733,37972,3181,97416,99403,67596,82108,74753,93440,84891,78927,26839,56263,13916,26044,31610,19880,80119,48111,71174,90655,23061,5964,12528,68392,8548,59444,93837,62623,76540,22665,17098,19484,75148,21471,8152,46919,43341,53280,96222,79721,39166,54273,72962,35985,24652};
    vector<ll> missed;
    ll hitC = -1, lo = 0, hi = BX;
    double hitL = 0;
    for (int c : SEQ) {
        double L = ask(0, c, BX, c);
        if (L > 1.0) { hitC = c; hitL = L; break; }
        missed.push_back(c);
    }
    if (hitC >= 0) for (ll m : missed) { if (m < hitC) lo = max(lo, m); else hi = min(hi, m); }
    if (hitC < 0) { printf("answer 50000 50000 100\n"); fflush(stdout); return 0; }
    // ---------- Phase 2: candidate enumeration ----------
    ll H = llround(hitL * hitL / 4.0);
    vector<Cand> C;
    for (ll u = 1; u * u <= H; u++) {
        if (H % u) continue;
        ll v = H / u;
        if ((u + v) & 1) continue;
        ll r = (u + v) / 2, t = (v - u) / 2;
        if (r < 100 || r > BX / 2) continue;
        for (int s = -1; s <= 1; s += 2) {
            if (t == 0 && s == 1) continue;
            ll y0 = hitC + s * t;
            if (y0 - r < lo || y0 + r > hi) continue;
            if (y0 - r < 0 || y0 + r > BX) continue;
            C.push_back({r, y0, r, BX - r});
        }
    }
    const ll NMAX = 1500000;
    while (true) {
        ll tot = totalCount(C);
        if (tot <= 1) break;
        // choose query
        ll bp = -1, bq = -1; double bestCost = 1e18;
        // horizontal options
        {
            ll ymin = LLONG_MAX, ymax = LLONG_MIN, rmax = 0;
            for (auto& x : C) { ymin = min(ymin, x.y0); ymax = max(ymax, x.y0); rmax = max(rmax, x.r); }
            vector<ll> cs;
            for (ll d = -3; d <= 3; d++) cs.push_back(hitC + d);
            for (auto& x : C) for (ll d = -2; d <= 2; d++) cs.push_back(x.y0 + d);
            ll A = max(0LL, ymin - rmax), Bb = min(BX, ymax + rmax);
            ll steps = max(1LL, (ll)(3000000 / max<size_t>(1, C.size())));
            steps = min(steps, 4000LL);
            for (ll k = 0; k <= steps; k++) cs.push_back(A + (Bb - A) * k / steps);
            sort(cs.begin(), cs.end()); cs.erase(unique(cs.begin(), cs.end()), cs.end());
            for (ll c : cs) {
                if (c < 0 || c > BX) continue;
                double cost = evalHorizontal(C, c) + 1e-9; // slight preference to slanted on ties
                if (cost < bestCost) { bestCost = cost; bp = bq = c; }
            }
        }
        if (bestCost > 0) {
            ll ymin = LLONG_MAX, ymax = LLONG_MIN, rmax = 0;
            for (auto& x : C) { ymin = min(ymin, x.y0 - x.r); ymax = max(ymax, x.y0 + x.r); rmax = max(rmax, x.r); }
            vector<int> order(C.size());
            iota(order.begin(), order.end(), 0);
            sort(order.begin(), order.end(), [&](int i, int j) { return C[i].xhi - C[i].xlo > C[j].xhi - C[j].xlo; });
            uniform_real_distribution<double> U01(0, 1);
            auto genOpt = [&](int k, ll& pi, ll& qi) -> bool {
                int fam = k % 3;
                if (fam == 0) {
                    // small/medium tilt around the hit line
                    double mag = exp(U01(rng) * log(2.0 * rmax + 2));
                    ll bb = (ll)mag; if (bb < 1) bb = 1;
                    if (rng() & 1) bb = -bb;
                    double dm = exp(U01(rng) * log(1.5 * rmax + 2)) - 1;
                    ll d = (ll)dm; if (rng() & 1) d = -d;
                    pi = hitC + d; qi = pi + bb;
                } else if (fam == 1) {
                    auto& x = C[order[(k / 3) % min<int>(order.size(), 8)]];
                    double al = 0.02 + 0.97 * U01(rng), be = 0.02 + 0.97 * U01(rng);
                    int side = (rng() & 1) ? 1 : -1;
                    double h1 = x.y0 - side * al * x.r, h2 = x.y0 - side * be * x.r;
                    double sl;
                    if (x.xhi > x.xlo) sl = (h2 - h1) / (double)(x.xhi - x.xlo);
                    else sl = (U01(rng) * 2 - 1) * (double)x.r / 1000.0;
                    double p = h1 - sl * x.xlo, q = p + sl * BX;
                    if (!(fabs(p) < 1e9 && fabs(q) < 1e9)) return false;
                    pi = llround(p); qi = llround(q);
                } else {
                    pi = ymin + rng() % (ymax - ymin + 1); qi = pi + (ll)((U01(rng) * 2 - 1) * (ymax - ymin + 1));
                }
                return pi != qi && validLine(pi, qi);
            };
            // fast screening
            vector<pair<double, pair<ll, ll>>> top;
            const int K = 6;
            double tStart = elapsed();
            double screenBudget = (tot <= 1000 ? 0.15 : 0.5);
            if (elapsed() > 6.0) screenBudget = 0.05;
            ll tries = 0;
            for (int k = 0;; k++) {
                if ((k & 63) == 0 && elapsed() - tStart > screenBudget) break;
                if (k > 3000000) break;
                ll pi, qi;
                if (!genOpt(k, pi, qi)) continue;
                tries++;
                double fc = fastCost(C, pi, qi);
                if (fc <= 0) { top.clear(); top.push_back({0, {pi, qi}}); break; }
                if ((int)top.size() < K || fc < top.back().first) {
                    bool dup = false;
                    for (auto& t : top) if (t.second == make_pair(pi, qi)) dup = true;
                    if (!dup) {
                        top.push_back({fc, {pi, qi}});
                        sort(top.begin(), top.end());
                        if ((int)top.size() > K) top.pop_back();
                    }
                }
            }
            if (!top.empty() && top[0].first <= 0) {
                bestCost = 0; bp = top[0].second.first; bq = top[0].second.second;
            } else if (tot <= NMAX && elapsed() < 5.5) {
                for (auto& t : top) {
                    if (elapsed() - tStart > 2.5 || elapsed() > 7.0) break;
                    double cost = evalSlanted(C, t.second.first, t.second.second, bestCost);
                    if (cost < bestCost) { bestCost = cost; bp = t.second.first; bq = t.second.second; }
                }
            } else if (!top.empty() && top[0].first < bestCost) {
                bestCost = top[0].first; bp = top[0].second.first; bq = top[0].second.second;
            }
        }
        double L = ask(0, bp, BX, bq);
        filterResult(C, bp, bq, L);
    }
    auto& x = C[0];
    printf("answer %lld %lld %lld\n", x.xlo, x.y0, x.r);
    fflush(stdout);
    return 0;
}

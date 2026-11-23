#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const double NMAX = 1e14;
#ifndef PG
#define PG 16
#define PRS 64
#define PRK 32
#define PROCT 3
#define PLIN 4
#endif
static const int LS = 256, G = PG;          // lo grid: exact up to LS, then G points per octave
static const int RS = PRS, RLIN = PLIN * RS, RK = PRK, ROCT = PROCT; // r grid: linear step 1/RS up to PLIN, then RK per octave
static const int NR = RLIN + RK * ROCT + 1;
static int NI;
static vector<double> Lv, Rv;
static vector<vector<float>> tab;   // tab[i][q*NR + j]
static vector<int> qsat;            // tab[i] valid for q < qsat[i]; q>=qsat[i] => saturated
static const float INF = 4e14f;

static inline int lidx(double lo) {
    if (lo <= LS) return (int)lo - 1;
    int i = LS - 1 + (int)floor(log2(lo / LS) * G + 1e-9);
    return min(i, NI - 1);
}
static inline int ridxr(double r) {
    if (r < (double)PLIN) { int j = (int)floor(r * RS + 1e-9); return j < 0 ? 0 : j; }
    int j = RLIN + (int)floor(log2(r / PLIN) * RK + 1e-9);
    return min(j, NR - 1);
}
static inline double WI(int q, int i, double B) {
    if (q < 0) return -1;
    if (B < 0) return -1;
    if (q >= qsat[i]) return INF;
    return tab[i][(size_t)q * NR + ridxr(B / Lv[i])];
}
static inline double Wf(int q, double lo, double B) {
    if (q < 0 || B < 0) return -1;
    int i = lidx(lo);
    if (q >= qsat[i]) return INF;
    return tab[i][(size_t)q * NR + ridxr(B / lo)];
}

// max d in [1,Dmax] with d-1 <= W(q, lo, B-lo-d); 0 if none
static ll maxFailD(int q, double lo, double B) {
    ll Dmax = (ll)floor(B - lo + 1e-6);
    if (Dmax < 1 || q < 0) return 0;
    int i = lidx(lo);
    if (q >= qsat[i]) return Dmax;
    const float* w = &tab[i][(size_t)q * NR];
    // candidate for bucket j: min(B-lo-R[j]*lo, w[j]+1); first term dec, second nondec in j
    int jmax = ridxr((B - lo - 1) / lo);
    if (B - lo - 1 < 0) return 0;
    auto t1 = [&](int j) { return floor(B - lo - Rv[j] * lo + 1e-6); };
    auto t2 = [&](int j) { return floor((double)w[j]) + 1; };
    // find first j where t2 >= t1
    int a = 0, b = jmax;
    if (t2(b) < t1(b)) a = b; // never crosses
    else {
        while (a < b) { int m = (a + b) / 2; if (t2(m) >= t1(m)) b = m; else a = m + 1; }
    }
    double best = 0;
    for (int j = max(0, a - 1); j <= min(jmax, a); j++) best = max(best, min(t1(j), t2(j)));
    ll d = (ll)min(best, (double)Dmax);
    // verify exactly with lookups
    while (d >= 1 && !((double)(d - 1) <= Wf(q, lo, B - lo - d))) d--;
    // try to extend (rounding)
    while (d < Dmax && (double)d <= Wf(q, lo, B - lo - d - 1)) d++;
    return d;
}

static void build() {
    for (int i = 1; i <= LS; i++) Lv.push_back(i);
    while (true) {
        double v = ceil(LS * pow(2.0, (double)(Lv.size() - LS + 1) / G));
        Lv.push_back(v);
        if (v > 2e14) break;
    }
    NI = Lv.size();
    for (int j = 0; j < RLIN; j++) Rv.push_back((double)j / RS);
    for (int j = 0; j <= RK * ROCT; j++) Rv.push_back(PLIN * pow(2.0, (double)j / RK));
    tab.assign(NI, {});
    qsat.assign(NI, 1 << 30);
    for (int i = 0; i < NI; i++) tab[i].assign(NR, 0.0f), qsat[i] = 1 << 30;
    for (int q = 1;; q++) {
        bool any = false;
        vector<char> satNow(NI, 0);
        for (int i = 0; i < NI; i++) {
            if (qsat[i] <= q) continue;
            double lo = Lv[i];
            double cap = NMAX - lo + 1;
            if (cap < 1) cap = 1;
            size_t base = tab[i].size();
            tab[i].resize(base + NR);
            bool allsat = true;
            for (int j = 0; j < NR; j++) {
                double B = floor(lo * Rv[j] + 1e-9);
                double best = WI(q - 1, i, B + lo);
                ll d = maxFailD(q - 1, lo, B);
                if (d >= 1) {
                    double v = (double)d + Wf(q - 1, lo + d, B + lo + d);
                    best = max(best, v);
                }
                if (j > 0) best = max(best, (double)tab[i][base + j - 1]);
                if (best >= cap) {
                    for (int k = j; k < NR; k++) tab[i][base + k] = INF;
                    if (j == 0) allsat = true;
                    break;
                }
                allsat = false;
                { float f = (float)best; if ((double)f > best) f = nextafterf(f, 0.0f); tab[i][base + j] = f; }
            }
            if (allsat) { tab[i].resize(base); satNow[i] = 1; }
            else any = true;
        }
#ifndef NOMONO
        for (int i = NI - 2; i >= 0; i--) {
            if (qsat[i] <= q || satNow[i]) continue;
            if (qsat[i + 1] <= q || satNow[i + 1]) continue;
            float* a = &tab[i][(size_t)q * NR];
            float* b = &tab[i + 1][(size_t)q * NR];
            for (int j = 0; j < NR; j++) a[j] = min(a[j], b[j]);
        }
#endif
        for (int i = 0; i < NI; i++) if (satNow[i]) qsat[i] = q;
        if (!any) break;
    }
}

// ---------------- policy ----------------
#ifndef PLOGW
#define PLOGW 0.0
#endif
#ifndef PLLC
#define PLLC 1.0
#endif
#ifndef BUDGET
#define BUDGET 0
#endif
#ifndef OPTAVG
#define OPTAVG 1
#endif
struct Policy {
    ll lo = 0, hi = (ll)NMAX, B = 1;
    int used = 0;
    bool done() const { return lo == hi; }
    int needQ() const {
        if (lo == hi) return 0;
        if (lo == 0) return 1 + (int)1e9; // not used
        int q = 0;
        while (Wf(q, (double)lo, (double)B) < (double)(hi - lo)) q++;
        return q;
    }
    static int minQ(ll lo, ll hi, ll B) {
        if (lo == hi) return 0;
        int q = 1;
        while (Wf(q, (double)lo, (double)B) < (double)(hi - lo)) q++;
        return q;
    }
    // fractional estimate of remaining queries
    static double estQ(ll lo, ll hi, ll B) {
        if (lo == hi) return 0;
        int q = minQ(lo, hi, B);
        double w1 = Wf(q - 1, (double)lo, (double)B), w2 = Wf(q, (double)lo, (double)B);
        double need = (double)(hi - lo);
        if (w2 > 1e15) w2 = max(need, w1) * 2 + 2;
        double f = (log(need + 1) - log(max(w1, 0.0) + 1)) / (log(w2 + 1) - log(max(w1, 0.0) + 1) + 1e-12);
        f = min(max(f, 0.0), 1.0);
        return q - 1 + f;
    }
    // P(M >= X | M in [lo,hi]) under a mixture of uniform and log-uniform priors
    double probSucc(ll X) const {
        double pu = (double)(hi - X + 1) / (double)(hi - lo + 1);
        auto Gf = [](double v) { return log(PLLC + log(1.0 + v)); };
        double a = Gf((double)lo), b = Gf((double)hi + 1), x = Gf((double)X);
        double pl = (b - x) / (b - a);
        return (1 - PLOGW) * pu + PLOGW * pl;
    }
    ll next() {  // returns X to query
        used++;
        if (lo == 0) return 1;
        ll need = hi - lo;
        for (int q = max(minQ(lo, hi, B), BUDGET - used + 1);; q++) {
            ll dmax = min(maxFailD(q - 1, (double)lo, (double)B), need);
            auto succOK = [&](ll d) {
                return d == need || (double)d + Wf(q - 1, (double)(lo + d), (double)(B + lo + d)) >= (double)need;
            };
            bool refillOK = Wf(q - 1, (double)lo, (double)(B + lo)) >= (double)need;
            if (dmax >= 1 && succOK(dmax)) {
                if (!OPTAVG) return lo + dmax;
                // smallest feasible d
                ll a = 1, b = dmax;
                while (a < b) { ll m = a + (b - a) / 2; if (succOK(m)) b = m; else a = m + 1; }
                ll dmin = a;
                double bestc = 1e18; ll bestX = lo + dmax;
                const int K = 48;
                for (int k = 0; k <= K; k++) {
                    ll d = dmin + (ll)((long double)(dmax - dmin) * k / K);
                    if (!succOK(d)) continue;
                    ll X = lo + d;
                    double ps = probSucc(X);
                    double c = ps * estQ(X, hi, B + X) + (1 - ps) * estQ(lo, X - 1, B - X);
                    if (c < bestc) { bestc = c; bestX = X; }
                }
                if (refillOK) {
                    double c = estQ(lo, hi, B + lo);
                    if (c < bestc) return lo;
                }
                return bestX;
            }
            if (refillOK) return lo;
            if (q > 400) { // safety fallback (should never happen)
                if (B >= lo + 1) return lo + max(1LL, min(need, (B - lo) / 2));
                return lo;
            }
        }
    }
    void update(ll X, bool ok) {
        if (ok) { B += X; lo = max(lo, X); }
        else { B -= X; hi = min(hi, X - 1); }
    }
};

#ifndef LOCAL
int main() {
    build();
    Policy p;
    char buf[64];
    while (!p.done()) {
        ll X = p.next();
        printf("? %lld\n", X);
        fflush(stdout);
        if (scanf("%63s", buf) != 1) return 0;
        string s(buf);
        if (s == "Lucky!") p.update(X, true);
        else if (s == "Fraudster!") p.update(X, false);
        else return 0;
    }
    printf("! %lld\n", p.lo);
    fflush(stdout);
    return 0;
}
#endif

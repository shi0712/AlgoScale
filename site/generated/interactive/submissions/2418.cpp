#include <bits/stdc++.h>
using namespace std;
typedef long long ll; typedef unsigned long long ull;
static const ll NMAX = 1000000000LL;

#ifdef LOCAL
ll SECRET, CUR; int QCNT;
int ask(ll x) {
    QCNT++;
    if (x < 1 || x > CUR) { fprintf(stderr, "BAD x=%lld cur=%lld secret=%lld\n", x, CUR, SECRET); exit(1); }
    CUR -= x; return __builtin_popcountll(CUR);
}
#else
int ask(ll x) {
    printf("- %lld\n", x); fflush(stdout);
    int r; if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
}
#endif

static mt19937_64 rng(12345);
static ll LIMIT = 3000000;

ll comb(int n, int k) { if (k < 0 || k > n) return 0; ll r = 1; for (int i = 1; i <= k; i++) r = r * (n - k + i) / i; return r; }


static ll CURSZ = 0;
static int NRAND = 3000, NTOP = 10, SMALLE = 20000;
static double HB = 13.5, HA = 0.38;
static int LA1 = 3000, LA2 = 300, LAK = 6;
static inline double hcost(ll n) {
    if (n <= 1) return 0;
    return max(1.0, HA + log((double)n) / log(HB));
}
// expected remaining cost (excluding this query) * |E| (over first lim)
static double scoreX(const vector<ll>& E, size_t lim, ll x) {
    int bucket[64] = {0};
    size_t n = min(lim, E.size());
    for (size_t i = 0; i < n; i++) bucket[__builtin_popcountll(E[i] - x)]++;
    double h = 0;
    for (int k = 0; k < 64; k++) if (bucket[k] > 1) h += bucket[k] * hcost(bucket[k]);
    return h;
}
// returns top candidates sorted by heuristic score
vector<pair<double, ll>> topX(vector<ll>& E, ll M, int nrand, int ntop, int keep) {
    shuffle(E.begin(), E.end(), rng);
    vector<ll> xs;
    auto add = [&](ll x) { if (x >= 1 && x <= M) xs.push_back(x); };
    if (M <= 4096) { for (ll x = 1; x <= M; x++) add(x); }
    else {
        for (ll x = 1; x <= 64; x++) add(x);
        for (int j = 0; j < 31; j++) {
            ll q = 1LL << j;
            add(q); add(q - 1); add(q + 1); add(M - q); add(M - q + 1); add(M & ~(q - 1)); add((M & ~(q - 1)) - 1);
            add(M ^ q);
        }
        add(M);
        for (int i = 0; i < nrand; i++) add(1 + (ll)(rng() % M));
    }
    sort(xs.begin(), xs.end()); xs.erase(unique(xs.begin(), xs.end()), xs.end());
    size_t lim = SMALLE;
    vector<pair<double, ll>> sc;
    for (ll x : xs) sc.push_back({scoreX(E, lim, x), x});
    sort(sc.begin(), sc.end());
    if ((int)sc.size() > ntop) sc.resize(ntop);
    set<ll> seen;
    vector<pair<double, ll>> fin;
    for (auto [s0, x0] : sc) {
        ll x = x0; double s = s0;
        if (M > 4096) for (int it = 0; it < 50; it++) {
            ll bxn = -1; double bs = s;
            for (int j = 0; j < 30; j++) {
                ll y = x ^ (1LL << j);
                if (y < 1 || y > M) continue;
                double t = scoreX(E, lim, y);
                if (t < bs - 1e-9) { bs = t; bxn = y; }
            }
            for (ll d : {-1LL, 1LL}) { ll y = x + d; if (y >= 1 && y <= M) { double t = scoreX(E, lim, y); if (t < bs - 1e-9) { bs = t; bxn = y; } } }
            if (bxn < 0) break;
            x = bxn; s = bs;
        }
        if (seen.insert(x).second) fin.push_back({scoreX(E, E.size(), x), x});
        if (!seen.count(x0)) { seen.insert(x0); fin.push_back({scoreX(E, E.size(), x0), x0}); }
    }
    sort(fin.begin(), fin.end());
    if ((int)fin.size() > keep) fin.resize(keep);
    return fin;
}
// exact-ish expected cost with lookahead; E = current values (all candidates)
pair<double, ll> lookahead(vector<ll> E, int depth) {
    if (E.size() <= 1) return {0, 1};
    ll M = *min_element(E.begin(), E.end());
    int K = depth > 0 ? LAK : 1;
    auto c = topX(E, M, depth > 0 ? 400 : 300, 6, K);
    double bestc = 1e18; ll bx = c[0].second;
    for (auto [s, x] : c) {
        double cost;
        if (depth == 0) cost = 1 + s / E.size();
        else {
            vector<vector<ll>> g(64);
            for (ll m : E) g[__builtin_popcountll(m - x)].push_back(m - x);
            cost = 1;
            for (auto& v : g) if (v.size() > 1) {
                cost += (double)v.size() / E.size() * lookahead(v, depth - 1).first;
                if (cost >= bestc) break;
            }
        }
        if (cost < bestc - 1e-12) { bestc = cost; bx = x; }
        if (bestc <= 1 + 1e-12) break;
    }
    return {bestc, bx};
}
ll chooseX(vector<ll>& E, ll M, bool exact) {
    if (exact && (int)E.size() <= LA2) return lookahead(E, 2).second;
    if (exact && (int)E.size() <= LA1) return lookahead(E, 1).second;
    return topX(E, M, NRAND, NTOP, 1)[0].second;
}

ll solve(int cnt0) {
    vector<pair<ll,int>> hist; // popcount(n - T) == r
    double TOTAL = 0;
    TOTAL = comb(30, cnt0);
    ll T = 0;
    vector<uint32_t> S; bool mat = false;
    vector<int> bucket(32);
    while (true) {
        vector<ll> E; ll M = LLONG_MAX;
        if (!mat && hist.size() > 11) {
            // fallback: full ordered scan (never expected in practice)
            S.clear();
            ll H = (1LL << cnt0) - 1;
            while (H <= NMAX) {
                bool ok = true;
                for (auto& [t, r] : hist) if (__builtin_popcountll(H - t) != r) { ok = false; break; }
                if (ok) S.push_back((uint32_t)H);
                ll u = H & -H, v = H + u;
                H = v + (((v ^ H) / u) >> 2);
            }
            mat = true;
        }
        if (!mat) {
            int q = hist.size();
            const int NHMAX = (int)(NMAX >> 15); const int NLLIM = (int)(NMAX & 32767);
            vector<pair<ull, uint16_t>> lo(32768);
            for (int nl = 0; nl < 32768; nl++) {
                ull key = __builtin_popcount(nl);
                for (int j = 0; j < q; j++) {
                    int tl = hist[j].first & 32767; int b = nl < tl; int low = (nl - tl) & 32767;
                    key |= (ull)((b << 4) | __builtin_popcount(low)) << (4 + 5 * j);
                }
                lo[nl] = {key, (uint16_t)nl};
            }
            sort(lo.begin(), lo.end());
            vector<ull> keys(32768); for (int i = 0; i < 32768; i++) keys[i] = lo[i].first;
            struct Mt { int nh, a, b; };
            vector<Mt> mts; ll total = 0;
            for (int nh = 0; nh <= NHMAX; nh++) {
                int pch = __builtin_popcount(nh); if (pch > cnt0 || cnt0 - pch > 15) continue;
                ull base = cnt0 - pch;
                auto rec = [&](auto&& self, int j, ull key) -> void {
                    if (j == q) {
                        auto it = lower_bound(keys.begin(), keys.end(), key);
                        if (it == keys.end() || *it != key) return;
                        int a = it - keys.begin();
                        int b = upper_bound(keys.begin() + a, keys.end(), key) - keys.begin();
                        if (nh == NHMAX) {
                            int bb = a; while (bb < b && lo[bb].second <= NLLIM) bb++;
                            b = bb;
                        }
                        if (b > a) { mts.push_back({nh, a, b}); total += b - a; }
                        return;
                    }
                    ll th = hist[j].first >> 15;
                    for (int bo = 0; bo < 2; bo++) {
                        ll h = nh - th - bo; if (h < 0) continue;
                        int pl = hist[j].second - __builtin_popcountll(h);
                        if (pl < 0 || pl > 15) continue;
                        self(self, j + 1, key | ((ull)((bo << 4) | pl) << (4 + 5 * j)));
                    }
                };
                rec(rec, 0, base);
            }
            CURSZ = total;
            if (total <= LIMIT) {
                S.clear(); S.reserve(total);
                for (auto& m : mts) for (int i = m.a; i < m.b; i++) S.push_back(((uint32_t)m.nh << 15) | lo[i].second);
                sort(S.begin(), S.end());
                mat = true;
            } else {
                // min
                ll mn = LLONG_MAX;
                for (auto& m : mts) { if (m.nh != mts[0].nh) break; mn = min(mn, ((ll)m.nh << 15) | lo[m.a].second); }
                M = mn - T;
                vector<ll> pre(mts.size() + 1, 0);
                for (size_t i = 0; i < mts.size(); i++) pre[i + 1] = pre[i] + (mts[i].b - mts[i].a);
                const int RS = 60000;
                for (int i = 0; i < RS; i++) {
                    ll idx = rng() % total;
                    size_t k = upper_bound(pre.begin(), pre.end(), idx) - pre.begin() - 1;
                    auto& m = mts[k];
                    E.push_back((((ll)m.nh << 15) | lo[m.a + (idx - pre[k])].second) - T);
                }
            }
        }
        if (mat) {
            CURSZ = S.size();
            if (S.size() == 1) { if (getenv("SZLOG")) fprintf(stderr, "SZ 1\n"); return S[0]; }
            M = LLONG_MAX;
            for (auto n : S) M = min(M, (ll)n - T);
            if (S.size() > 100000) { for (int i = 0; i < 100000; i++) E.push_back((ll)S[rng() % S.size()] - T); }
            else for (auto n : S) E.push_back((ll)n - T);
        }
        auto tq0 = chrono::steady_clock::now();
        ll bx = chooseX(E, M, mat && S.size() == E.size());
        if (getenv("PROF")) fprintf(stderr, "step mat=%d S=%zu E=%zu choose=%.2f\n", (int)mat, S.size(), E.size(), chrono::duration<double>(chrono::steady_clock::now() - tq0).count());
        double best = 0;
        if (getenv("LOGX")) fprintf(stderr, "M=%lld x=%lld (%s) cnt=%zu H=%.3f\n", M, bx, bitset<30>(bx).to_string().c_str(), E.size(), best);
        if (getenv("SZLOG")) fprintf(stderr, "SZ %lld\n", CURSZ);
        int r = ask(bx);
        T += bx;
        hist.push_back({T, r});
        if (mat) {
            vector<uint32_t> S2;
            for (auto n : S) if (__builtin_popcountll((ll)n - T) == r) S2.push_back(n);
            S.swap(S2);
        }
    }
}

int main(int argc, char** argv) {
#ifdef LOCAL
    int trials = atoi(argv[1]);
    if (argc > 2) LIMIT = atoll(argv[2]);
    if (getenv("NRAND")) NRAND = atoi(getenv("NRAND"));
    if (getenv("NTOP")) NTOP = atoi(getenv("NTOP"));
    if (getenv("SMALLE")) SMALLE = atoi(getenv("SMALLE"));
    if (getenv("HB")) HB = atof(getenv("HB"));
    if (getenv("HA")) HA = atof(getenv("HA"));
    if (getenv("LA1")) LA1 = atoi(getenv("LA1"));
    if (getenv("LA2")) LA2 = atoi(getenv("LA2"));
    if (getenv("LAK")) LAK = atoi(getenv("LAK"));
    mt19937_64 g(getenv("SEED") ? atoll(getenv("SEED")) : 777);
    vector<double> sum(31); vector<int> num(31);
    double tot = 0, totc = 0;
    for (int it = 0; it < trials; it++) {
        ll n = 1 + g() % NMAX;
        if (getenv("NLIST")) { static vector<ll> nl; static bool ld = false; if (!ld) { ld = true; FILE* f = fopen(getenv("NLIST"), "r"); ll v; while (fscanf(f, "%lld", &v) == 1) nl.push_back(v); fclose(f);} if (it >= (int)nl.size()) break; n = nl[it]; }
        if (getenv("FORCEC")) { int fc = atoi(getenv("FORCEC")); while (__builtin_popcountll(n) != fc) n = 1 + g() % NMAX; }
        auto t0 = chrono::steady_clock::now();
        SECRET = CUR = n; QCNT = 0;
        int c = __builtin_popcountll(n);
        ll a = solve(c);
        if (a != n) { fprintf(stderr, "WRONG n=%lld got %lld\n", n, a); return 1; }
        double el = chrono::duration<double>(chrono::steady_clock::now() - t0).count();
        if (getenv("TIME")) fprintf(stderr, "n=%lld c=%d q=%d t=%.2f\n", n, c, QCNT, el);
        sum[c] += QCNT; num[c]++; tot += QCNT; totc += c;
    }
    for (int c = 1; c <= 30; c++) if (num[c]) printf("c=%d n=%d avg=%.3f\n", c, num[c], sum[c] / num[c]);
    printf("avg q=%.4f avg cnt=%.4f\n", tot / trials, totc / trials);
#else
    int c; if (scanf("%d", &c) != 1) return 0;
    if (c < 0) return 0;
    ll a = solve(c);
    printf("! %lld\n", a); fflush(stdout);
#endif
}

#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

double C0=6, PW=1.2; int FS=1; double A2=1.5, B2=0.2;
struct Solver {
    ll N, x0, y0;
    function<int(ll, ll)> rawq;  // returns 1/0, or -1 on failure
    int cnt = 0; int cA,cB,cC,cD;
    bool dead = false;
    map<pair<ll, ll>, bool> cache;

    bool ask(ll x, ll y) {
        if (x < 1 || y < 1 || x > N || y > N) return false;
        auto it = cache.find({x, y});
        if (it != cache.end()) return it->second;
        int r = rawq(x, y);
        cnt++;
        if (r < 0) { dead = true; throw 0; }
        cache[{x, y}] = r;
        return r;
    }

    // x-direction state
    vector<pair<ll, bool>> PR, PL;  // probes right/left
    ll lo_r = 0, hi_r, lo_l = 0, hi_l, mlo = 3, mhi;
    ll Mmax;

    static ll cdiv(ll a, ll b) { return a >= 0 ? (a + b - 1) / b : -((-a) / b); }
    static ll oddUp(ll v) { return (v % 2 == 0) ? v + 1 : v; }
    static ll oddDown(ll v) { return (v % 2 == 0) ? v - 1 : v; }

    void update() {
        for (int it = 0; it < 100; it++) {
            ll a = lo_r, b = hi_r, c = lo_l, d = hi_l, e = mlo, f = mhi;
            for (auto [p, v] : PR) {
                if (v) {
                    if (p <= lo_r + mlo) lo_r = max(lo_r, p);
                    else {
                        mlo = max(mlo, cdiv(p + 1, 5));
                        mlo = max(mlo, cdiv(p - hi_r, 4));
                    }
                } else hi_r = min(hi_r, p - 1);
            }
            for (auto [p, v] : PL) {
                if (v) {
                    if (p <= lo_l + mlo) lo_l = max(lo_l, p);
                    else {
                        mlo = max(mlo, cdiv(p + 1, 5));
                        mlo = max(mlo, cdiv(p - hi_l, 4));
                    }
                } else hi_l = min(hi_l, p - 1);
            }
            mlo = max({mlo, lo_r + lo_l + 1, (ll)3});
            mlo = oddUp(mlo);
            mhi = min(mhi, hi_r + hi_l + 1);
            mhi = oddDown(mhi);
            lo_r = max(lo_r, mlo - 1 - hi_l);
            lo_l = max(lo_l, mlo - 1 - hi_r);
            hi_r = min(hi_r, mhi - 1 - lo_l);
            hi_l = min(hi_l, mhi - 1 - lo_r);
            if (a == lo_r && b == hi_r && c == lo_l && d == hi_l && e == mlo && f == mhi) break;
        }
    }
    // -1 unknown, 0 false, 1 true
    int knownR(ll p) {
        if (p <= lo_r) return 1;
        if (x0 + p > N) return 0;
        if (p > hi_r && p <= lo_r + mlo) return 0;
        if (p > hi_r + 4 * mhi) return 0;
        return -1;
    }
    int knownL(ll p) {
        if (p <= lo_l) return 1;
        if (x0 - p < 1) return 0;
        if (p > hi_l && p <= lo_l + mlo) return 0;
        if (p > hi_l + 4 * mhi) return 0;
        return -1;
    }
    bool probeR(ll p) {
        int k = knownR(p);
        if (k >= 0) return k;
        bool v = ask(x0 + p, y0);
        PR.push_back({p, v});
        update();
        return v;
    }
    bool probeL(ll p) {
        int k = knownL(p);
        if (k >= 0) return k;
        bool v = ask(x0 - p, y0);
        PL.push_back({p, v});
        update();
        return v;
    }
    int known(bool r, ll p) { return r ? knownR(p) : knownL(p); }
    bool probe(bool r, ll p) { return r ? probeR(p) : probeL(p); }
    ll& LO(bool r) { return r ? lo_r : lo_l; }
    ll& HI(bool r) { return r ? hi_r : hi_l; }
    void expSearch(bool r, bool isFirst) {
        ll s0 = LO(r) + mlo;
        vector<ll> pts;
        while (true) {
            ll p = pts.empty() ? s0 : pts.back() * 2;
            if (p > HI(r)) break;
            pts.push_back(p);
        }
        int K = pts.size();
        if (K == 0) return;
        int E = K + 1;  // leaves 0..K
        vector<double> w(E), pre(E + 1, 0);
        for (int l = 0; l < E; l++) {
            if (isFirst) w[l] = pow(1.0 / (l + C0), PW);
            else w[l] = max(pow(2.0, -A2 * l), B2 * pow(1.0 / (l + 1), 2.0));
            pre[l + 1] = pre[l] + w[l];
        }
        vector<vector<double>> cost(E, vector<double>(E, 0));
        vector<vector<int>> sp(E, vector<int>(E, -1));
        for (int len = 2; len <= E; len++)
            for (int lo = 0; lo + len - 1 < E; lo++) {
                int hi = lo + len - 1;
                double bv = 1e18;
                for (int t = lo; t < hi; t++) {  // test pts[t]: false -> [lo..t], true -> [t+1..hi]
                    double v = cost[lo][t] + cost[t + 1][hi];
                    if (v < bv - 1e-12) bv = v, sp[lo][hi] = t;
                }
                cost[lo][hi] = bv + pre[hi + 1] - pre[lo];
            }
        int a = 0, b = K;
        while (a < b) {
            int t = sp[a][b];
            if (pts[t] > HI(r)) { b = t; continue; }
            if (probe(r, pts[t])) a = t + 1;
            else b = t;
        }
    }
    void bsSide(bool r, int par) {
        while (LO(r) < HI(r)) {
            ll lo = LO(r), hi = HI(r);
            ll p;
            if (par < 0) {
                p = lo + (hi - lo + 1) / 2;
            } else {
                ll a = lo + ((lo & 1) != par ? 1 : 0);
                ll b = hi - ((hi & 1) != par ? 1 : 0);
                if (a >= b) { LO(r) = HI(r) = a; break; }
                ll k = (b - a) / 2 + 1;
                p = a + 2 * (k / 2);
            }
            p = min(p, lo + mlo);
            if (p <= lo) p = lo + 1;
            probe(r, p);
        }
    }

    pair<ll, ll> solve() {
        Mmax = oddDown(N / 5);
        hi_r = N - x0;
        hi_l = x0 - 1;
        mhi = Mmax;
        update();
        bool first = true;  // right first
        if (FS == 1) first = (hi_r <= hi_l);
        expSearch(first, true);
        cA = cnt;
        bsSide(first, -1);
        cB = cnt;
        ll dfirst = first ? lo_r : lo_l;
        expSearch(!first, false);
        bsSide(!first, dfirst & 1);
        cC = cnt;
        ll dr = lo_r, dl = lo_l;
        ll M = dl + dr + 1;
        ll L = x0 - dl;
        cD = cnt;
        return jointY(M, L);
    }
    struct Hyp { int c, r; ll lo, hi; };
    static ll ovl(ll lo, ll hi, ll a, ll b) { ll l = max(lo, a), h = min(hi, b); return h >= l ? h - l + 1 : 0; }
    pair<ll, ll> jointY(ll M, ll L) {
        vector<Hyp> H;
        for (int c = 0; c < 5; c++)
            for (int r = 0; r < 5; r++) {
                if ((c + r) % 2) continue;
                ll bx = L - c * M;
                if (bx < 1 || bx + 5 * M - 1 > N) continue;
                ll by0 = y0 - r * M;  // board bottom when d_d = 0
                bool ok = true;
                for (auto& [k, v] : cache)
                    if (k.second == y0 && expect(bx, by0, M, k.first, k.second) != v) { ok = false; break; }
                if (!ok) continue;
                ll lo = max(0LL, y0 - r * M + 5 * M - 1 - N), hi = min(M - 1, y0 - 1 - r * M);
                if (lo <= hi) H.push_back({c, r, lo, hi});
            }
        auto total = [&]() { ll t = 0; for (auto& h : H) t += h.hi - h.lo + 1; return t; };
        auto vals = [&](const Hyp& h, int a, int s, bool& v0, bool& v1) {
            int i = h.c + a;
            int j0 = h.r + s, j1 = j0 + 1;
            v0 = i >= 0 && i < 5 && j0 >= 0 && j0 < 5 && (i + j0) % 2 == 0;
            v1 = i >= 0 && i < 5 && j1 >= 0 && j1 < 5 && (i + j1) % 2 == 0;
        };
        while (true) {
            ll T = total();
            if (T <= 1) break;
            // choose probe (a, s, th): d_d < th -> row offset s, else s+1
            ll bestScore = LLONG_MAX; int ba = 0, bs = 0; ll bth = 0;
            for (int a = -4; a <= 4; a++) {
                ll sx = L + a * M;
                if (sx > N || sx + M - 1 < 1) continue;
                for (int s = -5; s <= 4; s++) {
                    // y = y0 + s*M + M - th in [1,N]
                    ll thlo = max(1LL, y0 + s * M + M - N), thhi = min(M, y0 + s * M + M - 1);
                    if (thlo > thhi) continue;
                    int n = H.size();
                    vector<char> V0(n), V1(n);
                    bool any = false;
                    for (int k = 0; k < n; k++) { bool a0, a1; vals(H[k], a, s, a0, a1); V0[k] = a0; V1[k] = a1; if (a0 || a1) any = true; }
                    if (!any) continue;
                    auto f = [&](ll th) {
                        ll t = 0;
                        for (int k = 0; k < n; k++) {
                            if (V0[k]) t += ovl(H[k].lo, H[k].hi, 0, th - 1);
                            if (V1[k]) t += ovl(H[k].lo, H[k].hi, th, M - 1);
                        }
                        return t;
                    };
                    vector<ll> bp = {thlo, thhi};
                    for (auto& h : H) for (ll b : {h.lo, h.lo + 1, h.hi, h.hi + 1}) if (b >= thlo && b <= thhi) bp.push_back(b);
                    sort(bp.begin(), bp.end());
                    bp.erase(unique(bp.begin(), bp.end()), bp.end());
                    auto consider = [&](ll th) {
                        ll t = f(th);
                        ll sc = max(t, T - t);
                        if (sc < bestScore) bestScore = sc, ba = a, bs = s, bth = th;
                    };
                    for (size_t q = 0; q < bp.size(); q++) {
                        consider(bp[q]);
                        if (q + 1 < bp.size() && bp[q + 1] - bp[q] > 1) {
                            ll f1 = f(bp[q]), f2 = f(bp[q + 1]);
                            if (f1 != f2) {
                                // linear between; target T/2
                                long double tt = bp[q] + (long double)(T / 2.0 - f1) * (bp[q + 1] - bp[q]) / (long double)(f2 - f1);
                                for (ll d = -1; d <= 1; d++) {
                                    ll th = (ll)floorl(tt) + d;
                                    if (th > bp[q] && th < bp[q + 1]) consider(th);
                                }
                            }
                        }
                    }
                }
            }
            if (bestScore >= T) break;  // should not happen
            ll x = max(1LL, L + ba * M);
            ll y = y0 + bs * M + M - bth;
            bool v = ask(x, y);
            vector<Hyp> NH;
            for (auto h : H) {
                bool a0, a1; vals(h, ba, bs, a0, a1);
                ll lo = h.lo, hi = h.hi;
                if (a0 == v && a1 == v) {}
                else if (a0 == v) hi = min(hi, bth - 1);
                else if (a1 == v) lo = max(lo, bth);
                else continue;
                if (lo <= hi) NH.push_back({h.c, h.r, lo, hi});
            }
            H = NH;
        }
        if (H.empty()) return {x0, y0};
        const Hyp& h = H[0];
        ll bx = L - h.c * M, by = y0 - h.lo - h.r * M;
        return {bx + (5 * M - 1) / 2, by + (5 * M - 1) / 2};
    }
    static bool expect(ll bx, ll by, ll M, ll x, ll y) {
        if (x < bx || y < by || x >= bx + 5 * M || y >= by + 5 * M) return false;
        ll i = (x - bx) / M, j = (y - by) / M;
        return (i + j) % 2 == 0;
    }
};

#ifdef SIM
int main(int argc, char** argv) {
    int T = atoi(argv[1]); if(getenv("C0")) C0=atof(getenv("C0")); if(getenv("PW")) PW=atof(getenv("PW")); if(getenv("FS")) FS=atoi(getenv("FS")); if(getenv("A2")) A2=atof(getenv("A2")); if(getenv("B2")) B2=atof(getenv("B2"));
    int mode = argc > 2 ? atoi(argv[2]) : 0;
    mt19937_64 rng(argc > 3 ? atoi(argv[3]) : 1);
    auto rnd = [&](ll a, ll b) { return a + (ll)(rng() % (unsigned long long)(b - a + 1)); };
    static double binc[64],binA[64],binT[64];
    double slog=0; long long tot = 0; double sA=0,sB=0,sC=0,sD=0,sE=0;
    int mx = 0;
    for (int t = 0; t < T; t++) {
        ll N, M;
        if (mode == 0 || mode == 5) {  // log-uniform M
            N = rnd(15, 2000000000);
            double lm = log(3.0) + (log((double)(N / 5)) - log(3.0)) * (rng() % 1000000) / 1e6;
            M = (ll)exp(lm);
            M = max(3LL, min(M, N / 5));
            if (M % 2 == 0) M--;
            if (M < 3) M = 3;
        } else if (mode == 1) {  // big
            N = 2000000000;
            M = rnd(1, N / 10) * 2 - 1;
            if (M < 3) M = 3;
        } else if (mode == 3) {
            N = rnd(15, 2000000000); if (rng()%2) N = rnd(15,200);
            M = N / 5; if (M % 2 == 0) M--; M -= 2 * rnd(0, 2); if (M < 3) M = 3;
        } else if (mode == 4) {
            N = rnd(15, 2000000000); if (rng()%2) N = 2000000000;
            M = 2 * rnd(1, 20) + 1; if (5 * M > N) M = 3;
        } else {
            N = rnd(15, 100);
            M = rnd(1, N / 10) * 2 + 1;
            if (5 * M > N) M = 3;
        }
        ll bx = rnd(1, N - 5 * M + 1), by = rnd(1, N - 5 * M + 1);
        ll x0, y0;
        while (true) {
            x0 = rnd(bx, bx + 5 * M - 1);
            y0 = rnd(by, by + 5 * M - 1);
            if (mode == 5) { // snap to square edges
                ll i = (x0 - bx) / M, j = (y0 - by) / M;
                int t = rng() % 3; if (t == 0) x0 = bx + i * M; else if (t == 1) x0 = bx + i * M + M - 1;
                t = rng() % 3; if (t == 0) y0 = by + j * M; else if (t == 1) y0 = by + j * M + M - 1;
            }
            if (Solver::expect(bx, by, M, x0, y0)) break;
        }
        Solver s;
        s.N = N, s.x0 = x0, s.y0 = y0;
        s.rawq = [&](ll x, ll y) { return (int)Solver::expect(bx, by, M, x, y); };
        auto [cx, cy] = s.solve();
        if (cx != bx + (5 * M - 1) / 2 || cy != by + (5 * M - 1) / 2) {
            printf("WRONG N=%lld M=%lld bx=%lld by=%lld x0=%lld y0=%lld\n", N, M, bx, by, x0, y0);
            return 1;
        }
        {int b=63-__builtin_clzll(M); binc[b]++; binA[b]+=s.cA; binT[b]+=s.cnt;}
        slog += log((double)max(1,s.cnt));
        tot += s.cnt; sA+=s.cA; sB+=s.cB-s.cA; sC+=s.cC-s.cB; sD+=s.cD-s.cC; sE+=s.cnt-s.cD;
        mx = max(mx, s.cnt);
    }
    if(getenv("BINS")) for(int b=0;b<64;b++) if(binc[b]) printf("log2M=%d n=%.0f A=%.2f T=%.2f\n",b,binc[b],binA[b]/binc[b],binT[b]/binc[b]);
    printf("glog %.4f avg %.3f max %d | A %.2f B %.2f C %.2f D %.2f E %.2f\n", slog/T, (double)tot / T, mx,sA/T,sB/T,sC/T,sD/T,sE/T);
}
#else
int main() {
    ll N, x0, y0;
    if (!(cin >> N >> x0 >> y0)) return 0;
    Solver s;
    s.N = N, s.x0 = x0, s.y0 = y0;
    s.rawq = [&](ll x, ll y) -> int {
        cout << "? " << x << " " << y << endl;
        string r;
        if (!(cin >> r)) return -1;
        if (r == "true") return 1;
        if (r == "false") return 0;
        return -1;
    };
    try {
        auto [cx, cy] = s.solve();
        cout << "! " << cx << " " << cy << endl;
    } catch (...) {
        return 0;
    }
    return 0;
}
#endif

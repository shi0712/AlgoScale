#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

// Model: n = 2m+1. Pair (a,b) with short arc a -> a+L (1<=L<=m).
// C = 2a+L (mod 2n). For query point x (0-based), X = 2x:
// f = clamp(dist_{2n}(X,C), L, n-L).

static ll n, m, N2;

static inline ll md(ll a, ll M) { a %= M; if (a < 0) a += M; return a; }
static inline ll dist2(ll X, ll C) { ll d = md(X - C, N2); return min(d, N2 - d); }
static inline ll fval(ll X, ll C, ll L) {
    ll D = dist2(X, C);
    if (D <= L) return L;
    if (D >= n - L) return n - L;
    return D;
}

// H segment: L fixed, C = p + 2*((s+i) mod n), i in [0,k), p = L&1
// V segment: C fixed, L = s + 2i, i in [0,k)
struct Seg { bool V; ll L; ll C; ll s; ll k; };

static inline ll hC(const Seg& g, ll i) { return (g.L & 1) + 2 * md(g.s + i, n); }

// intersect arc [s1,s1+k1) with [s2,s2+k2) in Z_n
static void arcInter(ll s1, ll k1, ll s2, ll k2, vector<pair<ll,ll>>& out) {
    if (k1 <= 0 || k2 <= 0) return;
    if (k1 >= n) { out.push_back({md(s2, n), min(k2, n)}); return; }
    if (k2 >= n) { out.push_back({md(s1, n), k1}); return; }
    ll d = md(s2 - s1, n);
    if (d < k1) out.push_back({md(s1 + d, n), min(d + k2, k1) - d});
    if (d + k2 > n) {
        ll e = min(k1, d + k2 - n);
        if (e > 0) out.push_back({md(s1, n), e});
    }
}
static inline bool inArc(ll s, ll k, ll i) { return md(i - s, n) < k; }

static ll idxOf(ll C) { C = md(C, N2); return (C - (C & 1)) / 2; }

static void filterSeg(const Seg& g, ll X, ll r, vector<Seg>& out) {
    if (!g.V) {
        ll L = g.L;
        vector<pair<ll,ll>> pcs;
        if (r == L) {
            arcInter(g.s, g.k, idxOf(X - L), L + 1, pcs);
        } else if (r == n - L) {
            arcInter(g.s, g.k, idxOf(X + n - L + 1), L, pcs);
        } else if (r > L && r < n - L && ((r - L) & 1) == 0) {
            ll c1 = md(X + r, N2), c2 = md(X - r, N2);
            if (inArc(g.s, g.k, idxOf(c1))) pcs.push_back({idxOf(c1), 1});
            if (c2 != c1 && inArc(g.s, g.k, idxOf(c2))) pcs.push_back({idxOf(c2), 1});
        }
        for (auto& [s, k] : pcs) out.push_back({false, L, 0, s, k});
    } else {
        ll C = g.C, D = dist2(X, C);
        ll last = g.s + 2 * (g.k - 1);
        ll hi = -1;
        if (r == D) {
            ll h = min(D, n - D);
            h = min(h, last);
            if (h >= g.s) {
                ll kk = (h - g.s) / 2 + 1;
                hi = g.s + 2 * (kk - 1);
                if (kk == 1) out.push_back({false, g.s, 0, idxOf(C), 1});
                else out.push_back({true, 0, C, g.s, kk});
            }
        }
        ll cands[2] = {r, n - r};
        for (ll L : cands) {
            if (L < g.s || L > last || ((L - g.s) & 1)) continue;
            if (hi >= 0 && L <= hi) continue;
            if (fval(X, C, L) != r) continue;
            out.push_back({false, L, 0, idxOf(C), 1});
        }
    }
}

static ll totalSize(const vector<Seg>& v) { ll t = 0; for (auto& g : v) t += g.k; return t; }

static double hb(ll s, ll L) {
    ll q = m - L; int steps = 0;
    while (s > 1) {
        steps++;
        if (s - q <= 2) break;
        ll s1 = (s - q + 1) / 2;
        if (s >= 2 * q + L) { ll s2 = max(L, s - 2 * q - L); s1 = min(s1, s2); }
        s = s1;
    }
    return steps;
}

static double estGroup(const vector<Seg>& g) {
    ll S = totalSize(g);
    if (S <= 1) return 0;
    double e = 1; int nontriv = 0;
    for (auto& s : g) {
        if (s.k <= 1) continue;
        nontriv++;
        double v = s.V ? 1.0 : hb(s.k, s.L);
        e = max(e, v);
    }
    return e + 0.01 * (g.size() - 1);
}

struct Eval { double cost; ll maxg; };

static Eval evaluate(const vector<Seg>& cand, ll X) {
    // mass responses
    vector<ll> Ms;
    for (auto& g : cand) {
        if (!g.V) { Ms.push_back(g.L); Ms.push_back(n - g.L); }
        else Ms.push_back(dist2(X, g.C));
    }
    sort(Ms.begin(), Ms.end()); Ms.erase(unique(Ms.begin(), Ms.end()), Ms.end());
    // density ranges (r0, cnt) step 2
    vector<pair<ll,ll>> dens;
    for (auto& g : cand) {
        if (!g.V) {
            ll L = g.L, q = m - L, p = L & 1;
            if (q <= 0) continue;
            vector<pair<ll,ll>> pcs;
            arcInter(g.s, g.k, idxOf(X + L + 2), q, pcs);
            for (auto& [s, k] : pcs) { ll c = p + 2 * s; dens.push_back({md(c - X, N2), k}); }
            pcs.clear();
            arcInter(g.s, g.k, idxOf(X - n + L + 1), q, pcs);
            for (auto& [s, k] : pcs) { ll c = p + 2 * s; ll rh = md(X - c, N2); dens.push_back({rh - 2 * (k - 1), k}); }
        } else {
            ll D = dist2(X, g.C), last = g.s + 2 * (g.k - 1);
            ll lo = max(g.s, D);
            if (lo <= last) { // parity same
                dens.push_back({lo, (last - lo) / 2 + 1});
            }
            ll a = max(g.s, n - D), b = min(last, D - 1);
            // align a to parity of g.s
            if (((a - g.s) & 1)) a++;
            if (a <= b) {
                ll k = (b - a) / 2 + 1; ll bb = a + 2 * (k - 1);
                dens.push_back({n - bb, k});
            }
        }
    }
    double cost = 0; ll maxg = 0;
    vector<Seg> tmp;
    for (ll r : Ms) {
        tmp.clear();
        for (auto& g : cand) filterSeg(g, X, r, tmp);
        ll S = totalSize(tmp);
        if (S == 0) continue;
        cost += S * estGroup(tmp);
        maxg = max(maxg, S);
    }
    // density-only histogram
    // events per parity
    vector<pair<ll,int>> ev[2];
    for (auto& [r0, k] : dens) { ev[r0 & 1].push_back({r0, 1}); ev[r0 & 1].push_back({r0 + 2 * k, -1}); }
    map<ll, ll> hist; // coverage -> number of r
    for (int p = 0; p < 2; p++) {
        auto& e = ev[p];
        sort(e.begin(), e.end());
        ll cov = 0;
        for (size_t i = 0; i < e.size();) {
            ll pos = e[i].first;
            while (i < e.size() && e[i].first == pos) { cov += e[i].second; i++; }
            if (i < e.size() && cov > 0) hist[cov] += (e[i].first - pos) / 2;
        }
    }
    for (ll r : Ms) {
        ll cov = 0;
        for (auto& [r0, k] : dens) if (r >= r0 && r < r0 + 2 * k && ((r - r0) & 1) == 0) cov++;
        if (cov > 0) hist[cov]--;
    }
    for (auto& [c, num] : hist) {
        if (num <= 0 || c <= 1) continue;
        cost += (double)num * c * (1.0 + 0.05 * (c - 2));
        maxg = max(maxg, c);
    }
    return {cost, maxg};
}

static void addX(vector<ll>& xs, ll X) {
    X = md(X, N2);
    if (X & 1) { xs.push_back(md(X - 1, N2)); xs.push_back(md(X + 1, N2)); }
    else xs.push_back(X);
}

static vector<ll> genCandidates(const vector<Seg>& cand) {
    vector<ll> xs;
    for (auto& g : cand) {
        if (g.k <= 0) continue;
        if (g.V) {
            ll C = g.C;
            for (ll o : {0LL, n}) for (ll d : {-1LL, 0LL, 1LL}) addX(xs, C + o + d);
            ll last = g.s + 2 * (g.k - 1);
            for (ll t : {g.s, (g.s + last) / 2, last}) for (ll sg : {-1LL, 1LL}) {
                addX(xs, C + sg * t); addX(xs, C + n + sg * t);
            }
        } else {
            ll L = g.L, s = g.k, q = m - L;
            vector<ll> pos = {0, 1, 2, s - 1, s - 2, s - 3, s / 2, s / 2 - 1, s / 2 + 1,
                              (s - q) / 2, (s - q) / 2 + 1, (s - q) / 2 - 1,
                              (s + q) / 2, (s + q) / 2 + 1, (s + q) / 2 - 1, s / 4, 3 * s / 4, s / 3, 2 * s / 3};
            vector<ll> offs = {0, 1, -1, L, L + 1, L + 2, L + 3, -L, -L - 1, -L - 2, -L - 3,
                               n - L, n - L + 1, n - L - 1, n - L + 2, n - L - 2,
                               -(n - L), -(n - L) + 1, -(n - L) - 1, -(n - L) + 2, -(n - L) - 2, n, n + 1, n - 1};
            for (ll p : pos) {
                if (p < 0 || p >= s) continue;
                ll T = hC(g, p);
                for (ll o : offs) addX(xs, T - o);
            }
        }
    }
    sort(xs.begin(), xs.end()); xs.erase(unique(xs.begin(), xs.end()), xs.end());
    return xs;
}

template <class Ask>
static pair<ll,ll> solve(ll nn, Ask ask) {
    n = nn; m = (n - 1) / 2; N2 = 2 * n;
    vector<Seg> cand;
    {
        ll r = ask(0);
        if (r < 0) return {-1, -1};
        if (r >= 1 && r <= m) cand.push_back({false, r, 0, idxOf(-r), r + 1});
        if (n - r >= 1 && n - r <= m) { ll L = n - r; cand.push_back({false, L, 0, idxOf(r + 1), L}); }
        ll hiL = min(min(r, n - r) - 1, m);
        ll lo = (r % 2 == 0) ? 2 : 1;
        if (hiL >= lo) {
            ll k = (hiL - lo) / 2 + 1;
            for (ll C : {md(r, N2), md(-r, N2)}) {
                if (k == 1) cand.push_back({false, lo, 0, idxOf(C), 1});
                else cand.push_back({true, 0, C, lo, k});
            }
        }
    }
    while (totalSize(cand) > 1) {
        ll S = totalSize(cand);
        vector<ll> xs = genCandidates(cand);
        ll bestX = -1; double bc = 1e300; ll bm = LLONG_MAX;
        for (ll X : xs) {
            Eval e = evaluate(cand, X);
            if (e.maxg >= S) continue;
            if (e.cost < bc - 1e-9 || (e.cost < bc + 1e-9 && e.maxg < bm)) { bc = e.cost; bm = e.maxg; bestX = X; }
        }
        if (bestX < 0) {
            // fallback: scan
            mt19937_64 rng(12345);
            for (int it = 0; it < 100000 && bestX < 0; it++) {
                ll X = 2 * (ll)(rng() % n);
                Eval e = evaluate(cand, X);
                if (e.maxg < S) bestX = X;
            }
            if (bestX < 0) bestX = 0;
        }
        ll r = ask(bestX);
        if (r < 0) return {-1, -1};
        vector<Seg> nc;
        for (auto& g : cand) filterSeg(g, bestX, r, nc);
        cand.swap(nc);
        if (cand.empty()) return {-1, -1};
    }
    Seg g = cand[0];
    ll C, L;
    if (g.V) { C = g.C; L = g.s; } else { C = hC(g, 0); L = g.L; }
    ll a = md((C - L) / 2, n), b = md(a + L, n);
    return {a, b};
}

#ifndef LOCAL_TEST
int main() {
    ll nn;
    if (!(cin >> nn)) return 0;
    auto ask = [&](ll X) -> ll {
        ll x = X / 2 + 1;
        printf("? %lld\n", x); fflush(stdout);
        ll r;
        if (scanf("%lld", &r) != 1) exit(0);
        if (r < 0) exit(0);
        return r;
    };
    auto res = solve(nn, ask);
    if (res.first < 0) return 0;
    printf("! %lld %lld\n", res.first + 1, res.second + 1); fflush(stdout);
    return 0;
}
#endif

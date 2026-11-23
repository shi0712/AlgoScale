#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

ll n, N2, H;
struct Box { ll l, r, d1, d2; };
static inline ll evc(ll l, ll r) { return r / 2 - (l + 1) / 2 + 1; }
static inline ll cntBox(const Box& b) {
    if (b.l > b.r || b.d1 > b.d2) return 0;
    ll Eu = evc(b.l, b.r), Ou = (b.r - b.l + 1) - Eu;
    ll Ed = evc(b.d1, b.d2), Od = (b.d2 - b.d1 + 1) - Ed;
    return Eu * Ed + Ou * Od;
}
static inline ll md(ll x, ll m) { x %= m; if (x < 0) x += m; return x; }

mt19937_64 rng(712367);

vector<pair<ll,ll>> segs(ll start, ll len) {
    vector<pair<ll,ll>> res;
    if (len >= N2) { res.push_back({0, N2 - 1}); return res; }
    ll e = start + len - 1;
    if (e < N2) res.push_back({start, e});
    else { res.push_back({start, N2 - 1}); res.push_back({0, e - N2}); }
    return res;
}

vector<Box> refine(const vector<Box>& boxes, ll x, ll v) {
    vector<Box> out;
    auto add = [&](Box b) { if (cntBox(b) > 0) out.push_back(b); };
    for (auto& B : boxes) {
        // case 1
        if (v >= B.d1 && v <= B.d2 && v <= H) {
            auto sg = segs(md(2 * x - v, N2), 2 * v + 1);
            for (auto& s : sg) {
                ll l = max(B.l, s.first), r = min(B.r, s.second);
                if (l <= r) add({l, r, v, v});
            }
        }
        // case 2
        {
            ll dd2 = min(B.d2, min(v - 1, n - v - 1));
            if (B.d1 <= dd2) {
                ll us[2] = {md(2 * x + v, N2), md(2 * x - v, N2)};
                for (int i = 0; i < 2; i++) {
                    if (i == 1 && us[1] == us[0]) continue;
                    ll u = us[i];
                    if (u >= B.l && u <= B.r) add({u, u, B.d1, dd2});
                }
            }
        }
        // case 3
        if (v >= (n + 1) / 2 && v <= n - 1) {
            ll d = n - v;
            if (d >= B.d1 && d <= B.d2 && d >= 1 && d <= H) {
                auto sg = segs(md(2 * x + v, N2), 2 * n - 2 * v + 1);
                for (auto& s : sg) {
                    ll l = max(B.l, s.first), r = min(B.r, s.second);
                    if (l <= r) add({l, r, d, d});
                }
            }
        }
    }
    return out;
}

int main() {
    scanf("%lld", &n);
    N2 = 2 * n; H = (n - 1) / 2;
    vector<Box> boxes = {{0, N2 - 1, 1, H}};
    const ll K = 800;
    while (true) {
        ll total = 0;
        vector<ll> cs;
        for (auto& b : boxes) { ll c = cntBox(b); cs.push_back(c); total += c; }
        vector<pair<ll,ll>> cand;
        if (total <= K) {
            for (auto& b : boxes) {
                for (ll d = b.d1; d <= b.d2; d++) {
                    ll u0 = b.l; if ((u0 & 1) != (d & 1)) u0++;
                    for (ll u = u0; u <= b.r; u += 2) cand.push_back({u, d});
                }
            }
        } else {
            for (ll i = 0; i < K; i++) {
                ull r = rng() % (ull)total;
                size_t bi = 0;
                while (r >= (ull)cs[bi]) { r -= cs[bi]; bi++; }
                Box& b = boxes[bi];
                ll Eu = evc(b.l, b.r), Ou = (b.r - b.l + 1) - Eu;
                ll Ed = evc(b.d1, b.d2), Od = (b.d2 - b.d1 + 1) - Ed;
                ll u, d;
                if ((ll)r < Eu * Ed) {
                    ll fu = b.l + (b.l & 1), fd = b.d1 + (b.d1 & 1);
                    u = fu + 2 * (ll)(rng() % Eu);
                    d = fd + 2 * (ll)(rng() % Ed);
                } else {
                    ll fu = b.l + !(b.l & 1), fd = b.d1 + !(b.d1 & 1);
                    u = fu + 2 * (ll)(rng() % Ou);
                    d = fd + 2 * (ll)(rng() % Od);
                }
                cand.push_back({u, d});
            }
        }
        if (total == 1) {
            ll u = cand[0].first, d = cand[0].second;
            ll a = md((u - d) / 2, n), b = md((u + d) / 2, n);
            printf("! %lld %lld\n", a + 1, b + 1);
            fflush(stdout);
            return 0;
        }
        // pool
        vector<ll> pool;
        int S = min<int>(cand.size(), 110);
        for (int i = 0; i < S; i++) {
            auto [u, d] = cand[rng() % cand.size()];
            ll a = md((u - d) / 2, n), b = md((u + d) / 2, n);
            ll m = md(u / 2, n);
            pool.push_back(a); pool.push_back(b); pool.push_back(m);
            pool.push_back(md(m + (n + 1) / 2, n));
            pool.push_back(md(m + 1, n));
        }
        for (size_t i = 0; i < boxes.size() && i < 30; i++) {
            Box& b = boxes[i];
            ll ds[2] = {b.d1, b.d2};
            for (ll d : ds) {
                ll vals[4] = {(b.l - d) / 2, (b.r + d) / 2, (b.l + d) / 2, (b.r - d) / 2};
                for (ll v : vals) {
                    pool.push_back(md(v, n));
                    pool.push_back(md(v - 1, n));
                    pool.push_back(md(v + 1, n));
                }
            }
            pool.push_back(md(b.l / 2, n));
            pool.push_back(md(b.r / 2, n));
            pool.push_back(md((b.l + b.r) / 4, n));
        }
        for (int i = 0; i < 100; i++) pool.push_back((ll)(rng() % n));
        if (n <= 1500) { pool.clear(); for (ll x = 0; x < n; x++) pool.push_back(x); }
        ll bestX = -1; ll bestMax = LLONG_MAX, bestSq = LLONG_MAX;
        int sz = cand.size();
        vector<int> outs(sz);
        auto evalPool = [&](const vector<ll>& pl) {
            for (ll x : pl) {
                for (int i = 0; i < sz; i++) {
                    ll u = cand[i].first, d = cand[i].second;
                    ll c = llabs(u - 2 * x) % N2;
                    c = min(c, N2 - c);
                    ll v = max(d, min(c, n - d));
                    outs[i] = (int)v;
                }
                sort(outs.begin(), outs.end());
                ll mx = 0, sq = 0;
                int i = 0;
                while (i < sz) {
                    int j = i;
                    while (j < sz && outs[j] == outs[i]) j++;
                    ll c = j - i;
                    mx = max(mx, c); sq += c * c;
                    i = j;
                }
                if (mx < bestMax || (mx == bestMax && sq < bestSq)) {
                    bestMax = mx; bestSq = sq; bestX = x;
                }
            }
        };
        evalPool(pool);
        int tries = 0;
        while (bestMax == sz && tries < 50) {
            vector<ll> more;
            for (int i = 0; i < 2000; i++) more.push_back((ll)(rng() % n));
            evalPool(more);
            tries++;
        }
        printf("? %lld\n", bestX + 1);
        fflush(stdout);
        ll v;
        if (scanf("%lld", &v) != 1) return 0;
        if (v < 0) return 0;
        boxes = refine(boxes, bestX, v);
        if (boxes.empty()) return 0;
    }
}

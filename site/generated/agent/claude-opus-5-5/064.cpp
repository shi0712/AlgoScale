#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll INF = (ll)1e18;
const ll NEG = -(ll)1e18;
struct Seg {
    int n; vector<ll> mw1, mw2, val, t1, t2;
    void init(const vector<ll>& w1, const vector<ll>& w2, const vector<ll>& base) {
        n = w1.size(); int sz = 4 * n + 4;
        mw1.assign(sz, INF); mw2.assign(sz, INF); val.assign(sz, INF); t1.assign(sz, INF); t2.assign(sz, INF);
        build(1, 0, n - 1, w1, w2, base);
    }
    void build(int x, int l, int r, const vector<ll>& w1, const vector<ll>& w2, const vector<ll>& base) {
        if (l == r) { mw1[x] = w1[l]; mw2[x] = w2[l]; val[x] = base[l]; return; }
        int m = (l + r) / 2; build(2*x, l, m, w1, w2, base); build(2*x+1, m+1, r, w1, w2, base);
        mw1[x] = min(mw1[2*x], mw1[2*x+1]); mw2[x] = min(mw2[2*x], mw2[2*x+1]); val[x] = min(val[2*x], val[2*x+1]);
    }
    inline void app(int x, ll c1, ll c2) {
        if (c1 < INF) { if (mw1[x] < INF) val[x] = min(val[x], c1 + mw1[x]); t1[x] = min(t1[x], c1); }
        if (c2 < INF) { if (mw2[x] < INF) val[x] = min(val[x], 2 * c2 + mw2[x]); t2[x] = min(t2[x], c2); }
    }
    inline void push(int x) {
        if (t1[x] < INF || t2[x] < INF) { app(2*x, t1[x], t2[x]); app(2*x+1, t1[x], t2[x]); t1[x] = t2[x] = INF; }
    }
    void upd(int x, int l, int r, int ql, int qr, ll c1, ll c2) {
        if (qr < l || r < ql) return;
        if (ql <= l && r <= qr) { app(x, c1, c2); return; }
        push(x); int m = (l + r) / 2;
        upd(2*x, l, m, ql, qr, c1, c2); upd(2*x+1, m+1, r, ql, qr, c1, c2);
        val[x] = min(val[2*x], val[2*x+1]);
    }
    void update(int ql, int qr, ll c1, ll c2) { if (ql > qr) return; upd(1, 0, n - 1, ql, qr, c1, c2); }
    // pop argmin, deactivate
    int pop(ll& v) {
        v = val[1]; int x = 1, l = 0, r = n - 1;
        vector<int> path;
        while (l < r) { push(x); path.push_back(x); int m = (l + r) / 2;
            if (val[2*x] == val[x]) { x = 2*x; r = m; } else { x = 2*x+1; l = m+1; } }
        mw1[x] = mw2[x] = val[x] = INF;
        for (int i = (int)path.size() - 1; i >= 0; i--) { int y = path[i];
            mw1[y] = min(mw1[2*y], mw1[2*y+1]); mw2[y] = min(mw2[2*y], mw2[2*y+1]); val[y] = min(val[2*y], val[2*y+1]); }
        return l;
    }
};
int main() {
    int n; ll s; int m;
    if (scanf("%d %lld %d", &n, &s, &m) != 3) return 0;
    vector<pair<ll,ll>> req(m);
    for (auto& [f, a] : req) { ll aa, ff; scanf("%lld %lld", &aa, &ff); f = ff; a = aa; }
    sort(req.begin(), req.end());
    vector<ll> X, A;
    for (auto& [f, a] : req) { if (!X.empty() && X.back() == f) A.back() = max(A.back(), a); else { X.push_back(f); A.push_back(a); } }
    int K = X.size();
    vector<ll> U(K), V(K), Up(K), Vp(K);
    { ll cur = NEG; for (int i = 0; i < K; i++) { U[i] = cur; cur = max(cur, A[i] - X[i]); Up[i] = cur; } }
    { ll cur = NEG; for (int i = K - 1; i >= 0; i--) { V[i] = cur; cur = max(cur, A[i] + X[i]); Vp[i] = cur; } }
    ll xmin = X[0], xmax = X[K - 1];
    // L tree: type1 val = c1 - y (w1 = -y), type2 val = 2c2 + U - y
    vector<ll> w1L(K), w2L(K), bL(K, INF), w1R(K), w2R(K), bR(K, INF);
    for (int i = 0; i < K; i++) {
        w1L[i] = -X[i]; w2L[i] = (U[i] == NEG) ? INF : U[i] - X[i];
        w1R[i] = X[i]; w2R[i] = (V[i] == NEG) ? INF : V[i] + X[i];
    }
    { ll M = max(xmax, s); bL[0] = (M - xmin) + (M - s); }
    { ll mn = min(xmin, s); bR[K - 1] = (xmax - mn) + (s - mn); }
    Seg L, R; L.init(w1L, w2L, bL); R.init(w1R, w2R, bR);
    vector<ll> CL(K, INF), CR(K, INF);
    for (int it = 0; it < 2 * K; it++) {
        ll vl = L.val[1], vr = R.val[1];
        if (vl >= INF && vr >= INF) break;
        if (vl <= vr) {
            ll v; int k = L.pop(v); CL[k] = v; ll y = X[k];
            // targets R indices i>k; type1 where V(i) <= v + y (suffix)
            ll lim = v + y;
            int lo = k + 1, hi = K; // find first i in [k+1,K) with V[i] <= lim
            while (lo < hi) { int md = (lo + hi) / 2; if (V[md] <= lim) hi = md; else lo = md + 1; }
            R.update(lo, K - 1, v - y, INF);
            R.update(k + 1, lo - 1, INF, -y);
        } else {
            ll v; int k = R.pop(v); CR[k] = v; ll z = X[k];
            ll lim = v - z; // type1 where U(i) <= lim (prefix)
            int lo = 0, hi = k; // first i in [0,k) with U[i] > lim
            while (lo < hi) { int md = (lo + hi) / 2; if (U[md] > lim) hi = md; else lo = md + 1; }
            L.update(0, lo - 1, v + z, INF);
            L.update(lo, k - 1, INF, z);
        }
    }
    ll ans = INF;
    for (int i = 0; i < K; i++) {
        if (CL[i] < INF) ans = min(ans, max(Vp[i] - X[i], CL[i]));
        if (CR[i] < INF) ans = min(ans, max(Up[i] + X[i], CR[i]));
    }
    printf("%lld\n", ans);
}

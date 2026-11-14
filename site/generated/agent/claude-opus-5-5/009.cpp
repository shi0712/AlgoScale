#include <bits/stdc++.h>
using namespace std;
typedef long long ll;
static ll LIM = 1000000000LL;

struct Ent { ll r; int idx; int role; }; // role 0: S-role known, 1: out-role known, 2: X

static bool verifySets(int n, ll W, const vector<ll>& w, const vector<ll>& r) {
    vector<int> o(n); iota(o.begin(), o.end(), 0);
    sort(o.begin(), o.end(), [&](int a, int b){ return w[a] != w[b] ? w[a] < w[b] : a < b; });
    vector<char> A(n, 0), Bs(n, 0);
    ll rem = W;
    for (int i : o) { if (w[i] > rem) break; rem -= w[i]; A[i] = 1; }
    sort(o.begin(), o.end(), [&](int a, int b){ return r[a] != r[b] ? r[a] > r[b] : a < b; });
    rem = W;
    for (int i : o) { if (w[i] <= rem) { rem -= w[i]; Bs[i] = 1; } }
    return A == Bs;
}

int main(int argc, char** argv) {
    if (argc > 1) LIM = atoll(argv[1]);
    int T; if (scanf("%d", &T) != 1) return 0;
    string out;
    while (T--) {
        int n; ll W; scanf("%d %lld", &n, &W);
        vector<ll> w(n), r(n);
        for (auto& x : w) scanf("%lld", &x);
        for (auto& x : r) scanf("%lld", &x);
        vector<int> K, X;
        for (int i = 0; i < n; i++) (w[i] ? K : X).push_back(i);
        sort(K.begin(), K.end(), [&](int a, int b){ return w[a] != w[b] ? w[a] < w[b] : a < b; });
        int m = K.size();
        vector<int> pos(n, 0);
        for (int p = 0; p < m; p++) pos[K[p]] = p + 1;
        vector<ll> pre(m + 1, 0);
        for (int p = 0; p < m; p++) pre[p + 1] = pre[p] + w[K[p]];
        int kmax = 0; while (kmax < m && pre[kmax + 1] <= W) kmax++;
        vector<Ent> E; E.reserve(2 * m + X.size());
        for (int i : K) {
            if (r[i]) { E.push_back({r[i], i, 0}); E.push_back({r[i], i, 1}); }
            else { E.push_back({LIM, i, 0}); E.push_back({1, i, 1}); }
        }
        for (int u : X) E.push_back({r[u], u, 2});
        sort(E.begin(), E.end(), [](const Ent& a, const Ent& b){
            if (a.r != b.r) return a.r > b.r; if (a.idx != b.idx) return a.idx < b.idx; return a.role > b.role; });
        // cond1: sweep from end with BIT
        vector<int> diff(m + 2, 0);
        {
            vector<ll> bit(m + 1, 0);
            int LOG = 1; while ((1 << LOG) <= m) LOG++;
            for (int e = (int)E.size() - 1; e >= 0; e--) {
                const Ent& en = E[e];
                if (en.role == 0) { for (int p = pos[en.idx]; p <= m; p += p & -p) bit[p] += w[en.idx]; }
                else if (en.role == 1) {
                    ll need = w[en.idx]; int p = 0;
                    for (int s = LOG; s >= 0; s--) {
                        int q = p + (1 << s);
                        if (q <= m && bit[q] < need) { p = q; need -= bit[q]; }
                    }
                    int k = p + 1; // smallest k with prefix >= w_j (may be m+1)
                    int j = pos[en.idx];
                    if (k <= j - 1) { diff[k]++; diff[j]--; }
                }
            }
        }
        auto capOf = [&](int k, int u) -> ll {
            if (k < m) { ll b = w[K[k]]; ll c = b - (u > K[k] ? 1 : 0); return min(c, LIM - 1); }
            return LIM - 1;
        };
        auto P2 = [&](int k) -> bool {
            ll KP = 0, XC = 0;
            for (const Ent& en : E) {
                if (en.role == 0) { if (pos[en.idx] <= k) KP += w[en.idx]; }
                else if (en.role == 1) {
                    int j = en.idx; if (pos[j] > k && KP + XC < W - w[j] + 1) return false;
                } else {
                    ll c = capOf(k, en.idx);
                    if (W == LIM && c == 0 && KP + XC < 1) return false;
                    XC += c;
                }
            }
            return true;
        };
        bool ok = false;
        vector<ll> wo, ro;
        if (P2(kmax)) {
            int lo = 0, hi = kmax;
            while (lo < hi) { int mid = (lo + hi) / 2; if (P2(mid)) hi = mid; else lo = mid + 1; }
            int cur = 0, tries = 0;
            for (int k = 0; k <= kmax && tries < 4; k++) {
                cur += diff[k];
                if (k < lo || cur > 0) continue;
                tries++;
                wo = w; ro = r;
                for (int i : K) if (!r[i]) ro[i] = (pos[i] <= k) ? LIM : 1;
                ll B = W - pre[k];
                for (const Ent& en : E) if (en.role == 2) {
                    ll x = min(capOf(k, en.idx), B); B -= x;
                    wo[en.idx] = x > 0 ? x : LIM;
                }
                if (verifySets(n, W, wo, ro)) { ok = true; break; }
            }
        }
        if (!ok) out += "No\n";
        else {
            out += "Yes\n";
            for (int i = 0; i < n; i++) { out += to_string(wo[i]); out += (i + 1 < n ? ' ' : '\n'); }
            for (int i = 0; i < n; i++) { out += to_string(ro[i]); out += (i + 1 < n ? ' ' : '\n'); }
        }
    }
    fputs(out.c_str(), stdout);
}

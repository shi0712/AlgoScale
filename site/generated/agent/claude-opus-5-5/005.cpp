#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const int LOG = 19;
int n, root_;
vector<int> par_, dep, idv, pc, posv, order_, prefUp, szv;
vector<ll> SG, Gv;
vector<int> up[LOG];
vector<ll> Gid;

static char *ibuf; static size_t ipos = 0, ilen = 0;
static inline long long readInt() {
    while (ipos < ilen && (ibuf[ipos] < '0' || ibuf[ipos] > '9')) ipos++;
    long long x = 0;
    while (ipos < ilen && ibuf[ipos] >= '0' && ibuf[ipos] <= '9') { x = x * 10 + (ibuf[ipos] - '0'); ipos++; }
    return x;
}

// number of vertices in anc*(x) with id <= v (ids increase upward)
static inline int cntLE(int x, long long v) {
    if (idv[x] > v) return 0;
    if (idv[root_] <= v) return dep[x] + 1;
    int c = 1;
    for (int k = LOG - 1; k >= 0; k--) {
        int w = up[k][x];
        if (idv[w] <= v) { x = w; c += 1 << k; }
    }
    return c;
}
static inline int kth(int v, ll k) {
    for (int b = 0; k; b++, k >>= 1) if (k & 1) v = up[b][v];
    return v;
}
static inline int lca(int a, int b) {
    if (dep[a] < dep[b]) swap(a, b);
    a = kth(a, dep[a] - dep[b]);
    if (a == b) return a;
    for (int k = LOG - 1; k >= 0; k--) if (up[k][a] != up[k][b]) { a = up[k][a]; b = up[k][b]; }
    return par_[a];
}
static inline bool isAnc(int a, int x) { return posv[a] <= posv[x] && posv[x] < posv[a] + szv[a]; }
static inline ll fval(int x, int y) {
    if (isAnc(y, x)) return 1 + dep[x] - dep[y];
    return Gv[y] + cntLE(x, idv[y]);
}

// #{(x,y): x in X, y in Y, id_x <= id_y}; X = xb and its xlen-1 ancestors, Y likewise.
ll cross(int xb, int xlen, int yb, ll ylen) {
    if (xlen <= 0 || ylen <= 0) return 0;
    ll total = 0; int y = yb; ll rem = ylen;
    int k = min(cntLE(xb, idv[y]), xlen);
    while (true) {
        if (k == xlen) { total += (ll)k * rem; break; }
        int xp = (k > 0) ? kth(xb, k - 1) : -1;
        if (k == 0 || pc[par_[xp]] != xp) {
            int xk = (k > 0) ? par_[xp] : xb;
            ll c = min<ll>(rem, cntLE(y, (ll)idv[xk] - 1));
            total += (ll)k * c; rem -= c;
            if (rem == 0) break;
            y = kth(y, c);
            k = min(cntLE(xb, idv[y]), xlen);
            continue;
        }
        if (rem == 1 || pc[par_[y]] != y) {
            total += k; rem--;
            if (rem == 0) break;
            y = par_[y]; k = min(cntLE(xb, idv[y]), xlen);
            continue;
        }
        ll ell = min<ll>({(ll)prefUp[xp], (ll)(xlen - k), (ll)prefUp[y], rem - 1});
        ll base = (ll)k * ell + ell * (ell - 1) / 2;
        int px = posv[xp], py = posv[y];
        if (idv[xp] < idv[y]) {
            total += base; y = order_[py - ell]; k += (int)ell; rem -= ell;
            continue;
        }
        // tie
        ll lo = 0, hi = ell;
        while (lo < hi) {
            ll mid = (lo + hi + 1) / 2;
            if (idv[order_[px - mid]] == idv[order_[py - mid]]) lo = mid; else hi = mid - 1;
        }
        ll T = lo;
        if (T == ell) {
            total += base; y = order_[py - ell]; k += (int)ell; rem -= ell;
            continue;
        }
        bool less = idv[order_[px - (T + 1)]] < idv[order_[py - (T + 1)]];
        if (less) { total += base; k += (int)ell; }
        else { total += base - (ell - 1 - T); k += (int)ell - 1; }
        y = order_[py - ell]; rem -= ell;
    }
    return total;
}

static inline ll chainAnc(ll k, ll r) {
    ll D = min(k - 1, r - 1);
    if (D <= 0) return 0;
    return D * k - D * (D + 1) / 2;
}

int main() {
    // read input
    {
        size_t cap = 1 << 22; ibuf = (char*)malloc(cap);
        size_t rd;
        while ((rd = fread(ibuf + ilen, 1, cap - ilen, stdin)) > 0) {
            ilen += rd;
            if (ilen == cap) { cap <<= 1; ibuf = (char*)realloc(ibuf, cap); }
        }
    }
    // skip c (arbitrary size)
    while (ipos < ilen && (ibuf[ipos] < '0' || ibuf[ipos] > '9')) ipos++;
    while (ipos < ilen && ibuf[ipos] >= '0' && ibuf[ipos] <= '9') ipos++;
    n = (int)readInt();
    vector<int> eu(n - 1), ev(n - 1), deg(n + 1, 0);
    for (int i = 0; i < n - 1; i++) {
        eu[i] = (int)readInt() - 1; ev[i] = (int)readInt() - 1;
        deg[eu[i]]++; deg[ev[i]]++;
    }
    vector<int> as(n + 1, 0);
    for (int i = 0; i < n; i++) as[i + 1] = as[i] + deg[i];
    vector<int> al(2 * (n - 1));
    {
        vector<int> fill(as.begin(), as.begin() + n);
        for (int i = 0; i < n - 1; i++) { al[fill[eu[i]]++] = ev[i]; al[fill[ev[i]]++] = eu[i]; }
    }
    // find center
    vector<int> dist(n), bp(n), q(n);
    auto bfs = [&](int src) {
        fill(dist.begin(), dist.end(), -1);
        int h = 0, t = 0; q[t++] = src; dist[src] = 0; bp[src] = -1;
        while (h < t) {
            int v = q[h++];
            for (int e = as[v]; e < as[v + 1]; e++) { int w = al[e]; if (dist[w] < 0) { dist[w] = dist[v] + 1; bp[w] = v; q[t++] = w; } }
        }
        return q[n - 1];
    };
    int A = bfs(0);
    int B = bfs(A);
    int D = dist[B];
    {
        int v = B;
        for (int i = 0; i < D / 2; i++) v = bp[v];
        root_ = v;
    }
    // root at center
    par_.assign(n, -1); dep.assign(n, 0);
    vector<int> bfo(n);
    {
        vector<char> vis(n, 0);
        int h = 0, t = 0; bfo[t++] = root_; vis[root_] = 1; par_[root_] = root_;
        while (h < t) {
            int v = bfo[h++];
            for (int e = as[v]; e < as[v + 1]; e++) { int w = al[e]; if (!vis[w]) { vis[w] = 1; par_[w] = v; dep[w] = dep[v] + 1; bfo[t++] = w; } }
        }
    }
    vector<int> hgt(n, 0), nch(n, 0);
    for (int i = n - 1; i >= 1; i--) {
        int v = bfo[i], p = par_[v];
        hgt[p] = max(hgt[p], hgt[v] + 1);
        nch[p]++;
    }
    int maxH = hgt[root_];
    // children CSR
    vector<int> cs(n + 1, 0);
    for (int i = 0; i < n; i++) cs[i + 1] = cs[i] + nch[i];
    vector<int> cl(max(1, n - 1));
    {
        vector<int> fill(cs.begin(), cs.begin() + n);
        for (int i = 1; i < n; i++) { int v = bfo[i]; cl[fill[par_[v]]++] = v; }
    }
    // group by height
    vector<int> hs(maxH + 2, 0);
    for (int i = 0; i < n; i++) hs[hgt[i] + 1]++;
    for (int h = 0; h <= maxH; h++) hs[h + 1] += hs[h];
    vector<int> grp(n);
    {
        vector<int> fill(hs.begin(), hs.begin() + maxH + 1);
        for (int i = 0; i < n; i++) grp[fill[hgt[i]]++] = i;
    }
    idv.assign(n, 0);
    vector<int> cb(max(1, n - 1));
    int curId = -1;
    for (int h = 0; h <= maxH; h++) {
        int gs = hs[h], ge = hs[h + 1];
        for (int i = gs; i < ge; i++) {
            int v = grp[i];
            for (int e = cs[v]; e < cs[v + 1]; e++) cb[e] = idv[cl[e]];
            sort(cb.begin() + cs[v], cb.begin() + cs[v + 1], greater<int>());
        }
        auto cmp = [&](int a, int b) {
            int la = cs[a + 1] - cs[a], lb = cs[b + 1] - cs[b];
            const int *X = cb.data() + cs[a], *Y = cb.data() + cs[b];
            int m = min(la, lb);
            for (int i = 0; i < m; i++) if (X[i] != Y[i]) return X[i] < Y[i];
            return la < lb;
        };
        sort(grp.begin() + gs, grp.begin() + ge, cmp);
        for (int i = gs; i < ge; i++) {
            if (i == gs || cmp(grp[i - 1], grp[i])) curId++;
            idv[grp[i]] = curId;
        }
    }
    int numIds = curId + 1;
    {
        vector<ll> cnt(numIds + 1, 0);
        for (int i = 0; i < n; i++) cnt[idv[i]]++;
        Gid.assign(numIds, 0);
        ll suf = 0;
        for (int i = numIds - 1; i >= 0; i--) { Gid[i] = 1 + suf; suf += cnt[i]; }
    }
    Gv.assign(n, 0);
    for (int i = 0; i < n; i++) Gv[i] = Gid[idv[i]];
    // preferred child: max id child
    pc.assign(n, -1);
    for (int v = 0; v < n; v++) {
        int best = -1;
        for (int e = cs[v]; e < cs[v + 1]; e++) { int w = cl[e]; if (best < 0 || idv[w] > idv[best]) best = w; }
        pc[v] = best;
    }
    // layout DFS, preferred child first
    posv.assign(n, 0); order_.assign(n, 0); szv.assign(n, 1);
    {
        vector<int> st; st.reserve(n); st.push_back(root_);
        int cnt = 0;
        while (!st.empty()) {
            int v = st.back(); st.pop_back();
            posv[v] = cnt; order_[cnt] = v; cnt++;
            for (int e = cs[v]; e < cs[v + 1]; e++) { int w = cl[e]; if (w != pc[v]) st.push_back(w); }
            if (pc[v] >= 0) st.push_back(pc[v]);
        }
        for (int i = n - 1; i >= 1; i--) { int v = bfo[i]; szv[par_[v]] += szv[v]; }
    }
    prefUp.assign(n, 0); SG.assign(n, 0);
    for (int i = 0; i < n; i++) {
        int v = bfo[i];
        if (v == root_) { prefUp[v] = 0; SG[v] = Gv[v]; }
        else { int p = par_[v]; prefUp[v] = (pc[p] == v) ? prefUp[p] + 1 : 0; SG[v] = SG[p] + Gv[v]; }
    }
    for (int k = 0; k < LOG; k++) up[k].assign(n, 0);
    for (int v = 0; v < n; v++) up[0][v] = par_[v];
    for (int k = 1; k < LOG; k++) for (int v = 0; v < n; v++) up[k][v] = up[k - 1][up[k - 1][v]];

    int m = (int)readInt();
    string out; out.reserve((size_t)m * 13);
    char tmp[24];
    for (int qi = 0; qi < m; qi++) {
        int s = (int)readInt() - 1, t = (int)readInt() - 1;
        int ox = (int)readInt(), oy = (int)readInt();
        ll r = readInt();
        int l = lca(s, t);
        ll p = dep[s] - dep[l], qq = dep[t] - dep[l];
        ll ans = 0;
        auto bAt = [&](ll i) { return kth(t, qq - i); };   // b_i, i in [0,qq]
        auto aAt = [&](ll j) { return kth(s, j); };        // a_j, j in [0,p]
        if (!ox && !oy) {
            ans = (fval(s, t) <= r) ? 1 : 0;
        } else if (!ox && oy) {
            ans = min(p + 1, r);
            ll lo = 0, hi = qq;
            while (lo < hi) {
                ll mid = (lo + hi + 1) / 2;
                int b = bAt(mid);
                if (Gv[b] + cntLE(s, idv[b]) <= r) lo = mid; else hi = mid - 1;
            }
            ans += lo;
        } else if (ox && !oy) {
            if (qq == 0) ans = min(p + 1, r);
            else {
                ans = 1;
                if (Gv[t] <= r) {
                    ans += qq;
                    ll pi = min<ll>(p, cntLE(s, idv[t]));
                    ans += p - max<ll>(0, pi + Gv[t] - r);
                }
            }
        } else {
            ll L = p + qq + 1;
            // theta: min id with Gid <= r
            int lo = 0, hi = numIds - 1;
            while (lo < hi) { int mid = (lo + hi) / 2; if (Gid[mid] <= r) hi = mid; else lo = mid + 1; }
            ll theta = lo;
            ll jlo = min<ll>(p, cntLE(s, theta - 1));
            ll ihi = qq - min<ll>(qq, cntLE(t, theta - 1));
            ans = L + chainAnc(p + 1, r) + chainAnc(qq + 1, r);
            ll mA = p - jlo; ans += mA * (mA + 1) / 2;
            ans += ihi * (ihi + 1) / 2;
            if (p > 0 && ihi > 0) {
                ll a = 0, b = ihi;
                while (a < b) {
                    ll mid = (a + b + 1) / 2;
                    int bv = bAt(mid);
                    if (Gv[bv] + cntLE(s, idv[bv]) <= r) a = mid; else b = mid - 1;
                }
                ll ilo = a + 1;
                ll C1 = ihi * p;
                if (ilo <= ihi) {
                    ll seglen = ihi - ilo + 1;
                    int bb = bAt(ihi), bt = bAt(ilo - 1);
                    ll sumG = SG[bb] - SG[bt];
                    C1 -= sumG - r * seglen;
                    C1 -= cross(s, (int)p, bb, seglen);
                }
                ans += C1;
            }
            if (qq > 0 && jlo < p) {
                // count c of j in [jlo, p-1] with f(t,a_j) > r (prefix)
                ll a = 0, b = p - jlo;
                while (a < b) {
                    ll mid = (a + b + 1) / 2;
                    int av = aAt(jlo + mid - 1);
                    if (Gv[av] + cntLE(t, idv[av]) > r) a = mid; else b = mid - 1;
                }
                ll c = a;
                ll C2 = (p - jlo) * qq;
                if (c > 0) {
                    int ab = aAt(jlo), at = aAt(jlo + c);
                    ll sumG = SG[ab] - SG[at];
                    C2 -= sumG - r * c;
                    C2 -= cross(t, (int)qq, ab, c);
                }
                ans += C2;
            }
        }
        int len = 0;
        if (ans == 0) tmp[len++] = '0';
        while (ans) { tmp[len++] = '0' + ans % 10; ans /= 10; }
        while (len) out.push_back(tmp[--len]);
        out.push_back('\n');
    }
    fwrite(out.data(), 1, out.size(), stdout);
    return 0;
}

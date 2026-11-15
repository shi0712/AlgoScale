#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using ull = unsigned long long;

static inline ull splitmix64(ull x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}
static inline ull modinv64(ull a) {
    // a must be odd
    ull x = a;
    for (int i = 0; i < 7; i++) x *= (2ULL - a * x);
    return x;
}

// ---------- persistent segment tree (global, reset per component) ----------
static vector<int> lc_, rc_;
static vector<ull> hs_;
static vector<ull> Rv_, invRv_;
static int gU = 0, gSize = 0;

static inline int newNode() {
    lc_.push_back(0);
    rc_.push_back(0);
    hs_.push_back(0);
    return (int)lc_.size() - 1;
}

static int insertVal(int node, int idx, int lo, int hi, ull rv) {
    int nn = newNode();
    lc_[nn] = lc_[node];
    rc_[nn] = rc_[node];
    hs_[nn] = hs_[node];
    if (lo == hi) {
        hs_[nn] += rv;
        return nn;
    }
    int mid = (lo + hi) >> 1;
    if (idx <= mid)
        lc_[nn] = insertVal(lc_[node], idx, lo, mid, rv);
    else
        rc_[nn] = insertVal(rc_[node], idx, mid + 1, hi, rv);
    hs_[nn] = hs_[lc_[nn]] + hs_[rc_[nn]];
    return nn;
}

// Compare A = tree(nA) + {vA} with B = tree(nB) + {vB} lexicographically
// on the sorted descending multiset. Return 1 if A > B, -1 if A < B, 0 if equal.
static int cmpMul(int nA, int nB, int lo, int hi, int vA, int vB) {
    ull ha = hs_[nA], hb = hs_[nB];
    if (vA >= lo && vA <= hi) ha += Rv_[vA];
    if (vB >= lo && vB <= hi) hb += Rv_[vB];
    if (ha == hb) return 0;
    if (lo == hi) {
        ull ca = (nA ? hs_[nA] * invRv_[lo] : 0ULL) + (vA == lo ? 1ULL : 0ULL);
        ull cb = (nB ? hs_[nB] * invRv_[lo] : 0ULL) + (vB == lo ? 1ULL : 0ULL);
        return ca > cb ? 1 : -1;
    }
    int mid = (lo + hi) >> 1;
    int r = cmpMul(rc_[nA], rc_[nB], mid + 1, hi, vA, vB);
    if (r != 0) return r;
    return cmpMul(lc_[nA], lc_[nB], lo, mid, vA, vB);
}

static void extractMul(int node, int lo, int hi, const vector<ll>& vals, vector<ll>& out) {
    if (node == 0) return;
    if (lo == hi) {
        ull cnt = hs_[node] * invRv_[lo];
        ll v = vals[lo];
        if (v > 0 && cnt > 0) {
            out.insert(out.end(), (size_t)cnt, v);
        }
        return;
    }
    int mid = (lo + hi) >> 1;
    extractMul(lc_[node], lo, mid, vals, out);
    extractMul(rc_[node], mid + 1, hi, vals, out);
}

static void solvePath(const vector<ll>& L, const vector<ll>& s, vector<ll>& ans) {
    int K = (int)L.size();
    if (K == 1) {
        if (L[0] > 0) ans.push_back(L[0]);
        return;
    }
    // collect candidate values
    vector<ll> vals;
    vals.reserve((size_t)4 * K);
    for (int i = 0; i < K; i++) {
        ll left = (i == 0 ? 0 : s[i - 1]);
        ll right = (i == K - 1 ? 0 : s[i]);
        vals.push_back(L[i]);
        vals.push_back(L[i] - left);
        vals.push_back(L[i] - right);
        vals.push_back(L[i] - left - right);
    }
    sort(vals.begin(), vals.end());
    vals.erase(unique(vals.begin(), vals.end()), vals.end());
    int U = (int)vals.size();
    int sz = 1, D = 0;
    while (sz < U) { sz <<= 1; D++; }

    Rv_.resize(U);
    invRv_.resize(U);
    for (int k = 0; k < U; k++) {
        ull r = splitmix64((ull)vals[k]) | 1ULL;
        Rv_[k] = r;
        invRv_[k] = modinv64(r);
    }
    gU = U;
    gSize = sz;

    auto getIdx = [&](ll v) -> int {
        return (int)(lower_bound(vals.begin(), vals.end(), v) - vals.begin());
    };
    vector<int> iBase(K), iLeft(K), iRight(K), iBoth(K);
    for (int i = 0; i < K; i++) {
        ll left = (i == 0 ? 0 : s[i - 1]);
        ll right = (i == K - 1 ? 0 : s[i]);
        iBase[i] = getIdx(L[i]);
        iLeft[i] = getIdx(L[i] - left);
        iRight[i] = getIdx(L[i] - right);
        iBoth[i] = getIdx(L[i] - left - right);
    }

    lc_.clear(); rc_.clear(); hs_.clear();
    size_t maxNodes = (size_t)(2 * (K - 1) + 2) * (size_t)(D + 1) + 8;
    lc_.reserve(maxNodes);
    rc_.reserve(maxNodes);
    hs_.reserve(maxNodes);
    // null node
    lc_.push_back(0); rc_.push_back(0); hs_.push_back(0);

    // dp after edge 0 (if K>=2)
    int root0 = insertVal(0, iRight[0], 0, sz - 1, Rv_[iRight[0]]); // edge0 charges node0
    int root1 = insertVal(0, iBase[0], 0, sz - 1, Rv_[iBase[0]]);   // edge0 charges node1

    for (int i = 1; i <= K - 2; i++) {
        // target state 0: edge i charges node i
        int vA = iRight[i];   // from parent state 0
        int vB = iBoth[i];    // from parent state 1
        int c = cmpMul(root0, root1, 0, sz - 1, vA, vB);
        int nr0;
        if (c >= 0) nr0 = insertVal(root0, vA, 0, sz - 1, Rv_[vA]);
        else        nr0 = insertVal(root1, vB, 0, sz - 1, Rv_[vB]);
        // target state 1: edge i charges node i+1
        int vC = iBase[i];    // from parent state 0
        int vD = iLeft[i];    // from parent state 1
        int c2 = cmpMul(root0, root1, 0, sz - 1, vC, vD);
        int nr1;
        if (c2 >= 0) nr1 = insertVal(root0, vC, 0, sz - 1, Rv_[vC]);
        else         nr1 = insertVal(root1, vD, 0, sz - 1, Rv_[vD]);
        root0 = nr0;
        root1 = nr1;
    }

    // last node K-1
    int vE = iBase[K - 1]; // from parent state 0
    int vF = iLeft[K - 1]; // from parent state 1
    int c = cmpMul(root0, root1, 0, sz - 1, vE, vF);
    int finalRoot;
    if (c >= 0) finalRoot = insertVal(root0, vE, 0, sz - 1, Rv_[vE]);
    else        finalRoot = insertVal(root1, vF, 0, sz - 1, Rv_[vF]);

    extractMul(finalRoot, 0, sz - 1, vals, ans);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;
    string outBuf;
    while (T--) {
        vector<ll> ans;
        int n, m;
        cin >> n >> m;
        vector<ll> l1(n), r1(n), l2(m), r2(m);
        for (int i = 0; i < n; i++) cin >> l1[i] >> r1[i];
        for (int j = 0; j < m; j++) cin >> l2[j] >> r2[j];

        // ---------- containment ----------
        vector<char> cont1(n, 0), cont2(m, 0);
        for (int i = 0; i < n; i++) {
            int j = (int)(upper_bound(l2.begin(), l2.end(), l1[i]) - l2.begin()) - 1;
            if (j >= 0 && r2[j] >= r1[i] && !(l2[j] == l1[i] && r2[j] == r1[i]))
                cont1[i] = 1;
        }
        for (int j = 0; j < m; j++) {
            int i = (int)(upper_bound(l1.begin(), l1.end(), l2[j]) - l1.begin()) - 1;
            if (i >= 0 && r1[i] >= r2[j] && !(l1[i] == l2[j] && r1[i] == r2[j]))
                cont2[j] = 1;
        }

        // ---------- nodes ----------
        vector<int> nid1(n, -1), nid2(m, -1);
        vector<ll> nL;
        nL.reserve(n + m);
        for (int i = 0; i < n; i++)
            if (!cont1[i]) { nid1[i] = (int)nL.size(); nL.push_back(r1[i] - l1[i] + 1); }
        for (int j = 0; j < m; j++)
            if (!cont2[j]) { nid2[j] = (int)nL.size(); nL.push_back(r2[j] - l2[j] + 1); }
        int K = (int)nL.size();

        // ---------- edges (crossing overlaps) ----------
        vector<vector<pair<int, ll>>> adj(K);
        for (int i = 0; i < n; i++) {
            if (cont1[i]) continue;
            int jlo = (int)(lower_bound(r2.begin(), r2.end(), l1[i]) - r2.begin());
            int jhi = (int)(upper_bound(l2.begin(), l2.end(), r1[i]) - l2.begin()) - 1;
            if (jlo > jhi) continue;
            int a = nid1[i];
            for (int t = 0; t < 2; t++) {
                int j = (t == 0 ? jlo : jhi);
                if (t == 1 && jhi == jlo) break;
                // J strictly contained in I -> J removed (no edge)
                if (l1[i] <= l2[j] && r2[j] <= r1[i] &&
                    !(l1[i] == l2[j] && r1[i] == r2[j]))
                    continue;
                if (cont2[j]) continue;  // safety
                int b = nid2[j];
                if (b < 0) continue;
                ll s = min(r1[i], r2[j]) - max(l1[i], l2[j]) + 1;
                adj[a].push_back({b, s});
                adj[b].push_back({a, s});
            }
        }

        // ---------- components (paths) ----------
        vector<char> vis(K, 0);
        for (int st = 0; st < K; st++) {
            if (vis[st]) continue;
            // move to an endpoint
            int cur = st, prev = -1;
            if (!adj[cur].empty()) {
                while (true) {
                    int nxt = -1;
                    for (auto& e : adj[cur]) {
                        if (e.first != prev) { nxt = e.first; break; }
                    }
                    if (nxt == -1) break;
                    prev = cur;
                    cur = nxt;
                }
            }
            int start = cur;
            vector<ll> Ls, ss;
            prev = -1; cur = start;
            while (true) {
                vis[cur] = 1;
                Ls.push_back(nL[cur]);
                int nxt = -1;
                ll nxtS = 0;
                for (auto& e : adj[cur]) {
                    if (e.first != prev) { nxt = e.first; nxtS = e.second; break; }
                }
                if (nxt == -1 || vis[nxt]) break;
                ss.push_back(nxtS);
                prev = cur;
                cur = nxt;
            }
            solvePath(Ls, ss, ans);
        }

        sort(ans.begin(), ans.end(), greater<ll>());
        outBuf += to_string(ans.size());
        outBuf += '\n';
        for (size_t i = 0; i < ans.size(); i++) {
            if (i) outBuf += ' ';
            outBuf += to_string(ans[i]);
        }
        outBuf += '\n';
    }
    fwrite(outBuf.data(), 1, outBuf.size(), stdout);
    return 0;
}

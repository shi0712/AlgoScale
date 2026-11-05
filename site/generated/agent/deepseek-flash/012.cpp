#include <bits/stdc++.h>
#ifndef MINMUL
#define MINMUL 32
#endif
#ifndef BLK
#define BLK 32
#endif

using namespace std;
typedef vector<int> vi;
static const int MOD = 998244353;

static int pw_(long long a, long long e) {
    long long r = 1; a %= MOD;
    while (e > 0) { if (e & 1) r = r * a % MOD; a = a * a % MOD; e >>= 1; }
    return (int)r;
}

static int se_[40], sie_[40];
static void init_ntt() {
    int cnt2 = 23;
    long long e = pw_(3, (MOD - 1) >> cnt2), ie = pw_(e, MOD - 2);
    long long es[40], ies[40];
    for (int i = cnt2; i >= 2; i--) { es[i - 2] = e; ies[i - 2] = ie; e = e * e % MOD; ie = ie * ie % MOD; }
    long long now = 1;
    for (int i = 0; i <= cnt2 - 2; i++) { se_[i] = (int)(now * es[i] % MOD); now = now * ies[i] % MOD; }
    now = 1;
    for (int i = 0; i <= cnt2 - 2; i++) { sie_[i] = (int)(now * ies[i] % MOD); now = now * es[i] % MOD; }
}
static inline int bsf_(unsigned int x) { return __builtin_ctz(x); }
static void butterfly(vi &a) {
    int n = (int)a.size();
    int h = 31 - __builtin_clz((unsigned)n);
    for (int ph = 1; ph <= h; ph++) {
        int w = 1 << (ph - 1), p = 1 << (h - ph);
        long long now = 1;
        for (int s = 0; s < w; s++) {
            int offset = s << (h - ph + 1);
            int *base = a.data() + offset;
            for (int i = 0; i < p; i++) {
                int l = base[i];
                int r = (int)((long long)base[i + p] * now % MOD);
                int x = l + r; if (x >= MOD) x -= MOD;
                int y = l - r; if (y < 0) y += MOD;
                base[i] = x; base[i + p] = y;
            }
            now = now * se_[bsf_(~(unsigned)s)] % MOD;
        }
    }
}
static void butterfly_inv(vi &a) {
    int n = (int)a.size();
    int h = 31 - __builtin_clz((unsigned)n);
    for (int ph = h; ph >= 1; ph--) {
        int w = 1 << (ph - 1), p = 1 << (h - ph);
        long long inow = 1;
        for (int s = 0; s < w; s++) {
            int offset = s << (h - ph + 1);
            int *base = a.data() + offset;
            for (int i = 0; i < p; i++) {
                int l = base[i], r = base[i + p];
                int x = l + r; if (x >= MOD) x -= MOD;
                int y = l - r; if (y < 0) y += MOD;
                base[i] = x; base[i + p] = (int)((long long)y * inow % MOD);
            }
            inow = inow * sie_[bsf_(~(unsigned)s)] % MOD;
        }
    }
    int ninv = pw_(n, MOD - 2);
    for (auto &x : a) x = (int)((long long)x * ninv % MOD);
}

static vi mul(const vi &a, const vi &b) {
    if (a.empty() || b.empty()) return vi();
    int n = (int)a.size(), m = (int)b.size();
    if (min(n, m) <= MINMUL) {
        vi c(n + m - 1, 0);
        for (int i = 0; i < n; i++) {
            if (!a[i]) continue;
            long long ai = a[i];
            for (int j = 0; j < m; j++) c[i + j] = (int)((c[i + j] + ai * b[j]) % MOD);
        }
        return c;
    }
    int need = n + m - 1, sz = 1;
    while (sz < need) sz <<= 1;
    vi fa(a.begin(), a.end()), fb(b.begin(), b.end());
    fa.resize(sz); fb.resize(sz);
    butterfly(fa); butterfly(fb);
    for (int i = 0; i < sz; i++) fa[i] = (int)((long long)fa[i] * fb[i] % MOD);
    butterfly_inv(fa);
    fa.resize(need);
    return fa;
}

static vi pinv(const vi &a, int n) {
    vi r(1, pw_(a[0], MOD - 2));
    while ((int)r.size() < n) {
        int m = min(2 * (int)r.size(), n);
        vi f(a.begin(), a.begin() + min((int)a.size(), m));
        vi fr = mul(f, r);
        fr.resize(m);
        for (auto &x : fr) if (x) x = MOD - x;
        fr[0] = (fr[0] + 2) % MOD;
        r = mul(r, fr);
        r.resize(m);
    }
    return r;
}

// a mod b ; irev = inverse of reversed b truncated (length >= deg(a)-deg(b)+2)
static vi pmod(const vi &a, const vi &b, const vi &irev) {
    int n = (int)a.size(), m = (int)b.size();
    if (n < m) return a;
    if (m == 1) return vi();
    int qlen = n - m + 1;
    vi ra(qlen);
    for (int i = 0; i < qlen; i++) ra[i] = a[n - 1 - i];
    vi t(qlen, 0);
    if (qlen <= (int)irev.size()) {
        for (int i = 0; i < qlen; i++) t[i] = irev[i];
    } else {                       // defensive: lengthen the inverse series if ever needed
        vi rev(b.rbegin(), b.rend());
        vi full = pinv(rev, qlen);
        for (int i = 0; i < qlen; i++) t[i] = full[i];
    }
    vi q = mul(ra, t);
    q.resize(qlen);
    reverse(q.begin(), q.end());
    vi bq = mul(b, q);
    vi r(m - 1, 0);
    for (int i = 0; i < m - 1; i++) {
        long long v = (i < n ? a[i] : 0);
        if (i < (int)bq.size()) v -= bq[i];
        v %= MOD; if (v < 0) v += MOD;
        r[i] = (int)v;
    }
    return r;
}

// ---------------- per test ----------------
struct PT {
    int lo, hi;
    vi poly, irev, actL;
    int lc, rc;
};
static vector<PT> pt;
static int n_;
static vi Ans, factAt, Evec, prefAct;
static const int B = BLK;

static int buildPT(int lo, int hi) {
    int id = (int)pt.size();
    pt.push_back(PT{lo, hi, {}, {}, {}, -1, -1});
    if (hi - lo == 1) {
        pt[id].poly = vi{(int)((MOD - (long long)(lo % MOD)) % MOD), 1};
        return id;
    }
    int mid = (lo + hi) >> 1;
    int l = buildPT(lo, mid), r = buildPT(mid, hi);
    pt[id].lc = l; pt[id].rc = r;
    pt[id].poly = mul(pt[l].poly, pt[r].poly);
    return id;
}

static vi truncMul(const vi &a, const vi &b, int n) {
    if (n <= 0 || a.empty() || b.empty()) return vi();
    if (min(a.size(), b.size()) <= MINMUL) {
        vi c(n, 0);
        for (int i = 0; i < (int)a.size() && i < n; i++) {
            if (!a[i]) continue;
            long long ai = a[i];
            int lim = min((int)b.size(), n - i);
            for (int j = 0; j < lim; j++) c[i + j] = (int)((c[i + j] + ai * b[j]) % MOD);
        }
        return c;
    }
    vi c = mul(a, b);
    c.resize(min((int)c.size(), n));
    return c;
}
// d = degree bound of the carried polynomial at this node (d < size(id))
static void buildIrevTop(int id, int d) {
    int lo = pt[id].lo, hi = pt[id].hi;
    if (hi - lo <= B) return;
    int lc = pt[id].lc, rc = pt[id].rc;
    int sL = pt[lc].hi - pt[lc].lo, sR = pt[rc].hi - pt[rc].lo;
    int mid = pt[rc].lo;
    int a = prefAct[mid + 1] - prefAct[lo + 1];   // # factors with lo < Delta <= mid
    int ql = d - sL + 1, qr = d + a - sR + 1;
    int needL = max(ql, sL + 2), needR = max(qr, sR + 2);
    {
        vi revR(pt[rc].poly.rbegin(), pt[rc].poly.rend());
        vi t(pt[id].irev.begin(), pt[id].irev.begin() + min((size_t)needL, pt[id].irev.size()));
        pt[lc].irev = truncMul(t, revR, needL);
        pt[lc].irev.resize(needL, 0);
    }
    {
        vi revL(pt[lc].poly.rbegin(), pt[lc].poly.rend());
        vi t(pt[id].irev.begin(), pt[id].irev.begin() + min((size_t)needR, pt[id].irev.size()));
        pt[rc].irev = truncMul(t, revL, needR);
        pt[rc].irev.resize(needR, 0);
    }
    buildIrevTop(lc, min(d, sL - 1));
    buildIrevTop(rc, min(d + a, sR - 1));
}

static vi factorPoly(int j) { // (E_j - x)
    return vi{(int)(Evec[j] % MOD), MOD - 1};
}

// returns product of factors with lo < Delta <= hi ; stores left-half product in pt[id].actL when node split
static vi buildAct(int id) {
    int lo = pt[id].lo, hi = pt[id].hi;
    if (hi - lo <= B) {
        vi p{1};
        for (int v = lo + 1; v <= hi; v++) {
            int j = (v <= n_ ? factAt[v] : -1);
            if (j >= 0) p = mul(p, factorPoly(j));
        }
        return p;
    }
    int mid = (lo + hi) >> 1;
    int lc = pt[id].lc, rc = pt[id].rc;
    vi L = buildAct(lc);
    vi R = buildAct(rc);
    pt[id].actL = L;
    return mul(L, R);
}

static void solve(int id, vi P) {
    int lo = pt[id].lo, hi = pt[id].hi;
    if (hi - lo <= B) {
        for (int u = lo; u < hi; u++) {
            long long val = 0;
            for (int k = (int)P.size() - 1; k >= 0; k--) val = (val * u + P[k]) % MOD;
            for (int v = lo + 1; v <= u; v++) {
                int j = (v <= n_ ? factAt[v] : -1);
                if (j >= 0) {
                    long long f = ((Evec[j] - u) % MOD + MOD) % MOD;
                    val = val * f % MOD;
                }
            }
            Ans[u] = (int)val;
        }
        return;
    }
    int mid = (lo + hi) >> 1;
    int lc = pt[id].lc, rc = pt[id].rc;
    vi PL = pmod(P, pt[lc].poly, pt[lc].irev);
    solve(lc, PL);
    vi PA = mul(P, pt[id].actL);
    vi PR = pmod(PA, pt[rc].poly, pt[rc].irev);
    solve(rc, PR);
}

static char ibuf_[1 << 16];
static int ipos_ = 0, ilen_ = 0;
static inline int gc_() {
    if (ipos_ == ilen_) { ilen_ = (int)fread(ibuf_, 1, sizeof(ibuf_), stdin); ipos_ = 0; if (ilen_ <= 0) return -1; }
    return ibuf_[ipos_++];
}
static inline int readInt_() {
    int c = gc_();
    while (c != -1 && (c < '0' || c > '9')) c = gc_();
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc_(); }
    return x;
}

int main() {
    init_ntt();
    int T = readInt_();
    if (T <= 0) return 0;
    // read all tests
    struct Test { int n, m; vector<int> a; };
    vector<Test> tests(T);
    int maxn = 2;
    for (int t = 0; t < T; t++) {
        int n = readInt_(), m = readInt_();
        tests[t].n = n; tests[t].m = m;
        tests[t].a.resize(m);
        for (int i = 0; i < m; i++) tests[t].a[i] = readInt_();
        maxn = max(maxn, n);
    }
    // inverses up to 2*maxn
    int LIM = 2 * maxn + 10;
    vector<int> inv(LIM);
    inv[1] = 1;
    for (int i = 2; i < LIM; i++) inv[i] = (int)((long long)(MOD - MOD / i) * inv[MOD % i] % MOD);

    for (int t = 0; t < T; t++) {
        int n = tests[t].n, m = tests[t].m;
        vector<int> &a = tests[t].a;
        // A[] and nxt
        vector<int> nxt(m, -1), last(2 * n + 2, -1);
        for (int i = m - 1; i >= 0; i--) { nxt[i] = last[a[i]]; last[a[i]] = i; }
        vector<long long> A(m, 0);
        for (int i = m - 2; i >= 0; i--) {
            long long b = A[i + 1] + 1;
            if (nxt[i] >= 0) b = max(b, A[nxt[i]] + n + 1);
            A[i] = b;
        }
        long long A0 = A[0];
        // first occurrences and Deltas
        vector<char> seen(2 * n + 2, 0);
        vector<long long> Deltas; // 1-based order
        for (int i = 0; i < m; i++) {
            if (!seen[a[i]]) { seen[a[i]] = 1; Deltas.push_back(A0 - A[i]); }
        }
        // active factors: Delta <= n-1
        Evec.clear(); factAt.assign(n + 2, -1);
        int Kact = 0;
        for (int j = 0; j < (int)Deltas.size(); j++) {
            long long d = Deltas[j];
            if (d <= n - 1) {
                Kact++;
                int jj = j + 1; // 1-based
                long long E = 2LL * n + d - jj;
                Evec.push_back((int)(E % MOD));
                factAt[(int)d] = (int)Evec.size() - 1;
            }
        }
        prefAct.assign(n + 2, 0);
        {
            vector<int> cntAt(n + 1, 0);
            for (int j = 0; j < (int)Deltas.size(); j++) if (Deltas[j] <= n - 1) cntAt[(int)Deltas[j]]++;
            for (int v = 0; v <= n; v++) prefAct[v + 1] = prefAct[v] + cntAt[v];
        }
        // compute N(u) for u=0..n-1
        Ans.assign(n, 0);
        n_ = n;
        double lg = log2((double)n);
        bool useNaive = (n <= B) || ((double)Kact * n <= 6.3 * lg * lg * n + 1e5);
#ifdef FORCE_NAIVE
        useNaive = (n <= B);
#endif
#ifdef FORCE_POLY
        useNaive = false;
#endif
        if (useNaive) {
            // naive: factors in Delta order (== 1-based j order)
            int nf = (int)Evec.size();
            for (int u = 0; u < n; u++) {
                long long val = 1;
                for (int j = 0; j < nf; j++) {
                    long long E = Evec[j];      // 2n + Delta_j - j
                    long long d = E - 2LL * n + (j + 1); // Delta_j
                    if (d > u) break;
                    val = val * ((E - u) % MOD) % MOD;
                }
                Ans[u] = (int)val;
            }
        } else {
            pt.clear();
            int root = buildPT(0, n);
            if (pt[root].lc >= 0) {
                int sL0 = pt[pt[root].lc].hi - pt[pt[root].lc].lo;
                int sR0 = pt[pt[root].rc].hi - pt[pt[root].rc].lo;
                int need = max(sL0, sR0) + 2;
                vi rev(pt[root].poly.rbegin(), pt[root].poly.rend());
                pt[root].irev = pinv(rev, need);
            }
            buildIrevTop(root, 1);
            buildAct(root);
            vi P = factorPoly(factAt[0] >= 0 ? factAt[0] : 0);
            // base at u=0: factor with Delta = 0 (always exists: first occurrence has Delta=0)
            solve(root, P);
            pt.clear();
        }
        // sum
        long long S = 0;
        long long Dinv = 1; // product_{j=1..q} inv(2n-j+1)
        int q = 0;
        for (int u = 0; u < n; u++) {
            while (q < Kact) {
                long long d = Deltas[q];
                if (d <= u) {
                    q++;
                    Dinv = Dinv * inv[2 * n - q + 1] % MOD;
                } else break;
            }
            long long Pv = (long long)Ans[u] * Dinv % MOD;
            S = (S + 1 - Pv) % MOD;
            if (S < 0) S += MOD;
        }
        long long ans = ((A0 % MOD) + 1 + S) % MOD;
        printf("%lld\n", ans);
    }
    return 0;
}

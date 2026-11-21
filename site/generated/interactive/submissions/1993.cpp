// Hidden Bipartite Graph  (interactive)
//
// Protocol (as in the statement):
//   query :  "? k"  newline  "v1 ... vk"  newline
//   answer:  "Y s"  newline  "a1 ... as"  newline
//         or "N l"  newline  "c1 ... cl"  newline
//
// Idea
// ----
// Build a spanning tree with a best-first search (the unexplored discovered
// vertex with the smallest index is expanded first).  The undiscovered vertices
// are kept in the sorted list U.  For an expanded vertex v we locate *all* of
// its neighbours inside U with a recursive split: for a range C we know how
// many neighbours of v lie in C; split C in halves, query both partial counts
// and recurse.  If a half contains fewer non-neighbours than neighbours we
// enumerate the non-neighbours and take the complement (fast for nearly
// complete halves).  Answers of set queries are memoised, and two adaptive
// probes try the first/last element of U directly (on many structured inputs
// the next vertex sits exactly at a border).
//
// The tree depth parity gives a 2-colouring.  Querying the two colour classes
// proves bipartiteness; otherwise an edge inside one class plus the tree path
// between its endpoints is an odd cycle.

#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<int> par, lev;      // spanning tree
static vector<int> U;             // undiscovered vertices (ascending)
static long long qU = 0;          // edges with both endpoints in U

/* ------------------------------------------------------------------ */
/* output buffer                                                       */
/* ------------------------------------------------------------------ */
static char obuf[1 << 22];
static size_t olen = 0;
static inline void oflush() {
    if (olen) { fwrite(obuf, 1, olen, stdout); fflush(stdout); olen = 0; }
}
static inline void oputc(char c) {
    if (olen == sizeof(obuf)) oflush();
    obuf[olen++] = c;
}
static inline void oputint(int x) {
    char t[12]; int k = 0;
    if (x == 0) { oputc('0'); return; }
    while (x) { t[k++] = char('0' + x % 10); x /= 10; }
    while (k) oputc(t[--k]);
}

/* ------------------------------------------------------------------ */
/* queries with memoisation (a query only depends on the set)          */
/* ------------------------------------------------------------------ */
struct VecHash {
    size_t operator()(const vector<int>& v) const {
        uint64_t h = 1469598103934665603ULL;
        for (int x : v) {
            h ^= (uint64_t)(x + 0x9e3779b97f4a7c15ULL);
            h *= 1099511628211ULL;
            h ^= h >> 29;
        }
        return (size_t)h;
    }
};
static unordered_map<vector<int>, long long, VecHash> qcache;

static long long askRaw(const vector<int>& s) {
    oputc('?'); oputc(' ');
    oputint((int)s.size());
    oputc('\n');
    for (size_t i = 0; i < s.size(); i++) {
        if (i) oputc(' ');
        oputint(s[i]);
    }
    oputc('\n');
    oflush();
    long long r = 0;
    if (scanf("%lld", &r) != 1) exit(0);
    return r;
}

static long long askSet(const vector<int>& s) {
    vector<int> t = s;
    bool sorted = true;
    for (size_t i = 1; i < t.size(); i++)
        if (t[i - 1] > t[i]) { sorted = false; break; }
    if (!sorted) sort(t.begin(), t.end());
    auto it = qcache.find(t);
    if (it != qcache.end()) return it->second;
    long long v = askRaw(t);
    qcache.emplace(std::move(t), v);
    return v;
}

static inline bool qKnown(const vector<int>& sortedSet) {
    return qcache.find(sortedSet) != qcache.end();
}

static void outputLine(char tag, const vector<int>& v) {
    oputc(tag); oputc(' ');
    oputint((int)v.size());
    oputc('\n');
    for (size_t i = 0; i < v.size(); i++) {
        if (i) oputc(' ');
        oputint(v[i]);
    }
    oputc('\n');
    oflush();
}

/* ------------------------------------------------------------------ */
/* searching the undiscovered set U                                    */
/* ------------------------------------------------------------------ */
// Let lo < mid < hi.  dC is the number of edges between v and U[lo..hi).
// Returns dL = edges(v, U[lo..mid)), dR = edges(v, U[mid..hi)).
static void splitCount(int lo, int mid, int hi, int v, long long dC,
                       long long& dL, long long& dR) {
    vector<int> sl(U.begin() + lo, U.begin() + mid);
    vector<int> sr(U.begin() + mid, U.begin() + hi);
    bool kl = sl.size() > 1 && qKnown(sl);
    bool kr = sr.size() > 1 && qKnown(sr);
    if (kr && !kl) {                       // the right half is already known
        long long qr = askSet(sr);
        vector<int> t = sr; t.push_back(v);
        dR = askSet(t) - qr;
        dL = dC - dR;
    } else {
        long long ql = (sl.size() <= 1) ? 0 : askSet(sl);
        vector<int> t = sl; t.push_back(v);
        dL = askSet(t) - ql;
        dR = dC - dL;
    }
}

static void collectHits(int lo, int hi, int v, long long cnt, vector<int>& out);
static void collectMiss(int lo, int hi, int v, long long cnt, vector<int>& out);

static void appendRange(int lo, int hi, vector<int>& out) {
    for (int i = lo; i < hi; i++) out.push_back(U[i]);
}

// append U[lo..hi) without the elements listed in excl (both ascending)
static void appendComplement(int lo, int hi, const vector<int>& excl, vector<int>& out) {
    size_t j = 0;
    for (int i = lo; i < hi; i++) {
        if (j < excl.size() && excl[j] == U[i]) j++;
        else out.push_back(U[i]);
    }
}

// out += neighbours of v inside U[lo..hi)   (cnt = their number, cnt > 0)
static void collectHits(int lo, int hi, int v, long long cnt, vector<int>& out) {
    if (cnt <= 0) return;
    int sz = hi - lo;
    if (cnt == sz) { appendRange(lo, hi, out); return; }
    if (sz == 1) { out.push_back(U[lo]); return; }
    int mid = (lo + hi) >> 1;
    long long dL, dR;
    splitCount(lo, mid, hi, v, cnt, dL, dR);
    int szL = mid - lo, szR = hi - mid;
    if (dL > 0) {
        if (szL - dL < dL) {               // fewer non-neighbours here
            vector<int> miss;
            collectMiss(lo, mid, v, szL - dL, miss);
            appendComplement(lo, mid, miss, out);
        } else collectHits(lo, mid, v, dL, out);
    }
    if (dR > 0) {
        if (szR - dR < dR) {
            vector<int> miss;
            collectMiss(mid, hi, v, szR - dR, miss);
            appendComplement(mid, hi, miss, out);
        } else collectHits(mid, hi, v, dR, out);
    }
}

// out += non-neighbours of v inside U[lo..hi)  (cnt = their number, cnt > 0)
static void collectMiss(int lo, int hi, int v, long long cnt, vector<int>& out) {
    if (cnt <= 0) return;
    int sz = hi - lo;
    if (cnt == sz) { appendRange(lo, hi, out); return; }
    if (sz == 1) { out.push_back(U[lo]); return; }
    int mid = (lo + hi) >> 1;
    long long dL, dR;
    splitCount(lo, mid, hi, v, sz - cnt, dL, dR);
    int szL = mid - lo, szR = hi - mid;
    long long nL = szL - dL, nR = szR - dR;      // non-neighbours in the halves
    if (nL > 0) {
        if (dL < nL) {
            vector<int> hit;
            collectHits(lo, mid, v, dL, hit);
            appendComplement(lo, mid, hit, out);
        } else collectMiss(lo, mid, v, nL, out);
    }
    if (nR > 0) {
        if (dR < nR) {
            vector<int> hit;
            collectHits(mid, hi, v, dR, hit);
            appendComplement(mid, hi, hit, out);
        } else collectMiss(mid, hi, v, nR, out);
    }
}

/* ------------------------------------------------------------------ */
/* one edge inside a colour class (only used when non-bipartite)       */
/* ------------------------------------------------------------------ */
static pair<int, int> findCross(vector<int> X, vector<int> Y);

static pair<int, int> findEdge(vector<int> S) {
    if (S.size() <= 1) return {-1, -1};
    size_t h = S.size() / 2;
    vector<int> S1(S.begin(), S.begin() + h), S2(S.begin() + h, S.end());
    long long q1 = (S1.size() <= 1) ? 0 : askSet(S1);
    if (q1 > 0) return findEdge(S1);
    long long q2 = (S2.size() <= 1) ? 0 : askSet(S2);
    if (q2 > 0) return findEdge(S2);
    return findCross(S1, S2);
}

static pair<int, int> findCross(vector<int> X, vector<int> Y) {
    while (!(X.size() == 1 && Y.size() == 1)) {
        if (Y.size() >= X.size() && Y.size() > 1) {
            size_t h = Y.size() / 2;
            vector<int> Y1(Y.begin(), Y.begin() + h), Y2(Y.begin() + h, Y.end());
            long long qX = (X.size() <= 1) ? 0 : askSet(X);
            long long qY1 = (Y1.size() <= 1) ? 0 : askSet(Y1);
            vector<int> u = X; u.insert(u.end(), Y1.begin(), Y1.end());
            long long c1 = askSet(u) - qX - qY1;
            if (c1 > 0) Y.swap(Y1); else Y.swap(Y2);
        } else {
            size_t h = X.size() / 2;
            vector<int> X1(X.begin(), X.begin() + h), X2(X.begin() + h, X.end());
            long long qY = (Y.size() <= 1) ? 0 : askSet(Y);
            long long qX1 = (X1.size() <= 1) ? 0 : askSet(X1);
            vector<int> u = X1; u.insert(u.end(), Y.begin(), Y.end());
            long long c1 = askSet(u) - qY - qX1;
            if (c1 > 0) X.swap(X1); else X.swap(X2);
        }
    }
    return {X[0], Y[0]};
}

// tree path u..w followed by the edge (w,u)  (length = dist+1, odd)
static vector<int> cycleFrom(int u, int w) {
    vector<int> pa, pb;
    int a = u, b = w;
    while (lev[a] > lev[b]) { pa.push_back(a); a = par[a]; }
    while (lev[b] > lev[a]) { pb.push_back(b); b = par[b]; }
    while (a != b) { pa.push_back(a); a = par[a]; pb.push_back(b); b = par[b]; }
    pa.push_back(a);                                  // LCA
    for (int i = (int)pb.size() - 1; i >= 0; i--) pa.push_back(pb[i]);
    return pa;
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */
static const int PROBE_MAX = 4;      // probes per border and vertex
static const int PROBE_WARMUP = 4;   // attempts before the hit rate is used
static const int PROBE_RATIO = 8;    // keep probing while hit rate >= 1/8
static int pAtt[2] = {0, 0}, pHit[2] = {0, 0};

static inline bool probeOn(int d) {
    return pAtt[d] < PROBE_WARMUP || PROBE_RATIO * pHit[d] >= pAtt[d];
}

static void solve() {
    if (n == 1) { outputLine('Y', vector<int>{1}); return; }

    par.assign(n + 1, 0);
    lev.assign(n + 1, 0);
    U.clear();
    for (int i = 2; i <= n; i++) U.push_back(i);
    qU = (U.size() <= 1) ? 0 : askSet(U);

    vector<char> inU(n + 1, 0);
    for (int i = 2; i <= n; i++) inU[i] = 1;

    priority_queue<int, vector<int>, greater<int>> que;
    que.push(1);

    while (!que.empty() && !U.empty()) {
        int v = que.top(); que.pop();

        vector<int> s = U; s.push_back(v);
        long long dU = askSet(s) - qU;           // neighbours of v in U
        if (dU <= 0) continue;

        vector<int> found;
        int lo = 0, hi = (int)U.size();
        long long rem = dU;

        if (dU != (long long)U.size()) {
            for (int d = 0; d < 2; d++) {        // 0 = front, 1 = back
                if (!probeOn(d)) continue;
                for (int t = 0; t < PROBE_MAX && rem > 0 && lo < hi; t++) {
                    int idx = (d == 0) ? lo : (hi - 1);
                    pAtt[d]++;
                    if (askSet(vector<int>{v, U[idx]}) > 0) {
                        found.push_back(U[idx]);
                        pHit[d]++;
                        if (d == 0) lo++; else hi--;
                        rem--;
                    } else break;
                }
            }
        }
        if (rem > 0) collectHits(lo, hi, v, rem, found);

        for (int u : found) {
            inU[u] = 0;
            par[u] = v;
            lev[u] = lev[v] + 1;
            que.push(u);
        }
        size_t w = 0;
        for (size_t i = 0; i < U.size(); i++) if (inU[U[i]]) U[w++] = U[i];
        U.resize(w);
        qU = (U.size() <= 1) ? 0 : askSet(U);
    }

    vector<int> A, B;
    A.reserve(n); B.reserve(n);
    for (int i = 1; i <= n; i++) { if (lev[i] & 1) B.push_back(i); else A.push_back(i); }

    long long qa = (A.size() <= 1) ? 0 : askSet(A);
    if (qa == 0) {
        long long qb = (B.size() <= 1) ? 0 : askSet(B);
        if (qb == 0) { outputLine('Y', A); return; }
        pair<int, int> e = findEdge(B);
        outputLine('N', cycleFrom(e.first, e.second));
        return;
    }
    pair<int, int> e = findEdge(A);
    outputLine('N', cycleFrom(e.first, e.second));
}

/* ------------------------------------------------------------------ */
int main() {
    if (scanf("%d", &n) != 1) return 0;
    solve();
    return 0;
}

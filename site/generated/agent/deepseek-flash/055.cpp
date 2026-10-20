// Tree Shadow Folding
//
// The current tree is kept as a "spine" (a diameter path) plus hanging
// subtrees ("items").  A fold merges spine position i with spine position
// K-1-i, so the item bags of symmetric positions are melded pairwise; the
// folded tree is again a path of m = ceil(K/2) positions carrying bags.
//
// Every item hanging at position i has height <= i, so the diameter of the
// folded tree is  D' = max_i (h_i + i)  (h_i = height of the deepest item at i).
// Hence the new spine is positions 0..j*  (j* = argmax h_i+i)  followed by the
// deepest chain inside the deepest item at j*, while positions j*+1..m-1 hang
// off j* as a bare "path item".
//
// Each bag only needs meld / max / pop-max, so bags are leftist heaps keyed by
// item height.
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
using namespace std;

// ------------------------------------------------------------------- I/O ----
static char ibuf[1 << 16];
static int ipos = 0, ilen = 0;
static inline int gc_() {
    if (ipos == ilen) {
        ilen = (int)fread(ibuf, 1, sizeof(ibuf), stdin);
        ipos = 0;
        if (ilen <= 0) return -1;
    }
    return ibuf[ipos++];
}
static inline int readInt() {
    int c = gc_();
    while (c != -1 && (c < '0' || c > '9')) c = gc_();
    if (c == -1) return -1;
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc_(); }
    return x;
}
static vector<char> outbuf;
static inline void writeInt(int x) {
    char tmp[12]; int l = 0;
    if (x == 0) tmp[l++] = '0';
    while (x) { tmp[l++] = char('0' + x % 10); x /= 10; }
    while (l) outbuf.push_back(tmp[--l]);
    outbuf.push_back('\n');
}

// ----------------------------------------------------------------- items ----
// An item node is simultaneously a leftist-heap node; index 0 is the null node.
//   typ 0 ("piece") : p1 = attachment vertex u, p2 = subtree root v;
//                     key = distance from u down to the deepest leaf through v.
//   typ 1 ("path")  : p1 = offset into pathEntries, p2 = number of entries;
//                     key = distance from the attachment point to the deepest
//                     leaf inside the path item.
static vector<int> ikey, ihl, ihr, idist, ityp, ip1, ip2;
static vector<pair<int, int> > pathEntries;  // (bag root, height of bag)

static inline int newItem(int key, int typ, int a, int b) {
    int id = (int)ikey.size();
    ikey.push_back(key); ihl.push_back(0); ihr.push_back(0);
    idist.push_back(1); ityp.push_back(typ); ip1.push_back(a); ip2.push_back(b);
    return id;
}

// Leftist heaps: a right spine has length <= floor(log2(size+1)) <= 20 here,
// so melding two heaps touches at most ~40 entries.
static int mstk[256];
static inline int meld(int a, int b) {
    if (!a) return b;
    if (!b) return a;
    int sp = 0;
    while (a && b) {
        if (ikey[a] < ikey[b]) { int t = a; a = b; b = t; }
        mstk[sp++] = a;
        a = ihr[a];
    }
    int m = a ? a : b;
    while (sp) {
        int x = mstk[--sp];
        ihr[x] = m;
        if (idist[ihl[x]] < idist[ihr[x]]) { int t = ihl[x]; ihl[x] = ihr[x]; ihr[x] = t; }
        idist[x] = idist[ihr[x]] + 1;
        m = x;
    }
    return m;
}
static inline int bagKey(int bag) { return bag ? ikey[bag] : 0; }

// --------------------------------------------------------------- globals ----
static int n;
static vector<int> adjStart, adjList;
static vector<int> par_, downv, upv, order_, bq, bdist, bpar;
static vector<int> best1, best2, arg1;
static vector<int> suf;
static vector<int> spine_, nbag_, nspine_;
static vector<char> onpath;

static inline int Hfun(int u, int v) { return par_[v] == u ? downv[v] : upv[u]; }

// Append to `out` the bags for the spine positions produced by walking the
// deepest chain inside item `start`.
static vector<int> frPos, frToff, frTlen, frSub;
static void promote(int start, vector<int>& out) {
    int it = start;
    for (;;) {
        bool descended = false;
        if (ityp[it] == 0) {
            int cur = ip2[it], pr = ip1[it];
            for (;;) {
                int s = adjStart[cur], e = adjStart[cur + 1];
                int bestw = -1, besth = -1;
                for (int p = s; p < e; p++) {
                    int w = adjList[p];
                    if (w == pr) continue;
                    int h = Hfun(cur, w);
                    if (h > besth) { besth = h; bestw = w; }
                }
                int bag = 0;
                for (int p = s; p < e; p++) {
                    int w = adjList[p];
                    if (w == pr || w == bestw) continue;
                    bag = meld(bag, newItem(1 + Hfun(cur, w), 0, cur, w));
                }
                out.push_back(bag);
                if (bestw < 0) break;
                pr = cur; cur = bestw;
            }
        } else {
            int off = ip1[it], len = ip2[it];
            suf.resize(len + 1);
            suf[len] = -1;
            for (int t = len - 1; t >= 0; t--) {
                int h = pathEntries[off + t].second;
                int nx = 1 + suf[t + 1];
                suf[t] = h > nx ? h : nx;
            }
            int t = 0;
            for (;;) {
                int h = pathEntries[off + t].second;
                if (t + 1 < len && 1 + suf[t + 1] > h) {
                    out.push_back(pathEntries[off + t].first);
                    t++;
                    continue;
                }
                int bag = pathEntries[off + t].first;
                int nb = bag ? meld(ihl[bag], ihr[bag]) : 0;
                int pos = (int)out.size();
                out.push_back(nb);
                int subH = (t + 1 < len) ? 1 + suf[t + 1] : 0;
                if (bag) {  // dive into the deepest item, continuing the chain
                    if (t + 1 < len) {
                        frPos.push_back(pos);
                        frToff.push_back(off + t + 1);
                        frTlen.push_back(len - t - 1);
                        frSub.push_back(subH);
                    }
                    it = bag;
                    descended = true;
                } else {  // nothing left to dive into
                    if (t + 1 < len) out[pos] = meld(out[pos], newItem(subH, 1, off + t + 1, len - t - 1));
                }
                break;
            }
        }
        if (descended) continue;
        // Every pending dive is finished now; attach the tails bottom-up.
        while (!frPos.empty()) {
            int pos = frPos.back(), toff = frToff.back(), tlen = frTlen.back(), sub = frSub.back();
            frPos.pop_back(); frToff.pop_back(); frTlen.pop_back(); frSub.pop_back();
            out[pos] = meld(out[pos], newItem(sub, 1, toff, tlen));
        }
        return;
    }
}

static int solveCase(int nn) {
    n = nn;
    if (n == 1) return 0;

    // ---- adjacency in CSR form ----
    adjStart.assign(n + 1, 0);
    vector<int> eu(n - 1), ev(n - 1);
    for (int i = 0; i < n - 1; i++) {
        int a = readInt() - 1, b = readInt() - 1;
        eu[i] = a; ev[i] = b;
        adjStart[a + 1]++;
        adjStart[b + 1]++;
    }
    for (int i = 0; i < n; i++) adjStart[i + 1] += adjStart[i];
    adjList.assign(2 * (n - 1), 0);
    {
        vector<int> cur(adjStart.begin(), adjStart.end() - 1);
        for (int i = 0; i < n - 1; i++) {
            adjList[cur[eu[i]]++] = ev[i];
            adjList[cur[ev[i]]++] = eu[i];
        }
    }
    vector<int>().swap(eu);
    vector<int>().swap(ev);

    // ---- root the tree at 0, BFS order, down/up heights ----
    par_.assign(n, -1);
    order_.assign(n, 0);
    bdist.assign(n, -1);
    {
        int qh = 0, qt = 0;
        order_[qt++] = 0; par_[0] = -1; bdist[0] = 0;
        while (qh < qt) {
            int x = order_[qh++];
            for (int p = adjStart[x]; p < adjStart[x + 1]; p++) {
                int y = adjList[p];
                if (y == par_[x]) continue;
                par_[y] = x; bdist[y] = bdist[x] + 1; order_[qt++] = y;
            }
        }
    }
    downv.assign(n, 0);
    for (int i = n - 1; i >= 0; i--) {
        int x = order_[i];
        for (int p = adjStart[x]; p < adjStart[x + 1]; p++) {
            int y = adjList[p];
            if (par_[y] == x && downv[y] + 1 > downv[x]) downv[x] = downv[y] + 1;
        }
    }
    upv.assign(n, 0);
    best1.assign(n, 0); best2.assign(n, 0); arg1.assign(n, -1);
    for (int i = 1; i < n; i++) {
        int x = order_[i], p = par_[x], v = downv[x] + 1;
        if (v > best1[p]) { best2[p] = best1[p]; best1[p] = v; arg1[p] = x; }
        else if (v > best2[p]) best2[p] = v;
    }
    for (int i = 1; i < n; i++) {
        int x = order_[i], p = par_[x];
        int best = (arg1[p] != x) ? best1[p] : best2[p];
        int cand = (p == 0) ? 0 : 1 + upv[p];
        upv[x] = best > cand ? best : cand;
    }

    // ---- pick a diameter ----
    int u = 0;
    for (int i = 1; i < n; i++) if (bdist[i] > bdist[u]) u = i;
    bpar.assign(n, -1);
    {
        int qh = 0, qt = 0;
        bq.swap(order_);
        bq[qt++] = u; bdist[u] = 0; bpar[u] = -1;
        while (qh < qt) {
            int x = bq[qh++];
            for (int p = adjStart[x]; p < adjStart[x + 1]; p++) {
                int y = adjList[p];
                if (y == bpar[x]) continue;
                bpar[y] = x; bdist[y] = bdist[x] + 1; bq[qt++] = y;
            }
        }
    }
    int v = 0;
    for (int i = 1; i < n; i++) if (bdist[i] > bdist[v]) v = i;

    // ---- item pool reset ----
    ikey.clear(); ihl.clear(); ihr.clear(); idist.clear();
    ityp.clear(); ip1.clear(); ip2.clear(); pathEntries.clear();
    // null node
    ikey.push_back(0); ihl.push_back(0); ihr.push_back(0);
    idist.push_back(0); ityp.push_back(0); ip1.push_back(0); ip2.push_back(0);

    // ---- initial spine and bags ----
    onpath.assign(n, 0);
    spine_.clear();
    for (int x = v; x != -1; x = bpar[x]) { onpath[x] = 1; spine_.push_back(x); }
    for (int i = 0; i < (int)spine_.size(); i++) {
        int x = spine_[i];
        int bag = 0;
        for (int p = adjStart[x]; p < adjStart[x + 1]; p++) {
            int y = adjList[p];
            if (onpath[y]) continue;
            bag = meld(bag, newItem(1 + Hfun(x, y), 0, x, y));
        }
        spine_[i] = bag;
    }

    // ---- fold until a single vertex remains ----
    int folds = 0;
    while ((int)spine_.size() > 1) {
        int K = (int)spine_.size();
        int m = (K + 1) >> 1;
        nbag_.resize(m);
        for (int i = 0; i < m; i++) {
            int j = K - 1 - i;
            nbag_[i] = (i == j) ? spine_[i] : meld(spine_[i], spine_[j]);
        }
        int best = -1, jstar = 0;
        for (int i = 0; i < m; i++) {
            int val = bagKey(nbag_[i]) + i;
            if (val > best) { best = val; jstar = i; }
        }
        nspine_.clear();
        for (int i = 0; i < jstar; i++) nspine_.push_back(nbag_[i]);
        int bag = nbag_[jstar];
        int pos = jstar;
        if (bag) {
            nspine_.push_back(meld(ihl[bag], ihr[bag]));
            promote(bag, nspine_);
        } else {
            nspine_.push_back(0);
        }
        if (jstar + 1 < m) {
            int off = (int)pathEntries.size();
            int len = m - 1 - jstar;
            int subH = 0;
            for (int t = jstar + 1; t < m; t++) {
                int h = bagKey(nbag_[t]);
                pathEntries.push_back(make_pair(nbag_[t], h));
                int val = h + (t - jstar);
                if (val > subH) subH = val;
            }
            nspine_[pos] = meld(nspine_[pos], newItem(subH, 1, off, len));
        }
        spine_.swap(nspine_);
        folds++;
    }
    return folds;
}

int main() {
    int T = readInt();
    if (T < 0) return 0;
    outbuf.reserve(1 << 22);
    for (int tc = 0; tc < T; tc++) {
        int nn = readInt();
        if (nn < 0) break;
        writeInt(solveCase(nn));
    }
    fwrite(outbuf.data(), 1, outbuf.size(), stdout);
    return 0;
}

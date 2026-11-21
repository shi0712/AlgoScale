// Grand Finale: Snakes
//
// Model
// -----
// The head of a snake follows a monotone (down/right) path h_1=(1,1), h_2=(2,1), ...,
// h_{2n-1}=(n,n).  At second T the snake of length l occupies h_{T-l+1},...,h_T, where
// for indices j <= 1 we put h_j = (1, 2-j) (the initial body sticking out to the right).
// Hence f(l,T) = maximum grid value over that window.
//
// Facts used
// ----------
// * f(l,1) = max G[1][1..l]                              (exactly known)
// * f(l,2) = max(G[2][1], max G[1][1..l-1])              (exactly known)
// * For T <= l the window is  {G[1][1..l-T+1]} U {h_2..h_T};  the second part is a
//   monotone segment of T-1 cells starting at (2,1).
// * For T = l+1 the window is a monotone segment of l cells starting at (2,1).
// * For T >= l+2 the window is an arbitrary monotone segment of l cells whose first
//   cell lies on anti-diagonal T-l (0-indexed) and which uses only cells with row>=2.
//
// Lower bounds
// ------------
// From the public grid alone we compute the *exact minimax*
//     staticLB(l,T) = min over all admissible windows of (max value inside window).
// Any real value satisfies f(l,T) >= staticLB(l,T).  This is computed with 2-D DPs.
//
// Additionally, the answer x of a query is the value of a unique grid cell, so that
// cell lies on the snake's path at a time determined by the cell's anti-diagonal:
//     query(l,T) = x  ==>  h_{t} = cell(x),  t = diag(cell(x)) + 1.
// Every window of the *same* snake that covers path time t has value >= x.  These
// "revealed" cells are kept in a per-snake range-maximum structure and raise the
// lower bound of other pairs, which is what makes the search cheap.
//
// Search
// ------
// Best first over pairs ordered by staticLB.  The current threshold W is the m-th
// smallest value already known: a pair whose lower bound is >= W can never change
// the multiset of the m smallest values, so search stops there.  The first m values
// (before W exists) are obtained with pipelined batches since no pruning is possible
// during that phase.
#include <bits/stdc++.h>
#include <unistd.h>
#include <time.h>
using namespace std;

static const int INF = 1000000000;

static int n, m;
static vector<int> G;   // n*n, 0-indexed

/* ------------------------------- fast I/O ------------------------------- */
static char ibuf[1 << 16];
static int ipos = 0, ilen = 0;
static inline int gc() {
    if (ipos >= ilen) {
        ssize_t r = read(0, ibuf, sizeof ibuf);
        if (r <= 0) exit(0);            // judge closed the stream
        ilen = (int)r; ipos = 0;
    }
    return (unsigned char)ibuf[ipos++];
}
static inline int readInt() {
    int c = gc();
    while (c < '0' || c > '9') c = gc();
    int x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return x;
}
static char obuf[1 << 16];
static int opos = 0;
static inline void flushOut() {
    int w = 0;
    while (w < opos) {
        ssize_t r = write(1, obuf + w, opos - w);
        if (r <= 0) exit(0);
        w += (int)r;
    }
    opos = 0;
}
static inline void putChar(char c) {
    if (opos >= (int)sizeof(obuf)) flushOut();
    obuf[opos++] = c;
}
static inline void putInt(int x) {
    if (x == 0) { putChar('0'); return; }
    char t[12]; int k = 0;
    while (x > 0) { t[k++] = (char)('0' + x % 10); x /= 10; }
    while (k > 0) putChar(t[--k]);
}
static inline void putQuery(int l, int T) {
    putChar('?'); putChar(' '); putInt(l); putChar(' '); putInt(T); putChar('\n');
}
static double nowSec() {
    struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main() {
    const double tStart = nowSec();

    n = readInt();
    m = readInt();
    G.resize((size_t)n * n);
    for (size_t i = 0; i < G.size(); i++) G[i] = readInt();

    const int NT = 2 * n - 1;              // time values 1..NT
    const int N = n * NT;                  // number of (l,T) pairs
    const int W = NT;

    /* value -> cell */
    vector<int> posv((size_t)n * n + 1);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) posv[G[i * n + j]] = i * n + j;

    /* R[k] = max of the first k cells of row 1 */
    vector<int> R(n + 1, 0);
    for (int k = 1; k <= n; k++) R[k] = max(R[k - 1], G[k - 1]);

    /* ---------------------- static (grid only) lower bounds ---------------------- */
    vector<int> LB(N, INF);
    vector<int> lb2(n + 1, INF);   // lb2[len]: segments of len cells starting at (2,1)
    lb2[0] = 0;

    {
        vector<int> cur(n, INF), nxt(n, INF);
        for (int g = 1; g <= 2 * n - 2; g++) {
            fill(cur.begin(), cur.end(), INF);
            int lo = max(0, g - (n - 1)), hi = min(n - 1, g);
            for (int i = lo; i <= hi; i++)
                if (i >= 1) cur[i] = G[i * n + (g - i)];

            int maxlen = min(n, 2 * n - 1 - g);
            int a = g;
            for (int len = 1; ; len++) {
                int best = INF;
                int lo2 = max(0, a - (n - 1)), hi2 = min(n - 1, a);
                for (int i = lo2; i <= hi2; i++)
                    if (cur[i] < best) best = cur[i];

                if (g == 1) lb2[len] = best;
                else {
                    int T = g + len;                       // T - l = g
                    LB[(size_t)(len - 1) * W + (T - 1)] = best;
                }
                if (len >= maxlen) break;

                int a2 = a + 1;
                int lo3 = max(0, a2 - (n - 1)), hi3 = min(n - 1, a2);
                for (int i = lo3; i <= hi3; i++) {
                    int v = G[i * n + (a2 - i)];
                    int up = (i - 1 >= 0) ? cur[i - 1] : INF;
                    int lf = cur[i];
                    int p = min(up, lf);
                    nxt[i] = (p >= INF) ? INF : max(v, p);
                }
                for (int i = lo2; i <= hi2; i++) cur[i] = INF;
                for (int i = lo3; i <= hi3; i++) { cur[i] = nxt[i]; nxt[i] = INF; }
                a = a2;
            }
        }
    }
    for (int l = 1; l <= n; l++)
        for (int T = 3; T <= NT; T++) {
            size_t idx = (size_t)(l - 1) * W + (T - 1);
            int lb;
            if (T <= l)          lb = max(R[l - T + 1], lb2[T - 1]);
            else if (T == l + 1) lb = lb2[l];
            else                 lb = LB[idx];
            LB[idx] = lb;
        }

    /* ------------------------------ answer heap ------------------------------ */
    priority_queue<int> hq;       // at most m smallest values seen so far
    for (int l = 1; l <= n; l++) {
        int v1 = R[l];
        int v2 = max(G[n], R[l - 1]);         // G[1*n+0] == cell (2,1)
        if ((int)hq.size() < m) hq.push(v1); else if (v1 < hq.top()) { hq.pop(); hq.push(v1); }
        if ((int)hq.size() < m) hq.push(v2); else if (v2 < hq.top()) { hq.pop(); hq.push(v2); }
    }

    /* ------------------- per snake structure of revealed cells ------------------- */
    int P = 1;
    while (P < NT) P <<= 1;
    vector<int> seg((size_t)n * 2 * P, 0);
    auto upd = [&](int l, int t, int val) {          // l: 1..n, t: 1..NT
        size_t base = (size_t)(l - 1) * 2 * P;
        int p = P + (t - 1);
        seg[base + p] = val;
        for (p >>= 1; p >= 1; p >>= 1)
            seg[base + p] = max(seg[base + 2 * p], seg[base + 2 * p + 1]);
    };
    auto qmax = [&](int l, int a, int b) -> int {    // inclusive 1-based [a,b]
        size_t base = (size_t)(l - 1) * 2 * P;
        int res = 0;
        int lo = P + (a - 1), hi = P + (b - 1) + 1;
        while (lo < hi) {
            if (lo & 1) res = max(res, seg[base + lo++]);
            if (hi & 1) res = max(res, seg[base + --hi]);
            lo >>= 1; hi >>= 1;
        }
        return res;
    };
    for (int l = 1; l <= n; l++) { upd(l, 1, G[0]); upd(l, 2, G[n]); }

    /* Record a revealed cell.  Only cells of the head path qualify: the answer is the
       maximum of the window, and the row-1 cells (except (1,1)) belong to the initial
       body, not to the path.  Their contribution is already covered by R[]. */
    auto reveal = [&](int l, int x) {
        if (x < 1 || x > n * n) return;
        int c = posv[x];
        int ci = c / n;
        if (ci < 1) return;                       // row 1 cell -> not a path cell
        int t = ci + (c % n) + 1;
        if (t >= 2 && t <= NT) upd(l, t, x);
    };
    /* dynamic lower bound of pair idx (0-based l,T decoded by caller) */
    auto dynLB = [&](int l, int T, int stat) -> int {
        int a = max(2, T - l + 1);
        return max(stat, qmax(l, a, T));
    };

    /* ------------------------- sort pairs by static bound ------------------------- */
    vector<int> cnt(n * n + 2, 0);
    for (int l = 1; l <= n; l++)
        for (int T = 3; T <= NT; T++)
            cnt[LB[(size_t)(l - 1) * W + (T - 1)]]++;
    vector<int> pos(n * n + 2, 0);
    int total = 0;
    for (int v = 1; v <= n * n + 1; v++) { pos[v] = total; total += cnt[v]; }
    vector<int> order(total);
    vector<int> sortedLB(total);
    {
        vector<int> p(pos);
        for (int l = 1; l <= n; l++)
            for (int T = 3; T <= NT; T++) {
                size_t idx = (size_t)(l - 1) * W + (T - 1);
                int v = LB[idx];
                order[p[v]] = (int)idx;
                p[v]++;
            }
    }
    for (int i = 0; i < total; i++) sortedLB[i] = LB[order[i]];

    vector<char> dead(N, 0);

    /* ------------------------------- phase 1 -------------------------------- */
    int oi = 0;
    double t1start = nowSec();
    long long q1 = 0;
    {
        const int BATCH1 = 256;   // 256 lines ~= 2.5 KB, safe for any pipe size
        while ((int)hq.size() < m && oi < total) {
            int need = m - (int)hq.size();
            int b = min({BATCH1, need, total - oi});
            for (int k = 0; k < b; k++) {
                int idx = order[oi + k];
                putQuery(idx / W + 1, idx % W + 1);
            }
            flushOut();
            for (int k = 0; k < b; k++) {
                int v = readInt();
                int idx = order[oi + k];
                dead[idx] = 1;
                if ((int)hq.size() < m) hq.push(v);
                else if (v < hq.top()) { hq.pop(); hq.push(v); }
                reveal(idx / W + 1, v);
            }
            oi += b; q1 += b;
        }
    }
    // per-query cost that does not depend on pipelining (judge processing + our work)
    double alpha = (q1 > 0) ? (nowSec() - t1start) / (double)q1 : 0.0;

    /* ------------------------------- phase 2 -------------------------------- */
    long long q2 = 0;
    double t2start = nowSec();
    int batchSize = 1;
    const double TIME_TARGET = 7.0;    // keep total interaction time below this
    long long nextAdjust = 256;

    vector<int> chosen;
    while (oi < total) {
        int Wc = ((int)hq.size() >= m) ? hq.top() : INF;
        if (sortedLB[oi] >= Wc) break;

        // build a batch of pairs to query
        chosen.clear();
        int lim = min(oi + batchSize, total);
        bool blocked = false;
        for (; oi < lim; oi++) {
            int idx = order[oi];
            if (sortedLB[oi] >= Wc) { blocked = true; break; }
            int l = idx / W + 1, T = idx % W + 1;
            if (dynLB(l, T, sortedLB[oi]) >= Wc) { dead[idx] = 1; continue; }
            chosen.push_back(idx);
        }
        if (chosen.empty()) {
            if (blocked) break;
            continue;
        }

        for (int idx : chosen) putQuery(idx / W + 1, idx % W + 1);
        flushOut();

        for (size_t k = 0; k < chosen.size(); k++) {
            int v = readInt();
            int idx = chosen[k];
            dead[idx] = 1;
            if ((int)hq.size() < m) hq.push(v);
            else if (v < hq.top()) { hq.pop(); hq.push(v); }
            reveal(idx / W + 1, v);
        }
        q2 += (long long)chosen.size();

        // Adaptive pipelining: grow the batch only when the round-trip latency (not the
        // judge's per-query work) dominates and we would otherwise run out of time.
        if (q2 >= nextAdjust) {
            double el = nowSec() - tStart;
            double perQuery1 = (nowSec() - t2start) / (double)max(1LL, q2);
            double lat = perQuery1 - alpha;                 // pure round-trip part
            int Wnow = ((int)hq.size() >= m) ? hq.top() : INF;
            long long remain = (long long)(lower_bound(sortedLB.begin(), sortedLB.end(), Wnow) - sortedLB.begin()) - oi;
            if (remain < 0) remain = 0;
            double budget = TIME_TARGET - el;
            if (budget < 0.5) budget = 0.5;
            double allowed = (remain > 0) ? budget / (double)remain : budget;
            if (allowed < perQuery1 && lat > 0.25 * perQuery1 && lat > 1e-6) {
                double denom = allowed - alpha;
                int want;
                if (denom <= 1e-9) want = 256;
                else want = (int)(lat / denom) + 1;
                want = max(want, batchSize + 1);
                batchSize = min(want, 256);
            }
            nextAdjust = q2 + max(1024LL, (long long)q2 / 4);
        }
    }

    /* ------------------------------- answer --------------------------------- */
    vector<int> ans;
    ans.reserve(hq.size());
    while (!hq.empty()) { ans.push_back(hq.top()); hq.pop(); }
    sort(ans.begin(), ans.end());
    putChar('!');
    for (size_t i = 0; i < ans.size(); i++) { putChar(' '); putInt(ans[i]); }
    putChar('\n');
    flushOut();
    return 0;
}

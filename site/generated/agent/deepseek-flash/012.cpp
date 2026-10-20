#include <bits/stdc++.h>
using namespace std;
typedef long long ll;

static const ll BIGW = 1000000000LL;   // maximum weight / value
static const ll BIGV = 1000000000LL;
static const int  INFB = 1 << 28;      // "A not equal to S"

static int n, nK;
static ll W;
static vector<ll>  w0, r0;             // input (0 = unknown)
static vector<ll>  wa, ra;             // filled answer
static vector<char> isVU;              // value unknown
static vector<int> K;                  // weight-known items, sorted by (weight asc, index asc)
static vector<int> U;                  // weight-unknown items, sorted by (value desc, index asc) == B order restricted to U
static vector<int> posK;               // position in K (or -1)
static vector<ll>  pre;                // prefix weight sums over K
static vector<int> topL, lowL, vuL, midL;
static vector<int> mergedT, mergedL;   // index-merged group lists (value 1e9 group / value 1 group)

static long long gwork = 0;            // item visits spent so far
static long long GLIMIT = 52000000;    // global work budget

static inline void updMax(ll v, int i, ll &mw, int &mi) {
    if (v > mw || (v == mw && i > mi)) { mw = v; mi = i; }
}

// Run loader B's order for parameter m, building S = K[0..m-1] plus a greedily chosen
// prefix of U.  Returns the number of out-of-S items that B would take (capped at
// cap+1), or INFB when loader A does not select exactly S.
static int evaluate(int m, bool needB, int cap) {
    const bool hasOK = (m < nK);
    const ll   theta = hasOK ? w0[K[m]] : BIGW;
    const int  idxK  = hasOK ? K[m] : -1;
    ll budget = W - pre[m];
    ll f = 0;                       // in-S weight seen so far (B order)
    ll maxWIn = -1; int maxIIn = -1; // largest A-key inside S
    if (m > 0) { maxWIn = w0[K[m - 1]]; maxIIn = K[m - 1]; }
    int minParked = INT_MAX;
    int nviol = 0;

    auto proc = [&](int i) {
        if (w0[i] == 0) {                       // weight unknown
            ll maxw = hasOK ? min(theta - ((i < idxK) ? 0 : 1), BIGW - 1) : BIGW - 1;
            if (maxw >= 1 && budget >= 1) {
                ll v = min(maxw, budget);
                wa[i] = v; budget -= v; f += v;
                updMax(v, i, maxWIn, maxIIn);
            } else {
                wa[i] = BIGW;
                if (i < minParked) minParked = i;
                if (needB && f + BIGW <= W) nviol++;
            }
        } else {                                // weight known
            if (posK[i] < m) { ll v = w0[i]; f += v; updMax(v, i, maxWIn, maxIIn); }
            else if (needB && f + w0[i] <= W) nviol++;
        }
    };

    for (int i : mergedT) {
        if (isVU[i] && posK[i] >= m) continue;
        gwork++;
        proc(i);
        if (needB && nviol > cap) return nviol;
    }
    for (int i : midL) {
        gwork++;
        proc(i);
        if (needB && nviol > cap) return nviol;
    }
    for (int i : mergedL) {
        if (isVU[i] && posK[i] < m) continue;
        gwork++;
        proc(i);
        if (needB && nviol > cap) return nviol;
    }

    // ---- loader A check ----
    ll ow = -1; int oi = -1;                 // smallest A-key outside S
    if (hasOK) { ow = theta; oi = idxK; }
    if (minParked != INT_MAX) {
        if (ow < 0 || BIGW < ow || (BIGW == ow && minParked < oi)) { ow = BIGW; oi = minParked; }
    }
    if (ow >= 0) {
        if (!(maxWIn < ow || (maxWIn == ow && maxIIn < oi))) return INFB;
        if (!(f + ow > W)) return INFB;
    }
    return nviol;
}

// ------------------------------------------------------------------ I/O
static char *ibuf; static size_t ipos, ilen;
static inline int gc() { return ipos < ilen ? ibuf[ipos++] : (int)(ilen = fread(ibuf, 1, 1 << 16, stdin), ipos = 0, ilen ? ibuf[ipos++] : EOF); }
static inline ll readInt() {
    int c = gc();
    while (c != '-' && (c < '0' || c > '9')) c = gc();
    bool neg = false; if (c == '-') { neg = true; c = gc(); }
    ll x = 0;
    while (c >= '0' && c <= '9') { x = x * 10 + (c - '0'); c = gc(); }
    return neg ? -x : x;
}
static char obuf[1 << 16]; static size_t opos = 0;
static inline void flushOut() { fwrite(obuf, 1, opos, stdout); opos = 0; }
static inline void putChar(char c) { if (opos == sizeof(obuf)) flushOut(); obuf[opos++] = c; }
static inline void putInt(ll x) {
    if (x < 0) { putChar('-'); x = -x; }
    char t[24]; int k = 0;
    do { t[k++] = char('0' + x % 10); x /= 10; } while (x);
    while (k) putChar(t[--k]);
}

int main() {
    ibuf = (char*)malloc(1 << 16);
    int T = (int)readInt();
    while (T--) {
        n = (int)readInt(); W = readInt();
        w0.assign(n, 0); r0.assign(n, 0);
        for (int i = 0; i < n; i++) w0[i] = readInt();
        for (int i = 0; i < n; i++) r0[i] = readInt();

        wa = w0; ra = r0;
        isVU.assign(n, 0); posK.assign(n, -1);
        K.clear(); U.clear();
        for (int i = 0; i < n; i++) {
            if (r0[i] == 0) isVU[i] = 1;
            if (w0[i] > 0) K.push_back(i); else U.push_back(i);
        }
        sort(K.begin(), K.end(), [](int a, int b) { return w0[a] != w0[b] ? w0[a] < w0[b] : a < b; });
        sort(U.begin(), U.end(), [](int a, int b) { return r0[a] != r0[b] ? r0[a] > r0[b] : a < b; });
        nK = (int)K.size();
        for (int i = 0; i < nK; i++) posK[K[i]] = i;
        pre.assign(nK + 1, 0);
        for (int i = 0; i < nK; i++) pre[i + 1] = pre[i] + w0[K[i]];

        topL.clear(); lowL.clear(); vuL.clear(); midL.clear();
        for (int i = 0; i < n; i++) {
            if (r0[i] == 0) vuL.push_back(i);
            else if (r0[i] == BIGV) topL.push_back(i);
            else if (r0[i] == 1) lowL.push_back(i);
            else midL.push_back(i);
        }
        sort(topL.begin(), topL.end());
        sort(lowL.begin(), lowL.end());
        sort(vuL.begin(), vuL.end());
        sort(midL.begin(), midL.end(), [](int a, int b) { return r0[a] != r0[b] ? r0[a] > r0[b] : a < b; });
        // index-merged groups
        mergedT.clear(); mergedL.clear();
        {
            int i = 0, j = 0, ts = (int)topL.size(), vs = (int)vuL.size();
            while (i < ts || j < vs) {
                if (j >= vs || (i < ts && topL[i] < vuL[j])) mergedT.push_back(topL[i++]);
                else mergedT.push_back(vuL[j++]);
            }
            i = 0; j = 0;
            int ls = (int)lowL.size();
            while (i < ls || j < vs) {
                if (j >= vs || (i < ls && lowL[i] < vuL[j])) mergedL.push_back(lowL[i++]);
                else mergedL.push_back(vuL[j++]);
            }
        }

        int mmax = 0;
        while (mmax + 1 <= nK && pre[mmax + 1] <= W) mmax++;

        // A-feasibility is a suffix of [0, mmax]
        int alo = -1;
        {
            int lo = 0, hi = mmax;
            while (lo <= hi) {
                int mid = lo + (hi - lo) / 2;
                if (evaluate(mid, false, 0) != INFB) { alo = mid; hi = mid - 1; }
                else lo = mid + 1;
            }
        }

        int best = -1;
        if (alo >= 0) {
            int bestCnt = INFB, bestGuess = -1;
            auto attempt = [&](int m) -> bool {
                int v = evaluate(m, true, 2);
                if (v == 0) { best = m; return true; }
                if (v < bestCnt) { bestCnt = v; bestGuess = m; }
                return false;
            };
            bool done = false;
            for (int m = mmax; m >= alo && !done; m--) {
                if (gwork > GLIMIT) break;
                if (attempt(m)) done = true;
            }
            if (!done) {                       // remaining budget: borders, valley and samples
                if (bestGuess >= 0) {
                    int lo2 = max(alo, bestGuess - 300), hi2 = min(mmax, bestGuess + 300);
                    for (int m = lo2; m <= hi2 && !done; m++) {
                        if (gwork > GLIMIT) break;
                        if (attempt(m)) done = true;
                    }
                }
                if (!done) {
                    long long remaining = GLIMIT - gwork;
                    int range = mmax - alo + 1;
                    long long afford = remaining / (2LL * n + 4);
                    if (afford > 0) {
                        long long step = max(1LL, (long long)range / afford);
                        for (int m = alo; m <= mmax && !done; m += (int)step) {
                            if (gwork > GLIMIT) break;
                            if (attempt(m)) done = true;
                        }
                    }
                }
            }
        }

        if (best < 0) { putChar('N'); putChar('o'); putChar('\n'); continue; }
        evaluate(best, true, INT_MAX);                      // rebuild wa[] for `best`
        for (int i = 0; i < n; i++) if (r0[i] == 0) ra[i] = (posK[i] < best) ? BIGV : 1;
        putChar('Y'); putChar('e'); putChar('s'); putChar('\n');
        for (int i = 0; i < n; i++) { putInt(wa[i]); putChar(i + 1 == n ? '\n' : ' '); }
        for (int i = 0; i < n; i++) { putInt(ra[i]); putChar(i + 1 == n ? '\n' : ' '); }
    }
    flushOut();
    return 0;
}

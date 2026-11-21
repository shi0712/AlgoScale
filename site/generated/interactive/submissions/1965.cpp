#include <bits/stdc++.h>
using namespace std;

static int n;
static long long qcount = 0;

// ---------------- state ----------------
static vector<int> disp;      // total moves applied to each dial
static vector<int> orig;      // known origin (0..n-1) or -1
static vector<int> lo, hi;    // bounds on hold[u] = #unidentified dials with origin u
static vector<int> knownAt;   // count of identified dials currently at value u
static vector<char> OcurArr;  // exact occupied set at last refresh (dial0 frame)
static int Rcnt;              // number of unidentified dials
static vector<int> parked;
static int T;

static int ask(int p) {       // p is 0-indexed
    cout << "? " << (p + 1) << "\n" << flush;
    int r;
    if (!(cin >> r)) exit(0);
    if (r < 0) exit(0);
    qcount++;
    return r;
}

static inline int pmod(int x) { int r = x % n; if (r < 0) r += n; return r; }
static int a1val() { return pmod(orig[0] + disp[0]); }

static void propagate() {
    int sm = 0;
    for (int u = 0; u < n; u++) sm += lo[u];
    for (int u = 0; u < n; u++) {
        int cap = Rcnt - (sm - lo[u]);
        if (hi[u] > cap) hi[u] = cap;
    }
}

static void rebuild() {
    fill(knownAt.begin(), knownAt.end(), 0);
    Rcnt = 0;
    for (int i = 0; i < n; i++) {
        if (orig[i] >= 0) {
            knownAt[(orig[i] + disp[i]) % n]++;
        } else Rcnt++;
    }
    propagate();
}

static void doMove(int p, int k) {
    for (int t = 0; t < k; t++) ask(p);
    disp[p] += k;
}

static vector<int> fullScan(int p) {
    vector<int> pat(n);
    for (int t = 0; t < n; t++) pat[t] = ask(p);
    disp[p] += n;
    return pat;
}

static void refresh(int scanned, const vector<char>& cand) {
    vector<int> pat = fullScan(0);
    int a1 = a1val();
    fill(OcurArr.begin(), OcurArr.end(), 0);
    OcurArr[a1] = 1;
    for (int m = 1; m < n; m++)
        if (pat[m - 1] == 1) OcurArr[(a1 + m) % n] = 1;
    rebuild();
    for (int u = 0; u < n; u++)
        if (!OcurArr[u]) hi[u] = 0;
    vector<char> skip(n, 0);
    if (scanned >= 0) {
        int dd = disp[scanned];
        for (int c = 0; c < n; c++)
            if (cand[c]) skip[(c + dd) % n] = 1;
    }
    for (int u = 0; u < n; u++) {
        if (!OcurArr[u]) continue;
        if (knownAt[u] > 0) continue;
        if (skip[u]) continue;
        if (lo[u] < 1) lo[u] = 1;
    }
    propagate();
}

static void move0ToGap() {
    int cur = a1val();
    int bestK = -1;
    // prefer a gap in the exact occupied set
    for (int u = 0; u < n; u++) {
        if (OcurArr[u]) continue;
        int k = (u - cur) % n;
        if (k == 0) k = n;
        if (bestK < 0 || k < bestK) bestK = k;
    }
    if (bestK < 0) {
        for (int u = 0; u < n; u++) {
            if (hi[u] != 0) continue;
            int k = (u - cur) % n;
            if (k == 0) k = n;
            if (bestK < 0 || k < bestK) bestK = k;
        }
    }
    if (bestK < 0) bestK = 1;
    doMove(0, bestK);
    rebuild();
}

static int identify(int i) {
    vector<char> cand(n, 0);
    int ncand = 0;
    for (int u = 0; u < n; u++)
        if (hi[u] > 0) { cand[u] = 1; ncand++; }
    // observations for retroactive bound updates
    vector<pair<int, int>> obsD;   // displacement, reply (-1 = uninformative)
    vector<vector<char>> obsIP;    // knownAt positions at observation time (only if reply==1)
    vector<char> haveIP;
    int since = -1;
    int cnt = 0, stuck = 0;
    while (ncand > 1) {
        if (cnt >= n) {
            if (stuck == 0) {
                refresh(i, cand);
            } else {
                move0ToGap();
                refresh(i, cand);
            }
            since = disp[i];
            cnt = 0;
            stuck++;
            continue;
        }
        // record knownAt positions (only needed for reply==1)
        int r = ask(i);
        disp[i]++;
        cnt++;
        int dd = disp[i];
        if (dd % n == 0) {
            continue;   // uninformative wrap
        }
        // compute known-at positions now (only relevant for reply 1)
        vector<char> ip;
        if (r == 1) {
            ip.assign(n, 0);
            for (int u = 0; u < n; u++) if (knownAt[u] > 0) ip[u] = 1;
        }
        if (since >= 0 && dd - since >= 1 && dd - since < n) {
            // exact model wrt OcurArr
            for (int c = 0; c < n; c++) {
                if (!cand[c]) continue;
                int x = (c + dd) % n;
                bool pred = OcurArr[x] != 0;
                if (pred != (r == 1)) { cand[c] = 0; ncand--; }
            }
        } else {
            if (r == 1) {
                for (int c = 0; c < n; c++) {
                    if (!cand[c]) continue;
                    int x = (c + dd) % n;
                    if (!(ip[x] || hi[x] > 0)) { cand[c] = 0; ncand--; }
                }
            } else {
                for (int c = 0; c < n; c++) {
                    if (!cand[c]) continue;
                    int x = (c + dd) % n;
                    if (!(knownAt[x] == 0 && lo[x] == 0)) { cand[c] = 0; ncand--; }
                }
            }
        }
        obsD.push_back({dd, r});
        if (r == 1) { haveIP.push_back(1); obsIP.push_back(ip); }
        else haveIP.push_back(0);
        if (ncand == 0) exit(0);   // should never happen
    }
    int val = -1;
    for (int u = 0; u < n; u++) if (cand[u]) { val = u; break; }
    // retroactive updates
    int pi = 0;
    for (size_t k = 0; k < obsD.size(); k++) {
        int dd = obsD[k].first, r = obsD[k].second;
        int x = (val + dd) % n;
        if (r == 0) {
            hi[x] = 0;
        } else {
            if (!obsIP[pi][x] && lo[x] < 1) lo[x] = 1;
        }
        if (r == 1) pi++;
    }
    propagate();
    return val;
}

static int identifyLast(int i) {
    int k = 0;
    while (true) {
        int r = ask(i);
        disp[i]++;
        k++;
        if (r == 1) break;
    }
    int val = pmod(T - disp[i]);
    return val;
}

static void park(int i) {
    int cur = pmod(orig[i] + disp[i]);
    int k = pmod(T - cur);
    doMove(i, k);
    parked.push_back(i);
}

static int chooseT() {
    vector<int> hist(n, 0);
    int k = 0;
    for (int i = 0; i < n; i++)
        if (orig[i] >= 0) { hist[orig[i]]++; k++; }
    int R = n - k;
    int K = 10;
    __int128 bestv = (__int128)1 << 120;
    int bestT = 0;
    for (int Tp = 0; Tp < n; Tp++) {
        long long sd = 0;
        long long cl = 0;
        for (int u = 0; u < n; u++) {
            if (!hist[u]) continue;
            int distc = ((Tp - u) % n + n) % n;
            sd += (long long)hist[u] * distc;
            if (distc < K) cl += hist[u];
        }
        __int128 v = (__int128)sd + (__int128)((long long)R * (sd + (long long)n * cl)) / k;
        if (v < bestv) { bestv = v; bestT = Tp; }
    }
    return bestT;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n)) return 0;
    disp.assign(n, 0);
    orig.assign(n, -1);
    lo.assign(n, 0);
    hi.assign(n, 0);
    knownAt.assign(n, 0);
    OcurArr.assign(n, 0);

    // initial full scan of dial 0
    vector<int> pat = fullScan(0);
    vector<char> inOinit(n, 0);
    inOinit[0] = 1;
    for (int m = 1; m < n; m++)
        if (pat[m - 1] == 1) inOinit[m] = 1;
    int cntO = 0;
    for (int u = 0; u < n; u++) cntO += inOinit[u];
    orig[0] = 0;
    int cap = n - cntO;
    for (int u = 0; u < n; u++) {
        if (inOinit[u]) {
            lo[u] = (u == 0) ? 0 : 1;
            hi[u] = (u == 0) ? cap : cap + 1;
        } else { lo[u] = 0; hi[u] = 0; }
    }
    rebuild();
    OcurArr = inOinit;
    if (cntO == n) {
        doMove(0, 1);
        rebuild();
    }
    int ks = min(n - 1, 8);
    for (int i = 1; i <= ks; i++) {
        int val = identify(i);
        orig[i] = val;
        if (lo[val] > 0) lo[val]--;
        if (hi[val] > 0) hi[val]--;
        rebuild();
    }
    T = chooseT();
    for (int i = 1; i <= ks; i++) park(i);
    doMove(0, pmod(T - a1val()));
    rebuild();
    for (int i = ks + 1; i < n; i++) {
        int val;
        if (Rcnt == 1) {
            doMove(0, pmod(T - a1val()));
            rebuild();
            val = identifyLast(i);
        } else val = identify(i);
        orig[i] = val;
        if (lo[val] > 0) lo[val]--;
        if (hi[val] > 0) hi[val]--;
        rebuild();
        park(i);
        rebuild();
    }
    doMove(0, pmod(T - a1val()));
    cout << "!" << endl;
    return 0;
}

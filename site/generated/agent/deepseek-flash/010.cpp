// 双律归减
//
// Operation S: x -> floor(sqrt(x))   (at most P times)
// Operation H: x -> floor(x/2)       (at most Q times)
// Minimise the total of all containers.
//
// For a single container an optimal plan is: a square-roots followed by b halvings
// (S^c(H^d(x)) <= H^d(S^c(x))), and a <= 6 is enough for x <= 2e6.
//
// Lagrangian relaxation on the number of halvings.  For integer nu >= 0 define
//     phi_nu(u) = min_{b>=0} ( u >> b ) + nu * b
// (the optimum of "halve u b times, pay nu per halving").  Then the Lagrangian
// dual value of the whole instance is
//     D(nu) = sum_i phi_nu(h_i) - (sum of the P largest sqrt-step gains) - nu*Q,
//     gain(i,a) = phi_nu(c_{i,a-1}) - phi_nu(c_{i,a}),  c_{i,0}=h_i, c_{i,a}=isqrt(c_{i,a-1}).
// D is concave in nu and  max_{nu>=0} D(nu)  equals the exact optimum (the relaxation
// is tight because the residual problem is an integral polymatroid; the gains along a
// sqrt chain are non-increasing, so the top-P choice is automatically prefix closed).
//
// Gains only depend on the value, and containers sharing a value share their whole
// chain, so we aggregate by value: one histogram of gains per distinct value and
// per sqrt level.  Evaluating D(nu) costs O(#distinct values + #gain values).
#include <bits/stdc++.h>
using namespace std;

static char *ibuf;
static size_t ipos = 0, ilen = 0;

static inline int readInt() {
    while (ipos < ilen && (ibuf[ipos] < '0' || ibuf[ipos] > '9')) ipos++;
    if (ipos >= ilen) return -1;
    int x = 0;
    while (ipos < ilen && ibuf[ipos] >= '0' && ibuf[ipos] <= '9') x = x * 10 + (ibuf[ipos++] - '0');
    return x;
}

int n, P, Q, maxh;
vector<int> sqv;                  // sqv[u] = floor(sqrt(u))
vector<int> phiArr;               // phi values at the needed points
vector<int> hist;                 // histogram of sqrt gains
vector<long long> memo;
vector<int> allvals;              // values where phi is needed, ascending
vector<pair<int, int>> levlist[7];// per chain level: (value, weight)
vector<int> touched;

static long long evalD(int nu) {
    long long &m = memo[nu];
    if (m != LLONG_MIN) return m;
    if (nu == 0) return m = 0;
    int b = 0;
    for (int v : allvals) {
        while (((long long)nu << (b + 1)) < (long long)v) b++;
        phiArr[v] = (int)((v >> b) + (long long)nu * b);
    }
    long long sumPhi = 0;
    int maxg = 0;
    touched.clear();
    for (int a = 1; a <= 6; a++) {
        for (const auto &pr : levlist[a]) {
            int v = pr.first, w = pr.second;
            int g = phiArr[v] - phiArr[sqv[v]];
            if (a == 1) sumPhi += (long long)w * phiArr[v];
            if (g > 0) {
                if (hist[g] == 0) touched.push_back(g);
                hist[g] += w;
                if (g > maxg) maxg = g;
            }
        }
    }
    long long rem = P, gain = 0;
    for (int g = maxg; g >= 1 && rem > 0; g--) {
        int c = hist[g];
        if (!c) continue;
        long long take = rem < c ? rem : c;
        gain += take * g;
        rem -= take;
    }
    for (int g : touched) hist[g] = 0;
    return m = sumPhi - gain - (long long)nu * Q;
}

int main() {
    {
        size_t cap = 1 << 20;
        ibuf = (char *)malloc(cap);
        while (true) {
            if (ilen == cap) { cap <<= 1; ibuf = (char *)realloc(ibuf, cap); }
            size_t r = fread(ibuf + ilen, 1, cap - ilen, stdin);
            if (r == 0) break;
            ilen += r;
        }
    }
    n = readInt();
    if (n < 0) return 0;
    P = readInt(); Q = readInt();
    {
        vector<int> h(n);
        maxh = 0;
        for (int i = 0; i < n; i++) {
            h[i] = readInt();
            if (h[i] < 0) h[i] = 0;
            if (h[i] > maxh) maxh = h[i];
        }
        free(ibuf); ibuf = nullptr;
        if (maxh == 0) { printf("0\n"); return 0; }
        sqv.assign(maxh + 1, 0);
        for (int u = 1; u <= maxh; u++) {
            int r = (int)sqrt((double)u);
            while ((long long)(r + 1) * (r + 1) <= (long long)u) ++r;
            while ((long long)r * r > (long long)u) --r;
            sqv[u] = r;
        }
        vector<int> cnt(maxh + 1, 0);
        for (int u : h) cnt[u]++;
        vector<int>().swap(h);
        vector<int> vals;
        vals.reserve(min((size_t)maxh + 1, (size_t)n));
        for (int u = 0; u <= maxh; u++) if (cnt[u]) vals.push_back(u);
        levlist[1].reserve(vals.size());
        for (int u : vals) levlist[1].push_back({u, cnt[u]});
        vector<int>().swap(cnt);
        vector<int> agg;
        int lim = (int)(sqrt((double)maxh)) + 2;
        agg.assign(lim + 1, 0);
        for (int a = 2; a <= 6; a++) {
            int hi = 0;
            for (const auto &pr : levlist[a - 1]) hi = max(hi, sqv[pr.first]);
            if ((int)agg.size() <= hi) agg.resize(hi + 1, 0);
            vector<int> used;
            used.reserve(levlist[a - 1].size());
            for (const auto &pr : levlist[a - 1]) {
                int v = sqv[pr.first];
                if (agg[v] == 0) used.push_back(v);
                agg[v] += pr.second;
            }
            levlist[a].reserve(used.size());
            for (int v : used) { levlist[a].push_back({v, agg[v]}); agg[v] = 0; }
            if (levlist[a].empty()) break;
            bool fixed = true;
            for (const auto &pr : levlist[a]) if (sqv[pr.first] != pr.first) { fixed = false; break; }
            if (fixed) break; // all chains reached 0/1: deeper gains vanish
        }
        // collect values where phi must be evaluated
        {
            vector<int> extra;
            extra.reserve(2 * (size_t)levlist[1].size());
            for (int a = 1; a <= 6; a++)
                for (const auto &pr : levlist[a]) {
                    extra.push_back(pr.first);
                    extra.push_back(sqv[pr.first]);
                }
            sort(extra.begin(), extra.end());
            extra.erase(unique(extra.begin(), extra.end()), extra.end());
            allvals.reserve(extra.size() + vals.size());
            size_t i = 0, j = 0;
            while (i < vals.size() || j < extra.size()) {
                if (j == extra.size() || (i < vals.size() && vals[i] < extra[j])) allvals.push_back(vals[i++]);
                else if (i == vals.size() || extra[j] < vals[i]) allvals.push_back(extra[j++]);
                else { allvals.push_back(vals[i]); i++; j++; }
            }
            vector<int>().swap(extra);
            allvals.shrink_to_fit();
            vector<int>().swap(vals);
        }
    }
    phiArr.assign(maxh + 1, 0);
    hist.assign(maxh + 1, 0);
    memo.assign(maxh + 2, LLONG_MIN);
    int lo = 0, hi = maxh;
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (evalD(mid) < evalD(mid + 1)) lo = mid + 1; else hi = mid;
    }
    printf("%lld\n", evalD(lo));
    return 0;
}

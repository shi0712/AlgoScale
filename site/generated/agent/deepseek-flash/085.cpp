#include <bits/stdc++.h>
using namespace std;

// ======================= Parity Echo =======================
// Insert positions left to right.  Once positions 1..M are known (their
// relative order / ranks), the hidden p_{M+1} is described by its rank
//     t = #{ k in [1,M] : p_k < p_{M+1} }   in [0, M].
// A query [l, M+1] returns the parity of inversions of that interval:
//     S(l, M+1) = S(l, M) xor #{ k in [l,M] : p_k > p_{M+1} }   (mod 2)
// and  #{k in [l,M] : p_k > p_{M+1}} = (M-l+1) - #{k in [l,M] : r_k < t},
// where r_k is the rank of position k inside the known prefix.  S(l,M) is
// computable (the whole prefix pattern is known), so one query reveals
//     psi_l(t) = #{ k in [l,M] : r_k < t }   (mod 2),
// a 1-bit function of t.  Pick the l whose split of the current candidate set
// is most balanced.  For t1 < t2 the choice l = max{pos(q) : q in [t1,t2-1]}
// separates them, and every query at least halves the candidate set, so at
// most ceil(log2 M) queries are spent on element M+1 => at most 19953 queries
// for n = 2000 (entropy bound log2(n!) = 19053).
// ==========================================================

// Core solver.  `query(l, m)` must return the parity of the number of inversions
// of the interval [l, m] (1-based) of the hidden permutation.
// Returns ans with ans[k] = value at position k+1 (0-based array of 1..n).
template <class QueryFn>
vector<int> solvePerm(int n, QueryFn query, long long* qcount = nullptr, int* maxAux = nullptr) {
    vector<int> r;              // r[k] = rank (0-based) of position k+1 among prefix
    r.reserve(n + 1);
    vector<int> inv(n + 2, 0);  // inv[l] = parity of inversions of prefix positions l..M
    vector<int> cand, nxt, cntle, psiArr, cntAt;
    vector<uint64_t> w;
    long long q = 0;

    for (int m = 1; m <= n; m++) {
        int M = m - 1;
        int tstar = 0;
        if (M >= 1) {
            cand.resize(M + 1);
            iota(cand.begin(), cand.end(), 0);
            while ((int)cand.size() > 1) {
                int K = (int)cand.size();

                // cntle[q] = number of candidates <= q, q = 0..M-1
                cntle.assign(M, 0);
                {
                    int idx = 0;
                    for (int q2 = 0; q2 < M; q2++) {
                        while (idx < K && cand[idx] <= q2) idx++;
                        cntle[q2] = idx;
                    }
                }

                int W = (K + 63) >> 6;
                int lb = K & 63;
                w.assign(W, 0);
                int count = 0;
                int bestl = -1, bestbal = INT_MAX;
                for (int l = M; l >= 1; l--) {
                    int pp = cntle[r[l - 1]];
                    if (pp < K) {                       // otherwise nothing flips
                        int f = pp >> 6, rem = pp & 63;
                        int onesPrefix = 0;
                        for (int i = 0; i < f; i++) onesPrefix += __builtin_popcountll(w[i]);
                        if (rem) onesPrefix += __builtin_popcountll(w[f] & ((1ULL << rem) - 1));
                        int ones = count - onesPrefix;
                        w[f] ^= (~0ULL << rem);
                        for (int i = f + 1; i < W; i++) w[i] = ~w[i];
                        if (lb) w[W - 1] &= ((1ULL << lb) - 1);
                        count += (K - pp) - 2 * ones;
                    }
                    int mx = max(count, K - count);
                    if (mx < bestbal) { bestbal = mx; bestl = l; }
                }

                int bit = query(bestl, m);
                q++;

                int psi = (((M - bestl + 1) & 1) ^ inv[bestl] ^ bit);

                cntAt.assign(M, 0);
                for (int k = bestl - 1; k < M; k++) cntAt[r[k]] ^= 1;
                psiArr.assign(M + 1, 0);
                int cur = 0;
                for (int t = 0; t <= M; t++) {
                    psiArr[t] = cur;
                    if (t < M) cur ^= cntAt[t];
                }
                nxt.clear();
                for (int t : cand) if (psiArr[t] == psi) nxt.push_back(t);
                // Safety net: some l always separates two candidates, so the set
                // strictly shrinks; guard anyway so we can never spin forever.
                if (nxt.empty() || (int)nxt.size() == K) break;
                cand.swap(nxt);
            }
            tstar = cand[0];

            int c = 0;
            for (int l = M; l >= 1; l--) {
                if (r[l - 1] < tstar) c ^= 1;
                inv[l] ^= (((M - l + 1) & 1) ^ c);
            }
        }
        inv[m] = 0;
        for (int k = 0; k < M; k++) if (r[k] >= tstar) r[k]++;
        r.push_back(tstar);
        if (maxAux) { int a = (int)cand.size(); if (a > *maxAux) *maxAux = a; }
    }
    if (qcount) *qcount = q;
    vector<int> ans(n);
    for (int k = 0; k < n; k++) ans[k] = r[k] + 1;
    return ans;
}

int main() {
    int n;
    if (scanf("%d", &n) != 1) return 0;
    if (n == 1) {
        printf("! 1\n");
        fflush(stdout);
        return 0;
    }

    auto query = [](int l, int rr) -> int {
        printf("? %d %d\n", l, rr);
        fflush(stdout);
        int bit;
        if (scanf("%d", &bit) != 1) exit(0);
        if (bit < 0) exit(0);
        return bit;
    };

    vector<int> ans = solvePerm(n, query);

    static char buf[16 * 2005 + 16];
    int len = 0;
    buf[len++] = '!';
    for (int k = 0; k < n; k++) len += sprintf(buf + len, " %d", ans[k]);
    buf[len++] = '\n';
    fwrite(buf, 1, len, stdout);
    fflush(stdout);
    return 0;
}

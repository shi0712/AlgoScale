/*
 * GCD Queries -- find two indices, one of which holds the value 0.
 *
 * Idea for large n.  Suppose every answer gcd(p_c, x) revealed for a vertex c
 * is gathered.  Each of them divides p_c, so if their lcm exceeds n-1 then c
 * must be the zero; equally, two equal answers from c prove that c is not the
 * zero.  A pair whose gcd exceeds (n-1)/2 necessarily contains the zero.
 *
 * Candidates are partitioned into random triples and the three pairs of each
 * triple are queried: after that every member of the triple has two revealed
 * values, which almost always certifies the zero (the lcm of two random values
 * from 1..n-1 exceeds n-1 with probability ~99.8%).  Therefore the zero's own
 * triple, whose position in the random order is uniform, is found after about
 * n/2 queries in expectation -- matching the information-theoretic bound
 * (every vertex but the answer pair must be touched).  A triple also allows a
 * cheap exact test of each member, plus duplicate/lcm bookkeeping across
 * triples.  When only a few candidates remain, a guaranteed 2-queries-per-
 * candidate routine finishes.
 *
 * For n <= 9 the exact set of consistent permutations is small enough to be
 * tracked, and a greedy decision-tree choice (minimize the largest remaining
 * class) is played, which is close to optimal.
 */

#include <bits/stdc++.h>
using namespace std;

static int N;
static unordered_map<uint32_t, int> cache_;

static inline uint32_t pkey(int i, int j) {
    if (i > j) swap(i, j);
    return (uint32_t)i * (uint32_t)N + (uint32_t)j;
}

// Ask gcd(p_i, p_j) (0-based indices). Answers are cached, so a pair that was
// already queried is never asked twice.
static int ask(int i, int j) {
    uint32_t k = pkey(i, j);
    auto it = cache_.find(k);
    if (it != cache_.end()) return it->second;
    printf("? %d %d\n", i + 1, j + 1);
    fflush(stdout);
    int g;
    if (scanf("%d", &g) != 1) exit(0);
    if (g == -1) exit(0);
    cache_.emplace(k, g);
    return g;
}

static void answer(int x, int y) {
    printf("! %d %d\n", x + 1, y + 1);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    exit(0);
}

/* =====================================================================
 *  Small n: track the exact set of permutations consistent with all
 *  answers and always play the query that minimizes the largest child
 *  class (a very effective greedy decision-tree construction).
 * ===================================================================== */
static void solve_small() {
    int n = N;
    int Q = n * (n - 1) / 2;
    vector<pair<int, int>> qs;
    qs.reserve(Q);
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) qs.push_back({i, j});

    long long M = 1;
    for (int i = 2; i <= n; i++) M *= i;

    vector<vector<uint8_t>> ans(Q, vector<uint8_t>((size_t)M));
    vector<uint8_t> zpos((size_t)M);

    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    long long t = 0;
    do {
        for (int qi = 0; qi < Q; qi++) {
            int i = qs[qi].first, j = qs[qi].second;
            ans[qi][(size_t)t] = (uint8_t)std::gcd(p[i], p[j]);
        }
        for (int k = 0; k < n; k++)
            if (p[k] == 0) { zpos[(size_t)t] = (uint8_t)k; break; }
        t++;
    } while (next_permutation(p.begin(), p.end()));

    int W = (int)((M + 63) / 64);
    vector<uint64_t> state((size_t)W, 0);
    for (long long i = 0; i < M; i++) state[(size_t)(i >> 6)] |= 1ULL << (i & 63);

    vector<int> bits;
    bits.reserve((size_t)M);
    vector<int> cnt(n);

    while (true) {
        bits.clear();
        for (int w = 0; w < W; w++) {
            uint64_t x = state[(size_t)w];
            while (x) {
                int b = __builtin_ctzll(x);
                bits.push_back(w * 64 + b);
                x &= x - 1;
            }
        }
        bool seen[16] = {false};
        int nz = 0, zp[2] = {0, 0};
        for (int tt : bits) {
            int z = zpos[(size_t)tt];
            if (!seen[z]) {
                seen[z] = true;
                if (nz < 2) zp[nz] = z;
                nz++;
                if (nz > 2) break;
            }
        }
        if (nz <= 2) answer(zp[0], nz > 1 ? zp[1] : zp[0]);

        int bestqi = -1;
        long long bestmax = LLONG_MAX, bestss = LLONG_MAX;
        for (int qi = 0; qi < Q; qi++) {
            for (int v = 0; v < n; v++) cnt[v] = 0;
            const uint8_t *a = ans[qi].data();
            for (int tt : bits) cnt[a[tt]]++;
            long long mx = 0, ss = 0;
            for (int v = 0; v < n; v++) {
                if (cnt[v] > mx) mx = cnt[v];
                ss += (long long)cnt[v] * cnt[v];
            }
            if (mx < bestmax || (mx == bestmax && ss < bestss)) {
                bestmax = mx; bestss = ss; bestqi = qi;
            }
        }
        if (bestmax == (long long)bits.size()) {
            // Degenerate state (should never happen): guaranteed pair algorithm.
            int a = 0, b = 1;
            for (int i = 2; i < n; i++) {
                int A = ask(a, i);
                if (2 * A > N - 1) answer(a, i);
                int B = ask(b, i);
                if (2 * B > N - 1) answer(b, i);
                if (A > B) b = i;
                else if (B > A) a = i;
            }
            answer(a, b);
        }
        int g = ask(qs[bestqi].first, qs[bestqi].second);
        vector<uint64_t> ns((size_t)W, 0);
        const uint8_t *a = ans[bestqi].data();
        for (int tt : bits)
            if (a[tt] == g) ns[(size_t)(tt >> 6)] |= 1ULL << (tt & 63);
        state.swap(ns);
    }
}

/* =====================================================================
 *  Large n: random triangles.
 *
 *  For every vertex we keep the values revealed by gcd queries that
 *  involve it.  Two facts are used:
 *    - if the lcm of all values revealed for c exceeds n-1, then p_c = 0
 *      (all revealed values divide p_c);
 *    - if two revealed values coincide, then c is not the zero.
 *  A queried pair whose gcd exceeds (n-1)/2 contains the zero.
 *  A triangle also gives a cheap complete test of each of its vertices:
 *  if c were the zero then its two incident answers would be the two
 *  values of the other vertices, so they must differ and their gcd must
 *  equal the third answer.
 * ===================================================================== */
static vector<char> alive_;
static vector<vector<int>> rnb_, rval_;
static vector<long long> lcmv_;
static mt19937 rng_((unsigned)chrono::steady_clock::now().time_since_epoch().count() ^
                    (unsigned)random_device{}());

// returns true iff this proves that vertex c is the zero
static bool revealCand(int c, int nb, int g) {
    if (!alive_[c]) return false;
    auto &nbv = rnb_[c];
    for (int x : nbv)
        if (x == nb) return false;  // already knew this value
    auto &vl = rval_[c];
    for (int v : vl)
        if (v == g) { alive_[c] = 0; return false; }  // duplicate => not zero
    nbv.push_back(nb);
    vl.push_back(g);
    long long L = lcmv_[c];
    L = L / std::gcd(L, (long long)g) * (long long)g;
    if (L > N - 1) return true;
    lcmv_[c] = L;
    return false;
}

// Guaranteed finishing routine: at most 2*(|S|-2) new queries.
static void pair_finish(const vector<int> &S) {
    if (S.size() == 1) answer(S[0], S[0]);
    if (S.size() == 2) answer(S[0], S[1]);
    int a = S[0], b = S[1];
    for (size_t k = 2; k < S.size(); k++) {
        int i = S[k];
        int A = ask(a, i);
        if (2 * A > N - 1) answer(a, i);
        if (revealCand(a, i, A)) answer(a, i);
        int B = ask(b, i);
        if (2 * B > N - 1) answer(b, i);
        if (revealCand(b, i, B)) answer(b, i);
        if (A > B) b = i;
        else if (B > A) a = i;
    }
    answer(a, b);
}

static void solve_large() {
    alive_.assign(N, 1);
    rnb_.assign(N, {});
    rval_.assign(N, {});
    lcmv_.assign(N, 1);

    vector<int> cand((size_t)N);
    iota(cand.begin(), cand.end(), 0);

    while (cand.size() > 2) {
        if (cand.size() <= 4) pair_finish(cand);
        if ((long long)cache_.size() > 2LL * N + 100) pair_finish(cand);

        shuffle(cand.begin(), cand.end(), rng_);

        size_t before = cand.size();
        vector<int> nxt;
        nxt.reserve(before);
        size_t i = 0;
        for (; i + 2 < before; i += 3) {
            int a = cand[i], b = cand[i + 1], c = cand[i + 2];

            int gab = ask(a, b);
            if (2 * gab > N - 1) answer(a, b);
            if (revealCand(a, b, gab)) answer(a, b);
            if (revealCand(b, a, gab)) answer(b, a);

            int gac = ask(a, c);
            if (2 * gac > N - 1) answer(a, c);
            if (revealCand(a, c, gac)) answer(a, c);
            if (revealCand(c, a, gac)) answer(c, a);

            int gbc = ask(b, c);
            if (2 * gbc > N - 1) answer(b, c);
            if (revealCand(b, c, gbc)) answer(b, c);
            if (revealCand(c, b, gbc)) answer(c, b);

            if (alive_[a] && (gab == gac || std::gcd(gab, gac) != gbc)) alive_[a] = 0;
            if (alive_[b] && (gab == gbc || std::gcd(gab, gbc) != gac)) alive_[b] = 0;
            if (alive_[c] && (gac == gbc || std::gcd(gac, gbc) != gab)) alive_[c] = 0;

            if (alive_[a]) nxt.push_back(a);
            if (alive_[b]) nxt.push_back(b);
            if (alive_[c]) nxt.push_back(c);
        }
        for (; i < before; i++) nxt.push_back(cand[i]);

        if (nxt.size() == before) pair_finish(cand);
        if ((long long)nxt.size() * 100 > (long long)before * 88) pair_finish(nxt);
        cand.swap(nxt);
    }
    answer(cand[0], cand.size() == 1 ? cand[0] : cand[1]);
}

int main() {
    if (scanf("%d", &N) != 1) return 0;
    cache_.reserve(1 << 16);
    if (N <= 9) solve_small();
    else solve_large();
    return 0;
}

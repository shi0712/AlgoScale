// Appraiser: identify M counterfeit coins among N using "same / different" queries.
//
// Key idea (min-sum criterion).  After a set of queries the coins are split into
// parity components; component i has two classes of sizes (a_i, b_i) and its
// true number of counterfeit coins is either a_i or b_i.
//   sum_i min(a_i,b_i) <= M,
// and if equality holds and no component is balanced (a_i != b_i), then the
// counterfeit set is forced: it is exactly the union of the smaller classes.
// So we may stop as soon as that equality is reached; every coin that was never
// queried is then certainly genuine.
//
// Greedy plan:
//   Phase 1: query disjoint pairs (2i,2i+1) in random order.  Every "different"
//     pair holds exactly one counterfeit, so after M such pairs all M
//     counterfeits are found; join them into one component plus one extra coin
//     (M queries) and read off the class of size M.
//   Phase 2 (a pair contained two counterfeits, i.e. only d < M different pairs
//     were found): join the d pairs into a cluster, then compare same-pairs in
//     groups of two.  A group answering "different" contains exactly one
//     double-counterfeit pair; attaching it raises the cluster minimum by 2.
//     Once the cluster minimum is M the answer is forced.
#include <bits/stdc++.h>
using namespace std;

#ifdef LOCAL_SIM
extern int sim_ask(int, int);
extern vector<int> sim_answer;
#endif

static int ask(int x, int y) {
#ifdef LOCAL_SIM
    return sim_ask(x, y);
#else
    printf("? %d %d\n", x + 1, y + 1);
    fflush(stdout);
    int r;
    if (scanf("%d", &r) != 1) exit(0);
    if (r < 0) exit(0);
    return r;
#endif
}

struct DSU {
    vector<int> p, rk, par, c0, c1;
    void init(int n) {
        p.resize(n); rk.assign(n, 0); par.assign(n, 0);
        c0.assign(n, 1); c1.assign(n, 0);
        for (int i = 0; i < n; i++) p[i] = i;
    }
    int find(int x, int &pr) {              // returns root, pr = parity to root
        if (p[x] == x) { pr = 0; return x; }
        int q;
        int r = find(p[x], q);
        pr = q ^ par[x];
        par[x] = pr; p[x] = r;
        return r;
    }
    void unite(int x, int y, int rel) {     // parity(x) ^ parity(y) = rel
        int px, py;
        int rx = find(x, px), ry = find(y, py);
        if (rx == ry) return;
        int t = px ^ py ^ rel;              // parity of ry relative to rx
        if (rk[rx] < rk[ry]) swap(rx, ry);
        p[ry] = rx; par[ry] = t;
        if (t == 0) { c0[rx] += c0[ry]; c1[rx] += c1[ry]; }
        else        { c0[rx] += c1[ry]; c1[rx] += c0[ry]; }
        if (rk[rx] == rk[ry]) rk[rx]++;
    }
    int root(int x) { int pr; return find(x, pr); }
} dsu;

static int N_, M_;

// The smaller class of every component is the counterfeit set.
static void finish() {
    // Safety net: if the current components do not yet force a unique answer
    // (or some component is balanced), unify everything into one component so
    // that the colouring is read off directly.  Every query merges two distinct
    // components, so the total stays below N.
    {
        long long sumMin = 0;
        bool balanced = false;
        for (int c = 0; c < N_; c++) if (dsu.p[c] == c) {
            sumMin += min(dsu.c0[c], dsu.c1[c]);
            if (dsu.c0[c] == dsu.c1[c]) balanced = true;
        }
        if (sumMin != M_ || balanced) {
            int rep = 0;
            for (int c = 1; c < N_; c++)
                if (dsu.root(c) != dsu.root(rep))
                    dsu.unite(rep, c, ask(rep, c));
        }
    }
    vector<int> rootOf(N_), parOf(N_);
    for (int c = 0; c < N_; c++) { int pr; rootOf[c] = dsu.find(c, pr); parOf[c] = pr; }
    vector<int> small(N_, -1);
    for (int c = 0; c < N_; c++)
        if (dsu.p[c] == c) small[c] = (dsu.c0[c] <= dsu.c1[c]) ? 0 : 1;
    vector<int> ans;
    for (int c = 0; c < N_; c++)
        if (parOf[c] == small[rootOf[c]]) ans.push_back(c + 1);

#ifdef LOCAL_SIM
    sim_answer = ans;
#else
    printf("!");
    for (int v : ans) printf(" %d", v);
    printf("\n");
    fflush(stdout);
#endif
}

static void solve(int N, int M) {
    N_ = N; M_ = M;
    dsu.init(N);
    if (M == 0) { finish(); return; }

    int half = N / 2;
    vector<int> ord(half);
    iota(ord.begin(), ord.end(), 0);
    {
        mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());
        shuffle(ord.begin(), ord.end(), rng);
    }

    vector<int> diffPairs, samePairs;
    int d = 0;
    for (int ii = 0; ii < half; ii++) {
        int i = ord[ii];
        int a = 2 * i, b = 2 * i + 1;
        int r = ask(a, b);
        dsu.unite(a, b, r);
        if (r == 1) {
            diffPairs.push_back(i);
            if (++d == M) break;            // all counterfeit coins found
        } else {
            samePairs.push_back(i);
        }
    }

    if (d == M) {
        // Join the M mixed pairs and one further (certified genuine) coin.
        int base = 2 * diffPairs[0];
        for (int j = 1; j < M; j++) {
            int c = 2 * diffPairs[j];
            dsu.unite(base, c, ask(base, c));
        }
        vector<char> inDiff(N, 0);
        for (int idx : diffPairs) { inDiff[2 * idx] = 1; inDiff[2 * idx + 1] = 1; }
        int extra = -1;
        for (int c = 0; c < N; c++) if (!inDiff[c]) { extra = c; break; }
        if (extra >= 0) dsu.unite(base, extra, ask(base, extra));
        finish();
        return;
    }

    // ---------- Phase 2: k = (M-d)/2 >= 1 same-pairs are double-counterfeit ----------
    int cluster = -1;
    if (d >= 1) {
        cluster = 2 * diffPairs[0];
        for (int j = 1; j < d; j++) {
            int c = 2 * diffPairs[j];
            dsu.unite(cluster, c, ask(cluster, c));
        }
    }

    int s = (int)samePairs.size();
    bool done = false;
    for (int i = 0; i + 1 < s; i += 2) {
        int A = samePairs[i], B = samePairs[i + 1];
        int r = ask(2 * A, 2 * B);
        dsu.unite(2 * A, 2 * B, r);
        if (r == 1) {                       // exactly one double-counterfeit pair
            if (cluster == -1) cluster = 2 * A;
            else dsu.unite(cluster, 2 * A, ask(cluster, 2 * A));
        }
        if (cluster != -1) {
            int pr; int rt = dsu.find(cluster, pr);
            int lo = min(dsu.c0[rt], dsu.c1[rt]);
            if (lo == M) {
                int hi = max(dsu.c0[rt], dsu.c1[rt]);
                if (lo == hi) {             // break the tie with one outside coin
                    int out = -1;
                    for (int c = 0; c < N; c++) if (dsu.root(c) != rt) { out = c; break; }
                    if (out >= 0) dsu.unite(cluster, out, ask(cluster, out));
                }
                done = true;
                break;
            }
        }
    }

    if (!done) {
        // Fallback: pull everything into one component spanning all coins.
        if (cluster == -1) cluster = 2 * samePairs[0];
        int rt = dsu.root(cluster);
        for (int c = 0; c < N; c++) {
            if (dsu.root(c) != rt) {
                dsu.unite(cluster, c, ask(cluster, c));
                rt = dsu.root(cluster);
            }
        }
    }
    finish();
}

#ifndef LOCAL_SIM
int main() {
    int N, M;
    if (scanf("%d %d", &N, &M) != 2) return 0;
    solve(N, M);
    return 0;
}
#endif

#include <bits/stdc++.h>
using namespace std;

static int n, k;

static inline bool ask(int c) {
    printf("? %d\n", c);
    fflush(stdout);
    char buf[16];
    if (scanf("%15s", buf) != 1) exit(0);
    if (buf[0] == 'Y') return true;
    if (buf[0] == 'N') return false;
    exit(0);  // e.g. 'E' : terminate immediately
}

static inline void reset_mem() {
    printf("R\n");
    fflush(stdout);
}

struct Node {
    int pos;  // 1-based position of the element inside its block
    int idx;  // 0-based cafe index
};

// A candidate together with its block id (used while merging blocks).
using BNode = pair<int, Node>;

int main() {
    if (scanf("%d %d", &n, &k) != 2) return 0;

    const int b = k;      // block size (k is a power of two and k <= n)
    const int m = n / k;  // number of blocks

    vector<vector<Node>> cur(m);

    // ------------------------------------------------------------------
    // Chain pass: one long tasting sequence over blocks 0..m-1, in order.
    // For block t this simultaneously detects duplicates inside the block
    // (an equal element earlier in this block is always in the window) and
    // gives the "suffix" half of the comparison against block t-1.
    // ------------------------------------------------------------------
    reset_mem();
    for (int t = 0; t < m; ++t) {
        for (int j = 0; j < b; ++j) {
            int idx = t * b + j;
            bool y = ask(idx + 1);
            if (!y) cur[t].push_back({j + 1, idx});
        }
    }

    vector<vector<int>> Uidx(m);  // globally-new representative elements of each block
    vector<vector<int>> Upos(m);  // their positions inside the block

    // Complete the comparison of block s against U[s-1]: the chain pass
    // already covered the suffix (positions >= pos); only the prefix, i.e.
    // representatives located before `minpos`, can still match.
    auto rev_chain_test = [&](const vector<int>& Us, vector<Node> cl, int minpos) -> vector<Node> {
        int r = 0;
        vector<char> need(cl.size(), 0);
        for (size_t i = 0; i < cl.size(); ++i)
            if (cl[i].pos > minpos) { need[i] = 1; ++r; }
        if (r == 0) return cl;

        reset_mem();
        for (int i = (int)Us.size() - 1; i >= 0; --i) ask(Us[i] + 1);
        vector<char> keep(cl.size(), 1);
        for (int i = (int)cl.size() - 1; i >= 0; --i) {
            if (need[i]) {
                bool y = ask(cl[i].idx + 1);
                if (y) keep[i] = 0;
            }
        }
        vector<Node> res;
        res.reserve(cl.size());
        for (size_t i = 0; i < cl.size(); ++i)
            if (keep[i]) res.push_back(cl[i]);
        return res;
    };

    // One forward pass: load U, then probe a batch of candidates.
    auto single_pass = [&](const vector<int>& Us, const vector<BNode>& flat,
                           int l, int r, vector<BNode>& out) {
        reset_mem();
        for (int x : Us) ask(x + 1);
        for (int j = l; j < r; ++j)
            if (!ask(flat[j].second.idx + 1)) out.push_back(flat[j]);
    };

    // Forward pass plus, for the candidates whose forward view does not
    // already cover all of U, the reversed complement pass.
    auto two_pass = [&](const vector<int>& Us, const vector<BNode>& flat,
                        int l, int r, vector<BNode>& out) {
        int u = (int)Us.size();
        reset_mem();
        for (int x : Us) ask(x + 1);

        vector<BNode> sur;
        vector<int> fp;
        sur.reserve(r - l);
        fp.reserve(r - l);
        for (int j = l; j < r; ++j) {
            if (!ask(flat[j].second.idx + 1)) {
                sur.push_back(flat[j]);
                fp.push_back(j - l + 1);
            }
        }
        if (sur.empty()) return;

        // Probe at position p sees U[max(1,u+p-k)..u]; if p <= k-u+1 it
        // already sees all of U.
        int thr = k - u + 1;
        bool any = false;
        vector<char> need(sur.size(), 0);
        for (size_t i = 0; i < sur.size(); ++i)
            if (fp[i] > thr) { need[i] = 1; any = true; }
        if (!any) {
            for (auto& x : sur) out.push_back(x);
            return;
        }

        reset_mem();
        for (int i = u - 1; i >= 0; --i) ask(Us[i] + 1);
        vector<char> keep(sur.size(), 1);
        for (int i = (int)sur.size() - 1; i >= 0; --i) {
            if (need[i]) {
                bool y = ask(sur[i].second.idx + 1);
                if (y) keep[i] = 0;
            }
        }
        for (size_t i = 0; i < sur.size(); ++i)
            if (keep[i]) out.push_back(sur[i]);
    };

    long long distinct = 0;
    for (int s = 0; s < m; ++s) {
        // Finish the immediate predecessor comparison (prefix part).
        if (s >= 1 && !Uidx[s - 1].empty()) {
            int minpos = *min_element(Upos[s - 1].begin(), Upos[s - 1].end());
            cur[s] = rev_chain_test(Uidx[s - 1], std::move(cur[s]), minpos);
        }
        for (const Node& nd : cur[s]) {
            Uidx[s].push_back(nd.idx);
            Upos[s].push_back(nd.pos);
        }
        distinct += (long long)cur[s].size();
        if (Uidx[s].empty()) continue;

        // Test the still-unmatched candidates of all blocks t >= s+2 against
        // U[s].  Candidates are kept in increasing block order, so a "Y" can
        // only be caused by U[s] or by a representative of an earlier block:
        // in both cases the candidate is a duplicate and may be dropped.
        vector<BNode> flat;
        for (int t = s + 2; t < m; ++t)
            for (const Node& nd : cur[t]) flat.push_back({t, nd});
        if (flat.empty()) continue;

        const int u = (int)Uidx[s].size();
        const int L1 = k - u + 1;      // batch size fully coverable in one pass
        const int L2 = 2 * k + 1 - u;  // batch size fully coverable in two passes
        const int M = (int)flat.size();

        // Pick the cheaper of: batches of L1 with one pass, or batches of L2
        // with two passes.
        long long Scost = (long long)((M + L1 - 1) / L1) * u + M;
        long long fullT = M / L2;
        int last = M % L2;
        long long lastcost = 0;
        if (last > 0) {
            if (last <= L1) lastcost = (long long)u + last;
            else lastcost = 2LL * u + last + (last - L1);
        }
        long long Tcost = fullT * ((long long)2 * u + L2 + (L2 - L1)) + lastcost;
        bool useT = (Tcost <= Scost);

        vector<BNode> nflat;
        nflat.reserve(M);
        int i = 0;
        while (i < M) {
            int rem = M - i;
            int L = useT ? min(rem, L2) : min(rem, L1);
            if (L <= L1) single_pass(Uidx[s], flat, i, i + L, nflat);
            else two_pass(Uidx[s], flat, i, i + L, nflat);
            i += L;
        }

        for (int t = s + 2; t < m; ++t) cur[t].clear();
        for (auto& x : nflat) cur[x.first].push_back(x.second);
    }

    printf("! %lld\n", distinct);
    fflush(stdout);
    return 0;
}

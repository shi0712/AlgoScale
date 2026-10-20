#include <bits/stdc++.h>
using namespace std;
static const long long MOD = 1000000007LL;

// items l..r of an ordered sequence.
// x[p] = weight when item p is used as a leaf block.
// w[p] = weight when item p is the root of an internal block covering a
//        non-empty consecutive range starting at p.
// Returns the total weight of all "plane forests" over items l..r.
static long long segDP(const vector<long long>& x, const vector<long long>& w, int l, int r) {
    int L = r - l + 1;
    if (L <= 0) return 1;
    vector<long long> cur(L + 2, 0), nxt(L + 2, 0);
    cur[0] = 1;
    for (int p = l; p <= r; p++) {
        fill(nxt.begin(), nxt.end(), 0);
        long long xp = x[p], wp = w[p];
        // leaf: after finishing this block we may close any number of open nodes
        long long run = 0;
        for (int d = L; d >= 0; d--) {
            run += cur[d] * xp % MOD;
            if (run >= MOD) run -= MOD;
            nxt[d] += run;
            if (nxt[d] >= MOD) nxt[d] -= MOD;
        }
        // internal: push a new open node
        for (int d = 0; d <= L; d++) {
            if (cur[d]) {
                nxt[d + 1] += cur[d] * wp % MOD;
                if (nxt[d + 1] >= MOD) nxt[d + 1] -= MOD;
            }
        }
        cur.swap(nxt);
    }
    return cur[0];
}

int main() {
    int n, m;
    if (scanf("%d %d", &n, &m) != 2) return 0;
    vector<vector<int>> perms(m, vector<int>(n));
    for (int k = 0; k < m; k++)
        for (int i = 0; i < n; i++) scanf("%d", &perms[k][i]);

    if (n == 1) { printf("1\n"); return 0; }

    vector<int> pos1(n + 1);
    for (int i = 0; i < n; i++) pos1[perms[0][i]] = i;

    // cnt[x][y] (x<y): times positions x,y are consecutive in some permutation
    vector<int> cnt((size_t)n * n, 0);
    for (int k = 0; k < m; k++)
        for (int s = 0; s + 1 < n; s++) {
            int x = pos1[perms[k][s]], y = pos1[perms[k][s + 1]];
            if (x > y) swap(x, y);
            cnt[(size_t)x * n + y]++;
        }

    // L[i]: largest j such that element at position i is first of [i..j]
    // in every permutation
    vector<int> L(n, n - 1);
    {
        vector<int> a(n), posk(n + 1), st;
        st.reserve(n);
        for (int k = 0; k < m; k++) {
            for (int i = 0; i < n; i++) posk[perms[k][i]] = i;
            for (int i = 0; i < n; i++) a[i] = posk[perms[0][i]];
            st.clear();
            for (int i = n - 1; i >= 0; i--) {
                while (!st.empty() && a[st.back()] > a[i]) st.pop_back();
                int nsr = st.empty() ? n : st.back();
                if (nsr - 1 < L[i]) L[i] = nsr - 1;
                st.push_back(i);
            }
        }
    }

    // common[i][j] : {positions i..j} is an interval in every permutation
    // good[i][j]   : ... and element at position i is the first one in every permutation
    vector<char> common((size_t)n * n, 0);
    vector<char> good((size_t)n * n, 0);
    {
        vector<int> W(n, 0);
        for (int j = 0; j < n; j++) {
            if (j > 0) {
                long long suff = 0;
                for (int i = j - 1; i >= 0; i--) {
                    suff += cnt[(size_t)i * n + j];
                    W[i] += (int)suff - m;
                }
                W[j] = 0;
            }
            for (int i = 0; i <= j; i++) {
                bool c = (W[i] == 0);
                common[(size_t)i * n + j] = c;
                good[(size_t)i * n + j] = c && (j <= L[i]);
            }
        }
    }

    // ---- strong intervals of the common-interval family ----
    vector<int> R(n, 0), Lb(n, n);
    for (int i = 0; i < n; i++)
        for (int j = i; j < n; j++)
            if (common[(size_t)i * n + j]) {
                if (j > R[i]) R[i] = j;
                if (i < Lb[j]) Lb[j] = i;
            }

    vector<char> isStrong((size_t)n * n, 0);
    for (int i = 0; i < n; i++) {
        int mr = -1, ml = n + 1;
        for (int j = i; j < n; j++) {
            if (j > i) {
                if (R[j] > mr) mr = R[j];
                if (Lb[j - 1] < ml) ml = Lb[j - 1];
            }
            if (common[(size_t)i * n + j] && !(mr > j) && !(ml < i))
                isStrong[(size_t)i * n + j] = 1;
        }
    }

    struct Block { int l, r; };
    vector<Block> blocks;
    for (int i = 0; i < n; i++)
        for (int j = i; j < n; j++)
            if (isStrong[(size_t)i * n + j]) blocks.push_back({i, j});
    int B = (int)blocks.size();

    vector<int> par(B, -1);
    vector<vector<int>> children(B);
    for (int i = 0; i < B; i++) {
        int best = -1, bestLen = INT_MAX;
        for (int j = 0; j < B; j++) {
            if (i == j) continue;
            if (blocks[j].l <= blocks[i].l && blocks[i].r <= blocks[j].r &&
                (blocks[j].l < blocks[i].l || blocks[j].r > blocks[i].r)) {
                int len = blocks[j].r - blocks[j].l;
                if (len < bestLen) { bestLen = len; best = j; }
            }
        }
        par[i] = best;
        if (best != -1) children[best].push_back(i);
    }
    for (int i = 0; i < B; i++)
        sort(children[i].begin(), children[i].end(),
             [&](int a, int b) { return blocks[a].l < blocks[b].l; });

    // orientation of complete nodes: 0 unknown, 1 all canonical, 2 all reverse, 3 mixed
    vector<int> orient(B, 0);   // 0 unknown, 1 canonical, 2 reversed, 3 mixed
    {
        vector<int> posk(n + 1);
        for (int k = 0; k < m; k++) {
            for (int i = 0; i < n; i++) posk[perms[k][i]] = i;
            for (int i = 0; i < B; i++) {
                auto& ch = children[i];
                if (ch.size() < 2) continue;
                int e0 = perms[0][blocks[ch.front()].l];
                int e1 = perms[0][blocks[ch.back()].l];
                int st = (posk[e0] < posk[e1]) ? 1 : 2;
                if (orient[i] == 0) orient[i] = st;
                else if (orient[i] != st) orient[i] = 3;
            }
        }
    }

    vector<int> order(B);
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return blocks[a].r - blocks[a].l < blocks[b].r - blocks[b].l;
    });

    vector<long long> Forest(B, 0), Tree(B, 0);
    for (int idx : order) {
        auto& ch = children[idx];
        int k = (int)ch.size();
        if (k == 0) { Forest[idx] = 1; Tree[idx] = 1; continue; }

        // Node type: "complete" iff every adjacent union of children is a common
        // interval and the children keep one orientation (canonical or reversed) in
        // every permutation.  Then every union of >=2 children that is common is a
        // good block iff the first child of the union is good, and segDP applies.
        // Otherwise no union of >=2 children is a good block at all.
        bool complete = (orient[idx] != 3);
        for (int p = 0; complete && p + 1 < k; p++) {
            if (!common[(size_t)blocks[ch[p]].l * n + blocks[ch[p + 1]].r]) complete = false;
        }

        bool isGood = good[(size_t)blocks[idx].l * n + blocks[idx].r];

        vector<long long> x(k), w(k);
        for (int t = 0; t < k; t++) {
            x[t] = Forest[ch[t]];
            w[t] = good[(size_t)blocks[ch[t]].l * n + blocks[ch[t]].r] ? Tree[ch[t]] : 0;
        }

        if (!complete) {
            // prime node: every allowed block is contained in a single child
            long long restProd = 1;
            for (int t = 1; t < k; t++) restProd = restProd * x[t] % MOD;
            long long tr = isGood ? w[0] * restProd % MOD : 0;
            long long all = x[0] * restProd % MOD;
            Tree[idx] = tr;
            Forest[idx] = (all + tr) % MOD;
        } else {
            // complete node with a uniform orientation: the root may take any
            // consecutive run of children as its own block
            if (orient[idx] == 2) {
                reverse(x.begin(), x.end());   // the leftmost child in the
                reverse(w.begin(), w.end());   // permutations comes last here
            }
            Forest[idx] = segDP(x, w, 0, k - 1);
            long long tr = 0;
            if (isGood) tr = w[0] * segDP(x, w, 1, k - 1) % MOD;
            Tree[idx] = tr;
        }
    }

    int rootIdx = -1;
    for (int i = 0; i < B; i++)
        if (blocks[i].l == 0 && blocks[i].r == n - 1) { rootIdx = i; break; }
    printf("%lld\n", Tree[rootIdx] % MOD);
    return 0;
}

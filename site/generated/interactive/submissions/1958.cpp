#include <bits/stdc++.h>
using namespace std;

static int M;

static int readInt() {
    int x;
    if (scanf("%d", &x) != 1) exit(0);
    if (x == -1) exit(0);   // interaction error: stop, print nothing more
    return x;
}

static int queryInsert(int u, int x) {
    printf("? 1 %d %d\n", u, x);
    fflush(stdout);
    return readInt();
}

static vector<int> queryCollect() {
    printf("? 2\n");
    fflush(stdout);
    int s = readInt();
    vector<int> v((size_t)s);
    for (int i = 0; i < s; i++) v[i] = readInt();
    return v;
}

static void printAnswer(int N, const vector<int>& R) {
    printf("! %d", N);
    for (int i = 0; i < N - 1; i++) printf(" %d", R[(size_t)i]);
    printf("\n");
    fflush(stdout);
}

// Build the reported parent array from a local tree.
//   parLocal[local id]  = parent local id (root = local 0, parLocal[0] = -1)
//   leafLocal[u]        = local id of hidden leaf u
// Labels are fixed for the leaves (u) and the root (N-1); internal vertices get
// M, M+1, ... in arbitrary order.
static vector<int> buildAnswer(int N, const vector<int>& parLocal,
                               const vector<int>& leafLocal) {
    if (N <= 0) return vector<int>();
    vector<int> label((size_t)N, -1);
    vector<char> used((size_t)N, 0);
    label[0] = N - 1;
    used[(size_t)(N - 1)] = 1;
    for (int u = 0; u < M; u++) {
        label[(size_t)leafLocal[(size_t)u]] = u;
        used[(size_t)u] = 1;
    }
    int nxt = M;
    for (int i = 1; i < N; i++) {
        if (label[(size_t)i] != -1) continue;
        while (used[(size_t)nxt]) nxt++;
        label[(size_t)i] = nxt;
        used[(size_t)nxt] = 1;
    }
    vector<int> R((size_t)(N - 1));
    for (int i = 1; i < N; i++)
        R[(size_t)label[(size_t)i]] = label[(size_t)parLocal[(size_t)i]];
    return R;
}

int main() {
    if (scanf("%d", &M) != 1) return 0;

    // ---------------- M == 1 : the tree is the chain 0 -> 1 -> ... -> N-1
    if (M == 1) {
        int N = 0;
        while (true) {
            int r = queryInsert(0, 0);
            if (r == 1) N++;
            else break;
        }
        queryCollect();   // empty the machine
        vector<int> R((size_t)(N - 1));
        for (int i = 0; i < N - 1; i++) R[(size_t)i] = i + 1;
        printAnswer(N, R);
        return 0;
    }

    // ---------------- M == 2 : one chain down to a branching vertex, then
    // two chains ending at leaves 0 and 1.  Two count-only rounds suffice.
    if (M == 2) {
        long long c0 = 0, c1 = 0;               // fill order 0, 1
        for (int u = 0; u < 2; u++) {
            while (true) {
                int r = queryInsert(u, 0);
                if (r == 1) { if (u == 0) c0++; else c1++; }
                else break;
            }
        }
        queryCollect();
        long long f0 = 0;                        // fill order 1, 0
        for (int u = 1; u >= 0; u--) {
            while (true) {
                int r = queryInsert(u, 0);
                if (r == 1) { if (u == 0) f0++; }
                else break;
            }
        }
        queryCollect();

        int N = (int)(c0 + c1);
        long long L = (c0 - 1) - f0;   // depth of the branching vertex
        long long leg0 = f0;           // edges from branching vertex to leaf 0
        long long leg1 = c1;           // edges from branching vertex to leaf 1

        vector<int> parLocal((size_t)N, -1);
        int base1 = (int)(L + 1);
        int base2 = (int)(base1 + leg0);
        for (int i = 1; i <= (int)L; i++) parLocal[(size_t)i] = i - 1;
        for (int i = 0; i < (int)leg0; i++)
            parLocal[(size_t)(base1 + i)] = (i == 0 ? (int)L : base1 + i - 1);
        for (int i = 0; i < (int)leg1; i++)
            parLocal[(size_t)(base2 + i)] = (i == 0 ? (int)L : base2 + i - 1);
        vector<int> leafLocal((size_t)M);
        leafLocal[0] = base1 + (int)leg0 - 1;
        leafLocal[1] = base2 + (int)leg1 - 1;
        vector<int> R = buildAnswer(N, parLocal, leafLocal);
        printAnswer(N, R);
        return 0;
    }

    // ---------------- General case (M >= 3).
    //
    // Values round: insert at every leaf, giving leaf u the value u.  The
    // machine is filled and then emptied by one collect.  Since every vertex
    // receives the value of the first leaf (in fill order) whose path passes
    // through it, and the fill order is 0,1,...,M-1, the block of vertices
    // with value u is exactly the path segment added by leaf u.  Blocks are
    // contiguous in the collected preorder, so the order of blocks is a DFS
    // order sigma of the leaves; we also learn N and the per-leaf counts.
    long long total = 0;
    vector<long long> cnt((size_t)M, 0);
    for (int u = 0; u < M; u++) {
        while (true) {
            int r = queryInsert(u, u);
            if (r == 1) { cnt[(size_t)u]++; total++; }
            else break;
        }
    }
    vector<int> seq = queryCollect();
    int N = (int)total;

    vector<int> sigma;
    sigma.reserve((size_t)M);
    {
        int last = -1;
        for (int v : seq)
            if (v != last) { sigma.push_back(v); last = v; }
    }

    // N == M+1: the only internal vertex is the root (a star).
    if (N == M + 1) {
        vector<int> R((size_t)(N - 1), N - 1);
        printAnswer(N, R);
        return 0;
    }

    // If the fill order is itself the DFS order, the counts of the values
    // round are already the forward DFS-order counts; otherwise run one.
    bool identityOrder = true;
    for (int i = 0; i < M; i++)
        if (sigma[(size_t)i] != i) { identityOrder = false; break; }

    // For a DFS order sigma, with d[i] = depth(sigma[i]) and
    // h[i] = depth(LCA(sigma[i-1], sigma[i])), a full fill in forward DFS order
    // gives f[i] = d[i] - h[i] (f[0] = d[0] + 1), and a full fill in reverse
    // DFS order gives r[i] = d[i] - h[i+1] (r[M-1] = d[M-1] + 1).
    // Solving yields every d[i] and h[i].
    vector<long long> f((size_t)M, 0), r((size_t)M, 0);
    if (identityOrder) {
        for (int i = 0; i < M; i++) f[(size_t)i] = cnt[(size_t)i];
    } else {
        for (int i = 0; i < M; i++) {
            int u = sigma[(size_t)i];
            long long c = 0;
            while (true) {
                int res = queryInsert(u, 0);
                if (res == 1) c++;
                else break;
            }
            f[(size_t)i] = c;
        }
        queryCollect();
    }
    for (int i = M - 1; i >= 0; i--) {
        int u = sigma[(size_t)i];
        long long c = 0;
        while (true) {
            int res = queryInsert(u, 0);
            if (res == 1) c++;
            else break;
        }
        r[(size_t)i] = c;
    }
    queryCollect();

    vector<long long> d((size_t)M, 0), h((size_t)M, 0);
    d[0] = f[0] - 1;
    for (int i = 0; i + 1 < M; i++) {
        h[(size_t)(i + 1)] = d[(size_t)i] - r[(size_t)i];
        d[(size_t)(i + 1)] = f[(size_t)(i + 1)] + h[(size_t)(i + 1)];
    }

    // Build the tree: for each leaf in DFS order, attach its new branch at the
    // ancestor of the previous leaf at depth h[i].
    vector<int> parLocal;
    vector<int> depthLocal;
    parLocal.push_back(-1);
    depthLocal.push_back(0);
    vector<int> leafLocal((size_t)M, -1);
    {
        int prev = 0;
        for (int k = 1; k <= (int)d[0]; k++) {
            int id = (int)parLocal.size();
            parLocal.push_back(prev);
            depthLocal.push_back(k);
            prev = id;
        }
        leafLocal[(size_t)sigma[0]] = prev;
    }
    for (int j = 1; j < M; j++) {
        int hj = (int)h[(size_t)j];
        int cur = leafLocal[(size_t)sigma[(size_t)(j - 1)]];
        while (depthLocal[(size_t)cur] > hj) cur = parLocal[(size_t)cur];
        int prev = cur;
        int add = (int)(d[(size_t)j] - h[(size_t)j]);
        for (int k = 1; k <= add; k++) {
            int id = (int)parLocal.size();
            parLocal.push_back(prev);
            depthLocal.push_back(hj + k);
            prev = id;
        }
        leafLocal[(size_t)sigma[(size_t)j]] = prev;
    }

    vector<int> R = buildAnswer(N, parLocal, leafLocal);
    printAnswer(N, R);
    return 0;
}

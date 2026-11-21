#include <bits/stdc++.h>
using namespace std;

static int N, K;
static string ANS;

// returns number of T in A, assuming the sheet has T exactly on A.
// Exits immediately if the answer has been found (reply == N).
static int ask_subset(const vector<int>& A) {
    string s(N, 'F');
    for (int p : A) s[p] = 'T';
    cout << s << '\n' << flush;
    int r;
    if (!(cin >> r)) exit(0);
    if (r == -1) exit(0);
    if (r == N) exit(0);
    // r = N - |A| - K + 2*x
    int x = (r - N + (int)A.size() + K) / 2;
    return x;
}

static void submit_answer() {
    cout << ANS << '\n' << flush;
    int r;
    if (!(cin >> r)) exit(0);
    exit(0);
}

// ---------- binary splitting with a known number of T's ----------
// pos is a random order of the ambiguous positions, c = number of T in pos.
static void solve_known(vector<int> pos, int c) {
    struct Group { int l, r, c; };
    vector<Group> st;
    st.push_back({0, (int)pos.size(), c});
    while (!st.empty()) {
        Group g = st.back(); st.pop_back();
        int len = g.r - g.l;
        if (g.c == 0) continue;
        if (g.c == len) {
            for (int i = g.l; i < g.r; ++i) ANS[pos[i]] = 'T';
            continue;
        }
        int mid = g.l + len / 2;
        vector<int> A;
        A.reserve(mid - g.l);
        for (int i = g.l; i < mid; ++i) A.push_back(pos[i]);
        int x = ask_subset(A);
        int lc = x, rc = g.c - x;
        if (rc > 0) st.push_back({mid, g.r, rc});
        if (lc > 0) st.push_back({g.l, mid, lc});
    }
}

// ---------- weighted block decision tree ----------
static int BB;
static vector<uint8_t> popc;
static vector<double> perPatternW; // weight of one pattern with a given popcount

static const int SENTINEL = INT_MIN;

struct Node {
    uint32_t q;
    int child[21];
};

static vector<Node> nodes;
static int treeRoot = SENTINEL;
static uint32_t rngState = 0x12345678u;
static inline uint32_t rnd32() {
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return rngState;
}

static double entropyWeighted(const vector<double>& cnt) {
    double tot = 0;
    for (double v : cnt) tot += v;
    if (tot <= 0) return 0;
    double H = 0;
    for (double v : cnt) if (v > 1e-300) {
        double p = v / tot;
        H -= p * log2(p);
    }
    return H;
}

static int buildTree(vector<uint32_t> V, const vector<double>& VW, int trials) {
    if (V.size() <= 1) return SENTINEL;
    double totalW = 0;
    for (double w : VW) totalW += w;
    if (totalW <= 0) return SENTINEL;

    double bestH = -1;
    uint32_t bestq = 0;
    for (int t = 0; t < trials; ++t) {
        int sz = BB / 2;
        if (t % 7 == 0) sz = 1 + (int)(rnd32() % BB);
        if (t % 11 == 0) sz = BB / 2 + (int)(rnd32() % 5) - 2;
        if (sz < 1) sz = 1;
        if (sz > BB) sz = BB;
        uint32_t q = 0;
        while (__builtin_popcount(q) < sz) q |= (1u << (rnd32() % BB));
        vector<double> cnt(BB + 1, 0.0);
        for (size_t i = 0; i < V.size(); ++i) {
            cnt[popc[V[i] & q]] += VW[i];
        }
        double H = entropyWeighted(cnt);
        if (H > bestH) { bestH = H; bestq = q; }
    }

    int idx = (int)nodes.size();
    nodes.push_back(Node());
    nodes[idx].q = bestq;
    for (int v = 0; v <= BB; ++v) nodes[idx].child[v] = SENTINEL;

    vector<vector<uint32_t>> parts(BB + 1);
    vector<vector<double>> partsW(BB + 1);
    vector<double> partSum(BB + 1, 0.0);
    for (size_t i = 0; i < V.size(); ++i) {
        int v = popc[V[i] & bestq];
        parts[v].push_back(V[i]);
        partsW[v].push_back(VW[i]);
        partSum[v] += VW[i];
    }
    for (int v = 0; v <= BB; ++v) {
        if (parts[v].empty()) { nodes[idx].child[v] = SENTINEL; continue; }
        if (partSum[v] <= 0) { nodes[idx].child[v] = SENTINEL; continue; }
        if (parts[v].size() == 1) {
            nodes[idx].child[v] = -(int)(parts[v][0]) - 1; // leaf pattern
        } else {
            nodes[idx].child[v] = buildTree(std::move(parts[v]), partsW[v], trials);
        }
    }
    return idx;
}

static void buildTreeWithWeights(int b, vector<double> weights) {
    BB = b;
    perPatternW = std::move(weights);
    popc.assign(1u << BB, 0);
    for (uint32_t i = 1; i < (1u << BB); ++i) popc[i] = popc[i >> 1] + (i & 1);
    vector<uint32_t> V;
    V.reserve(1u << BB);
    for (uint32_t m = 0; m < (1u << BB); ++m) V.push_back(m);
    vector<double> VW(V.size());
    for (size_t i = 0; i < V.size(); ++i) VW[i] = perPatternW[popc[V[i]]];

    nodes.clear();
    nodes.reserve(1000000);
    treeRoot = buildTree(std::move(V), VW, 30);
}

static void buildBlockTree() {
    int b = min(20, N);
    // per-pattern prior weight: C(N-b, K-j)/C(N, K), j = popcount of the block pattern
    int M = N - b;
    vector<double> w(b + 1, 0.0);
    long double logden = lgammal(N + 1) - lgammal(K + 1) - lgammal(N - K + 1);
    for (int j = 0; j <= b; ++j) {
        int r = K - j;
        if (r < 0 || r > M) { w[j] = 0; continue; }
        long double lw = (lgammal(M + 1) - lgammal(r + 1) - lgammal(M - r + 1)) - logden;
        w[j] = (double)expl(lw);
    }
    buildTreeWithWeights(b, std::move(w));
}

static void solve_block_tree(const vector<int>& block, int knownC, int& tcount) {
    int u = treeRoot;
    uint32_t mask = 0;
    bool solvedByFallback = false;
    while (true) {
        if (u == SENTINEL) {
            int c = (knownC >= 0 ? knownC : ask_subset(block));
            solve_known(block, c);
            tcount += c;
            solvedByFallback = true;
            break;
        }
        uint32_t q = nodes[u].q;
        vector<int> A;
        A.reserve(BB);
        for (int i = 0; i < BB; ++i) if ((q >> i) & 1u) A.push_back(block[i]);
        int x = ask_subset(A);
        int ch = nodes[u].child[x];
        if (ch < 0 && ch != SENTINEL) {
            mask = (uint32_t)(-ch - 1);
            break;
        }
        if (ch == SENTINEL) {
            int c = (knownC >= 0 ? knownC : ask_subset(block));
            solve_known(block, c);
            tcount += c;
            solvedByFallback = true;
            break;
        }
        u = ch;
    }
    if (!solvedByFallback) {
        for (int i = 0; i < BB; ++i)
            if ((mask >> i) & 1u) { ANS[block[i]] = 'T'; ++tcount; }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> N)) return 0;
    ANS.assign(N, 'F');

    // Baseline: all F.  Score = number of F in the key.
    {
        string s(N, 'F');
        cout << s << '\n' << flush;
        int r;
        if (!(cin >> r)) return 0;
        if (r == -1) return 0;
        if (r == N) return 0; // key is all F
        K = N - r;
    }
    if (K == 0) return 0;
    if (K == N) {
        string s(N, 'T');
        cout << s << '\n' << flush;
        int r;
        if (cin >> r) {}
        return 0;
    }

    // Random permutation of positions.
    vector<int> perm(N);
    iota(perm.begin(), perm.end(), 0);
    mt19937_64 rng(0x9e3779b97f4a7c15ULL ^ (uint64_t)N * 1000003ULL ^ (uint64_t)K);
    shuffle(perm.begin(), perm.end(), rng);

    int minority = min(K, N - K);
    bool useBinary = (minority <= 3) || ((long long)minority * 20 < N);

    if (useBinary) {
        // Identify the minority set (T if K is minority, otherwise F).
        if (K <= N - K) {
            solve_known(perm, K);
        } else {
            // Identify F positions by treating F as marked, then complement them.
            struct Group { int l, r, c; };
            vector<Group> st;
            st.push_back({0, N, N - K});
            vector<char> markF(N, 0);
            while (!st.empty()) {
                Group g = st.back(); st.pop_back();
                int len = g.r - g.l;
                if (g.c == 0) continue;
                if (g.c == len) {
                    for (int i = g.l; i < g.r; ++i) markF[perm[i]] = 1;
                    continue;
                }
                int mid = g.l + len / 2;
                vector<int> A;
                A.reserve(mid - g.l);
                for (int i = g.l; i < mid; ++i) A.push_back(perm[i]);
                int xT = ask_subset(A);
                int x = (int)A.size() - xT; // number of F (marked) in A
                int lc = x, rc = g.c - x;
                if (rc > 0) st.push_back({mid, g.r, rc});
                if (lc > 0) st.push_back({g.l, mid, lc});
            }
            for (int i = 0; i < N; ++i) ANS[i] = markF[i] ? 'F' : 'T';
        }
    } else {
        // Block strategy.
        buildBlockTree();
        int tcount = 0;
        int blockSize = BB;
        int fullBlocks = N / blockSize;
        for (int b = 0; b < fullBlocks; ++b) {
            vector<int> block;
            block.reserve(blockSize);
            for (int i = 0; i < blockSize; ++i) block.push_back(perm[b * blockSize + i]);
            solve_block_tree(block, -1, tcount);
        }

        int rem = N % blockSize;
        if (rem > 0) {
            vector<int> block;
            block.reserve(rem);
            for (int i = fullBlocks * blockSize; i < N; ++i) block.push_back(perm[i]);
            int c = K - tcount;
            if (c == rem) {
                for (int p : block) ANS[p] = 'T';
            } else if (c > 0) {
                // Build a small tree conditioned on the now-known count of the remainder.
                vector<double> w(rem + 1, 0.0);
                w[c] = 1.0;
                buildTreeWithWeights(rem, std::move(w));
                solve_block_tree(block, c, tcount);
            }
        }
    }

    submit_answer();
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

static int n;
static vector<int> A;

static char ask(int i, int j) {
    printf("? %d %d\n", i, j);
    fflush(stdout);
    char buf[32];
    if (scanf("%31s", buf) != 1) exit(0);
    if (buf[0] != '<' && buf[0] != '>') exit(0);
    return buf[0];
}
struct TNode { int i, j, lt, gt, out; };
struct BlockTree {
    int k = 0, M = 0, root = -1;
    vector<int> pw3; vector<pair<int,int>> pairs;
    vector<vector<unsigned long long>> bits; vector<TNode> nodes;
    long long budget = 0; bool aborted = false;
    void prepare(int kk) {
        k = kk; pw3.assign(k + 1, 1);
        for (int t = 1; t <= k; ++t) pw3[t] = pw3[t-1] * 3;
        M = pw3[k];
        vector<vector<int>> ranks(M, vector<int>(k + 2, 0));
        for (int o = 0; o < M; ++o) {
            int pl[40]; int sz = 2; pl[0] = 0; pl[1] = 1;
            int desc[40]; int ds = 0;
            for (int t = 1; t <= k; ++t) {
                int d = (o / pw3[t-1]) % 3;
                for (int q = sz; q > d; --q) pl[q] = pl[q-1];
                pl[d] = t + 1; ++sz; desc[ds++] = pl[sz-1]; --sz;
            }
            desc[ds++] = pl[1]; desc[ds++] = pl[0];
            for (int r = 0; r < ds; ++r) ranks[o][desc[r]] = r;
        }
        pairs.clear();
        for (int i = 0; i < k + 2; ++i) for (int j = i + 1; j < k + 2; ++j) pairs.push_back({i, j});
        int W = (M + 63) / 64;
        bits.assign(pairs.size(), vector<unsigned long long>(W, 0ULL));
        for (size_t c = 0; c < pairs.size(); ++c) {
            int i = pairs[c].first, j = pairs[c].second;
            for (int o = 0; o < M; ++o)
                if (ranks[o][i] > ranks[o][j]) bits[c][o >> 6] |= (1ULL << (o & 63));
        }
    }
    int build(vector<int>& S, int depth) {
        if (S.size() == 1) { nodes.push_back({-1,-1,-1,-1,S[0]}); return (int)nodes.size() - 1; }
        if (depth <= 0) return -1;
        if ((long long)S.size() > (1LL << depth)) return -1;
        int lim = 1 << (depth - 1);
        vector<pair<int,int>> cand;
        for (size_t c = 0; c < pairs.size(); ++c) {
            const vector<unsigned long long>& bs = bits[c]; int cnt1 = 0;
            for (int o : S) cnt1 += (int)((bs[o >> 6] >> (o & 63)) & 1ULL);
            budget -= (long long)S.size() + 16;
            if (budget < 0) { aborted = true; return -1; }
            int cnt0 = (int)S.size() - cnt1;
            if (cnt0 == 0 || cnt1 == 0) continue;
            int mx = max(cnt0, cnt1); if (mx > lim) continue;
            cand.push_back({mx, (int)c});
        }
        sort(cand.begin(), cand.end());
        size_t save = nodes.size();
        for (auto& pr : cand) {
            int c = pr.second; const vector<unsigned long long>& bs = bits[c];
            vector<int> S0, S1; S0.reserve(S.size()); S1.reserve(S.size());
            for (int o : S) { if ((bs[o >> 6] >> (o & 63)) & 1ULL) S1.push_back(o); else S0.push_back(o); }
            int l = build(S1, depth - 1);
            if (l < 0) { nodes.resize(save); if (aborted) return -1; continue; }
            int r = build(S0, depth - 1);
            if (r < 0) { nodes.resize(save); if (aborted) return -1; continue; }
            nodes.push_back({pairs[c].first, pairs[c].second, l, r, -1});
            return (int)nodes.size() - 1;
        }
        return -1;
    }
    bool make(int kk, int depth, long long bud) {
        prepare(kk); nodes.clear(); aborted = false; budget = bud;
        vector<int> S(M); for (int o = 0; o < M; ++o) S[o] = o;
        root = build(S, depth); return root >= 0;
    }
};
static void doBlock(BlockTree& T, int v, vector<int>& pool) {
    int k = T.k; vector<int> pos(k + 2);
    pos[0] = pool[0]; pos[1] = pool[1];
    for (int t = 1; t <= k; ++t) pos[t + 1] = v - t - 1;
    int cur = T.root;
    while (T.nodes[cur].i >= 0) {
        TNode nd = T.nodes[cur];
        char c = ask(pos[nd.i], pos[nd.j]);
        cur = (c == '<') ? nd.lt : nd.gt;
    }
    int o = T.nodes[cur].out;
    int pl[40]; int sz = 2; pl[0] = 0; pl[1] = 1;
    for (int t = 1; t <= k; ++t) {
        int d = (o / T.pw3[t-1]) % 3;
        for (int q = sz; q > d; --q) pl[q] = pl[q-1];
        pl[d] = t + 1; ++sz; int mx = pl[sz-1]; --sz;
        A[pos[mx]] = v - t + 1;
    }
    pool[0] = pos[pl[0]]; pool[1] = pos[pl[1]];
}
static void naiveRound(int v, vector<int>& pool) {
    int z = v - 2, a = pool[0], b = pool[1];
    char c = ask(z, a);
    if (c == '<') { A[b] = v; pool[0] = z; pool[1] = a; }
    else { char c2 = ask(z, b);
        if (c2 == '>') { A[z] = v; } else { A[b] = v; pool[1] = z; } }
}
int main() {
    if (scanf("%d", &n) != 1) return 0;
    A.assign(n + 1, 0);
    if (n == 1) { printf("! 1\n"); fflush(stdout); return 0; }
    if (n == 2) {
        char c = ask(1, 2);
        if (c == '<') { A[1] = 1; A[2] = 2; } else { A[1] = 2; A[2] = 1; }
        printf("! %d %d\n", A[1], A[2]); fflush(stdout); return 0;
    }
    BlockTree t3, t8;
    bool ok3 = t3.make(3, 5, 20000000LL);
    bool ok8 = false;
    if (n >= 12) ok8 = t8.make(8, 13, 120000000LL);
    int p1 = n - 2, p2 = n - 1, p3 = n, lo, hi;
    { char c = ask(p1, p2); if (c == '<') { lo = p1; hi = p2; } else { lo = p2; hi = p1; } }
    vector<int> pool(2);
    { char c = ask(hi, p3);
      if (c == '>') { A[hi] = n; char c2 = ask(lo, p3);
          if (c2 == '<') { pool[0] = lo; pool[1] = p3; } else { pool[0] = p3; pool[1] = lo; } }
      else { A[p3] = n; pool[0] = lo; pool[1] = hi; } }
    int v = n - 1;
    while (v >= 3) {
        if (ok8 && v >= 10) { doBlock(t8, v, pool); v -= 8; }
        else if (ok3 && v >= 5) { doBlock(t3, v, pool); v -= 3; }
        else { naiveRound(v, pool); v -= 1; }
    }
    A[pool[1]] = 2; A[pool[0]] = 1;
    printf("!");
    for (int i = 1; i <= n; ++i) printf(" %d", A[i]);
    printf("\n"); fflush(stdout);
    return 0;
}

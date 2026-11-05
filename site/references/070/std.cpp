#include <bits/stdc++.h>
using namespace std;

using i64 = long long;
using u64 = unsigned long long;

struct Tree {
    int n = 0, root = -1;
    vector<vector<int>> ch;
    vector<i64> w;                 // weight of parent edge; 0 for the real root
    vector<int> sz, heavy, tin, rtin, pre, rpre;

    void dfs(int u) {
        tin[u] = (int)pre.size();
        pre.push_back(u);
        sz[u] = 1;
        heavy[u] = -1;
        for (int v : ch[u]) {
            dfs(v);
            sz[u] += sz[v];
            if (heavy[u] == -1 || sz[v] > sz[heavy[u]]) heavy[u] = v;
        }
    }

    void dfsReverse(int u) {
        rtin[u] = (int)rpre.size();
        rpre.push_back(u);
        for (int i = (int)ch[u].size() - 1; i >= 0; --i) dfsReverse(ch[u][i]);
    }

    void read() {
        cin >> n;
        ch.assign(n, {});
        w.assign(n, 0);
        vector<int> indeg(n, 0);
        for (int u = 0; u < n; ++u) {
            int k;
            cin >> k;
            ch[u].reserve(k);
            while (k--) {
                int v;
                i64 weight;
                cin >> v >> weight;
                --v;
                ch[u].push_back(v);
                w[v] = weight;
                ++indeg[v];
            }
        }
        for (int i = 0; i < n; ++i) if (indeg[i] == 0) { root = i; break; }

        sz.assign(n, 0);
        heavy.assign(n, -1);
        tin.assign(n, -1);
        rtin.assign(n, -1);
        pre.clear(); pre.reserve(n);
        rpre.clear(); rpre.reserve(n);
        dfs(root);
        dfsReverse(root);
    }
};

// The smaller side never has more than 100 vertices.  A 128-bit mask is enough.
struct Mask128 {
    u64 lo = 0, hi = 0;

    bool empty() const { return lo == 0 && hi == 0; }
    int first() const {
        if (lo) return __builtin_ctzll(lo);
        return 64 + __builtin_ctzll(hi);
    }
    void set(int p) {
        if (p < 64) lo |= 1ULL << p;
        else hi |= 1ULL << (p - 64);
    }
    void reset(int p) {
        if (p < 64) lo &= ~(1ULL << p);
        else hi &= ~(1ULL << (p - 64));
    }
    Mask128 without(const Mask128& x) const {
        return Mask128{lo & ~x.lo, hi & ~x.hi};
    }
};

static Mask128 rangeMask(int l, int len) {
    Mask128 m;
    for (int i = 0; i < len; ++i) m.set(l + i);
    return m;
}

// Enumerate the O(q^2) forests obtainable by repeatedly deleting the leftmost
// or rightmost root from one subtree on the smaller side.
struct SmallForests {
    struct State {
        int root[2] = {0, 0};  // 0 = leftmost, 1 = rightmost, local preorder id
        int del[2] = {0, 0};   // delete only that root, promoting its children
        int cut[2] = {0, 0};   // delete that root's whole subtree
    };

    int q;
    vector<int> node, revPos, revToPre, id, childrenId;
    vector<Mask128> sub, rsub;
    vector<State> state;

    int get(Mask128 mask, Mask128 rmask) {
        if (mask.empty()) return 0;
        int left = mask.first();
        int firstReverse = rmask.first();
        int key = left * q + firstReverse;
        if (id[key] != -1) return id[key];

        State s;
        s.root[0] = left;
        s.root[1] = revToPre[firstReverse];
        for (int d = 0; d < 2; ++d) {
            int x = s.root[d];
            Mask128 a = mask, b = rmask;
            a.reset(x);
            b.reset(revPos[x]);
            s.del[d] = get(a, b);
            s.cut[d] = get(mask.without(sub[x]), rmask.without(rsub[x]));
        }

        // All transitions go to a strictly smaller forest and therefore have
        // already received ids.  Numbering in this postorder is useful in DP.
        int ans = (int)state.size();
        state.push_back(s);
        id[key] = ans;
        return ans;
    }

    SmallForests(const Tree& t, int r) : q(t.sz[r]) {
        assert(q <= 100);
        node.resize(q);
        revPos.resize(q);
        revToPre.resize(q);
        sub.resize(q);
        rsub.resize(q);
        childrenId.resize(q);
        id.assign(q * q, -1);
        state.reserve((size_t)q * q + 1);
        state.push_back(State{}); // state 0 = empty forest

        for (int i = 0; i < q; ++i) {
            int u = t.pre[t.tin[r] + i];
            node[i] = u;
            int j = t.rtin[u] - t.rtin[r];
            revPos[i] = j;
            revToPre[j] = i;
            sub[i] = rangeMask(i, t.sz[u]);
            rsub[i] = rangeMask(j, t.sz[u]);
        }

        Mask128 all = rangeMask(0, q);
        get(all, all);
        for (int i = 0; i < q; ++i) {
            Mask128 a = sub[i], b = rsub[i];
            a.reset(i);
            b.reset(revPos[i]);
            childrenId[i] = get(a, b);
        }
    }
};

struct Solver {
    Tree A, B;
    i64 c1, c2, c3;
    vector<i64> gain, inside; // index = a * B.n + b

    inline i64& INS(int a, int b) { return inside[(size_t)a * B.n + b]; }
    inline i64 GAIN(int a, int b) const { return gain[(size_t)a * B.n + b]; }

    struct Step {
        int u, dir;
        bool onPath;
    };

    using SFState = SmallForests::State;

    // DP for one heavy path on the currently larger subtree.  Values needed
    // from light subtrees have already been computed recursively.
    void pathDP(int ra, int rb, bool bigIsA) {
        const Tree& big = bigIsA ? A : B;
        const Tree& small = bigIsA ? B : A;
        int br = bigIsA ? ra : rb;
        int sr = bigIsA ? rb : ra;

        SmallForests f(small, sr);
        int p = big.sz[br], q = small.sz[sr];
        int states = (int)f.state.size();

        vector<Step> order;
        order.reserve(p);
        for (int u = br; u != -1; u = big.heavy[u]) {
            order.push_back({u, 0, true});
            int h = big.heavy[u];
            if (h == -1) break;

            for (int v : big.ch[u]) {
                if (v == h) break;
                int e = big.tin[v] + big.sz[v];
                for (int i = big.tin[v]; i < e; ++i) order.push_back({big.pre[i], 0, false});
            }
            for (int k = (int)big.ch[u].size() - 1; k >= 0; --k) {
                int v = big.ch[u][k];
                if (v == h) break;
                int e = big.rtin[v] + big.sz[v];
                for (int i = big.rtin[v]; i < e; ++i) order.push_back({big.rpre[i], 1, false});
            }
        }
        assert((int)order.size() == p);

        vector<i64> dp((size_t)(p + 1) * states, 0);
        vector<i64> bonus(q);
        for (int i = p - 1; i >= 0; --i) {
            int y = order[i].u;
            int d = order[i].dir;
            i64* cur = dp.data() + (size_t)i * states;
            const i64* nxt = cur + states;
            assert(i + big.sz[y] <= p);
            const i64* after = dp.data() + (size_t)(i + big.sz[y]) * states;

            for (int k = 0; k < q; ++k) {
                int x = f.node[k];
                int a = bigIsA ? y : x;
                int b = bigIsA ? x : y;
                if (order[i].onPath) INS(a, b) = nxt[f.childrenId[k]];
                bonus[k] = GAIN(a, b) + INS(a, b);
            }

            for (int s = 1; s < states; ++s) {
                const SFState& st = f.state[s];
                i64 best = cur[st.del[d]];
                if (nxt[s] > best) best = nxt[s];
                i64 take = after[st.cut[d]] + bonus[st.root[d]];
                if (take > best) best = take;
                cur[s] = best;
            }
        }
    }

    void solve(int ra, int rb) {
        if (A.sz[ra] == 1 || B.sz[rb] == 1) return;

        bool bigIsA = A.sz[ra] >= B.sz[rb];
        const Tree& big = bigIsA ? A : B;
        int br = bigIsA ? ra : rb;

        // Recurse on every light subtree of the chosen heavy decomposition.
        for (int u = br; u != -1; u = big.heavy[u]) {
            for (int v : big.ch[u]) {
                if (v == big.heavy[u]) continue;
                if (bigIsA) solve(v, rb);
                else solve(ra, v);
            }
        }
        pathDP(ra, rb, bigIsA);
    }

    void run() {
        cin >> c1 >> c2 >> c3;
        A.read();
        B.read();

        gain.assign((size_t)A.n * B.n, 0);
        inside.assign((size_t)A.n * B.n, 0);

        i64 base = 0;
        for (i64 w : A.w) base += c2 * w;
        for (i64 w : B.w) base += c1 * w;

        for (int a = 0; a < A.n; ++a) {
            for (int b = 0; b < B.n; ++b) {
                gain[(size_t)a * B.n + b] = c2 * A.w[a] + c1 * B.w[b]
                    - c3 * llabs(A.w[a] - B.w[b]);
            }
        }

        solve(A.root, B.root);
        cout << base - INS(A.root, B.root) << '\n';
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    solver.run();
    return 0;
}

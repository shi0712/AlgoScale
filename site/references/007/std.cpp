// "归流之树" — deterministic O((n+m) log n), O(n log n) words.
// Adapted from the final window-preprocessing algorithm in this conversation.
// n <= 500,000; m <= 100,000; exact signed 64-bit answers.
// No answer table, randomized hash, or slower algorithm is used.
// All tree-depth traversals are iterative; only O(log n)-depth PST recursion.
// Compile: g++ -std=c++17 -O2 -pipe -DONLINE_JUDGE std17.cpp -o std
#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

class FastInput {
    static constexpr int S = 1 << 16;
    char buf[S];
    int p = 0, len = 0;
    char get() {
        if (p == len) {
            len = (int)fread(buf, 1, S, stdin);
            p = 0;
            if (!len) return 0;
        }
        return buf[p++];
    }
public:
    // The legacy data identifier is non-negative but is not used by the solver.
    // Skip its decimal representation without converting it to a bounded int.
    void skipIdentifier() {
        char c;
        do { c = get(); } while (c && c <= ' ');
        while (c >= '0' && c <= '9') c = get();
    }
    int read() {
        char c;
        do { c = get(); } while (c && c <= ' ');
        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = get();
        }
        return x;
    }
};

class PersistentCounts {
    struct Node { int l = 0, r = 0, sum = 0; };
    vector<Node> tr;
    int K;

    int build(int l, int r, const vector<int>& cnt) {
        int u = (int)tr.size();
        tr.push_back({});
        if (l == r) {
            tr[u].sum = cnt[l];
        } else {
            int mid = (l + r) / 2;
            int a = build(l, mid, cnt);
            int b = build(mid + 1, r, cnt);
            tr[u] = {a, b, tr[a].sum + tr[b].sum};
        }
        return u;
    }

    int insert(int old, int l, int r, int pos) {
        int u = (int)tr.size();
        tr.push_back(tr[old]);
        ++tr[u].sum;
        if (l != r) {
            int mid = (l + r) / 2;
            if (pos <= mid) {
                int v = insert(tr[u].l, l, mid, pos);
                tr[u].l = v;
            } else {
                int v = insert(tr[u].r, mid + 1, r, pos);
                tr[u].r = v;
            }
        }
        return u;
    }

public:
    int all = 0;
    PersistentCounts(int k, int n, const vector<int>& cnt) : K(k) {
        int lg = 1;
        while ((1 << lg) < K) ++lg;
        tr.reserve(2 * K + (size_t)n * (lg + 1) + 5);
        tr.push_back({});
        all = build(1, K, cnt);
    }
    int insert(int old, int pos) { return insert(old, 1, K, pos); }

    // Number of keys <= pos in one root-to-vertex version.
    int prefixCount(int u, int pos) const {
        int l = 1, r = K, ans = 0;
        while (l < r) {
            int mid = (l + r) / 2;
            if (pos <= mid) {
                u = tr[u].l;
                r = mid;
            } else {
                ans += tr[tr[u].l].sum;
                u = tr[u].r;
                l = mid + 1;
            }
        }
        return ans + tr[u].sum;
    }

    // kth largest key in (all vertices) minus (one root-to-vertex path).
    int kthOutside(int pathRoot, int k) const {
        int a = all, b = pathRoot, l = 1, r = K;
        while (l < r) {
            int mid = (l + r) / 2;
            int rightCount = tr[tr[a].r].sum - tr[tr[b].r].sum;
            if (k <= rightCount) {
                a = tr[a].r; b = tr[b].r; l = mid + 1;
            } else {
                k -= rightCount;
                a = tr[a].l; b = tr[b].l; r = mid;
            }
        }
        return l;
    }
};

class Solver {
    int n, root = 0, K = 0, LOG = 0;
    vector<vector<int>> adj, up;
    vector<int> parent, dep, order, sz, height, tin;
    vector<int> type, best, head, baseRank, version;
    vector<int> typeParent, typeHeight, typeTin, typeFirst;
    vector<int> globalThreshold, lg;
    vector<vector<int>> rmq;
    vector<int64> rankSum;
    unique_ptr<PersistentCounts> pst;

    // Windows at scale B=2^k have stride 4B and width at most 8B.
    // Only vertices with subtree height >= B-1 are included.
    struct Window {
        int start = 0, len = 0;
        vector<int> special;  // local indices of parents of internal light edges
        vector<int64> forward, backward;
    };
    struct PrefixInfo {
        int index = -1, base = -1, row = -1;
        int low = 0, high = -1, headType = 0;
    };
    vector<int> byRank, position;
    vector<vector<Window>> windows;

    pair<int, vector<int>> farthest(int start) const {
        vector<int> p(n + 1, -1), d(n + 1, 0), q;
        q.reserve(n);
        q.push_back(start); p[start] = 0;
        int far = start;
        for (size_t i = 0; i < q.size(); ++i) {
            int u = q[i];
            if (d[u] > d[far]) far = u;
            for (int v : adj[u]) if (v != p[u]) {
                p[v] = u; d[v] = d[u] + 1; q.push_back(v);
            }
        }
        return {far, move(p)};
    }

    void rootTree() {
        int a = farthest(1).first;
        auto result = farthest(a);
        int b = result.first;
        vector<int> diameter;
        for (int u = b; u; u = result.second[u]) diameter.push_back(u);
        root = diameter[diameter.size() / 2];

        vector<int> st{root};
        order.reserve(n);
        while (!st.empty()) {
            int u = st.back(); st.pop_back();
            tin[u] = (int)order.size();
            order.push_back(u);
            for (int v : adj[u]) if (v != parent[u]) {
                parent[v] = u;
                dep[v] = dep[u] + 1;
                st.push_back(v);
            }
        }
        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            sz[u] = 1;
            for (int v : adj[u]) if (parent[v] == u) {
                sz[u] += sz[v];
                height[u] = max(height[u], height[v] + 1);
            }
        }
        LOG = 1;
        while ((1 << LOG) <= n) ++LOG;
        up.assign(LOG, vector<int>(n + 1));
        up[0] = parent;
        for (int k = 1; k < LOG; ++k)
            for (int u = 1; u <= n; ++u)
                up[k][u] = up[k - 1][up[k - 1][u]];
    }

    // Exact lexicographic sorting of sorted child-type sequences.
    // The signature tries avoid hashing and long-string comparison costs.
    void rankTypes() {
        vector<vector<int>> bucket(height[root] + 1);
        for (int u = 1; u <= n; ++u) bucket[height[u]].push_back(u);
        vector<int> terminal(n + 1);
        typeParent.push_back(0);
        typeHeight.push_back(-1);

        struct TrieNode {
            vector<pair<int, int>> next;
            int representative = 0, id = 0;
        };

        for (auto &nodes : bucket) {
            vector<TrieNode> tr(1);
            map<pair<int, int>, int> edge;
            for (int u : nodes) {
                vector<int> key;
                for (int v : adj[u]) if (parent[v] == u) {
                    key.push_back(type[v]);
                    if (!best[u] || type[v] > type[best[u]]) best[u] = v;
                }
                sort(key.begin(), key.end(), greater<int>());
                int cur = 0;
                for (int x : key) {
                    auto it = edge.find({cur, x});
                    if (it == edge.end()) {
                        int v = (int)tr.size();
                        tr.emplace_back();
                        edge.emplace(make_pair(cur, x), v);
                        tr[cur].next.push_back({x, v});
                        cur = v;
                    } else {
                        cur = it->second;
                    }
                }
                terminal[u] = cur;
                tr[cur].representative = u;
            }
            for (auto &v : tr) sort(v.next.begin(), v.next.end());
            vector<int> st{0};
            while (!st.empty()) {
                int u = st.back(); st.pop_back();
                if (tr[u].representative) {
                    int v = tr[u].representative;
                    tr[u].id = ++K;
                    typeParent.push_back(best[v] ? type[best[v]] : 0);
                    typeHeight.push_back(height[v]);
                }
                for (int i = (int)tr[u].next.size() - 1; i >= 0; --i)
                    st.push_back(tr[u].next[i].second);
            }
            for (int u : nodes) type[u] = tr[terminal[u]].id;
        }
    }

    // Type tree: parent(type) = its maximum child's type.
    // Its Euler RMQ gives common maximum-child-prefix lengths in O(1).
    void buildTypeTree() {
        vector<vector<int>> children(K + 1);
        for (int t = 2; t <= K; ++t) children[typeParent[t]].push_back(t);
        typeTin.assign(K + 1, 0);
        typeFirst.assign(K + 1, 0);
        vector<int> euler, next(K + 1, 0), st{1};
        euler.reserve(2 * K - 1);
        int timer = 0;
        typeTin[1] = ++timer;
        typeFirst[1] = 0;
        euler.push_back(0);
        while (!st.empty()) {
            int u = st.back();
            if (next[u] < (int)children[u].size()) {
                int v = children[u][next[u]++];
                typeTin[v] = ++timer;
                typeFirst[v] = (int)euler.size();
                euler.push_back(typeHeight[v]);
                st.push_back(v);
            } else {
                st.pop_back();
                if (!st.empty()) euler.push_back(typeHeight[st.back()]);
            }
        }
        int E = (int)euler.size();
        lg.assign(E + 1, 0);
        for (int i = 2; i <= E; ++i) lg[i] = lg[i / 2] + 1;
        rmq.push_back(move(euler));
        for (int k = 1; (1 << k) <= E; ++k) {
            int len = E - (1 << k) + 1;
            rmq.emplace_back(len);
            for (int i = 0; i < len; ++i)
                rmq[k][i] = min(rmq[k - 1][i],
                                rmq[k - 1][i + (1 << (k - 1))]);
        }
    }

    int commonHeight(int a, int b) const {
        int l = typeFirst[a], r = typeFirst[b];
        if (l > r) swap(l, r);
        int k = lg[r - l + 1];
        return min(rmq[k][l], rmq[k][r - (1 << k) + 1]);
    }

    void buildVersions() {
        vector<int> count(K + 1, 0), typeRank(K + 1, 0);
        for (int u = 1; u <= n; ++u) ++count[type[u]];
        globalThreshold.assign(n + 1, 0);
        int pos = 1;
        for (int t = K; t >= 1; --t) {
            typeRank[t] = pos;
            for (int j = 0; j < count[t]; ++j) globalThreshold[pos++] = t;
        }
        pst = make_unique<PersistentCounts>(K, n, count);
        for (int u : order) {
            baseRank[u] = typeRank[type[u]];
            rankSum[u] = rankSum[parent[u]] + baseRank[u];
            version[u] = pst->insert(version[parent[u]], type[u]);
            if (!parent[u] || best[parent[u]] != u) head[u] = u;
            else head[u] = head[parent[u]];
        }
    }

    bool ancestor(int u, int v) const {
        return tin[u] <= tin[v] && tin[v] < tin[u] + sz[u];
    }
    int lca(int u, int v) const {
        if (ancestor(u, v)) return u;
        if (ancestor(v, u)) return v;
        for (int k = LOG - 1; k >= 0; --k)
            if (up[k][u] && !ancestor(up[k][u], v)) u = up[k][u];
        return parent[u];
    }

    // Deepest vertex in [w,v] whose type >= threshold; w means empty
    // when this is used for the exclusive path (w,v].
    int prefixEnd(int w, int v, int threshold) const {
        if (type[v] >= threshold) return v;
        for (int k = LOG - 1; k >= 0; --k) {
            int a = up[k][v];
            if (a && dep[a] > dep[w] && type[a] < threshold) v = a;
        }
        int a = parent[v];
        return a && dep[a] > dep[w] ? a : w;
    }

    int outsideThreshold(int x, int r) const {
        if (r <= dep[x] + 1) return K + 1;
        return pst->kthOutside(version[x], r - dep[x] - 1);
    }

    // Exact number of larger-product pairs between two maximum-child
    // chain segments. Different heights are arithmetic; equal heights
    // are decided by one LCA in the type tree.
    static int64 positiveTriangle(int64 x) {
        return x > 0 ? x * (x + 1) / 2 : 0;
    }

    int64 chainGreater(const PrefixInfo& a, const PrefixInfo& b) const {
        int64 ans = positiveTriangle(a.high - b.low)
                  - positiveTriangle(a.low - 1 - b.low)
                  - positiveTriangle(a.high - b.high - 1)
                  + positiveTriangle(a.low - b.high - 2);
        if (typeTin[a.headType] > typeTin[b.headType]) {
            int low = max({a.low, b.low,
                           commonHeight(a.headType, b.headType) + 1});
            int high = min(a.high, b.high);
            ans += max(0, high - low + 1);
        }
        return ans;
    }

    void buildWindows() {
        byRank.resize(n);
        iota(byRank.begin(), byRank.end(), 1);
        sort(byRank.begin(), byRank.end(), [&](int u, int v) {
            if (type[u] != type[v]) return type[u] > type[v];
            return u < v;
        });
        position.assign(n + 1, -1);
        for (int i = 0; i < n; ++i) position[byRank[i]] = i;

        int limit = n;
        for (int B = 1; B <= height[root] + 1; B <<= 1) {
            while (limit && height[byRank[limit - 1]] < B - 1) --limit;
            windows.emplace_back();
            auto& level = windows.back();
            level.reserve((limit + 4 * B - 1) / (4 * B));

            for (int start = 0; start < limit; start += 4 * B) {
                level.emplace_back();
                Window& w = level.back();
                w.start = start;
                w.len = min(8 * B, limit - start);
                int L = w.len;
                vector<int> par(L, -1), length(L, 0);
                vector<unsigned char> special(L, 0), marked(L, 0);

                for (int i = 0; i < L; ++i) {
                    int u = byRank[start + i];
                    int p = position[parent[u]] - start;
                    if (p >= 0) {
                        par[i] = p;
                        length[i] = length[p] + 1;
                        if (head[u] == u) special[p] = 1;
                    } else {
                        length[i] = 1;
                    }
                }
                for (int i = 0; i < L; ++i)
                    if (special[i]) w.special.push_back(i);

                size_t cells = (size_t)L * w.special.size();
                w.forward.resize(cells);
                w.backward.resize(cells);

                for (int row = 0; row < (int)w.special.size(); ++row) {
                    int a = w.special[row];
                    fill(marked.begin(), marked.end(), 0);
                    for (int v = a; v != -1; v = par[v]) marked[v] = 1;
                    size_t offset = (size_t)row * L;
                    int larger = 0;

                    for (int i = 0; i < L; ) {
                        int j = i, equal = 0;
                        while (j < L && type[byRank[start + j]] ==
                                          type[byRank[start + i]]) {
                            equal += marked[j];
                            ++j;
                        }
                        int smaller = length[a] - larger - equal;
                        for (int v = i; v < j; ++v) {
                            w.forward[offset + v] = larger;
                            w.backward[offset + v] = smaller;
                            if (par[v] != -1) {
                                w.forward[offset + v] +=
                                    w.forward[offset + par[v]];
                                w.backward[offset + v] +=
                                    w.backward[offset + par[v]];
                            }
                        }
                        larger += equal;
                        i = j;
                    }
                }
            }
        }
    }

    // The path from the local window root to u, represented by its
    // final maximum-child segment and the tabulated prefix above it.
    PrefixInfo describe(const Window& w, int u) const {
        PrefixInfo p;
        if (!u || position[u] < w.start) return p;
        assert(position[u] < w.start + w.len);
        p.index = position[u] - w.start;
        p.low = height[u];
        p.headType = type[head[u]];

        int base = parent[head[u]];
        if (base && position[base] >= w.start) {
            p.base = position[base] - w.start;
            p.row = int(lower_bound(w.special.begin(), w.special.end(), p.base)
                        - w.special.begin());
            assert(p.row < (int)w.special.size() && w.special[p.row] == p.base);
            p.high = height[u] + dep[u] - dep[base] - 1;
        } else {
            int top = u;
            for (int k = LOG - 1; k >= 0; --k) {
                int v = up[k][top];
                if (v && position[v] >= w.start) top = v;
            }
            p.high = height[u] + dep[u] - dep[top];
        }
        return p;
    }

    int64 windowPrefix(const Window& w, const PrefixInfo& a,
                       const PrefixInfo& b) const {
        if (a.index == -1 || b.index == -1) return 0;
        int64 ans = chainGreater(a, b);
        if (a.row != -1)
            ans += w.forward[(size_t)a.row * w.len + b.index];
        if (b.row != -1)
            ans += w.backward[(size_t)b.row * w.len + a.index];
        if (a.row != -1 && b.row != -1)
            ans -= w.forward[(size_t)a.row * w.len + b.base];
        return ans;
    }

    int64 windowRectangle(const Window& w, int beforeA, int a,
                          int beforeB, int b) const {
        auto a0 = describe(w, beforeA), a1 = describe(w, a);
        auto b0 = describe(w, beforeB), b1 = describe(w, b);
        return windowPrefix(w, a1, b1) - windowPrefix(w, a0, b1)
             - windowPrefix(w, a1, b0) + windowPrefix(w, a0, b0);
    }

    int jump(int u, int distance) const {
        for (int k = 0; distance; ++k, distance >>= 1)
            if (distance & 1) u = up[k][u];
        return u;
    }

    // Count comparisons with a partially accepted target suffix.
    // After clipping, I=(type[b], type[firstB]] has fewer than 2H
    // vertices, where H=height[b]+1. One preprocessed window covers I.
    int64 partialGreaterPairs(int w, int a, int beforeB, int b) const {
        int lenB = dep[b] - dep[beforeB];
        if (!lenB || a == w) return 0;
        int firstB = jump(b, lenB - 1);
        int beforeA = prefixEnd(w, a, type[firstB] + 1);
        int lastA = prefixEnd(w, a, type[b] + 1);
        int64 ans = (int64)(dep[beforeA] - dep[w]) * lenB;
        if (beforeA == lastA) return ans;

        // Every remaining A vertex is strictly larger than b.
        ans += dep[lastA] - dep[beforeA];
        if (lenB == 1) return ans;

        int H = height[b] + 1;
        int k = 31 - __builtin_clz((unsigned)H);
        int B = 1 << k;
        int left = baseRank[firstB] - 1;
        assert(baseRank[b] - baseRank[firstB] < 2 * H);
        const Window& block = windows[k][left / (4 * B)];
        return ans + windowRectangle(block, beforeA, lastA,
                                     beforeB, parent[b]);
    }

    static int64 triangle(int64 x) { return x * (x + 1) / 2; }
    static int64 nearAncestorPairs(int len, int r) {
        int64 k = min(len, r - 1);
        return triangle(k) + (int64)(len - k) * (r - 1);
    }

public:
    explicit Solver(int vertices) : n(vertices), adj(n + 1), parent(n + 1), dep(n + 1),
        sz(n + 1), height(n + 1), tin(n + 1), type(n + 1), best(n + 1),
        head(n + 1), baseRank(n + 1), version(n + 1), rankSum(n + 1) {}
    void addEdge(int u, int v) { adj[u].push_back(v); adj[v].push_back(u); }
    void prepare() {
        rootTree();
        rankTypes();
        buildTypeTree();
        buildVersions();
        // Construction-only storage is released before building the windows.
        // This is a storage-lifetime change, not an algorithmic fallback.
        vector<vector<int>>().swap(adj);
        vector<int>().swap(order);
        vector<int>().swap(best);
        vector<int>().swap(typeParent);
        vector<int>().swap(typeHeight);
        buildWindows();
    }

    int64 query(int s, int t, int ox, int oy, int r) const {
        if (ox == 0 && oy == 0) {
            int rank;
            if (ancestor(t, s)) rank = dep[s] - dep[t] + 1;
            else rank = baseRank[t] + pst->prefixCount(version[s], type[t]);
            return rank <= r;
        }
        int w = lca(s, t);
        int A = dep[s] - dep[w], B = dep[t] - dep[w];
        if (ox == 0) {
            int q = prefixEnd(w, t, outsideThreshold(s, r));
            return min(A + 1, r) + dep[q] - dep[w];
        }
        if (oy == 0) {
            if (w == t) return min(A + 1, r);
            if (baseRank[t] > r) return 1;
            int suffix = pst->prefixCount(version[s], type[t]);
            int allowed = A - suffix + r - baseRank[t];
            return B + 1 + min(A, max(0, allowed));
        }

        int ar = prefixEnd(w, s, globalThreshold[r]);
        int br = prefixEnd(w, t, globalThreshold[r]);
        int ka = dep[ar] - dep[w], kb = dep[br] - dep[w];
        int64 ans = (int64)A + B + 1;
        ans += nearAncestorPairs(A, r) + nearAncestorPairs(B, r);
        ans += triangle(ka) + triangle(kb);
        if (!A || !B) return ans;

        int aq = prefixEnd(w, s, outsideThreshold(t, r));
        int bq = prefixEnd(w, t, outsideThreshold(s, r));
        int qa = dep[aq] - dep[w], qb = dep[bq] - dep[w];
        ans += (int64)A * qb + (int64)(kb - qb) * r
             - (rankSum[br] - rankSum[bq]);
        ans += (int64)B * qa + (int64)(ka - qa) * r
             - (rankSum[ar] - rankSum[aq]);
        ans += partialGreaterPairs(w, ar, bq, br);
        ans += partialGreaterPairs(w, br, aq, ar);
        return ans;
    }
};

int main() {
    FastInput in;
    in.skipIdentifier();
    int n = in.read();
    Solver solver(n);
    for (int i = 1; i < n; ++i) {
        int u = in.read(), v = in.read();
        solver.addEdge(u, v);
    }
    solver.prepare();
    int m = in.read();
    string output;
    output.reserve((size_t)m * 12);
    while (m--) {
        int s = in.read(), t = in.read(), ox = in.read(), oy = in.read(), r = in.read();
        output += to_string(solver.query(s, t, ox, oy, r));
        output.push_back('\n');
    }
    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}

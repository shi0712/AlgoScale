#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

inline int addmod(int a, int b) {
    int s = a + b;
    if (s >= MOD) s -= MOD;
    return s;
}
inline int submod(int a, int b) {
    int s = a - b;
    if (s < 0) s += MOD;
    return s;
}
inline int mulmod(int a, int b) {
    return int((long long)a * b % MOD);
}

struct Frac {
    int n = 0, d = 1;
};

// Sum of two fractions, without division.
inline Frac combine(const Frac& a, const Frac& b) {
    return {
        addmod(mulmod(a.n, b.d), mulmod(b.n, a.d)),
        mulmod(a.d, b.d)
    };
}

struct Mat {
    int a = 0, b = 0, c = 0, d = 0;
};

inline Mat matMul(const Mat& A, const Mat& B) {
    return {
        addmod(mulmod(A.a, B.a), mulmod(A.b, B.c)),
        addmod(mulmod(A.a, B.b), mulmod(A.b, B.d)),
        addmod(mulmod(A.c, B.a), mulmod(A.d, B.c)),
        addmod(mulmod(A.c, B.b), mulmod(A.d, B.d))
    };
}

inline Frac applyMat(const Mat& m, const Frac& x) {
    return {
        addmod(mulmod(m.a, x.n), mulmod(m.b, x.d)),
        addmod(mulmod(m.c, x.n), mulmod(m.d, x.d))
    };
}

// A function is either a constant pair, or x -> Mx.
struct Func {
    bool constant = true;
    Frac value{0, 1};
    Mat mat{};
};

inline Func constantFunc(Frac x) {
    Func f;
    f.constant = true;
    f.value = x;
    return f;
}

inline Func dependentFunc(Mat m) {
    Func f;
    f.constant = false;
    f.mat = m;
    return f;
}

inline Frac applyFunc(const Func& f, const Frac& x) {
    return f.constant ? f.value : applyMat(f.mat, x);
}

// Bottom-to-top composition used by the descendant DP.
inline Func composeDown(const Func& L, const Func& R) {
    if (L.constant) return L;
    if (R.constant) return constantFunc(applyMat(L.mat, R.value));
    return dependentFunc(matMul(L.mat, R.mat));
}

// Top-to-bottom composition used by the outside-subtree DP.
inline Func composeUp(const Func& L, const Func& R) {
    if (R.constant) return R;
    if (L.constant) return constantFunc(applyMat(R.mat, L.value));
    return dependentFunc(matMul(R.mat, L.mat));
}

struct GNode {
    Func down, up;
};

struct DSU {
    vector<int> p, sz, slot;

    explicit DSU(int n) : p(n), sz(n, 1), slot(n) {
        iota(p.begin(), p.end(), 0);
        iota(slot.begin(), slot.end(), 0);
    }

    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }

    int unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return a;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return a;
    }
};

struct ProductSegTree {
    int base = 1;
    vector<int> t;

    explicit ProductSegTree(int n) {
        while (base < n) base <<= 1;
        t.assign(2 * base, 1);
    }

    void set(int pos, int value) {
        int v = base + pos;
        t[v] = value;
        for (v >>= 1; v; v >>= 1)
            t[v] = mulmod(t[v << 1], t[v << 1 | 1]);
    }

    int total() const {
        return t[1];
    }
};

struct Solver {
    int n, root;
    vector<vector<int>> adj;

    vector<int> parent, heavy, subSize;
    vector<int> hTop, pos, pathEnd;
    vector<vector<int>> lightKids;
    vector<int> lightIdx;

    vector<char> active;
    vector<Frac> topValue;

    // Global HLD segment tree.
    vector<GNode> gt;

    // One small segment tree per vertex for its light children.
    vector<int> lBase, lStart;
    vector<Frac> lightTree;

    explicit Solver(int n_, vector<vector<int>> adj_, int root_)
        : n(n_), root(root_), adj(std::move(adj_)),
          parent(n, -1), heavy(n, -1), subSize(n, 1),
          hTop(n), pos(n), pathEnd(n, -1),
          lightKids(n), lightIdx(n, -1),
          active(n, 0), topValue(n, Frac{0, 1}),
          gt(4 * n), lBase(n), lStart(n) {}

    void buildHLD() {
        vector<int> order;
        order.reserve(n);

        vector<int> st = {root};
        parent[root] = -1;

        while (!st.empty()) {
            int v = st.back();
            st.pop_back();
            order.push_back(v);

            for (int u : adj[v]) {
                if (u == parent[v]) continue;
                parent[u] = v;
                st.push_back(u);
            }
        }

        for (int i = n - 1; i >= 0; --i) {
            int v = order[i];
            int p = parent[v];
            if (p == -1) continue;

            subSize[p] += subSize[v];
            if (heavy[p] == -1 || subSize[v] > subSize[heavy[p]])
                heavy[p] = v;
        }

        for (int v = 0; v < n; ++v) {
            for (int u : adj[v]) {
                if (parent[u] == v && u != heavy[v]) {
                    lightIdx[u] = int(lightKids[v].size());
                    lightKids[v].push_back(u);
                }
            }
        }

        int timer = 0;
        vector<pair<int, int>> stack2;
        stack2.push_back({root, root});

        while (!stack2.empty()) {
            auto [start, top] = stack2.back();
            stack2.pop_back();

            int v = start;
            int last = start;

            while (v != -1) {
                hTop[v] = top;
                pos[v] = timer++;

                for (int u : adj[v]) {
                    if (parent[u] == v && u != heavy[v])
                        stack2.push_back({u, u});
                }

                last = v;
                v = heavy[v];
            }

            pathEnd[top] = pos[last];
        }
    }

    void initLightTrees() {
        int total = 0;

        for (int v = 0; v < n; ++v) {
            int m = int(lightKids[v].size());
            int b = 1;
            while (b < m) b <<= 1;

            lBase[v] = b;
            lStart[v] = total;
            total += 2 * b;
        }

        lightTree.assign(total, Frac{0, 1});
    }

    Frac lightAll(int v) const {
        return lightTree[lStart[v] + 1];
    }

    void lightPointUpdate(int v, int idx, Frac value) {
        int base = lBase[v];
        int start = lStart[v];
        int p = base + idx;

        lightTree[start + p] = value;

        for (p >>= 1; p; p >>= 1) {
            lightTree[start + p] = combine(
                lightTree[start + 2 * p],
                lightTree[start + 2 * p + 1]
            );
        }
    }

    Frac lightRangeQuery(int v, int l, int r) const {
        Frac result{0, 1};
        if (l >= r) return result;

        int base = lBase[v];
        int start = lStart[v];

        l += base;
        r += base;

        while (l < r) {
            if (l & 1) {
                result = combine(result, lightTree[start + l]);
                ++l;
            }
            if (r & 1) {
                --r;
                result = combine(result, lightTree[start + r]);
            }
            l >>= 1;
            r >>= 1;
        }

        return result;
    }

    GNode mergeGNode(const GNode& L, const GNode& R) const {
        return {
            composeDown(L.down, R.down),
            composeUp(L.up, R.up)
        };
    }

    void gBuild(int v, int l, int r) {
        if (l == r) {
            Func z = constantFunc({0, 1});
            gt[v] = {z, z};
            return;
        }

        int m = (l + r) >> 1;
        gBuild(v << 1, l, m);
        gBuild(v << 1 | 1, m + 1, r);
        gt[v] = mergeGNode(gt[v << 1], gt[v << 1 | 1]);
    }

    void gPointUpdate(int v, int l, int r, int at,
                      bool isActive, const Mat& mat) {
        if (l == r) {
            Func f = isActive ? dependentFunc(mat) : constantFunc({0, 1});
            gt[v] = {f, f};
            return;
        }

        int m = (l + r) >> 1;
        if (at <= m)
            gPointUpdate(v << 1, l, m, at, isActive, mat);
        else
            gPointUpdate(v << 1 | 1, m + 1, r, at, isActive, mat);

        gt[v] = mergeGNode(gt[v << 1], gt[v << 1 | 1]);
    }

    GNode gQuery(int v, int l, int r, int ql, int qr) const {
        if (ql <= l && r <= qr) return gt[v];

        int m = (l + r) >> 1;

        if (qr <= m)
            return gQuery(v << 1, l, m, ql, qr);
        if (ql > m)
            return gQuery(v << 1 | 1, m + 1, r, ql, qr);

        return mergeGNode(
            gQuery(v << 1, l, m, ql, qr),
            gQuery(v << 1 | 1, m + 1, r, ql, qr)
        );
    }

    void recomputeVertex(int v) {
        Frac ag = lightAll(v);

        int a = submod(mulmod(int(adj[v].size()), ag.d), ag.n);
        int b = ag.d;

        Mat m{
            0,
            b,
            submod(0, b),
            a
        };

        gPointUpdate(1, 0, n - 1, pos[v], active[v], m);
    }

    Frac computeDown(int v) const {
        GNode result = gQuery(
            1, 0, n - 1,
            pos[v], pathEnd[hTop[v]]
        );

        return applyFunc(result.down, {0, 1});
    }

    void propagate(int v) {
        while (true) {
            int t = hTop[v];

            Frac newValue = computeDown(t);
            Frac oldValue = topValue[t];

            if (newValue.n == oldValue.n &&
                newValue.d == oldValue.d)
                break;

            topValue[t] = newValue;

            int p = parent[t];
            if (p == -1) break;

            lightPointUpdate(p, lightIdx[t], newValue);
            recomputeVertex(p);
            v = p;
        }
    }

    Func lightEdgeFunction(int p, int lightChild) const {
        if (!active[p]) return constantFunc({0, 1});

        int idx = lightIdx[lightChild];
        int m = int(lightKids[p].size());

        Frac lightSum = combine(
            lightRangeQuery(p, 0, idx),
            lightRangeQuery(p, idx + 1, m)
        );

        Frac heavyValue{0, 1};
        int h = heavy[p];

        if (h != -1 && active[h])
            heavyValue = computeDown(h);

        Frac childSum = combine(lightSum, heavyValue);

        int a = submod(mulmod(int(adj[p].size()), childSum.d),
                       childSum.n);
        int b = childSum.d;

        return dependentFunc(Mat{
            0,
            b,
            submod(0, b),
            a
        });
    }

    // Pair (N,D) for the component outside subtree(x),
    // rooted at parent[x]. Called only when that parent is active.
    Frac computeOutside(int x) const {
        vector<Func> operations;
        operations.reserve(64);

        int cur = x;

        while (cur != root) {
            int t = hTop[cur];

            if (cur != t) {
                operations.push_back(
                    gQuery(1, 0, n - 1, pos[t], pos[cur] - 1).up
                );
            }

            if (t == root) break;

            int p = parent[t];
            operations.push_back(lightEdgeFunction(p, t));
            cur = p;
        }

        Frac value{0, 1};

        for (auto it = operations.rbegin(); it != operations.rend(); ++it)
            value = applyFunc(*it, value);

        return value;
    }

    void run(const vector<int>& operations) {
        buildHLD();
        initLightTrees();
        gBuild(1, 0, n - 1);

        DSU dsu(n);
        ProductSegTree determinantProduct(n);

        vector<int> power2(n + 1, 1);
        for (int i = 1; i <= n; ++i)
            power2[i] = addmod(power2[i - 1], power2[i - 1]);

        int exponent = 0;

        struct Item {
            Frac h;
            int componentRoot;
        };

        for (int x : operations) {
            vector<Item> items;
            items.reserve(adj[x].size());

            for (int v : adj[x]) {
                if (!active[v]) continue;

                Frac h;
                if (v == parent[x])
                    h = computeOutside(x);
                else
                    h = computeDown(v);

                items.push_back({h, dsu.find(v)});
            }

            int p = int(items.size());
            int q = int(adj[x].size()) - p;

            // sum = sum_i N_i/D_i, with a common denominator.
            Frac sum{0, 1};
            for (const Item& item : items)
                sum = combine(sum, item.h);

            int newDeterminant = submod(
                mulmod(int(adj[x].size()), sum.d),
                sum.n
            );

            for (const Item& item : items) {
                int oldSlot = dsu.slot[item.componentRoot];
                determinantProduct.set(oldSlot, 1);
            }

            int newRoot = x;
            for (const Item& item : items)
                newRoot = dsu.unite(newRoot, item.componentRoot);

            dsu.slot[newRoot] = x;
            determinantProduct.set(x, newDeterminant);

            exponent += q - 1;

            int answer = mulmod(
                power2[exponent],
                determinantProduct.total()
            );

            cout << answer << '\n';

            active[x] = 1;
            recomputeVertex(x);
            propagate(x);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<vector<int>> adj(n);

    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        --u;
        --v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    if (n == 1) return 0;

    vector<int> operations(n - 1);
    vector<char> appears(n, 0);

    for (int& x : operations) {
        cin >> x;
        --x;
        appears[x] = 1;
    }

    int root = 0;
    while (appears[root]) ++root;

    Solver solver(n, std::move(adj), root);
    solver.run(operations);

    return 0;
}

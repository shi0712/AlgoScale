#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 1000000007;

static inline int addmod(int a, int b) {
    int s = a + b;
    return s >= MOD ? s - MOD : s;
}
static inline int submod(int a, int b) {
    int s = a - b;
    return s < 0 ? s + MOD : s;
}
static inline int mulmod(int a, int b) {
    return int(1LL * a * b % MOD);
}
static inline int negmod(int a) {
    return a ? MOD - a : 0;
}

vector<int> pw2, ipw2;
int invSmall[25];

struct Edge {
    int u, v, w;
};

struct DSU {
    vector<int> p;
    explicit DSU(int n) : p(n) {
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        return p[x] == x ? x : p[x] = find(p[x]);
    }
    void unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a != b) p[b] = a;
    }
};

/*
A product of star-shaped posets:
each coordinate is either zero or one of several incomparable nonzero values.

The convolution splits nonzero coordinates between the two factors.
Ranked zeta transforms give its inverse, logarithm and product.
*/
struct ProductAlgebra {
    int k, N;
    vector<int> radix, stride;
    vector<unsigned char> rank;

    explicit ProductAlgebra(const vector<int>& r) : k((int)r.size()), radix(r) {
        stride.resize(k);
        N = 1;
        for (int i = 0; i < k; ++i) {
            stride[i] = N;
            N *= radix[i];
        }
        rank.assign(N, 0);
        for (int i = 0; i < k; ++i) {
            int s = stride[i], block = s * radix[i];
            for (int base = 0; base < N; base += block) {
                for (int d = 1; d < radix[i]; ++d) {
                    for (int j = 0; j < s; ++j) {
                        rank[base + d * s + j] = rank[base + j] + 1;
                    }
                }
            }
        }
    }

    void zeta(vector<int>& a) const {
        for (int d = 1; d <= k; ++d) {
            int* row = a.data() + size_t(d) * N;
            for (int i = 0; i < k; ++i) {
                int s = stride[i], block = s * radix[i];
                for (int base = 0; base < N; base += block) {
                    for (int digit = 1; digit < radix[i]; ++digit) {
                        int dst = base + digit * s;
                        for (int j = 0; j < s; ++j) {
                            int x = dst + j;
                            if (rank[x] >= d)
                                row[x] = addmod(row[x], row[base + j]);
                        }
                    }
                }
            }
        }
    }

    void mobius(vector<int>& a) const {
        for (int d = 1; d <= k; ++d) {
            int* row = a.data() + size_t(d) * N;
            for (int i = 0; i < k; ++i) {
                int s = stride[i], block = s * radix[i];
                for (int base = 0; base < N; base += block) {
                    for (int digit = 1; digit < radix[i]; ++digit) {
                        int dst = base + digit * s;
                        for (int j = 0; j < s; ++j) {
                            int x = dst + j;
                            if (rank[x] <= d)
                                row[x] = submod(row[x], row[base + j]);
                        }
                    }
                }
            }
        }
    }

    vector<int> ranked(const vector<int>& f) const {
        vector<int> a(size_t(k + 1) * N, 0);
        fill(a.begin(), a.begin() + N, f[0]);
        for (int s = 1; s < N; ++s)
            a[size_t(rank[s]) * N + s] = f[s];
        zeta(a);
        return a;
    }

    // mode = 0: convolution inverse; mode = 1: convolution logarithm.
    // Requires f[0] = 1.
    vector<int> functionOf(const vector<int>& f, int mode) const {
        vector<int> a = ranked(f);
        int x[25], y[25], dy[25];

        for (int s = 0; s < N; ++s) {
            for (int j = 0; j <= k; ++j)
                x[j] = a[size_t(j) * N + s];

            if (mode == 0) {
                y[0] = 1;
                for (int j = 1; j <= k; ++j) {
                    uint64_t sum = 0;
                    int cnt = 0;
                    for (int i = 1; i <= j; ++i) {
                        sum += uint64_t(x[i]) * y[j - i];
                        if (++cnt == 16) {
                            sum %= MOD;
                            cnt = 0;
                        }
                    }
                    y[j] = negmod(int(sum % MOD));
                }
            } else {
                y[0] = 0;
                dy[0] = 0;
                for (int j = 1; j <= k; ++j) {
                    uint64_t sum = 0;
                    int cnt = 0;
                    for (int i = 1; i < j; ++i) {
                        sum += uint64_t(dy[i]) * x[j - i];
                        if (++cnt == 16) {
                            sum %= MOD;
                            cnt = 0;
                        }
                    }
                    y[j] = submod(x[j], mulmod(int(sum % MOD), invSmall[j]));
                    dy[j] = mulmod(j, y[j]);
                }
            }

            for (int j = 0; j <= k; ++j)
                a[size_t(j) * N + s] = y[j];
        }

        mobius(a);
        vector<int> ans(N);
        for (int s = 0; s < N; ++s)
            ans[s] = a[size_t(rank[s]) * N + s];
        return ans;
    }

    vector<int> convolution(const vector<int>& f, const vector<int>& g) const {
        vector<int> a = ranked(f);
        vector<int> b = ranked(g);
        int x[25], y[25];

        for (int s = 0; s < N; ++s) {
            for (int j = 0; j <= k; ++j) {
                x[j] = a[size_t(j) * N + s];
                y[j] = b[size_t(j) * N + s];
            }
            for (int j = 0; j <= k; ++j) {
                uint64_t sum = 0;
                int cnt = 0;
                for (int i = 0; i <= j; ++i) {
                    sum += uint64_t(x[i]) * y[j - i];
                    if (++cnt == 16) {
                        sum %= MOD;
                        cnt = 0;
                    }
                }
                a[size_t(j) * N + s] = int(sum % MOD);
            }
        }

        vector<int>().swap(b);
        mobius(a);
        vector<int> ans(N);
        for (int s = 0; s < N; ++s)
            ans[s] = a[size_t(rank[s]) * N + s];
        return ans;
    }
};

struct Node {
    uint32_t vertices = 0;
    vector<int> children;
    vector<pair<int, int>> edges;
    vector<pair<uint32_t, int>> distribution;
};

struct Child {
    uint32_t vertices;
    vector<uint32_t> roots; // option zero is not stored
    vector<int> probability;
    int total;
    bool ordinary;
};

class Solver {
    int n;
    vector<Node> nodes;

    static int arcCount(uint32_t sources, uint32_t targets,
                        const vector<uint32_t>& adj) {
        int ans = 0;
        while (targets) {
            int v = __builtin_ctz(targets);
            targets &= targets - 1;
            ans += __builtin_popcount(adj[v] & sources);
        }
        return ans;
    }

    vector<int> calculateE(
        const vector<Child>& child,
        const ProductAlgebra& alg,
        const vector<uint32_t>& roots,
        const vector<uint32_t>& support,
        const vector<uint32_t>& full,
        const vector<uint32_t>& adj,
        int ordinaryCount
    ) {
        int k = alg.k, N = alg.N, r = ordinaryCount;

        long double directWork = 1;
        for (const auto& c : child)
            directWork *= 1 + 2 * (int)c.roots.size();

        int M = 1 << r;
        int P = N / M;

        long double specialSplits = 1;
        for (int i = r; i < k; ++i)
            specialSplits *= 1 + 2 * (int)child[i].roots.size();

        long double fastWork =
            (specialSplits + P) * max(1, r * r) * (long double)M * 2.0L;

        bool useFast = (r == k) || (r >= 12 && fastWork < directWork);

        if (!useFast) {
            vector<int> E(N);
            E[0] = 1;

            int offset[20], exponent[20];

            for (int state = 1; state < N; ++state) {
                uint32_t supp = support[state];
                uint32_t vertices = full[supp];
                int p = 0;

                for (int i = 0; i < k; ++i) {
                    int digit = state / alg.stride[i] % alg.radix[i];
                    if (!digit) continue;
                    offset[p] = digit * alg.stride[i];
                    exponent[p] = arcCount(
                        vertices, child[i].roots[digit - 1], adj
                    );
                    ++p;
                }

                int remaining = state, exp = 0;
                unsigned gray = 0;
                uint64_t sum = 0;
                int cnt = 0;

                for (unsigned j = 1; j < (1u << p); ++j) {
                    int b = __builtin_ctz(j);
                    unsigned bit = 1u << b;
                    if (gray & bit) {
                        remaining += offset[b];
                        exp -= exponent[b];
                    } else {
                        remaining -= offset[b];
                        exp += exponent[b];
                    }
                    gray ^= bit;

                    sum += uint64_t(E[remaining]) * pw2[exp];
                    if (++cnt == 16) {
                        sum %= MOD;
                        cnt = 0;
                    }
                }
                E[state] = negmod(int(sum % MOD));
            }
            return E;
        }

        vector<int> binaryRadix(r, 2);
        ProductAlgebra bin(binaryRadix);

        vector<uint32_t> ordinaryVertices(M, 0);
        for (int x = 1; x < M; ++x) {
            int b = __builtin_ctz(x);
            ordinaryVertices[x] =
                ordinaryVertices[x & (x - 1)] | child[b].vertices;
        }

        vector<int> a(M), edgeNumber(M, 0);
        a[0] = 1;
        for (int x = 1; x < M; ++x) {
            int b = __builtin_ctz(x);
            int y = x & (x - 1);
            edgeNumber[x] = edgeNumber[y] +
                arcCount(ordinaryVertices[y], child[b].vertices, adj);
            a[x] = pw2[edgeNumber[x]];
        }

        vector<int> ainv = bin.functionOf(a, 0);

        if (r == k) {
            vector<int> E(N);
            for (int x = 0; x < N; ++x)
                E[x] = mulmod(a[x], ainv[x]);
            return E;
        }

        /*
        For special state z:
          e1[z,X] = arcs from ordinary X to the roots of z,
          e2[z,X] = arcs from all vertices of z's children to ordinary X.
        Both counts are at most m.
        */
        vector<unsigned short> e1(N, 0), e2(N, 0);
        vector<uint32_t> specialFull(P), specialRoot(P);

        for (int z = 0; z < P; ++z) {
            int state = z * M;
            specialRoot[z] = roots[state];
            specialFull[z] = full[support[state]];

            int c1[20], c2[20];
            for (int i = 0; i < r; ++i) {
                c1[i] = arcCount(child[i].vertices, specialRoot[z], adj);
                c2[i] = arcCount(specialFull[z], child[i].vertices, adj);
            }
            for (int x = 1; x < M; ++x) {
                int b = __builtin_ctz(x);
                int y = x & (x - 1);
                e1[state + x] = e1[state + y] + c1[b];
                e2[state + x] = e2[state + y] + c2[b];
            }
        }

        // H_z = (a * 2^{e1[z,.]}) convolution ainv.
        vector<int> H(N, 0);
        H[0] = 1;
        vector<int> f(M);

        for (int z = 1; z < P; ++z) {
            int base = z * M;
            for (int x = 0; x < M; ++x)
                f[x] = mulmod(a[x], pw2[e1[base + x]]);
            vector<int> h = bin.convolution(f, ainv);
            copy(h.begin(), h.end(), H.begin() + base);
        }

        // T_z = E_z / a on the ordinary coordinates.
        vector<int> T(N, 0);
        copy(ainv.begin(), ainv.end(), T.begin());

        vector<int> specialOffset(k - r);
        vector<int> accum(M), h(M);

        for (int z = 1; z < P; ++z) {
            int base = z * M;
            int p = 0;
            for (int i = r; i < k; ++i) {
                int digit = base / alg.stride[i] % alg.radix[i];
                if (digit)
                    specialOffset[p++] = digit * (alg.stride[i] / M);
            }

            fill(accum.begin(), accum.end(), 0);

            int bstate = 0;
            unsigned gray = 0;
            for (unsigned j = 1; j < (1u << p); ++j) {
                int bit = __builtin_ctz(j);
                unsigned flag = 1u << bit;
                if (gray & flag)
                    bstate -= specialOffset[bit];
                else
                    bstate += specialOffset[bit];
                gray ^= flag;

                int astate = z - bstate;
                int abase = astate * M;
                int bbase = bstate * M;

                for (int x = 0; x < M; ++x) {
                    int factor = mulmod(
                        pw2[e1[bbase + x]],
                        ipw2[e2[base + x]]
                    );
                    f[x] = mulmod(T[abase + x], factor);
                    h[x] = H[bbase + x];
                }

                vector<int> term = bin.convolution(f, h);
                int coefficient = pw2[
                    arcCount(specialFull[z], specialRoot[bstate], adj)
                ];
                for (int x = 0; x < M; ++x)
                    accum[x] = addmod(
                        accum[x], mulmod(coefficient, term[x])
                    );
            }

            for (int x = 0; x < M; ++x)
                T[base + x] =
                    negmod(mulmod(pw2[e2[base + x]], accum[x]));
        }

        for (int z = 0; z < P; ++z)
            for (int x = 0; x < M; ++x)
                T[z * M + x] = mulmod(T[z * M + x], a[x]);

        return T;
    }

    void solveNode(int id) {
        Node& node = nodes[id];
        if (node.children.empty()) {
            node.distribution = {{node.vertices, 1}};
            return;
        }

        for (int v : node.children) solveNode(v);

        vector<Child> child;
        for (int v : node.children) {
            if (nodes[v].distribution.empty()) {
                node.distribution.clear();
                return;
            }

            Child c;
            c.vertices = nodes[v].vertices;
            c.total = 0;

            for (auto [roots, prob] : nodes[v].distribution) {
                if (!prob) continue;
                c.roots.push_back(roots);
                c.probability.push_back(prob);
                c.total = addmod(c.total, prob);
            }

            if (c.roots.empty()) {
                node.distribution.clear();
                return;
            }

            c.ordinary =
                c.roots.size() == 1 && c.roots[0] == c.vertices;
            child.push_back(move(c));
        }

        stable_sort(child.begin(), child.end(),
                    [](const Child& a, const Child& b) {
                        return a.ordinary > b.ordinary;
                    });

        int k = (int)child.size(), ordinaryCount = 0;
        while (ordinaryCount < k && child[ordinaryCount].ordinary)
            ++ordinaryCount;

        vector<int> radix(k);
        for (int i = 0; i < k; ++i)
            radix[i] = (int)child[i].roots.size() + 1;

        ProductAlgebra alg(radix);
        int N = alg.N;

        vector<uint32_t> adj(n, 0);
        for (auto [u, v] : node.edges) {
            adj[u] |= 1u << v;
            adj[v] |= 1u << u;
        }

        vector<uint32_t> full(1 << k, 0);
        for (int s = 1; s < (1 << k); ++s) {
            int b = __builtin_ctz(s);
            full[s] = full[s & (s - 1)] | child[b].vertices;
        }

        vector<uint32_t> roots(N, 0), support(N, 0);
        vector<int> h(N, 1);

        for (int i = 0; i < k; ++i) {
            int stride = alg.stride[i];
            int block = stride * radix[i];

            for (int base = 0; base < N; base += block) {
                for (int digit = 1; digit < radix[i]; ++digit) {
                    uint32_t rr = child[i].roots[digit - 1];
                    int incoming = arcCount(node.vertices, rr, adj);
                    int factor = mulmod(
                        child[i].probability[digit - 1],
                        ipw2[incoming]
                    );

                    for (int j = 0; j < stride; ++j) {
                        int src = base + j;
                        int dst = base + digit * stride + j;
                        roots[dst] = roots[src] | rr;
                        support[dst] = support[src] | (1u << i);
                        h[dst] = mulmod(h[src], factor);
                    }
                }
            }
        }

        vector<int> E = calculateE(
            child, alg, roots, support, full, adj, ordinaryCount
        );

        // K is the number of strongly connected effective digraphs.
        vector<int> K = alg.functionOf(E, 1);
        for (int s = 1; s < N; ++s) K[s] = negmod(K[s]);

        vector<int> W(1 << k, 0);
        for (int s = 0; s < N; ++s)
            W[support[s]] = addmod(
                W[support[s]], mulmod(E[s], h[s])
            );

        // Weighted subset zeta transform.
        for (int i = 0; i < k; ++i) {
            for (int mask = 0; mask < (1 << k); ++mask) {
                if (mask & (1 << i))
                    W[mask] = addmod(
                        W[mask],
                        mulmod(W[mask ^ (1 << i)], child[i].total)
                    );
            }
        }

        int all = (1 << k) - 1;
        node.distribution.clear();
        for (int s = 1; s < N; ++s) {
            int prob = mulmod(
                mulmod(K[s], h[s]), W[all ^ support[s]]
            );
            if (prob) node.distribution.push_back({roots[s], prob});
        }
    }

public:
    int run(int n_, vector<Edge> edges) {
        n = n_;
        nodes.clear();
        nodes.reserve(2 * n);

        for (int i = 0; i < n; ++i) {
            Node leaf;
            leaf.vertices = 1u << i;
            nodes.push_back(move(leaf));
        }

        sort(edges.begin(), edges.end(),
             [](const Edge& a, const Edge& b) {
                 return a.w < b.w;
             });

        DSU dsu(n);
        vector<int> componentNode(n);
        iota(componentNode.begin(), componentNode.end(), 0);

        for (int l = 0; l < (int)edges.size();) {
            int r = l;
            while (r < (int)edges.size() && edges[r].w == edges[l].w) ++r;

            vector<int> old(n);
            for (int i = 0; i < n; ++i) old[i] = dsu.find(i);

            for (int i = l; i < r; ++i)
                dsu.unite(edges[i].u, edges[i].v);

            vector<vector<int>> groups(n);
            for (int i = 0; i < n; ++i)
                if (old[i] == i)
                    groups[dsu.find(i)].push_back(i);

            for (int g = 0; g < n; ++g) {
                if (groups[g].size() <= 1) continue;

                Node parent;
                for (int oldRoot : groups[g]) {
                    int ch = componentNode[oldRoot];
                    parent.children.push_back(ch);
                    parent.vertices |= nodes[ch].vertices;
                }

                for (int i = l; i < r; ++i) {
                    const auto& e = edges[i];
                    if (old[e.u] != old[e.v] && dsu.find(e.u) == g)
                        parent.edges.push_back({e.u, e.v});
                }

                componentNode[g] = (int)nodes.size();
                nodes.push_back(move(parent));
            }

            l = r;
        }

        int root = componentNode[dsu.find(0)];
        solveNode(root);

        int answer = 0;
        for (auto [mask, p] : nodes[root].distribution)
            answer = addmod(answer, p);
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    invSmall[1] = 1;
    for (int i = 2; i <= 20; ++i)
        invSmall[i] = MOD - mulmod(MOD / i, invSmall[MOD % i]);

    int c, T;
    cin >> c >> T;

    while (T--) {
        int n, m;
        cin >> n >> m;

        vector<Edge> edges(m);
        for (auto& e : edges) {
            cin >> e.u >> e.v >> e.w;
            --e.u;
            --e.v;
        }

        pw2.assign(2 * m + 1, 1);
        ipw2.assign(2 * m + 1, 1);
        const int inv2 = (MOD + 1) / 2;
        for (int i = 1; i <= 2 * m; ++i) {
            pw2[i] = addmod(pw2[i - 1], pw2[i - 1]);
            ipw2[i] = mulmod(ipw2[i - 1], inv2);
        }

        Solver solver;
        cout << solver.run(n, move(edges)) << '\n';
    }

    return 0;
}

#include <bits/stdc++.h>
#include <wmmintrin.h>
using namespace std;

using u32 = uint32_t;
using u64 = uint64_t;

__attribute__((target("pclmul")))
static inline u64 clmul64(u64 a, u64 b) {
    if (__builtin_cpu_supports("pclmul")) {
        __m128i x = _mm_cvtsi64_si128((long long)a);
        __m128i y = _mm_cvtsi64_si128((long long)b);
        __m128i z = _mm_clmulepi64_si128(x, y, 0);
        return (u64)_mm_cvtsi128_si64(z);
    }
    u64 r = 0;
    while (b) {
        if (b & 1) r ^= a;
        a <<= 1;
        b >>= 1;
    }
    return r;
}

struct Field {
    static inline u32 mul(u32 a, u32 b) {
        u64 p = clmul64(a, b);
        u32 lo = (u32)p;
        u32 hi = (u32)(p >> 32);
        u64 q = clmul64(hi, 0xafu);
        return lo ^ (u32)q ^ (u32)clmul64(q >> 32, 0xafu);
    }
    static inline u32 sq(u32 a) { return mul(a, a); }
    static u32 power(u32 a, u64 e) {
        u32 r = 1;
        while (e) {
            if (e & 1) r = mul(r, a);
            a = sq(a);
            e >>= 1;
        }
        return r;
    }
    static inline u32 inv(u32 a) { return power(a, 0xfffffffeULL); }
};

struct RNG {
    u64 state;
    explicit RNG(u64 seed) : state(seed) {}
    u64 next() {
        u64 z = (state += 0x9e3779b97f4a7c15ULL);
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        return z ^ (z >> 31);
    }
    u32 nonzero() {
        u32 x;
        do x = (u32)next(); while (!x);
        return x;
    }
};

struct Matrix {
    int n;
    explicit Matrix(int n_) : n(n_) {}
    virtual ~Matrix() = default;
    virtual void apply(const vector<u32>& x, vector<u32>& y) const = 0;
};

static vector<u32> berlekampMassey(const vector<u32>& s) {
    vector<u32> c{1}, b{1};
    int L = 0, shift = 1;
    u32 last = 1;
    for (int n = 0; n < (int)s.size(); ++n) {
        u32 d = s[n];
        for (int i = 1; i <= L; ++i) d ^= Field::mul(c[i], s[n - i]);
        if (!d) {
            ++shift;
            continue;
        }
        u32 coef = Field::mul(d, Field::inv(last));
        if (2 * L <= n) {
            vector<u32> old = c;
            if ((int)c.size() < (int)b.size() + shift) c.resize(b.size() + shift, 0);
            for (int i = 0; i < (int)b.size(); ++i)
                c[i + shift] ^= Field::mul(coef, b[i]);
            L = n + 1 - L;
            b.swap(old);
            last = d;
            shift = 1;
        } else {
            if ((int)c.size() < (int)b.size() + shift) c.resize(b.size() + shift, 0);
            for (int i = 0; i < (int)b.size(); ++i)
                c[i + shift] ^= Field::mul(coef, b[i]);
            ++shift;
        }
    }
    c.resize(L + 1);
    return c;
}

static u32 determinant(const Matrix& a, RNG& rng) {
    int n = a.n;
    if (!n) return 1;
    vector<u32> diag(n), x(n), y(n), u(n), seq;
    seq.reserve(2 * n);
    for (int attempt = 0; attempt < 3; ++attempt) {
        u32 detDiag = 1;
        for (int i = 0; i < n; ++i) {
            diag[i] = rng.nonzero();
            detDiag = Field::mul(detDiag, diag[i]);
            x[i] = rng.nonzero();
            u[i] = rng.nonzero();
        }
        seq.clear();
        for (int it = 0; it < 2 * n; ++it) {
            u32 dot = 0;
            for (int i = 0; i < n; ++i) dot ^= Field::mul(u[i], x[i]);
            seq.push_back(dot);
            for (int i = 0; i < n; ++i) y[i] = Field::mul(diag[i], x[i]);
            a.apply(y, x);
        }
        vector<u32> poly = berlekampMassey(seq);
        if ((int)poly.size() == n + 1) {
            return Field::mul(poly[n], Field::inv(detDiag));
        }
    }
    return 0;
}

struct Edge { int au, av, bu, bv; };

struct PrimalMatrix : Matrix {
    const vector<Edge>& edges;
    const vector<u32>& weight;
    int vertices, rootA, rootB;
    bool secondA;
    PrimalMatrix(const vector<Edge>& e, const vector<u32>& w, int n, bool same)
        : Matrix(n - 1), edges(e), weight(w), vertices(n), rootA(n - 1), rootB(n - 1), secondA(same) {}
    void apply(const vector<u32>& x, vector<u32>& y) const override {
        fill(y.begin(), y.end(), 0);
        for (int i = 0; i < (int)edges.size(); ++i) {
            const Edge& e = edges[i];
            int u = secondA ? e.au : e.bu;
            int v = secondA ? e.av : e.bv;
            u32 z = 0;
            if (u != rootB) z ^= x[u];
            if (v != rootB) z ^= x[v];
            z = Field::mul(z, weight[i]);
            if (e.au != rootA) y[e.au] ^= z;
            if (e.av != rootA) y[e.av] ^= z;
        }
    }
};

struct TreeRepresentation {
    int n, m, q;
    const vector<Edge>& edges;
    bool useA;
    vector<int> row, nonTree, parent, parentEdge, order;
    TreeRepresentation(const vector<Edge>& e, int n_, bool a)
        : n(n_), m(e.size()), q(0), edges(e), useA(a), row(m, -1), parent(n, -1), parentEdge(n, -1) {
        vector<int> dsu(n);
        iota(dsu.begin(), dsu.end(), 0);
        auto find = [&](int x) {
            while (dsu[x] != x) {
                dsu[x] = dsu[dsu[x]];
                x = dsu[x];
            }
            return x;
        };
        vector<vector<pair<int,int>>> adj(n);
        vector<char> inTree(m, false);
        for (int i = 0; i < m; ++i) {
            int u = useA ? edges[i].au : edges[i].bu;
            int v = useA ? edges[i].av : edges[i].bv;
            u = find(u); v = find(v);
            if (u != v) {
                dsu[u] = v;
                inTree[i] = true;
                int x = useA ? edges[i].au : edges[i].bu;
                int z = useA ? edges[i].av : edges[i].bv;
                adj[x].push_back({z, i});
                adj[z].push_back({x, i});
            }
        }
        order.push_back(0);
        parent[0] = 0;
        for (int at = 0; at < (int)order.size(); ++at) {
            int u = order[at];
            for (auto [v, id] : adj[u]) if (v != parent[u]) {
                parent[v] = u;
                parentEdge[v] = id;
                order.push_back(v);
            }
        }
        assert((int)order.size() == n);
        for (int i = 0; i < m; ++i) if (!inTree[i]) {
            row[i] = q++;
            nonTree.push_back(i);
        }
    }
    void transposeApply(const vector<u32>& x, vector<u32>& out, vector<u32>& diff) const {
        fill(out.begin(), out.end(), 0);
        fill(diff.begin(), diff.end(), 0);
        for (int i = 0; i < q; ++i) {
            int id = nonTree[i];
            const Edge& e = edges[id];
            int u = useA ? e.au : e.bu;
            int v = useA ? e.av : e.bv;
            out[id] = x[i];
            diff[u] ^= x[i];
            diff[v] ^= x[i];
        }
        for (int at = n - 1; at >= 1; --at) {
            int v = order[at];
            u32 z = diff[v];
            out[parentEdge[v]] = z;
            diff[parent[v]] ^= z;
        }
    }
    void pathPrefix(const vector<u32>& z, vector<u32>& prefix) const {
        prefix[0] = 0;
        for (int at = 1; at < n; ++at) {
            int v = order[at];
            prefix[v] = prefix[parent[v]] ^ z[parentEdge[v]];
        }
    }
};

struct DualMatrix : Matrix {
    const vector<Edge>& edges;
    const vector<u32>& weight;
    const TreeRepresentation& first;
    const TreeRepresentation& second;
    mutable vector<u32> temp, diff, prefix;
    DualMatrix(const vector<Edge>& e, const vector<u32>& w,
               const TreeRepresentation& a, const TreeRepresentation& b)
        : Matrix(a.q), edges(e), weight(w), first(a), second(b),
          temp(e.size()), diff(a.n), prefix(a.n) {}
    void apply(const vector<u32>& x, vector<u32>& y) const override {
        second.transposeApply(x, temp, diff);
        for (int i = 0; i < (int)edges.size(); ++i) temp[i] = Field::mul(temp[i], weight[i]);
        first.pathPrefix(temp, prefix);
        for (int i = 0; i < first.q; ++i) {
            int id = first.nonTree[i];
            const Edge& e = edges[id];
            y[i] = temp[id] ^ prefix[e.au] ^ prefix[e.av];
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    cin >> T;
    RNG rng(0x123456789abcdef0ULL);
    while (T--) {
        int n, m;
        cin >> n >> m;
        vector<Edge> edges(m);
        for (auto& e : edges) {
            cin >> e.au >> e.av >> e.bu >> e.bv;
            --e.au; --e.av; --e.bu; --e.bv;
        }
        vector<u32> weight(m);
        for (u32& w : weight) w = rng.nonzero();
        int r = n - 1, q = m - r;
        u32 da, dc;
        if (r <= q) {
            PrimalMatrix aa(edges, weight, n, true);
            PrimalMatrix ab(edges, weight, n, false);
            da = determinant(aa, rng);
            dc = determinant(ab, rng);
        } else {
            vector<u32> invWeight(m);
            for (int i = 0; i < m; ++i) invWeight[i] = Field::inv(weight[i]);
            TreeRepresentation a(edges, n, true), b(edges, n, false);
            DualMatrix aa(edges, invWeight, a, a);
            DualMatrix ab(edges, invWeight, a, b);
            da = determinant(aa, rng);
            dc = determinant(ab, rng);
        }
        cout << (da != dc ? "YES" : "NO") << '\n';
    }
    return 0;
}

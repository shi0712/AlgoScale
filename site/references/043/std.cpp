#include <bits/stdc++.h>
using namespace std;

using int64 = long long;
static constexpr int MAXK = 16;
static constexpr int64 INF64 = (1LL << 62);

class FastScanner {
    static constexpr size_t BUFSIZE = 1 << 20;
    char buf[BUFSIZE];
    size_t idx = 0, size = 0;
public:
    inline char gc() {
        if (idx >= size) {
            size = fread(buf, 1, BUFSIZE, stdin);
            idx = 0;
            if (size == 0) return 0;
        }
        return buf[idx++];
    }
    template<class T>
    bool readInt(T &out) {
        char c;
        do {
            c = gc();
            if (!c) return false;
        } while (c <= ' ');
        T x = 0;
        do {
            x = x * 10 + (c - '0');
            c = gc();
        } while (c >= '0' && c <= '9');
        out = x;
        return true;
    }
};

struct Edge {
    int u, v, w;
    bool operator<(const Edge &o) const { return w < o.w; }
};

struct DSU {
    vector<int> p;
    explicit DSU(int n = 0): p(n) { iota(p.begin(), p.end(), 0); }
    int find(int x) {
        int r = x;
        while (p[r] != r) r = p[r];
        while (p[x] != x) {
            int y = p[x];
            p[x] = r;
            x = y;
        }
        return r;
    }
};

int main() {
    FastScanner fs;
    int n, m, k;
    if (!fs.readInt(n)) return 0;
    fs.readInt(m); fs.readInt(k);

    vector<Edge> edges(m);
    for (auto &e : edges) {
        fs.readInt(e.u); fs.readInt(e.v); fs.readInt(e.w);
        --e.u; --e.v;
    }

    sort(edges.begin(), edges.end());

    const int maxNodes = 2 * n + 5;
    DSU dsu(maxNodes);
    vector<int> lc(maxNodes, -1), rc(maxNodes, -1), par(maxNodes, -1), val(maxNodes, 0);

    int tot = n;
    int used = 0;
    int64 baseMST = 0;
    for (const auto &e : edges) {
        int x = dsu.find(e.u), y = dsu.find(e.v);
        if (x == y) continue;
        int z = tot++;
        lc[z] = x; rc[z] = y; val[z] = e.w;
        par[x] = par[y] = z;
        dsu.p[x] = dsu.p[y] = z;
        dsu.p[z] = z;
        baseMST += e.w;
        if (++used == n - 1) break;
    }

    if (n == 1) {
        // For n=1 the original cities are already mutually connected.  All costs are nonnegative.
        // Still consume the remaining input so malformed benchmark pipelines cannot hide input cost.
        for (int j = 0; j < k; ++j) {
            int64 c; fs.readInt(c);
            for (int i = 0, x; i < n; ++i) fs.readInt(x);
        }
        puts("0");
        return 0;
    }

    if (used != n - 1) return 0; // Input validator guarantees connectedness.

    int root = dsu.find(0);

    vector<array<int, MAXK>> mn(tot);
    vector<int64> cityCost(k);
    for (int j = 0; j < k; ++j) {
        fs.readInt(cityCost[j]);
        for (int v = 0; v < n; ++v) fs.readInt(mn[v][j]);
    }

    if (k == 0) {
        printf("%lld\n", baseMST);
        return 0;
    }

    for (int x = n; x < tot; ++x) {
        for (int j = 0; j < k; ++j)
            mn[x][j] = min(mn[lc[x]][j], mn[rc[x]][j]);
    }

    int64 dist[MAXK][MAXK]{};
    for (int i = 0; i < k; ++i)
        for (int j = 0; j < k; ++j)
            dist[i][j] = (i == j ? 0 : INF64);

    for (int x = 0; x < tot; ++x) {
        const int vx = val[x];
        for (int i = 0; i < k; ++i) {
            for (int j = i + 1; j < k; ++j) {
                int64 cand = max<int64>(vx, max(mn[x][i], mn[x][j]));
                if (cand < dist[i][j]) dist[i][j] = dist[j][i] = cand;
            }
        }
    }

    const int SZ = 1 << k;
    const int ALL = SZ - 1;
    vector<int64> f(SZ, 0);

    int rootEnd = 0;
    for (int j = 0; j < k; ++j) rootEnd = max(rootEnd, mn[root][j]);

    array<pair<int,int>, MAXK> ord;
    for (int x = 0; x < tot; ++x) {
        int64 L = val[x];
        int64 R = (x == root ? rootEnd : val[par[x]]);
        if (R <= L) continue;

        for (int j = 0; j < k; ++j) ord[j] = {mn[x][j], j};
        sort(ord.begin(), ord.begin() + k);

        int mask = 0, p = 0;
        while (p < k && ord[p].first <= L) {
            mask |= 1 << ord[p].second;
            ++p;
        }
        int64 cur = L;
        while (p < k && ord[p].first < R) {
            int64 nxt = ord[p].first;
            f[mask] += nxt - cur;
            while (p < k && ord[p].first == nxt) {
                mask |= 1 << ord[p].second;
                ++p;
            }
            cur = nxt;
        }
        f[mask] += R - cur;
    }

    vector<int64> sub = f;
    for (int bit = 0; bit < k; ++bit)
        for (int mask = 0; mask < SZ; ++mask)
            if (mask & (1 << bit)) sub[mask] += sub[mask ^ (1 << bit)];

    vector<int64> activate(SZ, 0);
    for (int mask = 1; mask < SZ; ++mask) {
        int b = __builtin_ctz((unsigned)mask);
        activate[mask] = activate[mask ^ (1 << b)] + cityCost[b];
    }

    vector<int64> townMST(SZ, 0);
    int64 bestDist[MAXK];
    unsigned char vis[MAXK];

    for (int mask = 1; mask < SZ; ++mask) {
        if ((mask & (mask - 1)) == 0) continue;
        for (int i = 0; i < k; ++i) {
            bestDist[i] = INF64;
            vis[i] = 0;
        }
        int s = __builtin_ctz((unsigned)mask);
        bestDist[s] = 0;
        int need = __builtin_popcount((unsigned)mask);
        int64 sum = 0;
        for (int step = 0; step < need; ++step) {
            int v = -1;
            for (int i = 0; i < k; ++i) if ((mask >> i & 1) && !vis[i]) {
                if (v == -1 || bestDist[i] < bestDist[v]) v = i;
            }
            vis[v] = 1;
            sum += bestDist[v];
            for (int u = 0; u < k; ++u) if ((mask >> u & 1) && !vis[u])
                if (dist[v][u] < bestDist[u]) bestDist[u] = dist[v][u];
        }
        townMST[mask] = sum;
    }

    int64 ans = baseMST;
    for (int mask = 1; mask < SZ; ++mask) {
        int64 cur = activate[mask] + sub[ALL ^ mask] + townMST[mask];
        if (cur < ans) ans = cur;
    }

    printf("%lld\n", ans);
    return 0;
}

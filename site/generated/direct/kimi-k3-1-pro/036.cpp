#include <bits/stdc++.h>
using namespace std;

struct FastInput {
    static constexpr int BUFSIZE = 1 << 20;
    char buf[BUFSIZE];
    int pos = 0, len = 0;

    inline char getChar() {
        if (pos == len) {
            len = (int)fread(buf, 1, BUFSIZE, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buf[pos++];
    }

    int nextInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ');
        int x = 0;
        while (c > ' ') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
};

struct DSU {
    vector<int> p, sz;

    explicit DSU(int n) : p(n + 1), sz(n + 1, 1) {
        iota(p.begin(), p.end(), 0);
    }

    int find(int x) {
        while (p[x] != x) {
            p[x] = p[p[x]];
            x = p[x];
        }
        return x;
    }

    bool join(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return true;
    }
};

struct Edge {
    int w, u, v;
};

struct Event {
    int w, u, v;
    // v > 0: original-MST edge u-v
    // v < 0: town edge, town index = -v-1, city = u
};

int main() {
    FastInput in;

    int n = in.nextInt();
    int m = in.nextInt();
    int k = in.nextInt();

    vector<Edge> roads(m);
    for (auto &e : roads) {
        e.u = in.nextInt();
        e.v = in.nextInt();
        e.w = in.nextInt();
    }

    sort(roads.begin(), roads.end(),
         [](const Edge &a, const Edge &b) { return a.w < b.w; });

    DSU initialDSU(n);
    vector<Edge> baseTree;
    baseTree.reserve(max(0, n - 1));

    for (const auto &e : roads) {
        if (initialDSU.join(e.u, e.v)) {
            baseTree.push_back(e);
            if ((int)baseTree.size() == n - 1) break;
        }
    }
    vector<Edge>().swap(roads);

    vector<Event> events;
    events.reserve(baseTree.size() + (size_t)n * k);

    for (const auto &e : baseTree) {
        events.push_back({e.w, e.u, e.v});
    }

    vector<long long> activation(k);
    for (int j = 0; j < k; ++j) {
        activation[j] = in.nextInt();
        for (int city = 1; city <= n; ++city) {
            int a = in.nextInt();
            events.push_back({a, city, -j - 1});
        }
    }

    sort(events.begin(), events.end(),
         [](const Event &a, const Event &b) { return a.w < b.w; });

    const int W = events.empty() ? 0 : events.back().w;
    const int MASKS = 1 << k;
    const int FULL = MASKS - 1;
    const int INF = numeric_limits<int>::max();

    vector<long long> life(MASKS, 0);
    int d[16][16];
    for (int i = 0; i < k; ++i) {
        for (int j = 0; j < k; ++j) {
            d[i][j] = INF;
        }
    }

    DSU dsu(n);
    vector<int> componentMask(n + 1, 0);
    vector<int> segmentStart(n + 1, 0);

    auto closeSegment = [&](int root, int weight) {
        life[componentMask[root]] +=
            (long long)weight - segmentStart[root];
    };

    for (const auto &e : events) {
        if (e.v > 0) {
            int ra = dsu.find(e.u);
            int rb = dsu.find(e.v);

            int ma = componentMask[ra];
            int mb = componentMask[rb];

            closeSegment(ra, e.w);
            closeSegment(rb, e.w);

            // Pairs inside ma or inside mb already appeared earlier.
            for (int a = 0; a < k; ++a) {
                if (!(ma & (1 << a))) continue;
                for (int b = 0; b < k; ++b) {
                    if (!(mb & (1 << b)) || a == b) continue;
                    if (d[a][b] == INF) {
                        d[a][b] = d[b][a] = e.w;
                    }
                }
            }

            if (dsu.sz[ra] < dsu.sz[rb]) swap(ra, rb);
            dsu.p[rb] = ra;
            dsu.sz[ra] += dsu.sz[rb];
            componentMask[ra] = ma | mb;
            segmentStart[ra] = e.w;
        } else {
            int town = -e.v - 1;
            int root = dsu.find(e.u);
            int bit = 1 << town;

            if (!(componentMask[root] & bit)) {
                int oldMask = componentMask[root];
                closeSegment(root, e.w);

                for (int other = 0; other < k; ++other) {
                    if (!(oldMask & (1 << other))) continue;
                    if (d[town][other] == INF) {
                        d[town][other] = d[other][town] = e.w;
                    }
                }

                componentMask[root] = oldMask | bit;
                segmentStart[root] = e.w;
            }
        }
    }

    int root = dsu.find(1);
    closeSegment(root, W);

    // subset[mask] = sum of life[p] over p subset of mask
    vector<long long> subset = std::move(life);
    for (int b = 0; b < k; ++b) {
        int bit = 1 << b;
        for (int mask = 0; mask < MASKS; ++mask) {
            if (mask & bit) {
                subset[mask] += subset[mask ^ bit];
            }
        }
    }

    vector<long long> activationSum(MASKS, 0);

    long long answer = subset[FULL] - (long long)W; // no towns

    for (int S = 1; S < MASKS; ++S) {
        int b = __builtin_ctz((unsigned)S);
        activationSum[S] =
            activationSum[S ^ (1 << b)] + activation[b];

        long long townTreeCost = 0;

        if (S & (S - 1)) {
            // Prim on the complete graph induced by S.
            int vertices[16], q = 0;
            for (int v = 0; v < k; ++v) {
                if (S & (1 << v)) vertices[q++] = v;
            }

            long long best[16];
            bool used[16] = {};
            for (int i = 0; i < q; ++i) {
                best[i] = (i == 0 ? 0 : LLONG_MAX);
            }

            for (int it = 0; it < q; ++it) {
                int take = -1;
                for (int i = 0; i < q; ++i) {
                    if (!used[i] &&
                        (take == -1 || best[i] < best[take])) {
                        take = i;
                    }
                }

                used[take] = true;
                townTreeCost += best[take];

                int u = vertices[take];
                for (int i = 0; i < q; ++i) {
                    if (!used[i]) {
                        best[i] = min(best[i],
                                      (long long)d[u][vertices[i]]);
                    }
                }
            }
        }

        long long z = subset[FULL ^ S];
        answer = min(answer,
                     activationSum[S] + z + townTreeCost);
    }

    printf("%lld\n", answer);
    return 0;
}

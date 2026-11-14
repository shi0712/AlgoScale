#include <bits/stdc++.h>
using namespace std;

using int64 = long long;

class FastInput {
    static constexpr int SZ = 1 << 20;
    char buf[SZ];
    int pos = 0, len = 0;

    char getChar() {
        if (pos == len) {
            len = (int)fread(buf, 1, SZ, stdin);
            pos = 0;
            if (!len) return 0;
        }
        return buf[pos++];
    }

public:
    void skipToken() {
        char c;
        do c = getChar(); while (c && c <= ' ');
        while (c > ' ') c = getChar();
    }

    int nextInt() {
        char c;
        do c = getChar(); while (c && c <= ' ');
        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = getChar();
        }
        return x;
    }
};

struct Block {
    int lo, hi;
    int type;
};

class Solver {
    int n, LOG, root;
    int typeCount = 0;

    vector<int> adjStart, adj;
    vector<int> parent, depth, height, order;
    vector<int> childStart, children;
    vector<int> type, typeParent, typeHeight;
    vector<int> heavy, head, rankValue;
    vector<int64> rankPrefix;
    vector<vector<int>> up;

    // LCA structure for the tree of canonical subtree types.
    vector<int> typeDfn, typeFirst, rmqLog;
    vector<vector<int>> rmq;

    vector<Block> fullA, fullB, temporaryBlocks;

    int ancestor(int u, int distance) const {
        int bit = 0;
        while (distance) {
            if (distance & 1) u = up[bit][u];
            distance >>= 1;
            ++bit;
        }
        return u;
    }

    int vertexAtDepth(int u, int wantedDepth) const {
        return ancestor(u, depth[u] - wantedDepth);
    }

    int lca(int u, int v) const {
        if (depth[u] < depth[v]) swap(u, v);
        u = ancestor(u, depth[u] - depth[v]);
        if (u == v) return u;
        for (int k = LOG - 1; k >= 0; --k) {
            if (up[k][u] != up[k][v]) {
                u = up[k][u];
                v = up[k][v];
            }
        }
        return parent[u];
    }

    int typeLca(int u, int v) const {
        int l = typeFirst[u], r = typeFirst[v];
        if (l > r) swap(l, r);
        int k = rmqLog[r - l + 1];
        int a = rmq[k][l];
        int b = rmq[k][r - (1 << k) + 1];
        return typeHeight[a] < typeHeight[b] ? a : b;
    }

    /*
       Number of vertices v on (top, bottom] with rankValue[v] < bound.
       rankValue strictly increases down every root-to-leaf path.
    */
    int legLess(int bottom, int top, int bound) const {
        if (bottom == top || rankValue[top] >= bound) return 0;
        if (rankValue[bottom] < bound)
            return depth[bottom] - depth[top];

        int u = bottom;
        for (int k = LOG - 1; k >= 0; --k) {
            int v = up[k][u];
            if (v && rankValue[v] >= bound) u = v;
        }
        int lastGood = parent[u];
        return max(0, depth[lastGood] - depth[top]);
    }

    /*
       The target leg is (top, targetBottom].
       Its first k vertices already have rankValue <= r.

       Return the number of its initial vertices satisfying
           rankValue[y] + length(root leg)
                        - #(root-leg vertices with rankValue < rankValue[y])
           <= r.
    */
    int allRootsCutoff(
        int rootBottom, int top, int targetBottom, int k, int r
    ) const {
        if (k == 0) return 0;
        int a = depth[rootBottom] - depth[top];
        if (a == 0) return k;

        int w = vertexAtDepth(targetBottom, depth[top] + k);

        auto bad = [&](int v) {
            return rankValue[v] + a
                   - legLess(rootBottom, top, rankValue[v]) > r;
        };

        if (!bad(w)) return k;

        int u = w;
        for (int j = LOG - 1; j >= 0; --j) {
            int v = up[j][u];
            if (v && depth[v] > depth[top] && bad(v)) u = v;
        }
        return depth[u] - depth[top] - 1;
    }

    /*
       Decompose the ancestor path whose depths are [lowDepth, depth[bottom]]
       into maximal-product-child chain intervals.

       Blocks are returned in increasing subtree-height order.
    */
    void makeBlocks(int bottom, int lowDepth, vector<Block>& result) const {
        result.clear();

        while (depth[bottom] >= lowDepth) {
            int h = head[bottom];
            int topDepth = max(lowDepth, depth[h]);

            result.push_back({
                height[bottom],
                height[bottom] + depth[bottom] - topDepth,
                type[h]
            });

            if (topDepth > depth[h]) break;
            bottom = parent[h];
            if (!bottom) break;
        }
    }

    /*
       Count pairs (u,v), u in A and v in B, for which
           rankValue[u] < rankValue[v],
       equivalently product[u] > product[v].

       Each block contains exactly one vertex at each height in [lo,hi].
    */
    int64 inversionCount(
        const vector<Block>& A, const vector<Block>& B
    ) const {
        if (A.empty() || B.empty()) return 0;

        int j = 0;
        int bpos = B[0].lo;
        int64 below = 0;
        int64 answer = 0;

        for (const Block& x : A) {
            int cur = x.lo;

            while (cur <= x.hi) {
                if (j == (int)B.size()) {
                    answer += int64(x.hi - cur + 1) * below;
                    break;
                }

                const Block& y = B[j];

                if (bpos < cur) {
                    int last = min(y.hi, cur - 1);
                    below += last - bpos + 1;
                    bpos = last + 1;
                    if (bpos > y.hi) {
                        ++j;
                        if (j < (int)B.size()) bpos = B[j].lo;
                    }
                    continue;
                }

                if (bpos > cur) {
                    int last = min(x.hi, bpos - 1);
                    answer += int64(last - cur + 1) * below;
                    cur = last + 1;
                    continue;
                }

                // Both paths contain one vertex at every height here.
                int last = min(x.hi, y.hi);
                int64 len = last - cur + 1;

                answer += len * below + len * (len - 1) / 2;

                // Resolve comparisons at equal heights.
                if (typeDfn[x.type] > typeDfn[y.type]) {
                    int common = typeHeight[typeLca(x.type, y.type)];
                    int firstUnequal = max(cur, common + 1);
                    if (firstUnequal <= last)
                        answer += last - firstUnequal + 1;
                }

                below += len;
                cur = last + 1;
                bpos = last + 1;

                if (bpos > y.hi) {
                    ++j;
                    if (j < (int)B.size()) bpos = B[j].lo;
                }
            }
        }

        return answer;
    }

    int64 crossCount(
        int rootBottom, int targetBottom, int top,
        int targetLowCount, int cutoff, int r,
        const vector<Block>& rootBlocks
    ) {
        int rootLength = depth[rootBottom] - depth[top];

        int64 answer = int64(rootLength) * cutoff;
        if (cutoff == targetLowCount) return answer;

        int deep = vertexAtDepth(
            targetBottom, depth[top] + targetLowCount
        );
        int before = vertexAtDepth(
            targetBottom, depth[top] + cutoff
        );

        int count = targetLowCount - cutoff;
        int64 sumRanks = rankPrefix[deep] - rankPrefix[before];

        answer += int64(count) * r - sumRanks;

        makeBlocks(deep, depth[top] + cutoff + 1, temporaryBlocks);
        answer += inversionCount(rootBlocks, temporaryBlocks);
        return answer;
    }

    static int64 ancestorPairs(int length, int r) {
        // Sum_{i=1..length} min(i,r).
        if (length <= r)
            return int64(length) * (length + 1) / 2;
        return int64(length) * r - int64(r) * (r - 1) / 2;
    }

    void buildCanonicalTypes() {
        height.assign(n + 1, 0);
        int maximumHeight = 0;

        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            if (parent[u]) {
                height[parent[u]] =
                    max(height[parent[u]], height[u] + 1);
            }
            maximumHeight = max(maximumHeight, height[u]);
        }

        vector<int> heightStart(maximumHeight + 2, 0);
        for (int u = 1; u <= n; ++u)
            ++heightStart[height[u] + 1];
        for (int h = 1; h < (int)heightStart.size(); ++h)
            heightStart[h] += heightStart[h - 1];

        vector<int> cursor = heightStart;
        vector<int> byHeight(n);
        for (int u = 1; u <= n; ++u)
            byHeight[cursor[height[u]]++] = u;

        type.assign(n + 1, 0);
        typeParent.assign(n + 1, 0);
        typeHeight.assign(n + 1, 0);
        heavy.assign(n + 1, 0);

        auto compareVertices = [&](int u, int v) {
            int i = childStart[u], ie = childStart[u + 1];
            int j = childStart[v], je = childStart[v + 1];

            while (i < ie && j < je) {
                int a = type[children[i]];
                int b = type[children[j]];
                if (a != b) return a < b;
                ++i;
                ++j;
            }
            return ie - i < je - j;
        };

        auto equalVertices = [&](int u, int v) {
            int a = childStart[u + 1] - childStart[u];
            int b = childStart[v + 1] - childStart[v];
            if (a != b) return false;
            for (int i = 0; i < a; ++i) {
                if (type[children[childStart[u] + i]] !=
                    type[children[childStart[v] + i]])
                    return false;
            }
            return true;
        };

        for (int h = 0; h <= maximumHeight; ++h) {
            int begin = heightStart[h];
            int end = heightStart[h + 1];

            for (int i = begin; i < end; ++i) {
                int u = byHeight[i];
                sort(
                    children.begin() + childStart[u],
                    children.begin() + childStart[u + 1],
                    [&](int a, int b) {
                        return type[a] > type[b];
                    }
                );
                if (childStart[u] != childStart[u + 1])
                    heavy[u] = children[childStart[u]];
            }

            sort(
                byHeight.begin() + begin,
                byHeight.begin() + end,
                compareVertices
            );

            int previous = 0;
            for (int i = begin; i < end; ++i) {
                int u = byHeight[i];

                if (!previous || !equalVertices(previous, u)) {
                    ++typeCount;
                    typeParent[typeCount] =
                        heavy[u] ? type[heavy[u]] : 0;
                    typeHeight[typeCount] = h;
                }

                type[u] = typeCount;
                previous = u;
            }
        }

        typeParent.resize(typeCount + 1);
        typeHeight.resize(typeCount + 1);

        vector<int> frequency(typeCount + 1, 0);
        for (int u = 1; u <= n; ++u) ++frequency[type[u]];

        vector<int> rankOfType(typeCount + 1);
        int larger = 0;
        for (int t = typeCount; t >= 1; --t) {
            rankOfType[t] = larger + 1;
            larger += frequency[t];
        }

        rankValue.resize(n + 1);
        for (int u = 1; u <= n; ++u)
            rankValue[u] = rankOfType[type[u]];

        head.assign(n + 1, 0);
        rankPrefix.assign(n + 1, 0);

        for (int u : order) {
            if (!parent[u] || heavy[parent[u]] != u)
                head[u] = u;
            else
                head[u] = head[parent[u]];

            rankPrefix[u] = rankPrefix[parent[u]] + rankValue[u];
        }
    }

    void buildTypeLca() {
        vector<int> start(typeCount + 2, 0);
        for (int t = 2; t <= typeCount; ++t)
            ++start[typeParent[t] + 1];
        for (int i = 1; i < (int)start.size(); ++i)
            start[i] += start[i - 1];

        vector<int> cursor = start;
        vector<int> edges(max(0, typeCount - 1));

        // Inserting in increasing type order also sorts siblings.
        for (int t = 2; t <= typeCount; ++t)
            edges[cursor[typeParent[t]]++] = t;

        typeDfn.assign(typeCount + 1, 0);
        typeFirst.assign(typeCount + 1, 0);

        vector<int> euler;
        euler.reserve(2 * typeCount - 1);
        vector<int> stack;
        stack.reserve(typeCount);
        cursor = start;

        int timer = 0;
        stack.push_back(1);
        typeDfn[1] = timer++;
        typeFirst[1] = 0;
        euler.push_back(1);

        while (!stack.empty()) {
            int u = stack.back();
            if (cursor[u] < start[u + 1]) {
                int v = edges[cursor[u]++];
                typeDfn[v] = timer++;
                typeFirst[v] = (int)euler.size();
                euler.push_back(v);
                stack.push_back(v);
            } else {
                stack.pop_back();
                if (!stack.empty()) euler.push_back(stack.back());
            }
        }

        int length = (int)euler.size();
        rmqLog.assign(length + 1, 0);
        for (int i = 2; i <= length; ++i)
            rmqLog[i] = rmqLog[i / 2] + 1;

        int levels = rmqLog[length] + 1;
        rmq.resize(levels);
        rmq[0] = move(euler);

        for (int k = 1; k < levels; ++k) {
            int size = length - (1 << k) + 1;
            rmq[k].resize(size);
            int half = 1 << (k - 1);

            for (int i = 0; i < size; ++i) {
                int a = rmq[k - 1][i];
                int b = rmq[k - 1][i + half];
                rmq[k][i] =
                    typeHeight[a] < typeHeight[b] ? a : b;
            }
        }
    }

public:
    explicit Solver(int size) : n(size) {}

    void readTree(FastInput& input) {
        vector<int> eu(n - 1), ev(n - 1);
        vector<int> degree(n + 1, 0);

        for (int i = 0; i < n - 1; ++i) {
            eu[i] = input.nextInt();
            ev[i] = input.nextInt();
            ++degree[eu[i]];
            ++degree[ev[i]];
        }

        adjStart.assign(n + 2, 0);
        for (int u = 1; u <= n; ++u)
            adjStart[u + 1] = adjStart[u] + degree[u];

        adj.resize(2 * (n - 1));
        vector<int> cursor = adjStart;
        for (int i = 0; i < n - 1; ++i) {
            adj[cursor[eu[i]]++] = ev[i];
            adj[cursor[ev[i]]++] = eu[i];
        }

        // Any tree center is suitable.
        vector<int> queue(n);
        int qhead = 0, qtail = 0;
        for (int u = 1; u <= n; ++u)
            if (degree[u] == 1) queue[qtail++] = u;

        int remaining = n;
        while (remaining > 2) {
            int layer = qtail - qhead;
            remaining -= layer;
            while (layer--) {
                int u = queue[qhead++];
                for (int i = adjStart[u]; i < adjStart[u + 1]; ++i) {
                    int v = adj[i];
                    if (--degree[v] == 1) queue[qtail++] = v;
                }
            }
        }
        root = queue[qhead];

        parent.assign(n + 1, 0);
        depth.assign(n + 1, 0);
        order.reserve(n);
        order.push_back(root);

        for (int i = 0; i < n; ++i) {
            int u = order[i];
            for (int j = adjStart[u]; j < adjStart[u + 1]; ++j) {
                int v = adj[j];
                if (v == parent[u]) continue;
                parent[v] = u;
                depth[v] = depth[u] + 1;
                order.push_back(v);
            }
        }

        childStart.assign(n + 2, 0);
        for (int u = 1; u <= n; ++u) {
            int count = adjStart[u + 1] - adjStart[u];
            if (parent[u]) --count;
            childStart[u + 1] = childStart[u] + count;
        }

        children.resize(n - 1);
        cursor = childStart;
        for (int u = 1; u <= n; ++u)
            if (parent[u]) children[cursor[parent[u]]++] = u;

        LOG = 1;
        while ((1 << LOG) <= n) ++LOG;
        up.assign(LOG, vector<int>(n + 1, 0));
        up[0] = parent;
        for (int k = 1; k < LOG; ++k)
            for (int u = 1; u <= n; ++u)
                up[k][u] = up[k - 1][up[k - 1][u]];

        buildCanonicalTypes();
        buildTypeLca();

        fullA.reserve(1024);
        fullB.reserve(1024);
        temporaryBlocks.reserve(1024);
    }

    int64 query(int s, int t, int ox, int oy, int r) {
        int z = lca(s, t);
        int a = depth[s] - depth[z];
        int b = depth[t] - depth[z];

        if (ox == 0 && oy == 0) {
            int f;
            if (z == t) {
                f = depth[s] - depth[t] + 1;
            } else {
                f = rankValue[t] + depth[s]
                    - legLess(s, root, rankValue[t]);
            }
            return f <= r;
        }

        if (ox == 0 && oy == 1) {
            int k = legLess(t, z, r + 1);
            int p = allRootsCutoff(s, z, t, k, r);
            return min(a + 1, r) + p;
        }

        if (ox == 1 && oy == 0) {
            if (t == z) return min(a + 1, r);

            int64 answer = 1; // x = y = t.
            if (rankValue[t] <= r) {
                answer += b;
                int prefix = legLess(s, z, rankValue[t]);
                answer += min(a, prefix + r - rankValue[t]);
            }
            return answer;
        }

        int kA = legLess(s, z, r + 1);
        int kB = legLess(t, z, r + 1);

        int64 answer = 1; // (z,z).

        answer += ancestorPairs(a, r);
        answer += ancestorPairs(b, r);
        answer += int64(kA) * (kA - 1) / 2;
        answer += int64(kB) * (kB - 1) / 2;

        answer += min(a, r - 1) + kA;
        answer += min(b, r - 1) + kB;

        if (a == 0 || b == 0) return answer;

        int pA = allRootsCutoff(t, z, s, kA, r);
        int pB = allRootsCutoff(s, z, t, kB, r);

        if (pB < kB)
            makeBlocks(s, depth[z] + 1, fullA);
        else
            fullA.clear();

        if (pA < kA)
            makeBlocks(t, depth[z] + 1, fullB);
        else
            fullB.clear();

        answer += crossCount(s, t, z, kB, pB, r, fullA);
        answer += crossCount(t, s, z, kA, pA, r, fullB);

        return answer;
    }
};

int main() {
    FastInput input;

    // c is an arbitrarily long, unused decimal integer.
    input.skipToken();

    int n = input.nextInt();
    Solver solver(n);
    solver.readTree(input);

    int m = input.nextInt();
    string output;
    output.reserve(size_t(m) * 14);

    while (m--) {
        int s = input.nextInt();
        int t = input.nextInt();
        int ox = input.nextInt();
        int oy = input.nextInt();
        int r = input.nextInt();

        output += to_string(solver.query(s, t, ox, oy, r));
        output.push_back('\n');
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}

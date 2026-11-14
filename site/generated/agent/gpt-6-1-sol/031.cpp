#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

// For the processed set P and its complement U, the answer is
//     2^(components(T[U])-1) * det(L[P,P]).
// Splitting the two copies of each processed vertex into symmetric and
// antisymmetric coordinates proves this identity: the symmetric block is
// a weighted tree, and the antisymmetric block is twice L[P,P].
//
// For a rooted subtree let A be its principal minor, and let B be that
// minor with the root removed (B=0 when the root is unprocessed).
// If (p,q) is the product of the children's pairs under
// (A,B)*(C,D)=(AC,AD+BC), then (A,B)=(degree*p-q,p) for a processed
// root, and (A,B)=(p,0) otherwise. Heavy paths turn this into matrix
// products. Weight-balanced products of paths and light subtrees give
// O(log n) update time and O(n) space, without modular division.

struct FastInput {
    static const int S = 1 << 16;
    char buf[S];
    int pos = 0, len = 0;
    int get() {
        if (pos == len) {
            len = (int)fread(buf, 1, S, stdin);
            pos = 0;
            if (!len) return -1;
        }
        return buf[pos++];
    }
    int nextInt() {
        int c, x = 0;
        do { c = get(); } while (c <= ' ' && c != -1);
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = get();
        }
        return x;
    }
};

struct Node {
    int a = 1, b = 0, c = 0, d = 0;
    int left = 0, right = 0, parent = 0;
    // 0: inactive vertex, 1: active vertex, 2: matrix product,
    // 3: product of pairs (A,B), with (A,B)*(C,D)=(AC,AD+BC).
    int type = 0, degree = 0;
};

class Solver {
    int n;
    vector<vector<int>> adj;
    vector<int> parent, sz, heavy, vertexNode;
    vector<Node> nodes;

    void pull(int id) {
        Node &z = nodes[id];
        if (z.type <= 1) {
            int p = 1, q = 0;
            if (z.left) {
                p = nodes[z.left].a;
                q = nodes[z.left].c;
            }
            z.d = 0;
            if (z.type == 0) {
                z.a = p;
                z.b = z.c = 0;
            } else {
                z.a = ((long long)z.degree * p - q + MOD) % MOD;
                z.b = p ? MOD - p : 0;
                z.c = p;
            }
        } else {
            const Node &x = nodes[z.left], &y = nodes[z.right];
            if (z.type == 2) {
                z.a = ((uint64_t)x.a * y.a + (uint64_t)x.b * y.c) % MOD;
                z.b = ((uint64_t)x.a * y.b + (uint64_t)x.b * y.d) % MOD;
                z.c = ((uint64_t)x.c * y.a + (uint64_t)x.d * y.c) % MOD;
                z.d = ((uint64_t)x.c * y.b + (uint64_t)x.d * y.d) % MOD;
            } else {
                z.a = (uint64_t)x.a * y.a % MOD;
                z.c = ((uint64_t)x.a * y.c + (uint64_t)x.c * y.a) % MOD;
            }
        }
    }

    int join(int left, int right, int type) {
        int id = (int)nodes.size();
        nodes.emplace_back();
        nodes[id].left = left;
        nodes[id].right = right;
        nodes[id].type = type;
        nodes[left].parent = nodes[right].parent = id;
        pull(id);
        return id;
    }

    // Isolate the item containing the midpoint of the total weight.
    // Both remaining intervals have at most half the weight. This makes
    // the entire computation tree logarithmic, even across light edges.
    int balanced(const vector<int> &items, const vector<int> &prefix,
                 int l, int r, int type) {
        if (r - l == 1) return items[l];
        int target = (prefix[l] + prefix[r]) / 2;
        int mid = int(upper_bound(prefix.begin() + l, prefix.begin() + r,
                                  target) - prefix.begin()) - 1;
        int result = items[mid];
        if (l < mid) result = join(balanced(items, prefix, l, mid, type), result, type);
        if (mid + 1 < r) result = join(result, balanced(items, prefix, mid + 1, r, type), type);
        return result;
    }

    int buildPath(int head) {
        vector<int> pathNodes, pathPrefix(1, 0);
        for (int v = head; v; v = heavy[v]) {
            vector<int> lightNodes, lightPrefix(1, 0);
            for (int u : adj[v]) {
                if (parent[u] == v && u != heavy[v]) {
                    lightNodes.push_back(buildPath(u));
                    lightPrefix.push_back(lightPrefix.back() + sz[u]);
                }
            }
            int lightRoot = 0;
            if (!lightNodes.empty()) {
                lightRoot = balanced(lightNodes, lightPrefix, 0, (int)lightNodes.size(), 3);
            }
            int id = (int)nodes.size();
            nodes.emplace_back();
            nodes[id].left = lightRoot;
            nodes[id].degree = (int)adj[v].size();
            if (lightRoot) nodes[lightRoot].parent = id;
            vertexNode[v] = id;
            pull(id);
            pathNodes.push_back(id);
            pathPrefix.push_back(pathPrefix.back() + sz[v] - sz[heavy[v]]);
        }
        return balanced(pathNodes, pathPrefix, 0, (int)pathNodes.size(), 2);
    }

public:
    void run() {
        FastInput in;
        n = in.nextInt();
        if (n <= 1) return;
        adj.resize(n + 1);
        for (int i = 1; i < n; ++i) {
            int u = in.nextInt(), v = in.nextInt();
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
        vector<int> sequence(n - 1);
        vector<char> active(n + 1, false);
        int root = 0;
        for (int v = 1; v <= n; ++v) root ^= v;
        for (int &v : sequence) {
            v = in.nextInt();
            root ^= v;
        }

        parent.assign(n + 1, 0);
        sz.assign(n + 1, 0);
        heavy.assign(n + 1, 0);
        vertexNode.resize(n + 1);
        vector<int> order;
        order.reserve(n);
        order.push_back(root);
        for (int i = 0; i < n; ++i) {
            int v = order[i];
            for (int u : adj[v]) if (u != parent[v]) {
                parent[u] = v;
                order.push_back(u);
            }
        }
        for (int i = n - 1; i >= 0; --i) {
            int v = order[i];
            sz[v] = 1;
            for (int u : adj[v]) if (parent[u] == v) {
                sz[v] += sz[u];
                if (sz[u] > sz[heavy[v]]) heavy[v] = u;
            }
        }
        nodes.reserve(2 * n + 1);
        nodes.emplace_back();
        int expressionRoot = buildPath(root);

        vector<int> pow2(n + 1, 1);
        for (int i = 1; i <= n; ++i) pow2[i] = (2LL * pow2[i - 1]) % MOD;
        int exponent = 0;
        string output;
        output.reserve(11 * n);
        for (int v : sequence) {
            // Removing v from the forest induced by inactive vertices
            // changes its number of components by inactive_degree(v)-1.
            int inactiveDegree = 0;
            for (int u : adj[v]) inactiveDegree += !active[u];
            exponent += inactiveDegree - 1;
            active[v] = true;

            int id = vertexNode[v];
            nodes[id].type = 1;
            for (; id; id = nodes[id].parent) pull(id);

            int answer = (long long)pow2[exponent] * nodes[expressionRoot].a % MOD;
            output += to_string(answer);
            output += '\n';
        }
        fwrite(output.data(), 1, output.size(), stdout);
    }
};

int main() {
    Solver solver;
    solver.run();
    return 0;
}

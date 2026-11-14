#include <bits/stdc++.h>
using namespace std;

static constexpr int MOD = 998244353;

class FastInput {
    static constexpr int SZ = 1 << 16;
    char buf[SZ];
    int pos = 0, len = 0;

    char getChar() {
        if (pos == len) {
            len = (int)fread(buf, 1, SZ, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buf[pos++];
    }

public:
    int readInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ' && c);

        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + c - '0';
            c = getChar();
        }
        return x;
    }
};

class Solver {
    // Every expression stores either a matrix, or a pair in (a, c).
    struct Node {
        int a = 0, b = 0, c = 0, d = 0;
        int left = 0, right = 0, parent = 0;
        int type = 0, vertex = 0;
    };

    // Node types:
    // 0: construct the matrix of one original vertex
    // 1: multiply two matrices
    // 2: combine two light-child aggregates
    int n;
    vector<vector<int>> graph;
    vector<int> parent, subtree, heavy, vertexNode;
    vector<unsigned char> active;
    vector<Node> nodes;
    int expressionRoot = 0;

    void recalculate(int id) {
        Node &z = nodes[id];

        if (z.type == 0) {
            int A = 1, B = 0;
            if (z.left) {
                A = nodes[z.left].a;
                B = nodes[z.left].c;
            }

            if (!active[z.vertex]) {
                z.a = A;
                z.b = z.c = z.d = 0;
            } else {
                int value = (long long)graph[z.vertex].size() * A % MOD;
                value -= B;
                if (value < 0) value += MOD;

                z.a = value;
                z.b = A ? MOD - A : 0;
                z.c = A;
                z.d = 0;
            }
        } else if (z.type == 1) {
            const Node &x = nodes[z.left];
            const Node &y = nodes[z.right];

            z.a = ((long long)x.a * y.a + (long long)x.b * y.c) % MOD;
            z.b = ((long long)x.a * y.b + (long long)x.b * y.d) % MOD;
            z.c = ((long long)x.c * y.a + (long long)x.d * y.c) % MOD;
            z.d = ((long long)x.c * y.b + (long long)x.d * y.d) % MOD;
        } else {
            const Node &x = nodes[z.left];
            const Node &y = nodes[z.right];

            z.a = (long long)x.a * y.a % MOD;
            z.c = ((long long)x.c * y.a + (long long)x.a * y.c) % MOD;
            z.b = z.d = 0;
        }
    }

    int makeNode(int type, int left, int right, int vertex = 0) {
        int id = (int)nodes.size();
        nodes.push_back(Node{});
        nodes[id].type = type;
        nodes[id].left = left;
        nodes[id].right = right;
        nodes[id].vertex = vertex;

        if (left) nodes[left].parent = id;
        if (right) nodes[right].parent = id;

        recalculate(id);
        return id;
    }

    // Build an order-preserving, weight-balanced product tree.
    int buildWeighted(const vector<int> &ids, const vector<int> &prefix,
                      int l, int r, int type) {
        if (l == r) return ids[l];

        int total = prefix[r + 1] - prefix[l];
        int target = prefix[l] + (total + 1) / 2;

        int pivot = int(lower_bound(prefix.begin() + l + 1,
                                    prefix.begin() + r + 2,
                                    target) - prefix.begin()) - 1;

        int result = ids[pivot];

        if (l < pivot) {
            int left = buildWeighted(ids, prefix, l, pivot - 1, type);
            result = makeNode(type, left, result);
        }
        if (pivot < r) {
            int right = buildWeighted(ids, prefix, pivot + 1, r, type);
            result = makeNode(type, result, right);
        }

        return result;
    }

    int buildProduct(const vector<int> &ids, const vector<int> &weights,
                     int type) {
        if (ids.empty()) return 0;
        if (ids.size() == 1) return ids[0];

        vector<int> prefix(ids.size() + 1, 0);
        for (int i = 0; i < (int)ids.size(); ++i) {
            prefix[i + 1] = prefix[i] + weights[i];
        }

        return buildWeighted(ids, prefix, 0, (int)ids.size() - 1, type);
    }

    int buildChain(int head) {
        vector<int> chainNodes;
        vector<int> chainWeights;

        for (int u = head; u; u = heavy[u]) {
            vector<int> lightRoots;
            vector<int> lightWeights;

            for (int v : graph[u]) {
                if (parent[v] != u || v == heavy[u]) continue;

                lightRoots.push_back(buildChain(v));
                lightWeights.push_back(subtree[v]);
            }

            int lightAggregate = buildProduct(lightRoots, lightWeights, 2);
            int id = makeNode(0, lightAggregate, 0, u);
            vertexNode[u] = id;

            chainNodes.push_back(id);
            chainWeights.push_back(subtree[u] - subtree[heavy[u]]);
        }

        return buildProduct(chainNodes, chainWeights, 1);
    }

public:
    explicit Solver(int n)
        : n(n), graph(n + 1), parent(n + 1), subtree(n + 1),
          heavy(n + 1), vertexNode(n + 1), active(n + 1, 0) {
        nodes.reserve(3 * n + 5);
        nodes.push_back(Node{}); // Null node.
    }

    void addEdge(int u, int v) {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    void initialize(int root) {
        vector<int> order;
        order.reserve(n);
        order.push_back(root);

        for (int i = 0; i < (int)order.size(); ++i) {
            int u = order[i];
            for (int v : graph[u]) {
                if (v == parent[u]) continue;
                parent[v] = u;
                order.push_back(v);
            }
        }

        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            subtree[u] = 1;

            for (int v : graph[u]) {
                if (parent[v] != u) continue;
                subtree[u] += subtree[v];
                if (subtree[v] > subtree[heavy[u]]) {
                    heavy[u] = v;
                }
            }
        }

        expressionRoot = buildChain(root);
    }

    // Returns the change in the exponent of 2.
    int activate(int u) {
        int activeNeighbors = 0;
        for (int v : graph[u]) {
            activeNeighbors += active[v];
        }

        int exponentChange =
            (int)graph[u].size() - activeNeighbors - 1;

        active[u] = 1;

        int id = vertexNode[u];
        while (id) {
            int oldA = nodes[id].a;
            int oldB = nodes[id].b;
            int oldC = nodes[id].c;
            int oldD = nodes[id].d;

            recalculate(id);

            if (nodes[id].a == oldA && nodes[id].b == oldB &&
                nodes[id].c == oldC && nodes[id].d == oldD) {
                break;
            }

            id = nodes[id].parent;
        }

        return exponentChange;
    }

    int determinant() const {
        return nodes[expressionRoot].a;
    }
};

int main() {
    FastInput in;
    int n = in.readInt();

    if (n <= 1) return 0;

    Solver solver(n);

    for (int i = 0; i < n - 1; ++i) {
        int u = in.readInt();
        int v = in.readInt();
        solver.addEdge(u, v);
    }

    vector<int> sequence(n - 1);
    vector<unsigned char> scheduled(n + 1, 0);

    for (int &x : sequence) {
        x = in.readInt();
        scheduled[x] = 1;
    }

    int root = 1;
    while (scheduled[root]) ++root;

    solver.initialize(root);

    vector<int> powerOfTwo(n + 1, 1);
    for (int i = 1; i <= n; ++i) {
        powerOfTwo[i] = 2LL * powerOfTwo[i - 1] % MOD;
    }

    int exponent = 0;
    string output;
    output.reserve((size_t)n * 11);

    for (int x : sequence) {
        exponent += solver.activate(x);
        int answer =
            (long long)solver.determinant() * powerOfTwo[exponent] % MOD;
        output += to_string(answer);
        output.push_back('\n');
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}

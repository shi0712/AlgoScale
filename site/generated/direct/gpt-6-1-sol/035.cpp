#include <bits/stdc++.h>
using namespace std;

/*
At a vertex, introduce one port for every incident edge and one extra
"self" port.

A local routing is a single cycle of these ports:
    self -> first edge -> second edge -> ... -> last edge -> self.

While fixing routes, the already fixed transitions form disjoint chains.
An input port may be connected to any unused output port except the start
of its own chain, unless this closes the final remaining chain.

A link-cut tree maintains the minimum destination reachable under these
local restrictions.
*/

class Solver {
    static constexpr int INF = 1'000'000'000;

    struct Edge {
        int to;
        int portHere;
        int portThere;
    };

    struct Aggregate {
        int downMin = INF;
        int upMin = INF;
        bool downPass = false;
        bool upPass = false;
    };

    struct Node {
        int ch[2] = {0, 0};
        int fa = 0;
        int top = 0;

        Aggregate local;
        Aggregate aggregate;
    };

    int n;
    vector<vector<Edge>> graph;

    vector<int> parent, depth;
    vector<int> parentPort, childPort;
    vector<int> preferredPort;

    // DSU for the chains of ports at each original vertex.
    vector<int> dsu, dsuSize, chainStart;
    vector<int> componentCount;
    vector<unsigned char> inputActive, outputActive;

    vector<Node> tree;

    // One entry per virtual child:
    // (minimum reachable destination, port leading to this child).
    vector<multiset<pair<int, int>>> virtualChildren;

    int findSet(int x) {
        int r = x;
        while (dsu[r] != r) r = dsu[r];
        while (dsu[x] != x) {
            int p = dsu[x];
            dsu[x] = r;
            x = p;
        }
        return r;
    }

    int forbiddenOutput(int v, int inputPort) {
        if (componentCount[v] <= 1) return 0;
        return chainStart[findSet(inputPort)];
    }

    int localMinimum(int v, int forbidden) {
        int result = INF;

        if (outputActive[v] && forbidden != v) {
            result = v;
        }

        auto it = virtualChildren[v].begin();
        if (it != virtualChildren[v].end() && it->second == forbidden) {
            ++it;
        }
        if (it != virtualChildren[v].end()) {
            result = min(result, it->first);
        }

        return result;
    }

    static Aggregate combine(const Aggregate& a, const Aggregate& b) {
        Aggregate result;
        result.downMin = min(a.downMin, a.downPass ? b.downMin : INF);
        result.downPass = a.downPass && b.downPass;

        result.upMin = min(b.upMin, b.upPass ? a.upMin : INF);
        result.upPass = a.upPass && b.upPass;
        return result;
    }

    void recalculateLocal(int v) {
        Aggregate a;

        // Enter v from its parent.
        bool downInputAvailable =
            (parent[v] == 0 || inputActive[parentPort[v]]);

        if (downInputAvailable) {
            int forbidden = parent[v] == 0
                                ? 0
                                : forbiddenOutput(v, parentPort[v]);

            a.downMin = localMinimum(v, forbidden);
            a.downPass =
                preferredPort[v] != 0 &&
                preferredPort[v] != forbidden;
        }

        // Enter v from its preferred child, or start at v if there is none.
        int incoming = preferredPort[v] ? preferredPort[v] : v;

        if (inputActive[incoming]) {
            int forbidden = forbiddenOutput(v, incoming);
            a.upMin = localMinimum(v, forbidden);
            a.upPass =
                parent[v] != 0 &&
                parentPort[v] != forbidden;
        }

        tree[v].local = a;
    }

    void pull(int v) {
        Aggregate a = tree[v].local;

        int left = tree[v].ch[0];
        int right = tree[v].ch[1];

        if (left) a = combine(tree[left].aggregate, a);
        if (right) a = combine(a, tree[right].aggregate);

        tree[v].aggregate = a;
        tree[v].top = left ? tree[left].top : v;
    }

    bool isAuxiliaryRoot(int v) const {
        int f = tree[v].fa;
        return f == 0 ||
               (tree[f].ch[0] != v && tree[f].ch[1] != v);
    }

    void rotate(int v) {
        int p = tree[v].fa;
        int g = tree[p].fa;
        int direction = (tree[p].ch[1] == v);
        int middle = tree[v].ch[direction ^ 1];

        if (!isAuxiliaryRoot(p)) {
            tree[g].ch[tree[g].ch[1] == p] = v;
        }
        tree[v].fa = g;

        tree[v].ch[direction ^ 1] = p;
        tree[p].fa = v;

        tree[p].ch[direction] = middle;
        if (middle) tree[middle].fa = p;

        pull(p);
        pull(v);
    }

    void splay(int v) {
        while (!isAuxiliaryRoot(v)) {
            int p = tree[v].fa;
            int g = tree[p].fa;

            if (!isAuxiliaryRoot(p)) {
                bool sameDirection =
                    (tree[p].ch[1] == v) == (tree[g].ch[1] == p);
                rotate(sameDirection ? p : v);
            }
            rotate(v);
        }
    }

    void access(int v) {
        int last = 0;

        for (int u = v; u != 0; u = tree[u].fa) {
            splay(u);

            int oldRight = tree[u].ch[1];

            if (oldRight) {
                int child = tree[oldRight].top;
                virtualChildren[u].insert({
                    tree[oldRight].aggregate.downMin,
                    childPort[child]
                });
            }

            if (last) {
                int child = tree[last].top;
                pair<int, int> entry = {
                    tree[last].aggregate.downMin,
                    childPort[child]
                };
                auto it = virtualChildren[u].find(entry);
                virtualChildren[u].erase(it);
            }

            tree[u].ch[1] = last;
            if (last) tree[last].fa = u;

            preferredPort[u] =
                last ? childPort[tree[last].top] : 0;

            recalculateLocal(u);
            pull(u);

            last = u;
        }

        splay(v);
    }

    void fixLocalTransition(int v, int incoming, int outgoing) {
        access(v);

        int a = findSet(incoming);
        int b = findSet(outgoing);

        if (a != b) {
            int newStart = chainStart[a];

            if (dsuSize[a] < dsuSize[b]) swap(a, b);
            dsu[b] = a;
            dsuSize[a] += dsuSize[b];
            chainStart[a] = newStart;

            --componentCount[v];
        }

        inputActive[incoming] = false;
        outputActive[outgoing] = false;

        recalculateLocal(v);
        pull(v);
    }

    int portAt(int v, int neighbor) const {
        if (parent[v] == neighbor) return parentPort[v];
        return childPort[neighbor];
    }

public:
    explicit Solver(int size)
        : n(size),
          graph(n + 1),
          parent(n + 1),
          depth(n + 1),
          parentPort(n + 1),
          childPort(n + 1),
          preferredPort(n + 1),
          dsu(3 * n + 1),
          dsuSize(3 * n + 1, 1),
          chainStart(3 * n + 1),
          componentCount(n + 1),
          inputActive(3 * n + 1, true),
          outputActive(3 * n + 1, true),
          tree(n + 1),
          virtualChildren(n + 1) {
        iota(dsu.begin(), dsu.end(), 0);
        iota(chainStart.begin(), chainStart.end(), 0);
    }

    void addEdge(int index, int u, int v) {
        int portU = n + 2 * index + 1;
        int portV = n + 2 * index + 2;

        graph[u].push_back({v, portU, portV});
        graph[v].push_back({u, portV, portU});
    }

    void initialize() {
        vector<int> order;
        order.reserve(n);
        order.push_back(1);

        for (int i = 0; i < (int)order.size(); ++i) {
            int v = order[i];

            for (const Edge& e : graph[v]) {
                if (e.to == parent[v]) continue;

                parent[e.to] = v;
                depth[e.to] = depth[v] + 1;

                childPort[e.to] = e.portHere;
                parentPort[e.to] = e.portThere;

                order.push_back(e.to);
            }
        }

        vector<int> subtreeMinimum(n + 1);
        iota(subtreeMinimum.begin(), subtreeMinimum.end(), 0);

        for (int i = n - 1; i >= 0; --i) {
            int v = order[i];
            componentCount[v] = (int)graph[v].size() + 1;

            if (parent[v]) {
                int p = parent[v];
                subtreeMinimum[p] = min(subtreeMinimum[p], subtreeMinimum[v]);

                virtualChildren[p].insert({
                    subtreeMinimum[v],
                    childPort[v]
                });
            }
        }

        for (int v = 1; v <= n; ++v) {
            tree[v].fa = parent[v];
            tree[v].top = v;
            recalculateLocal(v);
            pull(v);
        }
    }

    vector<int> solve(const vector<int>& initialPosition) {
        initialize();

        vector<int> answer(n + 1);
        vector<int> firstPart, secondPart, path;

        firstPart.reserve(n);
        secondPart.reserve(n);
        path.reserve(n);

        for (int number = 1; number <= n; ++number) {
            int source = initialPosition[number];

            access(source);
            int destination = tree[source].aggregate.upMin;
            answer[number] = destination;

            firstPart.clear();
            secondPart.clear();
            path.clear();

            int u = source;
            int v = destination;

            while (depth[u] > depth[v]) {
                firstPart.push_back(u);
                u = parent[u];
            }
            while (depth[v] > depth[u]) {
                secondPart.push_back(v);
                v = parent[v];
            }
            while (u != v) {
                firstPart.push_back(u);
                secondPart.push_back(v);
                u = parent[u];
                v = parent[v];
            }

            firstPart.push_back(u);

            for (int x : firstPart) path.push_back(x);
            for (int i = (int)secondPart.size() - 1; i >= 0; --i) {
                path.push_back(secondPart[i]);
            }

            for (int i = 0; i < (int)path.size(); ++i) {
                int x = path[i];

                int incoming =
                    i == 0 ? x : portAt(x, path[i - 1]);

                int outgoing =
                    i + 1 == (int)path.size()
                        ? x
                        : portAt(x, path[i + 1]);

                fixLocalTransition(x, incoming, outgoing);
            }
        }

        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        vector<int> initialPosition(n + 1);
        for (int i = 1; i <= n; ++i) {
            cin >> initialPosition[i];
        }

        Solver solver(n);

        for (int i = 0; i < n - 1; ++i) {
            int u, v;
            cin >> u >> v;
            solver.addEdge(i, u, v);
        }

        vector<int> answer = solver.solve(initialPosition);

        for (int i = 1; i <= n; ++i) {
            if (i > 1) cout << ' ';
            cout << answer[i];
        }
        cout << '\n';
    }

    return 0;
}

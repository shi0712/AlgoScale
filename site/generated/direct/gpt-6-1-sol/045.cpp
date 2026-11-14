#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr size_t SZ = 1 << 20;
    char buffer[SZ];
    size_t pos = 0, len = 0;

    char getChar() {
        if (pos == len) {
            len = fread(buffer, 1, SZ, stdin);
            pos = 0;
            if (len == 0) return 0;
        }
        return buffer[pos++];
    }

public:
    int readInt() {
        char c;
        do {
            c = getChar();
        } while (c <= ' ' && c != 0);

        int x = 0;
        while (c >= '0' && c <= '9') {
            x = x * 10 + (c - '0');
            c = getChar();
        }
        return x;
    }
};

/*
Each current vertex is also a node of its parent's leftist heap.

heap[v] : heap of the current children of v
height[v] : maximum distance from v to a descendant
left[v], right[v], rank[v] : links of v as a heap entry

Vertices on a folded diameter are first removed from their parents'
heaps. Consequently, their heap-entry links are free when we rebuild
the folded path.
*/
class Solver {
    int n;
    vector<int> heap, height, leftChild, rightChild, heapRank;

    int meld(int a, int b) {
        if (!a) return b;
        if (!b) return a;

        if (height[a] < height[b]) swap(a, b);

        rightChild[a] = meld(rightChild[a], b);

        if (heapRank[leftChild[a]] < heapRank[rightChild[a]])
            swap(leftChild[a], rightChild[a]);

        heapRank[a] = heapRank[rightChild[a]] + 1;
        return a;
    }

    // Remove and return a highest child of v.
    int popChild(int v) {
        int x = heap[v];
        if (!x) return 0;

        heap[v] = meld(leftChild[x], rightChild[x]);
        leftChild[x] = rightChild[x] = 0;
        heapRank[x] = 1;
        return x;
    }

    void updateHeight(int v) {
        height[v] = height[heap[v]] + 1;
    }

    // Lengths, in edges, of the two longest downward branches.
    pair<int, int> longestBranches(int v) const {
        int x = heap[v];
        if (!x) return {0, 0};

        int first = height[x] + 1;
        int second = max(height[leftChild[x]], height[rightChild[x]]) + 1;
        return {first, second};
    }

    // Move the root to a center of the current tree.
    int findCenter(int root) {
        while (true) {
            auto [first, second] = longestBranches(root);
            if (first <= second + 1) return root;

            int nextRoot = popChild(root);
            updateHeight(root);

            heap[nextRoot] = meld(heap[nextRoot], root);
            updateHeight(nextRoot);

            root = nextRoot;
        }
    }

    // Extract a longest downward path, starting at start.
    void extractPath(int start, vector<int>& path) {
        int v = start;
        while (v) {
            path.push_back(v);
            v = popChild(v);
        }
    }

public:
    explicit Solver(int n)
        : n(n),
          heap(n + 1, 0),
          height(n + 1, 0),
          leftChild(n + 1, 0),
          rightChild(n + 1, 0),
          heapRank(n + 1, 1) {
        height[0] = -1;
        heapRank[0] = 0;
    }

    int solve(FastInput& in) {
        vector<int> head(n + 1, -1);
        vector<int> to(2 * (n - 1));
        vector<int> nextEdge(2 * (n - 1));

        int edgeCount = 0;
        auto addEdge = [&](int u, int v) {
            to[edgeCount] = v;
            nextEdge[edgeCount] = head[u];
            head[u] = edgeCount++;
        };

        for (int i = 1; i < n; ++i) {
            int u = in.readInt();
            int v = in.readInt();
            addEdge(u, v);
            addEdge(v, u);
        }

        if (n == 1) return 0;

        // Initially root the tree at vertex 1.
        vector<int> parent(n + 1, 0);
        vector<int> order;
        order.reserve(n);
        order.push_back(1);

        for (int i = 0; i < n; ++i) {
            int v = order[i];
            for (int e = head[v]; e != -1; e = nextEdge[e]) {
                int u = to[e];
                if (u == parent[v]) continue;
                parent[u] = v;
                order.push_back(u);
            }
        }

        // Build all child heaps from the bottom upwards.
        for (int i = n - 1; i > 0; --i) {
            int v = order[i];
            updateHeight(v);
            int p = parent[v];
            heap[p] = meld(heap[p], v);
        }
        updateHeight(1);

        int root = findCenter(1);
        int answer = 0;

        vector<int> a, b;
        a.reserve(n / 2 + 1);
        b.reserve(n / 2 + 1);

        while (heap[root] != 0) {
            ++answer;
            a.clear();
            b.clear();

            auto [first, second] = longestBranches(root);
            bool vertexCenter = (first == second);

            if (vertexCenter) {
                // Diameter has 2*first edges.
                // The root itself is not paired with another vertex.
                extractPath(popChild(root), a);
                extractPath(popChild(root), b);
            } else {
                // Diameter has 2*first-1 edges.
                // Pair the root with its highest child.
                extractPath(popChild(root), b);

                a.push_back(root);
                extractPath(popChild(root), a);
            }

            // Pair symmetric vertices, rebuilding from the endpoints
            // towards the old center.
            int rebuiltChild = 0;
            for (int i = int(a.size()) - 1; i >= 0; --i) {
                int u = a[i];
                int v = b[i];

                heap[u] = meld(heap[u], heap[v]);
                heap[v] = 0;

                if (rebuiltChild)
                    heap[u] = meld(heap[u], rebuiltChild);

                updateHeight(u);
                rebuiltChild = u;
            }

            if (vertexCenter) {
                heap[root] = meld(heap[root], rebuiltChild);
                updateHeight(root);
            }

            root = findCenter(root);
        }

        return answer;
    }
};

int main() {
    FastInput in;
    int T = in.readInt();

    string output;
    output.reserve(T * 8);

    while (T--) {
        int n = in.readInt();
        Solver solver(n);
        output += to_string(solver.solve(in));
        output.push_back('\n');
    }

    fwrite(output.data(), 1, output.size(), stdout);
    return 0;
}

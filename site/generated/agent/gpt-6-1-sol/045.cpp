#include <bits/stdc++.h>
using namespace std;

class FastInput {
    static constexpr size_t SZ = 1 << 16;
    char buf[SZ];
    size_t pos = 0, len = 0;
public:
    int readInt() {
        char c;
        do {
            if (pos == len) {
                len = fread(buf, 1, SZ, stdin);
                pos = 0;
                if (!len) return 0;
            }
            c = buf[pos++];
        } while (c <= ' ');
        int x = c - '0';
        while (true) {
            if (pos == len) {
                len = fread(buf, 1, SZ, stdin);
                pos = 0;
                if (!len) break;
            }
            c = buf[pos];
            if (c < '0' || c > '9') break;
            ++pos;
            x = x * 10 + c - '0';
        }
        return x;
    }
};

struct LeftistHeaps {
    vector<int> left, right, rank, key;
    explicit LeftistHeaps(int n)
        : left(n + 1), right(n + 1), rank(n + 1, 1), key(n + 1) {
        rank[0] = 0;
    }
    int meld(int a, int b) {
        if (!a || !b) return a | b;
        if (key[a] < key[b] || (key[a] == key[b] && a > b)) swap(a, b);
        right[a] = meld(right[a], b);
        if (rank[left[a]] < rank[right[a]]) swap(left[a], right[a]);
        rank[a] = rank[right[a]] + 1;
        return a;
    }
    int pop(int a) {
        int result = meld(left[a], right[a]);
        left[a] = right[a] = 0;
        rank[a] = 1;
        return result;
    }
};

int main() {
    FastInput in;
    int tests = in.readInt();
    string output;
    while (tests--) {
        int n = in.readInt();
        vector<int> head(n + 1, -1), to(2 * (n - 1)), next(2 * (n - 1));
        int edges = 0;
        auto addEdge = [&](int u, int v) {
            to[edges] = v;
            next[edges] = head[u];
            head[u] = edges++;
        };
        for (int i = 1; i < n; ++i) {
            int u = in.readInt(), v = in.readInt();
            addEdge(u, v);
            addEdge(v, u);
        }
        if (n == 1) {
            output += "0\n";
            continue;
        }

        LeftistHeaps hp(n);
        vector<int> parent(n + 1), order;
        order.reserve(n);
        order.push_back(1);
        int root = 1;
        for (size_t i = 0; i < order.size(); ++i) {
            int u = order[i];
            if (hp.key[u] > hp.key[root]) root = u;
            for (int e = head[u]; e != -1; e = next[e]) {
                int v = to[e];
                if (v == parent[u]) continue;
                parent[v] = u;
                hp.key[v] = hp.key[u] + 1;
                order.push_back(v);
            }
        }
        order.clear();
        order.push_back(root);
        parent[root] = 0;
        for (size_t i = 0; i < order.size(); ++i) {
            int u = order[i];
            for (int e = head[u]; e != -1; e = next[e]) {
                int v = to[e];
                if (v == parent[u]) continue;
                parent[v] = u;
                order.push_back(v);
            }
        }

        // Heap entries are the child vertices themselves. Their keys are
        // one plus the height of that child's rooted subtree.
        vector<int> heap(n + 1);
        for (int i = n - 1; i > 0; --i) {
            int u = order[i];
            hp.key[u] = hp.key[heap[u]] + 1;
            heap[parent[u]] = hp.meld(heap[parent[u]], u);
        }

        vector<int> path;
        path.reserve(n);
        int answer = 0;
        while (heap[root]) {
            path.clear();
            int u = root;
            path.push_back(u);
            // A farthest path from our peripheral root is a diameter.
            // Remove its edges from the child heaps before identifying nodes.
            while (heap[u]) {
                int v = heap[u];
                heap[u] = hp.pop(v);
                u = v;
                path.push_back(u);
            }
            int diameter = (int)path.size() - 1;
            int middle = diameter / 2;
            for (int i = 0; i <= middle; ++i) {
                int j = diameter - i;
                if (i != j) {
                    heap[path[i]] = hp.meld(heap[path[i]], heap[path[j]]);
                    heap[path[j]] = 0;
                }
            }
            // Restore the surviving half of the path, from bottom to top.
            for (int i = middle - 1; i >= 0; --i) {
                int v = path[i + 1];
                hp.key[v] = hp.key[heap[v]] + 1;
                heap[path[i]] = hp.meld(heap[path[i]], v);
            }
            ++answer;
        }
        output += to_string(answer);
        output += '\n';
    }
    fwrite(output.data(), 1, output.size(), stdout);
}

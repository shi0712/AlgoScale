#include <algorithm>
#include <cassert>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

class Solver {
    int n;
    vector<int> first, to, next_edge;
    vector<int> parent, height, heavy, offset, length;
    vector<int> bucket_head, bucket_tail, next_child;
    vector<int> order, path;
#ifdef LOCAL_CHECK
    long long bucket_scans = 0;
    long long diameter_sum = 0;
    int initial_potential = 0;
#endif

    void add_edge(int u, int v) {
        next_edge.push_back(first[u]);
        to.push_back(v);
        first[u] = static_cast<int>(to.size()) - 1;
    }

    int find_endpoint(int start) {
        fill(parent.begin(), parent.end(), 0);
        order.clear();
        order.push_back(start);
        parent[start] = -1;
        height[start] = 0;
        int farthest = start;
        for (int i = 0; i < static_cast<int>(order.size()); ++i) {
            int u = order[i];
            if (height[u] > height[farthest]) farthest = u;
            for (int e = first[u]; e != -1; e = next_edge[e]) {
                int v = to[e];
                if (v == parent[u]) continue;
                parent[v] = u;
                height[v] = height[u] + 1;
                order.push_back(v);
            }
        }
        return farthest;
    }

    void append_to_bucket(int u, int v) {
        int h = height[v];
        assert(0 <= h && h < length[u]);
        int index = offset[u] + h;
        next_child[v] = 0;
        if (bucket_head[index] == 0) {
            bucket_head[index] = v;
        } else {
            next_child[bucket_tail[index]] = v;
        }
        bucket_tail[index] = v;
    }

    void initialize_rooted_tree(int root) {
        fill(parent.begin(), parent.end(), 0);
        order.clear();
        order.push_back(root);
        parent[root] = -1;
        for (int i = 0; i < static_cast<int>(order.size()); ++i) {
            int u = order[i];
            for (int e = first[u]; e != -1; e = next_edge[e]) {
                int v = to[e];
                if (v == parent[u]) continue;
                parent[v] = u;
                order.push_back(v);
            }
        }

        int total_length = 0;
        for (int i = n - 1; i >= 0; --i) {
            int u = order[i];
            heavy[u] = 0;
            for (int e = first[u]; e != -1; e = next_edge[e]) {
                int v = to[e];
                if (parent[v] != u) continue;
                if (heavy[u] == 0 || height[v] > height[heavy[u]]) heavy[u] = v;
            }
            height[u] = heavy[u] == 0 ? 0 : height[heavy[u]] + 1;
            length[u] = 0;
            for (int e = first[u]; e != -1; e = next_edge[e]) {
                int v = to[e];
                if (parent[v] == u && v != heavy[u]) {
                    length[u] = max(length[u], height[v] + 1);
                }
            }
            offset[u] = total_length;
            total_length += length[u];
        }
        assert(total_length <= n - 1);
        bucket_head.assign(total_length, 0);
        bucket_tail.assign(total_length, 0);
        for (int u : order) {
            for (int e = first[u]; e != -1; e = next_edge[e]) {
                int v = to[e];
                if (parent[v] == u && v != heavy[u]) append_to_bucket(u, v);
            }
        }
#ifdef LOCAL_CHECK
        initial_potential = total_length;
#endif
    }

    void merge_buckets(int a, int b) {
        if (length[a] < length[b]) {
            swap(length[a], length[b]);
            swap(offset[a], offset[b]);
        }
        for (int h = 0; h < length[b]; ++h) {
#ifdef LOCAL_CHECK
            ++bucket_scans;
#endif
            int x = offset[a] + h;
            int y = offset[b] + h;
            if (bucket_head[y] == 0) continue;
            if (bucket_head[x] == 0) {
                bucket_head[x] = bucket_head[y];
            } else {
                next_child[bucket_tail[x]] = bucket_head[y];
            }
            bucket_tail[x] = bucket_tail[y];
        }
        length[b] = 0;
    }

    void restore_vertex(int u, int chain_child) {
        if (chain_child != 0 &&
            (length[u] == 0 || height[chain_child] >= length[u] - 1)) {
            heavy[u] = chain_child;
        } else if (length[u] != 0) {
            int index = offset[u] + length[u] - 1;
            int v = bucket_head[index];
            assert(v != 0);
            bucket_head[index] = next_child[v];
            if (bucket_head[index] == 0) bucket_tail[index] = 0;
            next_child[v] = 0;
            heavy[u] = v;

            // Insert before trimming. Thus the logical bucket array never grows.
            if (chain_child != 0) append_to_bucket(u, chain_child);
            while (length[u] != 0 &&
                   bucket_head[offset[u] + length[u] - 1] == 0) {
                --length[u];
#ifdef LOCAL_CHECK
                ++bucket_scans;
#endif
            }
        } else {
            heavy[u] = 0;
        }
        height[u] = heavy[u] == 0 ? 0 : height[heavy[u]] + 1;
    }

public:
    explicit Solver(int n_) : n(n_), first(n + 1, -1), parent(n + 1),
        height(n + 1), heavy(n + 1), offset(n + 1), length(n + 1),
        next_child(n + 1) {
        to.reserve(2 * max(0, n - 1));
        next_edge.reserve(2 * max(0, n - 1));
        order.reserve(n);
        path.reserve(n);
    }

    void read_edges() {
        for (int i = 1; i < n; ++i) {
            int u, v;
            cin >> u >> v;
            add_edge(u, v);
            add_edge(v, u);
        }
    }

    int run() {
        int root = find_endpoint(1);
        initialize_rooted_tree(root);
        int answer = 0;

        while (heavy[root] != 0) {
            path.clear();
            int u = root;
            while (u != 0) {
                path.push_back(u);
                int v = heavy[u];
                heavy[u] = 0;
                u = v;
            }
            int diameter = static_cast<int>(path.size()) - 1;
#ifdef LOCAL_CHECK
            diameter_sum += diameter;
#endif
            for (int i = 0; i < (diameter + 1) / 2; ++i) {
                merge_buckets(path[i], path[diameter - i]);
            }
            int last = diameter / 2;
            for (int i = last; i >= 0; --i) {
                restore_vertex(path[i], i == last ? 0 : path[i + 1]);
            }
            ++answer;
        }

#ifdef LOCAL_CHECK
        assert(bucket_scans == initial_potential);
        assert(diameter_sum <= 2LL * (n - 1));
#endif
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    cin >> tests;
    while (tests--) {
        int n;
        cin >> n;
        Solver solver(n);
        solver.read_edges();
        cout << solver.run() << '\n';
    }
    return 0;
}

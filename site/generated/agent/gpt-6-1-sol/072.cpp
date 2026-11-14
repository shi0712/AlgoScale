#include <bits/stdc++.h>
using namespace std;

class Solver {
    static constexpr int LOG = 16;
    struct Decision {
        int vertex, radius, yes = -1, no = -1, height = 0;
    };

    int n, timer = 0, samples = 12;
    vector<vector<int>> graph;
    vector<int> depth, tin, tout, first, logarithm;
    vector<array<int, LOG>> ancestor;
    vector<vector<int>> rmq;
    vector<Decision> decisions;
    uint64_t random_state = 0x9e3779b97f4a7c15ULL;

    uint64_t random_number() {
        random_state ^= random_state << 7;
        random_state ^= random_state >> 9;
        return random_state;
    }

    void prepare() {
        vector<int> euler;
        euler.reserve(2 * n - 1);
        // An iterative DFS also handles a tree consisting of one long path.
        vector<int> next_edge(n), stack = {0};
        ancestor[0].fill(0);
        tin[0] = timer++;
        first[0] = 0;
        euler.push_back(0);
        while (!stack.empty()) {
            int v = stack.back();
            if (next_edge[v] == (int)graph[v].size()) {
                tout[v] = timer;
                stack.pop_back();
                if (!stack.empty()) euler.push_back(stack.back());
                continue;
            }
            int w = graph[v][next_edge[v]++];
            if (w == ancestor[v][0]) continue;
            depth[w] = depth[v] + 1;
            ancestor[w][0] = v;
            for (int j = 1; j < LOG; ++j)
                ancestor[w][j] = ancestor[ancestor[w][j - 1]][j - 1];
            tin[w] = timer++;
            first[w] = (int)euler.size();
            euler.push_back(w);
            stack.push_back(w);
        }
        int length = (int)euler.size();
        logarithm.resize(length + 1);
        for (int i = 2; i <= length; ++i)
            logarithm[i] = logarithm[i / 2] + 1;
        rmq.push_back(move(euler));
        for (int k = 1; (1 << k) <= length; ++k) {
            rmq.emplace_back(length - (1 << k) + 1);
            for (int i = 0; i + (1 << k) <= length; ++i) {
                int a = rmq[k - 1][i];
                int b = rmq[k - 1][i + (1 << (k - 1))];
                rmq[k][i] = depth[a] < depth[b] ? a : b;
            }
        }
    }

    int lca(int a, int b) const {
        int left = first[a], right = first[b];
        if (left > right) swap(left, right);
        int k = logarithm[right - left + 1];
        a = rmq[k][left];
        b = rmq[k][right - (1 << k) + 1];
        return depth[a] < depth[b] ? a : b;
    }

    int distance(int a, int b) const {
        return depth[a] + depth[b] - 2 * depth[lca(a, b)];
    }

    // Candidate lists stay sorted by DFS entry time throughout construction.
    int build(const vector<int>& candidates, bool shuffle_centers) {
        int size = (int)candidates.size();
        int id = (int)decisions.size();
        decisions.push_back({candidates[0], 0});
        if (size == 1) return id;
        if (size <= 3) {
            vector<int> remaining(candidates.begin() + 1, candidates.end());
            int yes = build(vector<int>{candidates[0]}, shuffle_centers);
            int no = build(remaining, shuffle_centers);
            decisions[id] = {candidates[0], 0, yes, no,
                             1 + max(decisions[yes].height, decisions[no].height)};
            return id;
        }

        auto lower = [&](int entry) {
            return lower_bound(candidates.begin(), candidates.end(), entry,
                               [&](int v, int t) { return tin[v] < t; })
                   - candidates.begin();
        };
        auto subtree_count = [&](int v) {
            return lower(tout[v]) - lower(tin[v]);
        };

        // Find the centroid with unit weights on the remaining candidates.
        // Every subtree containing a majority contains the middle DFS vertex.
        int center = candidates[size / 2];
        if (subtree_count(center) <= size / 2) {
            for (int j = LOG - 1; j >= 0; --j)
                if (subtree_count(ancestor[center][j]) <= size / 2)
                    center = ancestor[center][j];
            center = ancestor[center][0];
        }
        vector<int> centers = {center};
        for (int v : graph[center]) centers.push_back(v);
        if (size <= 16) {
            for (int v : candidates) centers.push_back(v);
            for (int i = 1; i < size; ++i)
                centers.push_back(lca(candidates[i - 1], candidates[i]));
        } else {
            for (int i = 0; i < samples; ++i) {
                int a = candidates[random_number() % size];
                centers.push_back(a);
                if (i % 2 == 0)
                    centers.push_back(lca(a, candidates[random_number() % size]));
            }
        }
        sort(centers.begin(), centers.end());
        centers.erase(unique(centers.begin(), centers.end()), centers.end());
        if (shuffle_centers) {
            for (int i = (int)centers.size() - 1; i > 0; --i)
                swap(centers[i], centers[random_number() % (i + 1)]);
        }

        int best_size = size, best_vertex = candidates[0], best_radius = 0;
        vector<int> distances(size);
        for (int v : centers) {
            for (int i = 0; i < size; ++i)
                distances[i] = distance(v, candidates[i]);
            sort(distances.begin(), distances.end());
            // Only thresholds between unequal consecutive distances matter.
            for (int i = 1; i < size; ++i) {
                if (distances[i - 1] == distances[i]) continue;
                int score = max(i, size - i);
                if (score < best_size) {
                    best_size = score;
                    best_vertex = v;
                    best_radius = distances[i - 1];
                }
            }
            if (best_size == (size + 1) / 2) break;
        }
        vector<int> inside, outside;
        inside.reserve(size);
        outside.reserve(size);
        for (int v : candidates) {
            if (distance(best_vertex, v) <= best_radius) inside.push_back(v);
            else outside.push_back(v);
        }
        int yes = build(inside, shuffle_centers);
        int no = build(outside, shuffle_centers);
        decisions[id] = {best_vertex, best_radius, yes, no,
                         1 + max(decisions[yes].height, decisions[no].height)};
        return id;
    }

public:
    explicit Solver(int size)
        : n(size), graph(n), depth(n), tin(n), tout(n), first(n), ancestor(n) {}

    void add_edge(int a, int b) {
        graph[a].push_back(b);
        graph[b].push_back(a);
    }

    int make_strategy() {
        prepare();
        vector<int> candidates(n);
        for (int v = 0; v < n; ++v) candidates[tin[v]] = v;
        decisions.reserve(2 * n - 1);
        int attempt = 0;
        for (;;) {
            decisions.clear();
            build(candidates, attempt != 0);
            // Validate every possible reply sequence before sending signals.
            if (decisions[0].height <= 21) break;
            ++attempt;
            samples = min(256, samples * 2);
        }
        return decisions[0].height;
    }

    bool interact() const {
        int current = 0;
        while (decisions[current].yes != -1) {
            const auto& query = decisions[current];
            cout << "? " << query.vertex + 1 << ' ' << query.radius << endl;
            int reply;
            if (!(cin >> reply) || (reply != 0 && reply != 1)) return false;
            current = reply ? query.yes : query.no;
        }
        cout << "! " << decisions[current].vertex + 1 << endl;
        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int tests;
    if (!(cin >> tests)) return 0;
    while (tests--) {
        int n;
        if (!(cin >> n)) return 0;
        Solver solver(n);
        for (int v = 1; v < n; ++v) {
            int parent;
            if (!(cin >> parent)) return 0;
            solver.add_edge(parent - 1, v);
        }
        int height = solver.make_strategy();
#ifdef LOCAL_PLAN_TEST
        cout << height << '\n';
#else
        (void)height;
        if (!solver.interact()) return 0;
#endif
    }
    return 0;
}

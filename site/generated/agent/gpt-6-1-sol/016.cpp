#include <bits/stdc++.h>
using namespace std;

// A planar minimum-cost flow network. The source and sink share a face.
// Each shortest-path phase sends all possible flow on zero reduced-cost arcs.
class PlanarFlow {
public:
    struct Edge { int to, next, cap, cost; };
    int vertices, return_edge;
    vector<Edge> edges;
    vector<int> head, rotation, face, face_head;
    vector<long long> potential, distance, dual_distance;

    PlanarFlow(int n, int max_edges) : vertices(n), head(n, -1),
        potential(n), distance(n) { edges.reserve(2 * max_edges); }

    int add_edge(int u, int v, int capacity, int cost) {
        int id = (int)edges.size();
        edges.push_back({v, head[u], capacity, cost}); head[u] = id;
        edges.push_back({u, head[v], 0, -cost}); head[v] = id + 1;
        return id;
    }

    void build_faces() {
        face.assign(edges.size(), -1);
        for (int h = 0; h < (int)edges.size(); ++h) if (face[h] == -1) {
            int id = (int)face_head.size(), x = h;
            face_head.push_back(h);
            do {
                face[x] = id;
                x = rotation[x ^ 1];
            } while (x != h);
        }
        dual_distance.resize(face_head.size());
    }

    int admissible_max_flow() {
        // A directed primal arc gives a dual arc whose length is its capacity.
        // Omitting the dual arc of t -> s turns the minimum s-t cut into a
        // shortest dual path between the two faces incident to that edge.
        constexpr long long INF = (1LL << 60);
        fill(dual_distance.begin(), dual_distance.end(), INF);
        int start = face[return_edge], target = face[return_edge ^ 1];
        using Item = pair<long long, int>;
        priority_queue<Item, vector<Item>, greater<Item>> queue;
        dual_distance[start] = 0;
        queue.push({0, start});
        while (!queue.empty()) {
            auto [du, u] = queue.top(); queue.pop();
            if (du != dual_distance[u]) continue;
            if (u == target) break;
            int first = face_head[u], h = first;
            do {
                if (h != return_edge) {
                    int v = face[h ^ 1], length = edges[h].cap;
                    if (length && edges[h].cost + potential[edges[h ^ 1].to]
                                                - potential[edges[h].to] != 0)
                        length = 0;
                    if (du + length < dual_distance[v]) {
                        dual_distance[v] = du + length;
                        queue.push({du + length, v});
                    }
                }
                h = rotation[h ^ 1];
            } while (h != first);
        }
        long long value = dual_distance[target];
        for (long long &x : dual_distance) x = min(x, value);
        // Differences of dual distances give a maximum primal flow: they
        // satisfy capacities and telescope to zero at every internal vertex.
        for (int h = 0; h < (int)edges.size(); h += 2) if (h != return_edge) {
            int change = (int)(dual_distance[face[h ^ 1]] - dual_distance[face[h]]);
            edges[h].cap -= change;
            edges[h ^ 1].cap += change;
        }
        return (int)value;
    }

    bool solve(int source, int sink, int required, const vector<int> &order) {
        // Initially all residual arcs with positive capacity form a DAG.
        for (int u : order)
            for (int h = head[u]; h != -1; h = edges[h].next)
                if (edges[h].cap)
                    potential[edges[h].to] = min(potential[edges[h].to],
                                                potential[u] + edges[h].cost);
        constexpr long long INF = (1LL << 60);
        using Item = pair<long long, int>;
        while (required) {
            fill(distance.begin(), distance.end(), INF);
            priority_queue<Item, vector<Item>, greater<Item>> queue;
            distance[source] = 0;
            queue.push({0, source});
            while (!queue.empty()) {
                auto [du, u] = queue.top(); queue.pop();
                if (du != distance[u]) continue;
                if (u == sink) break;
                for (int h = head[u]; h != -1; h = edges[h].next) if (edges[h].cap) {
                    int v = edges[h].to;
                    long long nd = du + edges[h].cost + potential[u] - potential[v];
                    if (nd < distance[v]) {
                        distance[v] = nd;
                        queue.push({nd, v});
                    }
                }
            }
            if (distance[sink] == INF) return false;
            for (int u = 0; u < vertices; ++u)
                potential[u] += min(distance[u], distance[sink]);
            required -= admissible_max_flow();
        }
        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    string s;
    cin >> s;
    vector<int> color(n);
    int count[3] = {};
    for (int &x : color) { cin >> x; ++count[x]; }
    int length_a = count[0] + count[2], length_b = count[1] + count[2];
    if ((length_a | length_b) & 1) {
        cout << -1 << '\n';
        return 0;
    }
    int ka = length_a / 2, kb = length_b / 2;
    int root = ka + kb + 2, source = root + 1, sink = root + 2;
    int vertices = root + 3;
    PlanarFlow flow(vertices, n + ka + kb + 6);
    vector<int> previous(vertices, -1), next(vertices, -1), chord_tail(vertices);
    auto boundary_edge = [&](int u, int v, int cap) {
        int h = flow.add_edge(u, v, cap, 0);
        next[u] = h;
        previous[v] = h ^ 1;
        return h;
    };

    // A balanced sequence of length 2k has k closing brackets, and at most j
    // of them in its first 2j+1 positions. The even-prefix bounds follow.
    // A position p injects its closing-bracket unit at node floor(p/2).
    for (int j = 0; j < ka; ++j) boundary_edge(j, j + 1, j);
    boundary_edge(ka, sink, ka);
    flow.return_edge = boundary_edge(sink, source, 0);
    boundary_edge(source, ka + 1 + kb, kb);
    for (int j = kb; j > 0; --j) boundary_edge(ka + 1 + j, ka + j, j - 1);
    boundary_edge(ka + 1, 0, 0);

    vector<int> bracket_edge(n);
    int pa = 0, pb = 0;
    for (int i = 0; i < n; ++i) {
        int a = -1, b = -1;
        if (color[i] != 1) a = (++pa) / 2;
        if (color[i] != 0) b = ka + 1 + (++pb) / 2;
        int u = b == -1 ? root : b;
        int v = a == -1 ? root : a;
        bracket_edge[i] = flow.add_edge(u, v, 1, s[i] == '(' ? 1 : -1);
    }
    int source_spoke = flow.add_edge(source, root, max(ka - kb, 0), 0);
    int sink_spoke = flow.add_edge(root, sink, max(kb - ka, 0), 0);

    // The boundary order is A increasing, sink, source, B decreasing.
    // Blue arcs are noncrossing chords; private arcs go to the exterior root.
    vector<int> cycle;
    cycle.reserve(vertices - 1);
    for (int j = 0; j <= ka; ++j) cycle.push_back(j);
    cycle.push_back(sink);
    cycle.push_back(source);
    for (int j = kb; j >= 0; --j) cycle.push_back(ka + 1 + j);
    flow.rotation.resize(flow.edges.size());
    for (int v : cycle) {
        flow.rotation[previous[v]] = next[v];
        flow.rotation[next[v]] = previous[v];
        chord_tail[v] = previous[v];
    }
    auto insert_after = [&](int old, int h) {
        flow.rotation[h] = flow.rotation[old];
        flow.rotation[old] = h;
    };
    for (int i = 0; i < n; ++i) {
        int h = bracket_edge[i];
        int a = flow.edges[h].to, b = flow.edges[h ^ 1].to;
        if (color[i] == 2) {
            insert_after(chord_tail[a], h ^ 1);
            chord_tail[a] = h ^ 1;
            insert_after(previous[b], h);
        } else {
            int v = color[i] == 0 ? a : b;
            insert_after(next[v], color[i] == 0 ? h ^ 1 : h);
        }
    }
    insert_after(next[source], source_spoke);
    insert_after(next[sink], sink_spoke ^ 1);
    int root_first = -1, root_last = -1;
    for (int v : cycle) {
        int group_first = -1, group_last = -1;
        for (int h = flow.rotation[next[v]]; h != previous[v]; h = flow.rotation[h]) {
            int other = h ^ 1;
            flow.rotation[other] = group_first;
            group_first = other;
            if (group_last == -1) group_last = other;
        }
        if (group_first != -1) {
            if (root_last == -1) root_first = group_first;
            else flow.rotation[root_last] = group_first;
            root_last = group_last;
        }
    }
    flow.rotation[root_last] = root_first;
    flow.build_faces();

    vector<int> order;
    order.reserve(vertices);
    order.push_back(source);
    for (int j = kb; j >= 0; --j) order.push_back(ka + 1 + j);
    order.push_back(root);
    for (int j = 0; j <= ka; ++j) order.push_back(j);
    order.push_back(sink);
    if (!flow.solve(source, sink, max(ka, kb), order)) {
        cout << -1 << '\n';
        return 0;
    }
    int answer = 0;
    for (int i = 0; i < n; ++i)
        answer += bool(flow.edges[bracket_edge[i] ^ 1].cap) != (s[i] == ')');
    cout << answer << '\n';
}

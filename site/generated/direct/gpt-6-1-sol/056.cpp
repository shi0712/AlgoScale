#include <bits/stdc++.h>
using namespace std;

/*
    Edmonds' blossom algorithm.

    A phase grows alternating trees from all exposed vertices.
    When two different trees meet, an augmenting path is used and
    those two trees are retired for the remainder of the phase.
    Blossoms are contracted using a disjoint-set structure.
*/
class GeneralMatching {
    int n;
    vector<int> start, adj;

    vector<int> base, parent, root, seen;
    vector<unsigned char> outer, retired;
    vector<int> que, contracted;
    int timer = 0;

    mt19937 rng{712367821};

    int findBase(int v) {
        int r = v;
        while (base[r] != r) r = base[r];
        while (base[v] != v) {
            int next = base[v];
            base[v] = r;
            v = next;
        }
        return r;
    }

    int lca(int a, int b) {
        ++timer;
        while (true) {
            if (a != -1) {
                a = findBase(a);
                if (seen[a] == timer) return a;
                seen[a] = timer;

                if (mate[a] == -1) {
                    a = -1;
                } else {
                    a = parent[mate[a]];
                }
            }
            swap(a, b);
        }
    }

    void markPath(int v, int blossomBase, int child) {
        while (findBase(v) != blossomBase) {
            contracted.push_back(findBase(v));
            contracted.push_back(findBase(mate[v]));

            parent[v] = child;
            child = mate[v];
            v = parent[mate[v]];
        }
    }

    void shrinkBlossom(int u, int v) {
        int b = lca(u, v);

        contracted.clear();
        markPath(u, b, v);
        markPath(v, b, u);

        for (int x : contracted) {
            // A previously contracted component is already entirely outer.
            // Thus only a newly outer singleton needs to be enqueued.
            if (!outer[x]) {
                outer[x] = 1;
                que.push_back(x);
            }
            base[x] = b;
        }
    }

    // x was the old matched neighbor of an endpoint of the new connecting edge.
    void flipArm(int x) {
        while (x != -1) {
            int y = parent[x];
            int next = mate[y];
            mate[x] = y;
            mate[y] = x;
            x = next;
        }
    }

    void augmentBetween(int u, int v) {
        int x = mate[u];
        int y = mate[v];

        mate[u] = v;
        mate[v] = u;

        flipArm(x);
        flipArm(y);
    }

public:
    vector<int> mate;

    GeneralMatching(int vertices, const vector<pair<int, int>>& edges)
        : n(vertices),
          start(n + 1, 0),
          base(n),
          parent(n),
          root(n),
          seen(n, 0),
          outer(n),
          retired(n),
          mate(n, -1) {
        for (auto [u, v] : edges) {
            ++start[u + 1];
            ++start[v + 1];
        }
        for (int i = 1; i <= n; ++i) start[i] += start[i - 1];

        adj.resize(start[n]);
        vector<int> ptr = start;
        for (auto [u, v] : edges) {
            adj[ptr[u]++] = v;
            adj[ptr[v]++] = u;
        }

        que.reserve(n);
        contracted.reserve(64);
    }

    // Install a valid initial matching: first all center-internal edges,
    // then greedily replace some of them with complete trominoes.
    void initialize(int dotCount, const vector<int>& centerNodes) {
        for (int a : centerNodes) {
            mate[a] = a + 1;
            mate[a + 1] = a;
        }

        vector<int> order = centerNodes;
        shuffle(order.begin(), order.end(), rng);

        for (int a : order) {
            int b = a + 1;
            int x = -1, y = -1;

            for (int e = start[a]; e < start[a + 1]; ++e) {
                int v = adj[e];
                if (v < dotCount && mate[v] == -1) {
                    x = v;
                    break;
                }
            }
            if (x == -1) continue;

            for (int e = start[b]; e < start[b + 1]; ++e) {
                int v = adj[e];
                if (v < dotCount && v != x && mate[v] == -1) {
                    y = v;
                    break;
                }
            }
            if (y == -1) continue;

            mate[a] = x;
            mate[x] = a;
            mate[b] = y;
            mate[y] = b;
        }
    }

    void solve() {
        while (true) {
            iota(base.begin(), base.end(), 0);
            fill(parent.begin(), parent.end(), -1);
            fill(root.begin(), root.end(), -1);
            fill(outer.begin(), outer.end(), 0);
            fill(retired.begin(), retired.end(), 0);
            fill(seen.begin(), seen.end(), 0);
            timer = 0;
            que.clear();

            for (int v = 0; v < n; ++v) {
                if (mate[v] == -1 && start[v] != start[v + 1]) {
                    root[v] = v;
                    outer[v] = 1;
                    que.push_back(v);
                }
            }

            shuffle(que.begin(), que.end(), rng);

            int augmentations = 0;

            for (size_t head = 0; head < que.size(); ++head) {
                int u = que[head];
                int ru = root[u];
                if (retired[ru]) continue;

                for (int e = start[u]; e < start[u + 1]; ++e) {
                    int v = adj[e];

                    if (root[v] != -1 && retired[root[v]]) continue;
                    if (mate[u] == v || findBase(u) == findBase(v)) continue;

                    if (outer[v]) {
                        int rv = root[v];

                        if (ru != rv) {
                            augmentBetween(u, v);
                            retired[ru] = retired[rv] = 1;
                            ++augmentations;
                            break;
                        }

                        shrinkBlossom(u, v);
                    } else if (root[v] == -1) {
                        parent[v] = u;
                        root[v] = ru;

                        int w = mate[v];
                        // Every exposed vertex was already made a root.
                        root[w] = ru;
                        outer[w] = 1;
                        que.push_back(w);
                    }
                }
            }

            if (augmentations == 0) return;
        }
    }
};

struct Center {
    int position;
    int node;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<string> board(n);
    for (string& row : board) cin >> row;

    const int cells = n * m;
    vector<int> dotId(cells, -1);
    vector<int> dotPosition;
    dotPosition.reserve(cells);

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            if (board[r][c] == '.') {
                int p = r * m + c;
                dotId[p] = (int)dotPosition.size();
                dotPosition.push_back(p);
            }
        }
    }

    const int dotCount = (int)dotPosition.size();
    int vertices = dotCount;

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    vector<Center> centers;
    vector<int> centerNodes;

    // Centers that cannot form any tromino need no matching gadget.
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            if (board[r][c] == '.') continue;

            int vertical = 0, horizontal = 0;
            for (int d = 0; d < 4; ++d) {
                int nr = r + dr[d], nc = c + dc[d];
                if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
                if (board[nr][nc] != '.') continue;

                if (d < 2) ++vertical;
                else ++horizontal;
            }

            bool useful;
            if (board[r][c] == '*') {
                useful = vertical > 0 && horizontal > 0;
            } else {
                useful = vertical + horizontal >= 2;
            }

            if (useful) {
                centers.push_back({r * m + c, vertices});
                centerNodes.push_back(vertices);
                vertices += 2;
            }
        }
    }

    vector<pair<int, int>> edges;
    edges.reserve(centers.size() * 9);

    for (const Center& center : centers) {
        int r = center.position / m;
        int c = center.position % m;
        int a = center.node;
        int b = a + 1;

        edges.emplace_back(a, b);

        for (int d = 0; d < 4; ++d) {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;

            int id = dotId[nr * m + nc];
            if (id == -1) continue;

            if (board[r][c] == '+') {
                edges.emplace_back(a, id);
                edges.emplace_back(b, id);
            } else {
                // For '*', one endpoint must be horizontal and one vertical.
                edges.emplace_back(d < 2 ? b : a, id);
            }
        }
    }

    GeneralMatching matching(vertices, edges);
    vector<pair<int, int>>().swap(edges);

    matching.initialize(dotCount, centerNodes);
    matching.solve();

    vector<string> answer = board;

    for (const Center& center : centers) {
        int a = center.node;
        int b = a + 1;

        if (matching.mate[a] == b) continue;

        array<int, 3> positions = {
            center.position,
            dotPosition[matching.mate[a]],
            dotPosition[matching.mate[b]]
        };

        unsigned int forbidden = 0;
        for (int p : positions) {
            int r = p / m, c = p % m;
            for (int d = 0; d < 4; ++d) {
                int nr = r + dr[d], nc = c + dc[d];
                if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;

                char ch = answer[nr][nc];
                if ('a' <= ch && ch <= 'z') {
                    forbidden |= 1u << (ch - 'a');
                }
            }
        }

        int color = 0;
        while (forbidden & (1u << color)) ++color;

        for (int p : positions) {
            answer[p / m][p % m] = char('a' + color);
        }
    }

    for (const string& row : answer) {
        cout << row << '\n';
    }

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

// Edmonds' alternating-tree algorithm for a sparse general graph.
// Searches initialize only reached vertices; exhausted trees are removed.
// The graph uses O(n*m) vertices and edges, and O(n*m) memory.
class Blossom {
    int n;
    const vector<int> &start, &edges;
    vector<int> parent, base, state, mark, touched, que;
    vector<unsigned char> dead;
    int stamp = 0;

    int find(int x) {
        int r = x;
        while (base[r] != r) r = base[r];
        while (base[x] != x) {
            int y = base[x];
            base[x] = r;
            x = y;
        }
        return r;
    }

    int lca(int x, int y) {
        ++stamp;
        for (;;) {
            if (x != -1) {
                x = find(x);
                if (mark[x] == stamp) return x;
                mark[x] = stamp;
                x = (match[x] == -1 ? -1 : parent[match[x]]);
            }
            swap(x, y);
        }
    }

    void contract(int x, int y, int b) {
        while (find(x) != b) {
            parent[x] = y;
            y = match[x];
            if (state[y] == 2) {
                state[y] = 1;
                que.push_back(y);
            }
            // Walk through an older blossom before linking its base, so all
            // newly outer vertices get enqueued even for nested blossoms.
            if (base[x] == x) base[x] = b;
            if (base[y] == y) base[y] = b;
            x = parent[y];
        }
    }

    void visit(int x, int s) {
        state[x] = s;
        parent[x] = -1;
        base[x] = x;
        touched.push_back(x);
    }

    bool augment(int root) {
        touched.clear();
        que.clear();
        visit(root, 1);
        que.push_back(root);
        for (size_t head = 0; head < que.size(); ++head) {
            int v = que[head];
            for (int e = start[v]; e < start[v + 1]; ++e) {
                int u = edges[e];
                if (dead[u] || state[u] == 2 || find(v) == find(u)) continue;
                if (state[u] == 0) {
                    visit(u, 2);
                    parent[u] = v;
                    if (match[u] == -1) {
                        for (int x = u; x != -1; ) {
                            int p = parent[x];
                            int next = match[p];
                            match[x] = p;
                            match[p] = x;
                            x = next;
                        }
                        for (int x : touched) state[x] = 0, base[x] = x;
                        return true;
                    }
                    int w = match[u];
                    visit(w, 1);
                    que.push_back(w);
                } else {
                    int b = lca(v, u);
                    contract(v, u, b);
                    contract(u, v, b);
                }
            }
        }
        // An exhausted alternating tree can be set aside permanently. After
        // contracting its blossoms, its outer vertices have no outside edges,
        // and every inner vertex is matched to an outer vertex. No augmenting
        // path from the remaining graph can pass through this tree.
        for (int x : touched) dead[x] = 1, state[x] = 0, base[x] = x;
        return false;
    }

public:
    vector<int> match;
    Blossom(const vector<int>& offsets, const vector<int>& adjacency)
        : n((int)offsets.size() - 1), start(offsets), edges(adjacency),
          parent(n, -1), base(n), state(n), mark(n), dead(n), match(n, -1) {
        iota(base.begin(), base.end(), 0);
        touched.reserve(n);
        que.reserve(n);
    }

    void solve(vector<int> roots) {
        for (int v : roots) if (match[v] == -1 && !dead[v]) augment(v);
    }
};

struct Center {
    int pos, a;
    bool plus;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<string> board(n);
    for (string &s : board) cin >> s;
    int cells = n * m;
    vector<int> id(cells, -1);
    int vertices = 0;
    vector<int> roots;
    for (int r = 0; r < n; ++r)
        for (int c = 0; c < m; ++c)
            if (board[r][c] == '.') {
                id[r * m + c] = vertices++;
                roots.push_back(id[r * m + c]);
            }
    auto dot = [&](int r, int c) -> int {
        if (r < 0 || r >= n || c < 0 || c >= m) return -1;
        return id[r * m + c];
    };
    vector<Center> centers;
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            char ch = board[r][c];
            if (ch == '.') continue;
            int vertical = (dot(r - 1, c) >= 0) + (dot(r + 1, c) >= 0);
            int horizontal = (dot(r, c - 1) >= 0) + (dot(r, c + 1) >= 0);
            if (ch == '+' ? vertical + horizontal >= 2 : vertical && horizontal) {
                centers.push_back({r * m + c, vertices, ch == '+'});
                vertices += 2;
            }
        }
    }
    // Each center has two slots, initially matched to one another. Every
    // augmentation starts and ends at dots, so both slots always stay covered.
    // Thus a center is either unused or matched to exactly two legal dots.
    // With C centers and T selected trominoes, the matching has C + T edges.
    vector<pair<int,int>> links;
    links.reserve(centers.size() * 7);
    for (auto center : centers) {
        int r = center.pos / m, c = center.pos % m, a = center.a;
        links.emplace_back(a, a + 1);
        int neighbors[4] = {dot(r - 1, c), dot(r + 1, c), dot(r, c - 1), dot(r, c + 1)};
        for (int k = 0; k < 4; ++k) if (neighbors[k] != -1) {
            if (center.plus) {
                links.emplace_back(a, neighbors[k]);
                links.emplace_back(a + 1, neighbors[k]);
            } else {
                links.emplace_back(a + (k >= 2), neighbors[k]);
            }
        }
    }
    vector<int> offsets(vertices + 1);
    for (auto [a, b] : links) ++offsets[a + 1], ++offsets[b + 1];
    partial_sum(offsets.begin(), offsets.end(), offsets.begin());
    vector<int> adjacency(offsets.back()), cursor = offsets;
    for (auto [a, b] : links) {
        adjacency[cursor[a]++] = b;
        adjacency[cursor[b]++] = a;
    }
    vector<pair<int,int>>().swap(links);
    vector<int>().swap(cursor);
    Blossom matching(offsets, adjacency);
    for (auto center : centers) {
        int a = center.a;
        matching.match[a] = a + 1;
        matching.match[a + 1] = a;
    }
    mt19937 rng(712367821);
    vector<int> order(centers.size());
    iota(order.begin(), order.end(), 0);
    shuffle(order.begin(), order.end(), rng);
    for (int index : order) {
        auto center = centers[index];
        int a = center.a, first = -1, second = -1;
        int neighbors[4], count = 0;
        for (int e = offsets[a]; e < offsets[a + 1]; ++e) {
            int d = adjacency[e];
            if (d != a + 1 && matching.match[d] == -1) neighbors[count++] = d;
        }
        if (count) first = neighbors[rng() % count];
        count = 0;
        for (int e = offsets[a + 1]; e < offsets[a + 2]; ++e) {
            int d = adjacency[e];
            if (d != a && d != first && matching.match[d] == -1) neighbors[count++] = d;
        }
        if (count) second = neighbors[rng() % count];
        if (first != -1 && second != -1) {
            matching.match[a] = first;
            matching.match[first] = a;
            matching.match[a + 1] = second;
            matching.match[second] = a + 1;
        }
    }
    shuffle(roots.begin(), roots.end(), rng);
    matching.solve(roots);

    vector<int> position(vertices, -1);
    for (int p = 0; p < cells; ++p) if (id[p] != -1) position[id[p]] = p;
    // A tromino has eight boundary edges, so greedy coloring needs at most
    // nine letters, even when all its neighbors have different labels.
    const int dr[4] = {-1, 1, 0, 0}, dc[4] = {0, 0, -1, 1};
    for (auto center : centers) {
        int a = center.a;
        if (matching.match[a] == a + 1) continue;
        int occupied[3] = {center.pos, position[matching.match[a]], position[matching.match[a + 1]]};
        unsigned forbidden = 0;
        for (int p : occupied) {
            int r = p / m, c = p % m;
            for (int k = 0; k < 4; ++k) {
                int rr = r + dr[k], cc = c + dc[k];
                if (rr >= 0 && rr < n && cc >= 0 && cc < m && board[rr][cc] >= 'a' && board[rr][cc] <= 'z')
                    forbidden |= 1u << (board[rr][cc] - 'a');
            }
        }
        int color = 0;
        while (forbidden & (1u << color)) ++color;
        for (int p : occupied) board[p / m][p % m] = char('a' + color);
    }
    for (auto &s : board) cout << s << '\n';
    return 0;
}

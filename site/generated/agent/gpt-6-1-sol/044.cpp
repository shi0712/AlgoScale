#include <bits/stdc++.h>
using namespace std;

using Mask = uint64_t;

struct Random {
    uint64_t state = 19260817;
    unsigned next() {
        state ^= state << 7;
        state ^= state >> 9;
        return static_cast<unsigned>(state);
    }
    int operator()(int bound) { return next() % bound; }
} rnd;

class MapBuilder {
    int n, m, k, cells;
    vector<pair<int, int>> edges;
    Mask allowed[40]{};
    int edgeId[40][40]{}, distanceTo[40][40]{};
    vector<int> board, edgeCount;
    vector<array<int, 4>> neighbors;
    vector<int> neighborCount;
    vector<int> walk, order;
    int missing = 0;

    void dfs(int v, Mask &seen) {
        seen |= Mask(1) << v;
        walk.push_back(v);
        for (int u : order) {
            if ((allowed[v] & (Mask(1) << u)) && !(seen & (Mask(1) << u))) {
                dfs(u, seen);
                walk.push_back(v);
            }
        }
    }

    // Preserve both legality and at least one witness of every realized edge.
    bool recolor(int pos, int color) {
        int old = board[pos];
        if (old == color) return false;
        int ids[8], delta[8], used = 0;
        auto add = [&](int id, int amount) {
            for (int j = 0; j < used; ++j) {
                if (ids[j] == id) {
                    delta[j] += amount;
                    return;
                }
            }
            ids[used] = id;
            delta[used++] = amount;
        };
        for (int j = 0; j < neighborCount[pos]; ++j) {
            int other = board[neighbors[pos][j]];
            if (!(allowed[color] & (Mask(1) << other))) return false;
            if (old != other) add(edgeId[old][other], -1);
            if (color != other) add(edgeId[color][other], 1);
        }
        for (int j = 0; j < used; ++j) {
            if (delta[j] < 0 && edgeCount[ids[j]] + delta[j] == 0)
                return false;
        }
        for (int j = 0; j < used; ++j) {
            int id = ids[j];
            if (edgeCount[id] == 0 && delta[j] > 0) --missing;
            edgeCount[id] += delta[j];
        }
        board[pos] = color;
        return true;
    }

    void initialize() {
        for (int i = n - 1; i > 0; --i) swap(order[i], order[rnd(i + 1)]);
        walk.clear();
        Mask seen = 0;
        dfs(rnd(n), seen);

        // A closed DFS walk has 2N-1 entries. Repeating entries is harmless.
        // Spend the 2(K-N) extra entries on countries covering many chords.
        vector<char> chord(m, true);
        for (int i = 1; i < int(walk.size()); ++i)
            chord[edgeId[walk[i - 1]][walk[i]]] = false;
        vector<int> selected(n, 0);
        for (int t = 0; t < k - n; ++t) {
            int best = -1, bestScore = -1;
            for (int v = 0; v < n; ++v) {
                if (selected[v]) continue;
                int score = 0;
                for (int e = 0; e < m; ++e)
                    if (chord[e] && (edges[e].first == v || edges[e].second == v))
                        ++score;
                if (score > bestScore || (score == bestScore && rnd(3) == 0)) {
                    best = v;
                    bestScore = score;
                }
            }
            ++selected[best];
            for (int e = 0; e < m; ++e)
                if (edges[e].first == best || edges[e].second == best)
                    chord[e] = false;
        }
        vector<int> repeat(walk.size(), 0);
        for (int v = 0; v < n; ++v) {
            if (!selected[v]) continue;
            int bestPos = -1, bestScore = -1;
            for (int i = 0; i < int(walk.size()); ++i) {
                if (walk[i] != v) continue;
                int score = min(i + 1, 2 * k - 1 - i);
                if (score > bestScore || (score == bestScore && rnd(2))) {
                    bestPos = i;
                    bestScore = score;
                }
            }
            repeat[bestPos] += 2 * selected[v];
        }
        vector<int> extended;
        for (int i = 0; i < int(walk.size()); ++i)
            for (int j = 0; j <= repeat[i]; ++j)
                extended.push_back(walk[i]);
        walk.swap(extended);

        // Side neighbors have consecutive walk indices, hence allowed colors.
        for (int r = 0; r < k; ++r)
            for (int c = 0; c < k; ++c)
                board[r * k + c] = walk[r + c];
        fill(edgeCount.begin(), edgeCount.end(), 0);
        for (int p = 0; p < cells; ++p) {
            for (int j = 0; j < neighborCount[p]; ++j) {
                int q = neighbors[p][j];
                if (p < q && board[p] != board[q])
                    ++edgeCount[edgeId[board[p]][board[q]]];
            }
        }
        missing = count(edgeCount.begin(), edgeCount.end(), 0);
    }

    bool refine() {
        initialize();
        int steps = 0, lastProgress = 0, previousMissing = missing;
        Mask allColors = (Mask(1) << n) - 1;
        while (missing) {
            vector<int> absent;
            for (int e = 0; e < m; ++e)
                if (edgeCount[e] == 0) absent.push_back(e);
            int target = absent[rnd(absent.size())];
            auto [u, v] = edges[target];
            if (rnd(2)) swap(u, v);
            for (int it = 0; it < 20 * cells && !edgeCount[target]; ++it) {
                ++steps;
                if (missing != previousMissing) {
                    previousMissing = missing;
                    lastProgress = steps;
                }
                if (steps - lastProgress > 5 * cells || steps > 60 * cells)
                    return false;
                if (it == 2 * cells) swap(u, v);
                int pos = rnd(cells);
                Mask candidates = allColors;
                bool touchesV = false;
                for (int j = 0; j < neighborCount[pos]; ++j) {
                    int color = board[neighbors[pos][j]];
                    candidates &= allowed[color];
                    touchesV |= color == v;
                }
                if (touchesV && (candidates & (Mask(1) << u)) && recolor(pos, u))
                    continue;

                // Enlarge space for v, without destroying any existing witness.
                int chosen = board[pos];
                int best = distanceTo[chosen][v];
                for (Mask bits = candidates; bits; bits &= bits - 1) {
                    int color = __builtin_ctzll(bits);
                    int d = distanceTo[color][v];
                    if (d < best || (d == best && rnd(3) == 0)) {
                        best = d;
                        chosen = color;
                    }
                }
                recolor(pos, chosen);
            }
        }
        return true;
    }

    bool verify() const {
        vector<char> found(m, false);
        Mask present = 0;
        for (int p = 0; p < cells; ++p) {
            present |= Mask(1) << board[p];
            for (int j = 0; j < neighborCount[p]; ++j) {
                int q = neighbors[p][j];
                if (!(allowed[board[p]] & (Mask(1) << board[q]))) return false;
                if (board[p] != board[q]) found[edgeId[board[p]][board[q]]] = true;
            }
        }
        return present == (Mask(1) << n) - 1 &&
               count(found.begin(), found.end(), true) == m;
    }

public:
    void run() {
        cin >> n >> m;
        k = 5 * n / 4;
        cells = k * k;
        edges.resize(m);
        for (int i = 0; i < n; ++i) {
            allowed[i] = Mask(1) << i;
            for (int j = 0; j < n; ++j) distanceTo[i][j] = (i == j ? 0 : 100);
        }
        for (int e = 0; e < m; ++e) {
            auto &[u, v] = edges[e];
            cin >> u >> v;
            --u; --v;
            allowed[u] |= Mask(1) << v;
            allowed[v] |= Mask(1) << u;
            edgeId[u][v] = edgeId[v][u] = e;
            distanceTo[u][v] = distanceTo[v][u] = 1;
        }
        for (int t = 0; t < n; ++t)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    distanceTo[i][j] = min(distanceTo[i][j], distanceTo[i][t] + distanceTo[t][j]);
        board.resize(cells);
        neighbors.resize(cells);
        neighborCount.assign(cells, 0);
        edgeCount.resize(m);
        order.resize(n);
        iota(order.begin(), order.end(), 0);
        for (int r = 0; r < k; ++r) {
            for (int c = 0; c < k; ++c) {
                int p = r * k + c;
                auto add = [&](int q) { neighbors[p][neighborCount[p]++] = q; };
                if (r) add(p - k);
                if (r + 1 < k) add(p + k);
                if (c) add(p - 1);
                if (c + 1 < k) add(p + 1);
            }
        }
        while (!refine() || !verify()) {}
        cout << k << '\n';
        for (int r = 0; r < k; ++r) {
            for (int c = 0; c < k; ++c)
                cout << board[r * k + c] + 1 << (c + 1 == k ? '\n' : ' ');
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int tests;
    cin >> tests;
    while (tests--) {
        MapBuilder builder;
        builder.run();
    }
    return 0;
}

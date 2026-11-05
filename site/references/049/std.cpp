#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>
using namespace std;

namespace {

using Cell = uint16_t;
constexpr Cell NONE = UINT16_MAX;

class EdgeColoring {
    int n, k;
    vector<vector<Cell>> color, to, missing, position;

    void occupy(int v, int c) {
        int p = position[v][c];
        int last = missing[v].back();
        assert(p != NONE);
        missing[v][p] = static_cast<Cell>(last);
        position[v][last] = static_cast<Cell>(p);
        missing[v].pop_back();
        position[v][c] = NONE;
    }

    void release(int v, int c) {
        assert(position[v][c] == NONE);
        position[v][c] = static_cast<Cell>(missing[v].size());
        missing[v].push_back(static_cast<Cell>(c));
    }

    void eraseEdge(int u, int v) {
        int c = color[u][v];
        assert(c != NONE);
        color[u][v] = color[v][u] = NONE;
        to[u][c] = to[v][c] = NONE;
        release(u, c);
        release(v, c);
    }

    // Color a/b are swapped on the maximal alternating path starting at u.
    // Color a is missing at u before the operation.
    void swapAlternatingPath(int u, int a, int b) {
        vector<array<int, 3>> path;
        int x = u, c = b;
        while (to[x][c] != NONE) {
            int y = to[x][c];
            path.push_back({x, y, c});
            x = y;
            c = a ^ b ^ c;
        }
        for (const auto &e : path) {
            eraseEdge(e[0], e[1]);
        }
        for (const auto &e : path) {
            paint(e[0], e[1], a ^ b ^ e[2]);
        }
    }

public:
    EdgeColoring(int n_, int k_)
        : n(n_), k(k_), color(n, vector<Cell>(n, NONE)),
          to(n, vector<Cell>(k, NONE)), missing(n, vector<Cell>(k)),
          position(n, vector<Cell>(k)) {
        for (int v = 0; v < n; ++v) {
            iota(missing[v].begin(), missing[v].end(), Cell(0));
            iota(position[v].begin(), position[v].end(), Cell(0));
        }
    }

    void paint(int u, int v, int c) {
        assert(color[u][v] == NONE);
        occupy(u, c);
        occupy(v, c);
        color[u][v] = color[v][u] = static_cast<Cell>(c);
        to[u][c] = static_cast<Cell>(v);
        to[v][c] = static_cast<Cell>(u);
    }

    // Inserts and colors one new edge in O(n) using a Vizing fan.
    void insertEdge(int u, int v) {
        vector<int> fan{v};
        vector<int> where(n, -1);
        where[v] = 0;

        int beta = -1, last = -1;
        while (true) {
            assert(!missing[fan.back()].empty());
            beta = missing[fan.back()].back();
            int w = to[u][beta];

            if (w == NONE) {
                last = static_cast<int>(fan.size()) - 1;
                break;
            }
            if (where[w] != -1) {
                int j = where[w];
                assert(j > 0 && !missing[u].empty());
                int alpha = missing[u].back();
                swapAlternatingPath(u, alpha, beta);
                last = (to[fan[j - 1]][beta] == NONE)
                           ? j - 1
                           : static_cast<int>(fan.size()) - 1;
                break;
            }
            where[w] = static_cast<int>(fan.size());
            fan.push_back(w);
        }

        for (int i = 0; i < last; ++i) {
            int c = color[u][fan[i + 1]];
            eraseEdge(u, fan[i + 1]);
            paint(u, fan[i], c);
        }
        paint(u, fan[last], beta);
    }

    int oneMissingColor(int v) const {
        assert(missing[v].size() == 1);
        return missing[v].back();
    }

    const vector<vector<Cell>> &result() const { return color; }
};

vector<vector<Cell>> solve(const vector<string> &graph) {
    int n = static_cast<int>(graph.size());
    int colors = (n & 1) ? n - 2 : n - 3;
    EdgeColoring ec(n, colors);

    int center = 0;
    int reserved = -1;
    if ((n & 1) == 0) {
        vector<int> nonNeighbors;
        for (int v = 1; v < n; ++v) {
            if (graph[0][v] == '0') nonNeighbors.push_back(v);
        }
        assert(nonNeighbors.size() == 2);
        center = nonNeighbors[0];
        reserved = nonNeighbors[1];
    }

    vector<int> core;
    for (int v = 0; v < n; ++v) {
        if (v != center && ((n & 1) || v != 0)) core.push_back(v);
    }
    assert(static_cast<int>(core.size()) == colors + 1);

    // A 1-factorization of K_(colors+1), with the last vertex treated as infinity.
    // Removing absent edges preserves a valid partial coloring of the core.
    for (int i = 0; i <= colors; ++i) {
        for (int j = i + 1; j <= colors; ++j) {
            if (graph[core[i]][core[j]] == '0') continue;
            int c = (j == colors) ? (2 * i) % colors : (i + j) % colors;
            ec.paint(core[i], core[j], c);
        }
    }

    // In the even case, center-reserved is inserted last if it is an edge.
    for (int v : core) {
        if (v != reserved && graph[center][v] == '1') ec.insertEdge(center, v);
    }
    if (reserved != -1 && graph[center][reserved] == '1') {
        ec.insertEdge(center, reserved);
    }

    // For even n, every color is now missing at exactly one neighbor of vertex 0.
    if ((n & 1) == 0) {
        for (int v = 1; v < n; ++v) {
            if (graph[0][v] == '1') ec.paint(0, v, ec.oneMissingColor(v));
        }
    }
    return ec.result();
}

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    vector<string> graph(n);
    for (string &row : graph) cin >> row;

    int colors = (n & 1) ? n - 2 : n - 3;
    auto answer = solve(graph);

    cout << colors << '\n';
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int value = answer[i][j] == NONE ? -1 : answer[i][j] + 1;
            cout << value << (j + 1 == n ? '\n' : ' ');
        }
    }
    return 0;
}

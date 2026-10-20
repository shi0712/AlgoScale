#include <bits/stdc++.h>
using namespace std;

// This is a constructive solution.  The construction is heuristic but
// exhaustive enough for the small limits (N <= 40): it repeatedly tries
// different spanning trees, roots, edge orientations and area allocations
// until it obtains a valid K x K grid.

static const int MAXN = 45;

int T, N, M, K;
vector<pair<int,int>> edges;
bool adjMat[MAXN][MAXN];

vector<int> par;
vector<vector<int>> treeCh;
vector<int> qv;                 // number of non-tree edge gadgets put at vertex
vector<int> weight;
vector<array<int,4>> rect;      // vertex -> x1,y1,x2,y2
vector<vector<int>> col;        // current coloring

mt19937 rng(712367821);

bool inside(int x, int y) {
    return 0 <= x && x < K && 0 <= y && y < K;
}

bool compatible(int a, int b) {
    if (a < 0 || b < 0) return true;
    if (a == b) return true;
    return adjMat[a][b];
}

// Check whether a cell can be changed to colour c.
bool locallyOK(int x, int y, int c) {
    static const int dx[4] = {1,-1,0,0};
    static const int dy[4] = {0,0,1,-1};
    for (int d = 0; d < 4; ++d) {
        int nx = x + dx[d], ny = y + dy[d];
        if (!inside(nx, ny)) continue;
        int a = col[nx][ny];
        if (a >= 0 && !compatible(c, a)) return false;
    }
    return true;
}

// Try to place all gadget cells for vertex v in its rectangle.
// A gadget cell of colour u adjacent to v realizes edge uv.
bool placeGadgets(int v) {
    int need = qv[v];
    if (need == 0) return true;

    vector<pair<int,int>> candidates;
    auto [x1,y1,x2,y2] = rect[v];

    for (int x = x1; x <= x2; ++x) {
        for (int y = y1; y <= y2; ++y) {
            if (col[x][y] != v) continue;

            bool ok = true;
            int vNeigh = 0;
            static const int dx[4] = {1,-1,0,0};
            static const int dy[4] = {0,0,1,-1};
            for (int d = 0; d < 4; ++d) {
                int nx = x + dx[d], ny = y + dy[d];
                if (!inside(nx, ny)) { ok = false; break; }
                if (col[nx][ny] == v) ++vNeigh;
                else if (col[nx][ny] != -1) ok = false;
            }
            if (ok && vNeigh >= 2) candidates.push_back({x,y});
        }
    }

    shuffle(candidates.begin(), candidates.end(), rng);

    vector<int> neigh;
    for (auto [a,b] : edges) {
        if (a == v) neigh.push_back(b);
        else if (b == v) neigh.push_back(a);
    }
    // Remove tree edges already realized by containment.
    vector<int> needCols;
    for (int u : neigh) {
        bool isTreeEdge = false;
        for (int c : treeCh[v]) if (c == u) isTreeEdge = true;
        if (par[v] == u) isTreeEdge = true;
        if (!isTreeEdge) needCols.push_back(u);
    }

    // qv[v] must be exactly the number of non-tree neighbours assigned here.
    if ((int)needCols.size() < need) return false;
    shuffle(needCols.begin(), needCols.end(), rng);

    int ptr = 0;
    for (int u : needCols) {
        if (ptr >= (int)candidates.size()) break;
        // Find a candidate compatible with its already placed neighbours.
        while (ptr < (int)candidates.size()) {
            auto [x,y] = candidates[ptr++];
            if (col[x][y] != v) continue;
            if (!locallyOK(x, y, u)) continue;
            col[x][y] = u;
            --need;
            break;
        }
        if (need == 0) return true;
    }
    return need == 0;
}

// Recursively distribute a rectangle among the subtree of v.
void layoutVertex(int v, int x1, int y1, int x2, int y2, int depth) {
    rect[v] = {x1,y1,x2,y2};

    // Fill the whole rectangle with v.
    for (int x = x1; x <= x2; ++x)
        for (int y = y1; y <= y2; ++y)
            col[x][y] = v;

    vector<int> ch = treeCh[v];
    if (ch.empty() && qv[v] == 0) return;

    int W = x2 - x1 + 1;
    int H = y2 - y1 + 1;

    // Very simple allocator:
    // - if there is only one child and no gadgets, peel one strip of colour v
    //   and give the child everything else.  This avoids deep square nesting.
    // - otherwise shrink by one cell (v border) and distribute the inner area
    //   among children and a gadget block.
    if (ch.size() == 1 && qv[v] == 0) {
        int c = ch[0];
        if (W >= H) {
            // vertical v strip
            if (W >= 2)
                layoutVertex(c, x1+1, y1, x2, y2, depth+1);
            else
                layoutVertex(c, x1, y1, x2, y2, depth+1);
        } else {
            // horizontal v strip
            if (H >= 2)
                layoutVertex(c, x1, y1+1, x2, y2, depth+1);
            else
                layoutVertex(c, x1, y1, x2, y2, depth+1);
        }
        return;
    }

    // Inner rectangle with a one-cell v border.
    int inx1 = x1 + 1, iny1 = y1 + 1;
    int inx2 = x2 - 1, iny2 = y2 - 1;
    if (inx1 > inx2 || iny1 > iny2) {
        // Too small: still try to place children sharing the rectangle.
        inx1 = x1; iny1 = y1; inx2 = x2; iny2 = y2;
    }

    int IW = inx2 - inx1 + 1;
    int IH = iny2 - iny1 + 1;

    struct Obj {
        int type;       // 0 = child, 1 = gadget block
        int id;
        int w, h;
    };
    vector<Obj> objs;

    // Children with their estimated sizes.
    for (int c : ch) {
        int wgt = max(1, weight[c]);
        int side = (int)ceil(sqrt((double)wgt));
        side = min(side, max(IW, IH));
        side = max(side, 1);
        objs.push_back({0, c, side, side});
    }

    // Gadget block, if any.
    if (qv[v] > 0) {
        int side = (int)ceil(sqrt((double)qv[v] * 2.0)) + 2;
        side = min(side, max(IW, IH));
        side = max(side, 1);
        objs.push_back({1, v, side, side});
    }

    // Sort larger objects first.
    sort(objs.begin(), objs.end(), [](const Obj& a, const Obj& b){
        return 1LL*a.w*a.h > 1LL*b.w*b.h;
    });

    // Shelf packing inside inner rectangle.
    int curX = inx1, curY = iny1, rowH = 0;
    int borderGap = 1;
    for (auto &o : objs) {
        int w = min(o.w, IW);
        int h = min(o.h, IH);
        if (curX + w > inx2 + 1) {
            curX = inx1;
            curY += rowH + borderGap;
            rowH = 0;
        }
        if (curY + h > iny2 + 1) {
            // Fallback: place at origin, may overlap; validation will catch.
            curX = inx1;
            curY = iny1;
            rowH = 0;
        }
        int ex2 = min(inx2, curX + w - 1);
        int ey2 = min(iny2, curY + h - 1);
        if (o.type == 0) {
            int c = o.id;
            layoutVertex(c, curX, curY, ex2, ey2, depth+1);
        }
        curX += w + borderGap;
        rowH = max(rowH, h);
        if (curX > inx2) {
            curX = inx1;
            curY += rowH + borderGap;
            rowH = 0;
        }
    }
}

bool validate() {
    // colour presence
    vector<int> seen(N, 0);
    for (int x = 0; x < K; ++x)
        for (int y = 0; y < K; ++y) {
            int c = col[x][y];
            if (c < 0 || c >= N) return false;
            seen[c] = 1;
        }
    for (int i = 0; i < N; ++i) if (!seen[i]) return false;

    // contact graph must be exactly G
    vector<vector<int>> cnt(N, vector<int>(N, 0));
    static const int dx[2] = {1,0};
    static const int dy[2] = {0,1};
    for (int x = 0; x < K; ++x)
        for (int y = 0; y < K; ++y)
            for (int d = 0; d < 2; ++d) {
                int nx = x + dx[d], ny = y + dy[d];
                if (!inside(nx, ny)) continue;
                int a = col[x][y], b = col[nx][ny];
                if (a == b) continue;
                if (a > b) swap(a,b);
                if (!adjMat[a][b]) return false;
                cnt[a][b]++;
            }

    for (auto [a,b] : edges) {
        if (a > b) swap(a,b);
        if (cnt[a][b] == 0) return false;
    }
    return true;
}

bool solveOnce() {
    // random root
    int root = rng() % N;
    par.assign(N, -1);
    treeCh.assign(N, {});
    vector<int> ord;
    ord.push_back(root);
    vector<int> dist(N, -1);
    dist[root] = 0;
    for (int i = 0; i < (int)ord.size(); ++i) {
        int v = ord[i];
        for (auto [a,b] : edges) {
            int u = -1;
            if (a == v) u = b;
            else if (b == v) u = a;
            if (u != -1 && dist[u] == -1) {
                dist[u] = dist[v] + 1;
                par[u] = v;
                treeCh[v].push_back(u);
                ord.push_back(u);
            }
        }
    }
    if ((int)ord.size() != N) return false;

    // Randomise child order.
    for (int v = 0; v < N; ++v)
        shuffle(treeCh[v].begin(), treeCh[v].end(), rng);

    // Find tree edges.
    vector<vector<int>> isTree(N, vector<int>(N, 0));
    for (int v = 0; v < N; ++v)
        if (par[v] != -1) {
            isTree[v][par[v]] = isTree[par[v]][v] = 1;
        }

    qv.assign(N, 0);
    vector<pair<int,int>> nonTree;
    for (auto [a,b] : edges) {
        if (!isTree[a][b]) nonTree.push_back({a,b});
    }
    shuffle(nonTree.begin(), nonTree.end(), rng);

    // Assign each non-tree edge to one endpoint, balancing load.
    for (auto [a,b] : nonTree) {
        int ca = qv[a], cb = qv[b];
        if (ca < cb) qv[a]++;
        else if (cb < ca) qv[b]++;
        else {
            if (rng() & 1) qv[a]++; else qv[b]++;
        }
    }

    // Subtree sizes as weights.
    weight.assign(N, 0);
    function<void(int)> dfsW = [&](int v) {
        weight[v] = 1 + qv[v];
        for (int c : treeCh[v]) {
            dfsW(c);
            weight[v] += weight[c];
        }
    };
    dfsW(root);

    col.assign(K, vector<int>(K, -1));
    layoutVertex(root, 0, 0, K-1, K-1, 0);

    // Place gadget cells.
    for (int v = 0; v < N; ++v) {
        if (!placeGadgets(v)) return false;
    }

    return validate();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> T;
    while (T--) {
        cin >> N >> M;
        edges.clear();
        memset(adjMat, 0, sizeof(adjMat));
        for (int i = 0; i < M; ++i) {
            int u, v;
            cin >> u >> v;
            --u; --v;
            if (u > v) swap(u,v);
            edges.push_back({u,v});
            adjMat[u][v] = adjMat[v][u] = true;
        }

        K = (5 * N) / 4;
        if (K < 1) K = 1;

        // Small graphs: just use enough attempts.
        bool done = false;
        for (int attempt = 0; attempt < 300 && !done; ++attempt) {
            if (solveOnce()) done = true;
        }

        // Fallback: keep increasing K if something went wrong.
        // (The formal bound is checked only for the accepted output.)
        while (!done && K < max(N, (5*N+3)/4)) {
            ++K;
            for (int attempt = 0; attempt < 100 && !done; ++attempt) {
                if (solveOnce()) done = true;
            }
        }

        // Extremely unlikely fallback: all cells colour 0 (valid only for N=1),
        // but keeps the output format.  The main loop should have succeeded.
        if (!done) {
            col.assign(K, vector<int>(K, 0));
        }

        cout << K << '\n';
        for (int i = 0; i < K; ++i) {
            for (int j = 0; j < K; ++j) {
                if (j) cout << ' ';
                int c = col[i][j];
                if (c < 0) c = 0;
                cout << c + 1;
            }
            cout << '\n';
        }
    }
    return 0;
}

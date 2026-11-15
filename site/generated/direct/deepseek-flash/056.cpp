#include <bits/stdc++.h>
using namespace std;

struct Blossom {
    int n;
    vector<vector<int>> &adj;
    vector<int> match;

    vector<int> p, base;
    vector<int> pVer, baseVer, usedVer;
    vector<char> used;

    vector<int> lcaSeen;
    vector<int> blossomVer;
    int curBfs = 0, curBlossom = 0, curLca = 0;

    vector<int> q;

    Blossom(int n, vector<vector<int>> &g)
        : n(n), adj(g), match(n, -1), p(n), base(n),
          pVer(n, -1), baseVer(n, -1), usedVer(n, -1),
          used(n, false), lcaSeen(n, -1), blossomVer(n, -1) {
        q.reserve(n);
    }

    int getBase(int v) {
        if (baseVer[v] != curBfs) {
            baseVer[v] = curBfs;
            base[v] = v;
        }
        return base[v];
    }

    void setBase(int v, int b) {
        baseVer[v] = curBfs;
        base[v] = b;
    }

    int getP(int v) {
        return pVer[v] == curBfs ? p[v] : -1;
    }

    void setP(int v, int x) {
        pVer[v] = curBfs;
        p[v] = x;
    }

    bool isUsed(int v) {
        return usedVer[v] == curBfs ? used[v] : false;
    }

    void setUsed(int v, bool x) {
        usedVer[v] = curBfs;
        used[v] = x;
    }

    void markBlossom(int v) {
        blossomVer[v] = curBlossom;
    }

    bool isBlossom(int v) {
        return blossomVer[v] == curBlossom;
    }

    int lca(int a, int b) {
        ++curLca;
        while (true) {
            a = getBase(a);
            lcaSeen[a] = curLca;
            if (match[a] == -1) break;
            a = getP(match[a]);
        }
        while (true) {
            b = getBase(b);
            if (lcaSeen[b] == curLca) return b;
            b = getP(match[b]);
        }
    }

    void markPath(int v, int b, int child) {
        while (getBase(v) != b) {
            int bv = getBase(v);
            markBlossom(bv);

            int mv = match[v];
            if (mv != -1) markBlossom(getBase(mv));

            setP(v, child);
            child = mv;
            if (mv == -1) break;
            v = getP(mv);
        }
    }

    int findPath(int root) {
        ++curBfs;
        q.clear();
        q.push_back(root);
        setUsed(root, true);

        size_t head = 0;
        while (head < q.size()) {
            int v = q[head++];

            for (int u : adj[v]) {
                if (getBase(v) == getBase(u) || match[v] == u) continue;

                int mu = match[u];

                if (u == root || (mu != -1 && getP(mu) != -1)) {
                    int cur = lca(v, u);

                    ++curBlossom;
                    markPath(v, cur, u);
                    markPath(u, cur, v);

                    for (int i = 0; i < n; ++i) {
                        int bi = getBase(i);
                        if (isBlossom(bi)) {
                            setBase(i, cur);
                            if (!isUsed(i)) {
                                setUsed(i, true);
                                q.push_back(i);
                            }
                        }
                    }
                } else if (getP(u) == -1) {
                    setP(u, v);

                    if (match[u] == -1) return u;

                    int m = match[u];
                    setUsed(m, true);
                    q.push_back(m);
                }
            }
        }

        return -1;
    }

    void augment(int v) {
        while (v != -1) {
            int pv = getP(v);
            int nv = (pv == -1 ? -1 : match[pv]);

            match[v] = pv;
            if (pv != -1) match[pv] = v;

            v = nv;
        }
    }

    void solve(int dots) {
        bool changed = true;
        while (changed) {
            changed = false;

            for (int root = 0; root < dots; ++root) {
                if (match[root] == -1) {
                    int v = findPath(root);
                    if (v != -1) {
                        augment(v);
                        changed = true;
                    }
                }
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<string> board(n);
    for (int i = 0; i < n; ++i) cin >> board[i];

    const int N = n * m;

    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    vector<array<int, 3>> trominoes;

    for (int par = 0; par < 2; ++par) {
        int dotParity = 1 - par;

        vector<int> dotId(N, -1);
        vector<int> dotCell;

        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < m; ++c) {
                if (board[r][c] == '.' && ((r + c) & 1) == dotParity) {
                    int id = (int)dotCell.size();
                    int cell = r * m + c;
                    dotId[cell] = id;
                    dotCell.push_back(cell);
                }
            }
        }

        int D = (int)dotCell.size();

        struct Center {
            int cell;
            char type;
            int c1, c2;
        };

        vector<Center> centers;

        auto isDot = [&](int r, int c) -> bool {
            return 0 <= r && r < n && 0 <= c && c < m && board[r][c] == '.';
        };

        for (int r = 0; r < n; ++r) {
            for (int c = 0; c < m; ++c) {
                if (board[r][c] == '.') continue;
                if (((r + c) & 1) != par) continue;

                bool active = false;

                if (board[r][c] == '+') {
                    int cnt = 0;
                    for (int d = 0; d < 4; ++d) {
                        if (isDot(r + dr[d], c + dc[d])) ++cnt;
                    }
                    active = (cnt >= 2);
                } else { // '*'
                    bool vert = isDot(r - 1, c) || isDot(r + 1, c);
                    bool horz = isDot(r, c - 1) || isDot(r, c + 1);
                    active = (vert && horz);
                }

                if (active) {
                    int idx = (int)centers.size();
                    int cell = r * m + c;
                    int c1 = D + 2 * idx;
                    int c2 = c1 + 1;
                    centers.push_back({cell, board[r][c], c1, c2});
                }
            }
        }

        int C = (int)centers.size();
        if (C == 0) continue;

        int V = D + 2 * C;
        vector<vector<int>> adj(V);

        auto addEdge = [&](int a, int b) {
            adj[a].push_back(b);
            adj[b].push_back(a);
        };

        for (int idx = 0; idx < C; ++idx) {
            int cell = centers[idx].cell;
            int r = cell / m;
            int c = cell % m;
            int c1 = centers[idx].c1;
            int c2 = centers[idx].c2;

            addEdge(c1, c2);

            if (centers[idx].type == '+') {
                for (int d = 0; d < 4; ++d) {
                    int nr = r + dr[d];
                    int nc = c + dc[d];
                    if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
                    if (board[nr][nc] != '.') continue;

                    int id = dotId[nr * m + nc];
                    if (id != -1) {
                        addEdge(c1, id);
                        addEdge(c2, id);
                    }
                }
            } else { // '*'
                // vertical neighbours go to copy c1
                for (int d = 0; d < 2; ++d) { // up, down
                    int nr = r + dr[d];
                    int nc = c + dc[d];
                    if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
                    if (board[nr][nc] != '.') continue;

                    int id = dotId[nr * m + nc];
                    if (id != -1) addEdge(c1, id);
                }

                // horizontal neighbours go to copy c2
                for (int d = 2; d < 4; ++d) { // left, right
                    int nr = r + dr[d];
                    int nc = c + dc[d];
                    if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
                    if (board[nr][nc] != '.') continue;

                    int id = dotId[nr * m + nc];
                    if (id != -1) addEdge(c2, id);
                }
            }
        }

        vector<int> score(C);
        for (int i = 0; i < C; ++i) {
            int c1 = centers[i].c1;
            int c2 = centers[i].c2;

            int deg1 = 0, deg2 = 0;
            for (int v : adj[c1]) if (v < D) ++deg1;
            for (int v : adj[c2]) if (v < D) ++deg2;

            if (centers[i].type == '+') {
                score[i] = deg1 * (deg1 - 1) / 2;
            } else {
                score[i] = deg1 * deg2;
            }
        }

        vector<vector<int>> orders;

        vector<int> order(C);
        iota(order.begin(), order.end(), 0);
        orders.push_back(order);

        reverse(order.begin(), order.end());
        orders.push_back(order);

        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int a, int b) {
            if (score[a] != score[b]) return score[a] < score[b];
            return centers[a].cell < centers[b].cell;
        });
        orders.push_back(order);

        sort(order.begin(), order.end(), [&](int a, int b) {
            if (score[a] != score[b]) return score[a] > score[b];
            return centers[a].cell < centers[b].cell;
        });
        orders.push_back(order);

        iota(order.begin(), order.end(), 0);
        sort(order.begin(), order.end(), [&](int a, int b) {
            int ra = centers[a].cell / m, ca = centers[a].cell % m;
            int rb = centers[b].cell / m, cb = centers[b].cell % m;
            if (ca != cb) return ca < cb;
            return ra < rb;
        });
        orders.push_back(order);

        mt19937 rng(712367);
        for (int rep = 0; rep < 4; ++rep) {
            iota(order.begin(), order.end(), 0);
            shuffle(order.begin(), order.end(), rng);
            orders.push_back(order);
        }

        vector<int> bestChosen1(C, -1), bestChosen2(C, -1);
        int bestT = -1;

        for (auto &ord : orders) {
            vector<int> chosen1(C, -1), chosen2(C, -1);
            vector<char> usedDot(D, 0);

            int curT = 0;

            for (int idx : ord) {
                int c1 = centers[idx].c1;
                int c2 = centers[idx].c2;
                int d1 = -1, d2 = -1;

                if (centers[idx].type == '+') {
                    for (int u : adj[c1]) {
                        if (u < D && !usedDot[u]) {
                            d1 = u;
                            break;
                        }
                    }

                    if (d1 != -1) {
                        for (int u : adj[c2]) {
                            if (u < D && u != d1 && !usedDot[u]) {
                                d2 = u;
                                break;
                            }
                        }
                    }
                } else {
                    for (int u : adj[c1]) {
                        if (u < D && !usedDot[u]) {
                            d1 = u;
                            break;
                        }
                    }

                    for (int u : adj[c2]) {
                        if (u < D && !usedDot[u]) {
                            d2 = u;
                            break;
                        }
                    }

                    if (d1 == d2) d2 = -1;
                }

                if (d1 != -1 && d2 != -1) {
                    chosen1[idx] = d1;
                    chosen2[idx] = d2;
                    usedDot[d1] = usedDot[d2] = 1;
                    ++curT;
                }
            }

            if (curT > bestT) {
                bestT = curT;
                bestChosen1 = chosen1;
                bestChosen2 = chosen2;
            }

            int upper = min(C, D / 2);
            if (bestT == upper) break;
        }

        vector<int> bestMatch(V, -1);

        for (int i = 0; i < C; ++i) {
            if (bestChosen1[i] != -1) {
                int d1 = bestChosen1[i];
                int d2 = bestChosen2[i];
                int c1 = centers[i].c1;
                int c2 = centers[i].c2;

                bestMatch[c1] = d1;
                bestMatch[d1] = c1;
                bestMatch[c2] = d2;
                bestMatch[d2] = c2;
            }
        }

        for (int i = 0; i < C; ++i) {
            int c1 = centers[i].c1;
            int c2 = centers[i].c2;

            if (bestMatch[c1] == -1) {
                bestMatch[c1] = c2;
                bestMatch[c2] = c1;
            }
        }

        vector<int> finalMatch;

        int upper = min(C, D / 2);

        if (bestT == upper) {
            finalMatch = bestMatch;
        } else {
            Blossom blossom(V, adj);
            blossom.match = bestMatch;
            blossom.solve(D);
            finalMatch = blossom.match;
        }

        for (int i = 0; i < C; ++i) {
            int c1 = centers[i].c1;
            int c2 = centers[i].c2;

            int d1 = finalMatch[c1];
            int d2 = finalMatch[c2];

            if (d1 >= 0 && d1 < D && d2 >= 0 && d2 < D) {
                int dot1 = dotCell[d1];
                int dot2 = dotCell[d2];
                int centerCell = centers[i].cell;
                trominoes.push_back({centerCell, dot1, dot2});
            }
        }
    }

    int T = (int)trominoes.size();

    vector<int> idAt(N, -1);

    for (int i = 0; i < T; ++i) {
        for (int cell : trominoes[i]) {
            idAt[cell] = i;
        }
    }

    vector<vector<int>> adjT(T);

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            int id1 = idAt[r * m + c];
            if (id1 == -1) continue;

            if (r + 1 < n) {
                int id2 = idAt[(r + 1) * m + c];
                if (id2 != -1 && id1 != id2) {
                    adjT[id1].push_back(id2);
                    adjT[id2].push_back(id1);
                }
            }

            if (c + 1 < m) {
                int id2 = idAt[r * m + c + 1];
                if (id2 != -1 && id1 != id2) {
                    adjT[id1].push_back(id2);
                    adjT[id2].push_back(id1);
                }
            }
        }
    }

    vector<int> color(T, -1);

    for (int i = 0; i < T; ++i) {
        bool used[26] = {};

        for (int v : adjT[i]) {
            if (color[v] != -1) used[color[v]] = true;
        }

        int col = 0;
        while (col < 26 && used[col]) ++col;
        if (col == 26) col = 0; // should not happen

        color[i] = col;
    }

    vector<string> answer = board;

    for (int i = 0; i < T; ++i) {
        char ch = char('a' + color[i]);

        for (int cell : trominoes[i]) {
            int r = cell / m;
            int c = cell % m;
            answer[r][c] = ch;
        }
    }

    for (int r = 0; r < n; ++r) {
        cout << answer[r] << '\n';
    }

    return 0;
}

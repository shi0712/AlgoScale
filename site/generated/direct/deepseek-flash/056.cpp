#include <bits/stdc++.h>
using namespace std;

struct Blossom {
    int N;
    const vector<vector<int>>& g;
    vector<int> match;

    vector<int> par, base;
    vector<int> parTime, baseTime, usedTime, blossomTime, seen;

    int curBFS = 0;
    int blossomCur = 0;
    int lcaCur = 0;

    vector<int> touched;
    vector<int> bfsq;

    Blossom(const vector<vector<int>>& _g, vector<int> _match)
        : N((int)_g.size()), g(_g), match(std::move(_match)) {
        par.assign(N, 0);
        base.assign(N, 0);
        parTime.assign(N, 0);
        baseTime.assign(N, 0);
        usedTime.assign(N, 0);
        blossomTime.assign(N, 0);
        seen.assign(N, 0);

        touched.reserve(N);
        bfsq.reserve(N);
    }

    int getPar(int v) const {
        return parTime[v] == curBFS ? par[v] : -1;
    }

    void setPar(int v, int p) {
        par[v] = p;
        parTime[v] = curBFS;
    }

    bool used(int v) const {
        return usedTime[v] == curBFS;
    }

    void use(int v) {
        usedTime[v] = curBFS;
    }

    int getBase(int v) {
        if (baseTime[v] != curBFS) {
            baseTime[v] = curBFS;
            base[v] = v;
            touched.push_back(v);
        }
        return base[v];
    }

    void setBase(int v, int b) {
        if (baseTime[v] != curBFS) {
            baseTime[v] = curBFS;
            base[v] = v;
            touched.push_back(v);
        }
        base[v] = b;
    }

    int lca(int a, int b) {
        int stamp = ++lcaCur;

        while (true) {
            a = getBase(a);
            seen[a] = stamp;
            if (match[a] == -1) break;
            a = getPar(match[a]);
        }

        while (true) {
            b = getBase(b);
            if (seen[b] == stamp) return b;
            b = getPar(match[b]);
        }
    }

    void markPath(int v, int b, int child) {
        while (getBase(v) != b) {
            int bv = getBase(v);
            int mv = match[v];
            int bm = getBase(mv);

            blossomTime[bv] = blossomCur;
            blossomTime[bm] = blossomCur;

            setPar(v, child);
            child = mv;
            v = getPar(mv);
        }
    }

    int findPath(int root) {
        ++curBFS;
        touched.clear();
        bfsq.clear();

        getBase(root);
        setPar(root, -1);
        use(root);
        bfsq.push_back(root);

        size_t head = 0;

        while (head < bfsq.size()) {
            int v = bfsq[head++];

            for (int to : g[v]) {
                int bv = getBase(v);
                int bt = getBase(to);

                if (bv == bt || match[v] == to) continue;

                if (to == root ||
                    (match[to] != -1 && getPar(match[to]) != -1)) {
                    ++blossomCur;

                    int cb = lca(v, to);
                    markPath(v, cb, to);
                    markPath(to, cb, v);

                    for (size_t i = 0; i < touched.size(); ++i) {
                        int x = touched[i];
                        int bx = getBase(x);

                        if (blossomTime[bx] == blossomCur) {
                            setBase(x, cb);

                            if (!used(x)) {
                                use(x);
                                bfsq.push_back(x);
                            }
                        }
                    }
                } else if (getPar(to) == -1) {
                    setPar(to, v);

                    if (match[to] == -1) {
                        return to;
                    }

                    int nxt = match[to];
                    use(nxt);
                    bfsq.push_back(nxt);
                }
            }
        }

        return -1;
    }
};

struct CenterInfo {
    int r, c;
    int a, b;
    bool plus;
    vector<int> neighA;
    vector<int> neighB;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    cin >> n >> m;

    vector<string> board(n);
    for (int i = 0; i < n; ++i) cin >> board[i];

    int cells = n * m;
    const int dr[4] = {-1, 1, 0, 0};
    const int dc[4] = {0, 0, -1, 1};

    vector<vector<int>> g;
    vector<char> isDot;
    vector<int> cellOfVertex;
    vector<int> dotDegree;

    vector<int> dotVid(cells, -1);
    vector<int> cloneA(cells, -1), cloneB(cells, -1);

    vector<CenterInfo> centers;

    auto addVertex = [&](bool dot, int cell) -> int {
        int id = (int)g.size();
        g.push_back({});
        isDot.push_back(dot);
        cellOfVertex.push_back(cell);
        dotDegree.push_back(0);
        return id;
    };

    // Create clones for usable centers.
    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            char ch = board[r][c];
            if (ch == '.') continue;

            int cntAll = 0, cntH = 0, cntV = 0;

            for (int d = 0; d < 4; ++d) {
                int nr = r + dr[d];
                int nc = c + dc[d];

                if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
                if (board[nr][nc] != '.') continue;

                ++cntAll;
                if (dr[d] == 0) ++cntH;
                else ++cntV;
            }

            bool usable;
            if (ch == '+') {
                usable = (cntAll >= 2);
            } else {
                usable = (cntH >= 1 && cntV >= 1);
            }

            if (!usable) continue;

            int a = addVertex(false, -1);
            int b = addVertex(false, -1);

            g[a].push_back(b);
            g[b].push_back(a);

            int cell = r * m + c;
            cloneA[cell] = a;
            cloneB[cell] = b;

            CenterInfo ci;
            ci.r = r;
            ci.c = c;
            ci.a = a;
            ci.b = b;
            ci.plus = (ch == '+');
            centers.push_back(std::move(ci));
        }
    }

    // Add edges to dot vertices.
    for (auto& ci : centers) {
        int r = ci.r;
        int c = ci.c;

        for (int d = 0; d < 4; ++d) {
            int nr = r + dr[d];
            int nc = c + dc[d];

            if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;
            if (board[nr][nc] != '.') continue;

            int cell = nr * m + nc;
            int vid = dotVid[cell];

            if (vid == -1) {
                vid = addVertex(true, cell);
                dotVid[cell] = vid;
            }

            ++dotDegree[vid];

            if (ci.plus) {
                // Both clones may use any neighboring dot.
                g[ci.a].push_back(vid);
                g[vid].push_back(ci.a);

                g[ci.b].push_back(vid);
                g[vid].push_back(ci.b);

                ci.neighA.push_back(vid);
            } else {
                // Star center: clone A gets horizontal dots,
                // clone B gets vertical dots.
                if (dr[d] == 0) {
                    g[ci.a].push_back(vid);
                    g[vid].push_back(ci.a);
                    ci.neighA.push_back(vid);
                } else {
                    g[ci.b].push_back(vid);
                    g[vid].push_back(ci.b);
                    ci.neighB.push_back(vid);
                }
            }
        }
    }

    int V = (int)g.size();

    if (V == 0) {
        for (int i = 0; i < n; ++i) cout << board[i] << '\n';
        return 0;
    }

    auto runGreedy = [&](const vector<int>& order, vector<int>& outMatch) -> int {
        vector<int> mt(V, -1);
        int full = 0;

        for (int idx : order) {
            CenterInfo& ci = centers[idx];

            if (ci.plus) {
                int d1 = -1, d2 = -1;

                for (int d : ci.neighA) {
                    if (mt[d] != -1) continue;

                    if (d1 == -1 || dotDegree[d] < dotDegree[d1]) {
                        d2 = d1;
                        d1 = d;
                    } else if (d2 == -1 || dotDegree[d] < dotDegree[d2]) {
                        d2 = d;
                    }
                }

                if (d1 != -1 && d2 != -1) {
                    mt[ci.a] = d1;
                    mt[d1] = ci.a;

                    mt[ci.b] = d2;
                    mt[d2] = ci.b;

                    ++full;
                } else {
                    mt[ci.a] = ci.b;
                    mt[ci.b] = ci.a;
                }
            } else {
                int h = -1, v = -1;

                for (int d : ci.neighA) {
                    if (mt[d] == -1 &&
                        (h == -1 || dotDegree[d] < dotDegree[h])) {
                        h = d;
                    }
                }

                for (int d : ci.neighB) {
                    if (mt[d] == -1 &&
                        (v == -1 || dotDegree[d] < dotDegree[v])) {
                        v = d;
                    }
                }

                if (h != -1 && v != -1) {
                    mt[ci.a] = h;
                    mt[h] = ci.a;

                    mt[ci.b] = v;
                    mt[v] = ci.b;

                    ++full;
                } else {
                    mt[ci.a] = ci.b;
                    mt[ci.b] = ci.a;
                }
            }
        }

        outMatch.swap(mt);
        return full;
    };

    int C = (int)centers.size();

    vector<int> order(C);
    iota(order.begin(), order.end(), 0);

    auto score = [&](int idx) -> int {
        CenterInfo& ci = centers[idx];
        if (ci.plus) return (int)ci.neighA.size();
        return min((int)ci.neighA.size(), (int)ci.neighB.size());
    };

    sort(order.begin(), order.end(), [&](int x, int y) {
        int sx = score(x);
        int sy = score(y);
        if (sx != sy) return sx < sy;

        int cx = centers[x].r * m + centers[x].c;
        int cy = centers[y].r * m + centers[y].c;
        return cx < cy;
    });

    vector<int> bestMatch;
    int bestFull = -1;

    vector<int> tmpMatch;
    int got = runGreedy(order, tmpMatch);
    bestMatch = tmpMatch;
    bestFull = got;

    reverse(order.begin(), order.end());
    got = runGreedy(order, tmpMatch);
    if (got > bestFull) {
        bestFull = got;
        bestMatch = tmpMatch;
    }

    mt19937 rng(712367);
    shuffle(order.begin(), order.end(), rng);
    got = runGreedy(order, tmpMatch);
    if (got > bestFull) {
        bestFull = got;
        bestMatch = tmpMatch;
    }

    shuffle(order.begin(), order.end(), rng);
    got = runGreedy(order, tmpMatch);
    if (got > bestFull) {
        bestFull = got;
        bestMatch = tmpMatch;
    }

    Blossom blossom(g, bestMatch);

    vector<int> comp(V, -1);

    struct Component {
        int C = 0;
        int D = 0;
        int Fcur = 0;
        int Fub = 0;
        vector<int> freeDots;
    };

    vector<Component> comps;
    vector<int> st;
    st.reserve(V);

    for (int start = 0; start < V; ++start) {
        if (comp[start] != -1) continue;

        int id = (int)comps.size();
        comps.push_back(Component{});

        st.clear();
        st.push_back(start);
        comp[start] = id;

        int nonDots = 0;
        int dots = 0;

        while (!st.empty()) {
            int v = st.back();
            st.pop_back();

            if (isDot[v]) {
                ++dots;
                if (blossom.match[v] == -1) {
                    comps[id].freeDots.push_back(v);
                }
            } else {
                ++nonDots;
            }

            for (int to : g[v]) {
                if (comp[to] == -1) {
                    comp[to] = id;
                    st.push_back(to);
                }
            }
        }

        comps[id].C = nonDots / 2;
        comps[id].D = dots;
        comps[id].Fub = min(comps[id].C, comps[id].D / 2);
    }

    for (const auto& ci : centers) {
        int id = comp[ci.a];
        int ma = blossom.match[ci.a];

        if (ma != -1 && isDot[ma]) {
            ++comps[id].Fcur;
        }
    }

    for (int id = 0; id < (int)comps.size(); ++id) {
        Component& co = comps[id];

        if (co.Fcur >= co.Fub) continue;

        for (int v : co.freeDots) {
            if (blossom.match[v] != -1) continue;
            if (co.Fcur >= co.Fub) break;

            int w = blossom.findPath(v);

            if (w == -1) continue;

            int cur = w;

            while (cur != -1) {
                int pv = blossom.getPar(cur);
                if (pv == -1) break;

                int nxt = blossom.match[pv];

                blossom.match[cur] = pv;
                blossom.match[pv] = cur;

                cur = nxt;
            }

            ++co.Fcur;
        }
    }

    vector<int> triId(cells, -1);
    vector<array<int, 3>> tris;

    for (const auto& ci : centers) {
        int ma = blossom.match[ci.a];
        int mb = blossom.match[ci.b];

        if (ma != -1 && mb != -1 && isDot[ma] && isDot[mb]) {
            int centerCell = ci.r * m + ci.c;
            int leaf1 = cellOfVertex[ma];
            int leaf2 = cellOfVertex[mb];

            int id = (int)tris.size();
            tris.push_back({centerCell, leaf1, leaf2});

            triId[centerCell] = id;
            triId[leaf1] = id;
            triId[leaf2] = id;
        }
    }

    vector<int> color(tris.size(), -1);

    for (int id = 0; id < (int)tris.size(); ++id) {
        int mask = 0;

        for (int k = 0; k < 3; ++k) {
            int cell = tris[id][k];
            int r = cell / m;
            int c = cell % m;

            for (int d = 0; d < 4; ++d) {
                int nr = r + dr[d];
                int nc = c + dc[d];

                if (nr < 0 || nr >= n || nc < 0 || nc >= m) continue;

                int ncell = nr * m + nc;
                int nid = triId[ncell];

                if (nid != -1 && nid != id && color[nid] != -1) {
                    mask |= 1 << color[nid];
                }
            }
        }

        int col = 0;
        while (mask & (1 << col)) ++col;

        color[id] = col;
    }

    vector<char> ans(cells);

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            ans[r * m + c] = board[r][c];
        }
    }

    for (int id = 0; id < (int)tris.size(); ++id) {
        char ch = char('a' + color[id]);

        for (int k = 0; k < 3; ++k) {
            ans[tris[id][k]] = ch;
        }
    }

    for (int r = 0; r < n; ++r) {
        for (int c = 0; c < m; ++c) {
            cout << ans[r * m + c];
        }
        cout << '\n';
    }

    return 0;
}

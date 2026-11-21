#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, r;
    DSU() {}
    DSU(int n) { init(n); }
    void init(int n) {
        p.resize(n);
        r.assign(n, 0);
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        return p[x] == x ? x : p[x] = find(p[x]);
    }
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (r[a] < r[b]) swap(a, b);
        p[b] = a;
        if (r[a] == r[b]) ++r[a];
        return true;
    }
};

struct Dinic {
    struct Edge {
        int to, rev, cap, orig, id;
    };

    int N;
    vector<vector<Edge>> g;

    Dinic(int n = 0) : N(n), g(n) {}

    void addEdge(int u, int v, int cap, int id) {
        Edge a{v, (int)g[v].size(), cap, cap, id};
        Edge b{u, (int)g[u].size(), 0, 0, -1};
        g[u].push_back(a);
        g[v].push_back(b);
    }

    int maxflow(int s, int t, int need) {
        int flow = 0;
        vector<int> level(N), it(N);

        while (flow < need) {
            fill(level.begin(), level.end(), -1);
            queue<int> q;
            q.push(s);
            level[s] = 0;

            while (!q.empty()) {
                int v = q.front();
                q.pop();
                for (auto &e : g[v]) {
                    if (e.cap > 0 && level[e.to] == -1) {
                        level[e.to] = level[v] + 1;
                        q.push(e.to);
                    }
                }
            }

            if (level[t] == -1) break;

            fill(it.begin(), it.end(), 0);

            function<int(int, int)> dfs = [&](int v, int f) -> int {
                if (v == t) return f;
                for (int &i = it[v]; i < (int)g[v].size(); ++i) {
                    Edge &e = g[v][i];
                    if (e.cap > 0 && level[e.to] == level[v] + 1) {
                        int ret = dfs(e.to, min(f, e.cap));
                        if (ret) {
                            e.cap -= ret;
                            g[e.to][e.rev].cap += ret;
                            return ret;
                        }
                    }
                }
                return 0;
            };

            while (flow < need) {
                int f = dfs(s, need - flow);
                if (!f) break;
                flow += f;
            }
        }

        return flow;
    }

    vector<int> extractPath(int s, int t) {
        vector<int> pv(N, -1), pe(N, -1);
        queue<int> q;
        pv[s] = s;
        q.push(s);

        while (!q.empty() && pv[t] == -1) {
            int v = q.front();
            q.pop();

            for (int i = 0; i < (int)g[v].size(); ++i) {
                Edge &e = g[v][i];
                if (e.orig == 1 && e.cap == 0 && pv[e.to] == -1) {
                    pv[e.to] = v;
                    pe[e.to] = i;
                    q.push(e.to);
                }
            }
        }

        if (pv[t] == -1) return {};

        vector<pair<int, int>> path;
        int cur = t;

        while (cur != s) {
            int u = pv[cur];
            int ei = pe[cur];
            path.push_back({u, ei});
            cur = u;
        }

        reverse(path.begin(), path.end());

        vector<int> ids;

        for (auto [u, ei] : path) {
            Edge &e = g[u][ei];
            if (e.id != -1) ids.push_back(e.id);
            e.cap += 1;
            g[e.to][e.rev].cap -= 1;
        }

        return ids;
    }
};

int n, m;
vector<int> U, V;

int ask(const vector<int> &edges) {
    cout << '?';
    for (int id : edges) cout << ' ' << id;
    cout << '\n';
    cout.flush();

    int x;
    if (!(cin >> x)) exit(0);
    if (x == -1) exit(0);
    return x;
}

struct Block {
    vector<int> edges;
    vector<int> B;
    bool bridge = false;
    int bridgeEdge = -1;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n >> m;

    U.resize(m);
    V.resize(m);

    for (int i = 0; i < m; ++i) {
        cin >> U[i] >> V[i];
    }

    if (m == n - 1) {
        cout << '!';
        for (int i = 0; i < m; ++i) cout << ' ' << i;
        cout << '\n';
        return 0;
    }

    vector<vector<pair<int, int>>> adj(n);

    for (int id = 0; id < m; ++id) {
        adj[U[id]].push_back({V[id], id});
        adj[V[id]].push_back({U[id], id});
    }

    vector<int> disc(n, -1), low(n, -1);
    vector<int> estack;
    vector<Block> blocks;
    int timer = 0;

    function<void(int, int)> dfs = [&](int u, int pe) {
        disc[u] = low[u] = timer++;

        for (auto [v, id] : adj[u]) {
            if (id == pe) continue;

            if (disc[v] == -1) {
                estack.push_back(id);
                dfs(v, id);
                low[u] = min(low[u], low[v]);

                if (low[v] >= disc[u]) {
                    Block b;
                    while (true) {
                        int eid = estack.back();
                        estack.pop_back();
                        b.edges.push_back(eid);
                        if (eid == id) break;
                    }
                    blocks.push_back(b);
                }
            } else if (disc[v] < disc[u]) {
                estack.push_back(id);
                low[u] = min(low[u], disc[v]);
            }
        }
    };

    for (int i = 0; i < n; ++i) {
        if (disc[i] == -1) dfs(i, -1);
    }

    vector<char> chosenBase(m, 0);

    for (Block &b : blocks) {
        if ((int)b.edges.size() == 1) {
            b.bridge = true;
            b.bridgeEdge = b.edges[0];
            b.B = {b.bridgeEdge};
            chosenBase[b.bridgeEdge] = 1;
        } else {
            b.bridge = false;
            DSU dsu(n);

            for (int id : b.edges) {
                if (dsu.unite(U[id], V[id])) {
                    b.B.push_back(id);
                    chosenBase[id] = 1;
                }
            }
        }
    }

    vector<int> globalBaseEdges;

    for (int id = 0; id < m; ++id) {
        if (chosenBase[id]) globalBaseEdges.push_back(id);
    }

    int baseValue = ask(globalBaseEdges);

    if (baseValue == n - 1) {
        cout << '!';
        for (int id : globalBaseEdges) cout << ' ' << id;
        cout << '\n';
        return 0;
    }

    vector<int> isRoyal(m, 0);
    vector<int> knownStatus(m, -1);

    for (Block &b : blocks) {
        if (b.bridge) {
            isRoyal[b.bridgeEdge] = 1;
            knownStatus[b.bridgeEdge] = 1;
        }
    }

    for (Block &b : blocks) {
        if (b.bridge) continue;

        vector<vector<pair<int, int>>> badj(n);

        for (int id : b.edges) {
            badj[U[id]].push_back({V[id], id});
            badj[V[id]].push_back({U[id], id});
        }

        vector<char> inB(m, 0);

        for (int id : b.B) inB[id] = 1;

        int f = -1;

        for (int id : b.edges) {
            if (!inB[id]) {
                f = id;
                break;
            }
        }

        vector<int> path;

        {
            vector<int> parV(n, -1), parE(n, -1);
            queue<int> q;
            q.push(U[f]);
            parV[U[f]] = U[f];

            while (!q.empty() && parV[V[f]] == -1) {
                int u = q.front();
                q.pop();

                for (auto [v, id] : badj[u]) {
                    if (inB[id] && parV[v] == -1) {
                        parV[v] = u;
                        parE[v] = id;
                        q.push(v);
                    }
                }
            }

            int cur = V[f];

            while (cur != U[f]) {
                path.push_back(parE[cur]);
                cur = parV[cur];
            }
        }

        auto buildGlobal = [&](const vector<int> &Sblock) {
            vector<int> res;
            res.reserve(n - 1);

            for (int id : globalBaseEdges) {
                if (!inB[id]) res.push_back(id);
            }

            for (int id : Sblock) res.push_back(id);

            return res;
        };

        int z = -1;
        bool found = false;

        for (int e : path) {
            vector<int> Sblock;
            Sblock.reserve(b.B.size());

            for (int id : b.B) {
                Sblock.push_back(id == e ? f : id);
            }

            int ans = ask(buildGlobal(Sblock));
            int delta = ans - baseValue;

            if (delta == 1) {
                z = e;
                knownStatus[e] = 0;
                knownStatus[f] = 1;
                isRoyal[f] = 1;
                found = true;
                break;
            } else if (delta == -1) {
                z = f;
                knownStatus[e] = 1;
                isRoyal[e] = 1;
                knownStatus[f] = 0;
                found = true;
                break;
            }
        }

        if (!found) {
            z = f;
            knownStatus[f] = 0;

            for (int e : path) {
                if (knownStatus[e] == -1) knownStatus[e] = 0;
            }
        }

        knownStatus[z] = 0;

        auto completeSpanningTree = [&](int excludedEdge, const vector<int> &initial) {
            DSU dsu(n);
            vector<int> S;
            S.reserve(b.B.size());

            for (int id : initial) {
                if (dsu.unite(U[id], V[id])) S.push_back(id);
            }

            for (int id : b.edges) {
                if (id == excludedEdge) continue;
                if (dsu.unite(U[id], V[id])) S.push_back(id);
            }

            return S;
        };

        auto buildExchange = [&](int e, int zedge) -> vector<int> {
            int a = U[e], bb = V[e];
            int c = U[zedge], d = V[zedge];

            vector<int> initial;

            int shared = -1;
            int eOther = -1, zOther = -1;

            if (a == c) {
                shared = a;
                eOther = bb;
                zOther = d;
            } else if (a == d) {
                shared = a;
                eOther = bb;
                zOther = c;
            } else if (bb == c) {
                shared = bb;
                eOther = a;
                zOther = d;
            } else if (bb == d) {
                shared = bb;
                eOther = a;
                zOther = c;
            }

            if (shared != -1) {
                vector<int> parV(n, -1), parE(n, -1);
                queue<int> q;
                q.push(eOther);
                parV[eOther] = eOther;

                while (!q.empty() && parV[zOther] == -1) {
                    int u = q.front();
                    q.pop();

                    for (auto [v, id] : badj[u]) {
                        if (v == shared) continue;
                        if (parV[v] == -1) {
                            parV[v] = u;
                            parE[v] = id;
                            q.push(v);
                        }
                    }
                }

                initial.push_back(zedge);

                int cur = zOther;

                while (cur != eOther) {
                    initial.push_back(parE[cur]);
                    cur = parV[cur];
                }
            } else {
                Dinic din(2 * n + 2);
                int S = 2 * n;
                int T = 2 * n + 1;

                for (int v = 0; v < n; ++v) {
                    din.addEdge(2 * v, 2 * v + 1, 1, -1);
                }

                din.addEdge(S, 2 * a, 1, -1);
                din.addEdge(S, 2 * bb, 1, -1);
                din.addEdge(2 * c + 1, T, 1, -1);
                din.addEdge(2 * d + 1, T, 1, -1);

                for (int id : b.edges) {
                    if (id == e || id == zedge) continue;

                    int u = U[id], v = V[id];

                    din.addEdge(2 * u + 1, 2 * v, 1, id);
                    din.addEdge(2 * v + 1, 2 * u, 1, id);
                }

                int flow = din.maxflow(S, T, 2);

                if (flow >= 2) {
                    vector<int> p1 = din.extractPath(S, T);
                    vector<int> p2 = din.extractPath(S, T);

                    vector<vector<pair<int, int>>> temp(n);

                    auto addTemp = [&](int id) {
                        temp[U[id]].push_back({V[id], id});
                        temp[V[id]].push_back({U[id], id});
                    };

                    addTemp(e);
                    addTemp(zedge);

                    for (int id : p1) addTemp(id);
                    for (int id : p2) addTemp(id);

                    vector<int> parV(n, -1), parE(n, -1);
                    queue<int> q;
                    q.push(a);
                    parV[a] = a;

                    while (!q.empty() && parV[bb] == -1) {
                        int u = q.front();
                        q.pop();

                        for (auto [v, id] : temp[u]) {
                            if (id == e) continue;
                            if (parV[v] == -1) {
                                parV[v] = u;
                                parE[v] = id;
                                q.push(v);
                            }
                        }
                    }

                    int cur = bb;

                    while (cur != a) {
                        initial.push_back(parE[cur]);
                        cur = parV[cur];
                    }
                }
            }

            return completeSpanningTree(e, initial);
        };

        for (int id : b.B) {
            if (knownStatus[id] != -1) continue;

            vector<int> Sblock = buildExchange(id, z);

            vector<int> S2;

            for (int x : Sblock) {
                if (x != z) S2.push_back(x);
            }

            S2.push_back(id);

            int ans1 = ask(buildGlobal(Sblock));
            int ans2 = ask(buildGlobal(S2));

            int a = ans2 - ans1;
            knownStatus[id] = a;

            if (a == 1) isRoyal[id] = 1;
            else isRoyal[id] = 0;
        }

        for (int id : b.B) {
            if (knownStatus[id] == 1) isRoyal[id] = 1;
            else if (knownStatus[id] == 0) isRoyal[id] = 0;
        }

        vector<int> unknown;

        for (int id : b.edges) {
            if (inB[id]) continue;

            if (knownStatus[id] == -1) {
                unknown.push_back(id);
            } else if (knownStatus[id] == 1) {
                isRoyal[id] = 1;
            }
        }

        function<int(const vector<int> &)> countRoyal =
            [&](const vector<int> &F) -> int {
                if (F.empty()) return 0;

                DSU dsu(n);
                vector<int> Y;

                for (int id : F) {
                    dsu.unite(U[id], V[id]);
                }

                int sumX = 0;

                for (int id : b.B) {
                    if (dsu.unite(U[id], V[id])) {
                        Y.push_back(id);
                    } else {
                        if (knownStatus[id] == 1) ++sumX;
                    }
                }

                vector<int> qedges;
                qedges.reserve(n - 1);

                for (int id : globalBaseEdges) {
                    if (!inB[id]) qedges.push_back(id);
                }

                for (int id : F) qedges.push_back(id);
                for (int id : Y) qedges.push_back(id);

                int ans = ask(qedges);

                return ans - baseValue + sumX;
            };

        vector<vector<int>> usedColors(n);
        vector<vector<int>> colorLists;

        int maxColors = 2 * n + 5;
        vector<int> mark(maxColors, -1);
        int stamp = 0;

        for (int id : unknown) {
            int u = U[id], v = V[id];

            ++stamp;

            for (int c : usedColors[u]) {
                if (c < maxColors) mark[c] = stamp;
            }

            for (int c : usedColors[v]) {
                if (c < maxColors) mark[c] = stamp;
            }

            int c = 0;

            while (c < maxColors && mark[c] == stamp) ++c;

            if (c >= (int)colorLists.size()) colorLists.resize(c + 1);

            colorLists[c].push_back(id);
            usedColors[u].push_back(c);
            usedColors[v].push_back(c);
        }

        function<void(const vector<int> &, int)> solve =
            [&](const vector<int> &F, int c) {
                if (c == 0) return;

                if (c == (int)F.size()) {
                    for (int id : F) {
                        isRoyal[id] = 1;
                        knownStatus[id] = 1;
                    }
                    return;
                }

                if ((int)F.size() == 1) {
                    isRoyal[F[0]] = 1;
                    knownStatus[F[0]] = 1;
                    return;
                }

                int mid = (int)F.size() / 2;

                vector<int> L(F.begin(), F.begin() + mid);
                vector<int> R(F.begin() + mid, F.end());

                int cL = countRoyal(L);

                solve(L, cL);
                solve(R, c - cL);
            };

        for (auto &M : colorLists) {
            if (M.empty()) continue;

            int c = countRoyal(M);

            if (c > 0) solve(M, c);
        }
    }

    vector<int> answer;

    for (int id = 0; id < m; ++id) {
        if (isRoyal[id]) answer.push_back(id);
    }

    cout << '!';
    for (int id : answer) cout << ' ' << id;
    cout << '\n';

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

struct State {
    set<int> adj[105];

    bool has(int u, int v) const {
        return adj[u].find(v) != adj[u].end();
    }

    void add(int u, int v) {
        adj[u].insert(v);
        adj[v].insert(u);
    }

    void eraseEdge(int u, int v) {
        adj[u].erase(v);
        adj[v].erase(u);
    }
};

struct Operation {
    int a, b, c, d;
    array<pair<int,int>, 3> e;
};

int n;
State S, T;

void applyOp(State &st, int a, int b, int c, int d,
             vector<pair<int,int>> ne, vector<Operation> &ops) {
    st.eraseEdge(a, b);
    st.eraseEdge(b, c);
    st.eraseEdge(c, d);

    Operation op;
    op.a = a;
    op.b = b;
    op.c = c;
    op.d = d;

    for (int i = 0; i < 3; i++) {
        int u = ne[i].first, v = ne[i].second;
        op.e[i] = {u, v};
        st.add(u, v);
    }

    ops.push_back(op);
}

bool sameTree(const State &A, const State &B) {
    for (int i = 1; i <= n; i++) {
        for (int j = i + 1; j <= n; j++) {
            if (A.has(i, j) != B.has(i, j)) return false;
        }
    }
    return true;
}

bool isStar(const State &A) {
    for (int c = 1; c <= n; c++) {
        bool ok = true;
        for (int v = 1; v <= n; v++) {
            if (v == c) continue;
            if (!A.has(c, v)) {
                ok = false;
                break;
            }
        }
        if (ok) return true;
    }
    return false;
}

vector<int> getPath(const State &st, int s, int t) {
    vector<int> par(n + 1, -1);
    queue<int> q;
    q.push(s);
    par[s] = s;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        if (u == t) break;

        for (int v : st.adj[u]) {
            if (par[v] == -1) {
                par[v] = u;
                q.push(v);
            }
        }
    }

    vector<int> path;
    int cur = t;
    while (cur != s) {
        path.push_back(cur);
        cur = par[cur];
    }
    path.push_back(s);
    reverse(path.begin(), path.end());
    return path;
}

bool flattenToAlmostStar(State &st, int r, int &h1, int &h2,
                         vector<Operation> &ops) {
    while (true) {
        vector<int> non;
        for (int v = 1; v <= n; v++) {
            if (v != r && !st.has(r, v)) non.push_back(v);
        }

        if ((int)non.size() <= 1) break;

        // Find a vertex b at distance 2 from r, with parent a.
        int b = -1, a = -1;
        for (int v : non) {
            for (int u : st.adj[v]) {
                if (st.has(r, u)) {
                    b = v;
                    a = u;
                    break;
                }
            }
            if (b != -1) break;
        }

        if (b == -1) return false;

        int cnt = (int)non.size();

        if (cnt == 2) {
            // Attach b to r, but keep one vertex as the handle tail.
            int d = -1;
            for (int u : st.adj[r]) {
                if (u != a) {
                    d = u;
                    break;
                }
            }

            if (d != -1) {
                // path d-r-a-b -> star centered at r
                vector<pair<int,int>> ne = {
                    {r, d}, {r, a}, {r, b}
                };
                applyOp(st, d, r, a, b, ne, ops);
            } else {
                // r has degree 1. We need path r-a-b-c.
                int c = -1;
                for (int u : st.adj[b]) {
                    if (u != a) {
                        c = u;
                        break;
                    }
                }

                if (c == -1) {
                    // Try another distance-2 vertex which has a child.
                    bool found = false;
                    for (int v : non) {
                        int pa = -1;
                        for (int u : st.adj[v]) {
                            if (st.has(r, u)) {
                                pa = u;
                                break;
                            }
                        }
                        if (pa == -1) continue;

                        for (int u : st.adj[v]) {
                            if (u != pa) {
                                b = v;
                                a = pa;
                                c = u;
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    if (!found) return false;
                }

                // path r-a-b-c -> keep c as one extra vertex
                vector<pair<int,int>> ne = {
                    {r, a}, {r, b}, {b, c}
                };
                applyOp(st, r, a, b, c, ne, ops);
            }
        } else {
            int d = -1;
            for (int u : st.adj[r]) {
                if (u != a) {
                    d = u;
                    break;
                }
            }

            if (d != -1) {
                // path d-r-a-b -> star centered at r
                vector<pair<int,int>> ne = {
                    {r, d}, {r, a}, {r, b}
                };
                applyOp(st, d, r, a, b, ne, ops);
            } else {
                // r has degree 1. Find r-a-b-c.
                int c = -1;
                for (int u : st.adj[b]) {
                    if (u != a) {
                        c = u;
                        break;
                    }
                }

                if (c == -1) {
                    bool found = false;
                    for (int v : non) {
                        int pa = -1;
                        for (int u : st.adj[v]) {
                            if (st.has(r, u)) {
                                pa = u;
                                break;
                            }
                        }
                        if (pa == -1) continue;

                        for (int u : st.adj[v]) {
                            if (u != pa) {
                                b = v;
                                a = pa;
                                c = u;
                                found = true;
                                break;
                            }
                        }
                        if (found) break;
                    }
                    if (!found) return false;
                }

                // path r-a-b-c -> star centered at r
                vector<pair<int,int>> ne = {
                    {r, a}, {r, b}, {r, c}
                };
                applyOp(st, r, a, b, c, ne, ops);
            }
        }
    }

    h2 = -1;
    for (int v = 1; v <= n; v++) {
        if (v != r && !st.has(r, v)) {
            h2 = v;
            break;
        }
    }

    if (h2 == -1) return false;

    h1 = *st.adj[h2].begin();
    return true;
}

bool adjustHandle(State &st, int r, int &curA, int &curB,
                  int H1, int H2, vector<Operation> &ops) {
    if (curA == H1 && curB == H2) return true;

    map<pair<int,int>, pair<pair<int,int>, pair<int,int>>> parent;
    map<pair<int,int>, bool> vis;

    queue<pair<int,int>> q;
    pair<int,int> start = {curA, curB};
    pair<int,int> target = {H1, H2};

    vis[start] = true;
    q.push(start);

    while (!q.empty()) {
        auto [a, b] = q.front();
        q.pop();

        if (make_pair(a, b) == target) break;

        for (int x = 1; x <= n; x++) {
            if (x == r || x == a || x == b) continue;

            // Change internal vertex to x.
            pair<int,int> nxt1 = {x, b};
            if (!vis[nxt1]) {
                vis[nxt1] = true;
                parent[nxt1] = {{a, b}, {0, x}};
                q.push(nxt1);
            }

            // Change tail to x.
            pair<int,int> nxt2 = {a, x};
            if (!vis[nxt2]) {
                vis[nxt2] = true;
                parent[nxt2] = {{a, b}, {1, x}};
                q.push(nxt2);
            }
        }
    }

    if (!vis[target]) return false;

    vector<pair<int,int>> seq;
    pair<int,int> cur = target;

    while (cur != start) {
        auto it = parent.find(cur);
        if (it == parent.end()) return false;

        auto [prev, opInfo] = it->second;
        seq.push_back(opInfo);
        cur = prev;
    }

    reverse(seq.begin(), seq.end());

    for (auto [type, x] : seq) {
        int a = curA, b = curB;

        if (type == 0) {
            // set_internal(x)
            vector<pair<int,int>> ne = {
                {x, r}, {x, b}, {r, a}
            };
            applyOp(st, x, r, a, b, ne, ops);
            curA = x;
        } else {
            // set_tail(x)
            vector<pair<int,int>> ne = {
                {x, a}, {a, r}, {r, b}
            };
            applyOp(st, x, r, a, b, ne, ops);
            curB = x;
        }
    }

    return curA == H1 && curB == H2;
}

void buildPath(State &st, const vector<int> &Q, int r, int H1, int H2,
               vector<Operation> &ops) {
    int nxt = H2;

    for (int i = (int)Q.size() - 2; i >= 2; i--) {
        int x = Q[i];

        vector<pair<int,int>> ne = {
            {r, H1}, {H1, x}, {x, nxt}
        };

        applyOp(st, x, r, H1, nxt, ne, ops);
        nxt = x;
    }
}

bool pathToTarget(State &st, const State &target,
                  vector<Operation> &ops, int limit) {
    while (!sameTree(st, target)) {
        if ((int)ops.size() > limit) return false;

        int bu = -1, bv = -1;
        int bestDist = 1e9;

        for (int u = 1; u <= n; u++) {
            for (int v = u + 1; v <= n; v++) {
                if (target.has(u, v) && !st.has(u, v)) {
                    vector<int> path = getPath(st, u, v);
                    int d = (int)path.size() - 1;
                    if (d < bestDist) {
                        bestDist = d;
                        bu = u;
                        bv = v;
                    }
                }
            }
        }

        if (bu == -1) break;

        int U = bu, V = bv;

        while (true) {
            if ((int)ops.size() > limit) return false;

            vector<int> path = getPath(st, U, V);
            int k = (int)path.size() - 1;

            if (k == 3) {
                vector<pair<int,int>> orig;
                for (int i = 0; i < 3; i++) {
                    orig.push_back({path[i], path[i + 1]});
                }

                int eidx = -1;
                for (int i = 0; i < 3; i++) {
                    if (!target.has(orig[i].first, orig[i].second)) {
                        eidx = i;
                        break;
                    }
                }

                if (eidx == -1) return false;

                vector<pair<int,int>> ne;
                for (int i = 0; i < 3; i++) {
                    if (i != eidx) ne.push_back(orig[i]);
                }
                ne.push_back({U, V});

                applyOp(st, path[0], path[1], path[2], path[3], ne, ops);
                break;
            }

            if (k == 2) {
                int x = path[1];
                bool done = false;

                for (int w = 1; w <= n && !done; w++) {
                    if (w == U || w == x || w == V) continue;

                    if (st.has(w, U)) {
                        vector<pair<int,int>> orig = {
                            {w, U}, {U, x}, {x, V}
                        };

                        for (int eidx = 1; eidx <= 2; eidx++) {
                            if (!target.has(orig[eidx].first, orig[eidx].second)) {
                                vector<pair<int,int>> ne;
                                for (int i = 0; i < 3; i++) {
                                    if (i != eidx) ne.push_back(orig[i]);
                                }
                                ne.push_back({U, V});

                                applyOp(st, w, U, x, V, ne, ops);
                                done = true;
                                break;
                            }
                        }
                    }

                    if (done) break;

                    if (st.has(w, V)) {
                        vector<pair<int,int>> orig = {
                            {w, V}, {V, x}, {x, U}
                        };

                        for (int eidx = 1; eidx <= 2; eidx++) {
                            if (!target.has(orig[eidx].first, orig[eidx].second)) {
                                vector<pair<int,int>> ne;
                                for (int i = 0; i < 3; i++) {
                                    if (i != eidx) ne.push_back(orig[i]);
                                }
                                ne.push_back({U, V});

                                applyOp(st, w, V, x, U, ne, ops);
                                done = true;
                                break;
                            }
                        }
                    }
                }

                if (done) break;

                // Both U and V are leaves at x. Lift U away from x.
                int d = -1;
                for (int u : st.adj[x]) {
                    if (u != U && u != V) {
                        d = u;
                        break;
                    }
                }
                if (d == -1) return false;

                int e2 = -1;
                for (int u : st.adj[d]) {
                    if (u != x) {
                        e2 = u;
                        break;
                    }
                }
                if (e2 == -1) return false;

                vector<pair<int,int>> ne = {
                    {U, d}, {x, d}, {x, e2}
                };

                applyOp(st, U, x, d, e2, ne, ops);
                continue;
            }

            if (k > 3) {
                int idx = -1, dir = 0;

                for (int i = 0; i < k; i++) {
                    if (!target.has(path[i], path[i + 1])) {
                        if (i + 3 <= k) {
                            idx = i;
                            dir = 1;
                            break;
                        }
                        if (i - 2 >= 0) {
                            idx = i;
                            dir = -1;
                            break;
                        }
                    }
                }

                if (idx == -1) return false;

                if (dir == 1) {
                    int a = path[idx];
                    int b = path[idx + 1];
                    int c = path[idx + 2];
                    int d = path[idx + 3];

                    vector<pair<int,int>> ne = {
                        {a, d}, {b, c}, {c, d}
                    };

                    applyOp(st, a, b, c, d, ne, ops);
                } else {
                    int d = path[idx - 2];
                    int c = path[idx - 1];
                    int a = path[idx];
                    int b = path[idx + 1];

                    vector<pair<int,int>> ne = {
                        {d, b}, {d, c}, {c, a}
                    };

                    applyOp(st, d, c, a, b, ne, ops);
                }

                continue;
            }

            // k == 1 should not happen for a missing target edge.
            return false;
        }
    }

    return sameTree(st, target);
}

vector<int> dfsOrder(const State &tree, int root, int type) {
    vector<int> order;
    order.reserve(n);

    function<void(int,int)> dfs = [&](int u, int p) {
        order.push_back(u);

        if (type == 0) {
            for (int v : tree.adj[u]) {
                if (v != p) dfs(v, u);
            }
        } else {
            for (auto it = tree.adj[u].rbegin(); it != tree.adj[u].rend(); ++it) {
                int v = *it;
                if (v != p) dfs(v, u);
            }
        }
    };

    dfs(root, 0);
    return order;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cin >> n;

    for (int i = 0; i < n - 1; i++) {
        int u, v;
        cin >> u >> v;
        S.add(u, v);
    }

    for (int i = 0; i < n - 1; i++) {
        int u, v;
        cin >> u >> v;
        T.add(u, v);
    }

    if (sameTree(S, T)) {
        cout << "YES\n0\n";
        return 0;
    }

    if (isStar(S)) {
        cout << "NO\n";
        return 0;
    }

    vector<Operation> answer;
    bool found = false;

    for (int r = 1; r <= n && !found; r++) {
        State base = S;
        vector<Operation> baseOps;

        int h1, h2;
        if (!flattenToAlmostStar(base, r, h1, h2, baseOps)) continue;
        if ((int)baseOps.size() > 400) continue;

        for (int type = 0; type < 2 && !found; type++) {
            vector<int> Q = dfsOrder(T, r, type);

            int H1 = Q[1];
            int H2 = Q.back();

            State st = base;
            vector<Operation> ops = baseOps;

            int curA = h1, curB = h2;
            if (!adjustHandle(st, r, curA, curB, H1, H2, ops)) continue;
            if ((int)ops.size() > 400) continue;

            buildPath(st, Q, r, H1, H2, ops);
            if ((int)ops.size() > 400) continue;

            if (pathToTarget(st, T, ops, 400)) {
                if ((int)ops.size() <= 400) {
                    answer = ops;
                    found = true;
                }
            }
        }
    }

    if (!found) {
        cout << "NO\n";
        return 0;
    }

    cout << "YES\n";
    cout << answer.size() << '\n';

    for (const auto &op : answer) {
        cout << op.a << ' ' << op.b << ' ' << op.c << ' ' << op.d << '\n';

        for (int i = 0; i < 3; i++) {
            if (i) cout << ' ';
            cout << op.e[i].first << ' ' << op.e[i].second;
        }
        cout << '\n';
    }

    return 0;
}

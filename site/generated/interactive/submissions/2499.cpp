#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;
    DSU() {}
    explicit DSU(int n) : p(n), sz(n, 1) { iota(p.begin(), p.end(), 0); }
    int find(int x) { return p[x] == x ? x : p[x] = find(p[x]); }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a; sz[a] += sz[b];
        return true;
    }
};

int n, m;
vector<int> U, V, state, royalRoads, treeEdges;
DSU vertices;
mt19937 rng(712367);
int treeRoyal = 0;

int ask(const vector<int>& edges) {
    cout << '?';
    for (int e : royalRoads) cout << ' ' << e;
    for (int e : edges) cout << ' ' << e;
    cout << '\n' << flush;
    int answer;
    if (!(cin >> answer)) exit(0);
    return answer;
}

vector<int> pathInEdges(const vector<int>& edges, int edge) {
    int start = vertices.find(U[edge]), target = vertices.find(V[edge]);
    vector<vector<pair<int,int>>> adj(n);
    for (int e : edges) {
        int a = vertices.find(U[e]), b = vertices.find(V[e]);
        adj[a].push_back({b, e});
        adj[b].push_back({a, e});
    }
    vector<int> parent(n, -1), parentEdge(n, -1), queue(n);
    int qh = 0, qt = 0;
    queue[qt++] = start;
    parent[start] = start;
    while (qh < qt && parent[target] == -1) {
        int a = queue[qh++];
        for (auto [b, e] : adj[a]) if (parent[b] == -1) {
            parent[b] = a;
            parentEdge[b] = e;
            queue[qt++] = b;
        }
    }
    vector<int> path;
    for (int x = target; x != start; x = parent[x]) path.push_back(parentEdge[x]);
    reverse(path.begin(), path.end());
    return path;
}

vector<int> pathInTree(int edge) {
    return pathInEdges(treeEdges, edge);
}

void makeRoyal(int e) {
    if (state[e] == 1) return;
    state[e] = 1;
    royalRoads.push_back(e);
    vertices.unite(U[e], V[e]);
}

void initialReductions() {
    while (true) {
        bool changed = false;
        for (int e = 0; e < m; ++e) if (state[e] == -1 &&
                vertices.find(U[e]) == vertices.find(V[e])) {
            state[e] = 0;
            changed = true;
        }
        if ((int)royalRoads.size() == n - 1) return;

        vector<vector<pair<int,int>>> adj(n);
        for (int e = 0; e < m; ++e) if (state[e] == -1) {
            int a = vertices.find(U[e]), b = vertices.find(V[e]);
            adj[a].push_back({b, e});
            adj[b].push_back({a, e});
        }
        vector<int> tin(n, -1), low(n);
        int timer = 0, bridge = -1;
        function<void(int,int)> dfs = [&](int a, int incoming) {
            tin[a] = low[a] = timer++;
            for (auto [b, e] : adj[a]) {
                if (e == incoming) continue;
                if (tin[b] != -1) {
                    low[a] = min(low[a], tin[b]);
                } else {
                    dfs(b, e);
                    low[a] = min(low[a], low[b]);
                    if (low[b] > tin[a]) bridge = e;
                }
            }
        };
        for (int a = 0; a < n; ++a) if (vertices.find(a) == a && tin[a] == -1)
            dfs(a, -1);
        if (bridge != -1) {
            makeRoyal(bridge);
            changed = true;
        }
        if (!changed) return;
    }
}

void buildTree() {
    vector<int> edges;
    for (int e = 0; e < m; ++e) if (state[e] == -1) edges.push_back(e);
    shuffle(edges.begin(), edges.end(), rng);
    DSU local(n);
    treeEdges.clear();
    for (int e : edges) {
        int a = local.find(vertices.find(U[e]));
        int b = local.find(vertices.find(V[e]));
        if (a != b) {
            local.unite(a, b);
            treeEdges.push_back(e);
        }
    }
    treeRoyal = ask(treeEdges) - (int)royalRoads.size();
}

void eraseTreeEdge(int e) {
    auto it = find(treeEdges.begin(), treeEdges.end(), e);
    if (it != treeEdges.end()) treeEdges.erase(it);
}

void reduceWithTree() {
    while ((int)royalRoads.size() < n - 1) {
        bool changed = false;
        int unknownTree = 0, unknownOutside = 0;
        vector<char> inTree(m, 0);
        for (int e : treeEdges) {
            inTree[e] = 1;
            if (state[e] == -1) ++unknownTree;
        }
        for (int e = 0; e < m; ++e) if (state[e] == -1) {
            if (vertices.find(U[e]) == vertices.find(V[e])) {
                state[e] = 0;
                changed = true;
            } else if (!inTree[e]) {
                ++unknownOutside;
            }
        }
        if ((int)royalRoads.size() == n - 1) return;
        int rank = n - 1 - (int)royalRoads.size();
        int outsideRoyal = rank - treeRoyal;

        if (treeRoyal == 0) {
            for (int e : treeEdges) if (state[e] == -1) {
                state[e] = 0;
                changed = true;
            }
            unknownTree = 0;
        }
        if (outsideRoyal == 0) {
            for (int e = 0; e < m; ++e) if (state[e] == -1 && !inTree[e]) {
                state[e] = 0;
                changed = true;
            }
            unknownOutside = 0;
        }
        if (treeRoyal == unknownTree && unknownTree > 0) {
            vector<int> take;
            for (int e : treeEdges) if (state[e] == -1) take.push_back(e);
            for (int e : take) {
                eraseTreeEdge(e);
                makeRoyal(e);
            }
            treeRoyal = 0;
            changed = true;
            continue;
        }
        if (treeRoyal == 0 && outsideRoyal == unknownOutside && unknownOutside > 0) {
            for (int e = 0; e < m; ++e) if (state[e] == -1 && !inTree[e])
                makeRoyal(e);
            for (int e : treeEdges) state[e] = 0;
            return;
        }
        if (changed) continue;

        vector<vector<pair<int,int>>> adj(n);
        for (int e = 0; e < m; ++e) if (state[e] == -1) {
            int a = vertices.find(U[e]), b = vertices.find(V[e]);
            adj[a].push_back({b, e});
            adj[b].push_back({a, e});
        }
        vector<int> tin(n, -1), low(n);
        int timer = 0, bridge = -1;
        function<void(int,int)> dfs = [&](int a, int incoming) {
            tin[a] = low[a] = timer++;
            for (auto [b, e] : adj[a]) {
                if (e == incoming) continue;
                if (tin[b] != -1) low[a] = min(low[a], tin[b]);
                else {
                    dfs(b, e);
                    low[a] = min(low[a], low[b]);
                    if (low[b] > tin[a]) bridge = e;
                }
            }
        };
        for (int a = 0; a < n; ++a) if (vertices.find(a) == a && tin[a] == -1)
            dfs(a, -1);
        if (bridge == -1) return;

        auto it = find(treeEdges.begin(), treeEdges.end(), bridge);
        if (it != treeEdges.end()) {
            treeEdges.erase(it);
            --treeRoyal;
        } else {
            vector<int> path = pathInTree(bridge);
            int removable = -1;
            for (int e : path) if (state[e] == 0) {
                removable = e;
                break;
            }
            eraseTreeEdge(removable);
        }
        makeRoyal(bridge);
    }
}

void solveWithNonroyalBasis(vector<int> basis) {
    while ((int)royalRoads.size() < n - 1) {
        int rank = n - 1 - (int)royalRoads.size();
        vector<int> edges;
        for (int e = 0; e < m; ++e) if (state[e] == -1 &&
                vertices.find(U[e]) != vertices.find(V[e])) edges.push_back(e);
        shuffle(edges.begin(), edges.end(), rng);
        DSU local(n);
        vector<int> candidates;
        for (int e : edges) {
            int a = local.find(vertices.find(U[e]));
            int b = local.find(vertices.find(V[e]));
            if (a != b) {
                local.unite(a, b);
                candidates.push_back(e);
            }
        }
        int count = ask(candidates) - (int)royalRoads.size();
        if (count == 0) {
            for (int e : candidates) state[e] = 0;
            continue;
        }
        if (count == rank) {
            for (int e : candidates) makeRoyal(e);
            return;
        }
        vector<int> found;
        function<void(const vector<int>&,int)> collect = [&](const vector<int>& group, int wanted) {
            if (wanted == 0) return;
            if (wanted == (int)group.size()) {
                found.insert(found.end(), group.begin(), group.end());
                return;
            }
            int half = group.size() / 2;
            vector<int> first(group.begin(), group.begin() + half);
            vector<int> second(group.begin() + half, group.end());
            DSU extension(n);
            vector<int> query = first;
            for (int e : first)
                extension.unite(vertices.find(U[e]), vertices.find(V[e]));
            for (int e : basis) {
                int a = extension.find(vertices.find(U[e]));
                int b = extension.find(vertices.find(V[e]));
                if (a != b) {
                    extension.unite(a, b);
                    query.push_back(e);
                }
            }
            int inFirst = ask(query) - (int)royalRoads.size();
            collect(first, inFirst);
            collect(second, wanted - inFirst);
        };
        collect(candidates, count);
        for (int e : found) makeRoyal(e);
        DSU remaining(n);
        vector<int> newBasis;
        for (int e : basis) {
            int a = remaining.find(vertices.find(U[e]));
            int b = remaining.find(vertices.find(V[e]));
            if (a != b) {
                remaining.unite(a, b);
                newBasis.push_back(e);
            }
        }
        basis = move(newBasis);
    }
}

void lightReductions() {
    while ((int)royalRoads.size() < n - 1) {
        int unknownTree = 0;
        for (int e : treeEdges) if (state[e] == -1) ++unknownTree;
        if (treeRoyal == 0) {
            for (int e : treeEdges) if (state[e] == -1) state[e] = 0;
            unknownTree = 0;
        }
        if (unknownTree > 0 && treeRoyal == unknownTree) {
            vector<int> take;
            for (int e : treeEdges) if (state[e] == -1) take.push_back(e);
            for (int e : take) {
                eraseTreeEdge(e);
                makeRoyal(e);
            }
            treeRoyal = 0;
            continue;
        }
        int rank = n - 1 - (int)royalRoads.size();
        if (rank - treeRoyal == 0) {
            vector<char> inTree(m, 0);
            for (int e : treeEdges) inTree[e] = 1;
            for (int e = 0; e < m; ++e)
                if (state[e] == -1 && !inTree[e]) state[e] = 0;
            for (int e : treeEdges) if (state[e] == -1) {
                state[e] = 1;
                royalRoads.push_back(e);
            }
            return;
        }
        return;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n >> m)) return 0;
    U.resize(m); V.resize(m); state.assign(m, -1); vertices = DSU(n);
    for (int e = 0; e < m; ++e) cin >> U[e] >> V[e];

    initialReductions();
    if ((int)royalRoads.size() < n - 1) buildTree();

    int iterations = 0, rebuilds = 0;
    while ((int)royalRoads.size() < n - 1) {
        if (++iterations % 128 == 0) reduceWithTree();
        else lightReductions();
        if ((int)royalRoads.size() == n - 1) break;
        if (treeRoyal == 0) {
            for (int e : treeEdges) if (state[e] == -1) state[e] = 0;
            solveWithNonroyalBasis(treeEdges);
            break;
        }
        int rank = n - 1 - (int)royalRoads.size();
        if (treeRoyal * 8 <= rank && rebuilds < 8) {
            ++rebuilds;
            buildTree();
            continue;
        }

        vector<char> inTree(m, 0);
        for (int e : treeEdges) inTree[e] = 1;
        int f = -1;
        for (int attempt = 0; attempt < 40 && f == -1; ++attempt) {
            int e = rng() % m;
            if (state[e] == -1 && !inTree[e] &&
                    vertices.find(U[e]) != vertices.find(V[e])) f = e;
        }
        if (f == -1) {
            for (int e = 0; e < m; ++e) if (state[e] == -1 && !inTree[e]) {
                if (vertices.find(U[e]) == vertices.find(V[e])) state[e] = 0;
                else { f = e; break; }
            }
        }
        if (f == -1) {
            reduceWithTree();
            continue;
        }
        vector<int> path = pathInTree(f);
        vector<int> prefix;
        int baseline = (int)royalRoads.size() + treeRoyal;

        for (int e : path) {
            vector<int> query = treeEdges;
            query.erase(find(query.begin(), query.end(), e));
            query.push_back(f);
            int answer = ask(query);

            if (state[e] == 0) {
                if (answer == baseline) {
                    state[f] = 0;
                } else {
                    eraseTreeEdge(e);
                    makeRoyal(f);
                }
                break;
            }
            if (answer == baseline) {
                prefix.push_back(e);
                continue;
            }

            int fState = (answer == baseline + 1 ? 1 : 0);
            if (fState == 0) {
                state[f] = 0;
                for (int x : prefix) state[x] = 0;
                eraseTreeEdge(e);
                makeRoyal(e);
                --treeRoyal;
            } else {
                state[e] = 0;
                for (int x : prefix) {
                    eraseTreeEdge(x);
                    makeRoyal(x);
                    --treeRoyal;
                }
                eraseTreeEdge(e);
                makeRoyal(f);
            }
            break;
        }
        if (state[f] == -1) {
            state[f] = 0;
            for (int e : path) state[e] = 0;
        }
        rebuilds = 0;
    }

    cout << '!';
    for (int e : royalRoads) cout << ' ' << e;
    cout << '\n' << flush;
    return 0;
}

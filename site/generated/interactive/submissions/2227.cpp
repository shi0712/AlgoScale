#include <bits/stdc++.h>
using namespace std;

struct DSU {
    vector<int> p, sz;
    explicit DSU(int n = 0) { init(n); }
    void init(int n) {
        p.resize(n);
        sz.assign(n, 1);
        iota(p.begin(), p.end(), 0);
    }
    int find(int x) {
        if (p[x] == x) return x;
        return p[x] = find(p[x]);
    }
    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (sz[a] < sz[b]) swap(a, b);
        p[b] = a;
        sz[a] += sz[b];
        return true;
    }
    bool same(int a, int b) { return find(a) == find(b); }
};

struct Solver {
    int n, m;
    vector<int> eu, ev;
    vector<int> state; // -1: unknown, 0: non-royal, 1: royal
    DSU royalDSU, knownDSU;
    int royalCount = 0;
    int unknownCount;

    Solver(int n_, int m_, vector<int> u, vector<int> v)
        : n(n_), m(m_), eu(std::move(u)), ev(std::move(v)),
          state(m_, -1), royalDSU(n_), knownDSU(n_),
          unknownCount(m_) {}

    bool done() const { return royalCount == n - 1; }
    int remainingRoyal() const { return n - 1 - royalCount; }

    int ask(const vector<int>& tree) {
        cout << '?';
        for (int e : tree) cout << ' ' << e;
        cout << '\n' << flush;
        int answer;
        if (!(cin >> answer)) exit(0);
        if (answer < 0) exit(0);
        return answer;
    }

    void markRoyal(int e) {
        if (state[e] != -1) return;
        state[e] = 1;
        --unknownCount;
        ++royalCount;
        royalDSU.unite(eu[e], ev[e]);
        knownDSU.unite(eu[e], ev[e]);
    }

    void markNonRoyal(int e) {
        if (state[e] != -1) return;
        state[e] = 0;
        --unknownCount;
        knownDSU.unite(eu[e], ev[e]);
    }

    void markRoyal(const vector<int>& edges) {
        for (int e : edges) markRoyal(e);
    }

    void markNonRoyal(const vector<int>& edges) {
        for (int e : edges) markNonRoyal(e);
    }

    vector<int> unknownEdges() const {
        vector<int> result;
        result.reserve(unknownCount);
        for (int e = 0; e < m; ++e)
            if (state[e] == -1) result.push_back(e);
        return result;
    }

    vector<int> componentIds(const DSU& dsu) const {
        vector<int> id(n), rootToId(n, -1);
        int count = 0;
        for (int v = 0; v < n; ++v) {
            int r = dsu.p[v] == v ? v : dsu.p[v];
            // Path compression is not needed here; find the root explicitly.
            while (r != dsu.p[r]) r = dsu.p[r];
            if (rootToId[r] == -1) rootToId[r] = count++;
            id[v] = rootToId[r];
        }
        return id;
    }

    void freeReduce() {
        while (!done()) {
            if (unknownCount == remainingRoyal()) {
                markRoyal(unknownEdges());
                return;
            }

            vector<int> unknown = unknownEdges();
            for (int e : unknown) {
                if (state[e] == -1 && royalDSU.same(eu[e], ev[e]))
                    markNonRoyal(e);
            }

            if (done()) return;
            if (unknownCount == remainingRoyal()) {
                markRoyal(unknownEdges());
                return;
            }

            unknown = unknownEdges();
            if (unknown.empty()) return;

            vector<int> comp(n), rootToId(n, -1);
            int c = 0;
            for (int v = 0; v < n; ++v) {
                int r = royalDSU.find(v);
                if (rootToId[r] == -1) rootToId[r] = c++;
                comp[v] = rootToId[r];
            }

            vector<vector<pair<int, int>>> adj(c);
            for (int e : unknown) {
                int a = comp[eu[e]], b = comp[ev[e]];
                adj[a].push_back({b, e});
                adj[b].push_back({a, e});
            }

            vector<int> tin(c, -1), low(c, -1), bridges;
            int timer = 0;
            function<void(int, int)> dfs = [&](int v, int parentEdge) {
                tin[v] = low[v] = timer++;
                for (auto [to, e] : adj[v]) {
                    if (e == parentEdge) continue;
                    if (tin[to] != -1) {
                        low[v] = min(low[v], tin[to]);
                    } else {
                        dfs(to, e);
                        low[v] = min(low[v], low[to]);
                        if (low[to] > tin[v]) bridges.push_back(e);
                    }
                }
            };
            for (int v = 0; v < c; ++v)
                if (tin[v] == -1) dfs(v, -1);

            if (bridges.empty()) return;
            markRoyal(bridges);
        }
    }

    using ForestQuery = function<int(const vector<int>&)>;

    void classifyForest(const vector<int>& edges, int royal,
                        const ForestQuery& query) {
        if (done() || edges.empty()) return;
        if (royal == 0) {
            markNonRoyal(edges);
            return;
        }
        if (royal == (int)edges.size()) {
            markRoyal(edges);
            return;
        }

        int mid = (int)edges.size() / 2;
        vector<int> left(edges.begin(), edges.begin() + mid);
        vector<int> right(edges.begin() + mid, edges.end());

        int low = max(0, royal - (int)right.size());
        int high = min(royal, (int)left.size());
        int leftRoyal;
        if (low == high)
            leftRoyal = low;
        else
            leftRoyal = query(left);

        classifyForest(left, leftRoyal, query);
        if (done()) return;
        classifyForest(right, royal - leftRoyal, query);
    }

    void solveSet(const vector<int>& edges, int totalRoyal,
                  const ForestQuery& query) {
        int remainingInSet = totalRoyal;

        while (!done()) {
            vector<int> remaining;
            for (int e : edges) {
                if (state[e] != -1) continue;
                if (royalDSU.same(eu[e], ev[e]))
                    markNonRoyal(e);
                else
                    remaining.push_back(e);
            }
            if (remaining.empty()) return;

            if (remainingInSet == 0) {
                markNonRoyal(remaining);
                return;
            }
            if (remainingInSet == (int)remaining.size()) {
                markRoyal(remaining);
                return;
            }

            DSU dsu(n);
            vector<int> forest;
            for (int e : remaining) {
                if (dsu.unite(eu[e], ev[e])) forest.push_back(e);
            }

            int royal;
            if (remainingInSet >= 0 &&
                forest.size() == remaining.size()) {
                royal = remainingInSet;
            } else {
                royal = query(forest);
            }

            classifyForest(forest, royal, query);
            if (remainingInSet >= 0) remainingInSet -= royal;
        }
    }

    void cutIteration(const vector<int>& unknown) {
        vector<int> comp(n), rootToId(n, -1);
        int c = 0;
        for (int v = 0; v < n; ++v) {
            int r = royalDSU.find(v);
            if (rootToId[r] == -1) rootToId[r] = c++;
            comp[v] = rootToId[r];
        }

        DSU dsu(c);
        vector<int> treeEdges;
        vector<vector<pair<int, int>>> adj(c);
        for (int e : unknown) {
            int a = comp[eu[e]], b = comp[ev[e]];
            if (dsu.unite(a, b)) {
                treeEdges.push_back(e);
                adj[a].push_back({b, e});
                adj[b].push_back({a, e});
            }
        }

        vector<int> royalEdges;
        for (int e = 0; e < m; ++e)
            if (state[e] == 1) royalEdges.push_back(e);

        vector<int> base = royalEdges;
        base.insert(base.end(), treeEdges.begin(), treeEdges.end());
        int baseAnswer = ask(base);

        int k = (int)treeEdges.size();
        int r = baseAnswer - royalCount;

        if (r == k) {
            markRoyal(treeEdges);
            return;
        }
        if (r == 0) {
            markNonRoyal(treeEdges);
            return;
        }

        vector<char> inTree(m, 0);
        for (int e : treeEdges) inTree[e] = 1;
        vector<int> outside;
        for (int e : unknown)
            if (!inTree[e]) outside.push_back(e);

        int q = k - r;                 // royal roads outside treeEdges
        int s = (int)outside.size();
        int outsideNonRoyal = s - q;

        if (s > 0 && q == s) {
            markRoyal(outside);
            return;
        }

        vector<int> degreeU(c, 0);
        for (int e : unknown) {
            ++degreeU[comp[eu[e]]];
            ++degreeU[comp[ev[e]]];
        }

        int leaf = -1;
        for (int v = 0; v < c; ++v) {
            if (adj[v].size() == 1 &&
                (leaf == -1 || degreeU[v] < degreeU[leaf])) {
                leaf = v;
            }
        }

        int e0 = adj[leaf][0].second;
        vector<int> candidates;
        for (int e : unknown) {
            if (e == e0) continue;
            int a = comp[eu[e]], b = comp[ev[e]];
            if (a == leaf || b == leaf) candidates.push_back(e);
        }

        if (candidates.empty()) {
            markRoyal(e0);
            return;
        }

        auto otherEnd = [&](int e) {
            int a = comp[eu[e]], b = comp[ev[e]];
            return a == leaf ? b : a;
        };
        sort(candidates.begin(), candidates.end(),
             [&](int a, int b) {
                 return otherEnd(a) < otherEnd(b);
             });

        auto swappedQuery = [&](int f) {
            vector<int> tree = royalEdges;
            for (int e : treeEdges)
                if (e != e0) tree.push_back(e);
            tree.push_back(f);
            return ask(tree);
        };

        vector<int> sameEdges;
        set<int> neighbors;
        neighbors.insert(otherEnd(e0));
        bool royalPossible = true;

        for (int f : candidates) {
            int answer = swappedQuery(f);
            int delta = answer - baseAnswer;

            if (delta == 1) {
                markNonRoyal(e0);
                markNonRoyal(sameEdges);
                markRoyal(f);
                return;
            }
            if (delta == -1) {
                markRoyal(e0);
                markRoyal(sameEdges);
                markNonRoyal(f);
                return;
            }

            sameEdges.push_back(f);
            int z = (int)sameEdges.size();
            if (!neighbors.insert(otherEnd(f)).second)
                royalPossible = false;
            if (z > q) royalPossible = false;

            if (!royalPossible) {
                markNonRoyal(e0);
                markNonRoyal(sameEdges);
                return;
            }
            if (z > outsideNonRoyal) {
                markRoyal(e0);
                markRoyal(sameEdges);
                return;
            }
        }

        // Every possible edge across this cut has the same status.
        // They cannot all be non-royal, since the hidden tree crosses the cut.
        markRoyal(e0);
        markRoyal(sameEdges);
    }

    void run() {
        while (!done()) {
            freeReduce();
            if (done()) break;

            vector<int> unknown = unknownEdges();

            vector<int> kcomp(n), rootToId(n, -1);
            int p = 0;
            for (int v = 0; v < n; ++v) {
                int r = knownDSU.find(v);
                if (rootToId[r] == -1) rootToId[r] = p++;
                kcomp[v] = rootToId[r];
            }

            vector<int> internal;
            vector<int> cross;
            for (int e : unknown) {
                if (kcomp[eu[e]] == kcomp[ev[e]])
                    internal.push_back(e);
                else
                    cross.push_back(e);
            }

            if (internal.empty()) {
                cutIteration(unknown);
                continue;
            }

            // A known spanning tree inside every known component.
            DSU internalDSU(n);
            vector<int> knownTree;
            for (int e = 0; e < m; ++e) {
                if (state[e] != -1 && internalDSU.unite(eu[e], ev[e]))
                    knownTree.push_back(e);
            }

            // Unknown connectors between the known components.
            DSU connectorDSU(p);
            vector<int> connectors;
            for (int e : cross) {
                int a = kcomp[eu[e]], b = kcomp[ev[e]];
                if (connectorDSU.unite(a, b)) connectors.push_back(e);
            }

            vector<int> base = knownTree;
            base.insert(base.end(), connectors.begin(), connectors.end());

            int knownRoyalInBase = 0;
            for (int e : knownTree)
                knownRoyalInBase += (state[e] == 1);

            int baseAnswer;
            if (connectors.empty())
                baseAnswer = knownRoyalInBase;
            else
                baseAnswer = ask(base);

            int connectorRoyal = baseAnswer - knownRoyalInBase;

            if (!connectors.empty() && connectorRoyal == 0) {
                markNonRoyal(connectors);
                continue;
            }
            if (!connectors.empty() &&
                connectorRoyal == (int)connectors.size()) {
                markRoyal(connectors);
                continue;
            }

            int crossOutside =
                (int)cross.size() - (int)connectors.size();

            ForestQuery query = [&](const vector<int>& forest) {
                DSU dsu(n);
                vector<int> tree = connectors;
                tree.reserve(n - 1);

                for (int e : forest) {
                    dsu.unite(eu[e], ev[e]);
                    tree.push_back(e);
                }

                int addedKnownRoyal = 0;
                for (int e : knownTree) {
                    if (dsu.unite(eu[e], ev[e])) {
                        tree.push_back(e);
                        addedKnownRoyal += (state[e] == 1);
                    }
                }

                int answer = ask(tree);
                return answer - connectorRoyal - addedKnownRoyal;
            };

            int total = -1;
            if (crossOutside == 0)
                total = remainingRoyal() - connectorRoyal;

            solveSet(internal, total, query);
        }

        cout << '!';
        for (int e = 0; e < m; ++e)
            if (state[e] == 1) cout << ' ' << e;
        cout << '\n' << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;
    vector<int> u(m), v(m);
    for (int i = 0; i < m; ++i) {
        if (!(cin >> u[i] >> v[i])) return 0;
    }

    Solver solver(n, m, u, v);
    solver.run();
    return 0;
}

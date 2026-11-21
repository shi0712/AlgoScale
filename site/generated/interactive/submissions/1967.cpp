#include <bits/stdc++.h>
using namespace std;

// ------------------------------------------------------------------
// Interactive solution for "Simurgh".
//
// Hidden object: a spanning tree T* (the royal roads).
// Query: give a spanning tree T, receive |T intersect T*|.
//
// Outline:
//  1) Build a spanning tree T0 (BFS) and query it -> a0 = |T0 cap T*|.
//     If a0 == n-1 then T0 = T* and we are done.
//  2) Determine the status (royal / not royal) of every edge of T0.
//     For a non-tree edge f with fundamental path P (tree edges) a query
//     T0 - e + f yields the difference of answers  x_f - x_e.
//     * If no tree edge of P is known yet we query the whole path; the
//       values x_f - x_e cannot all be +1 (a whole cycle of royal roads
//       is impossible), so everything is determined.
//     * Otherwise one query anchors x_f using a known tree edge and one
//       query per still unknown tree edge of P determines it.
//     Each tree edge is determined at most once, hence phase 2 costs
//     at most 2(n-1) queries.
//  3) Every remaining unknown edge is a non-tree edge.  Exactly
//     d = (n-1) - a0 of them are royal and they form a forest (a subset
//     of T*).  A royal non-tree edge joins two different components of
//     T0 minus the non-royal tree edges, and those components are known.
//     The candidates are split into maximal forests; for any forest S the
//     exact number of royal edges inside S is obtained with one query
//     (Kruskal builds the spanning tree T0 - E + S).  Binary splitting,
//     where the count of one half determines the other half, locates all
//     royal candidates.
// ------------------------------------------------------------------

static int n, m;
static vector<int> eu, ev;
static vector<int> par_, depth_, pedge_;
static vector<int> treeEdges;
static vector<char> isTree, mark_;
static int a0;

struct DSU {
    vector<int> p;
    void init(int k) { p.resize(k); iota(p.begin(), p.end(), 0); }
    int find(int x) { while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; } return x; }
    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        p[a] = b;
        return true;
    }
};

static void emit(const char* tag, const vector<int>& ids) {
    string out;
    out.reserve(ids.size() * 4 + 2);
    out += tag[0];
    for (int e : ids) { out += ' '; out += to_string(e); }
    out += '\n';
    fwrite(out.data(), 1, out.size(), stdout);
    fflush(stdout);
}

static int doQuery(const vector<int>& edges) {
    emit("?", edges);
    int ans;
    if (scanf("%d", &ans) != 1) exit(0);   // input stream ended
    if (ans < 0) exit(0);                  // failure response
    return ans;
}

// Query T0 with one tree edge removed and one non-tree edge added.
static int swapQuery(int remTreeEdge, int addNonTree) {
    static vector<int> q;
    q.clear();
    q.reserve(n - 1);
    for (int e : treeEdges)
        if (e != remTreeEdge) q.push_back(e);
    q.push_back(addNonTree);
    return doQuery(q);
}

int main() {
    if (scanf("%d %d", &n, &m) != 2) return 0;
    eu.resize(m); ev.resize(m);
    vector<vector<pair<int, int>>> adj(n);
    for (int i = 0; i < m; i++) {
        if (scanf("%d %d", &eu[i], &ev[i]) != 2) return 0;
        adj[eu[i]].push_back({ev[i], i});
        adj[ev[i]].push_back({eu[i], i});
    }

    // If the graph is a tree it is the unique spanning tree: zero queries.
    if (m == n - 1) {
        vector<int> all(m);
        iota(all.begin(), all.end(), 0);
        emit("!", all);
        return 0;
    }

    // ---- spanning tree T0 via BFS ----
    par_.assign(n, -1);
    depth_.assign(n, 0);
    pedge_.assign(n, -1);
    isTree.assign(m, 0);
    {
        vector<char> vis(n, 0);
        vector<int> qu;
        qu.reserve(n);
        vis[0] = 1;
        qu.push_back(0);
        for (size_t qi = 0; qi < qu.size(); qi++) {
            int u = qu[qi];
            for (auto& pr : adj[u]) {
                int w = pr.first, id = pr.second;
                if (!vis[w]) {
                    vis[w] = 1;
                    par_[w] = u;
                    pedge_[w] = id;
                    depth_[w] = depth_[u] + 1;
                    isTree[id] = 1;
                    qu.push_back(w);
                }
            }
        }
    }
    for (int i = 0; i < m; i++)
        if (isTree[i]) treeEdges.push_back(i);

    a0 = doQuery(treeEdges);

    if (a0 == n - 1) {  // T0 is the royal set itself
        emit("!", treeEdges);
        return 0;
    }

    vector<int> xs(m, -1);  // -1 unknown, 0 not royal, 1 royal
    vector<int> nonTree;
    nonTree.reserve(m - (n - 1));
    for (int i = 0; i < m; i++)
        if (!isTree[i]) nonTree.push_back(i);

    mark_.assign(m, 0);

    // ---- Phase 1 : status of every tree edge ----
    {
        vector<int> path;
        for (int f : nonTree) {
            path.clear();
            int a = eu[f], b = ev[f];
            while (a != b) {
                if (depth_[a] < depth_[b]) swap(a, b);
                path.push_back(pedge_[a]);
                a = par_[a];
            }
            int e0 = -1;
            for (int e : path)
                if (xs[e] != -1) { e0 = e; break; }

            if (e0 == -1) {
                // Nothing known on the fundamental path: query the whole cycle.
                int L = (int)path.size();
                vector<int> val(L);
                int mn = INT_MAX, mx = INT_MIN;
                for (int i = 0; i < L; i++) {
                    int ans = swapQuery(path[i], f);
                    val[i] = ans - a0;   // = x_f - x_e
                    mn = min(mn, val[i]);
                    mx = max(mx, val[i]);
                }
                if (mn == 0 && mx == 0) {  // an all-royal cycle is impossible
                    xs[f] = 0;
                    for (int e : path) xs[e] = 0;
                } else if (mn < 0) {       // x_f = 0
                    xs[f] = 0;
                    for (int i = 0; i < L; i++) xs[path[i]] = (val[i] == -1) ? 1 : 0;
                } else {                   // x_f = 1
                    xs[f] = 1;
                    for (int i = 0; i < L; i++) xs[path[i]] = (val[i] == 1) ? 0 : 1;
                }
            } else {
                bool anyUnknown = false;
                for (int e : path)
                    if (xs[e] == -1) { anyUnknown = true; break; }
                if (!anyUnknown) continue;       // nothing new to learn
                if (xs[f] == -1) {
                    int ans = swapQuery(e0, f);
                    xs[f] = xs[e0] + (ans - a0);
                }
                for (int e : path)
                    if (xs[e] == -1) {
                        int ans = swapQuery(e, f);
                        xs[e] = xs[f] - (ans - a0);
                    }
            }
        }
        // A tree edge never determined is not covered by any non-tree edge:
        // it is a bridge and therefore royal.
        for (int e : treeEdges)
            if (xs[e] == -1) xs[e] = 1;
    }

    // ---- Phase 2 : locate royal non-tree edges ----
    int d = (n - 1) - a0;   // number of royal non-tree edges
    int knownGoodNT = 0;
    for (int f : nonTree)
        if (xs[f] == 1) knownGoodNT++;
    int rem = d - knownGoodNT;

    if (rem > 0) {
        DSU dsu;
        dsu.init(n);
        for (int e : treeEdges)
            if (xs[e] == 1) dsu.unite(eu[e], ev[e]);

        vector<int> cand;
        cand.reserve(nonTree.size());
        for (int f : nonTree)
            if (xs[f] == -1 && dsu.find(eu[f]) != dsu.find(ev[f]))
                cand.push_back(f);

        // Split candidates into maximal forests.
        vector<vector<int>> forests;
        {
            vector<char> used(cand.size(), 0);
            while (true) {
                DSU d2;
                d2.init(n);
                vector<int> F;
                bool any = false;
                for (size_t i = 0; i < cand.size(); i++) {
                    if (used[i]) continue;
                    any = true;
                    int f = cand[i];
                    if (d2.unite(eu[f], ev[f])) {
                        used[i] = 1;
                        F.push_back(f);
                    }
                }
                if (!any) break;
                forests.push_back(move(F));
            }
        }

        vector<int> removed;
        // number of royal edges inside the forest S
        auto countGood = [&](const vector<int>& S) -> int {
            DSU d3;
            d3.init(n);
            for (int f : S) d3.unite(eu[f], ev[f]);
            removed.clear();
            for (int e : treeEdges)
                if (!d3.unite(eu[e], ev[e])) removed.push_back(e);
            int sumE = 0;
            for (int e : removed) sumE += xs[e];
            static vector<int> q;
            q.clear();
            q.reserve(n - 1);
            for (int e : removed) mark_[e] = 1;
            for (int e : treeEdges)
                if (!mark_[e]) q.push_back(e);
            for (int e : removed) mark_[e] = 0;
            for (int f : S) q.push_back(f);
            int ans = doQuery(q);
            return ans - (a0 - sumE);
        };

        // Binary splitting; the count of one half determines the other.
        struct Item { vector<int> cur; int c; };
        vector<Item> stk;
        for (auto& F : forests) {
            if (rem <= 0) break;
            int c = countGood(F);
            if (c == 0) continue;
            if (c == (int)F.size()) {
                for (int f : F) xs[f] = 1;
                rem -= c;
                continue;
            }
            stk.clear();
            stk.push_back(Item{move(F), c});
            while (!stk.empty() && rem > 0) {
                Item it = move(stk.back());
                stk.pop_back();
                if ((int)it.cur.size() == it.c) {   // every edge is royal
                    for (int f : it.cur) xs[f] = 1;
                    rem -= it.c;
                    continue;
                }
                size_t mid = it.cur.size() / 2;
                vector<int> A(it.cur.begin(), it.cur.begin() + mid);
                vector<int> Bc(it.cur.begin() + mid, it.cur.end());
                int cA = countGood(A);
                int cB = it.c - cA;
                if (cA > 0) {
                    if (cA == (int)A.size()) {
                        for (int f : A) xs[f] = 1;
                        rem -= cA;
                    } else {
                        stk.push_back(Item{move(A), cA});
                    }
                }
                if (cB > 0 && rem > 0) {
                    if (cB == (int)Bc.size()) {
                        for (int f : Bc) xs[f] = 1;
                        rem -= cB;
                    } else {
                        stk.push_back(Item{move(Bc), cB});
                    }
                }
            }
        }
    }

    // ---- output the royal roads ----
    vector<int> ans;
    ans.reserve(n - 1);
    for (int e = 0; e < m; e++)
        if (xs[e] == 1) ans.push_back(e);
    emit("!", ans);
    return 0;
}

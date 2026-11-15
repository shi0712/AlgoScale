#include <bits/stdc++.h>
using namespace std;

/*
 * Numbers on a Tree
 *
 * A permutation sigma is reachable (where sigma(x) is the final vertex of the
 * token that starts at x) iff sigma is a single cycle and for every edge of the
 * tree, the component on one side of the edge is a contiguous arc of the cycle
 * (exactly one cycle edge leaves and one enters every component).
 *
 * Equivalent constructive model: give every vertex a linear order of its
 * incident edges.  The token starting at v leaves through the first edge; a
 * token entering v through the i-th edge leaves through the (i+1)-th edge, and
 * a token entering through the last edge stops at v.  Hence per vertex the
 * incident edges form one directed path: its first edge is a source
 * (no in-link) and its last edge is a sink (no out-link).
 *
 * Greedy: process tokens in increasing order of their number, and send the
 * token to the smallest reachable vertex such that the partial structure can
 * still be completed.  The completion condition turns out to be local: at
 * every vertex, if the first and the last edge already belong to the same
 * linked component, that component must already contain all incident edges.
 *
 * To make the search fast, we observe that at a vertex the only possible next
 * edges are the heads (sources) of the current link components inside that
 * vertex, so each vertex keeps an ordered set of (static component minimum,
 * head edge).  The search for the smallest feasible stop is a DFS of the tree
 * rooted at the token's start, pruned by those component minima.
 */

static const int INF = 1e9;

struct Solver {
    int n, m;
    vector<int> eu, ev;
    vector<vector<pair<int,int>>> adj;      // (neighbour, edge id)
    vector<int> compOfFirst, compOfSecond;  // static component minima
    vector<array<char,2>> usedD;            // used directions
    vector<int> inL, outL;                  // directed links between edge nodes
    vector<int> firstE, lastE;              // first/last edge of each vertex
    vector<char> occ;                       // vertex already chosen as a stop

    // permanent DSU over the 2*(n-1) edge-nodes
    vector<int> par, sz, headN, tailN, keyOfRoot;
    vector<char> inserted;                  // is root's entry present in S[v]
    vector<set<pair<int,int>>> S;           // vertex -> (min label, head node)

    inline int DIR(int v, int e) const { return v == eu[e] ? 0 : 1; }
    inline int NODE(int v, int e) const { return 2*e + (v == eu[e] ? 0 : 1); }
    inline int other(int v, int e) const { return eu[e] ^ ev[e] ^ v; }
    inline int deg(int v) const { return (int)adj[v].size(); }
    inline int mnDirOf(int v, int e) const {
        return v == eu[e] ? compOfSecond[e] : compOfFirst[e];
    }
    int find(int x) {
        while (par[x] != x) { par[x] = par[par[x]]; x = par[x]; }
        return x;
    }

    void init(int n_, vector<pair<int,int>> &E) {
        n = n_; m = (int)E.size();
        eu.resize(m); ev.resize(m); adj.assign(n, {});
        for (int e = 0; e < m; e++) {
            eu[e] = E[e].first; ev[e] = E[e].second;
            adj[eu[e]].push_back({ev[e], e});
            adj[ev[e]].push_back({eu[e], e});
        }
        usedD.assign(m, {0,0});
        inL.assign(2*m, -1); outL.assign(2*m, -1);
        firstE.assign(n, -1); lastE.assign(n, -1); occ.assign(n, 0);

        par.resize(2*m); sz.assign(2*m, 1);
        headN.resize(2*m); tailN.resize(2*m); keyOfRoot.assign(2*m, 0);
        for (int i = 0; i < 2*m; i++) { par[i] = i; headN[i] = i; tailN[i] = i; }

        // ---------- static minima of components after deleting each edge ----
        vector<int> submin(n, INF), upmin(n, INF), parent(n, -1), order;
        order.reserve(n);
        vector<char> vis(n, 0);
        vector<int> stk; stk.push_back(0); vis[0] = 1;
        while (!stk.empty()) {
            int u = stk.back(); stk.pop_back(); order.push_back(u);
            for (auto [v, e] : adj[u]) if (!vis[v]) { vis[v] = 1; parent[v] = u; stk.push_back(v); }
        }
        for (int i = n-1; i >= 0; i--) {
            int u = order[i]; submin[u] = u;
            for (auto [v, e] : adj[u]) if (parent[v] == u) submin[u] = min(submin[u], submin[v]);
        }
        upmin[0] = INF;
        vector<int> pref, suf;
        for (int u : order) {
            vector<pair<int,int>> ch;
            for (auto [v, e] : adj[u]) if (parent[v] == u) ch.push_back({v, e});
            int k = (int)ch.size();
            pref.assign(k+1, INF); suf.assign(k+1, INF);
            for (int i = 0; i < k; i++) pref[i+1] = min(pref[i], submin[ch[i].first]);
            for (int i = k-1; i >= 0; i--) suf[i] = min(suf[i+1], submin[ch[i].first]);
            for (int i = 0; i < k; i++) {
                int v = ch[i].first;
                upmin[v] = min({upmin[u], u, pref[i], suf[i+1]});
            }
        }
        compOfFirst.assign(m, 0); compOfSecond.assign(m, 0);
        for (int e = 0; e < m; e++) {
            int u = eu[e], v = ev[e];
            if (parent[v] == u) { compOfSecond[e] = submin[v]; compOfFirst[e] = upmin[v]; }
            else                { compOfFirst[e]  = submin[u]; compOfSecond[e] = upmin[u]; }
        }
        for (int v = 0; v < n; v++) {
            sort(adj[v].begin(), adj[v].end(), [&](const pair<int,int> &A, const pair<int,int> &B) {
                int ma = mnDirOf(v, A.second), mb = mnDirOf(v, B.second);
                if (ma != mb) return ma < mb;
                return A.second < B.second;
            });
        }

        // ---------- initial link components (every node separate) ----------
        S.assign(n, {});
        inserted.assign(2*m, 0);
        for (int v = 0; v < n; v++)
            for (auto [to, e] : adj[v]) {
                int node = NODE(v, e);
                keyOfRoot[node] = mnDirOf(v, e);
                S[v].insert({keyOfRoot[node], node});
                inserted[node] = 1;
            }
    }

    inline bool headAlive(int v, int headNode) {
        int e = headNode >> 1;
        int to = other(v, e);
        if (deg(to) == 1 && occ[to]) return false;               // dead leaf
        if (firstE[v] != -1 && NODE(v, firstE[v]) == headNode) return false;
        return true;
    }
    inline void eraseEntry(int v, int root) {
        if (inserted[root]) { S[v].erase({keyOfRoot[root], headN[root]}); inserted[root] = 0; }
    }
    inline void insertEntry(int v, int root) {
        int h = headN[root];
        if (headAlive(v, h)) {
            keyOfRoot[root] = mnDirOf(v, h >> 1);
            inserted[root] = 1;
            S[v].insert({keyOfRoot[root], h});
        } else inserted[root] = 0;
    }

    void mergeAt(int v, int aEdge, int dEdge) {
        int na = NODE(v, aEdge), nd = NODE(v, dEdge);
        int ra = find(na), rb = find(nd);
        eraseEntry(v, ra); eraseEntry(v, rb);
        outL[na] = nd; inL[nd] = na;
        int nh = headN[ra], nt = tailN[rb];
        int big = ra, small = rb;
        if (sz[big] < sz[small]) swap(big, small);
        par[small] = big; sz[big] += sz[small];
        headN[big] = nh; tailN[big] = nt;
        insertEntry(v, big);
    }

    bool okAfterMergeAt(int v, int aEdge, int dEdge) {
        int na = NODE(v, aEdge), nd = NODE(v, dEdge);
        int ra = find(na), rb = find(nd);
        if (ra == rb) return false;
        if (firstE[v] == -1 || lastE[v] == -1) return true;
        int rf = find(NODE(v, firstE[v])), rl = find(NODE(v, lastE[v]));
        auto inm = [&](int r) { return r == ra || r == rb; };
        if (inm(rf) && inm(rl)) return sz[ra] + sz[rb] == deg(v);
        return true;
    }
    bool okFirst(int v, int e) {
        if (lastE[v] == -1) return true;
        int rf = find(NODE(v, e)), rl = find(NODE(v, lastE[v]));
        if (rf != rl) return true;
        return sz[rf] == deg(v);
    }
    bool okLast(int v, int e) {
        if (firstE[v] == -1) return true;
        int rf = find(NODE(v, firstE[v])), rl = find(NODE(v, e));
        if (rf != rl) return true;
        return sz[rf] == deg(v);
    }
    void killHeadEdge(int v, int e) {
        int node = NODE(v, e);
        int r = find(node);
        if (headN[r] == node && inserted[r]) { S[v].erase({keyOfRoot[r], node}); inserted[r] = 0; }
    }

    vector<int> solve(const vector<int> &a) {
        vector<int> P(n, -1);
        vector<int> parEdge(n, -1);
        for (int i = 0; i < n; i++) {
            int x = a[i];
            int bestV = INF;

            struct Frame { int u, arr; set<pair<int,int>>::iterator it; };
            vector<Frame> stk;
            stk.push_back({x, -1, S[x].begin()});

            while (!stk.empty()) {
                int top = (int)stk.size() - 1;
                int u = stk[top].u, arr = stk[top].arr;

                // Can the token stop here?
                if (arr != -1 && !occ[u] && (deg(u) == 1 || arr != firstE[u]) && okLast(u, arr))
                    if (u < bestV) bestV = u;

                bool descended = false;
                set<pair<int,int>> &Su = S[u];
                while (stk[top].it != Su.end()) {
                    auto [mc, hnode] = *stk[top].it;
                    if (mc >= bestV) break;          // cannot improve any more
                    int e = hnode >> 1;
                    bool ok = true;
                    if (arr == -1) {                 // first edge of this token
                        if (deg(u) > 1 && e == lastE[u]) ok = false;
                        if (ok && usedD[e][DIR(u,e)]) ok = false;
                        if (ok && !okFirst(u, e)) ok = false;
                    } else {
                        if (find(NODE(u, arr)) == find(hnode)) ok = false; // same component
                        else if (usedD[e][DIR(u,e)]) ok = false;
                        else if (!okAfterMergeAt(u, arr, e)) ok = false;
                    }
                    ++stk[top].it;
                    if (!ok) continue;
                    int v = other(u, e);
                    parEdge[v] = e;
                    stk.push_back({v, e, S[v].begin()});
                    descended = true;
                    break;
                }
                if (!descended) stk.pop_back();
            }

            if (bestV == INF) {
                fprintf(stderr, "NO STOP token=%d x=%d n=%d\n", i, x, n);
                vector<int> stk2; vector<char> seen(n,0); vector<int> arr2(n,-1);
                stk2.push_back(x); seen[x]=1; int found=0;
                while(!stk2.empty()){
                    int u=stk2.back(); stk2.pop_back();
                    int arr=arr2[u];
                    if(arr!=-1 && !occ[u] && (deg(u)==1 || arr!=firstE[u]) && okLast(u,arr)){ found++; if(found<5)fprintf(stderr,"  STOP u=%d\n",u); }
                    for(auto [to,e]:adj[u]){
                        if(e==arr) continue;
                        bool okk=true;
                        if(arr==-1){
                            if(deg(u)>1 && e==lastE[u]) okk=false;
                            if(okk && usedD[e][DIR(u,e)]) okk=false;
                            if(okk && !okFirst(u,e)) okk=false;
                            if(okk && inL[NODE(u,e)]!=-1) okk=false;
                        } else {
                            int nd=NODE(u,e), na=NODE(u,arr);
                            if(inL[nd]!=-1) okk=false;
                            if(okk && find(na)==find(nd)) okk=false;
                            if(okk && usedD[e][DIR(u,e)]) okk=false;
                            if(okk && !okAfterMergeAt(u,arr,e)) okk=false;
                        }
                        if(!okk) continue;
                        int nd=NODE(u,e);
                        if(!S[u].count({mnDirOf(u,e),nd})) fprintf(stderr,"  MISSING u=%d e=%d to=%d arr=%d\n",u,e,to,arr);
                        if(seen[to]) continue;
                        seen[to]=1; arr2[to]=e; stk2.push_back(to);
                    }
                }
                fprintf(stderr,"  fullscan stops=%d\n",found);
            }
            if (bestV == INF) {          // cannot happen for a valid tree
                P[i] = -1;
                return P;
            }
            // apply the chosen path
            vector<int> path;
            {
                int cur = bestV;
                while (cur != x) { int e = parEdge[cur]; path.push_back(e); cur = other(cur, e); }
            }
            reverse(path.begin(), path.end());
            int cur = x, ar = -1;
            for (int e : path) {
                int d = DIR(cur, e); usedD[e][d] = 1;
                if (ar == -1) {
                    firstE[cur] = e;
                    int node = NODE(cur, e), r = find(node);
                    if (headN[r] == node && inserted[r]) {
                        S[cur].erase({keyOfRoot[r], node}); inserted[r] = 0;
                    }
                } else {
                    mergeAt(cur, ar, e);
                }
                cur = other(cur, e); ar = e;
            }
            occ[bestV] = 1; lastE[bestV] = ar;
            if (deg(bestV) == 1) {                   // a dead leaf is never useful again
                int nb = adj[bestV][0].first, e = adj[bestV][0].second;
                killHeadEdge(nb, e);
            }
            P[i] = bestV;
        }
        return P;
    }
};

int main() {
    // fast input
    static char buf[1 << 24];
    size_t len = fread(buf, 1, sizeof(buf), stdin);
    char *p = buf, *end = buf + len;
    auto readInt = [&](int &out) -> bool {
        while (p < end && (*p < '0' || *p > '9')) ++p;
        if (p >= end) return false;
        int v = 0;
        while (p < end && *p >= '0' && *p <= '9') { v = v*10 + (*p - '0'); ++p; }
        out = v; return true;
    };
    int T; readInt(T);
    while (T--) {
        int n; readInt(n);
        vector<int> a(n);
        for (int i = 0; i < n; i++) { int x; readInt(x); a[i] = x - 1; }
        if (n == 1) { puts("1"); continue; }
        vector<pair<int,int>> E(n - 1);
        for (int i = 0; i < n - 1; i++) { int x, y; readInt(x); readInt(y); E[i] = {x-1, y-1}; }
        Solver S; S.init(n, E);
        vector<int> P = S.solve(a);
        string out;
        out.reserve(n * 7);
        for (int i = 0; i < n; i++) {
            out += to_string(P[i] + 1);
            out += (i == n-1 ? '\n' : ' ');
        }
        fwrite(out.data(), 1, out.size(), stdout);
    }
    return 0;
}

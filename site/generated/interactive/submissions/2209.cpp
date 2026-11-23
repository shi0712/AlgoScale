#include <bits/stdc++.h>
using namespace std;

struct Query {
    int u = 1, k = 0, score = INT_MAX;
};

class Solver {
public:
    int n;
    vector<vector<int>> adj, children;
    vector<int> par, depth, tin, tout, sz;
    vector<vector<int>> up;
    vector<int> dist, freq, bfsQueue;
    int timer = 0, LOG = 17, treeCentroid = 1;

    explicit Solver(int n_) : n(n_) {
        adj.assign(n + 1, {});
        children.assign(n + 1, {});
        par.assign(n + 1, 1);
        depth.assign(n + 1, 0);
        tin.assign(n + 1, 0);
        tout.assign(n + 1, 0);
        sz.assign(n + 1, 1);
        dist.assign(n + 1, -1);
        freq.assign(n + 1, 0);
        bfsQueue.resize(n + 1);
        up.assign(LOG, vector<int>(n + 1, 1));
    }

    void build() {
        for (int i = 2; i <= n; ++i) {
            adj[par[i]].push_back(i);
            adj[i].push_back(par[i]);
            children[par[i]].push_back(i);
            depth[i] = depth[par[i]] + 1;
        }

        vector<int> index(n + 1, 0), st = {1};
        tin[1] = timer++;
        while (!st.empty()) {
            int u = st.back();
            if (index[u] < (int)children[u].size()) {
                int v = children[u][index[u]++];
                tin[v] = timer++;
                st.push_back(v);
            } else {
                tout[u] = timer - 1;
                st.pop_back();
            }
        }

        for (int i = n; i >= 2; --i) sz[par[i]] += sz[i];
        int u = 1;
        while (true) {
            int nxt = 0;
            for (int v : children[u]) {
                if (2 * sz[v] > n) {
                    nxt = v;
                    break;
                }
            }
            if (!nxt) break;
            u = nxt;
        }
        treeCentroid = u;

        for (int v = 1; v <= n; ++v) up[0][v] = par[v];
        for (int j = 1; j < LOG; ++j) {
            for (int v = 1; v <= n; ++v) {
                up[j][v] = up[j - 1][up[j - 1][v]];
            }
        }
    }

    int kthAncestor(int v, int k) const {
        for (int j = 0; j < LOG; ++j) {
            if (k & (1 << j)) v = up[j][v];
        }
        return v;
    }

    int lca(int a, int b) const {
        if (depth[a] < depth[b]) swap(a, b);
        a = kthAncestor(a, depth[a] - depth[b]);
        if (a == b) return a;
        for (int j = LOG - 1; j >= 0; --j) {
            if (up[j][a] != up[j][b]) {
                a = up[j][a];
                b = up[j][b];
            }
        }
        return up[0][a];
    }

    int lowerTin(const vector<int>& S, int value) const {
        return int(lower_bound(S.begin(), S.end(), value,
            [&](int v, int x) { return tin[v] < x; }) - S.begin());
    }

    int subtreeCount(const vector<int>& S, int u) const {
        return lowerTin(S, tout[u] + 1) - lowerTin(S, tin[u]);
    }

    int weightedCentroid(const vector<int>& S) const {
        int m = int(S.size()), u = 1;
        while (true) {
            int nxt = 0;
            for (int v : children[u]) {
                if (2 * subtreeCount(S, v) > m) {
                    nxt = v;
                    break;
                }
            }
            if (!nxt) break;
            u = nxt;
        }
        return u;
    }

    void bfs(int source) {
        fill(dist.begin(), dist.end(), -1);
        int qh = 0, qt = 0;
        bfsQueue[qt++] = source;
        dist[source] = 0;
        while (qh < qt) {
            int u = bfsQueue[qh++];
            for (int v : adj[u]) {
                if (dist[v] == -1) {
                    dist[v] = dist[u] + 1;
                    bfsQueue[qt++] = v;
                }
            }
        }
    }

    int evaluateCenter(int u, const vector<int>& S, Query& best) {
        bfs(u);
        int m = int(S.size());
        int farthest = S[0], maxDistance = 0;
        vector<int> touched;

        for (int v : S) {
            int d = dist[v];
            if (freq[d] == 0) touched.push_back(d);
            ++freq[d];
            maxDistance = max(maxDistance, d);
            if (d > dist[farthest]) farthest = v;
        }

        int cumulative = 0;
        for (int r = 0; r <= maxDistance; ++r) {
            cumulative += freq[r];
            if (cumulative > 0 && cumulative < m) {
                int score = max(cumulative, m - cumulative);
                if (score < best.score) best = {u, r, score};
            }
        }

        for (int d : touched) freq[d] = 0;
        return farthest;
    }

    int nodeOnPath(int a, int b, int t) const {
        int c = lca(a, b);
        int da = depth[a] - depth[c];
        int total = depth[a] + depth[b] - 2 * depth[c];
        if (t <= da) return kthAncestor(a, t);
        return kthAncestor(b, total - t);
    }

    Query findQuery(const vector<int>& S) {
        int m = int(S.size());
        int target = (m + 1) / 2;
        Query best;
        best.score = m + 1;

        evaluateCenter(1, S, best);
        if (best.score == target) return best;

        evaluateCenter(treeCentroid, S, best);
        if (best.score == target) return best;

        int wc = weightedCentroid(S);
        evaluateCenter(wc, S, best);
        if (best.score == target) return best;

        int lc = lca(S.front(), S.back());
        evaluateCenter(lc, S, best);
        if (best.score == target) return best;

        int a = evaluateCenter(S[0], S, best);
        if (best.score == target) return best;
        int b = evaluateCenter(a, S, best);
        if (best.score == target) return best;
        evaluateCenter(b, S, best);
        if (best.score == target) return best;

        vector<int> centers;
        auto add = [&](int u) {
            if (1 <= u && u <= n) centers.push_back(u);
        };

        int pathLength = depth[a] + depth[b] - 2 * depth[lca(a, b)];
        for (int i = 0; i <= 6; ++i) {
            add(nodeOnPath(a, b, pathLength * i / 6));
        }

        for (int u : {wc, lc, treeCentroid}) {
            add(par[u]);
            for (int v : children[u]) add(v);
        }

        for (int i = 0; i < 8; ++i) {
            add(S[int(1LL * i * (m - 1) / 7)]);
        }

        int minDepthVertex = S[0], maxDepthVertex = S[0];
        for (int v : S) {
            if (depth[v] < depth[minDepthVertex]) minDepthVertex = v;
            if (depth[v] > depth[maxDepthVertex]) maxDepthVertex = v;
        }
        add(minDepthVertex);
        add(maxDepthVertex);

        if (m <= 48) {
            for (int v : S) add(v);
        }
        if (n <= 256) {
            for (int v = 1; v <= n; ++v) add(v);
        }

        sort(centers.begin(), centers.end());
        centers.erase(unique(centers.begin(), centers.end()), centers.end());
        for (int u : centers) {
            evaluateCenter(u, S, best);
            if (best.score == target) break;
        }
        return best;
    }

    void run() {
        vector<int> candidates(n);
        iota(candidates.begin(), candidates.end(), 1);
        sort(candidates.begin(), candidates.end(),
            [&](int a, int b) { return tin[a] < tin[b]; });

        while (candidates.size() > 1) {
            int m = int(candidates.size());
            int minDepth = n, maxDepth = 0;
            for (int v : candidates) {
                minDepth = min(minDepth, depth[v]);
                maxDepth = max(maxDepth, depth[v]);
            }

            Query best = findQuery(candidates);
            int u, k;

            if (best.score <= 3 * m / 4) {
                u = best.u;
                k = best.k;
            } else if (minDepth != maxDepth) {
                u = 1;
                k = (minDepth + maxDepth) / 2;
            } else {
                int wc = weightedCentroid(candidates);
                int chosenChild = 0, chosenCount = 0;
                for (int v : children[wc]) {
                    int cnt = subtreeCount(candidates, v);
                    if (cnt > chosenCount) {
                        chosenCount = cnt;
                        chosenChild = v;
                    }
                }

                if (chosenChild) {
                    int fallbackScore = max(chosenCount, m - chosenCount);
                    if (best.score < fallbackScore) {
                        u = best.u;
                        k = best.k;
                    } else {
                        u = chosenChild;
                        k = minDepth - depth[chosenChild];
                    }
                } else {
                    u = candidates[0];
                    k = 0;
                }
            }

            cout << "? " << u << ' ' << k << '\n' << flush;
            int response;
            if (!(cin >> response) || response == -1) return;
            if (response != 0 && response != 1) return;

            bfs(u);
            vector<int> next;
            next.reserve(best.score > 0 && best.score <= m ? best.score : m);
            for (int v : candidates) {
                bool inside = dist[v] <= k;
                if ((response == 1 && inside) || (response == 0 && !inside)) {
                    next.push_back(v);
                }
            }
            candidates.swap(next);
            if (candidates.empty()) return;
        }

        cout << "! " << candidates[0] << '\n' << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    Solver solver(n);
    for (int i = 2; i <= n; ++i) {
        if (!(cin >> solver.par[i])) return 0;
    }
    solver.build();
    solver.run();
    return 0;
}

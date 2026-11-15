#include <bits/stdc++.h>
using namespace std;

struct LCA {
    int n, L, LOG;
    vector<int> depth, first, lg, euler;
    vector<vector<int>> st;

    LCA(int n_, vector<int>& parent, vector<int>& depth_, vector<vector<int>>& adj)
        : n(n_), depth(depth_) {
        first.assign(n + 1, -1);
        vector<int> it(n + 1, 0);
        euler.reserve(2 * n - 1);

        stack<int> stk;
        stk.push(1);
        first[1] = 0;
        euler.push_back(1);

        while (!stk.empty()) {
            int v = stk.top();
            if (it[v] == (int)adj[v].size()) {
                stk.pop();
                if (!stk.empty()) euler.push_back(stk.top());
            } else {
                int to = adj[v][it[v]++];
                if (to == parent[v]) continue;
                first[to] = (int)euler.size();
                euler.push_back(to);
                stk.push(to);
            }
        }

        L = (int)euler.size();

        lg.assign(L + 1, 0);
        for (int i = 2; i <= L; ++i) lg[i] = lg[i / 2] + 1;

        LOG = lg[L] + 1;
        st.assign(LOG, vector<int>(L));

        for (int i = 0; i < L; ++i) st[0][i] = euler[i];

        for (int k = 1; k < LOG; ++k) {
            int len = 1 << (k - 1);
            int span = 1 << k;
            for (int i = 0; i + span <= L; ++i) {
                int a = st[k - 1][i];
                int b = st[k - 1][i + len];
                st[k][i] = (depth[a] < depth[b] ? a : b);
            }
        }
    }

    int lcaQuery(int u, int v) const {
        int l = first[u], r = first[v];
        if (l > r) swap(l, r);
        int k = lg[r - l + 1];
        int a = st[k][l];
        int b = st[k][r - (1 << k) + 1];
        return (depth[a] < depth[b] ? a : b);
    }

    int dist(int u, int v) const {
        int w = lcaQuery(u, v);
        return depth[u] + depth[v] - 2 * depth[w];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    mt19937 rng((unsigned)chrono::steady_clock::now().time_since_epoch().count());

    while (T--) {
        int n;
        cin >> n;

        vector<int> parent(n + 1, 0), depth(n + 1, 0);
        vector<vector<int>> adj(n + 1);

        for (int i = 2; i <= n; ++i) {
            int p;
            cin >> p;
            parent[i] = p;
            depth[i] = depth[p] + 1;
            adj[p].push_back(i);
            adj[i].push_back(p);
        }

        LCA lca(n, parent, depth, adj);

        auto get_far = [&](int src) {
            vector<int> dist(n + 1, -1);
            queue<int> q;
            q.push(src);
            dist[src] = 0;

            int far = src;
            while (!q.empty()) {
                int v = q.front();
                q.pop();
                if (dist[v] > dist[far]) far = v;
                for (int to : adj[v]) {
                    if (dist[to] == -1) {
                        dist[to] = dist[v] + 1;
                        q.push(to);
                    }
                }
            }
            return far;
        };

        int diamA = get_far(1);
        int diamB = get_far(diamA);

        vector<int> sub(n + 1, 0);

        // Centroid of the whole tree.
        fill(sub.begin(), sub.end(), 1);
        sub[0] = 0;
        for (int v = n; v >= 2; --v) sub[parent[v]] += sub[v];

        int treeCentroid = 1;
        while (true) {
            int heavy = -1;
            for (int to : adj[treeCentroid]) {
                if (parent[to] == treeCentroid && sub[to] > n / 2) {
                    heavy = to;
                    break;
                }
            }
            if (heavy == -1) break;
            treeCentroid = heavy;
        }

        vector<int> candidates(n);
        iota(candidates.begin(), candidates.end(), 1);

        vector<int> seenCenters(n + 1, 0);
        int centerStamp = 0;

        vector<int> freq(n + 1, 0), used;
        vector<int> ds;
        ds.reserve(n);

        for (int queryId = 0; queryId < 21 && candidates.size() > 1; ++queryId) {
            int m = (int)candidates.size();

            ++centerStamp;
            vector<int> centers;
            centers.reserve(700);

            auto addCenter = [&](int v) {
                if (v < 1 || v > n) return;
                if (seenCenters[v] == centerStamp) return;
                seenCenters[v] = centerStamp;
                centers.push_back(v);
            };

            // Compute a weighted centroid of current candidates.
            fill(sub.begin(), sub.end(), 0);
            for (int v : candidates) sub[v] = 1;
            for (int v = n; v >= 2; --v) sub[parent[v]] += sub[v];

            int candCentroid = 1;
            while (true) {
                int heavy = -1;
                for (int to : adj[candCentroid]) {
                    if (parent[to] == candCentroid && sub[to] > m / 2) {
                        heavy = to;
                        break;
                    }
                }
                if (heavy == -1) break;
                candCentroid = heavy;
            }

            addCenter(1);
            addCenter(treeCentroid);
            addCenter(diamA);
            addCenter(diamB);
            addCenter(candCentroid);

            for (int to : adj[candCentroid]) addCenter(to);

            if (m <= 800) {
                for (int v : candidates) addCenter(v);
            } else {
                int sample = 260;
                for (int i = 0; i < sample; ++i) {
                    int idx = (int)(rng() % m);
                    addCenter(candidates[idx]);
                }
                for (int i = 0; i < sample; ++i) {
                    int idx = (int)((long long)i * m / sample);
                    if (idx >= m) idx = m - 1;
                    addCenter(candidates[idx]);
                }
            }

            // Some globally random centers, to avoid symmetric obstructions.
            for (int i = 0; i < 100; ++i) {
                addCenter((int)(rng() % n) + 1);
            }

            int bestCenter = -1;
            int bestK = 0;
            int bestMax = INT_MAX;
            int bestYes = 0;

            for (int u : centers) {
                int localBestMax = INT_MAX;
                int localBestK = 0;
                int localBestYes = 0;

                if (m <= 2000) {
                    ds.clear();
                    for (int v : candidates) {
                        ds.push_back(lca.dist(u, v));
                    }

                    sort(ds.begin(), ds.end());

                    for (int i = 0; i < m; ) {
                        int j = i;
                        while (j < m && ds[j] == ds[i]) ++j;

                        int yes = j;
                        int no = m - yes;
                        int curMax = max(yes, no);

                        if (curMax < localBestMax ||
                            (curMax == localBestMax &&
                             abs(yes - m / 2) < abs(localBestYes - m / 2))) {
                            localBestMax = curMax;
                            localBestK = ds[i];
                            localBestYes = yes;
                        }

                        i = j;
                    }
                } else {
                    used.clear();
                    int maxD = 0;

                    for (int v : candidates) {
                        int d = lca.dist(u, v);
                        if (freq[d] == 0) used.push_back(d);
                        ++freq[d];
                        if (d > maxD) maxD = d;
                    }

                    int acc = 0;
                    for (int d = 0; d <= maxD; ++d) {
                        if (freq[d] == 0) continue;
                        acc += freq[d];

                        int yes = acc;
                        int no = m - yes;
                        int curMax = max(yes, no);

                        if (curMax < localBestMax ||
                            (curMax == localBestMax &&
                             abs(yes - m / 2) < abs(localBestYes - m / 2))) {
                            localBestMax = curMax;
                            localBestK = d;
                            localBestYes = yes;
                        }
                    }

                    for (int d : used) freq[d] = 0;
                }

                if (localBestMax < bestMax ||
                    (localBestMax == bestMax &&
                     abs(localBestYes - m / 2) < abs(bestYes - m / 2))) {
                    bestMax = localBestMax;
                    bestCenter = u;
                    bestK = localBestK;
                    bestYes = localBestYes;
                }
            }

            if (bestCenter == -1) {
                bestCenter = candidates[0];
                bestK = 0;
            }

            cout << "? " << bestCenter << ' ' << bestK << '\n';
            cout.flush();

            int resp;
            cin >> resp;

            vector<int> nxt;
            nxt.reserve(candidates.size());

            for (int v : candidates) {
                bool inside = (lca.dist(bestCenter, v) <= bestK);
                if ((inside ? 1 : 0) == resp) {
                    nxt.push_back(v);
                }
            }

            candidates.swap(nxt);

            if (candidates.empty()) {
                // Should not happen for a valid interactor.
                candidates.push_back(bestCenter);
            }
        }

        cout << "! " << candidates[0] << '\n';
        cout.flush();
    }

    return 0;
}

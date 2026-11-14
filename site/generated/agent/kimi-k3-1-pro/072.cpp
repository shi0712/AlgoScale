#include <algorithm>
#include <iostream>
#include <numeric>
#include <queue>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    if (!(cin >> T)) return 0;

    while (T--) {
        int n;
        if (!(cin >> n)) return 0;

        vector<int> parent(n + 1, 0);
        vector<vector<int>> adj(n + 1);
        for (int v = 2; v <= n; ++v) {
            cin >> parent[v];
            adj[parent[v]].push_back(v);
            adj[v].push_back(parent[v]);
        }

        vector<int> candidates(n);
        iota(candidates.begin(), candidates.end(), 1);
        vector<int> dist(n + 1), bfsQueue(n + 1);

        auto getDistances = [&](int source) {
            fill(dist.begin(), dist.end(), -1);
            int head = 0, tail = 0;
            bfsQueue[tail++] = source;
            dist[source] = 0;
            while (head < tail) {
                int v = bfsQueue[head++];
                for (int to : adj[v]) {
                    if (dist[to] == -1) {
                        dist[to] = dist[v] + 1;
                        bfsQueue[tail++] = to;
                    }
                }
            }
        };

        while (candidates.size() > 1) {
            vector<int> subtree(n + 1, 0);
            for (int v : candidates) subtree[v] = 1;
            for (int v = n; v >= 2; --v) subtree[parent[v]] += subtree[v];

            int centroid = 1;
            int half = (int)candidates.size() / 2;
            while (true) {
                int next = -1;
                for (int to : adj[centroid]) {
                    if (to != parent[centroid] && subtree[to] > half) {
                        next = to;
                        break;
                    }
                }
                if (next == -1) break;
                centroid = next;
            }

            int bestWorst = (int)candidates.size() + 1;
            int bestVertex = centroid, bestK = 0;
            vector<int> bestCandidateDistances;

            auto consider = [&](int source) {
                getDistances(source);
                vector<int> current;
                current.reserve(candidates.size());
                for (int v : candidates) current.push_back(dist[v]);
                vector<int> sorted = current;
                sort(sorted.begin(), sorted.end());

                int m = (int)sorted.size();
                for (int i = 0; i < m; ) {
                    int j = i;
                    while (j < m && sorted[j] == sorted[i]) ++j;
                    int worst = max(j, m - j);
                    if (worst < bestWorst) {
                        bestWorst = worst;
                        bestVertex = source;
                        bestK = sorted[i];
                        bestCandidateDistances = current;
                    }
                    i = j;
                }
            };

            consider(centroid);
            for (int to : adj[centroid]) consider(to);

            cout << "? " << bestVertex << ' ' << bestK << endl;
            int response;
            if (!(cin >> response)) return 0;

            vector<int> next;
            next.reserve(bestWorst);
            for (int i = 0; i < (int)candidates.size(); ++i) {
                if ((bestCandidateDistances[i] <= bestK) == (response == 1)) {
                    next.push_back(candidates[i]);
                }
            }
            candidates.swap(next);
        }

        cout << "! " << candidates[0] << endl;
    }

    return 0;
}

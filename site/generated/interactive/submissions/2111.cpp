#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    int n;
    if (!(cin >> n)) return 0;
    vector<vector<int>> adj(n + 1);
    for (int i = 2; i <= n; ++i) {
        int p;
        cin >> p;
        adj[p].push_back(i);
        adj[i].push_back(p);
    }

    vector<int> candidates;
    candidates.reserve(n);
    for (int i = 1; i <= n; ++i) candidates.push_back(i);

    vector<int> dist(n + 1);
    while (candidates.size() > 1) {
        int u = candidates[0];               // pick any candidate as query vertex

        // BFS to compute distances from u to all vertices
        fill(dist.begin(), dist.end(), -1);
        queue<int> q;
        dist[u] = 0;
        q.push(u);
        while (!q.empty()) {
            int v = q.front(); q.pop();
            for (int to : adj[v]) {
                if (dist[to] == -1) {
                    dist[to] = dist[v] + 1;
                    q.push(to);
                }
            }
        }

        // collect distances for current candidates
        vector<int> dists;
        dists.reserve(candidates.size());
        for (int v : candidates) dists.push_back(dist[v]);

        // lower median (index (m-1)/2)
        size_t idx = (dists.size() - 1) / 2;
        nth_element(dists.begin(), dists.begin() + idx, dists.end());
        int k = dists[idx];

        // send query
        cout << "? " << u << " " << k << "\n";
        cout.flush();

        int resp;
        if (!(cin >> resp) || resp == -1) return 0;

        // filter candidates according to the answer
        vector<int> newcandidates;
        newcandidates.reserve(candidates.size());
        for (int v : candidates) {
            if ((resp == 1 && dist[v] <= k) || (resp == 0 && dist[v] > k))
                newcandidates.push_back(v);
        }
        candidates.swap(newcandidates);
    }

    if (!candidates.empty()) {
        cout << "! " << candidates[0] << "\n";
        cout.flush();
    }
    return 0;
}

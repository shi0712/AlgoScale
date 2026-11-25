#include <algorithm>
#include <iostream>
#include <numeric>
#include <tuple>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> parent(n, -1);
    vector<vector<int>> children(n), adj(n);
    for (int v = 1; v < n; ++v) {
        int p;
        if (!(cin >> p)) return 0;
        --p;
        parent[v] = p;
        children[p].push_back(v);
        adj[p].push_back(v);
        adj[v].push_back(p);
    }

    auto getDistances = [&](int source, vector<int>& dist) {
        fill(dist.begin(), dist.end(), -1);
        vector<int> queue(n);
        int head = 0, tail = 0;
        dist[source] = 0;
        queue[tail++] = source;
        while (head < tail) {
            int u = queue[head++];
            for (int v : adj[u]) {
                if (dist[v] != -1) continue;
                dist[v] = dist[u] + 1;
                queue[tail++] = v;
            }
        }
    };

    vector<int> candidates(n);
    iota(candidates.begin(), candidates.end(), 0);
    vector<int> subtreeCandidates(n), dist(n), centroidDist(n), branch(n);
    vector<int> nearest(n), farthest(n);
    vector<unsigned char> isCandidate(n);

    while (candidates.size() > 1) {
        int m = static_cast<int>(candidates.size());
        fill(isCandidate.begin(), isCandidate.end(), 0);
        for (int v : candidates) isCandidate[v] = 1;
        for (int u = 0; u < n; ++u) subtreeCandidates[u] = isCandidate[u];
        for (int u = n - 1; u > 0; --u) {
            subtreeCandidates[parent[u]] += subtreeCandidates[u];
        }

        int centroid = 0;
        bool moved = true;
        while (moved) {
            moved = false;
            for (int v : children[centroid]) {
                if (subtreeCandidates[v] > m / 2) {
                    centroid = v;
                    moved = true;
                    break;
                }
            }
        }

        fill(centroidDist.begin(), centroidDist.end(), -1);
        fill(branch.begin(), branch.end(), -1);
        vector<int> queue(n);
        int head = 0, tail = 0;
        centroidDist[centroid] = 0;
        queue[tail++] = centroid;
        while (head < tail) {
            int u = queue[head++];
            for (int v : adj[u]) {
                if (centroidDist[v] != -1) continue;
                centroidDist[v] = centroidDist[u] + 1;
                branch[v] = (u == centroid ? v : branch[u]);
                queue[tail++] = v;
            }
        }

        fill(nearest.begin(), nearest.end(), -1);
        fill(farthest.begin(), farthest.end(), -1);
        for (int v : candidates) {
            if (v == centroid) continue;
            int b = branch[v];
            if (nearest[b] == -1 || centroidDist[v] < centroidDist[nearest[b]]) {
                nearest[b] = v;
            }
            if (farthest[b] == -1 || centroidDist[v] > centroidDist[farthest[b]]) {
                farthest[b] = v;
            }
        }

        vector<int> centers(1, centroid);
        auto addCenter = [&](int u) {
            if (u != -1 && find(centers.begin(), centers.end(), u) == centers.end()) {
                centers.push_back(u);
            }
        };
        for (int b : adj[centroid]) {
            addCenter(nearest[b]);
            addCenter(farthest[b]);
        }
        if (m <= 48) {
            for (int v : candidates) addCenter(v);
        }

        int bestCenter = -1, bestK = -1;
        int bestLargest = m + 1, bestDifference = m + 1;
        vector<int> values;
        values.reserve(m);

        for (int center : centers) {
            const vector<int>& currentDist =
                (center == centroid ? centroidDist : dist);
            if (center != centroid) getDistances(center, dist);

            values.clear();
            for (int v : candidates) values.push_back(currentDist[v]);
            sort(values.begin(), values.end());

            for (int i = 0; i < m;) {
                int j = i + 1;
                while (j < m && values[j] == values[i]) ++j;
                int inside = j, outside = m - j;
                if (inside > 0 && outside > 0) {
                    int largest = max(inside, outside);
                    int difference = abs(inside - outside);
                    if (tie(largest, difference, values[i]) <
                        tie(bestLargest, bestDifference, bestK)) {
                        bestLargest = largest;
                        bestDifference = difference;
                        bestK = values[i];
                        bestCenter = center;
                    }
                }
                i = j;
            }
        }

        if (bestCenter == -1) {
            bestCenter = candidates[0];
            bestK = 0;
        }

        cout << "? " << bestCenter + 1 << ' ' << bestK << '\n' << flush;
        int response;
        if (!(cin >> response) || (response != 0 && response != 1)) return 0;

        getDistances(bestCenter, dist);
        vector<int> remaining;
        remaining.reserve(bestLargest);
        for (int v : candidates) {
            bool inside = dist[v] <= bestK;
            if ((response == 1 && inside) || (response == 0 && !inside)) {
                remaining.push_back(v);
            }
        }
        candidates.swap(remaining);
    }

    cout << "! " << candidates[0] + 1 << '\n' << flush;
    return 0;
}

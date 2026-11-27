#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> parent(n + 1, 0), depth(n + 1, 0);
    vector<vector<int>> children(n + 1);

    int maxDepth = 0, deepest = 1;
    for (int v = 2; v <= n; ++v) {
        cin >> parent[v];
        children[parent[v]].push_back(v);
        depth[v] = depth[parent[v]] + 1;
        maxDepth = max(maxDepth, depth[v]);
    }
    for (int v = 1; v <= n; ++v) {
        if (depth[v] > depth[deepest]) deepest = v;
    }

    auto ask = [&](int u, int k) -> int {
        cout << "? " << u << ' ' << k << endl;
        int response;
        if (!(cin >> response) || response < 0 || response > 1) exit(0);
        return response;
    };

    vector<int> subtreeSize(n + 1, 1);
    for (int v = n; v >= 2; --v) {
        subtreeSize[parent[v]] += subtreeSize[v];
    }

    int centroid = 1;
    int bestCentroidValue = n + 1;
    for (int v = 1; v <= n; ++v) {
        int largest = n - subtreeSize[v];
        for (int u : children[v]) {
            largest = max(largest, subtreeSize[u]);
        }
        if (largest < bestCentroidValue) {
            bestCentroidValue = largest;
            centroid = v;
        }
    }

    vector<int> tin(n + 1), first(n + 1, -1), childPosition(n + 1, 0);
    vector<int> euler, stack;
    euler.reserve(2 * n - 1);
    stack.reserve(n);

    int timer = 0;
    tin[1] = timer++;
    first[1] = 0;
    euler.push_back(1);
    stack.push_back(1);

    while (!stack.empty()) {
        int v = stack.back();
        if (childPosition[v] < (int)children[v].size()) {
            int u = children[v][childPosition[v]++];
            tin[u] = timer++;
            first[u] = (int)euler.size();
            euler.push_back(u);
            stack.push_back(u);
        } else {
            stack.pop_back();
            if (!stack.empty()) euler.push_back(stack.back());
        }
    }

    int eulerSize = (int)euler.size();
    vector<int> logarithm(eulerSize + 1, 0);
    for (int i = 2; i <= eulerSize; ++i) {
        logarithm[i] = logarithm[i / 2] + 1;
    }

    int levels = logarithm[eulerSize] + 1;
    vector<vector<int>> sparse(levels);
    sparse[0] = move(euler);

    for (int k = 1; k < levels; ++k) {
        int length = 1 << k;
        int half = length >> 1;
        sparse[k].resize(eulerSize - length + 1);
        for (int i = 0; i + length <= eulerSize; ++i) {
            int a = sparse[k - 1][i];
            int b = sparse[k - 1][i + half];
            sparse[k][i] = (depth[a] < depth[b] ? a : b);
        }
    }

    auto lca = [&](int u, int v) -> int {
        int l = first[u], r = first[v];
        if (l > r) swap(l, r);
        int k = logarithm[r - l + 1];
        int a = sparse[k][l];
        int b = sparse[k][r - (1 << k) + 1];
        return depth[a] < depth[b] ? a : b;
    };

    auto dist = [&](int u, int v) -> int {
        int a = lca(u, v);
        return depth[u] + depth[v] - 2 * depth[a];
    };

    vector<vector<int>> verticesAtDepth(maxDepth + 1);
    for (int v = 1; v <= n; ++v) {
        verticesAtDepth[depth[v]].push_back(v);
    }

    vector<int> candidates;
    candidates.reserve(n);
    for (int d = 0; d <= maxDepth; ++d) {
        for (int v : verticesAtDepth[d]) candidates.push_back(v);
    }

    constexpr int SAMPLE_SIZE = 64;
    vector<int> histogram(n + 1, 0);

    while (candidates.size() > 1) {
        int m = (int)candidates.size();

        vector<int> centers;
        centers.reserve(SAMPLE_SIZE + 3);
        if (m <= SAMPLE_SIZE) {
            centers = candidates;
        } else {
            for (int i = 0; i < SAMPLE_SIZE; ++i) {
                centers.push_back(candidates[(long long)i * m / SAMPLE_SIZE]);
            }
        }
        centers.push_back(1);
        centers.push_back(centroid);
        centers.push_back(deepest);
        sort(centers.begin(), centers.end());
        centers.erase(unique(centers.begin(), centers.end()), centers.end());

        int bestImbalance = m + 1;
        int bestCenter = -1, bestRadius = -1;

        for (int center : centers) {
            fill(histogram.begin(), histogram.end(), 0);
            int maximumDistance = 0;

            for (int v : candidates) {
                int d = dist(center, v);
                ++histogram[d];
                maximumDistance = max(maximumDistance, d);
            }

            int cumulative = 0;
            int localImbalance = m + 1;
            int localRadius = -1;

            for (int k = 0; k <= maximumDistance; ++k) {
                cumulative += histogram[k];
                if (cumulative == 0 || cumulative == m) continue;

                int imbalance = max(cumulative, m - cumulative);
                if (imbalance < localImbalance) {
                    localImbalance = imbalance;
                    localRadius = k;
                }
            }

            if (localImbalance < bestImbalance) {
                bestImbalance = localImbalance;
                bestCenter = center;
                bestRadius = localRadius;
            }

            if (bestImbalance <= (m + 1) / 2) break;
        }

        if (bestCenter == -1 || bestImbalance > (2 * m) / 3) {
            break;
        }

        int response = ask(bestCenter, bestRadius);
        vector<int> next;
        next.reserve(candidates.size());

        for (int v : candidates) {
            bool inside = dist(bestCenter, v) <= bestRadius;
            if (inside == (response == 1)) next.push_back(v);
        }
        candidates.swap(next);
    }

    if (candidates.size() > 1) {
        while (true) {
            int minimumDepth = n, maximumDepth = -1;
            for (int v : candidates) {
                minimumDepth = min(minimumDepth, depth[v]);
                maximumDepth = max(maximumDepth, depth[v]);
            }

            if (minimumDepth == maximumDepth) break;

            int middle = (minimumDepth + maximumDepth) / 2;
            int response = ask(1, middle);

            vector<int> next;
            next.reserve(candidates.size());
            for (int v : candidates) {
                bool keep = (response == 1 ? depth[v] <= middle : depth[v] > middle);
                if (keep) next.push_back(v);
            }
            candidates.swap(next);
        }

        int targetDepth = depth[candidates[0]];
        vector<char> marked(n + 1, false);
        vector<int> count(n + 1, 0);

        while (candidates.size() > 1) {
            fill(marked.begin(), marked.end(), false);
            fill(count.begin(), count.end(), 0);

            for (int v : candidates) {
                marked[v] = true;
                count[v] = 1;
            }
            for (int v = n; v >= 2; --v) {
                count[parent[v]] += count[v];
            }

            int m = (int)candidates.size();
            int chosen = -1;
            int bestImbalance = m + 1;

            for (int v = 1; v <= n; ++v) {
                int inside = count[v];
                if (inside == 0 || inside == m) continue;

                int imbalance = max(inside, m - inside);
                if (imbalance < bestImbalance) {
                    bestImbalance = imbalance;
                    chosen = v;
                }
            }

            int response = ask(chosen, targetDepth - depth[chosen]);
            int subtreeEnd = tin[chosen] + subtreeSize[chosen];

            vector<int> next;
            next.reserve(candidates.size());
            for (int v : candidates) {
                bool inside = tin[chosen] <= tin[v] && tin[v] < subtreeEnd;
                if (inside == (response == 1)) next.push_back(v);
            }
            candidates.swap(next);
        }
    }

    if (candidates.size() == 1) {
        cout << "! " << candidates[0] << endl;
    }

    return 0;
}

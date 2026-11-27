#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    vector<vector<int>> dist(N, vector<int>(N, -1));
    for (int i = 0; i < N; ++i) dist[i][i] = 0;

    auto ask = [&](int u, int v) -> int {
        if (dist[u][v] != -1) return dist[u][v];

        cout << "? " << u << ' ' << v << endl;
        int value;
        if (!(cin >> value) || value < 0) exit(0);

        dist[u][v] = dist[v][u] = value;
        return value;
    };

    // Find one endpoint a of a diameter.
    for (int i = 1; i < N; ++i) ask(0, i);

    int a = 1;
    for (int i = 2; i < N; ++i) {
        if (dist[0][i] > dist[0][a]) a = i;
    }

    // Find the other endpoint b and learn distances from a.
    for (int i = 0; i < N; ++i) {
        if (i != a) ask(a, i);
    }

    int b = 0;
    for (int i = 1; i < N; ++i) {
        if (i != a && dist[a][i] > dist[a][b]) b = i;
    }

    // Learn distances from b.
    for (int i = 0; i < N; ++i) {
        if (i != b) ask(b, i);
    }

    const int D = dist[a][b];

    // pos[x] is the projection of x onto the diameter path a--b.
    // off[x] is the distance from that projection to x.
    vector<int> pos(N), off(N);
    for (int x = 0; x < N; ++x) {
        pos[x] = (dist[a][x] - dist[b][x] + D) / 2;
        off[x] = (dist[a][x] + dist[b][x] - D) / 2;
    }

    // Every city on the diameter is the projection of some town
    // belonging to an off-diameter component.
    set<int> cityPositions;
    for (int x = 0; x < N; ++x) {
        if (0 < pos[x] && pos[x] < D) {
            cityPositions.insert(pos[x]);
        }
    }

    int R = INT_MAX;
    for (int p : cityPositions) {
        R = min(R, max(p, D - p));
    }

    vector<int> hubs;
    for (int p : cityPositions) {
        if (max(p, D - p) == R) hubs.push_back(p);
    }

    const int half = N / 2;
    bool hasBalancedHub = false;

    for (int hub : hubs) {
        int leftCount = 0;
        int rightCount = 0;
        vector<int> group;

        for (int x = 0; x < N; ++x) {
            if (pos[x] < hub) {
                ++leftCount;
            } else if (pos[x] > hub) {
                ++rightCount;
            } else {
                group.push_back(x);
            }
        }

        if (leftCount > half || rightCount > half) continue;

        if ((int)group.size() <= half) {
            hasBalancedHub = true;
            break;
        }

        // Two towns with the same projection are in the same
        // off-diameter component iff their mutual distance is smaller
        // than the sum of their distances to the hub.
        auto sameComponent = [&](int x, int y) -> bool {
            int d = ask(x, y);
            return d < off[x] + off[y];
        };

        // Boyer-Moore cancellation finds the only possible component
        // containing more than N/2 towns.
        int candidate = -1;
        int residual = 0;

        for (int x : group) {
            if (residual == 0) {
                candidate = x;
                residual = 1;
            } else if (sameComponent(candidate, x)) {
                ++residual;
            } else {
                --residual;
            }
        }

        if (residual == 0) {
            hasBalancedHub = true;
            break;
        }

        if (residual > half) continue;

        int componentSize = 1;
        for (int x : group) {
            if (x != candidate && sameComponent(candidate, x)) {
                ++componentSize;
            }
        }

        if (componentSize <= half) {
            hasBalancedHub = true;
            break;
        }
    }

    cout << "! " << (hasBalancedHub ? R : -R) << endl;
    return 0;
}

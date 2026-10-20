#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int N;
        cin >> N;

        if (N == 1) {
            cout << 0 << '\n';
            continue;
        }

        vector<vector<int>> adj(N + 1);
        for (int i = 0; i < N - 1; ++i) {
            int u, v;
            cin >> u >> v;
            adj[u].push_back(v);
            adj[v].push_back(u);
        }

        vector<int> dist(N + 1), parent(N + 1), q(N + 1);

        auto bfs_farthest = [&](int start, int &far) -> int {
            fill(dist.begin(), dist.end(), -1);
            fill(parent.begin(), parent.end(), 0);

            int head = 0, tail = 0;
            q[tail++] = start;
            dist[start] = 0;

            far = start;
            int maxd = 0;

            while (head < tail) {
                int v = q[head++];
                if (dist[v] > maxd) {
                    maxd = dist[v];
                    far = v;
                }

                for (int u : adj[v]) {
                    if (dist[u] == -1) {
                        dist[u] = dist[v] + 1;
                        parent[u] = v;
                        q[tail++] = u;
                    }
                }
            }
            return maxd;
        };

        int A;
        bfs_farthest(1, A);

        int B;
        int diameterEdges = bfs_farthest(A, B);

        auto count_at_depth = [&](int start, int forbid, int targetDepth) -> int {
            fill(dist.begin(), dist.end(), -1);

            int head = 0, tail = 0;
            q[tail++] = start;
            dist[start] = 0;

            int cnt = 0;

            while (head < tail) {
                int v = q[head++];

                if (dist[v] == targetDepth) ++cnt;
                if (dist[v] >= targetDepth) continue;

                for (int u : adj[v]) {
                    if (u != forbid && dist[u] == -1) {
                        dist[u] = dist[v] + 1;
                        q[tail++] = u;
                    }
                }
            }

            return cnt;
        };

        long long K = 0;

        if (diameterEdges % 2 == 0) {
            // The center is one vertex.
            int R = diameterEdges / 2;
            int C = B;
            for (int i = 0; i < R; ++i) C = parent[C];

            K = count_at_depth(C, 0, R);
        } else {
            // The center is an edge.
            int h = diameterEdges / 2;

            int C2 = B;
            for (int i = 0; i < h; ++i) C2 = parent[C2];
            int C1 = parent[C2];

            K = count_at_depth(C1, C2, h) + count_at_depth(C2, C1, h);
        }

        int D = diameterEdges + 1; // number of vertices on a diameter

        int lg = 0;
        long long pw = 1;
        while (pw < D) {
            pw <<= 1;
            ++lg;
        }

        long long ans = K + lg - 2;
        cout << ans << '\n';
    }

    return 0;
}

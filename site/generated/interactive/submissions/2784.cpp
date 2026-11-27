#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<vector<int>> adj(n + 1);
    for (int i = 0; i < n - 1; ++i) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    auto queryAll = [&]() -> long long {
        cout << "? 1 " << n;
        for (int u = 1; u <= n; ++u) cout << ' ' << u;
        cout << '\n' << flush;

        long long response;
        if (!(cin >> response)) exit(0);
        return response;
    };

    if (n == 2) {
        auto queryOne = [&](int u) -> long long {
            cout << "? 1 1 " << u << '\n' << flush;
            long long response;
            if (!(cin >> response)) exit(0);
            return response;
        };

        long long f1 = queryOne(1);
        long long f2 = queryOne(2);

        int a1, a2;
        if (abs(f1) == 1) {
            a1 = static_cast<int>(f1);
            a2 = static_cast<int>(f2 - f1);
        } else {
            a2 = static_cast<int>(f2);
            a1 = static_cast<int>(f1 - f2);
        }

        cout << "! " << a1 << ' ' << a2 << '\n' << flush;
        return 0;
    }

    int x = -1;
    for (int u = 1; u <= n; ++u) {
        if (adj[u].size() == 1) {
            x = u;
            break;
        }
    }
    int y = adj[x][0];

    vector<int> processed;
    processed.reserve(n - 2);
    for (int u = 1; u <= n; ++u) {
        if (u != x && u != y) processed.push_back(u);
    }

    const long long initialSum = queryAll();
    long long currentSum = initialSum;

    vector<int> initialValue(n + 1, 0);
    vector<int> observedSize(n + 1, 0);

    for (int u : processed) {
        cout << "? 2 " << u << '\n' << flush;
        long long nextSum = queryAll();

        long long delta = currentSum - nextSum;
        currentSum = nextSum;

        if (delta == 0) exit(0);
        initialValue[u] = (delta > 0 ? 1 : -1);
        observedSize[u] = static_cast<int>(llabs(delta) / 2);
    }

    vector<int> parent(n + 1), order;
    vector<int> subtreeSize(n + 1);

    auto calculateSubtrees = [&](int root) {
        fill(subtreeSize.begin(), subtreeSize.end(), 1);
        order.clear();
        order.push_back(root);
        parent[root] = 0;

        for (int i = 0; i < static_cast<int>(order.size()); ++i) {
            int u = order[i];
            for (int v : adj[u]) {
                if (v == parent[u]) continue;
                parent[v] = u;
                order.push_back(v);
            }
        }

        for (int i = static_cast<int>(order.size()) - 1; i > 0; --i) {
            int u = order[i];
            subtreeSize[parent[u]] += subtreeSize[u];
        }
    };

    struct Solution {
        int root;
        int valueX;
        int valueY;
    };
    vector<Solution> solutions;

    for (int root = 1; root <= n; ++root) {
        calculateSubtrees(root);

        bool consistent = true;
        for (int u : processed) {
            if (subtreeSize[u] != observedSize[u]) {
                consistent = false;
                break;
            }
        }
        if (!consistent) continue;

        long long needed = initialSum;
        for (int u : processed) {
            needed -= 1LL * initialValue[u] * subtreeSize[u];
        }

        for (int ax = -1; ax <= 1; ax += 2) {
            for (int ay = -1; ay <= 1; ay += 2) {
                long long contribution =
                    1LL * ax * subtreeSize[x] +
                    1LL * ay * subtreeSize[y];

                if (needed == contribution) {
                    solutions.push_back({root, ax, ay});
                }
            }
        }
    }

    if (solutions.size() != 1) exit(0);

    vector<int> answer(n + 1);
    for (int u : processed) {
        answer[u] = -initialValue[u];
    }
    answer[x] = solutions[0].valueX;
    answer[y] = solutions[0].valueY;

    cout << "!";
    for (int u = 1; u <= n; ++u) cout << ' ' << answer[u];
    cout << '\n' << flush;

    return 0;
}

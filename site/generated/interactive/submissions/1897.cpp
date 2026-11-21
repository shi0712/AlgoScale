#include <bits/stdc++.h>
using namespace std;

#if defined(__GNUC__) && defined(__x86_64__)
#pragma GCC target("popcnt")
#endif

constexpr int MAXN = 1024;
using Mask = bitset<MAXN>;

struct Cup {
    Mask possible;
    int single;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;

    Mask live;
    for (int i = 0; i < n; ++i) live.set(i);

    // An edge means we have not established that these representatives differ.
    vector<Mask> edge(n, live);
    vector<int> degree(n, n - 1);
    for (int i = 0; i < n; ++i) edge[i].reset(i);

    auto eraseEdge = [&](int x, int y) {
        if (!edge[x][y]) return;
        edge[x].reset(y);
        edge[y].reset(x);
        --degree[x];
        --degree[y];
    };

    deque<Cup> window;
    vector<int> score(n);
    while (true) {
        Mask useful;
        for (int i = 0; i < n; ++i)
            if (degree[i]) useful.set(i);

        if (useful.none()) {
            cout << "! " << live.count() << endl;
            return 0;
        }

        Mask blocked, certain;
        for (const Cup &p : window) {
            blocked |= p.possible;
            if (p.single >= 0) certain.set(p.single);
        }

        // Never query a representative which could itself be in memory.
        // Thus every Y safely eliminates the queried representative.
        Mask eligible = useful & ~blocked;
        int best = 0;
        for (int i = 0; i < n; ++i) {
            score[i] = eligible[i] ? int((certain & edge[i]).count()) : -1;
            best = max(best, score[i]);
        }

        int x = -1;
        if (best == 0) {
            // Start another run without a reset whenever possible. Pick a
            // seed with an available neighbor, ensuring progress next turn.
            pair<int,int> rankBest{-1, -1};
            for (int i = 0; i < n; ++i) {
                if (!eligible[i] || (eligible & edge[i]).none()) continue;
                pair<int,int> rank{k == 1 ? degree[i] % 2 : 0, degree[i]};
                if (rank > rankBest) rankBest = rank, x = i;
            }
            if (x < 0) {
                // With only singleton entries, the window is a known clique
                // of mutually different varieties. Thus this reset requires
                // an ambiguous cup, which came from an elimination.
                cout << "R" << endl;
                window.clear();
                continue;
            }
        } else {
            Mask bridges;
            if (k == 1 && best == 1 && degree[window.front().single] > 1) {
                // Prefer not to split the remaining comparison graph when
                // extending a trail of pairwise comparisons.
                int root = window.front().single, timer = 0;
                vector<int> tin(n), low(n);
                auto dfs = [&](auto &&self, int u, int par) -> void {
                    tin[u] = low[u] = ++timer;
                    for (int v = 0; v < n; ++v) {
                        if (!edge[u][v] || v == par) continue;
                        if (tin[v]) low[u] = min(low[u], tin[v]);
                        else {
                            self(self, v, u);
                            low[u] = min(low[u], low[v]);
                            if (u == root && low[v] > tin[u]) bridges.set(v);
                        }
                    }
                };
                dfs(dfs, root, -1);
            }

            // Among equally productive probes, prefer one which will be a
            // useful reference for other high-scoring probes in the window.
            Mask upcoming;
            for (int i = 0; i < n; ++i)
                if (score[i] >= best - 1) upcoming.set(i);

            tuple<int,int,int,int> rankBest{-1, -1, -1, -1};
            for (int i = 0; i < n; ++i) if (score[i] == best) {
                int future = int((upcoming & edge[i]).count());
                auto rank = make_tuple(int(!bridges[i]), min(future, k),
                                       degree[i], future);
                if (rank > rankBest) rankBest = rank, x = i;
            }
        }

        cout << "? " << x + 1 << endl;
        string reply;
        if (!(cin >> reply) || (reply != "Y" && reply != "N")) return 0;

        Cup added{};
        if (reply == "N") {
            for (const Cup &p : window)
                if (p.single >= 0) eraseEdge(x, p.single);
            added.possible.set(x);
            added.single = x;
        } else {
            added.possible = blocked & edge[x];
            added.single = -1;
            if (added.possible.count() == 1) {
                int u = 0;
                while (!added.possible[u]) ++u;
                added.single = u;
                // Exact alias: inherit all inequalities already proved for x.
                for (int j = 0; j < n; ++j)
                    if (!edge[x][j]) eraseEdge(u, j);
            }
            // All prior window descriptions remain valid: none contained x.
            live.reset(x);
            for (int j = 0; j < n; ++j) eraseEdge(x, j);
        }
        if (int(window.size()) == k) window.pop_front();
        window.push_back(added);
    }
}

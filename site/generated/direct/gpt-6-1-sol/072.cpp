#include <bits/stdc++.h>
using namespace std;

/*
    Randomized, verified decision-tree construction.

    All queries are planned before interacting. A construction is accepted only
    if every leaf is a singleton and its height is at most 21. Consequently,
    randomness can affect preprocessing time, but cannot cause an incorrect
    answer or a violation of the query limit.
*/

class Solver {
    static constexpr int LIMIT = 21;

    struct Decision {
        int u = 0, k = 0;
        int yes = -1, no = -1;
        int answer = 0;
    };

    int n;
    vector<array<int, 2>> child;
    vector<int> childCount;
    vector<int> depth, first, euler, lg;
    vector<vector<int>> rmq;
    vector<Decision> decisions;

    mt19937 rng;

    void makeEulerTour() {
        depth.assign(n + 1, 0);
        first.assign(n + 1, -1);
        euler.clear();
        euler.reserve(2 * n - 1);

        struct Frame {
            int u, nextChild;
        };

        vector<Frame> stack;
        stack.reserve(n);
        stack.push_back({1, 0});
        first[1] = 0;
        euler.push_back(1);

        while (!stack.empty()) {
            Frame &f = stack.back();
            int u = f.u;

            if (f.nextChild < childCount[u]) {
                int v = child[u][f.nextChild++];
                depth[v] = depth[u] + 1;
                first[v] = (int)euler.size();
                euler.push_back(v);
                stack.push_back({v, 0});
            } else {
                stack.pop_back();
                if (!stack.empty()) {
                    euler.push_back(stack.back().u);
                }
            }
        }

        int m = (int)euler.size();
        lg.assign(m + 1, 0);
        for (int i = 2; i <= m; ++i) {
            lg[i] = lg[i / 2] + 1;
        }

        rmq.assign(lg[m] + 1, vector<int>(m));
        rmq[0] = euler;

        for (int j = 1; (1 << j) <= m; ++j) {
            int half = 1 << (j - 1);
            int len = 1 << j;
            for (int i = 0; i + len <= m; ++i) {
                int a = rmq[j - 1][i];
                int b = rmq[j - 1][i + half];
                rmq[j][i] = depth[a] < depth[b] ? a : b;
            }
        }
    }

    int lca(int a, int b) const {
        int l = first[a], r = first[b];
        if (l > r) swap(l, r);
        int j = lg[r - l + 1];
        int x = rmq[j][l];
        int y = rmq[j][r - (1 << j) + 1];
        return depth[x] < depth[y] ? x : y;
    }

    int distance(int a, int b) const {
        return depth[a] + depth[b] - 2 * depth[lca(a, b)];
    }

    int randomIndex(int bound) {
        return int(rng() % unsigned(bound));
    }

    int build(vector<int> candidates, int remaining, int samples) {
        int m = (int)candidates.size();

        if (m == 1) {
            int id = (int)decisions.size();
            decisions.push_back({});
            decisions[id].answer = candidates[0];
            return id;
        }

        if (remaining == 0 || m > (1 << remaining)) {
            return -1;
        }

        if (m == 2) {
            int id = (int)decisions.size();
            decisions.push_back({});
            decisions[id].u = candidates[0];
            decisions[id].k = 0;

            int a = (int)decisions.size();
            decisions.push_back({});
            decisions[a].answer = candidates[0];

            int b = (int)decisions.size();
            decisions.push_back({});
            decisions[b].answer = candidates[1];

            decisions[id].yes = a;
            decisions[id].no = b;
            return id;
        }

        int bestU = candidates[0];
        int bestK = 0;
        int bestWorst = m;
        vector<int> distances(m);

        auto consider = [&](int u) {
            for (int i = 0; i < m; ++i) {
                distances[i] = distance(u, candidates[i]);
            }

            int mid = m / 2;
            nth_element(distances.begin(),
                        distances.begin() + mid,
                        distances.end());
            int radius = distances[mid];

            int less = 0, equal = 0;
            for (int d : distances) {
                less += d < radius;
                equal += d == radius;
            }

            auto update = [&](int k, int inside) {
                if (inside == 0 || inside == m || k < 0) return;
                int worst = max(inside, m - inside);
                if (worst < bestWorst) {
                    bestWorst = worst;
                    bestU = u;
                    bestK = k;
                }
            };

            update(radius - 1, less);
            update(radius, less + equal);
        };

        int attempts = min(m, samples);

        if (m <= samples) {
            // Trying every candidate is particularly useful near the leaves.
            int start = randomIndex(m);
            for (int i = 0; i < m; ++i) {
                consider(candidates[(start + i) % m]);
                if (bestWorst == (m + 1) / 2) break;
            }
        } else {
            for (int i = 0; i < attempts; ++i) {
                consider(candidates[randomIndex(m)]);
                if (bestWorst == (m + 1) / 2) break;
            }
        }

        // Centers outside the candidate set can give better partitions.
        if (bestWorst != (m + 1) / 2) {
            consider(1);
            for (int i = 0; i < 4; ++i) {
                consider(1 + randomIndex(n));
                if (bestWorst == (m + 1) / 2) break;
            }
        }

        if (bestWorst > (1 << (remaining - 1))) {
            return -1;
        }

        vector<int> inside, outside;
        inside.reserve(m);
        outside.reserve(m);

        for (int x : candidates) {
            if (distance(bestU, x) <= bestK) {
                inside.push_back(x);
            } else {
                outside.push_back(x);
            }
        }

        int id = (int)decisions.size();
        decisions.push_back({});
        decisions[id].u = bestU;
        decisions[id].k = bestK;

        int yes = build(move(inside), remaining - 1, samples);
        if (yes == -1) return -1;

        int no = build(move(outside), remaining - 1, samples);
        if (no == -1) return -1;

        decisions[id].yes = yes;
        decisions[id].no = no;
        return id;
    }

public:
    Solver()
        : rng(uint32_t(chrono::steady_clock::now()
                           .time_since_epoch().count())) {}

    bool run() {
        if (!(cin >> n)) return false;

        child.assign(n + 1, {0, 0});
        childCount.assign(n + 1, 0);

        for (int v = 2; v <= n; ++v) {
            int p;
            if (!(cin >> p)) return false;
            child[p][childCount[p]++] = v;
        }

        makeEulerTour();

        vector<int> all(n);
        iota(all.begin(), all.end(), 1);

        decisions.reserve(2 * n);

        int root = -1;
        int samples = 12;

        // No interaction takes place until the entire strategy is verified.
        while (root == -1) {
            decisions.clear();
            root = build(all, LIMIT, samples);
            samples = min(64, samples + 4);
        }

        int current = root;
        while (decisions[current].answer == 0) {
            const Decision &q = decisions[current];
            cout << "? " << q.u << ' ' << q.k << endl;

            int response;
            if (!(cin >> response)) return false;
            if (response != 0 && response != 1) return false;

            current = response ? q.yes : q.no;
        }

        cout << "! " << decisions[current].answer << endl;
        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int tests;
    if (!(cin >> tests)) return 0;

    Solver solver;
    while (tests--) {
        if (!solver.run()) return 0;
    }

    return 0;
}

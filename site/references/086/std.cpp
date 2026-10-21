#include <bits/stdc++.h>
using namespace std;

class Solver {
    int n, height = 0;
    vector<int> fa, dep, tin, tout, levelSize, cnt, capacity;
    vector<array<int, 2>> son;
    vector<long long> start;
    int planned = 0;

    bool inSubtree(int u, int x) const {
        return tin[u] <= tin[x] && tin[x] < tout[u];
    }

    int rankOf(int m) const {
        return int(lower_bound(capacity.begin(), capacity.end(), m)
                   - capacity.begin());
    }

public:
    Solver(int n_, const vector<int>& parent) : n(n_), fa(parent) {
        dep.assign(n + 1, 0);
        son.assign(n + 1, array<int, 2>{0, 0});
        tin.resize(n + 1);
        tout.resize(n + 1);
        cnt.resize(n + 1);
        for (int v = 2; v <= n; ++v) {
            int p = fa[v];
            if (!son[p][0]) son[p][0] = v;
            else son[p][1] = v;
            dep[v] = dep[p] + 1;
            height = max(height, dep[v]);
        }
        levelSize.assign(height + 1, 0);
        for (int v = 1; v <= n; ++v) ++levelSize[dep[v]];

        // Non-recursive DFS order; fa[v] < v also gives a bottom-up order.
        vector<int> sz(n + 1, 1), stk;
        for (int v = n; v >= 2; --v) sz[fa[v]] += sz[v];
        stk.push_back(1);
        int timer = 0;
        while (!stk.empty()) {
            int v = stk.back();
            stk.pop_back();
            tin[v] = timer++;
            tout[v] = tin[v] + sz[v];
            if (son[v][1]) stk.push_back(son[v][1]);
            if (son[v][0]) stk.push_back(son[v][0]);
        }

        // capacity[q]: q queries suffice for this many same-depth candidates.
        capacity = {1, 2};
        while (capacity.back() < n) {
            int s = int(capacity.size());
            capacity.push_back(capacity[s - 1] + capacity[s - 2] + 1);
        }

        // An aligned dyadic block reserves the within-level query budget.
        start.resize(height + 1);
        long long p = 0;
        for (int d = 0; d <= height; ++d) {
            long long w = 1LL << rankOf(levelSize[d]);
            p = (p + w - 1) / w * w;
            start[d] = p;
            p += w;
        }
        while ((1LL << planned) < p) ++planned;
    }

    int plannedQueries() const { return planned; }

    template<class Ask>
    int solve(Ask&& ask) {
        int l = 0, r = height;
        long long left = 0, right = 1LL << planned;

        // Locate the depth using the alphabetic prefix code of the blocks.
        while (l < r) {
            long long mid = (left + right) / 2;
            int cut = int(lower_bound(start.begin() + l,
                                      start.begin() + r + 1, mid)
                          - start.begin());
            if (cut == l) {
                left = mid;                // Empty left half: no query.
            } else if (cut == r + 1) {
                right = mid;               // Empty right half: no query.
            } else if (ask(1, cut - 1)) {
                r = cut - 1;
                right = mid;
            } else {
                l = cut;
                left = mid;
            }
        }
        const int D = l;
        vector<int> candidate;
        for (int v = 1; v <= n; ++v)
            if (dep[v] == D) candidate.push_back(v);

        // On level D, ? u (D-dep[u]) is exactly a subtree-membership query.
        auto querySubtree = [&](int u) -> int {
            int reply = ask(u, D - dep[u]);
            int len = 0;
            for (int x : candidate)
                if (int(inSubtree(u, x)) == reply)
                    candidate[len++] = x;
            candidate.resize(len);
            return reply;
        };

        while (candidate.size() > 1) {
            fill(cnt.begin(), cnt.end(), 0);
            for (int x : candidate) cnt[x] = 1;
            for (int v = n; v >= 2; --v) cnt[fa[v]] += cnt[v];

            int m = int(candidate.size());
            int q = rankOf(m);
            int A = capacity[q - 1];
            int low = m - A;
            int chosen = 0, best = m + 1;
            for (int v = 1; v <= n; ++v) {
                if (low <= cnt[v] && cnt[v] <= A) {
                    int imbalance = abs(m - 2 * cnt[v]);
                    if (imbalance < best) {
                        best = imbalance;
                        chosen = v;
                    }
                }
            }
            if (chosen) {
                querySubtree(chosen);
                continue;
            }

            // No balanced edge. Find the three-way junction.
            // Each of its two child groups and its outside group has
            // at most capacity[q-2] candidates.
            int u = 1;
            while (true) {
                int next = 0;
                for (int v : son[u])
                    if (v && cnt[v] > A) next = v;
                if (!next) break;
                u = next;
            }
            int a = son[u][0], b = son[u][1];
            assert(q >= 2 && a && b && cnt[a] && cnt[b]);
            if (querySubtree(a)) continue;
            querySubtree(b);
        }
        return candidate[0];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    if (!(cin >> T)) return 0;
    while (T--) {
        int n;
        cin >> n;
        vector<int> fa(n + 1, 0);
        for (int v = 2; v <= n; ++v) cin >> fa[v];
        Solver solver(n, fa);
        auto ask = [&](int u, int k) -> int {
            cout << "? " << u << ' ' << k << endl;
            int reply;
            if (!(cin >> reply) || (reply != 0 && reply != 1)) exit(0);
            return reply;
        };
        int answer = solver.solve(ask);
        cout << "! " << answer << endl;
    }
    return 0;
}

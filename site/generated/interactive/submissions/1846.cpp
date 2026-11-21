#include <bits/stdc++.h>
using namespace std;

class Solver {
    struct Block {
        int a, b;
    };

    int n, budget, queries = 0;
    int pairs = 0, lastPair = -1, nextTrial = 0;
    int target = 0, streak = 0, lastLeaf = -1, probes = 0;
    bool probed = false;

    set<int> live, singles;
    vector<Block> blocks;
    vector<int> ban, removedAt, banCount;
    vector<char> bad;

    pair<int, int> ask(int d) {
        cout << "? " << d << endl;
        ++queries;
        int v, w;
        if (!(cin >> v) || v < 0) exit(0);
        if (!(cin >> w) || w < 0) exit(0);
        if (v == 0) {
            if (w != 0) exit(0);
            return {0, 0};
        }
        if (v > n || w > n || !live.count(v) ||
            (w != 0 && (w == v || !live.count(w)))) {
            exit(0);
        }
        live.erase(v);
        removedAt[v] = queries;
        ban[v] = w;
        return {v, w};
    }

    int addSingle(int v) {
        int i = int(blocks.size());
        blocks.push_back({v, 0});
        singles.insert(i);
        ++banCount[ban[v]];
        return i;
    }

    void answer(const vector<int>& path) {
        cout << '!';
        for (int v : path) cout << ' ' << v;
        cout << endl;
        exit(0);
    }

    // The initial forest consists of the supplied path and all other
    // remaining vertices as singletons. Process the blocks backwards,
    // joining components only by edges certified by the replies.
    bool build(const vector<int>& spine) {
        vector<int> adj1(n + 1), adj2(n + 1), opposite(n + 1);
        set<int> ends;

        auto link = [&](int u, int v) {
            (adj1[u] ? adj2[u] : adj1[u]) = v;
            (adj1[v] ? adj2[v] : adj1[v]) = u;
        };
        auto put = [&](int u, int v) {
            opposite[u] = v;
            opposite[v] = u;
            ends.insert(u);
            ends.insert(v);
        };
        auto take = [&](int u) {
            int v = opposite[u];
            ends.erase(u);
            if (v != u) ends.erase(v);
            opposite[u] = opposite[v] = 0;
            return v;
        };
        auto join = [&](int u, int v, int a, int b) {
            int left = u ? take(u) : a;
            int right = v ? take(v) : (b ? b : a);
            if (u) link(u, a);
            if (b) link(a, b);
            if (v) link(b ? b : a, v);
            put(left, right);
        };

        for (int v : live) put(v, v);
        if (!spine.empty()) {
            for (int v : spine) {
                ends.erase(v);
                opposite[v] = 0;
            }
            for (int i = 1; i < int(spine.size()); ++i)
                link(spine[i - 1], spine[i]);
            put(spine.front(), spine.back());
        }

        // Protect the vertex that could block the last single-vertex
        // insertion when it is still a singleton.
        int x = singles.empty() ? 0 : ban[blocks[*singles.begin()].a];
        if (x && removedAt[x]) x = 0;

        auto otherEnd = [&](int u, int forbidden) {
            for (auto it = ends.rbegin(); it != ends.rend(); ++it) {
                int v = *it;
                if (v != forbidden && v != u && (!u || v != opposite[u]))
                    return v;
            }
            return 0;
        };

        auto knownFromBad = [&](int b, int v) {
            if (!removedAt[v] || removedAt[v] > removedAt[b])
                return v < ban[b];
            if (bad[v]) return b < ban[v];
            return ban[v] != b;
        };

        for (int i = int(blocks.size()) - 1; i >= 0; --i) {
            auto [a, b] = blocks[i];
            if (!a) continue;
            bool protect = x && opposite[x] == x;
            if (!b) {
                int u = protect && x != ban[a] ? x : otherEnd(0, ban[a]);
                int v = u ? otherEnd(u, ban[a]) : 0;
                join(u, v, a, 0);
            } else {
                // The anchor a can use any endpoint. Sometimes the
                // smallest reported non-neighbor also certifies an edge
                // from b, allowing this block to join two components.
                int u = 0, v = 0;
                if (protect && knownFromBad(b, x)) {
                    u = otherEnd(x, 0);
                    if (u) v = x;
                }
                if (!v && !ends.empty() &&
                    opposite[*ends.begin()] != *ends.rbegin()) {
                    int inspected = 0;
                    for (int t : ends) {
                        if (++inspected > 8) break;
                        if ((protect && t == x) || !knownFromBad(b, t))
                            continue;
                        int s = protect ? x : otherEnd(t, 0);
                        if (s) {
                            u = s;
                            v = t;
                            break;
                        }
                    }
                }
                if (!v) u = protect ? x : otherEnd(0, 0);
                join(u, v, a, b);
            }
        }

        if (ends.empty() || opposite[*ends.begin()] != *ends.rbegin())
            return false;

        vector<int> path;
        path.reserve(n);
        int prev = 0, cur = *ends.begin();
        while (cur) {
            path.push_back(cur);
            int next = adj1[cur] == prev ? adj2[cur] : adj1[cur];
            prev = cur;
            cur = next;
        }
        if (int(path.size()) == n) answer(path);
        return false;
    }

    void finishClique(int extra = 0) {
        vector<int> path(live.begin(), live.end());
        if (extra) path.push_back(extra);
        build(path);
        exit(0);
    }

    void checkFinish() {
        if (budget == 0) finishClique();

        int m = int(live.size()), g = int(singles.size());
        if (g >= m - 1 && g > 0) {
            int first = *singles.begin();
            int x = ban[blocks[first].a];
            if (!x || removedAt[x] || banCount[x] < g || lastPair > first)
                build({});
        }

        if (g + pairs >= m - 1 && queries >= nextTrial) {
            nextTrial = queries + max({1, m / 12, n / 40});
            build({});
        }
    }

    void useSmallBudget(int v, int w) {
        if (budget <= 0 || budget > 2) return;
        vector<int> low, high;
        for (int u : live) {
            if (u < v && u != w) low.push_back(u);
            else high.push_back(u);
        }
        // These low labels could not have complement degree one when
        // v was chosen, and deleting v did not change their degrees.
        if (budget == 2) {
            if (low.size() >= 2) build(low);
        } else if (!low.empty()) {
            // With at most one missing edge, every low label is universal.
            vector<int> path;
            if (!high.empty()) {
                path.push_back(high.back());
                high.pop_back();
            }
            for (int u : low) {
                path.push_back(u);
                if (!high.empty()) {
                    path.push_back(high.back());
                    high.pop_back();
                }
            }
            build(path);
        }
    }

    int takeMaximum(int anchor, bool relocate, bool noLeaves) {
        int r = int(live.size());
        auto [b, w] = ask(0);
        if (!b) exit(0);
        if (!w) finishClique(b);

        // For E missing edges, the maximum complement degree is at
        // least ceil(2E/r). E - ceil(2E/r) is nondecreasing in E.
        int newBudget = budget - (2 * budget + r - 1) / r;
        if (noLeaves) newBudget = min(newBudget, budget - 2);
        budget = newBudget;

        int a = blocks[anchor].a;
        singles.erase(anchor);
        --banCount[ban[a]];
        bad[b] = true;
        ++pairs;
        if (relocate) {
            blocks[anchor].a = 0;
            lastPair = int(blocks.size());
            blocks.push_back({a, b});
        } else {
            lastPair = anchor;
            blocks[anchor].b = b;
        }
        return b;
    }

    void useUniversal(int anchor, bool noLeaves) {
        checkFinish();
        if (noLeaves && budget < 3) finishClique();

        int b = takeMaximum(anchor, false, noLeaves);
        checkFinish();

        // If the recurring missing neighbor is the vertex just removed,
        // its last leaf is now a safe anchor for a maximum-degree query.
        if (streak >= 2 && b == target && lastLeaf >= 0 &&
            singles.count(lastLeaf)) {
            takeMaximum(lastLeaf, true, false);
            checkFinish();
        }
        streak = 0;
        target = 0;
        lastLeaf = -1;
        probed = false;
    }

public:
    explicit Solver(int size)
        : n(size), budget(n - 2), ban(n + 1), removedAt(n + 1),
          banCount(n + 1), bad(n + 1) {
        for (int v = 1; v <= n; ++v) live.insert(live.end(), v);
        blocks.reserve(n);
    }

    void run() {
        for (;;) {
            checkFinish();
            int m = int(live.size());

            // A repeated missing neighbor suggests a concentrated
            // complement. Look for a safe anchor before removing a hub.
            if (m >= 10 && budget >= 3 && streak >= 2 && !probed &&
                probes < 3 &&
                (int(singles.size()) + 2 < m ||
                 banCount[target] == int(singles.size()))) {
                probed = true;
                ++probes;
                auto [u, w] = ask(m - 1);
                if (u) {
                    if (w) exit(0);
                    useUniversal(addSingle(u), false);
                    continue;
                }
            }

            // At this point the complement has at most m-2 edges.
            // Thus a vertex of complement degree at most one exists.
            auto [v, w] = ask(m - 2);
            if (!v) exit(0);
            int index = addSingle(v);
            if (!w) {
                useUniversal(index, true);
                continue;
            }

            --budget;
            if (target == w) {
                ++streak;
            } else {
                target = w;
                streak = 1;
                probed = false;
            }
            lastLeaf = index;
            useSmallBudget(v, w);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2 || n > 100000) return 0;
    Solver(n).run();
    return 0;
}

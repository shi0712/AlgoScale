#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;

class Solver {
    struct Barrier {
        int l, r, cells;
    };
    struct Group {
        int cells, visible;
    };

    int n, total, allCells, frozen = 0;
    vector<Barrier> barriers;
    int id[10][10]{};
    U64 cover[10]{};

    U64 parity = 0, known = 0, value = 0;
    unordered_set<U64> forbidden[10];
    int excluded[10]{}, excludedOne[10][55]{};
    bool dirty = false;

    int target = -1;
    U64 dimensions = 0, anchor = 0, goal = 0;
    vector<int> order;
    unsigned grayIndex = 0;
    bool hasGoal = false;

    static U64 bit(int b) { return U64(1) << b; }

    int flip(int b) {
        const auto &e = barriers[b];
        cout << e.l + 1 << ' ' << e.r + 1 << endl;
        int next;
        if (!(cin >> next) || next < 0 || next >= n) exit(0);
        int change = next - total;
        total = next;
        parity ^= bit(b);
        return change;
    }

    void learn(U64 mask, U64 assignment) {
        mask &= ~known;
        if (!mask) return;
        known |= mask;
        value |= assignment & mask;
        dirty = true;
    }

    void addCompatible(int i, U64 key) {
        ++excluded[i];
        for (U64 s = key & ~known; s; s &= s - 1)
            ++excludedOne[i][countr_zero(s)];
    }

    void rebuild() {
        if (!dirty) return;
        memset(excluded, 0, sizeof excluded);
        memset(excludedOne, 0, sizeof excludedOne);
        for (int i = 0; i < n; ++i) {
            U64 fixed = known & cover[i];
            for (U64 key : forbidden[i])
                if (((key ^ value) & fixed) == 0)
                    addCompatible(i, key);
        }
        dirty = false;
    }

    void exclude(int cells, U64 assignment) {
        for (int s = cells; s; s &= s - 1) {
            int i = countr_zero(unsigned(s));
            U64 key = assignment & cover[i];
            if ((key ^ value) & known & cover[i]) continue;
            if (forbidden[i].insert(key).second && !dirty)
                addCompatible(i, key);
        }
    }

    int remaining(int i) const {
        return (1 << popcount(cover[i] & ~known)) - excluded[i];
    }

    bool possible(int i, U64 assignment) const {
        return !((assignment ^ value) & known & cover[i]) &&
               !forbidden[i].contains(assignment & cover[i]);
    }

    bool deduce() {
        U64 mask = 0, ones = 0;
        for (int i = 0; i < n; ++i) {
            U64 unknown = cover[i] & ~known;
            if (!unknown) continue;
            int half = 1 << (popcount(unknown) - 1);
            if (excluded[i] < half) continue;
            for (U64 s = unknown; s; s &= s - 1) {
                int b = countr_zero(s);
                int e = excludedOne[i][b];
                if (e == half) mask |= bit(b);
                if (excluded[i] - e == half) {
                    mask |= bit(b);
                    ones |= bit(b);
                }
            }
        }
        learn(mask, ones);
        return mask != 0;
    }

    void normalize(vector<Group> &groups) {
        for (;;) {
            rebuild();
            vector<Group> next;
            U64 inactive = 0;
            int maybe = 0;
            for (Group g : groups) {
                int candidates = 0, forced = 0;
                for (int s = g.cells; s; s &= s - 1) {
                    int i = countr_zero(unsigned(s));
                    if (!possible(i, parity)) continue;
                    candidates |= 1 << i;
                    if (remaining(i) == 1) forced |= 1 << i;
                }
                g.cells = candidates ^ forced;
                g.visible -= popcount(unsigned(forced));
                if (g.visible == popcount(unsigned(g.cells))) {
                    forced |= g.cells;
                    g.cells = 0;
                    g.visible = 0;
                }
                frozen |= forced;
                for (int s = forced; s; s &= s - 1)
                    inactive |= cover[countr_zero(unsigned(s))];

                if (g.visible == 0) continue;
                next.push_back(g);
                maybe |= g.cells;
                for (int b = 0; b < int(barriers.size()); ++b) {
                    if ((known | inactive) & bit(b)) continue;
                    if (popcount(unsigned(g.cells & ~barriers[b].cells)) < g.visible)
                        inactive |= bit(b);
                }
            }
            groups.swap(next);
            learn(inactive, parity);
            exclude(allCells & ~(frozen | maybe), parity);
            if (dirty) continue;
            if (!deduce()) break;
        }
    }

    int chooseProbe(const Group &g) const {
        vector<int> cells;
        for (int s = g.cells; s; s &= s - 1)
            cells.push_back(countr_zero(unsigned(s)));
        int m = int(cells.size());
        long double weight[10];
        for (int i = 0; i < m; ++i)
            weight[i] = 1.0L / remaining(cells[i]);

        // Weighted ordered splits, accounting for the cost of a restoration.
        long double z[10][10][11]{}, dp[10][10][11]{};
        int action[10][10][11]{};
        for (int l = 0; l < m; ++l) {
            long double w[11]{1};
            for (int r = l; r < m; ++r) {
                for (int k = r - l + 1; k > 0; --k)
                    w[k] += w[k - 1] * weight[r];
                for (int k = 0; k <= r - l + 1; ++k)
                    z[l][r][k] = w[k];
            }
        }
        for (int len = 2; len <= m; ++len) {
            for (int l = 0; l + len <= m; ++l) {
                int r = l + len - 1;
                for (int k = 1; k < len; ++k) {
                    dp[l][r][k] = 100;
                    for (int mid = l; mid < r; ++mid) {
                        long double future = 0, emptyLeft = 0, emptyRight = 0;
                        for (int a = max(0, k - (r - mid));
                             a <= min(k, mid - l + 1); ++a) {
                            int b = k - a;
                            long double p =
                                z[l][mid][a] * z[mid + 1][r][b] / z[l][r][k];
                            future += p * (dp[l][mid][a] + dp[mid + 1][r][b]);
                            if (!a) emptyLeft = p;
                            if (!b) emptyRight = p;
                        }
                        int left = id[cells[l]][cells[mid]];
                        int right = id[cells[mid + 1]][cells[r]];
                        long double restoreL = (known & bit(left)) ? 1 : 1 - emptyLeft;
                        long double restoreR = (known & bit(right)) ? 1 : 1 - emptyRight;
                        long double cost = 1 + future + min(restoreL, restoreR);
                        if (cost < dp[l][r][k]) {
                            dp[l][r][k] = cost;
                            action[l][r][k] = restoreL <= restoreR ? left : right;
                        }
                    }
                }
            }
        }
        return action[0][m - 1][g.visible];
    }

    void locate(vector<Group> groups) {
        normalize(groups);
        while (!groups.empty()) {
            Group g = groups.back();
            groups.pop_back();
            int b = chooseProbe(g);
            int interval = barriers[b].cells;
            U64 before = parity;
            int delta = flip(b);
            if (delta < 0) {
                int inside = -delta - popcount(unsigned(frozen & interval));
                learn(bit(b), before);
                exclude(interval, parity);
                flip(b);
                if (inside) groups.push_back({g.cells & interval, inside});
                if (g.visible > inside)
                    groups.push_back({g.cells & ~interval, g.visible - inside});
            } else {
                exclude(interval, before);
                groups.push_back({g.cells & ~interval, g.visible});
                if (delta > 0) {
                    learn(bit(b), parity);
                    groups.push_back({interval, delta});
                }
            }
            normalize(groups);
        }
    }

    void solveSmall(int l, int r, int visible) {
        vector<int> local;
        U64 mask = 0;
        int len = r - l + 1;
        int cellMask = ((1 << len) - 1) << l;
        for (int i = l; i <= r; ++i)
            for (int j = i; j <= r; ++j) {
                local.push_back(id[i][j]);
                mask |= bit(id[i][j]);
            }

        int m = int(local.size()), states = 1 << m;
        U64 obs[4]{}, low[6]{}, belief = 0;
        for (int s = 0; s < states; ++s) {
            int hidden = 0;
            U64 active = 0;
            for (int j = 0; j < m; ++j) {
                if (s & (1 << j)) {
                    hidden |= barriers[local[j]].cells;
                    active |= bit(local[j]);
                } else {
                    low[j] |= bit(s);
                }
            }
            int v = len - popcount(unsigned(hidden));
            obs[v] |= bit(s);
            if (v != visible) continue;
            U64 assignment = parity ^ active;
            if ((assignment ^ value) & known & mask) continue;
            bool valid = true;
            for (int i = l; i <= r; ++i)
                if (forbidden[i].contains(assignment & cover[i])) {
                    valid = false;
                    break;
                }
            if (valid) belief |= bit(s);
        }

        auto shift = [&](U64 h, int d) {
            for (unsigned s = d; s; s &= s - 1) {
                int j = countr_zero(s), k = 1 << j;
                h = ((h & low[j]) << k) | ((h & ~low[j]) >> k);
            }
            return h;
        };

        // For small beliefs, minimize the total cost over equiprobable states.
        // A test at displacement d costs popcount(d) before it partitions the belief.
        unordered_map<U64, pair<int, int>> memo;
        auto exact = [&](auto &&self, U64 h) -> pair<int, int> {
            int count = popcount(h);
            if (count == 1) {
                int s = countr_zero(h);
                return {popcount(unsigned(s)), s};
            }
            auto it = memo.find(h);
            if (it != memo.end()) return it->second;
            pair<int, int> best{INT_MAX, 0};
            for (int d = 1; d < states; ++d) {
                int cost = count * popcount(unsigned(d));
                if (cost >= best.first) continue;
                U64 moved = shift(h, d);
                bool informative = true;
                for (int v = 0; v <= len; ++v)
                    if ((moved & obs[v]) == moved) informative = false;
                if (!informative) continue;
                for (int v = 0; v < len && cost < best.first; ++v)
                    if (U64 part = moved & obs[v])
                        cost += self(self, part).first;
                if (cost < best.first) best = {cost, d};
            }
            memo.emplace(h, best);
            return best;
        };

        while (visible != len) {
            int count = popcount(belief), move = 0;
            if (count <= 8) {
                move = exact(exact, belief).second;
            } else {
                double best = 1e100;
                for (int d = 1; d < states; ++d) {
                    U64 moved = shift(belief, d);
                    double uncertainty = 0;
                    bool informative = true;
                    for (int v = 0; v <= len; ++v) {
                        int k = popcount(moved & obs[v]);
                        if (k == count) informative = false;
                        if (k > 1) uncertainty += k * log2(double(k));
                    }
                    if (!informative) continue;
                    int weight = 0;
                    for (int j = 0; j < m; ++j)
                        weight += popcount(moved & ~low[j]);
                    double score = popcount(unsigned(d)) +
                                   (weight + 1.4 * uncertainty) / count;
                    if (score < best) {
                        best = score;
                        move = d;
                    }
                }
            }
            while (move && visible != len) {
                int chosen = -1, best = INT_MAX;
                for (unsigned s = move; s; s &= s - 1) {
                    int j = countr_zero(s);
                    U64 h = shift(belief, 1 << j);
                    int collision = 0;
                    for (int v = 0; v <= len; ++v) {
                        int k = popcount(h & obs[v]);
                        collision += k * k;
                    }
                    if (collision < best) {
                        best = collision;
                        chosen = j;
                    }
                }
                move ^= 1 << chosen;
                U64 moved = shift(belief, 1 << chosen);
                visible += flip(local[chosen]);
                belief = moved & obs[visible];
                if (belief != moved) break;
            }
        }

        frozen |= cellMask;
        learn(mask, parity);
        locate({});
    }

    void solveOneInvisible() {
        double cost[11]{};
        int split[11]{};
        cost[1] = 1;
        for (int m = 2; m <= n; ++m) {
            cost[m] = (1 + (m - 1) * (2 + cost[m - 1])) / m;
            split[m] = 1;
            for (int k = 2; k < m; ++k) {
                double candidate = 2 + (k * cost[k] + (m - k) * cost[m - k]) / m;
                if (candidate < cost[m]) {
                    cost[m] = candidate;
                    split[m] = k;
                }
            }
        }
        int l = 0, r = n - 1;
        for (;;) {
            int k = split[r - l + 1];
            if (k <= 1) {
                flip(id[l][l]);
                flip(id[l][l]);
                ++l;
            } else {
                int b = id[l][l + k - 1];
                bool inside = flip(b) == 1 - k;
                flip(b);
                if (inside) r = l + k - 1;
                else l += k;
            }
        }
    }

    int searchMove() {
        int selected = -1;
        for (int s = allCells ^ frozen; s; s &= s - 1) {
            int i = countr_zero(unsigned(s));
            if (selected < 0 || remaining(i) < remaining(selected) ||
                (remaining(i) == remaining(selected) && i == target))
                selected = i;
        }
        U64 free = cover[selected] & ~known;
        if (selected != target || free != dimensions) {
            target = selected;
            dimensions = free;
            anchor = parity & cover[target];
            grayIndex = 0;
            hasGoal = false;
            order.clear();
            for (U64 s = free; s; s &= s - 1)
                order.push_back(countr_zero(s));
            sort(order.begin(), order.end(), [&](int a, int b) {
                return barriers[a].r - barriers[a].l > barriers[b].r - barriers[b].l;
            });
        }

        if (hasGoal && ((parity ^ goal) & free) == 0) hasGoal = false;
        if (!hasGoal) {
            // Earlier Gray-code vertices are already ruled out. Jump over
            // ruled-out vertices rather than paying for an entire Gray run.
            do {
                ++grayIndex;
                unsigned g = grayIndex ^ (grayIndex >> 1);
                goal = anchor;
                for (; g; g &= g - 1)
                    goal ^= bit(order[countr_zero(g)]);
            } while (forbidden[target].contains(goal));
            hasGoal = true;
        }

        U64 diff = (goal ^ parity) & free;
        int best = countr_zero(diff);
        if (!has_single_bit(diff)) {
            double bestScore = -1;
            for (U64 s = diff; s; s &= s - 1) {
                int b = countr_zero(s);
                double score = 0;
                U64 next = parity ^ bit(b);
                for (int c = barriers[b].cells; c; c &= c - 1) {
                    int i = countr_zero(unsigned(c));
                    if (possible(i, next)) score += 1.0 / remaining(i);
                }
                if (score > bestScore) {
                    bestScore = score;
                    best = b;
                }
            }
        }
        return best;
    }

public:
    void run() {
        if (!(cin >> n >> total) || n < 1 || n > 10 || total < 0 || total >= n)
            return;
        allCells = (1 << n) - 1;
        for (int l = 0; l < n; ++l) {
            for (int r = l; r < n; ++r) {
                int b = int(barriers.size());
                id[l][r] = b;
                int cells = ((1 << (r - l + 1)) - 1) << l;
                barriers.push_back({l, r, cells});
                for (int i = l; i <= r; ++i) cover[i] |= bit(b);
            }
        }

        if (n <= 3) {
            solveSmall(0, n - 1, total);
            return;
        }
        if (total == n - 1) {
            solveOneInvisible();
            return;
        }

        if (total) locate({{allCells, total}});
        else locate({});

        for (;;) {
            U64 active = known & (parity ^ value);
            int b = -1;
            if (active) {
                // Every currently active, identified barrier must be flipped.
                for (U64 s = active; s; s &= s - 1) {
                    int j = countr_zero(s);
                    if (b < 0 ||
                        barriers[j].r - barriers[j].l > barriers[b].r - barriers[b].l)
                        b = j;
                }
            } else {
                bool small = false;
                for (int l = 0; l < n;) {
                    if (frozen & (1 << l)) {
                        ++l;
                        continue;
                    }
                    int r = l;
                    while (r + 1 < n && !(frozen & (1 << (r + 1)))) ++r;
                    if (r - l < 3) {
                        solveSmall(l, r, 0);
                        target = -1;
                        small = true;
                        break;
                    }
                    l = r + 1;
                }
                if (small) continue;
                b = searchMove();
            }

            int delta = flip(b);
            if (delta > 0) {
                learn(bit(b), parity);
                locate({{barriers[b].cells, delta}});
                target = -1;
            } else {
                locate({});
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver solver;
    solver.run();
    return 0;
}

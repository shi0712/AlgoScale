#include <bits/stdc++.h>
using namespace std;

class Towns {
    struct Measurement {
        int u, v, lca;
    };

    struct RadiusBounds {
        int low, high;
        int gLow, gHigh;
        int cap;
        vector<int> vertices;
    };

    struct BranchResult {
        bool balanced;
        int representative;
    };

    struct DisjointSets {
        vector<int> parent;

        explicit DisjointSets(int n) : parent(n) {
            iota(parent.begin(), parent.end(), 0);
        }

        int find(int u) {
            if (parent[u] != u) parent[u] = find(parent[u]);
            return parent[u];
        }

        void join(int u, int v) {
            parent[find(u)] = find(v);
        }
    };

    const int n, half;
    int farTown = 0, pathLength = 0;
    // low and high bound the depth of each town's projection onto the main path.
    vector<int> depth, low, high, lastProbe;
    vector<vector<int>> lca;
    vector<Measurement> probes;

    int distance(int u, int v) const {
        cout << "? " << u << ' ' << v << endl;
        int answer;
        if (!(cin >> answer) || answer <= 0) exit(0);
        return answer;
    }

    void propagate() {
        bool changed;
        do {
            changed = false;
            auto update = [&](int u, int v, int common) {
                int newLow = max(low[u], min(low[v], common));
                int newHigh = high[u];
                if (high[v] < common) newHigh = min(newHigh, high[v]);
                if (low[v] > common) newHigh = min(newHigh, common);
                changed |= newLow != low[u] || newHigh != high[u];
                low[u] = newLow;
                high[u] = newHigh;
            };
            for (auto [u, v, common] : probes) {
                update(u, v, common);
                update(v, u, common);
            }
        } while (changed);
    }

    void probe(int u, int v) {
        if (lca[u][v] >= 0) return;
        int common = (depth[u] + depth[v] - distance(u, v)) / 2;
        lca[u][v] = lca[v][u] = common;
        probes.push_back({u, v, common});
        lastProbe[v] = static_cast<int>(probes.size());
        propagate();
    }

    int radiusAt(int x, int g) const {
        // If D is the diameter, g = D - pathLength. The two diameter
        // endpoints determine the eccentricity along the 0-to-farTown path.
        return max(pathLength - x, g + x);
    }

    int bestOnInterval(int left, int right, int g) const {
        int middle = clamp((pathLength - g) / 2, left, right);
        return radiusAt(middle, g);
    }

    RadiusBounds radiusBounds() const {
        RadiusBounds bounds{INT_MAX, INT_MAX, 0, 0, pathLength, {}};
        for (int i = 1; i < n; ++i) {
            if (i == farTown) continue;
            bounds.gLow = max(bounds.gLow, depth[i] - 2 * high[i]);
            bounds.gHigh = max(bounds.gHigh, depth[i] - 2 * low[i]);
            if (low[i] == high[i]) bounds.vertices.push_back(low[i]);
        }
        for (auto [u, v, common] : probes) {
            bounds.gLow = max(bounds.gLow, min(depth[u], depth[v]) - 2 * common);
            if (low[u] >= common || low[v] >= common) bounds.vertices.push_back(common);
        }

        sort(bounds.vertices.begin(), bounds.vertices.end());
        bounds.vertices.erase(unique(bounds.vertices.begin(), bounds.vertices.end()),
                              bounds.vertices.end());
        for (int x : bounds.vertices) {
            // No deeper vertex can be a hub once x reaches the midpoint.
            if (2 * x + bounds.gLow >= pathLength) bounds.cap = min(bounds.cap, x);
            bounds.high = min(bounds.high, radiusAt(x, bounds.gHigh));
        }
        // Each internal path vertex has a town projecting onto it.
        for (int i = 1; i < n; ++i) {
            if (i == farTown) continue;
            bounds.low = min(bounds.low, bestOnInterval(low[i], high[i], bounds.gLow));
            bounds.high = min(bounds.high, max(radiusAt(low[i], bounds.gHigh),
                                                radiusAt(high[i], bounds.gHigh)));
        }
        return bounds;
    }

    // Both ends of a feasible balanced interval come from the two path sides.
    pair<int, int> balancedInterval() const {
        vector<int> lower = low, upper = high;
        nth_element(lower.begin(), lower.begin() + n - 1 - half, lower.end());
        nth_element(upper.begin(), upper.begin() + half, upper.end());
        return {lower[n - 1 - half], upper[half]};
    }

    void refineRadius(const RadiusBounds& bounds) {
        int next = -1;
        tuple<bool, int, int, int> best{};
        for (int i = 1; i < n; ++i) {
            if (low[i] == high[i] || low[i] >= bounds.cap) continue;
            int upperG = depth[i] - 2 * low[i];
            int possibleRadius = bestOnInterval(low[i], high[i], bounds.gLow);
            auto score = make_tuple(upperG > bounds.gLow, upperG, -possibleRadius,
                                    high[i] - low[i]);
            if (next == -1 || score > best) {
                next = i;
                best = score;
            }
        }

        // The anchor's comparison resolves the projection up to anchorDepth.
        int anchorDepth = min(bounds.cap, high[next]);
        int pivot = farTown;
        for (int i = 1; i < n; ++i) {
            if (low[i] >= anchorDepth && lastProbe[i] > lastProbe[pivot] &&
                i != next &&
                lca[i][next] < 0) {
                pivot = i;
            }
        }
        probe(pivot, next);
    }

    // 1 means the same branch, -1 different branches, and 0 unresolved.
    int relation(int u, int v, int x) const {
        bool uRootSide = high[u] < x, vRootSide = high[v] < x;
        bool uBeyond = low[u] >= x, vBeyond = low[v] >= x;
        if (uRootSide && vRootSide) return 1;
        if ((uRootSide && vBeyond) || (vRootSide && uBeyond)) return -1;
        if ((uBeyond && depth[u] == x + 1) ||
            (vBeyond && depth[v] == x + 1)) return -1;
        if (low[u] > x && low[v] > x) return 1;
        if ((low[u] > x && high[v] <= x) || (low[v] > x && high[u] <= x)) return -1;
        int common = lca[u][v];
        if (common > x) return 1;
        if (common >= 0 && (uBeyond || vBeyond)) return -1;
        return 0;
    }

    void compareBranches(int u, int v, int x) {
        if (low[u] < x && low[v] < x && high[u] >= x && high[v] >= x) {
            probe(farTown, u);
        }
        if (relation(u, v, x) != 0) return;
        if (high[u] < x) probe(farTown, v);
        else if (high[v] < x) probe(farTown, u);
        else probe(u, v);
    }

    BranchResult checkHub(int x) {
        for (;;) {
            DisjointSets groups(n);
            for (int u = 0; u < n; ++u) {
                for (int v = u + 1; v < n; ++v) {
                    if (relation(u, v, x) == 1) groups.join(u, v);
                }
            }

            vector<vector<bool>> different(n, vector<bool>(n));
            vector<int> roots, representative(n, -1), size(n);
            for (int i = 0; i < n; ++i) {
                int group = groups.find(i);
                if (size[group]++ == 0) roots.push_back(group);
                int previous = representative[group];
                if (previous == -1 || (low[previous] < x && low[i] >= x)) {
                    representative[group] = i;
                }
            }
            for (int group : roots) {
                if (size[group] > half) {
                    return {false, representative[group]};
                }
            }
            for (int u = 0; u < n; ++u) {
                for (int v = u + 1; v < n; ++v) {
                    if (relation(u, v, x) < 0) {
                        int a = groups.find(u), b = groups.find(v);
                        different[a][b] = different[b][a] = true;
                    }
                }
            }

            // A group with too few compatible leaves cannot belong to a
            // majority. Removing it can rule out further groups for free.
            vector<int> eligible = size;
            int remaining = n;
            bool removed;
            do {
                removed = false;
                for (int a : roots) {
                    if (eligible[a] == 0) continue;
                    int possible = remaining;
                    for (int b : roots) {
                        if (different[a][b]) possible -= eligible[b];
                    }
                    if (possible <= half) {
                        remaining -= eligible[a];
                        eligible[a] = 0;
                        removed = true;
                    }
                }
                if (remaining <= half) return {true, -1};
            } while (removed);

            vector<int> active = eligible;
            int cancellations = 0;
            auto cancel = [&](int a, int b) {
                int amount = min(active[a], active[b]);
                cancellations += amount;
                active[a] -= amount;
                active[b] -= amount;
            };

            // Use the comparisons already paid for, in their original order.
            for (const auto& query : probes) {
                int a = groups.find(query.u), b = groups.find(query.v);
                if (different[a][b]) cancel(a, b);
            }
            for (int a : roots) {
                for (int b : roots) {
                    if (different[a][b]) cancel(a, b);
                }
            }
            if (remaining - cancellations <= half) return {true, -1};

            vector<int> candidates, survivors;
            for (int a : roots) {
                if (active[a] == 0) continue;
                survivors.push_back(a);
                int possible = remaining - 2 * cancellations;
                for (int b : roots) {
                    if (different[a][b]) possible -= active[b];
                }
                if (possible + cancellations > half) candidates.push_back(a);
            }
            if (candidates.empty()) return {true, -1};

            // If only one branch can still be a majority, verify its size.
            int a = -1, b = -1;
            if (candidates.size() == 1) {
                a = candidates.front();
                for (int other : roots) {
                    if (a == other || different[a][other] || eligible[other] == 0) continue;
                    if (b == -1 || eligible[other] > eligible[b]) b = other;
                }
            } else {
                sort(survivors.begin(), survivors.end(), [&](int first, int second) {
                    return make_tuple(active[first], size[first],
                                      low[representative[first]] >= x) >
                           make_tuple(active[second], size[second],
                                      low[representative[second]] >= x);
                });
                a = survivors[0];
                b = survivors[1];
            }
            compareBranches(representative[a], representative[b], x);
        }
    }

public:
    explicit Towns(int size)
        : n(size), half(n / 2), depth(n), low(n, 1), high(n), lastProbe(n),
          lca(n, vector<int>(n, -1)) {}

    int solve() {
        for (int i = 1; i < n; ++i) {
            depth[i] = distance(0, i);
            high[i] = depth[i] - 1;
            lca[0][i] = lca[i][0] = 0;
            if (depth[i] > pathLength) {
                pathLength = depth[i];
                farTown = i;
            }
        }
        low[0] = high[0] = 0;
        low[farTown] = high[farTown] = pathLength;

        for (;;) {
            RadiusBounds bounds = radiusBounds();
            if (bounds.low != bounds.high) {
                refineRadius(bounds);
                continue;
            }

            int radius = bounds.low;
            auto [left, right] = balancedInterval();
            left = max(left, pathLength - radius);
            right = min(right, radius - bounds.gLow);

            bool possible = false;
            for (int i = 1; i < n; ++i) {
                if (i != farTown && max(left, low[i]) <= min(right, high[i])) {
                    possible = true;
                    break;
                }
            }
            if (!possible) return -radius;

            int hub = -1;
            for (int x : bounds.vertices) {
                if (left <= x && x <= right && radiusAt(x, bounds.gHigh) <= radius) {
                    hub = x;
                    break;
                }
            }
            if (hub != -1) {
                auto [balanced, rep] = checkHub(hub);
                if (balanced) return radius;
                if (high[rep] < hub || low[rep] > hub) continue;
                if (low[rep] != high[rep]) probe(farTown, rep);
                if (low[rep] == hub) return -radius;
                continue;
            }

            int next = -1;
            tuple<int, int> best{};
            for (int i = 1; i < n; ++i) {
                if (low[i] == high[i]) continue;
                int upperG = depth[i] - 2 * low[i];
                auto score = make_tuple(upperG, high[i] - low[i]);
                if (next == -1 || score > best) {
                    next = i;
                    best = score;
                }
            }
            probe(farTown, next);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 6 || n > 110) return 0;
    Towns towns(n);
    int answer = towns.solve();
    cout << "! " << answer << endl;
    return 0;
}

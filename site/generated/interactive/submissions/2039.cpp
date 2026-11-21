#include <algorithm>
#include <array>
#include <bitset>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

constexpr int MaxN = 200;
using Set = bitset<MaxN>;

struct Decision {
    bool connected;
};

class Solver {
    struct Node {
        Set original;
        Set remaining;
        int left = -1;
        int right = -1;
        int parent = -1;
        int size = 0;
        int lower = 0;
        int upper = 0;
        int overlap = 0;
        bool disjointChildren = false;
    };

    int n;
    Set all;
    unordered_map<Set, int> known;
    vector<Node> tree;
    vector<int> leaf;
    // Redundant centers stay out of the query mask. Their closed neighborhoods
    // are already covered by core, and coreSize still counts their labels.
    // A speculative center can only fail to join the core if it is isolated.
    Set core;
    Set unverified;
    Set nonisolated;
    int coreCover = 0;
    int coreSize = 0;

    // For very small inputs, keep every graph consistent with the answers.
    // Pick the query that separates the most connected/disconnected pairs.
    void solveSmall() {
        struct PossibleGraph {
            array<unsigned char, 32> coverage{};
            bool connected;
        };
        vector<pair<int, int>> edges;
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                edges.emplace_back(u, v);
            }
        }
        int full = (1 << n) - 1;
        vector<PossibleGraph> graphs(1 << edges.size());
        for (int mask = 0; mask < static_cast<int>(graphs.size()); ++mask) {
            auto& graph = graphs[mask];
            array<int, 5> adjacent;
            for (int v = 0; v < n; ++v) {
                adjacent[v] = 1 << v;
            }
            for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
                if ((mask >> i) & 1) {
                    auto [u, v] = edges[i];
                    adjacent[u] |= 1 << v;
                    adjacent[v] |= 1 << u;
                }
            }
            graph.connected = true;
            for (int set = 1; set < full; ++set) {
                int reached = 0;
                for (int v = 0; v < n; ++v) {
                    if ((set >> v) & 1) {
                        reached |= adjacent[v];
                    }
                }
                graph.coverage[set] = __builtin_popcount(static_cast<unsigned>(reached));
                if (reached == set) {
                    graph.connected = false;
                }
            }
        }
        while (true) {
            bool connected = graphs.front().connected;
            if (all_of(graphs.begin(), graphs.end(), [&](const auto& g) {
                    return g.connected == connected;
                })) {
                throw Decision{connected};
            }
            int best = 1;
            int bestScore = numeric_limits<int>::max();
            for (int set = 1; set < full; ++set) {
                array<array<int, 2>, 6> bucket{};
                for (const auto& graph : graphs) {
                    ++bucket[graph.coverage[set]][graph.connected];
                }
                int score = 0;
                for (const auto& counts : bucket) {
                    score += counts[0] * counts[1];
                }
                if (score < bestScore) {
                    bestScore = score;
                    best = set;
                }
            }
            int answer = cover(Set(best));
            erase_if(graphs, [&](const auto& graph) { return graph.coverage[best] != answer; });
        }
    }

    static Set singleton(int v) {
        Set result;
        result.set(v);
        return result;
    }

    int remember(const Set& s, int size, int value) {
        if (size != 0 && size != n && value == size) {
            throw Decision{false};
        }
        if (size == 1 && value == n) {
            throw Decision{true};
        }
        if (size == 1 && value > 1) {
            nonisolated |= s;
        }
        known.emplace(s, value);
        return value;
    }

    // Return the size of the closed neighborhood of s.
    int cover(const Set& s) {
        auto it = known.find(s);
        if (it != known.end()) {
            return it->second;
        }
        int size = static_cast<int>(s.count());
        if (size == 0 || size == n) {
            return size;
        }
        cout << "? ";
        for (int v = 0; v < n; ++v) {
            cout << (s[v] ? '1' : '0');
        }
        cout << '\n' << flush;
        int response;
        if (!(cin >> response) || response < 0 || response > n - size) {
            exit(0);
        }
        return remember(s, size, size + response);
    }

    int build(int begin, int end, int parent) {
        int id = static_cast<int>(tree.size());
        tree.emplace_back();
        tree[id].parent = parent;
        if (end - begin == 1) {
            leaf[begin] = id;
            tree[id].original.set(begin);
        } else {
            int mid = begin + (end - begin) / 2;
            int left = build(begin, mid, id);
            int right = build(mid, end, id);
            tree[id].left = left;
            tree[id].right = right;
            tree[id].original = tree[left].original | tree[right].original;
        }
        tree[id].remaining = tree[id].original;
        tree[id].lower = tree[id].size = end - begin;
        tree[id].upper = n;
        return id;
    }

    void tighten(int id) {
        Node& node = tree[id];
        if (node.size == 0) {
            return;
        }
        auto it = known.find(node.remaining);
        if (it != known.end()) {
            node.lower = node.upper = it->second;
        }
        if (node.left != -1) {
            const Node& a = tree[node.left];
            const Node& b = tree[node.right];
            node.lower = max({node.lower, a.lower, b.lower});
            node.upper = min(node.upper, a.upper + b.upper);
            if (node.upper == a.lower + b.lower && node.lower == node.upper &&
                a.lower == a.upper && b.lower == b.upper) {
                node.disjointChildren = true;
            }
            if (node.disjointChildren) {
                node.lower = max(node.lower, a.lower + b.lower);
            }
        }
        if (node.lower == node.upper) {
            remember(node.remaining, node.size, node.lower);
        }
    }

    int nodeCover(int id) {
        tighten(id);
        Node& node = tree[id];
        if (node.lower != node.upper) {
            node.lower = node.upper = cover(node.remaining);
            tighten(id);
        }
        return node.lower;
    }

    void erase(int id, int removedOverlap, int gain = 0) {
        Set removed = tree[id].remaining;
        int removedSize = tree[id].size;
        int lostBound = tree[id].upper;
        for (int at = id; at != -1; at = tree[at].parent) {
            Node& node = tree[at];
            int oldUpper = node.upper;
            int oldOverlap = node.overlap;
            node.remaining &= ~removed;
            node.size -= removedSize;
            node.overlap = max(0, node.overlap - removedOverlap);
            if (node.size == 0) {
                node.lower = node.upper = node.overlap = 0;
            } else {
                node.lower = max(node.size, node.lower - lostBound);
                tighten(at);
            }
            lostBound = min(lostBound, oldUpper - node.lower);
            if (node.size > 0) {
                // The union with the core is unchanged at ancestors of the
                // removed group, so its overlap can lose at most lostBound - gain.
                node.overlap = max(node.overlap, oldOverlap + gain - lostBound);
            }
        }
    }

    void verify(const Set& group, int missing = -1) {
        if (group.none()) {
            return;
        }
        if (missing == -1) {
            missing = n - cover(all ^ group);
        }
        if (missing == 0) {
            nonisolated |= group;
            return;
        }
        Set half;
        bool take = true;
        for (int v = 0; v < n; ++v) {
            if (group[v]) {
                if (take) {
                    half.set(v);
                }
                take = !take;
            }
        }
        const Set other = group ^ half;
        int leftMissing = n - cover(all ^ half);
        verify(half, leftMissing);
        if (leftMissing == missing) {
            nonisolated |= other;
        } else {
            verify(other);
        }
    }

    void verifySpeculation() {
        unverified &= ~nonisolated;
        if (unverified.none()) {
            return;
        }
        Set odd;
        for (int v = 1; v < n; v += 2) {
            if (unverified[v]) {
                odd.set(v);
            }
        }
        verify(unverified ^ odd);
        verify(odd);
        unverified.reset();
    }

    void checkLowDegrees() {
        for (int v = 0; v < n; ++v) {
            auto it = known.find(singleton(v));
            if (it != known.end() && it->second > 3) {
                return;
            }
        }
        Set ends;
        int endCount = 0;
        for (int v = 0; v < n; ++v) {
            int f = cover(singleton(v));
            if (f > 3) {
                return;
            }
            if (f == 2) {
                ends.set(v);
                ++endCount;
                // A degree-one vertex left uncovered by this query has a
                // degree-one neighbor, giving an isolated two-vertex component.
                if (endCount > 1 && (endCount <= 6 || endCount % 4 == 0) &&
                    cover(all ^ ends) < n) {
                    throw Decision{false};
                }
            }
            if (tree[0].remaining[v]) {
                Node& node = tree[leaf[v]];
                node.lower = node.upper = f;
            }
        }
    }

    void verifyAllNonisolated() {
        int groups = max(2, (n + 15) / 16);
        vector<Set> buckets(groups);
        for (int v = 0; v < n; ++v) {
            if (!nonisolated[v]) {
                buckets[v % groups].set(v);
            }
        }
        for (const Set& bucket : buckets) {
            verify(bucket);
        }
    }

    void checkCore() {
        // When only one vertex could be outside the core's component, a
        // disconnected graph would have to leave that vertex isolated.
        if (coreCover == n || (coreCover == n - 1 && nonisolated == all)) {
            verifySpeculation();
            throw Decision{true};
        }
        if (coreCover == coreSize) {
            throw Decision{false};
        }
    }

    void attach(int id, int combined, int overlap, bool speculative = false) {
        Node& node = tree[id];
        int gain = combined - coreCover;
        if (speculative) {
            unverified |= node.remaining;
        }
        if (combined != coreCover) {
            core |= node.remaining;
        }
        coreSize += node.size;
        coreCover = combined;
        checkCore();
        erase(id, overlap, gain);
    }

    // Two disjoint closed neighborhoods partition all vertices. A vertex on
    // either side that sees across the partition connects the two anchors.
    Set firstVertices(const Set& candidates, int count) const {
        Set group;
        for (int v = 0; count > 0 && v < n; ++v) {
            if (candidates[v]) {
                group.set(v);
                --count;
            }
        }
        return group;
    }

    void resolveDominatingPair(Set a, int aCover, Set b, int bCover) {
        verifySpeculation();
        if (aCover + bCover > n) {
            throw Decision{true};
        }
        if (aCover < bCover) {
            swap(a, b);
            swap(aCover, bCover);
        }
        int inside = 0;
        int outside = static_cast<int>(b.count());
        Set candidates = all & ~(a | b);
        int target = aCover - static_cast<int>(a.count());
        if (target == 0) {
            throw Decision{false};
        }
        int bridgeTests = 4;
        while (inside < target && outside <= bCover) {
            int size = static_cast<int>(candidates.count());
            int budget = bCover - outside;
            int step = budget == 0 ? size : 1;
            if (budget > 0 && size > 3 * budget) {
                step = max(1, size / (budget + 1));
            }
            Set group = firstVertices(candidates, step);
            int growth = cover(a | group) - aCover;
            if (growth == 0) {
                inside += step;
                candidates ^= group;
                continue;
            }
            if (budget == 0) {
                throw Decision{true};
            }
            while (step > 1) {
                int halfSize = step / 2;
                Set half = firstVertices(group, halfSize);
                int halfGrowth = cover(a | half) - aCover;
                if (halfGrowth == 0) {
                    inside += halfSize;
                    candidates ^= half;
                    group ^= half;
                    step -= halfSize;
                } else {
                    group = half;
                    step = halfSize;
                    growth = halfGrowth;
                }
            }
            if (growth < bCover && bridgeTests > 0) {
                --bridgeTests;
                if (cover(b | group) > bCover) {
                    throw Decision{true};
                }
            }
            ++outside;
            candidates ^= group;
        }
        throw Decision{inside < target};
    }

    void shrinkDominating(int id) {
        while (tree[id].size > 2 && tree[id].left != -1) {
            int l = tree[id].left;
            int r = tree[id].right;
            if (tree[l].size && nodeCover(l) == n) {
                id = l;
            } else if (tree[r].size &&
                       (tree[l].upper <= tree[l].size + 2 || nodeCover(r) == n)) {
                id = r;
            } else {
                return;
            }
        }
        if (nodeCover(id) != n) {
            return;
        }
        Set s = tree[id].remaining;
        Set a;
        for (int v = 0; v < n; ++v) {
            if (s[v]) {
                a.set(v);
                break;
            }
        }
        Set b = s ^ a;
        int ca = cover(a);
        int cb = cover(b);
        resolveDominatingPair(a, ca, b, cb);
    }

    void tryDominating() {
        int l = tree[0].left;
        int r = tree[0].right;
        if (tree[l].size && nodeCover(l) == n) {
            shrinkDominating(l);
        } else if (tree[r].size && nodeCover(r) == n) {
            shrinkDominating(r);
        }
    }

    int unionCover(int id) {
        // Removed centers in the original mask are already covered by core.
        // Using the original mask also reuses the query as the core grows inside it.
        return cover(core | tree[id].original);
    }

    int expansionVertex() {
        int id = 0;
        while (tree[id].size > 1) {
            int a = tree[id].left;
            int b = tree[id].right;
            if (tree[a].size < tree[b].size) {
                swap(a, b);
            }
            if (tree[b].size == 0 || unionCover(a) > coreCover) {
                id = a;
            } else {
                attach(a, coreCover, tree[a].upper);
                id = b;
            }
        }
        for (int v = 0; v < n; ++v) {
            if (tree[id].remaining[v]) {
                return v;
            }
        }
        return -1;
    }

    bool tryVertex(int v) {
        if (v < 0 || !tree[0].remaining[v]) {
            return false;
        }
        int id = leaf[v];
        tighten(id);
        int combined = unionCover(id);
        int increase = combined - coreCover;
        // Any nonisolated vertex covers at least two labels by itself.
        if (increase == 1 && tree[id].lower == 1) {
            attach(id, combined, tree[id].upper - 1, !nonisolated[v]);
            return true;
        }
        if (increase == 0) {
            attach(id, combined, tree[id].upper);
            return true;
        }
        int f = nodeCover(id);
        if (f > increase) {
            attach(id, combined, f - increase);
            return true;
        }
        if (combined == n) {
            resolveDominatingPair(core, coreCover, singleton(v), f);
        }
        return false;
    }

    // Return whether the subtree contains a center that overlaps the core.
    // If no additional vertex is covered, every center in the subtree can join.
    bool intersects(int id, int& combined) {
        Node& node = tree[id];
        if (!node.size) {
            return false;
        }
        tighten(id);
        if (node.overlap > 0 || node.lower + coreCover > n) {
            node.overlap = max(node.overlap, node.lower + coreCover - n);
            return true;
        }
        combined = unionCover(id);
        int increase = combined - coreCover;
        if (increase == 0) {
            return true;
        }
        if (increase >= node.upper) {
            node.lower = node.upper = increase;
            remember(node.remaining, node.size, increase);
            return false;
        }
        if (increase < node.lower) {
            node.overlap = max(node.overlap, node.lower - increase);
            return true;
        }
        int f = nodeCover(id);
        node.overlap = max(node.overlap, f - increase);
        if (node.size == 1 && combined == n && f == increase) {
            resolveDominatingPair(core, coreCover, node.remaining, f);
        }
        return f > increase;
    }

    void grow(int id) {
        // Across this cut, count the boundary vertices on both sides. The
        // remaining side has a boundary whenever the core side does.
        tree[id].overlap = max(tree[id].overlap, coreCover - coreSize + 1);
        while (true) {
            Node& node = tree[id];
            if (node.size == 1) {
                int combined = unionCover(id);
                attach(id, combined, node.upper - (combined - coreCover));
                return;
            }
            int a = node.left;
            int b = node.right;
            if (!tree[a].size) {
                tree[b].overlap = max(tree[b].overlap, node.overlap);
                id = b;
                continue;
            }
            if (!tree[b].size) {
                tree[a].overlap = max(tree[a].overlap, node.overlap);
                id = a;
                continue;
            }
            tighten(a);
            tighten(b);
            auto priority = [&](int x) {
                const Node& c = tree[x];
                return pair{c.overlap > 0 || c.lower + coreCover > n,
                            (c.lower == c.upper ? n : 0) + c.size};
            };
            if (priority(b) > priority(a)) {
                swap(a, b);
            }
            int combined = -1;
            if (intersects(a, combined)) {
                if (combined == coreCover) {
                    attach(a, combined, tree[a].upper);
                    return;
                }
                int upper = tree[a].upper;
                if (combined != -1) {
                    upper -= combined - coreCover;
                }
                tree[b].overlap = max(tree[b].overlap, node.overlap - upper);
                id = a;
            } else {
                tree[b].overlap = max({1, tree[b].overlap, node.overlap});
                id = b;
            }
        }
    }

public:
    explicit Solver(int vertices) : n(vertices), leaf(vertices) {
        for (int v = 0; v < n; ++v) {
            all.set(v);
        }
        known.reserve(n * 12);
        tree.reserve(2 * n);
    }

    void run() {
        if (n <= 1) {
            throw Decision{true};
        }
        if (n <= 5) {
            solveSmall();
        }
        core = singleton(0);
        coreCover = cover(core);
        coreSize = 1;
        build(0, n, -1);
        int first = leaf[0];
        tree[first].lower = tree[first].upper = coreCover;
        erase(first, 0);

        int misses = 0;
        int redundant = 0;
        int batch = 2;
        bool expansionEnabled = true;
        bool triedFullBatch = false;
        for (int v = 1; v < n; ++v) {
            if (!tree[0].remaining[v]) {
                continue;
            }
            int previousCover = coreCover;
            if (tryVertex(v)) {
                misses = 0;
                redundant = coreCover == previousCover ? redundant + 1 : 0;
            } else {
                ++misses;
                redundant = 0;
            }
            if (n >= 12 && v == 2 && coreCover <= max(6, n / 8)) {
                tryDominating();
            }
            if (redundant >= 2) {
                if (!triedFullBatch && n >= 24 && coreCover * 3 >= n) {
                    triedFullBatch = true;
                    int needed = coreCover - coreSize;
                    Set group = firstVertices(tree[0].remaining, needed);
                    if (needed > 1 && cover(core | group) == coreCover) {
                        throw Decision{false};
                    }
                }
                if (expansionEnabled && coreCover >= n * 3 / 4 && n - coreCover <= 12 &&
                    tree[0].size >= 12) {
                    int next = expansionVertex();
                    expansionEnabled = tryVertex(next);
                    redundant = 0;
                } else if (v + batch < n) {
                    Set group;
                    for (int u = v + 1; u <= v + batch; ++u) {
                        if (tree[0].remaining[u]) {
                            group.set(u);
                        }
                    }
                    if (group.count() > 1 && cover(core | group) == coreCover) {
                        for (int u = v + 1; u <= v + batch; ++u) {
                            if (group[u]) {
                                attach(leaf[u], coreCover, tree[leaf[u]].upper);
                            }
                        }
                        v += batch;
                        batch *= 2;
                    } else {
                        redundant = 0;
                        batch = 2;
                    }
                }
            }
            if (misses >= (coreCover * 4 >= n ? 5 : 2)) {
                break;
            }
        }
        if (n >= 24 && coreCover <= n / 4) {
            verifyAllNonisolated();
            if (n >= 80) {
                while (coreCover <= max(12, n / 16)) {
                    grow(0);
                }
                checkLowDegrees();
            }
        }
        while (true) {
            if (expansionEnabled && n >= 24 && n - coreCover <= 12 && tree[0].size >= 12) {
                expansionEnabled = tryVertex(expansionVertex());
            } else {
                grow(0);
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 0 || n > MaxN) {
        return 0;
    }
    try {
        Solver(n).run();
    } catch (const Decision& result) {
        cout << "! " << (result.connected ? 1 : 0) << '\n' << flush;
    }
}

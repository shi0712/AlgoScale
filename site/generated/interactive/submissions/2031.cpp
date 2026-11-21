#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <unordered_map>
#include <vector>

using namespace std;

using Mask = unsigned __int128;

int bitCount(Mask mask) {
    return __builtin_popcountll(static_cast<uint64_t>(mask)) +
           __builtin_popcountll(static_cast<uint64_t>(mask >> 64));
}

int firstBit(Mask mask) {
    uint64_t low = static_cast<uint64_t>(mask);
    if (low != 0) return __builtin_ctzll(low);
    return 64 + __builtin_ctzll(static_cast<uint64_t>(mask >> 64));
}

struct Signal {
    int vertex;
    int radius;
};

struct Tree {
    static constexpr int levels = 15;

    int n;
    vector<int> parent, depth, degree;
    vector<array<int, 3>> neighbors;
    vector<array<int, levels>> ancestor;

    explicit Tree(int size)
        : n(size), parent(size + 1), depth(size + 1), degree(size + 1),
          neighbors(size + 1), ancestor(size + 1) {}

    void addVertex(int v, int p) {
        parent[v] = p;
        depth[v] = depth[p] + 1;
        neighbors[v][degree[v]++] = p;
        neighbors[p][degree[p]++] = v;
        ancestor[v][0] = p;
        for (int i = 1; i < levels; ++i) {
            ancestor[v][i] = ancestor[ancestor[v][i - 1]][i - 1];
        }
    }

    int ascend(int v, int steps) const {
        while (steps != 0) {
            int i = __builtin_ctz(static_cast<unsigned>(steps));
            v = ancestor[v][i];
            steps &= steps - 1;
        }
        return v;
    }

    int lca(int u, int v) const {
        if (depth[u] < depth[v]) swap(u, v);
        u = ascend(u, depth[u] - depth[v]);
        if (u == v) return u;
        for (int i = levels - 1; i >= 0; --i) {
            if (ancestor[u][i] != ancestor[v][i]) {
                u = ancestor[u][i];
                v = ancestor[v][i];
            }
        }
        return parent[u];
    }

    int distance(int u, int v) const {
        return depth[u] + depth[v] - 2 * depth[lca(u, v)];
    }

    int alongPath(int u, int v, int steps) const {
        int common = lca(u, v);
        int up = depth[u] - depth[common];
        if (steps <= up) return ascend(u, steps);
        return ascend(v, up + depth[v] - depth[common] - steps);
    }
};

class BallCounts {
    struct Histogram {
        int offset;
        int maxDistance;
    };

    struct Entry {
        int distance;
        int all;
        int excluded;
    };

    const Tree& tree;
    vector<array<Entry, Tree::levels>> entries;
    vector<int> entryCount;
    vector<Histogram> histograms;
    vector<int> counts;
    vector<char> removed;
    vector<int> traversalParent, subtreeSize, distance;
    vector<int> component, branch;
    int poolSize = 0;

    void allocateHistogram(int id, int maxDistance) {
        histograms[id] = {poolSize, maxDistance};
        poolSize += maxDistance + 1;
    }

    void build(int start) {
        component.clear();
        component.push_back(start);
        traversalParent[start] = 0;
        for (size_t i = 0; i < component.size(); ++i) {
            int u = component[i];
            subtreeSize[u] = 1;
            for (int j = 0; j < tree.degree[u]; ++j) {
                int v = tree.neighbors[u][j];
                if (v == traversalParent[u] || removed[v]) continue;
                traversalParent[v] = u;
                component.push_back(v);
            }
        }
        for (size_t i = component.size() - 1; i > 0; --i) {
            int u = component[i];
            subtreeSize[traversalParent[u]] += subtreeSize[u];
        }

        int centroid = start;
        int half = static_cast<int>(component.size()) / 2;
        while (true) {
            int heavy = 0;
            for (int j = 0; j < tree.degree[centroid]; ++j) {
                int v = tree.neighbors[centroid][j];
                if (!removed[v] && traversalParent[v] == centroid && subtreeSize[v] > half) {
                    heavy = v;
                    break;
                }
            }
            if (heavy == 0) break;
            centroid = heavy;
        }

        removed[centroid] = true;
        int all = static_cast<int>(histograms.size());
        histograms.push_back({});
        entries[centroid][entryCount[centroid]++] = {0, all, -1};

        int maxDistance = 0;
        for (int j = 0; j < tree.degree[centroid]; ++j) {
            int startBranch = tree.neighbors[centroid][j];
            if (removed[startBranch]) continue;
            int excluded = static_cast<int>(histograms.size());
            histograms.push_back({});
            branch.clear();
            branch.push_back(startBranch);
            traversalParent[startBranch] = centroid;
            distance[startBranch] = 1;
            for (size_t i = 0; i < branch.size(); ++i) {
                int u = branch[i];
                entries[u][entryCount[u]++] = {distance[u], all, excluded};
                for (int k = 0; k < tree.degree[u]; ++k) {
                    int v = tree.neighbors[u][k];
                    if (v == traversalParent[u] || removed[v]) continue;
                    traversalParent[v] = u;
                    distance[v] = distance[u] + 1;
                    branch.push_back(v);
                }
            }
            int branchMax = distance[branch.back()];
            allocateHistogram(excluded, branchMax);
            maxDistance = max(maxDistance, branchMax);
        }
        allocateHistogram(all, maxDistance);

        for (int j = 0; j < tree.degree[centroid]; ++j) {
            int v = tree.neighbors[centroid][j];
            if (!removed[v]) build(v);
        }
    }

    int prefix(int histogram, int distanceLimit) const {
        const Histogram& h = histograms[histogram];
        return counts[h.offset + min(distanceLimit, h.maxDistance)];
    }

public:
    explicit BallCounts(const Tree& input)
        : tree(input), entries(input.n + 1), entryCount(input.n + 1), removed(input.n + 1),
          traversalParent(input.n + 1), subtreeSize(input.n + 1), distance(input.n + 1) {
        component.reserve(input.n);
        branch.reserve(input.n);
        histograms.reserve(2 * input.n);
        build(1);
        counts.resize(poolSize);
    }

    void reset(const vector<int>& candidates) {
        fill(counts.begin(), counts.end(), 0);
        for (int v : candidates) {
            int last = entryCount[v] - 1;
            for (int i = 0; i < last; ++i) {
                const Entry& e = entries[v][i];
                ++counts[histograms[e.all].offset + e.distance];
                ++counts[histograms[e.excluded].offset + e.distance];
            }
            ++counts[histograms[entries[v][last].all].offset];
        }
        for (const Histogram& h : histograms) {
            for (int i = h.offset + 1; i <= h.offset + h.maxDistance; ++i) {
                counts[i] += counts[i - 1];
            }
        }
    }

    int count(int vertex, int radius) const {
        // At each ancestor centroid, exclude the branch containing the center
        // so each candidate is counted where its path first meets a centroid.
        int answer = 0;
        int last = entryCount[vertex] - 1;
        for (int i = 0; i < last; ++i) {
            const Entry& e = entries[vertex][i];
            int limit = radius - e.distance;
            if (limit >= 0) answer += prefix(e.all, limit) - prefix(e.excluded, limit);
        }
        if (radius >= 0) answer += prefix(entries[vertex][last].all, radius);
        return answer;
    }
};

class BalancedSearch {
    struct Trial {
        Signal signal;
        int inside;
    };

    const Tree& tree;
    BallCounts counts;
    vector<int> weight;
    int checksLeft = 96;

    int medianRadius(int u, int target) const {
        int low = 0, high = tree.n - 1;
        while (low < high) {
            int mid = (low + high) / 2;
            if (counts.count(u, mid) >= target) high = mid;
            else low = mid + 1;
        }
        return low;
    }

    Trial scan(const vector<int>& candidates) {
        counts.reset(candidates);
        int total = static_cast<int>(candidates.size());
        int target = total / 2;
        int bestBalance = 0;
        Trial best{{candidates.front(), 0}, 1};
        auto consider = [&](int vertex, int radius, int inside) {
            int balance = min(inside, total - inside);
            if (balance > bestBalance) {
                best = {{vertex, radius}, inside};
                bestBalance = balance;
            }
        };
        auto inspect = [&](int u) {
            int radius = medianRadius(u, target);
            consider(u, radius, counts.count(u, radius));
            if (bestBalance < target && radius > 0) consider(u, radius - 1, counts.count(u, radius - 1));
            return radius;
        };

        fill(weight.begin(), weight.end(), 0);
        for (int v : candidates) weight[v] = 1;
        for (int v = tree.n; v > 1; --v) weight[tree.parent[v]] += weight[v];
        int centroid = 1;
        while (true) {
            int heavy = 0;
            for (int j = 0; j < tree.degree[centroid]; ++j) {
                int v = tree.neighbors[centroid][j];
                if (tree.parent[v] == centroid && weight[v] > target) {
                    heavy = v;
                    break;
                }
            }
            if (heavy == 0) break;
            centroid = heavy;
        }
        int startRadius = inspect(centroid);
        if (bestBalance == target) return best;

        for (int u : {1, candidates.front(), candidates.back()}) {
            if (u != centroid) inspect(u);
            if (bestBalance == target) return best;
        }
        for (int i = 1; i < 32; ++i) {
            int u = candidates[static_cast<size_t>(i) * total / 32];
            if (u != centroid) inspect(u);
            if (bestBalance == target) return best;
        }

        struct Visit {
            int vertex;
            int previous;
            int median;
        };
        vector<Visit> stack;
        stack.push_back({centroid, 0, startRadius});
        while (!stack.empty()) {
            auto [u, previous, radius] = stack.back();
            stack.pop_back();
            for (int j = 0; j < tree.degree[u]; ++j) {
                int v = tree.neighbors[u][j];
                if (v == previous) continue;
                int side = tree.parent[v] == u ? weight[v] : total - weight[u];
                // Centers off the candidate hull induce the same cuts as
                // their projections onto the hull, with the radius reduced.
                if (side == 0 || side == total) continue;

                // The distance to each candidate changes by one across this edge.
                int inside = counts.count(v, radius);
                consider(v, radius, inside);
                if (bestBalance == target) return best;
                int nextRadius = radius;
                if (inside < target) {
                    ++nextRadius;
                    consider(v, nextRadius, counts.count(v, nextRadius));
                } else if (radius > 0) {
                    int below = counts.count(v, radius - 1);
                    if (below >= target) {
                        --nextRadius;
                        consider(v, radius - 2, counts.count(v, radius - 2));
                    }
                    consider(v, radius - 1, below);
                }
                if (bestBalance == target) return best;
                stack.push_back({v, u, nextRadius});
            }
        }
        return best;
    }

    vector<Trial> sample(const vector<int>& candidates, const Trial& best) const {
        int size = static_cast<int>(candidates.size());
        int target = size / 2;
        int minimum = min(best.inside, size - best.inside) - (size + 19) / 20;
        vector<Trial> trials;
        for (int i = 0; i < 96; ++i) {
            int u = candidates[static_cast<size_t>(i) * size / 96];
            int radius = medianRadius(u, target);
            for (int r = max(0, radius - 1); r <= radius; ++r) {
                int inside = counts.count(u, r);
                if (min(inside, size - inside) >= minimum && inside > 0 && inside < size) {
                    trials.push_back({{u, r}, inside});
                }
            }
        }
        stable_sort(trials.begin(), trials.end(), [&](const Trial& a, const Trial& b) {
            return min(a.inside, size - a.inside) > min(b.inside, size - b.inside);
        });
        return trials;
    }

public:
    explicit BalancedSearch(const Tree& input) : tree(input), counts(input), weight(input.n + 1) {}

    Signal choose(const vector<int>& candidates) {
        Trial best = scan(candidates);
        if (checksLeft < 2 || candidates.size() <= 4) return best.signal;
        int size = static_cast<int>(candidates.size());
        vector<Trial> alternatives = sample(candidates, best);
        vector<vector<int>> tested;
        vector<int> inside, outside;
        inside.reserve(size);
        outside.reserve(size);
        int bestAfterTwo = size;
        int bestAfterOne = size;
        auto evaluate = [&](const Trial& trial) {
            int afterOne = max(trial.inside, size - trial.inside);
            if ((afterOne + 1) / 2 > bestAfterTwo) return;
            inside.clear();
            outside.clear();
            for (int v : candidates) {
                auto& side = tree.distance(v, trial.signal.vertex) <= trial.signal.radius ? inside : outside;
                side.push_back(v);
            }
            // Canonicalize the partition so complementary or duplicate balls
            // do not consume more lookahead checks.
            const vector<int>& canonical = inside.front() == candidates.front() ? inside : outside;
            for (const auto& tried : tested) {
                if (tried == canonical) return;
            }
            tested.push_back(canonical);
            if (inside.size() < outside.size()) inside.swap(outside);
            auto worst = [&](const vector<int>& part) {
                if (part.size() <= 2) return 1;
                --checksLeft;
                int partSize = static_cast<int>(part.size());
                int hits = scan(part).inside;
                return max(hits, partSize - hits);
            };
            int afterTwo = worst(inside);
            if (afterTwo > bestAfterTwo) return;
            afterTwo = max(afterTwo, worst(outside));
            if (pair(afterTwo, afterOne) < pair(bestAfterTwo, bestAfterOne)) {
                best = trial;
                bestAfterTwo = afterTwo;
                bestAfterOne = afterOne;
            }
        };
        evaluate(best);
        for (const Trial& trial : alternatives) {
            if (bestAfterTwo == (size + 3) / 4 || checksLeft < 2 || tested.size() >= 10) break;
            evaluate(trial);
        }
        return best.signal;
    }
};

template<int blocks>
struct CandidateMask {
    array<Mask, blocks> bits{};

    static CandidateMask singleton(int index) {
        CandidateMask result;
        result.bits[index / 128] = Mask{1} << (index % 128);
        return result;
    }

    static CandidateMask prefix(int size) {
        CandidateMask result;
        for (int i = 0; i < blocks; ++i) {
            int filled = clamp(size - 128 * i, 0, 128);
            result.bits[i] = filled == 128 ? ~Mask{0} : (Mask{1} << filled) - 1;
        }
        return result;
    }

    int count() const {
        int result = 0;
        for (Mask block : bits) result += bitCount(block);
        return result;
    }

    int first() const {
        for (int i = 0; i < blocks; ++i) {
            if (bits[i] != 0) return 128 * i + firstBit(bits[i]);
        }
        return -1;
    }

    bool containsFirst() const { return (bits[0] & 1) != 0; }
    bool operator==(const CandidateMask&) const = default;

    CandidateMask operator&(const CandidateMask& other) const {
        CandidateMask result;
        for (int i = 0; i < blocks; ++i) result.bits[i] = bits[i] & other.bits[i];
        return result;
    }

    CandidateMask operator^(const CandidateMask& other) const {
        CandidateMask result;
        for (int i = 0; i < blocks; ++i) result.bits[i] = bits[i] ^ other.bits[i];
        return result;
    }

    struct Hash {
        size_t operator()(const CandidateMask& mask) const {
            uint64_t hash = 0;
            for (Mask block : mask.bits) {
                hash ^= static_cast<uint64_t>(block);
                hash = rotl(hash, 27) * 0x3c79ac492ba7b653ULL;
                hash ^= static_cast<uint64_t>(block >> 64);
            }
            hash ^= hash >> 33;
            hash *= 0xff51afd7ed558ccdULL;
            return hash ^ (hash >> 33);
        }
    };
};

template<int blocks>
class Endgame {
public:
    static constexpr int maxSize = blocks * 128;
    using Set = CandidateMask<blocks>;
    static constexpr int maxDepth = bit_width(static_cast<unsigned>(maxSize - 1));

private:
    struct Query {
        Set split;
        uint16_t a, b;
        uint16_t balance;
        bool secondMidpoint;
        bool inverted;
    };

    struct Bounds {
        int impossible = 0;
        int achievable = maxSize;
        int query = -1;
    };

    struct Option {
        int query;
        int next;
    };

    enum class Result { no, yes, unknown };

    const Tree& tree;
    const vector<int>& vertices;
    int m;
    Set full;
    vector<Query> queries;
    vector<int> order;
    vector<array<int, 2>> representatives;
    unordered_map<Set, Bounds, typename Set::Hash> memo;
    array<vector<Option>, maxSize + 1> deferred;
    // The search can stop at its work limit and use a certified strategy
    // already found for this state, or an informative balanced split.
    int totalBudget = 18'000'000 / blocks;
    int budget = 0;

    int pairIndex(int a, int b) const { return a * (2 * m - a - 1) / 2 + b; }

    void buildQueries() {
        vector<uint16_t> distance(m * m);
        int diameter = 0;
        for (int a = 0; a < m; ++a) {
            for (int b = a + 1; b < m; ++b) {
                int d = tree.distance(vertices[a], vertices[b]);
                distance[a * m + b] = distance[b * m + a] = static_cast<uint16_t>(d);
                diameter = max(diameter, d);
            }
        }

        vector<Set> prefix;
        prefix.reserve(m * min(m, diameter + 1));
        vector<int> offset(m);
        vector<array<uint16_t, 2>> endpoint(m * m);
        vector<int> byDistance(m);
        vector<int> layers;
        vector<int> bucket(diameter + 2);
        iota(byDistance.begin(), byDistance.end(), 0);
        for (int a = 0; a < m; ++a) {
            const uint16_t* row = &distance[a * m];
            if (diameter < m) {
                fill(bucket.begin(), bucket.end(), 0);
                for (int b = 0; b < m; ++b) ++bucket[row[b] + 1];
                partial_sum(bucket.begin(), bucket.end(), bucket.begin());
                for (int b = 0; b < m; ++b) byDistance[bucket[row[b]]++] = b;
            } else {
                sort(byDistance.begin(), byDistance.end(), [&](int b, int c) { return row[b] < row[c]; });
            }
            offset[a] = static_cast<int>(prefix.size());
            layers.clear();
            Set included;
            for (int i = 0; i < m;) {
                int layer = row[byDistance[i]];
                layers.push_back(layer);
                do {
                    int candidate = byDistance[i++];
                    included.bits[candidate / 128] |= Mask{1} << (candidate % 128);
                } while (i < m && row[byDistance[i]] == layer);
                prefix.push_back(included);
            }
            int at = 0;
            for (int b : byDistance) {
                if (layers[at] < row[b]) ++at;
                int plusOne = at;
                if (at + 1 < static_cast<int>(layers.size()) && layers[at + 1] == row[b] + 1) ++plusOne;
                endpoint[a * m + b] = {static_cast<uint16_t>(at), static_cast<uint16_t>(plusOne)};
            }
        }
        auto within = [&](int a, int b, bool plusOne) -> const Set& {
            return prefix[offset[a] + endpoint[a * m + b][plusOne]];
        };

        unordered_map<Set, int, typename Set::Hash> seen;
        seen.reserve(m * min(m / 2, 48));
        auto add = [&](const Set& ball, int a, int b, bool secondMidpoint) {
            bool inverted = ball.containsFirst();
            Set split = inverted ? full ^ ball : ball;
            if (split == Set{}) return -1;
            int id = static_cast<int>(queries.size());
            auto [it, inserted] = seen.try_emplace(split, id);
            if (inserted) {
                int count = split.count();
                queries.push_back({split, static_cast<uint16_t>(a), static_cast<uint16_t>(b),
                                   static_cast<uint16_t>(min(count, m - count)), secondMidpoint, inverted});
            }
            return it->second;
        };
        for (int a = 0; a < m; ++a) {
            representatives[pairIndex(a, a)] = {add(Set::singleton(a), a, a, false), -1};
        }

        // A midpoint ball of a diameter pair contains all candidates of the
        // original ball. For an odd diameter, the midpoint on the original
        // center's side keeps the new ball inside the original ball.
        for (int a = 0; a < m; ++a) {
            for (int b = a + 1; b < m; ++b) {
                auto& ids = representatives[pairIndex(a, b)];
                bool odd = distance[a * m + b] % 2 != 0;
                ids[0] = add(within(a, b, false) & within(b, a, odd), a, b, false);
                ids[1] = odd ? add(within(a, b, true) & within(b, a, false), a, b, true) : -1;
                if (ids[1] == ids[0]) ids[1] = -1;
            }
        }
        order.resize(queries.size());
        iota(order.begin(), order.end(), 0);
        stable_sort(order.begin(), order.end(), [&](int a, int b) {
            return queries[a].balance > queries[b].balance;
        });
    }

    Result canSolve(const Set& state, int size, int depth) {
        if (size <= depth + 1) return Result::yes;
        if (size > (1 << min(depth, maxDepth))) return Result::no;
        auto found = memo.find(state);
        if (found == memo.end()) {
            if (budget == 0 || memo.size() >= 160'000) return Result::unknown;
            found = memo.try_emplace(state).first;
        }
        Bounds& bounds = found->second;
        if (bounds.impossible >= depth) return Result::no;
        if (bounds.achievable <= depth) return Result::yes;

        int smallest = max(1, size - (1 << min(depth - 1, maxDepth)));
        int ideal = size / 2;
        array<int, maxSize / 2 + 1> head;
        fill(head.begin() + smallest, head.begin() + ideal + 1, -1);
        vector<Option>& pending = deferred[size];
        pending.clear();

        auto trySplit = [&](int id, Set part, int count) {
            Set other = state ^ part;
            int otherCount = size - count;
            if (count < otherCount) {
                swap(count, otherCount);
                swap(part, other);
            }
            Result result = canSolve(part, count, depth - 1);
            if (result == Result::yes) result = canSolve(other, otherCount, depth - 1);
            if (result == Result::yes) {
                bounds.achievable = depth;
                bounds.query = id;
            }
            return result;
        };
        auto examine = [&](int id) {
            if (budget == 0) return Result::unknown;
            --budget;
            if (id < 0 || queries[id].balance < smallest) return Result::no;
            Set part = state & queries[id].split;
            int count = part.count();
            int balance = min(count, size - count);
            if (balance < smallest) return Result::no;
            if (balance == ideal) return trySplit(id, part, count);
            pending.push_back({id, head[balance]});
            head[balance] = static_cast<int>(pending.size()) - 1;
            return Result::no;
        };

        if (size * size >= static_cast<int>(queries.size())) {
            for (int id : order) {
                if (queries[id].balance < smallest) break;
                Result result = examine(id);
                if (result != Result::no) return result;
            }
        } else {
            // Every split of this subset has a representative from a pair
            // in the subset, so small states need only a few queries examined.
            array<int, maxSize> members;
            int next = 0;
            for (int i = 0; i < blocks; ++i) {
                for (Mask block = state.bits[i]; block != 0; block &= block - 1) {
                    members[next++] = 128 * i + firstBit(block);
                }
            }
            for (int i = 0; i < size; ++i) {
                int row = pairIndex(members[i], 0);
                if (smallest == 1) {
                    Result result = examine(representatives[row + members[i]][0]);
                    if (result != Result::no) return result;
                }
                for (int j = i + 1; j < size; ++j) {
                    auto ids = representatives[row + members[j]];
                    Result result = examine(ids[0]);
                    if (result != Result::no) return result;
                    if (ids[1] >= 0) {
                        result = examine(ids[1]);
                        if (result != Result::no) return result;
                    }
                }
            }
        }
        for (int balance = ideal - 1; balance >= smallest; --balance) {
            for (int i = head[balance]; i != -1; i = pending[i].next) {
                if (budget == 0) return Result::unknown;
                --budget;
                int id = pending[i].query;
                Set part = state & queries[id].split;
                Result result = trySplit(id, part, part.count());
                if (result != Result::no) return result;
            }
        }
        bounds.impossible = depth;
        return Result::no;
    }

    int bestBalanced(const Set& state, int size) const {
        int best = -1, bestBalance = 0;
        for (int id : order) {
            if (queries[id].balance <= bestBalance) break;
            int count = (state & queries[id].split).count();
            int balance = min(count, size - count);
            if (balance <= bestBalance) continue;
            best = id;
            bestBalance = balance;
            if (balance == size / 2) break;
        }
        return best;
    }

public:
    Endgame(const Tree& input, const vector<int>& candidates)
        : tree(input), vertices(candidates), m(static_cast<int>(candidates.size())),
          full(Set::prefix(m)), representatives(m * (m + 1) / 2) {
        buildQueries();
        memo.reserve(2048);
    }

    Set initial() const { return full; }

    pair<Signal, Set> choose(const Set& state) {
        int size = state.count();
        if (size <= 3) {
            int candidate = state.first();
            return {{vertices[candidate], 0}, Set::singleton(candidate)};
        }

        int allowance = min(totalBudget, 6'000'000 / blocks);
        budget = allowance;
        for (int depth = bit_width(static_cast<unsigned>(size - 1)); depth < size; ++depth) {
            Result result = canSolve(state, size, depth);
            if (result != Result::no) break;
        }
        totalBudget -= allowance - budget;

        auto found = memo.find(state);
        int chosen = found != memo.end() ? found->second.query : -1;
        if (chosen < 0) chosen = bestBalanced(state, size);
        const Query& query = queries[chosen];
        int a = vertices[query.a], b = vertices[query.b];
        int distance = tree.distance(a, b);
        int midpoint = tree.alongPath(a, b, (distance + query.secondMidpoint) / 2);
        Set inside = query.inverted ? full ^ query.split : query.split;
        return {{midpoint, (distance + 1) / 2}, inside};
    }
};

int sendSignal(Signal signal) {
    cout << "? " << signal.vertex << ' ' << signal.radius << '\n' << flush;
    int reply;
    if (!(cin >> reply) || (reply != 0 && reply != 1)) return -1;
    return reply;
}

template<int blocks>
int locateRemaining(const Tree& tree, const vector<int>& candidates) {
    Endgame<blocks> endgame(tree, candidates);
    auto state = endgame.initial();
    while (state.count() > 1) {
        auto [signal, inside] = endgame.choose(state);
        int reply = sendSignal(signal);
        if (reply < 0) return 0;
        auto kept = state & inside;
        state = reply == 1 ? kept : state ^ kept;
        if (state.first() < 0) return 0;
    }
    return candidates[state.first()];
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;
    Tree tree(n);
    for (int v = 2; v <= n; ++v) {
        int parent;
        if (!(cin >> parent)) return 0;
        tree.addVertex(v, parent);
    }

    vector<int> candidates(n);
    iota(candidates.begin(), candidates.end(), 1);
    auto useLargerEndgame = [&]() {
        int size = static_cast<int>(candidates.size());
        if (size > Endgame<16>::maxSize) return false;
        if (size <= Endgame<4>::maxSize) return true;
        int endpoint = candidates.front();
        auto farthest = [&](int u) {
            pair<int, int> best{0, u};
            for (int v : candidates) best = max(best, {tree.distance(u, v), v});
            return best;
        };
        int diameter = farthest(farthest(endpoint).second).first;
        int bytesPerMask = size <= Endgame<8>::maxSize ? 128 : 256;
        if (static_cast<int64_t>(size) * (diameter + 1) * bytesPerMask > 26'000'000) return false;
        vector<int> weight(tree.n + 1);
        for (int v : candidates) weight[v] = 1;
        int hullSize = 1;
        for (int v = tree.n; v > 1; --v) {
            if (weight[v] > 0 && weight[v] < size) ++hullSize;
            weight[tree.parent[v]] += weight[v];
        }
        return static_cast<int64_t>(hullSize) * ((diameter + 1) / 2 + 1) <= 130'000;
    };
    if (!useLargerEndgame()) {
        BalancedSearch search(tree);
        while (!useLargerEndgame()) {
            Signal signal = search.choose(candidates);
            int reply = sendSignal(signal);
            if (reply < 0) return 0;
            auto remove = [&](int v) {
                return (tree.distance(v, signal.vertex) <= signal.radius) != (reply == 1);
            };
            candidates.erase(remove_if(candidates.begin(), candidates.end(), remove), candidates.end());
            if (candidates.empty()) return 0;
        }
    }

    int answer;
    if (candidates.size() == 1) answer = candidates.front();
    else if (candidates.size() <= Endgame<1>::maxSize) answer = locateRemaining<1>(tree, candidates);
    else if (candidates.size() <= Endgame<2>::maxSize) answer = locateRemaining<2>(tree, candidates);
    else if (candidates.size() <= Endgame<4>::maxSize) answer = locateRemaining<4>(tree, candidates);
    else if (candidates.size() <= Endgame<8>::maxSize) answer = locateRemaining<8>(tree, candidates);
    else answer = locateRemaining<16>(tree, candidates);
    if (answer != 0) cout << "! " << answer << '\n' << flush;
    return 0;
}

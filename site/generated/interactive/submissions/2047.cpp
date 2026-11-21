#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

constexpr int MAX_N = 1000;
constexpr int EXACT_EDGE_LIMIT = 120;
using Bits = bitset<MAX_N>;
using Edge = pair<int, int>;

struct DisjointSet {
    vector<int> parent, size;

    explicit DisjointSet(int n) : parent(n), size(n, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int v) {
        while (parent[v] != v) {
            v = parent[v] = parent[parent[v]];
        }
        return v;
    }

    bool join(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (size[a] < size[b]) swap(a, b);
        parent[b] = a;
        size[a] += size[b];
        return true;
    }
};

struct Reconstruction {
    int n;
    // Both adjacency tables are symmetric. A possible edge is still undecided.
    vector<Bits> possible, known;
    vector<Edge> edges;
    vector<Bits> selections, lamps;
    // Each entry is a lit lamp whose query has no confirmed source yet.
    vector<vector<int>> positive;
    vector<int> degree;
    DisjointSet components;
    mt19937 rng;
    long long remaining;
    bool stalled = false;

    explicit Reconstruction(int size, uint32_t seed = 823709)
        : n(size), possible(n), known(n), positive(n), degree(n),
          components(n), rng(seed), remaining(1LL * n * (n - 1) / 2) {
        Bits all;
        for (int v = 0; v < n; ++v) all.set(v);
        for (int v = 0; v < n; ++v) {
            possible[v] = all;
            possible[v].reset(v);
        }
    }

    void force(int a, int b) {
        if (known[a][b]) return;
        if (!possible[a][b] || !components.join(a, b)) exit(0);
        possible[a].reset(b);
        possible[b].reset(a);
        known[a].set(b);
        known[b].set(a);
        ++degree[a];
        ++degree[b];
        edges.emplace_back(a, b);
    }

    bool done() const { return int(edges.size()) == n - 1; }

    void removeCycles() {
        vector<Bits> members(n);
        for (int v = 0; v < n; ++v) members[components.find(v)].set(v);
        for (int v = 0; v < n; ++v) possible[v] &= ~members[components.find(v)];
    }

    bool forceBridges() {
        // Contract the confirmed forest. Parallel edges retain different IDs.
        vector<vector<pair<int, int>>> graph(n);
        vector<pair<int, int>> link;
        vector<int> roots(n);
        for (int v = 0; v < n; ++v) roots[v] = components.find(v);
        for (int v = 0; v < n; ++v) {
            for (int u = possible[v]._Find_next(v); u < n; u = possible[v]._Find_next(u)) {
                int id = link.size();
                link.emplace_back(v, u);
                graph[roots[v]].emplace_back(roots[u], id);
                graph[roots[u]].emplace_back(roots[v], id);
            }
        }
        vector<int> in(n, -1), low(n);
        int timer = 0;
        bool changed = false;
        auto dfs = [&](auto&& self, int v, int parentEdge) -> void {
            in[v] = low[v] = timer++;
            for (auto [u, id] : graph[v]) {
                if (id == parentEdge) continue;
                if (in[u] != -1) {
                    low[v] = min(low[v], in[u]);
                } else {
                    self(self, u, id);
                    low[v] = min(low[v], low[u]);
                    if (low[u] > in[v]) {
                        force(link[id].first, link[id].second);
                        changed = true;
                    }
                }
            }
        };
        dfs(dfs, roots[0], -1);
        return changed;
    }

    bool restrictRequiredConnections() {
        // If a lamp can only be explained by edges between two components,
        // their unique connecting edge must be one of those explanations.
        struct Requirement {
            int vertex;
            Bits neighbors;
        };
        vector<int> root(n);
        vector<Bits> members(n);
        for (int v = 0; v < n; ++v) {
            root[v] = components.find(v);
            members[root[v]].set(v);
        }

        DisjointSet contracted(n);
        vector<Requirement> required;
        unordered_map<int, int> byPair;
        for (int v = 0; v < n; ++v) {
            for (int i : positive[v]) {
                Bits support = possible[v] & selections[i];
                int u = support._Find_first();
                int a = root[v], b = root[u];
                if ((support & ~members[b]).any()) continue;

                int key = min(a, b) * n + max(a, b);
                auto [position, inserted] = byPair.emplace(key, required.size());
                if (inserted) {
                    if (!contracted.join(a, b)) exit(0);
                    required.push_back({v, support});
                } else {
                    auto& entry = required[position->second];
                    if (v == entry.vertex) {
                        entry.neighbors &= support;
                    } else {
                        if (!entry.neighbors[v] || !support[entry.vertex]) exit(0);
                        entry.neighbors = singleton(v);
                    }
                    if (entry.neighbors.none()) exit(0);
                }
            }
        }
        if (required.empty()) return false;

        // Required connections also rule out chords within their forest.
        vector<Bits> linked(n), allowed(n);
        for (int v = 0; v < n; ++v) linked[contracted.find(root[v])].set(v);
        for (const auto& entry : required) {
            int v = entry.vertex;
            allowed[v] |= entry.neighbors;
            for (int u = entry.neighbors._Find_first(); u < n; u = entry.neighbors._Find_next(u))
                allowed[u].set(v);
        }
        bool changed = false;
        for (int v = 0; v < n; ++v) {
            Bits next = possible[v] & (~linked[contracted.find(root[v])] | allowed[v]);
            if (next != possible[v]) {
                possible[v] = next;
                changed = true;
            }
        }
        return changed;
    }

    void update(const Bits& selected, const Bits& on) {
        const long long previous = remaining;
        const int knownBefore = edges.size();
        int q = selections.size();
        selections.push_back(selected);
        lamps.push_back(on);
        // An unlit vertex has no selected neighbor. Apply both orientations.
        for (int v = 0; v < n; ++v) {
            if (on[v]) positive[v].push_back(q);
            else possible[v] &= ~selected;
            if (selected[v]) possible[v] &= on;
        }

        while (!done()) {
            size_t before = edges.size();
            for (int v = 0; v < n; ++v) {
                auto& tests = positive[v];
                size_t keep = 0;
                for (int i : tests) {
                    if ((known[v] & selections[i]).any()) continue;
                    Bits support = possible[v] & selections[i];
                    int u = support._Find_first();
                    if (u >= n) exit(0);
                    if (support._Find_next(u) >= size_t(n)) force(v, u);
                    else tests[keep++] = i;
                }
                tests.resize(keep);
                if (degree[v] == 0) {
                    int u = possible[v]._Find_first();
                    if (u >= n) exit(0);
                    if (possible[v]._Find_next(u) >= size_t(n)) force(v, u);
                }
            }
            if (before != edges.size()) {
                removeCycles();
                continue;
            }
            long long count = 0;
            for (const Bits& row : possible) count += row.count();
            remaining = count / 2;
            if (remaining < 30LL * n && restrictRequiredConnections()) continue;
            if (remaining < 20LL * n && forceBridges()) {
                removeCycles();
                continue;
            }
            break;
        }
        stalled = remaining == previous && int(edges.size()) == knownBefore;
    }

    Bits singleton(int v) const {
        Bits query;
        query.set(v);
        return query;
    }

    Bits randomQuery(double probability) {
        Bits result;
        uint32_t cutoff = probability * double(mt19937::max());
        for (int v = 0; v < n; ++v)
            if (possible[v].any() && rng() <= cutoff) result.set(v);
        if (result.none()) {
            for (int v = 0; v < n; ++v)
                if (possible[v].any()) return singleton(v);
        }
        return result;
    }

    bool canLearn(const Bits& query) const {
        for (int v = 0; v < n; ++v) {
            if ((known[v] & query).any() || (possible[v] & query).none()) continue;
            bool explained = false;
            for (int i : positive[v]) {
                if ((possible[v] & selections[i] & ~query).none()) {
                    explained = true;
                    break;
                }
            }
            if (!explained) return true;
        }
        return false;
    }

    // The score estimates how informative one lamp will be for a selection.
    // These estimates are used only for choosing queries.
    struct TestRow {
        int count = 0;
        int selected = 0;
        int blocked = 0;
        array<int, 3> uncovered{};
        array<int, 3> clauseSizes{};
        int clauses = 0;
        double selectedRate = 0;
        double totalRate = 0;
        double score = 0;
        double weight = 1;
        bool required = false;

        double value() const {
            if (blocked || !selected) return 0;
            if (selected == 1) return weight * 1.32;
            for (int c = 0; c < clauses; ++c)
                if (!uncovered[c]) return 0;
            if (required && selected == count) return 0;
            double p0 = exp(-selectedRate);
            if (required) {
                double empty = exp(-totalRate);
                p0 = max(0.0, (p0 - empty) / (1.0 - empty));
            }
            return weight * (0.25 * selected * p0 + 3.0 * p0 * (1 - p0) * sqrt(count));
        }
    };

    struct Effect {
        int row;
        unsigned int clauses;
        double rate;
    };

    Bits adaptiveQuery(const Bits* allowed = nullptr) {
        vector<TestRow> rows(n);
        vector<vector<Effect>> effects(n);
        vector<vector<int>> blockers(n);
        vector<int> active;
        for (int v = 0; v < n; ++v) {
            Bits local = possible[v];
            if (allowed) local &= *allowed;
            if (local.none()) continue;
            TestRow& row = rows[v];
            row.count = local.count();
            row.weight = pow(row.count, -0.31);
            row.required = degree[v] == 0 && local == possible[v];
            vector<Bits> clauses;
            vector<pair<int, int>> order;
            for (int i : positive[v]) {
                Bits support = possible[v] & selections[i];
                if (allowed && (support & ~*allowed).any()) continue;
                order.emplace_back(support.count(), i);
            }
            sort(order.begin(), order.end());
            for (auto [size, i] : order) {
                Bits clause = possible[v] & selections[i];
                bool redundant = false;
                for (const Bits& smaller : clauses)
                    if ((smaller & ~clause).none()) {
                        redundant = true;
                        break;
                    }
                if (!redundant) clauses.push_back(clause);
                if (clauses.size() == 28) break;
            }
            row.required |= !clauses.empty();
            row.clauses = min<int>(3, clauses.size());
            vector<double> clauseWeight;
            for (int j = 0; j < int(clauses.size()); ++j) {
                int size = clauses[j].count();
                if (j < row.clauses) row.clauseSizes[j] = size;
                clauseWeight.push_back(1.15 / sqrt(size));
            }

            vector<int> neighbors;
            vector<uint32_t> masks;
            vector<double> raw;
            for (int u = local._Find_first(); u < n; u = local._Find_next(u)) {
                neighbors.push_back(u);
                uint32_t mask = 0;
                double evidence = 0;
                for (int j = 0; j < int(clauses.size()); ++j)
                    if (clauses[j][u]) {
                        mask |= uint32_t(1) << j;
                        evidence += clauseWeight[j];
                    }
                masks.push_back(mask);
                raw.push_back(1 + evidence);
            }

            int lower = 0;
            Bits packed;
            for (const Bits& clause : clauses) {
                if ((packed & clause).none()) {
                    packed |= clause;
                    ++lower;
                }
            }

            lower = max(lower, int(row.required));

            double rate = degree[v] ? 0.48 : 1.55;
            if (row.required) rate = degree[v] ? 1.12 : 1.62;
            if (lower >= 2) rate = lower + 0.45;
            if (!allowed && degree[v] == 0 && !selections.empty()) {
                int zero = 0;
                int rounds = min<int>(7, lamps.size());
                for (int j = 0; j < rounds; ++j) zero += !lamps[j][v];
                double earlyRate = -log((zero + 0.55) / (rounds + 1.35)) / 0.49;
                rate = max(rate, min(8.0, 0.78 * earlyRate));
            }

            // Give extra weight to candidates in consistent one- or two-edge neighborhoods.
            double rawSum = accumulate(raw.begin(), raw.end(), 0.0);
            uint32_t full = (uint32_t(1) << clauses.size()) - 1;
            if (row.required && row.count <= 120 && lower <= 2) {
                vector<int> partners(row.count);
                int pairs = 0, singles = 0;
                for (int i = 0; i < row.count; ++i) {
                    singles += masks[i] == full;
                    for (int j = 0; j < i; ++j) {
                        if ((masks[i] | masks[j]) != full) continue;
                        ++partners[i];
                        ++partners[j];
                        ++pairs;
                    }
                }
                if (singles || pairs) {
                    double odds = 1.5 / n;
                    double base = singles + 2 * pairs * odds;
                    rate = max(rate * 0.65, (base / (singles + pairs * odds)) + 0.07);
                    for (int i = 0; i < row.count; ++i) {
                        double posterior = (double(masks[i] == full) + odds * partners[i]) / base;
                        raw[i] = 0.82 * posterior + 0.18 * raw[i] / rawSum;
                    }
                    rawSum = 1;
                }
            }

            for (int i = 0; i < row.count; ++i) {
                double p = min(0.94, rate * raw[i] / rawSum);
                double logRate = -log1p(-p);
                row.totalRate += logRate;
                effects[neighbors[i]].push_back({v, masks[i] & 7U, logRate});
            }
            for (int u = known[v]._Find_first(); u < n; u = known[v]._Find_next(u))
                blockers[u].push_back(v);
        }

        for (int u = 0; u < n; ++u)
            if (!effects[u].empty() && (!allowed || (*allowed)[u])) active.push_back(u);

        vector<TestRow> work;
        auto change = [&](int u, int sign) {
            double delta = 0;
            for (const Effect& e : effects[u]) {
                TestRow& row = work[e.row];
                double old = row.score;
                row.selected += sign;
                row.selectedRate += sign * e.rate;
                unsigned int mask = e.clauses;
                while (mask) {
                    int c = countr_zero(mask);
                    row.uncovered[c] -= sign;
                    mask &= mask - 1;
                }
                row.score = row.value();
                delta += row.score - old;
            }
            for (int v : blockers[u]) {
                TestRow& row = work[v];
                double old = row.score;
                row.blocked += sign;
                row.score = row.value();
                delta += row.score - old;
            }
            return delta;
        };

        Bits best;
        double bestValue = -1;
        auto initialize = [&] {
            work = rows;
            for (auto& row : work) row.uncovered = row.clauseSizes;
        };
        constexpr array densities{0.22, 0.38, 0.53, 0.32, 0.44};
        int attempts = remaining < 4LL * n ? 5 : 3;
        for (int attempt = 0; attempt < attempts; ++attempt) {
            initialize();
            Bits query = randomQuery(densities[attempt]);
            if (allowed) query &= *allowed;
            double value = 0;
            for (int u = query._Find_first(); u < n; u = query._Find_next(u)) value += change(u, 1);
            int passes = remaining < 8LL * n ? 3 : 1;
            for (int pass = 0; pass < passes; ++pass) {
                bool moved = false;
                shuffle(active.begin(), active.end(), rng);
                for (int u : active) {
                    int sign = query[u] ? -1 : 1;
                    double gain = change(u, sign);
                    if (gain > 1e-8) {
                        query.flip(u);
                        value += gain;
                        moved = true;
                    } else {
                        change(u, -sign);
                    }
                }
                if (!moved) break;
            }
            if (value > bestValue && canLearn(query)) {
                bestValue = value;
                best = query;
            }
        }
        return best;
    }

    vector<Bits> makeCode(const Bits& centers, bool core = false) {
        // Equal-weight signatures distinguish individual sources from their unions.
        int rounds = 3;
        int weight = 1;
        int count = centers.count();
        auto choose = [](int r, int w) {
            int c = 1;
            for (int j = 1; j <= w; ++j) c = c * (r - j + 1) / j;
            return c;
        };
        while (choose(rounds, weight) < count) {
            ++rounds;
            weight = max(1, (rounds + int(core)) / 3);
        }
        vector<int> patterns;
        for (unsigned int mask = 0; mask < (1U << rounds); ++mask)
            if (popcount(mask) == weight) patterns.push_back(mask);
        shuffle(patterns.begin(), patterns.end(), rng);
        vector<Bits> queries(rounds);
        int label = 0;
        for (int v = centers._Find_first(); v < n; v = centers._Find_next(v), ++label)
            for (int i = 0; i < rounds; ++i)
                if (patterns[label] >> i & 1) queries[i].set(v);
        return queries;
    }

    Bits hot;
    vector<Bits> hotCode, codes;
    size_t hotIndex = 0, codeIndex = 0;
    int startHot = -1;
    int complementRound = -1, complementOf = -1;
    bool triedHot = false;
    int outsideProbe = -1;
    Bits satelliteHubs;

    bool chooseCoreQuery(int round, Bits& query) {
        // Lamps lit by both halves of a partition have at least two neighbors.
        if (n < 18) return false;
        if (!triedHot && round >= 1 && round <= 5) {
            if (lamps.back().count() <= size_t(max(2, n / 20))) {
                hot = lamps.back();
            } else if (complementRound == round - 1) {
                Bits common = lamps[complementOf] & lamps[complementRound];
                if (common.count() <= size_t(max(3, n / 7))) hot = common;
            }
            if (hot.any()) {
                triedHot = true;
                startHot = round;
                if (hot.count() > 4) hotCode = makeCode(hot, true);
            } else if (complementRound == -1 && (round <= 2 || (round <= 4 && n >= 160))) {
                int test = round - 1;
                double p = double(selections[test].count()) / n;
                double lit = double(lamps[test].count()) / n;
                bool concentrated = (lit > 0.260 && abs(lit - p) > 0.009 &&
                                     lit < 2 * p / (1 + p) - 0.045) || lit > 0.89;
                if (round == 2) {
                    int difference = (lamps[0] ^ lamps[1]).count();
                    concentrated |= difference * 5 < n || difference * 3 > 2 * n;
                }
                if (round >= 3) {
                    double total = 0;
                    int comparable = 0;
                    for (int i = 0; i < round; ++i) {
                        double fraction = double(lamps[i].count()) / n;
                        double selected = double(selections[i].count()) / n;
                        total += fraction - 2 * selected / (1 + selected);
                        comparable += fraction > 0.255 && fraction < 0.55 && abs(fraction - selected) > 0.008;
                    }
                    concentrated = comparable == round && total < -0.052 * round;
                }
                if (concentrated) {
                    for (int v = 0; v < n; ++v)
                        if (!selections[test][v]) query.set(v);
                    if (canLearn(query)) {
                        complementRound = round;
                        complementOf = test;
                        return true;
                    }
                    query.reset();
                }
            }
        }

        if (hot.none()) return false;
        if (hot.count() <= 4) {
            for (int u = hot._Find_first(); u < n; u = hot._Find_next(u)) {
                if (possible[u].none()) continue;
                query = singleton(u);
                return true;
            }
        } else {
            while (hotIndex < hotCode.size()) {
                query = hotCode[hotIndex++];
                for (int u = query._Find_first(); u < n; u = query._Find_next(u))
                    if (possible[u].none()) query.reset(u);
                if (canLearn(query)) return true;
            }
            if (outsideProbe == -1) {
                // A small group still detached from the core can expose a missed hub.
                outsideProbe = 0;
                Bits roots;
                for (int u = hot._Find_first(); u < n; u = hot._Find_next(u))
                    roots.set(components.find(u));
                Bits outside;
                for (int v = 0; v < n; ++v)
                    if (!roots[components.find(v)]) outside.set(v);
                if (outside.count() >= 4 && outside.count() * 5 <= size_t(n) && canLearn(outside)) {
                    outsideProbe = round;
                    query = outside;
                    return true;
                }
            } else if (outsideProbe == round - 1 && outsideProbe > 0) {
                if (lamps.back().count() <= 5) satelliteHubs = lamps.back();
                outsideProbe = 0;
            }
            while (satelliteHubs.any()) {
                int u = satelliteHubs._Find_first();
                satelliteHubs.reset(u);
                if (possible[u].any()) {
                    query = singleton(u);
                    return true;
                }
            }
            int hotLinks = 0;
            for (int u = hot._Find_first(); u < n; u = hot._Find_next(u))
                hotLinks += (possible[u] & hot).count();
            if (hotLinks > 0 && !stalled && round < startHot + int(hotCode.size()) + 5) {
                query = adaptiveQuery(&hot);
                if (query.any()) return true;
            }
        }
        hot.reset();
        query.reset();
        return false;
    }

    Bits next() {
        int pivot = -1;
        int bestCount = 0;
        for (int v = 0; v < n; ++v) {
            int count = possible[v].count();
            if (count > bestCount) {
                bestCount = count;
                pivot = v;
            }
        }
        if (pivot == -1) exit(0);
        int round = selections.size();
        if (n >= 14 && codes.empty()) {
            Bits all;
            for (int v = 0; v < n; ++v) all.set(v);
            codes = makeCode(all);
        }
        Bits query;
        if (chooseCoreQuery(round, query)) return query;
        // A singleton determines every still-possible edge incident to its center.
        if (stalled || round >= 48) return singleton(pivot);
        if (round < 9 && remaining > n * 13LL) {
            while (codeIndex < codes.size()) {
                query = codes[codeIndex++];
                for (int u = query._Find_first(); u < n; u = query._Find_next(u))
                    if (possible[u].none()) query.reset(u);
                if (canLearn(query)) return query;
            }
        }
        query = adaptiveQuery();
        return query.any() ? query : singleton(pivot);
    }

};

using EdgeMask = __uint128_t;

int edgeCount(EdgeMask mask) {
    return popcount(uint64_t(mask)) + popcount(uint64_t(mask >> 64));
}

int firstEdge(EdgeMask mask) {
    uint64_t low = mask;
    return low ? countr_zero(low) : 64 + countr_zero(uint64_t(mask >> 64));
}

struct Probe {
    Bits chosen, extended;
};

struct ExactTrees {
    const vector<Bits>& known;
    vector<Edge> options;
    vector<EdgeMask> trees;
    vector<pair<int, int>> ends;
    vector<EdgeMask> clauses;
    int rootCount = 0, visits = 0;
    bool complete = true;

    explicit ExactTrees(const Reconstruction& solve) : known(solve.known) {
        int n = solve.n;
        auto components = solve.components;
        vector<int> roots(n, -1), component(n);
        for (int v = 0; v < n; ++v) {
            int root = components.find(v);
            if (roots[root] == -1) roots[root] = rootCount++;
            component[v] = roots[root];
        }
        vector<vector<pair<int, int>>> edgeIds(n);
        for (int v = 0; v < n; ++v) {
            for (int u = solve.possible[v]._Find_next(v); u < n; u = solve.possible[v]._Find_next(u)) {
                int id = options.size();
                options.emplace_back(v, u);
                ends.emplace_back(component[v], component[u]);
                edgeIds[v].emplace_back(u, id);
                edgeIds[u].emplace_back(v, id);
            }
        }
        if (options.size() > EXACT_EDGE_LIMIT || rootCount > 128) {
            complete = false;
            return;
        }
        for (int v = 0; v < n; ++v) {
            for (int tested : solve.positive[v]) {
                EdgeMask clause = 0;
                for (auto [u, id] : edgeIds[v])
                    if (solve.selections[tested][u]) clause |= EdgeMask(1) << id;
                clauses.push_back(clause);
            }
        }
        sort(clauses.begin(), clauses.end(), [](EdgeMask a, EdgeMask b) {
            int ka = edgeCount(a), kb = edgeCount(b);
            return ka == kb ? a < b : ka < kb;
        });
        vector<EdgeMask> minimal;
        for (EdgeMask clause : clauses) {
            bool redundant = false;
            for (EdgeMask old : minimal)
                if ((clause & old) == old) {
                    redundant = true;
                    break;
                }
            if (!redundant) minimal.push_back(clause);
        }
        clauses.swap(minimal);

        array<int, 128> partition{};
        iota(partition.begin(), partition.end(), 0);
        enumerate((EdgeMask(1) << options.size()) - 1, 0, partition, rootCount);
    }

    void enumerate(EdgeMask available, EdgeMask chosen, const array<int, 128>& partition, int groups) {
        if (!complete) return;
        if (++visits > 16000 || trees.size() >= 256) {
            complete = false;
            return;
        }
        if (edgeCount(available) < groups - 1) return;
        if (groups == 1) {
            for (EdgeMask clause : clauses)
                if (!(clause & chosen)) return;
            trees.push_back(chosen);
            return;
        }

        // Every completion meets each unsatisfied lamp constraint and each cut.
        // Branch on one of them, using disjoint branches for its first chosen edge.
        EdgeMask split = available;
        int smallest = edgeCount(split);
        for (EdgeMask clause : clauses) {
            if (clause & chosen) continue;
            EdgeMask support = clause & available;
            int size = edgeCount(support);
            if (size == 0) return;
            if (size < smallest) {
                split = support;
                smallest = size;
            }
        }
        array<EdgeMask, 128> out{};
        for (EdgeMask scan = available; scan; scan &= scan - 1) {
            int i = firstEdge(scan);
            auto [a, b] = ends[i];
            EdgeMask bit = EdgeMask(1) << i;
            out[partition[a]] |= bit;
            out[partition[b]] |= bit;
        }
        for (int v = 0; v < rootCount; ++v) {
            if (partition[v] != v) continue;
            int size = edgeCount(out[v]);
            if (size == 0) return;
            if (size < smallest) {
                split = out[v];
                smallest = size;
            }
        }

        while (split && complete) {
            int i = firstEdge(split);
            EdgeMask bit = EdgeMask(1) << i;
            split ^= bit;
            available ^= bit;
            EdgeMask next = available;
            auto [a, b] = ends[i];
            a = partition[a];
            b = partition[b];
            for (EdgeMask scan = out[a] & out[b] & next; scan; scan &= scan - 1)
                next ^= EdgeMask(1) << firstEdge(scan);
            auto merged = partition;
            for (int v = 0; v < rootCount; ++v)
                if (merged[v] == b) merged[v] = a;
            enumerate(next, chosen | bit, merged, groups - 1);
        }
    }

    Bits baseResponse(const Probe& query) const {
        Bits result;
        for (int u = query.chosen._Find_first(); u < int(known.size()); u = query.chosen._Find_next(u)) {
            result |= known[u];
            if (!query.extended[u]) continue;
            Bits second;
            for (int v = known[u]._Find_first(); v < int(known.size()); v = known[u]._Find_next(v))
                second |= known[v];
            second.reset(u);
            result |= second;
        }
        return result;
    }

    Bits response(const Probe& query, const Bits& base, EdgeMask tree) const {
        Bits result = base;
        for (EdgeMask scan = tree; scan; scan &= scan - 1) {
            auto [a, b] = options[firstEdge(scan)];
            if (query.chosen[a]) result.set(b);
            if (query.chosen[b]) result.set(a);
        }
        for (int u = query.extended._Find_first(); u < int(known.size()); u = query.extended._Find_next(u)) {
            Bits first = known[u], second;
            for (EdgeMask scan = tree; scan; scan &= scan - 1) {
                auto [a, b] = options[firstEdge(scan)];
                if (a != u && b != u) continue;
                int v = a == u ? b : a;
                first.set(v);
                second |= known[v];
            }
            for (EdgeMask scan = tree; scan; scan &= scan - 1) {
                auto [a, b] = options[firstEdge(scan)];
                if (first[a]) second.set(b);
                if (first[b]) second.set(a);
            }
            second.reset(u);
            result |= second;
        }
        return result;
    }

    Probe chooseQuery(Reconstruction& solve) const {
        EdgeMask all = 0, common = ~EdgeMask(0);
        for (EdgeMask tree : trees) {
            all |= tree;
            common &= tree;
        }
        vector<int> active;
        Bits useful;
        for (EdgeMask varying = all ^ common; varying; varying &= varying - 1) {
            auto [a, b] = options[firstEdge(varying)];
            useful.set(a);
            useful.set(b);
        }
        for (int v = useful._Find_first(); v < solve.n; v = useful._Find_next(v)) active.push_back(v);

        Probe best;
        int bestScore = int(trees.size() * trees.size());
        int bestLargest = trees.size();
        auto consider = [&](const Bits& chosen, const Bits& extended = {}) {
            if (chosen.none()) return false;
            Probe query{chosen, extended};
            Bits base = baseResponse(query);
            unordered_map<Bits, int> groups;
            int score = 0, largest = 0;
            for (EdgeMask tree : trees) {
                int& size = groups[response(query, base, tree)];
                score += 2 * size + 1;
                largest = max(largest, ++size);
            }
            if (pair(score, largest) < pair(bestScore, bestLargest)) {
                bestScore = score;
                bestLargest = largest;
                best = query;
            }
            return bestScore == int(trees.size());
        };

        for (int v : active)
            if (consider(solve.singleton(v))) return best;
        if (active.size() <= 9) {
            uint32_t limit = uint32_t(1) << active.size();
            for (uint32_t mask = 1; mask < limit; ++mask) {
                if (has_single_bit(mask)) continue;
                Bits query;
                for (uint32_t scan = mask; scan; scan &= scan - 1)
                    query.set(active[countr_zero(scan)]);
                if (consider(query)) return best;
            }
        } else {
            constexpr array densities{0.22, 0.37, 0.51, 0.64};
            for (int trial = 0; trial < 150; ++trial) {
                Bits query;
                uint32_t cutoff = densities[trial % densities.size()] * double(mt19937::max());
                for (int v : active)
                    if (solve.rng() <= cutoff) query.set(v);
                if (consider(query)) return best;
            }
            for (int pass = 0; pass < 3; ++pass) {
                shuffle(active.begin(), active.end(), solve.rng);
                for (int v : active) {
                    Bits query = best.chosen;
                    query.flip(v);
                    if (consider(query)) return best;
                }
            }
        }
        if (trees.size() <= 96) {
            for (int u : active) {
                Bits selected = solve.singleton(u);
                if (consider(selected, selected)) return best;
            }
            for (int trial = 0; trial < 72; ++trial) {
                Bits chosen, extended;
                for (int u : active) {
                    unsigned r = solve.rng() % 12;
                    if (r < 5) chosen.set(u);
                    if (r == 0 || (r == 1 && trial >= 36)) extended.set(u);
                }
                if (extended.any() && consider(chosen, extended)) return best;
            }
        }
        return best;
    }
};

Bits readLamps(int n) {
    string reply;
    if (!(cin >> reply) || reply.size() != size_t(n)) exit(0);
    Bits on;
    for (int v = 0; v < n; ++v) {
        if (reply[v] == '1') on.set(v);
        else if (reply[v] != '0') exit(0);
    }
    return on;
}

Bits ask(int n, const Bits& query) {
    cout << '?';
    for (int v = 0; v < n; ++v) cout << ' ' << query[v];
    cout << endl;
    return readLamps(n);
}

Bits ask(const vector<int>& query) {
    cout << '?';
    for (int radius : query) cout << ' ' << radius;
    cout << endl;
    return readLamps(query.size());
}

void report(const vector<Edge>& edges) {
    cout << '!' << endl;
    for (auto [a, b] : edges) cout << a + 1 << ' ' << b + 1 << endl;
}

void solveLate(Reconstruction& solve, ExactTrees exact) {
    while (exact.trees.size() > 1) {
        Probe query = exact.chooseQuery(solve);
        Bits base = exact.baseResponse(query);
        vector<int> radii(solve.n);
        for (int v = query.chosen._Find_first(); v < solve.n; v = query.chosen._Find_next(v))
            radii[v] = query.extended[v] ? 2 : 1;
        Bits on = ask(radii);
        erase_if(exact.trees, [&](EdgeMask tree) {
            return exact.response(query, base, tree) != on;
        });
    }
    if (exact.trees.empty()) exit(0);
    for (EdgeMask tree = exact.trees[0]; tree; tree &= tree - 1)
        solve.edges.push_back(exact.options[firstEdge(tree)]);
}

struct SmallTrees {
    int n;
    vector<vector<Edge>> trees;
    vector<vector<int>> queries;
    vector<vector<unsigned char>> replies;

    explicit SmallTrees(int n) : n(n) {
        // Prüfer sequences enumerate every labeled tree for these small sizes.
        int count = 1;
        for (int i = 0; i < n - 2; ++i) count *= n;

        using Reach = array<array<unsigned char, 7>, 7>;
        vector<Reach> reach(count);
        for (int id = 0; id < count; ++id) {
            vector<int> code(n - 2), degree(n, 1);
            for (int rest = id, i = 0; i < n - 2; ++i, rest /= n) {
                code[i] = rest % n;
                ++degree[code[i]];
            }

            vector<Edge> tree;
            for (int v : code) {
                int leaf = find(degree.begin(), degree.end(), 1) - degree.begin();
                tree.emplace_back(leaf, v);
                --degree[leaf];
                --degree[v];
            }
            int a = find(degree.begin(), degree.end(), 1) - degree.begin();
            int b = find(degree.begin() + a + 1, degree.end(), 1) - degree.begin();
            tree.emplace_back(a, b);

            array<array<int, 7>, 7> distance{};
            for (int u = 0; u < n; ++u)
                for (int v = 0; v < n; ++v)
                    distance[u][v] = u == v ? 0 : n;
            for (auto [u, v] : tree) distance[u][v] = distance[v][u] = 1;
            for (int w = 0; w < n; ++w)
                for (int u = 0; u < n; ++u)
                    for (int v = 0; v < n; ++v)
                        distance[u][v] = min(distance[u][v], distance[u][w] + distance[w][v]);
            for (int u = 0; u < n; ++u)
                for (int v = 0; v < n; ++v)
                    if (u != v)
                        for (int r = distance[u][v]; r < n; ++r) reach[id][r][u] |= 1U << v;
            trees.push_back(move(tree));
        }

        auto addQuery = [&](const vector<int>& radius) {
            queries.push_back(radius);
            vector<pair<int, int>> centers;
            for (int v = 0; v < n; ++v)
                if (radius[v]) centers.emplace_back(v, radius[v]);
            vector<unsigned char> on(count);
            for (int id = 0; id < count; ++id)
                for (auto [v, r] : centers) on[id] |= reach[id][r][v];
            replies.push_back(move(on));
        };

        int radix = n <= 5 ? n : (n == 6 ? 3 : 2);
        int limit = 1;
        for (int v = 0; v < n; ++v) limit *= radix;
        for (int code = 1; code < limit; ++code) {
            vector<int> radius(n);
            for (int rest = code, v = 0; v < n; ++v, rest /= radix)
                radius[v] = rest % radix;
            addQuery(radius);
        }
        if (n == 7) {
            for (int a = 0; a < n; ++a) {
                vector<int> radius(n);
                radius[a] = 2;
                addQuery(radius);
                for (int b = 0; b < n; ++b) {
                    if (a == b) continue;
                    radius[b] = 1;
                    addQuery(radius);
                    radius[b] = 0;
                }
            }
        }
    }

    bool separable(const vector<int>& pool) const {
        if (pool.size() <= 2) return true;
        for (const auto& result : replies) {
            bitset<128> seen;
            bool unique = true;
            for (int tree : pool) {
                int mask = result[tree];
                if (seen[mask]) {
                    unique = false;
                    break;
                }
                seen.set(mask);
            }
            if (unique) return true;
        }
        return false;
    }

    int chooseQuery(const vector<int>& pool) const {
        if (n == 5 && pool.size() == trees.size()) {
            vector<int> radius{0, 0, 0, 1, 1};
            return find(queries.begin(), queries.end(), radius) - queries.begin();
        }

        int bestQuery = -1;
        int bestScore = pool.size() * pool.size(), bestLargest = pool.size();
        vector<pair<int, int>> byUnresolved;
        for (int i = 0; i < int(queries.size()); ++i) {
            array<int, 128> groups{};
            int score = 0, largest = 0;
            for (int id : pool) {
                int& size = groups[replies[i][id]];
                score += 2 * size + 1;
                largest = max(largest, ++size);
            }
            if (score == int(pool.size())) return i;
            if (pair(score, largest) < pair(bestScore, bestLargest)) {
                bestScore = score;
                bestLargest = largest;
                bestQuery = i;
            }
            if (pool.size() <= 24 && largest < int(pool.size())) {
                int unresolved = 0;
                for (int size : groups)
                    if (size > 1) unresolved += size;
                byUnresolved.emplace_back(unresolved, i);
            }
        }

        sort(byUnresolved.begin(), byUnresolved.end());
        for (auto [unresolved, i] : byUnresolved) {
            array<vector<int>, 128> groups;
            for (int id : pool) groups[replies[i][id]].push_back(id);
            if (all_of(groups.begin(), groups.end(), [&](const auto& group) {
                return separable(group);
            })) return i;
        }
        return bestQuery;
    }

    vector<Edge> run() const {
        vector<int> pool(trees.size());
        iota(pool.begin(), pool.end(), 0);
        while (pool.size() > 1) {
            int query = chooseQuery(pool);
            if (query < 0) exit(0);
            Bits on = ask(queries[query]);
            unsigned int mask = 0;
            for (int v = 0; v < n; ++v) if (on[v]) mask |= 1U << v;
            erase_if(pool, [&](int id) { return replies[query][id] != mask; });
        }
        if (pool.empty()) exit(0);
        return trees[pool[0]];
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2 || n > MAX_N) return 0;
    if (n <= 7) {
        report(SmallTrees(n).run());
        return 0;
    }
    Reconstruction solve(n, random_device{}());
    while (!solve.done()) {
        if (solve.remaining <= EXACT_EDGE_LIMIT) {
            ExactTrees exact(solve);
            if (exact.complete) {
                solveLate(solve, move(exact));
                break;
            }
        }
        Bits query = solve.next();
        solve.update(query, ask(n, query));
    }
    report(solve.edges);
    return 0;
}

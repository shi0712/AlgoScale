#include <bits/stdc++.h>
using namespace std;

enum class Query { Universal, Leaf, Maximum };

struct Reply {
    int vertex, missing;
};

struct Observation {
    Query kind;
    Reply reply;
};

struct LowVertex {
    int vertex, missing, attached = 0;
    bool inCore = false;
};

struct Separator {
    int forbidden, index;
};

// A maximum-query vertex can join two currently active labels smaller
// than its witness. The tree tracks how many such vertices can be used
// in one chain, placing increasingly restrictive deadlines first.
class ChainCapacity {
    struct Node {
        int active = 0, balance = 0, peak = 0;
    };

    int first = 1;
    vector<Node> tree;

    static Node join(Node left, Node right) {
        return {left.active + right.active, left.balance + right.balance,
                max(left.peak, left.balance + right.peak)};
    }

    void refresh(int index) {
        while (index > 1) {
            index /= 2;
            tree[index] = join(tree[2 * index], tree[2 * index + 1]);
        }
    }

public:
    explicit ChainCapacity(int n) {
        while (first < n)
            first *= 2;
        tree.resize(2 * first);
        for (int i = 0; i < n; ++i)
            tree[first + i] = {1, -1, 0};
        for (int index = first - 1; index >= 1; --index)
            tree[index] = join(tree[2 * index], tree[2 * index + 1]);
    }

    void remove(int label) {
        int index = first + label - 1;
        --tree[index].active;
        ++tree[index].balance;
        refresh(index);
    }

    void addDeadline(int witness) {
        int index = first + witness - 1;
        ++tree[index].balance;
        ++tree[index].peak;
        refresh(index);
    }

    int available(int secondSmallest) const {
        // Deadlines up to this label cannot join even the first pair.
        // One slot is available at the start of the remaining suffix.
        Node suffix;
        for (int left = first + secondSmallest, right = 2 * first;
             left < right; left /= 2, right /= 2) {
            if (left & 1)
                suffix = join(suffix, tree[left++]);
        }
        return suffix.balance + suffix.active - max(0, suffix.peak - 1);
    }
};

// Solve the last few vertices by keeping every complement consistent with
// the information used at entry. Masks refer to the vertices at entry.
class SmallSolver {
    using Mask = uint16_t;
    static constexpr int infinity = 100;

    struct Score {
        int worst, total, cap;
    };

    int size, edgeCount = 0;
    Mask edge[6][6]{}, incidence[6]{}, inside[64]{};
    vector<int8_t> pathExists;
    int poolSize;
    array<int, 6> forbiddenCount;

    bool path(Mask missing, vector<int>* result = nullptr) {
        if (!result && pathExists[missing] != -1)
            return pathExists[missing];

        uint64_t failed[6][6][2]{};
        vector<int> order;
        auto extend = [&](auto&& self, int last, int remaining, int used,
                          bool previousNeedsSeparator) -> bool {
            order.push_back(last);
            if (remaining == 0)
                return true;
            auto& knownFailure = failed[last][used][previousNeedsSeparator];
            if (!(knownFailure & (1ULL << remaining))) {
                for (int next = remaining; next; next &= next - 1) {
                    int v = countr_zero(static_cast<unsigned>(next));
                    bool needsSeparator = (missing & edge[last][v]) != 0;
                    if (needsSeparator &&
                        (used == poolSize || poolSize - forbiddenCount[last] - forbiddenCount[v] < 1 ||
                         (previousNeedsSeparator && poolSize - forbiddenCount[last] < 2)))
                        continue;
                    if (self(self, v, remaining ^ (1 << v),
                             used + needsSeparator, needsSeparator))
                        return true;
                }
                knownFailure |= 1ULL << remaining;
            }
            order.pop_back();
            return false;
        };

        int all = (1 << size) - 1;
        for (int start = 0; start < size; ++start) {
            if (extend(extend, start, all ^ (1 << start), 0, false)) {
                pathExists[missing] = 1;
                if (result)
                    *result = order;
                return true;
            }
        }
        pathExists[missing] = 0;
        return false;
    }

    int response(Mask missing, int active, int cap) const {
        missing &= inside[active];
        int selected = size, bestDegree = -1;
        for (int i = 0; i < size; ++i) {
            if (!(active & (1 << i)))
                continue;
            int degree = popcount(static_cast<unsigned>(missing & incidence[i]));
            if (degree <= cap && degree > bestDegree) {
                selected = i;
                bestDegree = degree;
            }
        }

        int witness = size;
        if (bestDegree > 0) {
            witness = 0;
            while (!(missing & edge[selected][witness]))
                ++witness;
        }
        return selected * (size + 1) + witness;
    }

    Mask unionOf(const vector<Mask>& candidates) const {
        Mask missing = 0;
        for (Mask graph : candidates)
            missing |= graph;
        return missing;
    }

    Score solve(const vector<Mask>& candidates, int active) {
        if (path(unionOf(candidates)))
            return {0, 0, -1};

        Score best{infinity, 0, -1};
        int count = popcount(static_cast<unsigned>(active));
        for (int cap = count - 1; cap >= 0; --cap) {
            array<vector<Mask>, 49> partition;
            for (Mask graph : candidates)
                partition[response(graph, active, cap)].push_back(graph);

            int failure = size * (size + 1) + size;
            if (partition[failure].size() == candidates.size())
                continue;

            int worst = 1, total = static_cast<int>(candidates.size());
            for (int code = 0; code <= failure; ++code) {
                if (partition[code].empty())
                    continue;
                int v = code / (size + 1);
                int next = v == size ? active : active ^ (1 << v);
                Score child = solve(partition[code], next);
                worst = max(worst, child.worst + 1);
                total += child.total;
                if (worst > best.worst || (worst == best.worst && total >= best.total))
                    break;
            }
            if (worst < infinity && tie(worst, total) < tie(best.worst, best.total))
                best = {worst, total, cap};
        }
        return best;
    }

    // Test a degree against earlier tie breaks. Degrees above one can be
    // merged because the constraints here distinguish only zero and one.
    int possibleDegrees(int label, const vector<Observation>& history) const {
        int possible = 4;
        for (int start = 0; start <= 1; ++start) {
            int states = 1 << start;
            for (auto it = history.rbegin(); it != history.rend() && states; ++it) {
                auto [kind, reply] = *it;
                auto [v, w] = reply;
                int increased = ((states & 1) << 1) | ((states & 6) ? 4 : 0);
                if (w == label)
                    states = increased;
                else if (kind == Query::Maximum && w && label > w)
                    states |= increased;

                if (kind == Query::Universal && (!v || label < v))
                    states &= ~1;
                if (kind == Query::Leaf && (!w || label < v))
                    states &= ~2;
                if (kind == Query::Leaf && !w && label < v)
                    states &= ~1;
            }
            if (states)
                possible |= 1 << start;
        }
        return possible;
    }

    vector<int> withSeparators(const vector<int>& labels, const vector<int>& order,
                               Mask missing, const vector<Separator>& separators,
                               vector<LowVertex>& low) const {
        // Hall's condition was checked while finding the order. Two gaps
        // share an exclusion only when adjacent. Three share none.
        vector<int> assigned(size - 1, -1), owner(separators.size(), -1);
        auto assign = [&](auto&& self, int gap, vector<bool>& visited) -> bool {
            for (int i = 0; i < static_cast<int>(separators.size()); ++i) {
                int forbidden = separators[i].forbidden;
                if (visited[i] || forbidden == labels[order[gap]] ||
                    forbidden == labels[order[gap + 1]])
                    continue;
                visited[i] = true;
                if (owner[i] == -1 || self(self, owner[i], visited)) {
                    owner[i] = gap;
                    assigned[gap] = i;
                    return true;
                }
            }
            return false;
        };

        vector<int> result{labels[order.front()]};
        for (int gap = 0; gap + 1 < size; ++gap) {
            if (missing & edge[order[gap]][order[gap + 1]]) {
                vector<bool> visited(separators.size(), false);
                if (!assign(assign, gap, visited))
                    exit(0);
            }
        }
        for (int gap = 0; gap + 1 < size; ++gap) {
            if (assigned[gap] != -1) {
                auto& separator = low[separators[assigned[gap]].index];
                separator.inCore = true;
                result.push_back(separator.vertex);
            }
            result.push_back(labels[order[gap + 1]]);
        }
        return result;
    }

public:
    SmallSolver(int count, int poolSize, array<int, 6> forbiddenCount)
        : size(count), poolSize(poolSize), forbiddenCount(forbiddenCount) {
        for (int i = 0; i < size; ++i) {
            for (int j = i + 1; j < size; ++j) {
                edge[i][j] = edge[j][i] = 1 << edgeCount++;
                incidence[i] |= edge[i][j];
                incidence[j] |= edge[i][j];
            }
        }
        for (int active = 0; active < (1 << size); ++active)
            for (int i = 0; i < size; ++i)
                for (int j = i + 1; j < size; ++j)
                    if ((active & (1 << i)) && (active & (1 << j)))
                        inside[active] |= edge[i][j];

        pathExists.assign(1 << edgeCount, -1);
    }

    template<class Ask>
    vector<int> run(const vector<int>& labels, int lower, int upper,
                    const vector<Observation>& history, const vector<Separator>& separators,
                    vector<LowVertex>& low, Ask&& ask) {
        int allowed[6];
        for (int i = 0; i < size; ++i)
            allowed[i] = possibleDegrees(labels[i], history);

        vector<Mask> candidates;
        for (int graph = 0; graph < (1 << edgeCount); ++graph) {
            int count = popcount(static_cast<unsigned>(graph));
            if (count < lower || count > upper)
                continue;
            bool valid = true;
            for (int i = 0; i < size; ++i) {
                int degree = popcount(static_cast<unsigned>(graph & incidence[i]));
                valid &= (allowed[i] & (1 << min(2, degree))) != 0;
            }
            if (valid)
                candidates.push_back(graph);
        }

        int active = (1 << size) - 1;
        while (!candidates.empty()) {
            vector<int> order;
            Mask missing = unionOf(candidates);
            if (path(missing, &order))
                return withSeparators(labels, order, missing, separators, low);
            Score score = solve(candidates, active);
            if (score.cap < 0)
                exit(0);
            auto [vertex, witness] = ask(score.cap);
            auto index = [&](int label) {
                return label ? int(lower_bound(labels.begin(), labels.end(), label) - labels.begin())
                             : size;
            };
            int v = index(vertex), w = index(witness);
            int code = v * (size + 1) + w;
            erase_if(candidates, [&](Mask graph) {
                return response(graph, active, score.cap) != code;
            });
            if (vertex)
                active ^= 1 << v;
        }
        exit(0);
    }
};

class Solver {
    int n, lower, upper;
    int poolSize = 0, knownPositive = 0;
    int completionProbes = 0, leavesSinceMaximum = 0, shortLeafRuns = 0;
    int unusableMaxima = 0;
    bool mayProbeUniversal = true, preserveUniversals = false;
    set<int> active;
    vector<LowVertex> low;
    vector<vector<int>> byMissing;
    set<pair<int, int>> groups;
    vector<int> lastProbe;
    vector<Observation> history;
    ChainCapacity capacity;
    vector<pair<int, int>> deadlines;

    Reply ask(int cap) {
        int degree = static_cast<int>(active.size()) - 1 - cap;
        cout << "? " << degree << endl;
        int v, w;
        if (!(cin >> v) || v < 0 || !(cin >> w) || w < 0)
            exit(0);
        if (!v && !w)
            return {v, w};
        if (!active.contains(v) || w == v || (w && !active.contains(w)))
            exit(0);
        active.erase(v);
        capacity.remove(v);
        return {v, w};
    }

    void addToPool(int index) {
        int w = low[index].missing;
        auto& group = byMissing[w];
        if (!group.empty())
            groups.erase({int(group.size()), w});
        group.push_back(index);
        groups.emplace(group.size(), w);
        ++poolSize;
    }

    int putInPool(Reply reply) {
        int index = static_cast<int>(low.size());
        low.push_back({reply.vertex, reply.missing});
        addToPool(index);
        return index;
    }

    void pairWith(int index, int vertex) {
        auto& leaf = low[index];
        auto& group = byMissing[leaf.missing];
        groups.erase({int(group.size()), leaf.missing});
        group.pop_back();
        if (!group.empty())
            groups.emplace(group.size(), leaf.missing);
        leaf.attached = vertex;
        --poolSize;
    }

    bool canBridgeMaximum() const {
        if (groups.empty())
            return false;
        return groups.size() > 1 || !active.contains(groups.begin()->second);
    }

    bool canAlternate() const {
        return poolSize + 1 >= static_cast<int>(active.size()) && canBridgeMaximum();
    }

    void finish(vector<int> core) {
        deque<int> path(core.begin(), core.end());
        // Every exposed endpoint was still present when the next connector
        // was queried. That connector misses at most one of the endpoints.
        for (auto it = low.rbegin(); it != low.rend(); ++it) {
            if (it->inCore)
                continue;
            if (path.front() == it->missing) {
                path.push_back(it->vertex);
                if (it->attached)
                    path.push_back(it->attached);
            } else {
                path.push_front(it->vertex);
                if (it->attached)
                    path.push_front(it->attached);
            }
        }

        cout << '!';
        for (int v : path)
            cout << ' ' << v;
        cout << endl;
    }

    void finishClique() {
        finish(vector<int>(active.begin(), active.end()));
    }

    void finishAlternating(int useHigh = 0) {
        vector<int> vertices(active.begin(), active.end());
        vector<int> chain, itemFor(n + 1, 0);
        if (useHigh) {
            sort(deadlines.begin(), deadlines.end());
            chain.push_back(vertices.front());
            int used = 0;
            for (auto [witness, index] : deadlines) {
                if (vertices[used + 1] >= witness)
                    continue;
                chain.push_back(low[index].attached);
                chain.push_back(vertices[++used]);
                low[index].attached = 0;
                addToPool(index);
                if (used == useHigh)
                    break;
            }
            assert(used == useHigh);
            itemFor[chain.back()] = chain.front();
            vertices.erase(vertices.begin() + 1, vertices.begin() + 1 + used);
        }
        for (int v : vertices)
            itemFor[v] = v;

        int count = static_cast<int>(vertices.size());
        vector<pair<int, int>> separators;
        separators.reserve(count - 1);
        bool different = false;
        for (auto [size, witness] : groups) {
            int forbidden = itemFor[witness];
            for (int index : byMissing[witness]) {
                if (separators.size() < vertices.size() - 1) {
                    different |= !separators.empty() && separators.front().first != forbidden;
                    separators.emplace_back(forbidden, index);
                } else if (!different && forbidden != separators.front().first) {
                    separators.back() = {forbidden, index};
                    different = true;
                    break;
                }
            }
            if (different && separators.size() == vertices.size() - 1)
                break;
        }
        sort(separators.begin(), separators.end());

        auto allowed = [&](int slot, int labelIndex) {
            int label = vertices[labelIndex];
            return (slot == 0 || separators[slot - 1].first != label) &&
                   (slot == count - 1 || separators[slot].first != label);
        };

        // Each slot excludes at most two labels. Grouping separators makes
        // each pair of different exclusions occur together in at most one
        // slot, and every label still has an allowed slot. Hall's condition
        // therefore holds. Only the last two slots need augmenting paths.
        vector<int> assigned(count, -1), slotOf(count, -1), free(count);
        iota(free.begin(), free.end(), 0);
        for (int slot = 0; slot < count - 2; ++slot) {
            int position = 0;
            while (!allowed(slot, free[position]))
                ++position;
            int label = free[position];
            free[position] = free.back();
            free.pop_back();
            assigned[slot] = label;
            slotOf[label] = slot;
        }

        for (int start = count - 2; start < count; ++start) {
            vector<int> previous(count, -1), unseen(count + 1), pending{start};
            iota(unseen.begin(), unseen.end(), 1);
            // The sentinel at index count heads the linked list.
            unseen[count] = 0;
            int end = -1;
            for (size_t i = 0; i < pending.size() && end == -1; ++i) {
                int slot = pending[i], before = count;
                for (int label = unseen[before]; label < count; label = unseen[before]) {
                    if (!allowed(slot, label)) {
                        before = label;
                        continue;
                    }
                    unseen[before] = unseen[label];
                    previous[label] = slot;
                    if (slotOf[label] == -1) {
                        end = label;
                        break;
                    }
                    pending.push_back(slotOf[label]);
                }
            }
            assert(end != -1);
            for (int label = end; label != -1;) {
                int slot = previous[label];
                int displaced = assigned[slot];
                assigned[slot] = label;
                slotOf[label] = slot;
                label = displaced;
            }
        }

        vector<int> core;
        for (int slot = 0; slot < count; ++slot) {
            if (slot) {
                auto& separator = low[separators[slot - 1].second];
                separator.inCore = true;
                core.push_back(separator.vertex);
            }
            if (assigned[slot] == 0 && !chain.empty())
                core.insert(core.end(), chain.begin(), chain.end());
            else
                core.push_back(vertices[assigned[slot]]);
        }
        finish(move(core));
    }

    bool tryFinishWithSeparators() {
        if (canAlternate()) {
            finishAlternating();
            return true;
        }
        // Only the two endpoints of the chain can forbid its item.
        // Three distinct groups keep at least one separator available
        // for every item after those two exclusions are merged.
        int needed = (static_cast<int>(active.size()) - poolSize) / 2;
        if (needed > 0 && groups.size() >= 3 && needed <= static_cast<int>(deadlines.size()) &&
            capacity.available(*next(active.begin())) >= needed) {
            finishAlternating(needed);
            return true;
        }
        return false;
    }

    void maximum(bool noLeaves = false, int target = 0, int connector = -1) {
        leavesSinceMaximum = 0;
        int count = static_cast<int>(active.size());
        auto [v, w] = ask(count - 1);
        if (!v)
            exit(0);
        if (!w) {
            active.insert(v);
            upper = lower = 0;
            return;
        }
        history.push_back({Query::Maximum, {v, w}});

        if (upper == 1) {
            putInPool({v, w});
            upper = lower = 0;
            return;
        }

        if (connector == -1) {
            auto group = groups.rbegin();
            if (group->second == v)
                ++group;
            connector = byMissing[group->second].back();
        }
        pairWith(connector, v);
        capacity.addDeadline(w);
        deadlines.emplace_back(w, connector);

        unusableMaxima = *next(active.begin()) >= w ? unusableMaxima + 1 : 0;
        if (shortLeafRuns >= 2 && unusableMaxima >= 2)
            preserveUniversals = true;

        // Degree one implies the entire complement is a matching. All
        // other possibilities remove at least two edges from the budget.
        int degreeLimit = min({upper, count - 1, n - w + 1 - (v >= w)});
        bool atLeastTwo = noLeaves || w < v || (knownPositive && knownPositive < v);
        int remaining = degreeLimit >= 2 ? upper - 2 : 0;
        if (!atLeastTwo)
            remaining = max(remaining, min(upper, count / 2) - 1);
        lower = max(noLeaves ? 1 : 0, lower - degreeLimit);
        upper = remaining;
        knownPositive = 0;
        mayProbeUniversal = true;
        if (target && v == target) {
            int support = static_cast<int>(byMissing[target].size());
            completionProbes = support >= 4 && 2 * support >= poolSize ? 2 : 1;
        }
    }

    bool universal(Reply reply, Query source, int target = 0) {
        history.push_back({source, reply});
        if (!reply.vertex) {
            mayProbeUniversal = false;
            return false;
        }
        int index = putInPool(reply);
        bool noLeaves = source == Query::Leaf;
        if (noLeaves)
            shortLeafRuns = leavesSinceMaximum <= 2 ? shortLeafRuns + 1 : 0;
        if (noLeaves && upper <= 2) {
            upper = lower = 0;
        } else if (tryFinishWithSeparators()) {
            return true;
        } else {
            maximum(noLeaves, target, index);
        }
        return false;
    }

    void finishSmall() {
        vector<int> labels(active.begin(), active.end());
        array<int, 6> forbiddenCount{};
        vector<Separator> separators;
        int freeSeparators = static_cast<int>(labels.size()) - 1;
        for (auto [size, witness] : groups) {
            auto where = lower_bound(labels.begin(), labels.end(), witness);
            bool restricted = where != labels.end() && *where == witness;
            if (restricted)
                forbiddenCount[where - labels.begin()] = size;
            int limit = min(size, restricted ? static_cast<int>(labels.size()) - 1 : freeSeparators);
            for (int i = 0; i < limit; ++i)
                separators.push_back({restricted ? witness : 0, byMissing[witness][i]});
            if (!restricted)
                freeSeparators -= limit;
        }
        SmallSolver endgame(labels.size(), poolSize, forbiddenCount);
        auto core = endgame.run(labels, lower, upper, history, separators, low,
                                [&](int cap) { return ask(cap); });
        finish(move(core));
    }

public:
    explicit Solver(int size)
        : n(size), lower(n - 2), upper(n - 2), byMissing(n + 1), lastProbe(n + 1),
          capacity(n) {
        for (int v = 1; v <= n; ++v)
            active.emplace_hint(active.end(), v);
        low.reserve(n);
        history.reserve(n + 1);
    }

    void run() {
        if (n >= 18 && universal(ask(0), Query::Universal))
            return;

        for (;;) {
            int count = static_cast<int>(active.size());
            if (upper == 0) {
                finishClique();
                return;
            }
            if (tryFinishWithSeparators())
                return;
            if (count <= 6) {
                finishSmall();
                return;
            }

            // Keep active small labels for the chain if short runs of
            // leaves keep exposing maxima with little usable adjacency.
            bool fewLeaves = preserveUniversals && leavesSinceMaximum == 1 &&
                             groups.size() >= 3;
            if (upper == 1 || (canBridgeMaximum() &&
                (completionProbes || fewLeaves || 4 * upper <= count - poolSize))) {
                if (completionProbes)
                    --completionProbes;
                maximum();
                continue;
            }
            completionProbes = 0;

            bool hadBridge = canBridgeMaximum();
            auto [v, w] = ask(1);
            if (!v)
                exit(0);
            if (!w) {
                if (universal({v, 0}, Query::Leaf))
                    return;
                continue;
            }
            history.push_back({Query::Leaf, {v, w}});
            putInPool({v, w});
            if (++leavesSinceMaximum > 2) {
                shortLeafRuns = 0;
                preserveUniversals = false;
            }
            --upper;
            lower = max(0, lower - 1);
            if (knownPositive == v || knownPositive == w)
                knownPositive = 0;
            if (w < v && (!knownPositive || w < knownPositive))
                knownPositive = w;

            // Repeated witnesses make it worthwhile to try removing their
            // common neighbor in the complement with a safe connector.
            bool hasBridge = canBridgeMaximum();
            int target = !hadBridge && hasBridge ? groups.rbegin()->second : w;
            int repetitions = static_cast<int>(byMissing[target].size());
            int required = 3 * repetitions >= 2 * poolSize ? 2 : 4;
            if (active.size() > 6 && active.contains(target) && repetitions >= required &&
                repetitions >= lastProbe[target] + 2) {
                if (tryFinishWithSeparators())
                    return;
                if (hasBridge) {
                    lastProbe[target] = repetitions;
                    maximum(false, target);
                } else if (mayProbeUniversal) {
                    auto reply = ask(0);
                    if (reply.vertex)
                        lastProbe[target] = repetitions;
                    if (universal(reply, Query::Universal, target))
                        return;
                }
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2 || n > 100000)
        return 0;
    Solver(n).run();
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

namespace {

constexpr int AREA_COUNT = 13;

struct TriangleTable {
    array<int, 64> code{};
    array<int, AREA_COUNT> values{};

    constexpr TriangleTable() {
        int used = 0;
        for (int i = 0; i < 64; ++i) {
            int a = (i & 3) + 1;
            int b = ((i >> 2) & 3) + 1;
            int c = (i >> 4) + 1;
            int area = max(0, (a + b + c) * (a + b - c) *
                                  (a + c - b) * (b + c - a));
            auto entry = find(values.begin(), values.begin() + used, area);
            code[i] = int(entry - values.begin());
            if (entry == values.begin() + used) values[used++] = area;
        }
    }

    constexpr int operator()(int a, int b, int c) const {
        return code[a | (b << 2) | (c << 4)];
    }
};

constexpr TriangleTable triangle;

int equilateralSide(int response) {
    for (int side = 0; side < 4; ++side) {
        if (response == triangle(side, side, side)) return side;
    }
    return -1;
}

// Check the five-position uniqueness lemma using complete reply signatures.
constexpr bool verifyFivePositions() {
    array<pair<uint64_t, bool>, 1024> signatures{};
    for (unsigned bits = 0; bits < signatures.size(); ++bits) {
        int values[5]{};
        int counts[4]{};
        for (int i = 0; i < 5; ++i) {
            values[i] = (bits >> (2 * i)) & 3;
            ++counts[values[i]];
        }
        uint64_t signature = 0;
        for (int i = 0; i < 5; ++i)
            for (int j = i + 1; j < 5; ++j)
                for (int k = j + 1; k < 5; ++k)
                    signature = (signature << 4) | triangle(values[i], values[j], values[k]);
        bool repeated = counts[1] >= 2 || counts[2] >= 2 || counts[3] >= 2;
        signatures[bits] = {signature, repeated};
    }
    sort(signatures.begin(), signatures.end());
    for (size_t i = 1; i < signatures.size(); ++i) {
        if (signatures[i - 1].first == signatures[i].first &&
            (signatures[i - 1].second || signatures[i].second)) return false;
    }
    return true;
}
static_assert(verifyFivePositions());

struct Interactor {
    int ask(array<int, 3> indices) {
        sort(indices.begin(), indices.end());
        cout << "? " << indices[0] + 1 << ' ' << indices[1] + 1 << ' '
             << indices[2] + 1 << endl;
        int raw;
        if (!(cin >> raw) || raw < 0) exit(0);
        auto entry = find(triangle.values.begin(), triangle.values.end(), raw);
        if (entry == triangle.values.end()) exit(0);
        return int(entry - triangle.values.begin());
    }

    void answer(const vector<int>& values) {
        cout << '!';
        for (int value : values) cout << ' ' << value + 1;
        cout << endl;
    }

    void ambiguous() {
        cout << "! -1" << endl;
    }
};

// For n <= 5, build exact answer labels from all query signatures and
// minimize the expected query count in the remaining decision tree.
struct SmallSolver {
    using State = vector<uint16_t>;

    struct StateHash {
        size_t operator()(const State& state) const {
            size_t h = 0;
            for (int index : state) h = h * 1000003 + index + 1;
            return h;
        }
    };

    struct Decision {
        int total;
        int query;
    };

    int n;
    vector<array<int, 3>> queries;
    vector<vector<uint8_t>> replies;
    vector<int> label;
    unordered_map<State, Decision, StateHash> memo;

    explicit SmallSolver(int size) : n(size), label(1 << (2 * n)) {
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j)
                for (int k = j + 1; k < n; ++k)
                    queries.push_back({i, j, k});

        vector<uint64_t> signature(label.size());
        unordered_map<uint64_t, int> multiplicity;
        for (auto [i, j, k] : queries) {
            replies.emplace_back(label.size());
            for (int x = 0; x < int(label.size()); ++x) {
                int response = triangle((x >> (2 * i)) & 3, (x >> (2 * j)) & 3,
                                        (x >> (2 * k)) & 3);
                replies.back()[x] = response;
                signature[x] = (signature[x] << 4) | response;
            }
        }
        for (auto sig : signature) ++multiplicity[sig];
        for (int x = 0; x < int(label.size()); ++x) {
            label[x] = multiplicity[signature[x]] == 1 ? x : -1;
        }
    }

    Decision plan(const State& state) {
        bool finished = true;
        for (int x : state) {
            if (label[x] != label[state.front()]) {
                finished = false;
                break;
            }
        }
        if (finished) return {0, -1};

        if (auto it = memo.find(state); it != memo.end()) return it->second;
        Decision best{INT_MAX, -1};
        for (int q = 0; q < int(queries.size()); ++q) {
            array<State, AREA_COUNT> parts;
            for (int x : state) parts[replies[q][x]].push_back(x);
            int cost = int(state.size());
            bool useful = true;
            for (const State& part : parts) {
                if (part.empty()) continue;
                if (part.size() == state.size()) {
                    useful = false;
                    break;
                }
                cost += plan(part).total;
                if (cost >= best.total) break;
            }
            if (useful && cost < best.total) best = {cost, q};
        }
        memo.emplace(state, best);
        return best;
    }

    template<class Judge>
    void run(Judge& judge) {
        State state(label.size());
        iota(state.begin(), state.end(), 0);

        // Before any observations all triples are equivalent.
        int q = 0;
        while (q >= 0) {
            int reply = judge.ask(queries[q]);
            erase_if(state, [&](int x) { return replies[q][x] != reply; });
            if (state.empty()) exit(0);
            if (n == 5 && q == 0 && equilateralSide(reply) >= 0) {
                // This can finish a constant array on the second query.
                q = find(queries.begin(), queries.end(), array<int, 3>{0, 3, 4})
                    - queries.begin();
                continue;
            }
            q = plan(state).query;
        }
        int x = label[state.front()];
        if (x < 0) {
            judge.ambiguous();
        } else {
            vector<int> answer(n);
            for (int i = 0; i < n; ++i) answer[i] = (x >> (2 * i)) & 3;
            judge.answer(answer);
        }
    }
};

struct Query {
    // Nonnegative entries are active slots. Negative entries encode known values.
    array<int, 3> entries{};
    double score = -1.0;
};

struct Candidate {
    unsigned bits;
    double weight;
};

struct Bucket {
    double mass = 0;
    unsigned any = 0;
    unsigned all = ~0u;

    void add(const Candidate& candidate) {
        mass += candidate.weight;
        any |= candidate.bits;
        all &= candidate.bits;
    }

    void add(const Bucket& other) {
        mass += other.mass;
        any |= other.any;
        all &= other.all;
    }
};

template<class Judge>
class Solver {
    static constexpr int CAPACITY = 1024;
    static constexpr int WINDOW_WIDTH = 8;
    static constexpr double PRIOR = 0.6;
    static constexpr double RECENT_WEIGHT = 0.8;
    static constexpr double DECAY = 0.94;
    static constexpr double TRANSITION_PRIOR = 45;

    Judge& judge;
    const int n;
    vector<int> answer;
    int next = 0;
    vector<int> active;
    vector<Candidate> candidates{{0, 1}};
    array<vector<int>, 4> anchors;
    array<int, 4> count{};
    array<double, 4> recent{};
    array<array<int, 4>, 4> transitions{};
    int nextPeriodCheck = 24;

    void fix(int index, int value) {
        answer[index] = value;
        if (index > 0 && answer[index - 1] >= 0) {
            ++transitions[answer[index - 1]][value];
        }
        if (index + 1 < n && answer[index + 1] >= 0) {
            ++transitions[value][answer[index + 1]];
        }
        if (anchors[value].size() < 2) anchors[value].push_back(index);
        ++count[value];
        for (auto& frequency : recent) frequency *= DECAY;
        recent[value] += 1;
    }

    void removeFixed() {
        unsigned any = 0, all = ~0u;
        double mass = 0;
        for (const auto& candidate : candidates) {
            any |= candidate.bits;
            all &= candidate.bits;
            mass += candidate.weight;
        }
        unsigned varying = any ^ all;

        vector<int> remaining;
        vector<pair<int, int>> shifts;
        for (int i = 0; i < int(active.size()); ++i) {
            if ((varying >> (2 * i)) & 3) {
                shifts.emplace_back(2 * i, 2 * int(remaining.size()));
                remaining.push_back(active[i]);
            } else {
                fix(active[i], (all >> (2 * i)) & 3);
            }
        }
        if (remaining.size() != active.size()) {
            for (auto& candidate : candidates) {
                unsigned newBits = 0;
                for (auto [from, to] : shifts)
                    newBits |= ((candidate.bits >> from) & 3) << to;
                candidate.bits = newBits;
            }
            active.swap(remaining);
        }

        for (auto& candidate : candidates) candidate.weight /= mass;
    }

    array<double, 4> probabilities() const {
        double total = accumulate(count.begin(), count.end(), 4.0 * PRIOR);
        double local = accumulate(recent.begin(), recent.end(), 4.0 * PRIOR);
        array<double, 4> probability;
        for (int v = 0; v < 4; ++v) {
            probability[v] = (1 - RECENT_WEIGHT) * (count[v] + PRIOR) / total
                           + RECENT_WEIGHT * (recent[v] + PRIOR) / local;
        }
        return probability;
    }

    void skipIntroduced() {
        while (next < n &&
               (answer[next] >= 0 || find(active.begin(), active.end(), next) != active.end())) {
            ++next;
        }
    }

    void addFresh(bool force = false) {
        auto probability = probabilities();
        array<array<double, 4>, 4> conditional;
        for (int from = 0; from < 4; ++from) {
            double total = accumulate(transitions[from].begin(), transitions[from].end(), 0.0);
            total += TRANSITION_PRIOR;
            for (int to = 0; to < 4; ++to) {
                conditional[from][to] =
                    (transitions[from][to] + probability[to] * TRANSITION_PRIOR) / total;
            }
        }
        skipIntroduced();
        while (next < n && int(active.size()) < WINDOW_WIDTH &&
               (force || 4 * int(candidates.size()) <= CAPACITY)) {
            int previous = next > 0 ? answer[next - 1] : -1;
            auto previousSlot = previous < 0
                ? find(active.begin(), active.end(), next - 1) : active.end();
            int previousShift = 2 * int(previousSlot - active.begin());
            int shift = 2 * int(active.size());
            active.push_back(next++);
            size_t size = candidates.size();
            candidates.reserve(size * 4);
            auto factor = [&](unsigned bits, int value) {
                if (previousShift != shift) {
                    return conditional[(bits >> previousShift) & 3][value];
                }
                if (previous >= 0) return conditional[previous][value];
                return probability[value];
            };
            for (int v = 1; v < 4; ++v) {
                for (size_t i = 0; i < size; ++i) {
                    candidates.push_back({candidates[i].bits | (unsigned(v) << shift),
                                          candidates[i].weight * factor(candidates[i].bits, v)});
                }
            }
            for (size_t i = 0; i < size; ++i) {
                candidates[i].weight *= factor(candidates[i].bits, 0);
            }
            force = false;
            skipIntroduced();
        }
    }

    vector<int> predictedPeriod() const {
        for (int period = 2; period <= 16 && 3 * period <= next; ++period) {
            vector<int> pattern(period, -1);
            int matches = 0;
            bool conflict = false;
            unsigned seen = 0;
            for (int i = max(0, next - 72); i < next; ++i) {
                int v = answer[i];
                if (v < 0) continue;
                int& expected = pattern[i % period];
                if (expected >= 0 && expected != v) {
                    conflict = true;
                    break;
                }
                if (expected >= 0) ++matches;
                expected = v;
                seen |= 1u << v;
            }
            if (!conflict && matches >= max(16, 2 * period) && popcount(seen) >= 2 &&
                find(pattern.begin(), pattern.end(), -1) == pattern.end()) {
                return pattern;
            }
        }
        return {};
    }

    void rememberFresh(const array<int, 3>& indices, int response) {
        array<int, 3> slot{};
        active.clear();
        for (int k = 0; k < 3; ++k) {
            slot[k] = int(active.size());
            if (answer[indices[k]] < 0) active.push_back(indices[k]);
        }
        candidates.clear();
        for (unsigned bits = 0; bits < (1u << (2 * active.size())); ++bits) {
            array<int, 3> sides;
            for (int k = 0; k < 3; ++k) {
                int fixed = answer[indices[k]];
                sides[k] = fixed >= 0 ? fixed : (bits >> (2 * slot[k])) & 3;
            }
            if (triangle(sides[0], sides[1], sides[2]) == response) {
                candidates.push_back({bits, 1.0});
            }
        }
        if (candidates.empty()) exit(0);
        removeFixed();
        skipIntroduced();
    }

    void scanPeriod(const vector<int>& pattern) {
        array<vector<int>, 4> groups;
        for (int i = next; i < n; ++i) {
            if (answer[i] < 0) groups[pattern[i % pattern.size()]].push_back(i);
        }
        for (int v = 0; v < 4; ++v) {
            const auto& group = groups[v];
            for (size_t p = 0; p + 2 < group.size(); p += 3) {
                array<int, 3> indices{group[p], group[p + 1], group[p + 2]};
                int response = judge.ask(indices);
                if (response == triangle(v, v, v)) {
                    for (int index : indices) fix(index, v);
                    continue;
                }

                // Keep the information from the failed prediction and return
                // to ordinary query selection.
                rememberFresh(indices, response);
                nextPeriodCheck = next + 32;
                return;
            }
        }
        skipIntroduced();
        if (pattern.size() == 1 && next < n) {
            // A constant prefix has at most two unqueried indices here.
            array<int, 3> indices{0, 1, next};
            if (next + 1 < n) indices[1] = next + 1;
            rememberFresh(indices, judge.ask(indices));
        }
        nextPeriodCheck = next + 16;
    }

    void score(const array<Bucket, AREA_COUNT>& bins, const array<int, 3>& entries,
               Query& best, vector<Query>* options) const {
        double entropy = 0, fixed = 0;
        int parts = 0;
        for (const auto& bin : bins) {
            if (bin.mass == 0) continue;
            ++parts;
            entropy -= bin.mass * log2(bin.mass);
            unsigned diff = bin.any ^ bin.all;
            int varying = popcount((diff | (diff >> 1)) & 0x55555555u);
            fixed += bin.mass * (int(active.size()) - varying);
        }
        if (parts < 2) return;
        int span = int(entries[0] >= 0) + int(entries[1] >= 0) + int(entries[2] >= 0);
        // Prefer informative replies that also resolve positions and leave
        // room to introduce more indices into the window.
        double utility = entropy + 0.30 * fixed + 0.55 * (span - 1);
        if (options) options->push_back({entries, utility});
        if (utility > best.score + 1e-10) best = {entries, utility};
    }

    Query choose(vector<Query>* options = nullptr) const {
        Query best;
        int width = int(active.size());

        vector<pair<int, int>> fixedPairs;
        bool injective = false;
        for (int a = 0; a < 4 && !injective; ++a) {
            if (anchors[a].empty()) continue;
            for (int b = a; b < 4; ++b) {
                if (anchors[b].size() < size_t(1 + (a == b))) continue;
                unsigned seen = 0;
                for (int x = 0; x < 4; ++x) seen |= 1u << triangle(a, b, x);
                if (popcount(seen) == 4) {
                    fixedPairs = {{a, b}};
                    injective = true;
                    break;
                }
                fixedPairs.emplace_back(a, b);
            }
        }

        for (int i = 0; i < width; ++i) {
            int si = 2 * i;
            if (!fixedPairs.empty()) {
                array<Bucket, 4> histogram;
                for (const auto& candidate : candidates)
                    histogram[(candidate.bits >> si) & 3].add(candidate);
                for (auto [a, b] : fixedPairs) {
                    array<Bucket, AREA_COUNT> bins;
                    for (int x = 0; x < 4; ++x) bins[triangle(a, b, x)].add(histogram[x]);
                    score(bins, {i, -a - 1, -b - 1}, best, options);
                }
            }

            for (int j = i + 1; j < width; ++j) {
                int sj = 2 * j;
                array<Bucket, 16> histogram;
                for (const auto& candidate : candidates) {
                    unsigned bits = candidate.bits;
                    histogram[((bits >> si) & 3) | (((bits >> sj) & 3) << 2)].add(candidate);
                }
                for (int a = 0; a < 4; ++a) {
                    if (anchors[a].empty()) continue;
                    array<Bucket, AREA_COUNT> bins;
                    for (int xy = 0; xy < 16; ++xy)
                        bins[triangle.code[xy | (a << 4)]].add(histogram[xy]);
                    score(bins, {i, j, -a - 1}, best, options);
                }

                for (int k = j + 1; k < width; ++k) {
                    int sk = 2 * k;
                    array<Bucket, AREA_COUNT> bins;
                    for (const auto& candidate : candidates) {
                        unsigned bits = candidate.bits;
                        unsigned xyz = ((bits >> si) & 3) | (((bits >> sj) & 3) << 2) |
                                       (((bits >> sk) & 3) << 4);
                        bins[triangle.code[xyz]].add(candidate);
                    }
                    score(bins, {i, j, k}, best, options);
                }
            }
        }
        return best;
    }

    unsigned knownNonOnes() const {
        unsigned seen = 0;
        for (int v = 1; v < 4; ++v) {
            if (count[v] >= 2) return 0;
            if (count[v] != 0) seen |= 1u << v;
        }
        return seen | 1u;
    }

    bool isAmbiguous(unsigned bits, unsigned seen) const {
        if (seen == 0) return false;
        for (int i = 0; i < int(active.size()); ++i) {
            int value = (bits >> (2 * i)) & 3;
            if (value == 0) continue;
            unsigned bit = 1u << value;
            if (seen & bit) return false;
            seen |= bit;
        }
        return seen != 1;
    }

    bool allAmbiguous() const {
        // A repeated side >= 2 uniquely identifies any five-position subset
        // containing both copies, by verifyFivePositions().
        // Those two equal sides distinguish all four values in every other
        // position. With distinct non-1 values, their names can be permuted
        // without changing any reply. All ones is identified by reply 3.
        unsigned known = knownNonOnes();
        return known != 0 && all_of(candidates.begin(), candidates.end(), [&](const auto& c) {
            return isAmbiguous(c.bits, known);
        });
    }

    int replyFor(const Query& query, unsigned bits) const {
        int code = 0;
        for (int k = 0; k < 3; ++k) {
            int entry = query.entries[k];
            int value = entry >= 0 ? (bits >> (2 * entry)) & 3 : -entry - 1;
            code |= value << (2 * k);
        }
        return triangle.code[code];
    }

    Query planTail(Query best, vector<Query> options) const {
        using Mask = uint64_t;
        struct Partition {
            Query query;
            vector<Mask> groups;
        };

        ranges::stable_sort(options, greater{}, &Query::score);
        vector<Partition> partitions;
        set<vector<Mask>> seen;
        for (Query query : options) {
            array<Mask, AREA_COUNT> bins{};
            for (int i = 0; i < int(candidates.size()); ++i) {
                bins[replyFor(query, candidates[i].bits)] |= Mask(1) << i;
            }
            vector<Mask> groups;
            for (Mask bin : bins) {
                if (bin != 0) groups.push_back(bin);
            }
            ranges::sort(groups);
            if (seen.insert(groups).second) partitions.push_back({query, move(groups)});
        }

        Mask unique = 0;
        unsigned known = knownNonOnes();
        for (int i = 0; i < int(candidates.size()); ++i) {
            if (!isAmbiguous(candidates[i].bits, known)) unique |= Mask(1) << i;
        }

        auto terminal = [unique](Mask state) {
            return (state & unique) == 0 || has_single_bit(state);
        };
        auto mass = [&](Mask state) {
            double weight = 0;
            for (; state != 0; state &= state - 1) {
                weight += candidates[countr_zero(state)].weight;
            }
            return weight;
        };

        struct Decision {
            double cost;
            int query;
        };
        unordered_map<Mask, Decision> memo;
        int budget = 5000;
        auto plan = [&](auto&& self, Mask state) -> Decision {
            if (terminal(state)) return {0, -1};
            if (auto it = memo.find(state); it != memo.end()) return it->second;
            if (--budget < 0) return {0, -1};
            Decision choice{numeric_limits<double>::infinity(), -1};
            double current = mass(state);
            for (int q = 0; q < int(partitions.size()); ++q) {
                double cost = current;
                bool useful = true;
                for (Mask group : partitions[q].groups) {
                    Mask part = group & state;
                    if (part == state) {
                        useful = false;
                        break;
                    }
                    cost += self(self, part).cost;
                    if (budget < 0) return {0, -1};
                    if (cost >= choice.cost - 1e-12) break;
                }
                if (useful && cost < choice.cost - 1e-12) choice = {cost, q};
            }
            return memo.emplace(state, choice).first->second;
        };

        Mask full = ~Mask(0) >> (64 - candidates.size());
        int q = plan(plan, full).query;
        return q >= 0 ? partitions[q].query : best;
    }

    void makeQuery(const Query& query) {
        array<int, 3> indices;
        array<int, 4> used{};
        for (int k = 0; k < 3; ++k) {
            int entry = query.entries[k];
            if (entry >= 0) {
                indices[k] = active[entry];
            } else {
                int value = -entry - 1;
                indices[k] = anchors[value][used[value]++];
            }
        }
        int reply = judge.ask(indices);
        erase_if(candidates, [&](const Candidate& candidate) {
            return replyFor(query, candidate.bits) != reply;
        });
        if (candidates.empty()) exit(0);
        removeFixed();
    }

public:
    Solver(Judge& judge, int n) : judge(judge), n(n), answer(n, -1) {}

    void run() {
        if (n <= 5) {
            SmallSolver(n).run(judge);
            return;
        }

        array<int, 3> first{0, 1, 2};
        int reply = judge.ask(first);
        rememberFresh(first, reply);
        int side = equilateralSide(reply);
        if (side >= 0) scanPeriod({side});

        vector<int> period;
        while (true) {
            if (period.empty() && next >= nextPeriodCheck &&
                n - next >= 24 && knownNonOnes() == 0) {
                nextPeriodCheck = next + 16;
                period = predictedPeriod();
            }
            if (!period.empty() && active.empty()) {
                scanPeriod(period);
                period.clear();
            }
            if (period.empty()) addFresh();
            if (next == n && allAmbiguous()) {
                judge.ambiguous();
                return;
            }
            if (active.empty()) {
                judge.answer(answer);
                return;
            }

            vector<Query> options;
            bool useTail = next == n && candidates.size() <= 64;
            Query query = choose(useTail ? &options : nullptr);
            if (query.score < 0) {
                // All triples on the introduced positions have a fixed reply.
                // For at least five positions, ambiguity requires distinct
                // non-1 sides. The positions of all ones are then given by
                // reply 3, or by the lone nonzero triple if there are two ones.
                // Thus at most three slots vary and a fresh index still fits.
                if (next < n) {
                    assert(active.size() < WINDOW_WIDTH);
                    addFresh(true);
                    continue;
                }
                judge.ambiguous();
                return;
            }
            if (useTail && options.size() > 1) query = planTail(query, move(options));
            makeQuery(query);
        }
    }
};

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 3 || n > 5000) return 0;
    Interactor judge;
    Solver(judge, n).run();
    return 0;
}

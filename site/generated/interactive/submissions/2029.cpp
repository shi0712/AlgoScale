#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <span>
#include <unordered_set>
#include <vector>

using namespace std;

static constexpr int queryBudget = 100000;

class Machine {
public:
    int used = 0;

    int ask(span<const int> labels) {
        assert(labels.size() >= 2 && used + labels.size() <= queryBudget);
        used += static_cast<int>(labels.size());
        cout << "? " << labels.size();
        for (int label : labels) cout << ' ' << label;
        cout << '\n' << flush;
        int response;
        if (!(cin >> response) || response < 0 ||
            response >= static_cast<int>(labels.size())) {
            exit(0);
        }
        return response;
    }

    int ask(initializer_list<int> labels) {
        return ask(span<const int>(labels.begin(), labels.size()));
    }
};

// Each small decision tree stores a query, followed by the child offsets for
// each possible response. Negative entries at the start of nodes are answers.
static constexpr int small2[] = {2,0,1,5,6,-2,-1};
static constexpr int small3[] = {3,1,0,2,7,8,9,-3,-2,-1};
static constexpr int small4[] = {
    4,0,1,2,3,9,10,20,32,-4,3,0,2,1,17,18,19,-3,-1,-2,
    4,0,1,3,2,-1,29,30,31,-3,-3,-2,-2
};
static constexpr int small5[] = {
    5,0,1,2,3,4,11,12,25,40,55,-5,4,0,3,1,2,21,22,23,24,-4,-1,-3,-2,
    5,1,0,2,4,3,-1,36,37,38,39,-4,-4,-3,-2,
    5,0,1,3,4,2,-1,51,52,53,54,-3,-2,-2,-3,-3
};
static constexpr int small6[] = {
    6,1,0,2,3,4,5,13,14,31,68,94,111,-6,6,0,1,4,2,5,3,-1,27,28,29,30,-1,
    -2,-5,-3,-4,6,0,1,2,5,3,4,-1,44,58,59,66,67,5,0,1,2,3,5,55,56,57,-1,
    -1,-5,-1,-4,-5,2,0,1,64,65,-3,-2,-4,-3,6,2,0,5,1,4,3,-1,81,82,91,92,
    93,-3,3,0,1,2,-1,89,90,-2,-4,-4,-3,-3,6,0,1,3,5,4,2,-1,107,108,109,-1,
    110,-4,-3,-2,-3,-3
};
static constexpr int small7[] = {
    7,0,1,2,3,4,5,6,15,16,35,65,122,158,177,-7,6,0,4,1,5,2,3,29,30,31,32,
    33,34,-6,-1,-5,-2,-4,-3,7,2,0,3,6,5,1,4,-1,50,51,52,53,63,64,-6,-6,-5,
    3,0,1,5,60,61,62,-4,-2,-3,-4,-3,7,1,6,0,2,5,3,4,-1,80,81,82,99,120,121,
    -5,-2,6,1,0,2,4,5,3,-1,-1,95,96,97,98,-5,-4,-2,-3,7,2,1,5,4,6,0,3,-1,
    114,115,116,117,118,119,-2,-3,-5,-2,-4,-3,-3,-4,7,2,5,0,3,6,1,4,-1,137,
    138,155,156,157,-1,-5,6,0,1,3,2,4,5,-1,151,152,153,154,-1,-4,-5,-3,-4,
    -4,-5,-3,7,0,1,3,6,2,4,5,-1,-1,173,174,175,-1,176,-4,-3,-3,-4,-4
};
static constexpr int small8[] = {
    8,0,1,2,3,4,5,6,7,17,18,40,100,217,349,461,483,-8,7,0,4,1,5,2,6,3,33,34,35,36,37,38,39,
    -7,-1,-6,-2,-5,-3,-4,8,2,6,1,5,0,3,7,4,-1,57,58,59,60,78,79,99,-7,-7,-6,7,0,1,2,3,4,7,5,
    -1,75,76,77,-1,-1,-1,-6,-5,-2,-5,8,0,1,6,7,2,3,4,5,-1,96,-1,97,98,-1,-1,-1,-4,-3,-4,-4,
    8,1,7,2,6,5,4,0,3,-1,117,137,158,177,196,215,216,7,0,1,3,2,4,6,5,-1,-1,132,133,134,135,
    136,-6,-2,-5,-3,-4,7,1,2,3,7,4,6,5,-1,152,153,154,155,156,157,-4,-2,-3,-6,-4,-5,6,0,2,1,
    6,4,5,171,172,173,174,175,176,-6,-3,-5,-4,-4,-3,7,0,1,4,6,2,5,3,-1,-1,192,193,194,195,
    -1,-6,-2,-5,-3,7,0,1,3,5,7,6,2,-1,-1,211,212,213,214,-1,-4,-3,-5,-4,-5,-4,8,1,6,0,3,7,2,
    4,5,-1,234,241,262,282,304,326,348,2,0,1,239,240,-5,-6,7,0,1,3,6,7,4,2,-1,256,257,258,
    259,260,261,-6,-6,-4,-5,-3,-4,7,0,1,3,2,5,4,6,-1,277,278,-1,279,280,281,-4,-6,-6,-5,-5,
    8,0,1,3,2,4,5,7,6,-1,-1,-1,299,300,301,302,303,-6,-4,-3,-5,-4,8,0,1,2,3,6,7,4,5,-1,-1,
    321,322,323,324,325,-1,-4,-5,-3,-5,-5,8,0,1,2,5,3,7,6,4,-1,-1,343,344,345,346,347,-1,-4,
    -5,-3,-4,-5,-4,8,1,4,7,6,2,0,5,3,-1,366,384,403,410,428,444,460,7,0,1,2,4,3,5,6,-1,-1,
    -1,381,382,383,-1,-3,-5,-4,7,0,1,2,3,5,4,6,-1,-1,-1,399,400,401,402,-4,-3,-3,-4,2,0,1,
    408,409,-5,-4,7,0,1,2,3,4,6,5,-1,-1,-1,-1,425,426,427,-5,-3,-4,6,0,1,2,3,5,4,-1,-1,441,
    442,443,-1,-4,-3,-5,6,0,1,2,3,5,4,-1,-1,457,458,459,-1,-3,-4,-5,-4,8,0,1,3,4,6,7,2,5,-1,
    -1,478,479,480,481,482,-1,-4,-5,-5,-4,-4,-4
};
static constexpr const int* smallTrees[] = {
    nullptr, nullptr, small2, small3, small4, small5, small6, small7, small8
};

int solve_small(Machine& machine, const int* tree) {
    int node = 0;
    while (tree[node] > 0) {
        int length = tree[node];
        int response = machine.ask(span<const int>(tree + node + 1, length));
        node = tree[node + 1 + length + response];
        if (node < 0) exit(0);
    }
    return -tree[node];
}

struct SparseCost {
    int queries = 0;
    int labels = 0;
};

class SparsePath {
    static constexpr int maxDefects = 24;
    Machine& machine;
    vector<array<SparseCost, maxDefects + 1>> costs;

    static int regular_count(int length, int type, bool alternating) {
        if (!alternating) return type == 0 ? length : 0;
        return (length + (type == 0)) / 2;
    }

    SparseCost bound(int edges, int defects) {
        defects = min(defects, edges - defects);
        if (defects == 0) return {};
        if (defects == 1) return {1, edges + 1};
        auto& cost = costs[edges][defects];
        if (cost.queries > 0) return cost;
        int half = edges / 2;
        for (int left = max(0, defects - (edges - half)); left <= min(half, defects); ++left) {
            auto a = bound(half, left);
            auto b = bound(edges - half, defects - left);
            cost.queries = max(cost.queries, 1 + a.queries + b.queries);
            cost.labels = max(cost.labels, half + 1 + a.labels + b.labels);
        }
        return cost;
    }

    // Count vertices [first, last), with the type of first known.
    int count(int first, int last, int type, bool alternating, int defects) {
        int edges = last - first;
        if (defects > edges - defects) {
            alternating = !alternating;
            defects = edges - defects;
        }
        if (defects == 0) return regular_count(edges, type, alternating);

        if (defects == 1) {
            int cut;
            vector<int> query;
            if (!alternating) {
                // The crossing count identifies the cut, using odd results
                // for cuts in the first half and even results for the rest.
                for (int left = first, right = last; left <= right; ++left, --right) {
                    query.push_back(left);
                    if (left < right) query.push_back(right);
                }
                int result = machine.ask(query);
                cut = result % 2 ? (result + 1) / 2 : edges + 1 - result / 2;
            } else {
                // Only the cut's parity affects the number of A mushrooms.
                for (int i = first; i < last; i += 2) {
                    query.push_back(i + 1);
                    query.push_back(i);
                }
                int crossings = static_cast<int>(query.size()) - 1 - machine.ask(query);
                bool odd = query.size() == 2 ? crossings == 1 : crossings >= 2;
                cut = odd ? 1 : 2;
            }
            int rightType = type ^ (alternating && (cut % 2)) ^ 1;
            return regular_count(cut, type, alternating) +
                   regular_count(edges - cut, rightType, alternating);
        }

        int mid = first + edges / 2;
        vector<int> query(mid - first + 1);
        iota(query.begin(), query.end(), first);
        int changes = machine.ask(query);
        int leftDefects = alternating ? mid - first - changes : changes;
        int leftCount = count(first, mid, type, alternating, leftDefects);
        return leftCount + count(mid, last, type ^ (changes % 2),
                                 alternating, defects - leftDefects);
    }

public:
    explicit SparsePath(Machine& judge) : machine(judge) {}

    int try_count(int n, int changes) {
        int defects = min(changes, n - 1 - changes);
        if (defects > maxDefects) return -1;
        costs.resize(n);
        auto cost = bound(n - 1, defects);
        if (cost.labels + machine.used > queryBudget ||
            100LL * cost.queries * cost.queries > 196LL * n) {
            return -1;
        }
        return count(0, n - 1, 0, changes != defects, defects) + (changes % 2 == 0);
    }
};

class CandidateSearch {
    struct Test {
        vector<int> labels;
        vector<unsigned char> response;
    };

    Machine& machine;
    int n;
    vector<unsigned> assignments;
    vector<int> totals;
    vector<Test> tests;
    unordered_set<uint64_t> seen;

    void add_test(vector<int> labels) {
        if (n <= 11) {
            if (labels.front() > labels.back()) reverse(labels.begin(), labels.end());
            uint64_t key = 0;
            for (int label : labels) key = (key << 4) | (label + 1);
            if (!seen.insert(key).second) return;
        }
        vector<unsigned> edges;
        for (size_t i = 1; i < labels.size(); ++i) {
            edges.push_back((1U << labels[i - 1]) | (1U << labels[i]));
        }
        auto& test = tests.emplace_back(Test{move(labels), {}});
        test.response.reserve(assignments.size());
        for (unsigned mask : assignments) {
            int changes = 0;
            for (unsigned edge : edges) changes += __builtin_parity(mask & edge);
            test.response.push_back(static_cast<unsigned char>(changes));
        }
    }

    int leaf(span<const int> state) const {
        int total = totals[state.front()];
        for (int id : state) {
            if (totals[id] != total) return 0;
        }
        return total;
    }

    int one_query(span<const int> state) const {
        for (int i = 0; i < static_cast<int>(tests.size()); ++i) {
            array<int, 16> value{};
            bool separates = true;
            for (int id : state) {
                int& at = value[tests[i].response[id]];
                if (at != 0 && at != totals[id]) {
                    separates = false;
                    break;
                }
                at = totals[id];
            }
            if (separates) return i;
        }
        return -1;
    }

    int choose(const vector<int>& state) const {
        if (int single = one_query(state); single >= 0) return single;

        vector<pair<int, int>> ranked;
        for (int i = 0; i < static_cast<int>(tests.size()); ++i) {
            array<int, 16> size{};
            array<unsigned, 16> possible{};
            for (int id : state) {
                int response = tests[i].response[id];
                ++size[response];
                possible[response] |= 1U << totals[id];
            }
            int score = 0, largest = 0;
            for (int j = 0; j < static_cast<int>(tests[i].labels.size()); ++j) {
                if (has_single_bit(possible[j]) || size[j] == 0) continue;
                score += size[j] * (size[j] + 8);
                largest = max(largest, size[j]);
            }
            if (largest < static_cast<int>(state.size())) {
                ranked.emplace_back(score + largest * largest, i);
            }
        }

        int shortlist = min(static_cast<int>(ranked.size()), state.size() <= 100 ? 80 : 1);
        partial_sort(ranked.begin(), ranked.begin() + shortlist, ranked.end());
        int best = ranked.front().second;
        int fewestContinuing = static_cast<int>(state.size()) + 1;
        for (int i = 0; i < shortlist && shortlist > 1; ++i) {
            int query = ranked[i].second;
            vector<vector<int>> branches(tests[query].labels.size());
            for (int id : state) branches[tests[query].response[id]].push_back(id);
            sort(branches.begin(), branches.end(), [](const auto& a, const auto& b) {
                return a.size() > b.size();
            });
            int continuing = 0;
            for (auto& branch : branches) {
                if (!branch.empty() && !leaf(branch)) continuing += static_cast<int>(branch.size());
            }
            if (continuing >= fewestContinuing) continue;
            bool solves = true;
            for (auto& branch : branches) {
                if (!branch.empty() && !leaf(branch) && one_query(branch) < 0) {
                    solves = false;
                    break;
                }
            }
            if (solves) {
                fewestContinuing = continuing;
                best = query;
            }
        }
        return best;
    }

public:
    static constexpr int limit = 16;

    CandidateSearch(Machine& judge, int count, int changes) : machine(judge), n(count) {
        unsigned edgeMask = (1U << (n - 1)) - 1;
        for (unsigned mask = 0; mask < (1U << n); mask += 2) {
            if (popcount((mask ^ (mask >> 1)) & edgeMask) == changes) {
                assignments.push_back(mask);
                totals.push_back(n - popcount(mask));
            }
        }
    }

    int run() {
        vector<int> state(assignments.size());
        iota(state.begin(), state.end(), 0);
        if (state.empty()) exit(0);
        if (int answer = leaf(state)) return answer;
        vector<int> labels(n);
        iota(labels.begin(), labels.end(), 0);
        add_test(labels);
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) add_test({i, j});
        }
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                for (int k = i + 1; k < n; ++k) {
                    if (i != j && j != k) add_test({i, j, k});
                }
            }
        }
        int samples = n == 9 ? 40000 : n == 10 ? 25000 : n <= 12 ? 12000 : 5000;
        mt19937 rng(13651);
        for (int i = 0; i < samples; ++i) {
            shuffle(labels.begin(), labels.end(), rng);
            int length = n - (i % 5 == 0 ? i % 4 : i % 3);
            add_test(vector<int>(labels.begin(), labels.begin() + length));
        }
        for (int step = 0; ; ++step) {
            if (step < n) {
                int id = choose(state);
                int response = machine.ask(tests[id].labels);
                erase_if(state, [&](int candidate) { return tests[id].response[candidate] != response; });
            } else {
                unsigned base = assignments[state.front()];
                auto different = find_if(state.begin(), state.end(), [&](int id) {
                    return assignments[id] != base;
                });
                int label = countr_zero(base ^ assignments[*different]);
                int response = machine.ask({0, label});
                erase_if(state, [&](int id) { return ((assignments[id] >> label) & 1U) !=
                                                     static_cast<unsigned>(response); });
            }
            if (state.empty()) exit(0);
            if (int answer = leaf(state)) return answer;
        }
    }
};

struct Term {
    int label;
    int sign;
};

struct RowSize {
    int terms = 0;
    int balance = 0;
};

struct RowPair {
    int left;
    int right;
    int rightSign;
    int extraSign;
};

struct Shape {
    int leftRows = 0;
    int variables = 0;
    int needMajor = 0;
    int needMinor = 0;
    int totalLength = 0;
    vector<RowSize> sizes;
    vector<RowPair> pairs;
};

class WeighingPlans {
    static vector<int> sorted_rows(const Shape& shape) {
        vector<int> rows(shape.sizes.size());
        iota(rows.begin(), rows.end(), 0);
        stable_sort(rows.begin(), rows.end(), [&](int a, int b) {
            return shape.sizes[a].terms < shape.sizes[b].terms;
        });
        return rows;
    }

    static void measure(Shape& shape) {
        for (auto [terms, balance] : shape.sizes) {
            int larger = (terms + abs(balance)) / 2;
            int smaller = terms - larger;
            shape.needMajor = max(shape.needMajor, larger + 1);
            if (smaller > 0) shape.needMinor = max(shape.needMinor, smaller + 1);
            shape.totalLength += 2 * terms + 2 + (smaller > 0);
        }
    }

public:
    vector<Shape> shapes;

    explicit WeighingPlans(int limit) : shapes(limit + 1) {
        shapes[1].variables = 1;
        shapes[1].sizes.push_back({1, 1});
        measure(shapes[1]);
        vector<vector<int>> orders(limit + 1);
        orders[1] = {0};
        for (int rows = 2; rows <= limit; ++rows) {
            auto& best = shapes[rows];
            for (int leftRows = (rows + 1) / 2; leftRows < rows; ++leftRows) {
                int rightRows = rows - leftRows;
                const auto& left = shapes[leftRows];
                const auto& right = shapes[rightRows];
                int variables = left.variables + right.variables + rightRows;
                if (variables < best.variables) continue;

                Shape candidate;
                candidate.leftRows = leftRows;
                candidate.variables = variables;
                candidate.sizes = left.sizes;
                candidate.sizes.resize(rows);
                for (int i = 0; i < rightRows; ++i) {
                    int a = orders[leftRows][i];
                    int b = orders[rightRows][rightRows - 1 - i];
                    auto l = left.sizes[a];
                    auto r = right.sizes[b];
                    int rightSign = l.balance * r.balance < 0 ? -1 : 1;
                    int sum = l.balance + rightSign * r.balance;
                    int extraSign = sum > 0 ? -1 : 1;
                    candidate.sizes[a] = {l.terms + r.terms + 1, sum + extraSign};
                    candidate.sizes[leftRows + b] = {
                        l.terms + r.terms, l.balance - rightSign * r.balance
                    };
                    candidate.pairs.push_back({a, b, rightSign, extraSign});
                }
                measure(candidate);
                array<int, 4> rank = {-variables, candidate.needMinor,
                                      candidate.needMajor, candidate.totalLength};
                array<int, 4> bestRank = {-best.variables, best.needMinor,
                                          best.needMajor, best.totalLength};
                if (rank < bestRank) best = move(candidate);
            }
            orders[rows] = sorted_rows(best);
        }
    }

    vector<vector<Term>> equations(int rows, int first) const {
        if (rows == 1) return {{{first, 1}}};
        const auto& shape = shapes[rows];
        int a = shape.leftRows;
        int middle = first + shapes[a].variables;
        int extra = middle + shapes[rows - a].variables;
        auto left = equations(a, first);
        auto right = equations(rows - a, middle);
        left.resize(rows);
        for (auto [i, j, rightSign, extraSign] : shape.pairs) {
            auto& top = left[i];
            auto& bottom = left[a + j];
            bottom = top;
            for (auto [label, sign] : right[j]) {
                top.push_back({label, rightSign * sign});
                bottom.push_back({label, -rightSign * sign});
            }
            top.push_back({extra++, extraSign});
        }
        return left;
    }

    // A paired row yields u + v + s*z and u - v. Their sum
    // exposes z by parity and then gives both child row values.
    void decode(span<int> values, span<int> types) const {
        int rows = static_cast<int>(values.size());
        if (rows == 1) {
            types[0] = values[0];
            return;
        }
        const auto& shape = shapes[rows];
        int a = shape.leftRows;
        int leftVariables = shapes[a].variables;
        int extra = leftVariables + shapes[rows - a].variables;
        for (auto [i, j, rightSign, extraSign] : shape.pairs) {
            int combined = values[i] + values[a + j];
            int z = (combined % 2 != 0);
            types[extra++] = z;
            int u = (combined - extraSign * z) / 2;
            values[a + j] = rightSign * (values[i] - extraSign * z - u);
            values[i] = u;
        }
        decode(values.first(a), types.first(leftVariables));
        decode(values.subspan(a), types.subspan(leftVariables));
    }
};

class Counter {
    static constexpr int maxRows = 128;

    Machine& machine;
    WeighingPlans plans;
    array<vector<int>, 2> known;
    vector<int> unseen;
    // For pending terms, the type is the same common bit, XOR the stored bit.
    vector<Term> pending;
    int next = 0;
    int answer = 0;
    int pureStreak = 0;
    bool precedingBulkPure = false;
    bool probe;
    bool longRuns;

    int remaining() const { return static_cast<int>(unseen.size()) - next; }
    int majority_type() const { return known[1].size() > known[0].size(); }

    void learn(int label, int type) {
        assert(type == 0 || type == 1);
        known[type].push_back(label);
        answer += type == 0;
    }

    bool can_weigh(int positive, int negative) const {
        return (positive == 0 || static_cast<int>(known[0].size()) > positive) &&
               (negative == 0 || static_cast<int>(known[1].size()) > negative);
    }

    // Inside each group, the anchors make every term contribute either zero
    // or two. The optional final endpoint contributes just one mismatch.
    pair<int, int> mismatch_sum(span<const Term> terms, int endpoint) {
        vector<int> query;
        query.reserve(terms.size() * 2 + 3);
        int groups = 0, lastType = 0;
        for (int type = 0; type < 2; ++type) {
            int index = 0;
            for (auto [label, sign] : terms) {
                if ((sign < 0) != type) continue;
                query.push_back(known[type][index++]);
                query.push_back(label);
            }
            if (index != 0) {
                query.push_back(known[type][index]);
                ++groups;
                lastType = type;
            }
        }
        if (endpoint >= 0) {
            if (query.empty()) query.push_back(known[lastType][0]);
            query.push_back(endpoint);
        }
        int result = machine.ask(query) - (groups == 2);
        return {result / 2, lastType ^ (result % 2)};
    }

    void apply_matrix(int rows) {
        const auto& shape = plans.shapes[rows];
        auto equations = plans.equations(rows, next);
        vector<int> values(rows);
        bool hasPending = !pending.empty();
        for (int i = 0; i < rows; ++i) {
            auto& equation = equations[i];
            int positive = 0;
            for (auto& term : equation) {
                positive += term.sign > 0;
                term.label = unseen[term.label];
            }
            int negative = static_cast<int>(equation.size()) - positive;
            int orientation = 1;
            if (!can_weigh(positive, negative)) {
                for (auto& term : equation) term.sign = -term.sign;
                swap(positive, negative);
                orientation = -1;
            }
            bool usePending = hasPending && i == 0;
            int endpoint = usePending ? pending.front().label
                                      : unseen[next + shape.variables + i - hasPending];
            auto [sum, type] = mismatch_sum(equation, endpoint);
            values[i] = orientation * (sum - negative);
            if (usePending) resolve(type ^ pending.front().sign);
            else learn(endpoint, type);
        }
        vector<int> types(shape.variables);
        plans.decode(values, types);
        for (int i = 0; i < shape.variables; ++i) learn(unseen[next + i], types[i]);
        next += shape.variables + rows - hasPending;
        pureStreak = 0;
        precedingBulkPure = false;
        probe = false;
    }

    void resolve(int bit) {
        for (auto [label, reversed] : pending) learn(label, bit ^ reversed);
        pending.clear();
    }

    void resolve_with_endpoint() {
        int label = pending.front().label;
        int major = majority_type();
        if (remaining() && known[major].size() >= 2) {
            int endpoint = unseen[next++];
            int result = machine.ask({known[major][0], label, known[major][1], endpoint});
            resolve((result / 2) ^ major ^ pending.front().sign);
            learn(endpoint, major ^ (result % 2));
        } else {
            resolve(machine.ask({known[0][0], label}) ^ pending.front().sign);
        }
    }

    void small_step() {
        precedingBulkPure = false;
        int major = majority_type();
        int anchors = static_cast<int>(known[major].size());
        if (anchors < 3 || remaining() < 3) {
            pureStreak = 0;
            if (!pending.empty()) {
                resolve_with_endpoint();
            } else if (remaining() >= 2 && anchors >= 2) {
                int x = unseen[next++], y = unseen[next++];
                int response = machine.ask({known[major][0], x, known[major][1], y});
                learn(x, major ^ (response / 2));
                learn(y, major ^ (response % 2));
            } else if (remaining() >= 2) {
                int x = unseen[next], y = unseen[next + 1];
                int response = machine.ask({known[major][0], x, y});
                learn(y, major ^ (response % 2));
                if (response % 2 == 0) {
                    learn(x, major ^ (response / 2));
                    next += 2;
                } else {
                    swap(unseen[next], unseen[next + 1]);
                    ++next;
                }
            } else if (remaining()) {
                int x = unseen[next++];
                learn(x, machine.ask({known[0][0], x}));
            }
            return;
        }

        vector<Term> terms;
        int chosenRef = -1, chosenFlip = 0;
        for (int ref : {major, 1 - major}) {
            for (int flip : {0, 1}) {
                terms.clear();
                int positive = ref == 0 ? 2 : 0;
                for (auto [label, reversed] : pending) {
                    int reference = reversed ^ flip;
                    positive += reference == 0;
                    terms.push_back({label, reference == 0 ? 1 : -1});
                }
                int negative = static_cast<int>(pending.size()) + 2 - positive;
                if (can_weigh(positive, negative)) {
                    chosenRef = ref;
                    chosenFlip = flip;
                    break;
                }
            }
            if (chosenRef >= 0) break;
        }
        if (chosenRef < 0) {
            pureStreak = 0;
            resolve_with_endpoint();
            return;
        }

        int x = unseen[next++], y = unseen[next++], endpoint = unseen[next++];
        int ref = chosenRef;
        terms.push_back({x, ref == 0 ? 1 : -1});
        terms.push_back({y, ref == 0 ? 1 : -1});
        auto [sum, endpointType] = mismatch_sum(terms, endpoint);
        learn(endpoint, endpointType);

        int older = static_cast<int>(pending.size());
        if (older == 2 && sum == 2) {
            for (auto& term : pending) term.sign ^= chosenFlip;
            pending.push_back({x, ref ^ 1});
            pending.push_back({y, ref ^ 1});
            pureStreak = 0;
            return;
        }
        if (older) {
            int commonMismatch = sum >= older;
            sum -= older * commonMismatch;
            resolve(commonMismatch ^ chosenFlip);
        }
        if (sum == 1) {
            pending = {{x, 0}, {y, 1}};
            pureStreak = 0;
        } else {
            int type = ref ^ (sum / 2);
            learn(x, type);
            learn(y, type);
            pureStreak = endpointType == type ? pureStreak + 1 : 0;
        }
    }

    void bulk_with_pending() {
        int type = majority_type();
        const auto& anchors = known[type];
        int amount = min(remaining(), static_cast<int>(anchors.size()) - 1);
        vector<int> query;
        for (int i = 0; i < amount; ++i) {
            query.push_back(anchors[i]);
            query.push_back(unseen[next + i]);
        }
        query.push_back(anchors[amount]);
        query.push_back(pending.front().label);

        int result = machine.ask(query);
        int inside = result / 2;
        answer += type == 0 ? amount - inside : inside;
        if (inside == 0 || inside == amount) {
            int insideType = type ^ (inside != 0);
            for (int i = 0; i < amount; ++i) known[insideType].push_back(unseen[next + i]);
        }
        resolve(type ^ (result % 2) ^ pending.front().sign);
        next += amount;
        pureStreak = 0;
        precedingBulkPure = (inside == 0 || inside == amount) && amount >= 3;
    }

    void bulk_step() {
        int type = majority_type();
        const auto& anchors = known[type];
        int capacity = static_cast<int>(anchors.size());
        int left = remaining();
        vector<int> query;
        pureStreak = 0;

        if (left == capacity + 1) {
            query.push_back(unseen[next]);
            for (int i = 0; i < capacity; ++i) {
                query.push_back(anchors[i]);
                query.push_back(unseen[next + i + 1]);
            }
            int result = machine.ask(query);
            int different;
            if (capacity == 1) different = result;
            else if (result == 0) different = 0;
            else if (result == 2 * capacity) different = left;
            else if (result % 2) different = (result + 1) / 2;
            else different = result / 2 + machine.ask({anchors[0], unseen[next]});
            answer += type == 0 ? left - different : different;
            next += left;
            return;
        }

        int amount = min(left, capacity);
        query.reserve(2 * amount);
        for (int i = 0; i < amount; ++i) {
            query.push_back(anchors[i]);
            query.push_back(unseen[next + i]);
        }

        int result = machine.ask(query);
        int inside = result / 2;
        int different = inside + (result % 2);
        answer += type == 0 ? amount - different : different;
        if (inside == 0 || inside == amount - 1) {
            int insideType = type ^ (inside != 0);
            for (int i = 0; i < amount - 1; ++i) known[insideType].push_back(unseen[next + i]);
        }
        known[type ^ (result % 2)].push_back(unseen[next + amount - 1]);
        precedingBulkPure = (inside == 0 || inside == amount - 1) && amount >= 3;
        next += amount;
    }

    // With i extra anchors, a majority has at least max(originalMajor,
    // ceil((originalTotal + i) / 2)) members. Sum the constant prefix and
    // the subsequent staircase to forecast the number of counting queries.
    static int bulk_depth(int left, int originalMajor, int total) {
        if (left <= originalMajor || (originalMajor == 1 && left == 2)) {
            return left > 0;
        }
        auto staircase = [](long long x) { return x * x / 4; };
        int low = 1, high = (left + originalMajor - 1) / originalMajor;
        while (low < high) {
            int middle = (low + high) / 2;
            int prefix = clamp(2 * originalMajor + 1 - total, 0, middle);
            long long capacity = 1LL * prefix * originalMajor +
                                 staircase(total + middle) - staircase(total + prefix);
            if (capacity >= left) high = middle;
            else low = middle + 1;
        }
        return low;
    }

    // Zero selects a counting query, -1 a small classification query,
    // and a positive value selects a signed weighing batch.
    int choose() const {
        int left = remaining();
        int a = static_cast<int>(known[0].size());
        int b = static_cast<int>(known[1].size());
        int major = max(a, b);
        if (left <= major + 1 || (longRuns && major >= 4)) return 0;
        if (pending.empty() && major >= 4 &&
            (precedingBulkPure || (probe && major >= 6) ||
             (major >= 8 && pureStreak >= 2 && min(a, b) * 6 <= major))) {
            return 0;
        }

        int total = a + b;
        int best = bulk_depth(left, major, total);
        int action = 0;
        int limit = min(left, 5 * static_cast<int>(sqrt(left)) + 8);
        vector<int> spent(limit + 1, best + 1), firstAction(limit + 1);
        spent[0] = 0;
        for (int gained = 0; gained <= limit; ++gained) {
            if (spent[gained] >= best) continue;
            if (gained > 0) {
                int estimate = spent[gained] + bulk_depth(left - gained, major, total + gained);
                if (estimate < best) {
                    best = estimate;
                    action = firstAction[gained];
                }
            }

            int stock = total + gained;
            int big = max(major, (stock + 1) / 2);
            int small = stock - big;
            auto consider = [&](int gain, int queries, int step) {
                int reach = gained + gain;
                int cost = spent[gained] + queries;
                if (reach <= limit && cost < spent[reach] && cost < best) {
                    spent[reach] = cost;
                    firstAction[reach] = gained == 0 ? step : firstAction[gained];
                }
            };

            if (gained > 0 || machine.used + 2 * left + 20 <= queryBudget) {
                consider(min(left - gained, min(3, big)), 1, -1);
            }
            for (int rows = 2; rows < static_cast<int>(plans.shapes.size()); ++rows) {
                if (spent[gained] + rows >= best) break;
                const auto& shape = plans.shapes[rows];
                if (shape.needMinor > small || shape.needMajor > big) continue;
                int gain = shape.variables + rows;
                if (gain > limit - gained) break;
                if (gained == 0 &&
                    machine.used + shape.totalLength + 2 * (left - gain) + 3 > queryBudget) {
                    continue;
                }
                consider(gain, rows, rows);
            }
        }
        return action;
    }

public:
    Counter(Machine& judge, int n, int changes)
        : machine(judge), plans(min(maxRows, n / 2)), unseen(n - 2),
          probe(changes * 12 <= n), longRuns(1LL * changes * changes <= 4LL * n) {
        iota(unseen.begin(), unseen.end(), 1);
        learn(0, 0);
        learn(n - 1, changes % 2);
    }

    int run() {
        while (remaining() > 0) {
            int action = choose();
            if (action == -1) {
                small_step();
                continue;
            }
            if (action > 0) {
                apply_matrix(action);
                continue;
            }
            probe = false;
            int capacity = static_cast<int>(known[majority_type()].size());
            if (pending.size() == 4 ||
                (!pending.empty() && remaining() > 2 * capacity)) {
                bulk_with_pending();
                continue;
            }
            if (pending.size() == 2) {
                ++answer;
                pending.clear();
            }
            bulk_step();
        }
        if (pending.size() == 2) ++answer;
        else if (!pending.empty()) resolve_with_endpoint();
        return answer;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2 || n > 20000) return 0;
    Machine machine;
    int answer;
    if (n < static_cast<int>(size(smallTrees))) {
        answer = solve_small(machine, smallTrees[n]);
    } else {
        vector<int> initial(n);
        iota(initial.begin(), initial.end(), 0);
        int changes = machine.ask(initial);
        answer = -1;
        if (n > CandidateSearch::limit || min(changes, n - 1 - changes) <= 1) {
            SparsePath sparse(machine);
            answer = sparse.try_count(n, changes);
        }
        if (answer < 0) {
            if (n <= CandidateSearch::limit) {
                CandidateSearch candidates(machine, n, changes);
                answer = candidates.run();
            } else {
                Counter counter(machine, n, changes);
                answer = counter.run();
            }
        }
    }
    cout << "! " << answer << '\n' << flush;
    return 0;
}

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

constexpr int maxN = 500;
constexpr int smallLimit = 12;
constexpr int enumerationLimit = 30;
constexpr int enumerationCapacity = 360000;
constexpr double epsilon = 1e-9;
using EdgeSet = vector<int>;
using CountQuery = function<int(const EdgeSet&)>;

[[noreturn]] void stop() {
    exit(0);
}

class Judge {
    int edges;

public:
    explicit Judge(int n) : edges(n - 1) {}

    int ask(int vertex, const string& bits) const {
        cout << "? " << vertex << ' ' << bits << '\n' << flush;
        int response;
        if (!(cin >> response) || response < 0 || response > edges) stop();
        return response;
    }

    void finish(const vector<pair<int, int>>& answer) const {
        cout << '!';
        for (auto [s, f] : answer) cout << ' ' << s << ' ' << f;
        cout << '\n' << flush;
    }
};

double binaryCost(int size) {
    if (size <= 1) return 0;
    int height = bit_width(static_cast<unsigned>(size - 1));
    return height + 1.0 - double(1 << height) / size;
}

vector<double> makeMassCosts(int limit) {
    vector<double> costs(limit + 1);
    for (int i = 2; i <= limit; ++i) costs[i] = i * log2(double(i));
    return costs;
}

void solveSmall(int n, const Judge& judge) {
    const int m = n - 1;
    const unsigned edgeMask = (1u << m) - 1;
    int rootWeight = m - judge.ask(1, string(m, '0'));
    vector<unsigned> orientations;
    for (unsigned mask = 0; mask <= edgeMask; ++mask) {
        if (popcount(mask) == rootWeight) orientations.push_back(mask);
    }
    vector<uint32_t> candidates;
    vector<int> p(n), distance(n), order(m), sequence(n);
    iota(order.begin(), order.end(), 1);

    auto generate = [&](auto&& self, int v) -> void {
        if (v < n) {
            for (int u = 0; u < n; ++u) {
                if (u == v) continue;
                p[v] = u;
                self(self, v + 1);
            }
            return;
        }
        for (int u = 1; u < n; ++u) {
            int a = u;
            int d = 0;
            while (a != 0 && d < n) {
                a = p[a];
                ++d;
            }
            if (a != 0) return;
            distance[u] = d;
        }

        array<int, 6> path{};
        iota(sequence.begin(), sequence.end(), 0);
        sort(sequence.begin(), sequence.end(),
             [&](int a, int b) { return distance[a] < distance[b]; });
        do {
            array<int, 6> incoming{};
            for (int i = 0; i < m; ++i) incoming[order[i]] = i;
            for (int i = 1; i < n; ++i) {
                int u = sequence[i];
                path[u] = path[p[u]] ^ (1 << incoming[u]);
            }
            for (unsigned orientation : orientations) {
                uint32_t code = 0;
                for (int u = 0; u < n; ++u) {
                    code |= (orientation ^ path[u]) << (u * m);
                }
                candidates.push_back(code);
            }
        } while (next_permutation(order.begin(), order.end()));
    };
    generate(generate, 1);

    const auto costs = makeMassCosts(int(candidates.size()));
    array<bool, 6> tested{true};
    array<int, 5> edgeClass{};
    while (candidates.size() > 1) {
        vector<unsigned> masks;
        for (unsigned mask = 0; mask < (1u << (m - 1)); ++mask) {
            bool representative = true;
            for (int i = 1; i < m; ++i) {
                if ((mask & (1u << i)) == 0) continue;
                for (int j = 0; j < i; ++j) {
                    if (edgeClass[i] == edgeClass[j] && (mask & (1u << j)) == 0) {
                        representative = false;
                    }
                }
            }
            if (representative) masks.push_back(mask);
        }

        double best = numeric_limits<double>::infinity();
        int bestMax = int(candidates.size());
        int chosenVertex = 0;
        unsigned chosenMask = 0;
        bool triedUntested = false;
        for (int v = 0; v < n; ++v) {
            if (!tested[v]) {
                if (triedUntested) continue;
                triedUntested = true;
            }
            int shift = v * m;
            for (unsigned mask : masks) {
                array<int, 6> hist{};
                for (uint32_t code : candidates) {
                    unsigned inward = (code >> shift) & edgeMask;
                    ++hist[popcount(inward ^ mask)];
                }
                double score = 0;
                int largest = 0;
                for (int count : hist) {
                    score += costs[count];
                    largest = max(largest, count);
                }
                if (score < best - epsilon ||
                    (abs(score - best) <= epsilon && largest < bestMax)) {
                    best = score;
                    bestMax = largest;
                    chosenVertex = v;
                    chosenMask = mask;
                }
            }
        }

        string bits(m, '0');
        array<int, 5> nextClass{};
        for (int i = 0; i < m; ++i) {
            unsigned bit = (chosenMask >> i) & 1u;
            if (bit) bits[i] = '1';
            nextClass[i] = i;
            for (int j = 0; j < i; ++j) {
                if (edgeClass[i] == edgeClass[j] && ((chosenMask >> j) & 1u) == bit) {
                    nextClass[i] = nextClass[j];
                    break;
                }
            }
        }
        edgeClass = nextClass;
        tested[chosenVertex] = true;
        int mismatches = m - judge.ask(chosenVertex + 1, bits);
        int shift = chosenVertex * m;
        erase_if(candidates, [&](uint32_t code) {
            return popcount(((code >> shift) & edgeMask) ^ chosenMask) != mismatches;
        });
        if (candidates.empty()) stop();
    }

    uint32_t code = candidates.front();
    vector<pair<int, int>> answer(m);
    for (int u = 0; u < n; ++u) {
        unsigned a = (code >> (u * m)) & edgeMask;
        for (int v = u + 1; v < n; ++v) {
            unsigned b = (code >> (v * m)) & edgeMask;
            unsigned diff = a ^ b;
            if (!has_single_bit(diff)) continue;
            int index = countr_zero(diff);
            answer[index] = (a & diff) ? pair{v + 1, u + 1} : pair{u + 1, v + 1};
        }
    }
    judge.finish(answer);
}

class SmallSubsetPlan {
    struct Node {
        int mask;
        array<int, smallLimit + 1> next;
    };

    int size;
    int root;
    vector<Node> nodes;
    vector<int> candidates;
    vector<vector<unsigned char>> responses;
    vector<double> costs;
    unordered_map<uint32_t, pair<int, int>> exact;

    pair<int, int> exactChoice(uint32_t state) {
        int count = popcount(state);
        if (count <= 1) return {0, 0};
        auto entry = exact.find(state);
        if (entry != exact.end()) return entry->second;
        int best = numeric_limits<int>::max();
        int chosen = 0;
        for (int mask = 1; mask < (1 << (size - 1)); ++mask) {
            array<uint32_t, smallLimit + 1> parts{};
            uint32_t left = state;
            while (left) {
                int i = countr_zero(left);
                left &= left - 1;
                parts[responses[mask][i]] |= 1u << i;
            }
            int total = count;
            for (uint32_t part : parts) {
                if (part == state) {
                    total = best;
                    break;
                }
                if (part) total += exactChoice(part).first;
                if (total >= best) break;
            }
            if (total < best) {
                best = total;
                chosen = mask;
            }
        }
        return exact.emplace(state, pair{best, chosen}).first->second;
    }

    pair<int, int> build(const vector<int>& current) {
        if (current.size() == 1) return {-1 - candidates[current.front()], 0};
        int chosen = 0;
        if (size <= 6) {
            uint32_t state = 0;
            for (int i : current) state |= 1u << i;
            chosen = exactChoice(state).second;
        } else {
            double best = numeric_limits<double>::infinity();
            int bestMax = int(current.size());
            for (int mask = 1; mask < (1 << (size - 1)); ++mask) {
                array<int, smallLimit + 1> hist{};
                for (int i : current) ++hist[responses[mask][i]];
                double score = 0;
                int largest = 0;
                for (int count : hist) {
                    score += costs[count];
                    largest = max(largest, count);
                }
                if (score < best - epsilon ||
                    (abs(score - best) <= epsilon && largest < bestMax)) {
                    best = score;
                    bestMax = largest;
                    chosen = mask;
                }
            }
        }
        array<vector<int>, smallLimit + 1> parts;
        for (int i : current) parts[responses[chosen][i]].push_back(i);
        Node node;
        node.mask = chosen;
        node.next.fill(numeric_limits<int>::min());
        int totalCost = int(current.size());
        for (int count = 0; count <= size; ++count) {
            if (parts[count].empty()) continue;
            auto [next, cost] = build(parts[count]);
            node.next[count] = next;
            totalCost += cost;
        }
        int id = int(nodes.size());
        nodes.push_back(node);
        return {id, totalCost};
    }

public:
    double average;

    SmallSubsetPlan(int n, int k) : size(n) {
        for (int mask = 0; mask < (1 << n); ++mask) {
            if (popcount(unsigned(mask)) == k) candidates.push_back(mask);
        }
        costs = makeMassCosts(int(candidates.size()));
        responses.resize(1 << (n - 1), vector<unsigned char>(candidates.size()));
        for (int mask = 1; mask < int(responses.size()); ++mask) {
            for (int i = 0; i < int(candidates.size()); ++i) {
                responses[mask][i] = popcount(unsigned(mask & candidates[i]));
            }
        }
        vector<int> all(candidates.size());
        iota(all.begin(), all.end(), 0);
        auto [first, total] = build(all);
        root = first;
        average = double(total) / all.size();
        responses.clear();
        exact.clear();
    }

    EdgeSet recover(const EdgeSet& pool, const CountQuery& query) const {
        int node = root;
        while (node >= 0) {
            EdgeSet selected;
            for (int j = 0; j < size; ++j) {
                if ((nodes[node].mask >> j) & 1) selected.push_back(pool[j]);
            }
            int count = query(selected);
            if (count < 0 || count > size) stop();
            node = nodes[node].next[count];
            if (node == numeric_limits<int>::min()) stop();
        }
        int mask = -1 - node;
        EdgeSet result;
        for (int j = 0; j < size; ++j) if ((mask >> j) & 1) result.push_back(pool[j]);
        return result;
    }
};

class DetectingMatrix {
    vector<vector<int>> rowColumns;
    vector<vector<int>> columnRows;
    vector<vector<int>> groupColumns;

public:
    DetectingMatrix(int columns, int rows)
        : rowColumns(rows), columnRows(columns), groupColumns(rows) {
        groupColumns[0].push_back(0);
        for (int row = 0; row < rows; ++row) {
            rowColumns[row].push_back(0);
            columnRows[0].push_back(row);
        }
        int column = 1;
        for (int group = 1; group < rows && column < columns; ++group) {
            vector<int> even;
            for (int sub = group;; sub = (sub - 1) & group) {
                if (popcount(unsigned(sub)) % 2 == 0) even.push_back(sub);
                if (sub == 0) break;
            }
            reverse(even.begin(), even.end());

            array<int, maxN> rank;
            rank.fill(maxN);
            for (int i = 0; i < int(even.size()); ++i) rank[even[i]] = i;
            for (int length = 1; length <= int(even.size()) && column < columns; length *= 2) {
                groupColumns[group].push_back(column);
                for (int row = 0; row < rows; ++row) {
                    if (rank[row & group] >= length) continue;
                    rowColumns[row].push_back(column);
                    columnRows[column].push_back(row);
                }
                ++column;
            }
        }
    }

    EdgeSet recover(const EdgeSet& pool, int weight, const CountQuery& query) const {
        int rows = int(rowColumns.size());
        vector<int> sums(rows);
        sums[0] = weight;
        for (int row = 1; row < rows; ++row) {
            EdgeSet selected;
            selected.reserve(rowColumns[row].size());
            for (int col : rowColumns[row]) selected.push_back(pool[col]);
            sums[row] = query(selected);
        }

        EdgeSet result;
        // The alternating sum over submasks cancels columns lacking any bit
        // of this group. Its remaining coefficients are distinct powers of two.
        for (int group = rows - 1; group >= 0; --group) {
            if (groupColumns[group].empty()) continue;
            int encoded = 0;
            for (int sub = group;; sub = (sub - 1) & group) {
                encoded += popcount(unsigned(sub)) % 2 ? -sums[sub] : sums[sub];
                if (sub == 0) break;
            }
            if (encoded < 0 || encoded >= (1 << groupColumns[group].size())) stop();
            for (int col : groupColumns[group]) {
                if (encoded & 1) {
                    result.push_back(pool[col]);
                    for (int row : columnRows[col]) --sums[row];
                }
                encoded >>= 1;
            }
        }
        sort(result.begin(), result.end());
        return result;
    }
};

EdgeSet recoverEnumeratedSubset(const EdgeSet& pool, int weight, const CountQuery& query) {
    int n = int(pool.size());
    unsigned all = (1u << n) - 1;
    vector<unsigned> candidates;
    for (unsigned mask = (1u << weight) - 1; mask <= all;) {
        candidates.push_back(mask);
        unsigned low = mask & -mask;
        unsigned next = mask + low;
        mask = next | (((next ^ mask) >> 2) / low);
    }

    mt19937 random(23076233u + 71u * n + weight);
    constexpr int sampleLimit = 512;
    const auto costs = makeMassCosts(min(sampleLimit, int(candidates.size())));
    unsigned selected = (1u << (n / 2)) - 1;
    vector<unsigned> sample;
    while (candidates.size() > 1) {
        EdgeSet edges;
        for (int j = 0; j < n; ++j) {
            if ((selected >> j) & 1u) edges.push_back(pool[j]);
        }
        int answer = query(edges);
        erase_if(candidates, [&](unsigned mask) { return popcount(mask & selected) != answer; });
        if (candidates.empty()) stop();
        if (candidates.size() == 1) break;

        unsigned known = all;
        unsigned possible = 0;
        for (unsigned mask : candidates) {
            known &= mask;
            possible |= mask;
        }
        unsigned varying = known ^ possible;
        unsigned eligible = varying & (varying - 1);

        if (candidates.size() <= sampleLimit) {
            sample = candidates;
        } else {
            sample.resize(sampleLimit);
            for (int i = 0; i < sampleLimit; ++i) {
                size_t index = (candidates.size() * i + random() % candidates.size()) / sampleLimit;
                sample[i] = candidates[index];
            }
        }
        double best = costs[sample.size()];
        auto test = [&](unsigned mask) {
            array<int, enumerationLimit + 1> hist{};
            for (unsigned candidate : sample) ++hist[popcount(candidate & mask)];
            double score = 0;
            for (int i = 0; i <= weight; ++i) score += costs[hist[i]];
            if (score < best - epsilon) {
                best = score;
                selected = mask;
            }
        };
        if (popcount(eligible) <= 10 && sample.size() == candidates.size()) {
            for (unsigned mask = eligible; mask; mask = (mask - 1) & eligible) test(mask);
        } else {
            for (int i = 0; i < 64; ++i) test(random() & eligible);
            for (unsigned bits = eligible; bits; bits &= bits - 1) test(bits & -bits);
        }
    }

    EdgeSet result;
    for (int j = 0; j < n; ++j) {
        if ((candidates.front() >> j) & 1u) result.push_back(pool[j]);
    }
    return result;
}

class PoolPlanner {
    vector<int> rows;
    vector<vector<double>> subsetCost;
    vector<vector<int>> subsetSplit;
    vector<double> permutationCost;
    vector<int> permutationSplit;
    vector<double> binaryPrefix;
    vector<unique_ptr<DetectingMatrix>> matrices;
    array<array<unique_ptr<SmallSubsetPlan>, smallLimit / 2 + 1>, smallLimit + 1> small;

    EdgeSet recoverParts(const EdgeSet& pool, const EdgeSet& left, int weight,
                         int leftWeight, const CountQuery& query) {
        EdgeSet right(pool.begin() + left.size(), pool.end());
        EdgeSet result = recover(left, leftWeight, query);
        EdgeSet rest = recover(right, weight - leftWeight, query);
        result.insert(result.end(), rest.begin(), rest.end());
        return result;
    }

public:
    explicit PoolPlanner(int m)
        : rows(m + 1), subsetCost(m + 1), subsetSplit(m + 1),
          permutationCost(m + 1), permutationSplit(m + 1), binaryPrefix(m + 1),
          matrices(m + 1) {
        int rowCount = 1;
        int capacity = 1;
        for (int n = 1; n <= m; ++n) {
            if (capacity < n) capacity += popcount(unsigned(rowCount++));
            rows[n] = rowCount;
            binaryPrefix[n] = binaryPrefix[n - 1] + binaryCost(n);
        }
        vector<vector<double>> binomial(m + 1);
        for (int n = 0; n <= m; ++n) {
            binomial[n].assign(n + 1, 1);
            for (int k = 1; k < n; ++k) binomial[n][k] = binomial[n - 1][k - 1] + binomial[n - 1][k];
            subsetCost[n].resize(n + 1);
            subsetSplit[n].resize(n + 1);
        }

        for (int n = 2; n <= m; ++n) {
            for (int k = 1; k <= n / 2; ++k) {
                double best = rows[n] - 1;
                int chosen = 0;
                if (k == 1) {
                    best = binaryCost(n);
                    chosen = n / 2;
                } else if (n <= smallLimit) {
                    small[n][k] = make_unique<SmallSubsetPlan>(n, k);
                    best = small[n][k]->average;
                    chosen = -1;
                } else {
                    if (n <= enumerationLimit && binomial[n][k] <= enumerationCapacity) {
                        int s = n / 2;
                        double entropy = 0;
                        for (int x = max(0, k - (n - s)); x <= min(k, s); ++x) {
                            double probability = binomial[s][x] * binomial[n - s][k - x] /
                                                 binomial[n][k];
                            entropy -= probability * log2(probability);
                        }
                        double expected = log2(binomial[n][k]) / entropy + 0.25;
                        if (expected < best) {
                            best = expected;
                            chosen = -2;
                        }
                    }
                    array<bool, maxN> trySplit{};
                    if (n <= 48) {
                        fill(trySplit.begin() + 1, trySplit.begin() + n / 2 + 1, true);
                    } else {
                        for (int s = 1; s <= 7; ++s) trySplit[s] = true;
                        for (int d : {10, 8, 6, 5, 4, 3, 2}) trySplit[n / d] = true;
                        trySplit[n * 2 / 5] = true;
                        for (int multiple : {1, 2, 3, 4, 6, 8}) {
                            int s = n * multiple / (2 * k);
                            trySplit[clamp(s, 1, n / 2)] = true;
                        }
                    }
                    for (int s = 1; s <= n / 2; ++s) {
                        if (!trySplit[s]) continue;
                        double sum = 0;
                        for (int x = max(0, k - (n - s)); x <= min(k, s); ++x) {
                            double cases = binomial[s][x] * binomial[n - s][k - x];
                            sum += cases * (subsetCost[s][x] + subsetCost[n - s][k - x]);
                        }
                        double average = 1 + sum / binomial[n][k];
                        if (average < best - epsilon) {
                            best = average;
                            chosen = s;
                        }
                    }
                }
                subsetCost[n][k] = subsetCost[n][n - k] = best;
                subsetSplit[n][k] = subsetSplit[n][n - k] = chosen;
            }

            permutationCost[n] = numeric_limits<double>::infinity();
            for (int pivot = 1; pivot <= n / 2; ++pivot) {
                double cost = permutationCost[pivot] + permutationCost[n - pivot] + subsetCost[n][pivot];
                if (cost < permutationCost[n] - epsilon) {
                    permutationCost[n] = cost;
                    permutationSplit[n] = pivot;
                }
            }
        }
    }

    double chainCost(int pool, int length) const {
        return subsetCost[pool][length] + permutationCost[length];
    }

    double individualCost(int pool, int length) const {
        return binaryPrefix[pool] - binaryPrefix[pool - length];
    }

    int pivot(int length) const {
        return permutationSplit[length];
    }

    EdgeSet recoverWithSplit(const EdgeSet& pool, int weight, int cut, int leftWeight,
                             const CountQuery& query) {
        int right = int(pool.size()) - cut;
        int rightWeight = weight - leftWeight;
        if (leftWeight < 0 || leftWeight > cut || rightWeight < 0 || rightWeight > right) stop();
        if (subsetCost[cut][leftWeight] + subsetCost[right][rightWeight] <
            subsetCost[pool.size()][weight] - epsilon) {
            EdgeSet left(pool.begin(), pool.begin() + cut);
            return recoverParts(pool, left, weight, leftWeight, query);
        }
        return recover(pool, weight, query);
    }

    EdgeSet recover(const EdgeSet& pool, int weight, const CountQuery& query) {
        int n = int(pool.size());
        if (weight < 0 || weight > n) stop();
        if (weight == 0) return {};
        if (weight == n) return pool;

        if (weight > n - weight) {
            auto complementQuery = [&](const EdgeSet& selected) {
                return int(selected.size()) - query(selected);
            };
            EdgeSet opposite = recover(pool, n - weight, complementQuery);
            EdgeSet result;
            set_difference(pool.begin(), pool.end(), opposite.begin(), opposite.end(),
                           back_inserter(result));
            return result;
        }
        if (weight == 1) {
            auto left = pool.begin();
            auto right = pool.end();
            while (right - left > 1) {
                auto mid = left + (right - left) / 2;
                int count = query(EdgeSet(left, mid));
                if (count == 0) left = mid;
                else if (count == 1) right = mid;
                else stop();
            }
            return {*left};
        }

        int cut = subsetSplit[n][weight];
        if (cut == -1) return small[n][weight]->recover(pool, query);
        if (cut == -2) return recoverEnumeratedSubset(pool, weight, query);
        if (cut == 0) {
            if (!matrices[n]) matrices[n] = make_unique<DetectingMatrix>(n, rows[n]);
            return matrices[n]->recover(pool, weight, query);
        }
        EdgeSet left(pool.begin(), pool.begin() + cut);
        int leftWeight = query(left);
        return recoverParts(pool, left, weight, leftWeight, query);
    }
};

struct PairBlock {
    // Within a block every parent can be paired with every edge.
    vector<int> parents;
    EdgeSet edges;
};

struct Branch {
    int above;
    EdgeSet edges;
    vector<int> leaves;
};

struct PairQuery {
    EdgeSet selected;
    vector<int> parentCounts;
    vector<int> cuts;
};

class PairPartition {
    static constexpr double imbalancePenalty = 0.35;

    const vector<PairBlock>& blocks;
    const vector<double>& costs;
    mt19937& random;
    vector<int> vertices;
    vector<int> start;
    vector<int> blockOf;
    vector<Branch> branches;
    int total = 0;

    vector<int> amounts;
    vector<int> cuts;
    vector<int> potential;
    vector<int> hist;
    vector<int> scratch;

    double cutCost(int block, int cut) const {
        int size = int(blocks[block].edges.size());
        return imbalancePenalty * blocks[block].parents.size() *
               (costs[cut] + costs[size - cut] - costs[size / 2] - costs[(size + 1) / 2]);
    }

    void initialize(int trial) {
        fill(potential.begin(), potential.end(), 0);
        fill(hist.begin(), hist.end(), 0);
        for (int i = 0; i < int(branches.size()); ++i) {
            int length = int(branches[i].edges.size());
            amounts[i] = 0;
            if (trial == 1) amounts[i] = (random() & 1u) ? length : 0;
            if (trial == 2) amounts[i] = int(random() % (length + 1));
            for (int leaf : branches[i].leaves) potential[leaf] += amounts[i];
        }
        for (int i = 0; i < int(blocks.size()); ++i) {
            int size = int(blocks[i].edges.size());
            cuts[i] = size / 2;
            if (trial == 1) cuts[i] = (random() & 1u) ? size : 0;
            if (trial == 2) cuts[i] = int(random() % (size + 1));
            for (int leaf = start[i]; leaf < start[i + 1]; ++leaf) {
                hist[potential[leaf]] += size - cuts[i];
                hist[potential[leaf] + 1] += cuts[i];
            }
        }
    }

    bool shiftBranch(int i) {
        int lo = int(hist.size());
        int hi = 0;
        for (int leaf : branches[i].leaves) {
            int block = blockOf[leaf];
            int value = potential[leaf];
            int cut = cuts[block];
            scratch[value] += int(blocks[block].edges.size()) - cut;
            scratch[value + 1] += cut;
            lo = min(lo, value);
            hi = max(hi, value + 1);
        }
        for (int value = lo; value <= hi; ++value) hist[value] -= scratch[value];

        auto score = [&](int delta) {
            double sum = 0;
            for (int value = lo; value <= hi; ++value) {
                if (scratch[value] == 0) continue;
                int count = hist[value + delta];
                sum += costs[count + scratch[value]] - costs[count];
            }
            return sum;
        };
        double best = score(0);
        int old = amounts[i];
        int chosen = old;
        for (int amount = 0; amount <= int(branches[i].edges.size()); ++amount) {
            if (amount == old) continue;
            double candidate = score(amount - old);
            if (candidate < best - epsilon) {
                best = candidate;
                chosen = amount;
            }
        }

        int delta = chosen - old;
        for (int value = lo; value <= hi; ++value) {
            if (scratch[value]) hist[value + delta] += scratch[value];
            scratch[value] = 0;
        }
        if (delta == 0) return false;
        amounts[i] = chosen;
        for (int leaf : branches[i].leaves) potential[leaf] += delta;
        return true;
    }

    bool balanceBlock(int block) {
        int lo = int(hist.size());
        int hi = 0;
        for (int leaf = start[block]; leaf < start[block + 1]; ++leaf) {
            int value = potential[leaf];
            --scratch[value];
            ++scratch[value + 1];
            lo = min(lo, value);
            hi = max(hi, value + 1);
        }

        int old = cuts[block];
        auto score = [&](int cut) {
            double sum = cutCost(block, cut);
            for (int value = lo; value <= hi; ++value) {
                sum += costs[hist[value] + (cut - old) * scratch[value]];
            }
            return sum;
        };
        int left = 0;
        int right = int(blocks[block].edges.size());
        while (left < right) {
            int mid = (left + right) / 2;
            if (score(mid) <= score(mid + 1)) right = mid;
            else left = mid + 1;
        }

        int chosen = old;
        if (score(left) < score(old) - epsilon) chosen = left;
        for (int value = lo; value <= hi; ++value) {
            hist[value] += (chosen - old) * scratch[value];
            scratch[value] = 0;
        }
        cuts[block] = chosen;
        return chosen != old;
    }

public:
    PairPartition(const vector<PairBlock>& options, const vector<int>& parent,
                  const vector<int>& incoming, const vector<int>& depth,
                  const vector<double>& massCosts, mt19937& rng)
        : blocks(options), costs(massCosts), random(rng) {
        start.push_back(0);
        for (int i = 0; i < int(blocks.size()); ++i) {
            for (int v : blocks[i].parents) {
                vertices.push_back(v);
                blockOf.push_back(i);
            }
            total += int(blocks[i].parents.size() * blocks[i].edges.size());
            start.push_back(int(vertices.size()));
        }

        array<bool, maxN + 1> active{};
        array<int, maxN + 1> children{}, onlyChild{}, group{};
        vector<int> subtree;
        for (int v : vertices) {
            for (int u = v; u != 1 && !active[u]; u = parent[u]) {
                active[u] = true;
                subtree.push_back(u);
                ++children[parent[u]];
                onlyChild[parent[u]] = u;
            }
        }
        int common = 1;
        while (children[common] == 1) common = onlyChild[common];
        sort(subtree.begin(), subtree.end(),
             [&](int a, int b) { return depth[a] < depth[b]; });
        for (int u : subtree) {
            // Edges on a unary branch affect exactly the same candidate parents.
            if (depth[u] <= depth[common]) continue;
            int p = parent[u];
            if (p == common || children[p] > 1) {
                group[u] = int(branches.size());
                branches.push_back({p == common ? -1 : group[p], {}, {}});
            } else {
                group[u] = group[p];
            }
            branches[group[u]].edges.push_back(incoming[u]);
        }
        for (int leaf = 0; leaf < int(vertices.size()); ++leaf) {
            for (int g = group[vertices[leaf]]; g != -1; g = branches[g].above) {
                branches[g].leaves.push_back(leaf);
            }
        }

        int bins = depth[vertices.front()] - depth[common] + 2;
        hist.resize(bins);
        scratch.resize(bins);
        cuts.resize(blocks.size());
        amounts.resize(branches.size());
        potential.resize(vertices.size());
    }

    PairQuery choose(const vector<int>& incoming) {
        double best = numeric_limits<double>::infinity();
        int bestLargest = total;
        PairQuery result;
        vector<int> bestAmounts;
        for (int trial = 0; trial < 3; ++trial) {
            initialize(trial);
            for (int pass = 0; pass < 3; ++pass) {
                bool change = false;
                for (int i = int(branches.size()) - 1; i >= 0; --i) change |= shiftBranch(i);
                for (int i = 0; i < int(blocks.size()); ++i) change |= balanceBlock(i);
                if (!change) break;
            }
            double score = 0;
            int largest = 0;
            for (int count : hist) {
                score += costs[count];
                largest = max(largest, count);
            }
            for (int i = 0; i < int(blocks.size()); ++i) score += cutCost(i, cuts[i]);
            if (score < best - epsilon ||
                (abs(score - best) <= epsilon && largest < bestLargest)) {
                best = score;
                bestLargest = largest;
                bestAmounts = amounts;
                result.cuts = cuts;
                result.parentCounts = potential;
            }
            if (best <= epsilon) break;
        }

        if (bestLargest == total) {
            result.selected = {incoming[vertices.front()]};
            result.parentCounts.assign(vertices.size(), 0);
            result.parentCounts.front() = 1;
            result.cuts.assign(blocks.size(), 0);
            return result;
        }
        for (int i = 0; i < int(branches.size()); ++i) {
            const auto& edges = branches[i].edges;
            result.selected.insert(result.selected.end(), edges.begin(), edges.begin() + bestAmounts[i]);
        }
        for (int i = 0; i < int(blocks.size()); ++i) {
            const auto& edges = blocks[i].edges;
            result.selected.insert(result.selected.end(), edges.begin(), edges.begin() + result.cuts[i]);
        }
        return result;
    }
};

class Solver {
    using Layers = vector<vector<int>>;

    int n;
    int m;
    const Judge& judge;
    PoolPlanner planner;
    vector<double> massCosts;
    mt19937 random{52736279};
    string inward;
    vector<int> depth;
    vector<int> parent;
    vector<int> incoming;
    vector<int> signedProbe;
    vector<int> probePrefix;
    vector<bool> used;
    int remaining;
    vector<pair<int, int>> answer;

    EdgeSet available() const {
        EdgeSet edges;
        edges.reserve(remaining);
        for (int i = 0; i < m; ++i) if (!used[i]) edges.push_back(i);
        return edges;
    }

    int countOnPath(int v, const EdgeSet& edges) const {
        if (edges.empty()) return 0;
        string mask = inward;
        for (int e : edges) mask[e] ^= '0' ^ '1';
        // A flip loses one match off the root path and gains one on it.
        int twice = judge.ask(v, mask) - (m - depth[v]) + int(edges.size());
        if (twice < 0 || (twice & 1) || twice > 2 * int(edges.size())) stop();
        return twice / 2;
    }

    EdgeSet recoverPathSet(int anchor, int tip, const EdgeSet& pool, int length) {
        auto query = [&](const EdgeSet& selected) { return countOnPath(tip, selected); };
        if (signedProbe[tip] != m + 1) {
            int cut = int(lower_bound(pool.begin(), pool.end(), m / 2) - pool.begin());
            int pathCount = (depth[tip] - signedProbe[tip]) / 2 - probePrefix[anchor];
            return planner.recoverWithSplit(pool, length, cut, pathCount, query);
        }
        return planner.recover(pool, length, query);
    }

    bool probeStars() {
        string mask = inward;
        for (int e = 0; e < m / 2; ++e) mask[e] ^= '0' ^ '1';
        vector<int> vertices(m);
        iota(vertices.begin(), vertices.end(), 2);
        shuffle(vertices.begin(), vertices.end(), random);
        int even = 1;
        int odd = 0;
        for (int v : vertices) {
            int distance = m - m / 2 - judge.ask(v, mask);
            signedProbe[v] = distance;
            if (distance % 2 == 0) ++even;
            else ++odd;
            if (abs(distance) > 2 || (even > 1 && odd > 1)) return false;
        }
        // A tree with one vertex in a bipartition class is a star.
        for (int v = 2; v <= n; ++v) {
            depth[v] = signedProbe[v] % 2 == 0 ? 2 : 1;
        }
        return true;
    }

    void setEdge(int v, int p, int e) {
        parent[v] = p;
        incoming[v] = e;
        probePrefix[v] = probePrefix[p] + (e < m / 2);
        if (used[e]) stop();
        used[e] = true;
        --remaining;
        answer[e] = inward[e] == '1' ? pair{v, p} : pair{p, v};
    }

    void orientRoot() {
        int weight = judge.ask(1, string(m, '1'));
        auto query = [&](const EdgeSet& selected) {
            string mask(m, '0');
            for (int e : selected) mask[e] = '1';
            int twice = judge.ask(1, mask) - (m - weight) + int(selected.size());
            if (twice < 0 || (twice & 1) || twice > 2 * int(selected.size())) stop();
            return twice / 2;
        };
        inward.assign(m, '0');
        for (int e : planner.recover(available(), weight, query)) inward[e] = '1';
    }

    void learnPair(int v, const vector<int>& parents) {
        EdgeSet pool = available();
        vector<PairBlock> blocks;
        if (signedProbe[v] == m + 1) {
            blocks.push_back({parents, move(pool)});
        } else {
            int count = (depth[v] - signedProbe[v]) / 2;
            for (int bit = 0; bit <= 1; ++bit) {
                PairBlock block;
                for (int p : parents) {
                    if (probePrefix[p] + bit == count) block.parents.push_back(p);
                }
                if (block.parents.empty()) continue;
                for (int e : pool) {
                    if ((e < m / 2) == bit) block.edges.push_back(e);
                }
                if (!block.edges.empty()) blocks.push_back(move(block));
            }
        }
        while (true) {
            if (blocks.empty()) stop();
            if (blocks.size() == 1 && blocks.front().parents.size() == 1) {
                const auto& block = blocks.front();
                auto query = [&](const EdgeSet& selected) { return countOnPath(v, selected); };
                int e = planner.recover(block.edges, 1, query).front();
                setEdge(v, block.parents.front(), e);
                return;
            }

            PairPartition partition(blocks, parent, incoming, depth, massCosts, random);
            PairQuery query = partition.choose(incoming);
            int count = countOnPath(v, query.selected);
            vector<PairBlock> next;
            int leaf = 0;
            for (int i = 0; i < int(blocks.size()); ++i) {
                const auto& block = blocks[i];
                int cut = query.cuts[i];
                for (int bit = 0; bit <= 1; ++bit) {
                    auto mid = block.edges.begin() + cut;
                    auto first = bit ? block.edges.begin() : mid;
                    auto last = bit ? mid : block.edges.end();
                    if (first == last) continue;
                    vector<int> compatible;
                    for (int j = 0; j < int(block.parents.size()); ++j) {
                        if (query.parentCounts[leaf + j] + bit == count) {
                            compatible.push_back(block.parents[j]);
                        }
                    }
                    if (!compatible.empty()) {
                        next.push_back({move(compatible), EdgeSet(first, last)});
                    }
                }
                leaf += int(block.parents.size());
            }
            blocks = move(next);
        }
    }

    void orderChain(const vector<int>& chain, int start, const EdgeSet& edges) {
        int length = int(edges.size());
        if (length == 1) {
            setEdge(chain[start + 1], chain[start], edges.front());
            return;
        }
        int leftLength = planner.pivot(length);
        EdgeSet left = recoverPathSet(chain[start], chain[start + leftLength], edges, leftLength);
        EdgeSet right;
        set_difference(edges.begin(), edges.end(), left.begin(), left.end(), back_inserter(right));
        orderChain(chain, start, left);
        orderChain(chain, start + leftLength, right);
    }

    int singletonRun(const Layers& layers, int first) const {
        if (layers[first - 1].size() != 1) return 0;
        int past = first;
        while (past < int(layers.size()) && layers[past].size() == 1) ++past;
        return past - first;
    }

    bool useLandmark(const Layers& layers) const {
        int height = int(layers.size()) - 1;
        if (height < 5 || singletonRun(layers, 1) == height) return false;

        double identificationCost = 0;
        double extraParentInfo = 0;
        for (int d = 1; d <= height; ++d) {
            int width = int(layers[d].size());
            int previous = int(layers[d - 1].size());
            if (width > 1) {
                if (d == height) identificationCost += previous == 1 ? 0 : width - 1;
                else if (previous == 1) identificationCost += (width + 1) / 2.0 - 1.0 / width;
                else identificationCost += width - 1.0 / width;
            }
            extraParentInfo += 0.25 * log2(double(previous));
        }
        double landmarkGain = planner.individualCost(remaining, height) -
                              planner.chainCost(remaining, height) -
                              identificationCost + extraParentInfo;

        int pool = remaining;
        double forcedGain = 0;
        for (int d = 1; d <= height;) {
            int run = singletonRun(layers, d);
            if (run > 1) {
                forcedGain += max(0.0, planner.individualCost(pool, run) - planner.chainCost(pool, run));
                pool -= run;
                d += run;
            } else {
                pool -= int(layers[d].size());
                ++d;
            }
        }
        return landmarkGain > forcedGain + 2;
    }

    void learnLandmark(const Layers& layers) {
        int height = int(layers.size()) - 1;
        int tip = layers.back().front();
        EdgeSet path = recoverPathSet(layers[0].front(), tip, available(), height);

        vector<int> chain{layers[0].front()};
        vector<Layers> hanging(height);
        vector<int> sizes(height);
        for (int d = 1; d <= height; ++d) {
            const auto& layer = layers[d];
            int onPath = d == height ? tip : 0;
            bool fixedAttachment = layers[d - 1].size() == 1;
            for (int j = 0; j < int(layer.size()); ++j) {
                int v = layer[j];
                if (v == onPath) continue;
                if (onPath == 0 && j + 1 == int(layer.size())) {
                    onPath = v;
                    continue;
                }
                // The selected landmark edges form the common prefix of the two paths.
                int attachment = (onPath != 0 && fixedAttachment) ? d - 1 : countOnPath(v, path);
                if (attachment == d) {
                    onPath = v;
                } else {
                    if (attachment >= d) stop();
                    auto& group = hanging[attachment];
                    if (int(group.size()) <= d - attachment) group.resize(d - attachment + 1);
                    group[d - attachment].push_back(v);
                    ++sizes[attachment];
                }
            }
            chain.push_back(onPath);
        }
        orderChain(chain, 0, path);

        vector<int> anchors;
        for (int h = 0; h < height; ++h) {
            if (sizes[h] > 0) anchors.push_back(h);
        }
        sort(anchors.begin(), anchors.end(), [&](int a, int b) {
            if (hanging[a].size() != hanging[b].size()) return hanging[a].size() > hanging[b].size();
            return sizes[a] > sizes[b];
        });
        for (int h : anchors) {
            hanging[h][0].push_back(chain[h]);
            learnGroup(hanging[h]);
        }
    }

    void learnGroup(const Layers& layers) {
        if (useLandmark(layers)) {
            learnLandmark(layers);
            return;
        }
        for (int d = 1; d < int(layers.size());) {
            int run = singletonRun(layers, d);
            double bestGain = 0;
            int length = 0;
            for (int k = 2; k <= run; ++k) {
                double gain = planner.individualCost(remaining, k) - planner.chainCost(remaining, k);
                if (gain > bestGain + epsilon) {
                    bestGain = gain;
                    length = k;
                }
            }
            if (length > 0) {
                vector<int> chain;
                for (int k = d - 1; k < d + length; ++k) chain.push_back(layers[k].front());
                EdgeSet edges = recoverPathSet(chain.front(), chain.back(), available(), length);
                orderChain(chain, 0, edges);
                d += length;
            } else {
                for (int v : layers[d]) learnPair(v, layers[d - 1]);
                ++d;
            }
        }
    }

public:
    Solver(int size, const Judge& interaction)
        : n(size), m(n - 1), judge(interaction), planner(m),
          massCosts(makeMassCosts(n * n)), depth(n + 1), parent(n + 1),
          incoming(n + 1, -1), signedProbe(n + 1, m + 1), probePrefix(n + 1),
          used(m), remaining(m), answer(m) {}

    void run() {
        orientRoot();
        bool isStar = probeStars();
        Layers layers{{1}};
        for (int v = 2; v <= n; ++v) {
            if (!isStar) depth[v] = m - judge.ask(v, inward);
            if (depth[v] == 0) stop();
            if (int(layers.size()) <= depth[v]) layers.resize(depth[v] + 1);
            layers[depth[v]].push_back(v);
        }
        learnGroup(layers);
        if (remaining != 0) stop();
        judge.finish(answer);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2 || n > maxN) return 0;
    Judge judge(n);
    if (n <= 6) solveSmall(n, judge);
    else Solver(n, judge).run();
}

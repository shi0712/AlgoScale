#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

struct Query {
    int row;
    int col;
};

class Interaction {
public:
    explicit Interaction(int size) : n(size), maskSize(bit_ceil(unsigned(size))) {
        replies.reserve(2 * n);
    }

    int ask(int i, int j) {
        unsigned key = unsigned(i * n + j);
        auto found = replies.find(key);
        if (found != replies.end()) return found->second;
        cout << "? " << i << ' ' << j << '\n' << flush;
        int answer;
        if (!(cin >> answer) || answer < 0 || answer >= maskSize) exit(0);
        replies.emplace(key, answer);
        return answer;
    }

    void report(const vector<int>& p) const {
        // A second representative must XOR both arrays by the same shift.
        // The shifts preserving [0, n) are exactly those below its low bit.
        int count = 0;
        for (int shift = 0; shift < (n & -n); ++shift) {
            int i = 0;
            while (i < n && p[i ^ shift] == (p[i] ^ shift)) ++i;
            count += i == n;
        }
        cout << "!\n" << flush;
        cout << count << '\n' << flush;
        for (int i = 0; i < n; ++i) {
            if (i) cout << ' ';
            cout << p[i];
        }
        cout << '\n' << flush;
    }

    const int n;
    const int maskSize;

private:
    unordered_map<unsigned, int> replies;
};

struct PackedPermutation {
    uint64_t p = 0;
    uint64_t b = 0;
    int shift = 0;
};

int digit(uint64_t packed, int i) {
    return (packed >> (4 * i)) & 15;
}

PackedPermutation pack(const vector<int>& p, const vector<int>& b, int shift) {
    PackedPermutation candidate;
    candidate.shift = shift;
    for (int i = 0; i < int(p.size()); ++i) {
        candidate.p |= uint64_t(p[i] ^ shift) << (4 * i);
        candidate.b |= uint64_t(b[i] ^ shift) << (4 * i);
    }
    return candidate;
}

void removeEquivalent(vector<PackedPermutation>& candidates) {
    ranges::sort(candidates, {}, [](const auto& candidate) {
        return pair{candidate.p, candidate.b};
    });
    candidates.erase(unique(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
        return a.p == b.p && a.b == b.b;
    }), candidates.end());
}

vector<PackedPermutation> generatePermutations(int size, bool normalize) {
    vector<int> p(size), b(size);
    iota(p.begin(), p.end(), 0);
    int factorial = 1;
    for (int i = 2; i <= size; ++i) factorial *= i;
    vector<PackedPermutation> result;
    result.reserve(factorial);
    do {
        for (int i = 0; i < size; ++i) b[p[i]] = i;
        result.push_back(pack(p, b, normalize ? b[0] : 0));
    } while (next_permutation(p.begin(), p.end()));
    if (normalize) removeEquivalent(result);
    return result;
}

// For small posteriors, optimize the complete decision tree. Otherwise use
// the expected number of surviving possibilities to rank the next queries.
template <class Reply, class Ask>
PackedPermutation identifySmall(vector<PackedPermutation> candidates,
                                const vector<Query>& queries, int maskSize,
                                Reply reply, Ask ask) {
    vector<int> frequency(maskSize);
    while (candidates.size() > 1) {
        int chosen = -1;
        if (candidates.size() <= 15) {
            vector<vector<unsigned>> partitions;
            vector<int> queryIndex;
            vector<unsigned> buckets(maskSize);
            vector<int> touched;
            for (int k = 0; k < int(queries.size()); ++k) {
                for (int i = 0; i < int(candidates.size()); ++i) {
                    int value = reply(candidates[i], queries[k]);
                    if (!buckets[value]) touched.push_back(value);
                    buckets[value] |= 1U << i;
                }
                if (touched.size() > 1) {
                    queryIndex.push_back(k);
                    partitions.emplace_back();
                    for (int value : touched) partitions.back().push_back(buckets[value]);
                }
                for (int value : touched) buckets[value] = 0;
                touched.clear();
            }
            unsigned full = (1U << candidates.size()) - 1;
            vector<int> cost(full + 1, -1), depth(full + 1), bestQuery(full + 1);
            auto solve = [&](auto&& self, unsigned mask) -> int {
                int size = popcount(mask);
                if (size <= 1) return 0;
                if (cost[mask] >= 0) return cost[mask];
                int best = numeric_limits<int>::max();
                for (int k = 0; k < int(partitions.size()); ++k) {
                    int sum = size;
                    int worst = 0;
                    for (unsigned bucket : partitions[k]) {
                        unsigned sub = bucket & mask;
                        if (sub == mask) {
                            sum = numeric_limits<int>::max();
                            break;
                        }
                        if (sub) {
                            sum += self(self, sub);
                            worst = max(worst, depth[sub] + 1);
                        }
                    }
                    if (sum < best || (sum == best && worst < depth[mask])) {
                        best = sum;
                        depth[mask] = worst;
                        bestQuery[mask] = k;
                    }
                }
                return cost[mask] = best;
            };
            if (!partitions.empty()) {
                solve(solve, full);
                chosen = queryIndex[bestQuery[full]];
            }
        } else {
            long long best = 1LL * candidates.size() * candidates.size();
            int bestLargest = int(candidates.size());
            for (int k = 0; k < int(queries.size()); ++k) {
                fill(frequency.begin(), frequency.end(), 0);
                long long squares = 0;
                int largest = 0;
                for (const auto& candidate : candidates) {
                    int count = ++frequency[reply(candidate, queries[k])];
                    squares += 2 * count - 1;
                    largest = max(largest, count);
                }
                if (tie(squares, largest) < tie(best, bestLargest)) {
                    best = squares;
                    bestLargest = largest;
                    chosen = k;
                }
            }
        }
        if (chosen == -1) break;
        Query q = queries[chosen];
        int value = ask(q);
        erase_if(candidates, [&](const auto& candidate) {
            return reply(candidate, q) != value;
        });
        if (candidates.empty()) exit(0);
    }
    return candidates.front();
}

vector<int> solveSmall(Interaction& io, vector<PackedPermutation> candidates) {
    vector<Query> queries;
    for (int i = 0; i < io.n; ++i) {
        for (int j = 0; j < io.n; ++j) queries.push_back({i, j});
    }
    auto answer = [](const PackedPermutation& p, Query q) {
        return digit(p.p, q.row) ^ digit(p.b, q.col);
    };
    auto ask = [&](Query q) { return io.ask(q.row, q.col); };
    auto result = identifySmall(move(candidates), queries, io.maskSize, answer, ask);
    vector<int> p(io.n);
    for (int i = 0; i < io.n; ++i) p[i] = digit(result.p, i) ^ result.shift;
    return p;
}

struct PartialMap {
    vector<int> value;
    vector<int> free;
    vector<int> position;

    explicit PartialMap(int n) : value(n, -1), free(n), position(n) {
        iota(free.begin(), free.end(), 0);
        iota(position.begin(), position.end(), 0);
    }

    void set(int index, int image) {
        value[index] = image;
        int slot = position[index];
        free[slot] = free.back();
        position[free[slot]] = slot;
        free.pop_back();
    }
};

struct ExplicitState {
    int shift;
    array<PartialMap, 2> map;
    vector<int> guesses;

    ExplicitState(int n, int anchor) : shift(anchor), map{PartialMap(n), PartialMap(n)} {}

    void match(int side, int index, int image) {
        if (map[side].value[index] != -1) return;
        map[side].set(index, image);
        map[side ^ 1].set(image, index);
    }

    void fillLast() {
        if (map[0].free.size() == 1) match(0, map[0].free.back(), map[1].free.back());
    }
};

class Solver {
    struct CompactState {
        int shift;
        int remaining;
        array<int, 2> freeXor;
    };

public:
    explicit Solver(Interaction& interaction)
        : io(interaction), n(io.n), universe(io.maskSize), random(924371U + 8191U * n),
          normal{vector<int>(n, -1), vector<int>(n, -1)},
          location{vector<int>(universe, -1), vector<int>(universe, -1)},
          forbidden(n), seenResponse(universe) {
        for (auto& sequence : order) {
            sequence.resize(n);
            iota(sequence.begin(), sequence.end(), 0);
            shuffle(sequence.begin(), sequence.end(), random);
        }
    }

    vector<int> run() {
        for (int side = 0; side < 2; ++side) root[side] = order[side][0];
        if (n <= 12) chooseSmallRoot();
        int checksum = 0;
        for (int i = 0; i < n; ++i) checksum ^= i;
        for (int c = 0; c < n; ++c) compact.push_back({c, n, {checksum, checksum}});
        origin = io.ask(root[0], root[1]);
        add(1, root[1], 0);
        add(0, root[0], origin);
        if (n <= 12) return solveSmallAnchored();

        // Translations give the same replies for every admissible shift. A
        // matching of queries is useful if such a permutation has a defect on
        // either the forward or the inverse side.
        if (origin == (root[0] ^ root[1])) {
            bool targetRange = true;
            while (compact.size() > explicitLimit && learned[0] < n && !allComplete()) {
                int index = -1;
                if (learned[0] >= 3 && targetRange) {
                    index = translationProbe();
                    targetRange = index != -1;
                    if (!targetRange && (n & -n) >= 32) {
                        if (!has_single_bit(unsigned(n))) {
                            // These rows seed the two cycles of the matching
                            // for every shift that preserves the domain.
                            for (int i : {0, 1}) {
                                if (normal[0][i] == -1) add(0, i, anchorQuery(0, i));
                            }
                        }
                        if (compact.size() > explicitLimit) return finishWithMatching();
                        break;
                    }
                }
                if (index == -1) {
                    index = advance(0);
                } else {
                    add(0, index, anchorQuery(0, index));
                }
                if (normal[0][index] != (index ^ root[1])) break;
            }
        }

        // Queries incident to a root discover normalized values directly.
        // Mixing both directions makes inverse consistency constrain the shift.
        int warmLimit = min(n, int(4 * sqrt(n * log(double(n)))) + 4);
        int initial = int(compact.size());
        for (int step = 0; step < warmLimit && compact.size() > explicitLimit && !allComplete(); ++step) {
            if (step == 12 && int(compact.size()) * 100 >= initial * 99) break;
            int side = learned[0] <= learned[1] ? 0 : 1;
            if (advance(side) == -1) break;
        }

        int primary = learned[0] >= learned[1] ? 0 : 1;
        while (compact.size() > explicitLimit && !allComplete()) {
            if (advance(primary) == -1 && advance(primary ^ 1) == -1) break;
        }
        if (compact.size() > explicitLimit) {
            return distinguish(compact, [&](const auto& state, int side, int index) {
                return compactValue(state, side, index);
            });
        }

        vector<ExplicitState> states;
        states.reserve(compact.size());
        for (const auto& candidate : compact) {
            states.emplace_back(n, candidate.shift);
            auto& state = states.back();
            for (int side = 0; side < 2; ++side) {
                for (int i = 0; i < n; ++i) {
                    if (normal[side][i] != -1) state.match(side, i, normal[side][i] ^ state.shift);
                }
            }
        }
        return finishExplicit(states);
    }

private:
    static constexpr size_t explicitLimit = 16;
    Interaction& io;
    int n;
    int universe;
    mt19937 random;
    array<vector<int>, 2> normal;
    array<vector<int>, 2> location;
    array<vector<int>, 2> order;
    array<int, 2> cursor{};
    array<int, 2> learned{};
    array<int, 2> root{};
    int origin = 0;
    vector<CompactState> compact;
    vector<int> forbidden;
    int forbiddenStamp = 0;
    vector<int> seenResponse;
    int responseStamp = 0;

    struct MatchingState {
        int shift;
        int slot;
        int remaining;
        array<int, 2> freeXor;
    };

    int draw(int size) {
        return int(random() % unsigned(size));
    }

    int orientedQuery(int side, int index, int opposite) {
        return side == 0 ? io.ask(index, opposite) : io.ask(opposite, index);
    }

    int anchorQuery(int side, int index) {
        return orientedQuery(side, index, root[side ^ 1]) ^ (side == 1 ? origin : 0);
    }

    void add(int side, int index, int normalized) {
        const int opposite = side ^ 1;
        if (normal[side][index] != -1 || location[side][normalized] != -1) exit(0);
        erase_if(compact, [&](auto& state) {
            int image = normalized ^ state.shift;
            if (image >= n) return true;
            int knownImage = location[opposite][index ^ state.shift];
            if (knownImage != -1 && knownImage != image) return true;
            int knownIndex = normal[opposite][image];
            if (knownIndex != -1 && (knownIndex ^ state.shift) != index) return true;
            if (knownImage == -1) {
                --state.remaining;
                state.freeXor[side] ^= index;
                state.freeXor[opposite] ^= image;
            }
            return false;
        });
        if (compact.empty()) exit(0);
        normal[side][index] = normalized;
        location[side][normalized] = index;
        ++learned[side];
    }

    int compactValue(const CompactState& state, int side, int index) const {
        int value = normal[side][index];
        if (value != -1) return value ^ state.shift;
        value = location[side ^ 1][index ^ state.shift];
        if (value == -1 && state.remaining == 1) return state.freeXor[side ^ 1];
        return value;
    }

    int advance(int side) {
        while (cursor[side] < n && normal[side][order[side][cursor[side]]] != -1) ++cursor[side];
        if (cursor[side] == n) return -1;
        int index = order[side][cursor[side]++];
        int value = compactValue(compact.front(), side, index);
        if (value != -1) {
            value ^= compact.front().shift;
            for (const auto& state : compact) {
                int predicted = compactValue(state, side, index);
                if (predicted == -1 || (predicted ^ state.shift) != value) {
                    value = -1;
                    break;
                }
            }
        }
        add(side, index, value == -1 ? anchorQuery(side, index) : value);
        return index;
    }

    bool allComplete() const {
        return ranges::all_of(compact, [](const auto& state) { return state.remaining <= 1; });
    }

    void chooseSmallRoot() {
        int best = numeric_limits<int>::max();
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                array<int, 16> frequency{};
                frequency[i ^ j] = n - 1;
                for (int x = 0; x < n; ++x) {
                    for (int y = 0; y < n; ++y) {
                        if (x != j && y != i) ++frequency[x ^ y];
                    }
                }
                int score = inner_product(frequency.begin(), frequency.end(), frequency.begin(), 0);
                if (score < best) {
                    best = score;
                    root = {i, j};
                }
            }
        }
    }

    vector<int> solveSmallAnchored() {
        // Enough rows are fixed now to enumerate at most 9! completions
        // per shift, including the case where the root indices are matched.
        while (learned[0] < n - 9) advance(0);
        vector<PackedPermutation> candidates;
        int count = 0;
        for (const auto& state : compact) {
            int factorial = 1;
            for (int k = 2; k <= state.remaining; ++k) factorial *= k;
            count += factorial;
        }
        candidates.reserve(count);
        for (const auto& state : compact) {
            vector<int> p(n), b(n), rows, cols;
            for (int i = 0; i < n; ++i) {
                p[i] = compactValue(state, 0, i);
                b[i] = compactValue(state, 1, i);
                if (p[i] == -1) rows.push_back(i);
                if (b[i] == -1) cols.push_back(i);
            }
            do {
                for (int k = 0; k < int(rows.size()); ++k) {
                    p[rows[k]] = cols[k];
                    b[cols[k]] = rows[k];
                }
                candidates.push_back(pack(p, b, state.shift));
            } while (next_permutation(cols.begin(), cols.end()));
        }
        removeEquivalent(candidates);
        return solveSmall(io, move(candidates));
    }

    int translationProbe() {
        int best = -1;
        int bestGain = 1;
        for (int attempt = 0; attempt < 16; ++attempt) {
            int shift = compact[draw(int(compact.size()))].shift ^ root[1];
            if (shift == 0) continue;
            int half = int(bit_floor(unsigned(shift)));
            int base = n & -(2 * half);
            int tail = n - base;
            if (tail == 0) continue;
            int i = tail <= half ? base : (base + 2 * half - 1) ^ shift;
            if (normal[0][i] != -1) continue;
            int gain = 0;
            for (const auto& state : compact) gain += (i ^ root[1] ^ state.shift) >= n;
            if (gain > bestGain) {
                bestGain = gain;
                best = i;
            }
        }
        return best;
    }

    vector<int> matchingPermutation() const {
        int size = n & -n;
        vector<int> multiply(size);
        iota(multiply.begin(), multiply.end(), 0);
        for (int taps = 1; taps < size; taps += 2) {
            auto next = [size, taps](int i) {
                return (i * 2) ^ ((i * 2 >= size) ? (size | taps) : 0);
            };
            int length = 0;
            int i = 1;
            do {
                i = next(i);
                ++length;
            } while (i != 1);
            if (length != size - 1) continue;
            for (i = 0; i < size; ++i) multiply[i] = next(i);
            break;
        }

        // A primitive multiplier followed by any translation has one long
        // cycle and one fixed point on a power-of-two domain.
        if (n == size) {
            int offset = multiply[root[0]] ^ root[1];
            for (int& x : multiply) x ^= offset;
            return multiply;
        }

        // Cycle through an odd number of blocks. The last two transitions
        // use alpha/(alpha+1) and alpha+1. After one full round, an XOR shift
        // on every transition cancels, leaving multiplication by alpha.
        int blocks = n / size;
        vector<int> inverseSum(size), result(n);
        for (int i = 0; i < size; ++i) inverseSum[multiply[i] ^ i] = i;
        for (int block = 0; block < blocks; ++block) {
            int next = (block + 1) % blocks;
            for (int i = 0; i < size; ++i) {
                int x = i;
                if (block == blocks - 2) x = inverseSum[multiply[i]];
                if (block == blocks - 1) x = multiply[i] ^ i;
                result[block * size + i] = next * size + x;
            }
        }
        return result;
    }

    vector<int> finishWithMatching() {
        const auto matching = matchingPermutation();
        vector<int> inverseMatching(n), response(n, -1);
        for (int i = 0; i < n; ++i) inverseMatching[matching[i]] = i;
        vector<int> observed;
        if (matching[root[0]] == root[1]) {
            response[root[0]] = origin;
            observed.push_back(root[0]);
        }

        vector<int16_t> values(2ULL * n * compact.size(), -1);
        auto getMap = [&](const MatchingState& state, int side) {
            return values.data() + (2 * state.slot + side) * n;
        };
        auto actualValue = [&](const MatchingState& state, int side, int i) -> int {
            return getMap(state, side)[i];
        };

        // An assignment and an already queried matching edge sharing one of
        // its indices force another assignment. Propagate both directions.
        vector<pair<int, int>> pending;
        auto insert = [&](MatchingState& state, int i, int j) {
            auto p = getMap(state, 0);
            auto b = getMap(state, 1);
            pending.clear();
            auto require = [&](int row, int col) {
                if (row >= n || col >= n) return false;
                if (p[row] != -1 || b[col] != -1) return p[row] == col && b[col] == row;
                p[row] = int16_t(col);
                b[col] = int16_t(row);
                --state.remaining;
                state.freeXor[0] ^= row;
                state.freeXor[1] ^= col;
                pending.emplace_back(row, col);
                return true;
            };
            if (!require(i, j)) return false;
            for (size_t k = 0; k < pending.size(); ++k) {
                auto [row, col] = pending[k];
                if (state.remaining == 1 && !require(state.freeXor[0], state.freeXor[1])) return false;
                if (response[row] != -1 && !require(col ^ response[row], matching[row])) return false;
                int other = inverseMatching[col];
                if (response[other] != -1 && !require(other, row ^ response[other])) return false;
            }
            return true;
        };

        int checksum = 0;
        for (int i = 0; i < n; ++i) checksum ^= i;
        vector<MatchingState> states;
        for (const auto& candidate : compact) {
            MatchingState state{candidate.shift, int(states.size()), n, {checksum, checksum}};
            for (int side = 0; side < 2; ++side) {
                for (int i = 0; i < n; ++i) {
                    if (normal[side][i] == -1) continue;
                    int x = normal[side][i] ^ state.shift;
                    bool ok = side == 0 ? insert(state, i, x) : insert(state, x, i);
                    if (!ok) exit(0);
                }
            }
            states.push_back(state);
        }
        auto complete = [&] {
            return ranges::all_of(states, [](const auto& state) { return state.remaining == 0; });
        };
        auto matchingOrder = order[0];
        stable_partition(matchingOrder.begin(), matchingOrder.end(), [&](int i) {
            return normal[0][i] == -1;
        });
        for (int i : matchingOrder) {
            if (response[i] != -1) continue;
            if (states.size() <= explicitLimit || complete()) break;
            int j = matching[i];
            int answer = -1;
            for (const auto& state : states) {
                int x = actualValue(state, 0, i);
                int y = actualValue(state, 1, j);
                if (x == -1 || y == -1 || (answer != -1 && answer != (x ^ y))) {
                    answer = -1;
                    break;
                }
                answer = x ^ y;
            }
            if (answer == -1) answer = io.ask(i, j);
            response[i] = answer;
            observed.push_back(i);
            erase_if(states, [&](auto& state) {
                int x = actualValue(state, 0, i);
                int y = actualValue(state, 1, j);
                if (x != -1) return !insert(state, x ^ answer, j);
                if (y != -1) return !insert(state, i, y ^ answer);
                return false;
            });
            if (states.empty()) exit(0);
        }

        auto seed = [&](int side, int i) {
            int result = anchorQuery(side, i);
            erase_if(states, [&](auto& state) {
                int x = result ^ state.shift;
                return side == 0 ? !insert(state, i, x) : !insert(state, x, i);
            });
            if (states.empty()) exit(0);
        };
        while (states.size() > explicitLimit && !complete()) {
            auto open = find_if(states.begin(), states.end(), [](const auto& state) {
                return state.remaining > 0;
            });
            auto p = getMap(*open, 0);
            int i = int(find(p, p + n, int16_t(-1)) - p);
            seed(0, i);
        }
        if (complete()) return distinguish(states, actualValue);

        for (int i : observed) {
            if (ranges::any_of(states, [&](const auto& state) { return actualValue(state, 0, i) == -1; })) {
                seed(0, i);
            }
        }
        vector<ExplicitState> partial;
        partial.reserve(states.size());
        for (const auto& candidate : states) {
            partial.emplace_back(n, candidate.shift);
            for (int i = 0; i < n; ++i) {
                int image = actualValue(candidate, 0, i);
                if (image != -1) partial.back().match(0, i, image);
            }
        }
        return finishExplicit(partial);
    }

    template <class State, class Value>
    vector<int> distinguish(vector<State>& states, Value actualValue) {
        vector<pair<int, int>> probes;
        if (states.size() > 1) {
            for (int side = 0; side < 2; ++side) {
                for (int i = 0; i < n; ++i) {
                    int base = actualValue(states.front(), side, i) ^ states.front().shift;
                    if (ranges::any_of(states, [&](const auto& state) {
                        return (actualValue(state, side, i) ^ state.shift) != base;
                    })) probes.emplace_back(side, i);
                }
            }
        }
        vector<int> frequency(universe);
        vector<int> touched;
        while (states.size() > 1 && !probes.empty()) {
            long long unsplit = 1LL * int(states.size()) * int(states.size());
            long long best = unsplit;
            int pick = -1;
            for (int attempt = 0; attempt < 12 && !probes.empty(); ++attempt) {
                int k = attempt == 0 ? int(probes.size()) - 1 : draw(int(probes.size()));
                auto [side, index] = probes[k];
                long long score = 0;
                for (const auto& state : states) {
                    int value = actualValue(state, side, index) ^ state.shift;
                    int& count = frequency[value];
                    if (count == 0) touched.push_back(value);
                    score += 2 * ++count - 1;
                }
                for (int value : touched) frequency[value] = 0;
                touched.clear();
                if (score < best) {
                    best = score;
                    pick = k;
                } else if (score == unsplit) {
                    if (pick == int(probes.size()) - 1) pick = k;
                    probes[k] = probes.back();
                    probes.pop_back();
                }
            }
            if (pick == -1) continue;
            auto [side, index] = probes[pick];
            probes[pick] = probes.back();
            probes.pop_back();
            int value = anchorQuery(side, index);
            erase_if(states, [&](const auto& state) {
                return (actualValue(state, side, index) ^ state.shift) != value;
            });
            if (states.empty()) exit(0);
        }
        vector<int> p(n);
        for (int i = 0; i < n; ++i) p[i] = actualValue(states.front(), 0, i);
        return p;
    }

    void exactTail(ExplicitState& state) {
        const auto& a = state.map[0].free;
        const auto& b = state.map[1].free;
        int size = int(a.size());
        int anchorP = state.map[0].value[root[0]], anchorB = state.map[1].value[root[1]];
        vector<Query> queries;
        for (int i = 0; i <= size; ++i) {
            for (int j = 0; j <= size; ++j) {
                if (i != size || j != size) queries.push_back({i, j});
            }
        }
        auto answer = [&](const PackedPermutation& p, Query q) {
            int x = q.row == size ? anchorP : b[digit(p.p, q.row)];
            int y = q.col == size ? anchorB : a[digit(p.b, q.col)];
            return x ^ y;
        };
        auto ask = [&](Query q) {
            return io.ask(q.row == size ? root[0] : a[q.row], q.col == size ? root[1] : b[q.col]);
        };
        auto p = identifySmall(generatePermutations(size, false), queries, universe, answer, ask);
        vector<int> rows = a, cols = b;
        for (int i = 0; i < size; ++i) state.match(0, rows[i], cols[digit(p.p, i)]);
    }

    pair<int, int> choosePair(const vector<ExplicitState>& states, int side,
                              const vector<int>& leaves, const vector<int>& pivots) {
        pair<int, int> best{leaves[draw(int(leaves.size()))], pivots[draw(int(pivots.size()))]};
        if (states.size() != 1 || leaves.size() > 192) return best;
        const auto& state = states.front();
        vector<int> frequency(universe);
        for (int i : leaves) {
            for (int j : pivots) ++frequency[i ^ j];
        }
        int bestScore = -1;
        int bestLink = n + 1;
        for (int attempt = 0; attempt < 20; ++attempt) {
            int i = attempt == 0 ? best.first : leaves[draw(int(leaves.size()))];
            int j = attempt == 0 ? best.second : pivots[draw(int(pivots.size()))];
            int link = i ^ j;
            int score = -(frequency[link] == 1);
            for (int other : pivots) {
                int value = other ^ i;
                if (value == link) continue;
                int y = j ^ value;
                int lost = 1 + (y < n && state.map[side].value[y] == -1);
                score += (frequency[value] - lost == 1) - (frequency[value] == 1);
            }
            for (int other : leaves) {
                int value = other ^ j;
                int x = i ^ value;
                if (x < n && state.map[side ^ 1].value[x] == -1) continue;
                score += (frequency[value] == 2) - (frequency[value] == 1);
            }
            if (attempt == 0 || score > bestScore || (score == bestScore && frequency[link] < bestLink)) {
                bestScore = score;
                bestLink = frequency[link];
                best = {i, j};
            }
        }
        return best;
    }

    void solveStar(vector<ExplicitState>& states, int side, vector<int>& leaves,
                   const vector<int>& pivots) {
        auto [first, pivot] = choosePair(states, side, leaves, pivots);
        for (auto& state : states) state.guesses = state.map[side].free;
        vector<pair<int, int>> observations;
        ++responseStamp;
        int next = int(find(leaves.begin(), leaves.end(), first) - leaves.begin());
        while (true) {
            int index = leaves[next];
            leaves[next] = leaves.back();
            leaves.pop_back();
            int response = orientedQuery(side, index, pivot);
            seenResponse[response] = responseStamp;
            observations.emplace_back(index, response);
            erase_if(states, [&](auto& state) {
                erase_if(state.guesses, [&](int y) {
                    int x = response ^ y;
                    return x >= n || state.map[side ^ 1].value[x] != -1 ||
                           ((y == index) != (x == pivot));
                });
                return state.guesses.empty();
            });
            if (states.empty()) exit(0);

            while (true) {
                bool resolved = ranges::all_of(states, [](const auto& state) {
                    return state.guesses.size() == 1;
                });
                if (resolved) {
                    for (auto& state : states) {
                        int y = state.guesses.front();
                        state.match(side, y, pivot);
                        for (auto [i, r] : observations) state.match(side, i, r ^ y);
                    }
                    return;
                }

                ++forbiddenStamp;
                for (const auto& state : states) {
                    for (int y : state.guesses) forbidden[y] = forbiddenStamp;
                }
                next = -1;
                if (!leaves.empty() && observations.size() < 48) {
                    for (int attempt = 0; attempt < 12; ++attempt) {
                        int k = draw(int(leaves.size()));
                        if (forbidden[leaves[k]] != forbiddenStamp) {
                            next = k;
                            break;
                        }
                    }
                    if (next == -1) {
                        for (int k = 0; k < int(leaves.size()); ++k) {
                            if (forbidden[leaves[k]] != forbiddenStamp) {
                                next = k;
                                break;
                            }
                        }
                    }
                }
                if (next != -1) break;

                // If the first queried pair might describe a single match,
                // resolve it using fresh assignments so every query still helps.
                if (states.size() == 1 && !leaves.empty() && forbidden[first] == forbiddenStamp) {
                    auto& state = states.front();
                    int k = int(find_if(leaves.begin(), leaves.end(), [&](int i) {
                        return forbidden[i] == forbiddenStamp;
                    }) - leaves.begin());
                    if (k == int(leaves.size())) k = draw(int(leaves.size()));
                    int i = leaves[k];
                    leaves[k] = leaves.back();
                    leaves.pop_back();
                    int x = anchorQuery(side, i) ^ state.shift;
                    if (x == pivot) {
                        state.guesses.assign(1, i);
                    } else {
                        state.match(side, i, x);
                        erase_if(state.guesses, [&](int y) {
                            return y == i || seenResponse[x ^ y] == responseStamp;
                        });
                    }
                    continue;
                }

                int value = anchorQuery(side ^ 1, pivot);
                erase_if(states, [&](auto& state) {
                    int y = value ^ state.shift;
                    if (find(state.guesses.begin(), state.guesses.end(), y) == state.guesses.end()) return true;
                    state.guesses.assign(1, y);
                    return false;
                });
                if (states.empty()) exit(0);
            }
        }
    }

    void advanceExplicit(vector<ExplicitState>& states) {
        int best = -1;
        pair<int, int> choice;
        for (int attempt = 0; attempt < 24; ++attempt) {
            const auto& base = states[draw(int(states.size()))];
            if (base.map[0].free.empty()) continue;
            int side = draw(2);
            int i = base.map[side].free[draw(int(base.map[side].free.size()))];
            int score = 0;
            for (const auto& state : states) score += state.map[side].value[i] == -1;
            if (score > best) {
                best = score;
                choice = {side, i};
            }
        }
        if (best == -1) {
            for (const auto& state : states) {
                if (!state.map[0].free.empty()) {
                    choice = {0, state.map[0].free.front()};
                    break;
                }
            }
        }
        auto [side, i] = choice;
        int value = anchorQuery(side, i);
        erase_if(states, [&](auto& state) {
            int x = value ^ state.shift;
            if (x >= n) return true;
            int old = state.map[side].value[i];
            if (old != -1) return old != x;
            if (state.map[side ^ 1].value[x] != -1) return true;
            state.match(side, i, x);
            return false;
        });
        if (states.empty()) exit(0);
    }

    vector<int> finishExplicit(vector<ExplicitState>& states) {
        while (true) {
            for (auto& state : states) state.fillLast();
            if (ranges::all_of(states, [](const auto& state) { return state.map[0].free.empty(); })) break;
            if (states.size() == 1 && states.front().map[0].free.size() <= 8) {
                exactTail(states.front());
                break;
            }
            array<vector<int>, 2> common;
            for (int side = 0; side < 2; ++side) {
                for (int i : states.front().map[side].free) {
                    if (ranges::all_of(states, [&](const auto& state) {
                        return state.map[side].value[i] == -1;
                    })) common[side].push_back(i);
                }
            }
            if (common[0].empty() || common[1].empty()) {
                advanceExplicit(states);
            } else {
                int side = states.size() == 1 ? draw(2) : int(common[1].size() > common[0].size());
                solveStar(states, side, common[side], common[side ^ 1]);
            }
        }
        return distinguish(states, [](const auto& state, int side, int index) {
            return state.map[side].value[index];
        });
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 1 || n > 5000) return 0;
    Interaction io(n);
    if (n <= 9) {
        io.report(solveSmall(io, generatePermutations(n, true)));
    } else {
        Solver solver(io);
        io.report(solver.run());
    }
    return 0;
}

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <utility>
#include <vector>

using namespace std;

enum Sign { nonpositive = -1, unknown = 0, nonnegative = 1 };

class Judge {
    vector<signed char> comparisons;
    int stride;
    int words;
    int queries = 0;
    static constexpr int maxLabels = 2 * 1000;
    using Row = array<uint64_t, (maxLabels + 63) / 64>;
    vector<Row> before, after;

    struct Addends {
        int low = 0, high = 0;
        int batch = 0;
        int lowBlock = 0, highBlock = 0;
        bool lowEqual = false, highEqual = false;
    };
    vector<Addends> addends;

    static bool contains(const Row& row, int label) {
        return (row[label / 64] >> (label % 64)) & 1;
    }

    static void include(Row& row, int label) {
        row[label / 64] |= uint64_t{1} << (label % 64);
    }

    // Store consequences of the total order on (value, label), even for
    // erased pivots. This reuses comparisons when blocks are regrouped.
    void rememberOrder(int a, int b) {
        if (contains(after[a], b)) return;
        Row predecessors = before[a], successors = after[b];
        include(predecessors, a);
        include(successors, b);
        for (int i = 0; i < words; ++i) {
            for (uint64_t bits = predecessors[i]; bits; bits &= bits - 1) {
                auto& row = after[64 * i + countr_zero(bits)];
                if (contains(row, b)) continue;
                for (int j = 0; j < words; ++j) row[j] |= successors[j];
            }
            for (uint64_t bits = successors[i]; bits; bits &= bits - 1) {
                auto& row = before[64 * i + countr_zero(bits)];
                if (contains(row, a)) continue;
                for (int j = 0; j < words; ++j) row[j] |= predecessors[j];
            }
        }
    }

    static int read() {
        int reply;
        if (!(cin >> reply) || reply == -1) exit(0);
        return reply;
    }

    bool knownAtMost(int a, int b) const {
        return a == b || contains(after[a], b) || comparisons[b * stride + a] == 0;
    }

    bool knownStrict(int a, int b) const {
        return comparisons[a * stride + b] == 1 || (a > b && contains(after[a], b));
    }

    bool orderedSums(int a, int b, bool strict) const {
        const auto& x = addends[a];
        const auto& y = addends[b];
        if (!x.low || !y.low) return false;
        bool sameBatch = x.batch && x.batch == y.batch;
        bool lowOrdered = knownAtMost(x.low, y.low) ||
            (sameBatch && (x.lowBlock < y.lowBlock ||
                           (x.lowBlock == y.lowBlock && x.lowEqual)));
        if (!lowOrdered) return false;
        bool highOrdered = knownAtMost(x.high, y.high) ||
            (sameBatch && (x.highBlock < y.highBlock ||
                           (x.highBlock == y.highBlock && x.highEqual)));
        return highOrdered && (!strict || knownStrict(x.low, y.low) ||
                                          knownStrict(x.high, y.high));
    }

public:
    explicit Judge(int n) : comparisons(4 * n * n, -1), stride(2 * n),
                            words((stride + 63) / 64), before(stride), after(stride),
                            addends(stride) {}

    bool strictlyLess(int a, int b) {
        auto& reply = comparisons[a * stride + b];
        if (reply == -1) {
            if (contains(after[b], a) || comparisons[b * stride + a] == 1) return false;
            if (a > b && contains(after[a], b)) return true;
            if (orderedSums(b, a, false)) reply = 0;
            else if (orderedSums(a, b, true)) reply = 1;
            else {
                ++queries;
                cout << "? " << a << ' ' << b << endl;
                reply = read();
            }
            if (reply == 1) rememberOrder(a, b);
            else if (a > b) rememberOrder(b, a);
        }
        return reply == 1;
    }

    bool spentComparisonBudget() const { return queries > 7 * stride; }

    // Use the label to break ties when partitioning a block.
    bool less(int a, int b) {
        if (a == b) return false;
        return a > b ? strictlyLess(a, b) : !strictlyLess(b, a);
    }

    int add(int a, int b) {
        cout << "+ " << a << ' ' << b << endl;
        int label = read();
        addends[label] = {a, b};
        return label;
    }

    void markBatch(int label, int batch, int lo, int hi, bool loEqual, bool hiEqual) {
        auto& terms = addends[label];
        terms.batch = batch;
        terms.lowBlock = lo;
        terms.highBlock = hi;
        terms.lowEqual = loEqual;
        terms.highEqual = hiEqual;
    }
};

struct Block {
    vector<int> labels;
    Sign sign = unknown;
    int equalClass = 0;

    int size() const { return static_cast<int>(labels.size()); }
    bool isPoint() const { return size() == 1 || (size() && equalClass); }
};

class Solver {
    Judge judge;
    // Every block precedes the next by value. Within a block only its sign
    // and any equal-value certificate are known.
    vector<Block> blocks;
    unsigned randomState = 0xb970573b;
    int count;
    int nearEdgeTries = 0;
    bool initialOrderKnown = false;

    unsigned nextRandom() {
        randomState ^= randomState << 13;
        randomState ^= randomState >> 17;
        randomState ^= randomState << 5;
        return randomState;
    }

    int blockAt(int rank) const {
        for (int i = 0; i < static_cast<int>(blocks.size()); ++i) {
            if (rank < blocks[i].size()) return i;
            rank -= blocks[i].size();
        }
        abort();
    }

    void replaceBlock(int index, vector<Block>& parts) {
        blocks.erase(blocks.begin() + index);
        blocks.insert(blocks.begin() + index,
                      make_move_iterator(parts.begin()), make_move_iterator(parts.end()));
    }

    void partitionOnPivot(int index, int pivot, bool repeated) {
        const Block& original = blocks[index];
        const auto& labels = original.labels;
        vector<Block> parts;
        Block lower{{}, original.sign};
        Block middle{{pivot}, original.sign, repeated ? pivot : 0};
        Block upper{{}, original.sign};
        for (int label : labels) {
            if (label == pivot) continue;
            if (!repeated) {
                if (judge.less(label, pivot)) lower.labels.push_back(label);
                else upper.labels.push_back(label);
            } else if (judge.strictlyLess(label, pivot)) {
                lower.labels.push_back(label);
            } else if (judge.strictlyLess(pivot, label)) {
                upper.labels.push_back(label);
            } else {
                middle.labels.push_back(label);
            }
        }
        if (lower.size()) parts.push_back(move(lower));
        parts.push_back(move(middle));
        if (upper.size()) parts.push_back(move(upper));
        replaceBlock(index, parts);
    }

    void partitionWithSample(int index, int rank, int other = -1) {
        const Block& original = blocks[index];
        int size = original.size();
        int sampleSize = clamp(size / 24, 17, 60);
        vector<int> sample = original.labels;
        for (int i = 0; i < sampleSize; ++i) {
            swap(sample[i], sample[i + nextRandom() % (size - i)]);
        }
        sample.resize(sampleSize);
        // Avoid sorting the sample when a few probes already find a repeated value.
        for (int i = 1; i < 4; ++i) {
            if (!judge.strictlyLess(sample[0], sample[i]) &&
                !judge.strictlyLess(sample[i], sample[0])) {
                partitionOnPivot(index, sample[0], true);
                return;
            }
        }
        sort(sample.begin(), sample.end(), [&](int a, int b) { return judge.less(a, b); });

        vector<int> cuts;
        int margin = max(2, sampleSize / 10);
        for (int target : {rank, other}) {
            if (target < 0) continue;
            int estimate = target * sampleSize / size;
            cuts.push_back(max(0, estimate - margin));
            cuts.push_back(min(sampleSize - 1, estimate + margin));
        }
        sort(cuts.begin(), cuts.end());
        cuts.erase(unique(cuts.begin(), cuts.end()), cuts.end());

        auto threeEqualAt = [&](int i) {
            return i > 0 && i + 1 < sampleSize &&
                   !judge.strictlyLess(sample[i - 1], sample[i]) &&
                   !judge.strictlyLess(sample[i], sample[i + 1]);
        };
        for (int i : cuts) {
            if (threeEqualAt(i)) {
                partitionOnPivot(index, sample[i], true);
                return;
            }
        }
        if (threeEqualAt(sampleSize / 2)) {
            partitionOnPivot(index, sample[sampleSize / 2], true);
            return;
        }

        int k = static_cast<int>(cuts.size());
        vector<int> pivots;
        for (int i : cuts) pivots.push_back(sample[i]);
        // Find a comparison tree for the buckets, weighting them by the sample.
        int cost[5][5]{}, decision[5][5]{};
        for (int gap = 1; gap <= k; ++gap) {
            for (int l = 0, r = gap; r <= k; ++l, ++r) {
                int weight = (r < k ? cuts[r] + 1 : sampleSize) -
                             (l > 0 ? cuts[l - 1] + 1 : 0);
                cost[l][r] = 10000;
                for (int at = l; at < r; ++at) {
                    int trial = weight + cost[l][at] + cost[at + 1][r];
                    if (trial < cost[l][r]) {
                        cost[l][r] = trial;
                        decision[l][r] = at;
                    }
                }
            }
        }

        vector<Block> buckets(k + 1, Block{{}, original.sign});
        for (int value : original.labels) {
            if (std::find(pivots.begin(), pivots.end(), value) != pivots.end()) continue;
            int l = 0, r = k;
            while (l < r) {
                int at = decision[l][r];
                if (judge.less(value, pivots[at])) r = at;
                else l = at + 1;
            }
            buckets[l].labels.push_back(value);
        }
        vector<Block> parts;
        for (int i = 0; i <= k; ++i) {
            if (buckets[i].size()) parts.push_back(move(buckets[i]));
            if (i < k) parts.push_back({{pivots[i]}, original.sign});
        }
        replaceBlock(index, parts);
    }

    void partition(int index, int rank) {
        const Block& original = blocks[index];
        const auto& labels = original.labels;
        const int size = original.size();
        vector<Block> parts;
        if (original.equalClass) {
            if (rank) parts.push_back({{labels.begin(), labels.begin() + rank},
                                       original.sign, original.equalClass});
            parts.push_back({{labels[rank]}, original.sign, original.equalClass});
            if (rank + 1 < size) parts.push_back({{labels.begin() + rank + 1, labels.end()},
                                                 original.sign, original.equalClass});
        } else if (rank == 0 || rank == size - 1 ||
                   (size >= 8 && (rank == 1 || rank == size - 2))) {
            bool reversed = rank > (size - 1) / 2;
            auto better = [&](int a, int b) {
                return reversed ? judge.less(b, a) : judge.less(a, b);
            };
            bool needSecond = rank != 0 && rank != size - 1;
            int first = labels[0], second = 0;
            for (int i = 1; i < size; ++i) {
                int x = labels[i];
                if (needSecond && second && !better(x, second)) continue;
                if (better(x, first)) {
                    second = first;
                    first = x;
                } else {
                    second = x;
                }
            }
            parts.push_back({{first}, original.sign});
            if (needSecond) parts.push_back({{second}, original.sign});
            Block rest{{}, original.sign};
            for (int x : labels) {
                if (x != first && (!needSecond || x != second)) rest.labels.push_back(x);
            }
            if (rest.size()) parts.push_back(move(rest));
            if (reversed) reverse(parts.begin(), parts.end());
        } else {
            if (size >= 130) {
                partitionWithSample(index, rank);
                return;
            }
            int pivot = labels[nextRandom() % size];
            bool repeated = false;
            if (size >= 12) {
                int a = labels[nextRandom() % size];
                int b = labels[nextRandom() % size];
                if (judge.less(pivot, a)) swap(a, pivot);
                if (judge.less(b, pivot)) {
                    if (judge.less(b, a)) swap(a, pivot);
                    else swap(b, pivot);
                }
                for (int z : {a, b}) {
                    if (pivot != z && !judge.strictlyLess(pivot, z)
                                   && !judge.strictlyLess(z, pivot)) {
                        repeated = true;
                        break;
                    }
                }
            }
            partitionOnPivot(index, pivot, repeated);
            return;
        }
        replaceBlock(index, parts);
    }

    int at(int rank) {
        int index = blockAt(rank);
        const int workLimit = 7 * blocks[index].size();
        int work = 0;
        while (blocks[index].size() > 1) {
            if (judge.spentComparisonBudget() || work > workLimit) {
                Block& block = blocks[index];
                sort(block.labels.begin(), block.labels.end(),
                     [&](int a, int b) { return judge.less(a, b); });
                vector<Block> singletons;
                for (int label : block.labels) {
                    singletons.push_back({{label}, block.sign, block.equalClass});
                }
                replaceBlock(index, singletons);
                return blocks[blockAt(rank)].labels[0];
            }
            work += blocks[index].size();
            int offset = rank;
            for (int i = 0; i < index; ++i) offset -= blocks[i].size();
            partition(index, offset);
            index = blockAt(rank);
        }
        return blocks[index].labels[0];
    }

    int find(int label) const {
        for (int i = 0; i < static_cast<int>(blocks.size()); ++i) {
            for (int x : blocks[i].labels) {
                if (x == label) return i;
            }
        }
        abort();
    }

    void markBefore(int index) {
        int end = index;
        int equal = blocks[index].equalClass;
        if (equal) {
            for (int i = index; i < static_cast<int>(blocks.size()); ++i) {
                if (blocks[i].equalClass == equal) end = i + 1;
            }
        }
        for (int i = 0; i < end; ++i) blocks[i].sign = nonpositive;
    }

    void markAfter(int index) {
        int start = index + 1;
        int equal = blocks[index].equalClass;
        if (equal) {
            for (int i = 0; i <= index; ++i) {
                if (blocks[i].equalClass == equal) {
                    start = i;
                    break;
                }
            }
        }
        for (int i = start; i < static_cast<int>(blocks.size()); ++i) {
            blocks[i].sign = nonnegative;
        }
    }

    int extreme(const Block& block, bool maximum) {
        int best = block.labels[0];
        if (block.equalClass) return best;
        for (int i = 1; i < block.size(); ++i) {
            int x = block.labels[i];
            if (maximum ? judge.less(best, x) : judge.less(x, best)) best = x;
        }
        return best;
    }

    void combine(int x, int y, int firstProbe, int secondProbe, int thirdProbe,
                 bool discovery) {
        int ix = find(x), iy = find(y);
        Sign sx = blocks[ix].sign, sy = blocks[iy].sign;
        const bool positivePartner = sy == nonnegative;
        const int sum = judge.add(x, y);
        for (auto [index, label] : {pair{ix, x}, pair{iy, y}}) {
            auto& labels = blocks[index].labels;
            auto it = std::find(labels.begin(), labels.end(), label);
            *it = labels.back();
            labels.pop_back();
        }
        --count;
        if (count == 2) {
            erase_if(blocks, [](const Block& b) { return b.labels.empty(); });
            blocks.push_back({{sum}, unknown});
            return;
        }

        const int size = static_cast<int>(blocks.size());
        int low = 0, high = size;
        Sign sign = unknown;

        auto deduceBounds = [&] {
            if (sy == nonnegative) low = max(low, ix + (blocks[ix].size() == 0));
            if (sy == nonpositive) high = min(high, ix + (blocks[ix].size() != 0));
            if (sx == nonnegative) low = max(low, iy + (blocks[iy].size() == 0));
            if (sx == nonpositive) high = min(high, iy + (blocks[iy].size() != 0));
            if (sx != unknown && sx == sy) sign = sx;
        };

        // Comparing the sum with a bound on one addend reveals the sign
        // of the other. The bound may come from a block containing several values.
        auto probe = [&](int z, int index, bool minimum, bool maximum, bool tiesFirst) {
            bool smaller = tiesFirst ? !judge.strictlyLess(z, sum) : judge.strictlyLess(sum, z);
            if (smaller) {
                high = min(high, index + !minimum);
                if (blocks[index].sign == nonpositive) sign = nonpositive;
                if (index < iy && sx == unknown) {
                    sx = nonpositive;
                    markBefore(ix);
                }
                if (index < ix && sy == unknown) {
                    sy = nonpositive;
                    markBefore(iy);
                }
            } else {
                low = max(low, index + maximum);
                if (blocks[index].sign == nonnegative) sign = nonnegative;
                if (index > ix && sy == unknown) {
                    sy = nonnegative;
                    markAfter(iy);
                }
                if (index > iy && sx == unknown) {
                    sx = nonnegative;
                    markAfter(ix);
                }
            }
            deduceBounds();
            // Crossing bounds can only cover equal values. Either cut is valid.
            if (low > high) low = high;
            return smaller;
        };

        auto probeLabel = [&](int label, bool tiesFirst) {
            int index = find(label);
            bool point = blocks[index].isPoint();
            return probe(label, index, point, point, tiesFirst);
        };

        deduceBounds();
        if (firstProbe) {
            bool smaller = probeLabel(firstProbe, !discovery && !positivePartner);
            if (discovery) {
                int guard = smaller ? secondProbe : thirdProbe;
                if (guard && probeLabel(guard, smaller) == smaller) nearEdgeTries = 2;
            } else if (secondProbe && (positivePartner ? !smaller : smaller)) {
                probeLabel(secondProbe, !positivePartner);
            }
        }

        while (true) {
            int neg = -1, pos = -1;
            int elements = 0, middle = -1, lastNegative = -1, firstPositive = size;
            bool hasFlat = false;
            bool hasUnknown = false;
            for (int i = 0; i < size; ++i) {
                if (!blocks[i].size()) continue;
                if (blocks[i].sign == nonpositive) lastNegative = i;
                if (blocks[i].sign == nonnegative) firstPositive = min(firstPositive, i);
                if (i < low || i >= high) continue;
                elements += blocks[i].size();
                hasFlat |= blocks[i].equalClass != 0;
                if (blocks[i].sign == nonpositive) neg = i;
                if (blocks[i].sign == nonnegative && pos == -1) pos = i;
                if (blocks[i].sign == unknown) hasUnknown = true;
            }
            if (firstPositive < low) sign = nonnegative;
            if (lastNegative >= high) sign = nonpositive;
            if (neg != -1 && (hasUnknown || pos != -1 || sign != nonpositive)) {
                probe(extreme(blocks[neg], true), neg, blocks[neg].isPoint(), true, true);
                continue;
            }
            if (pos != -1 && (hasUnknown || sign != nonnegative)) {
                probe(extreme(blocks[pos], false), pos, true, blocks[pos].isPoint(), false);
                continue;
            }
            if (!hasFlat && (neg != -1 || pos != -1 || elements <= (count < 18 ? 1 : 6))) break;
            int passed = 0, distance = elements;
            for (int i = low; i < high; ++i) {
                if (blocks[i].isPoint()) {
                    int delta = abs(2 * passed + blocks[i].size() - elements);
                    if (delta < distance) {
                        distance = delta;
                        middle = i;
                    }
                }
                passed += blocks[i].size();
            }
            if (middle == -1) break;
            probe(blocks[middle].labels[0], middle, true, true, false);
        }

        Block merged{{sum}, sign};
        for (int i = low; i < high; ++i) {
            if (blocks[i].size() && blocks[i].sign != sign) merged.sign = unknown;
            merged.labels.insert(merged.labels.end(), blocks[i].labels.begin(), blocks[i].labels.end());
        }
        blocks.erase(blocks.begin() + low, blocks.begin() + high);
        blocks.insert(blocks.begin() + low, move(merged));
        if (blocks[low].sign == nonpositive) markBefore(low);
        if (blocks[low].sign == nonnegative) markAfter(low);
        erase_if(blocks, [](const Block& b) { return b.labels.empty(); });
    }

    void combineOppositeBatch(int negatives, int positives) {
        int pairs = min(negatives, positives);
        if (negatives > pairs) at(negatives - pairs);
        if (positives > pairs) at(count - positives + pairs - 1);
        int lo = blockAt(negatives - pairs), splitLeft = blockAt(negatives - 1) + 1;
        int splitRight = blockAt(count - positives);
        int hi = blockAt(count - positives + pairs - 1) + 1;
        vector<pair<int, int>> lower, upper;
        Block combined;
        for (int i = lo; i < splitLeft; ++i) {
            for (int label : blocks[i].labels) lower.emplace_back(label, i);
        }
        for (int i = splitRight; i < hi; ++i) {
            for (int label : blocks[i].labels) upper.emplace_back(label, i);
        }
        for (int i = splitLeft; i < splitRight; ++i) {
            combined.labels.insert(combined.labels.end(), blocks[i].labels.begin(), blocks[i].labels.end());
        }
        int batch = 0;
        for (int i = 0; i < pairs; ++i) {
            auto [x, bx] = lower[i];
            auto [y, by] = upper[i];
            int sum = judge.add(x, y);
            if (!batch) batch = sum;
            judge.markBatch(sum, batch, bx, by, blocks[bx].equalClass, blocks[by].equalClass);
            combined.labels.push_back(sum);
        }
        blocks.erase(blocks.begin() + lo, blocks.begin() + hi);
        blocks.insert(blocks.begin() + lo, move(combined));
        count -= pairs;
    }

    bool finishEqualGroups() {
        const int allowed = min(10, max(2, count / 3));
        vector<vector<int>> groups;
        int previous = 0;
        for (const auto& block : blocks) {
            if (block.size() > 1 && !block.equalClass) return false;
            if (!block.equalClass || block.equalClass != previous) {
                if (static_cast<int>(groups.size()) == allowed) return false;
                groups.emplace_back();
            }
            auto& group = groups.back();
            group.insert(group.end(), block.labels.begin(), block.labels.end());
            previous = block.equalClass;
        }

        // Each cross-pair has the minimum and maximum values from the
        // start of its batch. All sums from that batch have the same value.
        while (groups.size() >= 2) {
            auto& low = groups.front();
            auto& high = groups.back();
            vector<int> sums;
            while (!low.empty() && !high.empty()) {
                sums.push_back(judge.add(low.back(), high.back()));
                low.pop_back();
                high.pop_back();
            }
            if (high.empty()) groups.pop_back();
            if (low.empty()) groups.erase(groups.begin());

            int l = 0, r = groups.size();
            if (r == 1) l = r;
            while (l < r) {
                int m = (l + r) / 2;
                if (judge.strictlyLess(sums[0], groups[m][0])) r = m;
                else l = m + 1;
            }
            groups.insert(groups.begin() + l, move(sums));
        }
        const auto& last = groups.front();
        int accumulator = last[0];
        for (int i = 1; i < static_cast<int>(last.size()); ++i) {
            accumulator = judge.add(accumulator, last[i]);
        }
        return true;
    }

    void selectSmallExtrema() {
        const auto& labels = blocks[0].labels;
        auto orderedPair = [&](int a, int b) {
            return judge.less(a, b) ? pair{a, b} : pair{b, a};
        };
        auto [lo, hi] = orderedPair(labels[0], labels[1]);
        if (count == 3) {
            int x = labels[2];
            if (judge.less(x, lo)) lo = x;
            else if (judge.less(hi, x)) hi = x;
        } else {
            auto [a, b] = orderedPair(labels[2], labels[3]);
            if (judge.less(a, lo)) lo = a;
            if (judge.less(hi, b)) hi = b;
        }
        Block middle;
        for (int x : labels) {
            if (x != lo && x != hi) middle.labels.push_back(x);
        }
        blocks = {{{lo}, unknown}, move(middle), {{hi}, unknown}};
    }

    void finishByExtrema() {
        vector<int> ordered;
        for (auto& block : blocks) {
            if (!block.isPoint()) {
                sort(block.labels.begin(), block.labels.end(),
                     [&](int a, int b) { return judge.less(a, b); });
            }
            ordered.insert(ordered.end(), block.labels.begin(), block.labels.end());
        }
        while (ordered.size() > 2) {
            int sum = judge.add(ordered.front(), ordered.back());
            ordered.pop_back();
            ordered.erase(ordered.begin());
            if (ordered.size() == 1) {
                judge.add(ordered[0], sum);
                return;
            }
            auto place = upper_bound(ordered.begin(), ordered.end(), sum,
                                     [&](int a, int b) { return judge.strictlyLess(a, b); });
            ordered.insert(place, sum);
        }
        if (ordered.size() == 2) judge.add(ordered[0], ordered[1]);
    }

    void finish() {
        int accumulator = 0;
        for (const auto& block : blocks) {
            for (int label : block.labels) {
                accumulator = accumulator ? judge.add(accumulator, label) : label;
            }
        }
    }

public:
    explicit Solver(int n) : judge(n), count(n) {
        Block initial;
        for (int i = 1; i <= n; ++i) initial.labels.push_back(i);
        if (n >= 6) {
            bool decreasing = judge.less(2, 1);
            int i = 3;
            while (i <= n) {
                bool violatesOrder = decreasing ? judge.strictlyLess(i - 1, i)
                                                : judge.strictlyLess(i, i - 1);
                if (violatesOrder) break;
                ++i;
            }
            if (i > n) {
                if (decreasing) reverse(initial.labels.begin(), initial.labels.end());
                initialOrderKnown = true;
                for (int label : initial.labels) blocks.push_back({{label}, unknown});
                return;
            }
        }
        blocks.push_back(move(initial));
    }

    // For ordered x <= y at zero-based ranks d < j, positive overflow is
    // impossible if 3*d <= count-2 and count-j >= d+1. If x+y>R, then
    // y/R is in [1/2,1] and x/R>1-y/R. The total divided by R is greater
    // than t*y/R + (count-d-t)*(1-y/R) - d, with t=count-j. This affine
    // bound is at least 1 at both endpoints. Reflecting the ranks bounds
    // negative overflow as well when the ranks are symmetric.
    void run() {
        while (count > 2) {
            int negatives = 0, positives = 0;
            for (const auto& block : blocks) {
                if (block.sign == nonpositive) negatives += block.size();
                if (block.sign == nonnegative) positives += block.size();
            }
            if (max(negatives, positives) >= count - 1) {
                // Start with the possible exception. Each addition then either
                // has opposite signs or occurs with a uniform sign on the board.
                Sign common = negatives > positives ? nonpositive : nonnegative;
                int accumulator = 0;
                for (const auto& block : blocks) {
                    if (block.sign != common) accumulator = block.labels[0];
                }
                for (const auto& block : blocks) {
                    if (block.sign != common) continue;
                    for (int label : block.labels) {
                        accumulator = accumulator ? judge.add(accumulator, label) : label;
                    }
                }
                return;
            }

            if (finishEqualGroups()) return;
            if (judge.spentComparisonBudget()) {
                finishByExtrema();
                return;
            }
            if (min(negatives, positives) >= 3) {
                combineOppositeBatch(negatives, positives);
            } else if (negatives && positives) {
                const auto& lower = blocks[blockAt(negatives - 1)];
                const auto& upper = blocks[blockAt(count - positives)];
                combine(lower.labels[0], upper.labels[0], 0, 0, 0, false);
            } else if (max(negatives, positives) >= (count <= 8 ? 1 : 4)) {
                int known = max(negatives, positives);
                int free = count - known;
                // A nonnegative partner rules out negative overflow. The two
                // ranks below also rule out positive overflow, or vice versa.
                int d = min({max(0, known - 2), (count - 2) / 3, free / 2});
                if (nearEdgeTries) {
                    d = min(d, 1);
                    --nearEdgeTries;
                }
                int x, y, guard = 0, other = 0;
                if (positives) {
                    x = at(d);
                    y = at(free + (known >= 2));
                    if (known >= 2) guard = at(free);
                    if (known >= 3) other = at(free + 2);
                } else {
                    x = at(count - 1 - d);
                    y = at(known - 1 - (known >= 2));
                    if (known >= 2) guard = at(known - 1);
                    if (known >= 3) other = at(known - 3);
                }
                if (positives) combine(x, y, guard, other, 0, false);
                else combine(y, x, guard, other, 0, false);
            } else {
                if (count <= 4 && blocks.size() == 1) selectSmallExtrema();
                int d = (count - 2) / 3;
                if (initialOrderKnown && d) d = 1;
                initialOrderKnown = false;
                int j = count - 1 - d;
                int sameBlock = blockAt(d);
                if (sameBlock == blockAt(j) && blocks[sameBlock].size() >= 150 &&
                    !blocks[sameBlock].equalClass) {
                    int preceding = 0;
                    for (int i = 0; i < sameBlock; ++i) preceding += blocks[i].size();
                    partitionWithSample(sameBlock, d - preceding, j - preceding);
                }
                int x = at(d), y = at(j);
                if (finishEqualGroups()) return;
                int probe = count >= 5 ? blocks[blockAt(count / 2)].labels[0] : 0;
                int lower = d ? blocks[blockAt(d - 1)].labels[0] : 0;
                int upper = d ? blocks[blockAt(count - d)].labels[0] : 0;
                combine(x, y, probe, lower, upper, true);
            }
        }
        finish();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n == -1) return 0;
    Solver(n).run();
    cout << '!' << endl;
}

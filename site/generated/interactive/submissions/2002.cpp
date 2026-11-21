#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

constexpr int Limit = 1000000000;
constexpr int Half = 1 << 15;
constexpr int Mask = Half - 1;
constexpr int Counts = 31;

struct Tables {
    array<unsigned char, Half> bits{};

    Tables() {
        for (int i = 1; i < Half; ++i) {
            bits[i] = bits[i / 2] + (i & 1);
        }
    }

    int count(int n) const {
        return bits[n >> 15] + bits[n & Mask];
    }
};

const Tables table;

// Each group represents every combination of its high and low halves.
struct Group {
    vector<uint16_t> high;
    vector<uint16_t> low;
};

using Histogram = array<int, Counts>;

class Candidates {
public:
    explicit Candidates(int bitCount) {
        array<vector<uint16_t>, 16> highs, lows;
        for (int low = 0; low < Half; ++low) {
            lows[table.bits[low]].push_back(low);
        }
        for (int high = 0; high < (Limit >> 15); ++high) {
            highs[table.bits[high]].push_back(high);
        }
        for (int highCount = 0; highCount <= 15; ++highCount) {
            int lowCount = bitCount - highCount;
            if (lowCount >= 0 && lowCount <= 15 && !highs[highCount].empty()) {
                groups.push_back({move(highs[highCount]), move(lows[lowCount])});
            }
        }
        int top = Limit >> 15;
        vector<uint16_t> topLows;
        for (int low = 0; low <= (Limit & Mask); ++low) {
            if (table.bits[top] + table.bits[low] == bitCount) {
                topLows.push_back(low);
            }
        }
        if (!topLows.empty()) {
            groups.push_back({{static_cast<uint16_t>(top)}, move(topLows)});
        }
        measure();
    }

    Histogram histogram(int offset, Histogram* minimums = nullptr) const {
        Histogram result{};
        if (minimums) {
            minimums->fill(Limit);
        }
        int highOffset = offset >> 15;
        int lowOffset = offset & Mask;
        for (const Group& group : groups) {
            if (group.high.size() * group.low.size() <= group.high.size() + group.low.size() + 20) {
                for (int high : group.high) {
                    for (int low : group.low) {
                        int n = high * Half + low;
                        int count = table.count(n - offset);
                        ++result[count];
                        if (minimums) {
                            (*minimums)[count] = min((*minimums)[count], n);
                        }
                    }
                }
                continue;
            }
            int lowCounts[2][16]{};
            int lowMins[2][16];
            int highCounts[2][16]{};
            int highMins[2][16];
            int minBorrow = group.low.back() < lowOffset;
            int maxBorrow = group.low.front() < lowOffset;
            for (int low : group.low) {
                int borrow = low < lowOffset;
                int count = table.bits[(low - lowOffset) & Mask];
                if (++lowCounts[borrow][count] == 1) {
                    lowMins[borrow][count] = low;
                }
            }
            for (int borrow = minBorrow; borrow <= maxBorrow; ++borrow) {
                for (int high : group.high) {
                    int count = table.bits[high - highOffset - borrow];
                    if (++highCounts[borrow][count] == 1) {
                        highMins[borrow][count] = high;
                    }
                }
                for (int lowCount = 0; lowCount <= 15; ++lowCount) {
                    if (int frequency = lowCounts[borrow][lowCount]) {
                        for (int highCount = 0; highCount <= 15; ++highCount) {
                            int highFrequency = highCounts[borrow][highCount];
                            int count = lowCount + highCount;
                            result[count] += frequency * highFrequency;
                            if (minimums && highFrequency) {
                                int n = highMins[borrow][highCount] * Half + lowMins[borrow][lowCount];
                                (*minimums)[count] = min((*minimums)[count], n);
                            }
                        }
                    }
                }
            }
        }
        return result;
    }

    void retain(int offset, int response) {
        vector<Group> next;
        int highOffset = offset >> 15;
        int lowOffset = offset & Mask;
        for (Group& group : groups) {
            if (group.high.size() == 1 && group.low.size() == 1) {
                if (table.count(group.high.front() * Half + group.low.front() - offset) == response) {
                    next.push_back(move(group));
                }
                continue;
            }
            // Low-half subtraction determines the borrow and the required
            // high-half popcount. Those two values define the new groups.
            array<array<Group, 16>, 2> bins;
            for (int low : group.low) {
                int needed = response - table.bits[(low - lowOffset) & Mask];
                if (needed >= 0 && needed <= 15) {
                    bins[low < lowOffset][needed].low.push_back(low);
                }
            }
            int minBorrow = group.low.back() < lowOffset;
            int maxBorrow = group.low.front() < lowOffset;
            for (int borrow = minBorrow; borrow <= maxBorrow; ++borrow) {
                for (int high : group.high) {
                    Group& bin = bins[borrow][table.bits[high - highOffset - borrow]];
                    if (!bin.low.empty()) {
                        bin.high.push_back(high);
                    }
                }
                for (Group& bin : bins[borrow]) {
                    if (!bin.low.empty() && !bin.high.empty()) {
                        next.push_back(move(bin));
                    }
                }
            }
        }
        groups = move(next);
        measure();
    }

    pair<int, int> bitMasks(int offset) const {
        int common = (1 << 30) - 1;
        int present = 0;
        int highOffset = offset >> 15;
        int lowOffset = offset & Mask;
        for (const Group& group : groups) {
            int lowAnd = Mask;
            int lowOr = 0;
            for (int low : group.low) {
                int rest = (low - lowOffset) & Mask;
                lowAnd &= rest;
                lowOr |= rest;
            }
            int highAnd = Mask;
            int highOr = 0;
            int minBorrow = group.low.back() < lowOffset;
            int maxBorrow = group.low.front() < lowOffset;
            for (int borrow = minBorrow; borrow <= maxBorrow; ++borrow) {
                for (int high : group.high) {
                    int rest = high - highOffset - borrow;
                    highAnd &= rest;
                    highOr |= rest;
                }
            }
            common &= highAnd * Half + lowAnd;
            present |= highOr * Half + lowOr;
            if (common == 0 && present == (1 << 30) - 1) {
                break;
            }
        }
        return {common, present};
    }

    vector<int> sample(int amount, uint64_t& seed) const {
        vector<int> values;
        if (size <= amount) {
            values.reserve(size);
            for (const Group& group : groups) {
                for (int high : group.high) {
                    for (int low : group.low) {
                        values.push_back(high * Half + low);
                    }
                }
            }
        } else {
            values.reserve(amount);
            for (int i = 0; i < amount; ++i) {
                int rank = random(seed) % size;
                int index = upper_bound(ends.begin(), ends.end(), rank) - ends.begin();
                if (index > 0) {
                    rank -= ends[index - 1];
                }
                const Group& group = groups[index];
                int lowSize = group.low.size();
                values.push_back(group.high[rank / lowSize] * Half + group.low[rank % lowSize]);
            }
        }
        return values;
    }

    static uint32_t random(uint64_t& seed) {
        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;
        return (seed * 2685821657736338717ULL) >> 32;
    }

    int size = 0;
    int minimum = Limit;

private:
    void measure() {
        size = 0;
        minimum = Limit;
        ends.clear();
        for (const Group& group : groups) {
            size += group.high.size() * group.low.size();
            minimum = min(minimum, group.high.front() * Half + group.low.front());
            ends.push_back(size);
        }
    }

    vector<Group> groups;
    vector<int> ends;
};

constexpr int SampleSize = 4096;
constexpr int TrialCount = 520;
constexpr int ShortlistSize = 40;
constexpr int LookaheadSize = 10000;
constexpr int InitialSparse = 5;
constexpr double SingletonBonus = 1.5;

struct Plan {
    int target;
    Histogram continuation{};
};

Histogram sampleHistogram(const vector<int>& values, int offset) {
    Histogram counts{};
    for (int n : values) {
        ++counts[table.count(n - offset)];
    }
    return counts;
}

class Game {
public:
    explicit Game(int initialCount)
        : candidates(initialCount), response(initialCount), sparse(initialCount <= InitialSparse) {}

    int choose() {
        if (int target = exchange(planned, 0)) {
            return target;
        }
        int minimum = candidates.minimum;
        if (minimum == offset + 1 || candidates.size == 2) {
            return minimum;
        }

        auto [common, present] = candidates.bitMasks(offset);
        int unknown = response - table.count(common);
        if (unknown == 1) {
            // Clear the common bits, then use the borrow to locate the one varying bit.
            return offset + common + 1;
        }
        if (table.count(present) == response + 1) {
            // If one bit is missing, leave a power of two minus that bit.
            int power = bit_floor(static_cast<unsigned>(present ^ common));
            return offset + present - power;
        }
        if (candidates.size <= Counts) {
            auto values = candidates.sample(Counts, seed);
            if (int target = separator(values, offset, minimum, true)) {
                return target;
            }
        }

        Plan plan = sparse || turns >= 14 || unknown == 2
            ? chooseBit(common, unknown)
            : search(common);
        continuation = plan.continuation;
        return plan.target;
    }

    void update(int target, int nextResponse) {
        ++turns;
        planned = continuation[nextResponse];
        continuation.fill(0);
        offset = target;
        response = nextResponse;
        if (response == 0) {
            candidates.size = 1;
            candidates.minimum = offset;
        } else {
            candidates.retain(offset, response);
        }
    }

    Candidates candidates;
    int offset = 0;

private:
    vector<int> probes(int from, int to, int exhaustiveLimit, int randomTrials, int margins) {
        vector<int> options{to};
        int span = to - from;
        if (span <= exhaustiveLimit) {
            for (int target = from + 1; target < to; ++target) {
                options.push_back(target);
            }
            return options;
        }
        for (int delta = 1; delta <= margins && delta < span; ++delta) {
            options.push_back(from + delta);
            options.push_back(to - delta);
        }
        for (int power = 1; power <= span; power <<= 1) {
            options.push_back(from + power);
            options.push_back(to - power + 1);
            int floor = to & -power;
            if (floor > from) {
                options.push_back(floor);
                if (floor > from + 1) {
                    options.push_back(floor - 1);
                }
            }
        }
        for (int i = 0; i < randomTrials; ++i) {
            options.push_back(from + 1 + Candidates::random(seed) % span);
        }
        return options;
    }

    static bool separates(const vector<int>& values, int target) {
        uint32_t seen = 0;
        for (int n : values) {
            uint32_t bit = uint32_t{1} << table.count(n - target);
            if (seen & bit) {
                return false;
            }
            seen |= bit;
        }
        return true;
    }

    static int exactTriple(const vector<int>& values, int from) {
        // Count the differences in borrows between three equal-popcount values.
        // The common popcount and the subtrahend cancel in each difference.
        array<int, 30> zeroMask{};
        for (int i = 0; i < 30; ++i) {
            for (int j = 0; j < 3; ++j) {
                if (((values[j] - from) & (1 << i)) == 0) {
                    zeroMask[i] |= 1 << j;
                }
            }
        }
        array<unordered_set<int>, 30> failed;
        auto dfs = [&](auto&& self, int bit, int borrow, int first, int second) -> int {
            if (bit == 30) {
                return borrow == 0 && first && second && first != second ? 0 : -1;
            }
            int key = ((first + 30) * 61 + second + 30) * 8 + borrow;
            if (failed[bit].contains(key)) {
                return -1;
            }
            for (int digit = 0; digit <= 1; ++digit) {
                int next = digit ? (borrow | zeroMask[bit]) : (borrow & zeroMask[bit]);
                int reference = next & 1;
                int rest = self(self, bit + 1, next,
                    first + ((next >> 1) & 1) - reference,
                    second + ((next >> 2) & 1) - reference);
                if (rest >= 0) {
                    return rest + (digit << bit);
                }
            }
            failed[bit].insert(key);
            return -1;
        };
        int delta = dfs(dfs, 0, 0, 0, 0);
        return delta > 0 ? from + delta : 0;
    }

    int separator(const vector<int>& values, int from, int to, bool thorough) {
        if (separates(values, to)) {
            return to;
        }
        constexpr int Exhaustive = 192;
        for (int target : probes(from, to, Exhaustive, thorough ? 300 : 100, 5)) {
            if (target != to && separates(values, target)) {
                return target;
            }
        }
        if (thorough && to - from > Exhaustive) {
            // A borrow changes when the offset passes a candidate's low bits.
            for (int n : values) {
                for (int power = 2; power <= Limit; power <<= 1) {
                    int target = (to & -power) + (n & (power - 1)) + 1;
                    if (target > to) {
                        target -= power;
                    }
                    if (target > from && separates(values, target)) {
                        return target;
                    }
                }
            }
            if (values.size() == 3) {
                return exactTriple(values, from);
            }
        }
        return 0;
    }

    Plan chooseBit(int common, int unknown) {
        int base = offset + common;
        int available = candidates.minimum - base;
        int bestTarget = base + 1;
        int mostSingles = -1;
        double bestCost = numeric_limits<double>::infinity();
        // A borrow exposes one previously varying bit. Its replacement bits
        // are common to the whole response branch and can be cleared next time.
        for (int power = 1; power <= available; power <<= 1) {
            int target = base + power;
            auto frequencies = candidates.histogram(target);
            double cost = 0.0;
            int singles = count(frequencies.begin(), frequencies.end(), 1);
            for (int frequency : frequencies) {
                if (frequency > 1) {
                    cost += frequency * log2(frequency);
                }
            }
            if (singles > mostSingles || (singles == mostSingles && cost < bestCost)) {
                bestCost = cost;
                mostSingles = singles;
                bestTarget = target;
            }
        }
        if (unknown == 2) {
            return finishInTwo(bestTarget, mostSingles);
        }
        return {bestTarget};
    }

    Plan finishInTwo(int fallback, int mostSingles) {
        auto values = candidates.sample(candidates.size, seed);
        vector<pair<int, int>> options;
        for (int target : probes(offset, candidates.minimum, 256, 256, 8)) {
            auto histogram = sampleHistogram(values, target);
            int singles = count(histogram.begin(), histogram.end(), 1);
            if (singles > mostSingles) {
                options.emplace_back(-singles, target);
            }
        }
        sort(options.begin(), options.end());
        options.erase(unique(options.begin(), options.end()), options.end());
        for (auto [negativeSingles, target] : options) {
            array<vector<int>, Counts> buckets;
            for (int n : values) {
                buckets[table.count(n - target)].push_back(n);
            }
            Plan plan{target};
            bool solved = true;
            for (int count = 0; count < Counts; ++count) {
                const auto& bucket = buckets[count];
                if (bucket.size() > 1) {
                    if (bucket.size() <= Counts) {
                        plan.continuation[count] = separator(bucket, target,
                            *min_element(bucket.begin(), bucket.end()), false);
                    }
                    if (plan.continuation[count] == 0) {
                        solved = false;
                        break;
                    }
                }
            }
            if (solved) {
                return plan;
            }
        }
        return {fallback};
    }

    static double score(const Histogram& histogram) {
        double cost = 0.0;
        for (int frequency : histogram) {
            if (frequency > 1) {
                cost += frequency * (log2(frequency) + SingletonBonus);
            }
        }
        return cost;
    }

    double lookahead(Plan& plan, const vector<int>& values, const vector<double>& costs) {
        array<vector<int>, Counts> buckets;
        for (int n : values) {
            buckets[table.count(n - plan.target)].push_back(n);
        }
        double cost = 0.0;
        for (int count = 0; count < Counts; ++count) {
            const auto& bucket = buckets[count];
            int size = bucket.size();
            if (size <= 1) {
                continue;
            }
            cost += 3.0 * size;
            int minimum = *min_element(bucket.begin(), bucket.end());
            if (size == 2) {
                plan.continuation[count] = minimum;
                continue;
            }
            double best = costs[size];
            for (int target : probes(plan.target, minimum, 80, 75, 3)) {
                auto histogram = sampleHistogram(bucket, target);
                double estimate = 0.0;
                for (int frequency : histogram) {
                    estimate += costs[frequency];
                }
                best = min(best, estimate);
                if (best == 0.0) {
                    plan.continuation[count] = target;
                    break;
                }
            }
            cost += best;
        }
        return cost;
    }

    Plan search(int common) {
        int last = candidates.minimum;
        const auto sample = candidates.sample(SampleSize, seed);
        bool complete = static_cast<int>(sample.size()) == candidates.size;
        vector<double> costs(sample.size() + 1);
        for (size_t i = 2; i < costs.size(); ++i) {
            costs[i] = i * (log2(i) + SingletonBonus);
        }
        vector<pair<double, int>> options;
        auto evaluate = [&](int target) {
            auto histogram = sampleHistogram(sample, target);
            double cost = 0.0;
            for (int frequency : histogram) {
                cost += costs[frequency];
            }
            options.emplace_back(cost, -target);
            return complete && cost == 0.0;
        };
        for (int target : probes(offset, last, 512, TrialCount, 16)) {
            if (evaluate(target)) {
                return {target};
            }
        }
        if (common && evaluate(offset + common + 1)) {
            return {offset + common + 1};
        }
        sort(options.begin(), options.end());
        options.erase(unique(options.begin(), options.end()), options.end());
        if (last - offset > 512) {
            array<int, 4> starts{};
            for (int i = 0; i < 4; ++i) {
                starts[i] = -options[i].second;
            }
            for (int start : starts) {
                for (int bit = 1; bit <= last; bit <<= 1) {
                    int target = start ^ bit;
                    if (offset < target && target <= last && evaluate(target)) {
                        return {target};
                    }
                }
            }
            sort(options.begin(), options.end());
            options.erase(unique(options.begin(), options.end()), options.end());
        }

        // Sampling proposes offsets. Exact branch sizes and minima decide
        // which offsets are worth keeping and how far the next query may go.
        vector<pair<double, int>> shortlist;
        auto measure = [&](int target) {
            Histogram minimums;
            auto histogram = candidates.histogram(target, &minimums);
            if (*max_element(histogram.begin(), histogram.end()) == candidates.size) {
                return;
            }
            double cost = score(histogram);
            for (int count = 0; count < Counts; ++count) {
                if (histogram[count] > 1) {
                    cost -= 0.08 * histogram[count] * log2(minimums[count] - target);
                }
            }
            shortlist.emplace_back(cost, -target);
        };
        measure(last);
        int checked = 0;
        for (auto [estimate, negativeTarget] : options) {
            if (-negativeTarget != last) {
                measure(-negativeTarget);
                if (++checked == ShortlistSize) {
                    break;
                }
            }
        }
        sort(shortlist.begin(), shortlist.end());
        Plan best{-shortlist.front().second};
        if (candidates.size > LookaheadSize) {
            return best;
        }
        const auto values = complete ? sample : candidates.sample(candidates.size, seed);
        costs.resize(values.size() + 1);
        for (size_t i = sample.size() + 1; i < costs.size(); ++i) {
            costs[i] = i * (log2(i) + SingletonBonus);
        }
        double bestCost = numeric_limits<double>::infinity();
        checked = 0;
        for (auto [cost, negativeTarget] : shortlist) {
            if (checked++ >= 10 && -negativeTarget != last) {
                continue;
            }
            Plan plan{-negativeTarget};
            double total = lookahead(plan, values, costs) + 0.12 * cost;
            if (total < bestCost) {
                bestCost = total;
                best = plan;
            }
        }
        return best;
    }

    uint64_t seed = 0xEC09F583482897E3ULL;
    int response;
    bool sparse;
    int turns = 0;
    int planned = 0;
    Histogram continuation{};
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int initialCount;
    if (!(cin >> initialCount) || initialCount < 1 || initialCount > 30) {
        return 0;
    }
    Game game(initialCount);
    while (game.candidates.size > 1) {
        int nextOffset = game.choose();
        cout << "- " << nextOffset - game.offset << '\n' << flush;
        int response;
        if (!(cin >> response) || response < 0 || response > 30) {
            return 0;
        }
        game.update(nextOffset, response);
    }
    if (game.candidates.size == 1) {
        cout << "! " << game.candidates.minimum << '\n' << flush;
    }
    return 0;
}

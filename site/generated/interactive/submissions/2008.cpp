#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cmath>
#include <coroutine>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace std;

constexpr int MaxN = 1000;
constexpr int LeafSize = 38;
using Mask = bitset<MaxN>;

struct Bounds {
    int low;
    int high;
};

Bounds intersect(Bounds a, Bounds b) {
    return {max(a.low, b.low), min(a.high, b.high)};
}

struct Weighing {
    Mask mask;
    Bounds count;
};

class Search {
public:
    struct promise_type {
        Weighing question;
        int response = 0;

        Search get_return_object() {
            return Search(coroutine_handle<promise_type>::from_promise(*this));
        }
        suspend_always initial_suspend() noexcept { return {}; }
        suspend_always final_suspend() noexcept { return {}; }
        void return_void() noexcept {}
        void unhandled_exception() { terminate(); }

        struct AwaitResponse {
            const promise_type& state;
            bool await_ready() const noexcept {
                return state.question.count.low == state.question.count.high;
            }
            void await_suspend(coroutine_handle<>) const noexcept {}
            int await_resume() const noexcept {
                return await_ready() ? state.question.count.low : state.response;
            }
        };

        AwaitResponse yield_value(Weighing next) {
            question = move(next);
            return {*this};
        }
    };

    explicit Search(coroutine_handle<promise_type> frame) : frame(frame) {}
    Search(const Search&) = delete;
    Search(Search&& other) noexcept : frame(exchange(other.frame, {})) {}
    ~Search() { if (frame) frame.destroy(); }

    void start() { frame.resume(); }
    bool done() const { return frame.done(); }
    Weighing question() const { return frame.promise().question; }
    void reply(int answer) {
        frame.promise().response = answer;
        frame.resume();
    }

private:
    coroutine_handle<promise_type> frame;
};

struct Plan {
    int right = 0;
    int extra = 0;
    double expected = 0;
};

struct Context {
    array<Plan, MaxN + 1> plans{};
    array<Mask, MaxN + 1> prefix{};
    array<double, MaxN + 1> logFactorial{};
    Mask known, trueBits;
    uint32_t randomState = 0x9e3779b9U;

    explicit Context(int n) {
        const double smallCost[] = {0, 0, 0.5, 1.25, 1.82, 2.38, 2.95, 3.36, 3.78, 4.17, 4.57,
                                    4.91, 5.26, 5.60, 5.99, 6.26, 6.61, 6.90, 7.25, 7.54,
                                    7.88, 8.16, 8.45, 8.72, 9.02, 9.28, 9.54, 9.81, 10.08};
        // Balance the two query streams and reserve roughly one extra bit
        // per expected pair. These estimates only choose the partition.
        for (int len = 1; len <= n; ++len) {
            if (len <= LeafSize) {
                plans[len].expected = len <= 26 ? smallCost[len] : smallCost[26] + 0.255 * (len - 26);
                continue;
            }
            double best = 1e9;
            for (int extra = 0; extra <= len / 3; ++extra) {
                int middle = (len - extra) / 2;
                for (int right = max(1, middle - 3); right <= middle + 3; ++right) {
                    int left = len - extra - right;
                    if (left < 1) continue;
                    double a = plans[left].expected + 0.35;
                    double b = plans[right].expected;
                    double small = min(a, b);
                    double safety = 0.45 + 0.045 * sqrt(double(len));
                    double spill = max(0.0, extra - small + safety);
                    double cost = 1 + a + b + 0.72 * spill;
                    if (cost >= best) continue;
                    best = cost;
                    plans[len] = {right, extra, cost};
                }
            }
        }

        for (int i = 1; i <= n; ++i) {
            prefix[i] = prefix[i - 1];
            prefix[i].set(i - 1);
            logFactorial[i] = logFactorial[i - 1] + log(double(i));
        }
    }

    Mask range(int first, int len) const {
        return prefix[first + len] ^ prefix[first];
    }

    void set(int i, bool value) {
        known.set(i);
        trueBits.set(i, value);
    }

    void fill(const Mask& mask, bool value) {
        known |= mask;
        if (value) trueBits |= mask;
        else trueBits &= ~mask;
    }

    uint32_t random() {
        randomState ^= randomState << 13;
        randomState ^= randomState >> 17;
        randomState ^= randomState << 5;
        return randomState;
    }

    double logChoose(int n, int k) const {
        return logFactorial[n] - logFactorial[k] - logFactorial[n - k];
    }

    double splitCost(int n, int w) const {
        double denom = logChoose(n, w);
        double result = 0;
        for (int pieces = 1; pieces < n; pieces *= 2) {
            int shortSize = n / pieces, longCount = n % pieces;
            for (auto [size, count] : {pair{shortSize, pieces - longCount},
                                      pair{shortSize + 1, longCount}}) {
                if (size < 2 || count == 0) continue;
                double mixed = 1.0;
                if (n - size >= w) mixed -= exp(logChoose(n - size, w) - denom);
                if (n - size >= n - w) mixed -= exp(logChoose(n - size, n - w) - denom);
                result += count * mixed;
            }
        }
        return result;
    }
};

class Block {
public:
    Block(Context& ctx, int first, int len, Block* parent = nullptr, int weight = -1)
        : ctx(ctx), first(first), len(len), full(ctx.range(first, len)),
          parent(parent), fixedWeight(weight), local{0, len} {}

    Search solve() {
        Bounds total = weightBounds();
        if (total.high == 0 || total.low == len) {
            finish(total.low);
            co_return;
        }
        const Plan plan = ctx.plans[len];
        if (len > 10 && total.low == total.high &&
            (min(total.low, len - total.low) == 1 ||
             (len > LeafSize && ctx.splitCost(len, total.low) < 0.95 * plan.expected))) {
            Search split = solveBySplitting(total.low);
            split.start();
            while (!split.done()) {
                int answer = co_yield split.question();
                split.reply(answer);
            }
            co_return;
        }
        if (len <= LeafSize) {
            Search small = solveSmall();
            small.start();
            while (!small.done()) {
                int answer = co_yield small.question();
                small.reply(answer);
            }
            finishIfKnown();
            co_return;
        }

        int rightBegin = first + len - plan.right;
        Mask rightMask = ctx.range(rightBegin, plan.right);
        int rightWeight = co_yield Weighing{rightMask, subsetBounds(rightMask, total)};
        extraCount = plan.extra;
        if (rightWeight == 0 || rightWeight == plan.right) extraCount = 0;
        int leftSize = len - plan.right - extraCount;
        extraBegin = first + leftSize;
        right = make_unique<Block>(ctx, rightBegin, plan.right, nullptr, rightWeight);
        left = make_unique<Block>(ctx, first, leftSize, this);
        Search fromLeft = left->solve(), fromRight = right->solve();
        fromRight.start();
        fromLeft.start();

        while (extraDone < extraCount && !fromLeft.done() && !fromRight.done()) {
            Weighing a = fromLeft.question(), b = fromRight.question();
            Mask extraBit;
            extraBit.set(extraBegin + extraDone);

            // The sum of the two counts is twice a's count plus rightWeight
            // and the extra bit, since the two right masks are complements.
            Mask firstMask = a.mask | b.mask | extraBit;
            int sum = co_yield Weighing{firstMask, {a.count.low + b.count.low,
                                                    a.count.high + b.count.high + 1}};
            Bounds possibleA = intersect(a.count, {sum - b.count.high - 1, sum - b.count.low});
            Bounds possibleB = intersect(b.count, {sum - a.count.high - 1, sum - a.count.low});
            int leftAnswer, rightAnswer, bit;
            if (possibleA.low == possibleA.high && possibleB.low == possibleB.high) {
                leftAnswer = possibleA.low;
                rightAnswer = possibleB.low;
                bit = sum - leftAnswer - rightAnswer;
            } else {
                Mask secondMask = a.mask | (rightMask ^ b.mask);
                int other = co_yield Weighing{secondMask,
                    {possibleA.low + rightWeight - possibleB.high,
                     possibleA.high + rightWeight - possibleB.low}};
                int doubled = sum + other - rightWeight;
                bit = doubled & 1;
                leftAnswer = doubled / 2;
                rightAnswer = sum - leftAnswer - bit;
            }
            ctx.set(extraBegin + extraDone, bit);
            ++extraDone;
            extraWeight += bit;
            fromLeft.reply(leftAnswer);
            fromRight.reply(rightAnswer);
        }

        // Learn the remaining extra bits before resuming a pending left query.
        // This makes the left total available as soon as the parent total is known.
        if (extraDone < extraCount) {
            int begin = extraBegin + extraDone, remaining = extraCount - extraDone;
            Mask rest = ctx.range(begin, remaining);
            Bounds whole = weightBounds();
            Bounds leftLocal = left->intrinsicBounds();
            Bounds restCount = intersect({0, remaining},
                {whole.low - rightWeight - extraWeight - leftLocal.high,
                 whole.high - rightWeight - extraWeight - leftLocal.low});
            int weight = co_yield Weighing{rest, restCount};
            Block cleanup(ctx, begin, remaining, nullptr, weight);
            Search clean = cleanup.solve();
            clean.start();
            while (!clean.done()) {
                int answer = co_yield clean.question();
                clean.reply(answer);
            }
            extraDone = extraCount;
            extraWeight += weight;
        }

        Search remainder = merge(fromLeft, fromRight);
        remainder.start();
        while (!remainder.done()) {
            int answer = co_yield remainder.question();
            remainder.reply(answer);
        }
        finishIfKnown();
    }

private:
    Context& ctx;
    int first, len;
    Mask full;
    Block* parent;
    int fixedWeight;
    Bounds local;
    bool finished = false;
    unique_ptr<Block> left, right;
    int extraBegin = 0, extraCount = 0, extraDone = 0, extraWeight = 0;
    vector<uint64_t> candidates;

    // A left block may start before its exact total is available. Its
    // measurements must distinguish candidates with the same total.
    Bounds intrinsicBounds() const {
        if (finished) return local;
        if (fixedWeight >= 0) return {fixedWeight, fixedWeight};
        if (!left) return local;
        Bounds child = left->intrinsicBounds();
        int base = right->fixedWeight + extraWeight;
        return {base + child.low, base + child.high + extraCount - extraDone};
    }

    Bounds weightBounds() const {
        Bounds own = intrinsicBounds();
        if (!parent || own.low == own.high) return own;
        Bounds above = parent->weightBounds();
        int other = parent->right->fixedWeight + parent->extraWeight;
        return intersect(own, {above.low - other - (parent->extraCount - parent->extraDone),
                               above.high - other});
    }

    Bounds subsetBounds(const Mask& subset, Bounds total) const {
        int knownInside = (subset & ctx.trueBits).count();
        int knownOutside = ((full ^ subset) & ctx.trueBits).count();
        int unknownInside = (subset & ~ctx.known).count();
        int unknownOutside = ((full ^ subset) & ~ctx.known).count();
        return {max(knownInside, total.low - knownOutside - unknownOutside),
                min(knownInside + unknownInside, total.high - knownOutside)};
    }

    void finishIfKnown() {
        Bounds weight = weightBounds();
        if (weight.low == weight.high) finish(weight.low);
    }

    void finish(int weight) {
        if (finished) return;
        if (weight == 0 || weight == len) {
            ctx.fill(full, weight != 0);
        } else if (left) {
            right->finish(right->fixedWeight);
            left->finish(weight - right->fixedWeight - extraWeight);
        } else {
            auto it = find_if(candidates.begin(), candidates.end(), [weight](uint64_t x) {
                return popcount(x) == weight;
            });
            if (it == candidates.end()) terminate();
            for (int j = 0; j < len; ++j) ctx.set(first + j, (*it >> j) & 1U);
        }
        local = {weight, weight};
        finished = true;
    }

    Search merge(Search& a, Search& b) {
        while (!a.done() && !b.done()) {
            Weighing x = a.question(), y = b.question();
            if (x.count.low == x.count.high) {
                a.reply(x.count.low);
                continue;
            }
            if (y.count.low == y.count.high) {
                b.reply(y.count.low);
                continue;
            }

            int sum = co_yield Weighing{x.mask | y.mask,
                {x.count.low + y.count.low, x.count.high + y.count.high}};
            x.count = intersect(x.count, {sum - y.count.high, sum - y.count.low});
            int leftAnswer = co_yield x;
            a.reply(leftAnswer);
            b.reply(sum - leftAnswer);
        }
        for (Search* child : {&a, &b}) {
            while (!child->done()) {
                int answer = co_yield child->question();
                child->reply(answer);
            }
        }
    }

    Search solveBySplitting(int weight) {
        int half = len / 2;
        Mask part = ctx.range(first, half);
        int a = co_yield Weighing{part, subsetBounds(part, {weight, weight})};
        Block lower(ctx, first, half, nullptr, a);
        Block upper(ctx, first + half, len - half, nullptr, weight - a);
        Search leftTask = lower.solve(), rightTask = upper.solve();
        leftTask.start();
        rightTask.start();
        Search both = merge(leftTask, rightTask);
        both.start();
        while (!both.done()) {
            int answer = co_yield both.question();
            both.reply(answer);
        }
        local = {weight, weight};
        finished = true;
    }

    void noteCertain(uint64_t all, uint64_t any) {
        uint64_t certain = ((1ULL << len) - 1) ^ (any ^ all);
        while (certain) {
            int j = countr_zero(certain);
            ctx.set(first + j, (all >> j) & 1U);
            certain &= certain - 1;
        }
    }

    uint64_t bestSmallQuery(uint64_t active) {
        vector<uint64_t> probes;
        size_t guesses = min<size_t>(candidates.size(), 16);
        for (size_t i = 0; i < guesses; ++i) probes.push_back(candidates[i] & active);
        if (popcount(active) <= 9) {
            uint64_t firstBit = active & -active;
            for (uint64_t q = active; q; q = (q - 1) & active) {
                if ((q & firstBit) && q != active) probes.push_back(q);
            }
        } else {
            for (uint64_t x = active; x; x &= x - 1) probes.push_back(x & -x);
            for (int i = 0; i < 72; ++i) probes.push_back(((uint64_t(ctx.random()) << 32) | ctx.random()) & active);
        }

        constexpr size_t SampleSize = 2048;
        vector<pair<uint64_t, int>> sample;
        size_t size = min(SampleSize, candidates.size());
        sample.reserve(size);
        for (size_t i = 0; i < size; ++i) {
            size_t begin = i * candidates.size() / size;
            size_t end = (i + 1) * candidates.size() / size;
            uint64_t key = candidates[begin + (end - begin > 1 ? ctx.random() % (end - begin) : 0)];
            sample.emplace_back(key, popcount(key));
        }

        uint64_t best = 0;
        int64_t bestScore = INT64_MAX;
        for (uint64_t q : probes) {
            if (!q || q == active) continue;
            array<array<int, LeafSize + 1>, LeafSize + 1> joint{};
            array<int, LeafSize + 1> counts{};
            int64_t score = 0;
            for (auto [key, w] : sample) {
                int k = popcount(key & q);
                score += 4 * joint[w][k]++ + counts[k]++;
            }
            if (score < bestScore) {
                bestScore = score;
                best = q;
            }
        }
        return best;
    }

    // Join the half-blocks by their subset-count signatures. Verify every
    // match so a signature collision cannot change the candidate set.
    bool enumerateWide(int firstCount, const vector<pair<uint64_t, int>>& constraints,
                       const array<uint64_t, LeafSize>& columns, uint64_t target) {
        constexpr size_t MaxCandidates = 65000;
        int half = len / 2;
        auto sums = [&](int first, int count) {
            vector<uint64_t> all(1U << count);
            for (uint32_t mask = 1; mask < all.size(); ++mask) {
                all[mask] = all[mask & (mask - 1)] + columns[first + countr_zero(mask)];
            }
            return all;
        };
        vector<pair<uint64_t, uint32_t>> lower;
        vector<uint64_t> leftSums = sums(0, half);
        for (uint32_t mask = 0; mask < leftSums.size(); ++mask) {
            if (popcount(mask) == firstCount) lower.emplace_back(leftSums[mask], mask);
        }
        sort(lower.begin(), lower.end());
        vector<uint64_t> rightSums = sums(half, len - half);
        Bounds allowed = weightBounds();
        for (uint32_t mask = 0; mask < rightSums.size(); ++mask) {
            int weight = firstCount + popcount(mask);
            if (weight < allowed.low || weight > allowed.high) continue;
            uint64_t want = target - rightSums[mask];
            auto it = lower_bound(lower.begin(), lower.end(), pair<uint64_t, uint32_t>{want, 0});
            for (; it != lower.end() && it->first == want; ++it) {
                uint64_t key = (uint64_t(mask) << half) | it->second;
                bool matches = true;
                for (auto [subset, count] : constraints) {
                    if (popcount(key & subset) != count) {
                        matches = false;
                        break;
                    }
                }
                if (!matches) continue;
                candidates.push_back(key);
                if (candidates.size() > MaxCandidates) {
                    candidates.clear();
                    return false;
                }
            }
        }
        return true;
    }

    Search prepareWide() {
        int half = len / 2, firstHalfCount;
        Mask leftHalf = ctx.range(first, half);
        firstHalfCount = co_yield Weighing{leftHalf, subsetBounds(leftHalf, weightBounds())};
        local = {firstHalfCount, firstHalfCount + len - half};
        if (firstHalfCount == 0 || firstHalfCount == half) ctx.fill(leftHalf, firstHalfCount != 0);

        uint64_t universe = (1ULL << len) - 1;
        uint64_t leftBits = (1ULL << half) - 1;
        vector<pair<uint64_t, int>> constraints;
        array<uint64_t, LeafSize> columns{};
        uint64_t target = 0;
        Bounds allowed = weightBounds();
        double rightStates = 0;
        for (int w = allowed.low; w <= allowed.high; ++w) {
            rightStates += exp(ctx.logChoose(len - half, w - firstHalfCount));
        }
        double bits = (ctx.logChoose(half, firstHalfCount) + log(rightStates)) / log(2.0);
        int firstAttempt = max(0, int((bits - 16.0) / 2.15));
        for (int j = 0; ; ++j) {
            if (j >= firstAttempt && enumerateWide(firstHalfCount, constraints, columns, target)) co_return;
            uint64_t best = 0;
            int bestPenalty = 100000;
            if (j < 13) {
                for (int i = 0; i < 64; ++i) {
                    uint64_t q = ((uint64_t(ctx.random()) << 32) | ctx.random()) & universe;
                    int penalty = 6 * (abs(2 * popcount(q & leftBits) - half) +
                                       abs(2 * popcount(q & (universe ^ leftBits)) - (len - half)));
                    for (auto [past, _] : constraints) {
                        penalty += abs(4 * popcount(q & past) - 2 * popcount(q) -
                                       2 * popcount(past) + len);
                    }
                    if (penalty < bestPenalty) {
                        best = q;
                        bestPenalty = penalty;
                    }
                }
            } else {
                // Singleton constraints guarantee progress even on an
                // unusually uninformative sequence of balanced probes.
                best = 1ULL << (j - 13);
            }
            Mask mask = Mask(best) << first;
            int count = co_yield Weighing{mask, subsetBounds(mask, weightBounds())};
            constraints.emplace_back(best, count);
            uint64_t coefficient = (uint64_t(ctx.random()) << 32) | ctx.random();
            target += coefficient * count;
            for (uint64_t set = best; set; set &= set - 1) {
                columns[countr_zero(set)] += coefficient;
            }
        }
    }

    Search solveSmall() {
        Bounds allowed = weightBounds();
        const uint64_t end = 1ULL << len;
        if (len >= 20) {
            Search prep = prepareWide();
            prep.start();
            while (!prep.done()) {
                int answer = co_yield prep.question();
                prep.reply(answer);
            }
        } else {
            for (uint64_t key = 0; key < end; ++key) {
                int w = popcount(key);
                if (allowed.low <= w && w <= allowed.high) candidates.push_back(key);
            }
        }
        while (true) {
            allowed = weightBounds();
            erase_if(candidates, [allowed](uint64_t key) {
                int w = popcount(key);
                return w < allowed.low || w > allowed.high;
            });
            uint64_t all = end - 1, any = 0;
            array<int, LeafSize + 1> byWeight{};
            bool needQuestion = false;
            local = {len, 0};
            for (uint64_t key : candidates) {
                all &= key;
                any |= key;
                int w = popcount(key);
                needQuestion |= ++byWeight[w] > 1;
                local.low = min(local.low, w);
                local.high = max(local.high, w);
            }
            noteCertain(all, any);
            if (candidates.capacity() > 4096 && candidates.capacity() / 4 > candidates.size()) {
                candidates.shrink_to_fit();
            }
            if (!needQuestion) co_return;

            uint64_t bits = bestSmallQuery(any ^ all);
            Mask subset = Mask(bits) << first;
            Bounds count{len, 0};
            for (uint64_t key : candidates) {
                int answer = popcount(key & bits);
                count.low = min(count.low, answer);
                count.high = max(count.high, answer);
            }
            if (count.low == count.high) {
                bits = (any ^ all) & -(any ^ all);
                subset = Mask(bits) << first;
                count = {0, 1};
            }
            int got = co_yield Weighing{subset, count};
            erase_if(candidates, [bits, got](uint64_t key) {
                return popcount(key & bits) != got;
            });
        }
    }
};

class ParityEquations {
public:
    void add(Mask mask, bool value) {
        reduce(mask, value);
        if (mask.none()) return;
        size_t pivot = mask._Find_first();
        rows[pivot] = mask;
        rhs[pivot] = value;
    }

    int get(Mask mask) const {
        bool value = false;
        reduce(mask, value);
        return mask.none() ? int(value) : -1;
    }

private:
    array<Mask, MaxN> rows{};
    Mask rhs;

    void reduce(Mask& mask, bool& value) const {
        for (size_t pivot = mask._Find_first(); pivot < MaxN; pivot = mask._Find_first()) {
            if (rows[pivot].none()) break;
            mask ^= rows[pivot];
            value ^= rhs[pivot];
        }
    }
};

class Judge {
public:
    Judge(int n, Context& ctx) : n(n), ctx(ctx), full(ctx.prefix[n]) {}

    int start() {
        weight = n - submit({});
        parity.add(full, weight & 1);
        if (weight == 1 || weight == n - 1) {
            solveOne(weight == 1 ? Mask{} : full, full);
        }
        return weight;
    }

    int weigh(const Weighing& q) {
        if (q.count.low == q.count.high) return q.count.low;
        Mask unknown = full & ~ctx.known;
        Mask pending = q.mask & unknown;
        int knownInQuery = (q.mask & ctx.trueBits).count();
        int remainingWeight = weight - int(ctx.trueBits.count());
        if (pending.none()) return knownInQuery;
        if (pending == unknown) return knownInQuery + remainingWeight;

        Mask opposite = unknown ^ pending;
        for (const auto& [old, count] : history) {
            Mask effective = old & unknown;
            if (effective != pending && effective != opposite) continue;
            int unresolvedCount = count - int((old & ctx.trueBits).count());
            return knownInQuery + (effective == pending ? unresolvedCount
                                                        : remainingWeight - unresolvedCount);
        }

        int low = max(q.count.low - knownInQuery, remainingWeight - int(opposite.count()));
        int high = min(q.count.high - knownInQuery, min(remainingWeight, int(pending.count())));
        if (low == high) return knownInQuery + low;
        if (high == low + 1) {
            Mask newlyKnown = ctx.known & ~parityKnown;
            for (size_t i = newlyKnown._Find_first(); i < MaxN; i = newlyKnown._Find_next(i)) {
                Mask one;
                one.set(i);
                parity.add(one, ctx.trueBits.test(i));
            }
            parityKnown |= newlyKnown;
            int bit = parity.get(pending);
            if (bit >= 0) return knownInQuery + low + ((low & 1) ^ bit);
        }

        bool flip = abs(int(opposite.count()) - remainingWeight) <
                    abs(int(pending.count()) - remainingWeight);
        Mask sheet = ctx.trueBits | (flip ? opposite : pending);
        int score = submit(sheet);
        if (score == 0) {
            submit(full ^ sheet);
            exit(0);
        }
        if (score == n - 1 || score == 1) {
            Mask base = score == n - 1 ? sheet : full ^ sheet;
            Mask mismatch = (base ^ ctx.trueBits) & ctx.known;
            Mask choices = mismatch.any() ? mismatch : unknown;
            choices &= int(base.count()) > weight ? base : full ^ base;
            for (const auto& [old, count] : history) {
                if (int((old & base).count()) == count) choices &= ~old;
                else choices &= old;
            }
            solveOne(base, choices);
        }
        // score = n - weight - |sheet| + 2 * |sheet intersect key|.
        int sheetCount = (score - n + weight + int(sheet.count())) / 2;
        history.emplace_back(sheet, sheetCount);
        parity.add(sheet, sheetCount & 1);
        int unansweredInSheet = sheetCount - int(ctx.trueBits.count());
        return knownInQuery + (flip ? remainingWeight - unansweredInSheet : unansweredInSheet);
    }

    void finish() { submit(ctx.trueBits); }

private:
    int n, weight = 0;
    Context& ctx;
    Mask full;
    vector<pair<Mask, int>> history;
    ParityEquations parity;
    Mask parityKnown;

    [[noreturn]] void solveOne(const Mask& base, Mask choices) const {
        vector<int> positions;
        for (int i = 0; i < n; ++i) {
            if (choices.test(i)) positions.push_back(i);
        }
        int size = positions.size();
        if (!size) terminate();
        vector<int> totalCost(size + 1), split(size + 1);
        totalCost[1] = 1;
        // A singleton probe can itself be the correct answer.
        for (int count = 2; count <= size; ++count) {
            totalCost[count] = count + totalCost[count - 1];
            split[count] = 1;
            for (int k = 2; k <= count / 2; ++k) {
                int cost = count + totalCost[k] + totalCost[count - k];
                if (cost <= totalCost[count]) {
                    totalCost[count] = cost;
                    split[count] = k;
                }
            }
        }
        int begin = 0;
        while (size > 1) {
            int take = split[size];
            Mask group;
            for (int i = 0; i < take; ++i) group.set(positions[begin + i]);
            int score = submit(base ^ group);
            if (n - score == take - 1) {
                size = take;
            } else {
                begin += take;
                size -= take;
            }
        }
        Mask answer = base;
        answer.flip(positions[begin]);
        submit(answer);
        exit(0);
    }

    int submit(const Mask& sheet) const {
        string line(n, 'F');
        for (int i = 0; i < n; ++i) {
            if (sheet.test(i)) line[i] = 'T';
        }
        cout << line << '\n' << flush;
        int score;
        if (!(cin >> score) || score == -1 || score == n) exit(0);
        return score;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1 || n > MaxN) return 0;
    Context ctx(n);
    Judge judge(n, ctx);
    int weight = judge.start();
    Block root(ctx, 0, n, nullptr, weight);
    Search task = root.solve();
    task.start();
    while (!task.done()) {
        int response = judge.weigh(task.question());
        task.reply(response);
    }
    judge.finish();
}

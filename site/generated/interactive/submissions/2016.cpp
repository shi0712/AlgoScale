#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <numeric>
#include <random>
#include <unordered_map>
#include <vector>

using namespace std;

using Slots = __uint128_t;

Slots slot(int position) {
    return Slots(1) << position;
}

int countSlots(Slots slots) {
    return __builtin_popcountll(static_cast<uint64_t>(slots))
         + __builtin_popcountll(static_cast<uint64_t>(slots >> 64));
}

int firstSlot(Slots slots) {
    uint64_t low = static_cast<uint64_t>(slots);
    if (low) {
        return __builtin_ctzll(low);
    }
    return 64 + __builtin_ctzll(static_cast<uint64_t>(slots >> 64));
}

struct Support {
    // Bounds on the occupied slots of a fixed population of untouched dials.
    Slots required;
    Slots possible;

    bool normalize(int population) {
        int minimum = countSlots(required);
        if (minimum > population || (population && !possible)) {
            return false;
        }
        if (minimum == population) {
            possible = required;
        } else if (!(possible & (possible - 1))) {
            required = possible;
        }
        return true;
    }

    bool observe(Slots value, bool occupied) {
        if (occupied) {
            if (!(possible & value)) {
                return false;
            }
            required |= value;
        } else {
            if (required & value) {
                return false;
            }
            possible &= ~value;
        }
        return true;
    }
};

struct Guess {
    int origin;
    Support others;
};

template <class Query>
class Solver {
    int n;
    Query &query;
    vector<int> position;
    array<int, 100> knownAt{};
    array<int, 100> samples{};
    int sampleCount = 0;
    Support remaining{};
    int population;
    int target = -1;

    struct Plan {
        int destination;
        double score;
    };

    struct Located {
        int origin;
        int moves;
    };

    int forward(int from, int to) const {
        return (to - from + n) % n;
    }

    int plus(int at, int steps) const {
        int result = at + steps;
        return result >= n ? result - n : result;
    }

    int moveKnown(int dial) {
        --knownAt[position[dial]];
        position[dial] = plus(position[dial], 1);
        ++knownAt[position[dial]];
        return query(dial);
    }

    void park(int dial, int destination) {
        while (position[dial] != destination) {
            int reply = moveKnown(dial);
            if (knownAt[position[dial]] == 1) {
                remaining.observe(slot(position[dial]), reply);
                remaining.normalize(population);
            }
        }
    }

    array<double, 100> estimatedCounts() const {
        array<double, 100> weight{};
        double extras = population - countSlots(remaining.required);
        double prior = remaining.possible ? min(0.8, 4.0 / countSlots(remaining.possible)) : 0;
        double totalWeight = 0;
        for (Slots values = remaining.possible; values; values &= values - 1) {
            int v = firstSlot(values);
            weight[v] = prior + samples[v];
            totalWeight += weight[v];
        }
        for (int v = 0; v < n; ++v) {
            if (totalWeight > 0) {
                weight[v] *= extras / totalWeight;
            }
            if (remaining.required & slot(v)) {
                weight[v] += 1;
            }
        }
        return weight;
    }

    void finish() {
        int best = n * n;
        int destination = 0;
        for (int t = 0; t < n; ++t) {
            int cost = 0;
            for (int v = 0; v < n; ++v) {
                cost += knownAt[v] * forward(v, t);
            }
            if (cost < best) {
                best = cost;
                destination = t;
            }
        }
        for (int dial = 0; dial < n; ++dial) {
            while (position[dial] != destination) {
                moveKnown(dial);
            }
        }
    }

    Slots sequence(Slots values, int origin) const {
        int rotation = origin + 1;
        return (values >> rotation) | ((values << (n - rotation)) & (slot(n) - 1));
    }

    // Compare travel costs and the risk that a dial reaches the target while
    // another origin still predicts the same replies.
    Plan planTarget(double reserve, int only = -1) const {
        auto weights = estimatedCounts();
        array<int, 100> origins{};
        int size = 0;
        for (Slots values = remaining.possible; values; values &= values - 1) {
            origins[size++] = firstSlot(values);
        }
        double best = 1e20;
        int chosen = 0;
        array<Slots, 100> lowerSequence{}, upperSequence{}, optimisticSequence{}, anchorSequence{};
        array<int, 100> distance{};
        Slots predicted = remaining.required;
        for (int i = 0; i < size; ++i) {
            int v = origins[i];
            if (weights[v] > 0.5) {
                predicted |= slot(v);
            }
        }
        for (int t = 0; t < n; ++t) {
            if (only >= 0 && t != only) {
                continue;
            }
            double score = n * weights[t];
            for (int v = 0; v < n; ++v) {
                score += forward(v, t) * (knownAt[v] + weights[v]);
            }
            if (score >= best - 1e-8) {
                continue;
            }
            for (int i = 0; i < size; ++i) {
                int origin = origins[i];
                distance[i] = forward(origin, t);
                if (!distance[i]) {
                    distance[i] = n;
                }
                lowerSequence[i] = sequence(remaining.required | slot(t), origin);
                upperSequence[i] = sequence(remaining.possible | slot(t), origin);
                optimisticSequence[i] = sequence(predicted | slot(t), origin);
                anchorSequence[i] = sequence(slot(t), origin);
            }
            for (int i = 0; i < size; ++i) {
                if (distance[i] == n) {
                    continue;
                }
                Slots prefix = slot(distance[i]) - 1;
                int unsafe = 0, plausible = 0, unsupported = 0;
                for (int j = 0; j < size; ++j) {
                    if (i == j) {
                        continue;
                    }
                    Slots conflict = (lowerSequence[i] & ~upperSequence[j])
                                   | (lowerSequence[j] & ~upperSequence[i]);
                    if (!(((anchorSequence[i] & ~upperSequence[j])
                          | (anchorSequence[j] & ~upperSequence[i])) & prefix)) {
                        ++unsupported;
                    }
                    if (!(conflict & prefix)) {
                        ++unsafe;
                        if (!((optimisticSequence[i] ^ optimisticSequence[j]) & prefix)) {
                            ++plausible;
                        }
                    }
                }
                double penalty = plausible ? min(1.0, 0.55 + 0.15 * plausible)
                               : unsafe ? min(0.65, 0.22 + 0.12 * unsafe) : 0;
                score += n * (0.5 * penalty + (unsupported ? reserve : 0)) * weights[origins[i]];
                if (score >= best - 1e-8) {
                    break;
                }
            }
            if (score < best - 1e-8) {
                best = score;
                chosen = t;
            }
        }
        return {chosen, best};
    }

    int sampleMarker() const {
        auto weights = estimatedCounts();
        Slots values = remaining.possible;
        if (values & slot(0)) {
            weights[0] += (population - countSlots(values)) * 0.6;
        }
        double best = 1e20;
        int marker = 0;
        for (int t = 0; t <= min(8, n / 10); ++t) {
            array<Slots, 100> signals{};
            for (Slots starts = values; starts; starts &= starts - 1) {
                int v = firstSlot(starts);
                signals[v] = sequence(values | slot(t), v);
            }
            double score = t * 3.0;
            for (Slots starts = values; starts; starts &= starts - 1) {
                int v = firstSlot(starts);
                int needed = 0;
                for (Slots rivals = values & ~slot(v); rivals; rivals &= rivals - 1) {
                    int r = firstSlot(rivals);
                    Slots conflict = signals[v] ^ signals[r];
                    needed = max(needed, conflict ? firstSlot(conflict) + 1 : n * 3 / 2);
                }
                score += weights[v] * needed / population;
            }
            if (score < best - 1e-8) {
                best = score;
                marker = t;
            }
        }
        return marker;
    }

    bool periodicSupport() const {
        Slots mask = slot(n) - 1;
        Slots values = remaining.possible;
        for (int shift = 1; shift * 2 <= n; ++shift) {
            if (n % shift == 0
                && (((values << shift) & mask) | (values >> (n - shift))) == values) {
                return true;
            }
        }
        return false;
    }

    void observeActive(vector<Guess> &guesses, int steps, int reply) {
        size_t survivors = 0;
        for (Guess guess : guesses) {
            int at = plus(guess.origin, steps);
            bool consistent = knownAt[at] ? reply == 1
                                        : guess.others.observe(slot(at), reply);
            if (consistent && guess.others.normalize(population - 1)) {
                guesses[survivors++] = guess;
            }
        }
        guesses.resize(survivors);
    }

    void observeHelper(vector<Guess> &guesses, int steps, int at, int reply) {
        size_t survivors = 0;
        for (Guess guess : guesses) {
            bool guaranteed = knownAt[at] > 1 || plus(guess.origin, steps) == at;
            bool consistent = guaranteed ? reply == 1
                                         : guess.others.observe(slot(at), reply);
            if (consistent && guess.others.normalize(population - 1)) {
                guesses[survivors++] = guess;
            }
        }
        guesses.resize(survivors);
    }

    // After a full turn, the active dial has seen every slot of a fixed
    // background. Landing the scout on an empty slot adds one occupied slot
    // at a known coordinate. If the only empty landing is the scout's original
    // slot, departing it removes that occupied slot instead. On another turn,
    // the corresponding change in reply pins down the active dial.
    template <class Resolved, class Observe, class Advance>
    void breakSymmetry(Resolved resolved, Observe observe, Advance advance) {
        int start = position[0];
        while (!resolved()) {
            int reply = moveKnown(0);
            observe(reply);
            if (!reply) {
                if (position[0] == start && !resolved()) {
                    observe(moveKnown(0));
                }
                break;
            }
        }
        while (!resolved()) {
            advance();
        }
    }

    Located identify(int dial) {
        vector<Guess> guesses;
        guesses.reserve(n);
        for (Slots values = remaining.possible; values; values &= values - 1) {
            int origin = firstSlot(values);
            Support others{remaining.required & ~slot(origin), remaining.possible};
            if (others.normalize(population - 1)) {
                guesses.push_back({origin, others});
            }
        }

        int steps = 0;
        int moves = 0;
        auto advance = [&] {
            steps = plus(steps, 1);
            ++moves;
            observeActive(guesses, steps, query(dial));
        };
        while (moves < n && guesses.size() > 1) {
            advance();
        }
        if (guesses.size() > 1) {
            breakSymmetry([&] { return guesses.size() == 1; },
                [&](int reply) { observeHelper(guesses, steps, position[0], reply); },
                advance);
        }

        Guess exact = guesses.front();
        remaining = exact.others;
        --population;
        ++sampleCount;
        ++samples[exact.origin];
        position[dial] = plus(exact.origin, steps);
        ++knownAt[position[dial]];
        return {exact.origin, moves};
    }

    // One successor has a known dial and the other is kept empty.
    // A single reply identifies which of the two origins a dial had.
    void locateTwoOrigins(const vector<int> &order, int index) {
        int a = firstSlot(remaining.possible);
        int b = firstSlot(remaining.possible & ~slot(a));
        auto weights = estimatedCounts();
        double best = 1e20;
        int marker = -1, gate = -1, helper = -1;
        for (int side = 0; side < 2; ++side, swap(a, b)) {
            int m = plus(a, 1), g = plus(b, 1);
            if (remaining.possible & slot(g)) {
                continue;
            }
            for (int h = 0; h < n; ++h) {
                if (position[h] < 0) {
                    continue;
                }
                array<int, 100> after = knownAt;
                --after[position[h]];
                int clear = after[g];
                after[g] = 0;
                after[plus(g, 1)] += clear;
                ++after[m];
                double finish = 1e20;
                for (int t = 0; t < n; ++t) {
                    double cost = weights[a] * forward(m, t)
                                + weights[b] * (1 + forward(plus(g, 1), t));
                    for (int v = 0; v < n; ++v) {
                        cost += after[v] * forward(v, t);
                    }
                    finish = min(finish, cost);
                }
                double score = 10.0 * forward(position[h], m) + clear + finish;
                if (score < best) {
                    best = score;
                    marker = m;
                    gate = g;
                    helper = h;
                }
            }
        }

        park(helper, marker);
        for (int h = 0; h < n; ++h) {
            if (position[h] == gate) {
                park(h, plus(gate, 1));
            }
        }
        for (; index < static_cast<int>(order.size()); ++index) {
            int dial = order[index];
            if (countSlots(remaining.possible) == 1) {
                identify(dial);
                continue;
            }
            bool hit = query(dial);
            int current = hit ? marker : gate;
            int origin = plus(current, n - 1);
            remaining.required &= ~slot(origin);
            --population;
            remaining.normalize(population);
            position[dial] = current;
            ++knownAt[current];
            if (!hit && countSlots(remaining.possible) > 1) {
                park(dial, plus(gate, 1));
            }
        }
    }

    // Every unused origin occurs exactly once. A dial that might have reached
    // the goal can wait while later queries pin down its origin.
    void solveDistinct(const vector<int> &order) {
        Slots unused = remaining.possible;
        int held = -1, heldSteps = 0, heldMoves = 0;
        Slots heldOrigins = 0;
        const int goal = 1;

        auto moveFinished = [&](int dial) {
            int reply = moveKnown(dial);
            int at = position[dial];
            if (held >= 0) {
                for (Slots options = heldOrigins; options; options &= options - 1) {
                    int h = firstSlot(options);
                    bool hit = knownAt[at] > 1 || plus(h, heldSteps) == at
                             || ((unused & ~slot(h)) & slot(at));
                    if (hit != reply) {
                        heldOrigins &= ~slot(h);
                    }
                }
            }
            return reply;
        };

        auto release = [&] {
            if (held < 0 || (heldOrigins & (heldOrigins - 1))) {
                return;
            }
            int origin = firstSlot(heldOrigins);
            unused &= ~slot(origin);
            position[held] = plus(origin, heldSteps);
            ++knownAt[position[held]];
            int wasHeld = held;
            held = -1;
            if (heldMoves <= forward(origin, goal) || forward(position[wasHeld], goal) <= n / 2) {
                while (position[wasHeld] != goal) {
                    moveFinished(wasHeld);
                }
            }
        };

        moveFinished(0);
        for (int dial : order) {
            struct Option {
                int origin, heldOrigin;
                Slots occupied;
            };
            vector<Option> options;
            for (Slots origins = unused; origins; origins &= origins - 1) {
                int a = firstSlot(origins);
                if (held < 0) {
                    options.push_back({a, -1, unused & ~slot(a)});
                } else {
                    for (Slots guesses = heldOrigins & ~slot(a); guesses; guesses &= guesses - 1) {
                        int h = firstSlot(guesses);
                        Slots still = (unused & ~slot(a) & ~slot(h)) | slot(plus(h, heldSteps));
                        options.push_back({a, h, still});
                    }
                }
            }

            int steps = 0;
            int distance = 0;
            auto filter = [&](int reply, int helperLocation) {
                size_t survivors = 0;
                for (const Option &option : options) {
                    int current = plus(option.origin, steps);
                    int at = helperLocation < 0 ? current : helperLocation;
                    bool otherKnown = knownAt[at] > (helperLocation >= 0);
                    bool hit = otherKnown || (option.occupied & slot(at))
                             || (helperLocation >= 0 && at == current);
                    if (hit == reply) {
                        options[survivors++] = option;
                    }
                }
                options.resize(survivors);
            };

            auto resolved = [&] { return options.front().origin == options.back().origin; };
            auto advance = [&] {
                steps = plus(steps, 1);
                ++distance;
                filter(query(dial), -1);
            };

            while (!resolved() && distance < n) {
                advance();
                if (held < 0 && !resolved() && options.size() <= 5
                    && any_of(options.begin(), options.end(), [&](const Option &option) {
                        return plus(option.origin, steps) == goal;
                    })) {
                    held = dial;
                    heldSteps = steps;
                    heldMoves = distance;
                    heldOrigins = 0;
                    for (const Option &option : options) {
                        heldOrigins |= slot(option.origin);
                    }
                    break;
                }
            }
            if (held == dial) {
                continue;
            }

            if (!resolved()) {
                breakSymmetry(resolved, [&](int reply) { filter(reply, position[0]); }, advance);
            }

            int origin = options.front().origin;
            if (held >= 0) {
                heldOrigins = 0;
                for (const Option &option : options) {
                    heldOrigins |= slot(option.heldOrigin);
                }
            }
            unused &= ~slot(origin);
            position[dial] = plus(origin, steps);
            ++knownAt[position[dial]];
            if (distance <= forward(origin, goal) || forward(position[dial], goal) <= n / 2) {
                while (position[dial] != goal) {
                    moveFinished(dial);
                }
            }
            release();
        }
        finish();
    }

    int arcWidth() const {
        int maxGap = 0;
        for (Slots values = remaining.possible; values; values &= values - 1) {
            int at = firstSlot(values);
            int gap = 1;
            while (!(remaining.possible & slot(plus(at, gap)))) {
                ++gap;
            }
            maxGap = max(maxGap, gap);
        }
        return n - maxGap;
    }

public:
    Solver(int dialCount, Query &ask)
        : n(dialCount), query(ask), position(dialCount, -1), population(dialCount - 1) {}

    void run() {
        // All subsequent coordinates are relative to dial 0's initial value.
        position[0] = 0;
        knownAt[0] = 1;
        for (int i = 0; i < n; ++i) {
            if (moveKnown(0) == 1) {
                remaining.required |= slot(position[0]);
            }
        }
        remaining.possible = remaining.required;

        vector<int> order(n - 1);
        iota(order.begin(), order.end(), 1);
        mt19937 random(0x484d + n);
        shuffle(order.begin(), order.end(), random);

        int supportSize = countSlots(remaining.possible);
        if (supportSize == n - 1 && !(remaining.possible & slot(0))) {
            solveDistinct(order);
            return;
        }

        int width = arcWidth();
        bool periodic = periodicSupport();
        bool narrow = width <= max(4, n / 10);
        double reserve = narrow ? 0.25 : 0.04;
        int delay = 0;
        if (n >= 14 && supportSize > 4 && !periodic && !narrow
            && population - supportSize >= n / 9) {
            int sampleSize = width > n / 2 && supportSize < n / 3 ? 8 : 3;
            delay = min(sampleSize, n / 5);
        }

        auto commit = [&] {
            target = planTarget(reserve).destination;
            for (int dial = 0; dial < n; ++dial) {
                if (position[dial] >= 0 && (!dial || forward(position[dial], target) <= n * 3 / 4)) {
                    park(dial, target);
                }
            }
        };

        if (delay > 0) {
            park(0, sampleMarker());
        } else if (supportSize > 2) {
            commit();
        }

        for (int index = 0; index < n - 1; ++index) {
            int size = countSlots(remaining.possible);
            if (size == 1) {
                int value = firstSlot(remaining.possible);
                knownAt[value] += population;
                for (; index < n - 1; ++index) {
                    position[order[index]] = value;
                }
                break;
            }
            if (size == 2 && (target < 0 || population * 3 >= n)) {
                locateTwoOrigins(order, index);
                break;
            }

            int dial = order[index];
            Located located = identify(dial);
            if (target < 0 && sampleCount >= delay && countSlots(remaining.possible) > 1) {
                commit();
            }

            if (!periodic && target >= 0 && population > n / 3
                && countSlots(remaining.possible) > 2 && sampleCount % 7 == 0) {
                Plan next = planTarget(reserve);
                Plan current = planTarget(reserve, target);
                if (next.score + n * 0.75 < current.score
                    && (forward(target, next.destination) <= n / 4 || knownAt[target] < 4)) {
                    target = next.destination;
                    for (int i = 0; i < n; ++i) {
                        if (position[i] >= 0 && forward(position[i], target) <= n / 2) {
                            park(i, target);
                        }
                    }
                }
            }

            // Leave a dial just beyond the target for the final cost comparison.
            // Continuing to the target here would send it nearly a full turn.
            if (target >= 0 && population > 0
                && (located.moves <= forward(located.origin, target)
                    || forward(position[dial], target) <= n / 2)) {
                park(dial, target);
            }
        }

        finish();
    }
};

template <class Query>
class SmallSolver {
    static constexpr double informationCost = 0.85;

    int n;
    Query &query;
    // The state stores relative values with dial 0 at zero. Even and odd
    // transition entries encode the two possible replies.
    vector<array<int, 7>> next;
    vector<uint8_t> cost;
    vector<uint8_t> destination;
    vector<float> uncertainty;

    struct Decision {
        int cost;
        int dial;
    };

    unordered_map<uint64_t, Decision> exactMemo;

    void finish(int state) {
        int target = destination[state];
        for (int i = 0; i < n; ++i) {
            int at = i ? state % n : 0;
            if (i) {
                state /= n;
            }
            for (int d = (target - at + n) % n; d > 0; --d) {
                query(i);
            }
        }
    }

    array<uint64_t, 2> split(uint64_t belief, int dial) const {
        array<uint64_t, 2> result{};
        for (; belief; belief &= belief - 1) {
            int state = __builtin_ctzll(belief);
            int transition = next[state][dial];
            result[transition % 2] |= uint64_t(1) << (transition / 2);
        }
        return result;
    }

    Decision exact(uint64_t belief) {
        if (!(belief & (belief - 1))) {
            return {cost[__builtin_ctzll(belief)], -1};
        }
        if (auto entry = exactMemo.find(belief); entry != exactMemo.end()) {
            return entry->second;
        }

        struct Reachable {
            uint64_t belief;
            int distance;
            int firstDial;
        };
        struct Branch {
            int lowerBound;
            int soFar;
            int firstDial;
            uint64_t miss, hit;
        };

        int size = __builtin_popcountll(belief);
        vector<Reachable> reachable{{belief, 0, -1}};
        vector<Branch> branches;

        // No-information queries translate every state equally. Try the shortest
        // routes through these translations to the first informative query.
        for (size_t i = 0; i < reachable.size(); ++i) {
            Reachable current = reachable[i];
            for (int dial = 0; dial < n; ++dial) {
                auto [miss, hit] = split(current.belief, dial);
                int first = current.firstDial < 0 ? dial : current.firstDial;
                if (miss && hit) {
                    int soFar = (current.distance + 1) * size;
                    int lowerBound = soFar;
                    for (uint64_t states = miss | hit; states; states &= states - 1) {
                        lowerBound += cost[__builtin_ctzll(states)];
                    }
                    branches.push_back({lowerBound, soFar, first, miss, hit});
                } else {
                    uint64_t moved = miss | hit;
                    if (none_of(reachable.begin(), reachable.end(), [moved](const Reachable &item) {
                            return item.belief == moved;
                        })) {
                        reachable.push_back({moved, current.distance + 1, first});
                    }
                }
            }
        }

        sort(branches.begin(), branches.end(), [](const Branch &a, const Branch &b) {
            return a.lowerBound < b.lowerBound;
        });
        Decision best{1000000000, -1};
        for (const Branch &branch : branches) {
            if (branch.lowerBound >= best.cost) {
                break;
            }
            int sum = branch.soFar + exact(branch.miss).cost + exact(branch.hit).cost;
            if (sum < best.cost) {
                best = {sum, branch.firstDial};
            }
        }
        exactMemo.emplace(belief, best);
        return best;
    }

    double lookAhead(const vector<int> &belief, int depth, int &chosen) const {
        int size = belief.size();
        if (!size) {
            return 0;
        }
        if (size == 1) {
            return cost[belief.front()];
        }
        if (!depth) {
            int sum = 0;
            for (int state : belief) {
                sum += cost[state];
            }
            return sum + uncertainty[size];
        }

        double best = 1e20;
        array<vector<int>, 2> branch;
        for (auto &child : branch) {
            child.reserve(size);
        }
        for (int dial = 0; dial < n; ++dial) {
            for (auto &child : branch) {
                child.clear();
            }
            for (int state : belief) {
                int transition = next[state][dial];
                branch[transition % 2].push_back(transition / 2);
            }
            int unused;
            double sum = size + lookAhead(branch[0], depth - 1, unused)
                              + lookAhead(branch[1], depth - 1, unused);
            if (sum < best - 1e-8) {
                best = sum;
                chosen = dial;
            }
        }
        return best;
    }

    int distinguish(int first, int second) const {
        struct Pair {
            int first;
            int second;
            int dial;
        };
        vector<Pair> queue{{first, second, -1}};
        vector<bool> seen(next.size());
        seen[first] = true;
        for (size_t i = 0; i < queue.size(); ++i) {
            Pair current = queue[i];
            for (int dial = 0; dial < n; ++dial) {
                int a = next[current.first][dial];
                int b = next[current.second][dial];
                int firstDial = current.dial < 0 ? dial : current.dial;
                if ((a ^ b) & 1) {
                    return firstDial;
                }
                if (!seen[a / 2]) {
                    seen[a / 2] = true;
                    queue.push_back({a / 2, b / 2, firstDial});
                }
            }
        }
        abort();
    }

public:
    SmallSolver(int dialCount, Query &ask) : n(dialCount), query(ask) {
        array<int, 7> place{};
        int worldCount = 1;
        for (int i = 1; i < n; ++i) {
            place[i] = worldCount;
            worldCount *= n;
        }
        next.resize(worldCount);
        cost.resize(worldCount);
        destination.resize(worldCount);
        for (int code = 0; code < worldCount; ++code) {
            array<int, 7> values{}, count{};
            ++count[0];
            int sum = 0;
            int rotated = 0;
            for (int state = code, i = 1; i < n; ++i, state /= n) {
                int at = state % n;
                values[i] = at;
                ++count[at];
                sum += at ? n - at : 0;
                rotated += (at ? at - 1 : n - 1) * place[i];
            }
            cost[code] = sum;
            for (int t = 1; t < n; ++t) {
                sum += n * (1 - count[t]);
                if (sum < cost[code]) {
                    cost[code] = sum;
                    destination[code] = t;
                }
            }
            next[code][0] = 2 * rotated + (count[1] > 0);
            for (int dial = 1; dial < n; ++dial) {
                int at = values[dial];
                int to = at + 1 == n ? 0 : at + 1;
                int moved = code + (to - at) * place[dial];
                next[code][dial] = 2 * moved + (count[to] > 0);
            }
        }
        uncertainty.resize(worldCount + 1);
        for (int size = 2; size <= worldCount; ++size) {
            uncertainty[size] = informationCost * size * log2(size);
        }
    }

    void run() {
        if (n <= 4) {
            uint64_t belief = next.size() == 64 ? ~uint64_t(1)
                                              : (uint64_t(1) << next.size()) - 2;
            while (belief & (belief - 1)) {
                int dial = exact(belief).dial;
                belief = split(belief, dial)[query(dial)];
            }
            finish(__builtin_ctzll(belief));
            return;
        }

        vector<int> belief(next.size() - 1);
        iota(belief.begin(), belief.end(), 1);
        int withoutInformation = 0;
        while (belief.size() > 1) {
            int dial = 0;
            if (withoutInformation >= n) {
                dial = distinguish(belief[0], belief[1]);
            } else if (belief.size() != next.size() - 1) {
                int depth = belief.size() > 15000 ? 2 : belief.size() > 2000 ? 3 : 4;
                lookAhead(belief, depth, dial);
            }
            int reply = query(dial);
            size_t survivors = 0;
            for (int state : belief) {
                int transition = next[state][dial];
                if (transition % 2 == reply) {
                    belief[survivors++] = transition / 2;
                }
            }
            withoutInformation = survivors < belief.size() ? 0 : withoutInformation + 1;
            belief.resize(survivors);
        }
        finish(belief.front());
    }
};

#ifndef SIMULATE
int main() {
    int n;
    if (scanf("%d", &n) != 1) {
        return 0;
    }
    auto query = [](int dial) {
        printf("? %d\n", dial + 1);
        fflush(stdout);
        int reply;
        if (scanf("%d", &reply) != 1 || (reply != 0 && reply != 1)) {
            exit(0);
        }
        return reply;
    };
    if (n <= 7) {
        SmallSolver(n, query).run();
    } else {
        Solver(n, query).run();
    }
    puts("!");
    fflush(stdout);
}
#endif

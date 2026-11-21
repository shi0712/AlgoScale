#include <bits/stdc++.h>
using namespace std;

using Mask = uint64_t;
constexpr int maxN = 10;
constexpr int maxIntervals = 55;
constexpr int models = 11;
using Density = array<double, models>;

struct Interval {
    int l, r, cells;
};

struct Domain {
    int cell;
    vector<int> bits;
    array<int, maxIntervals> toggle{};
    vector<unsigned char> excluded;
    vector<double> weight;
    vector<int> ones;
    int initialParity = 0;
    int remaining = 0;

    int pack(Mask mask) const {
        int result = 0;
        for (int i = 0; i < (int)bits.size(); ++i)
            result |= int((mask >> bits[i]) & 1) << i;
        return result;
    }

    bool exclude(int state) {
        if (excluded[state]) return false;
        excluded[state] = true;
        --remaining;
        bool forced = false;
        for (int i = 0; i < (int)bits.size(); ++i) {
            ones[i] -= (state >> i) & 1;
            forced |= ones[i] == 0 || ones[i] == remaining;
        }
        return forced;
    }
};

// Search the nearest informative configurations of a component of length at most three.
class SmallComponent {
    static constexpr Mask alternating[] = {
        0x5555555555555555ULL, 0x3333333333333333ULL,
        0x0f0f0f0f0f0f0f0fULL, 0x00ff00ff00ff00ffULL,
        0x0000ffff0000ffffULL, 0x00000000ffffffffULL
    };
    int length, states;
    array<array<Mask, 4>, 64> response{};
    array<vector<int>, 7> atDistance;
    unordered_map<Mask, pair<int, int>> memo;

public:
    SmallComponent(int length) : length(length), states(1 << (length * (length + 1) / 2)) {
        array<int, 3> covering{};
        int bit = 0;
        for (int l = 0; l < length; ++l)
            for (int r = l; r < length; ++r, ++bit)
                for (int i = l; i <= r; ++i)
                    covering[i] |= 1 << bit;
        for (int q = 0; q < states; ++q) {
            atDistance[popcount(unsigned(q))].push_back(q);
            for (int hidden = 0; hidden < states; ++hidden) {
                int count = 0;
                for (int i = 0; i < length; ++i)
                    count += (covering[i] & (q ^ hidden)) == 0;
                response[q][count] |= Mask{1} << hidden;
            }
        }
    }

    static Mask translate(Mask belief, int flip) {
        for (; flip; flip &= flip - 1) {
            int i = countr_zero(unsigned(flip));
            int shift = 1 << i;
            Mask low = alternating[i];
            belief = ((belief & low) << shift) | ((belief >> shift) & low);
        }
        return belief;
    }

    Mask observe(Mask belief, int count) const {
        return belief & response[0][count];
    }

    pair<int, int> choose(Mask belief) {
        int size = popcount(belief);
        if (size <= 1) {
            int target = belief ? countr_zero(belief) : 0;
            return {popcount(unsigned(target)), target};
        }
        if (auto it = memo.find(belief); it != memo.end()) return it->second;

        pair<int, int> best{INT_MAX, 0};
        for (int d = 1; d < (int)atDistance.size(); ++d) {
            if (size * d >= best.first) break;
            for (int q : atDistance[d]) {
                bool informative = true;
                for (int count = 0; count < length; ++count)
                    if ((belief & response[q][count]) == belief)
                        informative = false;
                if (!informative) continue;

                int cost = size * d;
                for (int count = 0; count < length && cost < best.first; ++count)
                    cost += choose(translate(belief & response[q][count], q)).first;
                if (cost < best.first) best = {cost, q};
            }
            if (length == 3 && best.second) break;
        }
        memo.emplace(belief, best);
        return best;
    }
};

// The coordinates of a walk are the bits flipped relative to its starting point.
vector<int> grayWalk(const vector<double>& weight, int anchor = 0) {
    int size = weight.size();
    int bits = countr_zero(unsigned(size));
    vector<pair<double, int>> priority;
    for (int i = 0; i < bits; ++i) {
        double probability = 0;
        for (int state = 0; state < size; ++state)
            if (((state ^ anchor) >> i) & 1) probability += weight[state];
        priority.emplace_back(-probability, i);
    }
    sort(priority.begin(), priority.end());

    vector<int> walk;
    for (int i = 1; i < size; ++i)
        walk.push_back(priority[countr_zero(unsigned(i))].second);
    return walk;
}

// Prefer likely configurations, using Gray order as a separate zero-revisit option.
vector<int> weightedWalk(const vector<double>& weight, const vector<unsigned char>& excluded) {
    int size = weight.size();
    int bits = countr_zero(unsigned(size));
    vector<unsigned char> open(size);
    vector<int> degree(size, 0), distance(size);
    vector<double> nearby(size, 0);
    int remaining = 0;
    for (int state = 1; state < size; ++state) {
        if (excluded[state]) continue;
        open[state] = true;
        ++remaining;
        for (int i = 0; i < bits; ++i) {
            int neighbor = state ^ (1 << i);
            ++degree[neighbor];
            nearby[neighbor] += weight[state];
        }
    }
    for (int state = 1; state < size; ++state)
        distance[state] = distance[state >> 1] + (state & 1);
    array<double, maxN + 1> discount{};
    for (int d = 1; d <= bits; ++d) discount[d] = 1.0 / (d * sqrt(d));

    vector<int> walk;
    int current = 0;
    while (remaining) {
        int destination = -1;
        double best = -1;
        for (int state = 1; state < size; ++state) {
            if (!open[state]) continue;
            double value = (weight[state] + 0.18 * nearby[state] / max(1, degree[state]))
                         * discount[distance[state ^ current]];
            if (destination == -1 || value > best + 1e-15 ||
                (abs(value - best) <= 1e-15 && degree[state] < degree[destination])) {
                best = value;
                destination = state;
            }
        }
        while (current != destination) {
            int chosen = -1;
            double bestStep = -1;
            for (int diff = current ^ destination; diff; diff &= diff - 1) {
                int i = countr_zero(unsigned(diff));
                int next = current ^ (1 << i);
                double gain = (open[next] ? weight[next] : 0) + nearby[next] * 0.01;
                if (gain > bestStep) {
                    bestStep = gain;
                    chosen = i;
                }
            }
            walk.push_back(chosen);
            current ^= 1 << chosen;
            if (!open[current]) continue;
            open[current] = false;
            --remaining;
            for (int i = 0; i < bits; ++i) {
                int neighbor = current ^ (1 << i);
                --degree[neighbor];
                nearby[neighbor] -= weight[current];
            }
        }
    }
    return walk;
}

class Solver {
    int n, allCells, visible;
    int cleared = 0;
    // Outside temporary probes, every known interval is inactive.
    Mask parity = 0, inactive = 0;
    vector<Interval> intervals;
    int id[maxN][maxN]{};
    array<Mask, maxN> incident{};
    array<unordered_set<Mask>, maxN> forbidden;
    function<int(int, int)> ask;
    array<array<Density, maxN + 2>, maxN + 2> likelihood{};
    array<array<double, maxIntervals + 1>, models> onPower{}, offPower{};
    SmallComponent one{1}, two{2}, three{3};

    int flip(int index) {
        const auto& segment = intervals[index];
        parity ^= Mask{1} << index;
        visible = ask(segment.l + 1, segment.r + 1);
        return visible;
    }

    void remember(int cells) {
        // Each invisible cell excludes the current flip pattern on its incident intervals.
        for (; cells; cells &= cells - 1) {
            int c = countr_zero(unsigned(cells));
            forbidden[c].insert(parity & incident[c]);
        }
    }

    void confirm(int cells) {
        cleared |= cells;
        for (; cells; cells &= cells - 1)
            inactive |= incident[countr_zero(unsigned(cells))];
    }

    Density prior() const {
        // This model ranks walks. Only proved exclusions remove configurations.
        int activeBefore = popcount(parity & inactive);
        int inactiveBefore = popcount(inactive) - activeBefore;
        Density result;
        double total = 0;
        for (int m = 0; m < models; ++m) {
            double mass = m == (models - 1) / 2 ? 0.57 : 0.043;
            result[m] = mass * onPower[m][activeBefore] * offPower[m][inactiveBefore];
            total += result[m];
        }
        for (double& x : result) x *= 0.88 / total;
        result[(models - 1) / 2] += 0.12;
        return result;
    }

    static double dot(const Density& a, const Density& b) {
        double result = 0;
        for (int m = 0; m < models; ++m) result += a[m] * b[m];
        return result;
    }

    void locate(int scope) {
        vector<pair<int, int>> groups;
        groups.emplace_back(scope & ~cleared, visible - popcount(unsigned(cleared)));
        while (!groups.empty()) {
            auto [candidates, count] = groups.back();
            groups.pop_back();
            for (int cells = candidates; cells; cells &= cells - 1) {
                int c = countr_zero(unsigned(cells));
                if (forbidden[c].contains(parity & incident[c])) candidates &= ~(1 << c);
            }
            if (count == 0) {
                remember(candidates);
                continue;
            }
            if (count == popcount(unsigned(candidates))) {
                confirm(candidates);
                continue;
            }

            // Rank splits by the possible sets of visible cells and the cost of restoring the probe.
            vector<pair<int, double>> sets;
            Density density = prior();
            double total = 0;
            for (int subset = candidates; subset; subset = (subset - 1) & candidates) {
                if (popcount(unsigned(subset)) != count) continue;
                Mask covering = 0;
                for (int cells = subset; cells; cells &= cells - 1)
                    covering |= incident[countr_zero(unsigned(cells))];
                covering &= ~inactive;
                int a = popcount(covering & parity);
                int b = popcount(covering) - a;
                double weight = 0;
                for (int m = 0; m < models; ++m)
                    weight += density[m] * onPower[m][a] * offPower[m][b];
                sets.emplace_back(subset, weight);
                total += weight;
            }

            int query = -1, offset = 0;
            double best = -1;
            for (int index = 0; index < (int)intervals.size(); ++index) {
                int area = intervals[index].cells;
                int inside = area & candidates;
                if (!inside || inside == candidates) continue;
                bool known = (inactive >> index) & 1;
                int extra = popcount(unsigned(cleared & area));
                bool usable = true;
                for (auto [other, knownCount] : groups) {
                    int overlap = other & area;
                    if (overlap == other) extra += knownCount;
                    else if (overlap && knownCount) usable = false;
                }
                if (!usable) continue;

                array<double, maxN + 1> outcomes{};
                for (auto [subset, weight] : sets)
                    outcomes[popcount(unsigned(subset & inside))] +=
                        0.94 * weight / total + 0.06 / sets.size();
                double information = 0;
                for (double p : outcomes)
                    if (p > 0) information -= p * log2(p);
                double cost = known ? 2.0 : 2.0 - outcomes[0];
                double score = information / cost;
                if (score > best + 1e-12 ||
                    (abs(score - best) <= 1e-12 && popcount(unsigned(area)) >
                     popcount(unsigned(intervals[query].cells)))) {
                    best = score;
                    query = index;
                    offset = extra;
                }
            }

            int area = intervals[query].cells;
            int inside = candidates & area;
            int outside = candidates & ~area;
            bool known = (inactive >> query) & 1;
            int previous = visible;
            Mask before = parity;
            int change = flip(query) - previous;
            if (change < 0) {
                flip(query);
                inactive |= Mask{1} << query;
                int under = -change - offset;
                groups.emplace_back(outside, count - under);
                groups.emplace_back(inside, under);
            } else {
                // With a nonnegative change there were no visible cells under this interval.
                for (int cells = area; cells; cells &= cells - 1) {
                    int c = countr_zero(unsigned(cells));
                    forbidden[c].insert(before & incident[c]);
                }
                for (auto& group : groups) group.first &= ~area;
                groups.emplace_back(outside, count);
                if (change > 0) {
                    inactive |= Mask{1} << query;
                    groups.emplace_back(area, change);
                } else {
                    remember(area);
                    if (known) flip(query);
                }
            }
        }
        remember(allCells & ~cleared);
    }

    Domain makeDomain(int c, const Density& density) const {
        Domain domain;
        domain.cell = c;
        Mask unknown = incident[c] & ~inactive;
        for (Mask mask = unknown; mask; mask &= mask - 1) {
            int index = countr_zero(mask);
            domain.toggle[index] = 1 << domain.bits.size();
            domain.bits.push_back(index);
        }
        domain.initialParity = domain.pack(parity);
        int k = domain.bits.size();
        int size = 1 << k;
        domain.excluded.resize(size);
        domain.weight.resize(size);
        domain.ones.resize(k);
        Mask fixed = incident[c] & inactive;
        for (Mask old : forbidden[c])
            if (((old ^ parity) & fixed) == 0)
                domain.excluded[domain.pack(old ^ parity)] = true;

        for (int state = 0; state < size; ++state) {
            if (domain.excluded[state]) continue;
            ++domain.remaining;
            for (int i = 0; i < k; ++i) domain.ones[i] += (state >> i) & 1;
            domain.weight[state] = dot(density, likelihood[k][popcount(unsigned(state ^ domain.initialParity))]);
        }
        return domain;
    }

    void searchSmall(int l, int r, int base) {
        int length = r - l + 1;
        SmallComponent& planner = length == 1 ? one : length == 2 ? two : three;
        vector<int> bits;
        for (int i = l; i <= r; ++i)
            for (int j = i; j <= r; ++j) bits.push_back(id[i][j]);
        Mask belief = 0;
        array<Mask, 64> active{};
        for (int state = 0; state < (1 << bits.size()); ++state) {
            if (state) {
                int i = countr_zero(unsigned(state));
                active[state] = active[state & (state - 1)] | (Mask{1} << bits[i]);
            }
            if (active[state] & inactive) continue;
            bool possible = true;
            for (int i = l; i <= r; ++i)
                if (forbidden[i].contains((parity ^ active[state]) & incident[i])) possible = false;
            if (possible) belief |= Mask{1} << state;
        }
        belief = planner.observe(belief, visible - base);
        int target = 0;
        while (visible < base + length) {
            if (!target) target = planner.choose(belief).second;
            int bit = countr_zero(unsigned(target));
            target ^= 1 << bit;
            int oldSize = popcount(belief);
            int count = flip(bits[bit]) - base;
            belief = planner.observe(SmallComponent::translate(belief, 1 << bit), count);
            if (popcount(belief) != oldSize) target = 0;
        }
        confirm(((1 << length) - 1) << l);
    }

    static void append(vector<int>& plan, const Domain& target, const vector<int>& walk) {
        for (int bit : walk) plan.push_back(target.bits[bit]);
    }

    struct PlanScore {
        double cost;
        int length;
    };

    PlanScore evaluate(const vector<int>& plan, const Domain& left, const Domain* right,
                       int shared, const Density& density) const {
        if (!right || !left.toggle[shared] || !right->toggle[shared]) {
            auto excluded = left.excluded;
            double mass = accumulate(left.weight.begin(), left.weight.end(), 0.0);
            double total = mass, cost = 0;
            int current = 0, remaining = left.remaining, moves = 0;
            for (int query : plan) {
                cost += max(0.0, mass);
                ++moves;
                current ^= left.toggle[query];
                if (!excluded[current]) {
                    excluded[current] = true;
                    mass -= left.weight[current];
                    if (--remaining == 0) break;
                }
            }
            return {remaining ? numeric_limits<double>::infinity() : cost / total, moves};
        }

        const Domain* domains[] = {&left, right};
        array<vector<unsigned char>, 2> excluded{left.excluded, right->excluded};
        int state[2]{};
        int remaining[2][2]{};
        int visited[2][2]{};
        Density common[2]{};
        Density mass[2][2]{};
        int sharedBit[2] = {left.toggle[shared], right->toggle[shared]};
        for (int s = 0; s < 2; ++s) {
            int k = domains[s]->bits.size() - 1;
            for (int x = 0; x < (int)excluded[s].size(); ++x) {
                if (excluded[s][x]) continue;
                int z = (x & sharedBit[s]) != 0;
                int a = popcount(unsigned((x ^ domains[s]->initialParity) & ~sharedBit[s]));
                for (int m = 0; m < models; ++m) mass[s][z][m] += likelihood[k][a][m];
                ++remaining[s][z];
            }
        }
        for (int z = 0; z < 2; ++z) {
            int original = z ^ int((parity >> shared) & 1);
            for (int m = 0; m < models; ++m) common[z][m] = density[m] * likelihood[1][original][m];
        }
        auto survival = [&]() {
            double sum = 0;
            for (int z = 0; z < 2; ++z)
                for (int m = 0; m < models; ++m)
                    sum += common[z][m] * max(0.0, mass[0][z][m]) * max(0.0, mass[1][z][m]);
            return sum;
        };

        double total = survival(), probability = total;
        double cost = 0, saved = 0;
        int moves = 0;
        for (int query : plan) {
            cost += probability;
            ++moves;
            for (int s = 0; s < 2; ++s) {
                const Domain& domain = *domains[s];
                state[s] ^= domain.toggle[query];
                int x = state[s];
                if (excluded[s][x]) continue;
                excluded[s][x] = true;
                int z = (x & sharedBit[s]) != 0;
                int a = popcount(unsigned((x ^ domain.initialParity) & ~sharedBit[s]));
                const Density& removed = likelihood[domain.bits.size() - 1][a];
                double success = 0;
                for (int m = 0; m < models; ++m) {
                    success += common[z][m] * removed[m] * max(0.0, mass[1 - s][z][m]);
                    mass[s][z][m] -= removed[m];
                }
                probability = max(0.0, probability - success);
                saved += success * visited[1 - s][z];
                --remaining[s][z];
                ++visited[s][z];
            }
            if (remaining[0][0] * remaining[1][0] == 0 &&
                remaining[0][1] * remaining[1][1] == 0)
                return {(cost - 0.26 * saved) / total, moves};
        }
        return {numeric_limits<double>::infinity(), moves};
    }

    vector<int> planSearch(const Domain& left, const Domain& right, int shared, const Density& density) const {
        const Domain* endpoints[] = {&left, &right};
        const bool paired = left.toggle[shared] && right.toggle[shared];
        vector<int> best;
        double bestCost = numeric_limits<double>::infinity();
        auto offer = [&](vector<int> plan, int focus) {
            PlanScore score = paired ? evaluate(plan, left, &right, shared, density)
                                     : evaluate(plan, *endpoints[focus], nullptr, shared, density);
            double cost = score.cost + 0.007 * score.length;
            if (cost < bestCost) {
                bestCost = cost;
                plan.resize(score.length);
                best = move(plan);
            }
        };

        for (int s = 0; s < 2; ++s) {
            const Domain& domain = *endpoints[s];
            vector<int> plan;
            append(plan, domain, grayWalk(domain.weight));
            offer(move(plan), s);
            plan.clear();
            append(plan, domain, weightedWalk(domain.weight, domain.excluded));
            offer(move(plan), s);

            int dense = ((1 << domain.bits.size()) - 1) ^ domain.initialParity;
            if (!domain.excluded[dense]) {
                plan.clear();
                for (int bit = dense; bit; bit &= bit - 1)
                    plan.push_back(domain.bits[countr_zero(unsigned(bit))]);
                stable_sort(plan.begin(), plan.end(), [&](int a, int b) {
                    return intervals[a].r - intervals[a].l > intervals[b].r - intervals[b].l;
                });
                append(plan, domain, grayWalk(domain.weight, dense));
                offer(move(plan), s);
            }
        }

        if (!paired) return best;
        vector<int> projection[2], privateBits[2];
        vector<double> weight[2];
        vector<unsigned char> excluded[2];
        for (int s = 0; s < 2; ++s) {
            const Domain& domain = *endpoints[s];
            int z = countr_zero(unsigned(domain.toggle[shared]));
            int size = domain.weight.size() / 2;
            weight[s].resize(size);
            excluded[s].resize(size);
            for (int x = 0; x < size; ++x) {
                int low = x & ((1 << z) - 1);
                int full = low | ((x ^ low) << 1);
                weight[s][x] = domain.weight[full] + domain.weight[full | (1 << z)];
                excluded[s][x] = domain.excluded[full] && domain.excluded[full | (1 << z)];
            }
            for (int index : domain.bits)
                if (index != shared) privateBits[s].push_back(index);
        }

        for (bool weighted : {false, true}) {
            for (int s = 0; s < 2; ++s) {
                projection[s] = weighted ? weightedWalk(weight[s], excluded[s]) : grayWalk(weight[s]);
            }
            vector<int> prefix{shared};
            int state[2] = {left.toggle[shared], right.toggle[shared]};
            int rounds = min(projection[0].size(), projection[1].size());
            for (int round = 0; round <= rounds; ++round) {
                // Try continuing the paired search and switching to either endpoint.
                if (round == rounds || round == 0 || round == rounds / 5 ||
                    round == 2 * rounds / 5 || round == 3 * rounds / 5) {
                    for (int s = 0; s < 2; ++s) {
                        vector<int> plan = prefix;
                        append(plan, *endpoints[s], grayWalk(endpoints[s]->weight, state[s]));
                        offer(move(plan), s);
                    }
                }
                if (round == rounds) break;
                int a = privateBits[0][projection[0][round]];
                int b = privateBits[1][projection[1][round]];
                state[0] ^= left.toggle[a];
                state[1] ^= right.toggle[b];
                if (left.weight[state[0]] >= right.weight[state[1]]) {
                    prefix.push_back(a);
                    prefix.push_back(b);
                } else {
                    prefix.push_back(b);
                    prefix.push_back(a);
                }
                prefix.push_back(shared);
                state[0] ^= left.toggle[shared];
                state[1] ^= right.toggle[shared];
            }
        }
        return best;
    }

    void advance(int l, int r) {
        if (r - l + 1 <= 3) {
            searchSmall(l, r, visible);
            return;
        }
        Density density = prior();
        vector<Domain> domains;
        for (int c = l; c <= r; ++c)
            if (popcount(incident[c] & ~inactive) <= maxN)
                domains.push_back(makeDomain(c, density));

        Mask forcedInactive = 0, forcedActive = 0;
        for (const auto& domain : domains)
            for (int i = 0; i < (int)domain.bits.size(); ++i) {
                Mask bit = Mask{1} << domain.bits[i];
                if (domain.ones[i] == 0) forcedInactive |= bit;
                if (domain.ones[i] == domain.remaining) forcedActive |= bit;
            }
        inactive |= forcedInactive;
        if (forcedActive || forcedInactive) {
            while (forcedActive) {
                int index = countr_zero(forcedActive);
                forcedActive &= forcedActive - 1;
                int before = visible;
                int after = flip(index);
                inactive |= Mask{1} << index;
                if (after > before) {
                    int scope = intervals[index].cells;
                    remember(allCells & ~cleared & ~scope);
                    locate(scope);
                    return;
                }
                remember(allCells & ~cleared);
            }
            return;
        }

        auto plan = planSearch(domains.front(), domains.back(), id[l][r], density);
        vector<int> state(domains.size());
        for (int index : plan) {
            int before = visible;
            int after = flip(index);
            if (after > before) {
                inactive |= Mask{1} << index;
                int scope = intervals[index].cells;
                remember(allCells & ~cleared & ~scope);
                locate(scope);
                return;
            }
            remember(allCells & ~cleared);
            bool forced = false;
            for (int s = 0; s < (int)domains.size(); ++s) {
                int bit = domains[s].toggle[index];
                if (!bit) continue;
                state[s] ^= bit;
                forced |= domains[s].exclude(state[s]);
            }
            if (forced) return;
        }
    }

public:
    Solver(int n, int count, function<int(int, int)> ask)
        : n(n), allCells((1 << n) - 1), visible(count), ask(move(ask)) {
        for (int l = 0; l < n; ++l)
            for (int r = l; r < n; ++r) {
                int index = intervals.size();
                id[l][r] = index;
                intervals.push_back({l, r, ((1 << (r - l + 1)) - 1) << l});
                for (int i = l; i <= r; ++i) incident[i] |= Mask{1} << index;
            }
        for (int m = 0; m < models; ++m) {
            double p = m / double(models - 1);
            onPower[m][0] = offPower[m][0] = 1;
            for (int k = 1; k <= maxIntervals; ++k) {
                onPower[m][k] = onPower[m][k - 1] * p;
                offPower[m][k] = offPower[m][k - 1] * (1 - p);
            }
            for (int k = 0; k <= maxN; ++k)
                for (int a = 0; a <= k; ++a)
                    likelihood[k][a][m] = onPower[m][a] * offPower[m][k - a];
        }
    }

    void run() {
        if (n <= 3) {
            searchSmall(0, n - 1, 0);
            return;
        }
        locate(allCells);
        while (cleared != allCells) {
            int bestL = 0, bestR = n;
            for (int l = 0; l < n;) {
                if ((cleared >> l) & 1) {
                    ++l;
                    continue;
                }
                int r = l;
                while (r + 1 < n && !((cleared >> (r + 1)) & 1)) ++r;
                if (r - l < bestR - bestL) bestL = l, bestR = r;
                l = r + 1;
            }
            advance(bestL, bestR);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, visible;
    if (!(cin >> n >> visible) || n < 1 || n > maxN || visible < 0 || visible >= n) return 0;
    Solver solver(n, visible, [n](int l, int r) {
        cout << l << ' ' << r << '\n' << flush;
        int count;
        if (!(cin >> count) || count < 0 || count >= n) exit(0);
        return count;
    });
    solver.run();
    return 0;
}

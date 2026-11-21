#include <bits/stdc++.h>
using namespace std;

using Mask = uint64_t;

static Mask bit(int x) { return Mask{1} << x; }
static int size_of(Mask mask) { return popcount(mask); }
static int first_bit(Mask mask) { return countr_zero(mask); }

static double assignments[51][51][51];

// Small instances can keep every configuration relative to the first wheel.
class SmallMachine {
    struct Outcome {
        unsigned state : 24;
        unsigned distinct : 4;
        unsigned misaligned : 4;
    };

    struct Action {
        int wheel, step;
        vector<Outcome> transition;
    };

    using Cost = pair<int, int>;  // Total over the belief, then worst case.

    int n, states = 1;
    vector<Action> actions;
    vector<int> replies, distance;
    vector<double> entropy_cost;
    unordered_map<Mask, pair<Cost, int>> exact_policy;

    array<int, 7> decode(int state) const {
        array<int, 7> positions{};
        for (int i = 1; i < n; ++i) {
            positions[i] = state % n;
            state /= n;
        }
        return positions;
    }

    template<class Apply>
    void align(int state, Apply apply) const {
        auto p = decode(state);
        array<int, 7> counts{};
        for (int i = 0; i < n; ++i) ++counts[p[i]];
        int target = max_element(counts.begin(), counts.end()) - counts.begin();
        for (int i = 0; i < n; ++i) {
            int step = (target - p[i] + n) % n;
            if (step > 0) apply(i * (n - 1) + step - 1);
        }
    }

    array<Mask, 5> exact_partition(Mask belief, const Action &action) const {
        array<Mask, 5> groups{};
        for (; belief; belief &= belief - 1) {
            auto next = action.transition[first_bit(belief)];
            groups[next.distinct] |= bit(next.state);
        }
        return groups;
    }

    Cost evaluate(Mask belief) {
        if (has_single_bit(belief)) {
            int d = distance[first_bit(belief)];
            return {d, d};
        }
        if (auto it = exact_policy.find(belief); it != exact_policy.end()) return it->second.first;

        // An informative move sends each branch to a smaller belief.
        // Non-informative moves form a graph within the current belief size.
        int count = size_of(belief);
        static constexpr int infinity = 1000000;
        struct Node {
            Mask belief;
            Cost cost{infinity, infinity};
            int action = -1;
            vector<pair<int, int>> predecessors{};
        };
        vector<Node> component{{belief}};
        for (int i = 0; i < int(component.size()); ++i) {
            for (int a = 0; a < int(actions.size()); ++a) {
                auto branches = exact_partition(component[i].belief, actions[a]);
                auto unchanged = find_if(branches.begin() + 1, branches.end(),
                    [count](Mask branch) { return size_of(branch) == count; });
                if (unchanged != branches.end()) {
                    auto destination = find_if(component.begin(), component.end(),
                        [&](const Node &node) { return node.belief == *unchanged; });
                    int j = destination - component.begin();
                    if (j == int(component.size())) component.push_back({*unchanged});
                    component[j].predecessors.emplace_back(i, a);
                    continue;
                }
                Cost cost{count, 1};
                for (Mask branch : branches) {
                    if (!branch) continue;
                    auto [sum, worst] = evaluate(branch);
                    cost.first += sum;
                    cost.second = max(cost.second, 1 + worst);
                }
                if (cost < component[i].cost) {
                    component[i].cost = cost;
                    component[i].action = a;
                }
            }
        }
        priority_queue<pair<Cost, int>, vector<pair<Cost, int>>, greater<>> pending;
        for (int i = 0; i < int(component.size()); ++i) pending.emplace(component[i].cost, i);
        while (!pending.empty()) {
            auto [cost, i] = pending.top();
            pending.pop();
            if (cost != component[i].cost) continue;
            Cost previous{cost.first + count, cost.second + 1};
            for (auto [j, a] : component[i].predecessors) {
                if (previous >= component[j].cost) continue;
                component[j].cost = previous;
                component[j].action = a;
                pending.emplace(previous, j);
            }
        }
        for (const auto &node : component) exact_policy[node.belief] = {node.cost, node.action};
        return component[0].cost;
    }

    double estimate(const vector<unsigned> &belief, const Action &action) const {
        int count[8]{}, sum = 0;
        for (unsigned state : belief) {
            const auto next = action.transition[state];
            ++count[next.distinct];
            sum += next.misaligned;
        }
        double cost = sum - 0.5 * count[1];
        for (int k = 2; k <= n; ++k) cost += entropy_cost[count[k]];
        return cost;
    }

    int choose_action(const vector<unsigned> &belief, unsigned signature,
                      const vector<unsigned> &visited) const {
        double best = numeric_limits<double>::infinity();
        int choice = -1;
        for (int a = 0; a < int(actions.size()); ++a) {
            const auto &action = actions[a];
            unsigned next_signature = action.transition[signature].state;
            if (find(visited.begin(), visited.end(), next_signature) != visited.end()) continue;
            double cost = estimate(belief, action);
            if (belief.size() <= 48) {
                array<vector<unsigned>, 8> groups;
                for (unsigned state : belief) {
                    auto next = action.transition[state];
                    groups[next.distinct].push_back(next.state);
                }
                double lookahead = 0;
                for (int k = 2; k <= n; ++k) {
                    const auto &branch = groups[k];
                    if (branch.empty()) continue;
                    if (branch.size() == 1) {
                        lookahead += distance[branch[0]];
                        continue;
                    }
                    double next_cost = numeric_limits<double>::infinity();
                    for (const auto &follow_up : actions) {
                        next_cost = min(next_cost, estimate(branch, follow_up));
                    }
                    lookahead += branch.size() + next_cost;
                }
                cost = 0.12 * cost + 0.88 * lookahead;
            }
            if (cost < best - 1e-9) best = cost, choice = a;
        }
        return choice;
    }

public:
    explicit SmallMachine(int n) : n(n) {
        array<int, 7> power{};
        for (int i = 1; i < n; ++i) {
            power[i] = states;
            states *= n;
        }
        for (int i = 0; i < n; ++i) {
            for (int d = 1; d < n; ++d) actions.push_back({i, d, vector<Outcome>(states)});
        }
        replies.resize(states);
        distance.resize(states);
        entropy_cost.resize(states + 1);
        for (int state = 0; state < states; ++state) {
            array<int, 7> counts{};
            auto p = decode(state);
            for (int i = 0; i < n; ++i) ++counts[p[i]];
            replies[state] = n - std::count(counts.begin(), counts.begin() + n, 0);
            distance[state] = n - *max_element(counts.begin(), counts.end());
        }
        for (int i = 1; i <= states; ++i) entropy_cost[i] = i * 0.82 * log2(i);
        for (int a = 0; a < int(actions.size()); ++a) {
            auto &action = actions[a];
            int wheel = action.wheel, step = action.step;
            for (int state = 0; state < states; ++state) {
                int y = state;
                if (wheel == 0) {
                    for (int i = 1, x = state; i < n; ++i, x /= n) {
                        int digit = x % n;
                        y += (digit >= step ? -step : n - step) * power[i];
                    }
                } else {
                    int digit = (state / power[wheel]) % n;
                    y += (digit + step < n ? step : step - n) * power[wheel];
                }
                action.transition[state] = {unsigned(y), unsigned(replies[y]), unsigned(distance[y])};
            }
        }
    }

    template<class Rotate>
    bool solve(int k, Rotate rotate) {
        auto execute = [&](int a) {
            const auto &action = actions[a];
            return rotate(action.wheel, action.step);
        };
        if (n <= 4) {
            Mask belief = 0;
            for (int state = 0; state < states; ++state) {
                if (replies[state] == k) belief |= bit(state);
            }
            evaluate(belief);
            while (!has_single_bit(belief)) {
                int a = exact_policy.at(belief).second;
                k = execute(a);
                belief = exact_partition(belief, actions[a])[k];
            }
            align(first_bit(belief), execute);
            return false;
        }

        vector<unsigned> belief, visited{0};
        for (unsigned state = 0; state < unsigned(states); ++state) {
            if (replies[state] == k) belief.push_back(state);
        }
        unsigned signature = 0;
        auto apply = [&](int a) {
            k = execute(a);
            const auto &transition = actions[a].transition;
            signature = transition[signature].state;
            size_t kept = 0;
            for (unsigned state : belief) {
                auto next = transition[state];
                if (next.distinct == unsigned(k)) belief[kept++] = next.state;
            }
            if (kept < belief.size()) visited.clear();
            belief.resize(kept);
            visited.push_back(signature);
        };

        while (belief.size() > 1) {
            // Many half-turn configurations need a probe outside their two sites.
            if (k == 2 && n % 2 == 0 && belief.size() >= 15 &&
                belief.size() < (1U << (n - 1))) {
                bool opposite = all_of(belief.begin(), belief.end(), [&](unsigned state) {
                    auto p = decode(state);
                    return all_of(p.begin() + 1, p.begin() + n,
                        [&](int x) { return x == 0 || x == n / 2; });
                });
                if (opposite) return true;
            }
            int a = visited.size() >= 4 ? -1 : choose_action(belief, signature, visited);
            if (a >= 0) apply(a);
            else {
                // This either wins or rules out the proposed configuration.
                align(belief.front(), apply);
                visited.assign(1, signature);
            }
        }
        align(belief.front(), apply);
        return false;
    }
};

class Machine {
    struct SurveyState {
        int count;
        Mask occupied, possible;

        bool observe(int x, bool hit) {
            if (hit) occupied |= bit(x);
            else possible &= ~bit(x);
            int lo = size_of(occupied), hi = size_of(possible);
            if (lo > count || hi < count || (occupied & ~possible)) return false;
            if (lo == count) possible = occupied;
            if (hi == count) occupied = possible;
            return true;
        }
    };

    static constexpr int planning_limit = 13;

    struct Candidate {
        int origin, unique;
        double weight;
    };

    int n, distinct;
    Mask universe;
    mt19937 rng;
    vector<int> wheel_order, position, site_order;
    array<int, 50> known_count{};
    // Unknown wheels have not been moved since the survey. Their occupied
    // positions contain required and are contained in possible.
    Mask occupied = 0, known = 0, required = 0, possible;
    int unknown, scout;

    int add(int x, int r) const {
        int y = x + r;
        return y >= n ? y - n : y;
    }

    void rotate_wheel(int wheel, int step) {
        if (step > n / 2) step -= n;
        if (step < -n / 2) step += n;
        cout << wheel + 1 << ' ' << step << '\n' << flush;
        if (!(cin >> distinct) || distinct <= 1 || distinct > n) exit(0);
    }

    void remove_known(int x) {
        if (--known_count[x] == 0) known &= ~bit(x);
    }

    void add_known(int x) {
        ++known_count[x];
        known |= bit(x);
    }

    void tighten_support() {
        if (unknown == size_of(required)) possible = required;
    }

    void move_known(int wheel, int y) {
        int x = position[wheel];
        if (x == y) return;
        int without_loss = distinct + !(occupied & bit(y));
        rotate_wheel(wheel, y - x);
        remove_known(x);
        if (distinct < without_loss) {
            occupied &= ~bit(x);
            possible &= ~bit(x);
            required &= ~bit(x);
        } else if (!(known & bit(x))) {
            required |= bit(x);
        }
        occupied |= bit(y);
        position[wheel] = y;
        add_known(y);
        tighten_support();
    }

    // Moving the reference wheel reveals membership in the fixed wheels' set.
    Mask survey() {
        int x = position[scout];
        remove_known(x);
        array<SurveyState, 2> models{};
        int model_count = 0;
        for (int s : {distinct - 1, distinct}) {
            if (s < 1 || s >= n) continue;
            SurveyState model{s, known | required, known | possible};
            if (model.observe(x, distinct == s)) models[model_count++] = model;
        }
        while (model_count != 1 || models[0].occupied != models[0].possible) {
            int chosen = -1, best = 1;
            for (int y : site_order) {
                if (y == x) continue;
                unsigned responses = 0;
                for (int i = 0; i < model_count; ++i) {
                    const auto &model = models[i];
                    int relative = model.count - models[0].count;
                    if (model.possible & bit(y)) responses |= 1U << relative;
                    if (!(model.occupied & bit(y))) responses |= 2U << relative;
                }
                int value = popcount(responses);
                if (value > best) best = value, chosen = y;
                if (best == 3) break;
            }
            assert(chosen >= 0);
            rotate_wheel(scout, chosen - x);
            x = chosen;
            int valid = 0;
            for (int i = 0; i < model_count; ++i) {
                auto model = models[i];
                if ((distinct == model.count || distinct == model.count + 1) &&
                    model.observe(x, distinct == model.count)) models[valid++] = model;
            }
            model_count = valid;
        }
        Mask fixed = models[0].occupied;
        required |= fixed & ~known;
        possible &= fixed;
        occupied = fixed | bit(x);
        position[scout] = x;
        add_known(x);
        tighten_support();
        return fixed;
    }

    Mask shift(Mask sites, int r) const {
        return ((sites << r) & universe) | (sites >> (n - r));
    }

    bool aperiodic(Mask sites) const {
        for (int r = 1; r < n; ++r) {
            if (n % r == 0 && shift(sites, r) == sites) return false;
        }
        return true;
    }

    void break_symmetry() {
        if (aperiodic(occupied)) return;
        int x = position[scout];
        if (occupied == universe) {
            for (int y : site_order) {
                if (x != y) {
                    move_known(scout, y);
                    return;
                }
            }
        }
        for (int y : site_order) {
            if (occupied & bit(y)) continue;
            Mask filled = occupied | bit(y);
            if (aperiodic(filled) && aperiodic(filled ^ bit(x))) {
                move_known(scout, y);
                return;
            }
        }

        // A periodic fixed set becomes aperiodic when one empty site is filled.
        // Otherwise putting the reference on an occupied site keeps its pattern.
        Mask fixed = survey();
        if (!aperiodic(occupied)) {
            move_known(scout, first_bit(aperiodic(fixed) ? fixed : universe ^ fixed));
        }
    }

    vector<Candidate> hypotheses(array<double, 50> &alignment) const {
        vector<Candidate> candidates;
        int available = size_of(possible), needed = size_of(required);
        double sum = 0;
        for (Mask sites = possible; sites; sites &= sites - 1) {
            int x = first_bit(sites);
            int remaining = needed - bool(required & bit(x));
            double total = assignments[unknown - 1][available][remaining];
            double unique = 0;
            if (!(known & bit(x)) && (available > 1 || unknown == 1)) {
                unique = assignments[unknown - 1][available - 1][remaining];
                candidates.push_back({x, 1, unique});
            }
            if ((known & bit(x)) || unknown > needed) {
                candidates.push_back({x, 0, max(total - unique, total * 1e-12)});
            }
            alignment[x] = total;
            sum += total;
        }
        double biggest = 0;
        for (int x = 0; x < n; ++x) {
            alignment[x] = known_count[x] + unknown * alignment[x] / sum;
            biggest = max(biggest, alignment[x]);
        }
        for (int x = 0; x < n; ++x) {
            double fraction = alignment[x] / biggest;
            alignment[x] = fraction * fraction * fraction;
        }
        return candidates;
    }

    static double information(double p) { return p > 0 ? -p * log2(p) : 0; }

    // A nonzero rotation loses the origin only if it was unique, and
    // gains the destination only if the destination was empty.
    int response(const Candidate &candidate, int r) const {
        int y = add(candidate.origin, r);
        return 1 - candidate.unique + !(occupied & bit(y));
    }

    int choose_short_plan(const vector<Candidate> &candidates,
                          const array<double, 50> &alignment, int starting_count) const {
        int h = candidates.size();
        array<double, 1 << planning_limit> weights{}, best_cost;
        array<array<unsigned, 3>, 50> branches{};
        double sum = 0;
        for (const auto &candidate : candidates) sum += candidate.weight;
        for (unsigned mask = 1; mask < (1U << h); ++mask) {
            int i = countr_zero(mask);
            weights[mask] = weights[mask & (mask - 1)] + 0.96 * candidates[i].weight / sum + 0.04 / h;
        }
        for (int r : site_order) {
            if (r == 0) continue;
            for (int i = 0; i < h; ++i) branches[r][response(candidates[i], r)] |= 1U << i;
        }
        best_cost.fill(-1);
        auto search = [&](auto &&self, unsigned mask, int &choice) -> double {
            if (best_cost[mask] >= 0) return best_cost[mask];
            double best = numeric_limits<double>::infinity();
            for (int r : site_order) {
                if (r == 0) continue;
                if ((mask & branches[r][0]) == mask || (mask & branches[r][1]) == mask ||
                    (mask & branches[r][2]) == mask) continue;
                double cost = weights[mask];
                for (int reply = 0; reply < 3; ++reply) {
                    unsigned group = mask & branches[r][reply];
                    if (!group) continue;
                    int reported_count = starting_count + reply - 1;
                    if (reported_count == 1) {
                        cost -= 0.99 * weights[group];
                    } else if (has_single_bit(group)) {
                        int y = add(candidates[countr_zero(group)].origin, r);
                        cost -= 0.83 * weights[group] * alignment[y];
                    } else {
                        int next_choice;
                        cost += self(self, group, next_choice);
                    }
                }
                if (cost < best - 1e-12) best = cost, choice = r;
            }
            return best_cost[mask] = best;
        };
        int choice = -1;
        search(search, (1U << h) - 1, choice);
        return choice;
    }

    int choose_rotation(const vector<Candidate> &candidates,
                        const array<double, 50> &alignment, int starting_count) const {
        if (candidates.size() <= planning_limit) return choose_short_plan(candidates, alignment, starting_count);
        array<double, 100> weight{};
        double total = 0;
        for (const auto &h : candidates) total += h.weight;
        for (int i = 0; i < int(candidates.size()); ++i) {
            weight[i] = 0.96 * candidates[i].weight / total + 0.04 / candidates.size();
        }

        int best_rotation = -1;
        double best_value = -1;
        for (int r : site_order) {
            if (r == 0) continue;
            double p[3]{};
            Mask origins[3]{};
            for (int i = 0; i < int(candidates.size()); ++i) {
                int reply = response(candidates[i], r);
                p[reply] += weight[i];
                origins[reply] |= bit(candidates[i].origin);
            }
            double value = 0;
            for (int i = 0; i < 3; ++i) {
                value += information(p[i]);
                if (has_single_bit(origins[i])) {
                    value += 0.83 * p[i] * alignment[add(first_bit(origins[i]), r)];
                }
            }
            if (value > best_value + 1e-12) best_value = value, best_rotation = r;
        }
        return best_rotation;
    }

    void locate(int wheel) {
        break_symmetry();
        array<double, 50> alignment{};
        auto candidates = hypotheses(alignment);
        int start_count = distinct;
        int current = 0;
        while (candidates.size() > 1) {
            int next = choose_rotation(candidates, alignment, start_count);
            assert(next != current && next > 0);
            int rotation = next - current;
            rotate_wheel(wheel, rotation);
            current = next;
            int reply = distinct - start_count + 1;
            erase_if(candidates, [&](const Candidate &h) { return response(h, current) != reply; });
        }
        const auto &found = candidates.front();
        int x = found.origin, y = add(x, current);
        if (found.unique) {
            occupied &= ~bit(x);
            required &= ~bit(x);
            possible &= ~bit(x);
        } else if (known & bit(x)) {
            required &= ~bit(x);
        }
        occupied |= bit(y);
        position[wheel] = y;
        add_known(y);
        --unknown;
        tighten_support();
        if (distinct > n / 3 && known_count[y] == 1 && !(possible & bit(y))) {
            int target = position[scout];
            for (int x : site_order) {
                if (known_count[x] > known_count[target]) target = x;
            }
            move_known(wheel, target);
        }
    }

    void finish() {
        if (unknown > 0) {
            assert(has_single_bit(possible));
            int x = first_bit(possible);
            for (int wheel : wheel_order) {
                if (position[wheel] < 0) position[wheel] = x, add_known(x);
            }
        }
        int target = 0;
        for (int x = 1; x < n; ++x) {
            if (known_count[x] > known_count[target]) target = x;
        }
        for (int wheel : wheel_order) {
            if (position[wheel] != target) rotate_wheel(wheel, target - position[wheel]);
        }
    }

    void merge_opposite_sites(int b) {
        int target = first_bit(universe & ~(bit(0) | bit(b)));
        move_known(scout, target);
        if (has_single_bit(possible)) {
            finish();
            return;
        }
        int guard = wheel_order.front();
        locate(guard);
        if (has_single_bit(possible)) {
            finish();
            return;
        }
        move_known(guard, b);

        Mask at_target = bit(scout);
        for (int wheel : wheel_order) {
            if (wheel == guard) continue;
            if (wheel == scout) break;
            int before = distinct;
            rotate_wheel(wheel, target);
            if (distinct > before) {
                rotate_wheel(wheel, -target);
            } else {
                at_target |= bit(wheel);
                if (distinct < before) break;
            }
        }

        // The original site zero has disappeared. Untested wheels are all at b.
        bool keep_target = size_of(at_target) * 2 >= n;
        for (int wheel : wheel_order) {
            if (bool(at_target & bit(wheel)) != keep_target) {
                rotate_wheel(wheel, keep_target ? target - b : b - target);
            }
        }
    }

    // With one wheel fixed at b, a rotation by b-a either joins it from a
    // or opens a previously empty site. Put the latter wheel straight back.
    bool merge_two_sites(Mask start_sites) {
        if (size_of(start_sites) != 2 || !(start_sites & bit(0))) return false;
        int b = first_bit(start_sites ^ bit(0));
        if (add(b, b) == 0) {
            merge_opposite_sites(b);
            return true;
        }
        move_known(scout, b);
        for (int wheel : wheel_order) {
            if (wheel == scout) break;
            int before = distinct;
            rotate_wheel(wheel, b);
            if (distinct > before) rotate_wheel(wheel, -b);
        }
        return true;
    }

public:
    static void prepare() {
        // assignments[m][p][t] counts ordered placements of m wheels in p
        // sites with t prescribed sites that must each be represented.
        for (int p = 0; p <= 50; ++p) assignments[0][p][0] = 1;
        for (int m = 1; m <= 50; ++m) {
            for (int p = 0; p <= 50; ++p) {
                for (int t = 0; t <= p; ++t) {
                    assignments[m][p][t] = (p - t) * assignments[m - 1][p][t];
                    if (t > 0) assignments[m][p][t] += t * assignments[m - 1][p][t - 1];
                }
            }
        }
    }

    Machine(int n, int k, unsigned seed) : n(n), distinct(k), universe(bit(n) - 1), rng(seed),
        wheel_order(n), position(n, -1), site_order(n), possible(universe), unknown(n - 1) {
        iota(wheel_order.begin(), wheel_order.end(), 0);
        iota(site_order.begin(), site_order.end(), 0);
        shuffle(wheel_order.begin(), wheel_order.end(), rng);
        shuffle(site_order.begin(), site_order.end(), rng);
        scout = wheel_order.back();
    }

    void solve() {
        if (n <= 7) {
            int direction = rng() & 1 ? 1 : -1;
            SmallMachine solver(n);
            bool opposite = solver.solve(distinct, [&](int wheel, int step) {
                rotate_wheel(wheel_order[wheel], direction * step);
                return distinct;
            });
            if (opposite) {
                position[scout] = 0;
                add_known(0);
                required = bit(n / 2);
                occupied = possible = bit(0) | required;
                merge_opposite_sites(n / 2);
            }
            return;
        }
        if (distinct < n && distinct * 6 > n * 5 && n >= 10) {
            uniform_int_distribution<int> step(1, n - 1);
            for (int i = 0; i < n / 2 && distinct * 6 > n * 5; ++i) {
                rotate_wheel(wheel_order[i], step(rng));
            }
        }
        bool starts_with_two = distinct == 2;
        position[scout] = 0;
        add_known(0);
        Mask fixed = survey();
        if (starts_with_two && merge_two_sites(fixed)) return;
        for (int wheel : wheel_order) {
            if (wheel == scout || unknown == 0 || has_single_bit(possible)) break;
            locate(wheel);
        }
        finish();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n, k;
    if (!(cin >> n >> k) || n < 3 || n > 50 || k <= 1 || k > n) return 0;
    Machine::prepare();
    unsigned seed = chrono::steady_clock::now().time_since_epoch().count();
    Machine(n, k, seed).solve();
}

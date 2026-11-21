#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

constexpr int maxFake = 10;
using Count = __uint128_t;
using Sizes = array<int, maxFake + 1>;
using Polynomial = array<Count, maxFake + 1>;

// An unresolved component of weight w contributes either 0 or w
// counterfeits beyond the smaller of its two sides.
Polynomial allocations(const Sizes& sizes, int remaining) {
    Polynomial ways{};
    ways[0] = 1;
    for (int w = 1; w <= remaining; ++w) {
        for (int i = 0; i < sizes[w]; ++i) {
            for (int r = remaining; r >= w; --r) {
                ways[r] += ways[r - w];
            }
        }
    }
    return ways;
}

Polynomial removeFactor(Polynomial ways, int w, int remaining) {
    for (int r = w; r <= remaining; ++r) {
        ways[r] -= ways[r - w];
    }
    return ways;
}

struct Action {
    int first = 0;
    int second = 0;  // Zero denotes an already identified genuine coin.
};

struct Outcomes {
    Count same;
    Count different;
};

Outcomes pairOutcomes(const Polynomial& ways, int a, int b, int remaining) {
    Polynomial rest = removeFactor(removeFactor(ways, a, remaining), b, remaining);
    return {
        rest[remaining] + (remaining >= a + b ? rest[remaining - a - b] : 0),
        rest[remaining - a] + rest[remaining - b]
    };
}

struct Cost {
    Count total = 0;  // Sum over the feasible allocations.
    int worst = 0;
};

bool better(Cost a, Cost b, bool minimizeWorst) {
    if (minimizeWorst && a.worst != b.worst) {
        return a.worst < b.worst;
    }
    if (a.total != b.total) {
        return a.total < b.total;
    }
    return a.worst < b.worst;
}

Cost transition(Outcomes outcomes, Cost same, Cost different, bool balanced) {
    if (balanced && outcomes.different != 0) {
        // Both orientations of the balanced component have the same future
        // search, followed by one appraisal to identify its counterfeit side.
        different.total = 2 * different.total + outcomes.different;
        ++different.worst;
    }
    const bool needsQuery = outcomes.same != 0 && outcomes.different != 0;
    return {
        same.total + different.total + (needsQuery ? outcomes.same + outcomes.different : 0),
        max(same.worst, different.worst) + needsQuery
    };
}

class TwoUnitPlanner {
    struct Plan {
        uint32_t total = 0;
        uint16_t worst = 0;
        uint8_t action = 0;
        uint8_t withoutReference = 0;

        Cost cost() const { return {total, worst}; }
    };
    struct Entry {
        Plan average;
        Plan worst;
    };
    vector<vector<Entry>> table;

    static size_t ways(int ones, int twos) {
        return ones * (ones - 1) / 2 + twos;
    }

    static Plan plan(Cost cost, int action, int withoutReference) {
        return {static_cast<uint32_t>(cost.total), static_cast<uint16_t>(cost.worst),
                static_cast<uint8_t>(action), static_cast<uint8_t>(withoutReference)};
    }

    void build(int n) {
        vector<Cost> oneFake(n + 1);
        oneFake[2] = {2, 1};
        for (int i = 3; i <= n; ++i) {
            oneFake[i] = {oneFake[i - 2].total + i + 2, 1 + max(1, oneFake[i - 2].worst)};
        }

        table.resize(n + 1);
        // With two units left, only two weight-one components or one
        // weight-two component can have a counterfeit majority.
        for (int a = 0; a <= n; ++a) {
            table[a].resize((n - a) / 2 + 1);
            for (int b = 0; b <= (n - a) / 2; ++b) {
                Entry& entry = table[a][b];
                if (a <= 1) {
                    entry.average = entry.worst = plan(oneFake[b], 0, 0);
                    continue;
                }
                const Count count = ways(a, b);
                if (count <= 1) {
                    continue;
                }
                for (bool worst : {false, true}) {
                    auto child = [&](int ones, int twos) {
                        const Entry& next = table[ones][twos];
                        return (worst ? next.worst : next.average).cost();
                    };
                    Cost best{};
                    int action = 0;
                    auto consider = [&](int code, Count diff, Cost same, Cost different,
                                        bool balanced = false) {
                        Cost cost = transition({count - diff, diff}, same, different, balanced);
                        if (!action || better(cost, best, worst)) {
                            best = cost;
                            action = code;
                        }
                    };
                    consider(1, 2 * (a - 2), child(a - 2, b + 1), oneFake[a - 2], true);
                    if (b >= 2) {
                        consider(2, 2, child(a, b - 2), {}, true);
                    }
                    if (b >= 1) {
                        consider(3, a, child(a - 1, b - 1), oneFake[a]);
                    }
                    const int withoutReference = action;
                    consider(4, a - 1, child(a - 1, b), oneFake[a - 1]);
                    if (b >= 1) {
                        consider(5, 1, child(a, b - 1), {});
                    }
                    (worst ? entry.worst : entry.average) = plan(best, action, withoutReference);
                }
            }
        }
    }

public:
    Action choose(int n, int ones, int twos, bool hasReference, bool minimizeWorst) {
        if (table.empty()) {
            build(n);
        }
        const Entry& entry = table[ones][twos];
        const Plan& plan = minimizeWorst ? entry.worst : entry.average;
        constexpr Action actions[] = {{}, {1, 1}, {2, 2}, {1, 2}, {1, 0}, {2, 0}};
        return actions[hasReference ? plan.action : plan.withoutReference];
    }
};

class SmallPlanner {
    struct Plan {
        Cost cost;
        Action action;
    };
    unordered_map<uint64_t, Plan> memo;

    Plan solve(Sizes sizes, int remaining) {
        fill(sizes.begin() + remaining + 1, sizes.end(), 0);
        Polynomial ways = allocations(sizes, remaining);
        Count count = ways[remaining];
        if (count <= 1) {
            return {};
        }

        Sizes reduced = sizes;
        int left = remaining;
        for (int w = 1; w <= remaining; ++w) {
            if (sizes[w] != 0) {
                Polynomial rest = removeFactor(ways, w, remaining);
                if (rest[remaining] == 0) {
                    left -= w * sizes[w];
                    reduced[w] = 0;
                } else if (rest[remaining - w] == 0) {
                    reduced[w] = 0;
                }
            }
        }
        if (reduced != sizes) {
            return solve(reduced, left);
        }

        uint64_t key = remaining;
        for (int w = maxFake; w >= 1; --w) {
            key = (key << 5) | sizes[w];
        }
        auto found = memo.find(key);
        if (found != memo.end()) {
            return found->second;
        }

        Plan best{};
        auto consider = [&](Action action, Cost cost) {
            if (best.action.first == 0 || better(cost, best.cost, false)) {
                best = {cost, action};
            }
        };
        for (int a = 1; a <= remaining; ++a) {
            if (sizes[a] == 0) {
                continue;
            }
            Sizes next = sizes;
            --next[a];
            Polynomial rest = removeFactor(ways, a, remaining);
            Cost genuine = solve(next, remaining).cost;
            Cost counterfeit = solve(next, remaining - a).cost;
            consider({a, 0}, transition({rest[remaining], rest[remaining - a]}, genuine, counterfeit, false));

            for (int b = a; b <= remaining; ++b) {
                if (next[b] == 0) {
                    continue;
                }
                --next[b];
                Outcomes outcomes = pairOutcomes(ways, a, b, remaining);
                Cost same{};
                Cost different{};
                if (outcomes.same != 0) {
                    if (a + b <= remaining) {
                        ++next[a + b];
                    }
                    same = solve(next, remaining).cost;
                    if (a + b <= remaining) {
                        --next[a + b];
                    }
                }
                if (outcomes.different != 0) {
                    if (b != a) {
                        ++next[b - a];
                    }
                    different = solve(next, remaining - a).cost;
                    if (b != a) {
                        --next[b - a];
                    }
                }
                ++next[b];
                consider({a, b}, transition(outcomes, same, different, a == b));
            }
        }
        memo.emplace(key, best);
        return best;
    }

public:
    Action choose(const Sizes& sizes, int remaining) {
        return solve(sizes, remaining).action;
    }
};

struct Component {
    vector<int> majority;
    vector<int> minority;
};

template<class Appraise>
class Search {
    int n;
    int remaining;
    Appraise appraise;
    TwoUnitPlanner& twoUnits;
    SmallPlanner small;
    int genuine = 0;
    array<vector<Component>, maxFake + 1> pending;
    vector<Component> balanced;

    Component take(int w) {
        Component result = move(pending[w].back());
        pending[w].pop_back();
        return result;
    }

    void orient(const Component& component, bool majorityFake) {
        const vector<int>& fakeSide = majorityFake ? component.majority : component.minority;
        const vector<int>& genuineSide = majorityFake ? component.minority : component.majority;
        answer.insert(answer.end(), fakeSide.begin(), fakeSide.end());
        if (!genuineSide.empty()) {
            genuine = genuineSide.front();
        }
    }

    void store(Component component, int w) {
        if (w == 0) {
            balanced.push_back(move(component));
        } else if (w > remaining) {
            orient(component, false);
        } else {
            pending[w].push_back(move(component));
        }
    }

    bool resolveForced(Sizes& sizes, Polynomial& ways) {
        sizes = {};
        for (int w = 1; w <= maxFake; ++w) {
            if (w > remaining) {
                for (const Component& component : pending[w]) {
                    orient(component, false);
                }
                pending[w].clear();
            }
            sizes[w] = static_cast<int>(pending[w].size());
        }
        ways = allocations(sizes, remaining);
        if (ways[remaining] == 0) {
            return false;
        }
        const int oldRemaining = remaining;
        bool changed = false;
        for (int w = 1; w <= oldRemaining; ++w) {
            if (sizes[w] == 0) {
                continue;
            }
            Polynomial rest = removeFactor(ways, w, oldRemaining);
            if (rest[oldRemaining] == 0 || rest[oldRemaining - w] == 0) {
                bool majorityFake = rest[oldRemaining] == 0;
                for (const Component& component : pending[w]) {
                    orient(component, majorityFake);
                }
                remaining -= majorityFake ? w * sizes[w] : 0;
                sizes[w] = 0;
                pending[w].clear();
                changed = true;
            }
        }
        if (changed) {
            ways = allocations(sizes, remaining);
        }
        return true;
    }

    Action choose(const Sizes& sizes, const Polynomial& ways) {
        int unit = 0;
        int weight = 0;
        for (int w = 1; w <= remaining; ++w) {
            if (sizes[w]) {
                unit = gcd(unit, w);
                weight += w * sizes[w];
            }
        }
        Sizes scaled{};
        for (int w = unit; w <= remaining; w += unit) {
            scaled[w / unit] = sizes[w];
        }
        int target = remaining / unit;
        if (target == 1) {
            return {unit, scaled[1] == 2 && genuine != 0 ? 0 : unit};
        }
        if (target == 2) {
            // Use the worst-case plan when both units represent groups of
            // several coins. These states arise after counterfeit groups
            // have survived earlier rounds intact.
            Action action = twoUnits.choose(n, scaled[1], scaled[2], genuine != 0, unit > 1);
            return {action.first * unit, action.second * unit};
        }
        if (weight / unit <= 28 && genuine != 0) {
            Action action = small.choose(scaled, target);
            return {action.first * unit, action.second * unit};
        }

        // Equal-weight comparisons eliminate two independent candidates when
        // they differ, and consolidate them into a less likely one otherwise.
        Action fallback;
        for (int a = 1; a <= remaining; ++a) {
            if (sizes[a] == 0) {
                continue;
            }
            for (int b = a; b <= remaining; ++b) {
                if (sizes[b] <= (a == b)) {
                    continue;
                }
                Outcomes outcome = pairOutcomes(ways, a, b, remaining);
                if (outcome.same == 0 || outcome.different == 0) {
                    return {a, b};
                }
                if (fallback.first == 0) {
                    fallback = {a, b};
                }
            }
        }
        for (int w = 1; w <= remaining; ++w) {
            if (sizes[w] >= 2) {
                return {w, w};
            }
        }
        return genuine && fallback.first ? Action{fallback.first, 0} : fallback;
    }

    bool perform(Action action, const Polynomial& ways) {
        const int a = action.first;
        const int b = action.second;
        Component first = take(a);
        if (b == 0) {
            int response = appraise(first.majority.front(), genuine);
            if (response < 0) {
                return false;
            }
            remaining -= response * a;
            orient(first, response);
            return remaining >= 0;
        }

        Component second = take(b);
        Outcomes outcome = pairOutcomes(ways, a, b, remaining);
        int response = outcome.same == 0 ? 1 : 0;
        // A relation forced by the counterfeit count can be merged for free.
        if (outcome.same != 0 && outcome.different != 0) {
            response = appraise(first.majority.front(), second.majority.front());
            if (response < 0) {
                return false;
            }
        }
        if (response == 0) {
            first.majority.insert(first.majority.end(), second.majority.begin(), second.majority.end());
            first.minority.insert(first.minority.end(), second.minority.begin(), second.minority.end());
            store(move(first), a + b);
        } else {
            if (b > a) {
                swap(first, second);
            }
            first.majority.insert(first.majority.end(), second.minority.begin(), second.minority.end());
            first.minority.insert(first.minority.end(), second.majority.begin(), second.majority.end());
            remaining -= min(a, b);
            store(move(first), max(a, b) - min(a, b));
        }
        return true;
    }

public:
    vector<int> answer;

    Search(int count, int fake, Appraise compare, TwoUnitPlanner& planner)
        : n(count), remaining(fake), appraise(move(compare)), twoUnits(planner) {}

    bool run(const vector<int>& order) {
        for (int coin : order) {
            pending[1].push_back({{coin}, {}});
        }
        while (true) {
            Sizes sizes;
            Polynomial ways;
            if (!resolveForced(sizes, ways)) {
                return false;
            }
            if (remaining == 0) {
                break;
            }
            Action action = choose(sizes, ways);
            if (action.first == 0 || !perform(action, ways)) {
                return false;
            }
        }
        for (const Component& component : balanced) {
            if (genuine == 0) {
                return false;
            }
            int response = appraise(component.majority.front(), genuine);
            if (response < 0) {
                return false;
            }
            orient(component, response);
        }
        return true;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) {
        return 0;
    }
    vector<int> order(n);
    iota(order.begin(), order.end(), 1);
    mt19937_64 random(chrono::steady_clock::now().time_since_epoch().count() ^ random_device{}());
    shuffle(order.begin(), order.end(), random);

    auto appraise = [](int x, int y) {
        cout << "? " << x << ' ' << y << endl;
        int response;
        if (!(cin >> response) || (response != 0 && response != 1)) {
            return -1;
        }
        return response;
    };
    TwoUnitPlanner planner;
    Search search(n, m, appraise, planner);
    if (search.run(order) && static_cast<int>(search.answer.size()) == m) {
        cout << '!';
        for (int coin : search.answer) {
            cout << ' ' << coin;
        }
        cout << endl;
    }
    return 0;
}

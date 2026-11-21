#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using namespace std;

using Answer = pair<int, int>;

Answer answer_from_mask(unsigned mask) {
    const int first = countr_zero(mask);
    mask &= mask - 1;
    return {first, mask ? int(countr_zero(mask)) : first};
}

struct Decision {
    int a, b;
    array<int, 6> next;
};

constexpr Decision policy_3[] = {
    { 0, 1, { 1, -3, } },
    { 0, 2, { -6, -5, } },
};
constexpr Decision policy_4[] = {
    { 0, 1, { 1, -3, -3, } },
    { 2, 3, { 2, -12, -12, } },
    { 0, 2, { -10, -5, -5, } },
};
constexpr Decision policy_5[] = {
    { 0, 1, { 1, 5, -3, -3, } },
    { 0, 2, { 2, 4, -5, -5, } },
    { 1, 2, { -24, 3, -6, -6, } },
    { 1, 3, { -20, -12, -2, -10, } },
    { 0, 3, { -20, -12, -1, -9, } },
    { 0, 2, { 6, -6, -1, -5, } },
    { 0, 3, { -18, -10, -1, -9, } },
};
constexpr Decision policy_6[] = {
    { 0, 1, { 1, 9, -3, -3, -3, } },
    { 2, 3, { 2, 6, -12, -12, -12, } },
    { 0, 2, { 3, 5, -5, -5, -5, } },
    { 1, 3, { -48, 4, -10, -10, -10, } },
    { 1, 4, { -40, -24, -2, -18, -2, } },
    { 0, 4, { -36, -20, -1, -17, -1, } },
    { 2, 4, { 7, -24, -4, -20, -4, } },
    { 3, 5, { 8, -36, -8, -40, -8, } },
    { 0, 2, { -10, -9, -4, -5, -4, } },
    { 0, 2, { 10, -6, -1, -5, -1, } },
    { 1, 3, { 11, -9, -2, -10, -2, } },
    { 0, 4, { -34, -18, -1, -17, -1, } },
};
constexpr Decision policy_7[] = {
    { 0, 1, { 1, 57, 75, -3, -3, -3, } },
    { 2, 3, { 2, 33, 47, -12, -12, -12, } },
    { 4, 5, { 3, 17, 27, -48, -48, -48, } },
    { 1, 6, { 4, 9, 14, -66, -64, -66, } },
    { 0, 6, { 0, 5, 0, -65, 0, -65, } },
    { 0, 2, { 6, 7, 8, -5, -1, -5, } },
    { 4, 6, { -40, -24, -72, -80, -64, -80, } },
    { 2, 4, { -96, -80, -68, -20, -4, -20, } },
    { 0, 3, { 0, -48, 0, -1, 0, 0, } },
    { 5, 6, { 10, 12, 13, -96, -64, -96, } },
    { 4, 6, { 0, 11, 0, -80, 0, -80, } },
    { 1, 2, { -24, -20, -18, -6, -2, -6, } },
    { 1, 2, { -40, -36, -34, -6, -2, -6, } },
    { 0, 2, { -72, 0, 0, 0, -4, 0, } },
    { 5, 6, { 15, 16, 0, -64, -64, -96, } },
    { 2, 6, { -24, -80, 0, -64, -64, -68, } },
    { 0, 2, { -72, -68, 0, -4, 0, 0, } },
    { 5, 6, { 18, 23, 25, -96, -32, -96, } },
    { 0, 4, { 19, 21, 22, -17, -16, -17, } },
    { 1, 4, { 0, 20, 0, -18, 0, -18, } },
    { 1, 2, { -40, -36, -34, -6, -2, -6, } },
    { 0, 2, { -40, -36, -33, -5, -1, -5, } },
    { 0, 2, { -24, 0, -20, 0, 0, 0, } },
    { 4, 6, { 0, 24, 0, -80, 0, -80, } },
    { 0, 2, { -10, 0, -5, 0, -5, 0, } },
    { 0, 5, { 26, -12, 0, -32, -32, -33, } },
    { 1, 5, { 0, -12, 0, -32, 0, -34, } },
    { 5, 6, { 28, 31, -80, -32, 0, -96, } },
    { 0, 4, { 29, 30, 0, -16, -16, -17, } },
    { 2, 4, { -10, -18, 0, -16, -16, -20, } },
    { 0, 2, { -24, -20, 0, -4, 0, 0, } },
    { 0, 5, { 32, -12, 0, -32, -32, -33, } },
    { 1, 5, { 0, -12, 0, -32, 0, -34, } },
    { 2, 4, { 34, 41, 44, -20, -4, -20, } },
    { 3, 5, { 35, 37, 40, -40, -8, -40, } },
    { 2, 6, { 0, 36, 0, -68, 0, -68, } },
    { 0, 3, { -66, -65, -72, -9, -8, -9, } },
    { 5, 6, { 38, -68, 39, -96, -32, -96, } },
    { 0, 2, { -34, -33, -36, -5, -4, -5, } },
    { 0, 2, { -34, -33, 0, -5, -4, 0, } },
    { 3, 6, { 0, -3, 0, -8, 0, -72, } },
    { 3, 4, { 0, 42, 0, -24, 0, -24, } },
    { 5, 6, { 43, 0, -96, 0, -96, 0, } },
    { 0, 5, { -66, 0, -33, 0, -33, 0, } },
    { 2, 5, { 45, 46, 0, -4, -4, -36, } },
    { 0, 2, { -66, -64, 0, -4, -4, -5, } },
    { 0, 2, { -66, 0, 0, 0, 0, -1, } },
    { 2, 4, { 48, 54, -24, -4, -4, -20, } },
    { 3, 5, { 49, 52, -36, -8, -8, -40, } },
    { 2, 6, { 50, 51, -72, -4, 0, -68, } },
    { 0, 2, { -10, 0, -9, 0, 0, 0, } },
    { 0, 2, { -2, 0, 0, 0, -4, -5, } },
    { 3, 6, { 53, -3, 0, -8, -8, -72, } },
    { 0, 2, { -10, 0, -9, 0, 0, 0, } },
    { 2, 5, { 55, 56, 0, -4, -4, -36, } },
    { 0, 2, { -66, -64, 0, -4, -4, -5, } },
    { 0, 2, { -66, 0, 0, 0, 0, -1, } },
    { 0, 2, { 58, 68, 72, -5, -1, -5, } },
    { 1, 3, { 59, 63, 66, -10, -2, -10, } },
    { 0, 4, { 60, 61, 62, -17, -1, -17, } },
    { 1, 5, { 0, -96, 0, -34, 0, -34, } },
    { 1, 4, { 0, -96, 0, -18, 0, -18, } },
    { 0, 3, { -96, 0, 0, 0, -1, 0, } },
    { 3, 4, { 64, -17, 65, -24, -8, -24, } },
    { 0, 3, { 0, -96, 0, -9, 0, -9, } },
    { 0, 3, { 0, -96, 0, -8, 0, -9, } },
    { 1, 4, { 67, -96, 0, -2, -2, -18, } },
    { 1, 2, { -96, 0, 0, 0, -2, 0, } },
    { 1, 2, { 0, 69, 0, -6, 0, -6, } },
    { 0, 3, { 70, -8, 71, -8, 0, -8, } },
    { 0, 4, { -96, -16, -96, -16, 0, -16, } },
    { 0, 4, { -96, 0, 0, 0, 0, -16, } },
    { 0, 3, { 73, 74, 0, -1, -1, -9, } },
    { 0, 4, { -96, -96, 0, -1, -1, -17, } },
    { 0, 4, { -96, 0, 0, 0, 0, -16, } },
    { 0, 2, { 76, 82, -6, -1, -1, -5, } },
    { 1, 3, { 77, 80, -9, -2, -2, -10, } },
    { 0, 4, { 78, 79, -18, -1, -1, -17, } },
    { 0, 5, { -66, -64, -34, 0, 0, -32, } },
    { 0, 3, { -96, -96, 0, -1, -1, 0, } },
    { 1, 4, { 81, -96, 0, -2, -2, -18, } },
    { 0, 5, { -66, 0, -34, 0, 0, 0, } },
    { 0, 3, { 83, 84, 0, -1, -1, -9, } },
    { 0, 4, { -96, -96, 0, -1, -1, -17, } },
    { 0, 4, { -96, 0, 0, 0, 0, -16, } },
};

// The small decision trees minimize total queries over the consistent
// permutations. Negative transitions encode the possible zero positions.
template<class Ask>
Answer solve_tiny(int n, Ask& ask) {
    const Decision* const policies[] = {policy_3, policy_4, policy_5,
                                        policy_6, policy_7};
    const Decision* policy = policies[n - 3];
    int node = 0;
    do {
        const auto& decision = policy[node];
        const int g = ask(decision.a, decision.b);
        node = decision.next[g - 1];
    } while (node >= 0);
    return answer_from_mask(-node);
}

template<class Ask>
class SmallSearch {
    using Permutation = uint64_t;
    struct Query {
        int a, b;
    };
    struct Outcome {
        int count = 0;
        unsigned zeros = 0;
        array<int, 10> counts{};
    };
    struct Option {
        int cost, edge;
    };

    int n;
    Ask& ask;
    array<array<int, 10>, 10> gcd_table{};
    vector<Query> edges;
    const bool use_distribution;

    int reply(Permutation p, Query q) const {
        return gcd_table[(p >> (4 * q.a)) & 15][(p >> (4 * q.b)) & 15];
    }

    static unsigned zero_bit(Permutation p) {
        return 1U << (p >> 40);
    }

    vector<Permutation> initial_belief(int g) const {
        vector<Permutation> belief;
        for (int a = 0; a < n; ++a) {
            for (int b = 0; b < n; ++b) {
                if (a == b || gcd_table[a][b] != g) continue;
                array<int, 8> rest{};
                int size = 0;
                for (int v = 0; v < n; ++v) {
                    if (v != a && v != b) rest[size++] = v;
                }
                do {
                    Permutation p = a | (b << 4);
                    int zero = a == 0 ? 0 : 1;
                    for (int i = 0; i < size; ++i) {
                        p |= Permutation(rest[i]) << (4 * (i + 2));
                        if (rest[i] == 0) zero = i + 2;
                    }
                    belief.push_back(p | (Permutation(zero) << 40));
                } while (next_permutation(rest.begin(), rest.begin() + size));
            }
        }
        return belief;
    }

    vector<Option> rank(const vector<Permutation>& belief, unsigned touched,
                        uint64_t asked) const {
        const unsigned unused = ((1U << n) - 1) ^ touched;
        const int first = countr_zero(unused);
        const int second = countr_zero(unused & (unused - 1));
        vector<Option> options;
        for (int e = 0; e < int(edges.size()); ++e) {
            if ((asked >> e) & 1) continue;
            const auto [a, b] = edges[e];
            const bool new_a = (unused >> a) & 1;
            const bool new_b = (unused >> b) & 1;
            if (new_a && new_b) {
                if (a != first || b != second) continue;
            } else if ((new_a && a != first) || (new_b && b != first)) {
                continue;
            }
            array<Outcome, 10> outcomes{};
            for (const Permutation p : belief) {
                auto& branch = outcomes[reply(p, edges[e])];
                ++branch.count;
                branch.zeros |= zero_bit(p);
                if (use_distribution) ++branch.counts[p >> 40];
            }
            int cost = 0;
            bool informative = true;
            for (auto& branch : outcomes) {
                if (branch.count == int(belief.size())) {
                    informative = false;
                    break;
                }
                const int candidates = popcount(branch.zeros);
                if (candidates <= 2) continue;
                if (!use_distribution) {
                    cost += branch.count * max(4, candidates);
                    continue;
                }
                // Estimate the cost of checking pairs in decreasing order
                // of their probability of containing zero.
                sort(branch.counts.begin(), branch.counts.end(), greater<>());
                for (int i = 0; i < candidates; ++i) {
                    const int checks = min(1 + i / 2, (candidates - 1) / 2);
                    cost += 4 * branch.counts[i] * checks;
                }
            }
            if (informative) options.push_back({cost, e});
        }
        sort(options.begin(), options.end(), [](Option a, Option b) {
            return pair(a.cost, a.edge) < pair(b.cost, b.edge);
        });
        return options;
    }

    int choose(const vector<Permutation>& belief, unsigned touched,
               uint64_t asked) const {
        const auto options = rank(belief, touched, asked);
        if (options.front().cost == 0 || options.size() == 1 ||
            belief.size() > 140000) return options.front().edge;

        int best_edge = options.front().edge;
        int best_cost = 40 * int(belief.size());
        for (int i = 0; i < min(5, int(options.size())); ++i) {
            const int e = options[i].edge;
            const auto [a, b] = edges[e];
            array<vector<Permutation>, 10> branches;
            array<unsigned, 10> zeros{};
            for (const Permutation p : belief) {
                const int g = reply(p, edges[e]);
                branches[g].push_back(p);
                zeros[g] |= zero_bit(p);
            }
            int cost = 0;
            for (int g = 1; g < n; ++g) {
                if (popcount(zeros[g]) <= 2) continue;
                const auto next = rank(branches[g], touched | (1U << a) | (1U << b),
                                       asked | (uint64_t{1} << e));
                cost += 4 * int(branches[g].size()) + next.front().cost;
                if (cost >= best_cost) break;
            }
            if (cost < best_cost) {
                best_cost = cost;
                best_edge = e;
            }
        }
        return best_edge;
    }

public:
    SmallSearch(int size, Ask& oracle)
        : n(size), ask(oracle), use_distribution(n >= 9) {}

    Answer run() {
        const int first_g = ask(0, 1);
        if (first_g > (n - 1) / 2) return {0, 1};
        for (int a = 0; a < n; ++a) {
            for (int b = 0; b < n; ++b) gcd_table[a][b] = gcd(a, b);
            for (int b = a + 1; b < n; ++b) edges.push_back({a, b});
        }

        auto belief = initial_belief(first_g);
        uint64_t asked = 1;
        unsigned touched = 3;
        for (;;) {
            unsigned zeros = 0;
            for (const Permutation p : belief) zeros |= zero_bit(p);
            if (popcount(zeros) <= 2) return answer_from_mask(zeros);
            const int e = choose(belief, touched, asked);
            const auto [a, b] = edges[e];
            const int g = ask(a, b);
            if (g > (n - 1) / 2) return {a, b};
            asked |= uint64_t{1} << e;
            touched |= (1U << a) | (1U << b);
            erase_if(belief, [&](Permutation p) { return reply(p, edges[e]) != g; });
        }
    }
};

// A bounded constraint check can prove additional exclusions in small
// permutations. Hitting its search budget leaves a candidate possible.
class Feasibility {
    using Mask = uint32_t;
    static constexpr int max_size = 26;
    using Domains = array<Mask, max_size>;
    struct Edge {
        int a, b, g;
    };

    int n;
    vector<Edge> edges;
    array<array<Mask, max_size>, max_size> compatible{};
    array<bool, max_size> touched{};

    Mask supported(Mask domain, Mask other, int g) const {
        Mask result = 0;
        while (domain) {
            const int v = countr_zero(domain);
            const Mask bit = Mask{1} << v;
            if (compatible[g][v] & other) result |= bit;
            domain ^= bit;
        }
        return result;
    }

    bool feasible(Domains domain, int& budget) const {
        if (budget == 0) return true;
        --budget;
        bool changed;
        do {
            changed = false;
            Mask assigned = 0;
            for (int i = 0; i < n; ++i) {
                const Mask d = domain[i];
                if (!d) return false;
                if (!has_single_bit(d)) continue;
                if (assigned & d) return false;
                assigned |= d;
            }
            for (int i = 0; i < n; ++i) {
                const Mask d = domain[i];
                if (has_single_bit(d)) continue;
                domain[i] &= ~assigned;
                changed |= domain[i] != d;
            }
            for (const auto [a, b, g] : edges) {
                const Mask old_a = domain[a], old_b = domain[b];
                domain[a] = supported(old_a, old_b, g);
                domain[b] = supported(old_b, domain[a], g);
                changed |= (domain[a] != old_a || domain[b] != old_b);
            }
        } while (changed);

        int choose = -1, best = n + 1;
        for (int i = 0; i < n; ++i) {
            if (!touched[i]) continue;
            const int count = popcount(domain[i]);
            if (count > 1 && count < best) {
                choose = i;
                best = count;
            }
        }
        if (choose < 0) return true;

        Mask choices = domain[choose];
        while (choices) {
            domain[choose] = choices & -choices;
            if (feasible(domain, budget)) return true;
            choices &= choices - 1;
        }
        return false;
    }

public:
    static constexpr bool enabled(int n) { return n <= max_size; }

    explicit Feasibility(int size) : n(size) {
        if (!enabled(n)) return;
        for (int a = 0; a < n; ++a) {
            for (int b = 0; b < n; ++b) {
                if (a != b) compatible[gcd(a, b)][a] |= Mask{1} << b;
            }
        }
    }

    void record(int a, int b, int g) {
        edges.push_back({a, b, g});
        touched[a] = touched[b] = true;
    }

    bool is_touched(int v) const { return touched[v]; }

    bool allows_zero(int v, const vector<int>& multiple) const {
        const Mask positive = (Mask{1} << n) - 2;
        Domains domain;
        for (int u = 0; u < n; ++u) {
            if (u == v) {
                domain[u] = 1;
            } else if (multiple[u] == 1) {
                domain[u] = positive;
            } else {
                domain[u] = 0;
                for (int s = multiple[u]; s < n; s += multiple[u]) {
                    domain[u] |= Mask{1} << s;
                }
            }
        }
        for (const auto [a, b, g] : edges) {
            const Mask value = Mask{1} << g;
            if (a == v) domain[b] &= value;
            if (b == v) domain[a] &= value;
        }
        int budget = 70;
        return feasible(domain, budget);
    }
};

template<class Ask>
class Search {
    struct Edge {
        int to, g;
    };

    const int n;
    Ask& ask;
    mt19937& random;
    vector<int> order;
    vector<int> multiple;
    vector<int> counts;
    vector<vector<Edge>> incident;
    vector<uint8_t> possible;
    int remaining;
    unordered_map<unsigned, int> answers;
    unordered_set<unsigned> observed;
    set<pair<int, int>> anchors;
    Feasibility constraints;

    unsigned key(int a, int b) const {
        return unsigned(a) * n + b;
    }

    unsigned edge_key(int a, int b) const {
        if (a > b) swap(a, b);
        return key(a, b);
    }

    bool queried(int a, int b) const {
        return answers.contains(edge_key(a, b));
    }

    void exclude(int v) {
        if (possible[v]) {
            possible[v] = false;
            --remaining;
        }
    }

    void finish_if_ready() const {
        if (remaining > 2) return;
        int a = -1, b = -1;
        for (const int v : order) {
            if (!possible[v]) continue;
            if (a < 0) a = v;
            else b = v;
        }
        throw Answer{a, b < 0 ? a : b};
    }

    void add_divisor(int d, int old) {
        if (old % d == 0) return;
        if (++counts[d] != 1 + (n - 1) / d) return;
        // There are only floor((n-1)/d) positive multiples of d.
        for (const int v : order) {
            if (multiple[v] % d != 0) exclude(v);
        }
    }

    void record(int a, int b, int g) {
        // With zero at a, its neighbors would all have distinct values.
        if (possible[a] && !observed.insert(key(a, g)).second) exclude(a);

        const int old = multiple[a];
        const int bound = lcm(old, g);
        if (bound >= n) throw Answer{a, a};
        if (bound != old) {
            if (old > 1) anchors.erase({old, a});
            anchors.emplace(bound, a);
            multiple[a] = bound;
            add_divisor(bound, old);
            for (int d = 2; d * d <= bound; ++d) {
                if (bound % d != 0) continue;
                add_divisor(d, old);
                if (d != bound / d) add_divisor(bound / d, old);
            }
            for (const auto [v, h] : incident[a]) {
                if (h != bound) exclude(v);
            }
        }

        // With zero at b, the value at a would have to be exactly g.
        if (g != bound) exclude(b);
        incident[a].push_back({b, g});
    }

    void check_feasibility() {
        bool allow_untouched = true, checked_untouched = false;
        for (const int v : order) {
            if (!possible[v]) continue;
            const bool touched = constraints.is_touched(v);
            if (touched) {
                if (!constraints.allows_zero(v, multiple)) exclude(v);
            } else {
                if (!checked_untouched) {
                    allow_untouched = constraints.allows_zero(v, multiple);
                    checked_untouched = true;
                }
                if (!allow_untouched) exclude(v);
            }
            finish_if_ready();
        }
    }

    int query(int a, int b) {
        const unsigned id = edge_key(a, b);
        const auto cached = answers.find(id);
        if (cached != answers.end()) return cached->second;
        const int g = ask(a, b);
        if (g > (n - 1) / 2) throw Answer{a, b};
        answers.emplace(id, g);
        record(a, b, g);
        record(b, a, g);
        finish_if_ready();
        if (Feasibility::enabled(n)) {
            constraints.record(a, b, g);
            if (answers.size() >= 3) check_feasibility();
        }
        return g;
    }

    int witness(int v) const {
        int best = -1, best_ratio = 1;
        int inspected = 0;
        for (auto it = anchors.rbegin(); it != anchors.rend() && inspected < 28;
             ++it, ++inspected) {
            const auto [bound, w] = *it;
            if (w == v || bound <= best_ratio || queried(v, w)) continue;
            const int ratio = bound / gcd(bound, multiple[v]);
            if (ratio > best_ratio) {
                best_ratio = ratio;
                best = w;
                // If the two multiples have LCM >= n, one probe either
                // rules out v or certifies it as zero.
                if (ratio * multiple[v] >= n) break;
            }
        }
        return best_ratio >= 3 ? best : -1;
    }

    int random_witness(int v) {
        uniform_int_distribution<int> pick(0, n - 1);
        for (int it = 0; it < 12; ++it) {
            const int w = pick(random);
            if (w != v && !queried(v, w)) return w;
        }
        // Verification is optional. The tournament can also use cached edges.
        return -1;
    }

    void verify(int v) {
        for (int attempt = 0; attempt < 2 && possible[v]; ++attempt) {
            const int old = multiple[v];
            int w = attempt == 0 ? witness(v) : -1;
            if (w < 0) w = random_witness(v);
            if (w < 0) return;
            query(v, w);
            if (multiple[v] == old) break;
        }
    }

    void scan(int threshold, int urgent) {
        int a = -1;
        // Cross two promising pairs to test up to four candidates with
        // two queries. Very large replies are checked immediately.
        Answer waiting{-1, -1};
        for (const int b : order) {
            if (!possible[b]) continue;
            if (a < 0 || !possible[a]) {
                a = b;
                continue;
            }
            const int g = query(a, b);
            if (g >= threshold) {
                auto [x, y] = waiting;
                if (x >= 0 && (possible[x] || possible[y])) {
                    int next = 0;
                    const array<int, 2> pending{x, y};
                    for (const int v : {a, b}) {
                        if (!possible[v]) continue;
                        while (next < 2 && !possible[pending[next]]) ++next;
                        if (next == 2) break;
                        const int w = pending[next++];
                        if (!queried(v, w)) query(v, w);
                    }
                    for (const int v : {a, b, x, y}) verify(v);
                    waiting = {-1, -1};
                } else if (g >= urgent) {
                    verify(a);
                    verify(b);
                } else {
                    waiting = {a, b};
                }
            }
            a = -1;
        }
    }

    vector<int> ranked_candidates() {
        vector<int> candidates;
        candidates.reserve(remaining);
        for (const int v : order) {
            if (possible[v]) candidates.push_back(v);
        }
        shuffle(candidates.begin(), candidates.end(), random);
        stable_sort(candidates.begin(), candidates.end(), [&](int a, int b) {
            return multiple[a] > multiple[b];
        });
        return candidates;
    }

    void rescan(int threshold) {
        auto candidates = ranked_candidates();
        int a = -1;
        for (int i = 0; i < int(candidates.size()); ++i) {
            int b = candidates[i];
            if (!possible[b]) continue;
            if (a < 0 || !possible[a]) {
                a = b;
                continue;
            }

            if (queried(a, b)) {
                int next = i + 1;
                while (next < int(candidates.size()) && next <= i + 8 &&
                       (!possible[candidates[next]] || queried(a, candidates[next]))) {
                    ++next;
                }
                if (next < int(candidates.size()) && next <= i + 8) {
                    swap(candidates[i], candidates[next]);
                    b = candidates[i];
                } else {
                    verify(a);
                    a = b;
                    continue;
                }
            }
            query(a, b);
            if (multiple[a] >= threshold) verify(a);
            if (multiple[b] >= threshold) verify(b);
            a = -1;
        }
    }

    void tournament() {
        const auto candidates = ranked_candidates();
        vector<int> live;
        live.reserve(3);
        for (const int c : candidates) {
            if (!possible[c]) continue;
            erase_if(live, [&](int v) { return !possible[v]; });
            live.push_back(c);
            if (live.size() < 3) continue;
            // Two edges sharing a vertex always rule out one of the three
            // zero positions, by divisibility or by a repeated reply.
            const int a = live[0], b = live[1];
            if (queried(a, b)) {
                query(a, c);
            } else if (queried(a, c)) {
                query(a, b);
            } else if (queried(b, c)) {
                query(b, a);
            } else {
                query(a, b);
                if (possible[a] && possible[b] && possible[c]) query(a, c);
            }
            erase_if(live, [&](int v) { return !possible[v]; });
            assert(live.size() <= 2);
        }
        finish_if_ready();
    }

public:
    Search(int size, Ask& oracle, mt19937& generator)
        : n(size), ask(oracle), random(generator), order(n), multiple(n, 1),
          counts(n), incident(n), possible(n, true), remaining(n), constraints(n) {
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin(), order.end(), random);
        answers.reserve(n);
        observed.reserve(n + n / 2);
    }

    Answer run() {
        try {
            const double root = sqrt(n);
            const int threshold = max(2, int(lround((n < 128 ? 0.92 : 0.82) * root)));
            const int urgent = threshold * (n < 128 ? 1 : n < 768 ? 2 : 3);
            scan(threshold, urgent);
            rescan(max(2, int(lround(0.35 * root))));
            tournament();
        } catch (const Answer& result) {
            return result;
        }
        abort();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;

    const auto clock_seed = chrono::steady_clock::now().time_since_epoch().count();
    random_device entropy;
    seed_seq seed{entropy(), entropy(), uint32_t(clock_seed), uint32_t(clock_seed >> 32)};
    mt19937 random(seed);

    vector<int> labels;
    if (n <= 10) {
        labels.resize(n);
        iota(labels.begin(), labels.end(), 0);
        shuffle(labels.begin(), labels.end(), random);
    }
    const auto index = [&](int v) { return (labels.empty() ? v : labels[v]) + 1; };
    auto ask = [&](int a, int b) {
        cout << "? " << index(a) << ' ' << index(b) << endl;
        int g;
        if (!(cin >> g) || g == -1) std::exit(0);
        return g;
    };

    Answer answer{0, 1};
    if (n >= 3 && n <= 7) answer = solve_tiny(n, ask);
    else if (n <= 10 && n >= 8) answer = SmallSearch(n, ask).run();
    else if (n > 10) answer = Search(n, ask, random).run();

    cout << "! " << index(answer.first) << ' ' << index(answer.second) << endl;
    int verdict;
    if (!(cin >> verdict) || verdict != 1) return 0;
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

namespace {

constexpr int cap = 15;
constexpr int overflow_rank = cap + 1;
constexpr int bridge_rank = cap + 2;
constexpr int max_outcomes = bridge_rank;

// A deleted edge can be bypassed along the rest of its unique cycle.
// Deleting a bridge, or deleting two edges of one cycle, leaves no such bypass.
// Thus a successful query deletes at most one edge from each cycle and no bridges.
struct Entry {
    int edge;
    int order;
    int query;
};

struct Pending {
    int edge;
    int order;
    int upper;  // The number of first-layer representatives at this edge's failed test.
};

// Every available comparison cuts an ordered list of outcomes at a threshold.
class SearchTree {
    double cost[max_outcomes][max_outcomes]{};
    int split[max_outcomes][max_outcomes]{};

public:
    double build(const double* weights, int n) {
        if (n < 2) return 0;
        double prefix[max_outcomes + 1]{};
        for (int i = 0; i < n; ++i) prefix[i + 1] = prefix[i] + weights[i];
        for (int width = 2; width <= n; ++width) {
            for (int l = 0, r = width - 1; r < n; ++l, ++r) {
                double best = numeric_limits<double>::infinity();
                for (int k = l; k < r; ++k) {
                    double next = cost[l][k] + cost[k + 1][r];
                    if (next < best) {
                        best = next;
                        split[l][r] = k;
                    }
                }
                cost[l][r] = best + prefix[r + 1] - prefix[l];
            }
        }
        return cost[0][n - 1];
    }

    int at(int l, int r) const {
        return split[l][r];
    }
};

class Fenwick {
    int n;
    vector<double> bit;

public:
    explicit Fenwick(int size) : n(size), bit(size + 1) {}

    void add(int i, double delta) {
        for (; i <= n; i += i & -i) bit[i] += delta;
    }

    double sum(int i) const {
        double value = 0;
        for (; i > 0; i -= i & -i) value += bit[i];
        return value;
    }

    int lower_bound(double target) const {
        int i = 0;
        for (int step = int(bit_floor(unsigned(n))); step; step >>= 1) {
            int next = i + step;
            if (next <= n && bit[next] < target) {
                i = next;
                target -= bit[next];
            }
        }
        return min(i + 1, n);
    }
};

class Solver {
    int m;
    int queries = 0;
    vector<int> answer;
    // After a block merge, a cached singleton can also delete fixed, inactive classes.
    vector<int> singleton;
    array<vector<Entry>, cap + 1> layer;
    vector<Pending> pending;
    double observed_bridges = 0;
    double observed_nonbridges = 0;
    bool root_first = false;
    int root_tested = 0;
    int root_bridges = 0;
    int first_failures = 0;
    array<double, overflow_rank + 1> exposure{}, hits{};
    double rank_events = 0;

    int ask(int parent, int edge) {
        cout << "? " << parent << ' ' << edge << '\n' << flush;
        ++queries;
        int reply;
        if (!(cin >> reply) || (reply != 0 && reply != 1)) exit(0);
        if (reply == 0) return 0;
        if (parent == 0) singleton[edge] = queries;
        return queries;
    }

    int test_singleton(int edge) {
        return singleton[edge] ? singleton[edge] : ask(0, edge);
    }

    static int tail_query(const vector<Entry>& entries) {
        return entries.empty() ? 0 : entries.back().query;
    }

    static int reported_size(int size) {
        return size >= cap ? 0 : size;
    }

    double bridge_probability() const {
        return (observed_bridges + 0.65) /
               (observed_bridges + observed_nonbridges + 4.0);
    }

    void observe_bridge(bool is_bridge) {
        observed_bridges = 0.985 * observed_bridges + is_bridge;
        observed_nonbridges = 0.985 * observed_nonbridges + !is_bridge;
    }

    void sample_pending(int limit) {
        for (auto it = pending.rbegin(); it != pending.rend() && limit > 0; ++it) {
            int edge = it->edge;
            if (singleton[edge] || answer[edge] == -1) continue;
            --limit;
            ++root_tested;
            bool bridge = !ask(0, edge);
            observe_bridge(bridge);
            if (bridge) {
                answer[edge] = -1;
                ++root_bridges;
            }
        }
    }

    int unconfirmed_pending() const {
        return count_if(pending.begin(), pending.end(), [&](const Pending& p) {
            return !singleton[p.edge] && answer[p.edge] != -1;
        });
    }

    void build_first_layer(const vector<int>& order, int begin, int end) {
        auto& first = layer[1];
        for (int i = begin; i < end; ++i) {
            int edge = order[i];
            int parent = tail_query(first);
            int result;
            if (root_first || parent == 0) {
                result = ask(0, edge);
                ++root_tested;
                if (!result) {
                    answer[edge] = -1;
                    ++root_bridges;
                } else if (parent) {
                    result = ask(parent, edge);
                }
            } else {
                result = ask(parent, edge);
            }

            if (result) {
                first.push_back({edge, i, result});
            } else {
                ++first_failures;
                if (answer[edge] != -1) pending.push_back({edge, i, int(first.size())});
            }

            if ((i == 47 || i == 191) && m > 2 * (i + 1) && !root_first &&
                first_failures * 100 > 54 * (i + 1)) {
                sample_pending(18);
                double estimated_fraction =
                    (root_bridges + bridge_probability() * unconfirmed_pending()) / (i + 1);
                root_first = estimated_fraction > 0.55 ||
                    (root_tested >= 8 && root_bridges * 100 >= root_tested * 82 &&
                     first_failures * 100 >= 75 * (i + 1));
            }
        }
    }

    bool prefer_online(int processed) {
        if (processed == m) return false;
        double minimum_collisions = 0.70 * processed * (processed - 1.0) / m;
        double possible_nonbridge_fraction = root_bridges ? 0.55 : 1;
        if (pending.size() < max(5.0, possible_nonbridge_fraction * minimum_collisions)) {
            return false;
        }
        int unknown = unconfirmed_pending();
        if (unknown >= 6 && unknown * 100 >= processed * 9 &&
            observed_bridges + observed_nonbridges < 14) {
            sample_pending(clamp(unknown / 2, 5, 18));
            unknown = unconfirmed_pending();
        }
        int positive = count_if(pending.begin(), pending.end(), [&](const Pending& p) {
            return singleton[p.edge] != 0;
        });
        double total = observed_bridges + observed_nonbridges;
        double bridge_fraction = total > 0 ? observed_bridges / total : 0;
        double nonbridge_fraction = 1 - (root_bridges + bridge_fraction * unknown) / processed;
        return positive + unknown * (1 - bridge_fraction) >=
               max(5.0, minimum_collisions * nonbridge_fraction);
    }

    bool use_direct_search() const {
        int classes = int(layer[1].size());
        if (classes <= 1) return true;
        double duplicates = 0, direct_work = 0;
        double bridge = bridge_probability();
        double observed = observed_bridges + observed_nonbridges;
        if (observed < 4) bridge = 0;
        for (const auto& p : pending) {
            double weight = singleton[p.edge] ? 1 : 1 - bridge;
            duplicates += weight;
            direct_work += weight * log2(p.upper + !singleton[p.edge]);
        }
        if (classes <= 4 && duplicates >= classes * 2.5) return true;
        if (classes <= 9 && duplicates >= classes * 5.0) return true;
        return direct_work <= classes * 1.12;
    }

    void solve_directly() {
        // Prefix tests locate the first-layer representative. Index zero denotes a bridge.
        const auto& first = layer[1];
        int classes = int(first.size());
        Fenwick mass(classes), small_mass(classes), big_count(classes);
        for (int i = 1; i <= classes; ++i) {
            mass.add(i, 1);
            small_mass.add(i, 1);
        }
        vector<int> length(classes + 1, 1);
        vector<int> group(m + 1);
        for (int i = 1; i <= classes; ++i) group[first[i - 1].edge] = i;

        vector<int> big_reps;
        int big_parent = 0, loaded = 0;
        for (int i = 0; i < int(pending.size()); ++i) {
            const Pending& p = pending[i];
            int e = p.edge;
            bool known_nonbridge = singleton[e] != 0;
            int l = known_nonbridge ? 1 : 0, r = p.upper;
            const Fenwick* weights = &mass;
            int verified = int(big_count.sum(r));
            bool definitely_big = verified == r;
            if (!definitely_big && verified > 0 && r - verified >= 2 &&
                int(pending.size()) - i >= 10 &&
                small_mass.sum(r) < 0.43 * mass.sum(r)) {
                while (loaded < int(big_reps.size())) {
                    int rep = first[big_reps[loaded++] - 1].edge;
                    big_parent = big_parent == 0 ? test_singleton(rep) : ask(big_parent, rep);
                }
                definitely_big = !ask(big_parent, e);
                if (!definitely_big) {
                    weights = &small_mass;
                    l = 1;
                }
            }
            if (definitely_big) {
                bool bridge = !known_nonbridge && !test_singleton(e);
                if (!known_nonbridge) observe_bridge(bridge);
                answer[e] = bridge ? -1 : 0;
                continue;
            }

            double probability = bridge_probability();
            double bridge_weight = l == 0 ? weights->sum(r) * probability / (1 - probability) : 0;
            while (l < r) {
                double before = l <= 1 ? 0 : weights->sum(l - 1);
                double after = weights->sum(r);
                double extra = l == 0 ? bridge_weight : 0;
                double half = (after - before + extra) / 2;
                if (weights == &small_mass) {
                    l = weights->lower_bound(before + 0.25);
                    r = weights->lower_bound(after - 0.25);
                    if (l == r) break;
                }
                int cut = half <= extra ? 0 : weights->lower_bound(before + half - extra);
                cut = clamp(cut, l, r - 1);
                auto imbalance = [&](int k) {
                    return abs((k == 0 ? 0.0 : weights->sum(k)) - before + extra - half);
                };
                if (cut > l && imbalance(cut - 1) < imbalance(cut)) --cut;
                if (ask(cut ? first[cut - 1].query : 0, e)) l = cut + 1;
                else r = cut;
            }
            if (!known_nonbridge) observe_bridge(l == 0);
            if (l == 0) {
                answer[e] = -1;
            } else {
                group[e] = l;
                mass.add(l, 2.5);
                if (++length[l] == cap) {
                    big_reps.push_back(l);
                    big_count.add(l, 1);
                    small_mass.add(l, -(small_mass.sum(l) - small_mass.sum(l - 1)));
                } else if (length[l] < cap) {
                    small_mass.add(l, 2.5);
                }
            }
        }

        for (int edge = 1; edge <= m; ++edge) {
            if (group[edge]) answer[edge] = reported_size(length[group[edge]]);
        }
    }

    int place_in_layer(const Pending& p, bool allow_first, double novelty,
                       double bridge_probability, SearchTree& tree) {
        // Layer k stores the kth occurrence of every class with at least k occurrences.
        // Its tail accepts precisely the nonbridges whose next occurrence rank is at most k.
        // Equal layer populations describe equal sets of classes, so skip those thresholds.
        int edge = p.edge;
        bool known_nonbridge = singleton[edge] != 0;
        array<int, max_outcomes> outcomes{}, accepted{};
        array<int, overflow_rank + 1> population{};
        array<double, max_outcomes> weights{};
        int n = allow_first;
        int previous = p.upper;
        double nonbridge_weight = 0;
        for (int rank = 2; rank <= overflow_rank; ++rank) {
            int count = rank <= cap ? int(layer[rank].size()) : 0;
            int difference = previous - count;
            previous = count;
            if (difference == 0) continue;
            population[rank] = difference;
            double share = double(difference) / p.upper;
            double average_share = (exposure[rank] + 6.0 * share + 0.04) /
                                   (rank_events + 6.0);
            outcomes[n] = rank;
            weights[n] = (hits[rank] + 0.65 * sqrt(share) + 0.10) *
                         sqrt(share / average_share);
            nonbridge_weight += weights[n++];
            if (rank <= cap && count == 0 && known_nonbridge) {
                accepted[n - 1] = singleton[edge];
            }
        }

        if (allow_first) {
            outcomes[0] = 1;
            weights[0] = n == 1 ? 1.0 : nonbridge_weight * novelty / (1 - novelty);
            nonbridge_weight += weights[0];
        }

        if (!known_nonbridge) {
            outcomes[n] = bridge_rank;
            weights[n++] = nonbridge_weight * bridge_probability / (1 - bridge_probability);
        }

        tree.build(weights.data(), n);
        int l = 0, r = n - 1;
        while (l < r) {
            int k = tree.at(l, r);
            int rank = outcomes[k];
            int parent = rank <= cap ? tail_query(layer[rank]) : 0;
            int result = ask(parent, edge);
            if (result) {
                accepted[k] = result;
                r = k;
            } else {
                l = k + 1;
            }
        }
        int rank = outcomes[l];
        if (rank == bridge_rank) {
            answer[edge] = -1;
        } else if (rank <= cap) {
            assert(accepted[l]);
            layer[rank].push_back({edge, p.order, accepted[l]});
        }
        if (rank >= 2 && rank <= overflow_rank) {
            rank_events = 0.985 * rank_events + 1;
            for (int j = 2; j <= overflow_rank; ++j) {
                exposure[j] = 0.985 * exposure[j] + double(population[j]) / p.upper;
                hits[j] = 0.985 * hits[j] + (j == rank);
            }
        }
        return rank;
    }

    void build_other_layers() {
        SearchTree tree;
        for (const Pending& p : pending) {
            bool known_nonbridge = singleton[p.edge] != 0;
            int rank = place_in_layer(p, false, 0, bridge_probability(), tree);
            if (!known_nonbridge) observe_bridge(rank == bridge_rank);
        }
    }

    void continue_online(const vector<int>& order, int begin) {
        const auto& first = layer[1];
        double novelty = 0, bridges = 0, nonbridges = 0, total = 0;
        int representative = 0;
        for (int i = 0; i < begin; ++i) {
            bool is_new = representative < int(first.size()) && first[representative].order == i;
            representative += is_new;
            bool bridge = answer[order[i]] == -1;
            novelty = 0.985 * novelty + is_new;
            bridges = 0.985 * bridges + bridge;
            nonbridges = 0.985 * nonbridges + !bridge;
            total = 0.985 * total + 1;
        }

        SearchTree tree;
        for (int i = begin; i < m; ++i) {
            Pending e{order[i], i, int(first.size())};
            int rank = place_in_layer(e, true, (novelty + 0.4) / (nonbridges + 0.8),
                                       (bridges + 0.3) / (total + 0.7), tree);
            novelty = 0.985 * novelty + (rank == 1);
            bridges = 0.985 * bridges + (rank == bridge_rank);
            nonbridges = 0.985 * nonbridges + (rank != bridge_rank);
            total = 0.985 * total + 1;
        }
    }

    static double classification_cost(const array<int, cap + 1>& counts,
                                      const array<bool, cap + 1>& matched,
                                      SearchTree& tree) {
        array<double, max_outcomes> weights{};
        int n = 0;
        for (int t = 1; t <= cap; ++t) {
            if (counts[t] && !matched[t]) weights[n++] = counts[t];
        }
        return tree.build(weights.data(), n);
    }

    void classify_layer(int rank) {
        const auto& current = layer[rank];
        if (current.empty()) return;
        array<int, cap + 1> counts{}, match_cost{};
        array<vector<Entry>, cap + 1> witnesses;
        if (rank < cap) {
            for (const Entry& e : layer[rank + 1]) {
                int size = answer[e.edge];
                witnesses[size ? size : cap].push_back(e);
            }
        }
        for (int t = rank + 1; t <= cap; ++t) {
            counts[t] = int(witnesses[t].size());
            int upper = 0;
            for (const Entry& e : witnesses[t]) {
                while (upper < int(current.size()) && current[upper].order < e.order) ++upper;
                match_cost[t] += bit_width(unsigned(max(0, upper - 1)));
            }
        }
        counts[rank] = int(current.size()) - (rank < cap ? int(layer[rank + 1].size()) : 0);

        // A sparse length can be cheaper to locate via its known next-layer representatives.
        array<bool, cap + 1> matched{};
        SearchTree tree;
        double estimate = classification_cost(counts, matched, tree);
        while (estimate > 0) {
            int best = 0;
            double saving = 0;
            double remaining_cost = 0;
            for (int t = rank + 1; t <= cap; ++t) {
                if (!counts[t] || matched[t]) continue;
                matched[t] = true;
                double cost = classification_cost(counts, matched, tree);
                matched[t] = false;
                double gain = estimate - cost - 1.20 * match_cost[t];
                if (gain > saving) {
                    best = t;
                    saving = gain;
                    remaining_cost = cost;
                }
            }
            if (!best) break;
            matched[best] = true;
            estimate = remaining_cost;
        }

        vector<bool> located(current.size());
        vector<pair<int, int>> details;
        for (int t = rank + 1; t <= cap; ++t) {
            if (!matched[t]) continue;
            for (const Entry& e : witnesses[t]) details.emplace_back(e.order, e.edge);
        }
        sort(details.begin(), details.end());
        if (!details.empty()) {
            vector<int> available(current.size());
            iota(available.begin(), available.end(), 0);
            for (const auto& [order, edge] : details) {
                int l = 0, r = int(lower_bound(available.begin(), available.end(), order,
                    [&](int index, int time) { return current[index].order < time; }) -
                    available.begin()) - 1;
                assert(r >= 0);
                while (l < r) {
                    int k = (l + r) / 2;
                    if (ask(current[available[k]].query, edge)) l = k + 1;
                    else r = k;
                }
                int index = available[l];
                available.erase(available.begin() + l);
                located[index] = true;
                answer[current[index].edge] = answer[edge];
            }
        }

        array<int, cap + 1> eligible{}, used{};
        // An occurrence in the next layer must come later than its mate in this layer.
        // Scanning backward lets us rule out lengths with no remaining later witness.
        for (int i = int(current.size()) - 1; i >= 0; --i) {
            if (located[i]) continue;
            const Entry& e = current[i];
            array<int, max_outcomes> outcomes{};
            array<double, max_outcomes> weights{};
            int n = 0;
            for (int t = rank; t <= cap; ++t) {
                if (matched[t] || used[t] == counts[t]) continue;
                int available = counts[t] - used[t];
                if (t > rank) {
                    while (eligible[t] < counts[t] &&
                           witnesses[t][counts[t] - 1 - eligible[t]].order > e.order) {
                        ++eligible[t];
                    }
                    available = eligible[t] - used[t];
                }
                if (available > 0) {
                    outcomes[n] = t;
                    weights[n++] = available;
                }
            }
            assert(n > 0);
            tree.build(weights.data(), n);
            int l = 0, r = n - 1;
            while (l < r) {
                int k = tree.at(l, r);
                int threshold = outcomes[k + 1];
                if (ask(tail_query(layer[threshold]), e.edge)) r = k;
                else l = k + 1;
            }
            int size = outcomes[l];
            ++used[size];
            answer[e.edge] = reported_size(size);
        }
    }

    void classify_layers() {
        for (int rank = cap; rank >= 1; --rank) classify_layer(rank);
    }

    void finish_pending() {
        if (pending.empty()) {
            for (const Entry& e : layer[1]) answer[e.edge] = 1;
        } else if (use_direct_search()) {
            solve_directly();
        } else {
            build_other_layers();
            classify_layers();
        }
    }

    void merge_blocks(const vector<int>& order, int begin, int initial_big,
                      const vector<int>& anchors, const vector<Entry>& block) {
        int big = initial_big;
        for (int edge : anchors) {
            int result = ask(big, edge);
            if (result) big = result;
        }
        for (auto& entries : layer) entries.clear();
        pending.clear();
        exposure.fill(0);
        hits.fill(0);
        rank_events = 0;

        auto add_active = [&](int edge, int time) {
            auto& first = layer[1];
            int result = first.empty() ? singleton[edge] : ask(tail_query(first), edge);
            if (result) first.push_back({edge, time, result});
            else pending.push_back({edge, time, int(first.size())});
        };

        if (!block.empty() && ask(big, block.front().edge)) {
            for (const Entry& e : block) add_active(e.edge, e.order);
        }
        for (int i = begin; i < m; ++i) {
            int edge = order[i];
            int result = ask(big, edge);
            if (result) {
                // All active edges avoid the mature classes deleted in this query.
                singleton[edge] = result;
                add_active(edge, i);
            } else if (!test_singleton(edge)) {
                answer[edge] = -1;
            }
        }
        finish_pending();
    }

    void finish_dominant_cycle(const vector<int>& order, int begin) {
        // A block of 15 confirmed equal edges needs no comparison with earlier full blocks.
        int initial_big = layer[1].front().query;
        vector<int> anchors;
        vector<Entry> block;
        for (int i = begin; i < m; ++i) {
            int edge = order[i];
            int single = test_singleton(edge);
            if (!single) {
                answer[edge] = -1;
                continue;
            }
            if (block.empty() && m - i >= cap) {
                block.push_back({edge, i, single});
                continue;
            }

            int anchor = block.empty() ? initial_big : block.front().query;
            if (ask(anchor, edge)) {
                merge_blocks(order, i, initial_big, anchors, block);
                return;
            }
            if (!block.empty()) {
                block.push_back({edge, i, singleton[edge]});
                if (int(block.size()) == cap) {
                    anchors.push_back(block.front().edge);
                    block.clear();
                }
            }
        }
        // A full block certifies itself. Only an unresolved block can need merging.
        if (!block.empty() && ask(initial_big, block.front().edge)) {
            merge_blocks(order, m, initial_big, anchors, block);
        }
    }

public:
    explicit Solver(int edges) : m(edges), answer(edges + 1), singleton(edges + 1) {}

    void run() {
        vector<int> order(m);
        iota(order.begin(), order.end(), 1);
        mt19937 random(0x7172643u + unsigned(m));
        shuffle(order.begin(), order.end(), random);
        int planned = m < 96 ? m : min(m / 3, int(7 * sqrt(m)));
        int prefix = min(planned, 48);
        build_first_layer(order, 0, prefix);
        if (prefix < planned && (layer[1].size() != 1 || root_first)) {
            build_first_layer(order, prefix, planned);
            prefix = planned;
        }
        bool online = prefer_online(prefix);
        if (!online) build_first_layer(order, prefix, m);
        erase_if(pending, [&](const Pending& p) { return answer[p.edge] == -1; });

        if (online) {
            build_other_layers();
            if (layer[1].size() == 1 && layer[cap].size() == 1) {
                finish_dominant_cycle(order, prefix);
            } else {
                continue_online(order, prefix);
                classify_layers();
            }
        } else {
            finish_pending();
        }

        cout << '!';
        for (int i = 1; i <= m; ++i) cout << ' ' << answer[i];
        cout << '\n' << flush;
        int verdict;
        if (!(cin >> verdict) || verdict != 1) return;
    }
};

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int m;
    if (!(cin >> m) || m <= 0) return 0;
    Solver(m).run();
}

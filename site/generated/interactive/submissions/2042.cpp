#include <algorithm>
#include <array>
#include <bit>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

using namespace std;

using Query = vector<pair<int, int>>;  // (vertex, multiplicity)

struct Judge {
    int ask(const Query& query) const {
        int length = 0;
        for (auto [vertex, weight] : query) length += weight;
        cout << "? 1 " << length;
        for (auto [vertex, weight] : query) {
            for (int i = 0; i < weight; ++i) cout << ' ' << vertex;
        }
        cout << endl;
        int reply;
        if (!(cin >> reply) || reply < -3 * length || reply > 3 * length) exit(0);
        return reply;
    }

    void flip(int vertex) const {
        cout << "? 2 " << vertex << endl;
    }

    void answer(const vector<int>& values) const {
        cout << '!';
        for (size_t i = 1; i < values.size(); ++i) cout << ' ' << values[i];
        cout << endl;
    }
};

int query_weight(const Query& query) {
    int total = 0;
    for (auto [vertex, weight] : query) total += weight;
    return total;
}

struct State {
    vector<int> free;
    vector<int> pending;
    array<vector<int>, 2> alternatives;
    int positives;

    int representative() const {
        for (int i = 0; i < int(pending.size()); ++i) {
            if (alternatives[0][i] != alternatives[1][i]) return i;
        }
        return -1;
    }
};

struct Assignment {
    uint64_t bits;
    int alternative;
};

class Decoder {
    const Judge& judge;
    int n;
    int path_center;
    int total_positives;
    int binary_digits;
    bool center_unknown;
    vector<int>& result;

    void set_value(int id, int value, State& state) {
        result[id] = value;
        state.positives -= (value == 1);
    }

    static int positives_in(const vector<int>& signs) {
        return count(signs.begin(), signs.end(), 1);
    }

    int free_positives(const State& state, int alternative) const {
        return state.positives - positives_in(state.alternatives[alternative]);
    }

    // Keep only feasible alternatives, and commit their shared coordinates.
    void retain(State& state, const vector<int>& ids, const vector<vector<int>>& choices) {
        vector<int> feasible;
        for (int i = 0; i < int(choices.size()); ++i) {
            int left = state.positives - positives_in(choices[i]);
            if (0 <= left && left <= int(state.free.size())) feasible.push_back(i);
        }
        if (feasible.empty()) exit(0);
        state.pending.clear();
        for (auto& alternative : state.alternatives) alternative.clear();
        for (int j = 0; j < int(ids.size()); ++j) {
            int value = choices[feasible[0]][j];
            if (feasible.size() == 1 || value == choices[feasible[1]][j]) {
                set_value(ids[j], value, state);
            } else {
                state.pending.push_back(ids[j]);
                state.alternatives[0].push_back(value);
                state.alternatives[1].push_back(choices[feasible[1]][j]);
            }
        }
    }

    int read_score(const Query& query, int center) const {
        int weight = query_weight(query);
        int shifted = judge.ask(query) - center * weight + weight;
        if (shifted < 0 || shifted > 2 * weight || shifted % 2) exit(0);
        return shifted / 2;
    }

    // The last weight extends the range when it cannot provide a whole new digit.
    // In the overlap, the rest of the batch has only two possible assignments.
    void read_binary_batch(State& state) {
        int old_digit = !state.pending.empty();
        int digits = min(binary_digits, int(state.free.size()) + old_digit);
        int base = (1 << digits) - 1;
        int extra = 0;
        if (int(state.free.size()) + old_digit > digits) extra = n - base;

        Query query;
        vector<int> ids = state.pending;
        int representative = state.representative();
        if (old_digit) query.emplace_back(state.pending[representative], 1);
        for (int j = old_digit; j < digits + (extra > 0); ++j) {
            int id = state.free.back();
            state.free.pop_back();
            ids.push_back(id);
            query.emplace_back(id, j == digits ? extra : 1 << j);
        }

        int score = read_score(query, path_center);
        vector<vector<int>> choices;
        for (int high = 0; high <= (extra > 0); ++high) {
            int low = score - high * extra;
            if (low < 0 || low > base) continue;
            vector<int> values;
            if (old_digit) {
                int bit_value = (low & 1) ? 1 : -1;
                int option = state.alternatives[0][representative] != bit_value;
                values = state.alternatives[option];
            }
            for (int j = old_digit; j < digits; ++j) {
                values.push_back((low >> j & 1) ? 1 : -1);
            }
            if (extra > 0) values.push_back(high ? 1 : -1);
            choices.push_back(move(values));
        }
        retain(state, ids, choices);
    }

    vector<Assignment> list_assignments(const State& state) const {
        vector<Assignment> assignments;
        int size = state.free.size();
        if (size > 62) return assignments;
        for (int option = 0; option <= !state.pending.empty(); ++option) {
            int count = free_positives(state, option);
            if (count < 0 || count > size) continue;
            int minority = min(count, size - count);
            uint64_t end = uint64_t{1} << size;
            uint64_t mask = (uint64_t{1} << minority) - 1;
            while (mask < end) {
                uint64_t bits = count == minority ? mask : (end - 1) ^ mask;
                assignments.push_back({bits, option});
                if (int(assignments.size()) > n + 1) return {};
                if (!mask) break;
                uint64_t ripple = mask & -mask;
                uint64_t next = mask + ripple;
                mask = next | (((mask ^ next) >> 2) / ripple);
            }
        }
        return assignments;
    }

    // Build distinct subset sums separately for each relevant cardinality.
    vector<int> subset_weights(int size, int max_cardinality, int budget) const {
        if (max_cardinality <= 1) {
            if (size * (size - 1) / 2 > budget) return {};
            vector<int> weights(size);
            iota(weights.begin(), weights.end(), 0);
            return weights;
        }
        vector<bitset<1001>> sums(max_cardinality + 1);
        sums[0].set(0);
        vector<int> weights;
        int weight = 0;
        for (int i = 0; i < size; ++i) {
            int limit = min(i + 1, max_cardinality);
            while (weight <= budget) {
                bool clash = false;
                for (int k = 1; k <= limit; ++k) {
                    if (((sums[k - 1] << weight) & sums[k]).any()) {
                        clash = true;
                        break;
                    }
                }
                if (!clash) break;
                ++weight;
            }
            if (weight > budget) return {};
            weights.push_back(weight);
            for (int k = limit; k >= 1; --k) sums[k] |= sums[k - 1] << weight;
            budget -= weight++;
        }
        return weights;
    }

    vector<int> distinguishing_weights(const State& state) const {
        int size = state.free.size();
        int has_pending = !state.pending.empty();
        int radix = 1 + has_pending;
        int representative = state.representative();
        int max_cardinality = 0;
        for (int option = 0; option <= has_pending; ++option) {
            int count = free_positives(state, option);
            max_cardinality = max(max_cardinality, min(count, size - count));
        }

        vector<int> weights = subset_weights(size, max_cardinality, (n - has_pending) / radix);
        if (int(weights.size()) != size) return {};
        for (int& weight : weights) weight *= radix;
        weights.resize(size + state.pending.size());
        if (has_pending) weights[size + representative] = 1;
        return weights;
    }

    static int sign_at(const State& state, const Assignment& assignment, int position) {
        int size = state.free.size();
        if (position < size) return (assignment.bits >> position & 1) ? 1 : -1;
        return state.alternatives[assignment.alternative][position - size];
    }

    Query make_query(const State& state, const vector<int>& weights) const {
        Query query;
        int size = state.free.size();
        for (int i = 0; i < int(weights.size()); ++i) {
            if (!weights[i]) continue;
            int id = i < size ? state.free[i] : state.pending[i - size];
            query.emplace_back(id, weights[i]);
        }
        return query;
    }

    static vector<pair<int, int>> nonzero_weights(const vector<int>& weights) {
        vector<pair<int, int>> terms;
        for (int i = 0; i < int(weights.size()); ++i) {
            if (weights[i]) terms.emplace_back(i, weights[i]);
        }
        return terms;
    }

    int score_of(const State& state, const Assignment& assignment,
                 const vector<pair<int, int>>& terms) const {
        int score = 0;
        for (auto [i, weight] : terms) {
            if (sign_at(state, assignment, i) == 1) score += weight;
        }
        return score;
    }

    void write_assignment(const State& state, const Assignment& assignment) {
        for (int i = 0; i < int(state.free.size()); ++i) {
            result[state.free[i]] = sign_at(state, assignment, i);
        }
        for (int i = 0; i < int(state.pending.size()); ++i) {
            result[state.pending[i]] = state.alternatives[assignment.alternative][i];
        }
    }

    struct FlipPlan {
        vector<int> weights;
        array<vector<int>, 2> assignment_by_score;
        int ambiguous = 0;
    };

    FlipPlan evaluate_flip(const State& state, const vector<Assignment>& assignments,
                           const vector<int>& weights) const {
        int weight_sum = accumulate(weights.begin(), weights.end(), 0);
        FlipPlan plan;
        plan.weights = weights;
        for (auto& lookup : plan.assignment_by_score) lookup.assign(weight_sum + 1, -1);
        array<vector<int>, 2> frequencies;
        for (auto& frequency : frequencies) frequency.assign(weight_sum + 1, 0);
        auto terms = nonzero_weights(weights);

        auto add = [&](int sign, int score, int index, int ways) {
            if (ways == 0) return;
            int& entry = plan.assignment_by_score[sign][score];
            if (entry == -1) entry = index;
            else if (entry != index) entry = -2;
            frequencies[sign][score] += ways;
        };

        for (int i = 0; i < int(assignments.size()); ++i) {
            const auto& assignment = assignments[i];
            int score = score_of(state, assignment, terms);
            array<int, 2> outside = {total_positives, n - 1 - total_positives};
            for (auto [position, weight] : terms) {
                // Sign 0 means the old center was negative and the root positive.
                int sign = sign_at(state, assignment, position) == -1;
                --outside[sign];
                add(sign, score + (sign ? weight : -weight), i, 1);
            }
            for (int sign = 0; sign < 2; ++sign) add(sign, score, i, outside[sign]);
        }
        for (int sign = 0; sign < 2; ++sign) {
            for (int score = 0; score <= weight_sum; ++score) {
                if (plan.assignment_by_score[sign][score] == -2) {
                    plan.ambiguous += frequencies[sign][score];
                }
            }
        }
        return plan;
    }

    vector<int> full_binary_weights(const State& state, const vector<int>& representatives,
                                    bool first_extra) const {
        int size = state.free.size();
        int digits = size + int(representatives.size());
        int extra = n - ((1 << binary_digits) - 1);
        if (digits > binary_digits + (extra > 0)) return {};
        vector<int> order = representatives;
        for (int i = 0; i < size; ++i) order.push_back(i);
        if (first_extra && !representatives.empty()) rotate(order.begin(), order.begin() + 1, order.end());
        vector<int> weights(size + state.pending.size());
        for (int i = 0; i < digits; ++i) {
            weights[order[i]] = i == binary_digits ? extra : 1 << i;
        }
        return weights;
    }

    bool try_finish(const State& state, bool last_group) {
        if (state.free.size() > 62) return false;
        vector<int> pure = distinguishing_weights(state);
        if (pure.empty()) return false;
        vector<Assignment> assignments = list_assignments(state);
        if (assignments.empty()) return false;
        if (assignments.size() == 1) {
            write_assignment(state, assignments[0]);
            return true;
        }

        if (center_unknown && last_group) {
            // Leave the center flip until the final batch. If the root is
            // outside the measured coordinates, their residuals are unchanged.
            FlipPlan best = evaluate_flip(state, assignments, pure);
            auto consider = [&](const vector<int>& weights) {
                if (weights.empty() || !best.ambiguous) return;
                FlipPlan candidate = evaluate_flip(state, assignments, weights);
                if (candidate.ambiguous < best.ambiguous) best = move(candidate);
            };
            vector<int> representatives;
            int size = state.free.size();
            int representative = state.representative();
            if (representative >= 0) representatives.push_back(size + representative);
            consider(full_binary_weights(state, representatives, false));
            if (representative >= 0) {
                consider(full_binary_weights(state, representatives, true));
                for (int i = 0; i < int(state.pending.size()); ++i) {
                    if (i == representative) continue;
                    representatives.push_back(size + i);
                    consider(full_binary_weights(state, representatives, false));
                    break;
                }
            } else {
                vector<int> odd = pure;
                for (int& weight : odd) weight = 2 * weight + 1;
                if (accumulate(odd.begin(), odd.end(), 0) <= n) consider(odd);
                int max_offset = (n - accumulate(pure.begin(), pure.end(), 0)) / size;
                for (int offset : {1, max_offset / 2, max_offset}) {
                    if (offset < 1 || offset > max_offset) continue;
                    vector<int> shifted = pure;
                    for (int& weight : shifted) weight += offset;
                    consider(shifted);
                }
            }

            if (3LL * best.ambiguous <= int64_t(assignments.size()) * (n - 1)) {
                Query query = make_query(state, best.weights);
                int weight = query_weight(query);
                judge.flip(1);
                int reply = judge.ask(query);
                int sign = reply < 0;
                int new_center = sign ? -1 : 1;
                int shifted = reply - 2 * new_center * weight + weight;
                if (shifted < 0 || shifted > 2 * weight || shifted % 2) exit(0);
                int index = best.assignment_by_score[sign][shifted / 2];
                if (index == -1) exit(0);
                center_unknown = false;
                if (index >= 0) {
                    result[1] = new_center;
                    write_assignment(state, assignments[index]);
                    return true;
                }

                judge.flip(1);
                result[1] = -new_center;
            }
        }

        int score = read_score(make_query(state, pure), path_center);
        auto terms = nonzero_weights(pure);
        for (const Assignment& assignment : assignments) {
            if (score_of(state, assignment, terms) == score) {
                write_assignment(state, assignment);
                return true;
            }
        }
        exit(0);
    }

    void split_single(State state, bool last_group) {
        // With at most c candidates per reply, fill the cheapest weights first.
        // This gives the maximum number that can be separated in one more query.
        int size = state.free.size();
        int capacity = 1;
        int previous_capacity = 1;
        while (capacity < size) {
            previous_capacity = capacity;
            capacity = 0;
            int budget = n;
            for (int weight = 0; ; ++weight) {
                int take = weight ? min(previous_capacity, budget / weight) : previous_capacity;
                if (!take) break;
                capacity += take;
                budget -= take * weight;
            }
        }

        vector<int> buckets;
        // Spend any spare weight on singleton replies to finish those cases early.
        for (int singles = 0; singles <= size; ++singles) {
            vector<int> candidate;
            for (int left = size - singles; left > 0; left -= previous_capacity) {
                candidate.push_back(min(left, previous_capacity));
            }
            candidate.resize(candidate.size() + singles, 1);
            int cost = 0;
            for (int i = 1; i < int(candidate.size()); ++i) cost += i * candidate[i];
            if (cost > n) break;
            buckets = move(candidate);
        }

        Query query;
        int offset = buckets[0];
        for (int i = 1; i < int(buckets.size()); ++i) {
            for (int j = 0; j < buckets[i]; ++j) {
                query.emplace_back(state.free[offset++], i);
            }
        }
        int score = read_score(query, path_center);
        int sign = state.positives == 1 ? 1 : -1;
        int bucket = sign == 1 ? score : query_weight(query) - score;
        int start = accumulate(buckets.begin(), buckets.begin() + bucket, 0);
        vector<int> remaining;
        for (int i = 0; i < size; ++i) {
            if (start <= i && i < start + buckets[bucket]) remaining.push_back(state.free[i]);
            else result[state.free[i]] = -sign;
        }
        int positives = sign == 1 ? 1 : int(remaining.size()) - 1;
        decode({move(remaining), {}, {}, positives}, last_group);
    }

    void split_sparse(State state, bool last_group) {
        int size = state.free.size();
        int minority = min(state.positives, size - state.positives);
        bool positive = state.positives == minority;
        int radix = minority + 1;
        vector<int> weights = {0, 1};
        while (weights.back() <= n / radix && int(weights.size()) < size) {
            weights.push_back(weights.back() * radix);
            int groups = weights.size();
            int cost = size / groups * accumulate(weights.begin(), weights.end(), 0)
                + accumulate(weights.begin(), weights.begin() + size % groups, 0);
            if (cost > n) {
                weights.pop_back();
                break;
            }
        }

        int groups = weights.size();
        vector<vector<int>> parts(groups);
        Query query;
        int offset = 0;
        for (int i = 0; i < groups; ++i) {
            int length = size / groups + (i < size % groups);
            parts[i].assign(state.free.begin() + offset, state.free.begin() + offset + length);
            offset += length;
            if (i > 0) {
                for (int id : parts[i]) query.emplace_back(id, weights[i]);
            }
        }

        int score = read_score(query, path_center);
        if (!positive) score = query_weight(query) - score;
        vector<int> count(groups);
        count[0] = minority;
        for (int i = 1; i < groups; ++i) {
            count[i] = score % radix;
            score /= radix;
            count[0] -= count[i];
        }

        int last_mixed = -1;
        for (int i = 0; i < groups; ++i) {
            if (0 < count[i] && count[i] < int(parts[i].size())) last_mixed = i;
        }
        for (int i = 0; i < groups; ++i) {
            int positives = positive ? count[i] : int(parts[i].size()) - count[i];
            decode({move(parts[i]), {}, {}, positives}, last_group && i == last_mixed);
        }
    }

    struct PackingItem {
        int weight;
        int radix;
    };

    vector<vector<int>> pack_jobs(const vector<PackingItem>& jobs) const {
        vector<int> order(jobs.size());
        iota(order.begin(), order.end(), 0);
        stable_sort(order.begin(), order.end(), [&](int i, int j) {
            return jobs[i].weight > jobs[j].weight;
        });

        // Sort the digits within each query to minimize its total multiplicity.
        auto less_radix_per_weight = [&](int i, int j) {
            return (jobs[i].radix - 1) * jobs[j].weight < (jobs[j].radix - 1) * jobs[i].weight;
        };
        vector<vector<int>> batches;
        vector<bool> used(jobs.size());
        for (int i : order) {
            if (used[i]) continue;
            vector<int> batch = {i};
            used[i] = true;
            for (int j : order) {
                if (used[j]) continue;
                vector<int> candidate = batch;
                candidate.insert(lower_bound(candidate.begin(), candidate.end(),
                                             j, less_radix_per_weight), j);
                int cost = 0;
                int weight = 1;
                for (int k : candidate) {
                    cost += weight * jobs[k].weight;
                    if (cost > n) break;
                    weight *= jobs[k].radix;
                }
                if (cost > n) continue;
                used[j] = true;
                batch = move(candidate);
            }
            batches.push_back(move(batch));
        }
        return batches;
    }

    double estimate_dense(int size, int minority) const {
        int power = 1 << binary_digits;
        int extra = n - (power - 1);
        double throughput = binary_digits;
        if (extra > 0) {
            double p = double(minority) / size;
            vector<double> probability(binary_digits + 1);
            for (int bits = 0; bits <= binary_digits; ++bits) {
                probability[bits] = pow(p, bits) * pow(1 - p, binary_digits - bits);
            }
            double overlap = 0;
            for (int mask = 0; mask < power; ++mask) {
                int count = popcount(unsigned(mask));
                if (mask >= extra) overlap += (1 - p) * probability[count];
                if (mask < power - extra) overlap += p * probability[count];
            }
            throughput += 1 - overlap;
        }
        return (size - 1) / throughput + 0.5;
    }

    static vector<double> block_distribution(int size, int minority, int block_size) {
        vector<double> probability(block_size + 1);
        probability[0] = 1;
        for (int i = 0; i < block_size; ++i) {
            probability[0] *= double(size - minority - i) / (size - i);
        }
        for (int k = 1; k <= min(block_size, minority); ++k) {
            probability[k] = probability[k - 1] * (minority - k + 1) * (block_size - k + 1)
                / (k * double(size - minority - block_size + k));
        }
        return probability;
    }

    pair<int, int> choose_screen(int size, int minority) const {
        if (n < 180 || 55 * minority < size || 4 * minority > size) return {0, 0};
        double dense_cost = estimate_dense(size, minority);
        double sparse_cost = minority * (0.55 + 0.22 * log2(double(size) / minority)) + 2;
        double best = min(dense_cost, sparse_cost) - 0.5;
        pair<int, int> choice = {0, 0};

        for (int block_size = 4; block_size <= 14; ++block_size) {
            int groups = (size + block_size - 1) / block_size;
            int small_size = size / groups;
            int larger = size % groups;
            auto small = block_distribution(size, minority, small_size);
            auto big = block_distribution(size, minority, small_size + (larger > 0));
            vector<PackingItem> jobs;
            double fallback_cost = 0;

            for (int length : {small_size, small_size + 1}) {
                int quantity = length == small_size ? groups - larger : larger;
                if (!quantity) continue;
                const auto& probability = length == small_size ? small : big;
                for (int count = 1; count <= length / 2; ++count) {
                    double frequency = quantity * probability[count];
                    if (count * 2 != length) frequency += quantity * probability[length - count];
                    vector<int> weights = subset_weights(length, count, n);
                    if (weights.empty()) {
                        fallback_cost += 2.2 * frequency;
                        continue;
                    }
                    int low = accumulate(weights.begin(), weights.begin() + count, 0);
                    int high = accumulate(weights.end() - count, weights.end(), 0);
                    PackingItem item{accumulate(weights.begin(), weights.end(), 0), high - low + 1};
                    jobs.insert(jobs.end(), int(frequency + 0.5), item);
                }
            }

            double job_cost = pack_jobs(jobs).size() + fallback_cost;
            for (int radix = 3; radix <= 5; ++radix) {
                auto carry_probability = [&](const vector<double>& probability) {
                    double expected = 0;
                    for (int i = radix; i < int(probability.size()); ++i) {
                        expected += (i / radix) * probability[i];
                    }
                    return expected;
                };
                double small_carry = carry_probability(small);
                double big_carry = carry_probability(big);
                double carries = 0;
                int batches = 0;
                for (int i = 0; i < groups; ) {
                    int weight = 1;
                    int cost = 0;
                    double last_carry = 0;
                    for (; i < groups; ++i) {
                        int length = small_size + (i < larger);
                        if (cost + length * weight > n) break;
                        cost += length * weight;
                        weight *= radix;
                        carries += last_carry;
                        last_carry = i < larger ? big_carry : small_carry;
                    }
                    ++batches;
                }
                double cost = batches + job_cost
                    + carries * (2 + log2(max(1.0, batches / (carries + 1))));
                if (cost >= best) continue;
                best = cost;
                choice = {block_size, radix};
            }
        }
        return choice;
    }

    vector<int> compressed_counts(const vector<State>& blocks, int minority, bool positive, int radix) {
        struct Counts {
            vector<int> digits;
            int total;
        };
        struct Batch {
            int first;
            int after;
            int leaf_count;
            int minimum;
            int maximum;
            vector<Counts> possibilities;
        };

        // Small radices fit more block counts into each reply. Keep every
        // compatible digit vector, then use the known total to resolve carries.
        vector<Batch> batches;
        for (int first = 0, group_count = blocks.size(); first < group_count; ) {
            Query query;
            int after = first;
            int weight = 1;
            int cost = 0;
            int leaf_count = 0;
            for (; after < group_count; ++after) {
                int size = blocks[after].free.size();
                if (cost + size * weight > n) break;
                for (int id : blocks[after].free) query.emplace_back(id, weight);
                cost += size * weight;
                leaf_count += size;
                weight *= radix;
            }
            int score = read_score(query, path_center);
            if (!positive) score = cost - score;
            Batch batch{first, after, leaf_count, minority + 1, -1, {}};
            vector<int> counts(after - first);
            auto enumerate = [&](auto&& self, int i, int remaining, int total) -> void {
                int limit = min(remaining, int(blocks[i].free.size()));
                if (i + 1 == after) {
                    if (remaining > limit || total + remaining > minority) return;
                    counts[i - first] = remaining;
                    total += remaining;
                    batch.minimum = min(batch.minimum, total);
                    batch.maximum = max(batch.maximum, total);
                    batch.possibilities.push_back({counts, total});
                    return;
                }
                for (int here = remaining % radix; here <= limit; here += radix) {
                    if (total + here > minority) break;
                    counts[i - first] = here;
                    self(self, i + 1, (remaining - here) / radix, total + here);
                }
            };
            enumerate(enumerate, first, score, 0);
            if (batch.possibilities.empty()) exit(0);
            batches.push_back(move(batch));
            first = after;
        }

        vector<int> totals(batches.size());
        vector<int> unresolved;
        for (int i = 0; i < int(batches.size()); ++i) {
            const auto& batch = batches[i];
            totals[i] = batch.minimum;
            minority -= batch.minimum;
            if (batch.minimum != batch.maximum) unresolved.push_back(i);
        }

        int carry_cost = radix - 1;
        if (minority < 0 || minority % carry_cost) exit(0);
        auto locate = [&](auto&& self, int first, int after, int extra) -> void {
            if (!extra || first == after) return;
            if (first + 1 == after) {
                totals[unresolved[first]] += extra * carry_cost;
                return;
            }
            int maximum = 0;
            for (int i = first; i < after; ++i) {
                const auto& batch = batches[unresolved[i]];
                maximum += batch.maximum - batch.minimum;
            }
            if (extra * carry_cost == maximum) {
                for (int i = first; i < after; ++i) {
                    int id = unresolved[i];
                    totals[id] = batches[id].maximum;
                }
                return;
            }

            int size = after - first;
            int range = extra + 1;
            int bins = 2;
            auto capacity = [&](int trial) {
                int cost = 0;
                int i = first;
                int weight = 0;
                for (int j = 0; j < trial; ++j) {
                    int end = i + size / trial + (j < size % trial);
                    for (; i < end; ++i) cost += weight * batches[unresolved[i]].leaf_count;
                    if (cost > n) return false;
                    weight = weight ? weight * range : 1;
                    if (weight > n && j + 1 < trial) return false;
                }
                return true;
            };
            while (bins < size && capacity(bins + 1)) ++bins;
            Query query;
            int baseline = 0;
            int cost = 0;
            vector<int> boundaries = {first};
            int weight = 0;
            for (int j = 0; j < bins; ++j) {
                int end = boundaries.back() + size / bins + (j < size % bins);
                for (int i = boundaries.back(); weight && i < end; ++i) {
                    const auto& batch = batches[unresolved[i]];
                    baseline += weight * batch.minimum;
                    cost += weight * batch.leaf_count;
                    for (int block = batch.first; block < batch.after; ++block) {
                        for (int id : blocks[block].free) query.emplace_back(id, weight);
                    }
                }
                boundaries.push_back(end);
                weight = weight ? weight * range : 1;
            }
            int score = read_score(query, path_center);
            if (!positive) score = cost - score;
            if (score < baseline || (score - baseline) % carry_cost) exit(0);
            score = (score - baseline) / carry_cost;
            vector<int> digits(bins);
            digits[0] = extra;
            for (int j = 1; j < bins; ++j) {
                digits[j] = score % range;
                score /= range;
                digits[0] -= digits[j];
            }
            if (score || digits[0] < 0) exit(0);
            for (int j = 0; j < bins; ++j) self(self, boundaries[j], boundaries[j + 1], digits[j]);
        };
        locate(locate, 0, unresolved.size(), minority / carry_cost);

        vector<int> counts(blocks.size());
        for (int i = 0; i < int(batches.size()); ++i) {
            auto& batch = batches[i];
            auto& options = batch.possibilities;
            erase_if(options, [&](const Counts& x) { return x.total != totals[i]; });
            while (options.size() > 1) {
                int best = -1;
                int largest_bucket = options.size();
                for (int j = 0; j < batch.after - batch.first; ++j) {
                    vector<int> counts_by_reply(blocks[batch.first + j].free.size() + 1);
                    int largest = 0;
                    for (const auto& option : options) {
                        largest = max(largest, ++counts_by_reply[option.digits[j]]);
                    }
                    if (largest < largest_bucket) {
                        largest_bucket = largest;
                        best = j;
                    }
                }
                Query query;
                for (int id : blocks[batch.first + best].free) query.emplace_back(id, 1);
                int answer = read_score(query, path_center);
                if (!positive) answer = query.size() - answer;
                erase_if(options, [&](const Counts& x) { return x.digits[best] != answer; });
            }
            if (options.empty()) exit(0);
            copy(options[0].digits.begin(), options[0].digits.end(), counts.begin() + batch.first);
        }
        return counts;
    }

    void screen_sparse(State state, int block_size, int radix) {
        int length = state.free.size();
        int minority = min(state.positives, length - state.positives);
        bool positive = state.positives == minority;
        int groups = (length + block_size - 1) / block_size;
        vector<State> blocks(groups);
        for (int i = 0, offset = 0; i < groups; ++i) {
            int size = length / groups + (i < length % groups);
            blocks[i].free.assign(state.free.begin() + offset, state.free.begin() + offset + size);
            offset += size;
        }

        vector<int> counts = compressed_counts(blocks, minority, positive, radix);
        for (int i = 0; i < groups; ++i) blocks[i].positives = counts[i];
        int last_mixed = -1;
        for (int i = 0; i < groups; ++i) {
            auto& block = blocks[i];
            int size = block.free.size();
            if (!positive) block.positives = size - block.positives;
            if (block.positives == 0 || block.positives == size) {
                for (int id : block.free) result[id] = block.positives ? 1 : -1;
                block.free.clear();
            } else {
                last_mixed = i;
            }
        }

        struct Job {
            int block;
            int low;
            int radix;
            vector<int> weights;
        };
        vector<Job> jobs;
        vector<PackingItem> packing;
        for (int i = 0; i < groups; ++i) {
            const auto& block = blocks[i];
            int size = block.free.size();
            if (!size || (center_unknown && i == last_mixed)) continue;
            vector<int> weights = distinguishing_weights(block);
            if (weights.empty()) {
                decode(move(blocks[i]), false);
                continue;
            }
            int low = accumulate(weights.begin(), weights.begin() + block.positives, 0);
            int high = accumulate(weights.end() - block.positives, weights.end(), 0);
            int weight = accumulate(weights.begin(), weights.end(), 0);
            packing.push_back({weight, high - low + 1});
            jobs.push_back({i, low, high - low + 1, move(weights)});
        }
        for (const auto& batch : pack_jobs(packing)) {
            Query query;
            int weight = 1;
            int offset = 0;
            for (int j : batch) {
                for (auto [id, w] : make_query(blocks[jobs[j].block], jobs[j].weights)) {
                    query.emplace_back(id, w * weight);
                }
                offset += jobs[j].low * weight;
                weight *= jobs[j].radix;
            }
            int score = read_score(query, path_center) - offset;
            for (int j : batch) {
                const auto& job = jobs[j];
                const auto& block = blocks[job.block];
                int digit = score % job.radix + job.low;
                score /= job.radix;
                auto terms = nonzero_weights(job.weights);
                auto assignments = list_assignments(block);
                for (const auto& assignment : assignments) {
                    if (score_of(block, assignment, terms) != digit) continue;
                    write_assignment(block, assignment);
                    break;
                }
            }
        }
        if (center_unknown && last_mixed >= 0) {
            decode(move(blocks[last_mixed]), true);
        }
    }

    void decode(State state, bool last_group) {
        while (!state.free.empty() || !state.pending.empty()) {
            int size = state.free.size();
            if (state.pending.empty() && (state.positives == 0 || state.positives == size)) {
                int value = state.positives ? 1 : -1;
                for (int id : state.free) result[id] = value;
                return;
            }
            if (!state.pending.empty() && size > 0) {
                int count = free_positives(state, 0);
                if ((count == 0 || count == size) && count == free_positives(state, 1)) {
                    for (int id : state.free) set_value(id, count ? 1 : -1, state);
                    state.free.clear();
                    size = 0;
                }
            }
            if (try_finish(state, last_group)) return;

            if (state.pending.empty()) {
                int minority = min(state.positives, size - state.positives);
                if (minority == 1) {
                    split_single(move(state), last_group);
                    return;
                }
                if (minority > 1 && (size >= 12 * minority || (minority <= 3 && size >= 64))) {
                    split_sparse(move(state), last_group);
                    return;
                }
            }
            read_binary_batch(state);
        }
    }

public:
    Decoder(const Judge& connection, int vertices, int offset, int positives,
            bool learn_center, vector<int>& values)
        : judge(connection), n(vertices), path_center(offset), total_positives(positives),
          binary_digits(bit_width(unsigned(vertices + 1)) - 1), center_unknown(learn_center),
          result(values) {}

    void run() {
        vector<int> leaves(n - 1);
        iota(leaves.begin(), leaves.end(), 2);
        int minority = min(total_positives, n - 1 - total_positives);
        auto [block_size, radix] = choose_screen(n - 1, minority);
        State state{move(leaves), {}, {}, total_positives};
        if (block_size) screen_sparse(move(state), block_size, radix);
        else decode(move(state), true);
        if (center_unknown) {
            judge.flip(1);
            int reply = judge.ask({{1, 1}});
            if (abs(reply) != 2) exit(0);
            result[1] = reply / 2;
        }
    }
};


// These decision graphs were obtained by exhaustive search over all roots,
// assignments, and legal operations. The first reply uses sign symmetry.
void solve_small(const Judge& judge, int n, vector<int>& result) {
    static constexpr int tree2[] = {
        4, 2, 1, 7, 3, 6,
        -6,
        3, 2, 0, 14, 1, 13,
        -5,
        -4,
    };
    static constexpr int tree3[] = {
        21, 7, 0, 34, 1, 48, 2, 47, 3, 26, 4, 16, 5, 46, 6, 46,
        -1, 18,
        16, 2, -1, 25, 1, 24,
        -8,
        -6,
        16, 2, 0, 33, 2, 32,
        -9,
        -7,
        -2, 36,
        16, 4, -3, 49, -1, 48, 1, 47, 3, 46,
        -11,
        -10,
        -5,
        -4,
    };
    static constexpr int tree4[] = {
        155, 8, 0, 109, 1, 67, 2, 56, 3, 37, 4, 29, 5, 19, 6, 18, 7, 18,
        -20,
        -1, 21,
        275, 3, -1, 108, 1, 98, 3, 86,
        275, 3, 2, 107, 4, 97, 6, 85,
        1, 2, 0, 116, 2, 43,
        -1, 45,
        275, 3, -3, 55, -1, 54, 1, 53,
        -13,
        -9,
        -7,
        275, 3, 0, 66, 2, 65, 4, 64,
        -14,
        -10,
        -8,
        275, 3, -1, 99, 1, 87, 3, 75,
        -1, 77,
        125, 3, -1, 86, 1, 85, 3, 85,
        -18,
        -17,
        -1, 89,
        125, 3, -1, 98, 1, 97, 3, 97,
        -16,
        -15,
        -1, 101,
        125, 2, -1, 108, 1, 107,
        -12,
        -11,
        1, 2, -1, 116, 1, 115,
        -6,
        -19,
    };
    static constexpr int tree5[] = {
        4140, 14, 0, 263, 1, 247, 2, 233, 3, 175, 4, 167, 5, 126, 6, 120, 7, 90, 8, 80, 9, 70, 10, 64, 11, 46, 13, 30, 15, 144,
        -2, 32,
        438, 6, -3, 231, 1, 207, 3, 145, 5, 144, 7, 145, 9, 144,
        -1, 48,
        438, 4, -3, 232, -1, 58, 1, 208, 3, 196,
        36, 2, -1, 261, 1, 220,
        6, 2, 0, 145, 2, 144,
        -1, 72,
        2598, 3, -1, 112, 1, 276, 3, 275,
        438, 4, 0, 231, 2, 219, 4, 207, 6, 195,
        -1, 92,
        1950, 5, -3, 114, -1, 220, 1, 113, 3, 104, 5, 196,
        2628, 3, -3, 154, -1, 112, 1, 208,
        -20,
        -18,
        1296, 2, -1, 155, 1, 232,
        6, 2, 0, 260, 2, 259,
        -1, 128,
        1560, 7, -5, 276, -3, 157, -1, 156, 1, 146, 9, 145, 11, 144, 13, 144,
        -37,
        -35,
        2808, 3, -3, 155, -1, 154, 1, 261,
        -16,
        -12,
        -8,
        2808, 3, -3, 166, -1, 165, 1, 262,
        -14,
        -10,
        6, 2, 0, 174, 2, 173,
        -21,
        -19,
        438, 4, -3, 221, -1, 209, 1, 197, 3, 185,
        -1, 187,
        1296, 3, -1, 196, 1, 195, 3, 195,
        -33,
        -32,
        -1, 199,
        1296, 3, -1, 208, 1, 207, 3, 207,
        -31,
        -30,
        -1, 211,
        1296, 3, -1, 220, 1, 219, 3, 219,
        -29,
        -28,
        -1, 223,
        1296, 3, -1, 232, 1, 231, 3, 231,
        -27,
        -26,
        438, 4, 0, 246, 2, 245, 4, 244, 6, 243,
        -17,
        -15,
        -13,
        -11,
        -1, 249,
        1554, 4, -8, 262, -6, 261, 4, 260, 6, 259,
        -25,
        -23,
        -24,
        -22,
        7, 4, -3, 276, -1, 275, 1, 274, 3, 273,
        -9,
        -7,
        -36,
        -34,
    };
    static constexpr int tree6[] = {
        36407, 14, 0, 541, 1, 435, 2, 415, 3, 329, 4, 299, 5, 250, 6, 228, 7, 172, 8, 158, 9, 102, 10, 96, 11, 52, 13, 30, 15, 268,
        -2, 32,
        5166, 9, -4, 413, -2, 401, 0, 389, 8, 269, 10, 269, 12, 269, 14, 268, 16, 268, 18, 268,
        -1, 54,
        5166, 6, -4, 414, -2, 402, 0, 88, 2, 82, 4, 76, 6, 68,
        33663, 3, -1, 460, 1, 354, 3, 553,
        49, 2, -1, 484, 1, 366,
        49, 2, -1, 503, 1, 378,
        4851, 3, -1, 528, 1, 390, 3, 554,
        7, 2, 0, 269, 2, 268,
        -1, 104,
        55230, 6, -4, 214, -2, 472, 0, 144, 2, 134, 4, 126, 6, 118,
        735, 3, -3, 484, -1, 366, 1, 354,
        735, 3, -3, 504, -1, 402, 1, 390,
        735, 4, -3, 280, -1, 528, 1, 503, 3, 378,
        51156, 6, -4, 200, -2, 190, 0, 460, 2, 540, 4, 516, 6, 414,
        5166, 6, 2, 413, 4, 401, 6, 389, 8, 377, 10, 365, 12, 353,
        -1, 174,
        7896, 7, -6, 540, -4, 222, -2, 215, 0, 201, 2, 192, 4, 191, 6, 190,
        -33,
        -31,
        33663, 3, -3, 281, -1, 200, 1, 484,
        -29,
        52871, 5, -5, 282, -3, 214, -1, 213, 1, 503, 3, 504,
        -27,
        -21,
        16807, 2, -1, 221, 1, 516,
        -19,
        16807, 2, -1, 283, 1, 528,
        7896, 7, 0, 539, 2, 527, 4, 515, 6, 244, 8, 483, 10, 471, 12, 459,
        2401, 2, 0, 501, 2, 502,
        -1, 252,
        19614, 7, -6, 554, -4, 285, -2, 284, 0, 270, 12, 269, 14, 268, 16, 268,
        -70,
        -68,
        55566, 4, -6, 283, -4, 282, -2, 281, 0, 280,
        -41,
        -25,
        -17,
        -13,
        -9,
        55566, 4, -6, 298, -4, 297, -2, 296, 0, 295,
        -39,
        -23,
        -15,
        -11,
        7896, 7, 0, 328, 2, 327, 4, 326, 6, 318, 8, 317, 10, 316, 12, 315,
        -34,
        -32,
        -30,
        2401, 2, 0, 325, 2, 324,
        -28,
        -22,
        -20,
        -42,
        -40,
        5166, 6, -4, 403, -2, 391, 0, 379, 2, 367, 4, 355, 6, 343,
        -1, 345,
        16807, 3, -1, 354, 1, 353, 3, 353,
        -66,
        -65,
        -1, 357,
        16807, 3, -1, 366, 1, 365, 3, 365,
        -62,
        -61,
        -1, 369,
        16807, 3, -1, 378, 1, 377, 3, 377,
        -54,
        -53,
        -1, 381,
        16807, 3, -1, 390, 1, 389, 3, 389,
        -64,
        -63,
        -1, 393,
        16807, 3, -1, 402, 1, 401, 3, 401,
        -60,
        -59,
        -1, 405,
        16807, 3, -1, 414, 1, 413, 3, 413,
        -52,
        -51,
        5166, 6, 0, 434, 2, 433, 4, 432, 6, 431, 8, 430, 10, 429,
        -26,
        -18,
        -14,
        -24,
        -16,
        -12,
        7896, 7, -6, 529, -4, 517, -2, 505, 0, 485, 2, 473, 4, 461, 6, 451,
        -1, 453,
        16807, 2, -1, 460, 1, 459,
        -38,
        -37,
        -1, 463,
        16807, 3, -3, 472, -1, 472, 1, 471,
        -36,
        -35,
        -1, 475,
        16807, 3, -1, 484, 1, 483, 3, 483,
        -58,
        -57,
        -1, 487,
        350, 6, -6, 504, -4, 504, -2, 503, 2, 502, 4, 501, 6, 501,
        -50,
        -56,
        -49,
        -55,
        -1, 507,
        16807, 3, -1, 516, 1, 515, 3, 515,
        -48,
        -47,
        -1, 519,
        16807, 3, -1, 528, 1, 527, 3, 527,
        -46,
        -45,
        -1, 531,
        16807, 3, -1, 540, 1, 539, 3, 539,
        -44,
        -43,
        8, 4, -3, 554, -1, 553, 1, 552, 3, 551,
        -10,
        -8,
        -69,
        -67,
    };
    static constexpr int tree7[] = {
        856065, 18, 0, 1019, 1, 993, 2, 879, 3, 853, 4, 739, 5, 713, 6, 610, 7, 560, 8, 443, 9, 425, 10, 355, 11, 337, 12, 267, 13, 248, 14, 140, 16, 90, 18, 56, 20, 38,
        -1, 40,
        2184, 7, -5, 700, -3, 688, -1, 676, 1, 664, 3, 652, 5, 640, 7, 628,
        -1, 58,
        16968, 7, -5, 82, -3, 74, -1, 768, 1, 712, 3, 116, 5, 108, 7, 628,
        1088, 3, -1, 816, 1, 792, 3, 780,
        1088, 3, -3, 840, -1, 828, 1, 804,
        -1, 92,
        131656, 7, -5, 132, -3, 124, -1, 908, 1, 712, 3, 116, 5, 108, 7, 628,
        1088, 3, -1, 676, 1, 652, 3, 640,
        1088, 3, -3, 700, -1, 688, 1, 664,
        1088, 3, -1, 956, 1, 932, 3, 920,
        1088, 3, -3, 980, -1, 968, 1, 944,
        -1, 142,
        103488, 8, -7, 1099, -5, 238, -3, 228, -1, 214, 1, 198, 3, 184, 5, 170, 7, 160,
        524296, 4, -3, 1116, -1, 1115, 1, 640, 3, 628,
        1056776, 6, -3, 1132, -1, 1131, 1, 780, 3, 768, 5, 664, 7, 652,
        1056776, 6, -3, 1148, -1, 1147, 1, 804, 3, 792, 5, 688, 7, 676,
        525448, 7, -5, 1163, -3, 712, -1, 700, 1, 828, 3, 816, 5, 920, 7, 908,
        139272, 6, -7, 1052, -5, 1051, -3, 944, -1, 932, 1, 852, 3, 840,
        8200, 4, -3, 1068, -1, 1067, 1, 968, 3, 956,
        8200, 4, -3, 1084, -1, 1083, 1, 992, 3, 980,
        2184, 8, 0, 711, 2, 699, 4, 687, 6, 675, 8, 663, 10, 651, 12, 639, 14, 266,
        -135,
        -1, 269,
        264264, 8, -7, 1164, -5, 325, -3, 315, -1, 816, 1, 309, 3, 297, 5, 287, 7, 768,
        790592, 4, -3, 473, -1, 1115, 1, 792, 3, 780,
        1056832, 5, -7, 501, -5, 486, -3, 1131, -1, 1116, 1, 804,
        4096, 2, -1, 523, 1, 1132,
        790592, 4, -3, 520, -1, 1147, 1, 840, 3, 828,
        1056832, 5, -7, 549, -5, 536, -3, 1163, -1, 1148, 1, 852,
        2184, 8, 0, 851, 2, 839, 4, 827, 6, 815, 8, 803, 10, 791, 12, 779, 14, 767,
        -1, 357,
        264264, 8, -7, 1164, -5, 413, -3, 403, -1, 956, 1, 397, 3, 385, 5, 375, 7, 908,
        819264, 4, -3, 474, -1, 1115, 1, 932, 3, 920,
        1114176, 5, -7, 502, -5, 487, -3, 1131, -1, 1116, 1, 944,
        32768, 2, -1, 524, 1, 1132,
        819264, 4, -3, 521, -1, 1147, 1, 980, 3, 968,
        1114176, 5, -7, 550, -5, 537, -3, 1163, -1, 1148, 1, 992,
        2184, 8, 0, 991, 2, 979, 4, 967, 6, 955, 8, 943, 10, 931, 12, 919, 14, 907,
        -1, 445,
        263816, 8, -7, 552, -5, 539, -3, 526, -1, 504, 1, 489, 3, 476, 5, 463, 7, 1051,
        856064, 4, -6, 475, -4, 474, -2, 473, 0, 1052,
        -54,
        -38,
        -22,
        856064, 4, -6, 488, -4, 487, -2, 486, 0, 1067,
        -52,
        -36,
        -20,
        856128, 5, -7, 503, -5, 502, -3, 501, -1, 1068, 1, 1083,
        -50,
        -34,
        -18,
        69832, 7, -7, 525, -5, 524, -3, 523, -1, 1084, 1, 522, 3, 521, 5, 520,
        -46,
        -30,
        -14,
        -48,
        -32,
        -16,
        856064, 4, -6, 538, -4, 537, -2, 536, 0, 1099,
        -44,
        -28,
        -12,
        856064, 4, -6, 551, -4, 550, -2, 549, 0, 1100,
        -42,
        -26,
        -10,
        32768, 2, -1, 559, 1, 558,
        -40,
        -24,
        1049160, 8, 0, 1162, 2, 602, 4, 594, 6, 1113, 8, 1098, 10, 586, 12, 578, 14, 1049,
        1088, 3, 2, 1081, 4, 1065, 6, 1050,
        1088, 3, 0, 1097, 2, 1082, 4, 1066,
        1088, 3, 2, 1145, 4, 1129, 6, 1114,
        1088, 3, 0, 1161, 2, 1146, 4, 1130,
        2184, 8, -7, 701, -5, 689, -3, 677, -1, 665, 1, 653, 3, 641, 5, 629, 7, 628,
        -134,
        -1, 631,
        262144, 3, -1, 640, 1, 639, 3, 639,
        -133,
        -132,
        -1, 643,
        262144, 3, -1, 652, 1, 651, 3, 651,
        -131,
        -130,
        -1, 655,
        262144, 3, -1, 664, 1, 663, 3, 663,
        -129,
        -128,
        -1, 667,
        262144, 3, -1, 676, 1, 675, 3, 675,
        -127,
        -126,
        -1, 679,
        262144, 3, -1, 688, 1, 687, 3, 687,
        -125,
        -124,
        -1, 691,
        262144, 3, -1, 700, 1, 699, 3, 699,
        -123,
        -122,
        -1, 703,
        262144, 3, -1, 712, 1, 711, 3, 711,
        -121,
        -120,
        2184, 8, 0, 738, 2, 737, 4, 736, 6, 735, 8, 734, 10, 733, 12, 732, 14, 731,
        -55,
        -53,
        -51,
        -49,
        -47,
        -45,
        -43,
        -41,
        2184, 8, -7, 841, -5, 829, -3, 817, -1, 805, 1, 793, 3, 781, 5, 769, 7, 757,
        -1, 759,
        262144, 3, -1, 768, 1, 767, 3, 767,
        -119,
        -118,
        -1, 771,
        262144, 3, -1, 780, 1, 779, 3, 779,
        -117,
        -116,
        -1, 783,
        262144, 3, -1, 792, 1, 791, 3, 791,
        -115,
        -114,
        -1, 795,
        262144, 3, -1, 804, 1, 803, 3, 803,
        -113,
        -112,
        -1, 807,
        262144, 3, -1, 816, 1, 815, 3, 815,
        -111,
        -110,
        -1, 819,
        262144, 3, -1, 828, 1, 827, 3, 827,
        -109,
        -108,
        -1, 831,
        262144, 3, -1, 840, 1, 839, 3, 839,
        -107,
        -106,
        -1, 843,
        262144, 3, -1, 852, 1, 851, 3, 851,
        -105,
        -104,
        2184, 8, 0, 878, 2, 877, 4, 876, 6, 875, 8, 874, 10, 873, 12, 872, 14, 871,
        -39,
        -37,
        -35,
        -33,
        -31,
        -29,
        -27,
        -25,
        2184, 8, -7, 981, -5, 969, -3, 957, -1, 945, 1, 933, 3, 921, 5, 909, 7, 897,
        -1, 899,
        262144, 3, -1, 908, 1, 907, 3, 907,
        -103,
        -102,
        -1, 911,
        262144, 3, -1, 920, 1, 919, 3, 919,
        -101,
        -100,
        -1, 923,
        262144, 3, -1, 932, 1, 931, 3, 931,
        -99,
        -98,
        -1, 935,
        262144, 3, -1, 944, 1, 943, 3, 943,
        -97,
        -96,
        -1, 947,
        262144, 3, -1, 956, 1, 955, 3, 955,
        -95,
        -94,
        -1, 959,
        262144, 3, -1, 968, 1, 967, 3, 967,
        -93,
        -92,
        -1, 971,
        262144, 3, -1, 980, 1, 979, 3, 979,
        -91,
        -90,
        -1, 983,
        262144, 3, -1, 992, 1, 991, 3, 991,
        -89,
        -88,
        2184, 8, 0, 1018, 2, 1017, 4, 1016, 6, 1015, 8, 1014, 10, 1013, 12, 1012, 14, 1011,
        -23,
        -21,
        -19,
        -17,
        -15,
        -13,
        -11,
        -9,
        1049664, 8, -7, 1149, -5, 1133, -3, 1117, -1, 1101, 1, 1085, 3, 1069, 5, 1053, 7, 1037,
        -1, 1039,
        299592, 4, -10, 1052, -8, 1051, 10, 1050, 12, 1049,
        -87,
        -85,
        -86,
        -84,
        -1, 1055,
        299592, 4, -12, 1068, -10, 1067, 8, 1066, 10, 1065,
        -83,
        -81,
        -82,
        -80,
        -1, 1071,
        299592, 4, -12, 1084, -10, 1083, 8, 1082, 10, 1081,
        -79,
        -77,
        -78,
        -76,
        -1, 1087,
        299592, 4, -14, 1100, -12, 1099, 6, 1098, 8, 1097,
        -75,
        -73,
        -74,
        -72,
        -1, 1103,
        299592, 4, -8, 1116, -6, 1115, 12, 1114, 14, 1113,
        -71,
        -69,
        -70,
        -68,
        -1, 1119,
        299592, 4, -10, 1132, -8, 1131, 10, 1130, 12, 1129,
        -67,
        -65,
        -66,
        -64,
        -1, 1135,
        299592, 4, -10, 1148, -8, 1147, 10, 1146, 12, 1145,
        -63,
        -61,
        -62,
        -60,
        -1, 1151,
        299592, 4, -12, 1164, -10, 1163, 8, 1162, 10, 1161,
        -59,
        -57,
        -58,
        -56,
    };

    const int* trees[] = {tree2, tree3, tree4, tree5, tree6, tree7};
    const int* tree = trees[n - 2];
    int position = 0;
    int sign = 1;
    while (true) {
        int action = tree[position];
        if (action < -n) {
            int mask = -action - n - 1;
            for (int id = 1; id <= n; ++id) {
                result[id] = (mask >> (id - 1) & 1) ? sign : -sign;
            }
            return;
        }
        if (action < 0) {
            judge.flip(-action);
            position = tree[position + 1];
            continue;
        }

        // The digits of the action are the query's vertex multiplicities.
        Query query;
        for (int id = 1; id <= n; ++id) {
            int weight = action % (n + 1);
            if (weight) query.emplace_back(id, weight);
            action /= n + 1;
        }
        int reply = judge.ask(query);
        if (position == 0 && reply < 0) sign = -1;
        reply *= sign;
        int branches = tree[position + 1];
        int next = -1;
        for (int i = 0; i < branches; ++i) {
            if (tree[position + 2 + 2 * i] == reply) {
                next = tree[position + 3 + 2 * i];
                break;
            }
        }
        if (next < 0) exit(0);
        position = next;
    }
}

void play(const Judge& judge, int n) {
    vector<int> result(n + 1);
    if (n <= 7) {
        solve_small(judge, n, result);
        judge.answer(result);
        return;
    }

    int leaves = n - 1;
    Query all;
    for (int u = 2; u <= n; ++u) all.emplace_back(u, 1);
    if (n % 2) all.emplace_back(1, 1);
    int length = query_weight(all);
    int reply = judge.ask(all);

    // The residual f(leaf) - f(1) is the leaf's value, except at a leaf
    // root where it is minus the center's value. Both are signs, so an odd
    // query length restricts the center path sum to one parity.
    vector<int> centers;
    for (int center = -2; center <= 2; ++center) {
        int sum = reply - length * center;
        if (abs(sum) > leaves || (sum + leaves) % 2) continue;
        if (center == 2 && sum == leaves) continue;
        if (center == -2 && sum == -leaves) continue;
        centers.push_back(center);
    }
    if (centers.empty()) return;
    int center = centers[0];
    if (centers.size() > 1) {
        center = judge.ask({{1, 1}});
        if (find(centers.begin(), centers.end(), center) == centers.end()) return;
    }

    int positives = (reply - length * center + leaves) / 2;
    if (abs(center) == 1) {
        result[1] = center;
    } else if (center == 0) {
        if (positives == 0 || positives == leaves) {
            // The root is a leaf, and its value cancels the center's.
            int sign = positives ? 1 : -1;
            fill(result.begin() + 1, result.end(), sign);
            result[1] = -sign;
            judge.answer(result);
            return;
        }
    } else {
        int sign = center / 2;
        if ((sign == 1 ? leaves - positives : positives) == 1) {
            // The root is the only opposite residual, so all values agree.
            fill(result.begin() + 1, result.end(), sign);
            judge.answer(result);
            return;
        }
        judge.flip(1);
        // The center and root now cancel. The root's residual changes sign;
        // every other residual stays the same.
        positives += sign;
        result[1] = -sign;
    }

    Decoder decoder(judge, n, abs(center) == 1 ? center : 0, positives,
                    center == 0, result);
    decoder.run();
    judge.answer(result);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 2 || n > 1000) return 0;
    for (int i = 1; i < n; ++i) {
        int u, v;
        if (!(cin >> u >> v)) return 0;
    }
    play(Judge{}, n);
}

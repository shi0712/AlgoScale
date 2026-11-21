#include <bits/stdc++.h>
using namespace std;

using Int = long long;
constexpr Int element_limit = 1'000'000'000;
constexpr Int infinity = 1LL << 60;

class Reconstruction {
    struct Observation {
        int left, right;
        Int value;
    };

    struct Block {
        int first, last;
        Int weight;
    };

    struct Gap {
        Int low, high;
        Int value = 0;
    };

    struct Deletion {
        int left, right, combined;
        Int weight;
    };

    int n;
    function<Int(int, int)> ask;
    vector<Int> positive;
    vector<Observation> observations;
    unordered_map<int, Int> replies;
    vector<Block> runs;
    vector<Gap> gaps;
    vector<vector<Int>> upper, lower;
    vector<Int> suffix_low, suffix_high, suffix_best;

    Int query(int left, int right) {
        int key = left * (n + 1) + right;
        auto it = replies.find(key);
        if (it != replies.end()) return it->second;
        Int value = ask(left, right);
        replies.emplace(key, value);
        observations.push_back({left, right, value});
        return value;
    }

    void discover_sparse(int left, int right, Int value) {
        if (!value) return;
        int length = right - left + 1;
        if (value == length * element_limit) {
            fill(positive.begin() + left, positive.begin() + right + 1, element_limit);
        } else if (length == 1) {
            positive[left] = value;
        } else if (length == 2) {
            positive[right] = query(right, right);
            positive[left] = value - positive[right];
        } else if (length == 3) {
            positive[right] = query(right, right);
            if (positive[right] == 0) {
                discover_sparse(left, right - 1, value);
            } else {
                positive[left] = query(left, left);
                positive[left + 1] = max(0LL, value - positive[left] - positive[right]);
            }
        } else {
            int mid = (left + right) / 2;
            Int left_value = query(left, mid);
            discover_sparse(left, mid, left_value);
            int start = mid + 1;
            if (left_value == value) {
                Int suffix = 0;
                for (int i = mid; i >= left && positive[i] > 0; --i) suffix += positive[i];
                if (suffix == value) ++start;
            }
            if (start <= right) {
                Int right_value = left_value == 0 ? value : query(start, right);
                discover_sparse(start, right, right_value);
            }
        }
    }

    int first_positive(int left, int right, Int value) {
        while (left < right) {
            int mid = (left + right) / 2;
            Int prefix = query(left, mid);
            if (prefix == 0) {
                left = mid + 1;
            } else {
                right = mid;
                value = prefix;
            }
        }
        positive[left] = value;
        return left;
    }

    void discover_thin(int left, int right, Int value) {
        // A suffix probe is cheap when the block has only a few positives.
        // Switch to balanced splitting if the initial sparse guess was wrong.
        for (int found = 0; found < 2 && value > 0 && right - left >= 7; ++found) {
            left = first_positive(left, right, value) + 1;
            if (left > right) return;
            value = query(left, right);
        }
        if (left <= right) discover_sparse(left, right, value);
    }

    void discover(Int total) {
        if (n <= 3 || total == n * element_limit) {
            discover_sparse(1, n, total);
            return;
        }
        int run_start = 1, zero_streak = 0, positives = 0;
        int zero_runs = 0, isolated_zeros = 0;
        Int run_weight = 0, largest_single = 0;
        bool tested_tail = false;
        for (int i = 1; i <= n;) {
            if (!tested_tail && positives == 1 && largest_single == total &&
                zero_streak > 0 && n - i >= 5) {
                tested_tail = true;
                if (query(i, n) == 0) return;
            }
            int end = min(n, i + 1);
            bool thin = i >= 32 && 12 * positives <= i - 1;
            bool align = zero_streak == 1 && positives > 0 &&
                         4 * isolated_zeros + 1 >= 3 * zero_runs;
            if (run_weight == total) {
                end = i;
            } else if (i == 1 || align) {
                end = i;
                positive[i] = query(i, i);
            } else if (positives == 0 && zero_streak >= 4) {
                if (n - i >= 16) {
                    Int prefix = query(i, i + 7);
                    end = prefix ? first_positive(i, i + 7, prefix)
                                 : first_positive(i + 8, n, total);
                } else {
                    end = first_positive(i, n, total);
                }
            } else if (zero_streak >= 4 || (thin && zero_streak > 0)) {
                int length = 2 * zero_streak;
                if (thin) length = max(length, (i - 1) / (2 * (positives + 1)));
                end = min(n, i + length - 1);
                Int value = query(i, end);
                if (thin) discover_thin(i, end, value);
                else discover_sparse(i, end, value);
            } else {
                int start = run_weight == 0 ? i : run_start;
                Int value = query(start, end);
                if (end == i) {
                    positive[i] = value - run_weight;
                } else if (value == run_weight + 2 * element_limit) {
                    positive[i] = positive[end] = element_limit;
                } else if (value == run_weight) {
                    // Extending a positive run by a positive element increases MSS.
                    // With no preceding run, a zero reply settles both positions.
                    if (run_weight > 0) end = i;
                } else {
                    positive[end] = query(end, end);
                    positive[i] = max(0LL, value - run_weight - positive[end]);
                }
            }
            for (; i <= end; ++i) {
                if (positive[i] > 0) {
                    if (zero_streak > 0) {
                        ++zero_runs;
                        isolated_zeros += (zero_streak == 1);
                    }
                    ++positives;
                    largest_single = max(largest_single, positive[i]);
                    if (run_weight == 0) run_start = i;
                    run_weight += positive[i];
                    zero_streak = 0;
                } else {
                    run_weight = 0;
                    ++zero_streak;
                }
            }
        }
    }

    // Bounds for the best suffix and prefix on the two sides of each gap.
    // A certified MSS bounds their crossing sum, and forces a crossing if
    // neither side alone can attain that MSS.
    void tighten(const vector<Int>& weights, const vector<int>& edges,
                 Int result, bool exact) {
        int size = static_cast<int>(weights.size());
        if (size < 2) return;
        suffix_low[size - 1] = suffix_high[size - 1] = suffix_best[size - 1] = weights.back();
        for (int i = size - 2; i >= 0; --i) {
            const Gap& gap = gaps[edges[i]];
            suffix_low[i] = weights[i] + max(0LL, suffix_low[i + 1] - gap.high);
            suffix_high[i] = weights[i] + max(0LL, suffix_high[i + 1] - gap.low);
            suffix_best[i] = max(suffix_best[i + 1], suffix_high[i]);
        }
        Int end_low = weights[0], end_high = weights[0], best = weights[0];
        for (int i = 0; i + 1 < size; ++i) {
            Gap& gap = gaps[edges[i]];
            gap.low = max(gap.low, end_low + suffix_low[i + 1] - result);
            if (exact && max(best, suffix_best[i + 1]) < result) {
                gap.high = min(gap.high, end_high + suffix_high[i + 1] - result);
            }
            end_low = weights[i + 1] + max(0LL, end_low - gap.high);
            end_high = weights[i + 1] + max(0LL, end_high - gap.low);
            best = max(best, end_high);
        }
    }

    void prepare_bounds() {
        int count = static_cast<int>(runs.size());
        upper.assign(count, vector<Int>(count, infinity));
        lower.assign(count, vector<Int>(count));
        suffix_low.resize(count);
        suffix_high.resize(count);
        suffix_best.resize(count);

        vector<int> next_positive(n + 2, n + 1), prev_positive(n + 1), run_at(n + 1);
        vector<Int> prefix(n + 1);
        for (int i = 1; i <= n; ++i) {
            prev_positive[i] = positive[i] ? i : prev_positive[i - 1];
            prefix[i] = prefix[i - 1] + positive[i];
        }
        for (int i = n; i >= 1; --i) {
            next_positive[i] = positive[i] ? i : next_positive[i + 1];
        }
        for (int i = 0; i < count; ++i) {
            for (int j = runs[i].first; j <= runs[i].last; ++j) run_at[j] = i;
        }

        vector<Observation> trimmed;
        for (const auto& [left, right, value] : observations) {
            if (value == 0) continue;
            int l = next_positive[left], r = prev_positive[right];
            int first = run_at[l], last = run_at[r];
            lower[first][last] = max(lower[first][last], value);
            if (l != runs[first].first) ++first;
            if (r != runs[last].last) --last;
            if (first <= last) upper[first][last] = min(upper[first][last], value);
            if (run_at[l] < run_at[r]) trimmed.push_back({l, r, value});
        }
        for (int i = 0; i < count; ++i) {
            for (int j = count - 1; j >= i; --j) {
                if (i > 0) upper[i][j] = min(upper[i][j], upper[i - 1][j]);
                if (j + 1 < count) upper[i][j] = min(upper[i][j], upper[i][j + 1]);
            }
        }
        for (int i = count - 1; i >= 0; --i) {
            for (int j = i; j < count; ++j) {
                if (i < j) lower[i][j] = max({lower[i][j], lower[i + 1][j], lower[i][j - 1]});
            }
        }

        vector<Int> weights;
        vector<int> edges;
        weights.reserve(count);
        edges.reserve(count);
        for (int pass = 0; pass < 4; ++pass) {
            for (const auto& [left, right, value] : trimmed) {
                int first = run_at[left], last = run_at[right];
                weights.clear();
                edges.clear();
                for (int i = first; i <= last; ++i) {
                    int l = max(left, runs[i].first), r = min(right, runs[i].last);
                    weights.push_back(prefix[r] - prefix[l - 1]);
                    if (i < last) edges.push_back(i);
                }
                tighten(weights, edges, value, true);
            }
        }
    }

    static void constrain(Gap& gap, Int left, Int right, Int low_mss, Int high_mss) {
        gap.low = max(gap.low, left + right - high_mss);
        if (low_mss > max(left, right)) gap.high = min(gap.high, left + right - low_mss);
    }

    // Active intervals represent the original intervals from the first run's
    // start through the last run's end. Every reduction preserves these MSS
    // values and specifies how to expand a solution back to the prior blocks.
    void reduce(Int total) {
        int count = static_cast<int>(runs.size());
        vector<Block> blocks;
        vector<int> edges;
        vector<Int> weights;
        vector<Deletion> deletions;
        blocks.reserve(count);
        edges.reserve(count);
        weights.reserve(count);
        deletions.reserve(count);
        for (int i = 0; i < count; ++i) {
            blocks.push_back({i, i, runs[i].weight});
            if (i + 1 < count) edges.push_back(i);
        }
        Int removed_max = 0;
        while (blocks.size() > 1) {
            int size = static_cast<int>(blocks.size());
            weights.clear();
            for (const Block& block : blocks) weights.push_back(block.weight);
            for (int i = 0; i + 1 < size; ++i) {
                int l = blocks[i].first, r = blocks[i + 1].last;
                constrain(gaps[edges[i]], weights[i], weights[i + 1], lower[l][r], upper[l][r]);
            }
            tighten(weights, edges, total, removed_max < total);
            tighten(weights, edges, total, removed_max < total);

            int merge_at = -1;
            for (int i = 0; i + 1 < size; ++i) {
                const Gap& gap = gaps[edges[i]];
                if (gap.low == gap.high && gap.low < min(weights[i], weights[i + 1])) {
                    merge_at = i;
                    break;
                }
            }
            if (merge_at >= 0) {
                int i = merge_at;
                Gap& gap = gaps[edges[i]];
                gap.value = gap.low;
                // Every crossing can start at the first block and end at the
                // second. A clipped endpoint just reduces that block's gain.
                blocks[i].weight += blocks[i + 1].weight - gap.value;
                blocks[i].last = blocks[i + 1].last;
                blocks.erase(blocks.begin() + i + 1);
                edges.erase(edges.begin() + i);
                continue;
            }

            int remove_at = -1, probe_at = -1, best_needed = 3;
            double best_chance = -1;
            for (int i = 0; i < size; ++i) {
                Int w = weights[i];
                bool has_left = i > 0, has_right = i + 1 < size;
                bool need_left = has_left && gaps[edges[i - 1]].low < w;
                bool need_right = has_right && gaps[edges[i]].low < w;
                if (!need_left && !need_right &&
                    (!has_left || !has_right || w <= min(weights[i - 1], weights[i + 1]))) {
                    remove_at = i;
                    break;
                }
                if ((has_left && weights[i - 1] < w) || (has_right && weights[i + 1] < w)) continue;
                int needed = need_left + need_right;
                for (int j = i - 1; j <= i; ++j) {
                    if ((j == i - 1 && !need_left) || (j == i && !need_right)) continue;
                    const Gap& gap = gaps[edges[j]];
                    // Prefer a probe that can finish this reduction, then one
                    // whose bounds leave a larger fraction of mergeable gaps.
                    double chance = static_cast<double>(min(w, gap.high + 1) - gap.low) /
                                    static_cast<double>(gap.high + 1 - gap.low);
                    if (needed < best_needed || (needed == best_needed && chance > best_chance)) {
                        best_needed = needed;
                        best_chance = chance;
                        probe_at = j;
                    }
                }
            }
            if (remove_at >= 0) {
                int i = remove_at;
                if (i == 0 || i + 1 == size) {
                    int j = i == 0 ? 0 : i - 1;
                    Gap& gap = gaps[edges[j]];
                    gap.value = gap.low;
                    edges.erase(edges.begin() + j);
                    removed_max = max(removed_max, weights[i]);
                } else {
                    int l = edges[i - 1], r = edges[i], combined = static_cast<int>(gaps.size());
                    // With both neighbors included, neither end of an optimum
                    // needs this block. Its removal combines the two gaps.
                    gaps.push_back({gaps[l].low + gaps[r].low - weights[i],
                                    gaps[l].high + gaps[r].high - weights[i]});
                    deletions.push_back({l, r, combined, weights[i]});
                    edges[i - 1] = combined;
                    edges.erase(edges.begin() + i);
                }
                blocks.erase(blocks.begin() + i);
            } else {
                int i = probe_at;
                Int value = query(runs[blocks[i].first].first, runs[blocks[i + 1].last].last);
                constrain(gaps[edges[i]], weights[i], weights[i + 1], value, value);
            }
        }
        for (auto it = deletions.rbegin(); it != deletions.rend(); ++it) {
            const auto& [l, r, combined, weight] = *it;
            Int sum = gaps[combined].value + weight;
            gaps[l].value = max(gaps[l].low, sum - gaps[r].high);
            gaps[r].value = sum - gaps[l].value;
        }
    }

public:
    Reconstruction(int size, function<Int(int, int)> query_judge)
        : n(size), ask(move(query_judge)), positive(n + 1) {
        replies.reserve(3 * n);
        observations.reserve(3 * n);
    }

    vector<Int> solve() {
        Int total = query(1, n);
        if (!total) return positive;
        discover(total);
        for (int i = 1; i <= n; ++i) {
            if (positive[i] == 0) continue;
            int start = i;
            Int weight = positive[i];
            while (i < n && positive[i + 1] > 0) weight += positive[++i];
            runs.push_back({start, i, weight});
        }
        for (int i = 0; i + 1 < static_cast<int>(runs.size()); ++i) {
            gaps.push_back({0, element_limit * (runs[i + 1].first - runs[i].last - 1)});
        }
        if (runs.size() > 1) {
            prepare_bounds();
            reduce(total);
            for (int i = 0; i + 1 < static_cast<int>(runs.size()); ++i) {
                positive[runs[i].last + 1] = -gaps[i].value;
            }
        }
        return positive;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1) return 0;
    Reconstruction solver(n, [](int left, int right) {
        cout << "? " << left << ' ' << right << '\n' << flush;
        Int result;
        if (!(cin >> result) || result < 0) exit(0);
        return result;
    });
    vector<Int> answer = solver.solve();
    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << answer[i];
    cout << '\n' << flush;
    return 0;
}

#include <algorithm>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

class Tournament {
public:
    Tournament(int n, bool reversed)
        : reversed_(reversed), defeated_(n + 1), weight_(n + 1, 1) {}

    int better(int a, int b) const {
        cout << "? " << a << ' ' << b << '\n' << flush;
        int winner;
        if (!(cin >> winner) || (winner != a && winner != b)) {
            exit(0);
        }
        return reversed_ ? (a ^ b ^ winner) : winner;
    }

    int build(const vector<int>& labels, int first, int last) {
        if (last - first == 1) {
            return labels[first];
        }
        int middle = first + (last - first) / 2;
        int a = build(labels, first, middle);
        int b = build(labels, middle, last);
        return merge(a, b);
    }

    int successor(int root) {
        auto children = std::move(defeated_[root]);
        int best = children.front();
        for (int i = 1; i < (int)children.size(); ++i) {
            best = merge(best, children[i]);
        }
        return best;
    }

    int combine(const vector<int>& roots, int wanted) {
        if (wanted == 1) {
            int best = roots.front();
            for (int i = 1; i < (int)roots.size(); ++i) {
                best = better(best, roots[i]);
            }
            return best;
        }

        using Entry = pair<int, int>;
        priority_queue<Entry, vector<Entry>, greater<Entry>> queue;
        for (int label : roots) {
            queue.emplace(weight_[label], label);
        }
        while (queue.size() > 1) {
            int a = queue.top().second;
            queue.pop();
            int b = queue.top().second;
            queue.pop();
            int best = merge(a, b);
            queue.emplace(weight_[best], best);
        }
        return queue.top().second;
    }

    vector<int> extract(int best, int count) {
        vector<int> result{best};
        while ((int)result.size() < count) {
            best = successor(best);
            result.push_back(best);
        }
        return result;
    }

    const vector<int>& defeated(int label) const {
        return defeated_[label];
    }

private:
    bool reversed_;
    vector<vector<int>> defeated_;
    vector<int> weight_;

    int merge(int a, int b) {
        int winner = better(a, b);
        int loser = a ^ b ^ winner;
        defeated_[winner].push_back(loser);
        weight_[winner] += weight_[loser];
        return winner;
    }
};

// Select from a sorted prefix and an unordered pool. If requested, return
// the weakest selected applicant as well, for use as a screening cutoff.
int select_best(const Tournament& tournament, const vector<int>& ordered,
                vector<int> pool, int wanted, vector<int>& selected,
                bool need_cutoff) {
    int first = 0;
    int last = ordered.size();
    vector<int> above, below;
    above.reserve(pool.size());
    below.reserve(pool.size());

    unordered_map<unsigned, int> comparisons;
    comparisons.reserve(pool.size() + ordered.size());
    auto better = [&](int a, int b) {
        unsigned key = (static_cast<unsigned>(min(a, b)) << 16) | max(a, b);
        auto entry = comparisons.find(key);
        if (entry != comparisons.end()) {
            return entry->second;
        }
        const auto& a_losers = tournament.defeated(a);
        const auto& b_losers = tournament.defeated(b);
        int winner;
        if (find(a_losers.begin(), a_losers.end(), b) != a_losers.end()) {
            winner = a;
        } else if (find(b_losers.begin(), b_losers.end(), a) != b_losers.end()) {
            winner = b;
        } else {
            winner = tournament.better(a, b);
        }
        comparisons.emplace(key, winner);
        return winner;
    };

    int partition_budget = 4 * (pool.size() + ordered.size());
    for (;;) {
        int sorted = last - first;
        int unsorted = pool.size();
        int total = sorted + unsorted;
        if (unsorted == 0) {
            selected.insert(selected.end(), ordered.begin() + first,
                            ordered.begin() + first + wanted);
            return ordered[first + wanted - 1];
        }
        if (wanted == total && !need_cutoff) {
            selected.insert(selected.end(), ordered.begin() + first, ordered.begin() + last);
            selected.insert(selected.end(), pool.begin(), pool.end());
            return 0;
        }
        if (wanted == 1 || wanted == total || (wanted == total - 1 && !need_cutoff)) {
            bool maximum = wanted == 1;
            int extreme = sorted ? ordered[maximum ? first : last - 1] : pool.front();
            for (int i = sorted ? 0 : 1; i < unsorted; ++i) {
                int best = better(extreme, pool[i]);
                extreme = maximum ? best : (extreme ^ pool[i] ^ best);
            }
            if (maximum) {
                selected.push_back(extreme);
            } else {
                bool include_extreme = wanted == total;
                for (int i = first; i < last; ++i) {
                    if (include_extreme || ordered[i] != extreme) {
                        selected.push_back(ordered[i]);
                    }
                }
                for (int label : pool) {
                    if (include_extreme || label != extreme) {
                        selected.push_back(label);
                    }
                }
            }
            return extreme;
        }
        partition_budget -= total;
        if (partition_budget < 0) {
            pool.insert(pool.end(), ordered.begin() + first, ordered.begin() + last);
            partial_sort(pool.begin(), pool.begin() + wanted, pool.end(), [&](int a, int b) {
                return a != b && better(a, b) == a;
            });
            selected.insert(selected.end(), pool.begin(), pool.begin() + wanted);
            return pool[wanted - 1];
        }

        above.clear();
        below.clear();
        int pivot;
        int split;
        bool from_ordered = sorted >= 3 || (sorted && sorted * 3 >= unsorted);
        if (from_ordered) {
            double target = wanted + (need_cutoff ? 0.0 : 0.5);
            int index = static_cast<int>(lround(target * (sorted + 1) / (total + 1)));
            split = first + clamp(index, 1, sorted) - 1;
            pivot = ordered[split];
        } else if (unsorted >= 6) {
            int sample_size = unsorted >= 45 ? 5 : 3;
            int rank = min(sample_size - 1, (wanted - 1) * sample_size / total);
            for (int i = 1; i < sample_size; ++i) {
                int label = pool[i];
                int j = i;
                while (j > 0 && better(label, pool[j - 1]) == label) {
                    pool[j] = pool[j - 1];
                    --j;
                }
                pool[j] = label;
            }
            pivot = pool[rank];
            above.insert(above.end(), pool.begin(), pool.begin() + rank);
            below.insert(below.end(), pool.begin() + rank + 1, pool.begin() + sample_size);
            for (int i = sample_size; i < unsorted; ++i) {
                int label = pool[i];
                (better(label, pivot) == label ? above : below).push_back(label);
            }
        } else {
            pivot = pool.back();
            pool.pop_back();
        }

        if (from_ordered || unsorted < 6) {
            for (int label : pool) {
                (better(label, pivot) == label ? above : below).push_back(label);
            }
        }
        if (!from_ordered) {
            int lo = first;
            int hi = last;
            while (lo < hi) {
                int middle = lo + (hi - lo) / 2;
                if (better(ordered[middle], pivot) == ordered[middle]) {
                    lo = middle + 1;
                } else {
                    hi = middle;
                }
            }
            split = lo;
        }

        int stronger = split - first + (int)above.size();
        if (stronger < wanted || (stronger == wanted && !need_cutoff)) {
            selected.insert(selected.end(), ordered.begin() + first, ordered.begin() + split);
            selected.insert(selected.end(), above.begin(), above.end());
            if (stronger == wanted) {
                return 0;
            }
            selected.push_back(pivot);
            wanted -= stronger + 1;
            if (!wanted) {
                return pivot;
            }
            first = split + from_ordered;
            pool.swap(below);
        } else {
            last = split;
            pool.swap(above);
        }
    }
}

// Before the cutoff is certified, a group tournament preserves its internal
// results if the cutoff turns out to be too strong. Small groups expose fewer
// runners-up when it is good. Estimate the risk using the random label order.
int screening_group(int n, int processed, int known, int wanted, int rejected) {
    int remaining = n - processed;
    int needed = wanted - known;

    // Beta-binomial probability of fewer than 'needed' wins in the unseen suffix.
    int a = known;
    int b = processed + 1 - a;
    double term = 1.0;
    for (int i = 0; i < a; ++i) {
        term *= double(b + i) / (b + remaining + i);
    }
    double risk = term;
    for (int i = 0; i + 1 < needed; ++i) {
        term *= double(remaining - i) * (a + i) / ((i + 1) * double(b + remaining - i - 1));
        risk += term;
    }
    double win_rate = double(a) / (processed + 1);
    int block = min(512, max(1, (int)(0.65 * risk / win_rate + 0.5)));
    int size = bit_ceil((unsigned)block);
    if (size > block * 1.4) {
        size /= 2;
    }

    int budget = clamp(n / 6, 80, 1800);
    if (risk >= 0.04 && rejected >= 3 * budget / 4) {
        int available = max(1, budget - rejected);
        size = max(size, (remaining + available - 1) / available);
    }
    return min(size, remaining);
}

vector<int> screened_top(Tournament& tournament, const vector<int>& labels, int wanted) {
    int n = labels.size();
    double fraction = clamp(4.55 / wanted, 0.15, 0.30);
    if (n < 2000 && wanted > 20) {
        fraction += 0.04;
    }
    int sample = max(1, (int)lround(n * fraction));
    double overshoot = 1.20 + 0.087 * log2(max(1.0, n / 60.0)) + 0.01 * (wanted - 18);
    int sample_rank = clamp((int)lround(fraction * wanted * overshoot),
                            1, min(wanted - 1, sample));

    int cutoff = tournament.build(labels, 0, sample);
    vector<int> ordered;
    for (int i = 1; i < sample_rank; ++i) {
        ordered.push_back(cutoff);
        cutoff = tournament.successor(cutoff);
    }

    vector<int> above, rejected;
    auto probe = [&](auto&& self, int root) -> void {
        if (tournament.better(root, cutoff) == cutoff) {
            rejected.push_back(root);
            return;
        }
        above.push_back(root);
        for (int child : tournament.defeated(root)) {
            self(self, child);
        }
    };

    int pos = sample;
    while (pos < n && (int)(ordered.size() + above.size()) < wanted - 1) {
        int known = 1 + ordered.size() + above.size();
        if (n - pos < wanted - known) {
            break;
        }
        int size = screening_group(n, pos, known, wanted, rejected.size());
        int root = tournament.build(labels, pos, pos + size);
        probe(probe, root);
        pos += size;
    }

    if ((int)(ordered.size() + above.size()) < wanted - 1) {
        // Even if every untested applicant beats the cutoff, all the known
        // leaders and the cutoff itself qualify.
        int missing = wanted - 1 - ordered.size() - above.size();
        ordered.insert(ordered.end(), above.begin(), above.end());
        ordered.push_back(cutoff);
        for (int root : tournament.defeated(cutoff)) {
            rejected.push_back(root);
        }
        if (pos < n) {
            rejected.push_back(tournament.build(labels, pos, n));
        }
        auto rest = tournament.extract(tournament.combine(rejected, missing), missing);
        ordered.insert(ordered.end(), rest.begin(), rest.end());
        return ordered;
    }

    // At least wanted - 1 applicants beat the cutoff. Its losers are eliminated.
    const int buffer_limit = 4 * wanted;
    for (; pos < n; ++pos) {
        int label = labels[pos];
        if (tournament.better(label, cutoff) == label) {
            above.push_back(label);
        }
        if ((int)(ordered.size() + above.size()) >= buffer_limit && pos + 1 < n) {
            vector<int> best;
            cutoff = select_best(tournament, ordered, std::move(above), wanted, best, true);
            auto end = find_if(ordered.begin(), ordered.end(), [&](int u) {
                return u == cutoff || find(best.begin(), best.end(), u) == best.end();
            });
            ordered.erase(end, ordered.end());
            above.clear();
            for (int u : best) {
                if (u != cutoff && find(ordered.begin(), ordered.end(), u) == ordered.end()) {
                    above.push_back(u);
                }
            }
        }
    }

    vector<int> result;
    int found = ordered.size() + above.size();
    if (found <= wanted) {
        result = std::move(ordered);
        result.insert(result.end(), above.begin(), above.end());
        if (found < wanted) {
            result.push_back(cutoff);
        }
    } else {
        select_best(tournament, ordered, std::move(above), wanted, result, false);
    }
    return result;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k) || n < 1 || k < 1 || k > n) {
        return 0;
    }

    vector<int> labels(n);
    iota(labels.begin(), labels.end(), 1);
    bool reversed = n - k < k;
    int wanted = min(k, n - k);
    Tournament tournament(n, reversed);
    vector<int> result;
    if (wanted == 1) {
        int best = 1;
        for (int i = 2; i <= n; ++i) {
            best = tournament.better(best, i);
        }
        result.push_back(best);
    } else if (wanted > 1) {
        mt19937_64 random(chrono::steady_clock::now().time_since_epoch().count());
        shuffle(labels.begin(), labels.end(), random);

        if (wanted >= 8 || (wanted >= 5 && n <= 4 * wanted)) {
            result = screened_top(tournament, labels, wanted);
        } else {
            result = tournament.extract(tournament.build(labels, 0, n), wanted);
        }
    }
    if (reversed) {
        vector<bool> excluded(n + 1, false);
        for (int label : result) {
            excluded[label] = true;
        }
        result.clear();
        for (int label = 1; label <= n; ++label) {
            if (!excluded[label]) {
                result.push_back(label);
            }
        }
    }

    cout << '!';
    for (int label : result) {
        cout << ' ' << label;
    }
    cout << '\n' << flush;
}

#include <bits/stdc++.h>
using namespace std;

static int ask(char op, int a) {
    cout << op << ' ' << a << endl;
    int response;
    if (!(cin >> response) || response < 0) exit(0);
    return response;
}

// For small universes, optimize the entire decision tree. Only the possible
// answers and which of them have survived a deletion affect future partitions.
class SmallSearch {
    struct Decision {
        int cost = 0, depth = 0, a = 0;
        bool erase = false;
    };
    int n;
    struct Test { int d; unsigned mask; };
    vector<Test> tests;
    unsigned independent = 0;
    unordered_map<uint64_t, Decision> memo;

    Decision plan(unsigned possible, unsigned hit) {
        int size = popcount(possible);
        if (size <= 1) return {};
        uint64_t key = (uint64_t(possible) << n) | hit;
        if (auto it = memo.find(key); it != memo.end()) return it->second;
        Decision best{INT_MAX, INT_MAX};
        int levels = bit_width(unsigned(size - 1));
        int lowerBound = size * (levels + 1) - (1 << levels);
        unsigned triedIndependent = 0;
        for (int a = 0; a < int(tests.size()); ++a) {
            auto [d, mask] = tests[a];
            // A prime above n/2 admits only its own singleton test and A 1.
            // Such primes with identical hit status have interchangeable actions.
            if (mask & independent && d != 1) {
                if (!(mask & possible)) continue;
                unsigned type = mask & hit ? 2 : 1;
                if (triedIndependent & type) continue;
                triedIndependent |= type;
            }
            unsigned yes = hit & mask, no = possible ^ yes;
            if (!no) continue;
            Decision positive = plan(yes, yes);
            for (int erase = 0; erase <= 1; ++erase) {
                unsigned newHit = no & hit;
                if (erase) {
                    if (d == 1 || !(no & mask)) continue;
                    newHit |= no & mask;
                } else if (!yes) {
                    continue;
                }
                Decision negative = plan(no, newHit);
                int cost = size + positive.cost + negative.cost;
                int depth = 1 + max(positive.depth, negative.depth);
                if (tie(cost, depth) < tie(best.cost, best.depth))
                    best = {cost, depth, a, bool(erase)};
                if (best.cost == lowerBound && best.depth == levels)
                    return memo[key] = best;
            }
        }
        return memo[key] = best;
    }

public:
    explicit SmallSearch(int n) : n(n) {
        for (int a = 1; a <= n; ++a) {
            unsigned mask = 0;
            for (int x = a; x <= n; x += a) mask |= 1u << (x - 1);
            tests.push_back({a, mask});
            if (a <= n / 2 || a == 1) continue;
            bool prime = true;
            for (int p = 2; p * p <= a; ++p) prime &= a % p != 0;
            if (prime) independent |= mask;
        }
    }

    SmallSearch(const vector<int> &values, const vector<vector<int>> &divisors)
        : n(values.size()) {
        unsigned full = (1u << n) - 1;
        unordered_set<unsigned> seen;
        for (int x : values) {
            for (int d : divisors[x]) {
                unsigned mask = 0;
                for (int i = 0; i < n; ++i)
                    if (values[i] % d == 0) mask |= 1u << i;
                if (mask != full && seen.insert(min(mask, full ^ mask)).second)
                    tests.push_back({d, mask});
            }
        }
    }

    template<class OutsideCount>
    int locate(bool covered, OutsideCount outsideCount) {
        unsigned possible = (1u << n) - 1, ordinary = covered ? 0 : possible;
        while (popcount(possible) > 1) {
            auto decision = plan(possible, possible & ~ordinary);
            auto [d, mask] = tests[decision.a];
            int baseline = outsideCount(d) + popcount(ordinary & mask);
            int extra = ask(decision.erase ? 'B' : 'A', d) - baseline;
            if (extra != 0 && extra != 1) exit(0);
            unsigned yes = possible & ~ordinary & mask;
            possible = extra ? yes : possible ^ yes;
            if (decision.erase) ordinary &= ~mask;
        }
        return countr_zero(possible);
    }
};

class PrimeSearch {
    int n;
    vector<int> primes, smallest, second, indexOf, weight, batchEnd;

    void sieve() {
        smallest.assign(n + 1, n + 1);
        second.assign(n + 1, n + 1);
        indexOf.resize(n + 1);
        for (int p = 2; p <= n; ++p) {
            if (smallest[p] != n + 1) continue;
            indexOf[p] = primes.size();
            primes.push_back(p);
            int removed = 0;
            for (int x = p; x <= n; x += p) {
                if (smallest[x] == n + 1) {
                    smallest[x] = p;
                    ++removed;
                } else if (second[x] == n + 1) {
                    second[x] = p;
                }
            }
            weight.push_back(removed);
        }
    }

    // Choose checkpoint positions by minimizing a query cost averaged over
    // the possible integers. Beyond sqrt(n) this is an exact batch cost:
    // one deletion per prime, one checkpoint, then a singleton search on a hit.
    void schedule() {
        int m = primes.size(), small = 0;
        while (small < m && primes[small] <= n / primes[small]) ++small;
        int limit = 2 * int(sqrt(m)) + 10;
        vector<int> remaining(m + 1, 1);
        vector<long long> cost(m + 1);
        batchEnd.resize(m);
        for (int i = m - 1; i >= 0; --i)
            remaining[i] = remaining[i + 1] + weight[i];

        // Only the first 'limit' primes after a small prime can be in its
        // batch. Summarize the second-factor signals in that window.
        int window = min(m, small + limit);
        vector<vector<int>> signals(small, vector<int>(window));
        vector<vector<int>> credit(small, vector<int>(window + 1));
        for (int x = 2; x <= n; ++x) {
            int p = smallest[x], i = indexOf[p];
            if (i >= small) continue;
            int q = second[x] > n ? m : indexOf[second[x]];
            if (q < window) ++signals[i][q];
            int rest = x;
            do { rest /= p; } while (rest % p == 0);
            int bound = n / (x / rest);
            int useful = upper_bound(primes.begin(), primes.begin() + window, bound)
                         - primes.begin() - 1;
            // A deletion through the second factor can replace a later
            // divisibility test in trial division once p's power is known.
            int last = min(q, useful);
            if (last > i) ++credit[i][last];
        }
        for (auto &row : credit)
            for (int j = window - 1; j > 0; --j) row[j - 1] += row[j];

        for (int i = m - 1; i >= 0; --i) {
            long long best = LLONG_MAX, work = 0, singleSearch = 0;
            int onPath = remaining[i];
            for (int j = i; j < m && j - i < limit; ++j) {
                int k = j - i + 1;
                work += onPath;
                singleSearch += 1LL * k * weight[j];
                // If B gives an extra count, the first prime is one of
                // the earlier primes in this batch. Charge a linear search
                // over the feasible first primes, inferring the final one.
                int origins = 0, lastWeight = 0;
                for (int t = i; t < min(j, small); ++t) {
                    work -= credit[t][j];
                    int w = signals[t][j];
                    if (!w) continue;
                    onPath -= w;
                    singleSearch -= 1LL * (t - i + 1) * w;
                    work += 1LL * ++origins * w;
                    lastWeight = w;
                }
                work -= lastWeight;
                long long estimate = work + onPath + singleSearch - weight[j] + cost[j + 1];
                if (estimate < best) {
                    best = estimate;
                    batchEnd[i] = j;
                }
            }
            cost[i] = best;
        }
    }

    int ordinaryCount(int d, int cutoff) const {
        if (smallest[d] <= cutoff) return 0;
        int count = 0;
        for (int x = d; x <= n; x += d)
            count += smallest[x] > cutoff;
        return count;
    }

    int extra(char op, int a, int ordinary) const {
        int result = ask(op, a) - ordinary;
        if (result != 0 && result != 1) exit(0);
        return result;
    }

    // All these candidates have been deleted in the ordinary sieve. Each A d
    // now tests x % d == 0. Pick the divisor that most evenly splits them.
    int identify(vector<int> candidates, int cutoff) const {
        if (candidates.empty()) exit(0);
        if (candidates.size() == 1) return candidates[0];
        if (all_of(candidates.begin(), candidates.end(),
                   [&](int v) { return smallest[v] == v; })) {
            for (size_t i = 0; i + 1 < candidates.size(); ++i)
                if (extra('A', candidates[i], 0)) return candidates[i];
            return candidates.back();
        }
        vector<vector<int>> divisors(n + 1);
        vector<int> count(n + 1), probes;
        for (int x : candidates) {
            auto &ds = divisors[x];
            ds.push_back(1);
            for (int v = x; v > 1; ) {
                int p = smallest[v], power = 1, baseSize = ds.size();
                do {
                    v /= p;
                    power *= p;
                    for (int i = 0; i < baseSize; ++i) ds.push_back(ds[i] * power);
                } while (v % p == 0);
            }
            for (int d : ds)
                if (count[d]++ == 0) probes.push_back(d);
        }
        sort(probes.begin(), probes.end());
        while (true) {
            int size = candidates.size(), split = 1, probe = 0;
            // A partition with at most one item on either side can never
            // regain a larger split as the candidate set shrinks.
            erase_if(probes, [&](int d) {
                int score = min(count[d], size - count[d]);
                if (score > split) {
                    split = score;
                    probe = d;
                }
                return score <= 1;
            });
            if (!probe) break;
            if (size <= 14) {
                SmallSearch search(candidates, divisors);
                int index = search.locate(true, [&](int d) { return ordinaryCount(d, cutoff); });
                return candidates[index];
            }
            int divides = extra('A', probe, ordinaryCount(probe, cutoff));
            erase_if(candidates, [&](int x) {
                if ((x % probe == 0) == divides) return false;
                for (int d : divisors[x]) --count[d];
                return true;
            });
        }

        // Only singleton tests remain. Try individually identifiable values
        // first, then the others in descending order, inferring the last.
        auto tail = stable_partition(candidates.begin(), candidates.end(),
                                     [&](int v) { return count[v] == 1; });
        reverse(tail, candidates.end());
        for (size_t i = 0; i + 1 < candidates.size(); ++i)
            if (extra('A', candidates[i], 0)) return candidates[i];
        return candidates.back();
    }

public:
    explicit PrimeSearch(int n) : n(n) {
        sieve();
        schedule();
    }

    int solve() const {
        int ordinary = n, m = primes.size();
        for (int i = 0; i < m; ) {
            int end = batchEnd[i], first = primes[i];
            for (int j = i; j <= end; ++j) {
                int p = primes[j];
                int hit = extra('B', p, weight[j]);
                ordinary -= weight[j];
                if (hit) {
                    vector<int> candidates;
                    for (int x = p; x <= n; x += p)
                        if (smallest[x] >= first && second[x] == p)
                            candidates.push_back(x);
                    return identify(move(candidates), p);
                }
            }
            if (extra('A', 1, ordinary)) {
                int last = primes[end];
                vector<int> candidates;
                for (int x = first; x <= n; ++x)
                    if (first <= smallest[x] && smallest[x] <= last && second[x] > last)
                        candidates.push_back(x);
                return identify(move(candidates), last);
            }
            i = end + 1;
        }
        return 1;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 1 || n > 100000) return 0;
    int x = n <= 20 ? 1 + SmallSearch(n).locate(false, [](int) { return 0; })
                    : PrimeSearch(n).solve();
    cout << "C " << x << endl;
}

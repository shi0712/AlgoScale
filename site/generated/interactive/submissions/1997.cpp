#include <bits/stdc++.h>
using namespace std;

using Mask = unsigned;

struct Cost {
    int total;
    int worst;

    auto operator<=>(const Cost&) const = default;
};

struct Question {
    int modulus = 0;
    Mask multiples = 0;
    bool deletes = false;
};

struct Decision {
    Cost cost{INT_MAX, INT_MAX};
    Question question;
};

Cost combine(int size, Cost yes, Cost no) {
    return {size + yes.total + no.total, 1 + max(yes.worst, no.worst)};
}

// Once x has survived a deletion, each question is a divisibility test.
class SmallDivider {
    vector<Question> questions;
    vector<Decision> memo;

public:
    SmallDivider(int size, vector<Question> queries)
        : questions(move(queries)), memo(Mask{1} << size) {}

    Decision best(Mask candidates) {
        int size = popcount(candidates);
        if (size <= 1) return {{0, 0}, {}};
        Decision& answer = memo[candidates];
        if (answer.question.modulus) return answer;

        for (const Question& question : questions) {
            Mask yes = candidates & question.multiples;
            Mask no = candidates ^ yes;
            if (!yes || !no) continue;
            Cost cost = combine(size, best(yes).cost, best(no).cost);
            if (cost < answer.cost) answer = {cost, question};
        }
        return answer;
    }

    template <class Test>
    int locate(const vector<int>& values, Mask candidates, Test test) {
        while (!has_single_bit(candidates)) {
            Question question = best(candidates).question;
            if (test(question.modulus)) candidates &= question.multiples;
            else candidates &= ~question.multiples;
        }
        return values[countr_zero(candidates)];
    }
};

class Solver {
    static constexpr int initialExactLimit = 18;
    static constexpr int protectedExactLimit = 13;

    int n;
    int ordinarySize;
    vector<unsigned char> ordinary;
    vector<int> smallestPrime;
    vector<int> primes;

    // The real set is the ordinary set, plus x if a deletion would remove x.
    bool ask(char type, int a, int expected) const {
        cout << type << ' ' << a << endl;
        int reply;
        if (!(cin >> reply) || reply < expected || reply > expected + 1) exit(0);
        return reply != expected;
    }

    bool askA(int a) const {
        if (a == 1) return ask('A', 1, ordinarySize);
        int expected = 0;
        for (int x = a; x <= n; x += a) expected += ordinary[x];
        return ask('A', a, expected);
    }

    void fillDivisors(int x, vector<int>& divisors) const {
        divisors.clear();
        divisors.push_back(1);
        while (x > 1) {
            int p = smallestPrime[x];
            int previousSize = static_cast<int>(divisors.size());
            int power = 1;
            do {
                x /= p;
                power *= p;
                for (int i = 0; i < previousSize; ++i) {
                    divisors.push_back(divisors[i] * power);
                }
            } while (smallestPrime[x] == p);
        }
    }

    int solveSmall() const {
        vector<int> values(n);
        iota(values.begin(), values.end(), 1);
        vector<Mask> multiples(n + 1);
        vector<Question> tests;
        for (int a = 1; a <= n; ++a) {
            for (int x = a; x <= n; x += a) multiples[a] |= Mask{1} << (x - 1);
            if (a > 1) tests.push_back({a, multiples[a]});
        }

        SmallDivider divider(n, move(tests));
        unordered_map<uint64_t, Decision> memo;
        // Until a reply reveals x outside live, every member of live is possible.
        auto best = [&](auto&& self, Mask live, Mask pending) -> Decision {
            int size = popcount(live | pending);
            if (size == 1) return {{0, 0}, {}};
            uint64_t key = live | (uint64_t{pending} << n);
            auto found = memo.find(key);
            if (found != memo.end()) return found->second;

            Decision answer;
            for (int a = 1; a <= n; ++a) {
                Mask yes = pending & multiples[a];
                Mask untested = pending ^ yes;
                Cost yesCost = divider.best(yes).cost;
                if (yes) {
                    Cost cost = combine(size, yesCost, self(self, live, untested).cost);
                    if (cost < answer.cost) answer = {cost, {a, multiples[a], false}};
                }
                Mask removed = live & multiples[a];
                if (a > 1 && removed) {
                    Cost noCost = self(self, live ^ removed, untested | removed).cost;
                    Cost cost = combine(size, yesCost, noCost);
                    if (cost < answer.cost) answer = {cost, {a, multiples[a], true}};
                }
            }
            memo.emplace(key, answer);
            return answer;
        };

        Mask live = (Mask{1} << n) - 1;
        Mask pending = 0;
        while (!has_single_bit(live | pending)) {
            Question question = best(best, live, pending).question;
            Mask removed = live & question.multiples;
            bool yes = ask(question.deletes ? 'B' : 'A', question.modulus, popcount(removed));
            if (question.deletes) live ^= removed;
            if (yes) {
                return divider.locate(values, pending & question.multiples, [&](int a) {
                    return ask('A', a, popcount(live & multiples[a]));
                });
            }
            pending &= ~question.multiples;
            if (question.deletes) pending |= removed;
        }
        return values[countr_zero(live | pending)];
    }

    vector<int> planBatches() const {
        int count = static_cast<int>(primes.size());
        vector<int> frequency(n + 1);
        for (int x = 2; x <= n; ++x) ++frequency[smallestPrime[x]];
        vector<int> prefix(count + 1);
        for (int i = 0; i < count; ++i) prefix[i + 1] = prefix[i] + frequency[primes[i]];

        vector<int> end(count);
        vector<long long> extraCost(count + 1);
        vector<int> worstCost(count + 1);
        // Optimize the batch overhead for a uniformly chosen x, modeling each
        // least-prime-factor group as one item. The prime-only suffix fits
        // this model exactly. A positive B reply can also end a batch early.
        for (int i = count - 1; i >= 0; --i) {
            pair<long long, int> best{LLONG_MAX, INT_MAX};
            int remaining = n - prefix[i];
            for (int j = i + 1; j <= count; ++j) {
                int length = j - i;
                int weight = prefix[j] - prefix[i];
                if (remaining + 1LL * (length - 1) * weight > best.first) break;

                // Charge the checkpoint to every remaining x. For a target
                // in the batch, waiting and locating add length operations,
                // except that the final group needs one fewer test.
                long long cost = remaining + 1LL * length * weight
                                 - frequency[primes[j - 1]] + extraCost[j];
                int worst = max(2 * length, length + 1 + worstCost[j]);
                if (pair{cost, worst} < best) {
                    best = {cost, worst};
                    end[i] = j;
                }
            }
            tie(extraCost[i], worstCost[i]) = best;
        }
        return end;
    }

    int finishExact(const vector<int>& candidates, vector<int>& divisors) const {
        int size = static_cast<int>(candidates.size());
        Mask all = (Mask{1} << size) - 1;
        vector<Mask> multiples(n + 1);
        for (int i = 0; i < size; ++i) {
            fillDivisors(candidates[i], divisors);
            for (int a : divisors) multiples[a] |= Mask{1} << i;
        }

        vector<unsigned char> used(all + 1);
        vector<Question> tests;
        for (int a = 2; a <= n; ++a) {
            Mask yes = multiples[a];
            if (!yes || yes == all) continue;
            Mask split = min(yes, all ^ yes);
            if (used[split]) continue;
            used[split] = true;
            tests.push_back({a, yes});
        }
        SmallDivider divider(size, move(tests));
        return divider.locate(candidates, all, [&](int a) { return askA(a); });
    }

    int finishSingletons(const vector<int>& candidates, const vector<int>& hits) const {
        int remaining = static_cast<int>(candidates.size());
        vector<int> deferred;
        for (int x : candidates) {
            if (hits[x] != 1) {
                deferred.push_back(x);
                continue;
            }
            if (remaining == 1 || askA(x)) return x;
            --remaining;
        }
        // The largest remaining candidate always has a singleton test.
        while (deferred.size() > 1) {
            if (askA(deferred.back())) return deferred.back();
            deferred.pop_back();
        }
        return deferred.front();
    }

    int finishProtected(vector<int> candidates) const {
        if (candidates.size() == 1) return candidates.front();
        sort(candidates.begin(), candidates.end());
        vector<int> divisors;
        divisors.reserve(128);
        if (candidates.size() <= protectedExactLimit) return finishExact(candidates, divisors);

        vector<int> hits(n + 1);
        for (int x : candidates) {
            fillDivisors(x, divisors);
            for (int a : divisors) ++hits[a];
        }
        set<pair<int, int>> splits;
        for (int a = 2; a <= n; ++a) {
            if (hits[a] >= 2) splits.emplace(hits[a], a);
        }

        while (candidates.size() > protectedExactLimit) {
            int size = static_cast<int>(candidates.size());
            auto upper = splits.lower_bound({(size + 1) / 2, 0});
            pair<int, int> best{size - 2, 0};
            auto consider = [&](set<pair<int, int>>::iterator it) {
                if (it != splits.end()) {
                    auto [count, a] = *it;
                    best = min(best, pair{abs(size - 2 * count), a});
                }
            };
            consider(upper);
            if (upper != splits.begin()) {
                consider(splits.lower_bound({prev(upper)->first, 0}));
            }
            int a = best.second;
            if (!a) return finishSingletons(candidates, hits);

            bool yes = askA(a);
            auto keepEnd = remove_if(candidates.begin(), candidates.end(), [&](int x) {
                if ((x % a == 0) == yes) return false;
                fillDivisors(x, divisors);
                for (int divisor : divisors) {
                    if (divisor == 1) continue;
                    int old = hits[divisor]--;
                    if (old == 2) {
                        splits.erase({old, divisor});
                    } else if (old > 2) {
                        auto node = splits.extract({old, divisor});
                        node.value().first = old - 1;
                        splits.insert(move(node));
                    }
                }
                return true;
            });
            candidates.erase(keepEnd, candidates.end());
        }
        return finishExact(candidates, divisors);
    }

public:
    explicit Solver(int n) : n(n), ordinarySize(n), ordinary(n + 1, true), smallestPrime(n + 1) {}

    int solve() {
        if (n <= initialExactLimit) return solveSmall();

        for (int p = 2; p <= n; ++p) {
            if (smallestPrime[p]) continue;
            primes.push_back(p);
            for (int x = p; x <= n; x += p) {
                if (!smallestPrime[x]) smallestPrime[x] = p;
            }
        }

        vector<int> end = planBatches();
        vector<unsigned char> eligible(n + 1);
        vector<int> pending;
        vector<int> overlap;
        int count = static_cast<int>(primes.size());
        for (int start = 0; start < count; start = end[start]) {
            pending.clear();
            for (int j = start; j < end[start]; ++j) {
                int p = primes[j];
                int removed = 0;
                overlap.clear();
                for (int x = p; x <= n; x += p) {
                    if (ordinary[x]) {
                        ++removed;
                        ordinary[x] = false;
                        pending.push_back(x);
                        eligible[x] = true;
                    } else if (eligible[x]) {
                        overlap.push_back(x);
                        eligible[x] = false;
                    }
                }
                ordinarySize -= removed;
                if (ask('B', p, removed)) return finishProtected(move(overlap));
            }

            if (askA(1)) {
                erase_if(pending, [&](int x) { return !eligible[x]; });
                return finishProtected(move(pending));
            }
            for (int x : pending) eligible[x] = false;
        }
        return 1;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1 || n > 100000) return 0;
    Solver solver(n);
    int answer = solver.solve();
    cout << 'C' << ' ' << answer << endl;
}

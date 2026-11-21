#include <bits/stdc++.h>
using namespace std;

int ask(char type, int a) {
    cout << type << ' ' << a << '\n' << flush;
    int reply;
    if (!(cin >> reply) || reply < 0) exit(0);
    return reply;
}

// The judge's set is now the ordinary sieve's set plus x.
// Enumerate the candidates consistent with the first detected discrepancy.
int refine(int n, const vector<int>& spf, const vector<int>& primes,
           int first, int last, bool detectedByB) {
    vector<int> candidates;
    const int otherFactorLimit = last - int(detectedByB);

    for (int v = 2; v <= n; ++v) {
        int p = spf[v];
        if (p < first || p > otherFactorLimit) continue;
        if (detectedByB && v % last != 0) continue;

        int rest = v;
        do {
            rest /= p;
        } while (rest % p == 0);

        // A second distinct prime in this range would have caused
        // an earlier B discrepancy.
        if (rest > 1 && spf[rest] <= otherFactorLimit) continue;
        candidates.push_back(v);
    }

    if (candidates.size() == 1) return candidates.front();

    vector<int> count(n + 1, 0), powers;
    auto update = [&](int v, int delta) {
        while (v > 1) {
            int p = spf[v], power = 1;
            do {
                v /= p;
                power *= p;
                count[power] += delta;
            } while (v % p == 0);
        }
    };

    for (int v : candidates) update(v, 1);
    for (int p : primes) {
        for (long long power = p; power <= n; power *= p) {
            if (count[power] != 0) powers.push_back(int(power));
        }
    }
    sort(powers.begin(), powers.end());

    while (candidates.size() > 1) {
        const int total = int(candidates.size());
        int best = 0, bestSplit = 0, keptPowers = 0;
        for (int a : powers) {
            int yes = count[a];
            if (yes == 0 || yes == total) continue;
            powers[keptPowers++] = a;
            int split = min(yes, total - yes);
            if (split > bestSplit) {
                bestSplit = split;
                best = a;
            }
        }
        powers.resize(keptPowers);

        int baseline = 0;
        if (spf[best] > last) {
            for (int v = best; v <= n; v += best) {
                baseline += (spf[v] > last);
            }
        }

        bool divides = ask('A', best) > baseline;
        int kept = 0;
        for (int v : candidates) {
            if ((v % best == 0) == divides) {
                candidates[kept++] = v;
            } else {
                update(v, -1);
            }
        }
        candidates.resize(kept);
    }
    return candidates.front();
}

int findLarge(const vector<int>& primes, int first) {
    const int total = int(primes.size()) - first;
    if (total == 0) return 1;

    // With m primes and t extra queries available, a batch of k
    // costs at most 2*k if it hits. A miss leaves (m-k, t-1).
    int needed = 0, reachable = 0;
    while (reachable < total) {
        ++needed;
        reachable = min(total, 2 * reachable + needed);
    }
    const int budget = max(needed, int(ceil(sqrt(double(total)) / 2.0)));

    vector<vector<int>> take(budget + 1, vector<int>(total + 1));
    vector<int> previous(total + 1), current(total + 1);
    int previousLimit = 0;

    // Minimize the sum of queries over the remaining primes and x=1,
    // subject to the worst-case bound. Subtracting the fixed cost of
    // reaching each prime gives transition m + (m-j)^2 + dp[j].
    // The squared term gives monotone minimizing indices j.
    for (int t = 1; t <= budget; ++t) {
        const int limit = min(total, 2 * previousLimit + t);
        current[0] = 0;
        auto build = [&](auto&& self, int lo, int hi, int optLo, int optHi) -> void {
            if (lo > hi) return;
            int m = (lo + hi) / 2;
            int from = max(optLo, max(0, (m - t + 1) / 2));
            int to = min(optHi, m - 1);
            int best = INT_MAX, bestJ = from;
            for (int j = from; j <= to; ++j) {
                int k = m - j;
                int cost = k * k + previous[j];
                if (cost < best) {
                    best = cost;
                    bestJ = j;
                }
            }
            current[m] = m + best;
            take[t][m] = m - bestJ;
            self(self, lo, m - 1, optLo, bestJ);
            self(self, m + 1, hi, bestJ, optHi);
        };
        build(build, 1, limit, 0, previousLimit);
        previous.swap(current);
        previousLimit = limit;
    }

    int left = total, t = budget;
    while (left > 0) {
        int k = take[t][left];
        for (int i = first; i < first + k; ++i) {
            ask('B', primes[i]);
        }
        left -= k;

        if (ask('A', 1) > left + 1) {
            for (int i = first; i + 1 < first + k; ++i) {
                if (ask('A', primes[i]) != 0) return primes[i];
            }
            return primes[first + k - 1];
        }

        first += k;
        --t;
    }
    return 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1) return 0;

    vector<int> spf(n + 1, 0), primes, baseline(n + 1, 0);
    for (int p = 2; p <= n; ++p) {
        if (spf[p] != 0) continue;
        primes.push_back(p);
        for (int v = p; v <= n; v += p) {
            if (spf[v] == 0) spf[v] = p;
        }
    }
    for (int v = 2; v <= n; ++v) ++baseline[spf[v]];

    int small = 0;
    while (small < int(primes.size()) &&
           1LL * primes[small] * primes[small] <= n) {
        ++small;
    }

    int alive = n, answer = 0;
    for (int i = 0; i < small && answer == 0;) {
        int k = i == 0 ? 1 : int(sqrt(2.0 * primes[i]));
        if (i > 0 && small - i == k + 1) ++k;
        int end = min(small, i + k);

        for (int j = i; j < end; ++j) {
            int p = primes[j];
            int reply = ask('B', p);
            alive -= baseline[p];
            if (reply > baseline[p]) {
                answer = refine(n, spf, primes, primes[i], p, true);
                break;
            }
        }
        if (answer == 0 && ask('A', 1) > alive) {
            answer = refine(n, spf, primes, primes[i], primes[end - 1], false);
        }
        i = end;
    }

    if (answer == 0) answer = findLarge(primes, small);
    cout << "C " << answer << '\n' << flush;
    return 0;
}

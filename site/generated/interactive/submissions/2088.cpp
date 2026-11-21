#include <bits/stdc++.h>
using namespace std;

using Weights = vector<pair<int, int>>; // (zero-based vertex, multiplicity)

int query(const Weights& w) {
    int k = 0;
    for (auto [v, count] : w) k += count;
    cout << "? 1 " << k;
    for (auto [v, count] : w)
        for (int i = 0; i < count; ++i) cout << ' ' << v + 1;
    cout << endl;
    int result;
    if (!(cin >> result)) exit(0);
    return result;
}

void flip(int v) {
    cout << "? 2 " << v + 1 << endl;
}

int bit(int mask, int i) { return (mask >> i) & 1; }
int signAt(int mask, int i) { return 2 * bit(mask, i) - 1; }

// The tiny stars fit in a 64-bit set of (root, sign vector) hypotheses.
// Optimize the worst depth, then the total path length over hypotheses.
class TinyStar {
    using U = uint64_t;
    struct Action {
        Weights weights;
        vector<pair<int, U>> outcomes;
    };
    struct Entry { int cost, action; };
    static constexpr int inf = 100000;
    int n, vectors;
    vector<Action> actions;
    vector<U> byValues, flipLow;
    vector<unordered_map<U, Entry>> memo;

    void enumerate(int i, int left, Weights& w) {
        if (i < n) {
            enumerate(i + 1, left, w);
            for (int k = 1; k <= left; ++k) {
                w.emplace_back(i, k);
                enumerate(i + 1, left - k, w);
                w.pop_back();
            }
            return;
        }
        int g = 0;
        for (auto [v, k] : w) g = gcd(g, k);
        if (g != 1) return;
        map<int, U> buckets;
        for (int root = 0; root < n; ++root)
            for (int a = 0; a < vectors; ++a) {
                int center = signAt(a, 0) + (root ? signAt(a, root) : 0);
                int sum = 0;
                for (auto [v, k] : w) {
                    int f = v == root ? signAt(a, root) : center + (v ? signAt(a, v) : 0);
                    sum += k * f;
                }
                buckets[sum] |= U(1) << (root * vectors + a);
            }
        actions.push_back({w, {buckets.begin(), buckets.end()}});
    }

    U toggled(U s, int v) const {
        int shift = 1 << v;
        return ((s & flipLow[v]) << shift) | ((s & ~flipLow[v]) >> shift);
    }

    Entry best(U s, int depth) {
        int a = countr_zero(s) % vectors;
        if ((s & ~byValues[a]) == 0) return {0, -1};
        if (!depth) return {inf, -1};
        auto it = memo[depth].find(s);
        if (it != memo[depth].end()) return it->second;
        Entry ans{inf, -1};
        int count = popcount(s);
        for (int i = 0; i < (int)actions.size(); ++i) {
            int cost = count;
            for (auto [response, bucket] : actions[i].outcomes) {
                U child = s & bucket;
                if (child == s) { cost = inf; break; }
                if (child) cost += best(child, depth - 1).cost;
                if (cost >= ans.cost) break;
            }
            if (cost < ans.cost) ans = {cost, i};
            if (ans.cost == count) break;
        }
        if (depth >= 2 && ans.cost > 2 * count) {
            for (int v = 0; v < n; ++v) {
                U next = toggled(s, v);
                if (next == s) continue;
                int cost = count + best(next, depth - 1).cost;
                if (cost < ans.cost) ans = {cost, int(actions.size()) + v};
            }
        }
        memo[depth][s] = ans;
        return ans;
    }

public:
    explicit TinyStar(int size) : n(size), vectors(1 << n), byValues(vectors), flipLow(n) {}
    vector<int> solve() {
        Weights w;
        enumerate(0, n, w);
        U s = 0;
        for (int root = 0; root < n; ++root)
            for (int a = 0; a < vectors; ++a) {
                U h = U(1) << (root * vectors + a);
                s |= h;
                byValues[a] |= h;
                for (int v = 0; v < n; ++v) if (!bit(a, v)) flipLow[v] |= h;
            }
        int depth = 0;
        memo.emplace_back();
        do {
            ++depth;
            memo.emplace_back();
        } while (best(s, depth).cost == inf);
        while (true) {
            int i = best(s, depth--).action;
            if (i < 0) break;
            if (i >= (int)actions.size()) {
                int v = i - actions.size();
                flip(v);
                s = toggled(s, v);
            } else {
                int response = query(actions[i].weights);
                U next = 0;
                for (auto [y, bucket] : actions[i].outcomes)
                    if (y == response) next = s & bucket;
                if (!next) exit(0);
                s = next;
            }
        }
        int a = countr_zero(s) % vectors;
        vector<int> ans(n);
        for (int i = 0; i < n; ++i) ans[i] = signAt(a, i);
        return ans;
    }
};

// Use binary weights, then spend the remaining capacity on one more bit.
// Every result has at most two decodings, distinguished by the last bit.
struct Pack {
    vector<int> w;
    int total = 0;
    Pack(int count, int budget) {
        for (int p = 1; (int)w.size() < count && total < budget; p *= 2) {
            w.push_back(min(p, budget - total));
            total += w.back();
        }
    }
    vector<int> decode(int y) const {
        if (w.empty()) return y == 0 ? vector<int>{0} : vector<int>{};
        int high = 1 << (w.size() - 1);
        vector<int> masks;
        if (0 <= y && y < high) masks.push_back(y);
        y -= w.back();
        if (0 <= y && y < high) masks.push_back(high | y);
        return masks;
    }
};

class Star {
    int n, centerPath = 0;
    vector<int> ans;
    deque<int> pending;
    // Each representative is a real leaf. All members equal its sign times phase.
    vector<Weights> group;

    vector<int> take(int count) {
        vector<int> ids;
        while (count--) {
            ids.push_back(pending.front());
            pending.pop_front();
        }
        return ids;
    }

    void fix(int v, int sign) {
        for (auto [u, phase] : group[v]) ans[u] = phase * sign;
        group[v].clear();
    }

    void accept(const vector<int>& ids, const vector<int>& masks) {
        if (masks.empty()) exit(0);
        int a = masks.front(), b = masks.back();
        int rep = -1, repSign = 0;
        for (int j = (int)ids.size() - 1; j >= 0; --j) {
            int v = ids[j], value = signAt(a, j);
            if (bit(a ^ b, j) == 0) {
                fix(v, value);
            } else if (rep == -1) {
                rep = v;
                repSign = value;
            } else {
                for (auto [u, phase] : group[v])
                    group[rep].emplace_back(u, phase * value * repSign);
                group[v].clear();
            }
        }
        if (rep != -1) pending.push_back(rep);
    }

    pair<vector<int>, vector<int>> readPack(bool first = false) {
        int k = first ? (n % 2 ? n : n - 1) : n;
        Pack pack(pending.size(), first ? k - 1 : k);
        auto ids = take(pack.w.size());
        Weights w;
        for (int j = 0; j < (int)ids.size(); ++j) w.emplace_back(ids[j], pack.w[j]);
        if (first) w.emplace_back(0, k - pack.total);
        else k = pack.total;
        int response = query(w);
        if (first) {
            // Odd k identifies the parity of F. The center padding makes
            // response intervals disjoint for F values of the same parity.
            bool found = false;
            for (int f = -2; f <= 2; ++f) {
                int t = response - k * f + pack.total;
                if (0 <= t && t <= 2 * pack.total && t % 2 == 0) {
                    centerPath = f;
                    found = true;
                    break;
                }
            }
            if (!found) exit(0);
        }
        int y = response - k * centerPath + pack.total;
        if (y % 2) exit(0);
        auto masks = pack.decode(y / 2);
        if (masks.empty()) exit(0);
        return {ids, masks};
    }

    // Resolve an already constrained block while using the high-order part
    // of the same reply to read fresh signs. A single bit always separates
    // at least two distinct hypotheses and serves as a fallback.
    void constrained(const vector<int>& ids, vector<int> states) {
        mt19937 rng(64297);
        int m = ids.size();
        while (states.size() > 1) {
            int h = states.size();
            int varying = 0;
            for (int a : states) varying |= a ^ states.front();
            vector<int> cols;
            for (int i = 0; i < m; ++i) if (bit(varying, i)) cols.push_back(i);
            vector<int> bestW(m);
            int bestRadix = 2, bestCost = INT_MAX;
            bool complete = false;
            auto consider = [&](const vector<int>& coeff, int radix) {
                int sum = accumulate(coeff.begin(), coeff.end(), 0);
                if (sum > n) return;
                // Capacity left for a fresh packed block, with a tiebreak
                // favoring shorter messages when all remaining signs fit.
                int cost = -(min(n, (n - sum) / radix)) * (n + 1) + sum;
                if (complete && cost >= bestCost) return;
                uint64_t used = 0;
                for (int a : states) {
                    int residue = 0;
                    for (int i : cols) if (bit(a, i)) residue += coeff[i];
                    uint64_t flag = uint64_t(1) << (residue % radix);
                    if (used & flag) return;
                    used |= flag;
                }
                complete = true;
                bestW = coeff;
                bestCost = cost;
                bestRadix = radix;
            };
            for (int radix = h; radix <= min({63, n + 1, 2 * h + 6}); ++radix) {
                if (complete && -(n / radix) * (n + 1) >= bestCost) break;
                vector<int> order = cols, coeff(m);
                for (int trial = 0; trial < 120; ++trial) {
                    if (trial >= 2) shuffle(order.begin(), order.end(), rng);
                    for (int j = 0; j < (int)order.size(); ++j)
                        coeff[order[j]] = trial < 80 ? (j + (trial & 1)) % radix : int(rng() % radix);
                    consider(coeff, radix);
                }
            }
            if (!complete) {
                int bestBalance = -1;
                for (int i : cols) {
                    int ones = 0;
                    for (int a : states) ones += bit(a, i);
                    if (min(ones, h - ones) > bestBalance) {
                        bestBalance = min(ones, h - ones);
                        fill(bestW.begin(), bestW.end(), 0);
                        bestW[i] = 1;
                    }
                }
            }
            Weights w;
            int lowSum = 0;
            for (int i : cols) if (bestW[i]) {
                w.emplace_back(ids[i], bestW[i]);
                lowSum += bestW[i];
            }
            Pack high(pending.size(), (n - lowSum) / bestRadix);
            auto fresh = take(high.w.size());
            for (int j = 0; j < (int)fresh.size(); ++j)
                w.emplace_back(fresh[j], bestRadix * high.w[j]);
            int total = lowSum + bestRadix * high.total;
            int z = query(w) + total; // centerPath is zero here
            if (z % 2) exit(0);
            int y = z / 2;
            vector<int> kept, freshMasks;
            for (int a : states) {
                int low = 0;
                for (int i : cols) if (bit(a, i)) low += bestW[i];
                if ((y - low) % bestRadix) continue;
                auto decoded = high.decode((y - low) / bestRadix);
                if (decoded.empty()) continue;
                kept.push_back(a);
                freshMasks = decoded; // unique low sum for this residue
            }
            if (kept.empty()) exit(0);
            states = move(kept);
            accept(fresh, freshMasks);
        }
        for (int j = 0; j < m; ++j) fix(ids[j], signAt(states[0], j));
    }

    // F = 2c. Before a center flip the root alone has an incorrect
    // observed leaf sign (-c in place of c). After the flip F is zero.
    void nonzeroLeafRoot(vector<int> ids, vector<int> masks) {
        int c = centerPath / 2, earlier = -1;
        for (;;) {
            if (earlier != -1) {
                for (int& a : masks) a |= int(-c == 1) << ids.size();
                ids.push_back(earlier);
            }
            if (masks.size() == 1 && !pending.empty()) {
                vector<int> candidates;
                for (int j = 0; j < (int)ids.size(); ++j)
                    if (signAt(masks[0], j) == -c) candidates.push_back(ids[j]);
                if (candidates.size() <= 1) {
                    earlier = candidates.empty() ? -1 : candidates[0];
                    for (int v : ids) if (v != earlier) fix(v, c);
                    tie(ids, masks) = readPack();
                    continue;
                }
            }
            vector<int> actual;
            for (int a : masks) {
                if (!pending.empty()) actual.push_back(a); // root outside
                for (int j = 0; j < (int)ids.size(); ++j)
                    if (signAt(a, j) == -c) actual.push_back(a ^ (1 << j));
            }
            sort(actual.begin(), actual.end());
            actual.erase(unique(actual.begin(), actual.end()), actual.end());
            if (actual.empty()) exit(0);
            if (pending.empty() && actual.size() == 1) {
                ans[0] = c;
                for (int j = 0; j < (int)ids.size(); ++j) fix(ids[j], signAt(actual[0], j));
                return;
            }
            flip(0);
            ans[0] = -c;
            centerPath = 0;
            constrained(ids, move(actual));
            return;
        }
    }

    // A merged bit with two equal-phase leaves can tolerate the root's
    // single exceptional observation after a flip. Read such pairs in base 3.
    bool finishPairs() {
        int mult = 1;
        Weights w;
        vector<int> phases;
        for (int v : pending) {
            if (mult * 3 - 1 > n) return false;
            int first[2] = {-1, -1};
            bool found = false;
            for (auto [u, phase] : group[v]) {
                int& prev = first[phase > 0];
                if (prev >= 0) {
                    w.emplace_back(prev, mult);
                    w.emplace_back(u, mult);
                    phases.push_back(phase);
                    found = true;
                    break;
                }
                prev = u;
            }
            if (!found) return false;
            mult *= 3;
        }
        flip(0);
        int response = query(w), c = response > 0 ? 1 : -1;
        ans[0] = c;
        int k = mult - 1;
        int y = (response - k * 2 * c + k) / 2;
        for (int phase : phases) {
            int digit = y % 3;
            fix(pending.front(), phase * (digit == 1 ? c : digit - 1));
            pending.pop_front();
            y /= 3;
        }
        return true;
    }

    struct Pool {
        vector<int> ids;
        int ones = 0;
    };

    // Decode sets with a known number of positive signs. For one outlier
    // its bucket number is itself an additive query result. Otherwise split
    // a set by counting a subset. Combine independent counts in mixed radix.
    void counted(vector<Pool> pools) {
        struct Split {
            int buckets = 0, prefix = 0, scale = 1, lo = 0, radix = 1;
        };
        while (!pools.empty()) {
            erase_if(pools, [&](const Pool& p) {
                if (p.ones && p.ones != (int)p.ids.size()) return false;
                for (int v : p.ids) ans[v] = p.ones ? 1 : -1;
                return true;
            });
            if (pools.empty()) break;
            sort(pools.begin(), pools.end(), [](const Pool& a, const Pool& b) {
                return a.ids.size() > b.ids.size();
            });
            Weights w;
            vector<Split> splits;
            int scale = 1, cost = 0, offset = 0;
            for (const Pool& p : pools) {
                int cap = (n - cost) / scale, len = p.ids.size();
                if (!cap) break;
                Split s;
                s.scale = scale;
                if (min(p.ones, len - p.ones) == 1) {
                    int low = 1, high = len;
                    while (low < high) {
                        int b = (low + high + 1) / 2;
                        // Sum of floor(i*b/len) for i from 0 through len-1.
                        int needed = (len * b - len - b + gcd(len, b)) / 2;
                        if (needed <= cap) low = b;
                        else high = b - 1;
                    }
                    if (low >= 2) s.buckets = low;
                }
                int sum = 0;
                if (s.buckets) {
                    s.radix = s.buckets;
                    for (int j = 0; j < len; ++j) {
                        int weight = j * s.buckets / len;
                        if (weight) w.emplace_back(p.ids[j], scale * weight);
                        sum += weight;
                    }
                    if (p.ones != 1) s.lo = sum - (s.radix - 1);
                } else {
                    s.prefix = min(cap, len / 2);
                    sum = s.prefix;
                    s.lo = max(0, p.ones - (len - s.prefix));
                    s.radix = min(s.prefix, p.ones) - s.lo + 1;
                    for (int j = 0; j < s.prefix; ++j) w.emplace_back(p.ids[j], scale);
                }
                offset += scale * s.lo;
                cost += scale * sum;
                scale *= s.radix;
                splits.push_back(s);
            }
            int y = (query(w) - cost * centerPath + cost) / 2 - offset;
            vector<Pool> next;
            for (int i = 0; i < (int)pools.size(); ++i) {
                auto& p = pools[i];
                if (i >= (int)splits.size()) { next.push_back(move(p)); continue; }
                const Split& s = splits[i];
                int digit = (y / s.scale) % s.radix;
                if (s.buckets) {
                    bool positive = p.ones == 1;
                    int bucket = positive ? digit : s.radix - 1 - digit;
                    vector<int> kept;
                    for (int j = 0; j < (int)p.ids.size(); ++j) {
                        if (j * s.buckets / int(p.ids.size()) == bucket) kept.push_back(p.ids[j]);
                        else ans[p.ids[j]] = positive ? -1 : 1;
                    }
                    int ones = positive ? 1 : int(kept.size()) - 1;
                    next.push_back({move(kept), ones});
                } else {
                    int ones = s.lo + digit;
                    auto mid = p.ids.begin() + s.prefix;
                    next.push_back({{mid, p.ids.end()}, p.ones - ones});
                    p.ids.resize(s.prefix);
                    next.push_back({move(p.ids), ones});
                }
            }
            pools = move(next);
        }
    }

    bool tryPooled(const vector<int>& ids, const vector<int>& masks) {
        int m = ids.size();
        if ((int)pending.size() < 3 * m || m < 5) return false;
        // Use the first batch as a sample. Pool nearly uniform signs or
        // positions that follow a short period, and verify by exact counts.
        int sample = masks.front(), period = 0, minority = m;
        auto inspect = [&](int pattern) {
            int ones = popcount(unsigned(pattern));
            minority = min(minority, min(ones, m - ones));
            for (int p = 1; 3 * p <= m && (!period || p < period); ++p) {
                bool matches = true;
                for (int j = p; j < m; ++j)
                    matches &= bit(pattern, j) == bit(pattern, j - p);
                if (matches) { period = p; sample = pattern; break; }
            }
        };
        for (int a : masks) {
            inspect(a);
            if (abs(centerPath) == 2)
                for (int j = 0; j < m; ++j)
                    if (signAt(a, j) == -centerPath / 2) inspect(a ^ (1 << j));
        }
        if (minority > int(m >= 7) && !period) return false;

        vector<Pool> pools(2);
        for (int v : pending) {
            int side = period ? bit(sample, (v - 1) % period) : 0;
            pools[side].ids.push_back(v);
        }
        auto tally = [&](const vector<int>& leaves) {
            Weights w;
            for (int v : leaves) w.emplace_back(v, 1);
            int k = leaves.size();
            return (query(w) - k * centerPath + k) / 2;
        };
        int exceptions = 0, tailOnes = 0;
        for (auto& p : pools) {
            int len = p.ids.size();
            if (!len) continue;
            p.ones = tally(p.ids);
            tailOnes += p.ones;
            exceptions += min(p.ones, len - p.ones);
            if (exceptions > n / 24 && (period != 1 || m < 6)) return false;
        }
        // A uniform initial run can precede a run of the opposite sign.
        // The population count gives a split at which both sides can be
        // checked with one additional measurement.
        if (exceptions > n / 24) {
            Pool& p = pools[bit(sample, 0)];
            int len = p.ids.size();
            int prefix = bit(sample, 0) ? p.ones : len - p.ones;
            vector<int> head(p.ids.begin(), p.ids.begin() + prefix);
            int positives = tally(head);
            int suffixOnes = p.ones - positives;
            if (min(positives, prefix - positives) +
                min(suffixOnes, len - prefix - suffixOnes) > n / 24) return false;
            p.ids.erase(p.ids.begin(), p.ids.begin() + prefix);
            p.ones = suffixOnes;
            pools.push_back({move(head), positives});
        }
        // The trimmed binary weight can leave two sample assignments.
        // Once pooling succeeds, one actual leaf distinguishes them.
        sample = masks.front();
        if (masks.size() == 2) {
            int j = countr_zero(unsigned(masks[0] ^ masks[1]));
            int d = query({{ids[j], 1}}) - centerPath;
            sample = signAt(masks[0], j) == d ? masks[0] : masks[1];
        }
        int totalOnes = tailOnes + popcount(unsigned(sample));
        if (abs(centerPath) == 2) {
            int c = centerPath / 2;
            int candidates = c == 1 ? n - 1 - totalOnes : totalOnes;
            if (candidates == 1) {
                fill(ans.begin(), ans.end(), c);
                return true;
            }
            if (count_if(pools.begin(), pools.end(), [](const Pool& p) { return !p.ids.empty(); }) == 1) {
                // The center flip corrects exactly one apparent -c to c.
                // Keep every leaf already known to be c out of the search.
                vector<int> unknown;
                int remainingOnes = totalOnes + c;
                for (int j = 0; j < m; ++j) {
                    if (signAt(sample, j) == c) { ans[ids[j]] = c; remainingOnes -= int(c == 1); }
                    else unknown.push_back(ids[j]);
                }
                for (auto& p : pools) {
                    bool certain = c == 1 ? p.ones == int(p.ids.size()) : p.ones == 0;
                    if (certain) {
                        for (int v : p.ids) ans[v] = c;
                        if (c == 1) remainingOnes -= p.ids.size();
                    } else unknown.insert(unknown.end(), p.ids.begin(), p.ids.end());
                }
                flip(0);
                centerPath = 0;
                ans[0] = -c;
                counted({{move(unknown), remainingOnes}});
                return true;
            }
        }
        for (int j = 0; j < m; ++j) ans[ids[j]] = signAt(sample, j);
        counted(move(pools));
        if (abs(centerPath) == 1) {
            ans[0] = centerPath;
        } else if (centerPath == 0) {
            if (totalOnes == 0 || totalOnes == n - 1) ans[0] = totalOnes ? -1 : 1;
            else { flip(0); ans[0] = query({{0, 1}}) / 2; }
        } else {
            int c = centerPath / 2;
            vector<int> possible;
            for (int v = 1; v < n; ++v) if (ans[v] == -c) possible.push_back(v);
            flip(0);
            ans[0] = -c;
            centerPath = 0;
            int positive = c == 1 ? 1 : int(possible.size()) - 1;
            counted({{move(possible), positive}});
        }
        return true;
    }

    bool uniform(int sign) const {
        return all_of(ans.begin() + 1, ans.end(), [=](int v) { return v == sign; });
    }

    // Two measurements straddle the center flip. A checksum modulo q+1
    // identifies which of the last q queried leaves, if any, was the root.
    bool finishChecksum() {
        int r = pending.size(), q = 0;
        for (int i = 1; (1 << i) - 1 <= n && i <= r; ++i) {
            int a = r - i;
            if (a > 10) continue;
            if ((i + 1) * ((1 << a) - 1) + i * (i + 1) / 2 <= n) q = i;
        }
        if (!q) return false;
        int a = r - q, radix = q + 1;
        auto head = take(a), tail = take(q);
        Weights w;
        int total = 0;
        for (int j = 0; j < a; ++j) w.emplace_back(head[j], radix << j);
        for (int j = 0; j < q; ++j) w.emplace_back(tail[j], j + 1);
        for (auto [v, weight] : w) total += weight;
        int y = (query(w) + total) / 2;
        if (y == 0 || y == total) {
            int sign = y ? 1 : -1;
            for (int v : head) fix(v, sign);
            for (int v : tail) fix(v, sign);
            if (uniform(sign)) { ans[0] = -sign; return true; }
            flip(0);
            ans[0] = query({{0, 1}}) / 2;
            return true;
        }

        w.clear();
        for (int j = 0; j < q; ++j) w.emplace_back(tail[j], 1 << j);
        flip(0);
        int response = query(w);
        int c = response > 0 ? 1 : -1;
        ans[0] = c;
        int k = (1 << q) - 1, mask = (response - 2 * c * k + k) / 2;
        int checksum = 0;
        for (int j = 0; j < q; ++j) checksum += bit(mask, j) * (j + 1);
        int correction = ((c * (y - checksum)) % radix + radix) % radix;
        if (correction) {
            mask ^= 1 << (correction - 1);
            checksum += c * correction;
        }
        accept(head, {(y - checksum) / radix});
        accept(tail, {mask});
        return true;
    }

    void zeroLeafRoot() {
        while (!pending.empty()) {
            if (finishPairs()) return;
            int r = pending.size();
            if (r > 1 && (r > 10 || (1 << r) - 1 > n) && finishChecksum()) return;
            auto [ids, masks] = readPack();
            accept(ids, masks);
        }
        for (int sign : {-1, 1}) if (uniform(sign)) { ans[0] = -sign; return; }
        flip(0);
        ans[0] = query({{0, 1}}) / 2;
    }

public:
    explicit Star(int size) : n(size), ans(n), group(n) {
        for (int i = 1; i < n; ++i) {
            pending.push_back(i);
            group[i].emplace_back(i, 1);
        }
    }
    vector<int> solve() {
        auto [ids, masks] = readPack(true);
        if (tryPooled(ids, masks)) return ans;
        if (abs(centerPath) == 2) {
            nonzeroLeafRoot(move(ids), move(masks));
        } else {
            accept(ids, masks);
            if (centerPath == 0) {
                zeroLeafRoot();
                return ans;
            }
            ans[0] = centerPath;
        }
        while (!pending.empty()) {
            auto [v, a] = readPack();
            accept(v, a);
        }
        return ans;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    for (int i = 1; i < n; ++i) {
        int u, v;
        if (!(cin >> u >> v)) return 0;
    }
    vector<int> ans = n <= 4 ? TinyStar(n).solve() : Star(n).solve();
    cout << '!';
    for (int x : ans) cout << ' ' << x;
    cout << endl;
}

#include <bits/stdc++.h>
using namespace std;

class Cave {
    struct Row {
        int door;
        vector<int> switches;
    };

    int n, queries = 0;
    // base opens every door before record and closes record, unless record == n.
    int record = -1;
    string base;
    vector<int> doorOf, switchOf, available, lower;
    vector<Row> rows;
    mt19937 random{9173851};
    bool longPrefix = false;

    string prediction;
    int nextFit = 3, wiringStep = -1, wiringOffset = 0;
    bool tryPrediction = false;
    set<string> tried;
    vector<double> entropy;

    int ask(const string &s) {
        ++queries;
        cout << "? " << s << '\n' << flush;
        int f;
        if (!(cin >> f) || f < -1 || f >= n) exit(0);
        return f == -1 ? n : f;
    }

    void identify(int d, int x) {
        doorOf[x] = d;
        switchOf[d] = x;
    }

    void simplify() {
        bool changed;
        do {
            changed = false;
            for (auto &row : rows) {
                if (switchOf[row.door] >= 0) continue;
                erase_if(row.switches, [&](int x) { return doorOf[x] >= 0; });
                if (row.switches.size() == 1) {
                    identify(row.door, row.switches[0]);
                    changed = true;
                }
            }
        } while (changed);
        erase_if(rows, [&](const Row &r) { return switchOf[r.door] >= 0; });
        erase_if(available, [&](int x) { return doorOf[x] >= 0; });
    }

    void extend(const string &q, int f) {
        // Very long stretches can be decoded in bulk, without allocating
        // a separate full-size candidate list for each newly passed door.
        longPrefix = f < n - 1 && (longPrefix || f - record > 28 || rows.size() > 90);
        if (f < n - 1 && !longPrefix) {
            for (int d = record + 1; d <= f; ++d)
                rows.push_back({d, available});
        }
        if (longPrefix) rows.push_back({f, available});
        record = f;
        base = q;
    }

    void observe(const string &q, int f) {
        if (longPrefix && f < record && switchOf[f] < 0) {
            auto it = lower_bound(rows.begin(), rows.end(), f, [](const Row &row, int d) {
                return row.door < d;
            });
            if (it == rows.end() || it->door != f) {
                Row row{f, {}};
                for (int x : available) {
                    if (lower[x] <= f && q[x] != base[x]) row.switches.push_back(x);
                }
                rows.insert(it, move(row));
            }
        }
        // A flipped switch cannot open its door in both probes. When both
        // probes stopped at the same door, that door's switch did not flip.
        int bound = min(record, f) + (f == record);
        for (int x : available) {
            if (q[x] != base[x]) lower[x] = max(lower[x], bound);
        }
        for (auto &row : rows) {
            if (row.door > f) break;
            // The switch changed iff exactly one of the two settings closed
            // this door. Both settings are known to have reached it.
            bool change = (row.door == f) != (row.door == record);
            erase_if(row.switches, [&](int x) { return (q[x] != base[x]) != change; });
        }
        if (f > record) extend(q, f);
        simplify();
    }

    char correctBit(int x) const {
        return char(base[x] ^ (doorOf[x] == record));
    }

    void answer() const {
        string s = base;
        for (int x = 0; x < n; ++x) s[x] = correctBit(x);
        cout << "! " << s;
        for (int d : doorOf) cout << ' ' << d;
        cout << '\n' << flush;
    }

    // Learn regularities only to choose probes. All identifications still
    // have to follow from the replies, even if a prediction fits every sample.
    void fitPredictions() {
        int known = n - (int)available.size();
        if (known < nextFit) return;
        nextFit = known + max(4, known / 4);
        vector<int> samples;
        for (int x = 0; x < n; ++x) if (doorOf[x] >= 0) samples.push_back(x);

        wiringStep = -1;
        int x0 = samples.front(), d0 = doorOf[x0];
        for (int step = 1; step < n; ++step) {
            int off = (x0 - step * d0 % n + n) % n;
            if (all_of(samples.begin() + 1, samples.end(), [&](int x) {
                return (step * doorOf[x] + off) % n == x;
            }) && gcd(step, n) == 1) {
                wiringStep = step;
                wiringOffset = off;
                break;
            }
        }

        int bestCost = known;
        string p;
        for (int period = 1; period <= min(16, known / 3); ++period) {
            int counts[16][2]{};
            for (int x : samples) ++counts[x % period][correctBit(x) - '0'];
            int cost = period;
            string pattern(period, '0');
            for (int j = 0; j < period; ++j) {
                cost += min(counts[j][0], counts[j][1]);
                pattern[j] += counts[j][1] > counts[j][0];
            }
            if (cost < bestCost) {
                bestCost = cost;
                p.resize(n);
                for (int x = 0; x < n; ++x) p[x] = pattern[x % period];
            }
        }
        // Estimate how well nearby known switches predict one another.
        int nearCost = 3;
        for (int i = 0; i < known; ++i) {
            int j = (i + 1 == known || (i && samples[i] - samples[i-1] <=
                       samples[i+1] - samples[i])) ? i - 1 : i + 1;
            nearCost += correctBit(samples[i]) != correctBit(samples[j]);
        }
        if (nearCost < bestCost) {
            bestCost = nearCost;
            p.resize(n);
            int i = 0;
            for (int x = 0; x < n; ++x) {
                while (i+1 < known && abs(samples[i+1]-x) < abs(samples[i]-x)) ++i;
                p[x] = correctBit(samples[i]);
            }
        }
        prediction = bestCost * 4 <= known ? p : "";
        tryPrediction = !prediction.empty() && bestCost * 8 <= known + 4;
        if (prediction.empty()) {
            int votes[2]{}, seen = 0;
            for (int d = record; d >= 0 && seen < 16; --d) {
                int x = switchOf[d];
                if (x < 0) continue;
                ++votes[correctBit(x) - '0'];
                ++seen;
            }
            if (seen == 16 && min(votes[0], votes[1]) <= 1) {
                prediction.assign(n, votes[1] > votes[0] ? '1' : '0');
                tryPrediction = true;
            }
        }
    }

    struct Cell {
        vector<int> switches, members;
        int selected = 0;
    };

    // Switches with identical membership in all live domains are
    // interchangeable for the current query's domain splits.
    string splitQuery() {
        int k = (int)rows.size();
        string q = base;
        if (k > 48) {
            // A cheap simultaneous split when there are many outstanding rows.
            vector<bool> used(n);
            vector<int> free;
            for (int j = 0; j < k; ++j) {
                auto &row = rows[j];
                int changed = 0;
                free.clear();
                for (int x : row.switches) {
                    if (!used[x]) {
                        used[x] = true;
                        free.push_back(x);
                    } else changed += q[x] != base[x];
                }
                int size = row.switches.size();
                int want = row.door == record ? (size + 1) / 2 :
                           max(1, size / min(12, k - j + 1));
                int count = clamp(want - changed, 0, (int)free.size());
                for (int i = 0; i < count; ++i) q[free[i]] ^= 1;
            }
        } else if (k) {
            vector<uint64_t> membership(n, 0);
            vector<int> size(k), selected(k);
            vector<double> weight(k), inv(k);
            for (int j = 0; j < k; ++j) {
                size[j] = rows[j].switches.size();
                inv[j] = 1.0 / size[j];
                weight[j] = pow(log2(double(available.size())) / log2(size[j]), 0.14);
                for (int x : rows[j].switches) membership[x] |= uint64_t(1) << j;
            }
            unordered_map<uint64_t, int> index;
            vector<Cell> cells;
            for (int x : available) {
                uint64_t m = membership[x];
                if (!m) {
                    if (!prediction.empty()) q[x] = prediction[x];
                    continue;
                }
                auto [it, inserted] = index.emplace(m, (int)cells.size());
                if (inserted) {
                    cells.emplace_back();
                    while (m) {
                        cells.back().members.push_back(countr_zero(m));
                        m &= m - 1;
                    }
                }
                cells[it->second].switches.push_back(x);
            }
            bool blocked = rows.back().door == record;
            // Under a uniform candidate estimate, the reply is the first
            // failed row. Score its entropy, including replies past record.
            auto value = [&]() {
                double h = 2.15;
                for (int j = k - 1; j >= 0; --j) {
                    double p = selected[j] * inv[j];
                    double pass = (j == k - 1 && blocked) ? p : 1.0 - p;
                    h = weight[j] * entropy[int(p * 16384 + 0.5)] + pass * h;
                }
                return h;
            };
            for (int round = 0; round < 4; ++round) {
                bool modified = false;
                for (int t = 0; t < (int)cells.size(); ++t) {
                    Cell &cell = cells[round % 2 ? t : cells.size() - 1 - t];
                    auto setCount = [&](int count) {
                        for (int j : cell.members) selected[j] += count - cell.selected;
                        cell.selected = count;
                    };
                    int original = cell.selected, best = original;
                    double score = value();
                    int l = 0, r = cell.switches.size();
                    while (r - l > 4) {
                        int a = l + (r-l)/3, b = r - (r-l)/3;
                        setCount(a);
                        double left = value();
                        setCount(b);
                        if (left > value()) r = b-1;
                        else l = a+1;
                    }
                    for (int i = l; i <= r; ++i) {
                        setCount(i);
                        double s = value();
                        if (s > score + 1e-9) { score = s; best = i; }
                    }
                    setCount(best);
                    modified |= best != original;
                }
                if (!modified) break;
            }
            for (auto &cell : cells) {
                int count = cell.selected;
                if (!count) continue;
                auto &v = cell.switches;
                int start = random() % v.size();
                rotate(v.begin(), v.begin() + start, v.end());
                if (!prediction.empty())
                    partition(v.begin(), v.end(), [&](int x) { return base[x] != prediction[x]; });
                for (int i = 0; i < count; ++i) q[v[i]] ^= 1;
            }
            // Protect progress independently of the quality of the score.
            bool informative = false;
            for (int j = 0; j < k; ++j) {
                if (selected[j] && selected[j] < size[j]) { informative = true; break; }
                if (j == k-1 && blocked) informative = selected[j] == size[j];
                else if (selected[j]) break;
                else if (j == k-1) informative = true;
            }
            if (!informative) {
                q = base;
                auto &v = rows.front().switches;
                for (int i = 0; i < (int)v.size()/2; ++i) q[v[i]] ^= 1;
            }
        } else if (!prediction.empty()) {
            for (int x : available) q[x] = prediction[x];
        }
        if (switchOf[record] >= 0) q[switchOf[record]] ^= 1;
        return q;
    }

    // If the adaptive probes have been expensive, halve the domain of
    // the earliest unresolved door on every probe. All earlier doors open.
    void balancedStep() {
        int d = 0;
        while (switchOf[d] >= 0) ++d;
        auto it = lower_bound(rows.begin(), rows.end(), d, [](const Row &row, int door) {
            return row.door < door;
        });
        string q = base;
        if (d <= record) {
            if (it == rows.end() || it->door != d) {
                Row row{d, {}};
                for (int x : available) if (lower[x] <= d) row.switches.push_back(x);
                it = rows.insert(it, move(row));
            }
            if (it->switches.size() == 1) {
                simplify();
                return;
            }
            for (int i = 0; i < (int)it->switches.size() / 2; ++i)
                q[it->switches[i]] ^= 1;
        }
        if (switchOf[record] >= 0) q[switchOf[record]] ^= 1;
        observe(q, ask(q));
    }

    // During a long run of open doors, a sparse set of flips can locate a
    // previously unobserved door in a small set with a single reply.
    void bulkStep() {
        int outstanding = record + 1 - (n - (int)available.size());
        if (outstanding <= 48) {
            vector<bool> present(n);
            for (const auto &row : rows) present[row.door] = true;
            for (int d = 0; d <= record; ++d) {
                if (switchOf[d] >= 0 || present[d]) continue;
                Row row{d, {}};
                for (int x : available) {
                    if (lower[x] <= d) row.switches.push_back(x);
                }
                rows.push_back(move(row));
            }
            sort(rows.begin(), rows.end(), [](const Row &a, const Row &b) {
                return a.door < b.door;
            });
            simplify();
            longPrefix = false;
            return;
        }

        int eligible = 0;
        for (int x : available) eligible += lower[x] <= record;
        int quota = max(1, (eligible + outstanding / 2) / outstanding);
        vector<bool> used(n);
        vector<int> options;
        string q = base;
        auto select = [&](int count) {
            count = clamp(count, 0, (int)options.size());
            if (!count) return;
            int start = random() % options.size();
            for (int i = 0; i < count; ++i) {
                q[options[start]] ^= 1;
                if (++start == (int)options.size()) start = 0;
            }
        };
        int k = rows.size();
        for (int j = 0; j < k; ++j) {
            const auto &row = rows[j];
            options.clear();
            int changed = 0;
            for (int x : row.switches) {
                if (!used[x]) {
                    used[x] = true;
                    options.push_back(x);
                } else {
                    changed += q[x] != base[x];
                }
            }
            int want = clamp((int)row.switches.size() / max(2, k - j + 1), 1, quota);
            if (row.door == record) {
                // These switches cannot disturb any earlier door.
                auto pivot = partition(options.begin(), options.end(), [&](int x) {
                    return lower[x] >= record;
                });
                int safe = pivot - options.begin();
                int take = (safe + 1) / 2;
                for (int i = 0; i < take; ++i) q[options[i]] ^= 1;
                changed += take;
                options.erase(options.begin(), pivot);
                want = quota;
            }
            select(want - changed);
        }
        options.clear();
        for (int x : available) {
            if (!used[x] && lower[x] <= record) options.push_back(x);
        }
        select(quota);
        if (switchOf[record] >= 0) q[switchOf[record]] ^= 1;
        observe(q, ask(q));
    }

    // Minimize total remaining path length for small sets of hypotheses.
    int exactDecision(const vector<array<unsigned, 7>> &parts, unsigned full, int m) {
        struct Decision {
            int cost, depth, mask;
        };
        unordered_map<unsigned, Decision> memo;
        auto solve = [&](auto &&self, unsigned subset) -> Decision {
            if (has_single_bit(subset)) return {0, 0, 0};
            auto it = memo.find(subset);
            if (it != memo.end()) return it->second;
            int count = popcount(subset);
            Decision best{INT_MAX, INT_MAX, 0};
            for (int mask = 0; mask < (int)parts.size(); ++mask) {
                int cost = count, depth = 1;
                for (int j = 0; j <= m; ++j) {
                    unsigned child = subset & parts[mask][j];
                    if (!child) continue;
                    if (child == subset) { cost = INT_MAX; break; }
                    Decision next = self(self, child);
                    cost += next.cost;
                    depth = max(depth, 1 + next.depth);
                    if (cost > best.cost) break;
                }
                if (tie(cost, depth) < tie(best.cost, best.depth))
                    best = {cost, depth, mask};
                if (best.cost == count) break;
            }
            memo.emplace(subset, best);
            return best;
        };
        return solve(solve, full).mask;
    }

    // With at most six remaining switches all feasible answers fit in a
    // small table. Count the exact reply distribution of every possible query.
    void finishSmall() {
        int m = available.size(), masks = 1 << m;
        vector<int> doors;
        for (int d = 0; d < n; ++d) if (switchOf[d] < 0) doors.push_back(d);
        vector<int> allowed(m, masks - 1);
        for (auto &row : rows) {
            int j = lower_bound(doors.begin(), doors.end(), row.door) - doors.begin();
            allowed[j] = 0;
            for (int i = 0; i < m; ++i)
                if (find(row.switches.begin(), row.switches.end(), available[i]) != row.switches.end())
                    allowed[j] |= 1 << i;
        }
        struct State { int perm, bits; };
        vector<State> states;
        vector<array<unsigned char, 6>> permutations;
        vector<array<unsigned char, 64>> first;
        array<unsigned char, 6> p{};
        iota(p.begin(), p.begin()+m, 0);
        do {
            bool ok = true;
            int bits = 0, unseen = 0;
            for (int j = 0; j < m; ++j) {
                int i = p[j];
                if (lower[available[i]] > doors[j] || !(allowed[j] >> i & 1)) { ok = false; break; }
                if (doors[j] > record) unseen |= 1 << i;
                else bits |= ((base[available[i]] - '0') ^ (doors[j] == record)) << i;
            }
            if (!ok) continue;
            int id = permutations.size();
            permutations.push_back(p);
            first.emplace_back();
            for (int mask = 0; mask < masks; ++mask) {
                int j = 0;
                while (j < m && !(mask >> p[j] & 1)) ++j;
                first.back()[mask] = j;
            }
            for (int extra = unseen;; extra = (extra-1)&unseen) {
                states.push_back({id, bits | extra});
                if (!extra) break;
            }
        } while (next_permutation(p.begin(), p.begin()+m));

        string q = base;
        for (int x = 0; x < n; ++x) if (doorOf[x] >= 0) q[x] = correctBit(x);
        while (states.size() > 1) {
            double best = numeric_limits<double>::infinity();
            int queryMask = 0;
            if (states.size() <= 15 || (m <= 3 && states.size() <= 24)) {
                vector<array<unsigned, 7>> parts(masks);
                for (int mask = 0; mask < masks; ++mask) {
                    for (int t = 0; t < (int)states.size(); ++t) {
                        auto st = states[t];
                        parts[mask][first[st.perm][st.bits ^ mask]] |= 1u << t;
                    }
                }
                queryMask = exactDecision(parts, (1u << states.size()) - 1, m);
            } else {
                for (int mask = 0; mask < masks; ++mask) {
                    int count[7]{};
                    for (auto st : states) ++count[first[st.perm][st.bits ^ mask]];
                    double cost = 0;
                    for (int j = 0; j <= m; ++j)
                        if (count[j] > 1) cost += count[j] * log2(count[j]);
                    if (cost < best - 1e-9) { best = cost; queryMask = mask; }
                }
            }
            for (int i = 0; i < m; ++i) q[available[i]] = '0' + (queryMask >> i & 1);
            int f = ask(q);
            int j = lower_bound(doors.begin(), doors.end(), f) - doors.begin();
            erase_if(states, [&](State st) { return first[st.perm][st.bits ^ queryMask] != j; });
        }
        auto st = states.front();
        for (int x = 0; x < n; ++x) if (doorOf[x] >= 0) q[x] = correctBit(x);
        for (int j = 0; j < m; ++j) {
            int i = permutations[st.perm][j], x = available[i];
            identify(doors[j], x);
            q[x] = '0' + (st.bits >> i & 1);
        }
        base = q;
        record = n;
    }

    void finishFromReference() {
        while (available.size() > 6) {
            int x = available.back();
            if (!rows.empty()) {
                auto it = min_element(rows.begin(), rows.end(), [](const Row &a, const Row &b) {
                    return a.switches.size() < b.switches.size();
                });
                x = it->switches.back();
            }
            string q = base;
            q[x] ^= 1;
            int f = ask(q);
            identify(f == n ? n-1 : f, x);
            simplify();
        }
        if (!available.empty()) finishSmall();
    }

public:
    explicit Cave(int size) : n(size), base(n, '0'), doorOf(n, -1), switchOf(n, -1),
                              available(n), lower(n), entropy(16385) {
        iota(available.begin(), available.end(), 0);
        shuffle(available.begin(), available.end(), random);
        for (int i = 1; i < 16384; ++i) {
            double p = i/16384.0;
            entropy[i] = -p*log2(p) - (1-p)*log2(1-p);
        }
    }

    void solve() {
        string q(n, '0');
        observe(q, ask(q));
        if (n >= 12 && record == 0) {
            q.assign(n, '1');
            observe(q, ask(q));
        }
        while (!available.empty()) {
            if (available.size() <= 6) { finishSmall(); break; }
            if (record >= n-1) { finishFromReference(); break; }

            if (queries > 5 * n) { balancedStep(); continue; }
            fitPredictions();
            if (tryPrediction) {
                tryPrediction = false;
                q = prediction;
                for (int x = 0; x < n; ++x) if (doorOf[x] >= 0) q[x] = correctBit(x);
                bool viable = all_of(rows.begin(), rows.end(), [&](const Row &row) {
                    return any_of(row.switches.begin(), row.switches.end(), [&](int x) {
                        return (q[x] != base[x]) == (row.door == record);
                    });
                });
                if (viable && tried.insert(q).second) {
                    observe(q, ask(q));
                    continue;
                }
            }
            if (wiringStep >= 0 && !rows.empty()) {
                int d = rows.size() > 5 ? rows.front().door : rows.back().door;
                int x = (wiringStep * d + wiringOffset) % n;
                auto &v = rows.size() > 5 ? rows.front().switches : rows.back().switches;
                if (find(v.begin(), v.end(), x) != v.end()) {
                    q = base;
                    if (switchOf[record] >= 0) q[switchOf[record]] ^= 1;
                    q[x] ^= 1;
                    int f = ask(q);
                    if (d == record ? f <= record : f != d) wiringStep = -1;
                    observe(q, f);
                    continue;
                }
                wiringStep = -1;
            }
            if (longPrefix) { bulkStep(); continue; }
            q = splitQuery();
            observe(q, ask(q));
        }
        answer();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (cin >> n && n >= 1 && n <= 5000) Cave(n).solve();
}

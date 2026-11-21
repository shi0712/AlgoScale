#include <bits/stdc++.h>
using namespace std;

constexpr int MAX_M = 10;
constexpr int EXACT_LIMIT = 12;

struct Group {
    vector<int> side[2];
};

struct Test {
    int x, y, mask;
};

int ask(int x, int y) {
    cout << "? " << x << ' ' << y << endl;
    int answer;
    if (!(cin >> answer) || (answer != 0 && answer != 1)) exit(0);
    return answer;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, m;
    if (!(cin >> n >> m)) return 0;

    vector<int> labels(n);
    iota(labels.begin(), labels.end(), 1);
    mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());
    shuffle(labels.begin(), labels.end(), rng);

    vector<Group> groups(n);
    for (int i = 0; i < n; ++i) groups[i].side[0].push_back(labels[i]);

    int remaining = m;
    int genuine = 0;
    vector<int> counterfeit;

    auto weight = [&](int i) {
        return int(groups[i].side[0].size()) - int(groups[i].side[1].size());
    };

    auto representative = [&](int i) {
        return groups[i].side[0].front();
    };

    // Each unresolved group already accounts for its smaller side in the budget.
    // Flipping the majority to counterfeit costs an additional 'weight' coins.
    auto certify = [&](int i, bool flipped) {
        if (flipped) remaining -= weight(i);
        auto& fake = groups[i].side[flipped ? 0 : 1];
        auto& real = groups[i].side[flipped ? 1 : 0];
        counterfeit.insert(counterfeit.end(), fake.begin(), fake.end());
        if (!genuine && !real.empty()) genuine = real.front();
        groups[i].side[0].clear();
        groups[i].side[1].clear();
    };

    auto merge_groups = [&](int a, int b, int different) {
        if (different) remaining -= min(weight(a), weight(b));
        for (int j = 0; j < 2; ++j) {
            auto& dst = groups[a].side[j];
            auto& src = groups[b].side[j ^ different];
            dst.insert(dst.end(), src.begin(), src.end());
            src.clear();
        }
        if (groups[a].side[0].size() < groups[a].side[1].size()) {
            swap(groups[a].side[0], groups[a].side[1]);
        }
    };

    vector<array<long double, MAX_M + 1>> choose(n + 1);
    choose[0][0] = 1;
    for (int i = 1; i <= n; ++i) {
        choose[i][0] = 1;
        for (int j = 1; j <= m; ++j) {
            choose[i][j] = choose[i - 1][j] + choose[i - 1][j - 1];
        }
    }

    // For a small number of possible assignments, optimize the whole decision
    // tree rather than choosing the next comparison in isolation.
    auto solve_exact = [&](const vector<int>& active) {
        int len = int(active.size());
        vector<unsigned> suffix(len + 1, 0);
        suffix[len] = 1;
        for (int i = len - 1; i >= 0; --i) {
            suffix[i] = suffix[i + 1] | (suffix[i + 1] << weight(active[i]));
        }

        int configurations = 0;
        vector<int> masks(len, 0), flipped;
        auto enumerate = [&](auto&& self, int i, int r) -> void {
            if (i == len) {
                for (int j : flipped) masks[j] |= 1 << configurations;
                ++configurations;
                return;
            }
            if ((suffix[i + 1] >> r) & 1U) self(self, i + 1, r);
            int w = weight(active[i]);
            if (r >= w && ((suffix[i + 1] >> (r - w)) & 1U)) {
                flipped.push_back(i);
                self(self, i + 1, r - w);
                flipped.pop_back();
            }
        };
        enumerate(enumerate, 0, remaining);

        int full = (1 << configurations) - 1;
        vector<Test> tests;
        vector<bool> seen(full + 1, false);
        auto add_test = [&](int x, int y, int mask) {
            int key = min(mask, full ^ mask);
            if (key && !seen[key]) {
                seen[key] = true;
                tests.push_back({x, y, mask});
            }
        };
        for (int i = 0; i < len; ++i) {
            int x = representative(active[i]);
            add_test(x, genuine, masks[i]);
            for (int j = 0; j < i; ++j) {
                add_test(x, representative(active[j]), masks[i] ^ masks[j]);
            }
        }

        vector<int> cost(full + 1, -1), depth(full + 1, 0), pick(full + 1, -1);
        auto plan = [&](auto&& self, int state) -> int {
            if ((state & (state - 1)) == 0) return 0;
            if (cost[state] != -1) return cost[state];
            int best = INT_MAX, best_depth = INT_MAX;
            int leaves = popcount(static_cast<unsigned>(state));
            for (int j = 0; j < int(tests.size()); ++j) {
                int one = state & tests[j].mask;
                if (!one || one == state) continue;
                int zero = state ^ one;
                int c0 = self(self, zero);
                int c1 = self(self, one);
                int total = leaves + c0 + c1;
                int d = 1 + max(depth[zero], depth[one]);
                if (total < best || (total == best && d < best_depth)) {
                    best = total;
                    best_depth = d;
                    pick[state] = j;
                }
            }
            depth[state] = best_depth;
            return cost[state] = best;
        };
        plan(plan, full);

        int state = full;
        while (state & (state - 1)) {
            const auto& t = tests[pick[state]];
            if (ask(t.x, t.y)) state &= t.mask;
            else state &= full ^ t.mask;
        }
        for (int i = 0; i < len; ++i) certify(active[i], (state & masks[i]) != 0);
    };

    while (true) {
        array<vector<int>, MAX_M + 1> bucket;
        vector<int> active;
        for (int i = 0; i < n; ++i) {
            if (groups[i].side[0].empty()) continue;
            int w = weight(i);
            if (w > remaining) {
                certify(i, false);
            } else if (w > 0) {
                bucket[w].push_back(i);
                active.push_back(i);
            }
        }
        if (active.empty()) break;

        vector<int> weights;
        for (int w = 1; w <= remaining; ++w) {
            if (!bucket[w].empty()) weights.push_back(w);
        }

        // Coefficients of the subset-sum polynomial, omitting up to two groups.
        auto ways = [&](int skip_a, int skip_b) {
            array<long double, MAX_M + 1> dp{};
            dp[0] = 1;
            for (int w : weights) {
                int cnt = int(bucket[w].size()) - (w == skip_a) - (w == skip_b);
                if (cnt == 0) continue;
                array<long double, MAX_M + 1> next{};
                for (int k = 0; k <= min(cnt, remaining / w); ++k) {
                    for (int s = 0; s + k * w <= remaining; ++s) {
                        next[s + k * w] += dp[s] * choose[cnt][k];
                    }
                }
                dp = next;
            }
            return dp;
        };

        int r = remaining;
        bool deduced = false;
        array<long double, MAX_M + 1> flip_ways{};
        for (int w : weights) {
            auto dp = ways(w, 0);
            flip_ways[w] = dp[r - w];
            if (dp[r] == 0 || dp[r - w] == 0) {
                bool flip = dp[r] == 0;
                for (int i : bucket[w]) certify(i, flip);
                deduced = true;
                break;
            }
        }
        if (deduced) continue;

        auto total = ways(0, 0)[r];
        if (genuine && total <= EXACT_LIMIT) {
            solve_exact(active);
            break;
        }

        int best_a = -1, best_b = -1;
        long double best_score = -1;
        int best_bonus = -1;

        auto consider = [&](int a, int b, long double score, int bonus) {
            if (score > best_score || (score == best_score && bonus > best_bonus)) {
                best_score = score;
                best_bonus = bonus;
                best_a = a;
                best_b = b;
            }
        };

        for (int a : weights) {
            int i = bucket[a][0];
            if (genuine) consider(i, -1, a * flip_ways[a], 0);
            for (int b : weights) {
                if (b < a || (a == b && bucket[a].size() < 2)) continue;
                int j = bucket[b][a == b ? 1 : 0];
                auto dp = ways(a, b);
                long double same = dp[r] + (r >= a + b ? dp[r - a - b] : 0);
                long double different = dp[r - a] + dp[r - b];

                if (same == 0 || different == 0) {
                    merge_groups(i, j, same == 0);
                    deduced = true;
                    break;
                }

                // Prefer comparisons that are likely to account for additional
                // counterfeits, especially comparisons of equally sized margins.
                int bonus = (a + b > r ? 2 : 0) + (a == b ? 1 : 0);
                consider(i, j, min(a, b) * different, bonus);
            }
            if (deduced) break;
        }
        if (deduced) continue;

        // In the heavily clustered case, use some double-sized groups to
        // certify smaller groups without creating yet another unresolved level.
        if (weights.size() == 2) {
            int a = weights[0], b = weights[1];
            if (a >= 4 && b == 2 * a && r == b &&
                bucket[b].size() >= bucket[a].size()) {
                best_a = bucket[a][0];
                best_b = bucket[b][0];
            }
        }

        int x = representative(best_a);
        if (best_b == -1) {
            certify(best_a, ask(x, genuine) == 1);
        } else {
            int y = representative(best_b);
            merge_groups(best_a, best_b, ask(x, y));
        }
    }

    // Balanced groups have no effect on the remaining budget. Each requires
    // one orientation comparison, regardless of its size.
    for (int i = 0; i < n; ++i) {
        if (!groups[i].side[0].empty()) {
            certify(i, ask(representative(i), genuine) == 1);
        }
    }

    cout << '!';
    for (int coin : counterfeit) cout << ' ' << coin;
    cout << endl;
    return 0;
}

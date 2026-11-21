#include <bits/stdc++.h>
using namespace std;

constexpr int MAXN = 1024;
constexpr int RESET = -1;
using Mask = bitset<MAXN>;
using Clock = chrono::steady_clock;

template<class F>
void each_bit(const Mask& b, F f) {
    for (size_t i = b._Find_first(); i < MAXN; i = b._Find_next(i))
        f(static_cast<int>(i));
}

struct Slot {
    Mask possible;
    int single = -1;
};

struct State {
    int n, k, distinct, edges, ambiguous = 0;
    vector<Mask> adj;
    vector<int> degree, gain, occurrences, certain;
    Mask alive, occupied, forced;
    deque<Slot> memory;

    State(int n_, int k_) : n(n_), k(k_), distinct(n), edges(n * (n - 1) / 2),
        adj(n), degree(n, n - 1), gain(n), occurrences(n), certain(n) {
        for (int i = 0; i < n; ++i) alive.set(i);
        for (int i = 0; i < n; ++i) {
            adj[i] = alive;
            adj[i].reset(i);
        }
    }

    void erase_edge(int u, int v) {
        adj[u].reset(v);
        adj[v].reset(u);
        --degree[u];
        --degree[v];
        --edges;
        if (certain[u]) --gain[v];
        if (certain[v]) --gain[u];
    }

    void pop_slot() {
        const Slot& s = memory.front();
        if (s.single != -1) {
            int x = s.single;
            if (--certain[x] == 0) {
                forced.reset(x);
                each_bit(adj[x], [&](int y) { --gain[y]; });
            }
            if (--occurrences[x] == 0) occupied.reset(x);
        } else {
            --ambiguous;
            each_bit(s.possible, [&](int x) {
                if (--occurrences[x] == 0) occupied.reset(x);
            });
        }
        memory.pop_front();
    }

    void reset() {
        memory.clear();
        occupied.reset();
        forced.reset();
        fill(gain.begin(), gain.end(), 0);
        fill(occurrences.begin(), occurrences.end(), 0);
        fill(certain.begin(), certain.end(), 0);
        ambiguous = 0;
    }

    // x is not in the support of any slot. Thus a Y can only
    // identify its variety with that of a different representative.
    void apply(int x, bool yes) {
        Slot s;
        if (yes) {
            s.possible = occupied & adj[x];
            size_t first = s.possible._Find_first();
            if (first == MAXN) exit(0);
            if (s.possible._Find_next(first) == MAXN)
                s.single = static_cast<int>(first);

            Mask incident = adj[x];
            each_bit(incident, [&](int y) { erase_edge(x, y); });
            alive.reset(x);
            --distinct;
        } else {
            Mask compared = adj[x] & forced;
            each_bit(compared, [&](int y) { erase_edge(x, y); });
            s.single = x;
        }

        // The oldest slot participates in the tasting, then expires.
        if (static_cast<int>(memory.size()) == k) pop_slot();

        if (s.single != -1) {
            int y = s.single;
            if (occurrences[y]++ == 0) occupied.set(y);
            if (certain[y]++ == 0) {
                forced.set(y);
                each_bit(adj[y], [&](int z) { ++gain[z]; });
            }
        } else {
            ++ambiguous;
            each_bit(s.possible, [&](int y) {
                if (occurrences[y]++ == 0) occupied.set(y);
            });
        }
        memory.push_back(std::move(s));
    }
};

struct Policy {
    int bias, expiry, finish;
    bool reset_weak;
    vector<int> tie;

    Policy(int n, int id, mt19937& rng) : tie(n) {
        static constexpr int biases[] = {8, 0, 24, -12, 48, -32, 72, 4, 16, -4, 96, -48};
        int p = id % 12;
        bias = biases[p];
        expiry = p % 3 == 0 ? 9 : (p % 3 == 1 ? 0 : 19);
        finish = p % 4 == 2 ? 17 : 0;
        reset_weak = p == 4 || p == 7 || p == 10;
        int noise = p % 4 == 0 ? 5 : (p % 4 == 1 ? 15 : 30);
        for (int i = 0; i < n; ++i)
            tie[i] = id == 0 ? -i : static_cast<int>(rng() % (noise * n));
    }

    int choose(const State& s) const {
        const int unit = 64 * s.n;
        const Mask available = s.alive & ~s.occupied;
        int oldest = static_cast<int>(s.memory.size()) == s.k
                     ? s.memory.front().single : -1;
        int best = -1, best_score = INT_MIN, max_gain = 0;
        each_bit(available, [&](int x) {
            int g = s.gain[x];
            if (!g) return;
            max_gain = max(max_gain, g);
            int score = unit * g + bias * s.degree[x] + tie[x];
            if (oldest != -1 && s.adj[x][oldest]) score += expiry * s.n;
            if (g == s.degree[x]) score += finish * s.n;
            if (score > best_score) {
                best_score = score;
                best = x;
            }
        });
        if (best != -1) {
            if (reset_weak && s.ambiguous && max_gain * 6 < s.k &&
                s.occupied.count() > s.forced.count() + s.k / 4)
                return RESET;
            return best;
        }

        // Put a vertex in memory only if it sets up a comparison on
        // the very next tasting. If no such vertex is free, reset.
        each_bit(available, [&](int x) {
            int d = s.ambiguous ? static_cast<int>((s.adj[x] & available).count())
                                : s.degree[x];
            if (!d) return;
            int score = d * unit + tie[x];
            if (score > best_score) {
                best_score = score;
                best = x;
            }
        });
        return best;
    }
};

vector<int> euler_plan(const State& s) {
    int n = s.n;
    int current = s.memory.empty() ? -1 : s.memory.back().single;
    Mask seen;
    vector<vector<int>> trails;

    for (int start = 0; start < n; ++start) {
        if (seen[start] || !s.degree[start]) continue;
        vector<int> component{start};
        seen.set(start);
        for (size_t i = 0; i < component.size(); ++i) {
            each_bit(s.adj[component[i]], [&](int y) {
                if (!seen[y]) {
                    seen.set(y);
                    component.push_back(y);
                }
            });
        }

        vector<vector<pair<int, int>>> graph(n + 1);
        int m = 0;
        auto link = [&](int x, int y) {
            graph[x].emplace_back(y, m);
            graph[y].emplace_back(x, m++);
        };
        int source = start;
        for (int x : component) {
            if (x == current && source != n) source = x;
            if (s.degree[x] & 1) {
                link(x, n);
                source = n;
            }
            each_bit(s.adj[x], [&](int y) {
                if (x < y) link(x, y);
            });
        }

        vector<char> used(m);
        vector<int> stack{source}, circuit;
        while (!stack.empty()) {
            int x = stack.back();
            auto& list = graph[x];
            while (!list.empty() && used[list.back().second]) list.pop_back();
            if (list.empty()) {
                circuit.push_back(x);
                stack.pop_back();
            } else {
                auto [y, id] = list.back();
                list.pop_back();
                used[id] = true;
                stack.push_back(y);
            }
        }
        reverse(circuit.begin(), circuit.end());
        vector<int> trail;
        for (int x : circuit) {
            if (x == n) {
                if (!trail.empty()) {
                    trails.push_back(std::move(trail));
                    trail.clear();
                }
            } else {
                trail.push_back(x);
            }
        }
        if (!trail.empty()) trails.push_back(std::move(trail));
    }

    for (auto& t : trails) {
        if (t.front() == current || t.back() == current) {
            if (t.front() != current) reverse(t.begin(), t.end());
            swap(t, trails.front());
            break;
        }
    }

    vector<int> result;
    for (const auto& t : trails) {
        if (current != t.front()) result.push_back(t.front());
        result.insert(result.end(), t.begin() + 1, t.end());
        current = t.back();
    }
    return result;
}

struct Solver {
    State s;
    mt19937 rng;
    vector<int> cafe;
    Policy standard;
    int tastings = 0;
    int plan_number = 0;
    chrono::nanoseconds planning_time{0};

    Solver(int n, int k) : s(n, k),
        rng(static_cast<unsigned>(Clock::now().time_since_epoch().count())),
        cafe(n), standard(n, 0, rng) {
        iota(cafe.begin(), cafe.end(), 1);
        shuffle(cafe.begin(), cafe.end(), rng);
    }

    void reset() {
        cout << "R" << endl;
        s.reset();
    }

    bool taste(int x) {
        cout << "? " << cafe[x] << endl;
        string reply;
        if (!(cin >> reply) || (reply != "Y" && reply != "N")) exit(0);
        ++tastings;
        bool yes = reply == "Y";
        s.apply(x, yes);
        return yes;
    }

    vector<int> plan(int remaining) {
        if (s.k == 1) return euler_plan(s);
        constexpr auto budget = chrono::milliseconds(1200);
        if (planning_time >= budget) return {};

        auto started = Clock::now();
        auto deadline = started + (budget - planning_time);
        int attempts = plan_number == 0 ? 26 : 5;
        ++plan_number;
        int best_count = remaining + 1;
        vector<int> best;

        for (int j = 0; j < attempts && Clock::now() < deadline; ++j) {
            int id = j == 0 ? 0 : (plan_number == 1 ? j : 1 + rng() % 24);
            Policy policy(s.n, id, rng);
            State trial = s;
            vector<int> actions;
            int count = 0;
            while (trial.edges && count < best_count) {
                if ((count & 127) == 0 && Clock::now() >= deadline) break;
                int x = policy.choose(trial);
                actions.push_back(x);
                if (x == RESET) {
                    trial.reset();
                } else {
                    trial.apply(x, false);
                    ++count;
                }
            }
            if (!trial.edges && count < best_count) {
                best_count = count;
                best = std::move(actions);
                if (best_count == (s.edges + s.k - 1) / s.k) break;
            }
        }
        planning_time += Clock::now() - started;
        return best;
    }

    void batch(const Mask& members) {
        if (!s.edges) return;
        bool needed = false;
        each_bit(members, [&](int x) {
            if ((s.adj[x] & members).any()) needed = true;
        });
        if (!needed) return;
        if (!s.memory.empty()) reset();
        each_bit(members, [&](int x) {
            if ((s.adj[x] & members).any()) taste(x);
        });
    }

    // A bounded backup: every pair of groups fits in a single window.
    void finish_in_batches() {
        Mask relevant;
        for (int x = 0; x < s.n; ++x)
            if (s.degree[x]) relevant.set(x);
        if (relevant.count() <= static_cast<size_t>(s.k + 1)) {
            batch(relevant);
            return;
        }

        int width = max(1, s.k / 2);
        vector<Mask> groups;
        int filled = 0;
        each_bit(relevant, [&](int x) {
            if (!filled) groups.emplace_back();
            groups.back().set(x);
            if (++filled == width) filled = 0;
        });

        for (size_t i = 0; i < groups.size() && s.edges; ++i)
            for (size_t j = i + 1; j < groups.size() && s.edges; ++j)
                batch(groups[i] | groups[j]);
        for (const Mask& g : groups) batch(g);
    }

    void run() {
        if (s.n > 1) {
            // Each café must be tasted at least once. Start with a sweep
            // so that easy duplicates are eliminated before any repeats.
            for (int x = 0; x < s.n; ++x) taste(x);

            const int limit = s.n + 2 * s.n * s.n / s.k;
            vector<int> actions;
            size_t next = 0;
            bool replan = true;
            while (s.edges) {
                if (tastings >= limit) {
                    finish_in_batches();
                    break;
                }
                if (replan) {
                    actions = plan(limit - tastings);
                    next = 0;
                    replan = false;
                }
                int x = next < actions.size() ? actions[next++] : standard.choose(s);
                if (x == RESET) {
                    reset();
                } else if (taste(x)) {
                    replan = true;
                }
            }
        }
        cout << "! " << s.distinct << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k) || n < 1 || k < 1) return 0;
    Solver solver(n, k);
    solver.run();
    return 0;
}

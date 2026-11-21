#include <bits/stdc++.h>
using namespace std;

vector<int> position_id;

int ask(int u, int v) {
    cout << "? " << position_id[u] << ' ' << position_id[v] << endl;
    int g;
    if (!(cin >> g) || g == -1) exit(0);
    return g;
}

[[noreturn]] void answer(int u, int v) {
    cout << "! " << position_id[u] << ' ' << position_id[v] << endl;
    int verdict;
    if (!(cin >> verdict) || verdict != 1) exit(0);
    exit(0);
}

struct SmallState {
    array<unsigned char, 36> result;
    unsigned char zero;
};

void solve_small(int n) {
    vector<pair<int, int>> pairs;
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            pairs.emplace_back(i, j);

    int m = (int)pairs.size();
    int table[9][9];
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            table[i][j] = gcd(i, j);

    vector<int> p(n);
    iota(p.begin(), p.end(), 0);
    int factorial = 1;
    for (int i = 2; i <= n; ++i) factorial *= i;
    vector<SmallState> states;
    states.reserve(factorial);
    do {
        SmallState s{};
        for (int i = 0; i < n; ++i)
            if (p[i] == 0) s.zero = i;
        for (int q = 0; q < m; ++q) {
            auto [u, v] = pairs[q];
            s.result[q] = table[p[u]][p[v]];
        }
        states.push_back(s);
    } while (next_permutation(p.begin(), p.end()));

    if (n <= 5) {
        using Mask = bitset<120>;
        array<Mask, 5> zero{};
        vector<array<Mask, 5>> out(m);
        Mask remaining;
        for (int i = 0; i < (int)states.size(); ++i) {
            remaining.set(i);
            zero[states[i].zero].set(i);
            for (int q = 0; q < m; ++q)
                out[q][states[i].result[q]].set(i);
        }

        unordered_map<Mask, pair<int, int>> memo;
        auto solve = [&](auto&& self, const Mask& mask) -> pair<int, int> {
            auto it = memo.find(mask);
            if (it != memo.end()) return it->second;
            int k = 0;
            for (int i = 0; i < n; ++i) k += (mask & zero[i]).any();
            if (k <= 2) return {0, -1};

            pair<int, int> best{INT_MAX, -1};
            int count = (int)mask.count();
            for (int q = 0; q < m; ++q) {
                int cost = count;
                bool splits = true;
                for (int g = 1; g < n; ++g) {
                    Mask next = mask & out[q][g];
                    if (next.none()) continue;
                    if (next == mask) {
                        splits = false;
                        break;
                    }
                    cost += self(self, next).first;
                    if (cost >= best.first) break;
                }
                if (splits && cost < best.first) best = {cost, q};
            }
            memo.emplace(mask, best);
            return best;
        };

        while (true) {
            int q = solve(solve, remaining).second;
            if (q == -1) {
                vector<int> z;
                for (int i = 0; i < n; ++i)
                    if ((remaining & zero[i]).any()) z.push_back(i);
                answer(z.front(), z.back());
            }
            auto [u, v] = pairs[q];
            remaining &= out[q][ask(u, v)];
        }
    }

    while (true) {
        int possible = 0;
        for (const auto& s : states) possible |= 1 << s.zero;
        if (__builtin_popcount((unsigned)possible) <= 2) {
            int u = __builtin_ctz((unsigned)possible);
            int v = 31 - __builtin_clz((unsigned)possible);
            answer(u, v);
        }

        tuple<int, int, long long> best{INT_MAX, INT_MAX, LLONG_MAX};
        int chosen = -1;
        for (int q = 0; q < m; ++q) {
            int histogram[9][9]{};
            for (const auto& s : states) ++histogram[s.result[q]][s.zero];

            int cost = 0, support = 0, branches = 0;
            long long concentration = 0;
            for (int g = 1; g < n; ++g) {
                int counts[9], k = 0, total = 0;
                for (int i = 0; i < n; ++i) {
                    if (histogram[g][i]) {
                        counts[k++] = histogram[g][i];
                        total += histogram[g][i];
                    }
                }
                if (!k) continue;
                ++branches;
                concentration += 1LL * total * total;
                if (k > 2) {
                    sort(counts, counts + k, greater<int>());
                    for (int i = 0; i < k; ++i)
                        cost += counts[i] * min(i / 2 + 1, (k - 1) / 2);
                    support += total * k;
                }
            }
            auto score = make_tuple(cost, support, concentration);
            if (branches > 1 && score < best) {
                best = score;
                chosen = q;
            }
        }
        auto [u, v] = pairs[chosen];
        int g = ask(u, v);
        erase_if(states, [&](const SmallState& s) {
            return s.result[chosen] != g;
        });
    }
}

class Search {
    int n, limit;
    vector<int> divisor, frequency;
    vector<char> live, restricted;
    vector<vector<pair<int, int>>> adj;
    set<pair<int, int>> candidates, seen;
    unordered_set<uint64_t> edges, replies;

    static uint64_t key(int a, int b) {
        return (uint64_t(a) << 32) | uint32_t(b);
    }

    bool adjacent(int u, int v) const {
        if (u > v) swap(u, v);
        return edges.contains(key(u, v));
    }

    void eliminate(int u) {
        if (live[u]) {
            candidates.erase({divisor[u], u});
            live[u] = false;
        }
    }

    void check_answer() const {
        if (candidates.size() <= 2)
            answer(candidates.begin()->second, candidates.rbegin()->second);
    }

    void update(int u, int d) {
        int old = divisor[u];
        if (!adj[u].empty()) seen.erase({old, u});
        seen.insert({d, u});
        if (old == d) return;
        if (live[u]) {
            candidates.erase({old, u});
            candidates.insert({d, u});
        }
        --frequency[old];
        ++frequency[d];
        divisor[u] = d;
    }

    void check_edge(int u, int v, int g) {
        if (gcd(divisor[u], divisor[v]) != g) answer(u, v);
        if (divisor[u] != g) eliminate(v);
        if (divisor[v] != g) eliminate(u);
    }

    void check_capacity(int d) {
        if (restricted[d] || frequency[d] <= limit / d) return;
        restricted[d] = true;
        for (auto it = candidates.begin(); it != candidates.end();) {
            auto [value, u] = *it;
            ++it;
            if (value != d) eliminate(u);
        }
    }

    int query(int u, int v) {
        int g = ask(u, v);
        if (2 * g > limit) answer(u, v);

        int du = lcm(divisor[u], g);
        int dv = lcm(divisor[v], g);
        if (du > limit) answer(u, u);
        if (dv > limit) answer(v, v);

        bool changed_u = du != divisor[u], changed_v = dv != divisor[v];
        update(u, du);
        update(v, dv);

        // A zero vertex has a different reply for every distinct neighbor.
        if (live[u] && !replies.insert(key(u, g)).second) eliminate(u);
        if (live[v] && !replies.insert(key(v, g)).second) eliminate(v);

        check_edge(u, v, g);
        if (changed_u)
            for (auto [w, old] : adj[u]) check_edge(u, w, old);
        if (changed_v)
            for (auto [w, old] : adj[v]) check_edge(v, w, old);

        adj[u].emplace_back(v, g);
        adj[v].emplace_back(u, g);
        edges.insert(key(min(u, v), max(u, v)));

        if (changed_u) check_capacity(du);
        if (changed_v) check_capacity(dv);
        check_answer();
        return g;
    }

    int choose_probe(int u) const {
        int best = -1;
        tuple<bool, bool, int, bool> best_score{};
        auto consider = [&](int v) {
            if (v == u || adjacent(u, v)) return;
            int combined = lcm(divisor[u], divisor[v]);
            bool decisive = combined > limit;
            auto score = make_tuple(decisive, decisive && live[v], combined, bool(live[v]));
            if (best == -1 || score > best_score) {
                best = v;
                best_score = score;
            }
        };
        int count = 0;
        for (auto it = seen.rbegin(); it != seen.rend() && count < 16; ++it, ++count)
            consider(it->second);
        count = 0;
        for (auto it = candidates.rbegin(); it != candidates.rend() && count < 8; ++it, ++count) {
            if (adj[it->second].empty()) break;
            consider(it->second);
        }
        return best;
    }

    void investigate(int u) {
        for (int steps = 0; steps < 3 && live[u]; ++steps) {
            int v = choose_probe(u);
            if (v == -1) break;
            query(u, v);
        }
    }

public:
    explicit Search(int n)
        : n(n), limit(n - 1), divisor(n, 1), frequency(n), live(n, true),
          restricted(n, false), adj(n) {
        frequency[1] = n;
        edges.reserve(3 * n);
        replies.reserve(4 * n);
        for (int i = 0; i < n; ++i) candidates.emplace(1, i);
    }

    void run() {
        int cutoff = (int)ceil(sqrt(1.2 * limit));
        bool deferred = false;

        // Fresh pairs cover two positions per query. Follow up on large replies.
        for (int u = 0; u + 1 < n; u += 2) {
            int g = query(u, u + 1);
            if (u == 0) {
                deferred = g >= cutoff;
                continue;
            }
            if (u + 3 >= n) break;
            if (deferred) {
                investigate(0);
                investigate(1);
                deferred = false;
            }
            if (g >= cutoff) {
                investigate(u);
                investigate(u + 1);
            }
        }

        // Among live vertices, queried edges form a matching: both endpoint
        // divisors must equal the reply, and a live vertex cannot repeat a reply.
        while (true) {
            auto it = candidates.rbegin();
            int u = (it++)->second;
            while (adjacent(u, it->second)) ++it;
            query(u, it->second);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n == -1) return 0;

    position_id.resize(n);
    iota(position_id.begin(), position_id.end(), 1);
    mt19937 rng((uint32_t)chrono::steady_clock::now().time_since_epoch().count());
    shuffle(position_id.begin(), position_id.end(), rng);

    if (n == 2) answer(0, 1);
    if (n <= 9) solve_small(n);
    else Search(n).run();
}

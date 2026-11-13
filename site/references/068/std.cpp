#include <bits/stdc++.h>
using namespace std;

// All plans below are synthesized from this run's replies, never loaded from a table.
// For n >= 8, the main algorithm is the previously selected support/anchor algorithm.
class SmallPlanner {
public:
    using State = uint32_t;
    struct Node {
        int action = -1;       // zero-based, or -1 for an exactly known state
        int cost = 0;
        State state = 0;
        int child[2] = {-1, -1};
    };
    struct VectorHash {
        size_t operator()(const vector<State>& v) const {
            uint64_t h = 1469598103934665603ULL;
            for (State x : v) h = (h ^ x) * 1099511628211ULL;
            return static_cast<size_t>(h);
        }
    };
    struct MemoEntry { int failed_budget = -1, solution = -1; };

    int n;
    uint64_t calls = 0;
    vector<Node> nodes;
    unordered_map<vector<State>, MemoEntry, VectorHash> memo;

    explicit SmallPlanner(int size) : n(size) {
        if (n < 3 || n > 7) throw runtime_error("invalid small-planner size");
    }
    static int get(State s, int i) { return int((s >> (3 * i)) & 7U); }
    State advance(State s, int i) const {
        int x = get(s, i);
        s &= ~(7U << (3 * i));
        return s | (State((x + 1) % n) << (3 * i));
    }
    State normalize(State s) const {
        int origin = get(s, 0);
        if (origin == 0) return s;
        State result = 0;
        for (int i = 1; i < n; ++i)
            result |= State((get(s, i) - origin + n) % n) << (3 * i);
        return result;
    }
    int response(State s, int i) const {
        for (int j = 0; j < n; ++j)
            if (j != i && get(s, j) == get(s, i)) return 1;
        return 0;
    }
    // Minimum physical moves to some uniform state when every digit is known.
    int physical_distance(State s) const {
        int count[8] = {}, current = 0;
        for (int i = 0; i < n; ++i) {
            int x = get(s, i);
            ++count[x];
            current += (n - x) % n;
        }
        int result = current;
        for (int z = 1; z < n; ++z) {
            current += n - n * count[z];
            result = min(result, current);
        }
        return result;
    }
    static int ceil_log2(int x) {
        return x <= 1 ? 0 : 32 - __builtin_clz(static_cast<unsigned>(x - 1));
    }
    vector<State> initial_states(int support_mask) const {
        vector<State> result;
        function<void(int, int, State)> enumerate = [&](int i, int seen, State s) {
            if (i == n) {
                if (seen == support_mask) result.push_back(s);
                return;
            }
            for (int v = 0; v < n; ++v) if ((support_mask >> v) & 1)
                enumerate(i + 1, seen | (1 << v), s | (State(v) << (3 * i)));
        };
        enumerate(1, 0, 0);
        sort(result.begin(), result.end());
        return result;
    }
    int search(const vector<State>& states, int budget) {
        ++calls;
        int lower = ceil_log2(static_cast<int>(states.size()));
        for (State s : states) lower = max(lower, physical_distance(s));
        if (lower > budget) return -1;
        if (states.size() == 1) {
            int id = static_cast<int>(nodes.size());
            nodes.push_back(Node{-1, lower, states[0], {-1, -1}});
            return id;
        }
        auto found = memo.find(states);
        if (found != memo.end()) {
            int id = found->second.solution;
            if (id >= 0 && nodes[id].cost <= budget) return id;
            if (found->second.failed_budget >= budget) return -1;
        }
        struct Option {
            int action = -1, score = 0, maximum_size = 0;
            long long sum_distance = 0;
            vector<State> branch[2];
        };
        vector<Option> options;
        for (int i = 0; i < n; ++i) {
            Option option;
            option.action = i;
            int maximum_distance[2] = {};
            for (State s : states) {
                State next = advance(s, i);
                int r = response(next, i), d = physical_distance(next);
                maximum_distance[r] = max(maximum_distance[r], d);
                option.sum_distance += d;
                option.branch[r].push_back(normalize(next));
            }
            bool impossible = false;
            for (int r = 0; r < 2; ++r) {
                auto& branch = option.branch[r];
                sort(branch.begin(), branch.end());
                int count = static_cast<int>(branch.size());
                if (count == 0) continue;
                // This score only orders the search, and is NOT a lower bound.
                option.score = max(option.score, maximum_distance[r] + 4 * ceil_log2(count));
                option.maximum_size = max(option.maximum_size, count);
                if (max(maximum_distance[r], ceil_log2(count)) >= budget)
                    impossible = true;
            }
            if (!impossible) options.push_back(move(option));
        }
        sort(options.begin(), options.end(), [](const Option& a, const Option& b) {
            return tie(a.score, a.maximum_size, a.sum_distance, a.action)
                 < tie(b.score, b.maximum_size, b.sum_distance, b.action);
        });
        for (const auto& option : options) {
            int child[2] = {-1, -1}, cost = 0;
            bool possible = true;
            for (int turn = 0; turn < 2; ++turn) {
                int r = option.branch[1].size() > option.branch[0].size() ? 1 - turn : turn;
                if (option.branch[r].empty()) continue;
                child[r] = search(option.branch[r], budget - 1);
                if (child[r] == -1) { possible = false; break; }
                cost = max(cost, nodes[child[r]].cost);
            }
            if (possible) {
                int id = static_cast<int>(nodes.size());
                nodes.push_back(Node{option.action, cost + 1, 0, {child[0], child[1]}});
                memo[states].solution = id;
                return id;
            }
        }
        auto& entry = memo[states];
        entry.failed_budget = max(entry.failed_budget, budget);
        return -1;
    }
    template<class Ask>
    void execute(int root, Ask&& ask) const {
        int id = root;
        while (nodes[id].action != -1) {
            const Node& node = nodes[id];
            int r = ask(node.action + 1);
            id = node.child[r];
            if (id < 0) throw runtime_error("inconsistent interactive reply");
        }
        State s = nodes[id].state;
        int target = 0, best = INT_MAX;
        for (int z = 0; z < n; ++z) {
            int cost = 0;
            for (int i = 0; i < n; ++i) cost += (z - get(s, i) + n) % n;
            if (cost < best) { best = cost; target = z; }
        }
        // Keep the coordinate system of the leaf, even while dial 1 moves.
        for (int i = 0; i < n; ++i) {
            int distance = (target - get(s, i) + n) % n;
            for (int step = 0; step < distance; ++step) ask(i + 1);
        }
    }
};

template<class Ask>
void solve_phase_lock(int n, Ask&& ask) {
    vector<int> present(n, 0), support;
    for (int step = 1; step <= n; ++step) present[step % n] = ask(1);
    for (int v = 0; v < n; ++v) if (present[v]) support.push_back(v);

    // Necessary amendment for the NEW n^2 cap; the old version exceeded it for n=3..7.
    if (n <= 7) {
        int mask = 0;
        for (int v : support) mask |= 1 << v;
        SmallPlanner planner(n);
        auto states = planner.initial_states(mask);
        if (states.empty() || mask == 1) throw runtime_error("invalid initial state");
        int root = planner.search(states, n * n - n);
        if (root == -1) throw runtime_error("small strategy synthesis failed");
        planner.execute(root, ask);
        return;
    }

    // Previously selected on-the-fly support/anchor/round-merging algorithm.
    int extra = n - 1 - static_cast<int>(support.size());
    int anchor = -1, best = INT_MAX;
    for (int x = 0; x < n; ++x) {
        if (present[x]) continue;
        int score = x, farthest = 0;
        for (int v : support) {
            int d = (x - v + n) % n;
            score += d;
            farthest = max(farthest, d);
        }
        score += extra * farthest;
        if (score < best) { best = score; anchor = x; }
    }
    if (anchor == -1) throw runtime_error("no empty anchor");
    for (int step = 0; step < anchor; ++step) ask(1);
    vector<int> baseline(n + 1);
    vector<char> active(n + 1, true);
    int remaining = n - 1;
    for (int i = 2; i <= n; ++i) baseline[i] = ask(i);
    while (remaining > 0) {
        for (int i = 2; i <= n; ++i) {
            if (!active[i]) continue;
            int reply = ask(i);
            if (reply == baseline[i]) continue;
            for (int step = 0; step < 1 + reply; ++step) ask(i);
            active[i] = false;
            --remaining;
        }
    }
    ask(1);
    ask(1);
}

#ifndef PHASE_LOCK_NO_MAIN
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int T;
    if (!(cin >> T) || T < 1 || T > 100) return 0;
    while (T--) {
        int n;
        if (!(cin >> n) || n == -1) return 0;
        if (n < 3 || n > 100) return 0;
        int queries = 0;
        auto ask = [&](int position) {
            if (position < 1 || position > n || ++queries > n * n) exit(2);
            cout << "? " << position << endl;
            int reply;
            if (!(cin >> reply) || reply == -1) exit(0);
            if (reply != 0 && reply != 1) exit(0);
            return reply;
        };
        solve_phase_lock(n, ask);
        cout << "!" << endl;
    }
    return 0;
}
#endif

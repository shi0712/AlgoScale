#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;
using Clock = chrono::steady_clock;

int ceil_log(int x) {
    return x <= 1 ? 0 : 32 - __builtin_clz(unsigned(x - 1));
}

int external_lower_bound(int x) {
    int h = ceil_log(x);
    return (h + 1) * x - (1 << h);
}

struct Model {
    int k, len, count, words, nq;
    bool ordered_anchors;
    vector<pair<int, int>> queries;
    vector<array<unsigned char, 11>> orders;
    vector<vector<U64>> masks;

    Model(int k_, bool ordered) : k(k_), len(k + 2), ordered_anchors(ordered) {
        count = ordered ? 1 : 2;
        for (int i = 0; i < k; ++i) count *= 3;
        words = (count + 63) / 64;

        for (int i = 0; i < len; ++i) {
            for (int j = i + 1; j < len; ++j) {
                if (ordered && i == k && j == k + 1) continue;
                queries.emplace_back(i, j);
            }
        }
        nq = int(queries.size());
        masks.assign(nq, vector<U64>(words));
        orders.reserve(count);

        array<unsigned char, 11> a{};
        auto gen = [&](auto&& self, int pos, int size) -> void {
            if (pos < 0) {
                int id = int(orders.size());
                orders.push_back(a);
                int rank[11];
                for (int i = 0; i < len; ++i) rank[a[i]] = i;
                for (int q = 0; q < nq; ++q) {
                    auto [u, v] = queries[q];
                    if (rank[u] < rank[v]) masks[q][id >> 6] |= U64(1) << (id & 63);
                }
                return;
            }
            for (int r = 0; r < 3; ++r) {
                for (int j = size; j > r; --j) a[j] = a[j - 1];
                a[r] = static_cast<unsigned char>(pos);
                self(self, pos - 1, size + 1);
                for (int j = r; j < size; ++j) a[j] = a[j + 1];
            }
        };
        a[0] = static_cast<unsigned char>(k);
        a[1] = static_cast<unsigned char>(k + 1);
        gen(gen, k - 1, 2);
        if (!ordered) {
            swap(a[0], a[1]);
            gen(gen, k - 1, 2);
        }
    }
};

struct Node {
    int q, less, greater, cost, height;
};

struct Plan {
    shared_ptr<Model> model;
    vector<Node> nodes;
    int root = 0, height = 0;
    double average = 0;
};

struct Search {
    struct Word {
        int index;
        U64 bits;
    };
    struct Entry {
        U64 yes = 0, no = 0;
        int h = 0, result = 0, epoch = 0;
    };
    struct Choice {
        int q, a, lb, imbalance;
        unsigned tie;
    };
    struct Exhausted {};

    shared_ptr<Model> m;
    Clock::time_point deadline;
    vector<Node> pool;
    vector<Entry> table;
    vector<int> lower;
    long long work = 0;
    unsigned calls = 0, seed = 0;
    int epoch = 0, optimize_small = 0, optimize_levels = 0;

    Search(shared_ptr<Model> model, Clock::time_point end)
        : m(std::move(model)), deadline(end) {
        table.resize(1 << (m->count <= 162 ? 13 : 17));
        lower.resize(m->count + 1);
        for (int i = 1; i <= m->count; ++i) lower[i] = external_lower_bound(i);
    }

    static U64 mix(U64 x) {
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    int cost(int id) const { return id > 0 ? pool[id].cost : 0; }
    int height(int id) const { return id > 0 ? pool[id].height : 0; }

    int solve(const vector<Word>& state, int size, U64 active, U64 yes, U64 no,
              int h, int level) {
        if (size == 1) {
            return -1 - (64 * state[0].index + __builtin_ctzll(state[0].bits));
        }
        if (h < ceil_log(size)) return 0;
        h = min(h, size - 1);

        work -= 24 + (static_cast<long long>(state.size()) + 2) * __builtin_popcountll(active);
        if (work < 0 || (++calls % 512 == 0 && Clock::now() >= deadline)) throw Exhausted{};

        Choice choices[55];
        int nc = 0, capacity = 1 << (h - 1);
        for (U64 bits = active; bits; bits &= bits - 1) {
            int q = __builtin_ctzll(bits);
            U64 bit = U64(1) << q;
            const U64* mask = m->masks[q].data();
            int a = 0;
            for (const Word& w : state) a += __builtin_popcountll(w.bits & mask[w.index]);
            if (a == 0) {
                no |= bit;
                active ^= bit;
            } else if (a == size) {
                yes |= bit;
                active ^= bit;
            } else if (max(a, size - a) <= capacity) {
                unsigned tie = seed ? unsigned(mix(U64(seed) * 1234567 + 67 * q + size)) : unsigned(q);
                choices[nc++] = {q, a, size + lower[a] + lower[size - a],
                                 abs(2 * a - size), tie};
            }
        }

        size_t slot = mix(yes ^ (mix(no + 0x9e3779b97f4a7c15ULL)) ^
                          (U64(h) * 0xd6e8feb86659fd93ULL)) & (table.size() - 1);
        Entry e = table[slot];
        if (e.epoch == epoch && e.h == h && e.yes == yes && e.no == no) return e.result;

        sort(choices, choices + nc, [](const Choice& a, const Choice& b) {
            if (a.lb != b.lb) return a.lb < b.lb;
            if (a.imbalance != b.imbalance) return a.imbalance < b.imbalance;
            return a.tie < b.tie;
        });

        bool exact = size <= optimize_small;
        int successes = 0, allowed = level < optimize_levels ? (level == 0 ? 4 : 2) : 1;
        int best_cost = INT_MAX;
        Node best{};
        vector<Word> left, right;
        if (nc) {
            left.reserve(state.size());
            right.reserve(state.size());
        }
        for (int i = 0; i < nc; ++i) {
            Choice c = choices[i];
            if (c.lb >= best_cost) break;
            U64 bit = U64(1) << c.q;
            U64 next = active ^ bit;
            const U64* mask = m->masks[c.q].data();
            left.clear();
            right.clear();
            for (const Word& w : state) {
                U64 a = w.bits & mask[w.index];
                U64 b = w.bits ^ a;
                if (a) left.push_back({w.index, a});
                if (b) right.push_back({w.index, b});
            }

            int l, r;
            if (c.a >= size - c.a) {
                l = solve(left, c.a, next, yes | bit, no, h - 1, level + 1);
                if (!l || size + cost(l) + lower[size - c.a] >= best_cost) continue;
                r = solve(right, size - c.a, next, yes, no | bit, h - 1, level + 1);
                if (!r) continue;
            } else {
                r = solve(right, size - c.a, next, yes, no | bit, h - 1, level + 1);
                if (!r || size + cost(r) + lower[c.a] >= best_cost) continue;
                l = solve(left, c.a, next, yes | bit, no, h - 1, level + 1);
                if (!l) continue;
            }
            ++successes;
            int total = size + cost(l) + cost(r);
            int ht = 1 + max(height(l), height(r));
            if (total < best_cost || (total == best_cost && ht < best.height)) {
                best_cost = total;
                best = {c.q, l, r, total, ht};
            }
            if (best_cost == lower[size] || (!exact && successes >= allowed)) break;
        }

        int id = 0;
        if (best_cost != INT_MAX) {
            id = int(pool.size());
            pool.push_back(best);
        }
        table[slot] = {yes, no, h, id, epoch};
        return id;
    }

    Plan run(int h, long long budget, int small, int levels, unsigned tiebreak) {
        if (Clock::now() >= deadline) return {};
        ++epoch;
        pool.clear();
        pool.push_back({});
        work = budget;
        calls = 0;
        seed = tiebreak;
        optimize_small = small;
        optimize_levels = levels;

        vector<Word> initial;
        initial.reserve(m->words);
        for (int i = 0; i < m->words; ++i) {
            int bits = min(64, m->count - 64 * i);
            initial.push_back({i, bits == 64 ? ~U64(0) : (U64(1) << bits) - 1});
        }

        int root;
        try {
            root = solve(initial, m->count, (U64(1) << m->nq) - 1, 0, 0, h, 0);
        } catch (const Exhausted&) {
            return {};
        }
        if (!root) return {};

        Plan p;
        p.model = m;
        p.height = height(root);
        p.average = double(cost(root)) / m->count;
        p.nodes.reserve(m->count);
        p.nodes.push_back({});
        auto copy_tree = [&](auto&& self, int id) -> int {
            if (id < 0) return id;
            Node x = pool[id];
            x.less = self(self, x.less);
            x.greater = self(self, x.greater);
            int loc = int(p.nodes.size());
            p.nodes.push_back(x);
            return loc;
        };
        p.root = copy_tree(copy_tree, root);
        return p;
    }
};

Plan make_plan(int k, bool ordered, Clock::time_point deadline) {
    if (Clock::now() >= deadline) return {};
    auto model = make_shared<Model>(k, ordered);
    Search search(model, deadline);
    static const long long budgets[] = {
        50000, 100000, 200000, 700000, 2200000,
        5500000, 12500000, 18000000, 33000000, 53000000
    };
    long long budget = budgets[k] * (ordered ? 1 : 2);
    int low = ceil_log(model->count);
    int high = min(2 * k + !ordered, low + 2);

    Plan best;
    for (int h = low; h <= high && Clock::now() < deadline; ++h) {
        best = search.run(h, budget, model->count <= 54 ? model->count : 16, 0, 0);
        if (best.root) break;
    }
    if (!best.root) return best;

    double bound = double(external_lower_bound(model->count)) / model->count;
    if (best.average > bound + 1e-10 && model->count > 54 && Clock::now() < deadline) {
        int small = model->count <= 162 ? model->count : 40;
        Plan refined = search.run(best.height, budget, small, 2, 37 + k);
        if (refined.root &&
            (refined.height < best.height ||
             (refined.height == best.height && refined.average < best.average))) {
            best = std::move(refined);
        }
    }
    return best;
}

bool ask(int a, int b) {
    cout << "? " << a << ' ' << b << '\n' << flush;
    char c;
    if (!(cin >> c) || (c != '<' && c != '>')) exit(0);
    return c == '<';
}

void use_plan(const Plan& plan, int first, deque<int>& order) {
    const Model& m = *plan.model;
    int indices[11];
    for (int i = 0; i < m.k; ++i) indices[i] = first + i;
    if (m.ordered_anchors) {
        indices[m.k] = order.front();
        order.pop_front();
        indices[m.k + 1] = order.front();
        order.pop_front();
    } else {
        indices[m.k] = first + m.k;
        indices[m.k + 1] = first + m.k + 1;
    }
    int id = plan.root;
    while (id > 0) {
        const Node& node = plan.nodes[id];
        auto [a, b] = m.queries[node.q];
        id = ask(indices[a], indices[b]) ? node.less : node.greater;
    }
    const auto& local = m.orders[-1 - id];
    for (int i = m.len - 1; i >= 0; --i) order.push_front(indices[local[i]]);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n <= 0) return 0;
    if (n == 1) {
        cout << "! 1\n" << flush;
        return 0;
    }

    auto deadline = Clock::now() + chrono::milliseconds(2800);
    array<Plan, 10> initial, block;
    for (int k = 0; k <= min(n - 2, 4); ++k) {
        initial[k] = make_plan(k, false, deadline);
    }
    if (n >= 7 && n <= 11) initial[n - 2] = make_plan(n - 2, false, deadline);

    for (int k : {1, 2, 3, 4, 6, 8, 9, 7, 5}) {
        if (k <= n - 2) block[k] = make_plan(k, true, deadline);
    }

    auto weight = [](const Plan& p) {
        return 0.55 * p.height + 0.45 * p.average;
    };
    vector<double> dp(n - 1, 1e100);
    vector<int> step(n - 1, 0);
    dp[0] = 0;
    for (int i = 1; i <= n - 2; ++i) {
        for (int k = 1; k <= min(i, 9); ++k) {
            if (!block[k].root) continue;
            double v = weight(block[k]) + dp[i - k];
            if (v < dp[i] - 1e-9) {
                dp[i] = v;
                step[i] = k;
            }
        }
    }

    int start = -1;
    double score = 1e100;
    for (int k = 0; k <= min(n - 2, 9); ++k) {
        if (!initial[k].root) continue;
        double v = weight(initial[k]) + dp[n - 2 - k];
        if (v < score - 1e-9) {
            score = v;
            start = k;
        }
    }

    deque<int> order;
    if (start >= 0) {
        int remaining = n - start - 2;
        use_plan(initial[start], remaining + 1, order);
        while (remaining) {
            int k = step[remaining];
            use_plan(block[k], remaining - k + 1, order);
            remaining -= k;
        }
    } else {
        if (ask(n - 1, n)) order = {n - 1, n};
        else order = {n, n - 1};
        for (int i = n - 2; i >= 1; --i) {
            if (!ask(i, order[1])) order.insert(order.begin() + 2, i);
            else if (ask(i, order[0])) order.push_front(i);
            else order.insert(order.begin() + 1, i);
        }
    }

    vector<int> answer(n + 1);
    for (int i = 0; i < n; ++i) answer[order[i]] = i + 1;
    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << answer[i];
    cout << '\n' << flush;
    return 0;
}

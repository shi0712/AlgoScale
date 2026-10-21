#include "ballmachine.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <map>
#include <numeric>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ballmachine_c27 {
using std::vector;
constexpr int INF = 1000000;
constexpr int INITIAL = 8;

// Keep validation active even when the grader defines NDEBUG.
inline void require(bool condition) {
    if (!condition) std::abort();
}

int ceil_sqrt(int x) {
    int b = 0;
    while (b * b < x) ++b;
    return b;
}
int triangle(int x) { return x * (x + 1) / 2; }
int popcount(int x) { return __builtin_popcount(static_cast<unsigned>(x)); }

// Bit k denotes row k. All 2^16 integer subset sums are distinct.
constexpr std::array<int, 16> SPECIAL = {
    466, 231, 382, 476, 441, 391, 459, 210,
    364, 62, 470, 341, 59, 60, 91, 306
};

int capacity(int r) {
    if (r == 9) return 16;
    int ans = 0;
    for (int a = 1; a <= r; ++a) ans += popcount(a);
    return ans;
}

struct Code {
    int rows = 0;
    vector<int> masks;                 // One bitmask per column.
    vector<vector<int>> at;            // at[a][j] is the column, or -1.

    Code() = default;
    Code(int r, int columns) : rows(r), at(r + 1) {
        require(columns <= capacity(r));
        if (r == 9) {
            masks.assign(SPECIAL.begin(), SPECIAL.begin() + columns);
            return;
        }
        for (int a = 1; a <= r; ++a) {
            int R = 0;
            for (int bit = 0; bit < 30; ++bit) {
                if (!(a & (1 << bit))) continue;
                R |= 1 << bit;
                int ix = -1;
                if (static_cast<int>(masks.size()) < columns) {
                    ix = static_cast<int>(masks.size());
                    int mask = 0;
                    for (int x = 1; x <= r; ++x) {
                        if (((a ^ R) & x) == (a ^ R) && (popcount(x & R) & 1))
                            mask |= 1 << (x - 1);
                    }
                    masks.push_back(mask);
                }
                at[a].push_back(ix);
            }
        }
        require(static_cast<int>(masks.size()) == columns);
    }
};

struct Group {
    vector<int> leaves;
    bool early = false;
    int early_color = -1;
    int late_color = -1;
    Code code;
    vector<vector<int>> observed;       // [row][original internal-node ID]
};

struct Plan {
    int cost = INF;
    int B = 0;
    vector<int> prefix_batches;         // Cumulative numbers of known long chains.
    bool combined = false;
    int early_colors = 0;
    int late_rounds = 0;
};

Plan choose_plan(int D, int a, int singles) {
    Plan best;
    for (int B = INITIAL; B <= 26; ++B) {
        vector<int> dp(D + 1, INF), previous(D + 1, -1);
        dp[a] = 0;
        for (int d = a; d < D; ++d) {
            if (dp[d] == INF) continue;
            int b = ceil_sqrt(d);
            if (b > B) continue;
            int add = triangle(B) - triangle(b - 1);
            for (int e = d + 1; e <= D && e <= d + add; ++e) {
                if (dp[e] > dp[d] + 1) {
                    dp[e] = dp[d] + 1;
                    previous[e] = d;
                }
            }
        }
        auto consider = [&](int cost, int endpoint, bool combined, int f, int r) {
            if (cost > 27 || cost > best.cost) return;
            // On a tie use fewer collect calls.
            if (cost == best.cost && B <= best.B) return;
            vector<int> batches;
            for (int v = endpoint; v != a; v = previous[v]) {
                require(previous[v] >= a);
                batches.push_back(v);
            }
            std::reverse(batches.begin(), batches.end());
            best = {cost, B, std::move(batches), combined, f, r};
        };
        if (singles == 0) {
            if (dp[D] != INF) consider(B + 1 + dp[D], D, false, 0, 0);
            continue;
        }
        int q = B + 1 - ceil_sqrt(D);
        if (q <= 0) continue;
        if (D == a) {
            for (int r = 1; B + 1 + r <= 27; ++r)
                if (q * capacity(r) >= singles)
                    consider(B + 1 + r, a, false, 0, r);
            continue;
        }
        for (int d = a; d < D; ++d) {
            if (dp[d] == INF) continue;
            int b = ceil_sqrt(d);
            int n = b;
            while (triangle(n) - triangle(b - 1) < D - d) ++n;
            int f = B - n;
            if (f < 0) continue;
            for (int r = 0; B + 2 + dp[d] + r <= 27; ++r) {
                int c = std::min(q, f);
                int can = q * capacity(r)
                        + c * (capacity(r + 1) - capacity(r))
                        + std::max(0, f - q);
                if (can >= singles)
                    consider(B + 2 + dp[d] + r, d, true, f, r);
            }
        }
    }
    require(best.cost <= 27);
    return best;
}

struct Solver {
    int M, N = 0, B = 0;
    vector<int> length, parent, long_leaves, single_leaves;
    vector<vector<int>> chain;
    vector<char> known;
    vector<Group> groups;
    int known_long = 0;

    explicit Solver(int m) : M(m), length(m, 0), chain(m) {}

    void allocate_nodes() {
        N = std::accumulate(length.begin(), length.end(), 0);
        parent.assign(N, -1);
        known.assign(N, false);
        int next = M;
        for (int i = 0; i < M; ++i) {
            chain[i].resize(length[i]);
            for (int k = 0; k < length[i]; ++k) {
                int u;
                if (k + 1 == length[i]) u = i;
                else if (i == 0 && k == 0) u = N - 1;
                else u = next++;
                chain[i][k] = u;
                if (k) parent[u] = chain[i][k - 1];
            }
            if (length[i] >= 2) long_leaves.push_back(i);
        }
        require(next == N - 1);
    }

    // Decode chains with distinct, decreasing, constant colors.
    int parse_initial(const vector<int>& s, int& pos, int color_base) {
        int color = s.at(pos);
        int leaf = color_base - color;
        require(0 <= leaf && leaf < M);
        for (int k = 0; k < length[leaf]; ++k) {
            require(s.at(pos++) == color);
            int u = chain[leaf][k];
            known[u] = true;
            if (k + 1 == length[leaf]) continue;
            while (pos < static_cast<int>(s.size()) && s[pos] < color) {
                int head = parse_initial(s, pos, color_base);
                parent[head] = u;
            }
        }
        return chain[leaf][0];
    }

    struct Scaffold {
        Solver& sol;
        int b;
        vector<vector<int>> children;
        vector<int> color, representative;
        vector<std::map<std::pair<int, int>, int>> identify;

        Scaffold(Solver& s, int alphabet) : sol(s), b(alphabet),
            children(s.N), color(s.N, 0), representative(s.N, -1), identify(s.N) {
            vector<vector<int>> all(s.N);
            for (int v = 0; v < s.N; ++v)
                if (s.known[v] && s.parent[v] >= 0)
                    all[s.parent[v]].push_back(v);
            for (int u = s.M; u < s.N; ++u) if (s.known[u]) {
                for (int v : all[u]) if (v >= s.M) children[u].push_back(v);
                if (children[u].empty()) {
                    require(!all[u].empty());
                    children[u].push_back(all[u][0]);
                    require(all[u][0] < s.M);
                }
            }
            paint(s.N - 1);
            prepare(s.N - 1);
        }
        void paint(int u) {
            if (u < sol.M) return;
            if (children[u][0] < sol.M) {
                require(children[u].size() == 1);
                color[children[u][0]] = 0;
                return;
            }
            for (int v : children[u]) paint(v);
            int L = 0;
            for (int v : children[u]) {
                vector<int> labels;
                for (int w : children[v]) labels.push_back(color[w]);
                std::sort(labels.begin(), labels.end());
                labels.erase(std::unique(labels.begin(), labels.end()), labels.end());
                int count = static_cast<int>(labels.size());
                require(1 <= count && count <= b);
                int R = L + count - 1;
                color[v] = R / b;
                require(color[v] < b);
                for (int w : children[v]) {
                    int rank = static_cast<int>(
                        std::lower_bound(labels.begin(), labels.end(), color[w]) - labels.begin());
                    color[w] = (L + rank) % b;
                }
                L += count;
            }
        }
        void prepare(int u) {
            if (u < sol.M) {
                representative[u] = u;
                return;
            }
            for (int v : children[u]) prepare(v);
            representative[u] = representative[children[u][0]];
            if (children[u][0] >= sol.M) {
                for (int v : children[u]) {
                    int low = b;
                    for (int w : children[v]) low = std::min(low, color[w]);
                    auto result = identify[u].emplace(std::make_pair(color[v], low), v);
                    require(result.second);
                }
            }
        }
        void fill(int u, int shift) const {
            bool ok = insert(representative[u], color[u] + shift);
            require(ok);
            for (int v : children[u]) fill(v, shift);
        }
    };

    // Negative entries encode singleton colors; nonnegative entries are chain heads.
    struct Parser {
        Solver& sol;
        Scaffold& old;
        const vector<int>& sequence;
        int pos = 0, shift, singleton_colors;
        const vector<vector<int>>& pair_leaf;
        const vector<std::pair<int, int>>& observations; // color -> (group, row)

        Parser(Solver& s, Scaffold& sc, const vector<int>& seq, int sh, int f,
               const vector<vector<int>>& pl,
               const vector<std::pair<int, int>>& obs)
            : sol(s), old(sc), sequence(seq), shift(sh), singleton_colors(f),
              pair_leaf(pl), observations(obs) {}

        void attach(int u, const vector<int>& pending) {
            for (int x : pending) {
                if (x >= 0) sol.parent[x] = u;
                else {
                    int color = -x - 1;
                    auto [g, row] = observations.at(color);
                    require(g >= 0 && row >= 0);
                    ++sol.groups[g].observed[row][u];
                }
            }
        }
        vector<int> extras(int upper) {
            vector<int> result;
            while (pos < static_cast<int>(sequence.size()) && sequence[pos] <= upper) {
                if (sequence[pos] < singleton_colors)
                    result.push_back(-sequence[pos++] - 1);
                else result.push_back(new_chain());
            }
            return result;
        }
        int new_chain() {
            int a = sequence.at(pos++);
            require(singleton_colors <= a && a < shift);
            vector<int> pending = extras(a);
            int y = sequence.at(pos++);
            require(a < y && y <= sol.B);
            int leaf = pair_leaf.at(a).at(y);
            require(0 <= leaf && leaf < sol.M);
            const auto& path = sol.chain[leaf];
            require(path.size() >= 2);
            for (int u : path) {
                require(!sol.known[u]);
                sol.known[u] = true;
            }
            attach(path[0], pending);
            for (int k = 1; k + 1 < static_cast<int>(path.size()); ++k) {
                attach(path[k], extras(a));
                require(sequence.at(pos++) == sol.B);
            }
            return path[0];
        }
        void old_children(int u) {
            if (u < sol.M) return;
            if (old.children[u][0] < sol.M) {
                int v = old.children[u][0];
                require(sequence.at(pos++) == shift + old.color[v]);
                return;
            }
            for (std::size_t k = 0; k < old.children[u].size(); ++k) {
                int own = sequence.at(pos++) - shift;
                require(0 <= own && own < old.b);
                vector<int> pending = extras(shift - 1);
                int first_child = sequence.at(pos) - shift;
                auto it = old.identify[u].find({own, first_child});
                require(it != old.identify[u].end());
                int v = it->second;
                attach(v, pending);
                old_children(v);
            }
        }
        void run() {
            int root = sol.N - 1;
            require(sequence.at(pos++) == shift + old.color[root]);
            attach(root, extras(shift - 1));
            old_children(root);
            require(pos == static_cast<int>(sequence.size()));
        }
    };

    vector<std::pair<int, int>> insert_observations(bool early, int round, int colors) {
        vector<std::pair<int, int>> mapping(colors, {-1, -1});
        for (int g = 0; g < static_cast<int>(groups.size()); ++g) {
            Group& G = groups[g];
            int color, row;
            if (early) {
                if (!G.early) continue;
                color = G.early_color;
                row = 0;
            } else {
                if (G.late_color < 0) continue;
                color = G.late_color;
                row = round + static_cast<int>(G.early);
            }
            require(0 <= color && color < colors && row < G.code.rows);
            mapping[color] = {g, row};
            for (int j = 0; j < static_cast<int>(G.leaves.size()); ++j) {
                if ((G.code.masks[j] >> row) & 1) {
                    bool ok = insert(G.leaves[j], color);
                    require(ok);
                }
            }
        }
        return mapping;
    }

    void discover(int new_count, int early_colors) {
        require(known_long < new_count);
        int b = ceil_sqrt(known_long);
        Scaffold old(*this, b);
        int shift = B + 1 - b;
        old.fill(N - 1, shift);
        vector<vector<int>> pair_leaf(B + 1, vector<int>(B + 1, -1));
        int current = known_long;
        // The heads are nonincreasing. This is essential, not cosmetic.
        for (int a = shift - 1; a >= early_colors && current < new_count; --a) {
            for (int y = a + 1; y <= B && current < new_count; ++y) {
                int leaf = long_leaves[current++];
                pair_leaf[a][y] = leaf;
                for (int k = 0; k < length[leaf]; ++k) {
                    int value = k == 0 ? a : (k == 1 ? y : B);
                    bool ok = insert(leaf, value);
                    require(ok);
                }
            }
        }
        require(current == new_count);
        auto obs = insert_observations(true, 0, early_colors);
        auto seq = collect();
        Parser(*this, old, seq, shift, early_colors, pair_leaf, obs).run();
        known_long = new_count;
    }

    void singleton_round(int round, int q) {
        int b = ceil_sqrt(static_cast<int>(long_leaves.size()));
        Scaffold old(*this, b);
        old.fill(N - 1, q);
        auto obs = insert_observations(false, round, q);
        auto seq = collect();
        vector<vector<int>> unused;
        Parser(*this, old, seq, q, q, unused, obs).run();
    }

    void make_groups(int q, int f, int r) {
        int at = 0;
        auto add = [&](int cap, int rows, int early_color, int late_color) {
            int take = std::min(cap, static_cast<int>(single_leaves.size()) - at);
            if (take == 0) return;
            Group G;
            G.leaves.assign(single_leaves.begin() + at, single_leaves.begin() + at + take);
            at += take;
            G.early = early_color >= 0;
            G.early_color = early_color;
            G.late_color = late_color;
            G.code = Code(rows, take);
            G.observed.assign(rows, vector<int>(N, 0));
            groups.push_back(std::move(G));
        };
        for (int j = 0; j < q; ++j) {
            bool e = j < f;
            add(capacity(r + static_cast<int>(e)), r + static_cast<int>(e),
                e ? j : -1, j);
        }
        for (int j = q; j < f; ++j) add(1, 1, j, -1);
        require(at == static_cast<int>(single_leaves.size()));
    }

    void decode_groups() {
        for (Group& G : groups) {
            int g = static_cast<int>(G.leaves.size());
            if (G.code.rows == 9) {
                vector<std::uint64_t> packed_columns(g, 0), sums(1 << g, 0);
                for (int j = 0; j < g; ++j)
                    for (int k = 0; k < 9; ++k)
                        if ((G.code.masks[j] >> k) & 1)
                            packed_columns[j] |= std::uint64_t(1) << (5 * k);
                std::unordered_map<std::uint64_t, int> lookup;
                lookup.reserve(1 << g);
                lookup.emplace(0, 0);
                for (int mask = 1; mask < (1 << g); ++mask) {
                    int bit = __builtin_ctz(static_cast<unsigned>(mask));
                    sums[mask] = sums[mask ^ (1 << bit)] + packed_columns[bit];
                    auto ok = lookup.emplace(sums[mask], mask);
                    require(ok.second);
                }
                for (int u = M; u < N; ++u) {
                    std::uint64_t key = 0;
                    for (int k = 0; k < 9; ++k)
                        key |= std::uint64_t(G.observed[k][u]) << (5 * k);
                    auto it = lookup.find(key);
                    require(it != lookup.end());
                    int mask = it->second;
                    for (int j = 0; j < g; ++j) if ((mask >> j) & 1) {
                        require(parent[G.leaves[j]] == -1);
                        parent[G.leaves[j]] = u;
                    }
                }
            } else {
                int r = G.code.rows;
                for (int u = M; u < N; ++u) {
                    vector<int> y(r + 1, 0);
                    for (int x = 1; x <= r; ++x) y[x] = G.observed[x - 1][u];
                    for (int a = r; a >= 1; --a) {
                        int coefficient = 0;
                        for (int x = a;; x = (x - 1) & a) {
                            coefficient += ((popcount(a) - popcount(x)) & 1) ? -y[x] : y[x];
                            if (x == 0) break;
                        }
                        for (int ix : G.code.at[a]) {
                            int digit = ((coefficient % 2) + 2) % 2;
                            coefficient = (coefficient - digit) / -2;
                            if (ix == -1) { require(digit == 0); continue; }
                            if (!digit) continue;
                            require(parent[G.leaves[ix]] == -1);
                            parent[G.leaves[ix]] = u;
                            for (int x = 1; x <= r; ++x)
                                if ((G.code.masks[ix] >> (x - 1)) & 1) --y[x];
                        }
                        require(coefficient == 0);
                    }
                    for (int x : y) require(x == 0);
                }
            }
        }
    }

    vector<int> run() {
        int base = M <= INITIAL ? M - 1 : INITIAL;
        for (int i = 0; i < M; ++i) {
            int value = M <= INITIAL ? base - i : (i < INITIAL ? INITIAL - i : 0);
            while (insert(i, value)) ++length[i];
        }
        vector<int> initial = collect();
        allocate_nodes();
        require(static_cast<int>(initial.size()) == N);
        if (M > INITIAL)
            initial.erase(std::remove(initial.begin(), initial.end(), 0), initial.end());
        int pos = 0;
        require(parse_initial(initial, pos, base) == N - 1);
        require(pos == static_cast<int>(initial.size()));
        if (M <= INITIAL) {
            parent.pop_back();
            return parent;
        }
        for (int i = 0; i < INITIAL; ++i) if (length[i] >= 2) ++known_long;
        for (int i = INITIAL; i < M; ++i) if (length[i] == 1) single_leaves.push_back(i);
        int D = static_cast<int>(long_leaves.size());
        Plan plan = choose_plan(D, known_long, static_cast<int>(single_leaves.size()));
        B = plan.B;
        // These rounds must not insert singleton observations before all long chains are filled.
        for (int endpoint : plan.prefix_batches) discover(endpoint, 0);
        int q = B + 1 - ceil_sqrt(D);
        if (!single_leaves.empty())
            make_groups(q, plan.early_colors, plan.late_rounds);
        if (plan.combined) discover(D, plan.early_colors);
        require(known_long == D);
        for (int r = 0; r < plan.late_rounds; ++r) singleton_round(r, q);
        decode_groups();
        for (int i = 0; i < N - 1; ++i) require(parent[i] >= M && parent[i] < N);
        require(parent[N - 1] == -1);
        parent.pop_back();
        return parent;
    }
};
} // namespace ballmachine_c27

std::vector<int> find_structure(int M) {
    return ballmachine_c27::Solver(M).run();
}

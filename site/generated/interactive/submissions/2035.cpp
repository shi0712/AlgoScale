#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>
using namespace std;

constexpr int MAX_N = 600;
constexpr int WORDS = (MAX_N + 63) / 64;

struct Mask {
    array<uint64_t, WORDS> word{};

    void add(int v) { word[v / 64] |= uint64_t(1) << (v % 64); }
    void remove(int v) { word[v / 64] &= ~(uint64_t(1) << (v % 64)); }
    bool contains(int v) const { return (word[v / 64] >> (v % 64)) & 1; }
    int size() const {
        int result = 0;
        for (uint64_t w : word) result += popcount(w);
        return result;
    }
    friend Mask operator|(Mask a, const Mask& b) {
        for (int i = 0; i < WORDS; ++i) a.word[i] |= b.word[i];
        return a;
    }
    friend Mask operator^(Mask a, const Mask& b) {
        for (int i = 0; i < WORDS; ++i) a.word[i] ^= b.word[i];
        return a;
    }
    bool operator==(const Mask&) const = default;
};

struct MaskHash {
    size_t operator()(const Mask& set) const {
        uint64_t hash = 0x9e3779b97f4a7c15ULL;
        for (uint64_t w : set.word) {
            hash ^= w + 0x9e3779b97f4a7c15ULL + (hash << 6) + (hash >> 2);
        }
        hash ^= hash >> 30;
        hash *= 0xbf58476d1ce4e5b9ULL;
        hash ^= hash >> 27;
        return hash;
    }
};

Mask make_mask(const vector<int>& vertices, int l, int r) {
    Mask result;
    for (int i = l; i < r; ++i) result.add(vertices[i]);
    return result;
}

Mask make_mask(const vector<int>& vertices) {
    return make_mask(vertices, 0, int(vertices.size()));
}

class Oracle {
    unordered_map<Mask, int, MaskHash> counts;

public:
    Oracle() { counts.reserve(16384); }

    void remember(const Mask& set, int edges) {
        if (set.size() >= 2) counts.emplace(set, edges);
    }

    int ask(const Mask& set) {
        int k = set.size();
        if (k < 2) return 0;
        auto it = counts.find(set);
        if (it != counts.end()) return it->second;

        cout << "? " << k << '\n';
        bool first = true;
        for (int i = 0; i < WORDS; ++i) {
            for (uint64_t w = set.word[i]; w; w &= w - 1) {
                if (!first) cout << ' ';
                first = false;
                cout << i * 64 + countr_zero(w) + 1;
            }
        }
        cout << '\n' << flush;
        int answer;
        if (!(cin >> answer) || answer < 0 || answer > k * (k - 1) / 2) exit(0);
        counts.emplace(set, answer);
        return answer;
    }
};

[[noreturn]] void finish(char type, const vector<int>& vertices) {
    cout << type << ' ' << vertices.size() << '\n';
    for (int i = 0; i < int(vertices.size()); ++i) {
        if (i != 0) cout << ' ';
        cout << vertices[i] + 1;
    }
    cout << '\n' << flush;
    exit(0);
}

// Recover a vector of bounded integer counts from subset sums. Query rows are
// indexed by a downwards-closed family of masks. Each column supported on a
// coordinate set M has a prescribed coefficient at M in the Boolean Mobius
// transform. The leading coefficients of columns at M are mixed-radix weights.
struct InvalidCounts {};

class CountCode {
    struct Column {
        int index;
        int mask;
        int weight;
    };

    vector<Column> columns;
    int length = 1;

    void add_column(int index, int mask, int weight) {
        columns.push_back({index, mask, weight});
        length = mask + 1;
    }

public:
    template<class Query>
    vector<int> recover(const vector<int>& vertices, const vector<int>& upper,
                        Query query, int unit_cost = 1) {
        int n = int(vertices.size());
        if (n == 0) return {};
        columns.clear();
        length = 1;
        vector<int> small, large;
        for (int i = n - 1; i >= 0; --i) {
            (upper[i] == 1 ? small : large).push_back(i);
        }
        auto take = [&](vector<int>& bucket, int mask, int weight) {
            add_column(bucket.back(), mask, weight);
            bucket.pop_back();
        };
        take(large.empty() ? small : large, 0, 1);
        for (int mask = 1; !small.empty() || !large.empty(); ++mask) {
            int limit = 1 << (popcount(unsigned(mask)) - 1);
            int weight = 1;
            while (!small.empty() || !large.empty()) {
                if (!small.empty() && weight * 2 <= limit) {
                    take(small, mask, weight);
                    weight *= 2;
                } else if (!large.empty() && weight * (upper[large.back()] + 1) <= limit) {
                    int radix = upper[large.back()] + 1;
                    take(large, mask, weight);
                    weight *= radix;
                } else {
                    take(large.empty() ? small : large, mask, weight);
                    break;
                }
            }
        }

        vector<int> values(n);
        if (length * unit_cost >= n) {
            for (int i = 0; i < n; ++i) {
                Mask set;
                set.add(vertices[i]);
                values[i] = query(set);
            }
            return values;
        }

        vector<Mask> sets(length);
        vector<vector<int>> coefficients(columns.size());
        for (int i = 0; i < int(columns.size()); ++i) {
            const auto& [index, mask, weight] = columns[i];
            vector<int>& coef = coefficients[i];
            coef.assign(mask + 1, 0);
            int left = weight;
            for (int sub = mask;; sub = (sub - 1) & mask) {
                if ((popcount(unsigned(mask ^ sub)) & 1) == 0) {
                    coef[sub] = 1;
                    if (--left == 0) break;
                }
            }
            for (int row = 0; row < length; ++row) {
                if (coef[row & mask]) sets[row].add(vertices[index]);
            }
            for (int bits = mask; bits; bits &= bits - 1) {
                int bit = bits & -bits;
                for (int sub = mask; sub != 0; sub = (sub - 1) & mask) {
                    if (sub & bit) coef[sub] -= coef[sub ^ bit];
                }
            }
        }
        vector<int> sums(length);
        for (int row = 0; row < length; ++row) sums[row] = query(sets[row]);
        for (int bit = 1; bit < length; bit *= 2) {
            for (int row = 0; row < length; ++row) {
                if (row & bit) sums[row] -= sums[row ^ bit];
            }
        }
        for (int i = int(columns.size()) - 1; i >= 0; --i) {
            const auto& [index, mask, weight] = columns[i];
            int value = sums[mask] / weight;
            if (value < 0 || value > upper[index]) throw InvalidCounts{};
            values[index] = value;
            for (int sub = mask;; sub = (sub - 1) & mask) {
                sums[sub] -= value * coefficients[i][sub];
                if (sub == 0) break;
            }
        }
        if (any_of(sums.begin(), sums.end(), [](int v) { return v != 0; })) throw InvalidCounts{};
        return values;
    }
};

class Solver {
    struct Group {
        vector<int> vertices;
        Mask mask;
    };
    struct ParentInfo {
        int vertex = -1;
        int begin = 0;
        int prefix = 0;
        int degree = 0;
    };
    struct PairRow {
        int v;
        vector<pair<int, int>> candidates;
    };
    struct Merge {
        Mask members;
        int left = -1, right = -1;
        int source = -1, target = -1;
        vector<int> candidates;
        int degree = 0;
        bool independent = false;
    };

    int n;
    int total = 0;
    vector<int> order;
    Mask full;
    Oracle oracle;
    mt19937 rng;
    vector<int> leader, component_size, parity, forest_root;
    vector<Merge> forest;
    int components;
    array<vector<int>, 2> sides;
    vector<ParentInfo> dense_parent;
    vector<int> dense_color;
    vector<int> dense_order;
    array<int, 2> scanned{};
    vector<int> rank;

    int representative(int v) {
        if (leader[v] == v) return v;
        int p = leader[v];
        leader[v] = representative(p);
        parity[v] ^= parity[p];
        return leader[v];
    }

    int cross(const Mask& a, const Mask& b, int ea, int eb) {
        return oracle.ask(a | b) - ea - eb;
    }

    int degree_into(int v, const Mask& set, int edges) {
        Mask together = set;
        together.add(v);
        return oracle.ask(together) - edges;
    }

    // The candidates contain at least one neighbor. An exact degree, when
    // available, lets us omit tests forced by the capacity of either half.
    int find_neighbor(int v, const vector<int>& candidates, bool independent, int degree = -1) {
        int l = 0, r = int(candidates.size());
        while (r - l > 1) {
            if (degree == r - l) return candidates[l];
            int mid = l + (r - l) / 2;
            if (degree > r - mid) {
                degree = -1;
                r = mid;
                continue;
            }
            Mask half = make_mask(candidates, l, mid);
            int count = degree_into(v, half, independent ? 0 : oracle.ask(half));
            if (count > 0) {
                degree = count;
                r = mid;
            } else {
                l = mid;
            }
        }
        return candidates[l];
    }

    pair<int, int> find_edge(const vector<int>& vertices, int edges) {
        int size = int(vertices.size());
        if (edges == size * (size - 1) / 2) return {vertices[0], vertices[1]};
        int mid = size / 2;
        Mask a = make_mask(vertices, 0, mid);
        int ea = oracle.ask(a);
        if (ea > 0) return find_edge(vector<int>(vertices.begin(), vertices.begin() + mid), ea);
        Mask b = make_mask(vertices, mid, size);
        int eb = oracle.ask(b);
        if (eb > 0) return find_edge(vector<int>(vertices.begin() + mid, vertices.end()), eb);

        int l = 0, r = mid;
        int between = edges;
        while (r - l > 1) {
            int split = (l + r) / 2;
            if (between > (r - split) * (size - mid)) {
                between = -1;
                r = split;
                continue;
            }
            int left = cross(make_mask(vertices, l, split), b, 0, 0);
            if (left > 0) {
                between = left;
                r = split;
            } else {
                l = split;
            }
        }
        return {vertices[l], find_neighbor(vertices[l], vector<int>(vertices.begin() + mid, vertices.end()),
                                           true, between)};
    }

    // A bridge can initially name a set of possible endpoints of the same
    // parity. Resolve it only if it lies on the certificate's tree path.
    void append_path(int node, int a, int b, vector<int>& path) {
        if (a == b) {
            path.push_back(a);
            return;
        }
        auto& merge = forest[node];
        bool left_a = forest[merge.left].members.contains(a);
        bool left_b = forest[merge.left].members.contains(b);
        if (left_a == left_b) {
            append_path(left_a ? merge.left : merge.right, a, b, path);
            return;
        }
        if (merge.target == -1) {
            merge.target = find_neighbor(merge.source, merge.candidates, merge.independent, merge.degree);
        }
        if (left_a) {
            append_path(merge.left, a, merge.source, path);
            append_path(merge.right, merge.target, b, path);
        } else {
            append_path(merge.right, a, merge.target, path);
            append_path(merge.left, merge.source, b, path);
        }
    }

    [[noreturn]] void odd_from_tree(int a, int b) {
        vector<int> path;
        append_path(forest_root[representative(a)], a, b, path);
        finish('N', path);
    }

    [[noreturn]] void verify_tree() {
        array<vector<int>, 2> color;
        for (int v : order) {
            representative(v);
            color[parity[v]].push_back(v);
        }
        if (color[0].size() > color[1].size()) swap(color[0], color[1]);
        if (total != n - 1) {
            for (const auto& side : color) {
                int edges = oracle.ask(make_mask(side));
                if (edges != 0) {
                    auto [a, b] = find_edge(side, edges);
                    odd_from_tree(a, b);
                }
            }
        }
        finish('Y', color[0]);
    }

    void link(int a, const vector<int>& candidates, int degree, bool independent = false) {
        int b = candidates.front();
        if (candidates.size() == 1) {
            Mask edge;
            edge.add(a);
            edge.add(b);
            oracle.remember(edge, 1);
        }
        int x = representative(a), y = representative(b);
        if (x == y) {
            if (parity[a] == parity[b]) {
                odd_from_tree(a, find_neighbor(a, candidates, independent, degree));
            }
            return;
        }
        int left = forest_root[x], right = forest_root[y];
        Merge merge;
        merge.members = forest[left].members | forest[right].members;
        merge.left = left;
        merge.right = right;
        merge.source = a;
        if (candidates.size() == 1 || int(candidates.size()) == degree) merge.target = b;
        else merge.candidates = candidates;
        merge.degree = degree;
        merge.independent = independent || total == n - 1;
        if (component_size[x] < component_size[y]) swap(x, y);
        leader[y] = x;
        parity[y] = parity[a] ^ parity[b] ^ 1;
        component_size[x] += component_size[y];
        forest_root[x] = int(forest.size());
        forest.push_back(move(merge));
        if (--components == 1) verify_tree();
    }

    void add_edge(int a, int b) { link(a, {b}, 1); }

    vector<Group> units_for(const vector<int>& vertices) {
        vector<Group> units;
        vector<int> slot(2 * n, -1);
        for (int v : vertices) {
            int root = representative(v);
            int& index = slot[2 * root + parity[v]];
            if (index == -1) {
                index = int(units.size());
                units.emplace_back();
            }
            units[index].vertices.push_back(v);
            units[index].mask.add(v);
        }
        return units;
    }

    void split_independent(const vector<int>& vertices, int l, int r, vector<Group>& groups) {
        Mask set = make_mask(vertices, l, r);
        if (oracle.ask(set) == 0) {
            groups.push_back({vector<int>(vertices.begin() + l, vertices.begin() + r), set});
            return;
        }
        int mid = (l + r) / 2;
        split_independent(vertices, l, mid, groups);
        split_independent(vertices, mid, r, groups);
    }

    vector<Group> pack_independent(const vector<int>& scope, bool global) {
        vector<Group> pieces, groups;
        split_independent(scope, 0, int(scope.size()), pieces);
        stable_sort(pieces.begin(), pieces.end(), [](const Group& a, const Group& b) {
            return a.vertices.size() > b.vertices.size();
        });
        Group first;
        for (auto& piece : pieces) {
            int size = int(piece.vertices.size());
            int between = cross(first.mask, piece.mask, 0, 0);
            vector<int> degree(size);
            get_degrees(piece.vertices, 0, size, first.mask, 0, between, 0,
                        int(first.vertices.size()), degree);
            vector<int> rejected;
            Mask remainder;
            for (int i = 0; i < size; ++i) {
                int v = piece.vertices[i];
                if (degree[i] == 0) {
                    first.vertices.push_back(v);
                    first.mask.add(v);
                } else {
                    rejected.push_back(v);
                    remainder.add(v);
                }
            }
            piece = {move(rejected), remainder};
        }
        if (global && oracle.ask(full ^ first.mask) == 0) finish('Y', first.vertices);
        for (auto& piece : pieces) {
            if (piece.vertices.empty()) continue;
            vector<int> options(groups.size());
            iota(options.begin(), options.end(), 0);
            stable_sort(options.begin(), options.end(), [&](int a, int b) {
                return groups[a].vertices.size() < groups[b].vertices.size();
            });
            int attempts = 0;
            bool merged = false;
            for (int i : options) {
                auto& group = groups[i];
                if (++attempts > 4) break;
                Mask joined = group.mask | piece.mask;
                if (oracle.ask(joined) == 0) {
                    group.mask = joined;
                    group.vertices.insert(group.vertices.end(), piece.vertices.begin(), piece.vertices.end());
                    merged = true;
                    break;
                }
            }
            if (!merged) groups.push_back(move(piece));
        }
        stable_sort(groups.begin(), groups.end(), [](const Group& a, const Group& b) {
            return a.vertices.size() > b.vertices.size();
        });
        groups.insert(groups.begin(), move(first));
        return groups;
    }

    void get_degrees(const vector<int>& from, int l, int r, const Mask& to, int base,
                     int count, int lo, int hi, vector<int>& degree) {
        if (count == (r - l) * lo) {
            fill(degree.begin() + l, degree.begin() + r, lo);
        } else if (count == (r - l) * hi) {
            fill(degree.begin() + l, degree.begin() + r, hi);
        } else if (r - l == 1) {
            degree[l] = count;
        } else {
            int mid = (l + r) / 2;
            int left_count = cross(make_mask(from, l, mid), to, 0, base);
            get_degrees(from, l, mid, to, base, left_count, lo, hi, degree);
            get_degrees(from, mid, r, to, base, count - left_count, lo, hi, degree);
        }
    }

    vector<int> measure(const vector<int>& rows, const vector<int>& cap, const vector<int>& lower,
                        const Mask& sample) {
        if (rows.empty()) return {};
        int edges = oracle.ask(sample);
        vector<pair<int, int>> offsets;
        for (int i = 0; i < int(rows.size()); ++i) {
            if (lower[i] != 0) offsets.emplace_back(rows[i], lower[i]);
        }
        CountCode code;
        auto result = code.recover(rows, cap, [&](const Mask& subset) {
            int value = cross(subset, sample, 0, edges);
            for (auto [v, offset] : offsets) {
                if (subset.contains(v)) value -= offset;
            }
            return value;
        });
        for (int i = 0; i < int(rows.size()); ++i) result[i] += lower[i];
        return result;
    }

    void link_pair(int v, pair<int, int> pair, const vector<Group>& to) {
        auto [a, b] = pair;
        if (a == b) {
            link(v, to[a].vertices, 2);
        } else {
            link(v, to[a].vertices, 1);
            link(v, to[b].vertices, 1);
        }
    }

    void resolve_pairs(vector<PairRow> pending, const vector<Group>& to) {
        while (!pending.empty()) {
            if (total == n - 1) {
                for (auto& row : pending) {
                    int root = representative(row.v);
                    erase_if(row.candidates, [&](pair<int, int> p) {
                        int a = representative(to[p.first].vertices.front());
                        int b = representative(to[p.second].vertices.front());
                        return a == b || root == a || root == b;
                    });
                    assert(!row.candidates.empty());
                    if (row.candidates.size() == 1) link_pair(row.v, row.candidates[0], to);
                }
                erase_if(pending, [](const PairRow& row) { return row.candidates.size() == 1; });
                if (pending.empty()) return;
            }
            vector<char> best(to.size());
            double best_score = -1;
            for (int attempt = 0; attempt < 7; ++attempt) {
                vector<char> test(to.size());
                uint32_t bits = 0;
                for (int i = 0; i < int(to.size()); ++i) {
                    if (i % 32 == 0) bits = rng();
                    test[i] = (bits >> (i % 32)) & 1;
                }
                double score = 0;
                for (const auto& row : pending) {
                    array<int, 3> counts{};
                    for (auto [a, b] : row.candidates) ++counts[test[a] + test[b]];
                    int size = int(row.candidates.size());
                    int squares = counts[0] * counts[0] + counts[1] * counts[1] + counts[2] * counts[2];
                    score += 1.0 - double(squares) / (size * size);
                }
                if (score > best_score) {
                    best_score = score;
                    best = move(test);
                }
            }
            if (best_score == 0.0) {
                auto [a, b] = pending[0].candidates[0];
                auto [x, y] = pending[0].candidates[1];
                fill(best.begin(), best.end(), 0);
                best[a != x && a != y ? a : b] = 1;
            }
            Mask sample;
            for (int i = 0; i < int(to.size()); ++i) if (best[i]) sample = sample | to[i].mask;
            vector<int> rows, cap, low, active;
            for (int i = 0; i < int(pending.size()); ++i) {
                int lo = 2, hi = 0;
                for (auto [a, b] : pending[i].candidates) {
                    int score = best[a] + best[b];
                    lo = min(lo, score);
                    hi = max(hi, score);
                }
                if (lo != hi) {
                    rows.push_back(pending[i].v);
                    cap.push_back(hi - lo);
                    low.push_back(lo);
                    active.push_back(i);
                }
            }
            vector<int> answers = measure(rows, cap, low, sample);
            for (int i = 0; i < int(active.size()); ++i) {
                auto& row = pending[active[i]];
                erase_if(row.candidates, [&](pair<int, int> p) {
                    return best[p.first] + best[p.second] != answers[i];
                });
                if (row.candidates.size() == 1) link_pair(row.v, row.candidates[0], to);
            }
            erase_if(pending, [](const PairRow& row) { return row.candidates.size() == 1; });
        }
    }

    void learn_low_degree(const vector<int>& rows, const vector<int>& degree, const vector<Group>& to) {
        int b = int(to.size());
        vector<int> different(rows.size()), shared(rows.size());
        for (int bit = int(bit_floor(unsigned(b - 1))); bit != 0; bit /= 2) {
            Mask subset;
            int selected = 0;
            for (int i = 0; i < b; ++i) {
                if (i & bit) {
                    subset = subset | to[i].mask;
                    selected += int(to[i].vertices.size());
                }
            }
            vector<int> active, vertices, cap, low;
            for (int i = 0; i < int(rows.size()); ++i) {
                if (degree[i] == 1 && different[i] + bit >= b) continue;
                active.push_back(i);
                vertices.push_back(rows[i]);
                cap.push_back(min(degree[i], selected));
                low.push_back(0);
            }
            vector<int> found = measure(vertices, cap, low, subset);
            for (int i = 0; i < int(active.size()); ++i) {
                if (found[i] == 1) different[active[i]] |= bit;
                if (found[i] == 2) shared[active[i]] |= bit;
            }
        }
        vector<PairRow> pending;
        for (int i = 0; i < int(rows.size()); ++i) {
            if (degree[i] == 1) {
                link(rows[i], to[different[i]].vertices, 1);
                continue;
            }
            int diff = different[i], both = shared[i];
            if (diff == 0) {
                link(rows[i], to[both].vertices, 2);
                continue;
            }
            int rest = diff ^ int(bit_floor(unsigned(diff)));
            PairRow row{rows[i], {}};
            for (int sub = rest;; sub = (sub - 1) & rest) {
                int a = both | sub, b2 = both | (sub ^ diff);
                if (b2 < b) row.candidates.emplace_back(a, b2);
                if (sub == 0) break;
            }
            assert(!row.candidates.empty());
            if (row.candidates.size() == 1) {
                link_pair(row.v, row.candidates[0], to);
            } else {
                pending.push_back(move(row));
            }
        }
        resolve_pairs(move(pending), to);
    }

    void span_star(int v, const vector<int>& to, int l, int r, int degree) {
        if (degree == 0) return;
        if (degree == r - l) {
            for (int i = l; i < r; ++i) add_edge(v, to[i]);
            return;
        }
        int root = representative(v);
        int first_root = representative(to[l]), first_color = parity[to[l]];
        if (all_of(to.begin() + l + 1, to.begin() + r,
                   [&](int w) { return representative(w) == first_root && parity[w] == first_color; })) {
            if (root != first_root) {
                link(v, vector<int>(to.begin() + l, to.begin() + r), degree);
            }
            return;
        }
        if (root == first_root && all_of(to.begin() + l + 1, to.begin() + r,
                                        [&](int w) { return representative(w) == root; })) return;
        int mid = (l + r) / 2;
        Mask half = make_mask(to, l, mid);
        int first = degree_into(v, half, oracle.ask(half));
        span_star(v, to, l, mid, first);
        span_star(v, to, mid, r, degree - first);
    }

    void span_rectangle(const vector<int>& from, const vector<int>& degrees, const vector<int>& to) {
        int width = int(to.size());
        vector<int> low_rows, low_degrees, high_rows, high_degrees;
        for (int i = 0; i < int(from.size()); ++i) {
            int degree = degrees[i];
            if (degree == 0) continue;
            if (degree == width) {
                for (int w : to) add_edge(from[i], w);
            } else if (degree <= 2) {
                low_rows.push_back(from[i]);
                low_degrees.push_back(degree);
            } else {
                high_rows.push_back(from[i]);
                high_degrees.push_back(degree);
            }
        }
        if (high_rows.size() >= 9 && width >= 10) {
            int mid = width / 2;
            vector<int> active, cap, lower, ix;
            vector<int> first(high_rows.size()), second(high_rows.size());
            for (int i = 0; i < int(high_rows.size()); ++i) {
                int lo = max(0, high_degrees[i] - (width - mid));
                int hi = min(high_degrees[i], mid);
                first[i] = lo;
                if (hi == lo) continue;
                active.push_back(high_rows[i]);
                cap.push_back(hi - lo);
                lower.push_back(lo);
                ix.push_back(i);
            }
            vector<int> values = measure(active, cap, lower, make_mask(to, 0, mid));
            for (int i = 0; i < int(ix.size()); ++i) first[ix[i]] = values[i];
            for (int i = 0; i < int(high_rows.size()); ++i) second[i] = high_degrees[i] - first[i];
            span_rectangle(high_rows, first, vector<int>(to.begin(), to.begin() + mid));
            span_rectangle(high_rows, second, vector<int>(to.begin() + mid, to.end()));
        } else {
            for (int i = 0; i < int(high_rows.size()); ++i) {
                span_star(high_rows[i], to, 0, width, high_degrees[i]);
            }
        }
        if (!low_rows.empty()) learn_low_degree(low_rows, low_degrees, units_for(to));
    }

    void sparse(const vector<int>& scope, int sub_total, bool global) {
        vector<Group> groups = pack_independent(scope, global);
        vector<Mask> suffix(groups.size() + 1);
        for (int i = int(groups.size()) - 1; i >= 0; --i) suffix[i] = suffix[i + 1] | groups[i].mask;
        for (int i = 0; i + 1 < int(groups.size()); ++i) {
            const auto& from = groups[i].vertices;
            int inside = oracle.ask(suffix[i + 1]);
            int between = sub_total - inside;
            sub_total = inside;
            if (between == 0) continue;
            vector<int> to;
            for (int j = i + 1; j < int(groups.size()); ++j) {
                to.insert(to.end(), groups[j].vertices.begin(), groups[j].vertices.end());
            }
            shuffle(to.begin(), to.end(), rng);
            vector<int> degree(from.size());
            get_degrees(from, 0, int(from.size()), suffix[i + 1], inside, between,
                        i == 0 && global ? 1 : 0, int(to.size()), degree);
            span_rectangle(from, degree, to);
        }
    }

    int actual_parent(int v) {
        ParentInfo& parent = dense_parent[v];
        if (parent.vertex == -1 && parent.prefix > 0) {
            const auto& base = sides[dense_color[v] ^ 1];
            vector<int> candidates(base.begin() + parent.begin, base.begin() + parent.prefix);
            parent.vertex = find_neighbor(v, candidates, true, parent.degree);
        }
        return parent.vertex;
    }

    [[noreturn]] void odd_from_dense(int a, int b) {
        vector<int> path_a, path_b;
        for (int v = a; v != -1; v = actual_parent(v)) path_a.push_back(v);
        for (int v = b; v != -1; v = actual_parent(v)) path_b.push_back(v);
        while (path_a.size() >= 2 && path_b.size() >= 2 &&
               path_a[path_a.size() - 2] == path_b[path_b.size() - 2]) {
            path_a.pop_back();
            path_b.pop_back();
        }
        path_a.insert(path_a.end(), next(path_b.rbegin()), path_b.rend());
        finish('N', path_a);
    }

    void check_dense_side(int color) {
        auto& side = sides[color];
        int edges = oracle.ask(make_mask(side));
        if (edges > 0) {
            auto [a, b] = find_edge(side, edges);
            odd_from_dense(a, b);
        }
    }

    void assign_dense(int v, int color, int prefix, int degree, int parent = -1) {
        dense_color[v] = color;
        dense_parent[v] = {parent, scanned[color ^ 1], prefix, degree};
        sides[color].push_back(v);
        dense_order.push_back(v);
    }

    void finish_sparse_remainder(const vector<int>& unknown, int internal, int preferred) {
        for (int i = 1; i < int(dense_order.size()); ++i) {
            int v = dense_order[i];
            const ParentInfo& parent = dense_parent[v];
            const auto& side = sides[dense_color[v] ^ 1];
            if (parent.vertex != -1) add_edge(v, parent.vertex);
            else link(v, vector<int>(side.begin() + parent.begin, side.begin() + parent.prefix),
                      parent.degree, true);
        }
        sparse(unknown, internal, false);

        array<Group, 2> frontier;
        for (int c = 0; c < 2; ++c) {
            auto& group = frontier[c];
            group.vertices.assign(sides[c].begin() + scanned[c], sides[c].end());
            group.mask = make_mask(group.vertices);
        }
        vector<vector<int>> parts(n);
        for (int v : unknown) parts[representative(v)].push_back(v);
        for (const auto& part : parts) {
            if (part.empty()) continue;
            int size = int(part.size());
            Mask set = make_mask(part);
            int inside = oracle.ask(set);
            int color = preferred;
            int count = cross(set, frontier[color].mask, inside, 0);
            if (count == 0) {
                color ^= 1;
                count = cross(set, frontier[color].mask, inside, 0);
            }
            assert(count > 0);
            const auto& target = frontier[color];
            int l = 0, r = size;
            while (r - l > 1) {
                int mid = (l + r) / 2;
                Mask half = make_mask(part, l, mid);
                int left_count = cross(half, target.mask, oracle.ask(half), 0);
                if (left_count > 0) {
                    count = left_count;
                    r = mid;
                } else {
                    l = mid;
                }
            }
            link(part[l], target.vertices, count, true);
        }
    }

    void dense_scan(const vector<int>& unknown, int l, int r, int begin_rank, int end_rank,
                    const Mask& base, int color, int count, vector<int>& left) {
        if (count == 0) {
            left.insert(left.end(), unknown.begin() + l, unknown.begin() + r);
            return;
        }
        int prefix = int(sides[color ^ 1].size());
        int width = prefix - scanned[color ^ 1];
        if (r - l == 1) {
            assign_dense(unknown[l], color, prefix, count);
            return;
        }
        if (count == (r - l) * width) {
            for (int i = l; i < r; ++i) assign_dense(unknown[i], color, prefix, width);
            return;
        }
        if (count > (r - l) / 3 || r - l < 5) {
            for (int i = l; i < r; ++i) {
                int v = unknown[i];
                int degree = degree_into(v, base, 0);
                if (degree == 0) left.push_back(v);
                else assign_dense(v, color, prefix, degree);
                if (i == l + 27 && r - l >= 80) check_dense_side(color);
            }
            return;
        }
        int middle_rank = (begin_rank + end_rank) / 2;
        int mid = int(lower_bound(unknown.begin() + l, unknown.begin() + r, middle_rank,
                                  [&](int v, int position) { return rank[v] < position; }) - unknown.begin());
        if (mid == l) {
            dense_scan(unknown, l, r, middle_rank, end_rank, base, color, count, left);
            return;
        }
        if (mid == r) {
            dense_scan(unknown, l, r, begin_rank, middle_rank, base, color, count, left);
            return;
        }
        Mask a = make_mask(unknown, l, mid);
        int first_count = cross(a, base, oracle.ask(a), 0);
        dense_scan(unknown, l, mid, begin_rank, middle_rank, base, color, first_count, left);
        dense_scan(unknown, mid, r, middle_rank, end_rank, base, color, count - first_count, left);
    }

    void try_complete_bipartite(int root_degree) {
        // Under the complete-bipartite hypothesis, e(S) = x (|S| - x).
        // A fixed sample from the larger side usually disambiguates the root.
        // The two final zero counts certify the candidate independently.
        int sampled = min(n, 24 + int(1.55 * sqrt(n)));
        array<vector<int>, 2> tentative;
        tentative[0].push_back(order[0]);
        for (int i = 1; i < sampled; ++i) {
            Mask pair;
            pair.add(order[0]);
            pair.add(order[i]);
            tentative[oracle.ask(pair)].push_back(order[i]);
        }
        int side = root_degree > n - root_degree;
        if (root_degree * 2 == n) side = tentative[1].size() > tentative[0].size();
        if (tentative[side].size() < 12) return;

        Mask anchors = make_mask(tentative[side]);
        int anchor_count = int(tentative[side].size());
        int anchor = tentative[side][0];
        vector<int> rows(order.begin() + sampled, order.end());
        try {
            CountCode code;
            auto values = code.recover(rows, vector<int>(rows.size(), 1), [&](const Mask& set) {
                int size = set.size();
                if (size == 0) return 0;
                Mask together = set | anchors;
                int k = size + anchor_count;
                int edges = oracle.ask(together);
                int delta = k * k - 4 * edges;
                if (delta < 0) throw InvalidCounts{};
                int difference = int(sqrt(delta));
                if (difference * difference != delta) throw InvalidCounts{};
                int opposite = (k - difference) / 2;
                if (opposite >= anchor_count && difference <= 12) {
                    together.remove(anchor);
                    opposite = edges - oracle.ask(together);
                }
                if (opposite < 0 || opposite > size || opposite * (k - opposite) != edges) {
                    throw InvalidCounts{};
                }
                return opposite;
            });
            for (int i = 0; i < int(rows.size()); ++i) {
                tentative[side ^ values[i]].push_back(rows[i]);
            }
            if (int(tentative[1].size()) == root_degree &&
                oracle.ask(make_mask(tentative[0])) == 0 && oracle.ask(make_mask(tentative[1])) == 0) {
                finish('Y', tentative[side ^ 1]);
            }
        } catch (const InvalidCounts&) {
            // An inconsistent hypothesis is discarded.
        }
    }

    void dense(double density) {
        int trial_count = density >= .37 ? 15 : density >= .23 ? 7 : 0;
        for (int a = 1; a <= n / 2; ++a) {
            if (a * (n - a) == total) {
                trial_count = min(trial_count, 5);
                break;
            }
        }
        for (int i = 0; i < trial_count; ++i) {
            int a = rng() % n, b = rng() % n, c = rng() % n;
            if (a == b || a == c || b == c) continue;
            vector<int> triple{a, b, c};
            if (oracle.ask(make_mask(triple)) == 3) finish('N', triple);
        }

        int best = 0, root_degree = 0;
        int preferred = max(4, int(density * n * .60));
        for (int i = 0; i < min(n, 8); ++i) {
            Mask rest = full;
            rest.remove(order[i]);
            int d = total - oracle.ask(rest);
            if (d > root_degree) {
                best = i;
                root_degree = d;
            }
            if (root_degree >= preferred) break;
        }
        swap(order[0], order[best]);
        rank.resize(n);
        for (int i = 0; i < n; ++i) rank[order[i]] = i;
        int root = order[0];
        assign_dense(root, 0, 0, 0);
        int seed_limit = min(n - 1, max(24, int(8 / density)));
        bool product = false;
        if (density >= .08 && n >= 32) product = root_degree * (n - root_degree) == total;
        if (product && n >= 140) {
            try_complete_bipartite(root_degree);
            product = false;
        }
        vector<int> unknown;
        if (product) {
            vector<int> rows(order.begin() + 1, order.end());
            CountCode code;
            auto values = code.recover(rows, vector<int>(rows.size(), 1), [&](const Mask& set) {
                return degree_into(root, set, oracle.ask(set));
            }, 2);
            for (int i = 0; i < int(rows.size()); ++i) {
                if (values[i]) assign_dense(rows[i], 1, 0, 1, root);
                else unknown.push_back(rows[i]);
            }
            check_dense_side(1);
        } else if (density >= .075) {
            for (int i = 1; i <= seed_limit; ++i) {
                Mask set;
                set.add(order[i]);
                set.add(root);
                if (oracle.ask(set) != 0) {
                    assign_dense(order[i], 1, 0, 1, root);
                    if (sides[1].size() == 3 || sides[1].size() == 6) check_dense_side(1);
                } else {
                    unknown.push_back(order[i]);
                }
            }
            check_dense_side(1);
            unknown.insert(unknown.end(), order.begin() + 1 + seed_limit, order.end());
        } else {
            unknown.assign(order.begin() + 1, order.end());
        }

        int base_color = sides[1].empty() ? 0 : 1;
        int seed_goal = min(n / 6, int(3.8 / density));
        if (!product && density >= .09 && !sides[1].empty() && int(sides[1].size()) < seed_goal) {
            Mask base = make_mask(sides[1]);
            vector<int> left;
            int i = 0;
            int limit = min(int(unknown.size()), min(n / 4, seed_goal * 3));
            for (; i < limit && int(sides[0].size()) < seed_goal; ++i) {
                int v = unknown[i];
                int degree = degree_into(v, base, 0);
                if (degree == 0) left.push_back(v);
                else assign_dense(v, 0, int(sides[1].size()), degree);
                if (i == 23 && limit >= 35) check_dense_side(0);
            }
            check_dense_side(0);
            left.insert(left.end(), unknown.begin() + i, unknown.end());
            unknown = move(left);
            if (sides[0].size() > sides[1].size()) base_color = 0;
        }
        while (!unknown.empty()) {
            int end = int(sides[base_color].size());
            Mask base = make_mask(sides[base_color], scanned[base_color], end);
            Mask rest = make_mask(unknown);
            int internal = oracle.ask(rest);
            int count = cross(base, rest, 0, internal);
            if (count == 0 && internal == 0) {
                vector<int> answer = sides[base_color];
                answer.insert(answer.end(), unknown.begin(), unknown.end());
                finish('Y', answer.size() < sides[base_color ^ 1].size() ? answer : sides[base_color ^ 1]);
            }
            int size = int(unknown.size());
            int bridge_limit = max(2, size / 65);
            if (size >= 100 && internal * 10 <= 12 * size && count <= bridge_limit) {
                int other = base_color ^ 1;
                Mask other_base = make_mask(sides[other], scanned[other], int(sides[other].size()));
                int other_count = cross(other_base, rest, 0, internal);
                if (count + other_count <= bridge_limit) {
                    finish_sparse_remainder(unknown, internal, count >= other_count ? base_color : other);
                }
            }
            vector<int> left;
            dense_scan(unknown, 0, int(unknown.size()), 0, n, base, base_color ^ 1, count, left);
            scanned[base_color] = end;
            if (left.size() != unknown.size()) {
                check_dense_side(base_color ^ 1);
                if (oracle.ask(make_mask(left)) == 0) {
                    // All rejected vertices have zero edges to this round's base.
                    vector<int> answer = sides[base_color];
                    answer.insert(answer.end(), left.begin(), left.end());
                    finish('Y', answer.size() < sides[base_color ^ 1].size() ? answer : sides[base_color ^ 1]);
                }
            }
            unknown = move(left);
            base_color ^= 1;
        }
        finish('Y', sides[0].size() < sides[1].size() ? sides[0] : sides[1]);
    }

public:
    explicit Solver(int n)
        : n(n), order(n), rng(uint32_t(chrono::steady_clock::now().time_since_epoch().count())),
          leader(n), component_size(n, 1), parity(n), forest_root(n), forest(n), components(n),
          dense_parent(n), dense_color(n, -1) {
        iota(order.begin(), order.end(), 0);
        shuffle(order.begin(), order.end(), rng);
        iota(leader.begin(), leader.end(), 0);
        iota(forest_root.begin(), forest_root.end(), 0);
        forest.reserve(2 * n - 1);
        for (int v = 0; v < n; ++v) forest[v].members.add(v);
        full = make_mask(order);
    }

    void run() {
        if (n <= 2) finish('Y', {0});
        total = oracle.ask(full);
        if (total == n * (n - 1) / 2) finish('N', {0, 1, 2});
        if (n == 3) {
            for (int v = 0; v < 2; ++v) {
                Mask pair = full;
                pair.remove(v);
                if (oracle.ask(pair) == 0) finish('Y', {v});
            }
            finish('Y', {2});
        }
        double density = double(total) / (n * (n - 1) / 2);
        if (n >= 22 && (density >= .085 || total * 5 >= 8 * n)) dense(density);
        sparse(order, total, true);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 1 || n > MAX_N) return 0;
    Solver(n).run();
}

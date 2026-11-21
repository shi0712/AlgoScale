#include <algorithm>
#include <array>
#include <chrono>
#include <compare>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;
using U64 = uint64_t;
using Clock = chrono::steady_clock;

// Slice denominators have only factors from the query denominator and edge spans.
// Doubled lattice area is an integer at most 2,000,000, so its residue is exact.
constexpr U64 prime = (U64(1) << 61) - 1;

U64 add_mod(U64 a, U64 b) {
    U64 sum = a + b;
    return sum >= prime ? sum - prime : sum;
}

U64 sub_mod(U64 a, U64 b) {
    return a >= b ? a - b : a + prime - b;
}

U64 mul_mod(U64 a, U64 b) {
    __uint128_t product = __uint128_t(a) * b;
    return add_mod(U64(product) & prime, U64(product >> 61));
}

U64 pow_mod(U64 a, U64 e) {
    U64 result = 1;
    for (; e; e >>= 1, a = mul_mod(a, a)) {
        if (e & 1) result = mul_mod(result, a);
    }
    return result;
}

struct Point { int x, y; };
using Edge = pair<int, int>;

int64_t cross(Point a, Point b, Point c) {
    return int64_t(b.x - a.x) * (c.y - a.y) - int64_t(b.y - a.y) * (c.x - a.x);
}

int64_t determinant(Point a, Point b) {
    return int64_t(a.x) * b.y - int64_t(a.y) * b.x;
}

bool overlap(int a, int b, int c, int d) {
    return max(min(a, b), min(c, d)) <= min(max(a, b), max(c, d));
}

bool incompatible(const vector<Point>& points, Edge first, Edge second) {
    auto [a, b] = first;
    auto [c, d] = second;
    if (b == c || b == d) swap(a, b);
    if (a == d) swap(c, d);
    Point A = points[a], B = points[b], C = points[c], D = points[d];
    if (a == c) {
        return cross(A, B, D) == 0 &&
               (B.x - A.x) * (D.x - A.x) + (B.y - A.y) * (D.y - A.y) > 0;
    }
    if (!overlap(A.x, B.x, C.x, D.x) || !overlap(A.y, B.y, C.y, D.y)) return false;
    return cross(A, B, C) * cross(A, B, D) <= 0 &&
           cross(C, D, A) * cross(C, D, B) <= 0;
}

struct Probe {
    int p = 0, q = 1;
    Probe() = default;
    Probe(int numerator, int denominator) {
        int divisor = gcd(numerator, denominator);
        p = numerator / divisor;
        q = denominator / divisor;
    }
    auto operator<=>(const Probe&) const = default;
};

struct Reply {
    U64 width;

    U64 residue(int multiplier) const { return mul_mod(width, multiplier); }
};

U64 read_residue(bool positive) {
    string token;
    if (!(cin >> token) || token.empty()) exit(0);
    U64 residue = 0;
    bool zero = true;
    for (char c : token) {
        if (c < '0' || c > '9') exit(0);
        zero &= c == '0';
        residue = add_mod(mul_mod(residue, 10), c - '0');
    }
    if (positive && (zero || residue == 0)) exit(0);
    return residue;
}

struct Oracle {
    map<Probe, Reply> cache;

    const Reply& ask(Probe probe) {
        auto found = cache.find(probe);
        if (found != cache.end()) return found->second;
        cout << "? " << probe.p << ' ' << probe.q << endl;
        U64 numerator = read_residue(false);
        U64 denominator = read_residue(true);
        return cache.emplace(probe, Reply{mul_mod(numerator, pow_mod(denominator, prime - 2))}).first->second;
    }
};

struct Arithmetic {
    array<U64, 1001> inverse{};

    Arithmetic() {
        for (int i = 1; i <= 1000; ++i) inverse[i] = pow_mod(i, prime - 2);
    }

    // Directed CCW boundary contribution to 2q times the slice length.
    U64 edge_residue(Point a, Point b, Probe probe, bool from_left = false) const {
        if (a.x == b.x) {
            return !from_left && probe.p == a.x * probe.q ? abs(a.y - b.y) * probe.q : 0;
        }
        bool positive = a.x > b.x;
        if (positive) swap(a, b);
        int offset = probe.p - a.x * probe.q;
        int span = b.x - a.x;
        if (offset < 0 || offset > span * probe.q || (from_left && offset == 0)) return 0;
        int height = a.y * span * probe.q + (b.y - a.y) * offset;
        if (from_left || (offset != 0 && offset != span * probe.q)) height *= 2;
        U64 value = mul_mod(height, inverse[span]);
        return positive ? value : sub_mod(0, value);
    }
};

struct Column {
    int x;
    vector<int> vertices;
    int left = -1, right = -1;
};

struct Node {
    int col, side, weight, id;
};

struct Slab { int left, right; };

vector<int> boundary_order(const vector<Point>& points);

struct Model {
    bool reflected;
    vector<Point> points;
    vector<int> hull, boundary_index;
    vector<Column> columns;
    vector<int> col_of;
    vector<Node> nodes;
    vector<Slab> slabs;
    vector<vector<int>> incident;

    Model(vector<Point> vertices, bool mirror) : reflected(mirror), points(move(vertices)), col_of(points.size()) {
        vector<int> order(points.size());
        iota(order.begin(), order.end(), 0);
        if (reflected) for (Point& p : points) p.x = 1000 - p.x;
        hull = boundary_order(points);
        boundary_index.assign(points.size(), -1);
        for (int i = 0; i < int(hull.size()); ++i) boundary_index[hull[i]] = i;
        sort(order.begin(), order.end(), [&](int a, int b) {
            return tie(points[a].x, points[a].y) < tie(points[b].x, points[b].y);
        });
        for (int v : order) {
            if (columns.empty() || columns.back().x != points[v].x) columns.push_back({points[v].x, {}});
            col_of[v] = int(columns.size()) - 1;
            columns.back().vertices.push_back(v);
        }
        auto make_node = [&](int col, int side, int weight) {
            int index = int(nodes.size());
            nodes.push_back({col, side, weight, index});
            return index;
        };
        for (int i = 0; i < int(columns.size()); ++i) {
            auto& col = columns[i];
            int left = i == 0 ? 0 : col.x - columns[i - 1].x;
            int right = i + 1 == int(columns.size()) ? 0 : columns[i + 1].x - col.x;
            bool may_jump = false;
            for (int j = 1; j < int(col.vertices.size()); ++j) {
                if (possible_edge(col.vertices[j - 1], col.vertices[j])) may_jump = true;
            }
            if (!may_jump) {
                col.left = col.right = make_node(i, 0, left + right);
            } else {
                if (left) col.left = make_node(i, -1, left);
                if (right) col.right = make_node(i, 1, right);
            }
        }
        if (reflected) for (auto& node : nodes) node.id = int(nodes.size()) - 1 - node.id;
        incident.resize(nodes.size());
        for (int i = 0; i + 1 < int(columns.size()); ++i) {
            int left = columns[i].right, right = columns[i + 1].left;
            incident[left].push_back(i);
            incident[right].push_back(i);
            slabs.push_back({left, right});
        }
    }

    bool possible_edge(int a, int b) const {
        // Boundary vertices occur in hull order in every simple polygon.
        int i = boundary_index[a], j = boundary_index[b];
        if (i == -1 || j == -1) return true;
        int gap = abs(i - j);
        return gap == 1 || gap + 1 == int(hull.size());
    }

    Probe original(Probe probe) const {
        return reflected ? Probe(1000 * probe.q - probe.p, probe.q) : probe;
    }
};

struct Measurement {
    bool done = false;
    optional<U64> width;
};

struct Action {
    enum Kind { Direct, WithKnown, Pair, Triple, Two };
    Kind kind = Direct;
    int first = -1, second = -1, slab = -1;
    array<Probe, 2> probes;
    int count = 1;
};

struct Plan {
    int cost;
    vector<Action> actions;
};

struct Integration {
    const Arithmetic& arith;
    Oracle& oracle;
    vector<Measurement> data;
    U64 total = 0;
    int remaining;

    Integration(const Arithmetic& arithmetic, Oracle& judge, const Model& model)
        : arith(arithmetic), oracle(judge), data(model.nodes.size()), remaining(model.nodes.size()) {
        for (const Node& n : model.nodes) {
            if (n.side == 0 && (n.col == 0 || n.col + 1 == int(model.columns.size()))) set_width(n, 0);
        }
    }

    void consume(const Node& node, U64 contribution) {
        auto& m = data[node.id];
        if (m.done) return;
        m.done = true;
        --remaining;
        total = add_mod(total, contribution);
    }

    void set_width(const Node& node, U64 width) {
        auto& m = data[node.id];
        if (m.width) return;
        if (!m.done) consume(node, mul_mod(width, node.weight));
        m.width = width;
    }

    Action two_queries(const Model& model, int i, int j, int slab) const {
        int a = model.columns[slab].x, b = model.columns[slab + 1].x;
        return {Action::Two, i, j, slab, {Probe(2 * a + b, 3), Probe(a + 2 * b, 3)}, 2};
    }

    Plan plan(const Model& model, bool prefer_extreme = false) const {
        int size = model.nodes.size();
        vector<int> dp(size + 1, 0);
        vector<Action> choice(size);
        for (int i = size - 1; i >= 0; --i) {
            const Node& node = model.nodes[i];
            dp[i] = dp[i + 1];
            if (data[node.id].done) continue;
            int best = 4 * size + 10;
            auto consider = [&](const Action& action, int next, bool prefer) {
                int cost = dp[next];
                for (int probe = 0; probe < action.count; ++probe) {
                    cost += !oracle.cache.contains(model.original(action.probes[probe]));
                }
                if (cost < best || (prefer && cost == best)) {
                    best = cost;
                    choice[i] = action;
                }
            };
            if (node.side == 0 || node.col == 0 || node.col + 1 == int(model.columns.size())) {
                consider({Action::Direct, i, -1, -1, {Probe(model.columns[node.col].x, 1), {}}, 1}, i + 1, false);
            }
            for (int slab : model.incident[i]) {
                auto [a, b] = model.slabs[slab];
                int other = (a == i ? b : a);
                if (data[model.nodes[other].id].width) {
                    int x = model.columns[slab].x + model.columns[slab + 1].x;
                    consider({Action::WithKnown, i, other, slab, {Probe(x, 2), {}}, 1}, i + 1, false);
                } else if (node.side != 0) {
                    consider(two_queries(model, i, -1, slab), i + 1, false);
                }
                // A slice at the weighted mean represents both trapezoid endpoint terms.
                if (a == i && b == i + 1 && !data[model.nodes[b].id].done) {
                    const Node& next = model.nodes[b];
                    int p = node.weight * model.columns[node.col].x + next.weight * model.columns[next.col].x;
                    Probe probe(p, node.weight + next.weight);
                    if (probe.q <= 1000) {
                        consider({Action::Pair, i, b, slab, {probe, {}}, 1}, i + 2, !prefer_extreme || node.col != 0);
                    } else {
                        consider(two_queries(model, i, b, slab), i + 2, false);
                    }
                    if (slab + 1 >= int(model.slabs.size()) || model.slabs[slab + 1].left != b) continue;
                    int c = model.slabs[slab + 1].right;
                    const Node& last = model.nodes[c];
                    if (c != b + 1 || data[last.id].done) continue;
                    int x = model.columns[node.col].x, y = model.columns[next.col].x, z = model.columns[last.col].x;
                    int left = y - x, right = z - y;
                    Probe first(node.weight * x + left * y, node.weight + left);
                    Probe second(right * y + last.weight * z, right + last.weight);
                    // Split the middle coefficient between its two incident slabs.
                    if (first.q <= 1000 && second.q <= 1000) {
                        consider({Action::Triple, i, c, slab, {first, second}, 2}, i + 3, false);
                    }
                }
            }
            dp[i] = best;
        }
        Plan result{dp[0], {}};
        for (int i = 0; i < size; ++i) {
            if (data[model.nodes[i].id].done) continue;
            auto action = choice[i];
            result.actions.push_back(action);
            if (action.kind == Action::Triple) i += 2;
            else if (action.kind == Action::Pair || (action.kind == Action::Two && action.second != -1)) ++i;
        }
        return result;
    }

    const Reply& execute(const Model& model, const Action& action) {
        const Node& first = model.nodes[action.first];
        const Reply& reply = oracle.ask(model.original(action.probes[0]));
        if (action.kind == Action::Direct) {
            set_width(first, reply.width);
        } else if (action.kind == Action::WithKnown) {
            const auto& other_width = *data[model.nodes[action.second].id].width;
            set_width(first, sub_mod(reply.residue(2), other_width));
        } else if (action.kind == Action::Pair) {
            const Node& second = model.nodes[action.second];
            consume(first, reply.residue(first.weight + second.weight));
            consume(second, 0);
        } else if (action.kind == Action::Triple) {
            const Reply& other = oracle.ask(model.original(action.probes[1]));
            const Node& middle = model.nodes[action.first + 1];
            const Node& last = model.nodes[action.second];
            int x = model.columns[first.col].x;
            int y = model.columns[middle.col].x;
            int z = model.columns[last.col].x;
            consume(first, reply.residue(first.weight + y - x));
            consume(middle, 0);
            consume(last, other.residue(last.weight + z - y));
        } else {
            const Reply& other = oracle.ask(model.original(action.probes[1]));
            U64 f = reply.width, g = other.width;
            auto [a, b] = model.slabs[action.slab];
            set_width(model.nodes[a], sub_mod(add_mod(f, f), g));
            set_width(model.nodes[b], sub_mod(add_mod(g, g), f));
        }
        return reply;
    }
};

vector<int> boundary_order(const vector<Point>& points) {
    vector<int> order(points.size());
    iota(order.begin(), order.end(), 0);
    sort(order.begin(), order.end(), [&](int a, int b) {
        return tie(points[a].x, points[a].y) < tie(points[b].x, points[b].y);
    });
    auto chain = [&](const vector<int>& sorted) {
        vector<int> result;
        for (int v : sorted) {
            while (result.size() >= 2 && cross(points[result[result.size() - 2]], points[result.back()], points[v]) < 0) {
                result.pop_back();
            }
            result.push_back(v);
        }
        result.pop_back();
        return result;
    };
    vector<int> hull = chain(order);
    reverse(order.begin(), order.end());
    vector<int> upper = chain(order);
    hull.insert(hull.end(), upper.begin(), upper.end());
    return hull;
}

struct Candidate {
    int area;
    int start;
    int count;
};

class EnumeratedSolver {
    const Model& model;
    const Arithmetic& arith;
    Oracle& oracle;
    Clock::time_point deadline;
    int n;
    vector<int> hull, interior;
    vector<int> slots;
    int hull_area = 0;
    vector<Candidate> candidates;
    vector<int> changes;
    vector<Probe> probes;
    vector<vector<U64>> signals;
    vector<U64> base;

    struct Bucket {
        U64 key = 0;
        int area = 0, count = 0, stamp = 0;
        bool mixed = false;
    };
    vector<Bucket> buckets;
    int stamp = 0;

    struct Tree {
        enum { AreaLeaf = -1, RefineLeaf = -2, LocalProbe = -3 };
        int probe = AreaLeaf;
        int area = 0;
        vector<pair<U64, int>> branches;
        vector<int> unresolved;
        Probe location;
    };
    vector<Tree> tree;
    int tree_work = 0;
    static constexpr int tree_budget = 205'000'000;

    Edge hull_edge(int slot) const {
        return {hull[slot], hull[(slot + 1) % hull.size()]};
    }

    void build_candidate() {
        array<Edge, 24> inserted;
        array<Edge, 12> removed;
        int ins = 0, rem = 0;
        for (int i = 0; i < int(slots.size());) {
            auto [a, b] = hull_edge(slots[i]);
            removed[rem++] = {a, b};
            int prev = a;
            do {
                inserted[ins++] = {prev, interior[i]};
                prev = interior[i++];
            } while (i < int(slots.size()) && slots[i] == slots[i - 1]);
            inserted[ins++] = {prev, b};
        }
        for (int i = 0; i < ins; ++i) {
            for (int j = 0; j < i; ++j) {
                if (incompatible(model.points, inserted[i], inserted[j])) return;
            }
        }
        int area = hull_area, start = changes.size();
        for (int i = 0; i < ins; ++i) {
            auto [a, b] = inserted[i];
            area += determinant(model.points[a], model.points[b]);
            changes.push_back(a * n + b);
        }
        for (int i = 0; i < rem; ++i) {
            auto [a, b] = removed[i];
            area -= determinant(model.points[a], model.points[b]);
            changes.push_back(~(a * n + b));
        }
        candidates.push_back({area, start, ins + rem});
    }

    void enumerate_slots(int index, int first) {
        if (index == int(slots.size())) {
            build_candidate();
            return;
        }
        for (int slot = first; slot < int(hull.size()); ++slot) {
            slots[index] = slot;
            enumerate_slots(index + 1, slot);
        }
    }

    void choose_probes(const Plan& plan) {
        int limit = min(200, max(18, int(6500000 / candidates.size())));
        if (interior.size() == 1) limit = 1400;
        auto add = [&](Probe probe) {
            if (int(probes.size()) < limit && find(probes.begin(), probes.end(), probe) == probes.end()) probes.push_back(probe);
        };
        for (int v : interior) {
            int x = model.points[v].x;
            add(Probe(x, 1));
            if (x > model.columns.front().x) {
                add(Probe(997 * x - 1, 997));
            }
            if (x < model.columns.back().x) {
                add(Probe(997 * x + 1, 997));
            }
        }
        for (int v : interior) {
            int x = model.points[v].x;
            add(Probe(2 * x - 1, 2));
            add(Probe(2 * x + 1, 2));
        }
        if (plan.cost <= 12) {
            for (const Action& action : plan.actions) {
                for (int i = 0; i < action.count; ++i) add(action.probes[i]);
            }
        }
        int gaps = model.columns.size() - 1;
        if (interior.size() == 1) {
            for (int i = 0; i < gaps; ++i) add(Probe(model.columns[i].x + model.columns[i + 1].x, 2));
        }
        int sample = min(gaps, max(1, (limit - int(probes.size())) / 2));
        for (int i = 0; i < sample; ++i) {
            int gap = sample == 1 ? gaps / 2 : i * (gaps - 1) / (sample - 1);
            int a = model.columns[gap].x, b = model.columns[gap + 1].x;
            add(Probe(a + b, 2));
            add(Probe(a, 1));
        }
        if (gaps <= 35) {
            for (int i = 0; i < gaps; ++i) {
                int a = model.columns[i].x, b = model.columns[i + 1].x;
                add(Probe(2 * a + b, 3));
                add(Probe(a + 2 * b, 3));
                add(Probe(997 * a + 1, 997));
                add(Probe(997 * b - 1, 997));
            }
        }
    }

    void collect_signals() {
        vector<U64> edge_values(n * n);
        vector<int> seen(n * n, -1);
        for (int i = 0; i < int(probes.size()); ++i) {
            if (Clock::now() >= deadline) break;
            auto edge_value = [&](int code) {
                if (seen[code] != i) {
                    seen[code] = i;
                    edge_values[code] = arith.edge_residue(model.points[code / n], model.points[code % n], probes[i]);
                }
                return edge_values[code];
            };
            U64 value = 0;
            for (int h = 0; h < int(hull.size()); ++h) {
                auto [a, b] = hull_edge(h);
                value = add_mod(value, edge_value(a * n + b));
            }
            base.push_back(value);
            auto& row = signals.emplace_back(candidates.size());
            for (int id = 0; id < int(candidates.size()); ++id) {
                const auto& c = candidates[id];
                U64 key = 0;
                for (int j = c.start; j < c.start + c.count; ++j) {
                    int code = changes[j];
                    key = code >= 0 ? add_mod(key, edge_value(code)) : sub_mod(key, edge_value(~code));
                }
                row[id] = key;
            }
        }
    }

    template<class GetKey>
    tuple<int, int, int> quality(const vector<int>& ids, GetKey key_of) {
        ++stamp;
        int maximum = 0, total_mixed = 0, groups = 0;
        for (int id : ids) {
            U64 key = key_of(id);
            size_t index = (key ^ (key >> 27) ^ (key >> 43)) & (buckets.size() - 1);
            while (buckets[index].stamp == stamp && buckets[index].key != key) index = (index + 1) & (buckets.size() - 1);
            auto& bucket = buckets[index];
            int area = candidates[id].area;
            if (bucket.stamp != stamp) {
                bucket = {key, area, 1, stamp, false};
                ++groups;
            } else {
                ++bucket.count;
                if (bucket.mixed) {
                    ++total_mixed;
                } else if (bucket.area != area) {
                    bucket.mixed = true;
                    total_mixed += bucket.count;
                }
                if (bucket.mixed) maximum = max(maximum, bucket.count);
            }
        }
        return {maximum, total_mixed, -groups};
    }

    U64 signal(int id, Probe probe) const {
        const Candidate& candidate = candidates[id];
        U64 value = 0;
        for (int j = candidate.start; j < candidate.start + candidate.count; ++j) {
            int change = changes[j];
            int code = change >= 0 ? change : ~change;
            U64 piece = arith.edge_residue(model.points[code / n], model.points[code % n], probe);
            value = change >= 0 ? add_mod(value, piece) : sub_mod(value, piece);
        }
        return value;
    }

    U64 hull_signal(Probe probe) const {
        U64 value = 0;
        for (int i = 0; i < int(hull.size()); ++i) {
            auto [a, b] = hull_edge(i);
            value = add_mod(value, arith.edge_residue(model.points[a], model.points[b], probe));
        }
        return value;
    }

    vector<Probe> local_probes(int first, int second) const {
        vector<int> xs;
        for (int id : {first, second}) {
            const Candidate& candidate = candidates[id];
            for (int j = candidate.start; j < candidate.start + candidate.count; ++j) {
                int code = changes[j];
                if (code < 0) code = ~code;
                xs.push_back(model.points[code / n].x);
                xs.push_back(model.points[code % n].x);
            }
        }
        sort(xs.begin(), xs.end());
        xs.erase(unique(xs.begin(), xs.end()), xs.end());
        vector<Probe> answer;
        for (int i = 1; i < int(xs.size()); ++i) {
            Probe probe(xs[i - 1] + xs[i], 2);
            if (signal(first, probe) != signal(second, probe)) answer.push_back(probe);
            Probe other(xs[i - 1] * 2 + xs[i], 3);
            if (signal(first, other) != signal(second, other)) answer.push_back(other);
        }
        return answer;
    }

    int finish(vector<int> ids) {
        while (true) {
            int first = ids.front();
            auto other = find_if(ids.begin(), ids.end(), [&](int id) { return candidates[id].area != candidates[first].area; });
            if (other == ids.end()) return candidates[first].area;
            auto options = local_probes(first, *other);
            Probe best = options.front();
            int ambiguity = ids.size();
            vector<pair<U64, int>> answers;
            for (Probe probe : options) {
                answers.clear();
                for (int id : ids) answers.emplace_back(signal(id, probe), candidates[id].area);
                sort(answers.begin(), answers.end());
                int mixed = 0;
                for (int i = 0, j = 0; i < int(answers.size()); i = j) {
                    for (j = i + 1; j < int(answers.size()) && answers[j].first == answers[i].first; ++j) {}
                    if (answers[i].second != answers[j - 1].second) mixed = max(mixed, j - i);
                }
                if (mixed < ambiguity) {
                    ambiguity = mixed;
                    best = probe;
                }
                if (mixed == 0) break;
            }
            U64 wanted = sub_mod(oracle.ask(best).residue(2 * best.q), hull_signal(best));
            erase_if(ids, [&](int id) { return signal(id, best) != wanted; });
        }
    }

    template<class GetKey>
    int split_tree(const vector<int>& ids, int depth, Tree node, GetKey key_of) {
        int start = tree.size();
        vector<pair<U64, int>> sorted;
        sorted.reserve(ids.size());
        for (int id : ids) sorted.emplace_back(key_of(id), id);
        sort(sorted.begin(), sorted.end());
        vector<int> group;
        for (size_t i = 0; i < sorted.size();) {
            U64 key = sorted[i].first;
            group.clear();
            do {
                group.push_back(sorted[i++].second);
            } while (i < sorted.size() && sorted[i].first == key);
            int child = make_tree(group, depth - 1);
            if (child == -1) {
                tree.resize(start);
                return -1;
            }
            node.branches.emplace_back(key, child);
        }
        tree.push_back(move(node));
        return int(tree.size()) - 1;
    }

    int make_tree(const vector<int>& ids, int depth) {
        int area = candidates[ids.front()].area;
        auto different = [&](int id) { return candidates[id].area != area; };
        auto other = find_if(ids.begin(), ids.end(), different);
        if (other == ids.end()) {
            tree.push_back({Tree::AreaLeaf, area, {}, {}, {}});
            return int(tree.size()) - 1;
        }
        if (depth == 0 || Clock::now() >= deadline || tree_work >= tree_budget) return -1;
        // A query separating two different candidate areas removes at least one candidate.
        if (ids.size() <= size_t(depth + 1)) {
            tree.push_back({Tree::RefineLeaf, 0, {}, ids, {}});
            return int(tree.size()) - 1;
        }
        vector<pair<tuple<int, int, int>, int>> ranked;
        for (int probe = 0; probe < int(signals.size()); ++probe) {
            auto score = quality(ids, [&](int id) { return signals[probe][id]; });
            tree_work += ids.size();
            if (get<0>(score) != int(ids.size())) ranked.emplace_back(score, probe);
            if (get<0>(score) == 0 || tree_work >= tree_budget) break;
        }
        sort(ranked.begin(), ranked.end());
        for (int i = 0; i < min(int(ranked.size()), ids.size() <= 24 ? 8 : 2); ++i) {
            auto [score, probe] = ranked[i];
            if (depth == 1 && get<0>(score) != 0) break;
            int child = split_tree(ids, depth, {probe, 0, {}, {}, {}}, [&](int id) { return signals[probe][id]; });
            if (child != -1) return child;
            if (Clock::now() >= deadline || tree_work >= tree_budget) return -1;
        }
        if (ids.size() > 18'000) return -1;
        if (depth == 1 && !ranked.empty() && get<0>(ranked.front().first) > 40 && ids.size() > 100) return -1;
        vector<Probe> locations = local_probes(ids.front(), *other);
        for (int j = 1; j <= 6; ++j) {
            int id = ids[j * (ids.size() - 1) / 6];
            if (!different(id)) continue;
            for (Probe probe : local_probes(ids.front(), id)) {
                if (locations.size() >= 30) break;
                if (find(locations.begin(), locations.end(), probe) == locations.end()) locations.push_back(probe);
            }
        }
        vector<pair<tuple<int, int, int>, Probe>> local_ranked;
        for (Probe probe : locations) {
            if (find(probes.begin(), probes.begin() + signals.size(), probe) != probes.begin() + signals.size()) continue;
            auto score = quality(ids, [&](int id) { return signal(id, probe); });
            tree_work += int(ids.size()) * (2 + interior.size());
            if (get<0>(score) != int(ids.size())) local_ranked.emplace_back(score, probe);
            if (get<0>(score) == 0 || tree_work >= tree_budget || Clock::now() >= deadline) break;
        }
        sort(local_ranked.begin(), local_ranked.end());
        for (int i = 0; i < min(2, int(local_ranked.size())); ++i) {
            auto [score, probe] = local_ranked[i];
            if (depth == 1 && get<0>(score) != 0) break;
            int child = split_tree(ids, depth, {Tree::LocalProbe, 0, {}, {}, probe}, [&](int id) { return signal(id, probe); });
            if (child != -1) return child;
            if (Clock::now() >= deadline || tree_work >= tree_budget) break;
        }
        return -1;
    }

public:
    EnumeratedSolver(const Model& geometry, const Arithmetic& arithmetic, Oracle& judge, Clock::time_point end)
        : model(geometry), arith(arithmetic), oracle(judge), deadline(end), n(model.points.size()), hull(model.hull) {}

    optional<int> solve(const Plan& plan) {
        vector<bool> is_boundary(n);
        for (int i = 0; i < int(hull.size()); ++i) {
            auto [a, b] = hull_edge(i);
            hull_area += determinant(model.points[a], model.points[b]);
            is_boundary[a] = true;
        }
        for (int i = 0; i < n; ++i) if (!is_boundary[i]) interior.push_back(i);
        if (interior.empty()) return hull_area;
        int64_t count = 1;
        int limit = interior.size() == 2 ? 1'000'000 : interior.size() == 3 ? 420'000 : 160'000;
        for (int i = hull.size(); i < n; ++i) {
            count *= i;
            if (count > limit) return nullopt;
        }
        if (interior.size() > 11 || hull_area <= 0) return nullopt;
        candidates.reserve(count);
        changes.reserve(count * min(3 * int(interior.size()), 12));
        slots.resize(interior.size());
        do enumerate_slots(0, 0); while (next_permutation(interior.begin(), interior.end()));
        if (candidates.empty()) return nullopt;
        int area = candidates.front().area;
        if (all_of(candidates.begin(), candidates.end(), [&](const Candidate& c) { return c.area == area; })) return area;
        choose_probes(plan);
        collect_signals();
        if (signals.empty() || Clock::now() >= deadline) return nullopt;
        size_t table_size = 1;
        while (table_size < 2 * candidates.size()) table_size *= 2;
        buckets.resize(table_size);
        vector<int> ids(candidates.size());
        iota(ids.begin(), ids.end(), 0);
        int root = -1;
        int previous_depth = 0;
        for (int target : {1, 2, 3, 6, 15, 42}) {
            if (ids.size() > 200'000 && target < 6 && target < plan.cost) continue;
            int depth = min(target, plan.cost);
            if (depth == previous_depth) break;
            previous_depth = depth;
            root = make_tree(ids, depth);
            if (root != -1 || Clock::now() >= deadline || tree_work >= tree_budget) break;
        }
        if (root == -1) return nullopt;
        while (tree[root].probe >= 0 || tree[root].probe == Tree::LocalProbe) {
            const Tree& node = tree[root];
            bool local = node.probe == Tree::LocalProbe;
            Probe probe = local ? node.location : probes[node.probe];
            U64 answer = oracle.ask(probe).residue(2 * probe.q);
            U64 key = sub_mod(answer, local ? hull_signal(probe) : base[node.probe]);
            auto child = lower_bound(node.branches.begin(), node.branches.end(), pair(key, -1));
            if (child == node.branches.end() || child->first != key) return nullopt;
            root = child->second;
        }
        if (tree[root].probe == Tree::RefineLeaf) return finish(tree[root].unresolved);
        return tree[root].area;
    }
};

class Sweep {
    const Model& model;
    Integration& integration;
    Clock::time_point deadline;
    int& work;
    int n, frontier = 0;
    vector<int> next, previous;
    vector<Edge> fixed;

    struct Endpoints {
        int source = -1;
        bool out = false, in = false;
        vector<int> destinations;
    };

    struct Choice {
        U64 key;
        int out, in;
    };

    void remember(Edge edge) {
        auto [a, b] = edge;
        next[a] = b;
        previous[b] = a;
        fixed.push_back(edge);
    }

    U64 width(int x, bool right) const {
        U64 answer = 0;
        for (auto [a, b] : fixed) {
            Point A = model.points[a], B = model.points[b];
            if (A.x == B.x) continue;
            bool positive = A.x > B.x;
            if (positive) swap(A, B);
            if (right ? (x < A.x || x >= B.x) : (x <= A.x || x > B.x)) continue;
            int span = B.x - A.x;
            int height = A.y * (B.x - x) + B.y * (x - A.x);
            U64 value = mul_mod(integration.arith.inverse[span], height);
            answer = positive ? add_mod(answer, value) : sub_mod(answer, value);
        }
        return answer;
    }

    void publish(int column, bool right) {
        const auto& col = model.columns[column];
        int index = right ? col.right : col.left;
        if (index != -1 && !integration.data[model.nodes[index].id].width) {
            integration.set_width(model.nodes[index], width(col.x, right));
        }
    }

    bool available_edge(int a, int b) const {
        if (!model.possible_edge(a, b)) return false;
        for (Edge e : fixed) {
            if (incompatible(model.points, {a, b}, e)) return false;
        }
        return true;
    }

    vector<int> destinations(int source, int end, bool out, bool in) const {
        Point origin = model.points[source];
        vector<array<int, 4>> rays;
        for (int v = 0; v < n; ++v) {
            int dx = model.points[v].x - origin.x;
            if (dx <= 0) continue;
            int dy = model.points[v].y - origin.y;
            int divisor = gcd(dx, abs(dy));
            rays.push_back({dx / divisor, dy / divisor, divisor, v});
        }
        sort(rays.begin(), rays.end());
        vector<int> answer;
        for (int i = 0; i < int(rays.size()); ++i) {
            if (i && rays[i][0] == rays[i - 1][0] && rays[i][1] == rays[i - 1][1]) continue;
            int v = rays[i][3];
            if (model.col_of[v] <= end || !((out && previous[v] == -1) || (in && next[v] == -1))) continue;
            if (available_edge(source, v)) answer.push_back(v);
        }
        return answer;
    }

    bool internal_edge(int a, int b) const {
        Point A = model.points[a], B = model.points[b];
        for (int v = 0; v < n; ++v) {
            if (v == a || v == b) continue;
            Point P = model.points[v];
            if (overlap(A.x, B.x, P.x, P.x) && overlap(A.y, B.y, P.y, P.y) && cross(A, B, P) == 0) return false;
        }
        return available_edge(a, b);
    }

    bool alternating(int slab, const vector<Edge>& added) const {
        int x2 = model.columns[slab].x + model.columns[slab + 1].x;
        struct Height { int numerator, span; bool positive; };
        vector<Height> heights;
        auto add = [&](Edge e) {
            auto [a, b] = e;
            Point A = model.points[a], B = model.points[b];
            bool positive = A.x > B.x;
            if (positive) swap(A, B);
            if (x2 <= 2 * A.x || x2 >= 2 * B.x) return;
            heights.push_back({A.y * (2 * B.x - x2) + B.y * (x2 - 2 * A.x), B.x - A.x, positive});
        };
        for (Edge e : fixed) add(e);
        for (Edge e : added) add(e);
        if (heights.size() % 2) return false;
        sort(heights.begin(), heights.end(), [](const Height& a, const Height& b) {
            return int64_t(a.numerator) * b.span < int64_t(b.numerator) * a.span;
        });
        for (int i = 0; i < int(heights.size()); ++i) {
            if (heights[i].positive != (i % 2 == 1)) return false;
        }
        return true;
    }

    bool valid(const vector<Edge>& added, int end) {
        for (int i = 0; i < int(added.size()); ++i) {
            for (int j = 0; j < i; ++j) {
                if (incompatible(model.points, added[i], added[j])) return false;
            }
        }
        int applied = 0;
        for (auto [a, b] : added) {
            if (next[a] != -1 || previous[b] != -1) break;
            next[a] = b;
            previous[b] = a;
            ++applied;
        }
        bool ok = applied == int(added.size());
        if (ok) {
            for (auto [a, b] : added) {
                int length = 1;
                for (; b != -1 && b != a; b = next[b]) ++length;
                if (b == a && length != n) ok = false;
            }
        }
        for (int i = 0; i < applied; ++i) {
            auto [a, b] = added[i];
            next[a] = previous[b] = -1;
        }
        if (!ok) return false;
        for (int slab = frontier; slab <= end; ++slab) {
            if (!alternating(slab, added)) return false;
        }
        return true;
    }

    bool can_be_vertical(int a, int b) const {
        auto can_direct = [&](int first, int second) {
            if (next[first] != -1 || previous[second] != -1) return false;
            int length = 1;
            for (int v = second; v != -1; v = next[v], ++length) {
                if (v == first) return length == n;
            }
            return true;
        };
        return (can_direct(a, b) || can_direct(b, a)) && available_edge(a, b);
    }

    void publish_from_vertical_edges(int column) {
        const auto& col = model.columns[column];
        if (col.left == col.right || col.right == -1 || integration.data[model.nodes[col.right].id].width) return;
        // Reconstructed vertical edges may already give a known change in width.
        U64 right = col.left == -1 ? 0 : *integration.data[model.nodes[col.left].id].width;
        for (int i = 1; i < int(col.vertices.size()); ++i) {
            int bottom = col.vertices[i - 1], top = col.vertices[i];
            int span = model.points[top].y - model.points[bottom].y;
            if (next[bottom] == top) right = sub_mod(right, span);
            else if (next[top] == bottom) right = add_mod(right, span);
            else if (can_be_vertical(bottom, top)) return;
        }
        integration.set_width(model.nodes[col.right], right);
    }

    void advance_known() {
        while (frontier < int(model.columns.size()) && integration.remaining) {
            publish(frontier, false);
            publish_from_vertical_edges(frontier);
            if (!integration.remaining) return;
            int source = -1;
            bool multiple = false;
            for (int v : model.columns[frontier].vertices) {
                if (next[v] != -1 && previous[v] != -1) continue;
                if (source != -1) multiple = true;
                source = v;
            }
            if (multiple) return;
            if (source != -1) {
                bool out = next[source] == -1, in = previous[source] == -1;
                if ((out && in) || Clock::now() >= deadline) return;
                vector<int> options = destinations(source, frontier, out, in);
                if (options.size() != 1) return;
                if (out) remember({source, options[0]});
                else remember({options[0], source});
            }
            publish(frontier, true);
            ++frontier;
        }
    }

    vector<Choice> choices(const Endpoints& ends, Probe probe, bool from_left) const {
        if (!ends.out && !ends.in) return {{0, -1, -1}};
        vector<pair<int, U64>> outs, ins;
        if (!ends.out) outs.emplace_back(-1, 0);
        if (!ends.in) ins.emplace_back(-1, 0);
        Point source = model.points[ends.source];
        for (int target : ends.destinations) {
            U64 key = integration.arith.edge_residue(model.points[target], source, probe, from_left);
            if (ends.out && previous[target] == -1) outs.emplace_back(target, sub_mod(0, key));
            if (ends.in && next[target] == -1) ins.emplace_back(target, key);
        }
        vector<Choice> result;
        result.reserve(outs.size() * ins.size());
        for (auto [a, first] : outs) {
            for (auto [b, second] : ins) {
                if (a != b) result.push_back({add_mod(first, second), a, b});
            }
        }
        return result;
    }

    void recover_first_vertical(U64 observed) {
        const auto& vertices = model.columns.front().vertices;
        if (observed > 1000 || vertices.size() < 2) return;
        int target = observed;
        int gaps = int(vertices.size()) - 1;
        vector<int> length(gaps);
        for (int i = 0; i < gaps; ++i) {
            length[i] = model.points[vertices[i + 1]].y - model.points[vertices[i]].y;
        }
        // Along the supporting line each boundary interval is a vertical edge.
        // Test which edges are required by every subset with the observed length.
        vector<vector<char>> prefix(gaps + 1, vector<char>(target + 1));
        auto suffix = prefix;
        prefix.front()[0] = suffix.back()[0] = true;
        for (int i = 0; i < gaps; ++i) {
            for (int sum = 0; sum <= target; ++sum) {
                prefix[i + 1][sum] = prefix[i][sum] || (sum >= length[i] && prefix[i][sum - length[i]]);
                int j = gaps - 1 - i;
                suffix[j][sum] = suffix[j + 1][sum] || (sum >= length[j] && suffix[j + 1][sum - length[j]]);
            }
        }
        if (!prefix.back()[target]) return;
        for (int i = 0; i < gaps; ++i) {
            bool can_omit = false;
            for (int sum = 0; sum <= target && !can_omit; ++sum) {
                can_omit = prefix[i][sum] && suffix[i + 1][target - sum];
            }
            if (!can_omit) remember({vertices[i + 1], vertices[i]});
        }
    }

    bool try_action(const Action& action) {
        if (action.count != 1) return false;
        if (frontier == 0 && action.kind == Action::Direct && model.nodes[action.first].col == 0) {
            recover_first_vertical(integration.execute(model, action).width);
            return true;
        }
        bool from_left = action.kind == Action::Direct;
        int end = from_left ? model.nodes[action.first].col - 1 : action.slab;
        if (end < frontier || end + 1 >= int(model.columns.size())) return false;
        array<Endpoints, 2> ends;
        int sources = 0;
        for (int i = frontier; i <= end; ++i) {
            for (int v : model.columns[i].vertices) {
                bool out = next[v] == -1, in = previous[v] == -1;
                if (!out && !in) continue;
                if (sources == 2) return false;
                ends[sources++] = {v, out, in, {}};
            }
        }
        if (!sources) return false;
        int estimate = 0;
        for (auto& e : ends) {
            if (e.source == -1) continue;
            e.destinations = destinations(e.source, end, e.out, e.in);
            int count = e.destinations.size();
            estimate += (e.out && e.in) ? count * count + 2 * count : 3 * count;
        }
        if (work + estimate > 12'000'000 || Clock::now() >= deadline) return false;
        work += estimate;
        bool internal = sources == 2 && internal_edge(ends[0].source, ends[1].source);
        Probe probe = action.probes[0];
        const Reply& reply = integration.execute(model, action);
        U64 wanted = reply.residue(2 * probe.q);
        for (auto [u, v] : fixed) {
            wanted = sub_mod(wanted, integration.arith.edge_residue(model.points[u], model.points[v], probe, from_left));
        }
        // Keep a reconstruction only if the necessary conditions admit one choice.
        // A modular collision can create another choice and force the exact fallback.
        optional<vector<Edge>> solution;
        int checked = 0;
        for (int variant = 0; variant < (internal ? 3 : 1); ++variant) {
            auto parts = ends;
            vector<Edge> new_edges;
            new_edges.reserve(5);
            if (variant) {
                int a = variant - 1, b = 1 - a;
                if (!parts[a].out || !parts[b].in) continue;
                parts[a].out = parts[b].in = false;
                new_edges.emplace_back(parts[a].source, parts[b].source);
            }
            auto first = choices(parts[0], probe, from_left);
            auto second = choices(parts[1], probe, from_left);
            int small = 0;
            if (first.size() > second.size()) {
                first.swap(second);
                small = 1;
            }
            sort(first.begin(), first.end(), [](const Choice& a, const Choice& b) { return a.key < b.key; });
            for (const Choice& b : second) {
                U64 target = sub_mod(wanted, b.key);
                auto it = lower_bound(first.begin(), first.end(), target, [](const Choice& a, U64 key) { return a.key < key; });
                for (; it != first.end() && it->key == target; ++it) {
                    if (++checked > 300 || Clock::now() >= deadline) return false;
                    new_edges.resize(variant != 0);
                    auto add = [&](const Choice& choice, int source) {
                        if (choice.out != -1) new_edges.emplace_back(source, choice.out);
                        if (choice.in != -1) new_edges.emplace_back(choice.in, source);
                    };
                    add(*it, parts[small].source);
                    add(b, parts[1 - small].source);
                    if (!valid(new_edges, end)) continue;
                    if (solution) return false;
                    solution = new_edges;
                }
            }
        }
        if (!solution) return false;
        for (Edge e : *solution) remember(e);
        for (; frontier <= end; ++frontier) {
            publish(frontier, false);
            publish(frontier, true);
        }
        return true;
    }

public:
    Sweep(const Model& geometry, Integration& sum, Clock::time_point end, int& used_work)
        : model(geometry), integration(sum), deadline(end), work(used_work), n(model.points.size()), next(n, -1), previous(n, -1) {}

    void run() {
        while (integration.remaining && Clock::now() < deadline) {
            advance_known();
            if (!integration.remaining) return;
            Plan plan = integration.plan(model, frontier == 0 && model.columns.front().vertices.size() > 2);
            if (!try_action(plan.actions.front())) return;
        }
    }
};

void report(int area) {
    int divisor = gcd(area, 2);
    cout << "! " << area / divisor << ' ' << 2 / divisor << endl;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 3 || n > 1000) return 0;
    vector<Point> points(n);
    for (auto& p : points) if (!(cin >> p.x >> p.y)) return 0;
    auto start = Clock::now();
    Arithmetic arith;
    Oracle oracle;
    Model forward(points, false);
    if (forward.slabs.empty()) {
        report(0);
        return 0;
    }
    Integration integration(arith, oracle, forward);
    {
        EnumeratedSolver solver(forward, arith, oracle, start + chrono::milliseconds(3000));
        auto area = solver.solve(integration.plan(forward));
        if (area) {
            report(*area);
            return 0;
        }
    }
    Model backward(move(points), true);
    const Model* initial = &forward;
    const Model* opposite = &backward;
    if (forward.columns.front().vertices.size() > 2 && backward.columns.front().vertices.size() <= 2) swap(initial, opposite);
    int sweep_work = 0;
    auto sweep_end = start + chrono::milliseconds(7000);
    Sweep(*initial, integration, sweep_end, sweep_work).run();
    Sweep(*opposite, integration, sweep_end, sweep_work).run();
    while (integration.remaining) {
        Plan plan = integration.plan(forward);
        integration.execute(forward, plan.actions.front());
    }
    report(int(integration.total));
}

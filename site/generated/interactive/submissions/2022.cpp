#include <bits/stdc++.h>
using namespace std;

struct Pattern {
    vector<int> coefficient;
    vector<int> decode;
    vector<int> overlap;
    vector<vector<int>> minimum;
    int sum = 0;

    explicit Pattern(vector<int> c) : coefficient(move(c)) {
        sum = accumulate(coefficient.begin(), coefficient.end(), 0);
        int size = 1 << coefficient.size();
        vector<int> subset(size);
        decode.assign(sum + 1, -1);
        decode[0] = 0;
        for (int mask = 1; mask < size; ++mask) {
            subset[mask] = subset[mask & (mask - 1)] + coefficient[countr_zero(unsigned(mask))];
            decode[subset[mask]] = mask;
        }
        sort(subset.begin(), subset.end());
        overlap.assign(sum + 1, 0);
        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < i; ++j) ++overlap[subset[i] - subset[j]];
        }
        overlap[0] = size;
        minimum.emplace_back(sum + 1);
        iota(minimum[0].begin(), minimum[0].end(), 0);
        for (int step = 1; 2 * step <= sum + 1; step *= 2) {
            const auto& last = minimum.back();
            vector<int> next(sum + 2 - 2 * step);
            for (int i = 0; i < int(next.size()); ++i) {
                int a = last[i], b = last[i + step];
                next[i] = overlap[a] <= overlap[b] ? a : b;
            }
            minimum.push_back(move(next));
        }
    }

    pair<int, int> bestShift(int lo, int hi) const {
        if (lo > sum) return {lo, 0};
        int end = min(hi, sum);
        int lg = bit_width(unsigned(end - lo + 1)) - 1;
        int a = minimum[lg][lo], b = minimum[lg][end - (1 << lg) + 1];
        int best = overlap[a] <= overlap[b] ? a : b;
        if (overlap[best] && hi > sum) return {sum + 1, 0};
        return {best, overlap[best]};
    }

    int maskFor(int value) const {
        if (value < 0 || value > sum) return -1;
        return decode[value];
    }
};

struct Layout {
    vector<int> vertices, parentIndex, boundary, steps;
    vector<int> zero;
    vector<int> difference;
};

struct Recovery {
    int layout = -1, size = 0, pattern = -1;
    bool extra = false;
    int extraCoefficient = 0;
    int padding = 0, witness = -1;
    int cost = 0;
    double gain = -1;
    vector<int> weights;
};

struct RootOption {
    vector<vector<int>> groups;
    vector<pair<int, int>> weights;
    int cost = 0;
};

struct RootProbe {
    int first = -1, second = -1;
    double score = 1e100;
    vector<pair<int, int>> weights;
};

struct ResidueCode {
    vector<int> weights;
    int pivotWeight = 0, cost = 0;
};

struct CombinedProbe {
    int pivot = -1, scale = 1, cost = 0;
    vector<pair<int, int>> weights;
    vector<int> directions, signs;
    double gain = -1;
};

class Solver {
    int n;
    vector<vector<int>> graph, children;
    vector<int> parent, height, subtree, order, depth;
    vector<vector<uint16_t>> towards;
    vector<char> known, flipped, allowedRoot;
    vector<int> offset;
    int anchor = -1, remaining;
    bool star = false;
    int starCenterValue = 0;
    vector<int> suspectLeaves;
    vector<vector<int>> state;
    vector<Pattern> patterns;
    vector<vector<int>> patternBySize;
    vector<vector<int8_t>> assignment;
    vector<int> classOf, mark;
    vector<double> logCount;
    int timestamp = 0;
    map<tuple<int, int, int>, ResidueCode> residueCodes;

    int ask(const vector<pair<int, int>>& weights) const {
        int count = 0;
        for (auto [v, w] : weights) count += w;
        if (count < 1 || count > n) exit(0);
        cout << "? 1 " << count;
        for (auto [v, w] : weights) {
            for (int i = 0; i < w; ++i) cout << ' ' << v + 1;
        }
        cout << '\n' << flush;
        int response;
        if (!(cin >> response)) exit(0);
        if (response < -n * count || response > n * count) exit(0);
        return response;
    }

    void flip(int v) {
        cout << "? 2 " << v + 1 << '\n' << flush;
        flipped[v] ^= 1;
    }

    void answer(int root) const {
        cout << '!';
        for (int v = 0; v < n; ++v) {
            int value = assignment[root][v];
            cout << ' ' << (flipped[v] ? -value : value);
        }
        cout << '\n' << flush;
    }

    int farthest(int v, int p, int d, pair<int, int>& best) {
        if (d > best.first) best = {d, v};
        for (int u : graph[v]) if (u != p) farthest(u, v, d + 1, best);
        return best.second;
    }

    void orient(int v, int p) {
        parent[v] = p;
        order.push_back(v);
        subtree[v] = 1;
        for (int u : graph[v]) if (u != p) {
            children[v].push_back(u);
            depth[u] = depth[v] + 1;
            orient(u, v);
            subtree[v] += subtree[u];
            height[v] = max(height[v], height[u] + 1);
        }
    }

    void makePatterns() {
        // Each consecutive difference set below has distinct subset sums.
        // Appending binary low bits preserves this property.
        constexpr int cg[] = {0, 1, 2, 4, 7, 13, 24, 44, 84, 161, 309, 594};
        patternBySize.resize(12);
        for (int b = 0; b <= 11; ++b) {
            for (int high = 0; high <= b; ++high) {
                if (high > 0 && high < 4) continue;
                int low = b - high;
                vector<int> coefficient;
                for (int i = 0; i < high; ++i) coefficient.push_back((cg[high] - cg[i]) << low);
                for (int i = low - 1; i >= 0; --i) coefficient.push_back(1 << i);
                if (!coefficient.empty() && coefficient.front() > n) continue;
                patternBySize[b].push_back(patterns.size());
                patterns.emplace_back(move(coefficient));
            }
        }
    }

    // The selected nodes form an initial segment below the known vertices.
    vector<Layout> makeLayouts() const {
        vector<int> frontier;
        for (int v : order) if (!known[v] && known[parent[v]]) frontier.push_back(v);
        vector<Layout> layouts;
        for (int policy = 0; policy < 5; ++policy) {
            auto priority = [&](int v) {
                if (policy == 1) return pair(subtree[v], height[v]);
                if (policy == 2) return pair(height[v], -subtree[v]);
                return pair(height[v], subtree[v]);
            };
            auto higher = [&](int a, int b) {
                return priority(a) < priority(b);
            };
            priority_queue<int, vector<int>, decltype(higher)> available(higher, frontier);
            priority_queue<int, vector<int>, decltype(higher)> attached(higher);
            vector<int> next;
            int last = -1;
            while (int(next.size()) < min(12, remaining)) {
                int v;
                if (policy != 4 && last != -1 && !children[last].empty()) {
                    const auto& options = children[last];
                    auto best = max_element(options.begin(), options.end(), higher);
                    v = *best;
                    for (int u : options) if (u != v) attached.push(u);
                } else {
                    bool useAttached = !attached.empty() &&
                                       (available.empty() || policy != 3 ||
                                        !higher(attached.top(), available.top()));
                    if (useAttached) {
                        v = attached.top();
                        attached.pop();
                    } else {
                        v = available.top();
                        available.pop();
                    }
                }
                next.push_back(v);
                if (policy == 4) for (int u : children[v]) available.push(u);
                last = v;
            }
            bool fresh = true;
            for (const Layout& item : layouts) if (item.vertices == next) fresh = false;
            if (!fresh) continue;
            Layout layout;
            layout.vertices = move(next);
            for (int i = 0; i < int(layout.vertices.size()); ++i) {
                int v = layout.vertices[i];
                int p = parent[v], j = -1;
                if (!known[p]) {
                    j = find(layout.vertices.begin(), layout.vertices.begin() + i, p) -
                        layout.vertices.begin();
                }
                layout.parentIndex.push_back(j);
                layout.boundary.push_back(j < 0 ? p : layout.boundary[j]);
                layout.steps.push_back(j < 0 ? 1 : layout.steps[j] + 1);
                int base = layout.boundary.back();
                layout.zero.push_back(state[0][base] - layout.steps.back());
                layout.difference.push_back(state.size() == 2 ? (state[1][base] - state[0][base]) / 2 : 0);
            }
            layouts.push_back(move(layout));
        }
        return layouts;
    }

    vector<int> toWeights(const Layout& layout, const vector<int>& coefficient) const {
        // An edge difference contributes to every queried descendant.
        vector<int> weight = coefficient;
        for (int i = 0; i < int(coefficient.size()); ++i) {
            if (layout.parentIndex[i] >= 0) weight[layout.parentIndex[i]] -= coefficient[i];
        }
        return weight;
    }

    void initialize() {
        known[anchor] = true;
        state.emplace_back(n, 0);
        auto layouts = makeLayouts();
        int count = n - (n % 2 == 0);
        int size = min(n - 1, int(bit_width(unsigned(count))));
        const Layout& layout = layouts.front();
        vector<int> c(size);
        int otherRoots = 0;
        for (int i = 1; i < size; ++i) {
            c[i] = 1 << (size - 1 - i);
            if (layout.parentIndex[i] < 0) otherRoots += c[i];
        }
        c[0] = min(1 << (size - 1), count - otherRoots);
        vector<int> weights = toWeights(layout, c);
        vector<pair<int, int>> query;
        int cost = 0, sum = accumulate(c.begin(), c.end(), 0);
        for (int i = 0; i < size; ++i) if (weights[i] > 0) {
            query.emplace_back(layout.vertices[i], weights[i]);
            cost += weights[i];
        }
        if (cost < count) query.emplace_back(anchor, count - cost);
        int result = ask(query);
        state.clear();
        // The odd total lets us decode the edge bits modulo count.
        // Each of the two possible leading bits has an injective residue map.
        for (int mask = 0; mask < (1 << size); ++mask) {
            int subset = 0;
            for (int i = 0; i < size; ++i) if ((mask >> i) & 1) subset += c[i];
            int numerator = result + sum - 2 * subset;
            if (numerator % count != 0) continue;
            vector<int> f(n);
            f[anchor] = numerator / count;
            for (int i = 0; i < size; ++i) {
                int v = layout.vertices[i];
                f[v] = f[parent[v]] + (((mask >> i) & 1) ? 1 : -1);
            }
            state.push_back(move(f));
        }
        for (int i = 0; i < size; ++i) known[layout.vertices[i]] = true;
        remaining -= size;
    }

    struct Feasible {
        bool root = false;
        int forcedThrough = -1, base = -1;
    };

    Feasible feasible(const vector<int>& f, const vector<int>& boundary) const {
        Feasible out;
        for (int v : order) if (allowedRoot[v]) {
            if (known[v]) {
                if (abs(f[v]) == 1) return {true, -1, -1};
                continue;
            }
            int base = boundary[v], distance = depth[v] - depth[base];
            int magnitude = abs(f[base]);
            if ((magnitude + distance) % 2 == 0 || magnitude > distance + 1) continue;
            if (magnitude != distance + 1) return {true, -1, -1};
            if (!out.root) {
                out = {true, v, base};
            } else if (out.forcedThrough >= 0) {
                if (out.base != base) return {true, -1, -1};
                int a = out.forcedThrough, b = v;
                while (depth[a] > depth[b]) a = parent[a];
                while (depth[b] > depth[a]) b = parent[b];
                while (a != b) {
                    a = parent[a];
                    b = parent[b];
                }
                if (a == base) return {true, -1, -1};
                out.forcedThrough = a;
            }
        }
        return out;
    }

    void deduct() {
        vector<int> boundary(n);
        for (int v : order) boundary[v] = known[v] ? v : boundary[parent[v]];
        vector<Feasible> possibilities;
        for (int i = 0; i < int(state.size());) {
            Feasible next = feasible(state[i], boundary);
            if (next.root) {
                possibilities.push_back(next);
                ++i;
            } else {
                state.erase(state.begin() + i);
            }
        }
        if (state.empty()) exit(0);
        int v = possibilities[0].forcedThrough, base = possibilities[0].base;
        if (v < 0) return;
        for (int i = 1; i < int(possibilities.size()); ++i) {
            if (possibilities[i].base != base || possibilities[i].forcedThrough < 0) return;
            int b = possibilities[i].forcedThrough;
            while (depth[v] > depth[b]) v = parent[v];
            while (depth[b] > depth[v]) b = parent[b];
            while (v != b) {
                v = parent[v];
                b = parent[b];
            }
        }
        while (v != base) {
            for (auto& f : state) {
                int sign = f[base] > 0 ? 1 : -1;
                f[v] = f[base] - sign * (depth[v] - depth[base]);
            }
            known[v] = true;
            --remaining;
            v = parent[v];
        }
    }

    Recovery chooseRecovery(const vector<Layout>& layouts, int budget, bool exactOnly = false) const {
        Recovery best;
        bool pending = state.size() == 2;
        int positive = -1, negative = -1;
        if (pending) {
            // Along an edge the two scenarios' difference changes by at most two.
            for (int v : order) if (known[v]) {
                int diff = state[1][v] - state[0][v];
                if (diff == 2) positive = v;
                if (diff == -2) negative = v;
            }
        }
        for (int l = 0; l < int(layouts.size()); ++l) {
            const Layout& layout = layouts[l];
            for (int size = int(layout.vertices.size()); size >= 1; --size) {
                if (size + int(pending) + 1e-9 < best.gain) break;
                for (int extra = 0; extra <= !pending; ++extra) {
                    int dimension = size - extra;
                    if (dimension >= int(patternBySize.size())) continue;
                    for (int code : patternBySize[dimension]) {
                        const Pattern& pattern = patterns[code];
                        vector<int> weight(size);
                        int fixedCost = 0, shift = 0, low = 0;
                        for (int i = extra; i < size; ++i) {
                            weight[i] = pattern.coefficient[i - extra];
                            if (layout.parentIndex[i] < 0) fixedCost += weight[i];
                        }
                        for (int i = extra; i < size; ++i) {
                            int p = layout.parentIndex[i];
                            if (p >= 0) weight[p] -= pattern.coefficient[i - extra];
                        }
                        bool valid = fixedCost <= budget;
                        for (int i = extra; i < size; ++i) if (weight[i] < 0) valid = false;
                        if (!valid) continue;
                        int padding = 0, witness = -1, coefficient = 0, overlaps = 0;
                        if (extra) {
                            low = max(1, -weight[0]);
                            if (fixedCost + low > budget) continue;
                            tie(coefficient, overlaps) = pattern.bestShift(low, budget - fixedCost);
                            weight[0] += coefficient;
                            fixedCost += coefficient;
                        } else if (pending) {
                            for (int i = 0; i < size; ++i) shift += weight[i] * layout.difference[i];
                            int room = budget - fixedCost;
                            int left = negative >= 0 ? shift - room : shift;
                            int right = positive >= 0 ? shift + room : shift;
                            int target = shift;
                            overlaps = INT_MAX;
                            if (left <= 0) {
                                auto [magnitude, count] = pattern.bestShift(max(0, -right), -left);
                                target = -magnitude;
                                overlaps = count;
                            }
                            if (right >= 0) {
                                auto [magnitude, count] = pattern.bestShift(max(0, left), right);
                                if (count < overlaps ||
                                    (count == overlaps && abs(magnitude - shift) < abs(target - shift))) {
                                    target = magnitude;
                                    overlaps = count;
                                }
                            }
                            padding = abs(target - shift);
                            witness = target > shift ? positive : negative;
                        }
                        if (exactOnly && overlaps) continue;
                        double gain = size + int(pending) - double(overlaps) / (1 << dimension);
                        if (gain > best.gain + 1e-9 ||
                            (abs(gain - best.gain) < 1e-9 && size == best.size && l == best.layout &&
                             fixedCost + padding < best.cost)) {
                            best = {l, size, code, bool(extra), coefficient, padding, witness,
                                    fixedCost + padding, gain, move(weight)};
                        }
                    }
                }
            }
        }
        return best;
    }

    static int mod(int value, int divisor) {
        int remainder = value % divisor;
        return remainder >= 0 ? remainder : remainder + divisor;
    }

    const ResidueCode& residueCode(int negatives, int positives, int self) {
        auto key = make_tuple(negatives, positives, self);
        auto [it, fresh] = residueCodes.try_emplace(key);
        auto& best = it->second;
        if (!fresh) return best;
        vector<int> signs;
        if (self) signs.push_back(self);
        signs.insert(signs.end(), negatives, -1);
        signs.insert(signs.end(), positives, 1);
        int size = signs.size(), limit = (1 << size) - 1;
        best.cost = INT_MAX;
        // Assign a separate residue to each direction and minimize repetitions.
        for (int total = 0; total < size; ++total) {
            vector<int> cost(limit + 1, INT_MAX), chosen(limit + 1);
            cost[0] = 0;
            for (int mask = 0; mask < limit; ++mask) {
                if (cost[mask] == INT_MAX) continue;
                int i = popcount(unsigned(mask));
                for (int r = 0; r < size; ++r) if (!(mask & (1 << r))) {
                    int w = mod(total + signs[i] * r, size);
                    if (i == 0 && self && w != 0) continue;
                    int next = mask | (1 << r);
                    if (cost[mask] + w < cost[next]) {
                        cost[next] = cost[mask] + w;
                        chosen[next] = r;
                    }
                }
            }
            int padding = mod(total - cost[limit], size);
            if (cost[limit] + padding >= best.cost) continue;
            best = {vector<int>(size), padding, cost[limit] + padding};
            int mask = limit;
            for (int i = size - 1; i >= 0; --i) {
                int r = chosen[mask];
                best.weights[i] = mod(total + signs[i] * r, size);
                mask ^= 1 << r;
            }
        }
        return best;
    }

    vector<int> possibleRoots() const {
        const auto& f = state[0];
        vector<int> roots, boundary(n);
        for (int v : order) {
            boundary[v] = known[v] ? v : boundary[parent[v]];
            if (!allowedRoot[v]) continue;
            int base = boundary[v], distance = depth[v] - depth[base];
            if ((f[base] + distance) % 2 != 0 && abs(f[base]) <= distance + 1) roots.push_back(v);
        }
        return roots;
    }

    bool knownRootConflict(const vector<int>& roots) const {
        int first = -1;
        for (int r : roots) if (known[r]) {
            first = r;
            break;
        }
        if (first < 0) return false;
        const auto& f = state[0];
        vector<char> equivalent(n);
        vector<int> pending = {first};
        equivalent[first] = true;
        for (int i = 0; i < int(pending.size()); ++i) {
            int v = pending[i];
            for (int u : graph[v]) {
                if (equivalent[u] || !known[u]) continue;
                int need = (depth[u] - depth[first]) % 2 == 0 ? f[first] : 0;
                if (f[u] == need) {
                    equivalent[u] = true;
                    pending.push_back(u);
                }
            }
        }
        for (int r : roots) if (known[r] && !equivalent[r]) return true;
        return false;
    }

    CombinedProbe chooseCombined(const vector<Layout>& layouts, Recovery& plan) {
        CombinedProbe best;
        if (state.size() != 1 || n < 7) return best;
        const auto& f = state[0];
        vector<int> roots = possibleRoots();
        if (roots.size() < 2 || !knownRootConflict(roots)) return best;
        int total = roots.size();
        vector<int> counts(n);
        map<int, CombinedProbe> optionByBudget;
        for (int p : order) if (known[p]) {
            vector<int> dirs;
            for (int r : roots) {
                int dir = towards[p][r];
                if (++counts[dir] == 1) dirs.push_back(dir);
            }
            double average = 0;
            int largest = 0;
            bool valid = true;
            for (int dir : dirs) {
                int c = exchange(counts[dir], 0);
                if (!known[dir]) valid = false;
                average += c * log2(c);
                largest = max(largest, c);
            }
            int size = dirs.size();
            if (!valid || size < 2 || size > 7 || 4 * largest > 3 * total) continue;
            auto sign = [&](int dir) { return dir == p ? f[p] : f[p] - f[dir]; };
            sort(dirs.begin(), dirs.end(), [&](int a, int b) {
                if (a == p || b == p) return a == p && b != p;
                return sign(a) < sign(b);
            });
            int self = 0, negatives = 0, positives = 0;
            vector<int> signs;
            for (int dir : dirs) {
                int s = sign(dir);
                signs.push_back(s);
                if (dir == p) self = s;
                else if (s < 0) ++negatives;
                else ++positives;
            }
            const auto& code = residueCode(negatives, positives, self);
            int budget = (n - code.cost) / size;
            if (budget < 1) continue;
            double gain = log2(total) - 0.7 * log2(largest) - 0.3 * average / total;
            auto& option = optionByBudget[budget];
            if (gain <= option.gain) continue;
            option = {p, size, code.cost, {}, move(dirs), move(signs), gain};
            if (code.pivotWeight) option.weights.emplace_back(p, code.pivotWeight);
            for (int i = 0; i < size; ++i) {
                if (code.weights[i]) option.weights.emplace_back(option.directions[i], code.weights[i]);
            }
        }
        Recovery picked;
        for (auto& [budget, candidate] : optionByBudget) {
            auto combined = chooseRecovery(layouts, budget);
            if (combined.layout < 0) continue;
            double loss = plan.gain - combined.gain;
            if (loss > 2.5 || (plan.size == remaining && combined.size != remaining)) continue;
            candidate.gain -= 0.16 * loss;
            if (candidate.gain > best.gain) {
                best = move(candidate);
                picked = move(combined);
            }
        }
        if (best.pivot >= 0) plan = move(picked);
        return best;
    }

    bool affected(int pivot, int direction, int vertex) const {
        return direction == pivot || towards[pivot][vertex] != direction;
    }

    void tryStarShift() {
        if (!star || state.size() != 1 || abs(state[0][anchor]) != 2 ||
            any_of(flipped.begin(), flipped.end(), [](char f) { return f; })) return;
        int sign = state[0][anchor] / 2;
        vector<int> suspects;
        for (int v : graph[anchor]) {
            if (known[v] && state[0][v] == sign) suspects.push_back(v);
        }
        int count = suspects.size();
        if (count < 2 || count * (count + 1) / 2 + count + 1 > n) return;
        // With a zero center sum, every leaf's path sum is its value.
        // At most one of the already read leaves keeps its old sum.
        flip(anchor);
        starCenterValue = -sign;
        suspectLeaves = move(suspects);
        state[0][anchor] = 0;
        for (int v : graph[anchor]) if (known[v]) state[0][v] -= 2 * sign;
    }

    int addStarProbe(vector<pair<int, int>>& query) const {
        int base = 0;
        for (int i = 0; i < int(suspectLeaves.size()); ++i) {
            int v = suspectLeaves[i], w = i + 1;
            query.emplace_back(v, w);
            base += w * state[0][v];
        }
        return base;
    }

    int settleStarProbe(int value) {
        int scale = suspectLeaves.size() + 1;
        int index = mod(-starCenterValue * value, scale);
        if (index) {
            for (auto& f : state) f[suspectLeaves[index - 1]] -= 2 * starCenterValue;
        }
        suspectLeaves.clear();
        return (value + starCenterValue * index) / scale;
    }

    int ambiguityWitness() const {
        for (int v : order) if (state[0][v] != state[1][v]) return v;
        exit(0);
    }

    void answerStar() {
        if (state.size() == 2 || !suspectLeaves.empty()) {
            int witness = state.size() == 2 ? ambiguityWitness() : -1;
            vector<pair<int, int>> query;
            int scale = suspectLeaves.size() + 1;
            int base = addStarProbe(query);
            if (witness >= 0) {
                query.emplace_back(witness, scale);
                base += scale * state[0][witness];
            }
            int result = ask(query);
            if ((result - base) % 2 != 0) exit(0);
            int choice = settleStarProbe((result - base) / 2);
            if (witness >= 0 && choice) state[0] = move(state[1]);
        }
        cout << '!';
        for (int v = 0; v < n; ++v) cout << ' ' << (v == anchor ? starCenterValue : state[0][v]);
        cout << '\n' << flush;
    }

    bool preferExactOnStar() const {
        if (!star || state.size() != 2 || remaining < 12 ||
            state[0][anchor] != state[1][anchor]) return false;
        int power = bit_floor(unsigned(n));
        if (n - power > power / 16) return false;
        int low = 0, high = 0;
        for (int v : graph[anchor]) if (known[v] && state[0][v] == state[1][v]) {
            if (state[0][v] < state[0][anchor]) ++low;
            else ++high;
        }
        // A nearly full binary range leaves very few unambiguous replies.
        // Clear the old ambiguity if the observed leaf values are skewed.
        return low + high >= 6 && 5 * min(low, high) <= max(low, high);
    }

    void recoverValues() {
        initialize();
        for (;;) {
            if (!starCenterValue) {
                deduct();
                tryStarShift();
            }
            if (remaining == 0) return;
            auto layouts = makeLayouts();
            Recovery plan = chooseRecovery(layouts, n);
            if (plan.layout < 0) exit(0);
            if (preferExactOnStar()) {
                Recovery exact = chooseRecovery(layouts, n, true);
                if (exact.layout >= 0 && exact.gain >= plan.gain - 1.15) plan = move(exact);
            }
            CombinedProbe combined;
            int starScale = 1;
            if (!starCenterValue) {
                combined = chooseCombined(layouts, plan);
            } else if (!suspectLeaves.empty()) {
                int count = suspectLeaves.size();
                int budget = (n - count * (count + 1) / 2) / (count + 1);
                Recovery packed = chooseRecovery(layouts, budget);
                if (packed.layout >= 0 && packed.size == plan.size && packed.gain >= plan.gain - 1e-9) {
                    plan = move(packed);
                    starScale = count + 1;
                }
            }
            const auto& layout = layouts[plan.layout];
            vector<pair<int, int>> query;
            int base = 0, shift = 0;
            for (int i = 0; i < plan.size; ++i) {
                int w = plan.weights[i];
                if (w == 0) continue;
                int v = layout.vertices[i];
                query.emplace_back(v, w);
                base += w * (layout.zero[i] + offset[v]);
                shift += w * layout.difference[i];
            }
            if (plan.padding) {
                query.emplace_back(plan.witness, plan.padding);
                base += plan.padding * (state[0][plan.witness] + offset[plan.witness]);
                shift += plan.padding * (state[1][plan.witness] - state[0][plan.witness]) / 2;
            }
            if (combined.pivot >= 0) {
                // The unscaled terms reveal the pivot direction modulo scale.
                for (auto& [v, w] : query) w *= combined.scale;
                base *= combined.scale;
                for (auto [v, w] : combined.weights) {
                    query.emplace_back(v, w);
                    base += w * (state[0][v] + offset[v]);
                }
                flip(combined.pivot);
            }
            if (starScale > 1) {
                for (auto& [v, w] : query) w *= starScale;
                base = starScale * base + addStarProbe(query);
            }
            int result = ask(query);
            if ((result - base) % 2 != 0) exit(0);
            int value = (result - base) / 2;
            if (starScale > 1) value = settleStarProbe(value);
            if (combined.pivot >= 0) {
                int p = combined.pivot;
                int choice = -1;
                for (int i = 0; i < int(combined.directions.size()); ++i) {
                    int dir = combined.directions[i], count = 0;
                    for (auto [v, w] : combined.weights) if (affected(p, dir, v)) count += w;
                    int change = -combined.signs[i] * count;
                    if (mod(value - change, combined.scale) == 0) {
                        choice = i;
                        break;
                    }
                }
                if (choice < 0) exit(0);
                int dir = combined.directions[choice], sign = combined.signs[choice];
                for (int i = 0; i < int(query.size()); ++i) {
                    auto [v, w] = query[i];
                    if (affected(p, dir, v)) value += sign * w;
                }
                value /= combined.scale;
                for (int v = 0; v < n; ++v) {
                    if (towards[p][v] != dir) allowedRoot[v] = false;
                    if (affected(p, dir, v)) offset[v] -= 2 * sign;
                }
            }
            vector<vector<int>> nextState;
            for (int s = 0; s < int(state.size()); ++s) {
                for (int x = 0; x <= int(plan.extra); ++x) {
                    int mask = patterns[plan.pattern].maskFor(value - s * shift - x * plan.extraCoefficient);
                    if (mask < 0) continue;
                    if (plan.extra) mask = (mask << 1) | x;
                    vector<int> f = state[s];
                    for (int i = 0; i < plan.size; ++i) {
                        int v = layout.vertices[i];
                        f[v] = f[parent[v]] + (((mask >> i) & 1) ? 1 : -1);
                    }
                    nextState.push_back(move(f));
                }
            }
            state = move(nextState);
            for (int i = 0; i < plan.size; ++i) known[layout.vertices[i]] = true;
            remaining -= plan.size;
        }
    }

    void buildDirections(int at, int prev, int first, vector<uint16_t>& from) const {
        from[at] = first;
        for (int v : graph[at]) if (v != prev) buildDirections(v, at, first, from);
    }

    void makeAssignments() {
        assignment.resize(n * state.size());
        classOf.assign(assignment.size(), -1);
        map<vector<int8_t>, int> classes;
        for (int s = 0; s < int(state.size()); ++s) {
            const auto& f = state[s];
            for (int root = 0; root < n; ++root) if (allowedRoot[root] && abs(f[root]) == 1) {
                int id = s * n + root;
                auto& values = assignment[id];
                values.resize(n);
                for (int v = 0; v < n; ++v) {
                    values[v] = v == root ? f[v] : f[v] - f[towards[v][root]];
                }
                int next = classes.size();
                classOf[id] = classes.try_emplace(values, next).first->second;
            }
        }
        mark.resize(classes.size());
        logCount.resize(assignment.size() + 1);
        for (int i = 2; i < int(logCount.size()); ++i) logCount[i] = log2(i);
    }

    bool sameAnswer(const vector<int>& candidates) const {
        if (candidates.empty()) exit(0);
        int group = classOf[candidates.front()];
        for (int r : candidates) if (classOf[r] != group) return false;
        return true;
    }

    double scoreGroups(const vector<vector<int>>& groups, int count, int ops) {
        double worst = 0, average = 0, roots = 0;
        int maxSize = 0;
        for (const auto& group : groups) {
            if (group.empty()) continue;
            int distinct = 0;
            ++timestamp;
            for (int r : group) {
                int c = classOf[r];
                if (mark[c] != timestamp) {
                    mark[c] = timestamp;
                    ++distinct;
                }
            }
            int size = group.size();
            double cost = 1.94 * logCount[distinct] + 0.23 * (distinct > 1);
            worst = max(worst, cost);
            average += size * cost;
            roots += size * logCount[size];
            maxSize = max(maxSize, size);
        }
        if (maxSize == count) return 1e100;
        return ops + 0.7 * worst + 0.3 * average / count + 0.002 * roots / count;
    }

    vector<RootOption> codeOptions(int pivot, const vector<pair<int, vector<int>>>& branches,
                                   int budget) const {
        int d = branches.size();
        if (d == 0) return {RootOption()};
        int self = -1;
        for (int i = 0; i < d; ++i) if (branches[i].first == pivot) self = i;
        int maxGroups = min(d, 46);
        vector<int> indices;
        for (int i = 0; i < d; ++i) if (i != self) indices.push_back(i);
        stable_sort(indices.begin(), indices.end(), [&](int a, int b) {
            return branches[a].second.size() > branches[b].second.size();
        });
        vector<RootOption> options;
        int exactCost = d * (d - 1) / 2;
        if (exactCost <= budget) {
            RootOption exact;
            exact.cost = exactCost;
            int code = self < 0 ? 0 : 1;
            if (self >= 0) exact.groups.push_back(branches[self].second);
            for (int index : indices) {
                exact.groups.push_back(branches[index].second);
                if (code) exact.weights.emplace_back(branches[index].first, code);
                ++code;
            }
            options.push_back(move(exact));
        }
        for (int k = 1; k <= maxGroups && k < d; ++k) {
            if (k * (k - 1) / 2 > budget) break;
            vector<vector<int>> bins(k);
            vector<int> load(k);
            if (self >= 0) {
                bins[0].push_back(self);
                load[0] = branches[self].second.size();
            }
            for (int index : indices) {
                int b = min_element(load.begin(), load.end()) - load.begin();
                bins[b].push_back(index);
                load[b] += branches[index].second.size();
            }
            stable_sort(bins.begin() + (self >= 0), bins.end(), [&](const auto& a, const auto& b) {
                return a.size() > b.size();
            });
            RootOption next;
            for (int code = 0; code < k; ++code) {
                vector<int> group;
                for (int index : bins[code]) {
                    const auto& [dir, roots] = branches[index];
                    group.insert(group.end(), roots.begin(), roots.end());
                    if (code) {
                        next.weights.emplace_back(dir, code);
                        next.cost += code;
                    }
                }
                next.groups.push_back(move(group));
            }
            if (next.cost <= budget) options.push_back(move(next));
        }
        return options;
    }

    void probePivot(int pivot, const array<vector<pair<int, vector<int>>>, 2>& branches,
                    int count, int ops, RootProbe& best) {
        int d0 = branches[0].size(), d1 = branches[1].size();
        int exact = d0 * (d0 - 1) / 2 + d1 * (d1 - 1) / 2;
        if (exact < n) {
            vector<vector<int>> groups;
            for (const auto& side : branches) {
                for (const auto& [v, roots] : side) groups.push_back(roots);
            }
            double score = scoreGroups(groups, count, ops);
            if (score >= best.score - 1e-9) return;
            best = {pivot, -1, score, {{pivot, 1}}};
            for (const auto& side : branches) {
                bool self = false;
                for (const auto& [v, roots] : side) if (v == pivot) self = true;
                int code = self;
                for (const auto& [v, roots] : side) if (v != pivot) {
                    if (code) best.weights.emplace_back(v, code);
                    ++code;
                }
            }
            return;
        }
        auto negative = codeOptions(pivot, branches[0], n - 1);
        auto positive = codeOptions(pivot, branches[1], n - 1);
        for (const auto& a : negative) for (const auto& b : positive) {
            if (a.cost + b.cost >= n) continue;
            vector<vector<int>> groups = a.groups;
            groups.insert(groups.end(), b.groups.begin(), b.groups.end());
            double score = scoreGroups(groups, count, ops);
            if (score >= best.score - 1e-9) continue;
            best = {pivot, -1, score, {{pivot, 1}}};
            best.weights.insert(best.weights.end(), a.weights.begin(), a.weights.end());
            best.weights.insert(best.weights.end(), b.weights.begin(), b.weights.end());
        }
    }

    int actualSign(int root, int v) const {
        return flipped[v] ? -assignment[root][v] : assignment[root][v];
    }

    vector<int> restrictBySum(const vector<int>& candidates, const vector<pair<int, int>>& weights,
                              const vector<int>& before, const vector<int>& pivots,
                              int reply) const {
        int base = 0;
        for (auto [v, w] : weights) base += before[v] * w;
        vector<int> next;
        for (int r : candidates) {
            int predicted = base;
            for (int pivot : pivots) {
                int dir = towards[pivot][r % n];
                int changed = 0;
                for (auto [v, w] : weights) {
                    if (affected(pivot, dir, v)) changed += w;
                }
                predicted += 2 * actualSign(r, pivot) * changed;
            }
            if (predicted == reply) next.push_back(r);
        }
        return next;
    }

    void applyChange(vector<int>& f, int pivot, int representative) const {
        int dir = towards[pivot][representative % n];
        int delta = 2 * actualSign(representative, pivot);
        for (int v = 0; v < n; ++v) {
            if (affected(pivot, dir, v)) f[v] += delta;
        }
    }

    struct Separator {
        int pivot, a, b;
        double rank;
        uint64_t hash;
    };

    RootProbe chooseRootProbe(const vector<int>& candidates, int active) {
        RootProbe best;
        vector<Separator> separators;
        int count = candidates.size();
        vector<int> slot(n, -1);
        for (int p = 0; p < n; ++p) {
            if (active >= 0 && p != active) continue;
            array<vector<pair<int, vector<int>>>, 2> branches;
            vector<int> used;
            for (int r : candidates) {
                int dir = towards[p][r % n];
                int side = actualSign(r, p) > 0;
                if (slot[dir] < 0) {
                    slot[dir] = branches[side].size();
                    branches[side].emplace_back(dir, vector<int>());
                    used.push_back(dir);
                }
                branches[side][slot[dir]].second.push_back(r);
            }
            for (int v : used) slot[v] = -1;
            probePivot(p, branches, count, active < 0 ? 2 : 1, best);
            if (active >= 0 || count < 3 || used.size() != 2 || used[0] == p || used[1] == p) continue;
            vector<int>* a = nullptr;
            for (auto& side : branches) {
                for (auto& [v, roots] : side) if (v == used[0]) a = &roots;
            }
            double fraction = double(min(int(a->size()), count - int(a->size()))) / count;
            if (fraction < 0.12) continue;
            uint64_t hash = 0;
            for (int r : *a) {
                uint64_t x = uint64_t(r + 1) * 0x9e3779b97f4a7c15ULL;
                hash ^= x ^ (x >> 23) ^ (x << 19);
            }
            // All eligible pivots that cut off the same side need only one representative.
            if (a->front() == candidates.front()) {
                for (int r : candidates) {
                    uint64_t x = uint64_t(r + 1) * 0x9e3779b97f4a7c15ULL;
                    hash ^= x ^ (x >> 23) ^ (x << 19);
                }
            }
            separators.push_back({p, used[0], used[1], abs(fraction - 1.0 / 3), hash});
        }
        if (active >= 0 || best.score < 3 || n < 6) return best;
        stable_sort(separators.begin(), separators.end(), [](const Separator& a, const Separator& b) {
            return a.rank < b.rank;
        });
        vector<Separator> shortlist;
        unordered_set<uint64_t> seen;
        for (const auto& item : separators) {
            if (seen.insert(item.hash).second) shortlist.push_back(item);
            if (shortlist.size() == 40) break;
        }
        vector<vector<int>> groups(25);
        // With two active directions at each pivot, the two endpoint changes
        // identify both directions. Multiplicities 1 and 5 pack the changes.
        for (int i = 0; i < int(shortlist.size()); ++i) {
            const auto& a = shortlist[i];
            for (int j = 0; j < i; ++j) {
                const auto& b = shortlist[j];
                int ab = towards[a.pivot][b.pivot], ba = towards[b.pivot][a.pivot];
                if ((ab != a.a && ab != a.b) || (ba != b.a && ba != b.b)) continue;
                for (auto& group : groups) group.clear();
                for (int r : candidates) {
                    int x = actualSign(r, a.pivot), y = actualSign(r, b.pivot);
                    int first = x + (towards[b.pivot][r % n] != ba ? y : 0);
                    int second = y + (towards[a.pivot][r % n] != ab ? x : 0);
                    groups[(first + 2) * 5 + second + 2].push_back(r);
                }
                double score = scoreGroups(groups, count, 3);
                if (score < best.score - 1e-9) {
                    best = {a.pivot, b.pivot, score, {{a.pivot, 1}, {b.pivot, 5}}};
                }
            }
        }
        return best;
    }

    void locateRoot() {
        makeAssignments();
        vector<int> candidates;
        for (int r = 0; r < int(classOf.size()); ++r) if (classOf[r] >= 0) candidates.push_back(r);
        int active = -1;
        if (!sameAnswer(candidates) && state.size() == 2) {
            int witness = ambiguityWitness();
            int reply = ask({{witness, 1}}) - offset[witness];
            erase_if(candidates, [&](int r) { return state[r / n][witness] != reply; });
        }
        if (sameAnswer(candidates)) {
            answer(candidates.front());
            return;
        }
        vector<int> f = state[candidates.front() / n];
        for (int v = 0; v < n; ++v) f[v] += offset[v];
        while (!sameAnswer(candidates)) {
            RootProbe probe = chooseRootProbe(candidates, active);
            if (probe.first < 0) exit(0);
            if (active < 0) flip(probe.first);
            if (probe.second >= 0) flip(probe.second);
            int reply = ask(probe.weights);
            vector<int> pivots = {probe.first};
            if (probe.second >= 0) pivots.push_back(probe.second);
            candidates = restrictBySum(candidates, probe.weights, f, pivots, reply);
            if (sameAnswer(candidates)) break;
            if (probe.second >= 0) {
                applyChange(f, probe.first, candidates.front());
                applyChange(f, probe.second, candidates.front());
            } else {
                int dir = towards[probe.first][candidates.front() % n];
                bool oneBranch = true;
                for (int r : candidates) if (towards[probe.first][r % n] != dir) oneBranch = false;
                if (oneBranch) {
                    applyChange(f, probe.first, candidates.front());
                    active = -1;
                } else {
                    active = probe.first;
                }
            }
        }
        answer(candidates.front());
    }

    // For tiny trees the whole set of hidden states fits in a short list.
    // Search all multisets and group any proposed flips with the next sum.
    bool solveSmall() {
        const int masks = 1 << n, total = n * masks;
        vector<vector<int>> allSums(total, vector<int>(n));
        vector<int> sumClass(total);
        map<vector<int>, int> classes;
        for (int root = 0; root < n; ++root) for (int m = 0; m < masks; ++m) {
            int id = root * masks + m;
            auto fillSums = [&](auto&& self, int v, int p, int sum) -> void {
                sum += (m >> v) & 1 ? 1 : -1;
                allSums[id][v] = sum;
                for (int u : graph[v]) if (u != p) self(self, u, v, sum);
            };
            fillSums(fillSums, root, -1, 0);
            sumClass[id] = classes.try_emplace(allSums[id], classes.size()).first->second;
        }
        vector<vector<pair<int, int>>> queries;
        vector<vector<int8_t>> replies;
        vector<pair<int, int>> weights;
        auto enumerate = [&](auto&& self, int v, int left, int divisor) -> void {
            if (v == n) {
                if (divisor != 1) return;
                queries.push_back(weights);
                replies.emplace_back(total);
                for (int id = 0; id < total; ++id) {
                    for (auto [u, w] : weights) replies.back()[id] += w * allSums[id][u];
                }
                return;
            }
            self(self, v + 1, left, divisor);
            for (int w = 1; w <= left; ++w) {
                weights.emplace_back(v, w);
                self(self, v + 1, left - w, gcd(divisor, w));
                weights.pop_back();
            }
        };
        enumerate(enumerate, 0, n, 0);

        auto bound = [&](const vector<int>& candidates) {
            if (candidates.empty()) return 0;
            int first = candidates.front() % masks;
            bool equal = true;
            for (int r : candidates) if (r % masks != first) equal = false;
            if (equal) return 0;
            array<int8_t, 160> seen;
            seen.fill(-1);
            for (int r : candidates) {
                auto& s = seen[sumClass[r]];
                if (s >= 0 && s != r % masks) return 2;
                s = r % masks;
            }
            return 1;
        };

        struct Action {
            int mask, query;
            double score;
        };
        using Buckets = array<vector<int>, 51>;
        auto split = [&](const vector<int>& candidates, const Action& action, Buckets& buckets) {
            for (auto& bucket : buckets) bucket.clear();
            for (int id : candidates) {
                int after = id ^ action.mask;
                buckets[replies[action.query][after] + 25].push_back(after);
            }
        };
        map<pair<int, vector<int>>, Action> memo;
        auto search = [&](auto&& self, const vector<int>& candidates, int budget) -> Action {
            int lower = bound(candidates);
            if (lower == 0) return {0, -2, 0};
            if (lower > budget) return {0, -1, 0};
            auto key = make_pair(budget, candidates);
            if (auto it = memo.find(key); it != memo.end()) return it->second;

            Buckets buckets;
            vector<Action> options;
            int flips = int(candidates.size()) == total ? 1 : masks;
            for (int m = 0; m < flips; ++m) {
                int ops = popcount(unsigned(m)) + 1;
                if (ops > budget) continue;
                for (int q = 0; q < int(queries.size()); ++q) {
                    Action action = {m, q, double(ops)};
                    split(candidates, action, buckets);
                    int sum = 0;
                    double uncertainty = 0;
                    bool viable = true;
                    for (const auto& bucket : buckets) if (!bucket.empty()) {
                        int rest = bound(bucket), size = bucket.size();
                        if (rest > budget - ops || size == int(candidates.size())) {
                            viable = false;
                            break;
                        }
                        sum += size * rest;
                        if (rest) uncertainty += size * log2(size);
                    }
                    if (!viable) continue;
                    if (m == 0 && sum == 0) return memo[key] = action;
                    action.score += (sum + 0.04 * uncertainty) / candidates.size();
                    options.push_back(action);
                }
            }
            stable_sort(options.begin(), options.end(), [](const Action& a, const Action& b) {
                return a.score < b.score;
            });
            for (const Action& action : options) {
                split(candidates, action, buckets);
                int rest = budget - 1 - popcount(unsigned(action.mask));
                bool viable = true;
                for (auto& bucket : buckets) if (!bucket.empty()) {
                    sort(bucket.begin(), bucket.end());
                    if (self(self, bucket, rest).query == -1) {
                        viable = false;
                        break;
                    }
                }
                if (viable) return memo[key] = action;
            }
            return memo[key] = {0, -1, 0};
        };

        vector<int> candidates(total);
        iota(candidates.begin(), candidates.end(), 0);
        int budget = 1;
        while (budget <= 5 && search(search, candidates, budget).query == -1) ++budget;
        if (budget > 5) return false;
        while (bound(candidates) != 0) {
            Action action = {0, -1, 0};
            for (int b = 1; b <= budget; ++b) {
                action = search(search, candidates, b);
                if (action.query >= 0) {
                    budget = b;
                    break;
                }
            }
            if (action.query < 0) exit(0);
            for (int v = 0; v < n; ++v) if (action.mask & (1 << v)) flip(v);
            int reply = ask(queries[action.query]);
            for (int& id : candidates) id ^= action.mask;
            erase_if(candidates, [&](int id) { return replies[action.query][id] != reply; });
            if (candidates.empty()) exit(0);
            sort(candidates.begin(), candidates.end());
            budget -= 1 + popcount(unsigned(action.mask));
        }
        cout << '!';
        int mask = candidates.front() % masks;
        for (int v = 0; v < n; ++v) cout << ((mask >> v) & 1 ? " 1" : " -1");
        cout << '\n' << flush;
        return true;
    }

public:
    explicit Solver(int size)
        : n(size), graph(n), children(n), parent(n), height(n), subtree(n),
          depth(n), known(n), flipped(n), allowedRoot(n, true), offset(n), remaining(n - 1) {}

    void run() {
        for (int i = 1; i < n; ++i) {
            int a, b;
            if (!(cin >> a >> b)) return;
            --a;
            --b;
            graph[a].push_back(b);
            graph[b].push_back(a);
        }
        if (n <= 5 && solveSmall()) return;
        pair<int, int> best = {-1, 0};
        anchor = farthest(0, -1, 0, best);
        if (n >= 24) {
            for (int v = 0; v < n; ++v) if (int(graph[v].size()) == n - 1) {
                star = true;
                anchor = v;
                break;
            }
        }
        orient(anchor, -1);
        towards.assign(n, vector<uint16_t>(n));
        for (int v = 0; v < n; ++v) {
            towards[v][v] = v;
            for (int u : graph[v]) buildDirections(u, v, u, towards[v]);
        }
        makePatterns();
        recoverValues();
        if (starCenterValue) answerStar();
        else locateRoot();
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (cin >> n && n >= 2 && n <= 1000) Solver(n).run();
}

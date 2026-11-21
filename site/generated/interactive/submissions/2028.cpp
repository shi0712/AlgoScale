#include <bits/stdc++.h>
using namespace std;

constexpr int MAX_N = 500;

struct DSU {
    array<int, MAX_N> p;
    void reset() { p.fill(-1); }
    int root(int u) const {
        while (p[u] >= 0) u = p[u];
        return u;
    }
    bool join(int u, int v) {
        u = root(u);
        v = root(v);
        if (u == v) return false;
        if (p[u] > p[v]) swap(u, v);
        p[u] += p[v];
        p[v] = u;
        return true;
    }
};

struct Pool {
    vector<int> edges;
    int count = -1;
};

struct Split {
    vector<int> side[2];
    int count, low, high;

    void orient() {
        swap(side[0], side[1]);
        tie(low, high) = pair(count - high, count - low);
    }
    bool binary() const { return high - low == 1; }
    int size() const { return int(side[0].size() + side[1].size()); }
};

// A row can use either half of a pool. The second half contributes the
// known pool count minus the first-half count, allowing signed coefficients.
struct Code {
    int rows, cols, child, incidence = 0;
    vector<vector<int>> matrix;
    vector<bool> unrestricted;
    vector<int> order, anchorOrder;
    vector<vector<pair<int, int>>> entries;

    explicit Code(int r, const Code* a = nullptr) : rows(r), child(a ? a->cols : 0) {
        if (r == 1) {
            matrix = {{1}};
            unrestricted = {true};
        } else if (r == 3) {
            matrix = {{1, 1, 1, 0, 0}, {1, -1, 1, 1, 0}, {1, 1, -1, 0, 1}};
            unrestricted = {true, true, true, false, false};
        } else {
            int h = r / 2;
            matrix.assign(r, vector<int>(2 * child + h));
            unrestricted = a->unrestricted;
            unrestricted.insert(unrestricted.end(), a->unrestricted.begin(), a->unrestricted.end());
            unrestricted.resize(2 * child + h, false);
            for (int i = 0; i < h; ++i) {
                for (int j = 0; j < child; ++j) {
                    int x = a->matrix[i][j];
                    matrix[i][j] = matrix[i][j + child] = matrix[i + h][j] = x;
                    matrix[i + h][j + child] = -x;
                }
                matrix[i][2 * child + i] = 1;
            }
        }
        cols = int(unrestricted.size());
        entries.resize(cols);
        for (int j = 0; j < cols; ++j) {
            for (int i = 0; i < rows; ++i) {
                if (matrix[i][j]) entries[j].push_back({i, matrix[i][j] < 0});
            }
            incidence += entries[j].size();
            order.push_back(j);
        }
        stable_sort(order.begin(), order.end(), [&](int i, int j) {
            return entries[i].size() > entries[j].size();
        });
        anchorOrder = order;
        reverse(anchorOrder.begin(), anchorOrder.end());
    }

    void decode(const int* y, int* x, const map<int, Code>& codes) const {
        if (rows == 1) {
            x[0] = y[0];
        } else if (rows == 3) {
            x[3] = (y[1] - y[0]) & 1;
            x[4] = (y[2] - y[0]) & 1;
            x[1] = (y[0] - y[1] + x[3]) / 2;
            x[2] = (y[0] - y[2] + x[4]) / 2;
            x[0] = y[0] - x[1] - x[2];
        } else {
            // Adding the two row blocks cancels the second child code.
            // Parity then reveals the extra binary variables.
            int h = rows / 2;
            array<int, 32> left, right;
            for (int i = 0; i < h; ++i) {
                int bit = (y[i] + y[i + h]) & 1;
                x[2 * child + i] = bit;
                left[i] = (y[i] + y[i + h] - bit) / 2;
                right[i] = left[i] - y[i + h];
            }
            const Code& a = codes.at(h);
            a.decode(left.data(), x, codes);
            a.decode(right.data(), x + child, codes);
        }
    }
};

struct Pack {
    const Code* code;
    vector<int> column;
    vector<vector<pair<int, int>>> terms;
    int used = 0, queries = 0;
};

// An unsigned code for testing independent omissions. With q-ary inputs,
// the off-diagonal blocks make their sum a multiple of q. The extra
// variables can then be read modulo q.
struct OmissionCode {
    int radix, rows, cols;
    const OmissionCode* child;
    vector<vector<int>> matrix;

    OmissionCode(int q, const OmissionCode* previous) : radix(q), child(previous) {
        if (!child) {
            rows = cols = 1;
            matrix = {{1}};
            return;
        }
        int h = child->rows, c = child->cols;
        rows = (q + 1) * h;
        cols = (q + 1) * c + h;
        matrix.assign(rows, vector<int>(cols));
        for (int block = 0; block <= q; ++block) {
            for (int i = 0; i < h; ++i) {
                for (int other = 0; other <= q; ++other) if (other != block) {
                    copy(child->matrix[i].begin(), child->matrix[i].end(),
                         matrix[block * h + i].begin() + other * c);
                }
                if (block == 0) matrix[i][(q + 1) * c + i] = 1;
            }
        }
    }

    void decode(vector<int> y, int* x) const {
        if (!child) {
            x[0] = y[0];
            return;
        }
        int h = child->rows, c = child->cols;
        for (int i = 0; i < h; ++i) {
            int sum = 0;
            for (int j = i; j < rows; j += h) sum += y[j];
            int residual = sum % radix;
            if (radix == 3) {
                if (residual == 2) residual = -1;
                if (residual == -2) residual = 1;
            }
            x[(radix + 1) * c + i] = residual;
            y[i] -= residual;
            int total = (sum - residual) / radix;
            for (int j = i; j < rows; j += h) y[j] = total - y[j];
        }
        for (int j = 0; j <= radix; ++j) {
            child->decode(vector<int>(y.begin() + j * h, y.begin() + (j + 1) * h), x + j * c);
        }
    }
};

class Solver {
    int n, m;
    vector<pair<int, int>> edge;
    vector<vector<pair<int, int>>> adj;
    vector<int8_t> state;
    vector<int> basis, basisIndex;
    vector<pair<vector<int>, int>> samples;
    vector<Pool> pools;
    vector<int> royal;
    mt19937 rng{0x73cf29a5};
    map<int, Code> codes;

    static void emit(char command, const vector<int>& tree) {
        cout << command;
        for (int e : tree) cout << ' ' << e;
        cout << '\n' << flush;
    }

    [[noreturn]] void finish() const {
        emit('!', royal);
        exit(0);
    }

    int ask(const vector<int>& tree) const {
        emit('?', tree);
        int result;
        if (!(cin >> result) || result < 0 || result >= n) exit(0);
        if (result == n - 1) {
            emit('!', tree);
            exit(0);
        }
        return result;
    }

    bool mark(int e, int x) {
        if (state[e] >= 0) return false;
        state[e] = x;
        if (x) {
            royal.push_back(e);
            if ((int)royal.size() == n - 1) finish();
        }
        return true;
    }

    bool structural() {
        // Royal edges cannot close a cycle. Bridges of the remaining graph
        // must belong to every remaining candidate tree.
        bool changed = false;
        DSU d;
        d.reset();
        for (int e : royal) d.join(edge[e].first, edge[e].second);
        for (int e = 0; e < m; ++e) {
            if (state[e] < 0 && d.root(edge[e].first) == d.root(edge[e].second)) {
                changed |= mark(e, 0);
            }
        }

        array<int, MAX_N> tin{};
        int timer = 0;
        auto visit = [&](auto&& self, int u, int parentEdge) -> int {
            int low = tin[u] = ++timer;
            for (auto [v, e] : adj[u]) {
                if (e == parentEdge || state[e] == 0) continue;
                if (tin[v]) {
                    low = min(low, tin[v]);
                } else {
                    int sub = self(self, v, e);
                    if (sub > tin[u]) changed |= mark(e, 1);
                    low = min(low, sub);
                }
            }
            return low;
        };
        visit(visit, 0, -1);
        return changed;
    }

    // On small candidate spaces, compare whole spanning trees directly.
    void smallInstance(bool constrained = false) {
        vector<int> undecided;
        for (int e = 0; e < m; ++e) if (state[e] < 0) undecided.push_back(e);
        int s = int(undecided.size()), rank = n - 1 - int(royal.size());
        int k = min(rank, s - rank);
        if (s > (constrained ? 18 : 63)) return;
        long long combinations = 1;
        for (int i = 1; i <= k; ++i) {
            combinations = combinations * (s - i + 1) / i;
            if (combinations > 600000) return;
        }
        uint64_t all = (uint64_t(1) << s) - 1;
        bool omitted = k != rank;
        vector<pair<uint64_t, int>> requirements;
        auto addRequirement = [&](const vector<int>& edges, int knownCount) {
            uint64_t group = 0;
            for (int e : edges) {
                if (state[e] < 0) {
                    int i = lower_bound(undecided.begin(), undecided.end(), e) - undecided.begin();
                    group |= uint64_t(1) << i;
                } else {
                    knownCount -= state[e];
                }
            }
            if (group) requirements.push_back({group, knownCount});
        };
        if (constrained) {
            for (const Pool& p : pools) addRequirement(p.edges, p.count);
        }
        for (const auto& [tree, score] : samples) addRequirement(tree, score);
        vector<uint64_t> possibilities;
        DSU fixed;
        fixed.reset();
        for (int e : royal) fixed.join(edge[e].first, edge[e].second);
        auto treeFor = [&](uint64_t mask) { return omitted ? all ^ mask : mask; };
        uint64_t mask = (uint64_t(1) << k) - 1;
        for (long long i = 0; i < combinations; ++i) {
            uint64_t selected = treeFor(mask);
            if (all_of(requirements.begin(), requirements.end(), [&](auto req) {
                return popcount(req.first & selected) == req.second;
            })) {
                DSU d = fixed;
                bool valid = true;
                for (uint64_t z = selected; z; z &= z - 1) {
                    auto [u, v] = edge[undecided[countr_zero(z)]];
                    if (!d.join(u, v)) {
                        valid = false;
                        break;
                    }
                }
                if (valid) {
                    possibilities.push_back(mask);
                    if (possibilities.size() > 20000) return;
                }
            }
            if (i + 1 < combinations) {
                uint64_t bit = mask & -mask;
                uint64_t next = mask + bit;
                mask = next | (((next ^ mask) >> 2) / bit);
            }
        }
        vector<uint64_t> available = possibilities;
        auto materialize = [&](uint64_t z) {
            vector<int> t = royal;
            for (z = treeFor(z); z; z &= z - 1) t.push_back(undecided[countr_zero(z)]);
            return t;
        };
        while (possibilities.size() > 1) {
            long long bestLoss = LLONG_MAX;
            uint64_t chosen = possibilities.front();
            auto evaluate = [&](uint64_t q) {
                array<int, MAX_N> buckets{};
                for (uint64_t x : possibilities) ++buckets[popcount(x & q)];
                long long loss = 0;
                for (int i = 0; i <= k; ++i) loss += 1LL * buckets[i] * buckets[i];
                if (loss < bestLoss) {
                    bestLoss = loss;
                    chosen = q;
                }
            };
            if (k > 1) {
                int trials = min<int>(available.size(), 160);
                if (available.size() * possibilities.size() <= 180000) trials = available.size();
                evaluate(chosen);
                for (int i = 0; i < trials; ++i) {
                    evaluate(trials == (int)available.size() ? available[i] : available[rng() % available.size()]);
                }
            }
            int common = ask(materialize(chosen)) - (omitted ? n - 1 - k : int(royal.size()));
            erase_if(possibilities, [&](uint64_t x) { return popcount(x & chosen) != common; });
        }
        royal = materialize(possibilities.front());
        finish();
    }

    vector<int> makeBasis(const vector<int>& order) const {
        DSU d;
        d.reset();
        vector<int> result;
        for (int e : order) {
            if (state[e] == 0) continue;
            auto [u, v] = edge[e];
            if (d.join(u, v)) {
                result.push_back(e);
                if ((int)result.size() == n - 1) break;
            }
        }
        return result;
    }

    vector<int> dfsBasis() {
        vector<int> candidate;
        array<bool, MAX_N> seen{};
        auto dfs = [&](auto&& self, int u) -> void {
            seen[u] = true;
            for (auto [v, e] : adj[u]) {
                if (!seen[v] && state[e] != 0) {
                    candidate.push_back(e);
                    self(self, v);
                }
            }
        };
        dfs(dfs, rng() % n);
        return candidate;
    }

    bool solveCycles() {
        vector<vector<int>> cycles;
        vector<int> stack, cycleOf(m, -1);
        array<int, MAX_N> tin{}, last{};
        int timer = 0, block = 0;
        auto find = [&](auto&& self, int u, int parentEdge) -> int {
            int low = tin[u] = ++timer;
            for (auto [v, e] : adj[u]) {
                if (e == parentEdge || state[e] == 0) continue;
                if (tin[v] == 0) {
                    stack.push_back(e);
                    int sub = self(self, v, e);
                    low = min(low, sub);
                    if (sub < tin[u]) continue;
                    int vertices = 0;
                    vector<int> component;
                    ++block;
                    do {
                        int f = stack.back();
                        stack.pop_back();
                        component.push_back(f);
                        auto [a, b] = edge[f];
                        for (int w : {a, b}) if (last[w] != block) {
                            last[w] = block;
                            ++vertices;
                        }
                    } while (component.back() != e);
                    if ((int)component.size() == vertices) {
                        for (int f : component) cycleOf[f] = cycles.size();
                        cycles.push_back(move(component));
                    }
                } else if (tin[v] < tin[u]) {
                    stack.push_back(e);
                    low = min(low, tin[v]);
                }
            }
            return low;
        };
        find(find, 0, -1);
        if (cycles.empty()) return false;

        vector<int> fixed = dfsBasis();
        erase_if(fixed, [&](int e) { return cycleOf[e] >= 0; });
        int g = cycles.size();
        vector<int> guess(g), first(g), position(g, 1), alternative(g);
        for (int i = 0; i < g; ++i) {
            shuffle(cycles[i].begin(), cycles[i].end(), rng);
            guess[i] = first[i] = cycles[i][0];
            alternative[i] = cycles[i][1];
        }
        auto tree = [&] {
            vector<int> t = fixed;
            for (int i = 0; i < g; ++i) {
                for (int e : cycles[i]) if (e != guess[i]) t.push_back(e);
            }
            return t;
        };
        auto completeCycle = [&](int i, int omitted) {
            guess[i] = omitted;
            position[i] = cycles[i].size();
            for (int e : cycles[i]) mark(e, e != omitted);
        };

        // The sum from the complex blocks is constant in all these queries.
        int base = ask(tree());
        auto testOmissions = [&](const vector<int>& open, int q) {
            vector<OmissionCode> library;
            library.reserve(6);
            library.emplace_back(q, nullptr);
            while (library.back().rows * (q + 1) <= (int)open.size()) {
                library.emplace_back(q, &library.back());
            }
            int size = open.size();
            vector<int> cost(size + 1), plan(size + 1);
            for (int count = 1; count <= size; ++count) {
                cost[count] = count;
                for (int i = 1; i < (int)library.size(); ++i) {
                    const auto& code = library[i];
                    int left = max(0, count - code.cols);
                    if (code.rows + cost[left] <= cost[count]) {
                        cost[count] = code.rows + cost[left];
                        plan[count] = i;
                    }
                }
            }
            vector<int> output;
            for (int start = 0; start < size;) {
                const auto& code = library[plan[size - start]];
                int take = min(code.cols, size - start);
                vector<int> answer(code.rows);
                for (int r = 0; r < code.rows; ++r) {
                    const auto& row = code.matrix[r];
                    auto same = [&](int j) {
                        return equal(row.begin(), row.begin() + take, code.matrix[j].begin());
                    };
                    int i = 0;
                    while (i < r && !same(i)) ++i;
                    if (i < r) {
                        answer[r] = answer[i];
                    } else if (any_of(row.begin(), row.begin() + take, [](int v) { return v != 0; })) {
                        for (int j = 0; j < take; ++j) if (row[j]) {
                            guess[open[start + j]] = alternative[open[start + j]];
                        }
                        answer[r] = ask(tree()) - base;
                        for (int j = 0; j < take; ++j) if (row[j]) {
                            guess[open[start + j]] = first[open[start + j]];
                        }
                    }
                }
                vector<int> decoded(code.cols);
                code.decode(move(answer), decoded.data());
                output.insert(output.end(), decoded.begin(), decoded.begin() + take);
                start += take;
            }
            return output;
        };

        vector<int> all(g);
        iota(all.begin(), all.end(), 0);
        auto comparison = testOmissions(all, 3);
        for (int i = 0; i < g; ++i) {
            if (comparison[i] < 0) completeCycle(i, first[i]);
            else if (comparison[i] > 0) {
                completeCycle(i, alternative[i]);
                ++base;
            } else if (cycles[i].size() == 3) {
                completeCycle(i, cycles[i][2]);
                ++base;
            } else {
                position[i] = 2;
            }
        }
        while (true) {
            vector<int> open;
            for (int i = 0; i < g; ++i) if (position[i] < (int)cycles[i].size()) {
                open.push_back(i);
                alternative[i] = cycles[i][position[i]];
            }
            if (open.empty()) break;
            auto hits = testOmissions(open, 2);
            for (int j = 0; j < (int)open.size(); ++j) {
                int i = open[j];
                ++position[i];
                if (hits[j] || position[i] == (int)cycles[i].size() - 1) {
                    completeCycle(i, hits[j] ? alternative[i] : cycles[i].back());
                    ++base;
                }
            }
        }
        if (base == n - 1) {
            royal = tree();
            finish();
        }
        auto t = tree();
        sort(t.begin(), t.end());
        samples.push_back({move(t), base});
        return true;
    }

    void findBasis() {
        int openRank = n - 1 - int(royal.size());
        int openEdges = count(state.begin(), state.end(), int8_t(-1));
        double expected = double(openRank) * openRank / openEdges;
        int initialTries = expected <= 3.15 ? 12 : expected <= 5.1 ? 6 : expected <= 7.5 ? 3 : 1;
        int tries = initialTries;
        if (openRank >= 60 && openEdges >= 16 * openRank && expected <= 15) {
            tries = max(tries, min(80, int(5 * expected + 20)));
        }

        vector<int> order(m), frequency(m), membership(m);
        iota(order.begin(), order.end(), 0);
        int best = n, lastImprovement = 0;
        if (!samples.empty()) {
            basis = samples.front().first;
            best = samples.front().second;
            for (int e : basis) if (state[e] < 0) ++frequency[e];
        }
        vector<int> stars, degree(n);
        for (int u = 0; u < n; ++u) if ((int)adj[u].size() == n - 1) {
            if (all_of(royal.begin(), royal.end(), [&](int e) {
                return edge[e].first == u || edge[e].second == u;
            }) && none_of(adj[u].begin(), adj[u].end(), [&](auto road) {
                return state[road.second] == 0;
            })) {
                stars.push_back(u);
            }
        }
        auto checkStars = [&] {
            if (stars.empty() || stars.size() > 3) return;
            for (int u : stars) {
                vector<int> t;
                for (auto [v, e] : adj[u]) t.push_back(e);
                int score = ask(t);
                sort(t.begin(), t.end());
                samples.push_back({move(t), score});
                if (score != 1) break;
            }
            stars.clear();
        };
        checkStars();
        for (int iteration = 0; iteration < tries; ++iteration) {
            if (best == (int)royal.size() || (tries == 1 && !basis.empty())) break;
            vector<int> candidate;
            if (tries == 1) {
                candidate = dfsBasis();
            } else {
                shuffle(order.begin(), order.end(), rng);
                if (!basis.empty() && iteration >= min(initialTries, 3)) {
                    // Retain most of the lowest-scoring tree and repair it
                    // after removing a random fraction of its edges.
                    int stamp = iteration + 1;
                    vector<int> favored, deferred;
                    favored.reserve(m);
                    double replace = clamp(1.1 / expected, 0.07, 0.6);
                    for (int e : basis) {
                        membership[e] = stamp;
                        if (state[e] == 1 || rng() > replace * mt19937::max()) {
                            favored.push_back(e);
                        } else {
                            deferred.push_back(e);
                        }
                    }
                    for (int e : order) if (membership[e] != stamp) favored.push_back(e);
                    favored.insert(favored.end(), deferred.begin(), deferred.end());
                    candidate = makeBasis(favored);
                } else {
                    stable_sort(order.begin(), order.end(), [&](int a, int b) {
                        return frequency[a] < frequency[b];
                    });
                    candidate = makeBasis(order);
                }
            }
            auto key = candidate;
            sort(key.begin(), key.end());
            bool duplicate = false;
            for (const auto& [t, score] : samples) duplicate |= t == key;
            for (int e : candidate) if (state[e] < 0) ++frequency[e];
            if (duplicate) continue;
            int b = ask(candidate);
            if (!stars.empty()) {
                fill(degree.begin(), degree.end(), 0);
                for (int e : candidate) {
                    ++degree[edge[e].first];
                    ++degree[edge[e].second];
                }
                erase_if(stars, [&](int u) { return degree[u] != b; });
                checkStars();
            }
            samples.push_back({move(key), b});
            if (b < best) {
                basis = move(candidate);
                best = b;
                lastImprovement = iteration;
            }
            if (iteration - lastImprovement >= max(15, int(2.1 * expected))) break;
        }
        if (best == (int)royal.size()) {
            for (int e : basis) mark(e, 0);
            return;
        }
        if (basis.empty()) {
            basis = samples.front().first;
            best = samples.front().second;
        }

        basisIndex.assign(m, -1);
        int remaining = 0, need = best;
        vector<vector<pair<int, int>>> treeAdj(n);
        for (int i = 0; i < n - 1; ++i) {
            int e = basis[i];
            basisIndex[e] = i;
            remaining += state[e] < 0;
            need -= state[e] == 1;
            auto [u, v] = edge[e];
            treeAdj[u].push_back({v, e});
            treeAdj[v].push_back({u, e});
        }
        auto classify = [&](int e, int x) {
            if (mark(e, x) && basisIndex[e] >= 0) {
                --remaining;
                need -= x;
            }
        };
        auto forced = [&] {
            if (need != 0 && need != remaining) return false;
            int x = need != 0;
            for (int e : basis) classify(e, x);
            return true;
        };
        if (forced()) return;

        vector<int> depth(n), parent(n), parentEdge(n);
        vector<vector<short>> distance(n, vector<short>(n));
        auto walk = [&](auto&& self, int root, int u, int p, int dist) -> void {
            distance[root][u] = dist;
            for (auto [v, e] : treeAdj[u]) if (v != p) {
                if (root == 0) {
                    parent[v] = u;
                    parentEdge[v] = e;
                    depth[v] = dist + 1;
                }
                self(self, root, v, u, dist + 1);
            }
        };
        for (int u = 0; u < n; ++u) walk(walk, u, u, -1, 0);
        vector<vector<int>> byLength(n);
        for (int e = 0; e < m; ++e) if (basisIndex[e] < 0) {
            auto [u, v] = edge[e];
            byLength[distance[u][v]].push_back(e);
        }

        auto exchange = [&](int e, int f) {
            int i = basisIndex[f];
            basis[i] = e;
            int score = -1;
            for (const auto& [sample, count] : samples) {
                if (!binary_search(sample.begin(), sample.end(), e) ||
                    binary_search(sample.begin(), sample.end(), f)) continue;
                if (all_of(basis.begin(), basis.end(), [&](int g) {
                    return binary_search(sample.begin(), sample.end(), g);
                })) {
                    score = count;
                    break;
                }
            }
            if (score < 0) score = ask(basis);
            basis[i] = f;
            return score - best;
        };

        vector<int> path, pending;
        // Exchanging a chord for any edge of its fundamental path gives the
        // difference of their binary statuses. An entire cycle cannot be royal.
        for (int len = n - 1; len >= 2; --len) {
            for (int e : byLength[len]) {
                auto [u, v] = edge[e];
                path.clear();
                int knownZero = -1;
                while (u != v) {
                    if (depth[u] < depth[v]) swap(u, v);
                    int f = parentEdge[u];
                    if (state[f] < 0) path.push_back(f);
                    if (state[f] == 0) knownZero = f;
                    u = parent[u];
                }
                if (path.empty()) continue;
                pending.clear();
                for (int f : path) {
                    int diff = exchange(e, f);
                    if (state[e] >= 0) {
                        classify(f, state[e] - diff);
                    } else if (diff != 0) {
                        classify(e, diff > 0);
                        classify(f, diff < 0);
                    } else {
                        pending.push_back(f);
                        if ((int)pending.size() > need) classify(e, 0);
                        else if ((int)pending.size() > remaining - need) classify(e, 1);
                    }
                    if (state[e] >= 0) {
                        for (int g : pending) classify(g, state[e]);
                        pending.clear();
                    }
                    if (forced()) return;
                }
                if (!pending.empty()) {
                    int x = knownZero >= 0 ? exchange(e, knownZero) : 0;
                    classify(e, x);
                    for (int f : pending) classify(f, x);
                    if (forced()) return;
                }
            }
        }
    }

    int forestCount(const vector<int>& forest) const {
        if (forest.empty()) return 0;
        // Complete the forest using only edges whose contribution is known.
        vector<int> query = forest;
        DSU d;
        d.reset();
        for (int e : forest) d.join(edge[e].first, edge[e].second);
        int padding = 0;
        for (int e : royal) {
            if (d.join(edge[e].first, edge[e].second)) {
                query.push_back(e);
                ++padding;
            }
        }
        for (int e : basis) {
            if ((int)query.size() == n - 1) break;
            if (d.join(edge[e].first, edge[e].second)) {
                query.push_back(e);
                padding += state[e];
            }
        }
        return ask(query) - padding;
    }

    void makePools() {
        vector<bool> used(m);
        for (const auto& [t, answer] : samples) {
            Pool p;
            p.count = answer;
            for (int e : t) {
                if (state[e] < 0) p.edges.push_back(e);
                else p.count -= state[e];
            }
            if (p.edges.empty() || any_of(p.edges.begin(), p.edges.end(), [&](int e) { return used[e]; })) {
                continue;
            }
            for (int e : p.edges) used[e] = true;
            pools.push_back(move(p));
        }
        vector<int> todo;
        for (int e = 0; e < m; ++e) if (state[e] < 0 && !used[e]) todo.push_back(e);
        if (todo.empty()) return;

        shuffle(todo.begin(), todo.end(), rng);
        vector<Pool> greedy, paths((n + 1) / 2);
        // Group consecutive endpoint sums modulo the next even vertex count.
        // Each group is contained in one Hamilton path of that complete graph.
        for (int e : todo) {
            auto [u, v] = edge[e];
            paths[((u + v + 1) / 2) % paths.size()].edges.push_back(e);
        }
        erase_if(paths, [](const Pool& p) { return p.edges.empty(); });
        for (auto pending = todo; !pending.empty();) {
            DSU d;
            d.reset();
            Pool p;
            vector<int> next;
            for (int e : pending) {
                if (d.join(edge[e].first, edge[e].second)) p.edges.push_back(e);
                else next.push_back(e);
            }
            greedy.push_back(move(p));
            pending = move(next);
        }

        double density = double(n - 1 - royal.size()) / count(state.begin(), state.end(), int8_t(-1));
        auto estimate = [&](const vector<Pool>& partition) {
            double result = partition.size();
            for (const Pool& p : partition) {
                int k = p.edges.size();
                result += 0.45 * density * k * log2(k);
            }
            return result;
        };
        auto& chosen = estimate(paths) < estimate(greedy) ? paths : greedy;
        for (Pool& p : chosen) pools.push_back(move(p));
    }

    void reducePools() {
        bool changed;
        do {
            changed = structural();
            for (const auto& [tree, score] : samples) {
                int left = score, unknown = 0;
                for (int e : tree) {
                    if (state[e] < 0) ++unknown;
                    else left -= state[e];
                }
                if (unknown && (left == 0 || left == unknown)) {
                    for (int e : tree) changed |= mark(e, left != 0);
                }
            }
            int unmeasuredEdges = 0, unmeasuredPools = 0;
            int remaining = n - 1 - int(royal.size());
            for (Pool& p : pools) {
                erase_if(p.edges, [&](int e) {
                    if (state[e] < 0) return false;
                    if (p.count >= 0) p.count -= state[e];
                    return true;
                });
                if (p.edges.empty()) continue;
                if (p.count >= 0) remaining -= p.count;
                else {
                    unmeasuredEdges += p.edges.size();
                    ++unmeasuredPools;
                }
            }
            for (Pool& p : pools) {
                if (p.edges.empty()) continue;
                if (p.count < 0) {
                    if (remaining == 0) p.count = 0;
                    else if (remaining == unmeasuredEdges) p.count = p.edges.size();
                    else if (unmeasuredPools == 1) p.count = remaining;
                }
                if (p.count == 0 || p.count == (int)p.edges.size()) {
                    for (int e : p.edges) changed |= mark(e, p.count != 0);
                    p.edges.clear();
                    p.count = 0;
                }
            }
        } while (changed);
        erase_if(pools, [](const Pool& p) { return p.edges.empty(); });
    }

    void measurePools() {
        vector<int> order;
        int remaining = n - 1 - int(royal.size()), openEdges = 0;
        for (int i = 0; i < (int)pools.size(); ++i) {
            const Pool& p = pools[i];
            if (p.count >= 0) {
                remaining -= p.count;
            } else {
                order.push_back(i);
                openEdges += p.edges.size();
            }
        }
        stable_sort(order.begin(), order.end(), [&](int a, int b) {
            return pools[a].edges.size() > pools[b].edges.size();
        });
        for (int i : order) {
            Pool& p = pools[i];
            if (remaining == 0) p.count = 0;
            else if (remaining == openEdges) p.count = p.edges.size();
            else if (openEdges == (int)p.edges.size()) p.count = remaining;
            else p.count = forestCount(p.edges);
            remaining -= p.count;
            openEdges -= p.edges.size();
        }
    }

    Pack buildPack(const Code& code, const vector<Split>& splits,
                   const vector<int>& active, int anchor) const {
        Pack result{&code, vector<int>(code.cols, -1), vector<vector<pair<int, int>>>(code.rows)};
        array<DSU, 64> rows, trial;
        array<int, 64> load{};
        for (int i = 0; i < code.rows; ++i) rows[i].reset();
        vector<bool> placed(splits.size());
        auto tryAdd = [&](int c, int id, int flip) {
            const Split& s = splits[id];
            for (auto [i, sign] : code.entries[c]) {
                if (load[i] + s.side[sign ^ flip].size() >= (size_t)n) return false;
            }
            for (auto [i, sign] : code.entries[c]) {
                trial[i] = rows[i];
                for (int e : s.side[sign ^ flip]) {
                    if (!trial[i].join(edge[e].first, edge[e].second)) return false;
                }
            }
            for (auto [i, sign] : code.entries[c]) {
                rows[i] = trial[i];
                load[i] += s.side[sign ^ flip].size();
                result.terms[i].push_back({id, sign ^ flip});
            }
            result.column[c] = id * 2 + flip;
            placed[id] = true;
            ++result.used;
            return true;
        };
        auto compatible = [&](int c, int id) {
            return code.unrestricted[c] || splits[id].binary();
        };

        for (int c : code.anchorOrder) if (compatible(c, anchor)) {
            tryAdd(c, anchor, 0);
            break;
        }
        for (int c : code.order) if (result.column[c] < 0) {
            int trials = 0;
            for (int id : active) {
                if (placed[id] || !compatible(c, id)) continue;
                if (tryAdd(c, id, 0) || tryAdd(c, id, 1)) break;
                if (++trials >= 15) break;
            }
        }
        for (int i = 0; i < code.rows; ++i) {
            auto& terms = result.terms[i];
            if (terms.empty()) continue;
            sort(terms.begin(), terms.end());
            bool duplicate = false;
            for (int j = 0; j < i; ++j) duplicate |= terms == result.terms[j];
            result.queries += !duplicate;
        }
        return result;
    }

    void splitPools() {
        vector<Split> splits;
        for (auto& [edges, count] : pools) {
            Split s;
            s.count = count;
            int divisor = edges.size() >= size_t(max(6, n / 3)) ? 3 : 2;
            auto middle = edges.begin() + edges.size() / divisor;
            s.side[0].assign(edges.begin(), middle);
            s.side[1].assign(middle, edges.end());
            s.low = max(0, count - int(s.side[1].size()));
            s.high = min(count, int(s.side[0].size()));
            splits.push_back(move(s));
        }
        vector<int> ready(splits.size());
        iota(ready.begin(), ready.end(), 0);
        stable_sort(ready.begin(), ready.end(), [&](int a, int b) {
            return splits[a].size() < splits[b].size();
        });
        vector<bool> used(splits.size());
        int todo = splits.size();
        vector<Pool> next;
        while (todo > 0) {
            erase_if(ready, [&](int id) { return used[id]; });
            int binary = 0;
            for (int id : ready) binary += splits[id].binary();
            int anchor = ready.back();
            Pack best{nullptr, {}, {}, 1, 1};
            for (auto it = codes.rbegin(); it != codes.rend(); ++it) {
                const Code& code = it->second;
                if (code.rows == 1 || todo <= code.rows || todo * 4 < code.cols * 3) continue;
                int capacity = min({todo, code.cols, code.rows + binary});
                if (best.used * code.rows >= best.queries * capacity) continue;
                if (code.rows >= 8) {
                    int typical = splits[ready[code.rows / 2]].side[0].size();
                    if (2 * typical * code.incidence > 3 * n * code.rows) continue;
                }
                Pack p = buildPack(code, splits, ready, anchor);
                if (p.used * best.queries > best.used * p.queries ||
                    (p.used * best.queries == best.used * p.queries && p.used > best.used && p.used > p.queries)) {
                    best = move(p);
                    if (best.used == code.cols && best.queries == code.rows) break;
                }
            }
            if (!best.code) {
                Split& s = splits[anchor];
                int companion = -1, firstSide = 0, secondSide = 0;
                vector<int> forest;
                for (int x = 0; x < 2 && companion < 0; ++x) {
                    DSU d;
                    d.reset();
                    for (int e : s.side[x]) d.join(edge[e].first, edge[e].second);
                    int trials = 0;
                    for (int id : ready) {
                        if (id == anchor || used[id]) continue;
                        const Split& t = splits[id];
                        if (s.side[x].size() + t.side[0].size() >= (size_t)n) break;
                        for (int y = 0; y < 2; ++y) {
                            DSU both = d;
                            if (all_of(t.side[y].begin(), t.side[y].end(), [&](int e) {
                                return both.join(edge[e].first, edge[e].second);
                            })) {
                                companion = id;
                                firstSide = x;
                                secondSide = y;
                                break;
                            }
                        }
                        if (companion >= 0 || ++trials >= 20) break;
                    }
                }
                int a;
                if (companion >= 0) {
                    Split& t = splits[companion];
                    if (firstSide) s.orient();
                    if (secondSide) t.orient();
                    forest = s.side[0];
                    forest.insert(forest.end(), t.side[0].begin(), t.side[0].end());
                    int sum = forestCount(forest);
                    int low = max(s.low, sum - t.high), high = min(s.high, sum - t.low);
                    a = low == high ? low : forestCount(s.side[0]);
                    next.push_back({move(t.side[0]), sum - a});
                    next.push_back({move(t.side[1]), t.count - sum + a});
                    used[companion] = true;
                    --todo;
                } else {
                    a = forestCount(s.side[0]);
                }
                next.push_back({move(s.side[0]), a});
                next.push_back({move(s.side[1]), s.count - a});
                used[anchor] = true;
                --todo;
                continue;
            }

            array<int, 64> measured{}, rhs{};
            for (int i = 0; i < best.code->rows; ++i) {
                const auto& terms = best.terms[i];
                if (terms.empty()) continue;
                int same = 0;
                while (same < i && best.terms[same] != terms) ++same;
                if (same < i) {
                    measured[i] = measured[same];
                } else {
                    vector<int> forest;
                    for (auto [id, side] : terms) {
                        const auto& group = splits[id].side[side];
                        forest.insert(forest.end(), group.begin(), group.end());
                    }
                    measured[i] = forestCount(forest);
                }
                rhs[i] = measured[i];
            }
            for (int c = 0; c < best.code->cols; ++c) {
                int id = best.column[c] / 2, flip = best.column[c] & 1;
                if (best.column[c] < 0) continue;
                const Split& s = splits[id];
                int low = flip ? s.count - s.high : s.low;
                for (auto [i, sign] : best.code->entries[c]) rhs[i] -= sign ? s.count - low : low;
            }
            vector<int> decoded(best.code->cols);
            best.code->decode(rhs.data(), decoded.data(), codes);
            for (int c = 0; c < best.code->cols; ++c) {
                if (best.column[c] < 0) continue;
                int id = best.column[c] / 2, flip = best.column[c] & 1;
                Split& s = splits[id];
                if (flip) s.orient();
                int a = decoded[c] + s.low;
                next.push_back({move(s.side[0]), a});
                next.push_back({move(s.side[1]), s.count - a});
                used[id] = true;
                --todo;
            }
        }
        pools = move(next);
    }

public:
    void run() {
        if (!(cin >> n >> m)) return;
        edge.resize(m);
        adj.resize(n);
        state.assign(m, -1);
        for (int e = 0; e < m; ++e) {
            auto& [u, v] = edge[e];
            if (!(cin >> u >> v)) return;
            adj[u].push_back({v, e});
            adj[v].push_back({u, e});
        }
        structural();
        smallInstance();
        for (auto& a : adj) shuffle(a.begin(), a.end(), rng);
        if (solveCycles()) smallInstance();
        findBasis();
        structural();
        makePools();
        reducePools();

        for (int r : {1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64}) {
            codes.try_emplace(r, r, r == 1 || r == 3 ? nullptr : &codes.at(r / 2));
        }
        measurePools();
        reducePools();
        while (true) {
            int undecided = 0;
            for (const Pool& p : pools) undecided += p.edges.size();
            if (undecided <= 18) smallInstance(true);
            splitPools();
            reducePools();
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Solver().run();
}

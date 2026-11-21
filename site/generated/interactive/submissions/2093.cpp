#include <algorithm>
#include <array>
#include <bitset>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>
using namespace std;

constexpr int M = 1024;
using Mask = bitset<M>;
int n, k;
array<int, M> cafeIndex;
// Every variety still occurs at some live café. An edge joins two live
// cafés whose varieties might coincide. A negative reply rules out edges,
// and a positive with a different live witness permits retirement. Once
// no edges remain, the live cafés represent all varieties exactly once.
array<Mask, M> unknown;
Mask live;
int edges;
mt19937 rng(8571319);

array<int, M> parent;
array<Mask, M> apart;
int resetsUsed;
struct Hit {
    int cup;
    vector<int> sources;
};
vector<Hit> hits;

int representative(int a) {
    while (a != parent[a]) a = parent[a] = parent[parent[a]];
    return a;
}

struct Entry {
    int original;
    Mask possible;
    int single;
};
deque<Entry> history;

void resetMemory() {
    if (!history.empty()) {
        cout << "R" << endl;
        ++resetsUsed;
        history.clear();
    }
}

void removeEdge(int a, int b) {
    if (unknown[a][b]) {
        unknown[a].reset(b);
        unknown[b].reset(a);
        --edges;
    }
}

bool different(int a, int b) {
    if (apart[a][b]) return false;
    apart[a].set(b);
    apart[b].set(a);
    removeEdge(a, b);
    return true;
}

int single(const Mask& options) {
    return options.count() == 1 ? (int)options._Find_first() : -1;
}

// Each forgotten representative has a witness among the remaining cafés.
// Substitute those witnesses into any still-relevant memory entry.
void retire(int c, const Mask& witnesses) {
    for (Entry& e : history) if (e.possible[c]) {
        e.possible.reset(c);
        e.possible |= witnesses;
        e.single = single(e.possible);
    }
    while (unknown[c].any()) removeEdge(c, (int)unknown[c]._Find_first());
    live.reset(c);
}

void mergeEqual(int a, int b) {
    if (a == b) return;
    if (live[a] && !live[b]) swap(a,b);
    if (live[a]) {
        Mask witness;
        witness.set(b);
        retire(a, witness);
    }
    parent[a] = b;
    for (size_t j = apart[a]._Find_first(); j < M; j = apart[a]._Find_next(j))
        different(b, representative((int)j));
}

bool taste(int c) {
    cout << "? " << cafeIndex[c] + 1 << endl;
    char answer;
    if (!(cin >> answer) || (answer != 'Y' && answer != 'N')) exit(0);
    Entry next{c, {}, c};
    next.possible.set(c);
    if (answer == 'N') {
        for (const Entry& e : history) {
            different(c, representative(e.original));
            if (e.single >= 0) different(c, e.single);
        }
    } else {
        Mask matches, sources;
        for (const Entry& e : history) {
            int a = representative(e.original);
            if (!apart[c][a]) {
                sources.set(a);
                matches |= e.possible;
            }
        }
        matches &= ~apart[c];
        if (sources.none() || matches.none()) exit(0);
        if (single(sources) >= 0) {
            int a = (int)sources._Find_first();
            mergeEqual(c, a);
        } else if (!sources[c]) {
            Hit h{c, {}};
            for (size_t j = sources._Find_first(); j < M; j = sources._Find_next(j))
                h.sources.push_back((int)j);
            hits.push_back(move(h));
        }
        if (!matches[c]) {
            next.possible = matches;
            next.single = single(matches);
            if (next.single >= 0) mergeEqual(representative(c), next.single);
            else if (live[c]) retire(c, matches);
        }
        int a = representative(c);
        if (live[a]) {
            next.possible.reset();
            next.possible.set(a);
            next.single = a;
        }
    }
    history.push_back(next);
    if ((int)history.size() > k) history.pop_front();
    return !live[c];
}

// A past positive can become an exact equality after later negatives.
// Also propagate inequalities shared by every remaining explanation.
void infer() {
    for (int pass = 0; pass < 4 && !hits.empty(); ++pass) {
        bool change = false;
        for (size_t i = 0; i < hits.size();) {
            Hit& h = hits[i];
            int a = representative(h.cup);
            Mask choices;
            for (int v : h.sources) {
                int b = representative(v);
                if (!apart[a][b]) choices.set(b);
            }
            if (choices[a] || choices.count() <= 1) {
                if (!choices[a] && choices.any()) {
                    mergeEqual(a, (int)choices._Find_first());
                    change = true;
                }
                swap(h, hits.back());
                hits.pop_back();
                continue;
            }
            Mask common;
            common.set();
            for (size_t j = choices._Find_first(); j < M; j = choices._Find_next(j)) {
                common &= apart[j];
                if (common.none()) break;
            }
            common &= ~apart[a];
            for (size_t j = common._Find_first(); j < M; j = common._Find_next(j))
                change |= different(a, representative((int)j));
            ++i;
        }
        if (!change) break;
    }
    for (Entry& e : history) {
        int a = representative(e.original);
        if (live[a]) { e.possible.reset(); e.possible.set(a); }
        else e.possible &= ~apart[a];
        e.single = single(e.possible);
    }
}

// With a one-cup memory, each trail tests one new edge per step.
// A temporary vertex joins odd endpoints for Hierholzer's algorithm.
void solveOne() {
    for (int i = 0; i < n && edges; ++i) taste(i);
    // Test opposite and reflected pairs early, using an unresolved edge to
    // travel between them. Each such move itself makes a comparison.
    while (edges) {
        int tail = history.back().single, direct = -1, bridge = -1;
        for (int i = 0; i < n && direct < 0; ++i) {
            int a = representative(i);
            for (int j : {i^(n/2), n-1-i}) {
                int b = representative(j);
                if (!unknown[a][b]) continue;
                if (a == tail) direct = b;
                else if (b == tail) direct = a;
                else if (unknown[tail][a]) bridge = a;
            }
        }
        int c = direct >= 0 ? direct : bridge;
        if (c < 0) break;
        taste(c);
    }
    while (edges) {
        int tail = history.empty() ? -1 : history.back().single;
        vector<vector<int>> adj(n + 1);
        vector<char> used;
        vector<array<int, 2>> ends;
        auto add = [&](int a, int b) {
            int e = used.size();
            used.push_back(0);
            ends.push_back({a, b});
            adj[a].push_back(e);
            adj[b].push_back(e);
        };
        vector<int> component(n);
        iota(component.begin(), component.end(), 0);
        auto root = [&](int x) {
            while (x != component[x]) x = component[x] = component[component[x]];
            return x;
        };
        for (int i = 0; i < n; ++i)
            for (int j = i + 1; j < n; ++j) if (unknown[i][j]) {
                add(i, j);
                component[root(i)] = root(j);
            }
        vector<int> start(n, -1);
        vector<bool> hasOdd(n, false);
        for (int i = 0; i < n; ++i) if (!adj[i].empty()) {
            int r = root(i);
            if (start[r] < 0 || i == tail) start[r] = i;
            if (adj[i].size() % 2) {
                hasOdd[r] = true;
                add(i, n);
            }
        }
        for (int r = 0; r < n; ++r) if (start[r] >= 0 && !hasOdd[r]) {
            add(start[r], n);
            add(start[r], n);
        }
        vector<int> stack{n}, circuit;
        while (!stack.empty()) {
            int a = stack.back();
            while (!adj[a].empty() && used[adj[a].back()]) adj[a].pop_back();
            if (adj[a].empty()) {
                circuit.push_back(a);
                stack.pop_back();
            } else {
                int e = adj[a].back();
                used[e] = true;
                stack.push_back(ends[e][0] ^ ends[e][1] ^ a);
            }
        }
        // Rotate an Euler circuit through the current cup when possible.
        circuit.pop_back();
        auto at = find(circuit.begin(), circuit.end(), tail);
        if (at != circuit.end()) rotate(circuit.begin(), at, circuit.end());
        circuit.push_back(circuit.front());
        bool first = true;
        for (int c : circuit) {
            if (c == n) continue;
            if (first && c == tail) { first = false; continue; }
            first = false;
            if (!edges || taste(c)) break;
        }
    }
}

using Plan = vector<int>; // -1 starts a fresh segment.
struct Counts {
    int full = 0, forward = 0, diagonal = 0;
};

// Plan on short ordered groups. At separation k / width, the later
// group can still compare the triangular part with earlier rank >= later rank.
Plan blockPlan(const vector<int>& ids, int width, int trial) {
    const int m = ((int)ids.size() + width - 1) / width;
    const int radius = k / width;
    vector<Counts> counts(m * m);
    vector<int> degree(m), order(m);
    iota(order.begin(), order.end(), 0);
    if (trial) shuffle(order.begin(), order.end(), rng);
    auto pairAt = [&](int a, int b) -> Counts& { return counts[a * m + b]; };
    vector<int> groupOf(n), rankOf(n);
    for (int i = 0; i < (int)ids.size(); ++i) {
        groupOf[ids[i]] = i / width;
        rankOf[ids[i]] = i % width;
    }
    for (int v : ids) {
        int a = groupOf[v], i = rankOf[v];
        for (size_t u = unknown[v]._Find_next(v); u < M; u = unknown[v]._Find_next(u)) {
            int b = groupOf[u], j = rankOf[u];
            auto& f = pairAt(a, b);
            ++f.full;
            ++degree[a];
            if (a == b) continue;
            auto& rev = pairAt(b, a);
            ++rev.full;
            ++degree[b];
            f.forward += i >= j;
            rev.forward += j >= i;
            if (i == j) ++f.diagonal, ++rev.diagonal;
        }
    }
    deque<int> window;
    vector<bool> present(m);
    Plan plan{-1};
    int remaining = edges;
    vector<int> gain(m);
    while (remaining) {
        for (int h = 0; h < m; ++h) gain[h] = pairAt(h, h).full;
        for (int g : window) {
            bool partial = (int)window.size() == radius && g == window.front();
            for (int h = 0; h < m; ++h)
                gain[h] += partial ? pairAt(g, h).forward : pairAt(g, h).full;
        }
        int best = -1;
        for (int h : order) if (!present[h] && gain[h] > 0) {
            if (best < 0 || gain[h] > gain[best] ||
                (trial % 3 != 0 && gain[h] == gain[best] && degree[h] > degree[best]))
                best = h;
        }
        if (best < 0) {
            if (!window.empty()) {
                plan.push_back(-1);
                window.clear();
                fill(present.begin(), present.end(), false);
            }
            int bestScore = -1;
            for (int h : order) if (degree[h]) {
                int score = degree[h];
                if (radius == 1) {
                    score = pairAt(h, h).full;
                    for (int j = 0; j < m; ++j) score += pairAt(h, j).forward;
                }
                if (score > bestScore) best = h, bestScore = score;
            }
        }
        int internal = pairAt(best, best).full;
        degree[best] -= internal;
        remaining -= internal;
        pairAt(best, best) = {};
        for (int g : window) {
            Counts &f = pairAt(g, best), &rev = pairAt(best, g);
            bool partial = (int)window.size() == radius && g == window.front();
            int take = partial ? f.forward : f.full;
            degree[g] -= take;
            degree[best] -= take;
            remaining -= take;
            if (partial) {
                f.full -= take;
                rev.full -= take;
                rev.forward -= f.diagonal;
                f.diagonal = rev.diagonal = f.forward = 0;
            } else {
                f = rev = {};
            }
        }
        for (int i = best * width; i < min((int)ids.size(), (best+1)*width); ++i)
            plan.push_back(ids[i]);
        window.push_back(best);
        present[best] = true;
        if ((int)window.size() > radius) {
            present[window.front()] = false;
            window.pop_front();
        }
    }
    return plan;
}

// Trim redundant occurrences. Deleting a cup creates new pairs at the
// boundary of the window, so update the coverage counts in both directions.
int compress(Plan& plan, bool verify = false) {
    int L = plan.size();
    vector<int> left(L), right(L), order;
    vector<int> covers(n * n);
    auto key = [&](int a, int b) { return min(a,b)*n + max(a,b); };
    int prev = -1;
    for (int i = 0; i < L; ++i) {
        right[i] = -1;
        if (plan[i] < 0) { prev = -1; continue; }
        left[i] = prev;
        if (prev >= 0) right[prev] = i;
        prev = i;
        order.push_back(i);
        int j = i;
        for (int d = 1; d <= k && (j = left[j]) >= 0; ++d) {
            if (verify && plan[i] == plan[j]) return 1000000000;
            ++covers[key(plan[i], plan[j])];
        }
    }
    if (verify) {
        for (int a = 0; a < n; ++a)
            for (int b = a+1; b < n; ++b)
                if (unknown[a][b] && !covers[key(a,b)]) return 1000000000;
    }
    vector<int> before(k), after(k), local(n);
    for (int pass = 0; pass < 3; ++pass) {
        if (pass) shuffle(order.begin(), order.end(), rng);
        bool changed = false;
        for (int i : order) if (plan[i] >= 0) {
            int c = plan[i], nl = 0, nr = 0;
            bool safe = true;
            for (int j = left[i]; j >= 0 && nl < k; j = left[j]) {
                int a = plan[j];
                before[nl++] = a;
                if (unknown[c][a] && covers[key(a,c)] <= 1) { safe = false; break; }
            }
            if (!safe) continue;
            for (int t = 0; t < nl; ++t) ++local[before[t]];
            for (int j = right[i]; j >= 0 && nr < k; j = right[j]) {
                int a = plan[j];
                after[nr++] = a;
                if (unknown[c][a] && covers[key(a,c)] <= 1 + local[a]) { safe = false; break; }
            }
            for (int t = 0; t < nl; ++t) --local[before[t]];
            if (!safe) continue;
            for (int t = k - nr; t < nl; ++t)
                if (before[t] == after[k-1-t]) { safe = false; break; }
            if (!safe) continue;
            for (int t = 0; t < nl; ++t) --covers[key(c,before[t])];
            for (int t = 0; t < nr; ++t) --covers[key(c,after[t])];
            for (int t = k - nr; t < nl; ++t) ++covers[key(before[t],after[k-1-t])];
            if (left[i] >= 0) right[left[i]] = right[i];
            if (right[i] >= 0) left[right[i]] = left[i];
            plan[i] = -2;
            changed = true;
        }
        if (!changed) break;
    }
    plan.erase(remove(plan.begin(), plan.end(), -2), plan.end());
    return count_if(plan.begin(), plan.end(), [](int x) { return x >= 0; });
}

struct Schedule {
    int groups, shift, wrap;
    vector<int> word;
};

Plan expand(const Schedule& s) {
    const int m = s.groups;
    Plan p;
    auto group = [&](int g) {
        for (int i = g*n/m; i < (g+1)*n/m; ++i) p.push_back(i);
    };
    if (s.shift < 0) {
        // Each stride is a union of disjoint cycles of groups.
        for (int stride : s.word) {
            int cycles = gcd(stride, m), length = m/cycles;
            for (int g = 0; g < cycles; ++g) {
                p.push_back(-1);
                int begin = p.size();
                for (int j = 0; j < length; ++j) group((g+j*stride)%m);
                if ((int)p.size() - begin > k+1)
                    for (int j = 0; j < s.wrap; ++j) group((g+j*stride)%m);
            }
        }
    } else if (s.shift == 0) {
        for (int g : s.word) group(g);
    } else {
        // Translate a short word until it returns to its initial offset.
        int period = s.word.size();
        int length = period * (m/gcd(m,s.shift));
        for (int j = 0; j < length+s.wrap; ++j)
            group((s.word[j%period] + (j/period)*s.shift)%m);
    }
    return p;
}

// Short covering words for the allowed power-of-two geometries. Their
// expansions are checked for full pair coverage before being used.
Plan initialPlan() {
    static const vector<Schedule> schedules = {
        {8, 4, 2, {2, 6, 7, 1, 3, 4, 0, 2, 1, 5, 7}},
        {17, -1, 2, {3, 1, 5, 4}},
        {16, 8, 4, {6, 12, 2, 11, 14, 0, 7, 3, 12, 8, 15, 1, 4, 2, 6, 13, 5, 0, 7, 11, 1, 9, 2}},
        {32, 1, 2, {22, 1, 4, 13, 8, 15, 0, 16, 10}},
        {32, 1, 4, {13, 8, 5, 12, 24}},
        {16, 0, 0, {4, 12, 14, 6, 1, 9, 2, 13, 11, 8, 3, 1, 15, 7, 5, 4, 13, 0, 9, 12, 2, 7, 5, 10, 8, 14, 13, 6, 0, 7, 15, 11, 14, 2, 8, 3, 4, 12, 11, 10, 15, 5, 6, 9, 3, 8, 0, 10, 1}},
        {65, -1, 2, {3, 2, 23, 18, 12, 13, 28, 32, 8, 5, 7, 17, 15, 20, 22, 27}},
        {64, 1, 4, {27, 48, 30, 31, 4, 23, 59, 47, 17}},
        {16, 0, 0, {7, 10, 0, 3, 14, 4, 13, 7, 1, 11, 15, 8, 14, 4, 2, 6, 10, 8, 15, 3, 13, 9, 11, 10, 12, 1, 5, 4, 0, 9, 15, 8, 12, 14, 5, 7, 9, 2, 6, 12, 3, 13, 1, 5, 0, 2, 6, 11}},
        {130, -1, 2, {3, 9, 1, 29, 41, 11, 19, 13, 51, 49, 35, 57, 37, 39, 33, 65, 27, 15, 5, 55, 43, 31, 7, 25, 59, 63, 17, 47, 23, 21, 45, 61, 53}},
        {131, -1, 4, {17, 10, 33, 28, 42, 7, 13, 53, 60, 24, 43, 9, 29, 50, 64, 54, 19, 4}},
        {64, -1, 4, {11, 1, 47, 57, 29, 5, 55, 45, 51, 40}},
        {272, -1, 8, {99, 17, 29, 77, 83, 67, 19, 93, 32, 115, 13, 3, 131, 45, 125, 109, 61, 35}},
    };
    static const int choice[][3] = {
        {8, 2, 0},
        {16, 2, 1},
        {16, 4, 2},
        {32, 2, 3},
        {32, 4, 4},
        {32, 8, 5},
        {64, 2, 6},
        {64, 4, 7},
        {64, 8, 4},
        {64, 16, 8},
        {128, 2, 9},
        {128, 4, 10},
        {128, 8, 11},
        {128, 16, 4},
        {128, 32, 8},
        {256, 8, 12},
        {256, 16, 11},
        {256, 32, 4},
        {256, 64, 8},
        {512, 32, 11},
        {512, 64, 4},
        {512, 128, 8},
        {1024, 128, 4},
        {1024, 256, 8},
    };
    for (const auto& row : choice) if (row[0] == n && row[1] == k) {
        Plan p = expand(schedules[row[2]]);
        if (compress(p, true) >= 1000000000) break;
        // Give cafés consecutive labels in order of their first tasting.
        vector<int> label(n, -1);
        int assigned = 0;
        for (int& c : p) if (c >= 0) {
            if (label[c] < 0) label[c] = assigned++;
            c = label[c];
        }
        return p;
    }
    Plan p(n);
    iota(p.begin(), p.end(), 0);
    return p;
}

Plan makePlan() {
    vector<int> ids;
    for (int i = 0; i < n; ++i) if (unknown[i].any()) ids.push_back(i);
    if ((int)ids.size() <= k+1) {
        Plan p{-1};
        p.insert(p.end(), ids.begin(), ids.end());
        return p;
    }
    int best = 1e9;
    Plan result;
    for (int width = k; width >= 1; width /= 2) {
        int groups = (ids.size()+width-1)/width;
        if (groups > 200 || (k/width > 16 && groups > 48)) break;
        for (int trial = 0; trial < 3; ++trial) {
            Plan p = blockPlan(ids, width, trial);
            int cost = compress(p);
            if (cost < best) best = cost, result = move(p);
        }
    }
    return result;
}

// Spend the current memory on comparisons before starting a new schedule.
void extendTail() {
    for (int step = 0; step < k && edges; ++step) {
        Mask singles, blocked;
        for (const Entry& e : history) {
            blocked |= e.possible;
            if (e.single >= 0) singles.set(e.single);
        }
        int best = -1, gain = 0;
        for (int i = 0; i < n; ++i) if (!blocked[i]) {
            int score = (unknown[i] & singles).count();
            if (score > gain) gain = score, best = i;
        }
        if (!gain) return;
        taste(best);
    }
}

// One fresh segment per pair of half-memory groups. All its indices are
// different, so every surviving pair in it receives a negative comparison.
void finishInGroups() {
    vector<int> ids;
    for (int i = 0; i < n; ++i) if (unknown[i].any()) ids.push_back(i);
    int size = ids.size(), width = k/2;
    for (int a = 0; a < size && edges; a += width)
        for (int b = a; b < size && edges; b += width) {
            resetMemory();
            for (int i = a; i < min(a+width,size); ++i)
                if (unknown[ids[i]].any()) taste(ids[i]);
            if (b != a)
                for (int i = b; i < min(b+width,size); ++i)
                    if (unknown[ids[i]].any()) taste(ids[i]);
        }
}

bool inMemory(int c) {
    for (const Entry& e : history) if (e.single == c) return true;
    return false;
}

void solveMany() {
    Plan opening = initialPlan();
    Mask seen;
    bool twoHalves = n == 2*k && n >= 16;
    int visited = 0, removed = 0;
    for (int c : opening) {
        if (!edges) break;
        if (c < 0) { resetMemory(); continue; }
        if (twoHalves && c == n-4) {
            // The first four representatives were the earliest to expire.
            // When the second half has supplied no new representative, a
            // short fresh segment can certify its final orbit directly.
            bool allMatched = true;
            for (int j = k; j < c; ++j) allMatched &= !live[j];
            bool missing = false;
            for (int j = 0; j < 4; ++j) missing |= live[j] && !inMemory(j);
            if (allMatched && missing) {
                resetMemory();
                for (int j = 0; j < 4; ++j) if (live[j]) taste(j);
            }
        }
        if (!unknown[c].any() || inMemory(c)) continue;
        if (!seen[c]) { seen.set(c); ++visited; }
        if (taste(c) && ++removed >= max(2, visited/9) && !twoHalves) break;
    }
    for (int i = 0; i < n && edges; ++i)
        if (!seen[i] && unknown[i].any()) taste(i);

    while (edges) {
        infer();
        if (!edges) break;
        extendTail();
        if (!edges) break;
        if (resetsUsed >= 12000) { finishInGroups(); break; }
        Plan plan = makePlan();
        int initial = live.count(), lost = 0, oldEdges = edges;
        for (int c : plan) {
            if (c < 0) {
                if (resetsUsed >= 12000) break;
                resetMemory();
                continue;
            }
            if (unknown[c].none() || inMemory(c)) continue;
            if (taste(c)) ++lost;
            if (!edges || lost >= max(3, initial/5)) break;
        }
        if (edges == oldEdges) { finishInGroups(); break; }
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    if (!(cin >> n >> k)) return 0;
    iota(cafeIndex.begin(), cafeIndex.end(), 0);
    // Keep small stretches together and bring corresponding stretches of
    // both halves close in time. A permutation preserves the covering plan.
    if (n == 2*k && n >= 16) {
        // The XOR operations pair adjacent cafés, opposite halves, and
        // mirrored positions. Split their eight-element orbits evenly.
        // Starting the second sweep one orbit ahead keeps all four possible
        // partners in memory for every orbit except the final one.
        Mask placed;
        int groups = 0;
        for (int x = 0; x < n; ++x) if (!placed[x]) {
            int opposite = k, mirror = n-1;
            int left = 4*groups, right = k + (left+k-4)%k;
            for (int v : {x, x^1^opposite, x^1^mirror, x^opposite^mirror}) {
                placed.set(v);
                cafeIndex[left++] = v;
            }
            for (int v : {x^1, x^opposite, x^mirror, x^1^opposite^mirror}) {
                placed.set(v);
                cafeIndex[right++] = v;
            }
            ++groups;
        }
    } else if (k >= 2 && n >= 8) {
        int width = max(2, k/8), blocks = n/width, next = 0;
        for (int g = 0; g < blocks/4; ++g) {
            for (int b : {g, g+blocks/2, blocks-1-g, blocks/2-1-g})
                for (int j = 0; j < width; ++j) cafeIndex[next++] = b*width+j;
        }
    }
    for (int i = 0; i < n; ++i) live.set(i);
    for (int i = 0; i < n; ++i) { unknown[i] = live; unknown[i].reset(i); }
    iota(parent.begin(), parent.end(), 0);
    edges = n * (n-1) / 2;
    if (k == 1) solveOne(); else solveMany();
    cout << "! " << live.count() << endl;
}

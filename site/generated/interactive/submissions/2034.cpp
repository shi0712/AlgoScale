#include <algorithm>
#include <bitset>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <initializer_list>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

using namespace std;

constexpr int MAX_N = 1024;
using Mask = bitset<MAX_N>;

struct Graph {
    vector<Mask> unknown;
    vector<int> degree;
    Mask alive;
    int edges = 0;

    explicit Graph(int n) : unknown(n), degree(n, n - 1), edges(n * (n - 1) / 2) {
        for (int i = 0; i < n; ++i) alive.set(i);
        for (int i = 0; i < n; ++i) {
            unknown[i] = alive;
            unknown[i].reset(i);
        }
    }

    void separate(int v, const Mask& others) {
        Mask change = unknown[v] & others;
        unknown[v] ^= change;
        for (size_t u = change._Find_first(); u < MAX_N; u = change._Find_next(u)) {
            unknown[u].reset(v);
            --degree[u];
            --degree[v];
            --edges;
        }
    }

    void remove(int v, const Mask& matches) {
        // If the match is unambiguous, its representative inherits the
        // inequalities that were already known for v.
        if (matches.count() == 1) {
            int u = matches._Find_first();
            Mask inherited = unknown[u] & ~unknown[v];
            inherited.reset(v);
            separate(u, inherited);
        }
        separate(v, unknown[v]);
        alive.reset(v);
    }
};

struct Memory {
    struct Slot {
        Mask candidates;
        int exact = -1;
    };

    int capacity;
    deque<Slot> slots;
    Mask direct, possible;

    explicit Memory(int k) : capacity(k) {}

    void clear() {
        slots.clear();
        direct.reset();
        possible.reset();
    }

    void exclude(int v) {
        if (!possible.test(v)) return;
        for (Slot& slot : slots) {
            if (slot.exact != -1 || !slot.candidates.test(v)) continue;
            slot.candidates.reset(v);
            if (slot.candidates.count() == 1) {
                slot.exact = slot.candidates._Find_first();
                direct.set(slot.exact);
            }
        }
    }

    void push(const Mask& candidates, int exact = -1) {
        if (slots.size() == static_cast<size_t>(capacity)) slots.pop_front();
        slots.push_back({candidates, exact});
        direct.reset();
        possible.reset();
        for (const Slot& slot : slots) {
            if (slot.exact == -1) possible |= slot.candidates;
            else direct.set(slot.exact);
        }
        possible |= direct;
    }

    void pushExact(int v) {
        Mask exact;
        exact.set(v);
        push(exact, v);
    }
};

class Solver {
    int n, k;
    Graph graph;
    Memory memory;
    int resets = 0;
    int tastings = 0;

    void reset() {
        if (memory.slots.empty()) return;
        cout << 'R' << endl;
        ++resets;
        memory.clear();
    }

    bool taste(int v) {
        cout << "? " << v + 1 << endl;
        ++tastings;
        char response;
        if (!(cin >> response) || (response != 'N' && response != 'Y')) exit(0);

        if (response == 'N') {
            memory.exclude(v);
            graph.separate(v, memory.direct);
            memory.pushExact(v);
            return true;
        }

        // If a previous slot might already represent v, a yes need not
        // certify that another live candidate has this variety.
        if (memory.possible.test(v)) {
            memory.pushExact(v);
            return true;
        }

        // Every old slot is represented among the live candidates. A yes
        // therefore leaves another live candidate of the same variety.
        Mask matches = graph.unknown[v] & memory.possible;
        graph.remove(v, matches);
        if (matches.count() == 1) memory.pushExact(matches._Find_first());
        else memory.push(matches);
        return false;
    }

    vector<vector<int>> eulerTrails() const {
        // Odd-degree vertices are joined to an extra vertex. Breaking its
        // Euler tour there gives the minimum number of trails.
        vector<Mask> remaining = graph.unknown;
        Mask odd;
        for (int v = 0; v < n; ++v)
            if (graph.degree[v] % 2 != 0) odd.set(v);
        vector<vector<int>> paths;

        auto extract = [&](int start) {
            vector<int> stack{start}, path;
            while (!stack.empty()) {
                int v = stack.back();
                int u = (v == n ? odd : remaining[v])._Find_first();
                if (u < n) {
                    if (v == n) {
                        odd.reset(u);
                    } else {
                        remaining[v].reset(u);
                        remaining[u].reset(v);
                    }
                    stack.push_back(u);
                } else if (v != n && odd.test(v)) {
                    odd.reset(v);
                    stack.push_back(n);
                } else {
                    path.push_back(v);
                    stack.pop_back();
                }
            }
            reverse(path.begin(), path.end());
            return path;
        };

        if (odd.any()) {
            vector<int> path;
            for (int v : extract(n)) {
                if (v != n) path.push_back(v);
                else if (!path.empty()) {
                    paths.push_back(move(path));
                    path.clear();
                }
            }
        }

        int last = memory.direct._Find_first();
        if (last < n && remaining[last].any()) paths.push_back(extract(last));
        for (int v = 0; v < n; ++v)
            if (remaining[v].any()) paths.push_back(extract(v));

        if (last == MAX_N) return paths;
        for (auto& path : paths) {
            if (path.back() == last) reverse(path.begin(), path.end());
            if (path.front() == last) return paths;
        }
        for (auto& path : paths) {
            auto it = find(path.begin(), path.end(), last);
            if (it != path.end()) {
                vector<int> suffix(it, path.end());
                path.erase(it + 1, path.end());
                paths.push_back(move(suffix));
                break;
            }
        }
        return paths;
    }

    void finishPairs() {
        while (graph.edges) {
            vector<vector<int>> paths = eulerTrails();
            int last = memory.direct._Find_first();
            auto it = find_if(paths.begin(), paths.end(), [&](const vector<int>& path) {
                return path.front() == last;
            });
            if (it != paths.end()) iter_swap(paths.begin(), it);

            bool changed = false;
            for (const auto& path : paths) {
                for (int v : path) {
                    if (memory.direct.test(v)) continue;
                    if (!taste(v)) {
                        changed = true;
                        break;
                    }
                }
                if (changed) break;
            }
        }
    }

    // A cycle followed by a short prefix covers every pair at circular
    // distance at most k. Dummies in the modulus can be omitted.
    vector<vector<int>> modularBatches(int modulus, const vector<int>& strides) const {
        vector<vector<int>> batches;
        for (int stride : strides) {
            vector<bool> visited(modulus);
            for (int start = 0; start < modulus; ++start) {
                if (visited[start]) continue;
                vector<int> cycle;
                for (int v = start; !visited[v]; v = (v + stride) % modulus) {
                    visited[v] = true;
                    if (v < n) cycle.push_back(v);
                }
                int count = cycle.size();
                if (count < 2) continue;
                int extra = clamp(count - k - 1, 0, k);
                for (int i = 0; i < extra; ++i) cycle.push_back(cycle[i]);
                batches.push_back(move(cycle));
            }
        }
        return batches;
    }

    vector<vector<int>> densePlan() const;
    vector<vector<int>> interleavedBatches(int groups, bool spaced, bool split) const;

    void compact(vector<vector<int>>& batches) const;

    void playDensePlan() {
        if (graph.alive.count() * 5 < static_cast<size_t>(n) * 4) return;
        auto batches = densePlan();
        if (n > 2 * k && graph.alive.count() == static_cast<size_t>(n)) compact(batches);
        bool first = true;
        for (const auto& batch : batches) {
            if (!graph.edges || graph.alive.count() * 5 < static_cast<size_t>(n) * 4) return;
            int start = first ? n : 0;
            first = false;
            if (start >= static_cast<int>(batch.size())) continue;

            if (start == 0) {
                // A batch which no longer has a useful pair can be ignored.
                Mask recent;
                deque<int> previous;
                bool useful = false;
                for (int v : batch) {
                    if (!graph.alive.test(v)) continue;
                    if ((recent & graph.unknown[v]).any()) {
                        useful = true;
                        break;
                    }
                    previous.push_back(v);
                    recent.set(v);
                    if (previous.size() > static_cast<size_t>(k)) {
                        recent.reset(previous.front());
                        previous.pop_front();
                    }
                }
                if (!useful) continue;
            }

            // Keep the old memory if the new batch starts with different
            // representatives. Extra comparisons across the seam are free.
            if (start == 0) {
                int offset = 0;
                for (int v : batch) {
                    if (!graph.alive.test(v)) continue;
                    if (offset == k) break;
                    if (memory.direct.test(v)) {
                        bool conflict = false;
                        for (int j = max(0, static_cast<int>(memory.slots.size()) - k + offset);
                             j < static_cast<int>(memory.slots.size()); ++j) {
                            if (memory.slots[j].exact == v) {
                                conflict = true;
                                break;
                            }
                        }
                        if (conflict) {
                            reset();
                            break;
                        }
                    }
                    ++offset;
                }
            }

            for (int i = start; i < static_cast<int>(batch.size()) && graph.edges; ++i) {
                int v = batch[i];
                if (!graph.alive.test(v) || !graph.degree[v] || memory.direct.test(v)) continue;
                if ((graph.unknown[v] & memory.direct).none()) {
                    // A tasting can also load v for an upcoming comparison.
                    bool needed = false;
                    int look = 0;
                    for (int j = i + 1; j < static_cast<int>(batch.size()) && look < k; ++j) {
                        int u = batch[j];
                        if (!graph.degree[u]) continue;
                        if (u == v) break;
                        ++look;
                        if (graph.unknown[v].test(u)) {
                            needed = true;
                            break;
                        }
                    }
                    if (!needed) continue;
                }
                taste(v);
            }
        }
    }

    void finishGreedy() {
        const int cutoff = tastings + 2 * n * n / k + 2 * n;
        while (graph.edges && resets < 18000 && tastings < cutoff) {
            int best = -1, most = 0;
            Mask eligible = graph.alive & ~memory.direct;
            for (size_t v = eligible._Find_first(); v < MAX_N; v = eligible._Find_next(v)) {
                if (!graph.degree[v]) continue;
                int gain = (graph.unknown[v] & memory.direct).count();
                gain *= memory.possible.test(v) ? 1 : 2;
                if (gain > most || (gain > 0 && gain == most && graph.degree[v] < graph.degree[best])) {
                    most = gain;
                    best = v;
                }
            }
            if (best == -1) {
                reset();
                best = max_element(graph.degree.begin(), graph.degree.end()) - graph.degree.begin();
            }
            taste(best);
        }
    }

    void finishBatches() {
        // Every batch can compare all its pairs in a clean window. Its
        // boundaries also bound the number of resets in the fallback.
        int size = k / 2;
        for (int a = 0; a < n && graph.edges; a += size) {
            for (int b = a; b < n && graph.edges; b += size) {
                Mask group;
                for (int v = a; v < min(n, a + size); ++v)
                    if (graph.alive.test(v)) group.set(v);
                for (int v = b; v < min(n, b + size); ++v)
                    if (graph.alive.test(v)) group.set(v);

                vector<int> batch;
                for (size_t v = group._Find_first(); v < MAX_N; v = group._Find_next(v)) {
                    if ((graph.unknown[v] & group).any()) batch.push_back(v);
                }
                if (batch.empty()) continue;
                reset();
                for (int v : batch)
                    if ((graph.unknown[v] & group).any()) taste(v);
            }
        }
    }

public:
    Solver(int n, int k) : n(n), k(k), graph(n), memory(k) {}

    void run() {
        for (int v = 0; v < n && graph.edges; ++v) taste(v);
        if (k == 1) {
            finishPairs();
        } else if (graph.edges) {
            playDensePlan();
            finishGreedy();
            if (graph.edges) finishBatches();
        }
        cout << "! " << graph.alive.count() << endl;
    }
};

void Solver::compact(vector<vector<int>>& batches) const {
    // Drop repeated tastings whose comparisons are supplied elsewhere.
    // The first scan has already been played and stays fixed.
    if (batches.empty()) return;
    struct Node {
        int vertex, prev, next;
    };
    vector<Node> nodes;
    vector<int> heads;
    vector<uint16_t> witnesses(n * n);
    auto edge = [&](int u, int v) { return min(u, v) * n + max(u, v); };
    for (const auto& batch : batches) {
        int head = nodes.size();
        heads.push_back(head);
        for (int i = 0; i < static_cast<int>(batch.size()); ++i) {
            int v = batch[i];
            nodes.push_back({v, i ? head + i - 1 : -1,
                             i + 1 < static_cast<int>(batch.size()) ? head + i + 1 : -1});
            for (int j = max(0, i - k); j < i; ++j) {
                int u = batch[j];
                if (graph.unknown[v].test(u)) ++witnesses[edge(v, u)];
            }
        }
    }

    vector<int> left, right, added, removed;
    left.reserve(k);
    right.reserve(k);
    added.reserve(k);
    removed.reserve(2 * k);
    auto eraseIfRedundant = [&](int index) {
        Node& node = nodes[index];
        int v = node.vertex;
        if (v == -1) return false;
        left.clear();
        right.clear();
        for (int i = node.prev; i != -1 && static_cast<int>(left.size()) < k; i = nodes[i].prev)
            left.push_back(nodes[i].vertex);
        for (int i = node.next; i != -1 && static_cast<int>(right.size()) < k; i = nodes[i].next)
            right.push_back(nodes[i].vertex);

        added.clear();
        for (int i = max(0, k - static_cast<int>(right.size())); i < static_cast<int>(left.size()); ++i) {
            int a = left[i], b = right[k - 1 - i];
            if (a == b) return false;
            if (graph.unknown[a].test(b)) added.push_back(edge(a, b));
        }
        removed.clear();
        for (int u : left)
            if (graph.unknown[v].test(u)) removed.push_back(edge(v, u));
        for (int u : right)
            if (graph.unknown[v].test(u)) removed.push_back(edge(v, u));

        for (int id : added) ++witnesses[id];
        bool keep = false;
        for (int id : removed)
            if (--witnesses[id] == 0) keep = true;
        if (keep) {
            for (int id : removed) ++witnesses[id];
            for (int id : added) --witnesses[id];
            return false;
        }

        if (node.prev != -1) nodes[node.prev].next = node.next;
        if (node.next != -1) nodes[node.next].prev = node.prev;
        node.vertex = -1;
        return true;
    };

    for (int pass = 0; pass < 6; ++pass) {
        bool changed = false;
        for (int b = 0; b < static_cast<int>(batches.size()); ++b) {
            int begin = heads[b];
            int end = begin + batches[b].size();
            if (b == 0) begin += n;
            int first = pass % 2 ? begin : end - 1;
            int step = pass % 2 ? 1 : -1;
            for (int i = first; i >= begin && i < end; i += step) changed |= eraseIfRedundant(i);
        }
        if (!changed) break;
    }

    for (int i = 0; i < static_cast<int>(batches.size()); ++i) {
        int head = heads[i];
        int end = head + batches[i].size();
        batches[i].clear();
        for (int j = head; j < end; ++j)
            if (nodes[j].vertex != -1) batches[i].push_back(nodes[j].vertex);
    }
}

// Multiplication in GF(2^m), for m = 2, 3 or 4.
static int fieldMultiply(int a, int b, int order) {
    const int polynomial = order == 4 ? 7 : order == 8 ? 11 : 19;
    int result = 0;
    while (b) {
        if (b & 1) result ^= a;
        b >>= 1;
        a <<= 1;
        if (a & order) a ^= polynomial;
    }
    return result;
}

vector<vector<int>> Solver::interleavedBatches(int groups, bool spaced, bool split) const {
    const int order = n / (2 * k);
    const int width = 2 * k / groups;
    vector<int> high{0}, rank(order), coefficient(groups);
    for (int t = 1; t != 0 && high.size() < static_cast<size_t>(order); t = fieldMultiply(t, 2, order))
        high.push_back(t);
    for (int i = 0; i < order; ++i) rank[high[i]] = i;
    for (int g = 0; g < groups; ++g)
        coefficient[g] = spaced && g != 0 ? high[2 * g - 1] : g;

    vector<vector<int>> batches;
    for (int r = 0; r < order; ++r) {
        // Interleave the groups, shifting each group's high indices by a
        // different field multiple of r. This varies the matchings between
        // groups without repeatedly comparing members within one group.
        vector<int> sequence;
        for (int i = 0; i < order * width; ++i) {
            if (split && r != 0 && i == width) {
                for (int j = 0; j < k - 1; ++j) sequence.push_back(sequence[j]);
                batches.push_back(move(sequence));
                sequence.clear();
            }
            for (int g = 0; g < groups; ++g) {
                int t = high[i / width] ^ fieldMultiply(r, coefficient[g], order);
                sequence.push_back((rank[t] * width + i % width) * groups + g);
            }
        }
        if (split)
            for (int j = 0; j < k; ++j) sequence.push_back(sequence[j]);
        batches.push_back(move(sequence));
    }
    return batches;
}

vector<vector<int>> Solver::densePlan() const {
    if (n <= 2 * k) return modularBatches(n, {1});
    if (n == 64 && k == 4) return interleavedBatches(4, false, false);
    if (n == 128 && k == 8) return interleavedBatches(8, false, true);
    if (n == 256 && k == 8) return interleavedBatches(8, true, true);
    if (n / k == 16 && k >= 8) return interleavedBatches(8, false, true);

    // Each symbol represents a block. With width k/2, adjacent blocks
    // cover all their cross pairs. A block two positions away covers one
    // triangle of cross pairs, so both directions can complete the pair.
    auto blocks = [&](int width, initializer_list<int> tail) {
        vector<int> sequence(n);
        iota(sequence.begin(), sequence.end(), 0);
        sequence.reserve(n + width * tail.size());
        for (int b : tail)
            for (int i = 0; i < width; ++i) sequence.push_back(width * b + i);
        return vector<vector<int>>{move(sequence)};
    };
    if (n == 8 && k == 2)
        return blocks(1, {
            1, 4, 5, 0, 2, 7, 6, 3, 0,
        });
    if (n == 16 && k == 2)
        return blocks(1, {
            9, 6, 3, 10, 13, 2, 8, 12, 1, 15, 5, 2, 0, 7, 8, 4, 14, 11, 7, 1, 13, 6, 4, 12,
            0, 9, 13, 5, 11, 8, 15, 3, 0, 11, 6, 2, 14, 9, 1, 4, 10, 15, 7, 12, 3, 5, 14, 10,
            0,
        });
    if (n == 16 && k == 4)
        return blocks(1, {
            6, 7, 2, 0, 1, 12, 10, 5, 4, 3, 15, 14, 9, 8, 1, 0, 2, 13, 11, 5, 3, 4, 6,
        });
    if (n == 32 && k == 2)
        return blocks(1, {
            19, 26, 11, 30, 0, 18, 28, 3, 15, 26, 29, 4, 13, 18, 7, 25, 20, 0, 29, 6, 11, 27, 18, 12,
            26, 24, 0, 15, 12, 7, 23, 26, 16, 22, 13, 17, 9, 12, 3, 20, 13, 24, 10, 4, 8, 12, 21, 31,
            13, 1, 27, 14, 21, 9, 15, 2, 19, 8, 1, 25, 28, 11, 8, 23, 27, 15, 4, 30, 13, 6, 8, 15,
            20, 11, 4, 13, 25, 11, 19, 22, 9, 28, 4, 17, 11, 20, 2, 27, 22, 9, 30, 25, 16, 31, 4, 23,
            14, 3, 11, 21, 24, 30, 8, 26, 16, 9, 1, 18, 29, 24, 19, 7, 27, 3, 17, 30, 7, 2, 10, 23,
            20, 28, 26, 6, 2, 31, 1, 17, 24, 14, 26, 10, 17, 5, 31, 20, 11, 16, 21, 2, 18, 8, 22, 4,
            1, 19, 12, 2, 28, 13, 19, 23, 18, 6, 14, 20, 30, 1, 23, 9, 29, 10, 16, 28, 5, 14, 22, 8,
            17, 0, 23, 8, 5, 31, 0, 20, 9, 24, 5, 2, 29, 17, 21, 10, 25, 15, 22, 10, 27, 30, 5, 12,
            25, 2, 14, 7, 29, 22, 12, 6, 16, 19, 3, 10, 6, 1, 21, 26, 5, 13, 19, 0, 14, 10, 31, 18,
            15, 5, 1, 11, 7, 0, 16, 27, 24, 31, 28, 7, 21, 4, 0, 22, 3, 31, 24, 9, 6, 17, 25, 29,
            3, 8,
        });
    if (n == 32 && k == 4)
        return blocks(1, {
            16, 7, 23, 22, 1, 10, 17, 27, 20, 5, 8, 2, 28, 23, 22, 14, 4, 9, 12, 18, 24, 19, 2, 7,
            4, 26, 17, 6, 18, 28, 3, 27, 16, 14, 8, 6, 24, 1, 19, 11, 25, 18, 30, 13, 8, 31, 0, 15,
            25, 6, 12, 29, 5, 8, 1, 21, 16, 26, 28, 10, 9, 15, 30, 24, 1, 5, 14, 13, 31, 19, 26, 3,
            10, 29, 23, 11, 18, 22, 0, 20, 5, 26, 21, 11, 12, 19, 27, 28, 7, 0, 13, 29, 10, 16, 4, 2,
            25, 11, 30, 17, 31, 12, 3, 20, 24, 9, 17, 0, 29, 25, 14, 20, 7, 3, 21, 15, 12, 22, 30, 23,
            6, 13, 20, 31, 21, 2, 4, 9, 15, 27,
        });
    if (n == 32 && k == 8)
        return blocks(1, {
            8, 10, 19, 22, 21, 9, 3, 0, 20, 12, 23, 31, 24, 11, 13, 6, 2, 1, 7, 21, 22, 5, 4, 19,
            28, 20, 29, 30, 11, 27, 9, 12, 13, 25, 14, 26, 18, 17, 7, 3, 2, 0, 28, 29, 30, 27, 16, 15,
            26, 25, 6, 1, 4, 5, 31, 24, 18, 10, 17, 14, 23, 8,
        });
    if (n / k == 4)
        return blocks(k / 2, {
            0, 4, 7, 1, 5, 4, 6, 2, 5, 0, 3, 6, 1, 3, 7, 2,
        });
    if (n / k == 8)
        return blocks(k / 2, {
            10, 4, 12, 2, 7, 9, 5, 13, 8, 4, 11, 0, 7, 14, 12, 15, 2, 13, 6, 0, 15, 5, 12, 0,
            9, 4, 7, 15, 9, 6, 3, 11, 0, 10, 13, 3, 9, 1, 12, 10, 6, 14, 4, 1, 13, 7, 11, 15,
            8, 1, 5, 15, 3, 7, 10, 1, 5, 11, 14, 9, 2, 11, 1, 6, 14, 2, 5, 10, 2, 8, 0, 14,
            3, 8, 12, 6, 3,
        });

    struct Progression {
        int n, k, modulus;
        vector<int> strides;
    };

    // A collection of strides whose first k multiples cover the circular
    // distances. The order starts with the ordinary scan used above.
    static const Progression progressions[] = {
        {64, 2, 65, {1, 18, 23, 7, 27, 6, 10, 28, 25, 22, 30, 26, 4, 31, 24, 16}},
        {128, 2, 131, {1, 55, 16, 25, 4, 52, 6, 31, 37, 60, 54, 33, 9, 43, 24, 46, 64,
                       10, 17, 36, 42, 41, 22, 13, 40, 7, 29, 15, 35, 63, 53, 19, 28}},
        {128, 4, 131, {1, 50, 38, 14, 15, 63, 35, 46, 40, 6, 22, 47, 52, 16, 41, 36, 10, 34}},
    };
    for (const auto& p : progressions)
        if (p.n == n && p.k == k) return modularBatches(p.modulus, p.strides);
    return {};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, k;
    if (!(cin >> n >> k)) return 0;
    Solver(n, k).run();
}

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <numeric>
#include <utility>
#include <vector>

using namespace std;

struct Sample {
    int id;
    int stamp;
};

class Belt {
public:
    deque<Sample> samples;
    vector<int> present;
    int distinct = 0;

    explicit Belt(int n) : present(n + 1, -1) {}

    pair<Sample, bool> add(int id) {
        assert(present[id] == -1);
        int old = distinct;
        cout << "+ " << id << endl;
        readCount();
        Sample sample{id, nextStamp++};
        samples.push_back(sample);
        present[id] = sample.stamp;
        return {sample, distinct != old};
    }

    bool remove() {
        int old = distinct;
        cout << '-' << endl;
        readCount();
        present[samples.front().id] = -1;
        samples.pop_front();
        return distinct != old;
    }

    bool removeThrough(int stamp) {
        while (samples.front().stamp < stamp) remove();
        assert(samples.front().stamp == stamp);
        return remove();
    }

    void clear() {
        while (!samples.empty()) remove();
    }

private:
    int nextStamp = 0;

    void readCount() {
        if (!(cin >> distinct) || distinct < 0) exit(0);
    }
};

class Groups {
public:
    explicit Groups(int n) : parent(n + 1), sizes(n + 1, 1) {
        iota(parent.begin(), parent.end(), 0);
    }

    int root(int id) {
        if (id != parent[id]) parent[id] = root(parent[id]);
        return parent[id];
    }

    void join(int a, int b) {
        a = root(a);
        b = root(b);
        if (a == b) return;
        if (sizes[a] < sizes[b]) swap(a, b);
        parent[b] = a;
        sizes[a] += sizes[b];
    }

    int size(int id) { return sizes[root(id)]; }

private:
    vector<int> parent, sizes;
};

class Solver {
    struct Task {
        vector<Sample> active;
        vector<int> candidates;
    };

    struct Occurrence {
        Sample sample;
        bool first;
        bool last = true;
    };

    struct Span {
        int representative;
        int first;
        int last;
    };

    struct Interior {
        int id;
        vector<int> candidates;
    };

    class Lookup {
    public:
        vector<int> representatives;

        Lookup(Solver& solver, vector<int> reps)
            : solver(solver), label(solver.n + 1, -1), copies(solver.n + 1) {
            representatives = move(reps);
            vector<int> rootLabel(solver.n + 1, -1);
            for (int i = 0; i < (int) representatives.size(); ++i) {
                int id = representatives[i];
                label[id] = rootLabel[solver.groups.root(id)] = i;
            }
            for (Sample sample : solver.belt.samples) {
                int id = sample.id;
                label[id] = rootLabel[solver.groups.root(id)];
                assert(label[id] >= 0);
                ++copies[label[id]];
            }
        }

        void trim(int limit, const vector<int>& relevant) {
            int occupied = count_if(relevant.begin(), relevant.end(),
                                    [&](int c) { return copies[c] > 0; });
            while (occupied > limit) {
                int c = take().first;
                if (copies[c] == 0 && binary_search(relevant.begin(), relevant.end(), c)) {
                    --occupied;
                }
            }
        }

        bool classify(int id, const vector<int>& candidates, bool allowNew) {
            if (!allowNew && candidates.size() == 1) {
                solver.groups.join(id, representatives[candidates.front()]);
                return false;
            }

            bool fresh = solver.belt.add(id).second;
            vector<int> possible;
            for (int c : candidates) {
                if ((copies[c] == 0) == fresh) possible.push_back(c);
            }
            if (fresh && allowNew) possible.push_back(-1);
            assert(!possible.empty());

            int chosen = possible.back();
            if (fresh && possible.size() > 1) {
                stable_sort(possible.begin(), possible.end(), [&](int a, int b) {
                    int sa = a < 0 ? 0 : solver.groups.size(representatives[a]);
                    int sb = b < 0 ? 0 : solver.groups.size(representatives[b]);
                    return sa > sb;
                });
                chosen = possible.back();
                for (int i = 0; i + 1 < (int) possible.size(); ++i) {
                    int c = possible[i];
                    bool grew = solver.belt.add(representatives[c]).second;
                    ++copies[c];
                    if (!grew) {
                        chosen = c;
                        break;
                    }
                }
            } else if (!fresh && possible.size() > 1) {
                vector<bool> remaining(representatives.size(), false);
                for (int c : possible) remaining[c] = true;
                int left = (int) possible.size();
                while (left > 1) {
                    auto [c, dropped] = take();
                    if (copies[c] != 0 || !remaining[c]) continue;
                    if (!dropped) {
                        chosen = c;
                        break;
                    }
                    remaining[c] = false;
                    --left;
                }
                if (left == 1) {
                    for (int c : possible) if (remaining[c]) chosen = c;
                }
            }

            bool discovered = chosen < 0;
            if (discovered) {
                chosen = (int) representatives.size();
                representatives.push_back(id);
            } else {
                solver.groups.join(id, representatives[chosen]);
            }
            label[id] = chosen;
            ++copies[chosen];
            return discovered;
        }

    private:
        Solver& solver;
        vector<int> label, copies;

        pair<int, bool> take() {
            int c = label[solver.belt.samples.front().id];
            assert(c >= 0);
            bool dropped = solver.belt.remove();
            --copies[c];
            return {c, dropped};
        }
    };

public:
    explicit Solver(int n) : n(n), belt(n), groups(n) {}

    void run() {
        if (n > 1) {
            Lookup warm(*this, {});
            vector<int> choices;
            int id = 1;
            while (id <= n && choices.size() < warmClassLimit) {
                if (warm.classify(id, choices, true)) {
                    choices.push_back((int) choices.size());
                }
                ++id;
            }
            Sample unresolved{0, -1};
            if (belt.samples.size() == choices.size() && belt.distinct == (int) choices.size()) {
                // A collision against the oldest class replaces its sample
                // in place in the queue order, with just one dequeue.
                while (id <= n) {
                    auto [sample, grew] = belt.add(id);
                    if (grew) {
                        warm.representatives.push_back(id);
                    } else {
                        int oldest = belt.samples.front().id;
                        if (belt.remove()) {
                            unresolved = sample;
                            if (id == n) {
                                int root = groups.root(oldest);
                                erase_if(warm.representatives, [&](int rep) {
                                    return groups.root(rep) == root;
                                });
                            }
                            ++id;
                            break;
                        }
                        groups.join(oldest, id);
                    }
                    ++id;
                }
            }
            if (id <= n || unresolved.stamp >= 0) {
                vector<Occurrence> scan;
                for (Sample sample : loadRepresentatives(warm.representatives)) {
                    scan.push_back({sample, true});
                }
                if (unresolved.stamp >= 0) {
                    auto where = find_if(scan.begin(), scan.end(), [&](const Occurrence& entry) {
                        return entry.sample.stamp > unresolved.stamp;
                    });
                    scan.insert(where, {unresolved, false});
                }
                for (; id <= n; ++id) {
                    auto [sample, grew] = belt.add(id);
                    scan.push_back({sample, grew});
                }
                finishScan(scan);
            }
        }

        vector<vector<int>> answer(n + 1);
        for (int id = 1; id <= n; ++id) answer[groups.root(id)].push_back(id);
        int count = 0;
        for (const auto& group : answer) count += !group.empty();
        cout << "! " << count << endl;
        for (const auto& group : answer) {
            if (group.empty()) continue;
            cout << group.size();
            for (int id : group) cout << ' ' << id;
            cout << endl;
        }
    }

private:
    static constexpr int warmClassLimit = 13;
    int n;
    Belt belt;
    Groups groups;

    vector<Sample> loadRepresentatives(const vector<int>& representatives) {
        vector<int> rootIndex(n + 1, -1);
        for (int i = 0; i < (int) representatives.size(); ++i) {
            rootIndex[groups.root(representatives[i])] = i;
        }

        vector<int> lastStamp(representatives.size(), -1);
        for (Sample s : belt.samples) {
            int i = rootIndex[groups.root(s.id)];
            if (i >= 0) lastStamp[i] = s.stamp;
        }
        vector<Sample> active;
        for (Sample s : belt.samples) {
            int i = rootIndex[groups.root(s.id)];
            if (i >= 0 && lastStamp[i] == s.stamp) active.push_back(s);
        }
        for (int i = 0; i < (int) representatives.size(); ++i) {
            if (lastStamp[i] == -1) active.push_back(belt.add(representatives[i]).first);
        }
        return active;
    }

    // A previous occurrence can be left in front of its active copy after a
    // split. Such an occurrence is no longer needed by any pending task.
    pair<Sample, bool> probe(int id, int firstActiveStamp) {
        if (belt.present[id] >= 0) {
            assert(belt.present[id] < firstActiveStamp);
            belt.removeThrough(belt.present[id]);
        }
        return belt.add(id);
    }

    void findSingle(Sample active, const vector<int>& candidates) {
        for (int id : candidates) {
            if (groups.root(id) == groups.root(active.id)) return;
        }
        for (int i = 0; i + 1 < (int) candidates.size(); ++i) {
            if (!probe(candidates[i], active.stamp).second) {
                groups.join(active.id, candidates[i]);
                return;
            }
        }
        groups.join(active.id, candidates.back());
    }

    void findPair(const vector<Sample>& active, const vector<int>& candidates) {
        bool firstPresent = belt.present[active[0].id] == active[0].stamp;
        bool secondPresent = belt.present[active[1].id] == active[1].stamp;
        int mate;
        if (firstPresent && secondPresent) {
            probe(candidates[0], active[0].stamp);
            mate = belt.removeThrough(active[0].stamp) ? 1 : 0;
        } else {
            int kept = secondPresent ? 1 : 0;
            int stamp = firstPresent || secondPresent ? active[kept].stamp
                                                      : belt.add(active[kept].id).first.stamp;
            mate = probe(candidates[0], stamp).second ? 1 - kept : kept;
        }
        groups.join(active[mate].id, candidates[0]);
        groups.join(active[1 - mate].id, candidates[1]);
    }

    // An interleaved pass tests each proposed pair from both ends. Its two
    // samples overlap on the belt exactly when both responses show a match.
    void matchExact(Task task, const vector<int>* rank) {
        vector<bitset<501>> edges(n + 1);
        for (Sample s : task.active) {
            for (int id : task.candidates) {
                if (!rank || (*rank)[id] < (*rank)[s.id]) {
                    edges[s.id].set(id);
                    edges[id].set(s.id);
                }
            }
        }

        for (int round = 0; !task.active.empty(); ++round) {
            int size = (int) task.active.size();
            vector<vector<int>> choices(size);
            for (int i = 0; i < size; ++i) {
                for (int j = 0; j < size; ++j) {
                    if (edges[task.active[i].id][task.candidates[j]]) choices[i].push_back(j);
                }
            }

            vector<int> owner(size, -1), visited(size, -1);
            auto augment = [&](auto&& self, int i, int attempt) -> bool {
                for (int j : choices[i]) {
                    if (visited[j] == attempt) continue;
                    visited[j] = attempt;
                    if (owner[j] < 0 || self(self, owner[j], attempt)) {
                        owner[j] = i;
                        return true;
                    }
                }
                return false;
            };
            for (int i = 0; i < size; ++i) {
                for (int j : choices[i]) {
                    if (owner[j] < 0) {
                        owner[j] = i;
                        break;
                    }
                }
            }
            vector<bool> matched(size, false);
            for (int i : owner) if (i >= 0) matched[i] = true;
            for (int i = 0; i < size; ++i) {
                if (!matched[i]) {
                    bool found = augment(augment, i, i);
                    assert(found);
                }
            }

            // Alternating cycles characterize which edges can occur in a
            // perfect matching. Their components can be treated independently.
            vector<vector<int>> reverse(size);
            for (int i = 0; i < size; ++i) {
                for (int j : choices[i]) reverse[owner[j]].push_back(i);
            }
            vector<int> order;
            vector<bool> seen(size, false);
            auto visit = [&](auto&& self, int i) -> void {
                seen[i] = true;
                for (int j : choices[i]) if (!seen[owner[j]]) self(self, owner[j]);
                order.push_back(i);
            };
            for (int i = 0; i < size; ++i) if (!seen[i]) visit(visit, i);
            vector<int> component(size, -1);
            vector<vector<int>> rows, columns;
            auto collect = [&](auto&& self, int i, int c) -> void {
                component[i] = c;
                for (int j : reverse[i]) if (component[j] < 0) self(self, j, c);
            };
            for (auto it = order.rbegin(); it != order.rend(); ++it) {
                if (component[*it] < 0) {
                    collect(collect, *it, (int) rows.size());
                    rows.emplace_back();
                    columns.emplace_back();
                }
            }

            for (int i = 0; i < size; ++i) {
                rows[component[i]].push_back(i);
                columns[component[owner[i]]].push_back(i);
            }
            vector<vector<int>> columnRows(size);
            for (int i = 0; i < size; ++i) {
                int id = task.active[i].id;
                for (int j : choices[i]) {
                    if (component[i] == component[owner[j]]) {
                        columnRows[j].push_back(i);
                    } else {
                        int mate = task.candidates[j];
                        edges[id].reset(mate);
                        edges[mate].reset(id);
                    }
                }
            }

            // Each bucket is inserted just before the corresponding old
            // sample is removed. Process all independent components together.
            vector<vector<int>> schedule(size);
            for (int c = 0; c < (int) rows.size(); ++c) {
                if (rows[c].size() == 1) {
                    groups.join(task.active[rows[c][0]].id, task.candidates[columns[c][0]]);
                } else if (rows[c].size() == 2) {
                    schedule[rows[c][1]].push_back(columns[c][0]);
                } else if (rows[c].size() == 3) {
                    schedule[rows[c][1]].push_back(columns[c][0]);
                    schedule[rows[c][2]].push_back(columns[c][1]);
                } else if (round == 0) {
                    auto orderedRows = rows[c];
                    auto orderedColumns = columns[c];
                    if (rank) {
                        sort(orderedRows.begin(), orderedRows.end(), [&](int a, int b) {
                            return (*rank)[task.active[a].id] < (*rank)[task.active[b].id];
                        });
                        sort(orderedColumns.begin(), orderedColumns.end(), [&](int a, int b) {
                            return (*rank)[task.candidates[a]] < (*rank)[task.candidates[b]];
                        });
                    }
                    for (int k = 0; k < (int) orderedRows.size(); ++k) {
                        schedule[orderedRows[k]].push_back(orderedColumns[k]);
                    }
                } else {
                    for (int j : columns[c]) {
                        const auto& options = columnRows[j];
                        unsigned h = (unsigned) task.candidates[j] * 0x9e3779b9U;
                        h ^= (unsigned) round * 0x85ebca6bU;
                        h ^= h >> 16;
                        int pct = (h & 1) ? 22 : 78;
                        int cut = clamp(((int) options.size() * pct + 50) / 100,
                                        1, (int) options.size() - 1);
                        schedule[options[cut]].push_back(j);
                    }
                }
            }

            int stop = size - 1;
            while (stop >= 0 && schedule[stop].empty()) --stop;
            if (stop < 0) return;
            vector<int> insertion(size, size + 1), kept(size, -1);
            vector<bool> droppedRow(size, false);
            Task next;
            for (int i = 0; i <= stop; ++i) {
                for (int j : schedule[i]) {
                    int c = component[owner[j]];
                    const auto& row = rows[c];
                    const auto& column = columns[c];
                    if (row.size() == 3 && j == column[1]) {
                        int firstMate = 0;
                        if (kept[column[0]]) firstMate = droppedRow[row[1]] ? 2 : 1;
                        int present = (firstMate == 2 ? 0 : 2);
                        int absent = 3 - firstMate - present;
                        int secondMate = present;
                        if (!edges[task.candidates[j]][task.active[row[present]].id]) {
                            secondMate = absent;
                        } else if (edges[task.candidates[j]][task.active[row[absent]].id]) {
                            if (present == 0) belt.add(task.active[row[0]].id);
                            if (belt.add(task.candidates[j]).second) secondMate = absent;
                        }
                        groups.join(task.active[row[firstMate]].id, task.candidates[column[0]]);
                        groups.join(task.active[row[secondMate]].id, task.candidates[j]);
                        groups.join(task.active[row[3 - firstMate - secondMate]].id,
                                    task.candidates[column[2]]);
                        continue;
                    }
                    auto [sample, grew] = belt.add(task.candidates[j]);
                    if (row.size() == 2) {
                        int mate = grew ? 0 : 1;
                        groups.join(task.active[rows[c][mate]].id, sample.id);
                        groups.join(task.active[rows[c][1 - mate]].id,
                                    task.candidates[columns[c][1]]);
                    } else if (row.size() == 3) {
                        kept[j] = !grew;
                    } else {
                        insertion[j] = i;
                        kept[j] = !grew;
                        next.active.push_back(sample);
                    }
                }
                if (i < stop) {
                    bool dropped = belt.removeThrough(task.active[i].stamp);
                    droppedRow[i] = dropped;
                    if (rows[component[i]].size() <= 3) continue;
                    for (int j : choices[i]) {
                        if (component[i] == component[owner[j]] &&
                            dropped == (insertion[j] <= i)) {
                            int id = task.active[i].id, mate = task.candidates[j];
                            edges[id].reset(mate);
                            edges[mate].reset(id);
                        }
                    }
                }
            }
            for (int j = 0; j < size; ++j) {
                if (insertion[j] > size) continue;
                int id = task.candidates[j];
                for (int i : columnRows[j]) {
                    if (kept[j] == (i >= insertion[j])) continue;
                    int mate = task.active[i].id;
                    edges[id].reset(mate);
                    edges[mate].reset(id);
                }
            }
            for (int i = 0; i < size; ++i) {
                if (rows[component[i]].size() > 3) next.candidates.push_back(task.active[i].id);
            }
            task = move(next);
        }
    }

    static int cutSize(int active, int candidates) {
        if (candidates <= 3) return 1;
        if (active == candidates) return (candidates + 1) / 2;
        if (active * 3 <= candidates) return (candidates + 2) / 3;
        return (candidates + 1) / 2;
    }

    // Every active sample has exactly one mate among its task's candidates.
    // The possible classes of different tasks are disjoint.
    void match(Task initial, const vector<int>* rank = nullptr) {
        if (initial.active.size() == initial.candidates.size() && initial.active.size() >= 2) {
            matchExact(move(initial), rank);
            return;
        }
        deque<Task> pending;
        pending.push_back(move(initial));
        while (!pending.empty()) {
            Task task = move(pending.front());
            pending.pop_front();
            if (task.active.empty()) continue;

            int a = (int) task.active.size();
            int b = (int) task.candidates.size();
            assert(a <= b);
            if (a == 1) {
                findSingle(task.active.front(), task.candidates);
                continue;
            }
            if (b == 2) {
                findPair(task.active, task.candidates);
                continue;
            }

            int cut = cutSize(a, b);
            vector<Sample> leftSamples;
            for (int i = 0; i < cut; ++i) {
                auto [sample, grew] = probe(task.candidates[i], task.active.front().stamp);
                if (!grew) leftSamples.push_back(sample);
            }
            vector<int> rightIds(task.candidates.begin() + cut, task.candidates.end());
            int leftSize = (int) leftSamples.size();
            int rightSize = a - leftSize;
            if (leftSize == 0) {
                task.candidates = move(rightIds);
                pending.push_front(move(task));
                continue;
            }

            vector<int> leftIds;
            vector<Sample> rightSamples;
            for (Sample s : task.active) {
                if ((int) leftIds.size() == leftSize) {
                    rightSamples.push_back(s);
                } else if ((int) rightSamples.size() == rightSize) {
                    leftIds.push_back(s.id);
                } else if (belt.removeThrough(s.stamp)) {
                    rightSamples.push_back(s);
                } else {
                    leftIds.push_back(s.id);
                }
            }

            if (leftSize == 1) {
                groups.join(leftSamples.front().id, leftIds.front());
            } else {
                pending.push_back({move(leftSamples), move(leftIds)});
            }

            if (rightSize == 0) continue;
            if (rightIds.size() == 1) {
                groups.join(rightSamples.front().id, rightIds.front());
            } else if (rightSize == 1) {
                Sample s = rightSamples.front();
                if (belt.present[s.id] != s.stamp) s = belt.add(s.id).first;
                findSingle(s, rightIds);
            } else if (rightSize == 2 && rightIds.size() == 2) {
                findPair(rightSamples, rightIds);
            } else if (belt.present[rightSamples.front().id] == rightSamples.front().stamp) {
                pending.push_front({move(rightSamples), move(rightIds)});
            } else {
                if (belt.present[rightSamples.back().id] == rightSamples.back().stamp) {
                    belt.removeThrough(rightSamples.back().stamp);
                }
                for (Sample& s : rightSamples) s = belt.add(s.id).first;
                pending.push_back({move(rightSamples), move(rightIds)});
            }
        }
    }

    void classifyBlock(const vector<int>& ids) {
        belt.clear();
        vector<Occurrence> scan;
        for (int id : ids) {
            auto [sample, grew] = belt.add(id);
            scan.push_back({sample, grew});
        }
        finishScan(scan);
    }

    void classifyInteriors(const vector<Interior>& unresolved, const vector<int>& representatives,
                           const vector<Span>& spans, const vector<int>& rank) {
        if (unresolved.empty()) return;

        vector<bool> eligible(n + 1, false);
        int width = 0, totalWidth = 0;
        for (const auto& item : unresolved) {
            totalWidth += (int) item.candidates.size();
            for (int c : item.candidates) {
                if (!eligible[c]) ++width;
                eligible[c] = true;
            }
        }
        if (unresolved.size() <= 4 || width <= 13 ||
            totalWidth <= 7 * (int) unresolved.size()) {
            Lookup lookup(*this, representatives);
            vector<int> label(n + 1, -1), relevant;
            for (int i = 0; i < (int) representatives.size(); ++i) {
                label[representatives[i]] = i;
                if (eligible[representatives[i]]) relevant.push_back(i);
            }
            for (const auto& item : unresolved) {
                vector<int> options;
                for (int c : item.candidates) options.push_back(label[c]);
                if (width <= 13) lookup.trim(max(1, (width * 2 + 1) / 3), relevant);
                lookup.classify(item.id, options, false);
            }
            return;
        }

        int blockSize = min(200, max(20, (width * 5) / 3));
        int count = (int) unresolved.size();
        int blocks = (count + blockSize - 1) / blockSize;
        for (int j = 0; j < blocks; ++j) {
            int lo = count * j / blocks;
            int hi = count * (j + 1) / blocks;
            vector<int> ids;
            for (int i = lo; i < hi; ++i) ids.push_back(unresolved[i].id);
            // Start with only samples from this block so live classes outside
            // the block cannot affect its later match against the references.
            classifyBlock(ids);

            vector<int> rootIndex(n + 1, -1);
            vector<int> local, earliest, latest;
            for (int id : ids) {
                int root = groups.root(id);
                if (rootIndex[root] == -1) {
                    rootIndex[root] = (int) local.size();
                    local.push_back(id);
                    earliest.push_back(rank[id]);
                    latest.push_back(rank[id]);
                } else {
                    latest[rootIndex[root]] = rank[id];
                }
            }

            vector<int> candidates;
            for (const Span& span : spans) {
                for (int t = 0; t < (int) local.size(); ++t) {
                    if (span.first < earliest[t] && latest[t] < span.last) {
                        candidates.push_back(span.representative);
                        break;
                    }
                }
            }

            if (local.size() == 1 && candidates.size() == 1) {
                groups.join(local.front(), candidates.front());
            } else {
                match({loadRepresentatives(local), move(candidates)});
            }
        }
    }

    void finishScan(vector<Occurrence>& scan) {
        int len = (int) scan.size();
        int classes = belt.distinct;
        int repeats = len - classes;
        if (repeats == 0) return;
        if (classes == 1) {
            for (const auto& entry : scan) groups.join(scan.front().sample.id, entry.sample.id);
            return;
        }
        if (repeats == 1) {
            int repeated = 1;
            while (scan[repeated].first) ++repeated;
            int mate = repeated - 1;
            for (int i = 0; i + 1 < repeated; ++i) {
                if (!belt.removeThrough(scan[i].sample.stamp)) {
                    mate = i;
                    break;
                }
            }
            groups.join(scan[mate].sample.id, scan[repeated].sample.id);
            return;
        }

        int nonlast = repeats;
        for (int i = 0; i < len && nonlast > 0; ++i) {
            if (nonlast == len - i - 1) {
                for (int j = i; j + 1 < len; ++j) {
                    scan[j].last = false;
                    groups.join(scan[j].sample.id, scan.back().sample.id);
                }
                break;
            }
            scan[i].last = belt.removeThrough(scan[i].sample.stamp);
            nonlast -= !scan[i].last;
        }

        vector<int> rank(n + 1, -1);
        vector<int> representatives, starts, ends, between;
        vector<Sample> active;
        vector<int> offBelt;
        for (int i = 0; i < len; ++i) {
            auto [s, first, last] = scan[i];
            rank[s.id] = i;
            if (first) {
                representatives.push_back(s.id);
                if (!last) starts.push_back(s.id);
            } else if (last) {
                ends.push_back(s.id);
                if (belt.present[s.id] == s.stamp) active.push_back(s);
                else offBelt.push_back(s.id);
            } else {
                between.push_back(s.id);
            }
        }
        assert(starts.size() == ends.size());

        // Pair endpoints that were already identified by the one-class tail,
        // then handle any remaining balanced components independently.
        vector<int> startOf(n + 1, -1);
        for (int id : starts) startOf[groups.root(id)] = id;
        vector<bool> paired(n + 1, false);
        for (int id : ends) {
            int mate = startOf[groups.root(id)];
            if (mate >= 0) paired[id] = paired[mate] = true;
        }
        int open = 0, loneStart = -1;
        int componentsStarts = 0;
        for (int i = 0; i < len; ++i) {
            auto [s, first, last] = scan[i];
            if (first == last || paired[s.id]) continue;
            if (first) {
                if (open == 0) componentsStarts = 0;
                ++open;
                ++componentsStarts;
                loneStart = s.id;
            } else if (--open == 0 && componentsStarts == 1) {
                paired[s.id] = paired[loneStart] = true;
                groups.join(s.id, loneStart);
            }
        }
        vector<int> unknownStarts;
        for (int id : starts) if (!paired[id]) unknownStarts.push_back(id);
        erase_if(active, [&](Sample s) { return paired[s.id]; });
        for (int id : offBelt) {
            if (!paired[id]) active.push_back(belt.add(id).first);
        }
        match({move(active), move(unknownStarts)}, &rank);

        vector<int> firstOf(n + 1, -1);
        for (int id : starts) firstOf[groups.root(id)] = id;
        vector<Span> spans;
        for (int id : ends) {
            int start = firstOf[groups.root(id)];
            assert(start > 0);
            spans.push_back({start, rank[start], rank[id]});
        }

        vector<Interior> unresolved;
        vector<bool> assigned(n + 1, false);
        for (int id : representatives) assigned[groups.root(id)] = true;
        for (int id : between) {
            if (assigned[groups.root(id)]) continue;
            Interior item{id, {}};
            for (const Span& span : spans) {
                if (span.first < rank[id] && rank[id] < span.last) {
                    item.candidates.push_back(span.representative);
                }
            }
            assert(!item.candidates.empty());
            if (item.candidates.size() == 1) groups.join(item.id, item.candidates.front());
            else unresolved.push_back(move(item));
        }
        classifyInteriors(unresolved, representatives, spans, rank);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n <= 0) return 0;
    Solver(n).run();
}

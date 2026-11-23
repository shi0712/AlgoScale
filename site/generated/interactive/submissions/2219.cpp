#include <bits/stdc++.h>
using namespace std;

struct Solver {
    static constexpr int LEFT = 0, RIGHT = 1, STEP = 2;
    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};

    int r, c, openCount = 0, n = 0;
    vector<string> grid;
    vector<int> cellPosition, positionCell;
    vector<int> observation, initialColor, color;
    vector<array<int, 3>> nxt;
    vector<vector<int>> pred[3];
    vector<vector<pair<int, int>>> history;
    vector<int> temporaryColor;
    int maxDepth = 0;
    bool impossible = false;

    void buildStates() {
        cellPosition.assign(r * c, -1);
        for (int i = 0; i < r; ++i) {
            for (int j = 0; j < c; ++j) {
                if (grid[i][j] == '.') {
                    cellPosition[i * c + j] = openCount;
                    positionCell.push_back(i * c + j);
                    ++openCount;
                }
            }
        }

        n = openCount * 4;
        observation.resize(n);
        nxt.resize(n);

        for (int p = 0; p < openCount; ++p) {
            int cell = positionCell[p];
            int row = cell / c, col = cell % c;
            for (int dir = 0; dir < 4; ++dir) {
                int s = p * 4 + dir;
                nxt[s][LEFT] = p * 4 + (dir + 3) % 4;
                nxt[s][RIGHT] = p * 4 + (dir + 1) % 4;

                int nr = row + dr[dir], nc = col + dc[dir];
                int d = 0;
                while (nr >= 0 && nr < r && nc >= 0 && nc < c &&
                       grid[nr][nc] == '.') {
                    ++d;
                    nr += dr[dir];
                    nc += dc[dir];
                }
                observation[s] = d;

                if (d == 0) {
                    nxt[s][STEP] = s;
                } else {
                    int fr = row + dr[dir], fc = col + dc[dir];
                    int fp = cellPosition[fr * c + fc];
                    nxt[s][STEP] = fp * 4 + dir;
                }
            }
        }

        for (int a = 0; a < 3; ++a) pred[a].assign(n, {});
        for (int s = 0; s < n; ++s) {
            for (int a = 0; a < 3; ++a) {
                pred[a][nxt[s][a]].push_back(s);
            }
        }
    }

    void refineStates() {
        initialColor.resize(n);
        color.resize(n);
        history.assign(n, {});

        vector<vector<int>> members(100);
        vector<int> dirty;
        for (int s = 0; s < n; ++s) {
            initialColor[s] = color[s] = observation[s];
            members[color[s]].push_back(s);
        }
        for (int b = 0; b < 100; ++b) {
            if (!members[b].empty()) dirty.push_back(b);
        }

        vector<int> dirtyMark(100, 0);
        unordered_map<uint64_t, int> groupNumber;

        struct Split {
            int block;
            vector<vector<int>> groups;
        };

        int iteration = 0;
        while (!dirty.empty()) {
            ++iteration;
            vector<Split> splits;

            for (int block : dirty) {
                if (members[block].size() <= 1) continue;

                groupNumber.clear();
                if (groupNumber.bucket_count() < members[block].size() * 2 + 1) {
                    groupNumber.reserve(members[block].size() * 2 + 1);
                }

                vector<vector<int>> groups;
                for (int s : members[block]) {
                    uint64_t l = color[nxt[s][LEFT]];
                    uint64_t rr = color[nxt[s][RIGHT]];
                    uint64_t f = color[nxt[s][STEP]];
                    uint64_t key = l | (rr << 16) | (f << 32);

                    auto it = groupNumber.find(key);
                    int g;
                    if (it == groupNumber.end()) {
                        g = static_cast<int>(groups.size());
                        groupNumber.emplace(key, g);
                        groups.push_back({});
                    } else {
                        g = it->second;
                    }
                    groups[g].push_back(s);
                }

                if (groups.size() > 1) {
                    splits.push_back({block, std::move(groups)});
                }
            }

            if (splits.empty()) break;

            vector<int> changed;
            for (auto &split : splits) {
                int largest = 0;
                for (int g = 1; g < static_cast<int>(split.groups.size()); ++g) {
                    if (split.groups[g].size() > split.groups[largest].size()) {
                        largest = g;
                    }
                }

                for (int g = 0; g < static_cast<int>(split.groups.size()); ++g) {
                    if (g == largest) continue;
                    int newColor = static_cast<int>(members.size());
                    members.push_back({});
                    dirtyMark.push_back(0);
                    members[newColor] = std::move(split.groups[g]);

                    for (int s : members[newColor]) {
                        color[s] = newColor;
                        history[s].push_back({iteration, newColor});
                        changed.push_back(s);
                    }
                }
                members[split.block] = std::move(split.groups[largest]);
            }

            maxDepth = iteration;
            vector<int> nextDirty;
            for (int s : changed) {
                for (int a = 0; a < 3; ++a) {
                    for (int p : pred[a][s]) {
                        int b = color[p];
                        if (!dirtyMark[b]) {
                            dirtyMark[b] = 1;
                            nextDirty.push_back(b);
                        }
                    }
                }
            }
            for (int b : nextDirty) dirtyMark[b] = 0;
            dirty.swap(nextDirty);
        }

        vector<int> representativePosition(members.size(), -1);
        for (int s = 0; s < n; ++s) {
            int b = color[s], p = s / 4;
            if (representativePosition[b] == -1) {
                representativePosition[b] = p;
            } else if (representativePosition[b] != p) {
                impossible = true;
            }
        }

        temporaryColor.assign(n, -1);
    }

    int colorAt(int s, int depth) const {
        if (depth >= maxDepth) return color[s];
        int result = initialColor[s];
        for (const auto &[time, newColor] : history[s]) {
            if (time > depth) break;
            result = newColor;
        }
        return result;
    }

    bool pickPairFromTemporary(const vector<int> &candidates, int &a, int &b) {
        unordered_map<int, int> groupId;
        vector<int> first, alternative;

        for (int s : candidates) {
            int cl = temporaryColor[s];
            auto it = groupId.find(cl);
            if (it == groupId.end()) {
                int id = static_cast<int>(first.size());
                groupId.emplace(cl, id);
                first.push_back(s);
                alternative.push_back(-1);
            } else {
                int id = it->second;
                if (alternative[id] == -1 &&
                    first[id] / 4 != s / 4) {
                    alternative[id] = s;
                }
            }
        }

        if (first.size() < 2) return false;
        for (int g = 1; g < static_cast<int>(first.size()); ++g) {
            for (int x : {first[0], alternative[0]}) {
                if (x == -1) continue;
                for (int y : {first[g], alternative[g]}) {
                    if (y == -1) continue;
                    if (x / 4 != y / 4) {
                        a = x;
                        b = y;
                        return true;
                    }
                }
            }
        }
        return false;
    }

    vector<int> makeWitness(int a, int b, int depth) const {
        vector<int> result;
        int x = a, y = b;

        for (int remaining = depth; remaining > 0; --remaining) {
            bool found = false;
            for (int action : {LEFT, RIGHT, STEP}) {
                int nx = nxt[x][action], ny = nxt[y][action];
                if (colorAt(nx, remaining - 1) !=
                    colorAt(ny, remaining - 1)) {
                    result.push_back(action);
                    x = nx;
                    y = ny;
                    found = true;
                    break;
                }
            }
            if (!found) break;
        }
        return result;
    }

    vector<int> makePlan(const vector<int> &candidates) {
        struct Event {
            int time, state, newColor;
        };

        vector<Event> events;
        for (int s : candidates) {
            temporaryColor[s] = initialColor[s];
            for (const auto &[time, newColor] : history[s]) {
                events.push_back({time, s, newColor});
            }
        }
        sort(events.begin(), events.end(),
             [](const Event &a, const Event &b) { return a.time < b.time; });

        unordered_map<int, int> counts;
        counts[initialColor[candidates[0]]] = static_cast<int>(candidates.size());

        int depth = -1, a = -1, b = -1;
        size_t index = 0;
        while (index < events.size()) {
            int time = events[index].time;
            do {
                int s = events[index].state;
                int oldColor = temporaryColor[s];
                int newColor = events[index].newColor;

                auto it = counts.find(oldColor);
                if (--(it->second) == 0) counts.erase(it);
                ++counts[newColor];
                temporaryColor[s] = newColor;
                ++index;
            } while (index < events.size() && events[index].time == time);

            if (counts.size() > 1) {
                depth = time;
                break;
            }
        }

        if (depth != -1 && pickPairFromTemporary(candidates, a, b)) {
            return makeWitness(a, b, depth);
        }

        for (int s : candidates) temporaryColor[s] = color[s];
        if (pickPairFromTemporary(candidates, a, b)) {
            int low = 0, high = maxDepth;
            while (high - low > 1) {
                int mid = (low + high) / 2;
                if (colorAt(a, mid) == colorAt(b, mid)) low = mid;
                else high = mid;
            }
            return makeWitness(a, b, high);
        }

        return {};
    }

    void run() {
        vector<int> candidates(n);
        iota(candidates.begin(), candidates.end(), 0);

        bool havePreviousAction = false;
        int previousAction = -1;
        vector<int> plan;
        size_t planIndex = 0;

        while (true) {
            int d;
            if (!(cin >> d)) return;
            if (d < 0) return;

            if (!havePreviousAction) {
                vector<int> filtered;
                for (int s : candidates) {
                    if (observation[s] == d) filtered.push_back(s);
                }
                candidates.swap(filtered);
            } else {
                vector<int> filtered;
                for (int s : candidates) {
                    int ns = nxt[s][previousAction];
                    if (observation[ns] == d) filtered.push_back(ns);
                }
                candidates.swap(filtered);
            }

            if (candidates.empty()) return;

            int commonPosition = candidates[0] / 4;
            bool samePosition = true;
            for (int s : candidates) {
                if (s / 4 != commonPosition) {
                    samePosition = false;
                    break;
                }
            }

            if (samePosition) {
                int cell = positionCell[commonPosition];
                cout << "yes " << cell / c + 1 << ' ' << cell % c + 1 << endl;
                return;
            }

            if (impossible) {
                cout << "no" << endl;
                return;
            }

            vector<int> actions = {LEFT, RIGHT};
            if (d > 0) actions.push_back(STEP);

            int bestAction = -1;
            tuple<int, long long, int> bestScore;
            bool firstBest = true;

            for (int action : actions) {
                array<int, 100> counts{};
                for (int s : candidates) {
                    ++counts[observation[nxt[s][action]]];
                }

                int buckets = 0, largest = 0;
                long long sumSquares = 0;
                for (int x : counts) {
                    if (x == 0) continue;
                    ++buckets;
                    largest = max(largest, x);
                    sumSquares += 1LL * x * x;
                }
                if (buckets <= 1) continue;

                auto score = make_tuple(largest, sumSquares, -buckets);
                if (firstBest || score < bestScore) {
                    firstBest = false;
                    bestScore = score;
                    bestAction = action;
                }
            }

            int action;
            if (bestAction != -1) {
                action = bestAction;
                plan.clear();
                planIndex = 0;
            } else {
                if (plan.empty() || planIndex >= plan.size()) {
                    plan = makePlan(candidates);
                    planIndex = 0;
                }
                if (plan.empty()) return;
                action = plan[planIndex++];
            }

            static const char *command[3] = {"left", "right", "step"};
            cout << command[action] << endl;
            previousAction = action;
            havePreviousAction = true;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    if (!(cin >> solver.r >> solver.c)) return 0;
    solver.grid.resize(solver.r);
    for (string &row : solver.grid) cin >> row;

    solver.buildStates();
    solver.refineStates();
    solver.run();
    return 0;
}

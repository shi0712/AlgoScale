#include <bits/stdc++.h>
using namespace std;

struct Automaton {
    int r, c, M, K;
    vector<int> obs;
    vector<array<int, 3>> nxt;
    vector<vector<int>> states;
    vector<int> clsOut;
    vector<array<int, 3>> quotient;
};

static Automaton buildAutomaton(const vector<string>& grid, int r, int c) {
    Automaton a;
    a.r = r;
    a.c = c;
    a.M = 4 * r * c;
    a.obs.assign(a.M, -1);
    a.nxt.resize(a.M);

    const int dr[4] = {-1, 0, 1, 0};
    const int dc[4] = {0, 1, 0, -1};

    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            if (grid[i][j] == '#') continue;

            int cell = i * c + j;
            for (int d = 0; d < 4; ++d) {
                int id = 4 * cell + d;
                int x = i + dr[d];
                int y = j + dc[d];
                int distance = 0;

                while (0 <= x && x < r && 0 <= y && y < c &&
                       grid[x][y] == '.') {
                    ++distance;
                    x += dr[d];
                    y += dc[d];
                }

                a.obs[id] = distance;
                a.nxt[id][0] = 4 * cell + (d + 3) % 4;
                a.nxt[id][1] = 4 * cell + (d + 1) % 4;
                a.nxt[id][2] = -1;

                if (distance > 0) {
                    int nx = i + dr[d];
                    int ny = j + dc[d];
                    a.nxt[id][2] = 4 * (nx * c + ny) + d;
                }
            }
        }
    }

    vector<vector<vector<int>>> pred(
        3, vector<vector<int>>(a.M)
    );

    for (int id = 0; id < a.M; ++id) {
        if (a.obs[id] < 0) continue;
        for (int act = 0; act < 3; ++act) {
            if (a.nxt[id][act] >= 0) {
                pred[act][a.nxt[id][act]].push_back(id);
            }
        }
    }

    map<int, int> initialBlock;
    vector<vector<int>> blocks;

    for (int id = 0; id < a.M; ++id) {
        if (a.obs[id] < 0) continue;
        auto it = initialBlock.find(a.obs[id]);
        if (it == initialBlock.end()) {
            int b = (int)blocks.size();
            initialBlock[a.obs[id]] = b;
            blocks.push_back({id});
        } else {
            blocks[it->second].push_back(id);
        }
    }

    vector<int> blockOf(a.M, -1);
    for (int b = 0; b < (int)blocks.size(); ++b) {
        for (int id : blocks[b]) blockOf[id] = b;
    }

    queue<int> work;
    vector<char> inWork(a.M + 1, false);
    for (int b = 0; b < (int)blocks.size(); ++b) {
        inWork[b] = true;
        work.push(b);
    }

    vector<int> marked(a.M, 0);
    vector<int> groupTag(a.M + 1, 0);
    vector<int> groupSlot(a.M + 1, 0);
    int markEpoch = 0;
    int groupEpoch = 0;

    while (!work.empty()) {
        int splitter = work.front();
        work.pop();
        inWork[splitter] = false;

        vector<int> splitterStates = blocks[splitter];

        for (int act = 0; act < 3; ++act) {
            ++markEpoch;
            vector<int> X;

            for (int destination : splitterStates) {
                for (int source : pred[act][destination]) {
                    if (marked[source] != markEpoch) {
                        marked[source] = markEpoch;
                        X.push_back(source);
                    }
                }
            }

            ++groupEpoch;
            vector<int> touched;
            vector<vector<int>> inside;

            for (int id : X) {
                int b = blockOf[id];
                if (groupTag[b] != groupEpoch) {
                    groupTag[b] = groupEpoch;
                    groupSlot[b] = (int)touched.size();
                    touched.push_back(b);
                    inside.push_back({});
                }
                inside[groupSlot[b]].push_back(id);
            }

            for (int index = 0; index < (int)touched.size(); ++index) {
                int b = touched[index];
                vector<int>& in = inside[index];
                vector<int> out;

                for (int id : blocks[b]) {
                    if (marked[id] != markEpoch) out.push_back(id);
                }

                if (in.size() == blocks[b].size()) continue;

                vector<int> small, large;
                if (in.size() <= out.size()) {
                    small = move(in);
                    large = move(out);
                } else {
                    small = move(out);
                    large = move(in);
                }

                blocks[b] = move(large);
                int newBlock = (int)blocks.size();
                blocks.push_back(move(small));

                for (int id : blocks[newBlock]) blockOf[id] = newBlock;

                if (!inWork[newBlock]) {
                    inWork[newBlock] = true;
                    work.push(newBlock);
                }
            }
        }
    }

    a.K = (int)blocks.size();
    a.states = move(blocks);
    a.clsOut.resize(a.K);
    a.quotient.resize(a.K);

    for (int b = 0; b < a.K; ++b) {
        int representative = a.states[b][0];
        a.clsOut[b] = a.obs[representative];

        for (int act = 0; act < 3; ++act) {
            int destination = a.nxt[representative][act];
            a.quotient[b][act] =
                destination < 0 ? -1 : blockOf[destination];
        }
    }

    return a;
}

struct Group {
    int cls;
    vector<int> states;
};

struct ActionEvaluation {
    int worstPositions = 0;
    int sumPositions = 0;
    int worstGroups = 0;
    int sumGroups = 0;
};

static ActionEvaluation evaluateAction(
    const Automaton& a,
    const vector<Group>& groups,
    int act
) {
    struct Branch {
        vector<int> cells;
        vector<int> destinations;
    };

    map<int, Branch> branches;

    for (const Group& group : groups) {
        int destinationClass = a.quotient[group.cls][act];
        int observation = a.clsOut[destinationClass];

        Branch& branch = branches[observation];
        branch.destinations.push_back(destinationClass);

        for (int state : group.states) {
            int nextState = a.nxt[state][act];
            branch.cells.push_back(nextState / 4);
        }
    }

    ActionEvaluation result;

    for (auto& [observation, branch] : branches) {
        sort(branch.cells.begin(), branch.cells.end());
        branch.cells.erase(
            unique(branch.cells.begin(), branch.cells.end()),
            branch.cells.end()
        );

        sort(branch.destinations.begin(), branch.destinations.end());
        branch.destinations.erase(
            unique(branch.destinations.begin(), branch.destinations.end()),
            branch.destinations.end()
        );

        int positions = (int)branch.cells.size();
        int groupCount = (int)branch.destinations.size();

        result.worstPositions = max(result.worstPositions, positions);
        result.sumPositions += positions;
        result.worstGroups = max(result.worstGroups, groupCount);
        result.sumGroups += groupCount;
    }

    return result;
}

struct PathParent {
    uint64_t parent;
    int action;
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int r, c;
    if (!(cin >> r >> c)) return 0;

    vector<string> grid(r);
    for (string& row : grid) cin >> row;

    Automaton automaton = buildAutomaton(grid, r, c);

    int observation;
    if (!(cin >> observation)) return 0;
    if (observation < 0) return 0;

    vector<Group> groups;
    for (int cls = 0; cls < automaton.K; ++cls) {
        if (automaton.clsOut[cls] == observation) {
            groups.push_back({cls, automaton.states[cls]});
        }
    }

    unordered_map<uint64_t, vector<int>> pathCache;

    auto findDistinguishingPath = [&](int first, int second) -> vector<int> {
        if (first > second) swap(first, second);
        uint64_t cacheKey =
            (uint64_t)first * automaton.K + second;

        auto cached = pathCache.find(cacheKey);
        if (cached != pathCache.end()) return cached->second;

        uint64_t root = cacheKey;
        unordered_map<uint64_t, PathParent> parent;
        parent.reserve(4096);

        vector<uint64_t> queue;
        queue.push_back(root);

        vector<int> answer;

        for (size_t head = 0; head < queue.size() && answer.empty(); ++head) {
            uint64_t currentKey = queue[head];
            int u = currentKey / automaton.K;
            int v = currentKey % automaton.K;

            for (int act = 0; act < 3; ++act) {
                if (act == 2 && automaton.clsOut[u] == 0) continue;

                int x = automaton.quotient[u][act];
                int y = automaton.quotient[v][act];
                if (x < 0 || y < 0) continue;

                if (automaton.clsOut[x] != automaton.clsOut[y] || x == y) {
                    vector<int> reversed;
                    uint64_t cursor = currentKey;

                    while (cursor != root) {
                        PathParent info = parent.at(cursor);
                        reversed.push_back(info.action);
                        cursor = info.parent;
                    }

                    reverse(reversed.begin(), reversed.end());
                    reversed.push_back(act);
                    answer = move(reversed);
                    break;
                }

                uint64_t nextKey = (uint64_t)x * automaton.K + y;
                if (parent.find(nextKey) != parent.end()) continue;

                parent.emplace(nextKey, PathParent{currentKey, act});
                queue.push_back(nextKey);
            }
        }

        pathCache.emplace(cacheKey, answer);
        return answer;
    };

    vector<int> plan;

    while (true) {
        unordered_set<int> allCells;
        allCells.reserve(groups.size() * 4 + 1);

        bool allGroupsAmbiguous = true;

        for (const Group& group : groups) {
            int firstCell = -1;
            bool ambiguous = false;

            for (int state : group.states) {
                int cell = state / 4;
                allCells.insert(cell);

                if (firstCell < 0) {
                    firstCell = cell;
                } else if (firstCell != cell) {
                    ambiguous = true;
                }
            }

            if (!ambiguous) allGroupsAmbiguous = false;
        }

        if (allCells.size() == 1) {
            int cell = *allCells.begin();
            cout << "yes " << cell / c + 1 << ' ' << cell % c + 1
                 << '\n' << flush;
            return 0;
        }

        if (allGroupsAmbiguous && !groups.empty()) {
            cout << "no\n" << flush;
            return 0;
        }

        int chosenAction = -1;
        tuple<int, int, int, int> bestScore;
        bool found = false;

        for (int act = 0; act < 3; ++act) {
            if (act == 2 && observation == 0) continue;

            ActionEvaluation evaluation =
                evaluateAction(automaton, groups, act);

            if (evaluation.worstGroups >= (int)groups.size()) continue;

            auto score = make_tuple(
                evaluation.worstPositions,
                evaluation.sumPositions,
                evaluation.worstGroups,
                evaluation.sumGroups
            );

            if (!found || score < bestScore) {
                found = true;
                bestScore = score;
                chosenAction = act;
            }
        }

        if (found) {
            plan.clear();
        } else {
            if (plan.empty()) {
                vector<pair<int, int>> candidatePairs;
                int limit = min((int)groups.size(), 7);

                for (int i = 0; i < limit; ++i) {
                    for (int j = i + 1; j < limit; ++j) {
                        candidatePairs.push_back({i, j});
                    }
                }

                if (candidatePairs.empty() && groups.size() >= 2) {
                    candidatePairs.push_back({0, 1});
                }

                vector<int> bestPath;
                for (auto [i, j] : candidatePairs) {
                    vector<int> candidatePath = findDistinguishingPath(
                        groups[i].cls, groups[j].cls
                    );

                    if (!candidatePath.empty() &&
                        (bestPath.empty() ||
                         candidatePath.size() < bestPath.size())) {
                        bestPath = move(candidatePath);
                    }
                }

                plan = move(bestPath);
            }

            if (plan.empty()) {
                for (int act = 0; act < 3; ++act) {
                    if (act == 2 && observation == 0) continue;
                    chosenAction = act;
                    break;
                }
            } else {
                chosenAction = plan.front();
                plan.erase(plan.begin());
            }
        }

        if (chosenAction < 0) return 0;

        if (chosenAction == 0) {
            cout << "left\n" << flush;
        } else if (chosenAction == 1) {
            cout << "right\n" << flush;
        } else {
            cout << "step\n" << flush;
        }

        int nextObservation;
        if (!(cin >> nextObservation)) return 0;
        if (nextObservation < 0) return 0;

        vector<Group> nextGroups;
        unordered_map<int, int> destinationIndex;

        for (const Group& group : groups) {
            int destinationClass =
                automaton.quotient[group.cls][chosenAction];

            if (automaton.clsOut[destinationClass] != nextObservation) {
                continue;
            }

            auto it = destinationIndex.find(destinationClass);
            int index;

            if (it == destinationIndex.end()) {
                index = (int)nextGroups.size();
                destinationIndex.emplace(destinationClass, index);
                nextGroups.push_back({destinationClass, {}});
            } else {
                index = it->second;
            }

            for (int state : group.states) {
                nextGroups[index].states.push_back(
                    automaton.nxt[state][chosenAction]
                );
            }
        }

        groups = move(nextGroups);
        observation = nextObservation;
    }
}

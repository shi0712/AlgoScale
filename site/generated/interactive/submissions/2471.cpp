#include <bits/stdc++.h>
using namespace std;

using Belief = vector<int>;

struct Result {
    bool feasible = false;
    double expected = numeric_limits<double>::infinity();
    int worst = numeric_limits<int>::max();
    int action = -1;
};

struct Key {
    Belief belief;
    int depth;
    bool exact;

    bool operator==(const Key& other) const {
        return depth == other.depth && exact == other.exact && belief == other.belief;
    }
};

struct KeyHash {
    size_t operator()(const Key& key) const {
        size_t h = key.belief.size() * 1000003ULL + key.depth * 97ULL + key.exact;
        for (int x : key.belief) {
            h ^= x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        }
        return h;
    }
};

class Solver {
public:
    int rows, cols, cells, states;
    vector<int> observation;
    vector<array<int, 3>> transition;
    vector<char> valid;
    vector<int> equivalence;
    unordered_map<Key, Result, KeyHash> memo;
    int visitedNodes = 0;
    int nodeBudget = 0;

    Solver(int r, int c, const vector<string>& grid) : rows(r), cols(c), cells(r * c), states(r * c * 4) {
        observation.resize(states);
        transition.resize(states);
        valid.assign(states, false);
        const int dr[4] = {-1, 0, 1, 0};
        const int dc[4] = {0, 1, 0, -1};

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (grid[i][j] == '#') continue;
                int p = i * cols + j;
                for (int dir = 0; dir < 4; ++dir) {
                    int s = p * 4 + dir;
                    valid[s] = true;
                    int distance = 0;
                    int x = i + dr[dir], y = j + dc[dir];
                    while (x >= 0 && x < rows && y >= 0 && y < cols && grid[x][y] == '.') {
                        ++distance;
                        x += dr[dir];
                        y += dc[dir];
                    }
                    observation[s] = distance;
                    transition[s][0] = p * 4 + (dir + 3) % 4;
                    transition[s][1] = p * 4 + (dir + 1) % 4;
                    int nx = i + dr[dir], ny = j + dc[dir];
                    if (nx >= 0 && nx < rows && ny >= 0 && ny < cols && grid[nx][ny] == '.') {
                        transition[s][2] = (nx * cols + ny) * 4 + dir;
                    } else {
                        transition[s][2] = -1;
                    }
                }
            }
        }
        computeEquivalence();
    }

    void computeEquivalence() {
        equivalence.resize(states);
        for (int s = 0; s < states; ++s) equivalence[s] = observation[s];

        vector<int> order(states);
        iota(order.begin(), order.end(), 0);
        vector<int> nextClass(states);
        while (true) {
            sort(order.begin(), order.end(), [&](int a, int b) {
                if (observation[a] != observation[b]) return observation[a] < observation[b];
                for (int action = 0; action < 3; ++action) {
                    int ta = transition[a][action];
                    int tb = transition[b][action];
                    int ca = ta < 0 ? -1 : equivalence[ta];
                    int cb = tb < 0 ? -1 : equivalence[tb];
                    if (ca != cb) return ca < cb;
                }
                return a < b;
            });

            int classes = 0;
            for (int k = 0; k < states; ++k) {
                int s = order[k];
                if (k > 0) {
                    int previous = order[k - 1];
                    bool different = observation[s] != observation[previous];
                    for (int action = 0; action < 3 && !different; ++action) {
                        int a = transition[s][action];
                        int b = transition[previous][action];
                        int ca = a < 0 ? -1 : equivalence[a];
                        int cb = b < 0 ? -1 : equivalence[b];
                        different = ca != cb;
                    }
                    if (different) ++classes;
                }
                nextClass[s] = classes;
            }
            ++classes;
            int previousClasses = 0;
            for (int s = 0; s < states; ++s) previousClasses = max(previousClasses, equivalence[s] + 1);
            equivalence.swap(nextClass);
            if (classes == previousClasses) break;
        }
    }

    static int position(int state) {
        return state / 4;
    }

    bool localized(const Belief& belief) const {
        for (int s : belief) {
            if (position(s) != position(belief[0])) return false;
        }
        return true;
    }

    bool impossible(const Belief& belief) const {
        unordered_map<int, int> seen;
        seen.reserve(belief.size() * 2 + 1);
        for (int s : belief) {
            auto [it, inserted] = seen.emplace(equivalence[s], position(s));
            if (!inserted && it->second != position(s)) return true;
        }
        return false;
    }

    vector<Belief> outcomes(const Belief& belief, int action) const {
        map<int, Belief> groups;
        for (int s : belief) {
            int t = transition[s][action];
            groups[observation[t]].push_back(t);
        }
        vector<Belief> result;
        for (auto& [obs, group] : groups) {
            sort(group.begin(), group.end());
            result.push_back(move(group));
        }
        return result;
    }

    Result heuristic(const Belief& belief) const {
        if (localized(belief)) return {true, 0.0, 0, -1};
        vector<int> positions;
        positions.reserve(belief.size());
        for (int s : belief) positions.push_back(position(s));
        sort(positions.begin(), positions.end());
        positions.erase(unique(positions.begin(), positions.end()), positions.end());
        double estimate = log2((double)positions.size()) + 0.25;
        return {true, estimate, (int)ceil(estimate), -1};
    }

    Result evaluate(const Belief& belief, int depth, bool exact) {
        if (++visitedNodes > nodeBudget) throw overflow_error("search budget");
        if (localized(belief)) return {true, 0.0, 0, -1};
        if (depth == 0) {
            if (exact) return {};
            return heuristic(belief);
        }

        Key key{belief, depth, exact};
        auto found = memo.find(key);
        if (found != memo.end()) return found->second;

        Result best;
        for (int action = 0; action < 3; ++action) {
            if (action == 2 && observation[belief[0]] == 0) continue;
            vector<Belief> next = outcomes(belief, action);
            if (next.size() == 1 && next[0] == belief) continue;

            Result candidate;
            candidate.feasible = true;
            candidate.expected = 1.0;
            candidate.worst = 1;
            candidate.action = action;
            for (const Belief& group : next) {
                Result child = evaluate(group, depth - 1, exact);
                if (!child.feasible) {
                    candidate.feasible = false;
                    break;
                }
                candidate.expected += (double)group.size() / belief.size() * child.expected;
                candidate.worst = max(candidate.worst, 1 + child.worst);
            }
            if (!candidate.feasible) continue;
            if (!best.feasible || candidate.expected < best.expected - 1e-9 ||
                (abs(candidate.expected - best.expected) <= 1e-9 && candidate.worst < best.worst)) {
                best = candidate;
            }
        }
        memo.emplace(move(key), best);
        return best;
    }

    int separatingAction(const Belief& belief) const {
        int a = -1, b = -1;
        for (int i = 0; i < (int)belief.size() && a < 0; ++i) {
            for (int j = i + 1; j < (int)belief.size(); ++j) {
                if (position(belief[i]) != position(belief[j])) {
                    a = belief[i];
                    b = belief[j];
                    break;
                }
            }
        }
        if (a < 0) return -1;

        struct Node {
            int a, b, parent, action;
        };
        vector<Node> nodes;
        unordered_map<long long, int> seen;
        queue<int> pending;
        long long startKey = (long long)a * states + b;
        nodes.push_back({a, b, -1, -1});
        seen[startKey] = 0;
        pending.push(0);

        while (!pending.empty()) {
            int index = pending.front();
            pending.pop();
            Node node = nodes[index];
            for (int action = 0; action < 3; ++action) {
                if (action == 2 && observation[node.a] == 0) continue;
                int na = transition[node.a][action];
                int nb = transition[node.b][action];
                if (observation[na] != observation[nb]) {
                    vector<int> path;
                    path.push_back(action);
                    int at = index;
                    while (nodes[at].parent >= 0) {
                        path.push_back(nodes[at].action);
                        at = nodes[at].parent;
                    }
                    reverse(path.begin(), path.end());
                    return path[0];
                }
                long long key = (long long)na * states + nb;
                if (seen.count(key)) continue;
                int newIndex = nodes.size();
                seen[key] = newIndex;
                nodes.push_back({na, nb, index, action});
                pending.push(newIndex);
            }
        }
        return 0;
    }

    int chooseAction(const Belief& belief) {
        memo.clear();
        visitedNodes = 0;
        nodeBudget = 60000;
        Result result;
        int n = belief.size();
        try {
            if (n <= 18) {
                for (int depth = 1; depth <= 9; ++depth) {
                    result = evaluate(belief, depth, true);
                    if (result.feasible) break;
                }
            } else {
                int depth = n <= 35 ? 6 : n <= 100 ? 5 : n <= 400 ? 4 : n <= 1500 ? 3 : 2;
                result = evaluate(belief, depth, false);
            }
        } catch (const overflow_error&) {
            result = {};
        }
        if (result.feasible && result.action >= 0) return result.action;
        return separatingAction(belief);
    }

    Belief apply(const Belief& belief, int action, int obs) const {
        Belief next;
        for (int s : belief) {
            int t = transition[s][action];
            if (observation[t] == obs) next.push_back(t);
        }
        sort(next.begin(), next.end());
        return next;
    }

    void run() {
        int obs;
        if (!(cin >> obs)) return;
        Belief belief;
        for (int s = 0; s < states; ++s) {
            if (valid[s] && observation[s] == obs) belief.push_back(s);
        }

        while (true) {
            if (localized(belief)) {
                int p = position(belief[0]);
                cout << "yes " << p / cols + 1 << ' ' << p % cols + 1 << endl;
                return;
            }
            if (impossible(belief)) {
                cout << "no" << endl;
                return;
            }

            int action = chooseAction(belief);
            if (action == 0) cout << "left" << endl;
            else if (action == 1) cout << "right" << endl;
            else cout << "step" << endl;
            if (!(cin >> obs)) return;
            belief = apply(belief, action, obs);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int r, c;
    if (!(cin >> r >> c)) return 0;
    vector<string> grid(r);
    for (string& row : grid) cin >> row;
    Solver solver(r, c, grid);
    solver.run();
    return 0;
}

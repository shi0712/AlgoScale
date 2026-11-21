#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

namespace {

constexpr int leftTurn = 0;
constexpr int rightTurn = 1;
constexpr int forwardStep = 2;
constexpr int noTarget = numeric_limits<int>::min();
constexpr double infinity = numeric_limits<double>::infinity();
constexpr double tolerance = 1e-9;
constexpr int searchDepth = 46;
constexpr int exhaustive = 4;
constexpr array<const char*, 3> command = {"left", "right", "step"};

struct Point {
    int row, col;
};

Point rotateClockwise(Point p, int turns) {
    switch (turns & 3) {
        case 1: return {p.col, -p.row};
        case 2: return {-p.row, -p.col};
        case 3: return {-p.col, p.row};
        default: return p;
    }
}

uint64_t mix(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

struct Component {
    vector<int> cells;
    array<int, 4> shape;
    array<int, 4> corner;
    int kind;
    int center = -1;
};

struct Candidate {
    // All four board rotations are stored with the initial heading facing up.
    // Each command updates a common displacement from these origins.
    int origin;
    int alternate = -1;
    int weight = 1;
    bool solvable;
    uint64_t hash;
    double weightLog = 0;
};

struct Bucket {
    int weight = 0;
    int solvableWeight = 0;
    int groups = 0;
    int position = -1;
    long long centerCost = 0;
    double groupLogs = 0;
    bool samePosition = true;
};

struct Estimate {
    double guess = 0;
    double lower = 0;
    double upper = infinity;
};

struct Option {
    double guess;
    double lower;
    double upper;
    int node;
    int cost;
    double priority = infinity;
};

struct Node {
    int index;
    int parent;
    int length;
    int action;
};

struct Policy {
    vector<int> belief;
    vector<unsigned char> actions;
    Estimate value;
    int relative;
    int strength;
};

struct Workspace {
    vector<int> seen;
    vector<Node> nodes;
    vector<int> queue;
    vector<Option> options;
    int stamp = 0;

    void reset(int size) {
        if (seen.empty()) seen.resize(size);
        ++stamp;
        nodes.clear();
        queue.clear();
        options.clear();
    }

    int add(int index, int parent, int length, int action, bool uniform) {
        seen[index] = stamp;
        int id = static_cast<int>(nodes.size());
        nodes.push_back({index, parent, length, action});
        if (uniform) queue.push_back(id);
        return id;
    }
};

class Navigator {
    int side, area, boardSize;
    vector<unsigned char> open;
    vector<unsigned char> rawDistance;
    vector<unsigned char> distance;
    vector<uint16_t> centerDistance;
    vector<int> absolute;
    vector<Candidate> candidates;
    array<vector<int>, 100> initial;
    array<int, 4> stride;
    vector<double> weightLog;
    array<Workspace, searchDepth> workspaces;
    unordered_map<uint64_t, Policy> policies;
    size_t savedCandidates = 0;
    size_t savedCommands = 0;
    long long effort = 0;
    long long searchLimit = 0;
    long long caseLimit = 280000000;
    chrono::steady_clock::duration thinkingTime{};

    int transition(int relative, int action) const {
        int heading = relative & 3;
        if (action == forwardStep) return relative + 4 * stride[heading];
        int nextHeading = (heading + (action == leftTurn ? 3 : 1)) & 3;
        return relative - heading + nextHeading;
    }

    Point coordinates(int position) const {
        return {position / side, position % side};
    }

    int rotatedPosition(int position, int heading) const {
        Point p = coordinates(position);
        switch (heading) {
            case 1: return (side - 1 - p.col) * side + p.row;
            case 2: return area - 1 - position;
            case 3: return p.col * side + side - 1 - p.row;
            default: return position;
        }
    }

    void buildDistances() {
        for (int p = 0; p < area; ++p) {
            if (!open[p]) continue;
            if (p >= side && open[p - side]) {
                rawDistance[4 * p] = rawDistance[4 * (p - side)] + 1;
            }
            if (p % side && open[p - 1]) {
                rawDistance[4 * p + 3] = rawDistance[4 * (p - 1) + 3] + 1;
            }
        }
        for (int p = area - 1; p >= 0; --p) {
            if (!open[p]) continue;
            if (p + side < area && open[p + side]) {
                rawDistance[4 * p + 2] = rawDistance[4 * (p + side) + 2] + 1;
            }
            if (p % side != side - 1 && open[p + 1]) {
                rawDistance[4 * p + 1] = rawDistance[4 * (p + 1) + 1] + 1;
            }
        }
        for (int h = 0; h < 4; ++h) {
            for (int p = 0; p < area; ++p) {
                int index = h * area + rotatedPosition(p, h);
                absolute[index] = p;
                for (int d = 0; d < 4; ++d) {
                    distance[4 * index + d] = rawDistance[4 * p + ((h + d) & 3)];
                }
            }
        }
    }

    vector<Component> findComponents(vector<int>& copies) const {
        vector<unsigned char> used(area);
        map<vector<int>, int> shapes;
        vector<Component> components;
        for (int start = 0; start < area; ++start) {
            if (!open[start] || used[start]) continue;
            Component comp;
            comp.cells.push_back(start);
            used[start] = 1;
            for (size_t i = 0; i < comp.cells.size(); ++i) {
                int p = comp.cells[i];
                for (int d = 0; d < 4; ++d) {
                    if (rawDistance[4 * p + d] == 0) continue;
                    int q = p + stride[d];
                    if (used[q]) continue;
                    used[q] = 1;
                    comp.cells.push_back(q);
                }
            }
            // Normalize each rotation by its bounding box. Tagged cells in
            // equal shapes have the same observations under every command.
            int maxRow = 0, maxCol = 0;
            for (int h = 0; h < 4; ++h) {
                int minRow = side, minCol = side;
                vector<int> key;
                key.reserve(comp.cells.size());
                for (int p : comp.cells) {
                    int q = rotatedPosition(p, h);
                    Point pt = coordinates(q);
                    minRow = min(minRow, pt.row);
                    minCol = min(minCol, pt.col);
                    if (h == 0) {
                        maxRow = max(maxRow, pt.row);
                        maxCol = max(maxCol, pt.col);
                    }
                    key.push_back(q);
                }
                comp.corner[h] = minRow * side + minCol;
                for (int& q : key) q -= comp.corner[h];
                sort(key.begin(), key.end());
                int id = static_cast<int>(shapes.size());
                auto entry = shapes.try_emplace(move(key), id);
                comp.shape[h] = entry.first->second;
            }
            comp.kind = *min_element(comp.shape.begin(), comp.shape.end());
            if (comp.shape[0] == comp.shape[2]) {
                Point corner = coordinates(comp.corner[0]);
                int rowSum = corner.row + maxRow;
                int colSum = corner.col + maxCol;
                if ((rowSum & 1) == 0 && (colSum & 1) == 0) {
                    int center = (rowSum / 2) * side + colSum / 2;
                    if (find(comp.cells.begin(), comp.cells.end(), center) != comp.cells.end()) {
                        comp.center = center;
                    }
                }
            }
            components.push_back(move(comp));
        }
        copies.resize(shapes.size());
        for (const Component& comp : components) ++copies[comp.kind];
        return components;
    }

    void buildCandidates() {
        vector<int> copies;
        vector<Component> components = findComponents(copies);
        unordered_map<int, int> roots;
        roots.reserve(boardSize);
        vector<int> rawCenterDistance(boardSize, -1);
        vector<int> queue;
        for (const Component& comp : components) {
            bool symmetric = comp.shape[0] == comp.shape[2];
            // Another copy can never share a physical square. For a unique
            // component, only the center is fixed by a nontrivial rotation.
            bool solvable = copies[comp.kind] == 1 && (!symmetric || comp.center >= 0);
            if (solvable && symmetric) {
                for (int h = 0; h < 4; ++h) {
                    int state = 4 * comp.center + h;
                    rawCenterDistance[state] = 0;
                    queue.push_back(state);
                }
            }
            for (int h = 0; h < 4; ++h) {
                for (int p : comp.cells) {
                    int q = rotatedPosition(p, h);
                    int key = comp.shape[h] * area + q - comp.corner[h];
                    int origin = 4 * (h * area + q);
                    int id = static_cast<int>(candidates.size());
                    auto entry = roots.try_emplace(key, id);
                    if (entry.second) {
                        candidates.push_back({origin, -1, 1, solvable, mix(id)});
                        initial[distance[origin]].push_back(id);
                    } else {
                        Candidate& candidate = candidates[entry.first->second];
                        if (candidate.alternate < 0) candidate.alternate = origin;
                        ++candidate.weight;
                    }
                }
            }
        }
        // Reverse shortest paths give the exact remaining cost inside a
        // single equivalence class that must move to its symmetry center.
        for (size_t i = 0; i < queue.size(); ++i) {
            int state = queue[i];
            int d = state & 3;
            array<int, 3> previous = {transition(state, leftTurn), transition(state, rightTurn), -1};
            if (rawDistance[(state - d) + ((d + 2) & 3)] > 0) {
                previous[2] = state - 4 * stride[d];
            }
            for (int other : previous) {
                if (other < 0 || rawCenterDistance[other] >= 0) continue;
                rawCenterDistance[other] = rawCenterDistance[state] + 1;
                queue.push_back(other);
            }
        }
        for (int h = 0; h < 4; ++h) {
            for (int p = 0; p < area; ++p) {
                int index = h * area + rotatedPosition(p, h);
                for (int d = 0; d < 4; ++d) {
                    centerDistance[4 * index + d] =
                        static_cast<uint16_t>(max(0, rawCenterDistance[4 * p + ((h + d) & 3)]));
                }
            }
        }
        for (Candidate& candidate : candidates) candidate.weightLog = weightLog[candidate.weight];
    }

    Point physicalPoint(int origin, Point displacement) const {
        Point originPoint = coordinates(absolute[origin / 4]);
        Point offset = rotateClockwise(displacement, origin / boardSize);
        return {originPoint.row + offset.row, originPoint.col + offset.col};
    }

    // Distinct placements can coincide at only one relative square, if any.
    int meetingPoint(const vector<int>& belief) const {
        const Candidate& first = candidates[belief.front()];
        int a = first.origin;
        int b = first.alternate;
        if (b < 0) b = candidates[belief[1]].origin;
        int h1 = a / boardSize, h2 = b / boardSize;
        if (h1 == h2) return noTarget;
        Point p1 = coordinates(absolute[a / 4]);
        Point p2 = coordinates(absolute[b / 4]);
        Point x1 = rotateClockwise({1, 0}, h1), x2 = rotateClockwise({1, 0}, h2);
        Point y1 = rotateClockwise({0, 1}, h1), y2 = rotateClockwise({0, 1}, h2);
        Point x = {x1.row - x2.row, x1.col - x2.col};
        Point y = {y1.row - y2.row, y1.col - y2.col};
        Point delta = {p2.row - p1.row, p2.col - p1.col};
        int det = x.row * y.col - y.row * x.col;
        int rowNumerator = delta.row * y.col - y.row * delta.col;
        int colNumerator = x.row * delta.col - delta.row * x.col;
        if (rowNumerator % det || colNumerator % det) return noTarget;
        Point target = {rowNumerator / det, colNumerator / det};
        Point p = physicalPoint(a, target);
        if (p.row < 0 || p.row >= side || p.col < 0 || p.col >= side || !open[p.row * side + p.col]) {
            return noTarget;
        }
        for (int id : belief) {
            const Candidate& candidate = candidates[id];
            Point q = physicalPoint(candidate.origin, target);
            if (q.row != p.row || q.col != p.col) return noTarget;
            // In a solvable symmetric component, agreement with one other
            // member of the class forces the common square to be its center.
            if (candidate.alternate >= 0) {
                q = physicalPoint(candidate.alternate, target);
                if (q.row != p.row || q.col != p.col) return noTarget;
            }
        }
        return 4 * (target.row * side + target.col);
    }

    int singleCost(const Candidate& candidate, int relative) const {
        return centerDistance[candidate.origin + relative];
    }

    int centerAction(const Candidate& candidate, int relative) const {
        for (int action : {forwardStep, leftTurn, rightTurn}) {
            if (action == forwardStep && distance[candidate.origin + relative] == 0) continue;
            if (singleCost(candidate, transition(relative, action)) < singleCost(candidate, relative)) {
                return action;
            }
        }
        return leftTurn;
    }

    void addToBucket(Bucket& bucket, const Candidate& candidate, int state) const {
        bucket.weight += candidate.weight;
        ++bucket.groups;
        if (!candidate.solvable) return;
        bucket.solvableWeight += candidate.weight;
        bucket.groupLogs += candidate.weightLog;
        int cost = centerDistance[state];
        bucket.centerCost += cost * candidate.weight;
        int position = absolute[state / 4];
        if (bucket.position < 0) bucket.position = position;
        if (bucket.position != position || cost > 0) bucket.samePosition = false;
    }

    double entropy(const Bucket& bucket) const {
        if (bucket.solvableWeight == 0 || bucket.groups == 1 ||
            (bucket.solvableWeight == bucket.weight && bucket.samePosition)) return 0;
        return (weightLog[bucket.weight] - bucket.groupLogs -
                weightLog[bucket.weight - bucket.solvableWeight]) / bucket.weight;
    }

    Estimate estimate(const Bucket& bucket) const {
        if (bucket.solvableWeight == 0 || bucket.groups == 1 ||
            (bucket.solvableWeight == bucket.weight && bucket.samePosition)) {
            double cost = static_cast<double>(bucket.centerCost) / bucket.weight;
            return {cost, cost, cost};
        }
        double center = static_cast<double>(bucket.centerCost) / bucket.weight;
        double lower = max(1.0, center);
        return {max(lower, 1.36 * entropy(bucket)), lower, infinity};
    }

    Option evaluateTurn(const vector<int>& belief, int relative, int length, int node, int weight, double rootEntropy) {
        array<Bucket, 100> buckets;
        array<int, 100> active;
        int outcomes = 0;
        for (int id : belief) {
            const Candidate& candidate = candidates[id];
            int state = candidate.origin + relative;
            int d = distance[state];
            if (buckets[d].weight == 0) active[outcomes++] = d;
            addToBucket(buckets[d], candidate, state);
        }
        effort += 4 * belief.size();
        double guess = 0, lower = 0, upper = 0, remaining = 0;
        for (int i = 0; i < outcomes; ++i) {
            const Bucket& bucket = buckets[active[i]];
            Estimate ev = estimate(bucket);
            guess += bucket.weight * ev.guess;
            lower += bucket.weight * ev.lower;
            upper += bucket.weight * ev.upper;
            remaining += bucket.weight * entropy(bucket);
        }
        guess = length + guess / weight;
        lower = length + lower / weight;
        upper = length + upper / weight;
        double gain = max(0.01, rootEntropy - remaining / weight);
        double priority = min(upper, 0.40 * guess + 0.60 * length * rootEntropy / gain);
        return {guess, lower, upper, node, length, priority};
    }

    // Search the graph of poses reachable before the next informative turn.
    // An observation after a step is already determined by the previous one.
    void enumerate(const vector<int>& belief, int relative, int target, const Bucket& root,
                   int level, double bestUpper) {
        Workspace& ws = workspaces[level];
        ws.reset(boardSize);
        int origin = candidates[belief.front()].origin;
        int base = origin % boardSize;
        ws.add(base + relative, -1, 0, -1, true);
        double rootEntropy = entropy(root);
        long long startEffort = effort;
        for (size_t i = 0; i < ws.queue.size(); ++i) {
            int parent = ws.queue[i];
            Node cur = ws.nodes[parent];
            int rel = cur.index - base;
            if ((rel - (rel & 3)) == target) {
                double cost = cur.length;
                if (cost < bestUpper - tolerance || ws.options.empty()) {
                    ws.options.push_back({cost, cost, cost, parent, cur.length, cost});
                }
                break;
            }
            int length = cur.length + 1;
            if (cur.length >= bestUpper - tolerance) break;
            if (length >= bestUpper - tolerance) continue;
            if (!ws.options.empty()) {
                bool outOfBudget = effort >= searchLimit ||
                                   (level > 0 && effort - startEffort > 1300000);
                if (outOfBudget) {
                    // One marker records a lower bound for paths left unexplored.
                    ws.options.push_back({infinity, static_cast<double>(cur.length), infinity, -1, 0});
                    break;
                }
            }
            for (int action : {leftTurn, rightTurn, forwardStep}) {
                if (action == forwardStep && distance[origin + rel] == 0) continue;
                int next = transition(cur.index, action);
                if (ws.seen[next] == ws.stamp) continue;
                bool uniform = true;
                if (action != forwardStep) {
                    int nextRel = next - base;
                    int d = distance[origin + nextRel];
                    size_t j = 1;
                    while (j < belief.size() && distance[candidates[belief[j]].origin + nextRel] == d) {
                        ++j;
                    }
                    effort += j;
                    uniform = j == belief.size();
                }
                int id = ws.add(next, parent, length, action, uniform);
                if (!uniform) {
                    Option option = evaluateTurn(belief, next - base, length, id, root.weight, rootEntropy);
                    bestUpper = min(bestUpper, option.upper);
                    ws.options.push_back(option);
                }
            }
            ++effort;
        }
    }

    vector<unsigned char> path(const Workspace& ws, int node) const {
        vector<unsigned char> actions;
        for (int i = node; ws.nodes[i].parent >= 0; i = ws.nodes[i].parent) {
            actions.push_back(static_cast<unsigned char>(ws.nodes[i].action));
        }
        reverse(actions.begin(), actions.end());
        return actions;
    }

    Estimate evaluatePolicy(const vector<int>& belief, const Option& option, int relative,
                            int level, int strength, int totalWeight, double incumbent) {
        array<vector<int>, 100> children;
        array<Bucket, 100> buckets;
        array<int, 100> order;
        int numChildren = 0;
        for (int id : belief) {
            const Candidate& candidate = candidates[id];
            int state = candidate.origin + relative;
            int obs = distance[state];
            if (children[obs].empty()) order[numChildren++] = obs;
            children[obs].push_back(id);
            addToBucket(buckets[obs], candidate, state);
        }
        effort += 5 * belief.size();
        sort(order.begin(), order.begin() + numChildren, [&](int a, int b) {
            return buckets[a].weight > buckets[b].weight;
        });
        array<Estimate, 100> immediate;
        double sumLower = 0;
        for (int i = 0; i < numChildren; ++i) {
            const Bucket& bucket = buckets[order[i]];
            immediate[i] = estimate(bucket);
            sumLower += bucket.weight * immediate[i].lower;
        }
        double sumGuess = 0, sumUpper = 0;
        for (int i = 0; i < numChildren; ++i) {
            int obs = order[i];
            const Bucket& bucket = buckets[obs];
            Estimate baseEstimate = immediate[i];
            Estimate child = baseEstimate;
            if (baseEstimate.upper > baseEstimate.lower + tolerance) {
                child = score(children[obs], relative, level + 1, strength, nullptr);
            }
            sumLower += bucket.weight * (child.lower - baseEstimate.lower);
            sumGuess += bucket.weight * child.guess;
            sumUpper += bucket.weight * child.upper;
            if (option.cost + sumLower / totalWeight >= incumbent - tolerance) {
                return {infinity, option.cost + sumLower / totalWeight, infinity};
            }
        }
        return {option.cost + sumGuess / totalWeight, option.cost + sumLower / totalWeight,
                option.cost + sumUpper / totalWeight};
    }

    Estimate score(const vector<int>& belief, int relative, int level, int strength, vector<int>* plan) {
        Bucket root;
        uint64_t hash = mix(0x100000000ULL + static_cast<uint32_t>(relative));
        for (int id : belief) {
            const Candidate& candidate = candidates[id];
            addToBucket(root, candidate, candidate.origin + relative);
            hash ^= candidate.hash;
        }
        effort += 2 * belief.size();
        Estimate baseEstimate = estimate(root);
        if (baseEstimate.lower == baseEstimate.upper) return baseEstimate;

        double bestUpper = infinity;
        double savedLower = baseEstimate.lower;
        vector<unsigned char> savedActions;
        auto found = policies.find(hash);
        if (found != policies.end() && found->second.relative == relative && found->second.belief == belief) {
            const Policy& policy = found->second;
            if (policy.value.upper <= policy.value.lower + tolerance ||
                (level > 0 && (policy.strength >= strength || effort >= searchLimit)) ||
                strength == 0) {
                if (plan) plan->assign(policy.actions.begin(), policy.actions.end());
                return policy.value;
            }
            bestUpper = policy.value.upper;
            savedLower = policy.value.lower;
            savedActions = policy.actions;
        }
        if (level >= searchDepth || (level > 0 && effort >= searchLimit)) return baseEstimate;

        int target = root.weight == root.solvableWeight ? meetingPoint(belief) : noTarget;
        enumerate(belief, relative, target, root, level, bestUpper);
        Workspace& ws = workspaces[level];
        auto& options = ws.options;
        sort(options.begin(), options.end(), [](const Option& a, const Option& b) {
            if (a.priority != b.priority) return a.priority < b.priority;
            if (a.guess != b.guess) return a.guess < b.guess;
            if (a.lower != b.lower) return a.lower < b.lower;
            return a.cost < b.cost;
        });
        int anchor = candidates[belief.front()].origin % boardSize;
        int bestNode = -1;
        for (const Option& option : options) {
            if (option.upper < bestUpper - tolerance) {
                bestUpper = option.upper;
                bestNode = option.node;
            }
        }
        // First obtain complete policies cheaply, using the observation gain
        // per command to decide which alternatives to try first.
        constexpr array<int, 5> beams = {1, 7, 16, 36, 8};
        int beam = beams[strength];
        vector<pair<double, int>> refinements;
        double bestGuess = infinity;
        int guessNode = -1;
        int evaluated = 0;
        for (int i = 0; i < static_cast<int>(options.size()); ++i) {
            Option& option = options[i];
            if (option.node < 0 || option.lower >= bestUpper - tolerance) continue;
            double guess = max(option.lower, option.priority);
            if (option.upper > option.lower + tolerance && evaluated < beam && effort < searchLimit) {
                ++evaluated;
                int nextRel = ws.nodes[option.node].index - anchor;
                Estimate ev = evaluatePolicy(belief, option, nextRel, level, 0, root.weight, bestUpper);
                option.lower = max(option.lower, ev.lower);
                option.upper = min(option.upper, ev.upper);
                if (ev.upper < bestUpper - tolerance) {
                    bestUpper = ev.upper;
                    bestNode = option.node;
                }
                if (strength >= 2 && strength < exhaustive &&
                    option.lower < bestUpper - tolerance && isfinite(ev.upper)) {
                    refinements.push_back({ev.upper, i});
                }
                if (!isfinite(ev.upper) && isfinite(ev.guess)) guess = max(guess, ev.guess);
            }
            if (guess < bestGuess) {
                bestGuess = guess;
                guessNode = option.node;
            }
        }
        if (strength == exhaustive) {
            for (int i = 0; i < static_cast<int>(options.size()); ++i) {
                const Option& option = options[i];
                if (option.node >= 0 && option.lower < bestUpper - tolerance) {
                    refinements.push_back({min(option.upper, option.priority), i});
                }
            }
        }
        sort(refinements.begin(), refinements.end());
        int refinementCount = 0;
        for (const auto& entry : refinements) {
            if (effort >= searchLimit ||
                (strength < exhaustive && refinementCount >= (strength == 3 ? 7 : 3))) break;
            Option& option = options[entry.second];
            if (option.lower >= bestUpper - tolerance) continue;
            ++refinementCount;
            int nextRel = ws.nodes[option.node].index - anchor;
            int childStrength = strength == exhaustive ? exhaustive : strength - 1;
            Estimate ev = evaluatePolicy(belief, option, nextRel, level, childStrength, root.weight, bestUpper);
            option.lower = max(option.lower, ev.lower);
            if (ev.upper < bestUpper - tolerance) {
                bestUpper = ev.upper;
                bestNode = option.node;
            }
        }
        double bestLower = infinity;
        for (const Option& option : options) bestLower = min(bestLower, option.lower);
        bestLower = min(bestUpper, max(savedLower, bestLower));
        if (isfinite(bestUpper)) {
            if (bestNode >= 0) savedActions = path(ws, bestNode);
            if (plan) plan->assign(savedActions.begin(), savedActions.end());
            Estimate result = {bestUpper, bestLower, bestUpper};
            int completedStrength = effort >= searchLimit ? 0 : strength;
            if (savedCandidates < 2200000 && savedCommands < 4000000 && policies.size() < 65000) {
                auto entry = policies.find(hash);
                if (entry == policies.end()) {
                    savedCandidates += belief.size();
                    savedCommands += savedActions.size();
                    policies.emplace(hash, Policy{belief, move(savedActions), result, relative, completedStrength});
                } else if (entry->second.relative == relative && entry->second.belief == belief) {
                    Policy& policy = entry->second;
                    savedCommands -= policy.actions.size();
                    savedCommands += savedActions.size();
                    policy.actions = move(savedActions);
                    policy.value = result;
                    policy.strength = max(policy.strength, completedStrength);
                }
            }
            return result;
        }
        if (plan && guessNode >= 0) {
            vector<unsigned char> actions = path(ws, guessNode);
            plan->assign(actions.begin(), actions.end());
        }
        return {max(bestLower, isfinite(bestGuess) ? bestGuess : baseEstimate.guess), bestLower, infinity};
    }

public:
    Navigator(int rows, int cols, const vector<string>& map)
        : side(max(rows, cols)), area(side * side), boardSize(4 * area),
          open(area), rawDistance(boardSize), distance(4 * boardSize), centerDistance(4 * boardSize),
          absolute(boardSize), stride{-side, 1, side, -1}, weightLog(boardSize + 1) {
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) open[i * side + j] = map[i][j] == '.';
        }
        for (int i = 1; i <= boardSize; ++i) weightLog[i] = i * log2(static_cast<double>(i));
        buildDistances();
        buildCandidates();
    }

    void interact(int d) {
        vector<int> belief = move(initial[d]);
        int relative = 0;
        vector<int> plan;
        while (!belief.empty()) {
            bool anySolvable = false;
            bool same = true;
            int pos = absolute[(candidates[belief.front()].origin + relative) / 4];
            for (int id : belief) {
                const Candidate& candidate = candidates[id];
                anySolvable |= candidate.solvable;
                int state = candidate.origin + relative;
                if (!candidate.solvable || centerDistance[state] > 0 || absolute[state / 4] != pos) {
                    same = false;
                }
            }
            if (same) {
                cout << "yes " << pos / side + 1 << ' ' << pos % side + 1 << endl;
                return;
            }
            if (!anySolvable) {
                cout << "no" << endl;
                return;
            }
            plan.clear();
            if (belief.size() == 1) {
                plan.push_back(centerAction(candidates[belief.front()], relative));
            } else {
                auto started = chrono::steady_clock::now();
                if (thinkingTime > chrono::seconds(6)) caseLimit = 0;
                int strength = effort < caseLimit ? (belief.size() <= 64 ? exhaustive : 3) : 0;
                if (strength > 0) {
                    searchLimit = min(caseLimit, effort + (belief.size() < 70 ? 13000000 : 23500000));
                } else {
                    searchLimit = caseLimit == 0 ? effort :
                        effort + max(30000LL, 12LL * static_cast<int>(belief.size()));
                }
                score(belief, relative, 0, strength, &plan);
                thinkingTime += chrono::steady_clock::now() - started;
            }
            if (plan.empty()) return;
            for (size_t i = 0; i < plan.size(); ++i) {
                int action = plan[i];
                if (action == forwardStep && d == 0) return;
                cout << command[action] << endl;
                if (!(cin >> d) || d < 0 || d >= 100) return;
                relative = transition(relative, action);
                // The prefix before a planned distinguishing turn has the
                // same observations for every candidate.
                if (i + 1 < plan.size() && distance[candidates[belief.front()].origin + relative] == d) continue;
                auto end = remove_if(belief.begin(), belief.end(), [&](int id) {
                    return distance[candidates[id].origin + relative] != d;
                });
                if (end != belief.end()) {
                    belief.erase(end, belief.end());
                    break;
                }
            }
        }
    }
};

}  // namespace

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int rows, cols;
    if (!(cin >> rows >> cols)) return 0;
    vector<string> map(rows);
    for (string& row : map) {
        if (!(cin >> row)) return 0;
    }
    int d;
    if (!(cin >> d) || d < 0 || d >= 100) return 0;
    Navigator navigator(rows, cols, map);
    navigator.interact(d);
}

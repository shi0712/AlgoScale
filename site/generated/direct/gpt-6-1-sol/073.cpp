#include <bits/stdc++.h>
using namespace std;

using U64 = uint64_t;

static constexpr int MAXW = 157;
static constexpr int LOCAL = 205;
static constexpr int ORIGIN = 102;

static U64 observationMask[4][100][MAXW];

const int dr[4] = {-1, 0, 1, 0};
const int dc[4] = {0, 1, 0, -1};

struct Component {
    vector<int> cells;
    int shape[4]{};
    int minX[4]{}, minY[4]{};
    int family = -1;
    int center = -1;
    bool good = false;
};

struct Frame {
    int x, y;
    int entryDirection;
    int nextDirection = 0;
};

pair<int, int> rotateCoordinates(int x, int y, int direction) {
    if (direction == 0) return {x, y};
    if (direction == 1) return {-y, x};
    if (direction == 2) return {-x, -y};
    return {y, -x};
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int r, c;
    if (!(cin >> r >> c)) return 0;

    vector<string> grid(r);
    for (string &s : grid) cin >> s;

    const int area = r * c;
    const int words = (area + 63) / 64;

    vector<int> componentOf(area, -1);
    vector<Component> components;

    // Connected components.
    for (int p = 0; p < area; ++p) {
        if (grid[p / c][p % c] == '#' || componentOf[p] != -1)
            continue;

        int id = (int)components.size();
        components.emplace_back();

        queue<int> q;
        q.push(p);
        componentOf[p] = id;

        while (!q.empty()) {
            int v = q.front();
            q.pop();
            components[id].cells.push_back(v);

            int x = v / c, y = v % c;
            for (int d = 0; d < 4; ++d) {
                int nx = x + dr[d], ny = y + dc[d];
                if (nx < 0 || nx >= r || ny < 0 || ny >= c)
                    continue;
                int u = nx * c + ny;
                if (grid[nx][ny] == '.' && componentOf[u] == -1) {
                    componentOf[u] = id;
                    q.push(u);
                }
            }
        }
    }

    // Identify component shapes up to translation, separately for each rotation.
    map<vector<int>, int> shapeIds;
    int nextShapeId = 0;

    for (Component &comp : components) {
        for (int direction = 0; direction < 4; ++direction) {
            vector<pair<int, int>> points;
            int minX = INT_MAX, minY = INT_MAX;
            int maxX = INT_MIN, maxY = INT_MIN;

            for (int p : comp.cells) {
                auto [x, y] =
                    rotateCoordinates(p / c, p % c, direction);
                points.emplace_back(x, y);
                minX = min(minX, x);
                minY = min(minY, y);
                maxX = max(maxX, x);
                maxY = max(maxY, y);
            }

            comp.minX[direction] = minX;
            comp.minY[direction] = minY;

            vector<int> encoded;
            encoded.reserve(points.size());
            for (auto [x, y] : points)
                encoded.push_back((x - minX) * 100 + (y - minY));
            sort(encoded.begin(), encoded.end());

            vector<int> key;
            key.reserve(encoded.size() + 2);
            key.push_back(maxX - minX + 1);
            key.push_back(maxY - minY + 1);
            key.insert(key.end(), encoded.begin(), encoded.end());

            auto [it, inserted] = shapeIds.emplace(key, nextShapeId);
            if (inserted) ++nextShapeId;
            comp.shape[direction] = it->second;
        }

        comp.family = *min_element(comp.shape, comp.shape + 4);
    }

    unordered_map<int, int> familyCount;
    for (const Component &comp : components)
        ++familyCount[comp.family];

    // A component is localizable precisely when it has no congruent other
    // component, and all its rotational symmetries have a common open cell.
    for (Component &comp : components) {
        if (familyCount[comp.family] != 1) continue;

        int symmetryCount = 0;
        for (int d = 0; d < 4; ++d)
            symmetryCount += (comp.shape[d] == comp.shape[0]);

        if (symmetryCount == 1) {
            comp.good = true;
            comp.center = comp.cells.front();
            continue;
        }

        int minX = r, maxX = -1, minY = c, maxY = -1;
        for (int p : comp.cells) {
            minX = min(minX, p / c);
            maxX = max(maxX, p / c);
            minY = min(minY, p % c);
            maxY = max(maxY, p % c);
        }

        if ((minX + maxX) % 2 == 0 && (minY + maxY) % 2 == 0) {
            int x = (minX + maxX) / 2;
            int y = (minY + maxY) / 2;
            int p = x * c + y;
            if (componentOf[p] == componentOf[comp.cells.front()]) {
                comp.good = true;
                comp.center = p;
            }
        }
    }

    // Behavioral-equivalence labels for oriented states.
    vector<vector<int>> label(4, vector<int>(area, -1));
    unordered_map<long long, int> labelIds;
    labelIds.reserve(4 * area);
    int nextLabelId = 0;

    for (const Component &comp : components) {
        for (int d = 0; d < 4; ++d) {
            for (int p : comp.cells) {
                auto [x, y] = rotateCoordinates(p / c, p % c, d);
                x -= comp.minX[d];
                y -= comp.minY[d];

                long long key =
                    (long long)comp.shape[d] * 10000 + x * 100 + y;

                auto [it, inserted] = labelIds.emplace(key, nextLabelId);
                if (inserted) ++nextLabelId;
                label[d][p] = it->second;
            }
        }
    }

    // Wall distances for all oriented states.
    vector<vector<int>> distance(4, vector<int>(area, 0));

    for (int x = 0; x < r; ++x) {
        for (int y = 0; y < c; ++y) {
            if (grid[x][y] == '#') continue;
            int p = x * c + y;
            if (x > 0 && grid[x - 1][y] == '.')
                distance[0][p] = distance[0][p - c] + 1;
            if (y > 0 && grid[x][y - 1] == '.')
                distance[3][p] = distance[3][p - 1] + 1;
        }
    }
    for (int x = r - 1; x >= 0; --x) {
        for (int y = c - 1; y >= 0; --y) {
            if (grid[x][y] == '#') continue;
            int p = x * c + y;
            if (x + 1 < r && grid[x + 1][y] == '.')
                distance[2][p] = distance[2][p + c] + 1;
            if (y + 1 < c && grid[x][y + 1] == '.')
                distance[1][p] = distance[1][p + 1] + 1;
        }
    }

    array<vector<U64>, 4> possible;
    for (auto &bits : possible) bits.assign(words, 0);

    vector<U64> goodMask(words, 0);

    for (int p = 0; p < area; ++p) {
        if (componentOf[p] == -1) continue;

        U64 bit = U64(1) << (p & 63);
        int w = p >> 6;

        if (components[componentOf[p]].good)
            goodMask[w] |= bit;

        for (int d = 0; d < 4; ++d) {
            possible[d][w] |= bit;
            observationMask[d][distance[d][p]][w] |= bit;
        }
    }

    // Map in coordinates relative to the initial robot state:
    // 0 = unknown, 1 = open, 2 = wall.
    static unsigned char known[LOCAL][LOCAL];
    static bool visited[LOCAL][LOCAL];

    int localX = ORIGIN, localY = ORIGIN, localDirection = 0;
    known[localX][localY] = 1;
    visited[localX][localY] = true;

    vector<Frame> dfs;
    dfs.push_back({localX, localY, localDirection, 0});

    vector<int> navigation;
    size_t navigationIndex = 0;

    auto shiftBits = [&](vector<U64> &bits, int amount) {
        vector<U64> result(words, 0);
        int shift = abs(amount);
        int whole = shift / 64;
        int part = shift % 64;

        if (amount > 0) {
            for (int i = 0; i < words; ++i) {
                int j = i + whole;
                if (j < words) {
                    result[j] |= bits[i] << part;
                    if (part && j + 1 < words)
                        result[j + 1] |= bits[i] >> (64 - part);
                }
            }
        } else {
            for (int i = 0; i < words; ++i) {
                int j = i - whole;
                if (j >= 0) {
                    result[j] |= bits[i] >> part;
                    if (part && j - 1 >= 0)
                        result[j - 1] |= bits[i] << (64 - part);
                }
            }
        }

        if (area % 64)
            result.back() &= (U64(1) << (area % 64)) - 1;
        bits.swap(result);
    };

    // Commands: 0 = left, 1 = right, 2 = step.
    auto execute = [&](int command) {
        if (command == 0) {
            auto old = move(possible[0]);
            possible[0] = move(possible[1]);
            possible[1] = move(possible[2]);
            possible[2] = move(possible[3]);
            possible[3] = move(old);

            localDirection = (localDirection + 3) % 4;
            cout << "left" << endl;
        } else if (command == 1) {
            auto old = move(possible[3]);
            possible[3] = move(possible[2]);
            possible[2] = move(possible[1]);
            possible[1] = move(possible[0]);
            possible[0] = move(old);

            localDirection = (localDirection + 1) % 4;
            cout << "right" << endl;
        } else {
            shiftBits(possible[0], -c);
            shiftBits(possible[1], 1);
            shiftBits(possible[2], c);
            shiftBits(possible[3], -1);

            localX += dr[localDirection];
            localY += dc[localDirection];
            cout << "step" << endl;
        }
    };

    auto turnToward = [&](int target) {
        int difference = (target - localDirection + 4) % 4;
        return difference == 3 ? 0 : 1;
    };

    auto makeNavigation = [&](int start, int direction) {
        int goal = components[componentOf[start]].center;
        vector<int> parent(area, -1), parentDirection(area, -1);
        queue<int> q;
        q.push(start);
        parent[start] = start;

        while (!q.empty() && parent[goal] == -1) {
            int p = q.front();
            q.pop();
            int x = p / c, y = p % c;

            for (int d = 0; d < 4; ++d) {
                int nx = x + dr[d], ny = y + dc[d];
                if (nx < 0 || nx >= r || ny < 0 || ny >= c)
                    continue;
                int u = nx * c + ny;
                if (grid[nx][ny] == '.' && parent[u] == -1) {
                    parent[u] = p;
                    parentDirection[u] = d;
                    q.push(u);
                }
            }
        }

        vector<int> path;
        for (int p = goal; p != start; p = parent[p])
            path.push_back(parentDirection[p]);
        reverse(path.begin(), path.end());

        navigation.clear();
        navigationIndex = 0;

        for (int target : path) {
            int difference = (target - direction + 4) % 4;
            if (difference == 3) {
                navigation.push_back(0);
            } else {
                for (int k = 0; k < difference; ++k)
                    navigation.push_back(1);
            }
            navigation.push_back(2);
            direction = target;
        }
    };

    while (true) {
        int observation;
        if (!(cin >> observation)) return 0;

        for (int d = 0; d < 4; ++d)
            for (int w = 0; w < words; ++w)
                possible[d][w] &= observationMask[d][observation][w];

        bool anyGood = false;
        int uniquePosition = -1;
        bool multiplePositions = false;
        int stateCount = 0;
        vector<pair<int, int>> smallStates;
        smallStates.reserve(5);

        for (int w = 0; w < words; ++w) {
            U64 positions = 0;
            for (int d = 0; d < 4; ++d) {
                U64 bits = possible[d][w];
                positions |= bits;
                anyGood |= (bits & goodMask[w]) != 0;

                if (stateCount <= 4) {
                    U64 remaining = bits;
                    while (remaining && stateCount <= 4) {
                        int b = countr_zero(remaining);
                        smallStates.emplace_back(d, 64 * w + b);
                        ++stateCount;
                        remaining &= remaining - 1;
                    }
                }
            }

            if (positions) {
                int p = 64 * w + countr_zero(positions);
                if (uniquePosition == -1)
                    uniquePosition = p;
                else if (uniquePosition != p)
                    multiplePositions = true;

                if (positions & (positions - 1))
                    multiplePositions = true;
            }
        }

        if (!multiplePositions && uniquePosition != -1) {
            cout << "yes " << uniquePosition / c + 1
                 << ' ' << uniquePosition % c + 1 << endl;
            return 0;
        }

        if (!anyGood) {
            cout << "no" << endl;
            return 0;
        }

        // Every observation also reveals an entire open ray and its end wall.
        for (int k = 0; k <= observation; ++k) {
            int x = localX + k * dr[localDirection];
            int y = localY + k * dc[localDirection];
            known[x][y] = 1;
        }
        known[localX + (observation + 1) * dr[localDirection]]
             [localY + (observation + 1) * dc[localDirection]] = 2;

        if (navigationIndex < navigation.size()) {
            execute(navigation[navigationIndex++]);
            continue;
        }

        // A good behavioral class contains at most four oriented states.
        if (stateCount <= 4 && !smallStates.empty()) {
            int firstLabel =
                label[smallStates[0].first][smallStates[0].second];
            bool sameClass = true;
            for (auto [d, p] : smallStates)
                sameClass &= label[d][p] == firstLabel;

            if (sameClass) {
                auto [d, p] = smallStates[0];
                makeNavigation(p, d);
                if (navigationIndex < navigation.size()) {
                    execute(navigation[navigationIndex++]);
                    continue;
                }
            }
        }

        // DFS, considering directions right, forward, left, backward
        // relative to the direction on first entering this square.
        bool issued = false;

        while (!issued) {
            Frame &frame = dfs.back();

            if (frame.nextDirection < 4) {
                static constexpr int offset[4] = {1, 0, 3, 2};
                int target =
                    (frame.entryDirection + offset[frame.nextDirection]) % 4;
                int nx = localX + dr[target];
                int ny = localY + dc[target];

                if (known[nx][ny] == 2 || visited[nx][ny]) {
                    ++frame.nextDirection;
                    continue;
                }

                if (localDirection != target) {
                    execute(turnToward(target));
                    issued = true;
                    continue;
                }

                // Facing this direction has just revealed its neighbor.
                if (known[nx][ny] == 1) {
                    ++frame.nextDirection;
                    visited[nx][ny] = true;
                    dfs.push_back({nx, ny, target, 0});
                    execute(2);
                    issued = true;
                } else {
                    ++frame.nextDirection;
                }
            } else {
                // At a completed exploration, the equivalence-class test
                // above has already supplied the final navigation.
                if (dfs.size() == 1) return 0;

                int target = (frame.entryDirection + 2) % 4;
                if (localDirection != target) {
                    execute(turnToward(target));
                    issued = true;
                } else {
                    dfs.pop_back();
                    execute(2);
                    issued = true;
                }
            }
        }
    }
}

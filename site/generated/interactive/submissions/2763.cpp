#include <bits/stdc++.h>
using namespace std;

class Solver {
    struct Key {
        int x, y, action, h;
        vector<pair<int, int>> extra;

        bool operator==(const Key& other) const {
            return x == other.x && y == other.y &&
                   action == other.action && h == other.h &&
                   extra == other.extra;
        }
    };

    struct KeyHash {
        size_t operator()(const Key& k) const {
            size_t h = static_cast<size_t>(k.x) * 1000003ULL;
            h ^= static_cast<size_t>(k.y) + 0x9e3779b9ULL + (h << 6) + (h >> 2);
            h ^= static_cast<size_t>(k.action) + 0x9e3779b9ULL + (h << 6) + (h >> 2);
            h ^= static_cast<size_t>(k.h) + 0x9e3779b9ULL + (h << 6) + (h >> 2);
            for (auto [x, y] : k.extra) {
                h ^= static_cast<size_t>(x) * 1000003ULL + 0x9e3779b9ULL + (h << 6) + (h >> 2);
                h ^= static_cast<size_t>(y) + 0x9e3779b9ULL + (h << 6) + (h >> 2);
            }
            return h;
        }
    };

    struct Pos {
        int x, y;
    };

    unordered_set<uint64_t> black;
    unordered_map<uint64_t, int> visits;
    unordered_map<Key, bool, KeyHash> winMemo;
    unordered_map<Key, long double, KeyHash> evalMemo;

    long long markedCount[8][8]{};

    static uint64_t encode(int x, int y) {
        return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) |
               static_cast<uint32_t>(y);
    }

    static bool validMark(int x, int y) {
        return 1 <= x && x <= 1000 && 1 <= y && y <= 1000;
    }

    bool isBlack(int x, int y, const vector<pair<int, int>>& extra) const {
        if (black.count(encode(x, y))) return true;
        for (auto [a, b] : extra) {
            if (a == x && b == y) return true;
        }
        return false;
    }

    vector<Pos> neighbors(int x, int y) const {
        static const int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        static const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};
        vector<Pos> result;
        for (int d = 0; d < 8; ++d) {
            int nx = x + dx[d];
            int ny = y + dy[d];
            if (nx > 0 && ny > 0) result.push_back({nx, ny});
        }
        return result;
    }

    Key makeKey(int x, int y, int action, int h,
                vector<pair<int, int>> extra) const {
        sort(extra.begin(), extra.end());
        return {x, y, action, h, move(extra)};
    }

    int blackAdjacent(int x, int y, const vector<pair<int, int>>& extra) const {
        int result = 0;
        for (auto p : neighbors(x, y)) {
            result += isBlack(p.x, p.y, extra);
        }
        return result;
    }

    int nearbyBlack(int x, int y, const vector<pair<int, int>>& extra) const {
        int result = 0;
        for (int nx = max(1, x - 2); nx <= x + 2; ++nx) {
            for (int ny = max(1, y - 2); ny <= y + 2; ++ny) {
                result += isBlack(nx, ny, extra);
            }
        }
        return result;
    }

    long double baseScore(int x, int y,
                          const vector<pair<int, int>>& extra) const {
        int degree = 0;
        for (auto p : neighbors(x, y)) {
            degree += !isBlack(p.x, p.y, extra);
        }
        if (degree == 0) return -1000000.0L;

        long double distance = max(x - 1, y - 1);
        int adjacent = blackAdjacent(x, y, extra);
        int nearby = nearbyBlack(x, y, extra);

        return 20.0L * degree + 0.45L * distance
             - 18.0L * adjacent - 1.5L * nearby;
    }

    long double actionUtility(int x, int y, int qx, int qy) const {
        static const int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        static const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

        int markedDirection = -1;
        for (int d = 0; d < 8; ++d) {
            if (x + dx[d] == qx && y + dy[d] == qy) {
                markedDirection = d;
                break;
            }
        }

        long double result = 0;
        if (markedDirection != -1) {
            long double total = 0;
            for (int d = 0; d < 8; ++d) {
                total += markedCount[markedDirection][d] + 0.1;
            }

            for (int d = 0; d < 8; ++d) {
                if (d == markedDirection) continue;
                int rx = x + dx[d];
                int ry = y + dy[d];
                if (rx <= 0 || ry <= 0) continue;

                long double probability =
                    (markedCount[markedDirection][d] + 0.1) / total;

                if (abs(qx - rx) <= 1 && abs(qy - ry) <= 1) {
                    result += probability;
                }
            }
        }

        auto it = visits.find(encode(qx, qy));
        if (it != visits.end()) result += 4.0L * it->second;

        result += 0.4L * blackAdjacent(qx, qy, {});
        return result;
    }

    long double evaluateState(int x, int y, int depth,
                              vector<pair<int, int>> extra) {
        Key key = makeKey(x, y, -1, depth, extra);
        auto it = evalMemo.find(key);
        if (it != evalMemo.end()) return it->second;

        vector<pair<int, int>> candidates;
        for (auto p : neighbors(x, y)) {
            if (validMark(p.x, p.y) &&
                !isBlack(p.x, p.y, extra)) {
                candidates.push_back({p.x, p.y});
            }
        }

        if (candidates.empty()) {
            long double value = baseScore(x, y, extra);
            evalMemo.emplace(move(key), value);
            return value;
        }

        long double best = numeric_limits<long double>::infinity();
        long double bestAverage = numeric_limits<long double>::infinity();
        long double bestUtility = -numeric_limits<long double>::infinity();

        for (auto [qx, qy] : candidates) {
            vector<pair<int, int>> nextExtra = extra;
            nextExtra.push_back({qx, qy});

            vector<Pos> responses;
            for (auto p : neighbors(x, y)) {
                if (p.x == qx && p.y == qy) continue;
                if (!isBlack(p.x, p.y, nextExtra)) {
                    responses.push_back(p);
                }
            }

            if (responses.empty()) {
                best = -1000000.0L;
                bestAverage = -1000000.0L;
                bestUtility = actionUtility(x, y, qx, qy);
                break;
            }

            long double worst = -numeric_limits<long double>::infinity();
            long double sum = 0;

            for (auto p : responses) {
                long double value;
                if (depth <= 1) {
                    value = baseScore(p.x, p.y, nextExtra);
                } else {
                    value = evaluateState(p.x, p.y, depth - 1, nextExtra);
                }
                worst = max(worst, value);
                sum += value;
            }

            long double average = sum / responses.size();
            long double combined = 0.8L * worst + 0.2L * average;
            long double utility = actionUtility(x, y, qx, qy);

            if (combined < best - 1e-12L ||
                (abs(combined - best) <= 1e-12L &&
                 (average < bestAverage - 1e-12L ||
                  (abs(average - bestAverage) <= 1e-12L &&
                   utility > bestUtility)))) {
                best = combined;
                bestAverage = average;
                bestUtility = utility;
            }
        }

        evalMemo.emplace(move(key), best);
        return best;
    }

    bool canWinState(int x, int y, int depth,
                     vector<pair<int, int>> extra) {
        if (depth <= 0) return false;

        Key key = makeKey(x, y, -1, depth, extra);
        auto it = winMemo.find(key);
        if (it != winMemo.end()) return it->second;

        for (auto p : neighbors(x, y)) {
            if (!validMark(p.x, p.y) ||
                isBlack(p.x, p.y, extra)) {
                continue;
            }

            if (actionWins(x, y, p.x, p.y, depth, extra)) {
                winMemo.emplace(move(key), true);
                return true;
            }
        }

        winMemo.emplace(move(key), false);
        return false;
    }

    bool actionWins(int x, int y, int qx, int qy, int depth,
                    vector<pair<int, int>> extra) {
        if (depth <= 0) return false;

        Key key = makeKey(x, y, encode(qx, qy), depth, extra);
        auto it = winMemo.find(key);
        if (it != winMemo.end()) return it->second;

        vector<pair<int, int>> nextExtra = extra;
        nextExtra.push_back({qx, qy});

        bool result = true;
        for (auto p : neighbors(x, y)) {
            if (p.x == qx && p.y == qy) continue;
            if (isBlack(p.x, p.y, nextExtra)) continue;

            if (!canWinState(p.x, p.y, depth - 1, nextExtra)) {
                result = false;
                break;
            }
        }

        winMemo.emplace(move(key), result);
        return result;
    }

    pair<int, int> chooseMark(int x, int y) {
        winMemo.clear();
        evalMemo.clear();

        vector<pair<int, int>> candidates;
        for (auto p : neighbors(x, y)) {
            if (validMark(p.x, p.y) &&
                !isBlack(p.x, p.y, {})) {
                candidates.push_back({p.x, p.y});
            }
        }

        if (candidates.empty()) {
            int cx = min(1000, max(1, x));
            int cy = min(1000, max(1, y));
            for (int radius = 1; radius <= 1000; ++radius) {
                for (int nx = max(1, cx - radius);
                     nx <= min(1000, cx + radius); ++nx) {
                    for (int ny = max(1, cy - radius);
                         ny <= min(1000, cy + radius); ++ny) {
                        if (!black.count(encode(nx, ny))) {
                            return {nx, ny};
                        }
                    }
                }
            }
            return {1, 1};
        }

        pair<int, int> winningChoice = {-1, -1};
        long double bestWinningScore = numeric_limits<long double>::infinity();

        for (auto [qx, qy] : candidates) {
            if (!actionWins(x, y, qx, qy, 5, {})) continue;

            vector<pair<int, int>> extra = {{qx, qy}};
            long double score = 0;
            bool first = true;
            for (auto p : neighbors(x, y)) {
                if (p.x == qx && p.y == qy) continue;
                if (isBlack(p.x, p.y, extra)) continue;
                long double value = baseScore(p.x, p.y, extra);
                if (first || value > score) score = value;
                first = false;
            }
            if (first) score = -1000000.0L;

            if (score < bestWinningScore) {
                bestWinningScore = score;
                winningChoice = {qx, qy};
            }
        }

        if (winningChoice.first != -1) return winningChoice;

        pair<int, int> best = candidates.front();
        long double bestCombined = numeric_limits<long double>::infinity();
        long double bestAverage = numeric_limits<long double>::infinity();
        long double bestUtility = -numeric_limits<long double>::infinity();

        for (auto [qx, qy] : candidates) {
            vector<pair<int, int>> extra = {{qx, qy}};
            vector<Pos> responses;

            for (auto p : neighbors(x, y)) {
                if (p.x == qx && p.y == qy) continue;
                if (!isBlack(p.x, p.y, extra)) responses.push_back(p);
            }

            if (responses.empty()) return {qx, qy};

            long double worst = -numeric_limits<long double>::infinity();
            long double sum = 0;

            for (auto p : responses) {
                long double value = evaluateState(p.x, p.y, 1, extra);
                worst = max(worst, value);
                sum += value;
            }

            long double average = sum / responses.size();
            long double combined = 0.8L * worst + 0.2L * average;
            long double utility = actionUtility(x, y, qx, qy);

            if (combined < bestCombined - 1e-12L ||
                (abs(combined - bestCombined) <= 1e-12L &&
                 (average < bestAverage - 1e-12L ||
                  (abs(average - bestAverage) <= 1e-12L &&
                   utility > bestUtility)))) {
                bestCombined = combined;
                bestAverage = average;
                bestUtility = utility;
                best = {qx, qy};
            }
        }

        return best;
    }

public:
    void run() {
        int x, y;
        if (!(cin >> x >> y)) return;
        if (x == 0 && y == 0) return;
        if (x <= 0 || y <= 0) return;

        visits[encode(x, y)] = 1;

        while (true) {
            auto [mx, my] = chooseMark(x, y);
            black.insert(encode(mx, my));

            cout << mx << ' ' << my << endl;

            int nx, ny;
            if (!(cin >> nx >> ny)) return;
            if (nx == 0 && ny == 0) return;
            if (nx < 0 || ny < 0) return;
            if (nx == 0 || ny == 0) return;

            static const int dx[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
            static const int dy[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

            int markedDirection = -1;
            for (int d = 0; d < 8; ++d) {
                if (x + dx[d] == mx && y + dy[d] == my) {
                    markedDirection = d;
                    break;
                }
            }

            int responseDirection = -1;
            for (int d = 0; d < 8; ++d) {
                if (x + dx[d] == nx && y + dy[d] == ny) {
                    responseDirection = d;
                    break;
                }
            }

            if (markedDirection != -1 && responseDirection != -1) {
                ++markedCount[markedDirection][responseDirection];
            }

            ++visits[encode(nx, ny)];
            x = nx;
            y = ny;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    Solver solver;
    solver.run();
    return 0;
}

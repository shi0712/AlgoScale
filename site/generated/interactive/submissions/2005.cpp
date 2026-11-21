#include <bits/stdc++.h>
using namespace std;

struct Cell {
    int x = 0, y = 0;
    bool operator==(const Cell&) const = default;
};

constexpr int LIMIT = 1000;
constexpr int MAX_SIDE = 32;
constexpr int CAPTURE_DEPTH = 6;
constexpr int WINDOW = 5;
constexpr int MASKS = 1 << (2 * WINDOW + 1);
constexpr int MIDDLE = 7 << (WINDOW - 1);
constexpr int dx[] = {-1,-1,-1,0,0,1,1,1};
constexpr int dy[] = {-1,0,1,-1,1,-1,0,1};

bool black[LIMIT + 2][LIMIT + 2];
int wallRank[MASKS], wallStep[MASKS];
int rectangleCost[MAX_SIDE + 1][MAX_SIDE + 1];

void prepareTables() {
    fill(wallRank, wallRank + MASKS, 5);
    for (int mask = 0; mask < MASKS; ++mask) {
        if (popcount(unsigned(mask & MIDDLE)) >= 2) wallRank[mask] = 0;
    }
    constexpr int offsets[] = {0,1,-1,2,-2,3,-3,4,-4,5,-5};
    for (int t = 1; t <= 4; ++t) {
        for (int mask = 0; mask < MASKS; ++mask) {
            if (wallRank[mask] != 5) continue;
            for (int offset : offsets) {
                if (abs(offset) > t + 1) break;
                int bit = 1 << (WINDOW + offset);
                if (mask & bit) continue;
                int next = mask | bit;
                if (wallRank[next] < t && wallRank[next >> 1] < t &&
                    wallRank[(next << 1) & (MASKS - 1)] < t) {
                    wallRank[mask] = t;
                    wallStep[mask] = offset;
                    break;
                }
            }
        }
    }
    assert(wallRank[0] == 4);
    for (int w = 1; w <= MAX_SIDE; ++w) {
        for (int h = 1; h <= MAX_SIDE; ++h) {
            int best = w * h;
            for (int i = 0; i < w; ++i)
                best = min(best, h + max(rectangleCost[i][h], rectangleCost[w-i-1][h]));
            for (int i = 0; i < h; ++i)
                best = min(best, w + max(rectangleCost[w][i], rectangleCost[w][h-i-1]));
            rectangleCost[w][h] = best;
        }
    }
}

optional<int> wallPlacement(int mask) {
    if (wallRank[mask] > 0) return wallStep[mask];
    unsigned missing = MIDDLE & ~mask;
    if (missing) return int(countr_zero(missing)) - WINDOW;
    return nullopt;
}

bool white(Cell p) {
    return p.x > 0 && p.y > 0 && p.x <= LIMIT && p.y <= LIMIT && !black[p.x][p.y];
}

int neighbors(Cell p, array<Cell, 8>& result) {
    int n = 0;
    for (int i = 0; i < 8; ++i) {
        Cell q{p.x + dx[i], p.y + dy[i]};
        if (white(q)) result[n++] = q;
    }
    return n;
}

int distance(Cell a, Cell b) {
    return max(abs(a.x - b.x), abs(a.y - b.y));
}

bool readPosition(Cell& p) {
    if (!(cin >> p.x) || p.x < 0) return false;
    return (cin >> p.y) && p.x > 0 && p.y > 0;
}

struct CaptureSearch {
    int budget = 0;

    // At move i, at most i of the currently white cells can have been
    // deleted. This backward count certifies some positions as survivable.
    bool canSurvive(Cell p, int turns) const {
        constexpr int span = 2 * CAPTURE_DEPTH + 3;
        bool layers[2][span][span]{};
        unsigned char freeCell[span][span]{};
        int mid = turns + 1;
        for (int i = -turns; i <= turns; ++i) {
            for (int j = -turns; j <= turns; ++j) {
                bool open = white({p.x+i, p.y+j});
                layers[0][mid+i][mid+j] = open;
                freeCell[mid+i][mid+j] = open;
            }
        }
        int layer = 0;
        for (int step = turns; step >= 1; --step) {
            for (int i = mid-step+1; i <= mid+step-1; ++i) {
                for (int j = mid-step+1; j <= mid+step-1; ++j) {
                    int count = 0;
                    if (freeCell[i][j]) {
                        for (int k = 0; k < 8; ++k)
                            count += layers[layer][i+dx[k]][j+dy[k]];
                    }
                    layers[layer^1][i][j] = count > step;
                }
            }
            layer ^= 1;
        }
        return layers[layer][mid][mid];
    }

    Cell solve(Cell p, int turns) {
        array<Cell, 8> next{};
        int n = neighbors(p, next);
        if (n == 0) return p;
        if (n == 1) return next[0];
        if (turns == 1 || --budget < 0 || canSurvive(p, turns)) return {};

        struct Candidate {
            int score;
            Cell p;
        };
        constexpr int side = 2 * CAPTURE_DEPTH + 1;
        array<Candidate, side * side> candidates{};
        int count = 0;
        for (int i = -turns; i <= turns; ++i) {
            for (int j = -turns; j <= turns; ++j) {
                Cell q{p.x+i, p.y+j};
                if (!white(q)) continue;
                int score = 0;
                for (int k = 0; k < n; ++k) {
                    int d = distance(next[k], q);
                    if (d == 0) score += 8;
                    if (d == 1) ++score;
                }
                if (score || turns > 2) candidates[count++] = {score, q};
            }
        }
        sort(candidates.begin(), candidates.begin() + count,
             [](const Candidate& a, const Candidate& b) { return a.score > b.score; });
        for (int i = 0; i < count && budget >= 0; ++i) {
            Cell q = candidates[i].p;
            black[q.x][q.y] = true;
            array<pair<int, int>, 8> order{};
            int size = 0;
            for (int k = 0; k < n; ++k) {
                if (next[k] == q) continue;
                array<Cell, 8> around{};
                pair<int, int> reply{-neighbors(next[k], around), k};
                int j = size++;
                while (j > 0 && reply < order[j-1]) {
                    order[j] = order[j-1];
                    --j;
                }
                order[j] = reply;
            }
            bool wins = true;
            for (int k = 0; k < size && wins; ++k)
                wins = solve(next[order[k].second], turns - 1).x != 0;
            black[q.x][q.y] = false;
            if (wins) return q;
        }
        return {};
    }

    Cell find(Cell p, int maxTurns, int& usedTurns) {
        for (int t = 1; t <= maxTurns; ++t) {
            budget = 700;
            Cell q = solve(p, t);
            if (q.x) {
                usedTurns = t;
                return q;
            }
        }
        return {};
    }
};

class Strategy {
    const int right, top;
    Cell robot;
    int seen[MAX_SIDE + 1][MAX_SIDE + 1]{};
    int generation = 0;
    int edgeAttacks = 0, localAttacks = 0;
    array<Cell, 6> history{};
    int historySize = 0;
    struct Pocket {
        int x = 0, y = 0, east = 0, north = 0;
        bool contains(Cell p) const {
            return east * (p.x - x) < 0 && north * (p.y - y) < 0;
        }
    };
    Pocket pocket;
    int pockets = 0;

    Cell wallCell(bool vertical, int offset) const {
        return vertical ? Cell{right, offset} : Cell{offset, top};
    }

    // A mask is centered at the projection onto a wall. Its rank is the
    // number of preparatory marks needed to cover two of the three cells
    // the robot could use to cross, against all shifts of that projection.
    int mask(Cell p, bool vertical) const {
        int center = vertical ? p.y : p.x;
        int end = vertical ? top : right;
        int result = 0;
        for (int i = -WINDOW; i <= WINDOW; ++i) {
            int s = center + i;
            Cell q = wallCell(vertical, s);
            if (s <= 0 || s > end || black[q.x][q.y])
                result |= 1 << (i + WINDOW);
        }
        return result;
    }

    bool nearestWall(Cell p) const {
        return right - p.x <= top - p.y;
    }

    // At distance d, a rank below d leaves time to prepare and then cover
    // the missing crossing cell. Test this for each possible white reply.
    bool safe(Cell action) const {
        black[action.x][action.y] = true;
        bool result = true;
        for (int i = 0; i < 8 && result; ++i) {
            Cell q{robot.x + dx[i], robot.y + dy[i]};
            if (!white(q)) continue;
            int d = min(right - q.x, top - q.y);
            result = d > 0 && (d >= 5 || wallRank[mask(q, nearestWall(q))] < d);
        }
        black[action.x][action.y] = false;
        return result;
    }

    // A change of nearest wall can only happen close to the corner when
    // its rank matters. The premarked arms cover those switches. Otherwise
    // this action reduces the current wall's rank or keeps it at zero.
    Cell guard() const {
        bool vertical = nearestWall(robot);
        int middle = vertical ? robot.y : robot.x;
        auto step = wallPlacement(mask(robot, vertical));
        return step ? wallCell(vertical, middle + *step) : Cell{};
    }

    Cell chooseCut() {
        int lowX = robot.x, highX = robot.x, lowY = robot.y, highY = robot.y;
        array<Cell, MAX_SIDE * MAX_SIDE> component{};
        array<int, MAX_SIDE + 1> columns{}, rows{};
        array<Cell, MAX_SIDE + 1> onColumn{}, onRow{};
        ++generation;
        seen[robot.x][robot.y] = generation;
        component[0] = robot;
        int size = 1;
        for (int i = 0; i < size; ++i) {
            Cell p = component[i];
            lowX = min(lowX, p.x); highX = max(highX, p.x);
            lowY = min(lowY, p.y); highY = max(highY, p.y);
            if (!columns[p.x]++ || abs(p.y - robot.y) < abs(onColumn[p.x].y - robot.y))
                onColumn[p.x] = p;
            if (!rows[p.y]++ || abs(p.x - robot.x) < abs(onRow[p.y].x - robot.x))
                onRow[p.y] = p;
            for (int k = 0; k < 8; ++k) {
                Cell q{p.x + dx[k], p.y + dy[k]};
                if (q.x <= 0 || q.x >= right || q.y <= 0 || q.y >= top ||
                    seen[q.x][q.y] == generation || black[q.x][q.y]) continue;
                seen[q.x][q.y] = generation;
                component[size++] = q;
            }
        }
        array<int, 4> best{INT_MAX, 0, 0, 0};
        Cell answer{};
        auto tryCut = [&](bool vertical, int location, int length, Cell mark,
                          int leftCost, int rightCost) {
            int a = vertical ? robot.x : robot.y;
            int b = vertical ? robot.y : robot.x;
            int d = abs(location - a);
            auto onLine = [&](int cross) {
                return vertical ? Cell{location,cross} : Cell{cross,location};
            };
            int m = 0;
            for (int i = -WINDOW; i <= WINDOW; ++i) {
                Cell p = onLine(b+i);
                if (p.x <= 0 || p.y <= 0 || p.x >= right || p.y >= top ||
                    seen[p.x][p.y] != generation)
                    m |= 1 << (WINDOW+i);
            }
            int needed = wallRank[m];
            int nearCost = a < location ? leftCost : rightCost;
            int rest = d >= length ? nearCost : max(leftCost, rightCost);
            if (location == a) nearCost = min(leftCost, rightCost);
            int estimate = (rest + nearCost + 1) / 2;
            if (d > needed) {
                // Prepare this cut to hold the robot on its current side.
                estimate = (rest + 2*nearCost + 2) / 3;
                if (auto step = wallPlacement(m)) mark = onLine(b + *step);
            }
            array<int, 4> score{length + estimate, nearCost, length, d};
            if (score < best) {
                best = score;
                answer = mark;
            }
        };
        int width = highX - lowX + 1, height = highY - lowY + 1;
        for (int x = lowX; x <= highX; ++x)
            tryCut(true, x, columns[x], onColumn[x],
                   rectangleCost[x-lowX][height], rectangleCost[highX-x][height]);
        for (int y = lowY; y <= highY; ++y)
            tryCut(false, y, rows[y], onRow[y],
                   rectangleCost[width][y-lowY], rectangleCost[width][highY-y]);
        if (pocket.east && !pocket.contains(robot)) pocket = {};
        if (!pocket.east && pockets < 3 && best[0] >= 23) {
            int bestPocket = best[0] - 7;
            for (int east : {-1, 1}) {
                for (int north : {-1, 1}) {
                    // Try a short separator across a corner when it can
                    // isolate much less space than a full-width cut.
                    int bx = east == 1 ? lowX : highX;
                    int by = north == 1 ? lowY : highY;
                    int w = abs(robot.x - bx) + 3;
                    int h = abs(robot.y - by) + 3;
                    if (w > 6 || h > 6 || w >= width || h >= height) continue;
                    Pocket candidate{bx + east*w, by + north*h, east, north};
                    int score = w + h + 1 + rectangleCost[w][h];
                    if (score < bestPocket) {
                        bestPocket = score;
                        pocket = candidate;
                    }
                }
            }
            if (pocket.east) ++pockets;
        }
        if (pocket.east) {
            Cell mark{};
            int farthest = -1;
            for (int i = 0; i < size; ++i) {
                Cell p = component[i];
                if ((p.x == pocket.x && pocket.north * (p.y - pocket.y) <= 0) ||
                    (p.y == pocket.y && pocket.east * (p.x - pocket.x) <= 0)) {
                    if (distance(p, robot) > farthest) {
                        farthest = distance(p, robot);
                        mark = p;
                    }
                }
            }
            if (mark.x) return mark;
            pocket = {};
        }
        return answer;
    }

    Cell attack(bool edgeOnly) {
        array<Cell, 8> moves{};
        int n = neighbors(robot, moves);
        bool edge = n <= 5 && edgeAttacks < 10;
        if (!edge) {
            if (edgeOnly || pocket.east || historySize < 6 || localAttacks >= 8)
                return {};
            for (Cell p : history)
                if (distance(p, robot) > 2) return {};
        }
        if (!n) return {};
        Cell answer{};
        int best = INT_MAX;
        for (int i = 0; i <= n; ++i) {
            if (i == n && edge) break;
            Cell mark = i < n ? moves[i] : robot;
            if (!safe(mark)) continue;
            black[mark.x][mark.y] = true;
            int score = 0;
            for (int j = 0; j < n; ++j) {
                if (j == i) continue;
                array<Cell, 8> around{};
                int degree = neighbors(moves[j], around);
                score += degree * degree;
            }
            if (!edge) {
                for (Cell p : history)
                    if (mark == p) score -= 25;
            }
            black[mark.x][mark.y] = false;
            if (score < best) {
                best = score;
                answer = mark;
            }
        }
        if (answer.x) {
            if (edge) ++edgeAttacks;
            else ++localAttacks;
        }
        return answer;
    }

    Cell prepareCaptureCorner() const {
        if (historySize < 3 || right - robot.x > 6 || top - robot.y > 6) return {};
        for (int i = 1; i <= 3; ++i) {
            Cell p = history[(historySize - i) % history.size()];
            if (robot.x - p.x != i || robot.y - p.y != i) return {};
        }
        for (Cell mark : {Cell{right-2, top-1}, Cell{right-1, top-2}}) {
            if (white(mark) && distance(robot, mark) >= 2 && safe(mark)) return mark;
        }
        return {};
    }

    bool turn(Cell mark) {
        assert(white(mark));
        black[mark.x][mark.y] = true;
        history[historySize++ % history.size()] = robot;
        cout << mark.x << ' ' << mark.y << endl;
        return readPosition(robot);
    }

public:
    explicit Strategy(Cell start) : right(start.x + 12), top(start.y + 12), robot(start) {}

    void run() {
        // Cover the corner and seed the two arms. Four turns of approach
        // remain available after these seven placements.
        const array<Cell, 7> corner{{
            {right, top}, {right, top-1}, {right-1, top}, {right, top-2},
            {right-2, top}, {right, top-3}, {right-4, top}
        }};
        CaptureSearch capture;
        int committed = 0;
        int placed = 0;
        while (true) {
            int horizon = committed ? committed : CAPTURE_DEPTH;
            int usedTurns = 0;
            Cell mark = capture.find(robot, horizon, usedTurns);
            if (mark.x) {
                // The same (or a shorter) winning continuation is available
                // after every white reply, with a fresh search budget.
                committed = usedTurns - 1;
            } else if (placed < int(corner.size())) {
                assert(!committed);
                int reserve = int(corner.size()) - placed + 5;
                int d = min(right - robot.x, top - robot.y);
                if (d > reserve) mark = attack(false);
                if (!mark.x) mark = corner[placed++];
            } else {
                // Each ordinary move marks a new cell in the bounded
                // rectangle and preserves confinement for every reply.
                assert(!committed);
                mark = attack(true);
                if (!mark.x) mark = prepareCaptureCorner();
                if (!mark.x) {
                    Cell cut = chooseCut();
                    mark = attack(false);
                    if (!mark.x) mark = cut;
                    if (!safe(mark)) {
                        mark = guard();
                        assert(mark.x && safe(mark));
                    }
                }
            }
            if (!turn(mark)) return;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Cell start;
    if (!readPosition(start)) return 0;
    prepareTables();
    Strategy(start).run();
    return 0;
}

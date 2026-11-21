#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <utility>
#include <vector>

using namespace std;

struct Point {
    int r, c;
    bool operator==(const Point&) const = default;
};

struct Rectangle {
    int top, bottom, left, right;

    bool contains(Point p) const {
        return top <= p.r && p.r <= bottom && left <= p.c && p.c <= right;
    }
    int height() const { return bottom - top + 1; }
    int width() const { return right - left + 1; }
};

struct Range {
    int lo, hi;
    bool known() const { return lo == hi; }
};

class Explorer {
    static constexpr array<int, 4> dr{-1, 0, 1, 0};
    static constexpr array<int, 4> dc{0, -1, 0, 1};
    static constexpr array<char, 4> messages{'^', '<', 'v', '>'};
    static constexpr int empty = 1;
    static constexpr int arrow = 2;

    struct Cut {
        bool vertical;
        int split, first, last, base;
        double cost;

        Point point(int along, int side) const {
            return vertical ? Point{along, split + side} : Point{split + side, along};
        }
        int along(Point p) const { return vertical ? p.r : p.c; }
        int across(Point p) const { return vertical ? p.c : p.r; }
        int acrossDirection() const { return vertical ? 3 : 2; }
        int alongDirection(int sign) const {
            return vertical ? (sign > 0 ? 2 : 0) : (sign > 0 ? 3 : 1);
        }
    };

    int n, size, stride;
    Point current, source;
    array<int, 4> delta;
    // Low three bits store the footprint, high bits store the known predecessor.
    vector<uint8_t> cells;
    vector<uint8_t> excluded;

    int index(Point p) const { return p.r * stride + p.c; }
    int out(int id) const { return cells[id] & 7; }

    static Point neighbor(Point p, int d) { return {p.r + dr[d], p.c + dc[d]}; }
    static int distance(Point a, Point b) { return abs(a.r - b.r) + abs(a.c - b.c); }

    void setArrow(int id, int d) {
        cells[id] = (cells[id] & ~7) | (arrow + d);
        int next = id + delta[d];
        cells[next] = (cells[next] & 7) | (((d ^ 2) + 1) << 3);
    }

    void move(int d) {
        cout.put(messages[d]).put('\n').flush();
        char reply;
        if (!(cin >> reply) || reply == 'G') exit(0);

        int direction = 0;
        if (reply != '.') {
            while (direction < 4 && messages[direction] != reply) ++direction;
            if (direction == 4) exit(0);
        }
        current = neighbor(current, d);
        int id = index(current);
        if (!out(id)) {
            if (reply == '.') cells[id] = empty;
            else setArrow(id, direction);
        }
    }

    void go(Point target) {
        while (current.r != target.r) move(current.r < target.r ? 2 : 0);
        while (current.c != target.c) move(current.c < target.c ? 3 : 1);
    }

    bool canEnter(int id, int from) const {
        if (out(id) == empty || id == index(source)) return false;
        int predecessor = cells[id] >> 3;
        if (predecessor) return predecessor == from + 1;
        if ((excluded[id] & (1 << from)) || out(id) == arrow + from) return false;
        int before = out(id + delta[from]);
        return before == 0 || before == arrow + (from ^ 2);
    }

    bool forcedPredecessor(int id, int from) const {
        if (out(id) < arrow) return false;
        for (int d = 0; d < 4; ++d) {
            if (d != from && canEnter(id, d)) return false;
        }
        return true;
    }

    bool shouldCheckAhead(int id, int across, int ahead) const {
        if (out(id) < arrow || out(id + delta[ahead]) || !canEnter(id, ahead)) return false;
        return out(id) == arrow + (ahead ^ 2) ||
            (!canEnter(id, ahead ^ 2) && !canEnter(id, across ^ 2));
    }

    // Positive flow goes from id to its neighbor in direction d.
    Range edge(int id, int d) const {
        if (excluded[id] & (1 << d)) return {0, 0};
        int next = id + delta[d];
        if (out(id) == arrow + d) return {1, 1};
        if (out(next) == arrow + (d ^ 2)) return {-1, -1};
        bool positive = canEnter(next, d ^ 2);
        bool negative = canEnter(id, d);
        if (positive && forcedPredecessor(next, d ^ 2)) return {1, 1};
        if (negative && forcedPredecessor(id, d)) return {-1, -1};
        return {-negative, positive};
    }

    void saveEdge(int id, int d, int flow) {
        int next = id + delta[d];
        if (flow > 0) setArrow(id, d);
        else if (flow < 0) setArrow(next, d ^ 2);
        else {
            excluded[id] |= 1 << d;
            excluded[next] |= 1 << (d ^ 2);
        }
    }

    Rectangle followPrefix() {
        Point start = current;
        Rectangle box{start.r, start.r, start.c, start.c};
        vector<int> prefix;
        const int allowance = min(42, n + 13) + n / 5;
        while (static_cast<int>(prefix.size()) - distance(start, current) < allowance) {
            int id = index(current);
            prefix.push_back(id);
            move(out(id) - arrow);
            box.top = min(box.top, current.r);
            box.bottom = max(box.bottom, current.r);
            box.left = min(box.left, current.c);
            box.right = max(box.right, current.c);
        }

        // The remaining suffix starts here. Its earlier cells are unavailable to it.
        for (int id : prefix) cells[id] = empty;
        source = current;
        cells[index(source)] &= 7;
        return box;
    }

    optional<Rectangle> enclosedSuffix() {
        // Earlier path cells cannot be visited again. If they enclose the next
        // cell, the suffix cannot leave its remaining connected component.
        constexpr int seen = 16;
        int start = index(source);
        vector<int> component{start + delta[out(start) - arrow]};
        excluded[component.front()] |= seen;
        int limit = 1024 + 6 * n;
        size_t head = 0;
        while (head < component.size() && static_cast<int>(component.size()) <= limit) {
            int id = component[head++];
            for (int step : delta) {
                int next = id + step;
                if (!(excluded[next] & seen) && !out(next)) {
                    excluded[next] |= seen;
                    component.push_back(next);
                }
            }
        }
        if (head < component.size()) {
            for (int id : component) excluded[id] &= ~seen;
            return nullopt;
        }

        Rectangle box{source.r, source.r, source.c, source.c};
        for (int id : component) {
            int r = id / stride, c = id % stride;
            box.top = min(box.top, r);
            box.bottom = max(box.bottom, r);
            box.left = min(box.left, c);
            box.right = max(box.right, c);
        }
        for (int r = box.top; r <= box.bottom; ++r) {
            for (int c = box.left; c <= box.right; ++c) {
                int id = index({r, c});
                if (id != start && !(excluded[id] & seen)) cells[id] = empty;
            }
        }
        for (int id : component) excluded[id] &= ~seen;
        for (int c = box.left; c <= box.right; ++c) {
            saveEdge(index({box.top, c}), 0, 0);
            saveEdge(index({box.bottom, c}), 2, 0);
        }
        for (int r = box.top; r <= box.bottom; ++r) {
            saveEdge(index({r, box.left}), 1, 0);
            saveEdge(index({r, box.right}), 3, 0);
        }
        return box;
    }

    pair<bool, bool> probeBox(const Rectangle& box) {
        vector<pair<Point, int>> boundary;
        for (int c = box.left; c <= box.right; ++c) boundary.push_back({{box.top, c}, 0});
        for (int r = box.top; r <= box.bottom; ++r) boundary.push_back({{r, box.right}, 3});
        for (int c = box.right; c >= box.left; --c) boundary.push_back({{box.bottom, c}, 2});
        for (int r = box.bottom; r >= box.top; --r) boundary.push_back({{r, box.left}, 1});
        int count = static_cast<int>(boundary.size());
        int start = 0;
        for (int i = 1; i < count; ++i) {
            if (distance(current, boundary[i].first) < distance(current, boundary[start].first)) start = i;
        }

        int flow = 0;
        for (int i = 0; i < count; ++i) {
            auto [point, d] = boundary[(start + i) % count];
            int id = index(point);
            Range value = edge(id, d);
            if (!value.known() && !out(id)) {
                go(point);
                value = edge(id, d);
            }
            const auto& next = boundary[(start + i + 1) % count];
            if (!value.known() && current == point && next.second == d) {
                int ahead = (d + 1) % 4;
                if (!(neighbor(point, ahead) == next.first)) ahead ^= 2;
                if (neighbor(point, ahead) == next.first && shouldCheckAhead(id, d, ahead)) {
                    move(ahead);
                    value = edge(id, d);
                }
            }
            if (!value.known()) {
                go(neighbor(point, d));
                value = edge(id, d);
            }
            saveEdge(id, d, value.lo);
            flow += value.lo;
        }
        int marked = 0;
        for (const auto& entry : boundary) marked += out(index(entry.first)) >= arrow;
        // The source is inside, so outward flow is zero exactly when the goal is inside.
        return {flow == 0, marked * 3 <= count};
    }

    Cut chooseCut(const Rectangle& box) const {
        vector<int> column(box.width() + 1), row(box.height() + 1);
        int leftFlow = 0, topFlow = 0;
        for (int c = box.left; c <= box.right; ++c) {
            int top = edge(index({box.top, c}), 0).lo;
            int bottom = edge(index({box.bottom, c}), 2).lo;
            int i = c - box.left;
            column[i + 1] = column[i] - top - bottom;
            topFlow -= top;
        }
        for (int r = box.top; r <= box.bottom; ++r) {
            int left = edge(index({r, box.left}), 1).lo;
            int right = edge(index({r, box.right}), 3).lo;
            int i = r - box.top;
            row[i + 1] = row[i] - left - right;
            leftFlow -= left;
        }
        bool containsSource = box.contains(source);
        Cut best{false, 0, 0, 0, 0, numeric_limits<double>::infinity()};
        for (bool vertical : {true, false}) {
            int low = vertical ? box.left : box.top;
            int high = vertical ? box.right : box.bottom;
            int width = high - low + 1;
            if (width < 2) continue;
            int mid = low + (width - 1) / 2;
            int at = vertical ? current.c : current.r;
            int margin = max(1, width / 4);
            int near = clamp(at, low + margin - 1, high - margin);
            vector<int> candidates{mid, near, near - 1, near + 1, mid - 1, mid + 1};
            for (int i : {-3, -2, -1, 1, 2, 3}) candidates.push_back(mid + width * i / 16);
            sort(candidates.begin(), candidates.end());
            candidates.erase(unique(candidates.begin(), candidates.end()), candidates.end());
            for (int split : candidates) {
                if (split < low + margin - 1 || split > high - margin) continue;
                int first = vertical ? box.top : box.left;
                int last = vertical ? box.bottom : box.right;
                int length = split - low + 1;
                int base = vertical ? leftFlow + column[length] : topFlow + row[length];
                if (containsSource && (vertical ? source.c : source.r) <= split) ++base;
                // Flow across this cut is base - 1 if the goal is in the lower
                // coordinates, or base if the goal is in the higher coordinates.
                Cut cut{vertical, split, first, last, base, 0};
                int lo = 0, hi = 0;
                int firstUnknown = last + 1, lastUnknown = first - 1;
                double extra = 0;
                for (int i = first; i <= last; ++i) {
                    int id = index(cut.point(i, 0));
                    int d = cut.acrossDirection();
                    Range value = edge(id, d);
                    lo += value.lo;
                    hi += value.hi;
                    if (!value.known()) {
                        firstUnknown = min(firstUnknown, i);
                        lastUnknown = i;
                        extra += (!out(id) && !out(id + delta[d])) ? 0.24 : 0.46;
                    }
                }
                int other = width - length;
                double remaining = 0.55 * max(length, other)
                    + 0.45 * (length * length + other * other) / width;
                if (lo >= base) remaining = width - length;
                else if (hi < base) remaining = length;
                else {
                    if (3 * width < last - first + 1) continue;
                    int atAlong = cut.along(current);
                    int atAcross = cut.across(current);
                    cut.cost = max({0, split - atAcross, atAcross - split - 1})
                        + lastUnknown - firstUnknown
                        + min(abs(atAlong - firstUnknown), abs(atAlong - lastUnknown))
                        + extra;
                }
                cut.cost += 1.9 * (remaining + last - first + 1);
                if (cut.cost < best.cost) best = cut;
            }
        }
        return best;
    }

    bool sweep(const Cut& cut) {
        const int d = cut.acrossDirection();
        vector<Range> values(cut.last - cut.first + 1, {0, 0});
        set<int> pending;
        int lo = 0, hi = 0;
        auto refresh = [&](int i) {
            if (i < cut.first || i > cut.last) return;
            Range& old = values[i - cut.first];
            Range value = edge(index(cut.point(i, 0)), d);
            if (old.lo == value.lo && old.hi == value.hi) return;
            lo += value.lo - old.lo;
            hi += value.hi - old.hi;
            if (value.known()) pending.erase(i);
            else pending.insert(i);
            old = value;
        };
        for (int i = cut.first; i <= cut.last; ++i) refresh(i);

        auto decided = [&] { return lo >= cut.base || hi < cut.base; };
        auto step = [&](int direction) {
            move(direction);
            int across = cut.across(current) - cut.split;
            if (across >= -1 && across <= 2) {
                int i = cut.along(current);
                refresh(i - 1);
                refresh(i);
                refresh(i + 1);
            }
        };

        if (!decided()) {
            int along = cut.along(current);
            int sign = abs(along - *pending.begin()) <= abs(along - *pending.rbegin()) ? 1 : -1;
            auto leading = [&] { return sign > 0 ? *pending.begin() : *pending.rbegin(); };
            while (!decided() && cut.across(current) < cut.split) step(d);
            while (!decided() && cut.across(current) > cut.split + 1) step(d ^ 2);
            // Use the approach to the first end of the cut to survey one lane.
            while (!decided() && sign * (cut.along(current) - leading()) > 0) {
                step(cut.alongDirection(-sign));
            }
            while (!decided()) {
                int i = leading();
                int here = cut.along(current);
                if (here != i) {
                    step(cut.alongDirection(i > here ? 1 : -1));
                    continue;
                }
                int side = cut.across(current) - cut.split;
                int cross = side == 0 ? d : d ^ 2;
                int ahead = cut.alongDirection(sign);
                int id = index(current);
                // The next cell may settle this crossing while advancing the sweep.
                if (i + sign >= cut.first && i + sign <= cut.last &&
                    shouldCheckAhead(id, cross, ahead)) {
                    step(ahead);
                    if (!decided() && pending.contains(i)) step(cross);
                } else {
                    step(cross);
                }
            }
        }
        bool lower = hi < cut.base;
        // The two feasible sums differ by one. Once one is ruled out, every
        // edge must attain the same end of its interval to reach the other sum.
        for (int i = cut.first; i <= cut.last; ++i) {
            Range value = values[i - cut.first];
            saveEdge(index(cut.point(i, 0)), d, lower ? value.hi : value.lo);
        }
        return lower;
    }

    vector<Point> frontiers(const Rectangle& box) const {
        vector<Point> result;
        auto follow = [&](Point p) {
            while (box.contains(p)) {
                int direction = out(index(p));
                if (!direction) {
                    result.push_back(p);
                    return;
                }
                if (direction < arrow) return;
                p = neighbor(p, direction - arrow);
            }
        };
        auto check = [&](Point p, int d) {
            if (edge(index(p), d).lo < 0) follow(p);
        };
        if (box.contains(source)) follow(source);
        for (int c = box.left; c <= box.right; ++c) {
            check({box.top, c}, 0);
            check({box.bottom, c}, 2);
        }
        for (int r = box.top; r <= box.bottom; ++r) {
            check({r, box.left}, 1);
            check({r, box.right}, 3);
        }
        return result;
    }

    void finish(const Rectangle& box) {
        while (true) {
            vector<Point> targets = frontiers(box);
            if (targets.empty()) exit(0);
            Point target = *min_element(targets.begin(), targets.end(), [&](Point a, Point b) {
                return distance(current, a) < distance(current, b);
            });
            go(target);
        }
    }

public:
    explicit Explorer(int radius)
        : n(radius), size(2 * n + 1), stride(size + 2), current{n + 1, n + 1},
          source(current), delta{-stride, -1, stride, 1},
          cells(stride * stride), excluded(stride * stride) {
        for (int i = 0; i < stride; ++i) {
            cells[i] = cells[(stride - 1) * stride + i] = empty;
            cells[i * stride] = cells[i * stride + stride - 1] = empty;
        }
        setArrow(index(current), 0);
    }

    void run() {
        Rectangle prefix = followPrefix();
        Rectangle box{1, size, 1, size};
        optional<Rectangle> enclosed = enclosedSuffix();
        int span = max(prefix.width(), prefix.height());
        if (enclosed) {
            box = *enclosed;
        } else if (n >= 12) {
            int pad = max(3, span / 3);
            int budget = 5 * n / 2 + 30;
            while (true) {
                Rectangle local{max(1, prefix.top - pad), min(size, prefix.bottom + pad),
                                max(1, prefix.left - pad), min(size, prefix.right + pad)};
                int perimeter = 2 * (local.width() + local.height());
                if (perimeter > budget) break;
                auto [containsTreasure, sparseBoundary] = probeBox(local);
                if (containsTreasure) {
                    box = local;
                    break;
                }
                if (!sparseBoundary) break;
                budget -= perimeter;
                pad = 2 * pad + span / 3;
            }
        }

        while (box.width() * box.height() > 24) {
            Cut cut = chooseCut(box);
            bool lower = sweep(cut);
            if (cut.vertical) {
                if (lower) box.right = cut.split;
                else box.left = cut.split + 1;
            } else {
                if (lower) box.bottom = cut.split;
                else box.top = cut.split + 1;
            }

            int span = max(box.width(), box.height());
            if (box.width() * box.height() > 24) {
                vector<Point> targets = frontiers(box);
                if (targets.size() == 1 && distance(current, targets.front()) <= min(12, 2 + span / 10)) {
                    go(targets.front());
                    for (int i = 0; i < min(72, 2 + span / 8); ++i) {
                        int direction = out(index(current)) - arrow;
                        if (direction < 0 || !box.contains(neighbor(current, direction))) break;
                        move(direction);
                    }
                }
            }
        }
        finish(box);
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (cin >> n && n >= 1 && n <= 2000) Explorer(n).run();
}

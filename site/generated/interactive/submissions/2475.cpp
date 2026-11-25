#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using int64 = long long;

struct Probe {
    bool vertical;
    int line;
    int64 value;
};

struct Disk {
    int x, y, r;
};

static vector<Probe> probes;

static int64 chordValue(bool vertical, int line) {
    cout << "query ";
    if (vertical) {
        cout << line << ' ' << 0 << ' ' << line << ' ' << 100000 << '\n';
    } else {
        cout << 0 << ' ' << line << ' ' << 100000 << ' ' << line << '\n';
    }
    cout.flush();

    string response;
    if (!(cin >> response)) exit(0);
    long double length;
    try {
        length = stold(response);
    } catch (...) {
        exit(0);
    }
    if (length < 0) exit(0);

    int64 value = length < 1e-5L ? 0 : llround(length * length / 4.0L);
    probes.push_back({vertical, line, value});
    return value;
}

static int64 integerSqrt(int64 value) {
    int64 root = sqrtl((long double)value);
    while ((root + 1) * (root + 1) <= value) ++root;
    while (root * root > value) --root;
    return root;
}

static int64 expectedValue(const Disk &d, bool vertical, int line) {
    int64 delta = vertical ? d.x - line : d.y - line;
    int64 value = int64(d.r) * d.r - delta * delta;
    return max<int64>(0, value);
}

static void midpointOrder(int left, int right, vector<int> &order) {
    if (left > right) return;
    int middle = (left + right) / 2;
    order.push_back(middle);
    midpointOrder(left, middle - 1, order);
    midpointOrder(middle + 1, right, order);
}

static vector<int> hittingGrid(int minimumRadius) {
    int step = 2 * minimumRadius - 1;
    vector<int> grid;
    for (int line = minimumRadius + 99; line <= 100000; line += step) {
        grid.push_back(line);
    }
    while (grid.back() + minimumRadius - 1 < 99900) {
        int next = min(100000, grid.back() + step);
        if (next == grid.back()) break;
        grid.push_back(next);
    }

    vector<int> order;
    midpointOrder(0, (int)grid.size() - 1, order);
    vector<int> result;
    for (int index : order) result.push_back(grid[index]);
    return result;
}

static vector<Disk> findCandidates(int horizontalLine, int64 horizontalValue,
                                   int verticalLine, int64 verticalValue) {
    vector<Disk> candidates;
    for (int r = 100; r <= 50000; ++r) {
        int64 radiusSquared = int64(r) * r;
        int64 dySquared = radiusSquared - horizontalValue;
        int64 dxSquared = radiusSquared - verticalValue;
        if (dySquared < 0 || dxSquared < 0) continue;

        int64 dy = integerSqrt(dySquared);
        int64 dx = integerSqrt(dxSquared);
        if (dy * dy != dySquared || dx * dx != dxSquared) continue;

        for (int sy = -1; sy <= 1; sy += 2) {
            if (dy == 0 && sy == 1) continue;
            int y = horizontalLine + sy * dy;
            for (int sx = -1; sx <= 1; sx += 2) {
                if (dx == 0 && sx == 1) continue;
                int x = verticalLine + sx * dx;
                if (x < r || x > 100000 - r || y < r || y > 100000 - r) continue;

                Disk d{x, y, r};
                bool consistent = true;
                for (const Probe &p : probes) {
                    if (expectedValue(d, p.vertical, p.line) != p.value) {
                        consistent = false;
                        break;
                    }
                }
                if (consistent) candidates.push_back(d);
            }
        }
    }
    return candidates;
}

static pair<bool, int> chooseProbe(const vector<Disk> &candidates) {
    vector<pair<bool, int>> options;
    if (candidates.size() <= 64) {
        for (int line = 0; line <= 100000; ++line) {
            options.push_back({false, line});
            options.push_back({true, line});
        }
    } else {
        set<pair<bool, int>> distinct;
        for (const Disk &d : candidates) {
            for (int line : {d.y - d.r, d.y - 1, d.y, d.y + 1, d.y + d.r}) {
                if (0 <= line && line <= 100000) distinct.insert({false, line});
            }
            for (int line : {d.x - d.r, d.x - 1, d.x, d.x + 1, d.x + d.r}) {
                if (0 <= line && line <= 100000) distinct.insert({true, line});
            }
        }
        options.assign(distinct.begin(), distinct.end());
    }

    pair<bool, int> best = options.front();
    int bestBucket = (int)candidates.size() + 1;
    int bestParts = -1;
    for (auto [vertical, line] : options) {
        vector<int64> signatures;
        signatures.reserve(candidates.size());
        for (const Disk &d : candidates) {
            signatures.push_back(expectedValue(d, vertical, line));
        }
        sort(signatures.begin(), signatures.end());
        int parts = 0, largestBucket = 0;
        for (int i = 0; i < (int)signatures.size();) {
            int j = i + 1;
            while (j < (int)signatures.size() && signatures[j] == signatures[i]) ++j;
            largestBucket = max(largestBucket, j - i);
            ++parts;
            i = j;
        }
        if (largestBucket < bestBucket ||
            (largestBucket == bestBucket && parts > bestParts)) {
            best = {vertical, line};
            bestBucket = largestBucket;
            bestParts = parts;
            if (largestBucket == 1) break;
        }
    }

    if (bestBucket == (int)candidates.size()) {
        for (int line = 0; line <= 100000; ++line) {
            for (bool vertical : {false, true}) {
                int64 first = expectedValue(candidates[0], vertical, line);
                for (size_t i = 1; i < candidates.size(); ++i) {
                    if (expectedValue(candidates[i], vertical, line) != first) {
                        return {vertical, line};
                    }
                }
            }
        }
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int horizontalLine = -1;
    int64 horizontalValue = 0;
    for (int line : hittingGrid(100)) {
        int64 value = chordValue(false, line);
        if (value > 0) {
            horizontalLine = line;
            horizontalValue = value;
            break;
        }
    }

    int secondHorizontalLine = horizontalLine + 1;
    int64 secondHorizontalValue = chordValue(false, secondHorizontalLine);
    if (secondHorizontalValue == 0) {
        secondHorizontalLine = horizontalLine - 1;
        secondHorizontalValue = chordValue(false, secondHorizontalLine);
    }
    int direction = secondHorizontalLine - horizontalLine;
    int64 centerOffset = (secondHorizontalValue - horizontalValue + 1) / (2 * direction);
    int64 radiusSquared = horizontalValue + centerOffset * centerOffset;
    int radius = integerSqrt(radiusSquared);

    int verticalLine = -1;
    int64 verticalValue = 0;
    for (int line : hittingGrid(radius)) {
        int64 value = chordValue(true, line);
        if (value > 0) {
            verticalLine = line;
            verticalValue = value;
            break;
        }
    }

    vector<Disk> candidates = findCandidates(horizontalLine, horizontalValue,
                                             verticalLine, verticalValue);
    while (candidates.size() > 1) {
        auto [vertical, line] = chooseProbe(candidates);
        int64 value = chordValue(vertical, line);
        vector<Disk> remaining;
        for (const Disk &d : candidates) {
            if (expectedValue(d, vertical, line) == value) remaining.push_back(d);
        }
        candidates.swap(remaining);
    }

    if (candidates.empty()) return 0;
    Disk answer = candidates[0];
    cout << "answer " << answer.x << ' ' << answer.y << ' ' << answer.r << '\n';
    cout.flush();
    return 0;
}

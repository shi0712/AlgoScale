#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

using Integer = int64_t;
using Wide = __int128_t;
constexpr int Side = 100000;
constexpr int MinRadius = 100;
constexpr Integer Scale = 10000000;
constexpr Integer Error = 10;

struct Disk {
    int x, y, r;
};

struct Line {
    int x1, y1, x2, y2;
    Integer a, b, c, norm;

    Line(int x1, int y1, int x2, int y2) : x1(x1), y1(y1), x2(x2), y2(y2) {
        int dx = x2 - x1, dy = y2 - y1;
        int divisor = gcd(dx, dy);
        a = -dy / divisor;
        b = dx / divisor;
        c = a * x1 + b * y1;
        norm = a * a + b * b;
    }

    // Every probe spans the full chord of each disk it is tested against.
    Wide chordNumerator(const Disk& disk) const {
        Integer offset = a * disk.x + b * disk.y - c;
        return max<Wide>(0, Wide(norm) * disk.r * disk.r - Wide(offset) * offset);
    }
};

Integer readLength() {
    string token;
    if (!(cin >> token) || cin.eof()) exit(0);

    size_t pos = 0;
    bool negative = token[pos] == '-';
    if (negative || token[pos] == '+') ++pos;
    if (pos == token.size() || token[pos] < '0' || token[pos] > '9') exit(0);

    Integer whole = 0;
    while (pos < token.size() && token[pos] >= '0' && token[pos] <= '9') {
        whole = 10 * whole + token[pos++] - '0';
        if (whole > Side + 1) exit(0);
    }

    Integer fraction = 0;
    int digits = 0;
    if (pos < token.size() && token[pos] == '.') {
        ++pos;
        while (pos < token.size() && token[pos] >= '0' && token[pos] <= '9') {
            if (++digits > 7) exit(0);
            fraction = 10 * fraction + token[pos++] - '0';
        }
    }
    if (pos != token.size()) exit(0);
    while (digits++ < 7) fraction *= 10;

    Integer value = whole * Scale + fraction;
    if (negative) value = -value;
    if (value < -Error || value > Side * Scale + Error) exit(0);
    return value;
}

Integer probe(const Line& line) {
    cout << "query " << line.x1 << ' ' << line.y1 << ' '
         << line.x2 << ' ' << line.y2 << endl;
    return readLength();
}

// The squared chord length is 4 * numerator / norm. The input and its
// error bound are both integral in units of 1e-7.
pair<Wide, Wide> numeratorBounds(const Line& line, Integer value) {
    Integer lo = max<Integer>(0, value - Error);
    Integer hi = value + Error;
    constexpr Wide denominator = Wide(4) * Scale * Scale;
    Wide lower = Wide(lo) * lo * line.norm;
    Wide upper = Wide(hi) * hi * line.norm;
    return {(lower + denominator - 1) / denominator, upper / denominator};
}

Integer floorRoot(Wide value) {
    Integer root = sqrt(static_cast<long double>(value));
    while (Wide(root) * root > value) --root;
    while (Wide(root + 1) * (root + 1) <= value) ++root;
    return root;
}

vector<int> searchOrder() {
    constexpr int spacing = 2 * MinRadius;
    vector<int> rows{-1};
    for (int s = spacing - 1; s < Side / 2; s += spacing) rows.push_back(s);
    for (int s = Side / 2; s < Side; s += spacing) rows.push_back(s);
    rows.push_back(Side);

    // Given misses at rows a and b, radius r has max(0,b-a-2r)
    // possible center rows in the intervening gap. Split wide gaps first.
    using Gap = tuple<int, int, int>;
    priority_queue<Gap> gaps;
    auto addGap = [&](int left, int right) {
        if (right > left + 1) gaps.emplace(rows[right] - rows[left], left, right);
    };
    const int middle = (Side / 2) / spacing;
    vector<int> order{rows[middle]};
    addGap(0, middle);
    addGap(middle, int(rows.size()) - 1);
    while (!gaps.empty()) {
        auto [width, left, right] = gaps.top();
        gaps.pop();
        // A slight bias toward the outer boundary staggers the sizes
        // of the gaps that will have to be split in later rounds.
        int percentage = width >= 10000 ? 45 : width >= 3000 ? 47 : 50;
        if (left >= middle) percentage = 100 - percentage;
        int target = rows[left] + (width * percentage + 50) / 100;
        auto first = rows.begin() + left + 1;
        auto end = rows.begin() + right;
        auto at = lower_bound(first, end, target);
        if (at == end || (at != first && target - at[-1] <= *at - target)) --at;
        int index = int(at - rows.begin());
        order.push_back(rows[index]);
        addGap(left, index);
        addGap(index, right);
    }

    // Keep the initial wide-gap probes, then check both boundary bands
    // so disks tangent to the top or bottom cannot be left until late.
    auto boundarySlot = order.begin() + 32;
    for (int row : {spacing - 1, Side - spacing}) {
        auto at = find(boundarySlot, order.end(), row);
        if (at != order.end()) {
            rotate(boundarySlot, at, next(at));
            ++boundarySlot;
        }
    }
    return order;
}

vector<Disk> firstCandidates(int row, Integer length, vector<int>& misses) {
    Line line(0, row, Side, row + 1);
    auto [lowerNumerator, upperNumerator] = numeratorBounds(line, length);
    sort(misses.begin(), misses.end());
    misses.push_back(Side);

    vector<Disk> disks;
    auto nextMiss = lower_bound(misses.begin(), misses.end(), row);
    int right = *nextMiss;
    int left = nextMiss == misses.begin() ? -1 : *prev(nextMiss);

    // For all valid x, a search probe at s hits exactly the integer
    // center rows s-r+1 through s+r. The closest misses bound the gap.
    int maxR = (right - left - 1) / 2;
    int minR = max<Integer>(MinRadius, (length - Error + 2 * Scale - 1) / (2 * Scale));
    for (int r = minR; r <= maxR; ++r) {
        int minY = max(left + r + 1, row - r + 1);
        int maxY = min(right - r, row + r);
        if (minY > maxY) continue;

        Wide squared = Wide(line.norm) * r * r;
        Wide highSquare = squared - lowerNumerator;
        if (highSquare < 0) continue;
        Wide lowSquare = max<Wide>(0, squared - upperNumerator);
        Integer lo = floorRoot(lowSquare);
        if (Wide(lo) * lo < lowSquare) ++lo;
        Integer hi = floorRoot(highSquare);

        // The signed offset is Side*(y-row)-x. For each offset there
        // is at most one x in the allowable range, by reduction mod Side.
        auto addOffset = [&](Integer offset) {
            int x = int((-offset % Side + Side) % Side);
            if (x < r || x > Side - r) return;
            int y = row + int((offset + x) / Side);
            if (minY <= y && y <= maxY) disks.push_back({x, y, r});
        };
        for (Integer offset = lo; offset <= hi; ++offset) {
            addOffset(offset);
            if (offset != 0) addOffset(-offset);
        }
    }
    return disks;
}

void filter(vector<Disk>& disks, const Line& line) {
    auto [lo, hi] = numeratorBounds(line, probe(line));
    erase_if(disks, [&](const Disk& disk) {
        Wide numerator = line.chordNumerator(disk);
        return numerator < lo || numerator > hi;
    });
    if (disks.empty()) exit(0);
}

Line horizontal(int y1, int y2) {
    return Line(0, y1, Side, y2);
}

Line vertical(int x1, int x2) {
    return Line(x1, 0, x2, Side);
}

bool fixedFamily(const vector<Disk>& disks, bool vertical) {
    const Disk& first = disks.front();
    return all_of(disks.begin(), disks.end(), [&](const Disk& disk) {
        return disk.r == first.r && (vertical ? disk.x == first.x : disk.y == first.y);
    });
}

Line familyProbe(const Disk& disk, bool alongVertical) {
    // Every center is on the same side of this probe, at a distance
    // between about r/4 and 3r/4. A unit change of the unknown center
    // coordinate then changes the chord length by well over 2e-6.
    int center = alongVertical ? disk.x : disk.y;
    int low = center + (disk.r + 3) / 4;
    int high = center + 3 * disk.r / 4;
    return alongVertical ? vertical(low, high) : horizontal(low, high);
}

Line separate(const Disk& p, const Disk& q) {
    if (p.y - p.r != q.y - q.r) {
        int row = min(p.y - p.r, q.y - q.r) + 1;
        return horizontal(row, row);
    }
    if (p.y + p.r != q.y + q.r) {
        int row = max(p.y + p.r, q.y + q.r) - 1;
        return horizontal(row, row);
    }
    int column = min(p.x - p.r, q.x - q.r) + 1;
    return vertical(column, column);
}

using Quality = tuple<int, Integer, int>;

Quality quality(const vector<Disk>& disks, const Line& line, vector<double>& lengths) {
    const double factor = 2.0 / sqrt(double(line.norm));
    lengths.clear();
    for (const Disk& disk : disks) {
        lengths.push_back(factor * sqrt(double(line.chordNumerator(disk))));
    }
    sort(lengths.begin(), lengths.end());
    constexpr double ambiguity = 0.0000020001;
    int unique = 0, worst = 0;
    Integer nearbyPairs = 0;
    for (int i = 0, left = 0, n = int(lengths.size()); i < n; ++i) {
        while (lengths[i] - lengths[left] > ambiguity) ++left;
        worst = max(worst, i - left + 1);
        nearbyPairs += i - left;
        if (left == i && (i + 1 == n || lengths[i + 1] - lengths[i] > ambiguity)) {
            ++unique;
        }
    }
    return {unique, -nearbyPairs, -worst};
}

Line chooseProbe(const vector<Disk>& disks, int row) {
    if (disks.size() == 2) return separate(disks[0], disks[1]);

    // Start with a separator that is guaranteed to split at least one
    // pair. Also try lines passing through the common vertical range
    // of the disks, and lines adapted to individual candidate disks.
    Line best = separate(disks.front(), disks.back());
    vector<Line> options{best};
    set<array<int, 4>> seen{{best.x1, best.y1, best.x2, best.y2}};
    auto add = [&](const Line& line) {
        if (line.x1 == 0 && line.x2 == Side && line.y1 == row && line.y2 == row + 1) return;
        if (seen.insert({line.x1, line.y1, line.x2, line.y2}).second) options.push_back(line);
    };

    int lo = 0, hi = Side;
    for (const Disk& disk : disks) {
        lo = max(lo, disk.y - disk.r);
        hi = min(hi, disk.y + disk.r);
    }
    vector<int> levels{row, row + 1};
    for (int i = 0; i <= 8; ++i) levels.push_back(lo + (hi - lo) * i / 8);
    sort(levels.begin(), levels.end());
    levels.erase(unique(levels.begin(), levels.end()), levels.end());
    for (int a : levels) {
        for (int b : levels) add(horizontal(a, b));
    }

    // Shifting one end inward permits more slopes within the same
    // one-row band. Keep both endpoints outside every candidate disk
    // and the segment wide enough to contain every center projection.
    array<int, 2> leftShift{Side / 4 - 1, Side / 4 - 1};
    array<int, 2> rightShift = leftShift;
    for (const Disk& disk : disks) {
        for (int i = 0; i < 2; ++i) {
            Integer dy = disk.y - row - i;
            Integer square = Integer(disk.r) * disk.r - dy * dy;
            int reach = int(floorRoot(square));
            if (Integer(reach) * reach < square) ++reach;
            reach = max(1, reach);
            leftShift[i] = min(leftShift[i], disk.x - reach);
            rightShift[i] = min(rightShift[i], Side - disk.x - reach);
        }
    }
    for (int i = 0; i < 2; ++i) {
        for (int pieces : {1, 2, 3, 5, 8, 13, 21, 34}) {
            int left = leftShift[i] / pieces;
            int right = rightShift[1 - i] / pieces;
            int y1 = row + i, y2 = row + 1 - i;
            if (left) add(Line(left, y1, Side, y2));
            if (right) add(Line(0, y1, Side - right, y2));
            if (left && right) {
                add(Line(left, y1, Side - right / 2, y2));
                add(Line(left / 2, y1, Side - right, y2));
            }
        }
    }

    for (int delta : {1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233}) {
        int low = max(0, row - delta);
        int high = min(Side, row + 1 + delta);
        add(horizontal(low, high));
        add(horizontal(high, low));
        add(horizontal(row, high));
        add(horizontal(low, row + 1));
        add(horizontal(high, row));
        add(horizontal(row + 1, low));
    }
    for (size_t i = 0, count = min<size_t>(disks.size(), 16); i < count; ++i) {
        const Disk& disk = disks[i * (disks.size() - 1) / (count - 1)];
        add(familyProbe(disk, false));
        add(familyProbe(disk, true));
        add(horizontal(disk.y, disk.y));
        add(vertical(disk.x, disk.x));
        add(separate(i == 0 ? disks.back() : disks.front(), disk));
    }
    add(Line(0, 0, Side, Side - 1));
    add(Line(0, Side, Side, 1));

    constexpr size_t sampleSize = 448;
    vector<double> lengths;
    lengths.reserve(disks.size());
    Quality bestScore = quality(disks, best, lengths);
    if (get<0>(bestScore) == int(disks.size())) return best;
    auto assess = [&](const Line& line) {
        Quality score = quality(disks, line, lengths);
        if (score > bestScore) {
            bestScore = score;
            best = line;
        }
        return get<0>(bestScore) == int(disks.size());
    };

    if (disks.size() <= sampleSize) {
        for (const Line& line : options) if (assess(line)) break;
    } else {
        // Sample large candidate sets, then measure the best probes
        // against all candidates before sending one to the judge.
        uint64_t state = 0x942d81eaab0d66abULL;
        vector<Disk> sample;
        for (size_t i = 0; i < sampleSize; ++i) {
            state ^= state << 13;
            state ^= state >> 7;
            state ^= state << 17;
            size_t begin = i * disks.size() / sampleSize;
            size_t end = (i + 1) * disks.size() / sampleSize;
            sample.push_back(disks[begin + state % (end - begin)]);
        }
        vector<pair<Quality, int>> ranking;
        for (int i = 0; i < int(options.size()); ++i) {
            ranking.emplace_back(quality(sample, options[i], lengths), -i);
        }
        int finalists = min<int>(32, ranking.size());
        partial_sort(ranking.begin(), ranking.begin() + finalists, ranking.end(), greater<>());
        for (int i = 0; i < finalists; ++i) {
            if (assess(options[-ranking[i].second])) break;
        }
    }
    return best;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    vector<int> misses;
    for (int row : searchOrder()) {
        Integer length = probe(horizontal(row, row + 1));
        if (length <= Error) {
            misses.push_back(row);
            continue;
        }

        vector<Disk> disks = firstCandidates(row, length, misses);
        if (disks.empty()) return 0;
        set<array<int, 4>> asked;
        auto apply = [&](const Line& line) {
            if (asked.insert({line.x1, line.y1, line.x2, line.y2}).second) {
                filter(disks, line);
            }
        };
        auto finishFamily = [&]() {
            if (disks.size() > 1 && fixedFamily(disks, false)) {
                apply(familyProbe(disks.front(), false));
            } else if (disks.size() > 1 && fixedFamily(disks, true)) {
                apply(familyProbe(disks.front(), true));
            }
        };

        for (int step = 0; step < 2 && disks.size() > 1; ++step) {
            finishFamily();
            if (disks.size() > 1) apply(chooseProbe(disks, row));
        }

        // The chords at row and row+1 fix the radius and y coordinate.
        // If one is tangent, its side of the search band is determined,
        // and the other chord has squared half-length 2r-1.
        finishFamily();
        if (disks.size() > 1) apply(horizontal(row, row));
        if (disks.size() > 1 && !fixedFamily(disks, false)) {
            apply(horizontal(row + 1, row + 1));
        }
        finishFamily();

        if (disks.size() != 1) return 0;
        const auto [x, y, r] = disks.front();
        cout << "answer " << x << ' ' << y << ' ' << r << endl;
        return 0;
    }
    return 0;
}

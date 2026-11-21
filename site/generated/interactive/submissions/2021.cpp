#include <bits/stdc++.h>
using namespace std;
using Int = long long;

Int field;

bool ask(Int x, Int y) {
    cout << "? " << x << ' ' << y << endl;
    string reply;
    if (!(cin >> reply)) exit(0);
    if (reply == "true") return true;
    if (reply == "false") return false;
    exit(0);
}

struct Line {
    Int slope, intercept;
    Int operator()(Int t) const { return slope * t + intercept; }
};

// On this interval, t = (M - 1) / 2 and the cell's offset in its square
// ranges between the two affine bounds. Tile numbers start at zero.
struct Band {
    Int first, last;
    Line low, high;
    int tile;
    Int weight = 1;
};

Int floorDiv(Int a, Int b) {
    return a >= 0 ? a / b : -((-a + b - 1) / b);
}

bool restrictRange(Int& first, Int& last, Line a, Line b) {
    // Keep exactly the integers t for which a(t) <= b(t).
    Int slope = a.slope - b.slope;
    Int intercept = b.intercept - a.intercept;
    if (slope > 0) last = min(last, floorDiv(intercept, slope));
    if (slope < 0) first = max(first, -floorDiv(intercept, -slope));
    return first <= last && (slope != 0 || intercept >= 0);
}

Line plusOne(Line a) {
    ++a.intercept;
    return a;
}

// Clip a band by optional offset bounds. Strict inequalities in the
// complementary parts make the resulting bands disjoint.
template<class Consumer>
void clip(const Band& source, const Line* low, const Line* high, Consumer&& consume) {
    auto finish = [&](Band b) {
        if (restrictRange(b.first, b.last, b.low, b.high)) consume(b);
    };
    auto clipHigh = [&](const Band& b) {
        if (!high) {
            finish(b);
            return;
        }
        Band current = b;
        if (restrictRange(current.first, current.last, b.high, *high)) finish(current);
        current = b;
        if (restrictRange(current.first, current.last, plusOne(*high), b.high)) {
            current.high = *high;
            finish(current);
        }
    };
    if (!low) {
        clipHigh(source);
        return;
    }
    Band current = source;
    if (restrictRange(current.first, current.last, *low, source.low)) clipHigh(current);
    current = source;
    if (restrictRange(current.first, current.last, plusOne(source.low), *low)) {
        current.low = *low;
        clipHigh(current);
    }
}

Int count(const Band& b) {
    Int left = b.high(b.first) - b.low(b.first) + 1;
    Int right = b.high(b.last) - b.low(b.last) + 1;
    return (b.last - b.first + 1) * (left + right) / 2;
}

Int count(const vector<Band>& bands) {
    Int total = 0;
    for (const Band& b : bands) total += count(b);
    return total;
}

Int sizeLimit(Int pos, int tile) {
    Int limit = field / 5;
    if (tile > 0) limit = min(limit, (pos - 1) / tile);
    if (tile < 4) limit = min(limit, (field - pos) / (4 - tile));
    return limit;
}

pair<Int, Int> offsetRange(Int pos, Int m, int tile) {
    return {max<Int>(0, pos - field - 1 + (5 - tile) * m),
            min(m - 1, pos - 1 - tile * m)};
}

vector<Band> initialBands(Int pos, Int perpendicular) {
    vector<Band> bands;
    for (int tile = 0; tile < 5; ++tile) {
        Int otherLimit = 0;
        for (int j = tile % 2; j < 5; j += 2) {
            otherLimit = max(otherLimit, sizeLimit(perpendicular, j));
        }
        Int limit = min(sizeLimit(pos, tile), otherLimit);
        if (limit < 3) continue;
        Band b{1, (limit - 1) / 2, {0, 0}, {2, 0}, tile};
        Line low{2 * (5 - tile), pos - field - 1 + (5 - tile)};
        Line high{-2 * tile, pos - 1 - tile};
        clip(b, &low, &high, [&](const Band& part) { bands.push_back(part); });
    }
    return bands;
}

Line transition(int tileShift, Int delta) {
    return {2 * tileShift, tileShift - delta};
}

// Offset bounds corresponding to a run of squares in a row or column.
template<class Consumer>
void redParts(const Band& b, Int delta, Consumer&& consume) {
    for (int k = -(b.tile / 2) * 2; k <= 4 - b.tile; k += 2) {
        Line low = transition(k, delta);
        Line high = transition(k + 1, delta);
        --high.intercept;
        clip(b, &low, &high, consume);
    }
}

void observe(vector<Band>& bands, Int delta, bool red) {
    vector<Band> next;
    auto save = [&](const Band& b) { next.push_back(b); };
    for (const Band& b : bands) {
        if (red) {
            redParts(b, delta, save);
        } else {
            int first = -(b.tile / 2) * 2;
            Line high = transition(first, delta);
            --high.intercept;
            clip(b, nullptr, &high, save);
            for (int k = first + 1; k < 4 - b.tile; k += 2) {
                Line low = transition(k, delta);
                high = transition(k + 1, delta);
                --high.intercept;
                clip(b, &low, &high, save);
            }
            int last = (4 - b.tile) / 2 * 2;
            Line low = transition(last + 1, delta);
            clip(b, &low, nullptr, save);
        }
    }
    auto key = [](const Band& b) {
        return tuple(b.tile, b.low.slope, b.low.intercept, b.high.slope, b.high.intercept, b.weight, b.first);
    };
    sort(next.begin(), next.end(), [&](const Band& a, const Band& b) { return key(a) < key(b); });
    size_t used = 0;
    for (const Band& b : next) {
        if (used) {
            Band& a = next[used - 1];
            if (a.last + 1 == b.first && a.tile == b.tile && a.weight == b.weight &&
                a.low.slope == b.low.slope && a.low.intercept == b.low.intercept &&
                a.high.slope == b.high.slope && a.high.intercept == b.high.intercept) {
                a.last = b.last;
                continue;
            }
        }
        next[used++] = b;
    }
    next.resize(used);
    bands.swap(next);
}

struct Formation {
    Int a, b, m;
    pair<Int, Int> center() const { return {a + 5 * m / 2, b + 5 * m / 2}; }
    bool red(Int x, Int y) const {
        return x >= a && y >= b && x < a + 5 * m && y < b + 5 * m &&
               ((x - a) / m + (y - b) / m) % 2 == 0;
    }
};

constexpr int enumerationLimit = 512;

vector<Formation> enumerate(const vector<Band>& bands, Int pos, Int other, bool swapped) {
    if (count(bands) > enumerationLimit) return {};
    vector<Formation> candidates;
    for (const Band& b : bands) {
        for (Int t = b.first; t <= b.last; ++t) {
            Int m = 2 * t + 1;
            for (Int u = b.low(t); u <= b.high(t); ++u) {
                Int a = pos - u - b.tile * m;
                for (int j = b.tile % 2; j < 5; j += 2) {
                    auto [low, high] = offsetRange(other, m, j);
                    if (high < low) continue;
                    if (high - low + 1 > enumerationLimit - static_cast<int>(candidates.size())) return {};
                    for (Int v = low; v <= high; ++v) {
                        Int c = other - v - j * m;
                        candidates.push_back(swapped ? Formation{c, a, m} : Formation{a, c, m});
                    }
                }
            }
        }
    }
    return candidates;
}

using Word = uint64_t;
constexpr int maxWords = (enumerationLimit + 63) / 64;
struct AxisMasks {
    Int coordinate;
    array<Word, maxWords> even{}, odd{};
};

vector<AxisMasks> makeMasks(const vector<Formation>& candidates, bool yAxis) {
    vector<Int> coordinates;
    for (const Formation& f : candidates) {
        Int start = yAxis ? f.b : f.a;
        for (int k = 0; k <= 5; ++k) {
            Int p = start + k * f.m;
            if (p <= field) coordinates.push_back(p);
        }
    }
    sort(coordinates.begin(), coordinates.end());
    coordinates.erase(unique(coordinates.begin(), coordinates.end()), coordinates.end());
    vector<AxisMasks> result;
    for (Int p : coordinates) {
        AxisMasks masks{p};
        bool any = false;
        for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
            const Formation& f = candidates[i];
            Int offset = p - (yAxis ? f.b : f.a);
            if (offset < 0 || offset >= 5 * f.m) continue;
            auto& words = (offset / f.m) % 2 ? masks.odd : masks.even;
            words[i / 64] |= Word{1} << (i % 64);
            any = true;
        }
        if (any) result.push_back(masks);
    }
    return result;
}

pair<Int, Int> smallQuery(const vector<Formation>& candidates) {
    int n = static_cast<int>(candidates.size());
    auto xs = makeMasks(candidates, false);
    auto ys = makeMasks(candidates, true);
    int words = (n + 63) / 64;
    pair<Int, Int> best{};
    int bestImbalance = n;
    constexpr int treeLimit = 16;
    unsigned full = n <= treeLimit ? (1u << n) - 1 : 0;
    vector<pair<Int, Int>> positions(full + 1);
    vector<unsigned> masks;
    for (const auto& x : xs) {
        for (const auto& y : ys) {
            int hits = 0;
            unsigned mask = 0;
            for (int w = 0; w < words; ++w) {
                Word bits = (x.even[w] & y.even[w]) | (x.odd[w] & y.odd[w]);
                hits += popcount(bits);
                if (full) mask = static_cast<unsigned>(bits);
            }
            int imbalance = abs(n - 2 * hits);
            if (imbalance < bestImbalance) {
                bestImbalance = imbalance;
                best = {x.coordinate, y.coordinate};
                if (!full && imbalance == n % 2) return best;
            }
            if (full && hits > 0 && hits < n) {
                mask = min(mask, full ^ mask);
                if (positions[mask].first == 0) {
                    positions[mask] = {x.coordinate, y.coordinate};
                    masks.push_back(mask);
                }
            }
        }
    }
    if (!full) return best;
    vector<unsigned> equivalent(n);
    bool distinct = true;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (candidates[i].center() == candidates[j].center()) equivalent[i] |= 1u << j;
        }
        distinct &= popcount(equivalent[i]) == 1;
    }
    struct Score {
        int depth, total;
    };
    vector<Score> cache(full + 1, {-1, 0});
    unsigned rootQuery = 0;
    sort(masks.begin(), masks.end(), [&](unsigned a, unsigned b) {
        return abs(n - 2 * popcount(a)) < abs(n - 2 * popcount(b));
    });
    auto solve = [&](auto&& self, unsigned state) -> Score {
        Score& result = cache[state];
        if (result.depth >= 0) return result;
        if ((state & ~equivalent[countr_zero(state)]) == 0) return result = {0, 0};
        int size = popcount(state);
        int idealDepth = bit_width(static_cast<unsigned>(size - 1));
        int idealTotal = size * (idealDepth + 1) - (1 << idealDepth);
        result = {n + 1, n * n};
        for (unsigned query : masks) {
            unsigned yes = state & query;
            unsigned no = state ^ yes;
            if (!yes || !no) continue;
            if (popcount(yes) < popcount(no)) swap(yes, no);
            int childDepth = bit_width(static_cast<unsigned>(popcount(yes) - 1));
            if (distinct && 1 + childDepth > result.depth) continue;
            Score a = self(self, yes);
            if (1 + a.depth > result.depth) continue;
            Score b = self(self, no);
            Score trial{1 + max(a.depth, b.depth), size + a.total + b.total};
            if (tie(trial.depth, trial.total) < tie(result.depth, result.total)) {
                result = trial;
                if (state == full) rootQuery = query;
                if (distinct && result.depth == idealDepth && result.total == idealTotal) break;
            }
        }
        return result;
    };
    solve(solve, full);
    return positions[rootQuery];
}

pair<Int, Int> finishSmall(vector<Formation> candidates) {
    while (true) {
        auto answer = candidates.front().center();
        if (all_of(candidates.begin(), candidates.end(), [&](const Formation& f) { return f.center() == answer; })) {
            return answer;
        }
        auto [x, y] = smallQuery(candidates);
        bool red = ask(x, y);
        erase_if(candidates, [&](const Formation& f) { return f.red(x, y) != red; });
    }
}

Int weightedCount(const vector<Band>& bands) {
    Int total = 0;
    for (const Band& b : bands) total += count(b) * b.weight;
    return total;
}

void setWeights(vector<Band>& bands, bool unknownSize) {
    // Equal extra mass per size bucket keeps small formations from being
    // overwhelmed by the much greater number of large ones. Give extra
    // weight to tile midpoints and to offsets against a tile or meadow edge.
    constexpr int fullSizeBucket = 31;
    Int fullSize = (field / 5 - 1) / 2;
    enum class Offset { ordinary, edge, middle };
    struct Piece {
        Band band;
        int bucket;
        Offset kind;
    };
    vector<Piece> pieces;
    array<Int, 32> population{}, edges{}, middles{};
    Int volume = count(bands);
    for (const Band& source : bands) {
        for (Int t = source.first; t <= source.last;) {
            int k = bit_width(static_cast<unsigned>(t));
            Band b = source;
            b.first = t;
            b.last = min((1LL << k) - 1, b.last);
            if (unknownSize) {
                if (t == fullSize) k = fullSizeBucket;
                else b.last = min(b.last, fullSize - 1);
            }
            t = b.last + 1;
            population[k] += count(b);

            auto addEdge = [&](const Band& p) {
                edges[k] += count(p);
                pieces.push_back({p, k, Offset::edge});
            };
            clip(b, nullptr, &b.low, addEdge);
            Band high = b;
            if (restrictRange(high.first, high.last, plusOne(b.low), b.high)) {
                clip(high, &b.high, nullptr, addEdge);
            }
            Line low = plusOne(b.low);
            Line upper = b.high;
            --upper.intercept;
            clip(b, &low, &upper, [&](const Band& interior) {
                Line mid{1, 0}, before{1, -1}, after{1, 1};
                clip(interior, &mid, &mid, [&](const Band& p) {
                    middles[k] += count(p);
                    pieces.push_back({p, k, Offset::middle});
                });
                auto addOrdinary = [&](const Band& p) { pieces.push_back({p, k, Offset::ordinary}); };
                clip(interior, nullptr, &before, addOrdinary);
                clip(interior, &after, nullptr, addOrdinary);
            });
        }
    }
    Int bucketCount = count_if(population.begin(), population.end(), [](Int n) { return n != 0; });
    Int bucketBonus = unknownSize ? volume / bucketCount : 0;
    bands.clear();
    for (auto [b, k, kind] : pieces) {
        Int bonus = bucketBonus;
        if (unknownSize && (k == 1 || k == fullSizeBucket)) bonus += volume / 8;
        b.weight = 1 + bonus / population[k];
        Int mass = b.weight * population[k];
        if (kind == Offset::edge) b.weight += mass / 4 / edges[k];
        if (kind == Offset::middle) b.weight += mass / 8 / middles[k];
        bands.push_back(b);
    }
}

Int redCount(const vector<Band>& bands, Int delta) {
    Int total = 0;
    for (const Band& b : bands) {
        if (b.first == b.last) {
            Int t = b.first;
            Int m = 2 * t + 1;
            Int low = b.low(t) + delta;
            Int high = b.high(t) + delta;
            for (int k = -(b.tile / 2) * 2; k <= 4 - b.tile; k += 2) {
                total += b.weight * max<Int>(0, min(high, (k + 1) * m - 1) - max(low, k * m) + 1);
            }
        } else {
            redParts(b, delta, [&](const Band& p) { total += b.weight * count(p); });
        }
    }
    return total;
}

Int axisQuery(const vector<Band>& bands, Int pos, Int total, bool useBoundary) {
    Int best = 0;
    Int imbalance = total;
    Int targetImbalance = max(total % 2, total / 64);
    auto consider = [&](Int delta) {
        Int hits = redCount(bands, delta);
        Int error = abs(total - 2 * hits);
        if (error < imbalance) {
            imbalance = error;
            best = delta;
        }
        return hits;
    };
    // A crossing can be bisected even if the counts between the endpoints
    // are nonmonotone. Try a crossing on each side of the known red cell.
    auto bisect = [&](Int red, Int white) {
        while (abs(red - white) > 1 && imbalance > targetImbalance) {
            Int middle = (red + white) / 2;
            if (2 * consider(middle) > total) red = middle;
            else white = middle;
        }
    };
    Int left = -pos;
    Int right = field - pos + 1;
    // Bound the first possible white cell on either side. A nearby crossing
    // usually gives the most balanced query and avoids the far red runs.
    Int leftNear = 0;
    Int rightNear = 0;
    for (const Band& b : bands) {
        leftNear = max({leftNear, b.high(b.first) + 1, b.high(b.last) + 1});
        rightNear = max({rightNear, 2 * b.first + 1 - b.low(b.first), 2 * b.last + 1 - b.low(b.last)});
    }
    if (useBoundary) {
        Int minSize = field;
        Int minLeft = field;
        Int minRight = field;
        for (const Band& b : bands) {
            minSize = min(minSize, 2 * b.first + 1);
            minLeft = min({minLeft, b.low(b.first) + 1, b.low(b.last) + 1});
            minRight = min({minRight, 2 * b.first + 1 - b.high(b.first), 2 * b.last + 1 - b.high(b.last)});
        }
        auto testDistance = [&](Int low, Int high) {
            // The first white run has length at least max(low, minSize),
            // so this query still distinguishes the first boundary.
            return min((low + high) / 2, low + max(low, minSize) - 1);
        };
        if (minRight < rightNear) return testDistance(minRight, rightNear);
        if (minLeft < leftNear) return -testDistance(minLeft, leftNear);
        // Both distances fixed determines M and the offset, leaving only
        // the tile index to distinguish with the general splitter.
    }
    auto tryNear = [&](Int delta, Int outside) {
        if (delta == outside || 2 * consider(delta) <= total) bisect(0, delta);
    };
    if (leftNear <= rightNear) {
        tryNear(-leftNear, left);
        if (imbalance > targetImbalance) tryNear(rightNear, right);
    } else {
        tryNear(rightNear, right);
        if (imbalance > targetImbalance) tryNear(-leftNear, left);
    }
    if (imbalance <= targetImbalance) return best;

    for (const Band& b : bands) {
        for (int k = -(b.tile / 2) * 2; k <= (4 - b.tile) / 2 * 2 + 1; ++k) {
            Int lo = min(k * (2 * b.first + 1) - b.high(b.first), k * (2 * b.last + 1) - b.high(b.last));
            Int hi = max(k * (2 * b.first + 1) - b.low(b.first), k * (2 * b.last + 1) - b.low(b.last));
            for (Int delta : {lo - 1, hi, lo + (hi - lo) / 2}) {
                if (delta > left && delta < right && delta != 0) consider(delta);
                if (imbalance <= targetImbalance) return best;
            }
        }
    }
    if (imbalance > total / 4) {
        bisect(0, left);
        bisect(0, right);
    }
    if (best == 0) {
        // Two distinct projections have different red intervals. At a
        // transition of one of them their answers must therefore differ.
        const Band& first = bands.front();
        Int t = first.first;
        Int u = first.low(t);
        auto tryTransitions = [&](int tile, Int m, Int offset) {
            for (int k = -(tile / 2) * 2; k <= 4 - tile; k += 2) {
                for (Int delta : {k * m - offset, (k + 1) * m - offset}) {
                    if (left < delta && delta < right) consider(delta);
                }
            }
        };
        tryTransitions(first.tile, 2 * t + 1, u);
        if (u < first.high(t)) {
            tryTransitions(first.tile, 2 * t + 1, u + 1);
        } else if (t < first.last) {
            tryTransitions(first.tile, 2 * t + 3, first.low(t + 1));
        } else {
            const Band& second = bands[1];
            tryTransitions(second.tile, 2 * second.first + 1, second.low(second.first));
        }
    }
    return best;
}

pair<Int, Int> solve(Int x, Int y) {
    auto bands = initialBands(x, y);
    auto vertical = initialBands(y, x);
    bool swapped = count(vertical) < count(bands);
    if (swapped) {
        bands.swap(vertical);
        swap(x, y);
    }
    auto query = [&](Int delta, bool otherAxis) {
        Int a = x + (otherAxis ? 0 : delta);
        Int b = y + (otherAxis ? delta : 0);
        return swapped ? ask(b, a) : ask(a, b);
    };

    // A scan through the red anchor has runs of length M. Its first run
    // and its number of runs determine the board origin on this axis.
    for (int probes = 0;; ++probes) {
        if (auto candidates = enumerate(bands, x, y, swapped); !candidates.empty()) {
            return finishSmall(move(candidates));
        }
        if (count(bands) == 1) break;
        if (probes == 0) setWeights(bands, true);
        Int delta = axisQuery(bands, x, weightedCount(bands), probes >= 68);
        observe(bands, delta, query(delta, false));
    }
    const Band& known = bands.front();
    Int m = 2 * known.first + 1;
    Int origin = x - known.low(known.first) - known.tile * m;
    vertical.clear();
    // The two tile indices have the same parity at the known red cell.
    for (int j = known.tile % 2; j < 5; j += 2) {
        auto [low, high] = offsetRange(y, m, j);
        if (low <= high) vertical.push_back({known.first, known.first, {0, low}, {0, high}, j});
    }
    setWeights(vertical, false);
    for (int probes = 0; count(vertical) > 1; ++probes) {
        Int delta = axisQuery(vertical, y, weightedCount(vertical), probes >= 36);
        observe(vertical, delta, query(delta, true));
    }
    const Band& other = vertical.front();
    Int otherOrigin = y - other.low(other.first) - other.tile * m;
    Int a = origin + 5 * m / 2;
    Int b = otherOrigin + 5 * m / 2;
    return swapped ? pair(b, a) : pair(a, b);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    Int x, y;
    if (!(cin >> field >> x >> y)) return 0;
    auto [a, b] = solve(x, y);
    cout << "! " << a << ' ' << b << endl;
}

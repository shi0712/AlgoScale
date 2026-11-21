#include <bits/stdc++.h>
using namespace std;
using i64 = long long;

static constexpr i64 LIMIT = 1LL << 30;
static constexpr int SMALL = 22;
static constexpr int DEN = 1 << SMALL;

static i64 floorPower(i64 x) {
    return bit_floor(static_cast<unsigned long long>(x));
}

static i64 ask(int left, int right) {
    cout << "? " << left << ' ' << right << endl;
    i64 value;
    if (!(cin >> value) || value < 0) exit(0);
    return value;
}

// Fewest aligned dyadic tiles covering [a,b).
static int tileCount(i64 a, i64 b) {
    if (a == b) return 0;
    i64 len = b - a;
    if ((len & (len - 1)) == 0 && a % len == 0) return 1;
    i64 bit = floorPower(a ^ (b - 1));
    i64 mid = (b - 1) & -bit;
    return __builtin_popcountll(mid - a) + __builtin_popcountll(b - mid);
}

static i64 minLeaves(i64 a, i64 b, i64 cap) {
    if (a / cap == (b - 1) / cap) return tileCount(a, b);
    i64 left = (a + cap - 1) / cap;
    i64 right = b / cap;
    return right - left + __builtin_popcountll(left * cap - a)
                        + __builtin_popcountll(b - right * cap);
}

struct Shape {
    vector<int> prefix;
    int smallest, largest;
};

// All reachable short arrays, with their total mass normalized to DEN.
// A shared starting array is either the initial singleton or the result
// of a concatenation. Its two independent flatten histories give the next
// shared starting array.
static vector<vector<Shape>> shortShapes(int limit) {
    using Row = vector<int>;
    auto refine = [](vector<Row> &rows, set<Row> &seen, int maxLength) {
        for (size_t i = 0; i < rows.size(); ++i) {
            if ((int)rows[i].size() >= maxLength) continue;
            Row a = rows[i];
            int m = *max_element(a.begin(), a.end());
            for (size_t j = 0; j < a.size(); ++j) if (a[j] == m) {
                Row b = a;
                b[j] /= 2;
                b.insert(b.begin() + j, m / 2);
                if (seen.insert(b).second) rows.push_back(move(b));
            }
        }
    };
    vector<Row> bases{{DEN}};
    set<Row> all{bases.front()};
    for (size_t t = 0; t < bases.size(); ++t) {
        int length = bases[t].size();
        if (2 * length > limit) continue;
        vector<Row> refinements{bases[t]};
        set<Row> visited{bases[t]};
        refine(refinements, visited, limit - length);
        for (const Row &a : refinements) {
            for (const Row &b : refinements) {
                if (a.size() + b.size() > (size_t)limit) break;
                Row c = a;
                c.insert(c.end(), b.begin(), b.end());
                for (int &x : c) x /= 2;
                if (all.insert(c).second) bases.push_back(move(c));
            }
        }
    }
    refine(bases, all, limit);
    vector<vector<Shape>> table(limit + 1);
    for (const Row &a : all) {
        Shape s{{0}, *min_element(a.begin(), a.end()), *max_element(a.begin(), a.end())};
        for (int x : a) s.prefix.push_back(s.prefix.back() + x);
        table[a.size()].push_back(move(s));
    }
    return table;
}

struct Hypothesis {
    const Shape *shape;
    int start;
    i64 scale, answer;
    // DEN and scale are powers of two. Split the calculation to keep it
    // within 64 bits even when the mass of the full array is large.
    i64 relative(int index) const {
        i64 x = shape->prefix[index - start];
        return scale >= DEN ? x * (scale / DEN) : x / (DEN / scale);
    }
    int end() const { return start + (int)shape->prefix.size() - 1; }
};

static i64 scaled(int value, i64 total) {
    return total >= DEN ? value * (total / DEN) : value / (DEN / total);
}

// For a short region, choose queries by the partition they induce on the
// remaining possibilities. Prefixes outside a candidate's region still
// give a comparison with its two boundaries.
static i64 distinguish(vector<Hypothesis> pool, i64 base, i64 regionEnd, bool unknownScale = false) {
    struct Query { int begin, end; bool prefix; };
    struct Bucket { int count = 0; double weight = 0; i64 first = 0; bool mixed = false; };
    for (;;) {
        i64 answer = pool[0].answer;
        if (!unknownScale && all_of(pool.begin(), pool.end(), [&](const auto &h) { return h.answer == answer; }))
            return answer;
        int minStart = INT_MAX, maxStart = -1, minEnd = INT_MAX, maxEnd = -1;
        for (const auto &h : pool) {
            minStart = min(minStart, h.start); maxStart = max(maxStart, h.start);
            minEnd = min(minEnd, h.end()); maxEnd = max(maxEnd, h.end());
        }
        int balanced = count_if(pool.begin(), pool.end(), [](const auto &h) {
            return h.shape->largest <= 2 * h.shape->smallest;
        });
        double bonus = balanced ? 1.0 * pool.size() / balanced : 0;
        auto predict = [&](const Hypothesis &h, Query q) -> i64 {
            if (q.prefix) {
                if (q.end < h.start) return -2;
                if (q.end > h.end()) return -3;
                return base + h.relative(q.end);
            }
            return h.relative(q.end) - h.relative(q.begin);
        };
        double best = numeric_limits<double>::infinity();
        Query choice{};
        unordered_map<i64, Bucket> groups;
        auto consider = [&](Query q) {
            groups.clear();
            for (const auto &h : pool) {
                i64 value = predict(h, q), key = h.answer;
                if (unknownScale) {
                    int exponent = countr_zero((unsigned long long)value);
                    value >>= exponent;
                    key = countr_zero((unsigned long long)h.answer) - exponent;
                }
                auto &g = groups[value];
                g.weight += 1 + (h.shape->largest <= 2 * h.shape->smallest ? bonus : 0);
                if (g.count++ == 0) g.first = key;
                else if (g.first != key) g.mixed = true;
            }
            if (groups.size() < 2 && !unknownScale) return;
            double cost = 0;
            for (const auto &entry : groups) {
                const auto &g = entry.second;
                if (g.mixed) cost += g.weight * (1.0 + 0.24 * log2(g.count));
            }
            if (cost < best) { best = cost; choice = q; }
        };
        for (int i = max(1, minStart); i <= maxEnd && best > 0; ++i)
            consider({0, i, true});
        for (int i = max(1, maxStart); i < minEnd && best > 0; ++i)
            for (int j = i + 1; j <= minEnd && best > 0; ++j)
                consider({i, j, false});
        i64 reply = ask(choice.begin + 1, choice.end);
        if (unknownScale) {
            // Scaling a whole array by a power of two preserves the odd
            // part of every interval sum. One answer fixes the scale of
            // each shape that survives this first query.
            i64 odd = reply / (reply & -reply);
            vector<Hypothesis> survivors;
            for (auto h : pool) {
                i64 value = predict(h, choice);
                if (value / (value & -value) != odd) continue;
                h.scale = (reply / odd) * (DEN / (value & -value));
                h.answer = scaled(h.shape->largest, h.scale);
                if (h.scale >= DEN / h.shape->smallest && h.answer <= LIMIT)
                    survivors.push_back(h);
            }
            pool = move(survivors);
            unknownScale = false;
            continue;
        }
        if (choice.prefix && reply < base) reply = -2;
        if (choice.prefix && reply > regionEnd) reply = -3;
        erase_if(pool, [&](const auto &h) { return predict(h, choice) != reply; });
    }
}

struct Mark {
    i64 mass;
    int index, low = 0, high = 0;
};

struct Geometry {
    vector<Mark> marks;
    bool possible = true;
    int minLength = 0, maxLength = 0, minDiff = 0, maxDiff = 0;
    const Mark &at(i64 v) const {
        return *lower_bound(marks.begin(), marks.end(), v,
                            [](const Mark &m, i64 x) { return m.mass < x; });
    }
};

class Search {
    int n;
    i64 total = 0, unit = 1, lower = 1, cap = LIMIT;
    map<i64, int> observed;
    vector<vector<Shape>> shapes;

    // If the maximum exceeds lower, every block of mass 2*lower lies
    // within a copy of an initial element. Its internal splits are all
    // maximum-element splits, so its leaf sizes differ by at most twofold.
    //
    // At any larger fixed dyadic width, the blocks are copies of a common
    // starting block, with local maximum-element splits. A boundary at p
    // introduced in one copy costs a split of size 2*lowbit(p). If another
    // copy lacks that boundary, the first copy's final maximum is at most
    // this split size.
    struct Locations {
        i64 width;
        vector<int> prefix;
        bool any(i64 a, i64 b) const {
            if (b - a < width) return false;
            return prefix[b / width] != prefix[a / width];
        }
    };

    int extraProbes = 0;

    Locations possibleLocations(i64 a, i64 b, int &probe) const {
        int size = total / unit, cell = 2 * lower / unit;
        int cells = size / cell;
        vector<unsigned char> required(size + 1), absent(size + 1);
        for (auto it = observed.begin(), jt = next(it); jt != observed.end(); ++it, ++jt) {
            int p = it->first / unit, q = jt->first / unit;
            int count = jt->second - it->second;
            int spare = count - minLeaves(it->first, jt->first, cap);
            int missing = q - p - count;
            int pos = p;
            while (pos < q) {
                int width = min<i64>(floorPower(q - pos), cap / unit);
                if (pos) width = min(width, pos & -pos);
                required[pos] = true;
                // Including a new point requires its chain of ancestors.
                // Excluding it removes all its descendants as well.
                int minStep = width >> min(spare, 30);
                for (int k = pos + 1; k < pos + width; ++k) {
                    int step = k & -k;
                    required[k] = 2 * step - 1 > missing;
                    absent[k] = step < minStep;
                }
                pos += width;
            }
        }
        vector<int> fine, removed(cells + 1);
        bool impossible = false;
        for (int start = 0; start < size; start += cell) {
            int smallest = cell, largestMissing = 0;
            for (int k = start + 1; k < start + cell; ++k) {
                if (required[k]) smallest = min(smallest, k & -k);
                if (absent[k]) largestMissing = max(largestMissing, k & -k);
            }
            for (int k = start + 1; k < start + cell; ++k) {
                int step = k & -k;
                if (step >= 2 * smallest) required[k] = true;
                if (step < largestMissing) absent[k] = true;
                if (absent[k] && required[k]) impossible = true;
                if (required[k] && 2 * step < cell) fine.push_back(k);
            }
            if (required[start + cell / 2]) {
                ++removed[start / cell]; --removed[start / cell + 1];
            }
        }
        // Fold absent positions by their residue modulo each block width.
        // For each fine boundary only the largest conflicting block matters.
        vector<vector<unsigned char>> folds;
        folds.push_back(move(absent));
        for (int w = size / 2; w >= cell && !fine.empty(); w /= 2) {
            const auto &larger = folds.back();
            vector<unsigned char> nextFold(w);
            for (int i = 0; i < w; ++i) nextFold[i] = larger[i] | larger[i+w];
            folds.push_back(move(nextFold));
        }
        int last = (int)folds.size() - 1;
        for (int p : fine) {
            if (!folds[last][p % (size >> last)]) continue;
            int lo = 0, hi = last;
            while (hi - lo > 1) {
                int mid = (lo + hi) / 2;
                if (folds[mid][p % (size >> mid)]) hi = mid;
                else lo = mid;
            }
            int w = size >> hi;
            int first = p / w * w / cell;
            ++removed[first]; --removed[first + w / cell];
        }
        Locations loc{2 * lower, vector<int>(cells + 1)};
        int coverage = 0;
        for (int i = 0; i < cells; ++i) {
            coverage += removed[i];
            loc.prefix[i+1] = loc.prefix[i] + (!impossible && coverage == 0);
        }
        // A short query next to an existing prefix can sometimes settle
        // the whole search: it tests for the same small split in two
        // different copies. Estimate its payoff from the measured local
        // proportions of the two leaf sizes bordering the average.
        i64 width = b - a;
        if (extraProbes < 5 && lower >= 2*unit && width / lower >= 32 &&
            loc.any(a,b) && width < total && !fine.empty()) {
            int level = countr_zero((unsigned long long)(total / width));
            if (level >= (int)folds.size()) return loc;
            double best = 0.29 + extraProbes * 0.055;
            for (auto it = observed.begin(), jt = next(it); jt != observed.end(); ++it, ++jt) {
                int count = jt->second - it->second;
                if (count < 6) continue;
                double frac = clamp((double)count * lower / (jt->first - it->first) - 1., 0., 1.);
                for (int dir : {-1, 1}) {
                    auto anchor = dir > 0 ? it : jt;
                    i64 p = anchor->first;
                    if (p % (lower/2)) continue;
                    map<i64, double> ways{{p, 1.0}};
                    for (int k = 1; k <= 4; ++k) {
                        map<i64, double> more;
                        for (auto [v, prob] : ways) {
                            double split = v % lower == 0 ? frac : 1.;
                            if (split > 0) more[v + dir*(lower/2)] += prob * split;
                            if (split < 1) more[v + dir*lower] += prob * (1.-split);
                        }
                        ways = move(more);
                        double hit = 0;
                        for (auto [v, prob] : ways) {
                            if (v <= it->first || v >= jt->first) continue;
                            i64 l = min(v,p), r = max(v,p);
                            for (i64 q = (l / lower) * lower + lower/2; q <= r; q += lower) {
                                if (q < l) continue;
                                int res = (q % width) / unit;
                                bool wantPresent = a < q && q < b && folds[level][res];
                                bool wantAbsent = required[a / unit + res];
                                if (!wantPresent && !wantAbsent) continue;
                                bool endpoint = q == l || q == r;
                                i64 needed = endpoint ? 0 : minLeaves(l, q, cap) + minLeaves(q, r, cap);
                                bool present = endpoint || needed == minLeaves(l, r, cap)
                                             || (r-l)/unit - k < lower/unit - 1;
                                if ((wantPresent && present) || (wantAbsent && needed > k)) {
                                    hit += prob;
                                    break;
                                }
                            }
                        }
                        if (hit > best + 1e-8) {
                            best = hit;
                            probe = anchor->second + k * dir;
                        }
                    }
                }
            }
        }
        return loc;
    }

    // Add the required mass boundaries. Between consecutive observed
    // prefix sums the number of leaves is fixed. Each subinterval has an
    // independently attainable range of leaf counts under the dyadic bounds.
    Geometry geometry(i64 a, i64 b, bool split) const {
        i64 mid = (a + b) / 2;
        Geometry g;
        for (auto [v, i] : observed) g.marks.push_back({v, i});
        if (split && (b - a < 2 || mid % unit)) { g.possible = false; return g; }
        if (!observed.count(a)) g.marks.push_back({a, -1});
        if (!observed.count(b)) g.marks.push_back({b, -1});
        if (split && !observed.count(mid)) g.marks.push_back({mid, -1});
        sort(g.marks.begin(), g.marks.end(), [](auto x, auto y) { return x.mass < y.mass; });
        vector<i64> lo(g.marks.size()), hi(g.marks.size());
        int end = 0;
        while (end + 1 < (int)g.marks.size()) {
            int begin = end++;
            while (g.marks[end].index < 0) ++end;
            i64 count = g.marks[end].index - g.marks[begin].index;
            i64 lowSum = 0, highSum = 0;
            i64 leftMin = 0, leftMax = 0, rightMin = 0, rightMax = 0;
            for (int j = begin + 1; j <= end; ++j) {
                i64 p = g.marks[j-1].mass, q = g.marks[j].mass;
                lo[j] = minLeaves(p, q, cap);
                hi[j] = (q - p) / unit;
                if (p >= a && q <= b) {
                    if (split && p >= mid) { rightMin += lo[j]; rightMax += hi[j]; }
                    else { leftMin += lo[j]; leftMax += hi[j]; }
                }
                if (lo[j] > hi[j]) { g.possible = false; return g; }
                lowSum += lo[j]; highSum += hi[j];
            }
            if (count < lowSum || count > highSum) { g.possible = false; return g; }
            i64 beforeMin = 0, beforeMax = 0;
            for (int j = begin; j <= end; ++j) {
                if (j != begin) { beforeMin += lo[j]; beforeMax += hi[j]; }
                g.marks[j].low = g.marks[begin].index + max(beforeMin, count - (highSum - beforeMax));
                g.marks[j].high = g.marks[begin].index + min(beforeMax, count - (lowSum - beforeMin));
            }
            i64 spare = count - lowSum, other = highSum - lowSum - (leftMax-leftMin) - (rightMax-rightMin);
            g.minLength += leftMin + rightMin + max(0LL, spare - other);
            g.maxLength += leftMin + rightMin + min(spare, leftMax-leftMin + rightMax-rightMin);
            // Maximize (#left - #right) by assigning spare leaves to the
            // left first, then outside the node, and finally to the right.
            g.maxDiff += leftMin - rightMin + min(spare, leftMax-leftMin)
                       - max(0LL, spare - (leftMax-leftMin) - other);
            g.minDiff += leftMin - rightMin - min(spare, rightMax-rightMin)
                       + max(0LL, spare - (rightMax-rightMin) - other);
        }
        return g;
    }

    i64 intervalUpper(i64 a, i64 b) const {
        i64 best = 0;
        for (auto it = prev(observed.upper_bound(a)); it->first < b; ++it) {
            auto jt = next(it);
            i64 p = max(a, it->first), q = min(b, jt->first);
            i64 count = jt->second - it->second;
            i64 room = jt->first - it->first - (count - 1) * unit;
            i64 size = floorPower(min({cap, room, q - p}));
            while ((p + size - 1) / size * size + size > q) size /= 2;
            best = max(best, size);
        }
        return best;
    }

    void improveLower() {
        for (auto it = observed.begin(), jt = next(it); jt != observed.end(); ++it, ++jt)
            while (minLeaves(it->first, jt->first, lower) > jt->second - it->second)
                lower *= 2;
    }

    // Suppose this short block attains the global maximum m. After
    // splitting all values larger than m, the common template for blocks
    // of this width is determined except for splits of m into m/2 pairs.
    // Undo all such optional pairs to obtain a common coarser template.
    bool compatibleCopies(const Shape &s, i64 width, i64 m) const {
        vector<i64> row;
        for (int x : s.prefix) row.push_back(scaled(x, width));
        vector<i64> skeleton{0};
        for (int i = 1; i + 1 < (int)row.size(); ++i) {
            bool pair = (row[i] & -row[i]) * 2 == m &&
                        (row[i] - row[i-1]) * 2 == m &&
                        (row[i+1] - row[i]) * 2 == m;
            if (!pair) skeleton.push_back(row[i]);
        }
        skeleton.push_back(width);
        map<i64, i64> caps;
        for (auto [p, index] : observed) {
            i64 offset = p % width;
            if (!binary_search(row.begin(), row.end(), offset)) {
                auto [it, fresh] = caps.emplace(p / width, m);
                it->second = min(it->second, 2 * (offset & -offset));
            }
        }
        auto fragment = [&](i64 block, i64 l, i64 r) {
            i64 maxSize = m, count = 0;
            if (auto it = caps.find(block); it != caps.end()) maxSize = it->second;
            for (size_t i = 1; i < skeleton.size(); ++i) {
                i64 from = max(l, skeleton[i-1]), to = min(r, skeleton[i]);
                if (from < to) count += minLeaves(from, to, maxSize);
            }
            return count;
        };
        for (auto it = observed.begin(), jt = next(it); jt != observed.end(); ++it, ++jt) {
            i64 p = it->first, q = jt->first, first = p / width, last = (q-1) / width;
            i64 count;
            if (first == last) count = fragment(first, p % width, q - first * width);
            else count = fragment(first, p % width, width) + fragment(last, 0, q - last * width)
                       + (last - first - 1) * (i64)(skeleton.size() - 1);
            if (count > jt->second - it->second) return false;
        }
        return true;
    }

    // The interior of every actual dyadic subtree is reachable on its
    // own. Flatten operations inherited from its ancestors are legal
    // maximum-element splits when restricted to that subtree.
    optional<i64> solveShort(i64 a, i64 b, const Geometry &g, const Locations &loc) {
        const auto &left = g.at(a), &right = g.at(b);
        if (g.maxLength > SMALL || right.high - left.low > 2 * SMALL + 8) return nullopt;
        vector<Hypothesis> pool;
        i64 mass = b - a;
        shapes = shortShapes(g.maxLength);
        for (int len = g.minLength; len <= g.maxLength; ++len) {
            for (const Shape &s : shapes[len]) {
                if (mass < DEN && s.smallest < DEN / mass) continue;
                i64 big = scaled(s.largest, mass);
                if (big > cap || scaled(s.smallest, mass) < unit) continue;
                int low = max(left.low, right.low - len);
                int high = min(left.high, right.high - len);
                for (auto it = observed.lower_bound(a); it != observed.end() && it->first <= b && low <= high; ++it) {
                    i64 delta = it->first - a;
                    if (mass >= DEN && delta % (mass / DEN)) { high = -1; break; }
                    int norm = mass >= DEN ? delta / (mass / DEN) : delta * (DEN / mass);
                    auto found = lower_bound(s.prefix.begin(), s.prefix.end(), norm);
                    if (found == s.prefix.end() || *found != norm) { high = -1; break; }
                    int start = it->second - (found - s.prefix.begin());
                    low = max(low, start); high = min(high, start);
                }
                if (low <= high && big > lower) {
                    bool fits = true;
                    for (size_t i = 1; i < s.prefix.size(); ++i) {
                        i64 p = a + scaled(s.prefix[i-1], mass), q = a + scaled(s.prefix[i], mass);
                        if (q-p > lower && loc.prefix[q / loc.width] - loc.prefix[p / loc.width] != (q-p) / loc.width) {
                            fits = false;
                            break;
                        }
                    }
                    if (!fits || !compatibleCopies(s, mass, big)) continue;
                }
                for (int start = low; start <= high; ++start)
                    pool.push_back({&s, start, mass, max(lower, big)});
            }
        }
        return distinguish(move(pool), a, b);
    }

public:
    explicit Search(int size) : n(size) {}

    i64 run() {
        if (n <= SMALL) {
            vector<Hypothesis> pool;
            shapes = shortShapes(n);
            for (const Shape &s : shapes[n]) pool.push_back({&s, 0, DEN, s.largest});
            return distinguish(move(pool), 0, LLONG_MAX, true);
        }
        // The last leaf is at most half the total. The prefix fixes the
        // unique power-of-two total and also reveals the last leaf.
        i64 prefix = ask(1, n - 1);
        total = 2 * floorPower(prefix);
        observed = {{0, 0}, {prefix, n - 1}, {total, n}};
        // A new minimum is made by splitting a uniform array. If it is
        // first made in one copy just before concatenation, the other
        // copy can at most double its total mass. Later shared copies
        // cannot increase the average while keeping that same minimum.
        if (total >= 4LL * n) unit = 2 * floorPower(total / (4LL * n));
        i64 a = 0, b = total;
        for (;;) {
            improveLower();
            cap = min(cap, max(lower, b - a));
            if (intervalUpper(a, b) <= lower) return lower;
            int probe = -1;
            Locations loc = possibleLocations(a, b, probe);
            if (!loc.any(a, b)) return lower;
            Geometry node = geometry(a, b, false);
            for (i64 v : {a, b}) {
                const auto &m = node.at(v);
                if (m.low == m.high) observed.emplace(v, m.low);
            }
            if (auto ans = solveShort(a, b, node, loc)) return *ans;
            i64 mid = (a + b) / 2;
            Geometry halves = geometry(a, b, true);
            if (!halves.possible) return max(lower, b - a);
            bool canBeLeaf = node.minLength == 1 && b - a <= cap;
            if (canBeLeaf) {
                const auto &first = node.at(a), &last = node.at(b);
                int neighbor = -1;
                if (first.low == first.high) neighbor = first.low + 1;
                else if (last.low == last.high) neighbor = last.low - 1;
                if (neighbor > 0) {
                    observed.emplace(ask(1, neighbor), neighbor);
                    continue;
                }
            }
            if (!canBeLeaf) {
                const auto &m = halves.at(mid);
                if (m.low == m.high) observed.emplace(mid, m.low);
                if (mid - a <= lower) return lower;
                i64 lu = intervalUpper(a, mid), ru = intervalUpper(mid, b);
                if (max(lu, ru) <= lower) return lower;
                if (!loc.any(a, mid)) lu = lower;
                if (!loc.any(mid, b)) ru = lower;
                if (max(lu, ru) <= lower) return lower;
                // Both half-blocks refine the same starting array. The
                // multiset after k greedy splits depends only on k, so
                // the shorter half has at least the longer half's maximum.
                bool left = halves.maxDiff <= 0 || ru <= lower;
                bool right = halves.minDiff >= 0 || lu <= lower;
                if (left || right) {
                    if (left && right) {
                        if (min(lu, ru) <= lower) return lower;
                        left = lu <= ru;
                    }
                    if (left) b = mid; else a = mid;
                    continue;
                }
            }

            if (probe > 0) {
                ++extraProbes;
                observed.emplace(ask(1, probe), probe);
                continue;
            }
            // Compare the midpoint index with the mean of the endpoint
            // indices. Often its sign is settled by a single probe, even
            // if none of the three indices is yet known exactly.
            const Mark *target = &halves.at(mid);
            if (!canBeLeaf) {
                for (i64 v : {a,b}) {
                    const auto &m = halves.at(v);
                    int weight = target->mass == mid ? 2 : 1;
                    if (m.high - m.low > weight * (target->high - target->low)) target = &m;
                }
            }
            auto it = observed.lower_bound(target->mass);
            auto prevIt = prev(it);
            int lo = target->low, hi = target->high;
            long double fraction = (long double)(target->mass - prevIt->first) / (it->first - prevIt->first);
            int index = lround(prevIt->second + fraction * (it->second - prevIt->second));
            if (target->mass == mid && !canBeLeaf) {
                const auto &x = halves.at(a), &y = halves.at(b);
                int firstThreshold = (x.low + y.low) / 2;
                int lastThreshold = (x.high + y.high + 1) / 2;
                index = clamp(index, firstThreshold, lastThreshold);
            }
            int margin = (hi - lo) / 6;
            index = clamp(index, lo + margin, hi - margin);
            observed.emplace(ask(1, index), index);
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n) || n < 1) return 0;
    i64 answer = Search(n).run();
    cout << "! " << answer << endl;
}

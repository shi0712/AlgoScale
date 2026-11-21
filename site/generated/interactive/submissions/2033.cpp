#include <bits/stdc++.h>
using namespace std;

constexpr int valueLimit = 1'000'000'000;

class Solver {
    struct Point {
        int index;
        int value;
    };

    int n;
    vector<int> order;
    int next = 0;
    vector<bool> excluded;
    unordered_map<int, int> answers;
    array<int, 2> equalPair{};
    int equalTriples = 0, triedTriples = 0;

    // These coordinates share one origin and orientation. The interval spans
    // all processed indices that could still hold the zero.
    vector<Point> points;
    unordered_map<int, int> occurrences;
    Point left{}, right{};

    static int key(array<int, 3> indices) {
        sort(indices.begin(), indices.end());
        return (indices[0] * 1024 + indices[1]) * 1024 + indices[2];
    }

    int ask(int a, int b, int c) {
        int code = key({a, b, c});
        auto previous = answers.find(code);
        if (previous != answers.end()) return previous->second;
        cout << "? " << a << ' ' << b << ' ' << c << endl;
        int result;
        if (!(cin >> result) || result < 0) exit(0);
        answers.emplace(code, result);
        return result;
    }

    [[noreturn]] static void report(int a, int b) {
        cout << "! " << a << ' ' << b << endl;
        exit(0);
    }

    [[noreturn]] static void report(const vector<int>& indices) {
        report(indices.front(), indices.back());
    }

    static array<int, 3> without(const array<int, 4>& group, int omit) {
        array<int, 3> result;
        int k = 0;
        for (int i = 0; i < 4; ++i) {
            if (i != omit) result[k++] = group[i];
        }
        return result;
    }

    void excludeEqual(const array<int, 3>& group) {
        for (int i : group) excluded[i] = true;
        equalPair = {group[0], group[1]};
    }

    [[noreturn]] void finishFullTriple(int x, int y, int z) {
        int other = 1;
        while (other == x || other == y || other == z) ++other;
        if (ask(y, z, other) == valueLimit) report(y, z);
        if (ask(x, z, other) == valueLimit) report(x, z);
        report(x, y);
    }

    int remaining() const { return n - next; }
    int take() { return order[next++]; }

    void observe(int index, int value) {
        points.push_back({index, value});
        ++occurrences[value];
        if (points.size() == 1 || value < left.value) left = {index, value};
        if (points.size() == 1 || value > right.value) right = {index, value};
    }

    bool possibleZero(Point p) const {
        return !excluded[p.index] && occurrences.at(p.value) == 1;
    }

    void markRepeated(int value) {
        int& count = occurrences[value];
        count = max(count + 1, 2);
    }

    void finishIfReady() {
        if (right.value - left.value == valueLimit) report(left.index, right.index);
        if (remaining() > 2) return;
        vector<int> candidates;
        if (possibleZero(left)) candidates.push_back(left.index);
        if (possibleZero(right)) candidates.push_back(right.index);
        if (candidates.size() + remaining() > 2) return;
        for (int i = next; i < n; ++i) candidates.push_back(order[i]);
        report(candidates);
    }

    [[noreturn]] void finishOne(int pivot) {
        int x = take();
        int span = ask(left.index, right.index, x);
        if (span == right.value - left.value) report(left.index, right.index);
        // For a strictly interior pivot, this triple preserves the larger
        // span exactly when x extends the right side.
        if (ask(left.index, pivot, x) == span) report(left.index, x);
        report(right.index, x);
    }

    [[noreturn]] void finishTwo() {
        int x = take();
        int y = take();
        int a = ask(left.index, x, y);
        if (a == 0) report(left.index, right.index);
        if (a == valueLimit && !possibleZero(left)) report(x, y);
        int b = ask(right.index, x, y);
        int span = max(a, b);
        if (span <= right.value - left.value) report(left.index, right.index);
        if (a == b) report(x, y);
        Point retained = a > b ? left : right;
        if (!possibleZero(retained)) report(x, y);
        if (ask(left.index, right.index, x) == span) report(retained.index, x);
        report(retained.index, y);
    }

    // With two fixed endpoints and an interior pivot, these ranges recover
    // the unordered coordinates of the pivot and a new point.
    void locateSeedPivots(int a, int b, int low, int high) {
        if (low == high) {
            observe(a, low);
            observe(b, high);
            return;
        }
        int x = take();
        int l = left.index, r = right.index, span = right.value;
        int fromLeft = ask(l, a, x);
        if (fromLeft == valueLimit) report(l, x);
        int fromRight = ask(r, a, x);
        if (fromRight == valueLimit) report(r, x);
        int p, position;
        if (fromLeft > span && fromLeft > fromRight) {
            position = fromLeft;
            p = fromLeft - fromRight;
        } else if (fromRight > span) {
            position = span - fromRight;
            p = fromLeft + position;
        } else {
            int minimum = span - fromRight;
            int maximum = fromLeft;
            if (minimum == low && maximum == high) {
                // x equals b, so this range gives b's coordinate directly.
                p = low + high - ask(l, b, x);
            } else {
                p = (minimum == low || maximum == low) ? low : high;
            }
            position = p == minimum ? maximum : minimum;
        }
        observe(a, p);
        observe(b, low + high - p);
        observe(x, position);
    }

    bool startWithEqualPair(const vector<int>& candidates) {
        points.clear();
        occurrences.clear();
        observe(equalPair[0], 0);
        observe(equalPair[1], 0);
        Point positive{};
        for (int index : candidates) {
            int distance = ask(equalPair[0], equalPair[1], index);
            if (distance == valueLimit) report(index, index);
            if (distance && positive.index) {
                int span = ask(equalPair[0], positive.index, index);
                if (span > max(positive.value, distance)) distance = -distance;
            } else if (distance) {
                positive = {index, distance};
            }
            observe(index, distance);
        }
        if (left.value != right.value) return true;
        for (int i : candidates) excluded[i] = true;
        return false;
    }

    Point bestPivot() const {
        Point best{};
        int bestRadius = -1;
        for (Point p : points) {
            if (p.index == left.index || p.index == right.index) continue;
            int radius = min(p.value - left.value, right.value - p.value);
            if (radius > bestRadius) {
                best = p;
                bestRadius = radius;
            }
        }
        return best;
    }

    // near has local coordinate 0 and far has coordinate span. The two
    // endpoint queries determine whether either side of the range grows.
    void compareEndpoints(Point near, Point far, Point pivot, int x, int y,
                          int nearSpread, bool pairSpansNear, int tripleSpread) {
        int sign = near.value < far.value ? 1 : -1;
        int span = sign * (far.value - near.value);
        int p = sign * (pivot.value - near.value);
        auto save = [&](int index, int position) {
            observe(index, near.value + sign * position);
        };
        int farSpread = ask(far.index, x, y);
        bool knownGap = pairSpansNear || (nearSpread == farSpread && nearSpread >= span);
        if (knownGap && tripleSpread == nearSpread) ++next;
        if (max(nearSpread, farSpread) <= span) {
            if (nearSpread == span) markRepeated(far.value);
            if (farSpread == span) markRepeated(near.value);
        } else if (nearSpread > farSpread) {
            if (!possibleZero(near) &&
                (remaining() == 0 || nearSpread == valueLimit)) report(x, y);
            int xRange = ask(near.index, pivot.index, x);
            if (xRange > p) save(x, xRange);
            if (xRange != nearSpread) save(y, nearSpread);
        } else if (farSpread > nearSpread) {
            if (!possibleZero(far) &&
                (remaining() == 0 || farSpread == valueLimit)) report(x, y);
            int xRange = ask(far.index, pivot.index, x);
            int bottom = span - farSpread;
            if (pairSpansNear) {
                int top = bottom + nearSpread;
                if (xRange == farSpread) {
                    save(x, bottom);
                    save(y, top);
                } else {
                    save(x, top);
                    save(y, bottom);
                }
            } else {
                if (xRange > span - p) save(x, span - xRange);
                if (xRange != farSpread) save(y, bottom);
            }
        } else {
            // The new pair brackets both old endpoints and spans the range.
            if (remaining() == 0 || nearSpread == valueLimit) report(x, y);
            int xRange = ask(near.index, pivot.index, x);
            bool xIsLower;
            if (xRange < span) {
                xIsLower = true;
            } else if (xRange > nearSpread + p - span) {
                xIsLower = false;
            } else {
                xIsLower = ask(far.index, pivot.index, x) == span - p + xRange;
            }
            if (xIsLower) {
                save(x, p - xRange);
                save(y, p - xRange + nearSpread);
            } else {
                save(x, xRange);
                save(y, xRange - nearSpread);
            }
        }
    }

    double pivotGain(int span, int radius, double value) const {
        double newRadius = min(value, span - value);
        if (newRadius <= radius) return 0;
        return 1.5 * remaining() * (newRadius * newRadius - double(radius) * radius) /
               (double(span) * span);
    }

    // If the pair is already certified, a single extra query can sometimes
    // identify a better pivot. Estimate its payoff using the observed spread.
    bool worthSampling(int span, int p, int spread) const {
        if (p == span / 2 || spread == 0) return false;
        double gain = pivotGain(span, p, p + spread) / 6;
        for (int sample = 0; sample < 4; ++sample) {
            gain += pivotGain(span, p, p + spread * ((sample + 0.5) / 4)) / 12;
        }
        return gain > 2.5;
    }

    // Returns the primary spread and the number of screening queries saved
    // compared with querying both endpoints.
    pair<int, int> processPair(Point near, Point far, Point pivot, int x, int y,
                               bool centered, int tripleSpread,
                               vector<pair<int, int>>& sampled, int sampleLimit) {
        if (!centered) {
            int spread = ask(near.index, x, y);
            if (spread == valueLimit && !possibleZero(near)) report(x, y);
            if (spread == 0) {
                markRepeated(near.value);
                if (tripleSpread > valueLimit - (right.value - left.value)) {
                    int sign = near.value < far.value ? 1 : -1;
                    observe(take(), near.value + sign * tripleSpread);
                }
            } else {
                compareEndpoints(near, far, pivot, x, y, spread, false, tripleSpread);
            }
            return {spread, 0};
        }

        int sign = near.value < far.value ? 1 : -1;
        int span = sign * (far.value - near.value);
        int p = sign * (pivot.value - near.value);
        auto save = [&](int index, int position) {
            observe(index, near.value + sign * position);
        };
        int spread = ask(pivot.index, x, y);
        if (spread == valueLimit) report(x, y);
        if (spread <= p) {
            if (tripleSpread >= 0 && spread + tripleSpread <= p) ++next;
            pair sample = {pivot.index, spread};
            if (int(sampled.size()) < sampleLimit && worthSampling(span, p, spread) &&
                ranges::find(sampled, sample) == sampled.end()) {
                sampled.push_back(sample);
                int xValue = ask(near.index, pivot.index, x);
                if (xValue > p) save(x, xValue);
            }
            return {spread, 1};
        }
        int nearSpread = ask(near.index, x, y);
        if (nearSpread > spread) {
            // The near endpoint strictly lowers the triple minimum.
            int top = nearSpread;
            int bottom = top - spread;
            if (bottom < p && tripleSpread == spread) ++next;
            if (top > span && !possibleZero(near) &&
                (remaining() == 0 || top == valueLimit)) report(x, y);

            if (top > span || pivotGain(span, p, top) > 1.6) {
                int xValue = ask(near.index, pivot.index, x);
                if (xValue == top) {
                    save(x, top);
                    if (bottom < p) save(y, bottom);
                } else {
                    save(x, bottom < p ? bottom : xValue);
                    save(y, top);
                }
            } else if (top == span) {
                markRepeated(far.value);
            }
        } else if (nearSpread < spread) {
            // The pivot is above both new points. The far endpoint stays.
            int bottom = p - spread;
            if (nearSpread > -bottom && tripleSpread == nearSpread) ++next;
            if (!possibleZero(far) &&
                (remaining() == 0 || span - bottom == valueLimit)) report(x, y);
            int xValue = span - ask(far.index, pivot.index, x);
            save(x, xValue);
            if (xValue != bottom) {
                save(y, bottom);
            } else if (nearSpread > -bottom) {
                save(y, bottom + nearSpread);
            }
        } else {
            compareEndpoints(near, far, pivot, x, y, spread, true, tripleSpread);
            return {spread, -1};
        }
        return {spread, 0};
    }

    [[noreturn]] void scan() {
        int repeatedSpreads = 0, lastSpread = -1, lastAnchor = 0;
        int probeCredit = equalPair[0] ? 2 : 0;
        int probeDelay = 12, probeAllowedAt = 0, probeFreshAt = 0;
        int endpointPairs = 0, centeredPairs = 0;
        double centerSaving = 0.3;
        vector<pair<int, int>> sampled;
        int sampleLimit = min(16, remaining() / 40);
        while (true) {
            finishIfReady();
            Point pivot = bestPivot();
            if (remaining() == 1) finishOne(pivot.index);
            int tripleSpread = -1;
            bool canProbe = repeatedSpreads >= (lastSpread == 0 ? 2 : 5) &&
                            next >= probeAllowedAt;
            bool frequentTies = triedTriples >= 3 && 2 * equalTriples > triedTriples;
            if (remaining() >= 3 && next >= probeFreshAt &&
                (frequentTies || probeCredit > 0 || canProbe)) {
                if (probeCredit <= 0) probeCredit = 3;
                int x = order[next], y = order[next + 1], z = order[next + 2];
                tripleSpread = ask(x, y, z);
                ++triedTriples;
                if (tripleSpread == valueLimit) finishFullTriple(x, y, z);
                if (tripleSpread == 0) {
                    ++equalTriples;
                    next += 3;
                    probeCredit = min(probeCredit + 1, 3);
                    probeDelay = max(12, probeDelay / 2);
                    continue;
                }
                probeCredit -= 2;
                probeFreshAt = next + 3;
                if (probeCredit <= 0) {
                    probeAllowedAt = next + probeDelay;
                    probeDelay = min(160, probeDelay * 2);
                }
                repeatedSpreads = 0;
            }
            Point near = left, far = right;
            if (pivot.value - left.value > right.value - pivot.value) swap(near, far);
            int radius = abs(pivot.value - near.value);
            bool centered = radius > 0 && endpointPairs == 0;
            if (!centered && occurrences.at(far.value) > occurrences.at(near.value)) {
                swap(near, far);
            }
            int anchor = centered ? pivot.index : near.index;
            int x = take();
            int y = take();
            auto [spread, saving] = processPair(near, far, pivot, x, y, centered,
                                                tripleSpread, sampled, sampleLimit);
            if (anchor == lastAnchor && spread == lastSpread) {
                ++repeatedSpreads;
            } else {
                repeatedSpreads = 1;
            }
            lastAnchor = anchor;
            lastSpread = spread;
            if (centered) {
                centerSaving += 0.08 * (saving - centerSaving);
                ++centeredPairs;
                double repeatRate = double(max(occurrences.at(left.value),
                                               occurrences.at(right.value))) / max(next, 12);
                if (centeredPairs >= 10 && centerSaving + 0.07 < repeatRate * repeatRate) {
                    endpointPairs = 16;
                    centeredPairs = 0;
                    centerSaving = 0.2;
                }
            } else if (endpointPairs) {
                --endpointPairs;
            }
        }
    }

    int tailWitness(const array<int, 4>& group) const {
        int best = 0, bestScore = -1;
        for (int i = 1; i <= n; ++i) {
            if (!excluded[i]) continue;
            int score = answers.contains(key({group[0], group[1], i})) +
                        answers.contains(key({group[0], group[2], i})) +
                        answers.contains(key({group[1], group[2], i}));
            if (score > bestScore) {
                best = i;
                bestScore = score;
            }
        }
        return best;
    }

    [[noreturn]] void startWithEndpoints(const array<int, 4>& group,
                                        const array<int, 4>& ranges, int span,
                                        int l, int r) {
        points.clear();
        occurrences.clear();
        observe(group[l], 0);
        observe(group[r], span);
        finishIfReady();
        int a = -1, b = -1;
        for (int i = 0; i < 4; ++i) {
            if (i == l || i == r) continue;
            if (a == -1) a = group[i];
            else b = group[i];
        }
        int low = span - ranges[l], high = ranges[r];
        if (low != high && remaining() <= 5) {
            while (remaining() > 2) {
                int x = take();
                int range = ask(left.index, right.index, x);
                if (range > right.value - left.value) {
                    int position = ask(left.index, a, x) == range
                                       ? left.value + range : right.value - range;
                    observe(x, position);
                }
                finishIfReady();
            }
            if (remaining() == 1) finishOne(a);
            finishTwo();
        }
        if (remaining() == 1) finishOne(a);
        locateSeedPivots(a, b, low, high);
        scan();
    }

public:
    Solver(int size, mt19937& random) : n(size), order(size), excluded(size + 1) {
        iota(order.begin(), order.end(), 1);
        shuffle(order.begin(), order.end(), random);
        answers.reserve(size * 2);
    }

    [[noreturn]] void solve() {
        vector<int> candidates;
        bool lastWasEqual = false;
        while (true) {
            if (candidates.size() + remaining() <= 2) {
                while (remaining()) candidates.push_back(take());
                report(candidates);
            }
            if (equalPair[0] && !candidates.empty() && !lastWasEqual && remaining() >= 3) {
                if (startWithEqualPair(candidates)) scan();
                candidates.clear();
            }
            array<int, 4> group;
            int count = 0;
            int fresh = min(remaining(), 4 - int(candidates.size()));
            for (int i = 0; i < fresh; ++i) group[count++] = take();
            for (int i : candidates) group[count++] = i;
            if (count == 3) group[3] = tailWitness(group);

            array<int, 4> ranges;
            ranges.fill(-1);
            auto omissions = array{3, 2, 1, 0};
            // Reuse any known ranges first, especially for the final group.
            stable_partition(omissions.begin(), omissions.end(), [&](int omit) {
                auto t = without(group, omit);
                return answers.contains(key(t));
            });
            lastWasEqual = false;
            int span = 0;
            for (int q = 0; q < 4; ++q) {
                int omit = omissions[q];
                auto t = without(group, omit);
                int range = ask(t[0], t[1], t[2]);
                if (q == 0 && fresh >= 3 && omit >= 3) {
                    ++triedTriples;
                    equalTriples += range == 0;
                }
                ranges[omit] = range;
                span = max(span, range);
                if (range == 0) {
                    excludeEqual(t);
                    candidates.clear();
                    if (!excluded[group[omit]]) candidates.push_back(group[omit]);
                    lastWasEqual = true;
                    if (span == valueLimit) report(candidates);
                    break;
                }
                if (span == valueLimit && q < 2) {
                    vector<int> possible;
                    for (int i = 0; i < 4; ++i) {
                        if (ranges[i] != valueLimit && !excluded[group[i]]) {
                            possible.push_back(group[i]);
                        }
                    }
                    if (possible.size() <= 2) report(possible);
                }
                if (q < 2) continue;

                // Three different triples cover every pair in the group.
                // Omitting a unique extreme strictly decreases its range.
                vector<int> low;
                candidates.clear();
                for (int i = 0; i < 4; ++i) {
                    if (ranges[i] == span) {
                        excluded[group[i]] = true;
                    } else {
                        if (ranges[i] >= 0) low.push_back(i);
                        if (!excluded[group[i]]) candidates.push_back(group[i]);
                    }
                }
                if (low.size() == 2) {
                    for (int i = 0; i < 4; ++i) {
                        if (ranges[i] < 0) excluded[group[i]] = true;
                    }
                    erase_if(candidates, [&](int i) { return excluded[i]; });
                }
                if (span == valueLimit) report(candidates);
                if (low.size() == 2 && remaining()) {
                    startWithEndpoints(group, ranges, span, low[0], low[1]);
                }
                if (q == 3 || candidates.size() + remaining() <= 2 ||
                    (low.empty() && remaining() < 6) || candidates.empty()) break;
            }
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 4) return 0;
    mt19937 random(chrono::steady_clock::now().time_since_epoch().count());
    Solver(n, random).solve();
}

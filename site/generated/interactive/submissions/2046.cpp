#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

using namespace std;

constexpr int LIMIT = 1'000'000'000;
constexpr int SEED_SIZE = 5;
constexpr int SIGNED_SIZE = 2 * SEED_SIZE;
using Score = long long;
using Scores = array<int, SEED_SIZE>;
using Triple = array<int, 3>;

constexpr array<Triple, 10> TRIPLES{{
    {0, 1, 2}, {0, 1, 3}, {0, 1, 4}, {0, 2, 3}, {0, 2, 4},
    {0, 3, 4}, {1, 2, 3}, {1, 2, 4}, {1, 3, 4}, {2, 3, 4}
}};

// Together with 012 and 034, these are the seven lines of the Fano plane.
constexpr array<Triple, 5> EQUAL_SCORE_CHECK{{
    {1, 3, 5}, {2, 4, 5}, {1, 4, 6}, {2, 3, 6}, {0, 5, 6}
}};

struct Range {
    Score lo, hi;
};

Range invertReply(Score a, Score b, Score reply) {
    const auto [lo, hi] = minmax(a, b);
    if (reply < Score(lo) + hi) return {reply - hi, reply - hi};
    if (reply > Score(lo) + hi) return {reply - lo, reply - lo};
    return {lo, hi};
}

Score floorHalf(Score value) {
    return (value < 0 ? value - 1 : value) / 2;
}

// Node 2*i is x[i] and node 2*i+1 is -x[i].
struct Bounds {
    array<array<Score, SIGNED_SIZE>, SIGNED_SIZE> d;

    Bounds() {
        for (int i = 0; i < SIGNED_SIZE; ++i) {
            d[i].fill(2LL * LIMIT);
            d[i][i] = 0;
            if (i & 1) d[i][i ^ 1] = 0;
        }
    }

    void constrain(int i, int j, Score bound) {
        d[i][j] = min(d[i][j], bound);
        d[j ^ 1][i ^ 1] = d[i][j];
    }

    void sumEquals(int i, int j, Score sum) {
        constrain(2 * i, (2 * j) ^ 1, sum);
        constrain((2 * i) ^ 1, 2 * j, -sum);
    }

    bool tighten() {
        array<Score, SIGNED_SIZE> unary;
        for (int i = 0; i < SIGNED_SIZE; ++i) {
            unary[i] = floorHalf(d[i][i ^ 1]);
        }
        for (int i = 0; i < SIGNED_SIZE; ++i) {
            if (unary[i] + unary[i ^ 1] < 0) return false;
        }
        for (int i = 0; i < SIGNED_SIZE; ++i) {
            for (int j = 0; j < SIGNED_SIZE; ++j) {
                d[i][j] = min(d[i][j], unary[i] + unary[j ^ 1]);
            }
        }
        return true;
    }

    bool close() {
        for (int k = 0; k < SIGNED_SIZE; ++k) {
            for (int i = 0; i < SIGNED_SIZE; ++i) {
                for (int j = 0; j < SIGNED_SIZE; ++j) {
                    d[i][j] = min(d[i][j], d[i][k] + d[k][j]);
                }
                if (d[i][i] < 0) return false;
            }
        }
        // Integer octagon closure rounds each bound on 2*x down to an even value.
        return tighten();
    }

    Range value(int i) const {
        return {-d[2 * i + 1][2 * i] / 2, d[2 * i][2 * i + 1] / 2};
    }

    Range sum(int i, int j) const {
        return {-d[2 * i + 1][2 * j], d[2 * i][2 * j + 1]};
    }

    void addUnary(int i, Score bound) {
        const int j = i ^ 1;
        if (bound >= d[i][j]) return;
        array<Score, SIGNED_SIZE> incoming, outgoing;
        for (int k = 0; k < SIGNED_SIZE; ++k) {
            incoming[k] = d[k][i] + bound;
            outgoing[k] = d[j][k];
        }
        for (int a = 0; a < SIGNED_SIZE; ++a) {
            for (int b = 0; b < SIGNED_SIZE; ++b) {
                d[a][b] = min(d[a][b], incoming[a] + outgoing[b]);
            }
        }
    }

    void fixValue(int i, Score x) {
        addUnary(2 * i, 2 * x);
        addUnary(2 * i + 1, -2 * x);
        tighten();
    }
};

struct Ordering {
    Scores indices;
    array<pair<int, int>, TRIPLES.size()> ends;
};

vector<Ordering> makeOrderings() {
    vector<Ordering> result;
    Scores indices{0, 1, 2, 3, 4};
    do {
        Scores rank;
        for (int i = 0; i < SEED_SIZE; ++i) rank[indices[i]] = i;
        Ordering ordering;
        ordering.indices = indices;
        for (int q = 0; q < int(TRIPLES.size()); ++q) {
            auto [lo, hi] = minmax_element(
                TRIPLES[q].begin(), TRIPLES[q].end(),
                [&](int a, int b) { return rank[a] < rank[b]; }
            );
            ordering.ends[q] = {*lo, *hi};
        }
        result.push_back(ordering);
    } while (next_permutation(indices.begin(), indices.end()));
    return result;
}

struct Hypothesis {
    Bounds bounds;
    const Ordering* ordering;
    array<Range, TRIPLES.size()> replies;
    array<bool, TRIPLES.size()> oneParity;
    bool fixed;
};

vector<Hypothesis> inferSeed(const array<Score, TRIPLES.size()>& replies,
                             const array<Range, SEED_SIZE>& ranges) {
    static const vector<Ordering> orderings = makeOrderings();
    vector<Hypothesis> hypotheses;
    for (const auto& ordering : orderings) {
        Hypothesis h;
        h.ordering = &ordering;
        for (int i = 0; i < SEED_SIZE; ++i) {
            h.bounds.constrain(2 * i, 2 * i + 1, 2 * ranges[i].hi);
            h.bounds.constrain(2 * i + 1, 2 * i, -2 * ranges[i].lo);
        }
        for (int i = 1; i < SEED_SIZE; ++i) {
            const int a = ordering.indices[i - 1];
            const int b = ordering.indices[i];
            // Break ties by the seed index so each score vector has one ordering.
            h.bounds.constrain(2 * a, 2 * b, -(a > b));
        }
        for (int q = 0; q < int(TRIPLES.size()); ++q) {
            if (replies[q] < 0) continue;
            auto [a, b] = ordering.ends[q];
            h.bounds.sumEquals(a, b, replies[q]);
        }
        if (!h.bounds.close()) continue;
        h.fixed = true;
        for (int i = 0; i < SEED_SIZE; ++i) {
            auto [lo, hi] = h.bounds.value(i);
            h.fixed &= lo == hi;
        }
        for (int q = 0; q < int(TRIPLES.size()); ++q) {
            auto [a, b] = ordering.ends[q];
            h.replies[q] = h.bounds.sum(a, b);
            h.oneParity[q] = h.bounds.d[2 * a][2 * b] + h.bounds.d[2 * b][2 * a] == 0;
        }
        hypotheses.push_back(h);
    }
    return hypotheses;
}

pair<Scores, double> sampleScores(const Bounds& bounds, mt19937_64& rng) {
    Scores scores;
    Bounds remaining = bounds;
    double weight = 1;
    for (int i = 0; i < SEED_SIZE; ++i) {
        auto [lo, hi] = remaining.value(i);
        if (lo == hi) {
            scores[i] = int(lo);
            continue;
        }
        scores[i] = uniform_int_distribution<int>(int(lo), int(hi))(rng);
        weight *= double(hi - lo + 1);
        remaining.fixValue(i, scores[i]);
    }
    return {scores, weight};
}

int chooseSeedQuery(const vector<Hypothesis>& hypotheses,
                    const array<Score, TRIPLES.size()>& replies, mt19937_64& rng) {
    // All ten replies distinguish the five scores. For sorted scores A..E,
    // the minimum, median and maximum replies are A+C, A+E and C+E.
    // The maximum reply containing any index below C is its score plus E.
    // The minimum reply containing any index above C is its score plus A.
    vector<int> available;
    for (int q = 0; q < int(TRIPLES.size()); ++q) {
        if (replies[q] >= 0) continue;
        const Score value = hypotheses.front().replies[q].lo;
        for (const auto& h : hypotheses) {
            if (h.replies[q].lo != value || h.replies[q].hi != value) {
                available.push_back(q);
                break;
            }
        }
    }
    if (available.size() == 1) return available.front();

    array<double, TRIPLES.size()> merit{};
    constexpr int SAMPLES = 6;
    for (const auto& h : hypotheses) {
        const int samples = h.fixed ? 1 : SAMPLES;
        for (int s = 0; s < samples; ++s) {
            auto [scores, weight] = sampleScores(h.bounds, rng);
            // Temper the volume estimate to give small feasible regions a voice.
            weight = sqrt(weight) / samples;
            for (int q : available) {
                auto [a, b] = h.ordering->ends[q];
                const Score answer = Score(scores[a]) + scores[b];
                int compatible = 0;
                for (const auto& alternative : hypotheses) {
                    auto [lo, hi] = alternative.replies[q];
                    if (answer < lo || answer > hi) continue;
                    if (alternative.oneParity[q] && (answer - lo) % 2 != 0) continue;
                    ++compatible;
                }
                const bool addsEquation = h.replies[q].lo != h.replies[q].hi;
                merit[q] += weight * (addsEquation - 0.60 * log2(compatible));
            }
        }
    }
    return *max_element(available.begin(), available.end(),
                        [&](int a, int b) { return merit[a] < merit[b]; });
}

struct KnownScores {
    vector<pair<int, int>> sorted;
    pair<int, int> closest{-1, -1};
    int gap = LIMIT + 1;
    map<int, int> frequency;

    void add(int index, int value) {
        auto it = lower_bound(sorted.begin(), sorted.end(), pair{value, index});
        auto compare = [&](pair<int, int> other) {
            const int difference = abs(value - other.first);
            if (difference < gap) {
                gap = difference;
                closest = {index, other.second};
            }
        };
        if (it != sorted.begin()) compare(*prev(it));
        if (it != sorted.end()) compare(*it);
        sorted.insert(it, {value, index});
        ++frequency[value];
    }

    int batchSize(int remaining) const {
        if (gap != 0 || remaining < 2) return 1;
        const auto zero = frequency.find(0);
        const auto top = frequency.find(LIMIT);
        const int countZero = zero == frequency.end() ? 0 : zero->second;
        const int countTop = top == frequency.end() ? 0 : top->second;
        if (countZero == 0 && countTop == 0) return 1;

        // One virtual observation tempers estimates based on the initial seed.
        const double count = sorted.size() + 1.;
        const double pZero = countZero / count, pTop = countTop / count;
        const double boundary = max(pZero, pTop);
        double samePair = 1. / (count * count);
        double sameTriple = samePair / count;
        for (auto [value, timesSeen] : frequency) {
            const double p = timesSeen / count;
            samePair += p * p;
            sameTriple += p * p * p;
        }
        const double gainTwo = 2 * boundary * boundary - (1 + samePair) / 2;
        const double gainThree = 2 * (pZero * pZero * pZero + pTop * pTop * pTop)
                               - (1 + 3 * samePair - 4 * sameTriple) / 3;
        const double margin = 0.015 + 0.18 / sqrt(count);
        if (remaining >= 3 && gainThree / 3 > margin && gainThree / 3 >= gainTwo / 2) return 3;
        if (gainTwo / 2 > margin) return 2;
        return 1;
    }

    pair<int, int> batchAnchor() const {
        const auto zero = frequency.find(0);
        const auto top = frequency.find(LIMIT);
        if (top == frequency.end() || (zero != frequency.end() && zero->second >= top->second)) {
            return sorted.front();
        }
        return sorted.back();
    }
};

struct Observation {
    Triple indices;
    Score reply;
};

template <class Ask>
vector<int> recover(int n, Ask ask, mt19937_64& rng) {
    vector<int> indices(n), scores(n, -1);
    iota(indices.begin(), indices.end(), 0);
    shuffle(indices.begin(), indices.end(), rng);

    KnownScores known;
    vector<Range> ranges(n, {0, LIMIT});
    vector<Observation> observations;
    auto record = [&](int index, int score) {
        scores[index] = score;
        ranges[index] = {score, score};
        known.add(index, score);
    };
    auto seedAsk = [&](int a, int b, int c) {
        Triple actual{indices[a], indices[b], indices[c]};
        sort(actual.begin(), actual.end());
        const Score reply = ask(actual[0], actual[1], actual[2]);
        observations.push_back({actual, reply});
        return reply;
    };
    auto cachedAsk = [&](int a, int b, int c) -> Score {
        Triple actual{a, b, c};
        sort(actual.begin(), actual.end());
        for (const auto& observation : observations) {
            if (observation.indices == actual) return observation.reply;
        }
        return ask(a, b, c);
    };

    auto propagate = [&] {
        // The reply is monotone in each score. Substitute lower and upper
        // bounds for the other two scores to bound a still unknown score.
        for (int pass = 0; pass < 7; ++pass) {
            bool changed = false;
            for (const auto& [which, reply] : observations) {
                for (int j = 0; j < 3; ++j) {
                    const int id = which[j];
                    if (scores[id] >= 0) continue;
                    const auto [loA, hiA] = ranges[which[(j + 1) % 3]];
                    const auto [loB, hiB] = ranges[which[(j + 2) % 3]];
                    const Score lo = max(ranges[id].lo, invertReply(hiA, hiB, reply).lo);
                    const Score hi = min(ranges[id].hi, invertReply(loA, loB, reply).hi);
                    changed |= lo != ranges[id].lo || hi != ranges[id].hi;
                    ranges[id] = {lo, hi};
                    if (lo == hi) record(id, int(lo));
                }
            }
            if (!changed) break;
        }
    };

    array<Score, TRIPLES.size()> seedReplies;
    seedReplies.fill(-1);
    seedReplies[0] = seedAsk(0, 1, 2);
    const Score initial = seedReplies[0];
    if (n >= 7 && initial != 0 && initial != 2LL * LIMIT) {
        seedReplies[5] = seedAsk(0, 3, 4);
        if (seedReplies[5] == initial && initial % 2 == 0) {
            bool allEqual = true;
            for (const auto& [a, b, c] : EQUAL_SCORE_CHECK) {
                if (seedAsk(a, b, c) != initial) {
                    allEqual = false;
                    break;
                }
            }
            if (allEqual) {
                // If scores deviate from initial/2, take the largest deviation
                // in absolute value. Each line through such a vertex needs an
                // opposite vertex, so both signs occur at least three times.
                // Every line is mixed even after giving any leftover vertex
                // a sign. The lines then count 14 opposite pairs, but a
                // two-coloring of seven vertices has at most 12.
                for (int i = 0; i < 7; ++i) record(indices[i], int(initial / 2));
            }
        }
    }
    propagate();

    while (known.gap != 0 && known.sorted.size() < SEED_SIZE) {
        array<Range, SEED_SIZE> seedRanges;
        for (int i = 0; i < SEED_SIZE; ++i) seedRanges[i] = ranges[indices[i]];
        auto hypotheses = inferSeed(seedReplies, seedRanges);
        const size_t previouslyKnown = known.sorted.size();
        for (int i = 0; i < SEED_SIZE; ++i) {
            const int index = indices[i];
            if (scores[index] >= 0) continue;
            Range possible{LIMIT, 0};
            for (const auto& h : hypotheses) {
                const auto [lo, hi] = h.bounds.value(i);
                possible.lo = min(possible.lo, lo);
                possible.hi = max(possible.hi, hi);
            }
            ranges[index] = possible;
            if (possible.lo == possible.hi) record(index, int(possible.lo));
        }
        if (known.sorted.size() != previouslyKnown) propagate();
        if (known.gap == 0 || known.sorted.size() >= SEED_SIZE) break;
        const int q = chooseSeedQuery(hypotheses, seedReplies, rng);
        const auto [a, b, c] = TRIPLES[q];
        seedReplies[q] = seedAsk(a, b, c);
        propagate();
    }

    auto individual = [&](int index) {
        const auto& sorted = known.sorted;
        const int last = int(sorted.size()) - 1;
        auto [a, b] = known.closest;
        if (known.gap != 0) {
            // A bound from the seed may already put the score outside an end pair.
            if (ranges[index].lo >= sorted[1].first) {
                a = sorted[0].second;
                b = sorted[1].second;
            } else if (ranges[index].hi <= sorted[last - 1].first) {
                a = sorted[last - 1].second;
                b = sorted[last].second;
            }
        }
        const int lo = min(scores[a], scores[b]), hi = max(scores[a], scores[b]);
        Range possible = invertReply(lo, hi, cachedAsk(a, b, index));
        possible.lo = max(possible.lo, ranges[index].lo);
        possible.hi = min(possible.hi, ranges[index].hi);
        if (possible.lo != possible.hi) {
            // With five distinct known values, one of these end pairs is disjoint.
            const int first = sorted[1].first < lo ? 0 : last - 1;
            const auto [x, i] = sorted[first];
            const auto [y, j] = sorted[first + 1];
            possible = invertReply(x, y, cachedAsk(i, j, index));
        }
        record(index, int(possible.lo));
    };

    vector<int> pending;
    for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
        if (scores[*it] < 0) pending.push_back(*it);
    }
    vector<bool> appearedInSeed(n);
    for (const auto& observation : observations) {
        for (int index : observation.indices) appearedInSeed[index] = true;
    }
    auto takePending = [&] {
        const int index = pending.back();
        pending.pop_back();
        return index;
    };
    auto finishBatch = [&](int index, int a, int b, Score reply) {
        auto [lo, hi] = invertReply(a, b, reply);
        lo = max(lo, ranges[index].lo);
        hi = min(hi, ranges[index].hi);
        if (lo == hi) record(index, int(lo));
        else individual(index);
    };
    while (!pending.empty()) {
        const int batch = known.batchSize(int(pending.size()));
        const int a = takePending();
        bool updateSeed = appearedInSeed[a];
        if (batch == 1) {
            individual(a);
        } else if (batch == 2) {
            const int b = takePending();
            updateSeed |= appearedInSeed[b];
            const auto [boundary, anchor] = known.batchAnchor();
            const Score reply = cachedAsk(anchor, a, b);
            if (reply == 2LL * boundary) {
                record(a, boundary);
                record(b, boundary);
            } else {
                individual(a);
                finishBatch(b, boundary, scores[a], reply);
            }
        } else {
            const int b = takePending(), c = takePending();
            updateSeed |= appearedInSeed[b] || appearedInSeed[c];
            const Score reply = cachedAsk(a, b, c);
            if (reply == 0 || reply == 2LL * LIMIT) {
                for (int index : {a, b, c}) record(index, int(reply / 2));
            } else {
                individual(a);
                individual(b);
                finishBatch(c, scores[a], scores[b], reply);
            }
        }
        if (updateSeed) {
            propagate();
            erase_if(pending, [&](int index) { return scores[index] >= 0; });
        }
    }
    return scores;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < SEED_SIZE || n > 1000) return 0;
    mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
    auto ask = [](int a, int b, int c) {
        cout << "? " << a + 1 << ' ' << b + 1 << ' ' << c + 1 << '\n' << flush;
        Score reply;
        if (!(cin >> reply) || reply < 0 || reply > 2LL * LIMIT) exit(0);
        return reply;
    };
    vector<int> answer = recover(n, ask, rng);
    cout << '!';
    for (int value : answer) cout << ' ' << value;
    cout << '\n' << flush;
    return 0;
}

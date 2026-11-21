#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

using namespace std;

constexpr int maxN = 2000;
constexpr int maxWords = (maxN + 63) / 64;
constexpr int weightUnit = 256;
constexpr int smallLimit = 8;

array<array<unsigned char, maxN + 1>, maxN + 1> replies{};
array<bool, maxN + 1> hasReply{};

uint64_t lowBits(int count) {
    return count == 64 ? ~uint64_t{0} : (uint64_t{1} << count) - 1;
}

int ask(int left, int right) {
    auto& cached = replies[right][left];
    if (cached) {
        return cached - 1;
    }
    cout << "? " << left << ' ' << right << '\n' << flush;
    int reply;
    if (!(cin >> reply) || (reply != 0 && reply != 1)) {
        exit(0);
    }
    cached = reply + 1;
    hasReply[right] = true;
    return reply;
}

struct Prediction {
    int rank;
    int extraWeight;
};

class RankModel {
    static constexpr int expertCount = 34;
    static constexpr float threshold = 0.46f;

    // Each expert tracks its overall accuracy and its accuracy by position modulo 2 and 3.
    array<array<float, 6>, expertCount> accuracy{};
    array<int, expertCount> guess{};
    array<int, maxN> insertionHistory{};
    bool increasing = true;
    bool decreasing = true;

public:
    void prepare(const array<int, maxN>& ranks, int size) {
        int expert = 0;
        guess[expert++] = 0;
        guess[expert++] = size;
        guess[expert++] = 1;
        guess[expert++] = size - 1;
        guess[expert++] = size / 2;
        guess[expert++] = (size + 1) / 2;
        for (int lag = 1; lag <= 8; ++lag) {
            guess[expert++] = size >= lag ? ranks[size - lag] : -1;
            guess[expert++] = size >= lag ? ranks[size - lag] + 1 : -1;
        }
        for (int lag = 1; lag <= 4; ++lag) {
            int previous = size >= lag ? insertionHistory[size - lag] : -2;
            guess[expert++] = previous;
            guess[expert++] = previous + 1;
            guess[expert++] = size >= 2 * lag
                ? clamp(2 * previous - insertionHistory[size - 2 * lag], 0, size)
                : -1;
        }
    }

    vector<Prediction> predict(int size) const {
        vector<pair<int, float>> confidence;
        auto add = [&](int rank, float score) {
            if (rank < 0 || score <= threshold) {
                return;
            }
            for (auto& [otherRank, otherScore] : confidence) {
                if (otherRank == rank) {
                    otherScore = max(otherScore, score);
                    return;
                }
            }
            confidence.emplace_back(rank, score);
        };

        for (int e = 0; e < expertCount; ++e) {
            add(guess[e], max({accuracy[e][0], accuracy[e][1 + size % 2],
                               accuracy[e][3 + size % 3]}));
        }
        if (size >= 2 && increasing) {
            add(size, 0.90f);
        }
        if (size >= 2 && decreasing) {
            add(0, 0.90f);
        }

        float best = 0;
        float sum = 0;
        for (const auto& [rank, score] : confidence) {
            best = max(best, score);
            float strength = score - threshold;
            sum += strength * strength;
        }
        vector<Prediction> predictions;
        if (sum > 0) {
            best = min(best, 0.98f);
            float scale = (size + 1) * weightUnit * best / ((1 - best) * sum);
            for (const auto& [rank, score] : confidence) {
                float strength = score - threshold;
                int extraWeight = static_cast<int>(scale * strength * strength);
                if (extraWeight > 0) {
                    predictions.push_back({rank, extraWeight});
                }
            }
        }
        return predictions;
    }

    void observe(int size, int inserted, int lastRank) {
        for (int e = 0; e < expertCount; ++e) {
            float hit = guess[e] == inserted ? 1.0f : 0.0f;
            auto& a = accuracy[e];
            a[0] += 0.20f * (hit - a[0]);
            a[1 + size % 2] += 0.25f * (hit - a[1 + size % 2]);
            a[3 + size % 3] += 0.25f * (hit - a[3 + size % 3]);
        }
        insertionHistory[size] = inserted;
        increasing &= inserted > lastRank;
        decreasing &= inserted <= lastRank;
    }
};

class InsertionSearch {
    // masks[k] gives the parity of the new inversions against the last k old elements.
    array<array<uint64_t, maxWords>, maxN> masks;
    array<uint64_t, maxWords> candidates;
    uint32_t randomState = 3172199853u;

    bool bit(int k, int rank) const {
        return (masks[k][rank / 64] >> (rank % 64)) & 1;
    }

    bool contains(int rank) const {
        return (candidates[rank / 64] >> (rank % 64)) & 1;
    }

    int sample(int size) {
        randomState ^= randomState << 13;
        randomState ^= randomState >> 17;
        randomState ^= randomState << 5;
        return 1 + static_cast<int>(randomState % size);
    }

    void buildMasks(const array<int, maxN>& ranks, int size, int words) {
        fill_n(masks[0].begin(), words, uint64_t{0});
        for (int k = 1; k <= size; ++k) {
            int cut = ranks[size - k] + 1;
            int word = cut / 64;
            int bit = cut % 64;
            const auto& previous = masks[k - 1];
            auto& current = masks[k];
            for (int w = 0; w < word; ++w) {
                current[w] = ~previous[w];
            }
            current[word] = previous[word] ^ lowBits(bit);
            copy(previous.begin() + word + 1, previous.begin() + words, current.begin() + word + 1);
        }
    }

    int chooseQuery(int size, int count, const vector<Prediction>& predictions, int words) {
        array<int, maxWords> activeWords;
        int usedWords = 0;
        for (int w = 0; w < words; ++w) {
            if (candidates[w] != 0) {
                activeWords[usedWords++] = w;
            }
        }
        int totalWeight = count * weightUnit;
        int heaviest = weightUnit;
        for (auto prediction : predictions) {
            totalWeight += prediction.extraWeight;
            heaviest = max(heaviest, weightUnit + prediction.extraWeight);
        }
        int minimumImbalance = max(0, 2 * heaviest - totalWeight);
        int bestQuery = 0;
        int bestImbalance = totalWeight;
        int bestCountImbalance = count;
        int idealGroupLimit = bit_ceil(static_cast<unsigned>(count)) / 2;
        bool weighted = !predictions.empty();

        auto evaluate = [&](int k) {
            const auto& mask = masks[k];
            int ones = 0;
            for (int i = 0; i < usedWords; ++i) {
                int w = activeWords[i];
                ones += popcount(mask[w] & candidates[w]);
            }
            if (ones == 0 || ones == count) {
                return false;
            }
            int oneWeight = ones * weightUnit;
            for (auto prediction : predictions) {
                if ((mask[prediction.rank / 64] >> (prediction.rank % 64)) & 1) {
                    oneWeight += prediction.extraWeight;
                }
            }
            int imbalance = abs(totalWeight - 2 * oneWeight);
            int countImbalance = abs(count - 2 * ones);
            if (pair(imbalance, countImbalance) < pair(bestImbalance, bestCountImbalance)) {
                bestQuery = k;
                bestImbalance = imbalance;
                bestCountImbalance = countImbalance;
            }
            if (!weighted) {
                return max(ones, count - ones) <= idealGroupLimit
                    && countImbalance <= max(1, count / 16);
            }
            return imbalance == minimumImbalance;
        };

        for (int k = 1; k <= min(8, size); ++k) {
            if (evaluate(k)) {
                return k;
            }
        }
        for (int k = size; k > max(8, size - 5); --k) {
            if (evaluate(k)) {
                return k;
            }
        }
        for (int trial = 0; trial < 40 && size > 32; ++trial) {
            int k = sample(size);
            if (evaluate(k)) {
                return k;
            }
        }

        if (!weighted && bestQuery && (count + bestCountImbalance) / 2 <= idealGroupLimit) {
            return bestQuery;
        }
        if (weighted && bestQuery &&
            bestImbalance <= minimumImbalance + 2 * weightUnit * (count / 128)) {
            return bestQuery;
        }
        for (int k = 9; k <= size - 5; ++k) {
            if (evaluate(k)) {
                return k;
            }
        }
        // Between any two surviving ranks, some old element flips exactly one of them.
        // At least one of the masks on either side of that flip separates the two ranks.
        return bestQuery;
    }

    pair<int, int> confirmation(const array<int, maxN>& ranks, int size, int count,
                                int words, int rank, int ordinary) const {
        auto matches = [&](int k) {
            int ones = 0;
            for (int w = 0; w < words; ++w) {
                ones += popcount(candidates[w] & masks[k][w]);
            }
            return bit(k, rank) ? ones : count - ones;
        };
        if (matches(ordinary) <= 2) {
            return {ordinary, 0};
        }

        int lower = rank - 1;
        while (lower >= 0 && !contains(lower)) {
            --lower;
        }
        int upper = rank + 1;
        while (upper <= size && !contains(upper)) {
            ++upper;
        }
        auto separates = [&](int old) {
            if (lower >= 0 && lower <= old && old < rank) {
                return -1;
            }
            if (upper <= size && rank <= old && old < upper) {
                return 1;
            }
            return 0;
        };

        pair<int, int> queries{ordinary, 0};
        int best = numeric_limits<int>::max();
        auto idealCost = [](int leaves) {
            if (leaves <= 1) {
                return 0;
            }
            int depth = bit_width(static_cast<unsigned>(leaves));
            return (depth + 1) * leaves - (1 << depth);
        };
        auto consider = [&](int first, int second, int equal) {
            int penalty = equal - 1 + idealCost(equal - 1) + idealCost(count - equal);
            if (penalty < best) {
                best = penalty;
                queries = {first, second};
            }
        };

        // XOR two masks whose added elements separate the predicted rank from both sides.
        // At an extreme there is only one side to separate.
        bool extreme = lower < 0 || upper > size;
        int previous = 0;
        for (int k = 1; k <= size; ++k) {
            int current = separates(ranks[size - k]);
            int first = extreme ? k - 1 : k - 2;
            bool isolates = extreme ? current != 0 : current != 0 && current == -previous;
            previous = current;
            if (isolates) {
                int firstMatches = matches(first);
                int secondMatches = matches(k);
                if (firstMatches == count) {
                    return {k, 0};
                }
                if (secondMatches == count) {
                    return {first, 0};
                }
                consider(first, k, firstMatches);
                consider(k, first, secondMatches);
            }
        }
        return queries;
    }

    int finishSmall(int size, int count, int words, const array<int, maxN + 1>& parity,
                    const vector<Prediction>& predictions) const {
        array<int, smallLimit> ranks;
        int index = 0;
        for (int w = 0; w < words; ++w) {
            for (uint64_t word = candidates[w]; word; word &= word - 1) {
                ranks[index++] = 64 * w + countr_zero(word);
            }
        }

        int full = (1 << count) - 1;
        array<int, 1 << smallLimit> queryFor{};
        vector<int> tests;
        tests.reserve(full / 2);
        for (int k = 1; k <= size; ++k) {
            int part = 0;
            for (int i = 0; i < count; ++i) {
                int rank = ranks[i];
                part |= ((masks[k][rank / 64] >> (rank % 64)) & 1) << i;
            }
            // Complementary masks induce the same partition.
            if (part & 1) {
                part ^= full;
            }
            if (part && queryFor[part] == 0) {
                queryFor[part] = k;
                tests.push_back(part);
                if (static_cast<int>(tests.size()) == full / 2) {
                    break;
                }
            }
        }

        array<int, smallLimit> rankWeights;
        fill_n(rankWeights.begin(), count, weightUnit);
        for (auto prediction : predictions) {
            int i = lower_bound(ranks.begin(), ranks.begin() + count, prediction.rank) - ranks.begin();
            rankWeights[i] += prediction.extraWeight;
        }
        array<int, 1 << smallLimit> cost{};
        array<int, 1 << smallLimit> subsetWeight{};
        array<int, 1 << smallLimit> bestPart;
        constexpr array<int, smallLimit + 1> idealCost{0, 0, 2, 5, 8, 12, 16, 20, 24};
        // Every split costs one query for every rank in the current subset.
        for (int set = 1; set <= full; ++set) {
            int i = countr_zero(static_cast<unsigned>(set));
            subsetWeight[set] = subsetWeight[set & (set - 1)] + rankWeights[i];
            if (has_single_bit(static_cast<unsigned>(set))) {
                continue;
            }

            int best = numeric_limits<int>::max();
            int lowerBound = predictions.empty()
                ? idealCost[popcount(static_cast<unsigned>(set))] * weightUnit
                : 0;
            for (int test : tests) {
                int one = test & set;
                if (one == 0 || one == set) {
                    continue;
                }
                int splitCost = cost[one] + cost[set ^ one];
                if (splitCost < best) {
                    best = splitCost;
                    bestPart[set] = test;
                    if (subsetWeight[set] + best == lowerBound) {
                        break;
                    }
                }
            }
            cost[set] = subsetWeight[set] + best;
        }

        int set = full;
        while (!has_single_bit(static_cast<unsigned>(set))) {
            int k = queryFor[bestPart[set]];
            int reply = ask(size - k + 1, size + 1) ^ parity[k];
            int part = bestPart[set];
            if (((masks[k][ranks[0] / 64] >> (ranks[0] % 64)) & 1) != 0) {
                part ^= full;
            }
            set &= reply ? part : (full ^ part);
        }
        return ranks[countr_zero(static_cast<unsigned>(set))];
    }

public:
    int findRank(const array<int, maxN>& ranks, const array<int, maxN + 1>& parity,
                 int size, vector<Prediction> predictions) {
        int count = size + 1;
        int words = (count + 63) / 64;
        fill_n(candidates.begin(), words, ~uint64_t{0});
        candidates[words - 1] = lowBits((count - 1) % 64 + 1);
        buildMasks(ranks, size, words);

        auto retain = [&](int k, int reply) {
            reply ^= parity[k];
            count = 0;
            for (int w = 0; w < words; ++w) {
                candidates[w] &= reply ? masks[k][w] : ~masks[k][w];
                count += popcount(candidates[w]);
            }
            erase_if(predictions, [&](Prediction prediction) {
                return !contains(prediction.rank);
            });
        };
        if (hasReply[size + 1]) {
            for (int k = 1; k <= size && count > 1; ++k) {
                int cached = replies[size + 1][size - k + 1];
                if (cached) {
                    retain(k, cached - 1);
                }
            }
        }
        while (count > smallLimit) {
            // The full suffix splits all insertion ranks by parity.
            int k = predictions.empty() && count == size + 1
                ? size : chooseQuery(size, count, predictions, words);
            int second = 0;
            int predictedRank = -1;
            if (!predictions.empty()) {
                int weight = count * weightUnit;
                Prediction strongest = predictions.front();
                for (auto prediction : predictions) {
                    weight += prediction.extraWeight;
                    if (prediction.extraWeight > strongest.extraWeight) {
                        strongest = prediction;
                    }
                }
                if (4LL * (weightUnit + strongest.extraWeight) >= 3LL * weight) {
                    predictedRank = strongest.rank;
                    auto plan = confirmation(ranks, size, count, words, predictedRank, k);
                    k = plan.first;
                    second = plan.second;
                }
            }
            retain(k, ask(size - k + 1, size + 1));
            if (second && count > 1 && contains(predictedRank)) {
                retain(second, ask(size - second + 1, size + 1));
            }
        }
        if (count == 1) {
            for (int w = 0; w < words; ++w) {
                if (candidates[w] != 0) {
                    return 64 * w + countr_zero(candidates[w]);
                }
            }
        }
        return finishSmall(size, count, words, parity, predictions);
    }
};

bool resolveRun(const array<int, maxN>& ranks, const array<int, maxN + 1>& parity,
                int size, int length, bool ascending, vector<int>& oldBefore) {
    // In the chosen direction, the run extends upwards from the last old element.
    int base = ascending ? ranks[size - 1] + 1 : size - ranks[size - 1];
    if (base == size) {
        oldBefore.assign(length, size);
        return true;
    }
    if (length < 10) {
        return false;
    }

    array<int, maxN> position;
    for (int i = 0; i < size; ++i) {
        int oriented = ascending ? ranks[i] : size - 1 - ranks[i];
        position[oriented] = i + 1;
    }

    // Removing the left endpoint isolates the inversions that involve that old element.
    // After removing its old inversions, the result is min(prefixLength, crossing) mod 2.
    auto crossingParity = [&](int old, int prefixLength) {
        int left = position[old];
        int right = size + prefixLength;
        int oldParity = parity[size - left + 1] ^ parity[size - left];
        int orientedFlip = !ascending && (prefixLength & 1);
        int withOld = ask(left, right);
        int withoutOld = ask(left + 1, right);
        return withOld ^ withoutOld ^ oldParity ^ orientedFlip;
    };
    auto lastBeforeOld = [&](int old) {
        int full = crossingParity(old, length);
        return full != crossingParity(old, length - 1);
    };

    if (lastBeforeOld(base)) {
        oldBefore.assign(length, base);
        return true;
    }
    if (crossingParity(size - 1, 1) == 0) {
        oldBefore.assign(length, size);
        return true;
    }

    int cutoff = min(size, base + max(1, length / 12));
    if (cutoff < size && !lastBeforeOld(cutoff)) {
        return false;
    }

    oldBefore.resize(length);
    int first = 0;
    int old = base;
    for (; old < cutoff; ++old) {
        if (old != base && lastBeforeOld(old)) {
            break;
        }
        int parityAtEnd = crossingParity(old, length);
        int low = first + ((first & 1) != parityAtEnd);
        int high = length - 1 - (((length - 1) & 1) != parityAtEnd);
        while (low < high) {
            // At the opposite parity, this query distinguishes the two possible sides.
            int mid = low + 2 * ((high - low) / 4) + 1;
            if (crossingParity(old, mid) == (mid & 1)) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        fill(oldBefore.begin() + first, oldBefore.begin() + low, old);
        first = low;
    }
    fill(oldBefore.begin() + first, oldBefore.end(), old);
    return true;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1) {
        return 0;
    }

    // Ranks and suffix parities refer to the prefix processed so far.
    array<int, maxN> ranks{};
    array<int, maxN + 1> parity{};
    RankModel model;
    InsertionSearch search;
    int cleanStreak = 0;
    bool ascending = true;
    int nextProbe = 1;
    vector<int> oldBefore;

    auto insert = [&](int size, int rank) {
        int last = ranks[size - 1];
        bool up = rank > last;
        int gap = up ? rank - last - 1 : last - rank;
        cleanStreak = gap == 0 ? (ascending == up ? cleanStreak + 1 : 1) : 0;
        ascending = up;
        model.observe(size, rank, last);

        int addedParity = 0;
        int previous = parity[0];
        for (int k = 0; k < size; ++k) {
            int oldParity = parity[k + 1];
            parity[k + 1] = previous ^ addedParity;
            previous = oldParity;

            int& oldRank = ranks[size - 1 - k];
            int greater = oldRank >= rank;
            addedParity ^= greater;
            oldRank += greater;
        }
        parity[size + 1] = previous ^ addedParity;
        ranks[size] = rank;
    };

    int size = 1;
    while (size < n) {
        if (cleanStreak >= 6 && size >= nextProbe) {
            int length = 0;
            while (size + length < n && ask(size + length, size + length + 1) == !ascending) {
                ++length;
            }
            nextProbe = size + length + 1;
            if (length > 0 && resolveRun(ranks, parity, size, length, ascending, oldBefore)) {
                bool runAscending = ascending;
                for (int j = 0; j < length; ++j) {
                    int rank = runAscending ? oldBefore[j] + j : size - oldBefore[j];
                    model.prepare(ranks, size + j);
                    insert(size + j, rank);
                }
                size += length;
                continue;
            }
        }

        model.prepare(ranks, size);
        int rank = search.findRank(ranks, parity, size, model.predict(size));
        insert(size, rank);
        ++size;
    }

    cout << '!';
    for (int i = 0; i < n; ++i) {
        cout << ' ' << ranks[i] + 1;
    }
    cout << '\n' << flush;
}

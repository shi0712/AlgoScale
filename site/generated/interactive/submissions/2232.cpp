#include <bits/stdc++.h>
using namespace std;

int n, baseScore;
bool finishedInteraction = false;
mt19937_64 rng(chrono::high_resolution_clock::now().time_since_epoch().count());

int askSheet(const string &s) {
    cout << s << '\n' << flush;
    int response;
    if (!(cin >> response)) {
        finishedInteraction = true;
        return -1;
    }
    if (response == -1 || response == n) finishedInteraction = true;
    return response;
}

int askBlock(int left, uint64_t queryMask) {
    string sheet(n, 'F');
    int k = __builtin_popcountll(queryMask);
    for (int i = 0; queryMask; ++i) {
        if (queryMask & (1ULL << i)) sheet[left + i] = 'T';
    }
    int response = askSheet(sheet);
    if (finishedInteraction) return 0;
    return (response - baseScore + k) / 2;
}

uint64_t balancedRandomMask(int k) {
    array<int, 40> positions{};
    iota(positions.begin(), positions.begin() + k, 0);
    shuffle(positions.begin(), positions.begin() + k, rng);
    uint64_t mask = 0;
    int take = max(1, k / 2);
    for (int i = 0; i < take; ++i) mask |= 1ULL << positions[i];
    return mask;
}

uint64_t arbitraryRandomMask(int k) {
    uint64_t mask = 0;
    for (int i = 0; i < k; ++i) {
        if (rng() & 1) mask |= 1ULL << i;
    }
    return mask;
}

uint64_t chooseDistinguishingQuery(const vector<uint64_t> &candidates, int k) {
    vector<uint64_t> queries;
    queries.reserve(140);

    for (int i = 0; i < k; ++i) queries.push_back(1ULL << i);
    for (int i = 0; i < 72; ++i) queries.push_back(balancedRandomMask(k));
    for (int i = 0; i < 24; ++i) queries.push_back(arbitraryRandomMask(k));

    if (k <= 40) {
        uint64_t full = (1ULL << k) - 1;
        queries.push_back(0xAAAAAAAAAAAAAAAAULL & full);
        queries.push_back(0x5555555555555555ULL & full);
    }

    sort(queries.begin(), queries.end());
    queries.erase(unique(queries.begin(), queries.end()), queries.end());

    uint64_t bestQuery = queries[0];
    int bestLargest = INT_MAX;
    long long bestSquares = LLONG_MAX;

    for (uint64_t query : queries) {
        array<int, 41> counts{};
        for (uint64_t candidate : candidates) {
            ++counts[__builtin_popcountll(candidate & query)];
        }

        int largest = 0;
        long long squares = 0;
        for (int i = 0; i <= k; ++i) {
            largest = max(largest, counts[i]);
            squares += 1LL * counts[i] * counts[i];
        }

        if (largest < bestLargest ||
            (largest == bestLargest && squares < bestSquares)) {
            bestLargest = largest;
            bestSquares = squares;
            bestQuery = query;
        }
    }
    return bestQuery;
}

uint64_t solveByIndividualBits(int left, int k, int weight) {
    uint64_t answer = 0;
    int remainingWeight = weight;

    for (int i = 0; i < k; ++i) {
        int positionsLeft = k - i;
        if (remainingWeight == 0) break;
        if (remainingWeight == positionsLeft) {
            for (int j = i; j < k; ++j) answer |= 1ULL << j;
            break;
        }

        int bit = askBlock(left, 1ULL << i);
        if (finishedInteraction) return 0;
        if (bit) {
            answer |= 1ULL << i;
            --remainingWeight;
        }
    }
    return answer;
}

uint64_t solveSmallBlock(int left, int k, int knownWeight) {
    uint64_t full = (1ULL << k) - 1;
    int weight = knownWeight;

    if (weight < 0) {
        weight = askBlock(left, full);
        if (finishedInteraction) return 0;
    }

    if (weight == 0) return 0;
    if (weight == k) return full;

    vector<uint64_t> candidates;
    uint64_t limit = 1ULL << k;
    for (uint64_t mask = 0; mask < limit; ++mask) {
        if (__builtin_popcountll(mask) == weight) candidates.push_back(mask);
    }

    int steps = 0;
    while (candidates.size() > 1) {
        if (++steps > 2 * k) return solveByIndividualBits(left, k, weight);

        uint64_t query = chooseDistinguishingQuery(candidates, k);
        int intersection = askBlock(left, query);
        if (finishedInteraction) return 0;

        size_t out = 0;
        for (uint64_t candidate : candidates) {
            if (__builtin_popcountll(candidate & query) == intersection) {
                candidates[out++] = candidate;
            }
        }
        candidates.resize(out);
    }

    if (candidates.empty()) return solveByIndividualBits(left, k, weight);
    return candidates[0];
}

struct Measurement {
    uint64_t query;
    int value;
};

struct Signature {
    uint64_t a = 0, b = 0;
};

struct Node {
    Signature signature;
    uint32_t assignment;
    int32_t next;
};

struct DecodeResult {
    bool tooMany = false;
    vector<uint64_t> candidates;
};

void setField(Signature &signature, int field, int value) {
    if (field < 12) {
        signature.a |= uint64_t(value) << (5 * field);
    } else {
        signature.b |= uint64_t(value) << (5 * (field - 12));
    }
}

int getField(const Signature &signature, int field) {
    if (field < 12) return (signature.a >> (5 * field)) & 31;
    return (signature.b >> (5 * (field - 12))) & 31;
}

uint64_t hashSignature(const Signature &signature) {
    uint64_t x = signature.a ^ (signature.b + 0x9e3779b97f4a7c15ULL +
                               (signature.a << 6) + (signature.a >> 2));
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

DecodeResult decodeBlock(const vector<Measurement> &measurements,
                         int k, int leftSize, int leftWeight) {
    constexpr size_t CANDIDATE_LIMIT = 512;
    DecodeResult result;

    int rightSize = k - leftSize;
    int rightWeight = measurements[0].value - leftWeight;
    int m = measurements.size();

    uint32_t leftCount = 1u << leftSize;
    uint32_t rightCount = 1u << rightSize;

    vector<Signature> leftSignatures(leftCount), rightSignatures(rightCount);
    vector<Signature> leftBitSignatures(leftSize), rightBitSignatures(rightSize);

    for (int bit = 0; bit < leftSize; ++bit) {
        for (int j = 0; j < m; ++j) {
            if ((measurements[j].query >> bit) & 1ULL) {
                setField(leftBitSignatures[bit], j, 1);
            }
        }
    }
    for (int bit = 0; bit < rightSize; ++bit) {
        for (int j = 0; j < m; ++j) {
            if ((measurements[j].query >> (leftSize + bit)) & 1ULL) {
                setField(rightBitSignatures[bit], j, 1);
            }
        }
    }

    for (uint32_t mask = 1; mask < leftCount; ++mask) {
        uint32_t low = mask & -mask;
        int bit = __builtin_ctz(low);
        uint32_t previous = mask ^ low;
        leftSignatures[mask].a =
            leftSignatures[previous].a + leftBitSignatures[bit].a;
        leftSignatures[mask].b =
            leftSignatures[previous].b + leftBitSignatures[bit].b;
    }
    for (uint32_t mask = 1; mask < rightCount; ++mask) {
        uint32_t low = mask & -mask;
        int bit = __builtin_ctz(low);
        uint32_t previous = mask ^ low;
        rightSignatures[mask].a =
            rightSignatures[previous].a + rightBitSignatures[bit].a;
        rightSignatures[mask].b =
            rightSignatures[previous].b + rightBitSignatures[bit].b;
    }

    uint32_t tableSize = 1u << (leftSize + 1);
    vector<int> heads(tableSize, 0);
    vector<Node> nodes;
    nodes.reserve(leftCount / 4 + 1);

    for (uint32_t mask = 0; mask < leftCount; ++mask) {
        if (__builtin_popcount(mask) != leftWeight) continue;
        uint32_t bucket = hashSignature(leftSignatures[mask]) & (tableSize - 1);
        int next = heads[bucket] - 1;
        nodes.push_back(Node{leftSignatures[mask], mask, next});
        heads[bucket] = int(nodes.size());
    }

    uint64_t leftQueryMask = (1ULL << leftSize) - 1;
    array<int, 24> maximumLeft{};

    for (int j = 0; j < m; ++j) {
        maximumLeft[j] =
            __builtin_popcountll(measurements[j].query & leftQueryMask);
    }

    result.candidates.reserve(16);

    for (uint32_t rightMask = 0; rightMask < rightCount; ++rightMask) {
        if (__builtin_popcount(rightMask) != rightWeight) continue;

        Signature needed;
        bool valid = true;

        for (int j = 0; j < m; ++j) {
            int rightValue = getField(rightSignatures[rightMask], j);
            int leftValue = measurements[j].value - rightValue;
            if (leftValue < 0 || leftValue > maximumLeft[j]) {
                valid = false;
                break;
            }
            setField(needed, j, leftValue);
        }
        if (!valid) continue;

        uint32_t bucket = hashSignature(needed) & (tableSize - 1);
        int id = heads[bucket];
        while (id > 0) {
            const Node &node = nodes[id - 1];
            if (node.signature.a == needed.a && node.signature.b == needed.b) {
                uint64_t candidate = uint64_t(node.assignment) |
                                     (uint64_t(rightMask) << leftSize);
                result.candidates.push_back(candidate);
                if (result.candidates.size() > CANDIDATE_LIMIT) {
                    result.tooMany = true;
                    result.candidates.clear();
                    return result;
                }
            }
            id = node.next + 1;
        }
    }

    return result;
}

uint64_t solveLargeBlock(int left, int k, int knownWeight) {
    uint64_t full = (1ULL << k) - 1;
    int weight = knownWeight;

    if (weight < 0) {
        weight = askBlock(left, full);
        if (finishedInteraction) return 0;
    }

    if (weight == 0) return 0;
    if (weight == k) return full;

    int leftSize = k / 2;
    int rightSize = k - leftSize;
    uint64_t leftHalf = (1ULL << leftSize) - 1;

    int leftWeight = askBlock(left, leftHalf);
    if (finishedInteraction) return 0;
    int rightWeight = weight - leftWeight;

    uint64_t fixedAnswer = 0;
    bool leftFixed = (leftWeight == 0 || leftWeight == leftSize);
    bool rightFixed = (rightWeight == 0 || rightWeight == rightSize);

    if (leftFixed && leftWeight == leftSize) fixedAnswer |= leftHalf;
    if (rightFixed && rightWeight == rightSize) {
        fixedAnswer |= ((1ULL << rightSize) - 1) << leftSize;
    }
    if (leftFixed && rightFixed) return fixedAnswer;

    vector<Measurement> measurements;
    measurements.push_back({full, weight});
    measurements.push_back({leftHalf, leftWeight});

    int randomQueries = 0;
    int decodeStart = (k >= 35 ? 7 : 5);

    while (true) {
        if (randomQueries >= decodeStart) {
            DecodeResult decoded =
                decodeBlock(measurements, k, leftSize, leftWeight);

            if (!decoded.tooMany && !decoded.candidates.empty()) {
                vector<uint64_t> candidates = move(decoded.candidates);
                int adaptiveSteps = 0;

                while (candidates.size() > 1) {
                    if (++adaptiveSteps > 2 * k) {
                        return solveByIndividualBits(left, k, weight);
                    }

                    uint64_t query = chooseDistinguishingQuery(candidates, k);
                    int intersection = askBlock(left, query);
                    if (finishedInteraction) return 0;

                    size_t out = 0;
                    for (uint64_t candidate : candidates) {
                        if (__builtin_popcountll(candidate & query) == intersection) {
                            candidates[out++] = candidate;
                        }
                    }
                    candidates.resize(out);
                }

                if (!candidates.empty()) return candidates[0];
                return solveByIndividualBits(left, k, weight);
            }

            if (!decoded.tooMany && decoded.candidates.empty()) {
                return solveByIndividualBits(left, k, weight);
            }
        }

        if (measurements.size() >= 24) {
            return solveByIndividualBits(left, k, weight);
        }

        uint64_t query;
        bool duplicate;
        do {
            query = balancedRandomMask(k);
            duplicate = false;
            for (const Measurement &measurement : measurements) {
                if (measurement.query == query) {
                    duplicate = true;
                    break;
                }
            }
        } while (duplicate);

        int value = askBlock(left, query);
        if (finishedInteraction) return 0;
        measurements.push_back({query, value});
        ++randomQueries;
    }
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    string initial(n, 'F');
    baseScore = askSheet(initial);
    if (finishedInteraction) return 0;

    int remainingWeight = n - baseScore;
    int remainingPositions = n;
    string answer(n, 'F');

    int left = 0;
    while (left < n) {
        if (remainingWeight == 0) break;
        if (remainingWeight == remainingPositions) {
            for (int i = left; i < n; ++i) answer[i] = 'T';
            break;
        }

        int k = min(40, n - left);
        int knownWeight = (left + k == n ? remainingWeight : -1);
        uint64_t blockAnswer;

        if (k <= 20) {
            blockAnswer = solveSmallBlock(left, k, knownWeight);
        } else {
            blockAnswer = solveLargeBlock(left, k, knownWeight);
        }

        if (finishedInteraction) return 0;

        for (int i = 0; i < k; ++i) {
            if ((blockAnswer >> i) & 1ULL) answer[left + i] = 'T';
        }

        remainingWeight -= __builtin_popcountll(blockAnswer);
        remainingPositions -= k;
        left += k;
    }

    askSheet(answer);
    return 0;
}

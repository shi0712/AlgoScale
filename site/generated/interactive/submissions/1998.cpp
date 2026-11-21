#include <algorithm>
#include <bit>
#include <cmath>
#include <iostream>
#include <utility>
#include <vector>

using namespace std;

class SmallSplits {
public:
    static constexpr int limit = 40;
    int cut[limit + 1][limit + 1][limit + 1]{};

    SmallSplits() {
        double choose[limit + 1][limit + 1]{};
        for (int length = 0; length <= limit; ++length) {
            choose[length][0] = choose[length][length] = 1;
            for (int j = 1; j < length; ++j)
                choose[length][j] = choose[length - 1][j - 1] + choose[length - 1][j];
        }

        // For small mixed blocks, minimize the expected remaining queries
        // under uniformly distributed zero positions within the block.
        double cost[limit + 1][limit + 1][limit + 1]{};
        constexpr double epsilon = 1e-11;
        for (int length = 2; length <= limit; ++length) {
            for (int zeros = 1; zeros < length; ++zeros) {
                for (int rank = 1; rank <= zeros; ++rank) {
                    double best = length;
                    double center = double(rank) * (length + 1) / (zeros + 1) - 0.5;
                    int& bestCut = cut[length][zeros][rank];
                    for (int left = rank; left < length - zeros + rank; ++left) {
                        int right = length - left;
                        double expected = 0;
                        for (int z = max(0, zeros - right); z <= min(zeros, left); ++z) {
                            double next = z >= rank ? cost[left][z][rank]
                                                    : cost[right][zeros - z][rank - z];
                            expected += choose[left][z] * choose[right][zeros - z] * next;
                        }
                        expected /= choose[length][zeros];
                        if (expected < best - epsilon ||
                            (abs(expected - best) <= epsilon &&
                             abs(left - center) < abs(bestCut - center))) {
                            best = expected;
                            bestCut = left;
                        }
                    }
                    cost[length][zeros][rank] = 1 + best;
                }
            }
        }
    }
};

const SmallSplits smallSplits;

class Fenwick {
    vector<int> sums;
    int highest;

public:
    explicit Fenwick(int n) : sums(n + 1), highest(bit_floor(unsigned(n))) {}

    void add(int index, int delta) {
        for (int i = index; i < int(sums.size()); i += i & -i)
            sums[i] += delta;
    }

    // Returns the block endpoint and the rank within that block.
    pair<int, int> locate(int rank) const {
        int position = 0;
        for (int bit = highest; bit; bit >>= 1) {
            int next = position + bit;
            if (next < int(sums.size()) && sums[next] < rank) {
                position = next;
                rank -= sums[next];
            }
        }
        return {position + 1, rank};
    }
};

class ZeroFinder {
    enum class Order : unsigned char { Unknown, ZerosFirst, OnesFirst };

    int n;
    Fenwick weights;
    vector<int> start;
    vector<int> count;
    vector<Order> suggestedOrder;
    int knownZeros = 0;
    int tailLowerBound = 0;
    int previousRank = -1;
    int clusterEvidence = 0;

    void setCount(int end, int zeros) {
        weights.add(end, zeros - count[end]);
        count[end] = zeros;
    }

    void split(int end, int middle, int leftZeros, int rightZeros) {
        start[middle] = start[end];
        start[end] = middle + 1;
        setCount(middle, leftZeros);
        setCount(end, rightZeros);
    }

    void determineTail(int zeros) {
        tailLowerBound = -1;
        knownZeros += zeros;
        setCount(n, zeros);

        int preceding = start[n] - 1;
        if (preceding > 0 && start[preceding] > 0) {
            int leftLength = preceding - start[preceding] + 1;
            int leftZeros = count[preceding];
            int length = n - preceding + leftLength;
            int missing = expectedMissing(length, zeros + leftZeros, leftLength, leftZeros);
            if (missing >= 3)
                suggestedOrder[n] = leftZeros == 0 ? Order::OnesFirst : Order::ZerosFirst;
        }
    }

    int chooseTailCut(int length, int rank, bool nearbyRank, bool firstCut) const {
        int slack = length - tailLowerBound;
        if (length > SmallSplits::limit && length < n && slack >= 8 &&
            (!nearbyRank || !firstCut))
            return length;
        return rank + (slack - 1) / 2;
    }

    int chooseKnownCut(int length, int zeros, int rank, Order order,
                       bool nearbyRank, int slowSteps) const {
        int ones = length - zeros;
        int lastCut = rank + ones - 1;
        if (order != Order::Unknown)
            return clamp(order == Order::ZerosFirst ? zeros : ones, rank, lastCut);

        // The target has only ones + 1 feasible positions. Bisect those
        // positions for nearby requests and after insufficient progress.
        if (nearbyRank || clusterEvidence >= 2 || slowSteps >= 3)
            return rank + (ones - 1) / 2;
        if (length <= SmallSplits::limit)
            return smallSplits.cut[length][zeros][rank];

        // Interpolate from the end nearer the requested rank.
        int rankFromRight = zeros - rank + 1;
        bool fromRight = rankFromRight < rank;
        int nearRank = min(rank, rankFromRight);
        int cut = int((nearRank - 0.25) * (length + 1) / (zeros + 0.5));
        return clamp(fromRight ? length - cut : cut, rank, lastCut);
    }

    static int expectedMissing(int length, int zeros, int partLength, int partZeros) {
        if (partZeros == 0)
            return int(1LL * partLength * zeros / length);
        if (partZeros == partLength)
            return int(1LL * partLength * (length - zeros) / length);
        return 0;
    }

    void recordSplit(int end, int middle, int leftLength, int leftZeros,
                     int rightLength, int rightZeros, Order order) {
        int length = leftLength + rightLength;
        int zeros = leftZeros + rightZeros;
        // A homogeneous side can suggest a single transition in its
        // neighbor. Test that suggestion at most once before splitting again.
        int leftMissing = expectedMissing(length, zeros, leftLength, leftZeros);
        int rightMissing = expectedMissing(length, zeros, rightLength, rightZeros);
        suggestedOrder[middle] = suggestedOrder[end] = Order::Unknown;
        if (order == Order::Unknown) {
            if (leftMissing >= 3)
                suggestedOrder[end] = leftZeros == 0 ? Order::OnesFirst : Order::ZerosFirst;
            if (rightMissing >= 3)
                suggestedOrder[middle] = rightZeros == 0 ? Order::ZerosFirst : Order::OnesFirst;
        }

        int missing = max(leftMissing, rightMissing);
        if (missing >= 5)
            clusterEvidence += missing >= 9 ? 2 : 1;
    }

public:
    explicit ZeroFinder(int size) : n(size), weights(n), start(n + 1),
                                    count(n + 1), suggestedOrder(n + 1) {
        // Only the final block can have an unknown count. Its Fenwick
        // weight is its length until the count becomes exact.
        start[n] = 1;
        setCount(n, n);
    }

    template <class Ask>
    int findAndRemove(int rank, int remaining, Ask ask) {
        if (tailLowerBound >= 0) {
            tailLowerBound = max(tailLowerBound, max(rank, remaining) - knownZeros);
            if (tailLowerBound == count[n])
                determineTail(tailLowerBound);
        }

        bool nearbyRank = remaining > 1 && abs(rank - previousRank) <= 1;
        previousRank = rank;
        auto [end, localRank] = weights.locate(rank);
        int slowSteps = 0;
        bool firstCut = true;

        while (true) {
            int left = start[end];
            int length = end - left + 1;
            int zeros = count[end];
            bool unknown = end == n && tailLowerBound >= 0;
            if (!unknown && zeros == length) {
                int answer = left + localRank - 1;
                if (answer > left) {
                    start[answer - 1] = left;
                    suggestedOrder[answer - 1] = Order::Unknown;
                    setCount(answer - 1, answer - left);
                }
                start[end] = answer + 1;
                setCount(end, end - answer);
                --knownZeros;
                return answer;
            }

            Order order = suggestedOrder[end];
            int leftLength = unknown ? chooseTailCut(length, localRank, nearbyRank, firstCut)
                                     : chooseKnownCut(length, zeros, localRank, order,
                                                      nearbyRank, slowSteps);
            firstCut = false;
            int middle = left + leftLength - 1;
            int rightLength = length - leftLength;
            int leftZeros;
            if (!unknown && rightLength < leftLength) {
                int rightZeros = ask(middle + 1, end);
                if (rightZeros < 0)
                    return -1;
                leftZeros = zeros - rightZeros;
            } else {
                leftZeros = ask(left, middle);
                if (leftZeros < 0)
                    return -1;
            }

            int rightZeros = unknown ? rightLength : zeros - leftZeros;
            if (leftZeros < 0 || leftZeros > leftLength ||
                rightZeros < 0 || rightZeros > rightLength ||
                (unknown && leftZeros + rightZeros < tailLowerBound))
                return -1;

            if (leftLength == length) {
                determineTail(leftZeros);
                slowSteps = 0;
                continue;
            }

            split(end, middle, leftZeros, rightZeros);
            if (unknown) {
                knownZeros += leftZeros;
                tailLowerBound = max(0, tailLowerBound - leftZeros);
                if (tailLowerBound == rightLength)
                    determineTail(rightLength);
            } else {
                recordSplit(end, middle, leftLength, leftZeros, rightLength, rightZeros, order);
            }

            bool goLeft = localRank <= leftZeros;
            int width = unknown ? length : length - zeros;
            int nextWidth = goLeft ? leftLength - leftZeros
                                   : (unknown ? rightLength : rightLength - rightZeros);
            slowSteps = 4 * nextWidth > 3 * width ? slowSteps + 1 : 0;
            if (goLeft)
                end = middle;
            else
                localRank -= leftZeros;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, t;
    if (!(cin >> n >> t) || n <= 0 || t <= 0)
        return 0;

    ZeroFinder finder(n);
    for (int remaining = t; remaining > 0; --remaining) {
        int rank;
        if (!(cin >> rank) || rank <= 0)
            return 0;

        int answer = finder.findAndRemove(rank, remaining, [](int left, int right) {
            cout << "? " << left << ' ' << right << endl;
            int ones;
            int length = right - left + 1;
            if (!(cin >> ones) || ones < 0 || ones > length)
                return -1;
            return length - ones;
        });
        if (answer < 0)
            return 0;
        cout << "! " << answer << endl;
    }
    return 0;
}

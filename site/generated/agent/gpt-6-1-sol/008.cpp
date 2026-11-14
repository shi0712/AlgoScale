#include <bits/stdc++.h>
using namespace std;

// Square roots can be moved before halvings on each mountain.
// For a fixed halving price t, let c_t(h) = min_k ((h >> k) + t*k).
// The savings c_t(h)-c_t(floor(sqrt(h))) diminish along each root chain,
// so the P best savings can be selected globally. The resulting optimum
// as a function of the halving budget is discrete convex; its supporting
// integer price is found by binary search. Frequencies of all root-chain
// states allow each price evaluation in O(max(h)), with O(max(h)) memory.

class FastInput {
    static constexpr int SIZE = 1 << 16;
    char buffer[SIZE];
    int pos = 0, len = 0;
public:
    int readInt() {
        int value = 0;
        char c;
        do {
            if (pos == len) { len = fread(buffer, 1, SIZE, stdin); pos = 0; }
            c = buffer[pos++];
        } while (c <= ' ');
        do {
            value = value * 10 + c - '0';
            if (pos == len) {
                len = fread(buffer, 1, SIZE, stdin); pos = 0;
                if (!len) return value;
            }
            c = buffer[pos++];
        } while (c >= '0' && c <= '9');
        return value;
    }
};

struct Result {
    long long cost;
    int halves;
};

class Optimizer {
    int maximum, remaining, positive;
    vector<int> weight, cost, counts, halfSums;
    vector<unsigned short> root;
    vector<unsigned char> halves;
public:
    Optimizer(vector<int>&& frequency, int mx, int p, int nonzero)
        : maximum(mx), positive(nonzero), weight(move(frequency)),
          cost(mx + 1), counts(mx + 1), halfSums(mx + 1),
          root(mx + 1), halves(mx + 1) {
        int r = 0;
        for (int h = 0; h <= maximum; ++h) {
            if ((r + 1) * (r + 1) <= h) ++r;
            root[h] = r;
        }
        int total = 0;
        for (int h = maximum; h >= 2; --h) {
            total += weight[h];
            weight[root[h]] += weight[h];
        }
        remaining = max(0, total - p);
    }

    bool allRooted() const { return remaining == 0; }

    Result evaluate(int price) {
        // c[h] = min_k (floor(h / 2^k) + price * k).
        // Among ties, retain the smallest number of halvings.
        for (int h = 1; h <= maximum; ++h) {
            int candidate = cost[h >> 1] + price;
            if (candidate < h) {
                cost[h] = candidate;
                halves[h] = halves[h >> 1] + 1;
            } else {
                cost[h] = h;
                halves[h] = 0;
            }
        }
        int maxGain = cost[maximum];
        fill(counts.begin(), counts.begin() + maxGain + 1, 0);
        fill(halfSums.begin(), halfSums.begin() + maxGain + 1, 0);
        for (int h = 2; h <= maximum; ++h) {
            int w = weight[h];
            if (!w) continue;
            int s = root[h];
            int gain = cost[h] - cost[s];
            counts[gain] += w;
            halfSums[gain] += w * (halves[h] - halves[s]);
        }
        // All possible square-root operations form diminishing chains.
        // Applying the P largest savings leaves 'remaining' smallest ones.
        int left = remaining;
        int boundary = 0, usedHalves = 0;
        long long value = positive;
        for (; boundary <= maxGain; ++boundary) {
            if (counts[boundary] >= left) break;
            left -= counts[boundary];
            value += 1LL * boundary * counts[boundary];
            usedHalves += halfSums[boundary];
        }
        value += 1LL * boundary * left;
        if (left == counts[boundary]) {
            usedHalves += halfSums[boundary];
        } else {
            // Break equal-cost choices by minimizing the half-operation count.
            int ties[32] = {};
            for (int h = 2; h <= maximum; ++h) {
                if (weight[h] && cost[h] - cost[root[h]] == boundary)
                    ties[halves[h] - halves[root[h]]] += weight[h];
            }
            for (int d = 0; left; ++d) {
                int take = min(left, ties[d]);
                usedHalves += d * take;
                left -= take;
            }
        }
        return {value, usedHalves};
    }
};

int main() {
    FastInput in;
    int n = in.readInt(), p = in.readInt(), q = in.readInt();
    constexpr int LIMIT = 2000000;
    vector<int> frequency(LIMIT + 1);
    int maximum = 0, positive = 0;
    for (int i = 0; i < n; ++i) {
        int h = in.readInt();
        ++frequency[h];
        maximum = max(maximum, h);
        positive += h != 0;
    }
    if (!positive) { puts("0"); return 0; }
    frequency.resize(maximum + 1);
    Optimizer opt(move(frequency), maximum, p, positive);
    if (opt.allRooted()) {
        printf("%d\n", max(0, positive - q));
        return 0;
    }
    Result first = opt.evaluate(1);
    if (first.cost <= q) { puts("0"); return 0; }
    int lo = 1, hi = (maximum + 1) / 2;
    if (first.halves <= q) {
        printf("%lld\n", first.cost - q);
        return 0;
    }
    while (lo < hi) {
        int mid = (lo + hi) / 2;
        Result r = opt.evaluate(mid);
        if (r.halves <= q) hi = mid;
        else lo = mid + 1;
    }
    Result answer = opt.evaluate(lo);
    printf("%lld\n", answer.cost - 1LL * lo * q);
    return 0;
}

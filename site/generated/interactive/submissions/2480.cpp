#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto ask = [&](const string &sheet) -> int {
        cout << sheet << '\n' << flush;
        int reply;
        if (!(cin >> reply)) return -1;
        return reply;
    };

    string base(n, 'F');
    int baseline = ask(base);
    if (baseline < 0 || baseline == n) return 0;

    vector<int> blocks;
    if (n % 25) blocks.push_back(n % 25);
    for (int i = 0; i < n / 25; ++i) blocks.push_back(25);

    string answer(n, 'F');
    vector<uint32_t> candidates;
    candidates.reserve(size_t{1} << min(n, 25));
    mt19937 rng(0x9e3779b9u);
    int remaining = n - baseline;
    int offset = 0;

    for (int k : blocks) {
        int after = n - offset - k;
        int minWeight = max(0, remaining - after);
        int maxWeight = min(k, remaining);
        bool unconstrained = minWeight == 0 && maxWeight == k;

        candidates.clear();
        uint32_t limit = uint32_t{1} << k;
        if (unconstrained) {
            candidates.resize(limit);
            iota(candidates.begin(), candidates.end(), 0);
        } else {
            for (uint32_t x = 0; x < limit; ++x) {
                int w = __builtin_popcount(x);
                if (minWeight <= w && w <= maxWeight) candidates.push_back(x);
            }
        }

        int depth = 0;
        while (candidates.size() > 1) {
            uint32_t mask = 0;
            int half = max(1, k / 2);
            if (depth == 0 || (depth == 1 && unconstrained)) {
                for (int i = 0; i < k; ++i) {
                    if ((i + depth * half) % k < half) mask |= uint32_t{1} << i;
                }
            } else {
                double bestEntropy = -1.0;
                int trials = max(32, min(4096, int(1000000 / candidates.size())));
                for (int t = 0; t < trials; ++t) {
                    uint32_t proposal = rng() & (limit - 1);
                    int counts[26] = {};
                    for (uint32_t x : candidates) {
                        ++counts[__builtin_popcount(x & proposal)];
                    }
                    double entropy = 0.0;
                    for (int i = 0; i <= k; ++i) {
                        if (!counts[i]) continue;
                        double p = double(counts[i]) / candidates.size();
                        entropy -= p * log2(p);
                    }
                    if (entropy > bestEntropy) {
                        bestEntropy = entropy;
                        mask = proposal;
                    }
                }
                if (bestEntropy <= 0.0) {
                    mask = candidates[0] & ~candidates[1];
                    if (!mask) mask = candidates[1] & ~candidates[0];
                }
            }

            string sheet = base;
            for (int i = 0; i < k; ++i) {
                if (mask >> i & 1) sheet[offset + i] = 'T';
            }
            int reply = ask(sheet);
            if (reply < 0 || reply == n) return 0;

            int subsetSize = __builtin_popcount(mask);
            int trueCount = (reply - baseline + subsetSize) / 2;
            size_t newSize = 0;
            for (uint32_t x : candidates) {
                if (__builtin_popcount(x & mask) == trueCount) {
                    candidates[newSize++] = x;
                }
            }
            if (newSize == 0) return 0;
            candidates.resize(newSize);
            ++depth;
        }

        uint32_t value = candidates.empty() ? 0 : candidates[0];
        for (int i = 0; i < k; ++i) {
            if (value >> i & 1) answer[offset + i] = 'T';
        }
        remaining -= __builtin_popcount(value);
        offset += k;
    }

    int reply = ask(answer);
    (void)reply;
    return 0;
}

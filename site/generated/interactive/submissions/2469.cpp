#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> order = {0};
    vector<int> rank(n, 0);
    vector<unsigned char> intervalParity(n, 0);

    for (int r = 1; r < n; ++r) {
        const int count = r + 1;
        const int words = (count + 63) / 64;
        const uint64_t lastMask = (count % 64 == 0)
            ? ~uint64_t(0) : ((uint64_t(1) << (count % 64)) - 1);

        vector<uint64_t> bits(static_cast<size_t>(r + 1) * words, 0);
        for (int l = r - 1; l >= 0; --l) {
            uint64_t* row = bits.data() + static_cast<size_t>(l) * words;
            const uint64_t* next = row + words;
            for (int w = 0; w < words; ++w) row[w] = next[w];

            const int setWords = rank[l] / 64;
            const int rem = rank[l] % 64;
            for (int w = 0; w < setWords; ++w) row[w] = ~row[w];
            row[setWords] ^= (rem == 63) ? ~uint64_t(0)
                : ((uint64_t(1) << (rem + 1)) - 1);
        }

        vector<uint64_t> candidates(words, ~uint64_t(0));
        candidates.back() &= lastMask;
        int remaining = count;

        auto applyQuery = [&](int l) {
            cout << "? " << l + 1 << ' ' << r + 1 << '\n' << flush;
            int response;
            if (!(cin >> response) || response == -1) exit(0);
            const int bit = response ^ intervalParity[l];

            const uint64_t* row = bits.data() + static_cast<size_t>(l) * words;
            remaining = 0;
            for (int w = 0; w < words; ++w) {
                if (bit) candidates[w] &= row[w];
                else candidates[w] &= ~row[w];
            }
            candidates.back() &= lastMask;
            for (uint64_t word : candidates) remaining += __builtin_popcountll(word);
        };

        if (rank[r - 1] == 0 || rank[r - 1] == r - 1) {
            applyQuery(r - 1);
        }

        while (remaining > 1) {
            int bestL = -1;
            int bestScore = -1;
            for (int l = r - 1; l >= 0; --l) {
                const uint64_t* row = bits.data() + static_cast<size_t>(l) * words;
                int ones = 0;
                for (int w = 0; w < words; ++w) {
                    ones += __builtin_popcountll(row[w] & candidates[w]);
                }
                const int score = min(ones, remaining - ones);
                if (score > bestScore) {
                    bestScore = score;
                    bestL = l;
                    if (score == remaining / 2) break;
                }
            }

            applyQuery(bestL);
        }

        int k = 0;
        for (int w = 0; w < words; ++w) {
            if (candidates[w]) {
                k = w * 64 + __builtin_ctzll(candidates[w]);
                break;
            }
        }

        for (int l = 0; l < r; ++l) {
            const uint64_t value = bits[static_cast<size_t>(l) * words + k / 64];
            intervalParity[l] ^= (value >> (k % 64)) & 1;
        }
        intervalParity[r] = 0;

        order.insert(order.begin() + k, r);
        for (int i = 0; i < r; ++i) {
            if (rank[i] >= k) ++rank[i];
        }
        rank[r] = k;
    }

    vector<int> answer(n);
    for (int i = 0; i < n; ++i) answer[order[i]] = i + 1;
    cout << '!';
    for (int value : answer) cout << ' ' << value;
    cout << '\n' << flush;
    return 0;
}

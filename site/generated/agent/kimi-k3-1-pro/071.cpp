#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto query = [&](int l, int r) {
        cout << "? " << l << ' ' << r << '\n' << flush;
        int response;
        if (!(cin >> response) || response == -1) exit(0);
        return response;
    };

    vector<int> p(n + 1), sorted;
    vector<vector<unsigned char>> inv(n + 1, vector<unsigned char>(n + 1));
    p[1] = 1;
    sorted.push_back(1);

    for (int i = 2; i <= n; ++i) {
        int m = i - 1;
        int full = query(1, i);
        int total = full ^ inv[1][m];

        int words = (m + 64) / 64;
        vector<vector<uint64_t>> parity(m + 1, vector<uint64_t>(words));
        for (int t = 1; t <= m; ++t) {
            parity[t] = parity[t - 1];
            int rank = lower_bound(sorted.begin(), sorted.end(), p[t]) - sorted.begin();
            int last_word = rank >> 6;
            if (last_word == 0) {
                parity[t][0] ^= (2ULL << (rank & 63)) - 1;
            } else {
                parity[t][0] ^= ~0ULL;
                for (int w = 1; w < last_word; ++w) parity[t][w] ^= ~0ULL;
                parity[t][last_word] ^= (2ULL << (rank & 63)) - 1;
            }
        }

        vector<uint64_t> candidates(words, 0);
        for (int k = 0; k <= m; ++k) {
            if (((m - k) & 1) == total) candidates[k >> 6] |= 1ULL << (k & 63);
        }

        auto contains = [&](const vector<uint64_t>& bits, int k) {
            return (bits[k >> 6] >> (k & 63)) & 1;
        };

        int candidate_count = (m + 2) / 2;
        while (candidate_count > 1) {
            int best_t = 1, best_worst = candidate_count + 1;
            for (int t = 1; t < m; ++t) {
                int ones = 0;
                for (int w = 0; w < words; ++w) {
                    ones += __builtin_popcountll(parity[t][w] & candidates[w]);
                }
                int worst = max(ones, candidate_count - ones);
                if (worst < best_worst) {
                    best_worst = worst;
                    best_t = t;
                }
            }

            int response = query(best_t + 1, i);
            int prefix_parity = response ^ inv[best_t + 1][m] ^ total;
            for (int w = 0; w < words; ++w) {
                if (prefix_parity) candidates[w] &= parity[best_t][w];
                else candidates[w] &= ~parity[best_t][w];
            }
            candidate_count = 0;
            for (uint64_t word : candidates) candidate_count += __builtin_popcountll(word);
        }

        int k = 0;
        while (!contains(candidates, k)) ++k;
        int value = k + 1;
        for (int j = 1; j <= m; ++j) {
            if (p[j] >= value) ++p[j];
        }
        p[i] = value;
        for (int& old_value : sorted) {
            if (old_value >= value) ++old_value;
        }
        sorted.insert(sorted.begin() + k, value);

        int greater_parity = 0;
        inv[i][i] = 0;
        for (int l = m; l >= 1; --l) {
            greater_parity ^= (p[l] > p[i]);
            inv[l][i] = inv[l][m] ^ greater_parity;
        }
    }

    cout << '!';
    for (int i = 1; i <= n; ++i) cout << ' ' << p[i];
    cout << '\n' << flush;
    return 0;
}

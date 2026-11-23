#include <bits/stdc++.h>
using namespace std;

static constexpr int MAXW = 32;
using Bits = array<uint64_t, MAXW>;

static uint64_t rngState = 0x9e3779b97f4a7c15ULL;
static uint32_t nextRandom() {
    rngState ^= rngState << 7;
    rngState ^= rngState >> 9;
    return static_cast<uint32_t>(rngState >> 16);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    if (n == 1) {
        cout << "! 1\n" << flush;
        return 0;
    }

    // Positions of the already processed elements, sorted by their values.
    vector<int> ordered = {0};

    for (int m = 1; m < n; ++m) {
        const int newPosition = m;       // Zero-based.
        const int newColumn = m + 1;     // One-based, for queries.
        const int rankCount = m + 1;
        const int W = (rankCount + 63) / 64;

        vector<int> rankAt(m);
        for (int i = 0; i < m; ++i) rankAt[ordered[i]] = i;

        /*
           F[l][r] is the parity of the number of elements in positions
           l,...,m-1 which are greater than a new element of insertion rank r.
           Ranks 0,...,m are represented by bits.
        */
        vector<Bits> F(m + 1);
        for (auto &b : F) b.fill(0);

        for (int l = m - 1; l >= 0; --l) {
            for (int w = 0; w < W; ++w) F[l][w] = F[l + 1][w];

            int t = rankAt[l];
            int fullWords = t / 64;
            int rem = t % 64;

            for (int w = 0; w < fullWords; ++w) F[l][w] = ~F[l][w];
            uint64_t mask = (rem == 63) ? ~0ULL : ((1ULL << (rem + 1)) - 1);
            F[l][fullWords] ^= mask;
        }

        auto getBit = [&](int l, int r) -> int {
            return static_cast<int>((F[l][r / 64] >> (r % 64)) & 1ULL);
        };

        // Inversion parity of every suffix of the known prefix.
        vector<int> inv(m + 1, 0);
        for (int l = m - 1; l >= 0; --l) {
            int after = m - 1 - l;
            int greaterParity = getBit(l + 1, rankAt[l]);
            inv[l] = inv[l + 1] ^ (after & 1) ^ greaterParity;
        }

        vector<uint64_t> possible(W, ~0ULL);
        if (rankCount % 64 != 0) {
            possible[W - 1] = (1ULL << (rankCount % 64)) - 1;
        }

        auto trimPossible = [&]() {
            if (rankCount % 64 != 0) {
                possible[W - 1] &= (1ULL << (rankCount % 64)) - 1;
            }
        };

        auto possibleSize = [&]() {
            int result = 0;
            for (int w = 0; w < W; ++w) {
                result += __builtin_popcountll(possible[w]);
            }
            return result;
        };

        int remaining = rankCount;
        vector<int> answer(m + 1, -1);
        answer[m] = 0; // The interval consisting only of the new element.

        auto ask = [&](int l) {
            if (answer[l] == -1) {
                cout << "? " << (l + 1) << ' ' << newColumn << '\n' << flush;
                int response;
                if (!(cin >> response) || response < 0) exit(0);
                answer[l] = response;
            }

            int a = answer[l];
            for (int w = 0; w < W; ++w) {
                uint64_t predictedOne = F[l][w];
                if (inv[l]) predictedOne = ~predictedOne;

                if (a) possible[w] &= predictedOne;
                else possible[w] &= ~predictedOne;
            }
            trimPossible();
            remaining = possibleSize();
        };

        // The complete suffix always splits ranks by their parity.
        ask(0);

        int round = 0;
        while (remaining > 1) {
            vector<int> ranks;
            ranks.reserve(remaining);
            for (int w = 0; w < W; ++w) {
                uint64_t bits = possible[w];
                while (bits) {
                    int b = __builtin_ctzll(bits);
                    ranks.push_back(w * 64 + b);
                    bits &= bits - 1;
                }
            }

            int best = -1;
            int bestWorstPart = remaining;

            auto consider = [&](int l) {
                if (l < 0 || l >= m || answer[l] != -1) return;

                int onesInF = 0;
                for (int w = 0; w < W; ++w) {
                    onesInF += __builtin_popcountll(possible[w] & F[l][w]);
                }

                int ones = inv[l] ? remaining - onesInF : onesInF;
                int zeros = remaining - ones;
                int worstPart = max(ones, zeros);

                if (worstPart < bestWorstPart) {
                    bestWorstPart = worstPart;
                    best = l;
                }
            };

            if (m <= 128) {
                for (int l = 0; l < m; ++l) consider(l);
            } else {
                for (int j = 0; j < 16; ++j) {
                    consider(static_cast<int>((static_cast<long long>(j) * m) / 16));
                }

                int sampleIndices[] = {
                    0,
                    (remaining - 1) / 4,
                    (remaining - 1) / 2,
                    3 * (remaining - 1) / 4,
                    remaining - 1
                };

                for (int index : sampleIndices) {
                    int r = ranks[index];
                    consider(r);
                    consider(m - r);

                    int t = min(r, m - 1);
                    int l = ordered[t];
                    consider(l);
                    consider(l + 1);
                }

                for (int j = 0; j < 24; ++j) {
                    consider(static_cast<int>(nextRandom() % m));
                }
            }

            // A sufficiently informative single parity query is preferable.
            if (best != -1 && 3 * bestWorstPart <= 2 * remaining) {
                ask(best);
                ++round;
                continue;
            }

            /*
               Fall back to a binary-search comparison.  For the element of
               sorted rank t at position l, the two queries starting at l and
               l+1 distinguish insertion ranks r<=t from ranks r>t.
            */
            int t = ranks[(remaining - 1) / 2];
            if (t >= m) t = m - 1;
            int l = ordered[t];

            ask(l);
            if (remaining > 1) ask(l + 1);
            ++round;
        }

        int finalRank = 0;
        for (int w = 0; w < W; ++w) {
            if (possible[w]) {
                finalRank = w * 64 + __builtin_ctzll(possible[w]);
                break;
            }
        }

        ordered.insert(ordered.begin() + finalRank, newPosition);
    }

    vector<int> permutation(n);
    for (int i = 0; i < n; ++i) {
        permutation[ordered[i]] = i + 1;
    }

    cout << '!';
    for (int x : permutation) cout << ' ' << x;
    cout << '\n' << flush;
    return 0;
}

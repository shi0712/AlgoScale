#include <bits/stdc++.h>
using namespace std;

class Solver {
    static constexpr int QUERY_LIMIT = 35000;
    static constexpr int SAMPLE_SIZE = 160;
    static constexpr int SMALL = 8;

    int n;
    int queries = 0;
    mt19937 rng;

    struct Fenwick {
        int n;
        vector<int> bit;

        explicit Fenwick(int n) : n(n), bit(n + 1, 0) {}

        void add(int x) {
            for (; x <= n; x += x & -x) ++bit[x];
        }

        int sum(int x) const {
            int ans = 0;
            for (; x > 0; x -= x & -x) ans += bit[x];
            return ans;
        }
    };

    int ask(int l, int r) {
        if (queries >= QUERY_LIMIT) exit(0);
        ++queries;

        cout << "? " << l << ' ' << r << endl;

        int answer;
        if (!(cin >> answer) || answer == -1) exit(0);
        if (answer != 0 && answer != 1) exit(0);
        return answer;
    }

public:
    explicit Solver(int n)
        : n(n),
          rng(static_cast<unsigned>(
              chrono::steady_clock::now().time_since_epoch().count())) {}

    void run() {
        if (n == 1) {
            cout << "! 1" << endl;
            return;
        }

        // 'order' contains the known positions in increasing value order.
        int first = uniform_int_distribution<int>(1, n)(rng);
        int L = first, R = first;
        vector<int> order{first};
        vector<int> rankAt(n + 1);

        while (static_cast<int>(order.size()) < n) {
            int m = static_cast<int>(order.size());

            bool insertLeft;
            if (L == 1) {
                insertLeft = false;
            } else if (R == n) {
                insertLeft = true;
            } else {
                int availableLeft = L - 1;
                int availableRight = n - R;
                insertLeft =
                    uniform_int_distribution<int>(
                        1, availableLeft + availableRight)(rng)
                    <= availableLeft;
            }

            int newPosition = insertLeft ? L - 1 : R + 1;

            for (int i = 0; i < m; ++i)
                rankAt[order[i]] = i + 1;

            // A candidate k means exactly k known values are smaller
            // than the value at newPosition.
            int candidateCount = m + 1;
            int words = (candidateCount + 63) / 64;

            vector<uint64_t> patterns(static_cast<size_t>(m) * words);
            vector<uint64_t> current(words, 0);
            vector<int> offset(m);

            Fenwick fw(m);
            int knownParity = 0;
            int length = 0;

            auto appendPosition = [&](int position) {
                int rank = rankAt[position];

                if (insertLeft) {
                    knownParity ^= (length - fw.sum(rank)) & 1;
                } else {
                    knownParity ^= fw.sum(rank - 1) & 1;
                }

                fw.add(rank);
                ++length;

                // This known value is greater than the new value
                // precisely for candidates k < rank.
                int fullWords = rank / 64;
                int remaining = rank % 64;

                for (int w = 0; w < fullWords; ++w)
                    current[w] = ~current[w];

                if (remaining)
                    current[fullWords] ^=
                        (uint64_t(1) << remaining) - 1;

                int row = position - L;
                copy(current.begin(), current.end(),
                     patterns.begin() + static_cast<size_t>(row) * words);

                offset[row] =
                    knownParity ^ (insertLeft ? (length & 1) : 0);
            };

            if (insertLeft) {
                for (int position = L; position <= R; ++position)
                    appendPosition(position);
            } else {
                for (int position = R; position >= L; --position)
                    appendPosition(position);
            }

            auto queryRow = [&](int row) {
                int position = L + row;
                int response;
                if (insertLeft)
                    response = ask(newPosition, position);
                else
                    response = ask(position, newPosition);

                return response ^ offset[row];
            };

            vector<uint64_t> candidates(words, ~uint64_t(0));
            if (candidateCount % 64)
                candidates.back() =
                    (uint64_t(1) << (candidateCount % 64)) - 1;

            int remainingCandidates = candidateCount;

            auto applyResponse = [&](int row, int response) {
                const uint64_t* pattern =
                    patterns.data() + static_cast<size_t>(row) * words;

                remainingCandidates = 0;
                for (int w = 0; w < words; ++w) {
                    candidates[w] &=
                        response ? pattern[w] : ~pattern[w];
                    remainingCandidates += popcount(candidates[w]);
                }
            };

            // The whole known interval tests the parity of the
            // insertion rank and always gives an almost equal split.
            if (remainingCandidates > SMALL) {
                int row = insertLeft ? m - 1 : 0;
                applyResponse(row, queryRow(row));
            }

            int insertionRank = -1;

            while (remainingCandidates > SMALL) {
                vector<int> activeWords;
                activeWords.reserve(words);
                for (int w = 0; w < words; ++w)
                    if (candidates[w])
                        activeWords.push_back(w);

                auto numberOfOnes = [&](int row) {
                    const uint64_t* pattern =
                        patterns.data() + static_cast<size_t>(row) * words;

                    int result = 0;
                    for (int w : activeWords)
                        result += popcount(pattern[w] & candidates[w]);
                    return result;
                };

                int bestRow = -1;
                int bestScore = -1;

                auto consider = [&](int row) {
                    int ones = numberOfOnes(row);
                    int score =
                        min(ones, remainingCandidates - ones);

                    if (score > bestScore) {
                        bestScore = score;
                        bestRow = row;
                    }
                };

                int samples = min(m, SAMPLE_SIZE);
                for (int j = 0; j < samples; ++j) {
                    int row = uniform_int_distribution<int>(0, m - 1)(rng);
                    consider(row);
                    if (bestScore == remainingCandidates / 2)
                        break;
                }

                // If sampling did not find a sufficiently balanced
                // query, inspect every available interval.
                if (bestScore * 20 < remainingCandidates * 9) {
                    for (int row = 0; row < m; ++row) {
                        consider(row);
                        if (bestScore == remainingCandidates / 2)
                            break;
                    }
                }

                if (bestScore <= 0) exit(0);
                applyResponse(bestRow, queryRow(bestRow));
            }

            vector<int> slots;
            for (int w = 0; w < words; ++w) {
                uint64_t bits = candidates[w];
                while (bits) {
                    int b = countr_zero(bits);
                    slots.push_back(w * 64 + b);
                    bits &= bits - 1;
                }
            }

            if (slots.size() == 1) {
                insertionRank = slots[0];
            } else {
                // Construct an optimal minimax decision tree for
                // the remaining at most eight insertion ranks.
                int c = static_cast<int>(slots.size());
                int all = (1 << c) - 1;

                vector<int> representative(1 << c, -1);
                vector<pair<int, int>> tests;

                for (int row = 0; row < m; ++row) {
                    const uint64_t* pattern =
                        patterns.data() + static_cast<size_t>(row) * words;

                    int mask = 0;
                    for (int j = 0; j < c; ++j) {
                        int slot = slots[j];
                        if ((pattern[slot / 64] >> (slot % 64)) & 1)
                            mask |= 1 << j;
                    }

                    if (mask == 0 || mask == all) continue;

                    int canonical = min(mask, all ^ mask);
                    if (representative[canonical] == -1) {
                        representative[canonical] = row;
                        tests.emplace_back(mask, row);
                    }
                }

                vector<int> depth(1 << c, 100);
                vector<int> totalCost(1 << c, INT_MAX / 4);
                vector<int> chosenTest(1 << c, -1);

                depth[0] = 0;
                totalCost[0] = 0;

                for (int subset = 1; subset <= all; ++subset) {
                    if ((subset & (subset - 1)) == 0) {
                        depth[subset] = 0;
                        totalCost[subset] = 0;
                        continue;
                    }

                    for (int t = 0; t < static_cast<int>(tests.size()); ++t) {
                        int a = subset & tests[t].first;
                        int b = subset ^ a;
                        if (a == 0 || b == 0) continue;

                        int d = 1 + max(depth[a], depth[b]);
                        int cost =
                            popcount(static_cast<unsigned>(subset))
                            + totalCost[a] + totalCost[b];

                        if (d < depth[subset] ||
                            (d == depth[subset] &&
                             cost < totalCost[subset])) {
                            depth[subset] = d;
                            totalCost[subset] = cost;
                            chosenTest[subset] = t;
                        }
                    }
                }

                int subset = all;
                while ((subset & (subset - 1)) != 0) {
                    int t = chosenTest[subset];
                    if (t == -1) exit(0);

                    int response = queryRow(tests[t].second);
                    if (response)
                        subset &= tests[t].first;
                    else
                        subset &= all ^ tests[t].first;
                }

                insertionRank =
                    slots[countr_zero(static_cast<unsigned>(subset))];
            }

            order.insert(order.begin() + insertionRank, newPosition);

            if (insertLeft)
                --L;
            else
                ++R;
        }

        vector<int> answer(n + 1);
        for (int i = 0; i < n; ++i)
            answer[order[i]] = i + 1;

        cout << '!';
        for (int i = 1; i <= n; ++i)
            cout << ' ' << answer[i];
        cout << endl;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n) || n < 1) return 0;

    Solver solver(n);
    solver.run();
    return 0;
}

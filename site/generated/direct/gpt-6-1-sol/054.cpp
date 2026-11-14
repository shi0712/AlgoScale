#include <bits/stdc++.h>
using namespace std;

using u32 = uint32_t;
using u64 = uint64_t;

static constexpr u32 MOD = 998244353;
static constexpr int MAXN = 405;
static constexpr int WORDS = 7;

// weight[r][l] is the weight of the gap between selected positions l and r.
static u32 weight[MAXN][MAXN];

static inline u32 addmod(u32 a, u32 b) {
    u32 s = a + b;
    if (s >= MOD) s -= MOD;
    return s;
}

static inline u32 mulmod(u32 a, u32 b) {
    return static_cast<u64>(a) * b % MOD;
}

using Mask = array<u64, WORDS>;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    cin >> n;

    vector<int> a(n + 2), b(n + 2);
    for (int i = 1; i <= n; ++i) {
        cin >> a[i] >> b[i];
    }

    /*
       Construct gap weights.

       For every l < r, let
           E(l,r) = { i : l < i < r, a[i] > l, b[i] < r }.

       The weights are chosen so that the total weight of all increasing
       paths from l to r whose internal vertices belong to E(l,r) equals 1.
    */
    int positions[MAXN];
    u32 pathValue[MAXN];

    for (int len = 1; len < n; ++len) {
        for (int l = 1; l + len <= n; ++l) {
            int r = l + len;
            int m = 0;

            for (int i = l + 1; i < r; ++i) {
                if (a[i] > l && b[i] < r) {
                    positions[m++] = i;
                }
            }

            for (int k = 0; k < m; ++k) {
                int i = positions[k];
                u64 acc = weight[i][l];
                int cnt = 0;

                for (int j = 0; j < k; ++j) {
                    acc += static_cast<u64>(pathValue[j])
                         * weight[i][positions[j]];

                    // Sixteen products fit safely in uint64_t.
                    if (++cnt == 16) {
                        acc %= MOD;
                        cnt = 0;
                    }
                }

                pathValue[k] = acc % MOD;
            }

            u64 acc = 0;
            int cnt = 0;

            for (int k = 0; k < m; ++k) {
                acc += static_cast<u64>(pathValue[k])
                     * weight[r][positions[k]];

                if (++cnt == 16) {
                    acc %= MOD;
                    cnt = 0;
                }
            }

            u32 otherPaths = acc % MOD;
            weight[r][l] = (otherPaths <= 1)
                         ? 1 - otherPaths
                         : MOD + 1 - otherPaths;
        }
    }

    /*
       DP state:
         last = last selected position,
         pending = left endpoint of the earliest cut not yet crossed
                   by a right-to-left edge; 0 means no such cut,
         reach = maximum b[i] among selected positions.

       Only states with pending < last and reach >= last are stored.
    */
    vector<size_t> offset(n + 2);
    vector<int> rowBase(n + 2);

    size_t totalValues = n + 1; // Padding for shifted row pointers.
    int totalRows = 0;

    for (int p = 1; p <= n; ++p) {
        offset[p] = totalValues;
        rowBase[p] = totalRows;
        totalValues += static_cast<size_t>(p) * (n - p + 1);
        totalRows += p;
    }

    vector<u32> dp(totalValues, 0);
    vector<Mask> masks(totalRows);
    for (auto &mask : masks) mask.fill(0);

    auto rowPointer = [&](int p, int pending) -> u32* {
        // The pointer is shifted so that row[reach] accesses the state.
        return dp.data() + offset[p]
             + static_cast<size_t>(pending) * (n - p + 1) - p;
    };

    auto mark = [&](int row, int reach) {
        int bit = reach - 1;
        masks[row][bit >> 6] |= 1ULL << (bit & 63);
    };

    for (int p = 1; p <= n; ++p) {
        rowPointer(p, 0)[b[p]] = 1;
        mark(rowBase[p], b[p]);
    }

    vector<int> futureMin(n + 2, n + 1);
    for (int i = n; i >= 1; --i) {
        futureMin[i] = min(a[i], futureMin[i + 1]);
    }

    const int usedWords = (n + 63) / 64;
    u32 answer = 0;

    vector<u32> suffix;
    vector<Mask> suffixMask;
    vector<u32> resolved(n + 1);

    for (int p = 1; p <= n; ++p) {
        u32* noPending = rowPointer(p, 0);
        const Mask& noPendingMask = masks[rowBase[p]];

        // Every state without a pending cut is a valid finished set.
        for (int word = 0; word < usedWords; ++word) {
            u64 bits = noPendingMask[word];
            while (bits) {
                int bit = __builtin_ctzll(bits);
                bits &= bits - 1;
                int reach = word * 64 + bit + 1;
                answer = addmod(answer, noPending[reach]);
            }
        }

        if (p == n) continue;

        /*
           suffix[t][reach] sums states with pending >= t.
           This combines all states whose pending cuts are resolved by
           the next selected vertex.
        */
        suffix.assign(static_cast<size_t>(p + 1) * (n + 1), 0);
        suffixMask.assign(p + 1, Mask{});

        for (int t = p - 1; t >= 1; --t) {
            u32* cur = suffix.data() + static_cast<size_t>(t) * (n + 1);
            u32* next = cur + (n + 1);
            u32* source = rowPointer(p, t);

            for (int reach = b[p]; reach <= n; ++reach) {
                cur[reach] = addmod(next[reach], source[reach]);
            }

            for (int word = 0; word < usedWords; ++word) {
                suffixMask[t][word] =
                    suffixMask[t + 1][word] |
                    masks[rowBase[p] + t][word];
            }
        }

        /*
           Append q to all states in one source row.

           The new reach is max(old reach, b[q]).
           Sources with old reach <= b[q] share one destination and
           are summed before multiplication.
        */
        auto transfer = [&](const u32* source, const Mask& sourceMask,
                            int q, int newPending, u32 gapWeight) {
            u32* destination = rowPointer(q, newPending);
            int destinationRow = rowBase[q] + newPending;

            u32 collapsed = 0;
            int firstWord = (q - 1) >> 6;
            int firstBit = (q - 1) & 63;

            for (int word = firstWord; word < usedWords; ++word) {
                u64 bits = sourceMask[word];
                if (word == firstWord) bits &= (~0ULL << firstBit);

                while (bits) {
                    int bit = __builtin_ctzll(bits);
                    bits &= bits - 1;
                    int reach = word * 64 + bit + 1;

                    u32 value = source[reach];
                    if (value == 0) continue;

                    if (reach <= b[q]) {
                        collapsed = addmod(collapsed, value);
                    } else {
                        u32 contribution = mulmod(value, gapWeight);
                        if (contribution == 0) continue;

                        destination[reach] =
                            addmod(destination[reach], contribution);
                        mark(destinationRow, reach);
                    }
                }
            }

            if (collapsed != 0) {
                u32 contribution = mulmod(collapsed, gapWeight);
                if (contribution != 0) {
                    destination[b[q]] =
                        addmod(destination[b[q]], contribution);
                    mark(destinationRow, b[q]);
                }
            }
        };

        for (int q = p + 1; q <= n; ++q) {
            u32 gapWeight = weight[q][p];
            if (gapWeight == 0) continue;

            if (a[q] <= p) {
                // No pending cut, or pending >= a[q], becomes resolved.
                const u32* sumRow =
                    suffix.data() + static_cast<size_t>(a[q]) * (n + 1);

                Mask combinedMask{};
                for (int word = 0; word < usedWords; ++word) {
                    combinedMask[word] =
                        noPendingMask[word] | suffixMask[a[q]][word];
                }

                for (int word = 0; word < usedWords; ++word) {
                    u64 bits = combinedMask[word];
                    while (bits) {
                        int bit = __builtin_ctzll(bits);
                        bits &= bits - 1;
                        int reach = word * 64 + bit + 1;
                        resolved[reach] =
                            addmod(noPending[reach], sumRow[reach]);
                    }
                }

                transfer(resolved.data(), combinedMask, q, 0, gapWeight);
            } else if (p >= futureMin[q + 1]) {
                // The new cut remains pending at position p.
                transfer(noPending, noPendingMask, q, p, gapWeight);
            }

            // Pending cuts smaller than a[q] are not resolved.
            // If no future vertex can resolve one, discard that state.
            int firstPending = max(1, futureMin[q + 1]);
            int lastPending = min(p - 1, a[q] - 1);

            for (int t = firstPending; t <= lastPending; ++t) {
                transfer(rowPointer(p, t), masks[rowBase[p] + t],
                         q, t, gapWeight);
            }
        }
    }

    cout << answer << '\n';
    return 0;
}

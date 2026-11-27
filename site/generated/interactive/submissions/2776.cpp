#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto printAnswer = [&](int count, const vector<int>& p) {
        cout << "!\n";
        cout << count << '\n';
        for (int i = 0; i < n; ++i) {
            if (i) cout << ' ';
            cout << p[i];
        }
        cout << '\n' << flush;
    };

    if (n == 1) {
        printAnswer(1, vector<int>{0});
        return 0;
    }

    if (n == 2) {
        printAnswer(2, vector<int>{0, 1});
        return 0;
    }

    auto ask = [&](int i, int j) -> int {
        cout << "? " << i << ' ' << j << '\n' << flush;
        int response;
        if (!(cin >> response) || response < 0) exit(0);
        return response;
    };

    // P[i] = p[i] XOR b[0].
    vector<int> P(n);
    for (int i = 0; i < n; ++i) {
        P[i] = ask(i, 0);
    }

    // The XOR translations preserving [0, n) are exactly [0, lowbit(n)).
    const int lowbit = n & (-n);

    // H is the stabilizer of p under valid XOR translations.
    vector<int> H;
    for (int d = 0; d < lowbit; ++d) {
        bool good = true;
        for (int i = 0; i < n; ++i) {
            if (P[i ^ d] != (P[i] ^ d)) {
                good = false;
                break;
            }
        }
        if (good) H.push_back(d);
    }

    unordered_set<int> hset;
    hset.reserve(2 * H.size() + 1);
    for (int d : H) hset.insert(d);

    // Find all shifts compatible with the queried column.
    vector<int> candidates;
    for (int z = 0; z < n; ++z) {
        if (P[z] != z) continue;

        bool valid = true;
        if (lowbit != n) {
            for (int i = 0; i < n; ++i) {
                if ((P[i] ^ z) >= n) {
                    valid = false;
                    break;
                }
            }
        }

        if (valid) candidates.push_back(z);
    }

    // Candidates in the same coset of H induce identical query matrices.
    vector<int> groupId(candidates.size(), -1);
    vector<int> groupShift;

    for (int i = 0; i < (int)candidates.size(); ++i) {
        if (groupId[i] != -1) continue;

        int g = (int)groupShift.size();
        groupShift.push_back(candidates[i]);
        groupId[i] = g;

        for (int j = i + 1; j < (int)candidates.size(); ++j) {
            if (groupId[j] == -1 &&
                hset.count(candidates[i] ^ candidates[j])) {
                groupId[j] = g;
            }
        }
    }

    const int groups = (int)groupShift.size();
    if (groups == 0) return 0;

    // inverse[g][j] is the inverse permutation of P XOR groupShift[g].
    vector<int> inverse((size_t)groups * n);
    for (int g = 0; g < groups; ++g) {
        int* row = inverse.data() + (size_t)g * n;
        int z = groupShift[g];
        for (int i = 0; i < n; ++i) {
            row[P[i] ^ z] = i;
        }
    }

    vector<int> active(groups);
    iota(active.begin(), active.end(), 0);

    auto filterGroups = [&](int j, int response) {
        vector<int> next;
        next.reserve(active.size());
        for (int g : active) {
            int prediction =
                (P[0] ^ groupShift[g]) ^ inverse[(size_t)g * n + j];
            if (prediction == response) next.push_back(g);
        }
        active.swap(next);
    };

    // Prefer columns whose predictions divide the remaining groups well.
    long long searchBudget = 50'000'000;
    vector<int> seen(8192);

    while (active.size() > 1) {
        long long cost = 1LL * n * (long long)active.size();

        if (cost <= searchBudget) {
            searchBudget -= cost;
            fill(seen.begin(), seen.end(), -1);

            int bestColumn = -1;
            int bestCount = -1;

            for (int j = 0; j < n; ++j) {
                int distinct = 0;
                for (int g : active) {
                    int prediction =
                        (P[0] ^ groupShift[g]) ^ inverse[(size_t)g * n + j];
                    if (seen[prediction] != j) {
                        seen[prediction] = j;
                        ++distinct;
                    }
                }

                if (distinct > bestCount) {
                    bestCount = distinct;
                    bestColumn = j;
                }
                if (bestCount == (int)active.size()) break;
            }

            int response = ask(0, bestColumn);
            filterGroups(bestColumn, response);
        } else {
            int g1 = active[0];
            int g2 = active[1];
            int d = groupShift[g1] ^ groupShift[g2];

            int column = 0;
            while (column < n) {
                int difference =
                    inverse[(size_t)g1 * n + column] ^
                    inverse[(size_t)g2 * n + column];
                if (difference != d) break;
                ++column;
            }
            if (column == n) return 0;

            int response = ask(0, column);
            filterGroups(column, response);
        }
    }

    if (active.empty()) return 0;

    int chosenGroup = active[0];
    int chosenShift = groupShift[chosenGroup];

    vector<int> answer(n);
    for (int i = 0; i < n; ++i) {
        answer[i] = P[i] ^ chosenShift;
    }

    printAnswer((int)H.size(), answer);
    return 0;
}

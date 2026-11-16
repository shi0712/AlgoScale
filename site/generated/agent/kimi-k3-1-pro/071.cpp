#include <bits/stdc++.h>
using namespace std;

int n;
int queries = 0;

int ask(int l, int r, int internalParity, int suffixLength) {
    cout << "? " << l << ' ' << r << '\n';
    cout.flush();
    ++queries;
    int response;
    if (!(cin >> response) || response == -1) exit(0);
    return response ^ internalParity ^ (suffixLength & 1);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;

    vector<vector<unsigned char>> inv(n + 2, vector<unsigned char>(n + 2, 0));
    vector<int> rankAt(n + 2, 0);
    vector<int> sortedPositions;
    sortedPositions.reserve(n);

    for (int k = 0; k < n; ++k) {
        int newPosition = k + 1;
        int newRank = 0;

        if (k > 0) {
            vector<int> reversedRanks(k);
            for (int m = 1; m <= k; ++m)
                reversedRanks[m - 1] = rankAt[k - m + 1];

            vector<int> candidates(k + 1);
            iota(candidates.begin(), candidates.end(), 0);

            while (candidates.size() > 1) {
                int s = (int)candidates.size();
                int words = (s + 63) >> 6;
                unsigned long long lastMask = (s & 63)
                    ? ((1ULL << (s & 63)) - 1)
                    : ~0ULL;

                vector<int> flipStart(k);
                int ptr = 0;
                for (int rank = 0; rank < k; ++rank) {
                    while (ptr < s && candidates[ptr] <= rank) ++ptr;
                    flipStart[rank] = ptr;
                }

                vector<unsigned long long> signature(words, 0), bestSignature;
                int bestBalance = s + 1;
                int bestM = 0;
                int ones = 0;

                for (int m = 1; m <= k; ++m) {
                    int first = flipStart[reversedRanks[m - 1]];
                    if (first < s) {
                        int firstWord = first >> 6;
                        int offset = first & 63;
                        for (int w = firstWord; w < words; ++w) {
                            unsigned long long mask = ~0ULL;
                            if (w == firstWord) mask <<= offset;
                            if (w == words - 1) mask &= lastMask;
                            unsigned long long oldWord = signature[w];
                            unsigned long long newWord = oldWord ^ mask;
                            signature[w] = newWord;
                            ones += __builtin_popcountll(newWord)
                                  - __builtin_popcountll(oldWord);
                        }
                    }

                    int balance = max(ones, s - ones);
                    if (balance < bestBalance) {
                        bestBalance = balance;
                        bestM = m;
                        bestSignature = signature;
                    }
                }

                int l = k - bestM + 1;
                int response = ask(l, newPosition, inv[l][k], bestM);

                vector<int> nextCandidates;
                nextCandidates.reserve(bestBalance);
                for (int i = 0; i < s; ++i) {
                    int bit = (bestSignature[i >> 6] >> (i & 63)) & 1ULL;
                    if (bit == response) nextCandidates.push_back(candidates[i]);
                }
                candidates.swap(nextCandidates);
            }
            newRank = candidates[0];
        }

        sortedPositions.insert(sortedPositions.begin() + newRank, newPosition);
        for (int pos = 1; pos < newPosition; ++pos) {
            if (rankAt[pos] >= newRank) ++rankAt[pos];
        }
        rankAt[newPosition] = newRank;

        int greaterParity = 0;
        inv[newPosition][newPosition] = 0;
        for (int l = newPosition - 1; l >= 1; --l) {
            if (rankAt[l] > newRank) greaterParity ^= 1;
            inv[l][newPosition] = inv[l][newPosition - 1] ^ greaterParity;
        }
    }

    vector<int> answer(n + 1);
    for (int value = 0; value < n; ++value)
        answer[sortedPositions[value]] = value + 1;

    cout << '!';
    for (int pos = 1; pos <= n; ++pos) cout << ' ' << answer[pos];
    cout << '\n';
    cout.flush();
    return 0;
}

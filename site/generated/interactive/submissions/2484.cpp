#include <bits/stdc++.h>
using namespace std;

using i128 = __int128_t;

struct Score {
    i128 weight;
    int candidates;
    int states;

    bool operator<(const Score& other) const {
        if (weight != other.weight) return weight < other.weight;
        if (candidates != other.candidates) return candidates < other.candidates;
        return states < other.states;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    // 0: no history, 1/2: honest run of 1/2, 3/4: dishonest run of 1/2.
    const int nextHistory[5][2] = {
        {1, 3}, {2, 3}, {-1, 3}, {1, 4}, {1, -1}
    };

    vector<array<i128, 5>> ways(100);
    for (int h = 0; h < 5; ++h) ways[0][h] = 1;
    for (int r = 1; r < 100; ++r) {
        for (int h = 0; h < 5; ++h) {
            ways[r][h] = 0;
            for (int d = 0; d < 2; ++d) {
                int nh = nextHistory[h][d];
                if (nh != -1) ways[r][h] += ways[r - 1][nh];
            }
        }
    }

    int remaining = 0;
    i128 powerOfTwo = 1;
    while ((i128)n * ways[remaining][0] > 2 * powerOfTwo) {
        ++remaining;
        powerOfTwo *= 2;
    }
    ++remaining;

    vector<unsigned char> masks(n, 1);
    int candidateCount = n;

    vector<i128> prefixWeight(n + 1);
    vector<int> prefixStates(n + 1), prefixCandidates(n + 1);

    auto ceilHalf = [](i128 value) -> i128 {
        if (value >= 0) return (value + 1) / 2;
        return -((-value) / 2);
    };

    while (candidateCount > 2) {
        if (remaining == 0) remaining = 8;

        i128 totalWeight = 0, baseWeight = 0;
        int totalStates = 0, baseStates = 0;
        int totalCandidates = 0, baseCandidates = 0;

        prefixWeight[0] = 0;
        prefixStates[0] = 0;
        prefixCandidates[0] = 0;

        for (int x = 0; x < n; ++x) {
            i128 outsideWeight = 0, insideWeight = 0;
            int outsideStates = 0, insideStates = 0;
            int outsideCandidate = 0, insideCandidate = 0;

            for (int h = 0; h < 5; ++h) {
                if (!(masks[x] & (1u << h))) continue;
                totalWeight += ways[remaining][h];
                for (int d = 0; d < 2; ++d) {
                    int nh = nextHistory[h][d];
                    if (nh == -1) continue;
                    ++totalStates;
                    if (d == 0) {
                        outsideWeight += ways[remaining - 1][nh];
                        ++outsideStates;
                        outsideCandidate = 1;
                    } else {
                        insideWeight += ways[remaining - 1][nh];
                        ++insideStates;
                        insideCandidate = 1;
                    }
                }
            }

            baseWeight += outsideWeight;
            baseStates += outsideStates;
            baseCandidates += outsideCandidate;
            totalCandidates += outsideCandidate + insideCandidate;

            prefixWeight[x + 1] = prefixWeight[x] + insideWeight - outsideWeight;
            prefixStates[x + 1] = prefixStates[x] + insideStates - outsideStates;
            prefixCandidates[x + 1] = prefixCandidates[x] + insideCandidate - outsideCandidate;
        }

        const i128 targetTwice = totalWeight - 2 * baseWeight;
        set<pair<i128, int>> starts;
        starts.insert({0, 0});

        Score bestScore{numeric_limits<i128>::max(), numeric_limits<int>::max(),
                        numeric_limits<int>::max()};
        int bestL = 0, bestR = n - 1;

        auto consider = [&](int l, int r) {
            i128 weight0 = baseWeight + prefixWeight[r + 1] - prefixWeight[l];
            int states0 = baseStates + prefixStates[r + 1] - prefixStates[l];
            int candidates0 = baseCandidates + prefixCandidates[r + 1] - prefixCandidates[l];
            Score score{max(weight0, totalWeight - weight0),
                        max(candidates0, totalCandidates - candidates0),
                        max(states0, totalStates - states0)};
            if (score < bestScore) {
                bestScore = score;
                bestL = l;
                bestR = r;
            }
        };

        for (int r = 0; r < n; ++r) {
            i128 desiredStart = ceilHalf(2 * prefixWeight[r + 1] - targetTwice);
            auto it = starts.lower_bound({desiredStart, -1});
            if (it != starts.end()) consider(it->second, r);
            if (it != starts.begin()) {
                --it;
                consider(it->second, r);
            }
            starts.insert({prefixWeight[r + 1], r + 1});
        }

        cout << "? " << bestL + 1 << ' ' << bestR + 1 << '\n' << flush;
        int raised;
        if (!(cin >> raised)) return 0;
        int length = bestR - bestL + 1;
        int answer = length - raised;
        if (answer < 0 || answer > 1) return 0;

        candidateCount = 0;
        for (int x = 0; x < n; ++x) {
            if (!masks[x]) continue;
            bool inside = bestL <= x && x <= bestR;
            int d = answer ^ int(inside);
            unsigned char updated = 0;
            for (int h = 0; h < 5; ++h) {
                if (!(masks[x] & (1u << h))) continue;
                int nh = nextHistory[h][d];
                if (nh != -1) updated |= 1u << nh;
            }
            masks[x] = updated;
            candidateCount += updated != 0;
        }
        --remaining;
    }

    vector<int> answer;
    for (int x = 0; x < n; ++x) {
        if (masks[x]) answer.push_back(x + 1);
    }
    if (answer.empty()) return 0;

    for (int x : answer) {
        cout << "! " << x << '\n' << flush;
        int verdict;
        if (!(cin >> verdict)) return 0;
        if (verdict == 1) break;
    }
    cout << "#\n" << flush;
    return 0;
}

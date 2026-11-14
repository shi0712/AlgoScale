#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>
#include <vector>

using namespace std;

int n, k;
int actions = 0;

bool rotateWheel(int i, int j) {
    if (j == 0) return true;
    cout << i + 1 << ' ' << j << endl;
    ++actions;
    if (!(cin >> k)) return false;
    return k != 1;
}

bool aperiodic(const vector<bool>& s) {
    for (int h = 1; h < n; ++h) {
        bool same = true;
        for (int x = 0; x < n; ++x) {
            if (s[(x + h) % n] != s[x]) {
                same = false;
                break;
            }
        }
        if (same) return false;
    }
    return true;
}

int responseIndex(bool duplicated, bool occupied) {
    if (duplicated) return occupied ? 1 : 2;
    return occupied ? 0 : 1;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (!(cin >> k)) return 0;

    int initialK = k;
    vector<int> responses;
    responses.reserve(n - 1);
    for (int j = 1; j < n; ++j) {
        if (!rotateWheel(0, 1)) return 0;
        responses.push_back(k);
    }
    if (!rotateWheel(0, 1)) return 0;

    bool referenceDuplicated = false;
    for (int r : responses) {
        if (r == initialK + 1) referenceDuplicated = true;
    }

    vector<bool> occupied(n, false);
    occupied[0] = true;
    for (int j = 1; j < n; ++j) {
        int r = responses[j - 1];
        if ((referenceDuplicated && r == initialK) ||
            (!referenceDuplicated && r == initialK - 1)) {
            occupied[j] = true;
        }
    }

    int target = 0;
    if (!aperiodic(occupied)) {
        if (referenceDuplicated) {
            int u = 0;
            while (occupied[u]) ++u;
            if (!rotateWheel(0, u)) return 0;
            occupied[u] = true;
            target = u;
        } else {
            int v = 1;
            while (!occupied[v]) ++v;
            if (!rotateWheel(0, v)) return 0;
            occupied[0] = false;
            target = v;
        }
    }

    for (int i = 1; i < n; ++i) {
        int temporary = -1;
        if (!aperiodic(occupied)) {
            int u = 0;
            while (occupied[u]) ++u;
            if (!rotateWheel(0, u - target)) return 0;
            occupied[u] = true;
            temporary = u;
        }

        vector<bool> baseOccupied = occupied;
        int baseK = k;
        struct Candidate {
            int x;
            bool duplicated;
        };
        vector<Candidate> candidates;
        for (int x = 0; x < n; ++x) {
            if (baseOccupied[x]) {
                candidates.push_back({x, false});
                candidates.push_back({x, true});
            }
        }

        vector<bool> used(n, false);
        used[0] = true;
        int current = 0;
        auto identified = [&]() {
            return all_of(candidates.begin(), candidates.end(), [&](const Candidate& c) {
                return c.x == candidates[0].x;
            });
        };

        while (!identified()) {
            int bestQ = -1;
            tuple<int, int, int> bestScore{n + 1, n + 1, n + 1};
            for (int q = 1; q < n; ++q) {
                if (used[q]) continue;
                array<int, 3> count{};
                array<vector<bool>, 3> seen;
                for (auto& v : seen) v.assign(n, false);
                for (const auto& c : candidates) {
                    bool occ = baseOccupied[(c.x + q) % n];
                    int idx = responseIndex(c.duplicated, occ);
                    ++count[idx];
                    seen[idx][c.x] = true;
                }
                int largest = 0;
                int mostPositions = 0;
                for (int z = 0; z < 3; ++z) {
                    largest = max(largest, count[z]);
                    mostPositions = max(mostPositions,
                                        static_cast<int>(count_if(seen[z].begin(), seen[z].end(),
                                                                  [](bool v) { return v; })));
                }
                tuple<int, int, int> score{largest, mostPositions, q};
                if (score < bestScore) {
                    bestScore = score;
                    bestQ = q;
                }
            }

            used[bestQ] = true;
            if (!rotateWheel(i, bestQ - current)) return 0;
            current = bestQ;
            vector<Candidate> filtered;
            for (const auto& c : candidates) {
                bool occ = baseOccupied[(c.x + current) % n];
                int idx = responseIndex(c.duplicated, occ);
                int predicted = baseK + idx - 1;
                if (predicted == k) filtered.push_back(c);
            }
            candidates = move(filtered);
        }

        int x = candidates[0].x;
        bool duplicated = candidates[0].duplicated;
        int finalRotation = target - (x + current) % n;
        if (!rotateWheel(i, finalRotation)) return 0;
        if (!duplicated) occupied[x] = false;

        if (temporary != -1) {
            if (!rotateWheel(0, target - temporary)) return 0;
            occupied[temporary] = false;
        }
    }

    return 0;
}

#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n, K;
    if (!(cin >> n)) return 0;
    if (!(cin >> K)) return 0;
    if (K <= 1) return 0;

    auto action = [&](int wheel, int rotation) -> int {
        cout << wheel << ' ' << rotation << endl;

        int response;
        if (!(cin >> response)) exit(0);
        if (response < 0) exit(0);
        if (response == 1) exit(0);

        return response;
    };

    auto aperiodic = [&](const vector<char>& occupied) -> bool {
        for (int shift = 1; shift < n; ++shift) {
            bool same = true;
            for (int x = 0; x < n; ++x) {
                if (occupied[x] != occupied[(x + shift) % n]) {
                    same = false;
                    break;
                }
            }
            if (same) return false;
        }
        return true;
    };

    const int pivot = 1;
    vector<char> occupied(n, 0);

    // Find a wheel belonging to a group of at least two wheels.
    // At the same time, learn the set of occupied relative positions.
    while (true) {
        vector<int> response(n, -1);
        bool merged = false;

        for (int delta = 1; delta < n; ++delta) {
            int r = action(pivot, delta);

            if (r == K - 1) {
                // The pivot was a singleton and has just joined another group.
                K = r;
                merged = true;
                break;
            }

            response[delta] = r;
            action(pivot, -delta);
        }

        if (merged) continue;

        occupied.assign(n, 0);
        occupied[0] = 1;
        for (int delta = 1; delta < n; ++delta) {
            occupied[delta] = (response[delta] == K);
        }
        break;
    }

    int target = 0;

    // A periodic occupied set can make different relative positions
    // indistinguishable. Add one fresh position to destroy all symmetry.
    if (!aperiodic(occupied)) {
        int chosen = -1;

        for (int c = 0; c < n; ++c) {
            if (occupied[c]) continue;

            vector<char> trial = occupied;
            trial[c] = 1;

            if (aperiodic(trial)) {
                chosen = c;
                break;
            }
        }

        if (chosen == -1) return 0;

        K = action(pivot, chosen);
        occupied[chosen] = 1;
        target = chosen;
    }

    vector<int> positions;
    for (int x = 0; x < n; ++x) {
        if (occupied[x]) positions.push_back(x);
    }

    // Find a small collection of shifts whose occupancy signatures
    // distinguish every occupied relative position.
    vector<int> shifts;
    vector<vector<int>> signatures(positions.size());

    while (true) {
        set<vector<int>> distinct(signatures.begin(), signatures.end());
        if ((int)distinct.size() == (int)positions.size()) break;

        int bestShift = -1;
        size_t bestCount = distinct.size();

        for (int delta = 1; delta < n; ++delta) {
            if (find(shifts.begin(), shifts.end(), delta) != shifts.end()) {
                continue;
            }

            vector<vector<int>> trial = signatures;
            for (int i = 0; i < (int)positions.size(); ++i) {
                trial[i].push_back(
                    occupied[(positions[i] + delta) % n] ? 1 : 0
                );
            }

            set<vector<int>> trialDistinct(trial.begin(), trial.end());
            if (trialDistinct.size() > bestCount) {
                bestCount = trialDistinct.size();
                bestShift = delta;
            }
        }

        if (bestShift == -1) {
            for (int delta = 1; delta < n; ++delta) {
                if (find(shifts.begin(), shifts.end(), delta) == shifts.end()) {
                    bestShift = delta;
                    break;
                }
            }
        }

        shifts.push_back(bestShift);
        for (int i = 0; i < (int)positions.size(); ++i) {
            signatures[i].push_back(
                occupied[(positions[i] + bestShift) % n] ? 1 : 0
            );
        }
    }

    vector<int> coordinate(n, -1);
    coordinate[pivot] = target;

    for (int wheel = 1; wheel <= n; ++wheel) {
        if (wheel == pivot) continue;

        vector<int> responses;
        responses.reserve(shifts.size());

        for (int delta : shifts) {
            int r = action(wheel, delta);
            action(wheel, -delta);
            responses.push_back(r);
        }

        int candidate[2] = {-1, -1};

        for (int multiplicity = 0; multiplicity <= 1; ++multiplicity) {
            for (int i = 0; i < (int)positions.size(); ++i) {
                bool valid = true;

                for (int j = 0; j < (int)shifts.size(); ++j) {
                    int occupiedTarget =
                        occupied[(positions[i] + shifts[j]) % n] ? 1 : 0;
                    int inferred = K + multiplicity - responses[j];

                    if (inferred < 0 || inferred > 1 ||
                        inferred != occupiedTarget) {
                        valid = false;
                        break;
                    }
                }

                if (valid) {
                    candidate[multiplicity] = positions[i];
                    break;
                }
            }
        }

        if (candidate[0] == -1 && candidate[1] == -1) {
            return 0;
        }

        if (candidate[0] != -1 && candidate[1] != -1) {
            int distinguishingShift = -1;

            for (int delta = 1; delta < n; ++delta) {
                int q0 = occupied[(candidate[0] + delta) % n] ? 1 : 0;
                int q1 = occupied[(candidate[1] + delta) % n] ? 1 : 0;

                int predicted0 = K - q0;
                int predicted1 = K + 1 - q1;

                if (predicted0 != predicted1) {
                    distinguishingShift = delta;
                    break;
                }
            }

            if (distinguishingShift == -1) return 0;

            int observed;
            int index = int(find(shifts.begin(), shifts.end(),
                                 distinguishingShift) - shifts.begin());

            if (index < (int)shifts.size()) {
                observed = responses[index];
            } else {
                observed = action(wheel, distinguishingShift);
                action(wheel, -distinguishingShift);
            }

            int q0 = occupied[(candidate[0] + distinguishingShift) % n] ? 1 : 0;
            int q1 = occupied[(candidate[1] + distinguishingShift) % n] ? 1 : 0;

            if (observed == K - q0) {
                coordinate[wheel] = candidate[0];
            } else {
                coordinate[wheel] = candidate[1];
            }
        } else if (candidate[0] != -1) {
            coordinate[wheel] = candidate[0];
        } else {
            coordinate[wheel] = candidate[1];
        }
    }

    // Move every wheel directly to the target position.
    for (int wheel = 1; wheel <= n; ++wheel) {
        if (wheel == pivot) continue;

        int rotation = (target - coordinate[wheel] + n) % n;
        if (rotation != 0) {
            action(wheel, rotation);
        }
    }

    return 0;
}

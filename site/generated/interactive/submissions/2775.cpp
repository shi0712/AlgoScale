#include <bits/stdc++.h>
using namespace std;

class CaveSolver {
    int n;
    vector<int> remaining, position;
    vector<int> switchToDoor, doorToSwitch, correctPosition;
    vector<char> assigned;
    int firstUnassignedDoor = 0;
    int baseColor = 0;
    int baseResponse = -1;

    static int ceilLog2(int x) {
        int result = 0;
        while ((1LL << result) < x) ++result;
        return result;
    }

    int ask(const string& query) {
        cout << "? " << query << '\n' << flush;
        int response;
        if (!(cin >> response)) exit(0);
        return response;
    }

    void advanceFirstDoor() {
        while (firstUnassignedDoor < n &&
               doorToSwitch[firstUnassignedDoor] != -1) {
            ++firstUnassignedDoor;
        }
    }

    void removeRemaining(int sw) {
        int p = position[sw];
        int last = remaining.back();

        if (p != (int)remaining.size() - 1) {
            remaining[p] = last;
            position[last] = p;
        }

        remaining.pop_back();
        position[sw] = -1;
    }

    void assignSwitch(int sw, int door, int color) {
        assigned[sw] = true;
        correctPosition[sw] = color;
        switchToDoor[sw] = door;
        doorToSwitch[door] = sw;
        removeRemaining(sw);
    }

    int queryBaseline() {
        string query(n, '0');

        for (int i = 0; i < n; ++i) {
            if (assigned[i]) {
                query[i] = char('0' + correctPosition[i]);
            } else {
                query[i] = char('0' + baseColor);
            }
        }

        return ask(query);
    }

    int querySelected(const vector<int>& selected,
                      int selectedGuess, int otherGuess) {
        vector<char> marked(n, false);
        for (int sw : selected) marked[sw] = true;

        string query(n, '0');

        for (int i = 0; i < n; ++i) {
            if (assigned[i]) {
                query[i] = char('0' + correctPosition[i]);
            } else if (marked[i]) {
                query[i] = char('0' + selectedGuess);
            } else {
                query[i] = char('0' + otherGuess);
            }
        }

        return ask(query);
    }

    void mapBlockBinary(int blockEnd, int color) {
        int door = firstUnassignedDoor;

        while (door < blockEnd) {
            vector<int> candidates = remaining;

            while (candidates.size() > 1) {
                int half = (int)candidates.size() / 2;
                vector<int> selected(candidates.begin(),
                                     candidates.begin() + half);

                int response = querySelected(
                    selected, 1 - color, color
                );

                bool inFirstHalf = (response == door);
                vector<int> next;

                if (inFirstHalf) {
                    next.assign(selected.begin(), selected.end());
                } else {
                    next.assign(candidates.begin() + half,
                                candidates.end());
                }

                candidates.swap(next);
            }

            assignSwitch(candidates[0], door, color);
            advanceFirstDoor();
            ++door;
        }
    }

    int findMissingBlockDoor(int blockEnd) const {
        for (int door = firstUnassignedDoor; door < blockEnd; ++door) {
            if (doorToSwitch[door] == -1) return door;
        }
        return -1;
    }

    void mapBlockSingleton(int blockEnd, int color) {
        int need = blockEnd - firstUnassignedDoor;
        vector<int> snapshot = remaining;
        vector<char> tried(n, false);
        int untried = (int)remaining.size();

        for (int sw : snapshot) {
            if (need == 0) break;
            if (assigned[sw]) continue;

            if (untried == 1) {
                int missingDoor = findMissingBlockDoor(blockEnd);
                if (missingDoor != -1) {
                    assignSwitch(sw, missingDoor, color);
                    --need;
                }
                untried = 0;
                break;
            }

            tried[sw] = true;
            --untried;

            vector<int> selected = {sw};
            int response = querySelected(selected, 1 - color, color);

            bool belongsToBlock = false;
            if (response >= 0 && response < n &&
                doorToSwitch[response] == -1) {
                if (blockEnd == n) {
                    belongsToBlock = true;
                } else if (response < blockEnd) {
                    belongsToBlock = true;
                }
            }

            if (belongsToBlock) {
                assignSwitch(sw, response, color);
                --need;
            }
        }

        advanceFirstDoor();
    }

    void mapOppositeDoor(int color) {
        int door = firstUnassignedDoor;
        vector<int> candidates = remaining;
        int m = (int)candidates.size();

        if (m - 1 <= ceilLog2(m)) {
            vector<int> snapshot = candidates;
            int untried = m;

            for (int sw : snapshot) {
                if (untried == 1) {
                    assignSwitch(sw, door, color);
                    break;
                }

                --untried;
                vector<int> selected = {sw};
                int response = querySelected(selected, color, baseColor);

                if (response != door) {
                    assignSwitch(sw, door, color);
                    break;
                }
            }
        } else {
            while (candidates.size() > 1) {
                int half = (int)candidates.size() / 2;
                vector<int> selected(candidates.begin(),
                                     candidates.begin() + half);

                int response = querySelected(
                    selected, color, baseColor
                );

                bool inFirstHalf = (response != door);
                vector<int> next;

                if (inFirstHalf) {
                    next.assign(selected.begin(), selected.end());
                } else {
                    next.assign(candidates.begin() + half,
                                candidates.end());
                }

                candidates.swap(next);
            }

            assignSwitch(candidates[0], door, color);
        }

        advanceFirstDoor();
    }

public:
    explicit CaveSolver(int n)
        : n(n),
          position(n, -1),
          switchToDoor(n, -1),
          doorToSwitch(n, -1),
          correctPosition(n, -1),
          assigned(n, false) {
        remaining.resize(n);
        iota(remaining.begin(), remaining.end(), 0);
        for (int i = 0; i < n; ++i) position[i] = i;
    }

    void solve() {
        baseColor = 0;
        baseResponse = queryBaseline();

        while (!remaining.empty()) {
            advanceFirstDoor();

            if (baseResponse == -1) {
                mapBlockSingleton(n, baseColor);
                continue;
            }

            if (baseResponse > firstUnassignedDoor) {
                int blockEnd = baseResponse;
                int m = (int)remaining.size();
                int blockLength = blockEnd - firstUnassignedDoor;

                long long binaryCost = 0;
                for (int k = 0; k < blockLength; ++k) {
                    binaryCost += ceilLog2(m - k);
                }

                if (m - 1 <= binaryCost) {
                    mapBlockSingleton(blockEnd, baseColor);
                } else {
                    mapBlockBinary(blockEnd, baseColor);
                }

                continue;
            }

            int frontierColor = 1 - baseColor;
            mapOppositeDoor(frontierColor);

            if (!remaining.empty()) {
                baseColor = frontierColor;
                baseResponse = queryBaseline();
            }
        }
    }

    void printAnswer() {
        string answer(n, '0');
        for (int i = 0; i < n; ++i) {
            answer[i] = char('0' + correctPosition[i]);
        }

        cout << "! " << answer;
        for (int i = 0; i < n; ++i) {
            cout << ' ' << switchToDoor[i];
        }
        cout << '\n' << flush;
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    CaveSolver solver(n);
    solver.solve();
    solver.printAnswer();
    return 0;
}

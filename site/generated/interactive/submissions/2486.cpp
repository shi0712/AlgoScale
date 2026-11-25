#include <algorithm>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    string switches(n, '0');
    vector<int> door(n, -1);
    vector<int> candidates(n);
    vector<string> correctIfConnected(n);
    iota(candidates.begin(), candidates.end(), 0);

    auto ask = [&](int currentDoor) -> int {
        cout << "? " << switches << '\n' << flush;
        int response;
        if (!(cin >> response)) exit(0);

        int lastOpen = (response == -1 ? n - 1 : response - 1);
        for (int d = currentDoor; d <= lastOpen; ++d) {
            if (correctIfConnected[d].empty()) {
                correctIfConnected[d] = switches;
            }
        }
        if (response >= 0 && correctIfConnected[response].empty()) {
            correctIfConnected[response] = switches;
            for (char &bit : correctIfConnected[response]) bit ^= 1;
        }
        return response;
    };

    for (int currentDoor = 0; currentDoor < n; ++currentDoor) {
        if (correctIfConnected[currentDoor].empty()) ask(currentDoor);

        vector<int> group = candidates;
        const string &correct = correctIfConnected[currentDoor];
        while (group.size() > 1) {
            int half = (group.size() + 1) / 2;
            for (int i = 0; i < half; ++i) {
                switches[group[i]] = correct[group[i]] ^ 1;
            }
            for (int i = half; i < (int)group.size(); ++i) {
                switches[group[i]] = correct[group[i]];
            }

            int response = ask(currentDoor);
            if (response == currentDoor) {
                group.resize(half);
            } else {
                group.erase(group.begin(), group.begin() + half);
            }
        }

        int sw = group[0];
        switches[sw] = correct[sw];
        door[sw] = currentDoor;
        candidates.erase(find(candidates.begin(), candidates.end(), sw));
    }

    cout << "! " << switches;
    for (int d : door) cout << ' ' << d;
    cout << '\n' << flush;
    return 0;
}

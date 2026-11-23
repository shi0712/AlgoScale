#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N;
    if (!(cin >> N)) return 0;

    string state(N, '0');
    vector<int> remaining(N), doorOfSwitch(N, -1), doorBit(N, -1);
    iota(remaining.begin(), remaining.end(), 0);

    auto ask = [&]() -> int {
        cout << "? " << state << '\n' << flush;
        int result;
        if (!(cin >> result)) exit(0);
        if (result < -1 || result >= N) exit(0);
        return result;
    };

    for (int door = 0; door < N; ++door) {
        if (doorBit[door] == -1) {
            for (int s : remaining) state[s] = '0';
            int result = ask();

            if (result == -1) {
                for (int d = door; d < N; ++d) doorBit[d] = 0;
            } else {
                for (int d = door; d < result; ++d) doorBit[d] = 0;
                doorBit[result] = 1;
            }
        }

        int correct = doorBit[door];
        vector<int> candidates = remaining;

        while (candidates.size() > 1) {
            int half = static_cast<int>(candidates.size()) / 2;

            for (int s : candidates) state[s] = char('0' + (1 - correct));
            for (int j = 0; j < half; ++j) {
                state[candidates[j]] = char('0' + correct);
            }

            int result = ask();
            if (result == door) {
                candidates.erase(candidates.begin(), candidates.begin() + half);
            } else {
                candidates.resize(half);
            }
        }

        int chosen = candidates[0];
        state[chosen] = char('0' + correct);
        doorOfSwitch[chosen] = door;
        remaining.erase(find(remaining.begin(), remaining.end(), chosen));
    }

    cout << "! " << state;
    for (int s = 0; s < N; ++s) cout << ' ' << doorOfSwitch[s];
    cout << '\n' << flush;
    return 0;
}

#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    const string M = "RPS";
    int beat[3] = {1, 2, 0};

    int perm[6][3] = {
        {0, 1, 2},
        {0, 2, 1},
        {1, 0, 2},
        {1, 2, 0},
        {2, 0, 1},
        {2, 1, 0}
    };

    int streak = 0;
    int round = 1;
    bool alive = true;

    auto play = [&](int x) -> bool {
        cout << M[x] << '\n' << flush;
        string res;
        if (!(cin >> res)) {
            alive = false;
            return false;
        }
        if (res == "-1") {
            alive = false;
            return false;
        }

        bool win = (res == "player");
        if (win) ++streak;
        else streak = 0;

        ++round;
        return win;
    };

    bool w1 = play(0);
    if (!alive) return 0;

    int known = -1;

    if (w1) {
        bool w2 = play(1);
        if (!alive) return 0;
        known = w2 ? 4 : 5;
    } else {
        bool w2 = play(0);
        if (!alive) return 0;

        if (w2) {
            bool w3 = play(2);
            if (!alive) return 0;
            known = w3 ? 1 : 3;
        } else {
            bool w3 = play(0);
            if (!alive) return 0;
            bool w4 = play(1);
            if (!alive) return 0;
            known = w4 ? 0 : 2;
        }
    }

    while (streak < 10) {
        int pos = (round - 1) % 3;
        int ai = perm[known][pos];
        bool w = play(beat[ai]);
        if (!alive) return 0;
    }

    return 0;
}

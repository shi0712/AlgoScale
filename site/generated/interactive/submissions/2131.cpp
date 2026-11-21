#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int sx, sy;
    if (!(cin >> sx >> sy)) return 0;   // no initial position

    auto read_answer = []() -> pair<int,int> {
        int x, y;
        if (!(cin >> x >> y)) return {-1, -1};
        return {x, y};
    };

    // Phase 1: paint the whole first column
    for (int i = 1; i <= 1000; ++i) {
        cout << i << ' ' << 1 << "\n" << flush;
        auto [x, y] = read_answer();
        if (x == 0 && y == 0) return 0;          // robot exploded
        if (x == -1 && y == -1) return 0;        // error / closed stream
    }

    // Phase 2: paint the whole first row
    for (int j = 1; j <= 1000; ++j) {
        cout << 1 << ' ' << j << "\n" << flush;
        auto [x, y] = read_answer();
        if (x == 0 && y == 0) return 0;
        if (x == -1 && y == -1) return 0;
    }

    // Phase 3: paint the remaining cells (2..1000, 2..1000)
    for (int i = 2; i <= 1000; ++i) {
        for (int j = 2; j <= 1000; ++j) {
            cout << i << ' ' << j << "\n" << flush;
            auto [x, y] = read_answer();
            if (x == 0 && y == 0) return 0;
            if (x == -1 && y == -1) return 0;
        }
    }
    return 0;
}

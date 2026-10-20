#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;

    while (T--) {
        int n;
        cin >> n;

        int known = -1;       // a dial already placed at the final target
        bool made_all_distinct = false;

        for (int i = 1; i <= n; ++i) {
            int r = -1;
            int steps = 0;

            while (steps < n) {
                cout << "? " << i << '\n';
                cout.flush();
                cin >> r;
                if (r == -1) return 0;

                ++steps;

                if (r == 0) break;
            }

            // If the first 0 appears only after a full round, the state
            // before this move was already all distinct.
            if (steps == n && r == 0) {
                // Make dial i the known target and create one missing value.
                cout << "? " << i << '\n';
                cout.flush();
                cin >> r;
                if (r == -1) return 0;

                known = i;
                made_all_distinct = true;
                break;
            }

            // Otherwise just keep the last moved dial. If all dials become
            // distinct after the whole pass, this dial is still useful.
            known = i;
        }

        // If we did not stop early, the pass itself has made the state all distinct.
        // Move the last moved dial once to create a unique missing value and put it
        // directly onto the target.
        if (!made_all_distinct) {
            int r;
            cout << "? " << known << '\n';
            cout.flush();
            cin >> r;
            if (r == -1) return 0;
        }

        // Now 'known' is already at the common target value, and there is a
        // unique missing value. Move every other dial to the same target.
        for (int i = 1; i <= n; ++i) {
            if (i == known) continue;

            int r;
            while (true) {
                cout << "? " << i << '\n';
                cout.flush();
                cin >> r;
                if (r == -1) return 0;
                if (r == 0) break;
            }

            // Move one more step from the missing value to the target value.
            cout << "? " << i << '\n';
            cout.flush();
            cin >> r;
            if (r == -1) return 0;
        }

        cout << "!" << '\n';
        cout.flush();
    }

    return 0;
}

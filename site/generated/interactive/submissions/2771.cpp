#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    vector<int> answer(n + 1);

    auto greaterThan = [&](int x, int y) -> bool {
        cout << "? " << x << ' ' << y << '\n' << flush;

        char response;
        if (!(cin >> response) || response == '-') exit(0);
        if (response != '<' && response != '>') exit(0);

        return response == '>';
    };

    if (n == 1) {
        answer[1] = 1;
        cout << "! 1\n" << flush;
        return 0;
    }

    if (n == 2) {
        if (greaterThan(1, 2)) {
            answer[1] = 2;
            answer[2] = 1;
        } else {
            answer[1] = 1;
            answer[2] = 2;
        }
        cout << "! " << answer[1] << ' ' << answer[2] << '\n' << flush;
        return 0;
    }

    // The three candidates contain the current largest unassigned value,
    // while high and low satisfy value[high] > value[low].
    auto processTriple = [&](int value, int high, int low, int fresh)
        -> pair<int, int> {
        if (!greaterThan(fresh, low)) {
            // value[high] > value[low] > value[fresh].
            answer[high] = value;
            return {low, fresh};
        }

        // value[fresh] > value[low].
        if (greaterThan(high, fresh)) {
            // value[high] > value[fresh] > value[low].
            answer[high] = value;
            return {fresh, low};
        } else {
            // value[fresh] > value[high] > value[low].
            answer[fresh] = value;
            return {high, low};
        }
    };

    bool firstGreater = greaterThan(n - 2, n - 1);
    int high = firstGreater ? n - 2 : n - 1;
    int low = firstGreater ? n - 1 : n - 2;

    auto state = processTriple(n, high, low, n);
    high = state.first;
    low = state.second;

    for (int value = n - 1; value >= 3; --value) {
        state = processTriple(value, high, low, value - 2);
        high = state.first;
        low = state.second;
    }

    answer[high] = 2;
    answer[low] = 1;

    cout << '!';
    for (int i = 1; i <= n; ++i) {
        cout << ' ' << answer[i];
    }
    cout << '\n' << flush;

    return 0;
}

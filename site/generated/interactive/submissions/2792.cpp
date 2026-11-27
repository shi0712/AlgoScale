#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    auto move = [](char direction) {
        cout << direction << '\n' << flush;
    };

    // The footprint on the central cell is already known.
    move('^');

    char response;
    while (cin >> response) {
        if (response == 'G' || response == '-') return 0;

        if (response == '^' || response == '<' ||
            response == 'v' || response == '>') {
            move(response);
        } else {
            return 0;
        }
    }

    return 0;
}

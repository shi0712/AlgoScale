#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int n;
    if (!(cin >> n)) return 0;

    char direction = '^';
    while (true) {
        cout << direction << '\n' << flush;

        string response;
        if (!(cin >> response)) return 0;
        if (response == "G" || response == "-1") return 0;
        if (response.size() != 1) return 0;

        char c = response[0];
        if (c == '^' || c == '<' || c == 'v' || c == '>') {
            direction = c;
        } else {
            return 0;
        }
    }
}

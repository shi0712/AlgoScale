#include <iostream>

using namespace std;

int n;

int ask(int i, int j) {
    cout << "? " << i << ' ' << j << '\n' << flush;
    int response;
    if (!(cin >> response) || response == -1) exit(0);
    return response;
}

void answer(int x, int y) {
    cout << "! " << x << ' ' << y << '\n' << flush;
    int verdict;
    if (!(cin >> verdict) || verdict == -1) exit(0);
    exit(0);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    if (!(cin >> n)) return 0;
    if (n == 2) answer(1, 2);

    int x = 1, y = 2;
    int cached = ask(x, y);
    if (cached > (n - 1) / 2) answer(x, y);

    for (int i = 3; i <= n; ++i) {
        if (cached == -1) {
            cached = ask(x, y);
            if (cached > (n - 1) / 2) answer(x, y);
        }

        int current = ask(y, i);
        if (current > (n - 1) / 2) answer(y, i);

        if (cached < current) {
            x = y;
            y = i;
            cached = current;
        } else if (cached == current) {
            y = i;
            cached = -1;
        }
    }

    answer(x, y);
}

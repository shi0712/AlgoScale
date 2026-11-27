#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int cnt;
    if (!(cin >> cnt) || cnt <= 0) return 0;

    auto ask = [](long long x) -> int {
        cout << "- " << x << endl;
        int response;
        if (!(cin >> response) || response == -1) return -1;
        return response;
    };

    long long answer = 0;

    int response = ask(1);
    if (response == -1) return 0;

    int bit = response - (cnt - 1);
    if (bit < 0 || bit >= 30) return 0;
    answer |= 1LL << bit;

    for (int j = 2; j <= cnt; ++j) {
        response = ask(1LL << bit);
        if (response == -1) return 0;

        bit = response - (cnt - j);
        if (bit < 0 || bit >= 30) return 0;
        answer |= 1LL << bit;
    }

    cout << "! " << answer << endl;
    return 0;
}
